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

#include <cstdint>
#include <cstring>
#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

namespace {

// ---------------------------------------------------------------------------
// Named constants – no magic numbers allowed
// ---------------------------------------------------------------------------
static constexpr size_t SMALL_BUFFER_BYTES = 16;
static constexpr size_t MEDIUM_BUFFER_BYTES = 64;
static constexpr size_t LARGE_BUFFER_BYTES = 256;
static constexpr size_t EXTRA_LARGE_BUFFER_BYTES = 1024;

static constexpr size_t TYPED_ARRAY_LENGTH = 8;
static constexpr size_t TYPED_ARRAY_OFFSET_ZERO = 0;

static constexpr size_t BYTES_PER_INT8 = 1;
static constexpr size_t BYTES_PER_UINT8 = 1;
static constexpr size_t BYTES_PER_INT16 = 2;
static constexpr size_t BYTES_PER_UINT16 = 2;
static constexpr size_t BYTES_PER_INT32 = 4;
static constexpr size_t BYTES_PER_UINT32 = 4;
static constexpr size_t BYTES_PER_FLOAT32 = 4;
static constexpr size_t BYTES_PER_FLOAT64 = 8;
static constexpr size_t BYTES_PER_BIGINT64 = 8;
static constexpr size_t BYTES_PER_BIGUINT64 = 8;

static constexpr size_t DATAVIEW_DEFAULT_OFFSET = 0;
static constexpr size_t DATAVIEW_CUSTOM_OFFSET = 4;
static constexpr size_t DATAVIEW_LENGTH = 8;

static constexpr uint8_t FILL_BYTE_VALUE = 0xAB;
static constexpr uint8_t SECOND_FILL_BYTE = 0xCD;
static constexpr uint8_t BUFFER_COPY_BYTE = 0x42;
static constexpr uint8_t EXTERNAL_FILL_BYTE = 0x55;

static constexpr int32_t INT8_TEST_VALUE = -42;
static constexpr int32_t UINT8_TEST_VALUE = 200;
static constexpr int32_t INT16_TEST_VALUE = -1000;
static constexpr int32_t UINT16_TEST_VALUE = 50000;
static constexpr int32_t INT32_TEST_VALUE = -100000;
static constexpr uint32_t UINT32_TEST_VALUE = 3000000;
static constexpr float FLOAT32_TEST_VALUE = 3.14f;
static constexpr double FLOAT64_TEST_VALUE = 2.718281828;

static constexpr size_t ZERO_LENGTH = 0;
static constexpr size_t SINGLE_ELEMENT = 1;
static constexpr size_t TWO_ELEMENTS = 2;

static constexpr size_t MULTI_VIEW_OFFSET_A = 0;
static constexpr size_t MULTI_VIEW_OFFSET_B = 4;
static constexpr size_t MULTI_VIEW_ELEM_COUNT = 4;

static constexpr uint32_t MODULE_VERSION = 1;
static constexpr uint32_t NO_MODULE_FLAGS = 0;

// ---------------------------------------------------------------------------
// Helper utilities
// ---------------------------------------------------------------------------
static bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value)
{
    napi_value napiValue = nullptr;
    if (napi_get_boolean(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

static bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int32(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

static bool SetNamedDouble(napi_env env, napi_value object, const char* name, double value)
{
    napi_value napiValue = nullptr;
    if (napi_create_double(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

static bool SetNamedString(napi_env env, napi_value object, const char* name, const char* value)
{
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

static napi_value CreateResultObject(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    return result;
}

// ---------------------------------------------------------------------------
// ArrayBuffer helpers
// ---------------------------------------------------------------------------
static napi_value CreateArrayBufferWithSize(napi_env env, size_t byteLength, void** data)
{
    napi_value arrayBuffer = nullptr;
    NAPI_CALL(env, napi_create_arraybuffer(env, byteLength, data, &arrayBuffer));
    return arrayBuffer;
}

static void FillBuffer(void* data, size_t length, uint8_t value)
{
    (void)memset_s(data, length, value, length);
}

// ---------------------------------------------------------------------------
// Test 01: napi_create_arraybuffer – basic creation
// ---------------------------------------------------------------------------
static napi_value TestCreateArrayBuffer(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, SMALL_BUFFER_BYTES, &data);

    SetNamedBool(env, result, "created", arrayBuffer != nullptr);
    SetNamedBool(env, result, "dataNotNull", data != nullptr);
    SetNamedInt32(env, result, "byteLength", static_cast<int32_t>(SMALL_BUFFER_BYTES));
    return result;
}

// ---------------------------------------------------------------------------
// Test 02: napi_get_arraybuffer_info
// ---------------------------------------------------------------------------
static napi_value TestGetArrayBufferInfo(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, MEDIUM_BUFFER_BYTES, &createData);

    void* infoData = nullptr;
    size_t infoLength = ZERO_LENGTH;
    NAPI_CALL(env, napi_get_arraybuffer_info(env, arrayBuffer, &infoData, &infoLength));

    SetNamedBool(env, result, "dataMatch", createData == infoData);
    SetNamedBool(env, result, "lengthMatch", infoLength == MEDIUM_BUFFER_BYTES);
    SetNamedInt32(env, result, "byteLength", static_cast<int32_t>(infoLength));
    return result;
}

// ---------------------------------------------------------------------------
// Test 03: napi_is_arraybuffer – positive
// ---------------------------------------------------------------------------
static napi_value TestIsArrayBuffer(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, SMALL_BUFFER_BYTES, &data);

    bool isArrayBuffer = false;
    NAPI_CALL(env, napi_is_arraybuffer(env, arrayBuffer, &isArrayBuffer));
    SetNamedBool(env, result, "isArrayBuffer", isArrayBuffer);
    return result;
}

// ---------------------------------------------------------------------------
// Test 04: napi_is_arraybuffer – negative (plain object)
// ---------------------------------------------------------------------------
static napi_value TestIsArrayBufferNegative(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value plainObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &plainObject));

    bool isArrayBuffer = true;
    NAPI_CALL(env, napi_is_arraybuffer(env, plainObject, &isArrayBuffer));
    SetNamedBool(env, result, "isNotArrayBuffer", !isArrayBuffer);
    return result;
}

// ---------------------------------------------------------------------------
// Test 05: napi_create_typedarray – Int8Array
// ---------------------------------------------------------------------------
static napi_value TestCreateTypedArrayInt8(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_INT8;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_int8_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    SetNamedBool(env, result, "created", typedArray != nullptr);
    SetNamedString(env, result, "type", "Int8Array");
    SetNamedInt32(env, result, "length", static_cast<int32_t>(TYPED_ARRAY_LENGTH));
    return result;
}

// ---------------------------------------------------------------------------
// Test 06: napi_create_typedarray – Uint8Array
// ---------------------------------------------------------------------------
static napi_value TestCreateTypedArrayUint8(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_UINT8;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_uint8_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    SetNamedBool(env, result, "created", typedArray != nullptr);
    SetNamedString(env, result, "type", "Uint8Array");
    return result;
}

// ---------------------------------------------------------------------------
// Test 07: napi_create_typedarray – Uint8ClampedArray
// ---------------------------------------------------------------------------
static napi_value TestCreateTypedArrayUint8Clamped(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_UINT8;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_uint8_clamped_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    SetNamedBool(env, result, "created", typedArray != nullptr);
    SetNamedString(env, result, "type", "Uint8ClampedArray");
    return result;
}

// ---------------------------------------------------------------------------
// Test 08: napi_create_typedarray – Int16Array
// ---------------------------------------------------------------------------
static napi_value TestCreateTypedArrayInt16(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_INT16;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_int16_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    SetNamedBool(env, result, "created", typedArray != nullptr);
    SetNamedString(env, result, "type", "Int16Array");
    return result;
}

// ---------------------------------------------------------------------------
// Test 09: napi_create_typedarray – Uint16Array
// ---------------------------------------------------------------------------
static napi_value TestCreateTypedArrayUint16(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_UINT16;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_uint16_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    SetNamedBool(env, result, "created", typedArray != nullptr);
    SetNamedString(env, result, "type", "Uint16Array");
    return result;
}

// ---------------------------------------------------------------------------
// Test 10: napi_create_typedarray – Int32Array
// ---------------------------------------------------------------------------
static napi_value TestCreateTypedArrayInt32(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_INT32;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_int32_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    SetNamedBool(env, result, "created", typedArray != nullptr);
    SetNamedString(env, result, "type", "Int32Array");
    return result;
}

// ---------------------------------------------------------------------------
// Test 11: napi_create_typedarray – Uint32Array
// ---------------------------------------------------------------------------
static napi_value TestCreateTypedArrayUint32(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_UINT32;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_uint32_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    SetNamedBool(env, result, "created", typedArray != nullptr);
    SetNamedString(env, result, "type", "Uint32Array");
    return result;
}

// ---------------------------------------------------------------------------
// Test 12: napi_create_typedarray – Float32Array
// ---------------------------------------------------------------------------
static napi_value TestCreateTypedArrayFloat32(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_FLOAT32;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_float32_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    SetNamedBool(env, result, "created", typedArray != nullptr);
    SetNamedString(env, result, "type", "Float32Array");
    return result;
}

// ---------------------------------------------------------------------------
// Test 13: napi_create_typedarray – Float64Array
// ---------------------------------------------------------------------------
static napi_value TestCreateTypedArrayFloat64(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_FLOAT64;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_float64_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    SetNamedBool(env, result, "created", typedArray != nullptr);
    SetNamedString(env, result, "type", "Float64Array");
    return result;
}

// ---------------------------------------------------------------------------
// Test 14: napi_create_typedarray – BigInt64Array
// ---------------------------------------------------------------------------
static napi_value TestCreateTypedArrayBigInt64(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_BIGINT64;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_bigint64_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    SetNamedBool(env, result, "created", typedArray != nullptr);
    SetNamedString(env, result, "type", "BigInt64Array");
    return result;
}

// ---------------------------------------------------------------------------
// Test 15: napi_create_typedarray – BigUint64Array
// ---------------------------------------------------------------------------
static napi_value TestCreateTypedArrayBigUint64(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_BIGUINT64;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_biguint64_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    SetNamedBool(env, result, "created", typedArray != nullptr);
    SetNamedString(env, result, "type", "BigUint64Array");
    return result;
}

// ---------------------------------------------------------------------------
// Test 16: napi_get_typedarray_info – Int32Array
// ---------------------------------------------------------------------------
static napi_value TestGetTypedArrayInfo(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_INT32;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_int32_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    napi_typedarray_type arrayType = napi_int8_array;
    size_t length = ZERO_LENGTH;
    void* arrayData = nullptr;
    napi_value outBuffer = nullptr;
    size_t byteOffset = ZERO_LENGTH;
    NAPI_CALL(env, napi_get_typedarray_info(env, typedArray, &arrayType, &length, &arrayData, &outBuffer, &byteOffset));

    SetNamedBool(env, result, "typeMatch", arrayType == napi_int32_array);
    SetNamedBool(env, result, "lengthMatch", length == TYPED_ARRAY_LENGTH);
    SetNamedBool(env, result, "dataNotNull", arrayData != nullptr);
    SetNamedBool(env, result, "offsetZero", byteOffset == TYPED_ARRAY_OFFSET_ZERO);
    return result;
}

// ---------------------------------------------------------------------------
// Test 17: napi_create_dataview – basic creation
// ---------------------------------------------------------------------------
static napi_value TestCreateDataView(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, MEDIUM_BUFFER_BYTES, &data);

    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, DATAVIEW_LENGTH, arrayBuffer, DATAVIEW_DEFAULT_OFFSET, &dataView));

    SetNamedBool(env, result, "created", dataView != nullptr);
    SetNamedInt32(env, result, "length", static_cast<int32_t>(DATAVIEW_LENGTH));
    return result;
}

// ---------------------------------------------------------------------------
// Test 18: napi_get_dataview_info
// ---------------------------------------------------------------------------
static napi_value TestGetDataViewInfo(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, MEDIUM_BUFFER_BYTES, &data);

    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, DATAVIEW_LENGTH, arrayBuffer, DATAVIEW_CUSTOM_OFFSET, &dataView));

    size_t byteLength = ZERO_LENGTH;
    void* viewData = nullptr;
    napi_value outBuffer = nullptr;
    size_t byteOffset = ZERO_LENGTH;
    NAPI_CALL(env, napi_get_dataview_info(env, dataView, &byteLength, &viewData, &outBuffer, &byteOffset));

    SetNamedBool(env, result, "lengthMatch", byteLength == DATAVIEW_LENGTH);
    SetNamedBool(env, result, "dataNotNull", viewData != nullptr);
    SetNamedBool(env, result, "offsetMatch", byteOffset == DATAVIEW_CUSTOM_OFFSET);
    return result;
}

// ---------------------------------------------------------------------------
// Test 19: napi_is_dataview – positive
// ---------------------------------------------------------------------------
static napi_value TestIsDataView(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, MEDIUM_BUFFER_BYTES, &data);

    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, DATAVIEW_LENGTH, arrayBuffer, DATAVIEW_DEFAULT_OFFSET, &dataView));

    bool isDataView = false;
    NAPI_CALL(env, napi_is_dataview(env, dataView, &isDataView));
    SetNamedBool(env, result, "isDataView", isDataView);
    return result;
}

// ---------------------------------------------------------------------------
// Test 20: napi_is_dataview – negative
// ---------------------------------------------------------------------------
static napi_value TestIsDataViewNegative(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, SMALL_BUFFER_BYTES, &data);

    bool isDataView = true;
    NAPI_CALL(env, napi_is_dataview(env, arrayBuffer, &isDataView));
    SetNamedBool(env, result, "isNotDataView", !isDataView);
    return result;
}

// ---------------------------------------------------------------------------
// Test 21: napi_detach_arraybuffer
// ---------------------------------------------------------------------------
static napi_value TestDetachArrayBuffer(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, SMALL_BUFFER_BYTES, &data);

    NAPI_CALL(env, napi_detach_arraybuffer(env, arrayBuffer));

    bool isDetached = false;
    NAPI_CALL(env, napi_is_detached_arraybuffer(env, arrayBuffer, &isDetached));
    SetNamedBool(env, result, "detached", isDetached);
    return result;
}

// ---------------------------------------------------------------------------
// Test 22: napi_is_detached_arraybuffer – not detached
// ---------------------------------------------------------------------------
static napi_value TestIsDetachedArrayBufferFalse(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, SMALL_BUFFER_BYTES, &data);

    bool isDetached = true;
    NAPI_CALL(env, napi_is_detached_arraybuffer(env, arrayBuffer, &isDetached));
    SetNamedBool(env, result, "isNotDetached", !isDetached);
    return result;
}

// ---------------------------------------------------------------------------
// Test 23: napi_create_buffer
// ---------------------------------------------------------------------------
static napi_value TestCreateBuffer(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value buffer = nullptr;
    NAPI_CALL(env, napi_create_buffer(env, SMALL_BUFFER_BYTES, &data, &buffer));

    SetNamedBool(env, result, "created", buffer != nullptr);
    SetNamedBool(env, result, "dataNotNull", data != nullptr);
    SetNamedInt32(env, result, "byteLength", static_cast<int32_t>(SMALL_BUFFER_BYTES));
    return result;
}

// ---------------------------------------------------------------------------
// Test 24: napi_create_buffer_copy – helper to prepare source data
// ---------------------------------------------------------------------------
static void PrepareSourceData(uint8_t* source, size_t length, uint8_t fillValue)
{
    for (size_t i = ZERO_LENGTH; i < length; i++) {
        source[i] = fillValue;
    }
}

// Test 24: napi_create_buffer_copy
static napi_value TestCreateBufferCopy(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    uint8_t source[SMALL_BUFFER_BYTES] = {};
    PrepareSourceData(source, SMALL_BUFFER_BYTES, BUFFER_COPY_BYTE);

    void* resultData = nullptr;
    napi_value buffer = nullptr;
    NAPI_CALL(env, napi_create_buffer_copy(env, SMALL_BUFFER_BYTES, source, &resultData, &buffer));

    auto* copied = static_cast<uint8_t*>(resultData);
    bool dataMatch = true;
    for (size_t i = ZERO_LENGTH; i < SMALL_BUFFER_BYTES; i++) {
        if (copied[i] != BUFFER_COPY_BYTE) {
            dataMatch = false;
            break;
        }
    }
    SetNamedBool(env, result, "created", buffer != nullptr);
    SetNamedBool(env, result, "dataCopied", dataMatch);
    return result;
}

// ---------------------------------------------------------------------------
// Test 25: napi_is_buffer – positive
// ---------------------------------------------------------------------------
static napi_value TestIsBuffer(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value buffer = nullptr;
    NAPI_CALL(env, napi_create_buffer(env, SMALL_BUFFER_BYTES, &data, &buffer));

    bool isBuffer = false;
    NAPI_CALL(env, napi_is_buffer(env, buffer, &isBuffer));
    SetNamedBool(env, result, "isBuffer", isBuffer);
    return result;
}

// ---------------------------------------------------------------------------
// Test 26: napi_is_buffer – negative
// ---------------------------------------------------------------------------
static napi_value TestIsBufferNegative(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value plainObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &plainObject));

    bool isBuffer = true;
    NAPI_CALL(env, napi_is_buffer(env, plainObject, &isBuffer));
    SetNamedBool(env, result, "isNotBuffer", !isBuffer);
    return result;
}

// ---------------------------------------------------------------------------
// Test 27: napi_get_buffer_info
// ---------------------------------------------------------------------------
static napi_value TestGetBufferInfo(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value buffer = nullptr;
    NAPI_CALL(env, napi_create_buffer(env, MEDIUM_BUFFER_BYTES, &createData, &buffer));

    FillBuffer(createData, MEDIUM_BUFFER_BYTES, FILL_BYTE_VALUE);

    void* infoData = nullptr;
    size_t infoLength = ZERO_LENGTH;
    NAPI_CALL(env, napi_get_buffer_info(env, buffer, &infoData, &infoLength));

    SetNamedBool(env, result, "dataMatch", createData == infoData);
    SetNamedBool(env, result, "lengthMatch", infoLength == MEDIUM_BUFFER_BYTES);

    auto* bytes = static_cast<uint8_t*>(infoData);
    SetNamedBool(env, result, "firstByteCorrect", bytes[ZERO_LENGTH] == FILL_BYTE_VALUE);
    return result;
}

// ---------------------------------------------------------------------------
// External ArrayBuffer cleanup callback
// ---------------------------------------------------------------------------
static uint8_t g_externalData[MEDIUM_BUFFER_BYTES] = {};

static void ExternalArrayBufferFinalizer(napi_env env, void* finalizeData, void* finalizeHint)
{
    // No-op: g_externalData is static, no deallocation needed.
    (void)env;
    (void)finalizeData;
    (void)finalizeHint;
}

// ---------------------------------------------------------------------------
// Test 28: napi_create_external_arraybuffer
// ---------------------------------------------------------------------------
static napi_value TestCreateExternalArrayBuffer(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    FillBuffer(g_externalData, MEDIUM_BUFFER_BYTES, EXTERNAL_FILL_BYTE);

    napi_value arrayBuffer = nullptr;
    NAPI_CALL(env, napi_create_external_arraybuffer(
        env, g_externalData, MEDIUM_BUFFER_BYTES, ExternalArrayBufferFinalizer, nullptr, &arrayBuffer));

    bool isArrayBuffer = false;
    NAPI_CALL(env, napi_is_arraybuffer(env, arrayBuffer, &isArrayBuffer));

    void* data = nullptr;
    size_t byteLength = ZERO_LENGTH;
    NAPI_CALL(env, napi_get_arraybuffer_info(env, arrayBuffer, &data, &byteLength));

    SetNamedBool(env, result, "isArrayBuffer", isArrayBuffer);
    SetNamedBool(env, result, "lengthMatch", byteLength == MEDIUM_BUFFER_BYTES);
    SetNamedBool(env, result, "dataMatch", data == g_externalData);
    return result;
}

// ---------------------------------------------------------------------------
// Test 29: ArrayBuffer with different byte lengths
// ---------------------------------------------------------------------------
static bool VerifyArrayBufferSize(napi_env env, size_t expectedSize, napi_value result, const char* label)
{
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, expectedSize, &data);
    void* infoData = nullptr;
    size_t infoLength = ZERO_LENGTH;
    NAPI_CALL_RETURN(env, napi_get_arraybuffer_info(env, arrayBuffer, &infoData, &infoLength), false);

    std::string sizeKey = std::string(label) + "SizeOk";
    SetNamedBool(env, result, sizeKey.c_str(), infoLength == expectedSize);
    return true;
}

static napi_value TestArrayBufferDifferentSizes(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    VerifyArrayBufferSize(env, SMALL_BUFFER_BYTES, result, "small");
    VerifyArrayBufferSize(env, MEDIUM_BUFFER_BYTES, result, "medium");
    VerifyArrayBufferSize(env, LARGE_BUFFER_BYTES, result, "large");
    VerifyArrayBufferSize(env, EXTRA_LARGE_BUFFER_BYTES, result, "extraLarge");
    return result;
}

// ---------------------------------------------------------------------------
// Test 30: ArrayBuffer data manipulation
// ---------------------------------------------------------------------------
static napi_value TestArrayBufferDataManipulation(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, SMALL_BUFFER_BYTES, &data);

    auto* bytes = static_cast<uint8_t*>(data);
    FillBuffer(data, SMALL_BUFFER_BYTES, FILL_BYTE_VALUE);

    bool allFilled = true;
    for (size_t i = ZERO_LENGTH; i < SMALL_BUFFER_BYTES; i++) {
        if (bytes[i] != FILL_BYTE_VALUE) {
            allFilled = false;
            break;
        }
    }

    bytes[ZERO_LENGTH] = SECOND_FILL_BYTE;
    SetNamedBool(env, result, "fillCorrect", allFilled);
    SetNamedBool(env, result, "modifyCorrect", bytes[ZERO_LENGTH] == SECOND_FILL_BYTE);
    return result;
}

// ---------------------------------------------------------------------------
// Test 31: TypedArray element read/write – Int8Array
// ---------------------------------------------------------------------------
static napi_value TestTypedArrayReadWriteInt8(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_INT8;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_int8_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    auto* elements = static_cast<int8_t*>(data);
    elements[ZERO_LENGTH] = static_cast<int8_t>(INT8_TEST_VALUE);

    SetNamedBool(env, result, "writeCorrect", elements[ZERO_LENGTH] == static_cast<int8_t>(INT8_TEST_VALUE));
    SetNamedInt32(env, result, "readValue", static_cast<int32_t>(elements[ZERO_LENGTH]));
    return result;
}

// ---------------------------------------------------------------------------
// Test 32: TypedArray element read/write – Int32Array
// ---------------------------------------------------------------------------
static napi_value TestTypedArrayReadWriteInt32(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_INT32;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_int32_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    auto* elements = static_cast<int32_t*>(data);
    elements[ZERO_LENGTH] = INT32_TEST_VALUE;

    SetNamedBool(env, result, "writeCorrect", elements[ZERO_LENGTH] == INT32_TEST_VALUE);
    SetNamedInt32(env, result, "readValue", elements[ZERO_LENGTH]);
    return result;
}

// ---------------------------------------------------------------------------
// Test 33: TypedArray element read/write – Float64Array
// ---------------------------------------------------------------------------
static napi_value TestTypedArrayReadWriteFloat64(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_FLOAT64;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_float64_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    auto* elements = static_cast<double*>(data);
    elements[ZERO_LENGTH] = FLOAT64_TEST_VALUE;

    SetNamedBool(env, result, "writeCorrect", elements[ZERO_LENGTH] == FLOAT64_TEST_VALUE);
    SetNamedDouble(env, result, "readValue", elements[ZERO_LENGTH]);
    return result;
}

// ---------------------------------------------------------------------------
// Test 34: TypedArray read/write – Uint8Array
// ---------------------------------------------------------------------------
static napi_value TestTypedArrayReadWriteUint8(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_UINT8;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_uint8_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    auto* elements = static_cast<uint8_t*>(data);
    elements[ZERO_LENGTH] = static_cast<uint8_t>(UINT8_TEST_VALUE);

    SetNamedBool(env, result, "writeCorrect", elements[ZERO_LENGTH] == static_cast<uint8_t>(UINT8_TEST_VALUE));
    SetNamedInt32(env, result, "readValue", static_cast<int32_t>(elements[ZERO_LENGTH]));
    return result;
}

// ---------------------------------------------------------------------------
// Test 35: TypedArray read/write – Int16Array
// ---------------------------------------------------------------------------
static napi_value TestTypedArrayReadWriteInt16(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_INT16;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_int16_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    auto* elements = static_cast<int16_t*>(data);
    elements[ZERO_LENGTH] = static_cast<int16_t>(INT16_TEST_VALUE);

    SetNamedBool(env, result, "writeCorrect", elements[ZERO_LENGTH] == static_cast<int16_t>(INT16_TEST_VALUE));
    SetNamedInt32(env, result, "readValue", static_cast<int32_t>(elements[ZERO_LENGTH]));
    return result;
}

// ---------------------------------------------------------------------------
// Test 36: TypedArray read/write – Uint16Array
// ---------------------------------------------------------------------------
static napi_value TestTypedArrayReadWriteUint16(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_UINT16;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_uint16_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    auto* elements = static_cast<uint16_t*>(data);
    elements[ZERO_LENGTH] = static_cast<uint16_t>(UINT16_TEST_VALUE);

    SetNamedBool(env, result, "writeCorrect", elements[ZERO_LENGTH] == static_cast<uint16_t>(UINT16_TEST_VALUE));
    SetNamedInt32(env, result, "readValue", static_cast<int32_t>(elements[ZERO_LENGTH]));
    return result;
}

// ---------------------------------------------------------------------------
// Test 37: TypedArray read/write – Uint32Array
// ---------------------------------------------------------------------------
static napi_value TestTypedArrayReadWriteUint32(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_UINT32;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_uint32_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    auto* elements = static_cast<uint32_t*>(data);
    elements[ZERO_LENGTH] = UINT32_TEST_VALUE;

    SetNamedBool(env, result, "writeCorrect", elements[ZERO_LENGTH] == UINT32_TEST_VALUE);
    SetNamedInt32(env, result, "readValue", static_cast<int32_t>(elements[ZERO_LENGTH]));
    return result;
}

// ---------------------------------------------------------------------------
// Test 38: TypedArray read/write – Float32Array
// ---------------------------------------------------------------------------
static napi_value TestTypedArrayReadWriteFloat32(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_FLOAT32;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_float32_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    auto* elements = static_cast<float*>(data);
    elements[ZERO_LENGTH] = FLOAT32_TEST_VALUE;

    SetNamedBool(env, result, "writeCorrect", elements[ZERO_LENGTH] == FLOAT32_TEST_VALUE);
    SetNamedDouble(env, result, "readValue", static_cast<double>(elements[ZERO_LENGTH]));
    return result;
}

// ---------------------------------------------------------------------------
// Test 39: Multiple TypedArray views on same ArrayBuffer
// ---------------------------------------------------------------------------
static napi_value CreateMultiViewBuffer(napi_env env, void** outData)
{
    size_t totalBytes = TYPED_ARRAY_LENGTH * BYTES_PER_INT32;
    return CreateArrayBufferWithSize(env, totalBytes, outData);
}

static napi_value TestMultipleTypedArrayViews(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateMultiViewBuffer(env, &data);

    size_t offsetA = MULTI_VIEW_OFFSET_A * BYTES_PER_INT32;
    size_t offsetB = MULTI_VIEW_OFFSET_B * BYTES_PER_INT32;

    napi_value viewA = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_int32_array, MULTI_VIEW_ELEM_COUNT, arrayBuffer, offsetA, &viewA));

    napi_value viewB = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_int32_array, MULTI_VIEW_ELEM_COUNT, arrayBuffer, offsetB, &viewB));

    auto* elements = static_cast<int32_t*>(data);
    elements[MULTI_VIEW_OFFSET_A] = INT32_TEST_VALUE;

    SetNamedBool(env, result, "viewACreated", viewA != nullptr);
    SetNamedBool(env, result, "viewBCreated", viewB != nullptr);
    SetNamedBool(env, result, "sharedWrite", elements[MULTI_VIEW_OFFSET_A] == INT32_TEST_VALUE);
    return result;
}

// ---------------------------------------------------------------------------
// Test 40: DataView with various byte offsets
// ---------------------------------------------------------------------------
static napi_value TestDataViewByteOffsets(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, LARGE_BUFFER_BYTES, &data);

    napi_value viewAtZero = nullptr;
    NAPI_CALL(env, napi_create_dataview(
        env, DATAVIEW_LENGTH, arrayBuffer, DATAVIEW_DEFAULT_OFFSET, &viewAtZero));

    napi_value viewAtFour = nullptr;
    NAPI_CALL(env, napi_create_dataview(
        env, DATAVIEW_LENGTH, arrayBuffer, DATAVIEW_CUSTOM_OFFSET, &viewAtFour));

    size_t offsetZero = ZERO_LENGTH;
    size_t offsetFour = ZERO_LENGTH;
    NAPI_CALL(env, napi_get_dataview_info(env, viewAtZero, nullptr, nullptr, nullptr, &offsetZero));
    NAPI_CALL(env, napi_get_dataview_info(env, viewAtFour, nullptr, nullptr, nullptr, &offsetFour));

    SetNamedBool(env, result, "offsetZeroOk", offsetZero == DATAVIEW_DEFAULT_OFFSET);
    SetNamedBool(env, result, "offsetFourOk", offsetFour == DATAVIEW_CUSTOM_OFFSET);
    return result;
}

// ---------------------------------------------------------------------------
// Test 41: napi_get_typedarray_info – Float64Array with element verification
// ---------------------------------------------------------------------------
static napi_value TestGetTypedArrayInfoFloat64(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_FLOAT64;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_float64_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    napi_typedarray_type arrayType = napi_int8_array;
    size_t length = ZERO_LENGTH;
    NAPI_CALL(env, napi_get_typedarray_info(env, typedArray, &arrayType, &length, nullptr, nullptr, nullptr));

    SetNamedBool(env, result, "typeMatch", arrayType == napi_float64_array);
    SetNamedBool(env, result, "lengthMatch", length == TYPED_ARRAY_LENGTH);
    SetNamedInt32(env, result, "bytesPerElement", static_cast<int32_t>(BYTES_PER_FLOAT64));
    return result;
}

// ---------------------------------------------------------------------------
// Test 42: ArrayBuffer zero-length creation
// ---------------------------------------------------------------------------
static napi_value TestArrayBufferZeroLength(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = nullptr;
    NAPI_CALL(env, napi_create_arraybuffer(env, ZERO_LENGTH, &data, &arrayBuffer));

    bool isArrayBuffer = false;
    NAPI_CALL(env, napi_is_arraybuffer(env, arrayBuffer, &isArrayBuffer));

    size_t byteLength = SINGLE_ELEMENT;
    NAPI_CALL(env, napi_get_arraybuffer_info(env, arrayBuffer, nullptr, &byteLength));

    SetNamedBool(env, result, "created", arrayBuffer != nullptr);
    SetNamedBool(env, result, "isArrayBuffer", isArrayBuffer);
    SetNamedBool(env, result, "zeroLength", byteLength == ZERO_LENGTH);
    return result;
}

// ---------------------------------------------------------------------------
// Test 43: DataView is not a TypedArray
// ---------------------------------------------------------------------------
static napi_value TestDataViewIsNotTypedArray(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, MEDIUM_BUFFER_BYTES, &data);

    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, DATAVIEW_LENGTH, arrayBuffer, DATAVIEW_DEFAULT_OFFSET, &dataView));

    bool isTypedArray = true;
    NAPI_CALL(env, napi_is_typedarray(env, dataView, &isTypedArray));

    bool isDataView = false;
    NAPI_CALL(env, napi_is_dataview(env, dataView, &isDataView));

    SetNamedBool(env, result, "notTypedArray", !isTypedArray);
    SetNamedBool(env, result, "isDataView", isDataView);
    return result;
}

// ---------------------------------------------------------------------------
// Test 44: TypedArray is not a DataView
// ---------------------------------------------------------------------------
static napi_value TestTypedArrayIsNotDataView(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    size_t bufferBytes = TYPED_ARRAY_LENGTH * BYTES_PER_INT32;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, bufferBytes, &data);

    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(
        env, napi_int32_array, TYPED_ARRAY_LENGTH, arrayBuffer, TYPED_ARRAY_OFFSET_ZERO, &typedArray));

    bool isDataView = true;
    NAPI_CALL(env, napi_is_dataview(env, typedArray, &isDataView));

    bool isTypedArray = false;
    NAPI_CALL(env, napi_is_typedarray(env, typedArray, &isTypedArray));

    SetNamedBool(env, result, "notDataView", !isDataView);
    SetNamedBool(env, result, "isTypedArray", isTypedArray);
    return result;
}

// ---------------------------------------------------------------------------
// Module descriptor table (UPPER_CASE for global constant array)
// ---------------------------------------------------------------------------
struct TestEntry {
    const char* name;
    napi_callback callback;
};

static const TestEntry TYPEDARRAY_TESTS[] = {
    { "testCreateArrayBuffer",            TestCreateArrayBuffer },
    { "testGetArrayBufferInfo",           TestGetArrayBufferInfo },
    { "testIsArrayBuffer",                TestIsArrayBuffer },
    { "testIsArrayBufferNegative",        TestIsArrayBufferNegative },
    { "testCreateTypedArrayInt8",         TestCreateTypedArrayInt8 },
    { "testCreateTypedArrayUint8",        TestCreateTypedArrayUint8 },
    { "testCreateTypedArrayUint8Clamped", TestCreateTypedArrayUint8Clamped },
    { "testCreateTypedArrayInt16",        TestCreateTypedArrayInt16 },
    { "testCreateTypedArrayUint16",       TestCreateTypedArrayUint16 },
    { "testCreateTypedArrayInt32",        TestCreateTypedArrayInt32 },
    { "testCreateTypedArrayUint32",       TestCreateTypedArrayUint32 },
    { "testCreateTypedArrayFloat32",      TestCreateTypedArrayFloat32 },
    { "testCreateTypedArrayFloat64",      TestCreateTypedArrayFloat64 },
    { "testCreateTypedArrayBigInt64",     TestCreateTypedArrayBigInt64 },
    { "testCreateTypedArrayBigUint64",    TestCreateTypedArrayBigUint64 },
    { "testGetTypedArrayInfo",            TestGetTypedArrayInfo },
    { "testCreateDataView",              TestCreateDataView },
    { "testGetDataViewInfo",             TestGetDataViewInfo },
    { "testIsDataView",                  TestIsDataView },
    { "testIsDataViewNegative",          TestIsDataViewNegative },
    { "testDetachArrayBuffer",           TestDetachArrayBuffer },
    { "testIsDetachedArrayBufferFalse",  TestIsDetachedArrayBufferFalse },
    { "testCreateBuffer",               TestCreateBuffer },
    { "testCreateBufferCopy",            TestCreateBufferCopy },
    { "testIsBuffer",                    TestIsBuffer },
    { "testIsBufferNegative",            TestIsBufferNegative },
    { "testGetBufferInfo",               TestGetBufferInfo },
    { "testCreateExternalArrayBuffer",   TestCreateExternalArrayBuffer },
    { "testArrayBufferDifferentSizes",   TestArrayBufferDifferentSizes },
    { "testArrayBufferDataManipulation", TestArrayBufferDataManipulation },
    { "testTypedArrayReadWriteInt8",     TestTypedArrayReadWriteInt8 },
    { "testTypedArrayReadWriteInt32",    TestTypedArrayReadWriteInt32 },
    { "testTypedArrayReadWriteFloat64",  TestTypedArrayReadWriteFloat64 },
    { "testTypedArrayReadWriteUint8",    TestTypedArrayReadWriteUint8 },
    { "testTypedArrayReadWriteInt16",    TestTypedArrayReadWriteInt16 },
    { "testTypedArrayReadWriteUint16",   TestTypedArrayReadWriteUint16 },
    { "testTypedArrayReadWriteUint32",   TestTypedArrayReadWriteUint32 },
    { "testTypedArrayReadWriteFloat32",  TestTypedArrayReadWriteFloat32 },
    { "testMultipleTypedArrayViews",     TestMultipleTypedArrayViews },
    { "testDataViewByteOffsets",         TestDataViewByteOffsets },
    { "testGetTypedArrayInfoFloat64",    TestGetTypedArrayInfoFloat64 },
    { "testArrayBufferZeroLength",       TestArrayBufferZeroLength },
    { "testDataViewIsNotTypedArray",     TestDataViewIsNotTypedArray },
    { "testTypedArrayIsNotDataView",     TestTypedArrayIsNotDataView },
};

static constexpr size_t TYPEDARRAY_TEST_COUNT = sizeof(TYPEDARRAY_TESTS) / sizeof(TYPEDARRAY_TESTS[0]);

}  // namespace

// ---------------------------------------------------------------------------
// Module initialization
// ---------------------------------------------------------------------------
static napi_value InitTypedArraySuite(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[TYPEDARRAY_TEST_COUNT];
    for (size_t i = 0; i < TYPEDARRAY_TEST_COUNT; i++) {
        descriptors[i] = napi_property_descriptor{
            TYPEDARRAY_TESTS[i].name, nullptr, TYPEDARRAY_TESTS[i].callback,
            nullptr, nullptr, nullptr, napi_default, nullptr
        };
    }
    NAPI_CALL(env, napi_define_properties(env, exports, TYPEDARRAY_TEST_COUNT, descriptors));
    return exports;
}

static napi_module g_typedarraySuiteModule = {
    .nm_version = MODULE_VERSION,
    .nm_flags = NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitTypedArraySuite,
    .nm_modname = "typedarray_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterTypedArraySuiteModule(void)
{
    napi_module_register(&g_typedarraySuiteModule);
}
