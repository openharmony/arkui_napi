/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ark_idle_monitor.h"


#include "utils/log.h"
#if defined(ENABLE_FFRT)
#include "ffrt.h"
#include "c/executor_task.h"
#endif
#ifdef ENABLE_UCOLLECTION
#include "cpu_collector_client.h"
#endif

namespace panda::ecmascript {

void ArkIdleMonitor::NotifyLooperIdleStart(int64_t timestamp, int idleTime)
{
    SetIdleState(true);
    AddIdleNotifyCount();
    if (idleTime < MIN_TRIGGER_GC_IDLE_INTERVAL || !ShouldTryTriggerGC(timestamp)) {
        return;
    }
    JSNApi::NotifyLooperIdleStart(vm_, timestamp, idleTime);
}

bool ArkIdleMonitor::ShouldTryTriggerGC(int64_t timestamp)
{
    int64_t notifyInterval = timestamp - GetNotifyTimestamp();
    recordedIdleNotifyInterval_.Push(notifyInterval);
    SetNotifyTimestamp(timestamp);
    if (notifyInterval < MIN_TRIGGER_GC_IDLE_INTERVAL ||
        recordedIdleNotifyInterval_.Count() != IDLE_CHECK_INTERVAL_LENGTH) {
        return false;
    }
    int64_t sumInterval = recordedIdleNotifyInterval_.Sum([](int64_t a, int64_t b) {return a + b;}, 0);
    int64_t averageInterval = sumInterval / recordedIdleNotifyInterval_.Count();
    if (averageInterval > MIN_TRIGGER_GC_IDLE_INTERVAL && notifyInterval < averageInterval * DOUBLE_INTERVAL_CHECK) {
        return true;
    }
    return false;
}

void ArkIdleMonitor::NotifyLooperIdleEnd(int64_t timestamp)
{
    SetIdleState(false);
    AddIdleDuration(timestamp - GetNotifyTimestamp());
    JSNApi::NotifyLooperIdleEnd(vm_, timestamp);
}

bool ArkIdleMonitor::CheckLowNotifyState() const
{
    int checkCounts = IsInBackground() ? IDLE_INBACKGROUND_CHECK_LENGTH : IDLE_CHECK_LENGTH;
    HILOG_DEBUG("ArkIdleMonitor: low Notify checkCounts '%{public}d', result '%{public}d' ",
        checkCounts, static_cast<int>(numberOfLowIdleNotifyCycles_));
    return numberOfLowIdleNotifyCycles_ >= static_cast<int64_t>(checkCounts);
}

bool ArkIdleMonitor::CheckLowRunningDurationState() const
{
    int checkCounts = IsInBackground() ? IDLE_INBACKGROUND_CHECK_LENGTH : IDLE_CHECK_LENGTH;
    HILOG_DEBUG("ArkIdleMonitor: low Duration checkCounts '%{public}d', result '%{public}d' ",
        checkCounts, static_cast<int>(numberOfHighIdleTimeRatio_));
    return numberOfHighIdleTimeRatio_ >= static_cast<int64_t>(checkCounts);
}

void ArkIdleMonitor::IntervalMonitor()
{
    auto nowTimestamp = std::chrono::time_point_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now()).time_since_epoch().count();
    if (IsIdleState()) {
        AddIdleDuration(nowTimestamp - GetNotifyTimestamp());
        SetNotifyTimestamp(nowTimestamp);
    }
    if (GetIdleNotifyCount() <= LOW_IDLE_NOTIFY_THRESHOLD) {
        numberOfLowIdleNotifyCycles_++;
    } else {
        numberOfLowIdleNotifyCycles_ = 0;
    }
    ResetIdleNotifyCount();
    int64_t recordTotalDuration = nowTimestamp - startRecordTimestamp_;
    if (recordTotalDuration <= 0) {
        numberOfHighIdleTimeRatio_ = 0;
        HILOG_ERROR("ArkIdleMonitor: recordTotalDuration <= 0");
    } else {
        double idleTimeRatio = static_cast<double>(GetTotalIdleDuration()) / recordTotalDuration;
        idleTimeRatio >= IDLE_RATIO ? numberOfHighIdleTimeRatio_++ : (numberOfHighIdleTimeRatio_ = 0);
    }
    startRecordTimestamp_ = nowTimestamp;
    ResetTotalIdleDuration();

    if (CheckLowNotifyState() && CheckLowRunningDurationState()) {
        NotifyTryCompressGC();
        PostMonitorTask(SLEEP_MONITORING_INTERVAL);
        ClearIdleStats();
    } else {
        PostMonitorTask(IDLE_MONITORING_INTERVAL);
    }
}

void ArkIdleMonitor::PostMonitorTask(uint64_t delayMs)
{
#if defined(ENABLE_FFRT)
    auto task = [](void* idleMonitorPtr) {
        if (idleMonitorPtr != nullptr) {
            ArkIdleMonitor* arkIdleMonitor = reinterpret_cast<ArkIdleMonitor *>(idleMonitorPtr);
            arkIdleMonitor->IntervalMonitor();
        }
    };
    if (waitForStopTimerHandler_ != -1) {
        int ret = ffrt_timer_stop(ffrt_qos_user_initiated, waitForStopTimerHandler_);
        if (ret != 0) {
            HILOG_ERROR("ArkIdleMonitor: ffrt_timer_stop error handler: timerHandler='%{public}d', ret='%{public}d'",
                waitForStopTimerHandler_, ret);
        }
    }
    waitForStopTimerHandler_ = currentTimerHandler_;
    currentTimerHandler_ = ffrt_timer_start(ffrt_qos_user_initiated, delayMs, this, task, false);
#endif
}

ArkIdleMonitor::~ArkIdleMonitor()
{
#if defined(ENABLE_FFRT)
    if (waitForStopTimerHandler_ != -1) {
        ffrt_timer_stop(ffrt_qos_user_initiated, waitForStopTimerHandler_);
    }
    if (currentTimerHandler_ != -1) {
        ffrt_timer_stop(ffrt_qos_user_initiated, currentTimerHandler_);
    }
#endif
}

void ArkIdleMonitor::ClearIdleStats()
{
    ResetIdleNotifyCount();
    ResetTotalIdleDuration();
    startRecordTimestamp_ = 0;
    numberOfLowIdleNotifyCycles_ = 0;
    numberOfHighIdleTimeRatio_ = 0;
}

void ArkIdleMonitor::NotifyTryCompressGC()
{
#if defined(ENABLE_EVENT_HANDLER)
    double cpuUsage = GetCpuUsage();
    if (cpuUsage >= IDLE_CPU_USAGE) {
        HILOG_INFO("ArkIdleMonitor: Sending a quiet notification is canceled due to high CPU usage: %{public}.2f",
            cpuUsage);
        return;
    }
    if (mainThreadHandler_ == nullptr) {
        mainThreadHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(
            OHOS::AppExecFwk::EventRunner::GetMainEventRunner());
    };
    auto task = [this]() {
        JSNApi::TriggerIdleGC(vm_, TRIGGER_IDLE_GC_TYPE::FULL_GC);
        JSNApi::TriggerIdleGC(vm_, TRIGGER_IDLE_GC_TYPE::SHARED_FULL_GC);
    };
    mainThreadHandler_->PostTask(task, "ARKTS_IDLE_COMPRESS", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE);
#endif
}

void ArkIdleMonitor::SetStartTimerCallback()
{
    JSNApi::SetStartIdleMonitorCallback([this]() {
        this->PostMonitorTask();
    });
}

void ArkIdleMonitor::NotifyChangeBackgroundState(bool inBackground)
{
    inBackground_.store(inBackground, std::memory_order_relaxed);
    ClearIdleStats();
}

double ArkIdleMonitor::GetCpuUsage() const
{
#ifdef ENABLE_UCOLLECTION
    auto collector = OHOS::HiviewDFX::UCollectClient::CpuCollector::Create();
    auto collectResult = collector->GetSysCpuUsage();
    if (collectResult.retCode == OHOS::HiviewDFX::UCollect::UcError::SUCCESS) {
        HILOG_DEBUG("ArkIdleMonitor cpu usage: %{public}.2f", collectResult.data);
        return collectResult.data;
    }
    HILOG_ERROR("ArkIdleMonitor get cpu usage failed, error code:%{public}d", collectResult.retCode);
#endif
    return 0.0f;
}
}

