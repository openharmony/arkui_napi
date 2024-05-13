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

#ifndef FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_STORAGE_JSSTORAGE_H
#define FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_STORAGE_JSSTORAGE_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "securec.h"

#include <map>
#include <string>

namespace StorageBufferSize {
    constexpr size_t EVENT_TYPE_SIZE = 32;
    constexpr size_t KEY_BUFFER_SIZE = 32;
    constexpr size_t VALUE_BUFFER_SIZE = 128;
};

enum StorageArgIndex {
    FIRST_ARG = 0,
    SECOND_ARG,
    THIRD_ARG,
    FOURTH_ARG,
};

enum StorageEvent {
    STORAGE_EVENT_UNKNOWN = -1,
    STORAGE_EVENT_CHANGE,
    STORAGE_EVENT_CLEAR,
    STORAGE_EVENT_ERROR,
};

enum GetParaType {
    GET = 0,
    SET,
    DELETE,
    CLEAR,
};

/***********************************************
 * Storage Constructor
 ***********************************************/

struct EventHandler {
    napi_ref callbackRef = nullptr;
    EventHandler* next = nullptr;
};

class EventListener {
public:
    EventListener() : type_(nullptr), handlers_(nullptr) {}

    virtual ~EventListener() {}

    void Add(napi_env env, napi_value handler);

    void Del(napi_env env, napi_value handler);

    void Clear(napi_env env);

    const char* type_;
    EventHandler* handlers_;

protected:
    EventHandler* Find(napi_env env, napi_value handler);
};

class StorageObjectInfo {
public:
    explicit StorageObjectInfo(napi_env env);

    virtual ~StorageObjectInfo();

    void On(const char* type, napi_value handler);

    void Off(const char* type, napi_value handler = nullptr);

    void Emit(napi_value thisArg, const char* type);

protected:
    StorageEvent Find(const char* type) const;

private:
    napi_env env_;
    EventListener listeners_[3];
};

struct StorageAsyncContext {
    napi_env env = nullptr;
    napi_async_work work = nullptr;

    char key[StorageBufferSize::KEY_BUFFER_SIZE] = { 0 };
    size_t keyLen = 0;
    char value[StorageBufferSize::VALUE_BUFFER_SIZE] = { 0 };
    size_t valueLen = 0;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;

    int status = 0;
    StorageObjectInfo* objectInfo = nullptr;
};

/***********************************************
 * Method in Storage
 ***********************************************/

bool GetParameOfGet(napi_env env, size_t argc, napi_value *argv, StorageAsyncContext *asyncContext);
bool JSStorageGetParameterFromArg(napi_env env, size_t argc, napi_value *argv,
                                  StorageAsyncContext *asyncContext, GetParaType paraType);

void CreateAsyncWorkOfGet(napi_env env, StorageAsyncContext *asyncContext, napi_value resource, napi_value result);
void CreateAsyncWorkOfSet(napi_env env, StorageAsyncContext *asyncContext, napi_value resource, napi_value result);
void CreateAsyncWorkOfDelete(napi_env env, StorageAsyncContext *asyncContext, napi_value resource, napi_value result);
void CreateAsyncWorkOfClear(napi_env env, StorageAsyncContext *asyncContext, napi_value resource, napi_value result);

#endif