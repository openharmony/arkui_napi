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

#include "js_queue_helper.h"

#include <algorithm>
#include <cmath>
#include <cstring>

/***********************************************
 * QueueEventListener Implementation
 ***********************************************/
void QueueEventListener::Add(napi_env env, napi_value handler)
{
    auto* node = new QueueEventHandler();
    node->next = handlers_;
    handlers_ = node;
    napi_create_reference(env, handler, 1, &node->callbackRef);
}

void QueueEventListener::Remove(napi_env env, napi_value handler)
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

void QueueEventListener::Clear(napi_env env)
{
    for (QueueEventHandler* curr = handlers_; curr != nullptr;) {
        QueueEventHandler* next = curr->next;
        napi_delete_reference(env, curr->callbackRef);
        delete curr;
        curr = next;
    }
    handlers_ = nullptr;
}

void QueueEventListener::Emit(napi_env env, napi_value thisArg)
{
    for (QueueEventHandler* curr = handlers_; curr != nullptr; curr = curr->next) {
        napi_value callback = nullptr;
        napi_value result = nullptr;
        napi_get_reference_value(env, curr->callbackRef, &callback);
        if (thisArg == nullptr) {
            napi_get_undefined(env, &thisArg);
        }
        napi_call_function(env, thisArg, callback, 0, nullptr, &result);
    }
}

/***********************************************
 * QueueObjectInfo Implementation
 ***********************************************/
QueueObjectInfo::QueueObjectInfo(napi_env env) : env_(env), data_(), listeners_() {}

QueueObjectInfo::~QueueObjectInfo()
{
    listeners_[QUEUE_EVENT_PUSH].Clear(env_);
    listeners_[QUEUE_EVENT_POP].Clear(env_);
    listeners_[QUEUE_EVENT_CLEAR].Clear(env_);
}

void QueueObjectInfo::PushBack(double value)
{
    if (data_.size() < QUEUE_MAX_SIZE) {
        data_.push_back(value);
    }
}

void QueueObjectInfo::PushFront(double value)
{
    if (data_.size() < QUEUE_MAX_SIZE) {
        data_.push_front(value);
    }
}

bool QueueObjectInfo::PopBack(double& value)
{
    if (data_.empty()) {
        return false;
    }
    value = data_.back();
    data_.pop_back();
    return true;
}

bool QueueObjectInfo::PopFront(double& value)
{
    if (data_.empty()) {
        return false;
    }
    value = data_.front();
    data_.pop_front();
    return true;
}

bool QueueObjectInfo::PeekBack(double& value) const
{
    if (data_.empty()) {
        return false;
    }
    value = data_.back();
    return true;
}

bool QueueObjectInfo::PeekFront(double& value) const
{
    if (data_.empty()) {
        return false;
    }
    value = data_.front();
    return true;
}

size_t QueueObjectInfo::Size() const { return data_.size(); }
bool QueueObjectInfo::IsEmpty() const { return data_.empty(); }
bool QueueObjectInfo::IsFull() const { return data_.size() >= QUEUE_MAX_SIZE; }

void QueueObjectInfo::ClearQueue()
{
    data_.clear();
}

std::vector<double> QueueObjectInfo::ToArray() const
{
    return std::vector<double>(data_.begin(), data_.end());
}

bool QueueObjectInfo::Contains(double value) const
{
    for (double v : data_) {
        if (std::abs(v - value) < 1e-10) {
            return true;
        }
    }
    return false;
}

void QueueObjectInfo::Reverse()
{
    std::reverse(data_.begin(), data_.end());
}

void QueueObjectInfo::Sort(bool ascending)
{
    if (ascending) {
        std::sort(data_.begin(), data_.end());
    } else {
        std::sort(data_.begin(), data_.end(), std::greater<double>());
    }
}

QueueEvent QueueObjectInfo::FindEvent(const char* type) const
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

void QueueObjectInfo::OnEvent(const char* type, napi_value handler)
{
    QueueEvent event = FindEvent(type);
    if (event == QUEUE_EVENT_UNKNOWN) {
        return;
    }
    listeners_[event].Add(env_, handler);
}

void QueueObjectInfo::OffEvent(const char* type, napi_value handler)
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

void QueueObjectInfo::EmitEvent(napi_value thisArg, const char* type)
{
    QueueEvent event = FindEvent(type);
    if (event == QUEUE_EVENT_UNKNOWN) {
        return;
    }
    listeners_[event].Emit(env_, thisArg);
}

/***********************************************
 * NAPI Queue Constructor
 ***********************************************/
static napi_value JSQueueConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, &data);

    auto objectInfo = new QueueObjectInfo(env);
    auto status = napi_wrap(
        env, thisVar, objectInfo,
        [](napi_env env, void* data, void* hint) {
            auto objectInfo = (QueueObjectInfo*)data;
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

/***********************************************
 * NAPI Queue Methods
 ***********************************************/
static napi_value JSQueuePushBack(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    double value;
    napi_get_value_double(env, argv[0], &value);

    QueueObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);
    objectInfo->PushBack(value);
    objectInfo->EmitEvent(thisVar, "push");

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value JSQueuePushFront(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    double value;
    napi_get_value_double(env, argv[0], &value);

    QueueObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);
    objectInfo->PushFront(value);
    objectInfo->EmitEvent(thisVar, "push");

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value JSQueuePopBack(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);

    double value = 0.0;
    napi_value result = nullptr;
    if (objectInfo->PopBack(value)) {
        napi_create_double(env, value, &result);
        objectInfo->EmitEvent(thisVar, "pop");
    } else {
        napi_get_undefined(env, &result);
    }
    return result;
}

static napi_value JSQueuePopFront(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);

    double value = 0.0;
    napi_value result = nullptr;
    if (objectInfo->PopFront(value)) {
        napi_create_double(env, value, &result);
        objectInfo->EmitEvent(thisVar, "pop");
    } else {
        napi_get_undefined(env, &result);
    }
    return result;
}

static napi_value JSQueuePeekBack(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);

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

    QueueObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);

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

    QueueObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);

    napi_value result = nullptr;
    napi_create_int32(env, static_cast<int32_t>(objectInfo->Size()), &result);
    return result;
}

static napi_value JSQueueIsEmpty(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);

    napi_value result = nullptr;
    napi_get_boolean(env, objectInfo->IsEmpty(), &result);
    return result;
}

static napi_value JSQueueIsFull(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);

    napi_value result = nullptr;
    napi_get_boolean(env, objectInfo->IsFull(), &result);
    return result;
}

static napi_value JSQueueClear(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);
    objectInfo->ClearQueue();
    objectInfo->EmitEvent(thisVar, "clear");

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value JSQueueToArray(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);

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
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    double value;
    napi_get_value_double(env, argv[0], &value);

    QueueObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);

    napi_value result = nullptr;
    napi_get_boolean(env, objectInfo->Contains(value), &result);
    return result;
}

static napi_value JSQueueReverse(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);
    objectInfo->Reverse();

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value JSQueueSort(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));

    bool ascending = true;
    if (argc >= 1) {
        napi_get_value_bool(env, argv[0], &ascending);
    }

    QueueObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);
    objectInfo->Sort(ascending);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value JSQueueOn(napi_env env, napi_callback_info info)
{
    size_t argc = QueueArgCount::TWO;
    napi_value argv[QueueArgCount::TWO] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= QueueArgCount::TWO, "requires 2 parameters");

    char eventType[QUEUE_NAME_SIZE] = { 0 };
    size_t eventTypeLen = 0;
    napi_get_value_string_utf8(env, argv[0], eventType, QUEUE_NAME_SIZE, &eventTypeLen);

    napi_valuetype callbackType = napi_undefined;
    napi_typeof(env, argv[1], &callbackType);
    NAPI_ASSERT(env, callbackType == napi_function, "second parameter must be function");

    QueueObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);
    objectInfo->OnEvent(eventType, argv[1]);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

// pushAll(values: number[]): void
static napi_value JSQueuePushAll(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    bool isArray = false;
    napi_is_array(env, argv[0], &isArray);
    NAPI_ASSERT(env, isArray, "argument must be array");

    uint32_t length = 0;
    napi_get_array_length(env, argv[0], &length);

    QueueObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);

    for (uint32_t i = 0; i < length; i++) {
        napi_value element = nullptr;
        napi_get_element(env, argv[0], i, &element);
        double value;
        napi_get_value_double(env, element, &value);
        objectInfo->PushBack(value);
    }
    objectInfo->EmitEvent(thisVar, "push");

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

// popAll(): number[]
static napi_value JSQueuePopAll(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);

    std::vector<double> arr = objectInfo->ToArray();
    objectInfo->ClearQueue();

    napi_value result = nullptr;
    napi_create_array_with_length(env, arr.size(), &result);
    for (size_t i = 0; i < arr.size(); i++) {
        napi_value element = nullptr;
        napi_create_double(env, arr[i], &element);
        napi_set_element(env, result, i, element);
    }
    if (!arr.empty()) {
        objectInfo->EmitEvent(thisVar, "pop");
    }
    return result;
}

// swap(): void - swap front and back
static napi_value JSQueueSwap(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

    QueueObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);

    std::vector<double> arr = objectInfo->ToArray();
    if (arr.size() >= QueueArgCount::TWO) {
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

// rotate(n: number): void - rotate elements by n positions
static napi_value JSQueueRotate(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    int32_t n;
    napi_get_value_int32(env, argv[0], &n);

    QueueObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);

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

static napi_value JSQueueOff(napi_env env, napi_callback_info info)
{
    size_t argc = QueueArgCount::TWO;
    napi_value argv[QueueArgCount::TWO] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires at least 1 parameter");

    char eventType[QUEUE_NAME_SIZE] = { 0 };
    size_t eventTypeLen = 0;
    napi_get_value_string_utf8(env, argv[0], eventType, QUEUE_NAME_SIZE, &eventTypeLen);

    QueueObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);

    if (argc > 1) {
        napi_valuetype callbackType = napi_undefined;
        napi_typeof(env, argv[1], &callbackType);
        NAPI_ASSERT(env, callbackType == napi_function, "second parameter must be function");
        objectInfo->OffEvent(eventType, argv[1]);
    } else {
        objectInfo->OffEvent(eventType);
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

/***********************************************
 * Module export and register
 ***********************************************/
static napi_value QueueExport(napi_env env, napi_value exports)
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
    };
    napi_define_class(env, queueClassName, strlen(queueClassName), JSQueueConstructor, nullptr,
                      sizeof(queueDesc) / sizeof(queueDesc[0]), queueDesc, &queueClass);

    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_PROPERTY("Queue", queueClass),
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

static napi_module g_queueModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = QueueExport,
    .nm_modname = "queue",
    .nm_priv = nullptr,
    .reserved = { nullptr },
};

extern "C" __attribute__((constructor)) void QueueRegister()
{
    napi_module_register(&g_queueModule);
}
