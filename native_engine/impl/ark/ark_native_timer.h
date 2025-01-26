/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_ARK_NATIVE_TIMER_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_ARK_NATIVE_TIMER_H

#include "ark_native_engine.h"
#include "ecmascript/napi/include/jsnapi_expo.h"
#if !defined(PREVIEW) && !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
#include <uv.h>
#endif

using TimerCallbackFunc = panda::TimerCallbackFunc;
using EcmaVM = panda::EcmaVM;
using JSNApi = panda::JSNApi;

class NativeTimerCallbackInfo {
public:
    NativeTimerCallbackInfo(ArkNativeEngine* engine, TimerCallbackFunc cb, void* data, bool repeat)
        : engine_(engine), cb_(cb), data_(data), repeat_(repeat) {}

    ~NativeTimerCallbackInfo()
    {
        engine_ = nullptr;
        cb_ = nullptr;
        data_ = nullptr;
        if (timerReq_ == nullptr) {
            return;
        }
        timerReq_->data = nullptr;
        uv_timer_stop(timerReq_);
        uv_close(reinterpret_cast<uv_handle_t*>(timerReq_), [](uv_handle_t* handle) {
            if (handle != nullptr) {
                delete (uv_timer_t*)handle;
                handle = nullptr;
            }
        });
    }
    static void* TimerTaskCallback(
        EcmaVM* vm, void* data, TimerCallbackFunc func, uint64_t timeout, bool repeat);
    static void CancelTimerCallback(void* timerCallbackInfo);

    bool Init(uint64_t timeout);
    void Insert();
    void Erase();
    static void ReleaseTimerList(ArkNativeEngine* engine);

private:
    static void TimerCallback(uv_timer_t* handle);

    ArkNativeEngine* engine_ {nullptr};
    TimerCallbackFunc cb_ {nullptr};
    void* data_ {nullptr};
    bool repeat_ {};
    bool timeoutExecuting_ {};
    uv_timer_t* timerReq_ {nullptr};
    NativeTimerCallbackInfo* prev_ {nullptr};
    NativeTimerCallbackInfo* next_ {nullptr};
};
#endif