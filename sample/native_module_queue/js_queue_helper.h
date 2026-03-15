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

#ifndef FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_QUEUE_JS_QUEUE_HELPER_H
#define FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_QUEUE_JS_QUEUE_HELPER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include <deque>
#include <string>
#include <vector>

constexpr size_t QUEUE_MAX_SIZE = 4096;
constexpr size_t QUEUE_NAME_SIZE = 128;

namespace QueueArgCount {
    constexpr size_t TWO = 2;
};

enum QueueEvent {
    QUEUE_EVENT_UNKNOWN = -1,
    QUEUE_EVENT_PUSH,
    QUEUE_EVENT_POP,
    QUEUE_EVENT_CLEAR,
};

struct QueueEventHandler {
    napi_ref callbackRef = nullptr;
    QueueEventHandler* next = nullptr;
};

class QueueEventListener {
public:
    QueueEventListener() : handlers_(nullptr) {}
    ~QueueEventListener() {}

    void Add(napi_env env, napi_value handler);
    void Remove(napi_env env, napi_value handler);
    void Clear(napi_env env);
    void Emit(napi_env env, napi_value thisArg);

    QueueEventHandler* handlers_;
};

class QueueObjectInfo {
public:
    explicit QueueObjectInfo(napi_env env);
    virtual ~QueueObjectInfo();

    void PushBack(double value);
    void PushFront(double value);
    bool PopBack(double& value);
    bool PopFront(double& value);
    bool PeekBack(double& value) const;
    bool PeekFront(double& value) const;
    size_t Size() const;
    bool IsEmpty() const;
    bool IsFull() const;
    void ClearQueue();
    std::vector<double> ToArray() const;
    bool Contains(double value) const;
    void Reverse();
    void Sort(bool ascending);

    void OnEvent(const char* type, napi_value handler);
    void OffEvent(const char* type, napi_value handler = nullptr);
    void EmitEvent(napi_value thisArg, const char* type);

private:
    napi_env env_;
    std::deque<double> data_;
    QueueEventListener listeners_[3];

    QueueEvent FindEvent(const char* type) const;
};

#endif
