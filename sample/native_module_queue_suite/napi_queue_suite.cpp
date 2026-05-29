/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <algorithm>
#include <cmath>
#include <cstring>
#include <deque>
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

/***********************************************
 * Constants and Structs
 ***********************************************/
static constexpr int REQUIRED_ARGS_ONE = 1;
static constexpr int REQUIRED_ARGS_TWO = 2;
static constexpr int REQUIRED_ARGS_THREE = 3;
static constexpr int ARG_INDEX_ZERO = 0;
static constexpr int ARG_INDEX_ONE = 1;
static constexpr int ARG_INDEX_TWO = 2;
static constexpr size_t QUEUE_MAX_SIZE = 1024;
static constexpr size_t QUEUE_NAME_SIZE = 128;
static constexpr uint32_t INITIAL_REF_COUNT = 1;
static constexpr size_t QUEUE_EVENT_COUNT = 3;
static constexpr double EPSILON = 1e-10;

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

/***********************************************
 * Event Listener Class
 ***********************************************/
class QueueEventListener {
public:
    QueueEventListener() : handlers_(nullptr) {}
    ~QueueEventListener() {}

    void Add(napi_env env, napi_value handler)
    {
        auto* node = new QueueEventHandler();
        node->next = handlers_;
        handlers_ = node;
        napi_create_reference(env, handler, INITIAL_REF_COUNT, &node->callbackRef);
    }

    void Remove(napi_env env, napi_value handler)
    {
        QueueEventHandler* prev = nullptr;
        for (QueueEventHandler* curr = handlers_; curr != nullptr;) {
            napi_value callback = nullptr;
            napi_get_reference_value(env, curr->callbackRef, &callback);
            bool isEquals = false;
            napi_strict_equals(env, handler, callback, &isEquals);
            if (isEquals) {
                if (prev == nullptr) {
                    handlers_ = curr->next;
                } else {
                    prev->next = curr->next;
                }
                napi_delete_reference(env, curr->callbackRef);
                auto* toDelete = curr;
                curr = curr->next;
                delete toDelete;
            } else {
                prev = curr;
                curr = curr->next;
            }
        }
    }

    void Clear(napi_env env)
    {
        for (QueueEventHandler* curr = handlers_; curr != nullptr;) {
            QueueEventHandler* next = curr->next;
            napi_delete_reference(env, curr->callbackRef);
            delete curr;
            curr = next;
        }
        handlers_ = nullptr;
    }

    void Emit(napi_env env, napi_value thisArg, size_t argc = 0, const napi_value* argv = nullptr)
    {
        for (QueueEventHandler* curr = handlers_; curr != nullptr; curr = curr->next) {
            napi_value callback = nullptr;
            napi_value result = nullptr;
            napi_get_reference_value(env, curr->callbackRef, &callback);
            napi_value actualThisArg = thisArg;
            if (actualThisArg == nullptr) {
                napi_get_undefined(env, &actualThisArg);
            }
            napi_call_function(env, actualThisArg, callback, argc, argv, &result);
        }
    }

    QueueEventHandler* handlers_;
};

/***********************************************
 * QueueObjectInfo Class
 ***********************************************/
class QueueObjectInfo {
public:
    explicit QueueObjectInfo(napi_env env) : env_(env), data_(), listeners_() {}
    virtual ~QueueObjectInfo()
    {
        listeners_[QUEUE_EVENT_PUSH].Clear(env_);
        listeners_[QUEUE_EVENT_POP].Clear(env_);
        listeners_[QUEUE_EVENT_CLEAR].Clear(env_);
    }

    void PushBack(double value)
    {
        if (data_.size() < QUEUE_MAX_SIZE) {
            data_.push_back(value);
        }
    }

    void PushFront(double value)
    {
        if (data_.size() < QUEUE_MAX_SIZE) {
            data_.push_front(value);
        }
    }

    bool PopBack(double& value)
    {
        if (data_.empty()) {
            return false;
        }
        value = data_.back();
        data_.pop_back();
        return true;
    }

    bool PopFront(double& value)
    {
        if (data_.empty()) {
            return false;
        }
        value = data_.front();
        data_.pop_front();
        return true;
    }

    bool PeekBack(double& value) const
    {
        if (data_.empty()) {
            return false;
        }
        value = data_.back();
        return true;
    }

    bool PeekFront(double& value) const
    {
        if (data_.empty()) {
            return false;
        }
        value = data_.front();
        return true;
    }

    size_t Size() const { return data_.size(); }
    bool IsEmpty() const { return data_.empty(); }
    bool IsFull() const { return data_.size() >= QUEUE_MAX_SIZE; }

    void ClearQueue()
    {
        data_.clear();
    }

    std::vector<double> ToArray() const
    {
        return std::vector<double>(data_.begin(), data_.end());
    }

    bool Contains(double value) const
    {
        for (double v : data_) {
            if (std::abs(v - value) < EPSILON) {
                return true;
            }
        }
        return false;
    }

    void Reverse()
    {
        std::reverse(data_.begin(), data_.end());
    }

    void Sort(bool ascending)
    {
        if (ascending) {
            std::sort(data_.begin(), data_.end());
        } else {
            std::sort(data_.begin(), data_.end(), std::greater<double>());
        }
    }

    void OnEvent(const char* type, napi_value handler)
    {
        QueueEvent event = FindEvent(type);
        if (event == QUEUE_EVENT_UNKNOWN) {
            return;
        }
        listeners_[event].Add(env_, handler);
    }

    void OffEvent(const char* type, napi_value handler = nullptr)
    {
        QueueEvent event = FindEvent(type);
        if (event == QUEUE_EVENT_UNKNOWN) {
            return;
        }
        if (handler == nullptr) {
            listeners_[event].Clear(env_);
        } else {
            listeners_[event].Remove(env_, handler);
        }
    }

    void EmitEvent(napi_value thisArg, const char* type, double elementValue)
    {
        QueueEvent event = FindEvent(type);
        if (event == QUEUE_EVENT_UNKNOWN) {
            return;
        }
        napi_value valNode = nullptr;
        napi_create_double(env_, elementValue, &valNode);
        napi_value argv[REQUIRED_ARGS_ONE] = { valNode };
        listeners_[event].Emit(env_, thisArg, REQUIRED_ARGS_ONE, argv);
    }

    void EmitClearEvent(napi_value thisArg)
    {
        listeners_[QUEUE_EVENT_CLEAR].Emit(env_, thisArg, 0, nullptr);
    }

    const std::deque<double>& GetData() const { return data_; }
    void SetData(const std::deque<double>& data) { data_ = data; }

private:
    napi_env env_;
    std::deque<double> data_;
    QueueEventListener listeners_[QUEUE_EVENT_COUNT];

    QueueEvent FindEvent(const char* type) const
    {
        if (strcmp(type, "push") == 0) {
            return QUEUE_EVENT_PUSH;
        }
        if (strcmp(type, "pop") == 0) {
            return QUEUE_EVENT_POP;
        }
        if (strcmp(type, "clear") == 0) {
            return QUEUE_EVENT_CLEAR;
        }
        return QUEUE_EVENT_UNKNOWN;
    }
};

/***********************************************
 * Helper Functions
 ***********************************************/
static bool GetDouble(napi_env env, napi_value value, double* result)
{
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, value, &type) != napi_ok || type != napi_number) {
        return false;
    }
    return napi_get_value_double(env, value, result) == napi_ok;
}

static bool GetInt32(napi_env env, napi_value value, int32_t* result)
{
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, value, &type) != napi_ok || type != napi_number) {
        return false;
    }
    return napi_get_value_int32(env, value, result) == napi_ok;
}

static QueueObjectInfo* UnwrapQueue(napi_env env, napi_value value)
{
    QueueObjectInfo* info = nullptr;
    if (napi_unwrap(env, value, reinterpret_cast<void**>(&info)) == napi_ok) {
        return info;
    }
    return nullptr;
}

/***********************************************
 * NAPI Queue Constructor and Basic Methods
 ***********************************************/
static napi_value JSQueueConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    auto* objectInfo = new QueueObjectInfo(env);
    napi_status status = napi_wrap(
        env, thisVar, objectInfo,
        [](napi_env env, void* data, void* hint) {
            auto* objectInfo = static_cast<QueueObjectInfo*>(data);
            if (objectInfo != nullptr) {
                delete objectInfo;
            }
        },
        nullptr, nullptr);
    if (status != napi_ok) {
        delete objectInfo;
    }
    return thisVar;
}

static napi_value JSQueuePushBack(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "requires 1 parameter");
    double value = 0.0;
    NAPI_ASSERT(env, GetDouble(env, argv[ARG_INDEX_ZERO], &value), "parameter must be double");

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");
    objectInfo->PushBack(value);
    objectInfo->EmitEvent(thisVar, "push", value);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value JSQueuePushFront(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "requires 1 parameter");
    double value = 0.0;
    NAPI_ASSERT(env, GetDouble(env, argv[ARG_INDEX_ZERO], &value), "parameter must be double");

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");
    objectInfo->PushFront(value);
    objectInfo->EmitEvent(thisVar, "push", value);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value JSQueuePopBack(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    double value = 0.0;
    napi_value result = nullptr;
    if (objectInfo->PopBack(value)) {
        napi_create_double(env, value, &result);
        objectInfo->EmitEvent(thisVar, "pop", value);
    } else {
        napi_get_undefined(env, &result);
    }
    return result;
}

static napi_value JSQueuePopFront(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    double value = 0.0;
    napi_value result = nullptr;
    if (objectInfo->PopFront(value)) {
        napi_create_double(env, value, &result);
        objectInfo->EmitEvent(thisVar, "pop", value);
    } else {
        napi_get_undefined(env, &result);
    }
    return result;
}

static napi_value JSQueuePeekBack(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    double value = 0.0;
    napi_value result = nullptr;
    if (objectInfo->PeekBack(value)) {
        napi_create_double(env, value, &result);
    } else {
        napi_get_undefined(env, &result);
    }
    return result;
}

static napi_value JSQueuePeekFront(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    double value = 0.0;
    napi_value result = nullptr;
    if (objectInfo->PeekFront(value)) {
        napi_create_double(env, value, &result);
    } else {
        napi_get_undefined(env, &result);
    }
    return result;
}

static napi_value JSQueueSize(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    napi_value result = nullptr;
    napi_create_int32(env, static_cast<int32_t>(objectInfo->Size()), &result);
    return result;
}

static napi_value JSQueueIsEmpty(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    napi_value result = nullptr;
    napi_get_boolean(env, objectInfo->IsEmpty(), &result);
    return result;
}

static napi_value JSQueueIsFull(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    napi_value result = nullptr;
    napi_get_boolean(env, objectInfo->IsFull(), &result);
    return result;
}

static napi_value JSQueueClear(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");
    objectInfo->ClearQueue();
    objectInfo->EmitClearEvent(thisVar);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value JSQueueToArray(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    std::vector<double> arr = objectInfo->ToArray();
    napi_value result = nullptr;
    napi_create_array_with_length(env, arr.size(), &result);
    for (size_t i = 0; i < arr.size(); i++) {
        napi_value element = nullptr;
        napi_create_double(env, arr[i], &element);
        napi_set_element(env, result, i, element);
    }
    return result;
}

static napi_value JSQueueContains(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "requires 1 parameter");
    double value = 0.0;
    NAPI_ASSERT(env, GetDouble(env, argv[ARG_INDEX_ZERO], &value), "parameter must be double");

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    napi_value result = nullptr;
    napi_get_boolean(env, objectInfo->Contains(value), &result);
    return result;
}

static napi_value JSQueueReverse(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");
    objectInfo->Reverse();

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value JSQueueSort(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));

    bool ascending = true;
    if (argc >= REQUIRED_ARGS_ONE) {
        NAPI_CALL(env, napi_get_value_bool(env, argv[ARG_INDEX_ZERO], &ascending));
    }

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");
    objectInfo->Sort(ascending);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

/***********************************************
 * Advanced Event and Operations Methods
 ***********************************************/
static napi_value JSQueueOn(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "requires 2 parameters");

    char eventType[QUEUE_NAME_SIZE] = { 0 };
    size_t eventTypeLen = 0;
    napi_get_value_string_utf8(env, argv[ARG_INDEX_ZERO], eventType, QUEUE_NAME_SIZE, &eventTypeLen);

    napi_valuetype callbackType = napi_undefined;
    napi_typeof(env, argv[ARG_INDEX_ONE], &callbackType);
    NAPI_ASSERT(env, callbackType == napi_function, "second parameter must be function");

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");
    objectInfo->OnEvent(eventType, argv[ARG_INDEX_ONE]);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value JSQueueOff(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "requires at least 1 parameter");

    char eventType[QUEUE_NAME_SIZE] = { 0 };
    size_t eventTypeLen = 0;
    napi_get_value_string_utf8(env, argv[ARG_INDEX_ZERO], eventType, QUEUE_NAME_SIZE, &eventTypeLen);

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    if (argc > REQUIRED_ARGS_ONE) {
        napi_valuetype callbackType = napi_undefined;
        napi_typeof(env, argv[ARG_INDEX_ONE], &callbackType);
        NAPI_ASSERT(env, callbackType == napi_function, "second parameter must be function");
        objectInfo->OffEvent(eventType, argv[ARG_INDEX_ONE]);
    } else {
        objectInfo->OffEvent(eventType);
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value JSQueuePushAll(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "requires 1 parameter");

    bool isArray = false;
    napi_is_array(env, argv[ARG_INDEX_ZERO], &isArray);
    NAPI_ASSERT(env, isArray, "argument must be array");

    uint32_t length = 0;
    napi_get_array_length(env, argv[ARG_INDEX_ZERO], &length);

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    for (uint32_t i = 0; i < length; i++) {
        napi_value element = nullptr;
        napi_get_element(env, argv[ARG_INDEX_ZERO], i, &element);
        double value = 0.0;
        if (GetDouble(env, element, &value)) {
            objectInfo->PushBack(value);
            objectInfo->EmitEvent(thisVar, "push", value);
        }
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value JSQueuePopAll(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    std::vector<double> arr = objectInfo->ToArray();
    objectInfo->ClearQueue();

    napi_value result = nullptr;
    napi_create_array_with_length(env, arr.size(), &result);
    for (size_t i = 0; i < arr.size(); i++) {
        napi_value element = nullptr;
        napi_create_double(env, arr[i], &element);
        napi_set_element(env, result, i, element);
        objectInfo->EmitEvent(thisVar, "pop", arr[i]);
    }
    return result;
}

static napi_value JSQueueSwap(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    std::vector<double> arr = objectInfo->ToArray();
    if (arr.size() >= REQUIRED_ARGS_TWO) {
        double temp = arr.front();
        arr.front() = arr.back();
        arr.back() = temp;
        objectInfo->ClearQueue();
        for (double v : arr) {
            objectInfo->PushBack(v);
        }
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value JSQueueRotate(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "requires 1 parameter");

    int32_t n = 0;
    NAPI_ASSERT(env, GetInt32(env, argv[ARG_INDEX_ZERO], &n), "parameter must be integer");

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    std::vector<double> arr = objectInfo->ToArray();
    if (!arr.empty()) {
        int size = static_cast<int>(arr.size());
        n = ((n % size) + size) % size;
        std::vector<double> rotated;
        rotated.reserve(size);
        for (int i = n; i < size; i++) {
            rotated.push_back(arr[i]);
        }
        for (int i = 0; i < n; i++) {
            rotated.push_back(arr[i]);
        }
        objectInfo->ClearQueue();
        for (double v : rotated) {
            objectInfo->PushBack(v);
        }
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value JSQueueConcat(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "requires 1 parameter");

    QueueObjectInfo* thisQueue = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, thisQueue != nullptr, "failed to unwrap this object");

    QueueObjectInfo* otherQueue = UnwrapQueue(env, argv[ARG_INDEX_ZERO]);
    NAPI_ASSERT(env, otherQueue != nullptr, "parameter must be a Queue instance");

    std::vector<double> arr2 = otherQueue->ToArray();
    for (double val : arr2) {
        thisQueue->PushBack(val);
        thisQueue->EmitEvent(thisVar, "push", val);
    }

    return thisVar;
}

static napi_value JSQueueCompare(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "requires 1 parameter");

    QueueObjectInfo* thisQueue = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, thisQueue != nullptr, "failed to unwrap this object");

    QueueObjectInfo* otherQueue = UnwrapQueue(env, argv[ARG_INDEX_ZERO]);
    NAPI_ASSERT(env, otherQueue != nullptr, "parameter must be a Queue instance");

    std::vector<double> arr1 = thisQueue->ToArray();
    std::vector<double> arr2 = otherQueue->ToArray();

    bool equals = (arr1.size() == arr2.size());
    if (equals) {
        for (size_t i = 0; i < arr1.size(); i++) {
            if (std::abs(arr1[i] - arr2[i]) >= EPSILON) {
                equals = false;
                break;
            }
        }
    }

    napi_value result = nullptr;
    napi_get_boolean(env, equals, &result);
    return result;
}

/***********************************************
 * JS Array-like Callback Utility Methods
 ***********************************************/
static napi_value JSQueueForEach(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "requires 1 parameter");

    napi_valuetype callbackType = napi_undefined;
    napi_typeof(env, argv[ARG_INDEX_ZERO], &callbackType);
    NAPI_ASSERT(env, callbackType == napi_function, "parameter must be function");

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    std::vector<double> arr = objectInfo->ToArray();
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    for (size_t i = 0; i < arr.size(); i++) {
        napi_value jsValue = nullptr;
        napi_value jsIndex = nullptr;
        napi_create_double(env, arr[i], &jsValue);
        napi_create_int32(env, static_cast<int32_t>(i), &jsIndex);

        napi_value cbArgs[REQUIRED_ARGS_TWO] = { jsValue, jsIndex };
        napi_value cbResult = nullptr;
        napi_call_function(env, undefined, argv[ARG_INDEX_ZERO], REQUIRED_ARGS_TWO, cbArgs, &cbResult);
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value JSQueueMap(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "requires 1 parameter");

    napi_valuetype callbackType = napi_undefined;
    napi_typeof(env, argv[ARG_INDEX_ZERO], &callbackType);
    NAPI_ASSERT(env, callbackType == napi_function, "parameter must be function");

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    std::vector<double> arr = objectInfo->ToArray();
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value resultArray = nullptr;
    napi_create_array_with_length(env, arr.size(), &resultArray);

    for (size_t i = 0; i < arr.size(); i++) {
        napi_value jsValue = nullptr;
        napi_value jsIndex = nullptr;
        napi_create_double(env, arr[i], &jsValue);
        napi_create_int32(env, static_cast<int32_t>(i), &jsIndex);

        napi_value cbArgs[REQUIRED_ARGS_TWO] = { jsValue, jsIndex };
        napi_value cbResult = nullptr;
        napi_call_function(env, undefined, argv[ARG_INDEX_ZERO], REQUIRED_ARGS_TWO, cbArgs, &cbResult);

        napi_set_element(env, resultArray, i, cbResult);
    }

    return resultArray;
}

static napi_value JSQueueFilter(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "requires 1 parameter");

    napi_valuetype callbackType = napi_undefined;
    napi_typeof(env, argv[ARG_INDEX_ZERO], &callbackType);
    NAPI_ASSERT(env, callbackType == napi_function, "parameter must be function");

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    std::vector<double> arr = objectInfo->ToArray();
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value resultArray = nullptr;
    napi_create_array(env, &resultArray);

    uint32_t resultIndex = 0;
    for (size_t i = 0; i < arr.size(); i++) {
        napi_value jsValue = nullptr;
        napi_value jsIndex = nullptr;
        napi_create_double(env, arr[i], &jsValue);
        napi_create_int32(env, static_cast<int32_t>(i), &jsIndex);

        napi_value cbArgs[REQUIRED_ARGS_TWO] = { jsValue, jsIndex };
        napi_value cbResult = nullptr;
        napi_call_function(env, undefined, argv[ARG_INDEX_ZERO], REQUIRED_ARGS_TWO, cbArgs, &cbResult);

        bool keep = false;
        napi_get_value_bool(env, cbResult, &keep);
        if (keep) {
            napi_set_element(env, resultArray, resultIndex++, jsValue);
        }
    }

    return resultArray;
}

static napi_value JSQueueReduce(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "requires 2 parameters: callback, initialValue");

    napi_valuetype callbackType = napi_undefined;
    napi_typeof(env, argv[ARG_INDEX_ZERO], &callbackType);
    NAPI_ASSERT(env, callbackType == napi_function, "first parameter must be function");

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    std::vector<double> arr = objectInfo->ToArray();
    napi_value accumulator = argv[ARG_INDEX_ONE];
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    for (size_t i = 0; i < arr.size(); i++) {
        napi_value jsValue = nullptr;
        napi_value jsIndex = nullptr;
        napi_create_double(env, arr[i], &jsValue);
        napi_create_int32(env, static_cast<int32_t>(i), &jsIndex);

        napi_value cbArgs[REQUIRED_ARGS_THREE] = { accumulator, jsValue, jsIndex };
        napi_value cbResult = nullptr;
        napi_call_function(env, undefined, argv[ARG_INDEX_ZERO], REQUIRED_ARGS_THREE, cbArgs, &cbResult);
        accumulator = cbResult;
    }

    return accumulator;
}

static napi_value JSQueueFind(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "requires 1 parameter");

    napi_valuetype callbackType = napi_undefined;
    napi_typeof(env, argv[ARG_INDEX_ZERO], &callbackType);
    NAPI_ASSERT(env, callbackType == napi_function, "parameter must be function");

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    std::vector<double> arr = objectInfo->ToArray();
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    for (size_t i = 0; i < arr.size(); i++) {
        napi_value jsValue = nullptr;
        napi_value jsIndex = nullptr;
        napi_create_double(env, arr[i], &jsValue);
        napi_create_int32(env, static_cast<int32_t>(i), &jsIndex);

        napi_value cbArgs[REQUIRED_ARGS_TWO] = { jsValue, jsIndex };
        napi_value cbResult = nullptr;
        napi_call_function(env, undefined, argv[ARG_INDEX_ZERO], REQUIRED_ARGS_TWO, cbArgs, &cbResult);

        bool match = false;
        napi_get_value_bool(env, cbResult, &match);
        if (match) {
            return jsValue;
        }
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value JSQueueFindIndex(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "requires 1 parameter");

    napi_valuetype callbackType = napi_undefined;
    napi_typeof(env, argv[ARG_INDEX_ZERO], &callbackType);
    NAPI_ASSERT(env, callbackType == napi_function, "parameter must be function");

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    std::vector<double> arr = objectInfo->ToArray();
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    for (size_t i = 0; i < arr.size(); i++) {
        napi_value jsValue = nullptr;
        napi_value jsIndex = nullptr;
        napi_create_double(env, arr[i], &jsValue);
        napi_create_int32(env, static_cast<int32_t>(i), &jsIndex);

        napi_value cbArgs[REQUIRED_ARGS_TWO] = { jsValue, jsIndex };
        napi_value cbResult = nullptr;
        napi_call_function(env, undefined, argv[ARG_INDEX_ZERO], REQUIRED_ARGS_TWO, cbArgs, &cbResult);

        bool match = false;
        napi_get_value_bool(env, cbResult, &match);
        if (match) {
            napi_value resIndex = nullptr;
            napi_create_int32(env, static_cast<int32_t>(i), &resIndex);
            return resIndex;
        }
    }

    napi_value result = nullptr;
    napi_create_int32(env, -1, &result);
    return result;
}

static napi_value JSQueueSome(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "requires 1 parameter");

    napi_valuetype callbackType = napi_undefined;
    napi_typeof(env, argv[ARG_INDEX_ZERO], &callbackType);
    NAPI_ASSERT(env, callbackType == napi_function, "parameter must be function");

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    std::vector<double> arr = objectInfo->ToArray();
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    bool hasSome = false;
    for (size_t i = 0; i < arr.size(); i++) {
        napi_value jsValue = nullptr;
        napi_value jsIndex = nullptr;
        napi_create_double(env, arr[i], &jsValue);
        napi_create_int32(env, static_cast<int32_t>(i), &jsIndex);

        napi_value cbArgs[REQUIRED_ARGS_TWO] = { jsValue, jsIndex };
        napi_value cbResult = nullptr;
        napi_call_function(env, undefined, argv[ARG_INDEX_ZERO], REQUIRED_ARGS_TWO, cbArgs, &cbResult);

        napi_get_value_bool(env, cbResult, &hasSome);
        if (hasSome) {
            break;
        }
    }

    napi_value result = nullptr;
    napi_get_boolean(env, hasSome, &result);
    return result;
}

static napi_value JSQueueEvery(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "requires 1 parameter");

    napi_valuetype callbackType = napi_undefined;
    napi_typeof(env, argv[ARG_INDEX_ZERO], &callbackType);
    NAPI_ASSERT(env, callbackType == napi_function, "parameter must be function");

    QueueObjectInfo* objectInfo = UnwrapQueue(env, thisVar);
    NAPI_ASSERT(env, objectInfo != nullptr, "failed to unwrap object");

    std::vector<double> arr = objectInfo->ToArray();
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    bool hasEvery = true;
    for (size_t i = 0; i < arr.size(); i++) {
        napi_value jsValue = nullptr;
        napi_value jsIndex = nullptr;
        napi_create_double(env, arr[i], &jsValue);
        napi_create_int32(env, static_cast<int32_t>(i), &jsIndex);

        napi_value cbArgs[REQUIRED_ARGS_TWO] = { jsValue, jsIndex };
        napi_value cbResult = nullptr;
        napi_call_function(env, undefined, argv[ARG_INDEX_ZERO], REQUIRED_ARGS_TWO, cbArgs, &cbResult);

        bool current = false;
        napi_get_value_bool(env, cbResult, &current);
        if (!current) {
            hasEvery = false;
            break;
        }
    }

    napi_value result = nullptr;
    napi_get_boolean(env, hasEvery, &result);
    return result;
}

/***********************************************
 * 25 Predefined Test Cases for Coverage Integration
 ***********************************************/
static constexpr size_t K_QUEUE_CASE_COUNT = 25;
static constexpr size_t K_QUEUE_ARG_COUNT = 2;
static constexpr size_t K_FIRST_CASE_NUMBER = 1;
static constexpr int K_CASE_NUMBER_WIDTH = 2;
static constexpr double PUSH_VALUE_MULTIPLIER = 2.0;
static constexpr int32_t INITIAL_COUNT_MULTIPLIER = 2;
static constexpr double CASE_PUSH_MULTIPLIER = 1.5;
static constexpr int32_t CASE_ROTATE_OFFSET = 10;
static constexpr int32_t EVEN_CASE_MODULUS = 2;

struct QueueCaseSpec {
    std::string name;
    int32_t initialCount;
    double pushValue;
    int32_t rotateSteps;
    bool sortAscending;
};

static std::string BuildIndexedName(const char* prefix, size_t caseNumber)
{
    std::string suffix = std::to_string(caseNumber);
    if (suffix.size() < static_cast<size_t>(K_CASE_NUMBER_WIDTH)) {
        suffix.insert(0, static_cast<std::string::size_type>(K_CASE_NUMBER_WIDTH - suffix.size()), '0');
    }
    return std::string(prefix) + suffix;
}

static QueueCaseSpec GetQueueCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    return {
        BuildIndexedName("queueCase", caseNumber),
        static_cast<int32_t>(caseNumber * INITIAL_COUNT_MULTIPLIER),
        static_cast<double>(caseNumber) * CASE_PUSH_MULTIPLIER,
        static_cast<int32_t>(caseNumber) - CASE_ROTATE_OFFSET,
        (caseNumber % EVEN_CASE_MODULUS == 0)
    };
}

static bool InitializeQueueFromArgs(napi_env env, napi_value jsArray, QueueObjectInfo& q)
{
    uint32_t length = 0;
    if (napi_get_array_length(env, jsArray, &length) != napi_ok) {
        return false;
    }
    for (uint32_t i = 0; i < length; i++) {
        napi_value item = nullptr;
        napi_get_element(env, jsArray, i, &item);
        double val = 0.0;
        if (GetDouble(env, item, &val)) {
            q.PushBack(val);
        }
    }
    return true;
}

static void ApplyQueueCaseMutations(QueueObjectInfo& q, const QueueCaseSpec& spec)
{
    q.PushFront(spec.pushValue);
    q.PushBack(spec.pushValue * PUSH_VALUE_MULTIPLIER);

    double tempVal = 0.0;
    q.PopFront(tempVal);
    q.PopBack(tempVal);

    if (spec.sortAscending) {
        q.Sort(true);
    } else {
        q.Sort(false);
    }

    std::vector<double> elements = q.ToArray();
    if (!elements.empty()) {
        int rotateSteps = spec.rotateSteps;
        int size = static_cast<int>(elements.size());
        rotateSteps = ((rotateSteps % size) + size) % size;
        std::rotate(elements.begin(), elements.begin() + rotateSteps, elements.end());
        q.ClearQueue();
        for (double v : elements) {
            q.PushBack(v);
        }
    }
}

static napi_value CreateCaseSummary(napi_env env, const QueueCaseSpec& spec, QueueObjectInfo& q)
{
    bool isEmpty = q.IsEmpty();
    bool isFull = q.IsFull();
    double pBack = 0.0;
    double pFront = 0.0;
    q.PeekBack(pBack);
    q.PeekFront(pFront);

    napi_value resultObj = nullptr;
    napi_create_object(env, &resultObj);

    napi_value nameVal = nullptr;
    napi_create_string_utf8(env, spec.name.c_str(), spec.name.size(), &nameVal);
    napi_set_named_property(env, resultObj, "name", nameVal);

    napi_value sizeVal = nullptr;
    napi_create_int32(env, static_cast<int32_t>(q.Size()), &sizeVal);
    napi_set_named_property(env, resultObj, "size", sizeVal);

    napi_value isEmptyVal = nullptr;
    napi_get_boolean(env, isEmpty, &isEmptyVal);
    napi_set_named_property(env, resultObj, "isEmpty", isEmptyVal);

    napi_value isFullVal = nullptr;
    napi_get_boolean(env, isFull, &isFullVal);
    napi_set_named_property(env, resultObj, "isFull", isFullVal);

    napi_value backVal = nullptr;
    napi_create_double(env, pBack, &backVal);
    napi_set_named_property(env, resultObj, "peekBack", backVal);

    napi_value frontVal = nullptr;
    napi_create_double(env, pFront, &frontVal);
    napi_set_named_property(env, resultObj, "peekFront", frontVal);

    napi_value arrayVal = nullptr;
    std::vector<double> finalArr = q.ToArray();
    napi_create_array_with_length(env, finalArr.size(), &arrayVal);
    for (size_t i = 0; i < finalArr.size(); i++) {
        napi_value item = nullptr;
        napi_create_double(env, finalArr[i], &item);
        napi_set_element(env, arrayVal, i, item);
    }
    napi_set_named_property(env, resultObj, "elements", arrayVal);

    return resultObj;
}

static napi_value RunQueueCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_QUEUE_ARG_COUNT;
    napi_value args[K_QUEUE_ARG_COUNT] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));

    uintptr_t caseIndex = reinterpret_cast<uintptr_t>(data);
    if (caseIndex >= K_QUEUE_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid queue case index");
        return nullptr;
    }

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "at least 1 argument required (initial values array)");
    bool isArray = false;
    napi_is_array(env, args[ARG_INDEX_ZERO], &isArray);
    NAPI_ASSERT(env, isArray, "first argument must be an array");

    QueueObjectInfo q(env);
    NAPI_ASSERT(env, InitializeQueueFromArgs(env, args[ARG_INDEX_ZERO], q), "failed to parse array");

    QueueCaseSpec spec = GetQueueCaseSpec(caseIndex);
    ApplyQueueCaseMutations(q, spec);

    if (argc >= K_QUEUE_ARG_COUNT) {
        napi_valuetype callbackType = napi_undefined;
        napi_typeof(env, args[ARG_INDEX_ONE], &callbackType);
        if (callbackType == napi_function) {
            napi_value undefined = nullptr;
            napi_get_undefined(env, &undefined);
            napi_value sizeNode = nullptr;
            napi_create_int32(env, static_cast<int32_t>(q.Size()), &sizeNode);
            napi_value cbArg[REQUIRED_ARGS_ONE] = { sizeNode };
            napi_value cbRes = nullptr;
            napi_call_function(env, undefined, args[ARG_INDEX_ONE], REQUIRED_ARGS_ONE, cbArg, &cbRes);
        }
    }

    return CreateCaseSummary(env, spec, q);
}

/***********************************************
 * Module Initialization and Registration
 ***********************************************/
static napi_value DefineQueueClass(napi_env env)
{
    const char* queueClassName = "Queue";
    napi_value queueClass = nullptr;
    static napi_property_descriptor queueDesc[] = {
        DECLARE_NAPI_FUNCTION("pushBack", JSQueuePushBack),
        DECLARE_NAPI_FUNCTION("pushFront", JSQueuePushFront),
        DECLARE_NAPI_FUNCTION("popBack", JSQueuePopBack),
        DECLARE_NAPI_FUNCTION("popFront", JSQueuePopFront),
        DECLARE_NAPI_FUNCTION("peekBack", JSQueuePeekBack),
        DECLARE_NAPI_FUNCTION("peekFront", JSQueuePeekFront),
        DECLARE_NAPI_FUNCTION("size", JSQueueSize),
        DECLARE_NAPI_FUNCTION("isEmpty", JSQueueIsEmpty),
        DECLARE_NAPI_FUNCTION("isFull", JSQueueIsFull),
        DECLARE_NAPI_FUNCTION("clear", JSQueueClear),
        DECLARE_NAPI_FUNCTION("toArray", JSQueueToArray),
        DECLARE_NAPI_FUNCTION("contains", JSQueueContains),
        DECLARE_NAPI_FUNCTION("reverse", JSQueueReverse),
        DECLARE_NAPI_FUNCTION("sort", JSQueueSort),
        DECLARE_NAPI_FUNCTION("on", JSQueueOn),
        DECLARE_NAPI_FUNCTION("off", JSQueueOff),
        DECLARE_NAPI_FUNCTION("pushAll", JSQueuePushAll),
        DECLARE_NAPI_FUNCTION("popAll", JSQueuePopAll),
        DECLARE_NAPI_FUNCTION("swap", JSQueueSwap),
        DECLARE_NAPI_FUNCTION("rotate", JSQueueRotate),
        DECLARE_NAPI_FUNCTION("concat", JSQueueConcat),
        DECLARE_NAPI_FUNCTION("compare", JSQueueCompare),
        DECLARE_NAPI_FUNCTION("forEach", JSQueueForEach),
        DECLARE_NAPI_FUNCTION("map", JSQueueMap),
        DECLARE_NAPI_FUNCTION("filter", JSQueueFilter),
        DECLARE_NAPI_FUNCTION("reduce", JSQueueReduce),
        DECLARE_NAPI_FUNCTION("find", JSQueueFind),
        DECLARE_NAPI_FUNCTION("findIndex", JSQueueFindIndex),
        DECLARE_NAPI_FUNCTION("some", JSQueueSome),
        DECLARE_NAPI_FUNCTION("every", JSQueueEvery),
    };
    napi_define_class(env, queueClassName, strlen(queueClassName), JSQueueConstructor, nullptr,
                      sizeof(queueDesc) / sizeof(queueDesc[0]), queueDesc, &queueClass);
    return queueClass;
}

static napi_value InitQueueSuite(napi_env env, napi_value exports)
{
    napi_value queueClass = DefineQueueClass(env);

    std::vector<std::string> exportNames;
    std::vector<napi_property_descriptor> descriptors;
    
    descriptors.push_back(napi_property_descriptor{
        "Queue", nullptr, nullptr, nullptr, nullptr, queueClass, napi_default, nullptr
    });

    exportNames.reserve(K_QUEUE_CASE_COUNT);
    for (size_t caseIndex = 0; caseIndex < K_QUEUE_CASE_COUNT; caseIndex++) {
        exportNames.emplace_back(BuildIndexedName("testQueueCase", caseIndex + K_FIRST_CASE_NUMBER));
        descriptors.push_back(napi_property_descriptor{
            exportNames.back().c_str(), nullptr, RunQueueCase, nullptr,
            nullptr, nullptr, napi_default, reinterpret_cast<void*>(static_cast<uintptr_t>(caseIndex))
        });
    }

    NAPI_CALL(env, napi_define_properties(env, exports, descriptors.size(), descriptors.data()));
    return exports;
}

static napi_module g_queueSuiteModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitQueueSuite,
    .nm_modname = "queue_suite",
    .nm_priv = nullptr,
    .reserved = { nullptr },
};

extern "C" __attribute__((constructor)) void RegisterQueueSuiteModule(void)
{
    napi_module_register(&g_queueSuiteModule);
}
