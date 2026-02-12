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

#if defined(LINUX_PLATFORM) || defined(OHOS_PLATFORM)
#include <dlfcn.h>
#include <signal.h>
#endif
#include "utils/log.h"
#if defined(ENABLE_FFRT)
#include "ffrt.h"
#include "c/executor_task.h"
#endif
#ifdef ENABLE_UCOLLECTION
#include "cpu_collector_client.h"
#endif
#if defined(ENABLE_HITRACE) || defined(ENABLE_EVENT_HANDLER)
#include "hitrace/trace.h"
#include "hitrace_meter.h"
#include "parameter.h"
#include "parameters.h"
#include "musl_preinit_common.h"
#include "memory_trace.h"
#endif
#include "ark_native_engine.h"

namespace panda::ecmascript {
const std::string RES_SCHED_CLIENT_SO = "libressched_client.z.so";
using ReportDataFunc = void (*)(uint32_t resType, int64_t value,
    const std::unordered_map<std::string, std::string>& payload);
#if defined(ENABLE_EVENT_HANDLER)
static constexpr uint64_t IDLE_GC_TIME_MIN = 1000;
static constexpr uint64_t IDLE_GC_TIME_MAX = 10000;
uint64_t ArkIdleMonitor::gIdleMonitoringInterval = ArkIdleMonitor::GetIdleMonitoringInterval();
bool ArkIdleMonitor::gEnableIdleGC =
    OHOS::system::GetBoolParameter("persist.ark.enableidlegc", true);
bool ArkIdleMonitor::gEnableDeferFreeze =
    OHOS::system::GetBoolParameter("persist.ark.deferfreeze", true);
#else
uint64_t ArkIdleMonitor::gIdleMonitoringInterval = 1000; // ms
#endif
// gDelayOverTime Detect whether there is any process freezing during the delay process of the delay task
uint64_t ArkIdleMonitor::gDelayOverTime = gIdleMonitoringInterval + 100; // ms


class TraceScope {
public:
    explicit TraceScope(std::string context) : context_(context)
    {
#ifdef ENABLE_HITRACE
        StartTrace(HITRACE_TAG_ACE, context_);
#endif
    };

    ~TraceScope()
    {
#ifdef ENABLE_HITRACE
        FinishTrace(HITRACE_TAG_ACE);
#endif
    }

private:
    std::string context_;
};

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
    int64_t intervalDuration = nowTimestamp - intervalTimestamp_;
    if (numberOfLowIdleNotifyCycles_ >= checkCounts &&
            numberOfHighIdleTimeRatio_ >= checkCounts &&
            intervalDuration < static_cast<int64_t>(gDelayOverTime)) {
        triggerTaskStartTimestamp_ = nowTimestamp;
        if (IsInBackground()) {
            TryTriggerCompressGCOfProcess();
        } else {
            NotifyMainThreadTryCompressGC();
        }
        PostMonitorTask(SLEEP_MONITORING_INTERVAL);
        ClearIdleStats();
    } else {
        PostMonitorTask(gIdleMonitoringInterval);
    }
    intervalTimestamp_ = nowTimestamp;
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
    if (mainVM_ == nullptr) {
        return;
    }
    auto task = [this]() {
        JSNApi::TriggerIdleGC(mainVM_, TRIGGER_IDLE_GC_TYPE::FULL_GC);
        if (CheckWorkerEnvQueueAllInIdle(IDLE_WORKER_TRIGGER_COUNT)) {
            JSNApi::TriggerIdleGC(mainVM_, TRIGGER_IDLE_GC_TYPE::SHARED_FULL_GC);
        }
    };
    mainThreadHandler_->PostTask(task, "ARKTS_IDLE_COMPRESS", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE);
#endif
}

void ArkIdleMonitor::SetStartTimerCallback()
{
    JSNApi::SetStartIdleMonitorCallback([this]() {
        // prevents duplicate invok to avoid deadlocks
        if (!started_) {
            this->IntervalMonitor();
            started_ = true;
        }
    });
}

void ArkIdleMonitor::NotifyNeedFreeze(bool needFreeze)
{
#if defined(ENABLE_EVENT_HANDLER)
    if (!gEnableDeferFreeze || !IsSwitchToBackgroundTask()) {
        return;
    }
    if (!needFreeze) {
        if (!deferfreeze_.load(std::memory_order_relaxed)) {
            deferfreeze_.store(true, std::memory_order_relaxed);
            ReportDataToRSS(false);
        } else {
            // The delay in freezing has been notified
            HILOG_DEBUG("ArkIdleMonitor: NotifyFreeze Abandoned message");
        }
    } else {
        if (deferfreeze_.load(std::memory_order_relaxed)) {
            deferfreeze_.store(false, std::memory_order_relaxed);
            SetSwitchToBackgroundTask(false);
            ReportDataToRSS(true);
        } else {
            // It is not in a delayed freeze state, so there is no need to notify the freeze
            HILOG_DEBUG("ArkIdleMonitor: NotifyFreeze Abandoned message");
        }
    }
#endif
}

void ArkIdleMonitor::PostLooperTriggerIdleGCTask()
{
#if defined(ENABLE_EVENT_HANDLER)
    std::shared_ptr<OHOS::AppExecFwk::EventRunner> mainThreadRunner =
        OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
    if (mainThreadRunner.get() == nullptr) {
        HILOG_FATAL("ArkNativeEngine:: the mainEventRunner is nullptr");
        return;
    }
    std::weak_ptr<ArkIdleMonitor> weakArkIdleMonitor = ArkIdleMonitor::GetInstance();
    auto callback = [weakArkIdleMonitor](OHOS::AppExecFwk::EventRunnerStage stage,
        const OHOS::AppExecFwk::StageInfo* info) -> int {
        auto arkIdleMonitor = weakArkIdleMonitor.lock();
        if (nullptr == arkIdleMonitor) {
            HILOG_ERROR("ArkIdleMonitor has been destructed.");
            return 0;
        }
        switch (stage) {
            case OHOS::AppExecFwk::EventRunnerStage::STAGE_BEFORE_WAITING:
                arkIdleMonitor->NotifyLooperIdleStart(info->timestamp, info->sleepTime);
                break;
            case OHOS::AppExecFwk::EventRunnerStage::STAGE_AFTER_WAITING:
                arkIdleMonitor->NotifyLooperIdleEnd(info->timestamp);
                break;
            default:
                HILOG_ERROR("this branch is unreachable");
        }
        return 0;
    };
    uint32_t stage = (static_cast<uint32_t>(OHOS::AppExecFwk::EventRunnerStage::STAGE_BEFORE_WAITING) |
        static_cast<uint32_t>(OHOS::AppExecFwk::EventRunnerStage::STAGE_AFTER_WAITING));
    mainThreadRunner->GetEventQueue()->AddObserver(OHOS::AppExecFwk::Observer::ARKTS_GC, stage, callback);
#endif
}

void ArkIdleMonitor::EnableIdleGC(NativeEngine *engine)
{
#if defined(ENABLE_EVENT_HANDLER)
    auto vm = const_cast<EcmaVM *>(engine->GetEcmaVm());
    if (gEnableIdleGC && JSNApi::IsJSMainThreadOfEcmaVM(vm)) {
        SetMainThreadEcmaVM(vm);
        JSNApi::SetTriggerGCTaskCallback(vm, [engine](TriggerGCData& data) {
            engine->PostTriggerGCTask(data, nullptr);
        });
        SetStartTimerCallback();
        PostLooperTriggerIdleGCTask();
        JSNApi::SetNotifyDeferFreezeCallback([this](bool isNeedFreeze) {
            NotifyNeedFreeze(isNeedFreeze);
        });
    } else {
        RegisterWorkerEnv(reinterpret_cast<napi_env>(engine));
    }
    if (mainThreadHandler_ == nullptr) {
        mainThreadHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(
            OHOS::AppExecFwk::EventRunner::GetMainEventRunner());
    };
#endif
}

void ArkIdleMonitor::UnregisterEnv(NativeEngine *engine)
{
#if defined(ENABLE_EVENT_HANDLER)
    if (!gEnableIdleGC || !JSNApi::IsJSMainThreadOfEcmaVM(engine->GetEcmaVm())) {
        UnregisterWorkerEnv(reinterpret_cast<napi_env>(engine));
    }
#endif
}

uint64_t ArkIdleMonitor::GetIdleMonitoringInterval()
{
#if defined(ENABLE_EVENT_HANDLER)
    uint64_t value =
        static_cast<uint64_t>(OHOS::system::GetIntParameter("const.arkui.idle_monitoring_interval", 1000)); // ms
    if (value < IDLE_GC_TIME_MIN) {
        value = IDLE_GC_TIME_MIN;
    }
    if (value > IDLE_GC_TIME_MAX) {
        value = IDLE_GC_TIME_MAX;
    }
    return value;
#else
    return gIdleMonitoringInterval;
#endif
}

void ArkIdleMonitor::NotifyChangeBackgroundState(bool inBackground)
{
    inBackground_.store(inBackground, std::memory_order_relaxed);
    ClearIdleStats();
    if (!started_ && inBackground) {
        HILOG_DEBUG("ArkIdleMonitor change to background but not started idle check");
        return;
    }
#if defined(ENABLE_FFRT)
    if (started_ && inBackground) {
        HILOG_DEBUG("ArkIdleMonitor post check switch background gc task");
        StopIdleMonitorTimerTask();
        CheckWorkerEnvQueue();
        PostSwitchBackgroundGCTask();
    }
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

void ArkIdleMonitor::SwitchBackgroundCheckGCTask(int64_t timestamp, int64_t idleDuration)
{
    int64_t nowTimestamp = std::chrono::time_point_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now()).time_since_epoch().count();
    int64_t sumDuration = nowTimestamp - timestamp;
    int64_t sumIdleDuration = (GetTotalIdleDuration() - idleDuration) + (nowTimestamp - GetNotifyTimestamp());
    double idlePercentage = static_cast<double>(sumIdleDuration) / static_cast<double>(sumDuration);
    double cpuUsage = GetCpuUsage();
    if (idlePercentage > BACKGROUND_IDLE_RATIO && sumDuration < static_cast<int64_t>(gDelayOverTime)) {
        triggerTaskStartTimestamp_ = nowTimestamp;
        CheckWorkerEnvQueue();
        SetSwitchToBackgroundTask(true);
        TryTriggerCompressGCOfProcess();
    } else {
        HILOG_INFO("ArkIdleMonitor cancel BGGCTask,idlePer:%{public}.2f;cpuUsage:%{public}.2f;duration:%{public}s",
            idlePercentage, cpuUsage, std::to_string(sumDuration).c_str());
    }
    StopIdleMonitorTimerTaskAndPostSleepTask();
    ClearIdleStats();
}

void ArkIdleMonitor::PostSwitchBackgroundGCTask()
{
#if defined(ENABLE_FFRT)
    if (IsDuringBackgroundTask()) {
        return;
    }
    if (switchBackgroundTimerHandler_ != -1) {
        ffrt_timer_stop(ffrt_qos_user_initiated, switchBackgroundTimerHandler_);
    }
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
        std::get<0>(*tuple)->SwitchBackgroundCheckGCTask(std::get<1>(*tuple), std::get<2>(*tuple));
        std::get<0>(*tuple)->SetDuringBackgroundTask(false);
        delete tuple;
    };
    switchBackgroundTimerHandler_ = ffrt_timer_start(ffrt_qos_user_initiated, gIdleMonitoringInterval,
        reinterpret_cast<void*>(data), task, false);
    SetDuringBackgroundTask(true);
#endif
}

void ArkIdleMonitor::CheckWorkerEnvQueue()
{
    std::lock_guard<std::mutex> lock(envVectorMutex_);
    for (auto it = workerEnvVector_.begin(); it != workerEnvVector_.end();) {
        auto arkNativeEngine = reinterpret_cast<ArkNativeEngine*>(*it);
        arkNativeEngine->GetWorkerThreadState()->CheckIdleState();
        HILOG_DEBUG("ArkIdleMonitor::CheckWorkerEnvQueue,tid=%{public}d, workerCount=%{public}d",
            arkNativeEngine->GetSysTid(), arkNativeEngine->GetWorkerThreadState()->GetCheckCount());
        ++it;
    }
}

bool ArkIdleMonitor::CheckWorkerEnvQueueAllInIdle(uint32_t idleCount)
{
    std::lock_guard<std::mutex> lock(envVectorMutex_);
    for (auto it = workerEnvVector_.begin(); it != workerEnvVector_.end();) {
        auto arkNativeEngine = reinterpret_cast<ArkNativeEngine*>(*it);
        if (arkNativeEngine->GetWorkerThreadState()->CheckIdleState() < idleCount) {
            return false;
        }
        ++it;
    }
    return true;
}

void ArkIdleMonitor::StopIdleMonitorTimerTask()
{
#if defined(ENABLE_FFRT)
    std::lock_guard<std::mutex> lock(timerMutex_);
    HILOG_INFO("Get timerMutex_");
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

void ArkIdleMonitor::StopIdleMonitorTimerTaskAndPostSleepTask()
{
#if defined(ENABLE_FFRT)
    std::lock_guard<std::mutex> lock(timerMutex_);
    if (currentTimerHandler_ != -1) {
        ffrt_timer_stop(ffrt_qos_user_initiated, currentTimerHandler_);
        currentTimerHandler_ = -1;
    }
    if (waitForStopTimerHandler_ != -1) {
        ffrt_timer_stop(ffrt_qos_user_initiated, waitForStopTimerHandler_);
        waitForStopTimerHandler_ = -1;
    }
    PostMonitorTask(SLEEP_MONITORING_INTERVAL);
#endif
}

void ArkIdleMonitor::ReportDataToRSS(bool needFreeze)
{
#if defined(LINUX_PLATFORM) || defined(OHOS_PLATFORM)
    auto pid = getpid();
    auto bundleName = JSNApi::GetBundleName(mainVM_);
    std::unordered_map<std::string, std::string> eventParams {
        { "pid", std::to_string(pid) },
        { "bundleName", bundleName }
    };
    if (!reportDataFunc_) {
        reportDataFunc_ = LoadReportDataFunc();
    }
    if (reportDataFunc_ != nullptr) {
        reportDataFunc_(RES_TYPE_GC_EVENT, needFreeze, eventParams);
        HILOG_INFO("ArkIdleMonitor: ReportDataToRSS %{public}d, pid: %{public}d, bundleName: %{public}s",
            needFreeze, pid, bundleName.c_str());
    } else {
        HILOG_WARN("ArkIdleMonitor: ReportDataToRSS func is nullptr.");
    }
#else
    HILOG_WARN("ArkIdleMonitor: Only linux supports LoadReportDataFunc");
#endif
}

ReportDataFunc ArkIdleMonitor::LoadReportDataFunc()
{
#if defined(LINUX_PLATFORM) || defined(OHOS_PLATFORM)
    if (dynamicLoadHandle_ != nullptr) {
        dlclose(dynamicLoadHandle_);
    }
    dynamicLoadHandle_ = dlopen(RES_SCHED_CLIENT_SO.c_str(), RTLD_NOW);
    auto func = reinterpret_cast<ReportDataFunc>(dlsym(dynamicLoadHandle_, "ReportData"));
    if (func == nullptr) {
        dlclose(dynamicLoadHandle_);
        HILOG_ERROR("ArkIdleMonitor: LoadReportDataFunc failed!");
        return nullptr;
    }
    HILOG_INFO("ArkIdleMonitor: LoadReportDataFunc success.");
    return func;
#else
    HILOG_DEBUG("ArkIdleMonitor: Only linux supports LoadReportDataFunc");
    return nullptr;
#endif
}

ArkIdleMonitor::~ArkIdleMonitor()
{
#if defined(LINUX_PLATFORM) || defined(OHOS_PLATFORM)
    if (dynamicLoadHandle_ != nullptr) {
        dlclose(dynamicLoadHandle_);
        dynamicLoadHandle_ = nullptr;
    }
#endif
#if defined(ENABLE_FFRT)
    StopIdleMonitorTimerTask();
    while (timerHandlerQueue_.size() > 0) {
        ffrt_timer_stop(ffrt_qos_user_initiated, timerHandlerQueue_.front());
        timerHandlerQueue_.pop();
    }
#endif
}


std::shared_ptr<ArkIdleMonitor> ArkIdleMonitor::instance_ = std::make_shared<ArkIdleMonitor>();

std::shared_ptr<ArkIdleMonitor> ArkIdleMonitor::GetInstance()
{
    return instance_;
}

bool ArkIdleMonitor::CheckIfInBackgroundInCompressGC()
{
    if (!IsInBackground() && IsSwitchToBackgroundTask()) {
        deferfreeze_.store(false, std::memory_order_relaxed);
        SetSwitchToBackgroundTask(false);
        return false;
    }
    return true;
}

void ArkIdleMonitor::TryTriggerCompressGCOfProcess()
{
    NotifyNeedFreeze(false);
#ifndef ENABLE_EVENT_HANDLER
    HILOG_WARN("ArkIdleMonitor: not enable ENABLE_EVENT_HANDLER");
    return;
#endif

#ifdef ENABLE_EVENT_HANDLER
    // trigger main thread local gc.
    auto mainThreadLocalTask = [this]() {
        HILOG_DEBUG("ArkIdleMonitor: try trigger local full gc start");
        std::unique_lock<std::mutex> lock(waitGCFinishjedMutex_);
        JSNApi::TriggerIdleGC(mainVM_, TRIGGER_IDLE_GC_TYPE::FULL_GC);
        HILOG_DEBUG("ArkIdleMonitor: try trigger local full gc end");
        gcFinishCV_.notify_one();
    };

    {
        TraceScope trace("TriggerMainThreadLocalGC");
        std::unique_lock<std::mutex> lock(waitGCFinishjedMutex_);
        mainThreadHandler_->PostTask(mainThreadLocalTask, "ARKTS_IDLE_COMPRESS", 0,
            OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE);
        gcFinishCV_.wait(lock);
    }
#endif

    bool isAllInIdle = true;

    // Traverse all threads to attempt to trigger the compression GC
    for (uint32_t index = 0; index < workerEnvVector_.size(); index++) {
        if (!gEnableDeferFreeze) {
            isAllInIdle = CheckWorkerEnvQueueAllInIdle(IDLE_WORKER_TRIGGER_COUNT);
            break;
        }
        std::unique_lock<std::mutex> vectorLock(envVectorMutex_);
        auto it = workerEnvVector_.at(index);
        if (!CheckIfInBackgroundInCompressGC()) {
            return;
        }

        auto arkNativeEngine = reinterpret_cast<ArkNativeEngine*>(it);
#if (defined(LINUX_PLATFORM) || defined(OHOS_PLATFORM))
        if (kill(arkNativeEngine->GetSysTid(), 0) != 0) {
            HILOG_ERROR("ArkIdleMonitor: tid: %{public}u is dead!", arkNativeEngine->GetSysTid());
            workerEnvVector_.erase(workerEnvVector_.begin() + index);
            --index;
            continue;
        }
#endif
        if (IsSentTask(it)) {
            HILOG_DEBUG("ArkIdleMonitor: tid: %{public}u is not respond!", arkNativeEngine->GetSysTid());
            continue;
        }

        if (arkNativeEngine->GetWorkerThreadState()->GetCheckCount() == 0) {
            isAllInIdle = false;
            continue;
        }

        if (arkNativeEngine->IsNativeThread()) {
            HILOG_DEBUG("ArkIdleMonitor: tid: %{public}u is native thread.", arkNativeEngine->GetSysTid());
            continue;
        }

        size_t expectedSize = JSNApi::GetEcmaVMExpectedMemoryReclamationSize(arkNativeEngine->GetEcmaVm());
        HILOG_DEBUG("ArkIdleMonitor:checkcount:%{public}d,expectedSize:%{public}zu,threadid:%{public}u",
            arkNativeEngine->GetWorkerThreadState()->GetCheckCount(), expectedSize, arkNativeEngine->GetSysTid());

        if (expectedSize > IDLE_MIN_EXPECT_RECLAIM_SIZE) {
            TraceScope trace("TriggerWorkerThreadLocalGC");
            std::unique_lock<std::mutex> lock(waitGCFinishjedMutex_);
            std::pair<void*, uint8_t> data(reinterpret_cast<void*>(const_cast<EcmaVM*>(arkNativeEngine->GetEcmaVm())),
                static_cast<uint8_t>(TRIGGER_IDLE_GC_TYPE::FULL_GC));

            auto callbackTask = [it]() {
                HILOG_DEBUG("ArkIdleMonitor: try trigger thread full gc end");
                ArkIdleMonitor::GetInstance()->UnRegisterSentTaskWorkerEnv(it);
                ArkIdleMonitor::GetInstance()->GCTaskFinishedCallback();
            };
            HILOG_DEBUG("ArkIdleMonitor: try trigger thread full gc start,tid: %{public}u",
                arkNativeEngine->GetSysTid());
            RegisterSentTaskWorkerEnv(it);
            arkNativeEngine->PostTriggerGCTask(data, callbackTask);
            vectorLock.unlock();
            gcFinishCV_.wait_for(lock, std::chrono::milliseconds(WAIT_GC_FINISH_INTERVAL));
            auto nowTimestamp = std::chrono::time_point_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now()).time_since_epoch().count();
            if (nowTimestamp - triggerTaskStartTimestamp_ > WAIT_LOCAL_GC_INTERVAL) {
                NotifyNeedFreeze(true);
                HILOG_INFO("ArkIdleMonitor: cancel shared gc because over time.");
                return;
            }
        }
    }

    // cancel gc if in high cpu usage or not all in idle.
    if (!isAllInIdle) {
        NotifyNeedFreeze(true);
        HILOG_INFO("ArkIdleMonitor: cancel shared gc because not all in idle");
        return;
    }
    double cpuUsage = GetCpuUsage();
    if (cpuUsage > IDLE_BACKGROUND_CPU_USAGE) {
        NotifyNeedFreeze(true);
        SetSwitchToBackgroundTask(false);
        HILOG_INFO("ArkIdleMonitor: cancel process gc because high cpu usage:%{public}.2f", cpuUsage);
        return;
    }

#ifdef ENABLE_EVENT_HANDLER
    // try trigger shared gc
    auto mainThreadSharedTask = [this]() {
        HILOG_DEBUG("ArkIdleMonitor: try trigger shared full gc");
        std::unique_lock<std::mutex> lock(waitGCFinishjedMutex_);
        JSNApi::TriggerIdleGC(mainVM_, TRIGGER_IDLE_GC_TYPE::SHARED_FULL_GC);
        gcFinishCV_.notify_one();
    };
    if (CheckIfInBackgroundInCompressGC()) {
        TraceScope trace("TriggerMainThreadSharedGC");
        std::unique_lock<std::mutex> lock(waitGCFinishjedMutex_);
        mainThreadHandler_->PostTask(mainThreadSharedTask, "ARKTS_IDLE_SHARED_COMPRESS", 0,
            OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE);
    } else {
        NotifyNeedFreeze(true);
        HILOG_WARN("ArkIdleMonitor: app is not in idle or in background.");
    }
#endif
}
}