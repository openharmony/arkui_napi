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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_ARK_NATIVE_IDLE_MONITOR_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_ARK_NATIVE_IDLE_MONITOR_H

#include <atomic>
#include <ctime>
#include <chrono>
#include <array>
#include <algorithm>
#include <memory>
#include <mutex>
#include <queue>
#include <unordered_set>
#include <set>
#include <condition_variable>

#if defined(ENABLE_EVENT_HANDLER)
#include "event_handler.h"
#endif
#include "uv.h"
#include "ecmascript/napi/include/dfx_jsnapi.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "interfaces/inner_api/napi/native_node_api.h"

class NativeEngine;

namespace panda::ecmascript {
class Heap;
class SharedHeap;
class ConcurrentMarker;
class MemController;
class SharedMemController;

class ArkIdleMonitor {
using Clock = std::chrono::high_resolution_clock;
using TRIGGER_IDLE_GC_TYPE = panda::JSNApi::TRIGGER_IDLE_GC_TYPE;
using ReportDataFunc = void (*)(uint32_t resType, int64_t value,
    const std::unordered_map<std::string, std::string>& payload);
public:
    ArkIdleMonitor(){};
    ~ArkIdleMonitor();

    static std::shared_ptr<ArkIdleMonitor> GetInstance();

    void GCTaskFinishedCallback()
    {
        std::unique_lock<std::mutex> lock(waitGCFinishjedMutex_);
        gcFinishCV_.notify_one();
    }

    void SetEnableDeferFreeze(bool state)
    {
        gEnableDeferFreeze = state;
    }

    bool IsEnableDeferFreeze()
    {
        return gEnableDeferFreeze;
    }

    bool IsIdleState() const
    {
        return idleState_.load(std::memory_order_relaxed);
    }

    void SetIdleState(bool idleState)
    {
        idleState_.store(idleState, std::memory_order_relaxed);
    }

    bool IsInBackground() const
    {
        return inBackground_.load(std::memory_order_relaxed);
    }

    void AddIdleNotifyCount()
    {
        idleNotifyCount_.fetch_add(1, std::memory_order_relaxed);
    }

    int64_t GetIdleNotifyCount() const
    {
        return idleNotifyCount_.load(std::memory_order_relaxed);
    }

    void ResetIdleNotifyCount()
    {
        return idleNotifyCount_.store(0, std::memory_order_relaxed);
    }

    int64_t GetNotifyTimestamp() const
    {
        return idleStartTimestamp_.load(std::memory_order_relaxed);
    }

    void SetNotifyTimestamp(int64_t timestamp)
    {
        idleStartTimestamp_.store(timestamp, std::memory_order_relaxed);
    }

    int64_t GetTotalIdleDuration() const
    {
        return totalIdleDuration_.load(std::memory_order_relaxed);
    }

    void ResetTotalIdleDuration()
    {
        totalIdleDuration_.store(0, std::memory_order_relaxed);
    }

    void AddIdleDuration(int64_t duration)
    {
        totalIdleDuration_.fetch_add(duration, std::memory_order_relaxed);
    }

    void SetDeferfreeze(bool state)
    {
        deferfreeze_.store(state, std::memory_order_relaxed);
    }

    bool IsDeferfreeze()
    {
        return deferfreeze_.load(std::memory_order_relaxed);
    }

    void SetSwitchToBackgroundTask(bool state)
    {
        isSwitchToBackgroundTask_.store(state, std::memory_order_relaxed);
    }

    bool IsSwitchToBackgroundTask()
    {
        return isSwitchToBackgroundTask_.load(std::memory_order_relaxed);
    }

    void SetMainThreadEcmaVM(EcmaVM* vm)
    {
        mainVM_ = vm;
    }

    void RegisterSentTaskWorkerEnv(napi_env workerEnv)
    {
        std::lock_guard<std::mutex> lock(sentTaskMutex_);
        sentTaskWorkerEnvSet_.insert(workerEnv);
    }

    bool IsSentTask(napi_env workerEnv)
    {
        std::lock_guard<std::mutex> lock(sentTaskMutex_);
        if (sentTaskWorkerEnvSet_.find(workerEnv) != sentTaskWorkerEnvSet_.end()) {
            return true;
        }
        return false;
    }

    void UnRegisterSentTaskWorkerEnv(napi_env workerEnv)
    {
        std::lock_guard<std::mutex> lock(sentTaskMutex_);
        sentTaskWorkerEnvSet_.erase(workerEnv);
    }

    void RegisterWorkerEnv(napi_env workerEnv)
    {
        std::lock_guard<std::mutex> lock(envVectorMutex_);
        workerEnvVector_.push_back(workerEnv);
    }

    void UnregisterWorkerEnv(napi_env workerEnv)
    {
        std::lock_guard<std::mutex> lock(envVectorMutex_);
        for (auto it = workerEnvVector_.begin(); it != workerEnvVector_.end();) {
            if (*it == workerEnv) {
                it = workerEnvVector_.erase(it);
            } else {
                ++it;
            }
        }
    }

    template<typename T, int N>
    class RingBuffer {
    public:
        RingBuffer() = default;
        ~RingBuffer() = default;

        void Push(const T &value)
        {
            if (count_ == N) {
                elements_[start_++] = value;
                if (start_ == N) {
                    start_ = 0;
                }
            } else {
                ASSERT(start_ == 0);
                elements_[count_++] = value;
            }
        }

        int Count() const
        {
            return count_;
        }

        template<typename Callback>
        T Sum(Callback callback, const T &initial) const
        {
            T result = initial;
            for (int i = 0; i < count_; i++) {
                result = callback(result, elements_[i]);
            }
            return result;
        }

        void Reset()
        {
            start_ = count_ = 0;
        }

    private:
        std::array<T, N> elements_;
        int start_ {0};
        int count_ {0};
    };

    void NotifyChangeBackgroundState(bool inBackground);
    void NotifyLooperIdleStart(int64_t timestamp, int idleTime);
    void NotifyLooperIdleEnd(int64_t timestamp);
    void PostMonitorTask(uint64_t delayMs);
    void SetStartTimerCallback();
    void PostLooperTriggerIdleGCTask();
    void EnableIdleGC(NativeEngine *engine);
    void UnregisterEnv(NativeEngine *engine);
    void NotifyNeedFreeze(bool needFreeze);
    ReportDataFunc LoadReportDataFunc();

private:
    double GetCpuUsage() const;
    bool ShouldTryTriggerGC(int64_t interval);
    bool CheckLowNotifyState() const;
    bool CheckLowRunningDurationState() const;
    bool CheckIntervalIdle(int64_t timestamp, int64_t idleDuration);
    bool CheckWorkerEnvQueueAllInIdle(uint32_t idleCount);
    bool CheckIfInBackgroundInCompressGC();
    void SwitchBackgroundCheckGCTask(int64_t timestamp, int64_t idleDuration);
    void IntervalMonitor();
    void NotifyMainThreadTryCompressGC();
    void ClearIdleStats();
    void TryTriggerGC(TriggerGCType gcType);
    void PostIdleCheckTask();
    void CheckWorkerEnvQueue();
    void StopIdleMonitorTimerTask();
    void StopIdleMonitorTimerTaskAndPostSleepTask();
    void CheckShortIdleTask(int64_t timestamp, int idleTime);
    void PostSwitchBackgroundGCTask();
    void ReportDataToRSS(bool isGCStart);
    void TryTriggerCompressGCOfProcess();
    static uint64_t GetIdleMonitoringInterval();

    static std::shared_ptr<ArkIdleMonitor> instance_;

    EcmaVM* mainVM_;

    static constexpr uint32_t IDLE_CHECK_LENGTH = 15;
    static constexpr uint32_t IDLE_INBACKGROUND_CHECK_LENGTH = 4;
    static constexpr int IDLE_CHECK_INTERVAL_LENGTH = 4;
    static constexpr int MIN_TRIGGER_FULLGC_INTERVAL = 90;
    static constexpr int LOW_IDLE_NOTIFY_THRESHOLD = 10;
    static constexpr uint64_t SLEEP_MONITORING_INTERVAL = 90 * 1000; // ms
    static constexpr int64_t MIN_TRIGGER_GC_IDLE_INTERVAL = 10; // ms
    static constexpr int64_t MAX_TRIGGER_GC_RUNNING_INTERVAL = 1; //ms
    static constexpr double IDLE_RATIO = 0.985f;
    static constexpr double SHORT_IDLE_RATIO = 0.96f;
    static constexpr double BACKGROUND_IDLE_RATIO = 0.85f;
    static constexpr uint64_t  SHORT_IDLE_DELAY_INTERVAL = 50; // ms;
    static constexpr double IDLE_CPU_USAGE = 0.5f;
    static constexpr double IDLE_BACKGROUND_CPU_USAGE = 0.7f;
    static constexpr int DOUBLE_INTERVAL_CHECK = 2;
    static constexpr uint32_t IDLE_WORKER_TRIGGER_COUNT = 1; // it needs over IDLE_INBACKGROUND_CHECK_LENGTH
    static constexpr uint32_t IDLE_WORKER_CHECK_TASK_COUNT = 4;
    static constexpr uint32_t IDLE_WORKER_CHECK_TASK_COUNT_BACKGROUND = 1;
    // It needs to be synchronized with ResType in the res_type.h file.
    static constexpr uint32_t RES_TYPE_GC_EVENT = 185;
    static constexpr size_t IDLE_MIN_EXPECT_RECLAIM_SIZE = 1_MB;

    std::atomic<bool> idleState_ {false};
    std::atomic<bool> inBackground_ {true};
    std::atomic<bool> deferfreeze_ {false};
    std::atomic<bool> isSwitchToBackgroundTask_ {false};
    std::atomic<bool> isBackgroundTask_ {false};
    std::atomic<int64_t> idleNotifyCount_ {0};
    std::atomic<int64_t> idleStartTimestamp_ {0};
    std::atomic<int64_t> totalIdleDuration_ {0};
    int64_t idleEndTimestamp_ {0};
    int64_t lastTotalIdleDuration_ {0};
    int64_t startRecordTimestamp_ {0};
    int64_t intervalTimestamp_ {0};
    int64_t triggerTaskStartTimestamp_ {0};
    bool started_ {false};
    bool triggeredGC_ {false};
    bool needCheckIntervalIdle_ = {true};
    int currentTimerHandler_ {-1};
    int waitForStopTimerHandler_ {-1};
    int switchBackgroundTimerHandler_ {-1};
    uint32_t numberOfLowIdleNotifyCycles_ {0U};
    uint32_t numberOfHighIdleTimeRatio_ {0U};
    std::queue<int> timerHandlerQueue_;
    uint32_t handlerWaitToStopCount_ {0};
    RingBuffer<int64_t, IDLE_CHECK_INTERVAL_LENGTH> recordedIdleNotifyInterval_;
    RingBuffer<int64_t, IDLE_CHECK_INTERVAL_LENGTH> recordedRunningNotifyInterval_;
    std::mutex timerMutex_;
    std::mutex envVectorMutex_;
    std::mutex sentTaskMutex_;
    std::set<napi_env> sentTaskWorkerEnvSet_;
    std::vector<napi_env> workerEnvVector_;
    static uint64_t gIdleMonitoringInterval;
    static uint64_t gDelayOverTime;
#if defined(ENABLE_EVENT_HANDLER)
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainThreadHandler_ {};
    static bool gEnableIdleGC;
#endif
    static bool gEnableDeferFreeze;
    void* dynamicLoadHandle_ {nullptr};
    ReportDataFunc reportDataFunc_ {nullptr};

    std::mutex waitGCFinishjedMutex_;
    std::condition_variable gcFinishCV_;
};

}

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_ARK_NATIVE_IDLE_MONITOR_H */