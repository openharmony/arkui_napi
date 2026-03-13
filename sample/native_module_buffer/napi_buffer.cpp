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

#include <cstddef>
#include <cstring>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

static constexpr size_t DEFAULT_BUFFER_SIZE = 1024;
static constexpr size_t MAX_BUFFER_SIZE = 1048576;
static constexpr int REQUIRED_ARGS_ONE = 1;
static constexpr int REQUIRED_ARGS_TWO = 2;

static napi_value CreateArrayBuffer(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    size_t bufferSize = DEFAULT_BUFFER_SIZE;
    if (argc > 0) {
        napi_valuetype valueType = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
        NAPI_ASSERT(env, valueType == napi_number, "Size must be a number");

        double sizeValue = 0;
        NAPI_CALL(env, napi_get_value_double(env, argv[0], &sizeValue));
        bufferSize = static_cast<size_t>(sizeValue);
        NAPI_ASSERT(env, bufferSize > 0, "Size must be positive");
        NAPI_ASSERT(env, bufferSize <= MAX_BUFFER_SIZE, "Size exceeds maximum");
    }

    void* data = nullptr;
    napi_value arrayBuffer = nullptr;
    NAPI_CALL(env, napi_create_arraybuffer(env, bufferSize, &data, &arrayBuffer));

    if (data != nullptr) {
        int32_t* intData = static_cast<int32_t*>(data);
        size_t intCount = bufferSize / sizeof(int32_t);
        for (size_t i = 0; i < intCount; i++) {
            intData[i] = static_cast<int32_t>(i);
        }
    }

    return arrayBuffer;
}

static napi_value CreateTypedArray(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires at least 1 argument");

    napi_valuetype typeType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &typeType));
    NAPI_ASSERT(env, typeType == napi_number, "Type must be a number");

    int32_t typeValue = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &typeValue));

    size_t elementCount = 16;
    if (argc > 1) {
        napi_valuetype countType = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[1], &countType));
        NAPI_ASSERT(env, countType == napi_number, "Count must be a number");

        double countValue = 0;
        NAPI_CALL(env, napi_get_value_double(env, argv[1], &countValue));
        elementCount = static_cast<size_t>(countValue);
    }

    napi_typedarray_type typedArrayType = napi_uint8_array;
    size_t elementSize = 1;

    switch (typeValue) {
        case 0:
            typedArrayType = napi_uint8_array;
            elementSize = sizeof(uint8_t);
            break;
        case REQUIRED_ARGS_ONE:
            typedArrayType = napi_int32_array;
            elementSize = sizeof(int32_t);
            break;
        case REQUIRED_ARGS_TWO:
            typedArrayType = napi_float64_array;
            elementSize = sizeof(double);
            break;
        default:
            NAPI_ASSERT(env, false, "Invalid typed array type");
    }

    void* data = nullptr;
    napi_value arrayBuffer = nullptr;
    size_t bufferSize = elementCount * elementSize;
    NAPI_CALL(env, napi_create_arraybuffer(env, bufferSize, &data, &arrayBuffer));

    napi_value typedArray = nullptr;
    size_t byteOffset = 0;
    NAPI_CALL(env, napi_create_typedarray(env, typedArrayType, elementCount,
                                      arrayBuffer, byteOffset, &typedArray));

    return typedArray;
}

static napi_value GetArrayBufferInfo(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Argument must be an object");

    bool isArrayBuffer = false;
    NAPI_CALL(env, napi_is_arraybuffer(env, argv[0], &isArrayBuffer));
    NAPI_ASSERT(env, isArrayBuffer, "Argument must be an ArrayBuffer");

    void* data = nullptr;
    size_t byteLength = 0;
    NAPI_CALL(env, napi_get_arraybuffer_info(env, argv[0], &data, &byteLength));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value dataValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, reinterpret_cast<int64_t>(data), &dataValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "data", dataValue));

    napi_value lengthValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, static_cast<int64_t>(byteLength), &lengthValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "length", lengthValue));

    return result;
}

static napi_value CopyArrayBuffer(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments");

    napi_valuetype srcType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &srcType));
    NAPI_ASSERT(env, srcType == napi_object, "Source must be an object");

    napi_valuetype dstType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[1], &dstType));
    NAPI_ASSERT(env, dstType == napi_object, "Destination must be an object");

    bool isSrcArrayBuffer = false;
    NAPI_CALL(env, napi_is_arraybuffer(env, argv[0], &isSrcArrayBuffer));
    NAPI_ASSERT(env, isSrcArrayBuffer, "Source must be an ArrayBuffer");

    bool isDstArrayBuffer = false;
    NAPI_CALL(env, napi_is_arraybuffer(env, argv[1], &isDstArrayBuffer));
    NAPI_ASSERT(env, isDstArrayBuffer, "Destination must be an ArrayBuffer");

    void* srcData = nullptr;
    size_t srcLength = 0;
    NAPI_CALL(env, napi_get_arraybuffer_info(env, argv[0], &srcData, &srcLength));

    void* dstData = nullptr;
    size_t dstLength = 0;
    NAPI_CALL(env, napi_get_arraybuffer_info(env, argv[1], &dstData, &dstLength));

    size_t copyLength = (srcLength < dstLength) ? srcLength : dstLength;
    errno_t err = memcpy_s(dstData, dstLength, srcData, copyLength);
    NAPI_ASSERT(env, err == EOK, "Memory copy failed");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int64(env, static_cast<int64_t>(copyLength), &result));

    return result;
}

static napi_value DetachArrayBuffer(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Argument must be an object");

    bool isArrayBuffer = false;
    NAPI_CALL(env, napi_is_arraybuffer(env, argv[0], &isArrayBuffer));
    NAPI_ASSERT(env, isArrayBuffer, "Argument must be an ArrayBuffer");

    void* data = nullptr;
    size_t byteLength = 0;
    NAPI_CALL(env, napi_get_arraybuffer_info(env, argv[0], &data, &byteLength));

    NAPI_CALL(env, napi_detach_arraybuffer(env, argv[0]));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int64(env, static_cast<int64_t>(byteLength), &result));

    return result;
}

static napi_value IsDetached(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Argument must be an object");

    bool isArrayBuffer = false;
    NAPI_CALL(env, napi_is_arraybuffer(env, argv[0], &isArrayBuffer));
    NAPI_ASSERT(env, isArrayBuffer, "Argument must be an ArrayBuffer");

    void* data = nullptr;
    size_t byteLength = 0;
    NAPI_CALL(env, napi_get_arraybuffer_info(env, argv[0], &data, &byteLength));

    bool isDetached = (data == nullptr && byteLength == 0);

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, isDetached, &result));

    return result;
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("createArrayBuffer", CreateArrayBuffer),
        DECLARE_NAPI_FUNCTION("createTypedArray", CreateTypedArray),
        DECLARE_NAPI_FUNCTION("getArrayBufferInfo", GetArrayBufferInfo),
        DECLARE_NAPI_FUNCTION("copyArrayBuffer", CopyArrayBuffer),
        DECLARE_NAPI_FUNCTION("detachArrayBuffer", DetachArrayBuffer),
        DECLARE_NAPI_FUNCTION("isDetached", IsDetached),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
                                      sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module bufferModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "buffer",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void BufferRegisterModule(void)
{
    napi_module_register(&bufferModule);
}
