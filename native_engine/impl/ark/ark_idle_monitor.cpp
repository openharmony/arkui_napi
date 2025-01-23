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
#ifdef ENABLE_HITRACE
#include "hitrace/trace.h"
#include "hitrace_meter.h"
#include "parameter.h"
#include "musl_preinit_common.h"
#include "memory_trace.h"
#endif
#include "ark_native_engine.h"

namespace panda::ecmascript {

void ArkIdleMonitor::NotifyLooperIdleStart(int64_t timestamp, [[maybe_unused]] int idleTime)
{
    SetIdleState(true);
    AddIdleNotifyCount();
    recordedRunningNotifyInterval_.Push(timestamp - idleEndTimestamp_);
#ifndef DISABLE_SHORT_IDLE_CHECK
    CheckShortIdleTask(timestamp, idleTime);
#endif
    SetNotifyTimestamp(timestamp);
}

void ArkIdleMonitor::CheckShortIdleTask(int64_t timestamp, int idleTime)
{
#if defined(ENABLE_FFRT)
    while (handlerWaitToStopCount_ > 0) {
        if (timerHandlerQueue_.size() <= 0) {
            handlerWaitToStopCount_ = 0;
            break;
        }
        int handler = timerHandlerQueue_.front();
        timerHandlerQueue_.pop();
        int ret = ffrt_timer_stop(ffrt_qos_user_initiated, handler);
        if (ret != 0) {
            HILOG_ERROR("ArkIdleMonitor: ffrt_timer_stop error handler: timerHandler='%{public}d', ret='%{public}d'",
                handler, ret);
        }
        handlerWaitToStopCount_--;
    }
#endif
    if (triggeredGC_ && mainVM_ != nullptr) {
#ifdef ENABLE_HITRACE
        StartTrace(HITRACE_TAG_ACE, "NotifyLooperIdleStart::TriggeredGC");
#endif
        triggeredGC_ = JSNApi::NotifyLooperIdleStart(mainVM_, timestamp, idleTime);
#ifdef ENABLE_HITRACE
        FinishTrace(HITRACE_TAG_ACE);
#endif
         return;
     }
    if (ShouldTryTriggerGC(timestamp - GetNotifyTimestamp()) &&
        idleTime > MIN_TRIGGER_GC_IDLE_INTERVAL &&
        needCheckIntervalIdle_) {
        PostIdleCheckTask();
    }
    if (!needCheckIntervalIdle_) {
        needCheckIntervalIdle_ = true;
    }
}

bool ArkIdleMonitor::ShouldTryTriggerGC(int64_t interval)
{
    if (interval < MIN_TRIGGER_GC_IDLE_INTERVAL ||
        recordedIdleNotifyInterval_.Count() != IDLE_CHECK_INTERVAL_LENGTH) {
        return false;
    }
    int64_t sumIdleInterval = recordedIdleNotifyInterval_.Sum([](int64_t a, int64_t b) {return a + b;}, 0);
    int64_t averageIdleInterval = sumIdleInterval / recordedIdleNotifyInterval_.Count();
    int64_t sumRunningInterval = recordedRunningNotifyInterval_.Sum([](int64_t a, int64_t b) {return a + b;}, 0);
    int64_t averageRunningInterval = sumRunningInterval / recordedRunningNotifyInterval_.Count();
    if (averageIdleInterval > MIN_TRIGGER_GC_IDLE_INTERVAL &&
            averageRunningInterval <= MAX_TRIGGER_GC_RUNNING_INTERVAL) {
        return true;
    }
    return false;
}

void ArkIdleMonitor::NotifyLooperIdleEnd(int64_t timestamp)
{
    idleEndTimestamp_ = timestamp;
    SetIdleState(false);
    int64_t duration = timestamp - GetNotifyTimestamp();
    recordedIdleNotifyInterval_.Push(duration);
    AddIdleDuration(duration);
    if (mainVM_ != nullptr) {
        JSNApi::NotifyLooperIdleEnd(mainVM_, timestamp);
    }
}

bool ArkIdleMonitor::CheckLowNotifyState() const
{
    uint32_t checkCounts = IsInBackground() ? IDLE_INBACKGROUND_CHECK_LENGTH : IDLE_CHECK_LENGTH;
    HILOG_DEBUG("ArkIdleMonitor: low Notify checkCounts '%{public}d', result '%{public}d' ",
        checkCounts, static_cast<int>(numberOfLowIdleNotifyCycles_));
    return numberOfLowIdleNotifyCycles_ >= checkCounts;
}

bool ArkIdleMonitor::CheckLowRunningDurationState() const
{
    uint32_t checkCounts = IsInBackground() ? IDLE_INBACKGROUND_CHECK_LENGTH : IDLE_CHECK_LENGTH;
    HILOG_DEBUG("ArkIdleMonitor: low Duration checkCounts '%{public}d', result '%{public}d' ",
        checkCounts, static_cast<int>(numberOfHighIdleTimeRatio_));
    return numberOfHighIdleTimeRatio_ >= checkCounts;
}

void ArkIdleMonitor::IntervalMonitor()
{
    if (!timerMutex_.try_lock()) {
        HILOG_INFO("ArkIdleMonitor: IntervalMonitor stop by timerMutex_");
        return;
    }
    auto nowTimestamp = std::chrono::time_point_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now()).time_since_epoch().count();
    int64_t idleDuration = GetTotalIdleDuration() - lastTotalIdleDuration_;
    if (IsIdleState()) {
        idleDuration += (nowTimestamp - GetNotifyTimestamp());
    }
    lastTotalIdleDuration_ = GetTotalIdleDuration();
    if (GetIdleNotifyCount() <= LOW_IDLE_NOTIFY_THRESHOLD) {
        numberOfLowIdleNotifyCycles_++;
    } else {
        numberOfLowIdleNotifyCycles_ = 0U;
    }
    ResetIdleNotifyCount();
    int64_t recordTotalDuration = nowTimestamp - startRecordTimestamp_;
    if (recordTotalDuration <= 0) {
        numberOfHighIdleTimeRatio_ = 0U;
        HILOG_ERROR("ArkIdleMonitor: recordTotalDuration <= 0");
    } else {
        double idleTimeRatio = static_cast<double>(idleDuration) / recordTotalDuration;
        HILOG_DEBUG("ArkIdleMonitor: idleTimeRatio '%{public}.2f'", idleTimeRatio);
        idleTimeRatio >= IDLE_RATIO ? numberOfHighIdleTimeRatio_++ : (numberOfHighIdleTimeRatio_ = 0U);
    }
    startRecordTimestamp_ = nowTimestamp;
    CheckWorkerEnvQueue();
    uint32_t checkCounts = IsInBackground() ? IDLE_INBACKGROUND_CHECK_LENGTH : IDLE_CHECK_LENGTH;
    uint32_t workThreadCheckCounts = checkCounts - IDLE_WORKER_TRIGGER_COUNT;
    if (numberOfLowIdleNotifyCycles_ >= checkCounts &&
            numberOfHighIdleTimeRatio_ >= checkCounts) {
        NotifyMainThreadTryCompressGC();
        PostMonitorTask(SLEEP_MONITORING_INTERVAL);
        ClearIdleStats();
    } else if (numberOfLowIdleNotifyCycles_ >= workThreadCheckCounts &&
                    numberOfHighIdleTimeRatio_ >= workThreadCheckCounts) {
        NotifyOneWorkerThreadTryCompressGC();
        PostMonitorTask(IDLE_MONITORING_INTERVAL);
    } else {
        PostMonitorTask(IDLE_MONITORING_INTERVAL);
    }
    timerMutex_.unlock();
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
    StopIdleMonitorTimerTask();
    while (timerHandlerQueue_.size() > 0) {
        ffrt_timer_stop(ffrt_qos_user_initiated, timerHandlerQueue_.front());
        timerHandlerQueue_.pop();
    }
#endif
}

void ArkIdleMonitor::ClearIdleStats()
{
    ResetIdleNotifyCount();
    numberOfLowIdleNotifyCycles_ = 0U;
    numberOfHighIdleTimeRatio_ = 0U;
}

void ArkIdleMonitor::NotifyMainThreadTryCompressGC()
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
    if (mainVM_ == nullptr) {
        return;
    }
    auto task = [this]() {
        JSNApi::TriggerIdleGC(mainVM_, TRIGGER_IDLE_GC_TYPE::FULL_GC);
        if (CheckWorkerEnvQueueAllInIdle()) {
            JSNApi::TriggerIdleGC(mainVM_, TRIGGER_IDLE_GC_TYPE::SHARED_FULL_GC);
        }
    };
    mainThreadHandler_->PostTask(task, "ARKTS_IDLE_COMPRESS", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE);
#endif
}

void ArkIdleMonitor::SetStartTimerCallback()
{
    JSNApi::SetStartIdleMonitorCallback([this]() {
        this->IntervalMonitor();
        started_ = true;
    });
}

void ArkIdleMonitor::NotifyChangeBackgroundState(bool inBackground)
{
    inBackground_.store(inBackground, std::memory_order_relaxed);
    ClearIdleStats();
    if (!started_) {
        return;
    }
#if defined(ENABLE_FFRT)
    if (inBackground == false) {
        return;
    }
    StopIdleMonitorTimerTask();
    IntervalMonitor();
#endif
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

bool ArkIdleMonitor::CheckIntervalIdle(int64_t timestamp, int64_t idleDuration)
{
    if (!IsIdleState()) {
        return false;
    }
    int64_t nowTimestamp = std::chrono::time_point_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now()).time_since_epoch().count();
    int64_t sumDuration = nowTimestamp - timestamp;
    int64_t sumIdleDuration = (GetTotalIdleDuration() - idleDuration) + (nowTimestamp - GetNotifyTimestamp());
    [[maybe_unused]] double idlePercentage = static_cast<double>(sumIdleDuration) / static_cast<double>(sumDuration);
#ifdef ENABLE_HITRACE
    StartTrace(HITRACE_TAG_ACE, "CheckIntervalIdle::sumDuration:" + std::to_string(sumDuration)
        + "sumIdleDuration:" + std::to_string(sumIdleDuration)
        + "idlePercentage" + std::to_string(idlePercentage));
#endif
#if defined(ENABLE_EVENT_HANDLER)
    if (idlePercentage > SHORT_IDLE_RATIO && mainVM_!= nullptr) {
        if (mainThreadHandler_ == nullptr) {
            mainThreadHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(
                OHOS::AppExecFwk::EventRunner::GetMainEventRunner());
        };
        auto task = [this]() {
            triggeredGC_ = JSNApi::NotifyLooperIdleStart(mainVM_, 0, 0);
            needCheckIntervalIdle_ = false;
            handlerWaitToStopCount_++;
            // If GC is triggered, reset the statistics to avoid triggering monitoring tasks continuously.
            if (!triggeredGC_) {
                recordedIdleNotifyInterval_.Reset();
            }
        };
        mainThreadHandler_->PostTask(task, "ARKTS_IDLE_NOTIFY", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE);
    }
#endif
#ifdef ENABLE_HITRACE
    FinishTrace(HITRACE_TAG_ACE);
#endif
    return true;
}

void ArkIdleMonitor::PostIdleCheckTask()
{
#ifdef ENABLE_HITRACE
        StartTrace(HITRACE_TAG_ACE, "NotifyLooperIdleStart::PostIdleCheckTask");
#endif
#if defined(ENABLE_FFRT)
    auto nowTimestamp = std::chrono::time_point_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now()).time_since_epoch().count();
    std::tuple<ArkIdleMonitor*, int64_t, int64_t> myTuple = std::make_tuple(this, nowTimestamp, GetTotalIdleDuration());
    std::tuple<ArkIdleMonitor*, int64_t, int64_t> *data = new std::tuple<ArkIdleMonitor*, int64_t, int64_t>(myTuple);
    auto task = [](void* data) {
        std::tuple<ArkIdleMonitor*, int64_t, int64_t>* tuple =
            reinterpret_cast<std::tuple<ArkIdleMonitor*, int64_t, int64_t>*>(data);
        if (tuple == nullptr || std::get<0>(*tuple) == nullptr) {
            return;
        }
        std::get<0>(*tuple)->CheckIntervalIdle(std::get<1>(*tuple), std::get<2>(*tuple));
        delete tuple;
    };
    int timerHandler = ffrt_timer_start(ffrt_qos_user_initiated, SHORT_IDLE_DELAY_INTERVAL,
        reinterpret_cast<void*>(data), task, false);
    timerHandlerQueue_.push(timerHandler);
#endif
#ifdef ENABLE_HITRACE
    FinishTrace(HITRACE_TAG_ACE);
#endif
}

void ArkIdleMonitor::CheckWorkerEnvQueue()
{
    std::lock_guard<std::mutex> lock(queueMutex_);
    for (size_t i = 0; i < workerEnvQueue_.size(); i++) {
        napi_env env = workerEnvQueue_.front();
        auto arkNativeEngine = reinterpret_cast<ArkNativeEngine*>(env);
        workerEnvQueue_.pop();
        workerEnvQueue_.push(env);
        arkNativeEngine->GetWorkerThreadState()->CheckIdleState();
        HILOG_DEBUG("ArkIdleMonitor::CheckWorkerEnvQueue,tid=%{public}d, workerCount=%{public}d",
            arkNativeEngine->GetSysTid(), arkNativeEngine->GetWorkerThreadState()->GetCheckCount());
    }
}

bool ArkIdleMonitor::CheckWorkerEnvQueueAllInIdle()
{
    std::lock_guard<std::mutex> lock(queueMutex_);
    for (size_t i = 0; i < workerEnvQueue_.size(); i++) {
        napi_env env = workerEnvQueue_.front();
        auto arkNativeEngine = reinterpret_cast<ArkNativeEngine*>(env);
        if (arkNativeEngine->GetWorkerThreadState()->GetCheckCount() < IDLE_INBACKGROUND_CHECK_LENGTH) {
            return false;
        }
        workerEnvQueue_.pop();
        workerEnvQueue_.push(env);
    }
    return true;
}

void ArkIdleMonitor::NotifyOneWorkerThreadTryCompressGC()
{
    std::lock_guard<std::mutex> lock(queueMutex_);
    for (size_t i = 0; i < workerEnvQueue_.size(); i++) {
        napi_env env = workerEnvQueue_.front();
        workerEnvQueue_.pop();
        workerEnvQueue_.push(env);
        auto arkNativeEngine = reinterpret_cast<ArkNativeEngine*>(env);
        if (arkNativeEngine->GetWorkerThreadState()->GetCheckCount() >= IDLE_INBACKGROUND_CHECK_LENGTH) {
            std::pair<void*, uint8_t> data(reinterpret_cast<void*>(const_cast<EcmaVM*>(arkNativeEngine->GetEcmaVm())),
                static_cast<uint8_t>(TRIGGER_IDLE_GC_TYPE::FULL_GC));
            arkNativeEngine->PostTriggerGCTask(data);
            return;
        }
    }
}

void ArkIdleMonitor::StopIdleMonitorTimerTask()
{
#if defined(ENABLE_FFRT)
    std::lock_guard<std::mutex> lock(timerMutex_);
    HILOG_INFO("StopIdleMonitorTimerTask get timerMutex_");
    if (currentTimerHandler_ != -1) {
        ffrt_timer_stop(ffrt_qos_user_initiated, currentTimerHandler_);
        currentTimerHandler_ = -1;
    }
    if (waitForStopTimerHandler_ != -1) {
        ffrt_timer_stop(ffrt_qos_user_initiated, waitForStopTimerHandler_);
        waitForStopTimerHandler_ = -1;
    }
#endif
}


std::shared_ptr<ArkIdleMonitor> ArkIdleMonitor::instance_ = std::make_shared<ArkIdleMonitor>();

std::shared_ptr<ArkIdleMonitor> ArkIdleMonitor::GetInstance()
{
    return instance_;
}
}

