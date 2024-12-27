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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_SAFE_ASYNC_WORK_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_SAFE_ASYNC_WORK_H

#include "native_value.h"

#include <mutex>
#include <deque>
#include <uv.h>
#ifdef LINUX_PLATFORM
#include <condition_variable>
#endif

#include "native_async_context.h"

#if defined(ENABLE_EVENT_HANDLER)
namespace OHOS::AppExecFwk {
    class EventHandler;
}
#endif

enum class SafeAsyncCode {
    UNKNOWN = 0,
    SAFE_ASYNC_QUEUE_FULL,
    SAFE_ASYNC_INVALID_ARGS,
    SAFE_ASYNC_CLOSED,
    SAFE_ASYNC_FAILED,
    SAFE_ASYNC_OK,
};

enum class SafeAsyncStatus {
    UNKNOW = 0,
    SAFE_ASYNC_STATUS_INTE,
    SAFE_ASYNC_STATUS_CLOSING,
    SAFE_ASYNC_STATUS_CLOSED,
};

class NativeSafeAsyncWork {
public:
    static void AsyncCallback(uv_async_t* asyncHandler);
    static void CallJs(NativeEngine* engine, napi_value js_call_func, void* context, void* data);

    NativeSafeAsyncWork(NativeEngine* engine,
                        napi_value func,
                        napi_value asyncResource,
                        napi_value asyncResourceName,
                        size_t maxQueueSize,
                        size_t threadCount,
                        void* finalizeData,
                        NativeFinalize finalizeCallback,
                        void* context,
                        NativeThreadSafeFunctionCallJs callJsCallback);

    virtual ~NativeSafeAsyncWork();
    virtual bool Init();
    virtual SafeAsyncCode Send(void* data, NativeThreadSafeFunctionCallMode mode);
    virtual SafeAsyncCode Acquire();
    virtual SafeAsyncCode Release(NativeThreadSafeFunctionReleaseMode mode);
    virtual bool Ref();
    virtual bool Unref();
    virtual void* GetContext();
    virtual napi_status PostTask(void *data, int32_t priority, bool isTail);

protected:
    void ProcessAsyncHandle();
    SafeAsyncCode CloseHandles();
    void CleanUp();
    bool IsSameTid();
    bool IsMaxQueueSize();

    SafeAsyncCode ValidEngineCheck();

    NativeEngine* engine_ = nullptr;
    uint64_t engineId_ = 0;
    NativeReference* ref_ = nullptr;
    size_t maxQueueSize_ = 0;
    size_t threadCount_ = 0;
    void* finalizeData_ = nullptr;
    NativeFinalize finalizeCallback_ = nullptr;
    void* context_ = nullptr;
    NativeThreadSafeFunctionCallJs callJsCallback_ = nullptr;
    NativeAsyncContext asyncContext_;
    uv_async_t asyncHandler_;
    std::mutex mutex_;
    std::deque<void*> queue_;
    std::condition_variable condition_;
    SafeAsyncStatus status_ = SafeAsyncStatus::UNKNOW;
#if defined(ENABLE_EVENT_HANDLER)
    std::mutex eventHandlerMutex_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> eventHandler_ = nullptr;
#endif

#ifdef ENABLE_CONTAINER_SCOPE
    int32_t containerScopeId_;
#endif
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_SAFE_ASYNC_WORK_H */
