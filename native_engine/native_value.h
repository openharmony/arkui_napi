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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_VALUE_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_VALUE_H

#include <cstddef>
#include <cstdint>
#include <string>

#include "interfaces/inner_api/napi/native_node_api.h"

namespace panda {
class JsiRuntimeCallInfo;
namespace ecmascript {
struct JsFrameInfo;
}
}

class NativeEngine;
class NativeReference;

// To be refactor
typedef napi_value (*NapiNativeCallback)(napi_env env, panda::JsiRuntimeCallInfo*);
typedef void (*NativeFinalize)(NativeEngine* engine, void* data, void* hint);
typedef void (*NativeAsyncExecuteCallback)(NativeEngine* engine, void* data);
typedef void (*NativeAsyncCompleteCallback)(NativeEngine* engine, int status, void* data);
typedef void* (*DetachCallback)(NativeEngine* engine, void* value, void* hint);

using ErrorPos = std::pair<uint32_t, uint32_t>;
using NativeThreadSafeFunctionCallJs =
    void (*)(NativeEngine* env, napi_value js_callback, void* context, void* data);

struct NativeObjectInfo {
    static NativeObjectInfo* CreateNewInstance() { return new(std::nothrow) NativeObjectInfo(); }
    NativeEngine* engine = nullptr;
    void* nativeObject = nullptr;
    NativeFinalize callback = nullptr;
    void* hint = nullptr;
};

struct NapiFunctionInfo {
    static NapiFunctionInfo* CreateNewInstance() { return new(std::nothrow) NapiFunctionInfo(); }
    NapiNativeCallback callback = nullptr;
    void* data = nullptr;
    napi_env env = nullptr;
#ifdef ENABLE_CONTAINER_SCOPE
    int32_t scopeId = -1;
#endif
};

typedef void (*NaitveFinalize)(NativeEngine* env, void* data, void* hint);

// To be delete
enum NativeValueType {
    NATIVE_UNDEFINED,
    NATIVE_NULL,
    NATIVE_BOOLEAN,
    NATIVE_NUMBER,
    NATIVE_STRING,
    NATIVE_SYMBOL,
    NATIVE_OBJECT,
    NATIVE_FUNCTION,
    NATIVE_EXTERNAL,
    NATIVE_BIGINT,
};

// To be delete
enum NativeThreadSafeFunctionCallMode {
    NATIVE_TSFUNC_NONBLOCKING,
    NATIVE_TSFUNC_BLOCKING,
};

// To be delete
enum NativeThreadSafeFunctionReleaseMode {
    NATIVE_TSFUNC_RELEASE,
    NATIVE_TSFUNC_ABORT,
};

struct JSValueWrapper {
    JSValueWrapper()
    {
        u.ptr = nullptr;
        tag = 0;
    }
    template<typename T>
    JSValueWrapper(T value)
    {
        *(T*)this = value;
    }
    template<typename T> operator T()
    {
        return *(T*)this;
    }
    template<typename T> JSValueWrapper& operator=(T value)
    {
        *(T*)this = value;
        return *this;
    }
    union {
        int32_t int32;
        double float64;
        void* ptr;
    } u;
    int64_t tag = 0;
};

struct NapiTypeTag {
    uint64_t lower;
    uint64_t upper;
};

// To be delete
enum NativeTypedArrayType {
    NATIVE_INT8_ARRAY,
    NATIVE_UINT8_ARRAY,
    NATIVE_UINT8_CLAMPED_ARRAY,
    NATIVE_INT16_ARRAY,
    NATIVE_UINT16_ARRAY,
    NATIVE_INT32_ARRAY,
    NATIVE_UINT32_ARRAY,
    NATIVE_FLOAT32_ARRAY,
    NATIVE_FLOAT64_ARRAY,
    NATIVE_BIGINT64_ARRAY,
    NATIVE_BIGUINT64_ARRAY,
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_VALUE_H */