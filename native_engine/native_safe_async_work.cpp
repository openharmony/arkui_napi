/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <cinttypes>

#include "native_safe_async_work.h"

#include "native_api_internal.h"
#include <securec.h>

#ifdef ENABLE_HITRACE
#include "hitrace_meter.h"
#include "parameter.h"
#endif

#ifdef ENABLE_CONTAINER_SCOPE
#include "core/common/container_scope.h"
#endif

#ifdef ENABLE_CONTAINER_SCOPE
using OHOS::Ace::ContainerScope;
#endif

#if defined(ENABLE_EVENT_HANDLER)
#include "event_handler.h"
using namespace OHOS::AppExecFwk;
#endif

#ifdef ENABLE_HITRACE
std::atomic<bool> g_SafeWorkTraceIdEnabled(false);
std::once_flag g_SafeWorkParamUpdated;
constexpr size_t TRACEID_PARAM_SIZE = 10;
using namespace OHOS::HiviewDFX;
#endif

// static methods start
void NativeSafeAsyncWork::AsyncCallback(uv_async_t* asyncHandler)
{
    HILOG_DEBUG("NativeSafeAsyncWork::AsyncCallback called");
    NativeSafeAsyncWork* that = NativeAsyncWork::DereferenceOf(&NativeSafeAsyncWork::asyncHandler_, asyncHandler);
    if (that == nullptr) {
        HILOG_ERROR("NativeSafeAsyncWork:: DereferenceOf failed!");
        return;
    }
    that->ProcessAsyncHandle();
}

void NativeSafeAsyncWork::CallJs(NativeEngine* engine, napi_value js_call_func, void* context, void* data)
{
    if (engine == nullptr || js_call_func == nullptr) {
        HILOG_ERROR("CallJs failed. engine or js_call_func is nullptr!");
        return;
    }
    napi_value value = nullptr;
    napi_get_undefined(reinterpret_cast<napi_env>(engine), &value);
    if (value == nullptr) {
        HILOG_ERROR("CreateUndefined failed");
        return;
    }

    auto resultValue = engine->CallFunction(value, js_call_func, nullptr, 0);
    if (resultValue == nullptr) {
        HILOG_ERROR("CallFunction failed");
    }
}

NativeSafeAsyncWork::NativeSafeAsyncWork(NativeEngine* engine,
                                         napi_value func,
                                         napi_value asyncResource,
                                         napi_value asyncResourceName,
                                         size_t maxQueueSize,
                                         size_t threadCount,
                                         void* finalizeData,
                                         NativeFinalize finalizeCallback,
                                         void* context,
                                         NativeThreadSafeFunctionCallJs callJsCallback)
    :engine_(engine), engineId_(engine->GetId()), maxQueueSize_(maxQueueSize),
    threadCount_(threadCount), finalizeData_(finalizeData), finalizeCallback_(finalizeCallback),
    context_(context), callJsCallback_(callJsCallback)
{
    asyncContext_.napiAsyncResource = asyncResource;
    asyncContext_.napiAsyncResourceName = asyncResourceName;

    errno_t err = EOK;
    err = memset_s(&asyncHandler_, sizeof(asyncHandler_), 0, sizeof(asyncHandler_));
    if (err != EOK) {
        HILOG_ERROR("faild to init asyncHandler_");
        return;
    }

    if (func != nullptr) {
        uint32_t initialRefcount = 1;
        ref_ = engine->CreateReference(func, initialRefcount);
    }

#ifdef ENABLE_CONTAINER_SCOPE
    if (engine->IsContainerScopeEnabled()) {
        containerScopeId_ = ContainerScope::CurrentId();
    }
#endif

#if defined(ENABLE_EVENT_HANDLER)
    runner_ = EventRunner::Current();
#endif

    InitSafeAsyncWorkTraceId();
}

NativeSafeAsyncWork::~NativeSafeAsyncWork()
{
    if (ref_ != nullptr) {
        delete ref_;
        ref_ = nullptr;
    }
}

bool NativeSafeAsyncWork::Init()
{
    HILOG_DEBUG("NativeSafeAsyncWork::Init called");

    uv_loop_t* loop = engine_->GetUVLoop();
    if (loop == nullptr) {
        HILOG_ERROR("Get loop failed");
        return false;
    }

    int ret = uv_async_init(loop, &asyncHandler_, AsyncCallback);
    if (ret != 0) {
        HILOG_ERROR("uv async send failed in Init ret = %{public}d", ret);
        return false;
    }

    status_ = SafeAsyncStatus::SAFE_ASYNC_STATUS_INTE;
    return true;
}

bool NativeSafeAsyncWork::IsMaxQueueSize()
{
    return (queue_.size() > maxQueueSize_ &&
           maxQueueSize_ > 0 &&
           status_ != SafeAsyncStatus::SAFE_ASYNC_STATUS_CLOSING &&
           status_ != SafeAsyncStatus::SAFE_ASYNC_STATUS_CLOSED);
}

SafeAsyncCode NativeSafeAsyncWork::ValidEngineCheck()
{
    if (!NativeEngine::IsAlive(engine_)) {
        HILOG_ERROR("napi_env has been destoryed");
        return SafeAsyncCode::SAFE_ASYNC_FAILED;
    } else if (engineId_ != engine_->GetId()) {
        LOG_IF_SPECIAL(engine_, UNLIKELY(engine_->IsCrossThreadCheckEnabled()),
                       "current tsfn was created by dead env, "
                       "owner id: %{public}" PRIu64 ", current env id: %{public}" PRIu64,
                       engineId_, engine_->GetId());
        return SafeAsyncCode::SAFE_ASYNC_CLOSED;
    }
    return SafeAsyncCode::SAFE_ASYNC_OK;
}

SafeAsyncCode NativeSafeAsyncWork::Send(void* data, NativeThreadSafeFunctionCallMode mode)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (IsMaxQueueSize()) {
        HILOG_INFO("queue size bigger than max queue size");
        if (mode == NATIVE_TSFUNC_BLOCKING) {
            while (IsMaxQueueSize()) {
                condition_.wait(lock);
            }
        } else {
            return SafeAsyncCode::SAFE_ASYNC_QUEUE_FULL;
        }
    }

    if (status_ == SafeAsyncStatus::SAFE_ASYNC_STATUS_CLOSED ||
        status_ == SafeAsyncStatus::SAFE_ASYNC_STATUS_CLOSING) {
        if (threadCount_ == 0) {
            return SafeAsyncCode::SAFE_ASYNC_INVALID_ARGS;
        } else {
            threadCount_--;
            return SafeAsyncCode::SAFE_ASYNC_CLOSED;
        }
    } else {
        SafeAsyncCode checkRet = ValidEngineCheck();
        if (checkRet != SafeAsyncCode::SAFE_ASYNC_OK) {
            return checkRet;
        }
        queue_.emplace_back(data);
        auto ret = uv_async_send(&asyncHandler_);
        if (ret != 0) {
            HILOG_ERROR("uv async send failed in Send ret = %{public}d", ret);
            return SafeAsyncCode::SAFE_ASYNC_FAILED;
        }
    }

    return SafeAsyncCode::SAFE_ASYNC_OK;
}

SafeAsyncCode NativeSafeAsyncWork::Acquire()
{
    HILOG_DEBUG("NativeSafeAsyncWork::Acquire called");

    std::unique_lock<std::mutex> lock(mutex_);

    if (status_ == SafeAsyncStatus::SAFE_ASYNC_STATUS_CLOSED ||
        status_ == SafeAsyncStatus::SAFE_ASYNC_STATUS_CLOSING) {
        HILOG_WARN("Do not acquire, thread is closed!");
        return SafeAsyncCode::SAFE_ASYNC_CLOSED;
    }

    // increase thread count
    threadCount_++;

    return SafeAsyncCode::SAFE_ASYNC_OK;
}

SafeAsyncCode NativeSafeAsyncWork::Release(NativeThreadSafeFunctionReleaseMode mode)
{
    HILOG_DEBUG("NativeSafeAsyncWork::Release called");

    std::unique_lock<std::mutex> lock(mutex_);

    if (status_ == SafeAsyncStatus::SAFE_ASYNC_STATUS_CLOSED ||
        status_ == SafeAsyncStatus::SAFE_ASYNC_STATUS_CLOSING) {
        HILOG_WARN("Do not release, thread is closed!");
        return SafeAsyncCode::SAFE_ASYNC_CLOSED;
    }

    if (threadCount_ == 0) {
        HILOG_ERROR("Do not release, thread count is zero.");
        return SafeAsyncCode::SAFE_ASYNC_INVALID_ARGS;
    }

    // decrease thread count
    threadCount_--;

    if (mode == NativeThreadSafeFunctionReleaseMode::NATIVE_TSFUNC_ABORT) {
        status_ = SafeAsyncStatus::SAFE_ASYNC_STATUS_CLOSING;
        if (maxQueueSize_ > 0) {
            condition_.notify_one();
        }
    }

    if (threadCount_ == 0 ||
        mode == NativeThreadSafeFunctionReleaseMode::NATIVE_TSFUNC_ABORT) {
        SafeAsyncCode checkRet = ValidEngineCheck();
        if (checkRet != SafeAsyncCode::SAFE_ASYNC_OK) {
            return checkRet;
        }
        // trigger async handle
        auto ret = uv_async_send(&asyncHandler_);
        if (ret != 0) {
            HILOG_ERROR("uv async send failed in Release ret = %{public}d", ret);
            return SafeAsyncCode::SAFE_ASYNC_FAILED;
        }
    }

    return SafeAsyncCode::SAFE_ASYNC_OK;
}

bool NativeSafeAsyncWork::Ref()
{
    if (!IsSameTid()) {
        HILOG_ERROR("tid not same");
        return false;
    }

    uv_ref(reinterpret_cast<uv_handle_t*>(&asyncHandler_));

    return true;
}

bool NativeSafeAsyncWork::Unref()
{
    if (!IsSameTid()) {
        HILOG_ERROR("tid not same");
        return false;
    }

    uv_unref(reinterpret_cast<uv_handle_t*>(&asyncHandler_));

    return true;
}

void* NativeSafeAsyncWork::GetContext()
{
    return context_;
}

void NativeSafeAsyncWork::ProcessAsyncHandle()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (status_ == SafeAsyncStatus::SAFE_ASYNC_STATUS_CLOSED) {
        HILOG_ERROR("Process failed, thread is closed!");
        return;
    }

    if (status_ == SafeAsyncStatus::SAFE_ASYNC_STATUS_CLOSING) {
        HILOG_ERROR("thread is closing!");
        CloseHandles();
        return;
    }

    size_t size = queue_.size();
    void* data = nullptr;

    auto vm = engine_->GetEcmaVm();
    panda::LocalScope scope(vm);
#ifdef ENABLE_CONTAINER_SCOPE
    ContainerScope containerScope(containerScopeId_, engine_->IsContainerScopeEnabled());
#endif
    TryCatch tryCatch(reinterpret_cast<napi_env>(engine_));

    bool isValidTraceId = SaveAndSetTraceId();
    while (size > 0) {
        data = queue_.front();

        // when queue is full, notify send.
        if (size == maxQueueSize_ && maxQueueSize_ > 0) {
            condition_.notify_one();
        }

        napi_value func_ = (ref_ == nullptr) ? nullptr : ref_->Get(engine_);
        lock.unlock();
        if (callJsCallback_ != nullptr) {
            callJsCallback_(engine_, func_, context_, data);
        } else {
            CallJs(engine_, func_, context_, data);
        }
        lock.lock();

        if (tryCatch.HasCaught()) {
            engine_->HandleUncaughtException();
        }
        queue_.pop_front();
        size--;
    }
    RestoreTraceId(isValidTraceId);

    if (!queue_.empty()) {
        auto ret = uv_async_send(&asyncHandler_);
        if (ret != 0) {
            HILOG_ERROR("uv async send failed in ProcessAsyncHandle ret = %{public}d", ret);
        }
    }

    if (queue_.empty() && threadCount_ == 0) {
        CloseHandles();
    }
}

SafeAsyncCode NativeSafeAsyncWork::CloseHandles()
{
    HILOG_DEBUG("NativeSafeAsyncWork::CloseHandles called");

    if (status_ == SafeAsyncStatus::SAFE_ASYNC_STATUS_CLOSED) {
        HILOG_INFO("Close failed, thread is closed!");
        return SafeAsyncCode::SAFE_ASYNC_CLOSED;
    }

    status_ = SafeAsyncStatus::SAFE_ASYNC_STATUS_CLOSED;

    // close async handler
    uv_close(reinterpret_cast<uv_handle_t*>(&asyncHandler_), [](uv_handle_t* handle) {
        NativeSafeAsyncWork* that = NativeAsyncWork::DereferenceOf(&NativeSafeAsyncWork::asyncHandler_,
            reinterpret_cast<uv_async_t*>(handle));
        that->CleanUp();
    });

    return SafeAsyncCode::SAFE_ASYNC_OK;
}

void NativeSafeAsyncWork::CleanUp()
{
    HILOG_DEBUG("NativeSafeAsyncWork::CleanUp called");
    bool isValidTraceId = SaveAndSetTraceId();
    if (finalizeCallback_ != nullptr) {
        finalizeCallback_(engine_, finalizeData_, context_);
    }

    // clean data
    while (!queue_.empty()) {
        if (callJsCallback_ != nullptr) {
            callJsCallback_(nullptr, nullptr, context_, queue_.front());
        } else {
            CallJs(nullptr, nullptr, context_, queue_.front());
        }
        queue_.pop_front();
    }
    ClearTraceId(isValidTraceId);

    delete this;
}

bool NativeSafeAsyncWork::IsSameTid()
{
    return pthread_self() == engine_->GetTid();
}

napi_status NativeSafeAsyncWork::PostTask(void *data, int32_t priority, bool isTail)
{
#if defined(ENABLE_EVENT_HANDLER)
    HILOG_DEBUG("NativeSafeAsyncWork::PostTask called");
    std::unique_lock<std::mutex> lock(eventHandlerMutex_);
    if (runner_ == nullptr || engine_ == nullptr) {
        HILOG_ERROR("post task failed due to nullptr engine or eventRunner");
        return napi_status::napi_generic_failure;
    }
    // the task will be execute at main thread or worker thread
    auto task = [this, data]() {
        HILOG_DEBUG("The task is executing in main thread or worker thread");
        panda::LocalScope scope(engine_->GetEcmaVm());
        napi_value func_ = (ref_ == nullptr) ? nullptr : ref_->Get(engine_);
        bool isValidTraceId = SaveAndSetTraceId();
        if (callJsCallback_ != nullptr) {
            callJsCallback_(engine_, func_, context_, data);
        } else {
            CallJs(engine_, func_, context_, data);
        }
        RestoreTraceId(isValidTraceId);
    };

    if (UNLIKELY(eventHandler_ == nullptr)) {
        eventHandler_ = std::make_shared<EventHandler>(runner_);
    }
    bool res = false;
    if (isTail) {
        HILOG_DEBUG("The task is posted from tail");
        res = eventHandler_->PostTask(task, static_cast<EventQueue::Priority>(priority));
    } else {
        HILOG_DEBUG("The task is posted from head");
        res = eventHandler_->PostTaskAtFront(task, std::string(), static_cast<EventQueue::Priority>(priority));
    }

    return res ? napi_status::napi_ok : napi_status::napi_generic_failure;
#else
    HILOG_WARN("EventHandler feature is not supported");
    return napi_status::napi_generic_failure;
#endif
}

void NativeSafeAsyncWork::InitSafeAsyncWorkTraceId()
{
#ifdef ENABLE_HITRACE
    std::call_once(g_SafeWorkParamUpdated, []() {
        char napiTraceIdEnabled[TRACEID_PARAM_SIZE] = {0};
        int ret = GetParameter("persist.hiviewdfx.napitraceid.enabled", "false",
            napiTraceIdEnabled, sizeof(napiTraceIdEnabled));
        if (ret > 0 && strcmp(napiTraceIdEnabled, "true") == 0) {
            g_SafeWorkTraceIdEnabled.store(true);
        }
    });
    bool createdTraceId = false;
    HiTraceId thisId = HiTraceChain::GetId();
    if (g_SafeWorkTraceIdEnabled.load() && (!thisId.IsValid())) {
        thisId = HiTraceChain::Begin("New NativeAsyncWork", 0);
        createdTraceId = true;
    }
    if (thisId.IsValid()) {
        taskTraceId_ = HiTraceChain::CreateSpan();
    }
    if (createdTraceId) {
        OHOS::HiviewDFX::HiTraceChain::ClearId();
    }
#endif
}
