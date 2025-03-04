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
#include "native_event.h"
#include <string>

#ifdef ENABLE_CONTAINER_SCOPE_SEND_EVENT
#include "core/common/container_scope.h"
#endif
#include "native_api_internal.h"
#if defined(ENABLE_EVENT_HANDLER)
#include "event_handler.h"
#endif
#ifdef ENABLE_HITRACE
#include "hitrace_meter.h"
#endif
#include "native_engine.h"
#include "utils/log.h"
#if defined(ENABLE_EVENT_HANDLER)
using namespace OHOS::AppExecFwk;
#endif
#ifdef ENABLE_CONTAINER_SCOPE_SEND_EVENT
using OHOS::Ace::ContainerScope;
#endif

static constexpr uint64_t INVALID_EVENT_ID = std::numeric_limits<uint64_t>::max();
static constexpr const char DEFAULT_NAME[] = "defaultName";

// trace and log
class TraceLogClass {
#ifdef ENABLE_HITRACE
    uint64_t traceLable = HITRACE_TAG_ACE;
#endif
    std::string value;
    public:
        explicit TraceLogClass(std::string value) : value(value)
        {
#ifdef ENABLE_HITRACE
            StartTrace(traceLable, value);
#endif
            HILOG_DEBUG("log start{%{public}s}", value.c_str());
        }
        ~TraceLogClass()
        {
#ifdef ENABLE_HITRACE
            FinishTrace(traceLable);
#endif
            HILOG_DEBUG("log end{%{public}s}", value.c_str());
        }
};

// API
NAPI_EXTERN napi_status napi_send_event(napi_env env, const std::function<void()>& cb, napi_event_priority priority)
{
    CHECK_ENV(env);
    if (cb == nullptr) {
        HILOG_ERROR("invalid function is nullptr");
        return napi_status::napi_invalid_arg;
    }
    if (priority < napi_eprio_vip || priority > napi_eprio_idle) {
        HILOG_ERROR("invalid priority %{public}d", static_cast<int32_t>(priority));
        return napi_status::napi_invalid_arg;
    }
    NativeEngine *eng = reinterpret_cast<NativeEngine *>(env);
    NativeEngine::GetAliveEngineMutex().lock();
    if (!NativeEngine::IsAliveLocked(eng)) {
        NativeEngine::GetAliveEngineMutex().unlock();
        HILOG_ERROR("call NativeEngine not alive");
        return napi_status::napi_closing;
    }
    std::shared_lock<std::shared_mutex> readLock(eng->GetEventMutex());
    NativeEngine::GetAliveEngineMutex().unlock();

    // Prevent cb from becoming a floating pointer
    auto realCb = [cbIn = cb](void *data) {
        if (cbIn != nullptr) {
            HILOG_DEBUG("napi_send_event callBack called");
            cbIn();
        } else {
            HILOG_ERROR("napi_send_event callBack is null");
        }
    };
    uint64_t handleId = 0;

    if (!eng->GetDefaultFunc()) {
        HILOG_ERROR("default function is nullptr!");
        return napi_status::napi_generic_failure;
    }
    auto safeAsyncWork = reinterpret_cast<NativeEvent*>(eng->GetDefaultFunc());
    return safeAsyncWork->SendCancelableEvent(realCb, nullptr, priority, DEFAULT_NAME, &handleId);
}

NAPI_EXTERN napi_status napi_send_cancelable_event(napi_env env,
                                                   const std::function<void(void*)>& cb,
                                                   void* data,
                                                   napi_event_priority priority,
                                                   uint64_t* handleId,
                                                   const char* name)
{
    CHECK_ENV(env);
    if (cb == nullptr) {
        HILOG_ERROR("invalid function is nullptr");
        return napi_status::napi_invalid_arg;
    }
    if (priority < napi_eprio_vip || priority > napi_eprio_idle) {
        HILOG_ERROR("invalid priority %{public}d", static_cast<int32_t>(priority));
        return napi_status::napi_invalid_arg;
    }
    if (handleId == nullptr) {
        HILOG_ERROR("invalid handleId");
        return napi_status::napi_invalid_arg;
    }
    NativeEngine *eng = reinterpret_cast<NativeEngine *>(env);

    NativeEngine::GetAliveEngineMutex().lock();
    if (!NativeEngine::IsAliveLocked(eng)) {
        NativeEngine::GetAliveEngineMutex().unlock();
        HILOG_ERROR("call NativeEngine not alive");
        return napi_status::napi_closing;
    }
    std::shared_lock<std::shared_mutex> readLock(eng->GetEventMutex());
    NativeEngine::GetAliveEngineMutex().unlock();

    if (!eng->GetDefaultFunc()) {
        HILOG_ERROR("default function is nullptr!");
        return napi_status::napi_generic_failure;
    }
    auto safeAsyncWork = reinterpret_cast<NativeEvent*>(eng->GetDefaultFunc());
    return safeAsyncWork->SendCancelableEvent(cb, data, priority,
                                              ((name == nullptr) ? DEFAULT_NAME: name), handleId);
}

NAPI_EXTERN napi_status napi_cancel_event(napi_env env, uint64_t handleId, const char* name)
{
    CHECK_ENV(env);
    if (handleId == 0) {
        HILOG_ERROR("invalid handleId 0");
        return napi_status::napi_invalid_arg;
    }
    NativeEngine *eng = reinterpret_cast<NativeEngine *>(env);

    NativeEngine::GetAliveEngineMutex().lock();
    if (!NativeEngine::IsAliveLocked(eng)) {
        NativeEngine::GetAliveEngineMutex().unlock();
        HILOG_ERROR("call NativeEngine not alive");
        return napi_status::napi_closing;
    }
    std::shared_lock<std::shared_mutex> readLock(eng->GetEventMutex());
    NativeEngine::GetAliveEngineMutex().unlock();

    if (!eng->GetDefaultFunc()) {
        HILOG_ERROR("default function is nullptr!");
        return napi_status::napi_generic_failure;
    }
    auto safeAsyncWork = reinterpret_cast<NativeEvent*>(eng->GetDefaultFunc());
    return safeAsyncWork->CancelEvent(((name == nullptr) ? DEFAULT_NAME: name), handleId);
}

// static method
static void ThreadSafeCallback(napi_env env, napi_value jsCallback, void* context, void* data)
{
    if (data != nullptr) {
        CallbackWrapper *cbw = static_cast<CallbackWrapper *>(data);
        uint64_t expected = cbw->handleId.load(std::memory_order_acquire);
        if (expected != INVALID_EVENT_ID &&
            cbw->handleId.compare_exchange_strong(expected, INVALID_EVENT_ID,
                                                  std::memory_order_acq_rel, std::memory_order_relaxed)) {
#ifdef ENABLE_HITRACE
            StartTrace(HITRACE_TAG_ACE, "ThreadSafeCallback excute");
#endif
            cbw->cb();
#ifdef ENABLE_HITRACE
            FinishTrace(HITRACE_TAG_ACE);
#endif
        }
        delete cbw;
    }
}

void NativeEvent::CreateDefaultFunction(NativeEngine* eng, napi_threadsafe_function &defaultFunc,
                                        std::shared_mutex &eventMutex)
{
    std::unique_lock<std::shared_mutex> writeLock(eventMutex);
    if (defaultFunc) {
        return;
    }
    napi_env env = reinterpret_cast<napi_env>(eng);
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, "call_default_threadsafe_function", NAPI_AUTO_LENGTH, &resourceName);

    auto callJsCallback = reinterpret_cast<NativeThreadSafeFunctionCallJs>(ThreadSafeCallback);
    auto safeAsyncWork = new NativeEvent(eng, nullptr, nullptr, resourceName, 0, 1,
                                         nullptr, nullptr, nullptr, callJsCallback);
    auto ret = safeAsyncWork->Init();
    if (ret) {
        defaultFunc = reinterpret_cast<napi_threadsafe_function>(safeAsyncWork);
    } else {
        delete safeAsyncWork;
    }
}

void NativeEvent::DestoryDefaultFunction(bool release, napi_threadsafe_function &defaultFunc,
                                         std::shared_mutex &eventMutex)
{
    napi_threadsafe_function toReleaseFunc = nullptr;
    {
        std::unique_lock<std::shared_mutex> writeLock(eventMutex);
        if (!defaultFunc) {
            return;
        }
        toReleaseFunc = defaultFunc;
        defaultFunc = nullptr;
    }

    if (release) {
        napi_release_threadsafe_function(toReleaseFunc, napi_tsfn_abort);
    } else {
        NativeEvent* work = reinterpret_cast<NativeEvent*>(toReleaseFunc);
        delete work; // only free mem due to uv_loop is invalid
    }
}

//NativeEvent method
NativeEvent::NativeEvent(NativeEngine* engine,
    napi_value func,
    napi_value asyncResource,
    napi_value asyncResourceName,
    size_t maxQueueSize,
    size_t threadCount,
    void* finalizeData,
    NativeFinalize finalizeCallback,
    void* context,
    NativeThreadSafeFunctionCallJs callJsCallback) : NativeSafeAsyncWork(engine,
        func, asyncResource, asyncResourceName, maxQueueSize, threadCount,
        finalizeData, finalizeCallback, context, callJsCallback)
{
}

bool NativeEvent::Init()
{
    sequence_.store(1, std::memory_order_relaxed);
    return NativeSafeAsyncWork::Init();
}

napi_status NativeEvent::SendCancelableEvent(const std::function<void(void*)> &callback,
                                             void* data,
                                             int32_t priority,
                                             const char* name,
                                             uint64_t* handleId)
{
    uint64_t eventId = GenerateUniqueID();
#ifdef ENABLE_CONTAINER_SCOPE_SEND_EVENT
    int32_t containerScopeId = ContainerScope::CurrentId();
    std::function<void()> task = [eng = engine_, callback, data, eventId, containerScopeId]() {
        ContainerScope containerScope(containerScopeId);
#else
    std::function<void()> task = [eng = engine_, callback, data, eventId]() {
#endif
        auto tcin = TraceLogClass("Cancelable Event callback:handleId:" + std::to_string(eventId));
        auto vm = eng->GetEcmaVm();
        panda::LocalScope scope(vm);
        callback(data);
    };

    napi_status sentEventRes = SendEventByEventHandler(task, eventId, priority, name, handleId);
    if (sentEventRes != napi_status::napi_invalid_arg) {
        return sentEventRes;
    }

    return SendEventByUv(task, eventId, name, handleId);
}

napi_status NativeEvent::SendEventByEventHandler(const std::function<void()> &task, uint64_t eventId,
                                                 int32_t priority, const char* name, uint64_t* handleId)
{
#ifdef ENABLE_EVENT_HANDLER
    if (!eventHandler_) {
        // Internal temporary code
        return napi_status::napi_invalid_arg;
    }
    bool postRes = eventHandler_->PostTask(task,
                                           std::string(name) + std::to_string(eventId),
                                           0,
                                           static_cast<EventQueue::Priority>(priority),
                                           {});
    std::string res = (postRes ? "ok" : "fail");
    auto evt = TraceLogClass(
        "eventHandler Send task: " + std::string(name) +
        " | handleId: " + std::to_string(eventId) +
        " | postRes: " + res
    );
    if (postRes) {
        *handleId = eventId;
        return napi_status::napi_ok;
    }
    *handleId = 0;
    HILOG_ERROR("PostTask fail %{public}s", std::to_string(eventId).c_str());
    return napi_status::napi_generic_failure;
#endif
    // Internal temporary code
    return napi_status::napi_invalid_arg;
}

napi_status NativeEvent::SendEventByUv(const std::function<void()> &task, uint64_t eventId,
                                       const char* name, uint64_t* handleId)
{
    CallbackWrapper* cbw = new (std::nothrow) CallbackWrapper();
    if (!cbw) {
        HILOG_ERROR("malloc failed!");
        return napi_status::napi_generic_failure;
    }
    engine_->IncreaseWaitingRequestCounter();
    auto incCountTask = [eng = engine_, task]() {
        eng->DecreaseWaitingRequestCounter();
        task();
    };
    cbw->cb = incCountTask;
    cbw->handleId.store(eventId, std::memory_order_release);
    napi_status status = SendConvertStatus2NapiStatus(reinterpret_cast<void *>(cbw), NATIVE_TSFUNC_NONBLOCKING);
    *handleId = eventId;
    std::string res = (status == napi_status::napi_ok? "ok": "fail");
    auto uvt = TraceLogClass(
        "uv Send task:" + std::string(name) + " | handleId:" + std::to_string(eventId) + " | postRes: " + res);
    if (status != napi_status::napi_ok) {
        HILOG_ERROR("send event failed(%{public}d)", status);
        delete cbw;
        *handleId = 0;
        engine_->DecreaseWaitingRequestCounter();
    }
    return status;
}

napi_status NativeEvent::CancelEvent(const char* name, uint64_t handleId)
{
    auto tc = TraceLogClass("Cancel Event:" + std::string(name) + "|handleId:" + std::to_string(handleId));
    if (handleId == 0) {
        HILOG_ERROR("handleId = 0");
        return napi_status::napi_invalid_arg;
    }
#ifdef ENABLE_EVENT_HANDLER
    if (eventHandler_) {
        auto evt = TraceLogClass("eventHandler remove task");
        eventHandler_->RemoveTask(std::string(name) + std::to_string(handleId));
        return napi_status::napi_ok;
    }
#endif
    auto code = UvCancelEvent(handleId);
    std::string res = (code == SafeAsyncCode::SAFE_ASYNC_OK ? "ok" : "fail");
    auto uvt = TraceLogClass("uv remove task res:" + res);
    napi_status status = napi_status::napi_ok;
    if (code == SafeAsyncCode::SAFE_ASYNC_FAILED || code == SafeAsyncCode::SAFE_ASYNC_CLOSED) {
        status = napi_status::napi_generic_failure;
    }
    return status;
}

SafeAsyncCode NativeEvent::UvCancelEvent(uint64_t handleId)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (status_ == SafeAsyncStatus::SAFE_ASYNC_STATUS_CLOSED ||
        status_ == SafeAsyncStatus::SAFE_ASYNC_STATUS_CLOSING) {
        HILOG_WARN("Do not cancel, thread is closed!");
        return SafeAsyncCode::SAFE_ASYNC_CLOSED;
    }
    if (callJsCallback_ == nullptr) {
        HILOG_ERROR("callJsCallback is nullptr");
        return SafeAsyncCode::SAFE_ASYNC_FAILED;
    }

    for (const auto &current : queue_) {
        if (current == nullptr) {
            continue;
        }
        CallbackWrapper* cbw = reinterpret_cast<CallbackWrapper *>(current);
        if (cbw->handleId.load(std::memory_order_relaxed) != handleId) {
            continue;
        }
        if (cbw->handleId.compare_exchange_strong(handleId, INVALID_EVENT_ID,
                                                  std::memory_order_acq_rel, std::memory_order_relaxed)) {
            engine_->DecreaseWaitingRequestCounter();
            return SafeAsyncCode::SAFE_ASYNC_OK;
        }
        HILOG_WARN("UvCancelEvent false %{public}s", std::to_string(handleId).c_str());
        return SafeAsyncCode::SAFE_ASYNC_FAILED;
    }
    return SafeAsyncCode::SAFE_ASYNC_FAILED;
}

napi_status NativeEvent::SendConvertStatus2NapiStatus(void* data, NativeThreadSafeFunctionCallMode mode)
{
    auto code = Send(data, mode);
    napi_status status = napi_status::napi_ok;
    switch (code) {
        case SafeAsyncCode::SAFE_ASYNC_OK:
            status = napi_status::napi_ok;
            break;
        case SafeAsyncCode::SAFE_ASYNC_QUEUE_FULL:
            status = napi_status::napi_queue_full;
            break;
        case SafeAsyncCode::SAFE_ASYNC_INVALID_ARGS:
            status = napi_status::napi_invalid_arg;
            break;
        case SafeAsyncCode::SAFE_ASYNC_CLOSED:
            status = napi_status::napi_closing;
            break;
        case SafeAsyncCode::SAFE_ASYNC_FAILED:
            status = napi_status::napi_generic_failure;
            break;
        default:
            HILOG_ERROR("this branch is unreachable, code is %{public}d", code);
            status = napi_status::napi_generic_failure;
            break;
    }
    return status;
}

uint64_t NativeEvent::GenerateUniqueID()
{
    uint64_t currentSequence = sequence_.fetch_add(1, std::memory_order_relaxed);
    // if sequence_ max, store 1
    if (currentSequence == INVALID_EVENT_ID - 1) {
        sequence_.store(1, std::memory_order_relaxed);
        return 1;
    } else {
        return currentSequence;
    }
}