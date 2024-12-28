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
#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_EVENT_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_EVENT_H
#include <atomic>
#include <cstdint>
#include <shared_mutex>
#include <string>
#include "native_safe_async_work.h"

typedef struct CallbackWrapper_ {
    std::function<void()> cb;
    std::atomic<uint64_t> handleId;
} CallbackWrapper;

class NativeEvent : public NativeSafeAsyncWork {
public:
    NativeEvent(NativeEngine* engine,
                napi_value func,
                napi_value asyncResource,
                napi_value asyncResourceName,
                size_t maxQueueSize,
                size_t threadCount,
                void* finalizeData,
                NativeFinalize finalizeCallback,
                void* context,
                NativeThreadSafeFunctionCallJs callJsCallback);
    bool Init() override;
    virtual napi_status SendCancelableEvent(const std::function<void(void*)> &callback,
                                            void* data,
                                            int32_t priority,
                                            const char* name,
                                            uint64_t* handleId);
    virtual napi_status CancelEvent(const char* name, uint64_t handleId);
    virtual SafeAsyncCode UvCancelEvent(uint64_t handleId);
    static void CreateDefaultFunction(NativeEngine* eng, napi_threadsafe_function &defaultFunc,
                                      std::shared_mutex &eventMutex);
    static void DestoryDefaultFunction(bool release, napi_threadsafe_function &defaultFunc,
                                       std::shared_mutex &eventMutex);
protected:
    //unique key for event
    std::atomic<uint64_t> sequence_;
    uint64_t GenerateUniqueID();
    napi_status SendEventByEventHandler(const std::function<void()> &task, uint64_t eventId,
                                        int32_t priority, const char* name, uint64_t* handleId);
    napi_status SendEventByUv(const std::function<void()> &task, uint64_t eventId,
                              const char* name, uint64_t* handleId);
    napi_status SendConvertStatus2NapiStatus(void* data, NativeThreadSafeFunctionCallMode mode);
};
#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_EVENT_H */