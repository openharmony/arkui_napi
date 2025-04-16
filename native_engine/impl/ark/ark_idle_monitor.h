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
public:
    ArkIdleMonitor(){};
    ~ArkIdleMonitor();

    static std::shared_ptr<ArkIdleMonitor> GetInstance();

    bool IsIdleState() const
    {
        return idleState_.load(std::memory_order_relaxed);
    }

    void SetIdleState(bool idleState)
    {
        idleState_.store(idleState, std::memory_order_relaxed);
    }

    void NotifyChangeBackgroundState(bool inBackground);

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

    void SetMainThreadEcmaVM(EcmaVM* vm)
    {
        mainVM_ = vm;
    }

    void RegisterWorkerEnv(napi_env workerEnv)
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        workerEnvQueue_.push(workerEnv);
    }

    void UnregisterWorkerEnv(napi_env workerEnv)
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        for (size_t i = 0; i < workerEnvQueue_.size(); i++) {
            napi_env env = workerEnvQueue_.front();
            workerEnvQueue_.pop();
            if (env == workerEnv) {
                return;
            }
            workerEnvQueue_.push(env);
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

    void NotifyLooperIdleStart(int64_t timestamp, int idleTime);
    void NotifyLooperIdleEnd(int64_t timestamp);
    void PostMonitorTask(uint64_t delayMs = IDLE_MONITORING_INTERVAL);
    void SetStartTimerCallback();
    void PostLooperTriggerIdleGCTask();
    void EnableIdleGC(NativeEngine *engine);
    void UnregisterEnv(NativeEngine *engine);

private:
    double GetCpuUsage() const;
    bool ShouldTryTriggerGC(int64_t interval);
    bool CheckLowNotifyState() const;
    bool CheckLowRunningDurationState() const;
    bool CheckIntervalIdle(int64_t timestamp, int64_t idleDuration);
    bool CheckWorkerEnvQueueAllInIdle();
    void SwitchBackgroundCheckGCTask(int64_t timestamp, int64_t idleDuration);
    void IntervalMonitor();
    void NotifyMainThreadTryCompressGC();
    void NotifyMainThreadTryCompressGCByBackground();
    void NotifyOneWorkerThreadTryCompressGC();
    void ClearIdleStats();
    void TryTriggerGC(TriggerGCType gcType);
    void PostIdleCheckTask();
    void CheckWorkerEnvQueue();
    void StopIdleMonitorTimerTask();
    void StopIdleMonitorTimerTaskAndPostSleepTask();
    void CheckShortIdleTask(int64_t timestamp, int idleTime);
    void PostSwitchBackgroundGCTask();

    static std::shared_ptr<ArkIdleMonitor> instance_;

    EcmaVM* mainVM_;

    static constexpr uint32_t IDLE_CHECK_LENGTH = 15;
    static constexpr uint32_t IDLE_INBACKGROUND_CHECK_LENGTH = 4;
    static constexpr int IDLE_CHECK_INTERVAL_LENGTH = 4;
    static constexpr int MIN_TRIGGER_FULLGC_INTERVAL = 90;
    static constexpr int LOW_IDLE_NOTIFY_THRESHOLD = 10;
    static constexpr uint64_t IDLE_MONITORING_INTERVAL = 1 * 1000; // ms
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

    std::atomic<bool> idleState_ {false};
    std::atomic<bool> inBackground_ {true};
    std::atomic<int64_t> idleNotifyCount_ {0};
    std::atomic<int64_t> idleStartTimestamp_ {0};
    std::atomic<int64_t> totalIdleDuration_ {0};
    int64_t idleEndTimestamp_ {0};
    int64_t lastTotalIdleDuration_ {0};
    int64_t startRecordTimestamp_ {0};
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
    std::mutex queueMutex_;
    std::queue<napi_env> workerEnvQueue_;
#if defined(ENABLE_EVENT_HANDLER)
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainThreadHandler_ {};
    static bool gEnableIdleGC;
#endif
};

}

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_ARK_NATIVE_IDLE_MONITOR_H */