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

#include "native_safe_async_work.h"

#include "ecmascript/napi/include/jsnapi.h"
#include "napi/native_api.h"
#include "native_async_work.h"
#include "native_engine.h"
#include "native_value.h"
#include "securec.h"
#include "utils/log.h"

#ifdef ENABLE_CONTAINER_SCOPE
#include "core/common/container_scope.h"
#endif

#ifdef ENABLE_CONTAINER_SCOPE
using OHOS::Ace::ContainerScope;
#endif

// static methods start
void NativeSafeAsyncWork::AsyncCallback(uv_async_t* asyncHandler)
{
    HILOG_INFO("NativeSafeAsyncWork::AsyncCallback called");
    NativeSafeAsyncWork* that = NativeAsyncWork::DereferenceOf(&NativeSafeAsyncWork::asyncHandler_, asyncHandler);

    that->ProcessAsyncHandle();
}

void NativeSafeAsyncWork::CallJs(NativeEngine* engine, napi_value js_call_func, void* context, void* data)
{
    HILOG_INFO("NativeSafeAsyncWork::CallJs called");

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
    :engine_(engine), maxQueueSize_(maxQueueSize),
    threadCount_(threadCount), finalizeData_(finalizeData), finalizeCallback_(finalizeCallback),
    context_(context), callJsCallback_(callJsCallback)
{
    errno_t err = EOK;
    err = memset_s(&asyncContext_, sizeof(asyncContext_), 0, sizeof(asyncContext_));
    if (err != EOK) {
        return;
    }

    asyncContext_.napiAsyncResource = asyncResource;
    asyncContext_.napiAsyncResourceName = asyncResourceName;
    if (func != nullptr) {
        uint32_t initialRefcount = 1;
        ref_ = engine->CreateReference(func, initialRefcount);
    }

#ifdef ENABLE_CONTAINER_SCOPE
    containerScopeId_ = ContainerScope::CurrentId();
#endif
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
    HILOG_INFO("NativeSafeAsyncWork::Init called");

    uv_loop_t* loop = engine_->GetUVLoop();
    if (loop == nullptr) {
        HILOG_ERROR("Get loop failed");
        return false;
    }

    int ret = uv_async_init(loop, &asyncHandler_, AsyncCallback);
    if (ret != 0) {
        HILOG_ERROR("uv async init failed %d", ret);
        uv_close(reinterpret_cast<uv_handle_t*>(&asyncHandler_), nullptr);
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

SafeAsyncCode NativeSafeAsyncWork::Send(void* data, NativeThreadSafeFunctionCallMode mode)
{
    HILOG_INFO("NativeSafeAsyncWork::Send called");

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
        queue_.push(data);
        auto ret = uv_async_send(&asyncHandler_);
        if (ret != 0) {
            HILOG_ERROR("uv async send failed %d", ret);
            return SafeAsyncCode::SAFE_ASYNC_FAILED;
        }
    }

    return SafeAsyncCode::SAFE_ASYNC_OK;
}

SafeAsyncCode NativeSafeAsyncWork::Acquire()
{
    HILOG_INFO("NativeSafeAsyncWork::Acquire called");

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
    HILOG_INFO("NativeSafeAsyncWork::Release called");

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
        // trigger async handle
        auto ret = uv_async_send(&asyncHandler_);
        if (ret != 0) {
            HILOG_ERROR("uv async send failed %d", ret);
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
    HILOG_INFO("NativeSafeAsyncWork::ProcessAsyncHandle called");

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
    ContainerScope containerScope(containerScopeId_);
#endif
    HILOG_INFO("queue size %d", (int32_t)size);
    TryCatch tryCatch(reinterpret_cast<napi_env>(engine_));
    while (size > 0) {
        data = queue_.front();

        // when queue is full, notify send.
        if (size == maxQueueSize_ && maxQueueSize_ > 0) {
            condition_.notify_one();
        }

        napi_value func_ = (ref_ == nullptr) ? nullptr : ref_->Get();
        if (callJsCallback_ != nullptr) {
            callJsCallback_(engine_, func_, context_, data);
        } else {
            CallJs(engine_, func_, context_, data);
        }

        if (tryCatch.HasCaught()) {
            engine_->HandleUncaughtException();
        }
        queue_.pop();
        size--;
    }

    if (size == 0 && threadCount_ == 0) {
        CloseHandles();
    }
}

SafeAsyncCode NativeSafeAsyncWork::CloseHandles()
{
    HILOG_INFO("NativeSafeAsyncWork::CloseHandles called");

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
    HILOG_INFO("NativeSafeAsyncWork::CleanUp called");

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
        queue_.pop();
    }
}

bool NativeSafeAsyncWork::IsSameTid()
{
    auto tid = pthread_self();
    return (tid == engine_->GetTid()) ? true : false;
}
