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

#include <cstring>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

namespace {
constexpr uint32_t MODULE_VERSION = 1;
constexpr uint32_t MODULE_FLAGS = 0;
constexpr int32_t TEST_INT_42 = 42;
constexpr int32_t TEST_INT_100 = 100;
constexpr int32_t TEST_INT_200 = 200;
constexpr double TEST_DOUBLE_VAL = 3.14159265;
constexpr size_t BUFFER_SIZE_8 = 8;
constexpr size_t BUFFER_SIZE_16 = 16;
constexpr size_t BUFFER_SIZE_32 = 32;
constexpr size_t ARRAY_SIZE_5 = 5;
constexpr size_t CIRCULAR_DEPTH = 3;
constexpr size_t STRING_LEN_100 = 100;
constexpr size_t BUF_LEN_256 = 256;
constexpr float FLOAT_STEP = 0.5f;
constexpr size_t FLOAT_COUNT_8 = 8;
constexpr size_t DOUBLE_COUNT_8 = 8;
constexpr size_t TOTAL_BYTES_64 = 64;
constexpr uint32_t HOLE_INDEX_10 = 10;
constexpr uint8_t GARBAGE_VAL = 0xAA;
constexpr uint8_t BYTE_OFFSET_2 = 2;
constexpr uint8_t BYTE_OFFSET_3 = 3;
constexpr double TIME_VAL_CONST = 1718616773000.0;
constexpr uint64_t BIG_VAL_CONST = 12345678901234ULL;
constexpr size_t ARGS_COUNT_2 = 2;
constexpr size_t ARGS_COUNT_3 = 3;
constexpr size_t IDX_ZERO = 0;
constexpr size_t IDX_ONE = 1;
constexpr size_t IDX_TWO = 2;
} // namespace

static napi_value GetUndefined(napi_env env)
{
    napi_value undefinedVal = nullptr;
    napi_get_undefined(env, &undefinedVal);
    return undefinedVal;
}

static napi_value GetNull(napi_env env)
{
    napi_value nullVal = nullptr;
    napi_get_null(env, &nullVal);
    return nullVal;
}

static napi_value CreateInt32(napi_env env, int32_t value)
{
    napi_value numVal = nullptr;
    napi_create_int32(env, value, &numVal);
    return numVal;
}

// 用例 1: 验证普通基本对象的序列化和反序列化
static napi_value TestSerializeBasicObject(napi_env env, napi_callback_info /* info */)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_value propInt = CreateInt32(env, TEST_INT_100);
    napi_set_named_property(env, object, "intProp", propInt);
    napi_value propStr = nullptr;
    napi_create_string_utf8(env, "hello", NAPI_AUTO_LENGTH, &propStr);
    napi_set_named_property(env, object, "strProp", propStr);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, object, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value checkInt = nullptr;
    napi_get_named_property(env, result, "intProp", &checkInt);
    int32_t valInt = 0;
    napi_get_value_int32(env, checkInt, &valInt);
    if (valInt != TEST_INT_100) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 2: 验证布尔值 true 的序列化和反序列化
static napi_value TestSerializeBooleanTrue(napi_env env, napi_callback_info /* info */)
{
    napi_value booleanVal = nullptr;
    napi_get_boolean(env, true, &booleanVal);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, booleanVal, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    bool checkVal = false;
    napi_get_value_bool(env, result, &checkVal);
    if (!checkVal) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 3: 验证布尔值 false 的序列化和反序列化
static napi_value TestSerializeBooleanFalse(napi_env env, napi_callback_info /* info */)
{
    napi_value booleanVal = nullptr;
    napi_get_boolean(env, false, &booleanVal);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, booleanVal, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    bool checkVal = true;
    napi_get_value_bool(env, result, &checkVal);
    if (checkVal) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 4: 验证整型数字的序列化和反序列化
static napi_value TestSerializeNumberInt(napi_env env, napi_callback_info /* info */)
{
    napi_value numVal = CreateInt32(env, TEST_INT_200);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, numVal, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    int32_t checkVal = 0;
    napi_get_value_int32(env, result, &checkVal);
    if (checkVal != TEST_INT_200) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 5: 验证浮点数字的序列化和反序列化
static napi_value TestSerializeNumberDouble(napi_env env, napi_callback_info /* info */)
{
    napi_value numVal = nullptr;
    napi_create_double(env, TEST_DOUBLE_VAL, &numVal);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, numVal, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    double checkVal = 0.0;
    napi_get_value_double(env, result, &checkVal);
    if (checkVal != TEST_DOUBLE_VAL) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 6: 验证 Null 值的序列化和反序列化
static napi_value TestSerializeNull(napi_env env, napi_callback_info /* info */)
{
    napi_value nullVal = GetNull(env);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, nullVal, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_valuetype type = napi_undefined;
    napi_typeof(env, result, &type);
    if (type != napi_null) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 7: 验证 Undefined 的序列化和反序列化
static napi_value TestSerializeUndefined(napi_env env, napi_callback_info /* info */)
{
    napi_value undefinedVal = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, undefinedVal, undefinedVal, undefinedVal, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_valuetype type = napi_null;
    napi_typeof(env, result, &type);
    if (type != napi_undefined) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 8: 验证普通数组的序列化和反序列化
static napi_value TestSerializeArray(napi_env env, napi_callback_info /* info */)
{
    napi_value arrayVal = nullptr;
    napi_create_array(env, &arrayVal);
    for (uint32_t i = 0; i < ARRAY_SIZE_5; i++) {
        napi_value element = CreateInt32(env, static_cast<int32_t>(i));
        napi_set_element(env, arrayVal, i, element);
    }
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, arrayVal, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    bool isArray = false;
    napi_is_array(env, result, &isArray);
    if (!isArray) {
        return CreateInt32(env, -1);
    }
    uint32_t length = 0;
    napi_get_array_length(env, result, &length);
    if (length != ARRAY_SIZE_5) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 9: 验证循环引用的对象序列化和反序列化
static napi_value TestSerializeCircularReference(napi_env env, napi_callback_info /* info */)
{
    napi_value objectA = nullptr;
    napi_create_object(env, &objectA);
    napi_value objectB = nullptr;
    napi_create_object(env, &objectB);
    napi_set_named_property(env, objectA, "next", objectB);
    napi_set_named_property(env, objectB, "prev", objectA);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, objectA, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value resultB = nullptr;
    napi_get_named_property(env, result, "next", &resultB);
    napi_value resultA = nullptr;
    napi_get_named_property(env, resultB, "prev", &resultA);
    bool equals = false;
    napi_strict_equals(env, result, resultA, &equals);
    if (!equals) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 10: 验证 Map 类型的序列化和反序列化
static napi_value TestSerializeMap(napi_env env, napi_callback_info /* info */)
{
    napi_value mapVal = nullptr;
    napi_create_map(env, &mapVal);
    napi_value key = nullptr;
    napi_create_string_utf8(env, "mapKey", NAPI_AUTO_LENGTH, &key);
    napi_value value = CreateInt32(env, TEST_INT_42);
    napi_map_set_property(env, mapVal, key, value);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, mapVal, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value checkVal = nullptr;
    napi_map_get_property(env, result, key, &checkVal);
    int32_t val = 0;
    napi_get_value_int32(env, checkVal, &val);
    if (val != TEST_INT_42) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 11: 验证 Set 类型的序列化和反序列化
static napi_value TestSerializeSet(napi_env env, napi_callback_info /* info */)
{
    napi_value global = nullptr;
    napi_get_global(env, &global);
    napi_value setConstructor = nullptr;
    napi_get_named_property(env, global, "Set", &setConstructor);
    napi_value setVal = nullptr;
    napi_new_instance(env, setConstructor, 0, nullptr, &setVal);
    napi_value addFunc = nullptr;
    napi_get_named_property(env, setVal, "add", &addFunc);
    napi_value arg = CreateInt32(env, TEST_INT_42);
    napi_call_function(env, setVal, addFunc, 1, &arg, nullptr);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, setVal, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value hasFunc = nullptr;
    napi_get_named_property(env, result, "has", &hasFunc);
    napi_value hasRes = nullptr;
    napi_call_function(env, result, hasFunc, 1, &arg, &hasRes);
    bool checkHas = false;
    napi_get_value_bool(env, hasRes, &checkHas);
    if (!checkHas) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 12: 验证 ArrayBuffer 的序列化和反序列化
static napi_value TestSerializeArrayBuffer(napi_env env, napi_callback_info /* info */)
{
    void* rawData = nullptr;
    napi_value arrayBufferVal = nullptr;
    napi_create_arraybuffer(env, BUFFER_SIZE_8, &rawData, &arrayBufferVal);
    if (rawData == nullptr) {
        return CreateInt32(env, -1);
    }
    uint8_t* byteData = static_cast<uint8_t*>(rawData);
    for (size_t i = 0; i < BUFFER_SIZE_8; i++) {
        byteData[i] = static_cast<uint8_t>(i);
    }
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, arrayBufferVal, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    void* resultData = nullptr;
    size_t resultLen = 0;
    napi_get_arraybuffer_info(env, result, &resultData, &resultLen);
    if (resultLen != BUFFER_SIZE_8 || resultData == nullptr) {
        return CreateInt32(env, -1);
    }
    uint8_t* resBytes = static_cast<uint8_t*>(resultData);
    for (size_t i = 0; i < BUFFER_SIZE_8; i++) {
        if (resBytes[i] != static_cast<uint8_t>(i)) {
            return CreateInt32(env, -1);
        }
    }
    return CreateInt32(env, 1);
}

// 用例 13: 验证 TypedArray (Uint8Array) 的序列化和反序列化
static napi_value TestSerializeTypedArray(napi_env env, napi_callback_info /* info */)
{
    napi_value ab = nullptr;
    void* rawData = nullptr;
    napi_create_arraybuffer(env, BUFFER_SIZE_8, &rawData, &ab);
    uint8_t* byteData = static_cast<uint8_t*>(rawData);
    for (size_t i = 0; i < BUFFER_SIZE_8; i++) {
        byteData[i] = static_cast<uint8_t>(i + 1);
    }
    napi_value typedArray = nullptr;
    napi_create_typedarray(env, napi_uint8_array, BUFFER_SIZE_8, ab, 0, &typedArray);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, typedArray, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_typedarray_type type = napi_int8_array;
    size_t length = 0;
    void* resultData = nullptr;
    napi_get_typedarray_info(env, result, &type, &length, &resultData, nullptr, nullptr);
    if (type != napi_uint8_array || length != BUFFER_SIZE_8 || resultData == nullptr) {
        return CreateInt32(env, -1);
    }
    uint8_t* resBytes = static_cast<uint8_t*>(resultData);
    for (size_t i = 0; i < BUFFER_SIZE_8; i++) {
        if (resBytes[i] != static_cast<uint8_t>(i + 1)) {
            return CreateInt32(env, -1);
        }
    }
    return CreateInt32(env, 1);
}

// 用例 14: 验证 transferList 传输列表的特殊机制 (ArrayBuffer 序列化后原对象 detach 并且反序列化正常)
static napi_value TestSerializeTransferArrayBuffer(napi_env env, napi_callback_info /* info */)
{
    void* rawData = nullptr;
    napi_value arrayBufferVal = nullptr;
    napi_create_arraybuffer(env, BUFFER_SIZE_8, &rawData, &arrayBufferVal);
    uint8_t* byteData = static_cast<uint8_t*>(rawData);
    for (size_t i = 0; i < BUFFER_SIZE_8; i++) {
        byteData[i] = static_cast<uint8_t>(i + BYTE_OFFSET_2);
    }
    napi_value transferList = nullptr;
    napi_create_array(env, &transferList);
    napi_set_element(env, transferList, 0, arrayBufferVal);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, arrayBufferVal, transferList, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    bool isDetached = false;
    napi_is_detached_arraybuffer(env, arrayBufferVal, &isDetached);
    if (!isDetached) {
        napi_delete_serialization_data(env, data);
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    void* resultData = nullptr;
    size_t resultLen = 0;
    napi_get_arraybuffer_info(env, result, &resultData, &resultLen);
    if (resultLen != BUFFER_SIZE_8 || resultData == nullptr) {
        return CreateInt32(env, -1);
    }
    uint8_t* resBytes = static_cast<uint8_t*>(resultData);
    for (size_t i = 0; i < BUFFER_SIZE_8; i++) {
        if (resBytes[i] != static_cast<uint8_t>(i + BYTE_OFFSET_2)) {
            return CreateInt32(env, -1);
        }
    }
    return CreateInt32(env, 1);
}

// 用例 15: 验证 cloneList 克隆列表机制下，ArrayBuffer 原对象不被 detach
static napi_value TestSerializeCloneArrayBuffer(napi_env env, napi_callback_info /* info */)
{
    void* rawData = nullptr;
    napi_value arrayBufferVal = nullptr;
    napi_create_arraybuffer(env, BUFFER_SIZE_8, &rawData, &arrayBufferVal);
    uint8_t* byteData = static_cast<uint8_t*>(rawData);
    for (size_t i = 0; i < BUFFER_SIZE_8; i++) {
        byteData[i] = static_cast<uint8_t>(i + BYTE_OFFSET_3);
    }
    napi_value cloneList = nullptr;
    napi_create_array(env, &cloneList);
    napi_set_element(env, cloneList, 0, arrayBufferVal);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, arrayBufferVal, undefined, cloneList, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    bool isDetached = true;
    napi_is_detached_arraybuffer(env, arrayBufferVal, &isDetached);
    if (isDetached) {
        napi_delete_serialization_data(env, data);
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    void* resultData = nullptr;
    size_t resultLen = 0;
    napi_get_arraybuffer_info(env, result, &resultData, &resultLen);
    if (resultLen != BUFFER_SIZE_8 || resultData == nullptr) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 16: 异常与边界：当 env 传入 nullptr 时的安全表现
static napi_value TestSerializeInvalidEnv(napi_env env, napi_callback_info /* info */)
{
    napi_value numVal = CreateInt32(env, TEST_INT_100);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(nullptr, numVal, undefined, undefined, &data);
    if (status != napi_invalid_arg) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 17: 异常与边界：被序列化对象为 nullptr 时的表现
static napi_value TestSerializeNullObject(napi_env env, napi_callback_info /* info */)
{
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, nullptr, undefined, undefined, &data);
    if (status != napi_invalid_arg) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 18: 异常与边界：序列化数据接收地址 result 为 nullptr 时的表现
static napi_value TestSerializeNullResult(napi_env env, napi_callback_info /* info */)
{
    napi_value numVal = CreateInt32(env, TEST_INT_100);
    napi_value undefined = GetUndefined(env);
    napi_status status = napi_serialize(env, numVal, undefined, undefined, nullptr);
    if (status != napi_invalid_arg) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 19: 异常与边界：反序列化时传入 nullptr 的 buffer 的表现
static napi_value TestDeserializeNullBuffer(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    napi_status status = napi_deserialize(env, nullptr, &result);
    if (status != napi_invalid_arg) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 20: 异常与边界：反序列化时接收对象 result 指针为 nullptr 的表现
static napi_value TestDeserializeNullObject(napi_env env, napi_callback_info /* info */)
{
    napi_value numVal = CreateInt32(env, TEST_INT_100);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, numVal, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    status = napi_deserialize(env, data, nullptr);
    napi_delete_serialization_data(env, data);
    if (status != napi_invalid_arg) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 21: 异常与边界：释放序列化数据时传入 nullptr 的表现
static napi_value TestDeleteSerializationDataNullBuffer(napi_env env, napi_callback_info /* info */)
{
    napi_status status = napi_delete_serialization_data(env, nullptr);
    if (status != napi_invalid_arg) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 22: 异常与边界：反序列化非法/损坏的二级制缓冲区
static napi_value TestDeserializeCorruptedData(napi_env env, napi_callback_info /* info */)
{
    uint8_t garbage[BUFFER_SIZE_16] = { 0 };
    for (size_t i = 0; i < BUFFER_SIZE_16; i++) {
        garbage[i] = static_cast<uint8_t>(i + GARBAGE_VAL);
    }
    napi_value result = nullptr;
    napi_status status = napi_deserialize(env, static_cast<void*>(garbage), &result);
    if (status == napi_ok) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 23: 验证 RegExp 正则表达式对象的序列化和反序列化
static napi_value TestSerializeRegExp(napi_env env, napi_callback_info /* info */)
{
    napi_value global = nullptr;
    napi_get_global(env, &global);
    napi_value regExpConstructor = nullptr;
    napi_get_named_property(env, global, "RegExp", &regExpConstructor);
    napi_value pattern = nullptr;
    napi_create_string_utf8(env, "abc", NAPI_AUTO_LENGTH, &pattern);
    napi_value flags = nullptr;
    napi_create_string_utf8(env, "g", NAPI_AUTO_LENGTH, &flags);
    napi_value args[ARGS_COUNT_2];
    args[IDX_ZERO] = pattern;
    args[IDX_ONE] = flags;
    napi_value regExpVal = nullptr;
    napi_new_instance(env, regExpConstructor, ARGS_COUNT_2, args, &regExpVal);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, regExpVal, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value checkSource = nullptr;
    napi_get_named_property(env, result, "source", &checkSource);
    char buf[128] = { 0 };
    size_t readLen = 0;
    napi_get_value_string_utf8(env, checkSource, buf, sizeof(buf), &readLen);
    if (strcmp(buf, "abc") != 0) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 24: 验证 Date 日期对象的序列化和反序列化
static napi_value TestSerializeDate(napi_env env, napi_callback_info /* info */)
{
    napi_value dateVal = nullptr;
    napi_create_date(env, TIME_VAL_CONST, &dateVal);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, dateVal, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    double checkTime = 0.0;
    napi_get_date_value(env, result, &checkTime);
    if (checkTime != TIME_VAL_CONST) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 25: 验证同一份序列化数据包反序列化多次的机制
static napi_value TestSerializeMultiDeserialize(napi_env env, napi_callback_info /* info */)
{
    napi_value numVal = CreateInt32(env, TEST_INT_100);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, numVal, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result1 = nullptr;
    status = napi_deserialize(env, data, &result1);
    if (status != napi_ok || result1 == nullptr) {
        napi_delete_serialization_data(env, data);
        return CreateInt32(env, -1);
    }
    napi_value result2 = nullptr;
    status = napi_deserialize(env, data, &result2);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result2 == nullptr) {
        return CreateInt32(env, -1);
    }
    int32_t val1 = 0;
    napi_get_value_int32(env, result1, &val1);
    int32_t val2 = 0;
    napi_get_value_int32(env, result2, &val2);
    if (val1 != TEST_INT_100 || val2 != TEST_INT_100) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 26: 验证 BigInt 的序列化和反序列化
static napi_value TestSerializeBigInt(napi_env env, napi_callback_info /* info */)
{
    napi_value bigIntVal = nullptr;
    napi_create_bigint_uint64(env, BIG_VAL_CONST, &bigIntVal);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, bigIntVal, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    uint64_t checkVal = 0;
    bool lossless = false;
    napi_get_value_bigint_uint64(env, result, &checkVal, &lossless);
    if (checkVal != BIG_VAL_CONST) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 27: 验证空对象 {} 的序列化和反序列化
static napi_value TestSerializeEmptyObject(napi_env env, napi_callback_info /* info */)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, object, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_valuetype type = napi_undefined;
    napi_typeof(env, result, &type);
    if (type != napi_object) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 28: 验证普通嵌套对象的序列化与反序列化
static napi_value TestSerializeNestedObject(napi_env env, napi_callback_info /* info */)
{
    napi_value level3 = nullptr;
    napi_create_object(env, &level3);
    napi_value propVal = CreateInt32(env, TEST_INT_100);
    napi_set_named_property(env, level3, "value", propVal);
    napi_value level2 = nullptr;
    napi_create_object(env, &level2);
    napi_set_named_property(env, level2, "child", level3);
    napi_value level1 = nullptr;
    napi_create_object(env, &level1);
    napi_set_named_property(env, level1, "child", level2);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, level1, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value checkLevel2 = nullptr;
    napi_get_named_property(env, result, "child", &checkLevel2);
    napi_value checkLevel3 = nullptr;
    napi_get_named_property(env, checkLevel2, "child", &checkLevel3);
    napi_value checkValObj = nullptr;
    napi_get_named_property(env, checkLevel3, "value", &checkValObj);
    int32_t val = 0;
    napi_get_value_int32(env, checkValObj, &val);
    if (val != TEST_INT_100) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 29: 异常与边界：测试不支持的类型 (Function) 序列化的表现
static napi_value TestSerializeUnsupportedTypeFunction(napi_env env, napi_callback_info /* info */)
{
    napi_value func = nullptr;
    napi_create_function(env, "test", NAPI_AUTO_LENGTH, TestSerializeBasicObject, nullptr, &func);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, func, undefined, undefined, &data);
    if (status == napi_ok) {
        napi_delete_serialization_data(env, data);
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 30: 异常与边界：测试不支持的类型 (Symbol) 序列化的表现
static napi_value TestSerializeUnsupportedTypeSymbol(napi_env env, napi_callback_info /* info */)
{
    napi_value symbol = nullptr;
    napi_value description = nullptr;
    napi_create_string_utf8(env, "test", NAPI_AUTO_LENGTH, &description);
    napi_create_symbol(env, description, &symbol);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, symbol, undefined, undefined, &data);
    if (status == napi_ok) {
        napi_delete_serialization_data(env, data);
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 31: 验证空字符串的序列化和反序列化
static napi_value TestSerializeEmptyString(napi_env env, napi_callback_info /* info */)
{
    napi_value strVal = nullptr;
    napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &strVal);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, strVal, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    char buf[128] = { 0 };
    size_t readLen = 0;
    napi_get_value_string_utf8(env, result, buf, sizeof(buf), &readLen);
    if (readLen != 0 || strcmp(buf, "") != 0) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 32: 验证长字符串的序列化和反序列化
static napi_value TestSerializeLongString(napi_env env, napi_callback_info /* info */)
{
    std::string longStr(STRING_LEN_100, 'a');
    napi_value strVal = nullptr;
    napi_create_string_utf8(env, longStr.c_str(), NAPI_AUTO_LENGTH, &strVal);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, strVal, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    char buf[BUF_LEN_256] = { 0 };
    size_t readLen = 0;
    napi_get_value_string_utf8(env, result, buf, sizeof(buf), &readLen);
    if (readLen != STRING_LEN_100 || strcmp(buf, longStr.c_str()) != 0) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 33: 验证 Int8Array 类型的序列化和反序列化
static napi_value TestSerializeInt8Array(napi_env env, napi_callback_info /* info */)
{
    napi_value ab = nullptr;
    void* rawData = nullptr;
    napi_create_arraybuffer(env, BUFFER_SIZE_8, &rawData, &ab);
    int8_t* byteData = static_cast<int8_t*>(rawData);
    for (size_t i = 0; i < BUFFER_SIZE_8; i++) {
        byteData[i] = static_cast<int8_t>(-1 * static_cast<int8_t>(i));
    }
    napi_value typedArray = nullptr;
    napi_create_typedarray(env, napi_int8_array, BUFFER_SIZE_8, ab, 0, &typedArray);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, typedArray, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_typedarray_type type = napi_uint8_array;
    size_t length = 0;
    void* resultData = nullptr;
    napi_get_typedarray_info(env, result, &type, &length, &resultData, nullptr, nullptr);
    if (type != napi_int8_array || length != BUFFER_SIZE_8 || resultData == nullptr) {
        return CreateInt32(env, -1);
    }
    int8_t* resBytes = static_cast<int8_t*>(resultData);
    for (size_t i = 0; i < BUFFER_SIZE_8; i++) {
        if (resBytes[i] != static_cast<int8_t>(-1 * static_cast<int8_t>(i))) {
            return CreateInt32(env, -1);
        }
    }
    return CreateInt32(env, 1);
}

// 用例 34: 验证 Float32Array 类型的序列化和反序列化
static napi_value TestSerializeFloat32Array(napi_env env, napi_callback_info /* info */)
{
    napi_value ab = nullptr;
    void* rawData = nullptr;
    napi_create_arraybuffer(env, BUFFER_SIZE_32, &rawData, &ab);
    float* floatData = static_cast<float*>(rawData);
    for (size_t i = 0; i < FLOAT_COUNT_8; i++) {
        floatData[i] = static_cast<float>(i) * FLOAT_STEP;
    }
    napi_value typedArray = nullptr;
    napi_create_typedarray(env, napi_float32_array, FLOAT_COUNT_8, ab, 0, &typedArray);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, typedArray, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_typedarray_type type = napi_int8_array;
    size_t length = 0;
    void* resultData = nullptr;
    napi_get_typedarray_info(env, result, &type, &length, &resultData, nullptr, nullptr);
    if (type != napi_float32_array || length != FLOAT_COUNT_8 || resultData == nullptr) {
        return CreateInt32(env, -1);
    }
    float* resBytes = static_cast<float*>(resultData);
    for (size_t i = 0; i < FLOAT_COUNT_8; i++) {
        if (resBytes[i] != static_cast<float>(i) * FLOAT_STEP) {
            return CreateInt32(env, -1);
        }
    }
    return CreateInt32(env, 1);
}

// 用例 35: 验证 Float64Array 类型的序列化和反序列化
static napi_value TestSerializeFloat64Array(napi_env env, napi_callback_info /* info */)
{
    napi_value ab = nullptr;
    void* rawData = nullptr;
    napi_create_arraybuffer(env, TOTAL_BYTES_64, &rawData, &ab);
    double* doubleData = static_cast<double*>(rawData);
    for (size_t i = 0; i < DOUBLE_COUNT_8; i++) {
        doubleData[i] = static_cast<double>(i) * TEST_DOUBLE_VAL;
    }
    napi_value typedArray = nullptr;
    napi_create_typedarray(env, napi_float64_array, DOUBLE_COUNT_8, ab, 0, &typedArray);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, typedArray, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_typedarray_type type = napi_int8_array;
    size_t length = 0;
    void* resultData = nullptr;
    napi_get_typedarray_info(env, result, &type, &length, &resultData, nullptr, nullptr);
    if (type != napi_float64_array || length != DOUBLE_COUNT_8 || resultData == nullptr) {
        return CreateInt32(env, -1);
    }
    double* resBytes = static_cast<double*>(resultData);
    for (size_t i = 0; i < DOUBLE_COUNT_8; i++) {
        if (resBytes[i] != static_cast<double>(i) * TEST_DOUBLE_VAL) {
            return CreateInt32(env, -1);
        }
    }
    return CreateInt32(env, 1);
}

// 用例 36: 验证 Uint8ClampedArray 类型的序列化和反序列化
static napi_value TestSerializeUint8ClampedArray(napi_env env, napi_callback_info /* info */)
{
    napi_value ab = nullptr;
    void* rawData = nullptr;
    napi_create_arraybuffer(env, BUFFER_SIZE_8, &rawData, &ab);
    uint8_t* byteData = static_cast<uint8_t*>(rawData);
    for (size_t i = 0; i < BUFFER_SIZE_8; i++) {
        byteData[i] = static_cast<uint8_t>(i + TEST_INT_100);
    }
    napi_value typedArray = nullptr;
    napi_create_typedarray(env, napi_uint8_clamped_array, BUFFER_SIZE_8, ab, 0, &typedArray);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, typedArray, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_typedarray_type type = napi_int8_array;
    size_t length = 0;
    void* resultData = nullptr;
    napi_get_typedarray_info(env, result, &type, &length, &resultData, nullptr, nullptr);
    if (type != napi_uint8_clamped_array || length != BUFFER_SIZE_8 || resultData == nullptr) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 37: 验证包含空洞（Holes）的稀疏数组序列化与反序列化
static napi_value TestSerializeArrayWithHoles(napi_env env, napi_callback_info /* info */)
{
    napi_value arrayVal = nullptr;
    napi_create_array(env, &arrayVal);
    napi_value element = CreateInt32(env, TEST_INT_200);
    napi_set_element(env, arrayVal, HOLE_INDEX_10, element);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, arrayVal, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    bool isArray = false;
    napi_is_array(env, result, &isArray);
    if (!isArray) {
        return CreateInt32(env, -1);
    }
    napi_value checkElement = nullptr;
    napi_get_element(env, result, HOLE_INDEX_10, &checkElement);
    int32_t val = 0;
    napi_get_value_int32(env, checkElement, &val);
    if (val != TEST_INT_200) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 38: 验证嵌套 Map 的序列化与反序列化
static napi_value TestSerializeMapNested(napi_env env, napi_callback_info /* info */)
{
    napi_value innerMap = nullptr;
    napi_create_map(env, &innerMap);
    napi_value keyInner = nullptr;
    napi_create_string_utf8(env, "innerKey", NAPI_AUTO_LENGTH, &keyInner);
    napi_value valInner = CreateInt32(env, TEST_INT_200);
    napi_map_set_property(env, innerMap, keyInner, valInner);
    napi_value outerMap = nullptr;
    napi_create_map(env, &outerMap);
    napi_value keyOuter = nullptr;
    napi_create_string_utf8(env, "outerKey", NAPI_AUTO_LENGTH, &keyOuter);
    napi_map_set_property(env, outerMap, keyOuter, innerMap);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, outerMap, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value checkInner = nullptr;
    napi_map_get_property(env, result, keyOuter, &checkInner);
    napi_value checkVal = nullptr;
    napi_map_get_property(env, checkInner, keyInner, &checkVal);
    int32_t finalVal = 0;
    napi_get_value_int32(env, checkVal, &finalVal);
    if (finalVal != TEST_INT_200) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 39: 验证嵌套 Set 的序列化与反序列化
static napi_value TestSerializeSetNested(napi_env env, napi_callback_info /* info */)
{
    napi_value global = nullptr;
    napi_get_global(env, &global);
    napi_value setConstructor = nullptr;
    napi_get_named_property(env, global, "Set", &setConstructor);
    napi_value innerSet = nullptr;
    napi_new_instance(env, setConstructor, 0, nullptr, &innerSet);
    napi_value addFunc = nullptr;
    napi_get_named_property(env, innerSet, "add", &addFunc);
    napi_value argInner = CreateInt32(env, TEST_INT_200);
    napi_call_function(env, innerSet, addFunc, 1, &argInner, nullptr);
    napi_value outerSet = nullptr;
    napi_new_instance(env, setConstructor, 0, nullptr, &outerSet);
    napi_value outerAddFunc = nullptr;
    napi_get_named_property(env, outerSet, "add", &outerAddFunc);
    napi_call_function(env, outerSet, outerAddFunc, 1, &innerSet, nullptr);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, outerSet, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 40: 验证长度为 0 的 ArrayBuffer 的序列化与反序列化
static napi_value TestSerializeArrayBufferZeroLength(napi_env env, napi_callback_info /* info */)
{
    void* rawData = nullptr;
    napi_value arrayBufferVal = nullptr;
    napi_create_arraybuffer(env, 0, &rawData, &arrayBufferVal);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, arrayBufferVal, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    void* resultData = nullptr;
    size_t resultLen = 1;
    napi_get_arraybuffer_info(env, result, &resultData, &resultLen);
    if (resultLen != 0) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 41: 验证包含 getter 属性的对象的序列化
static napi_value TestSerializeObjectWithGetters(napi_env env, napi_callback_info /* info */)
{
    napi_value global = nullptr;
    napi_get_global(env, &global);
    napi_value objectClass = nullptr;
    napi_get_named_property(env, global, "Object", &objectClass);
    napi_value definePropertyFunc = nullptr;
    napi_get_named_property(env, objectClass, "defineProperty", &definePropertyFunc);
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    napi_value propName = nullptr;
    napi_create_string_utf8(env, "getterProp", NAPI_AUTO_LENGTH, &propName);
    napi_value descriptor = nullptr;
    napi_create_object(env, &descriptor);
    napi_value getterFunc = nullptr;
    napi_create_function(env, "getVal", NAPI_AUTO_LENGTH, TestSerializeNull, nullptr, &getterFunc);
    napi_set_named_property(env, descriptor, "get", getterFunc);
    napi_value args[ARGS_COUNT_3];
    args[IDX_ZERO] = obj;
    args[IDX_ONE] = propName;
    args[IDX_TWO] = descriptor;
    napi_call_function(env, objectClass, definePropertyFunc, ARGS_COUNT_3, args, nullptr);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, obj, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 用例 42: 验证深层循环引用对象的序列化
static napi_value TestSerializeCircularDeep(napi_env env, napi_callback_info /* info */)
{
    napi_value root = nullptr;
    napi_create_object(env, &root);
    napi_value current = root;
    for (size_t i = 0; i < CIRCULAR_DEPTH; i++) {
        napi_value next = nullptr;
        napi_create_object(env, &next);
        napi_set_named_property(env, current, "node", next);
        current = next;
    }
    napi_set_named_property(env, current, "root", root);
    napi_value undefined = GetUndefined(env);
    void* data = nullptr;
    napi_status status = napi_serialize(env, root, undefined, undefined, &data);
    if (status != napi_ok || data == nullptr) {
        return CreateInt32(env, -1);
    }
    napi_value result = nullptr;
    status = napi_deserialize(env, data, &result);
    napi_delete_serialization_data(env, data);
    if (status != napi_ok || result == nullptr) {
        return CreateInt32(env, -1);
    }
    return CreateInt32(env, 1);
}

// 模块初始化，在此处导出上述 42 个用例供 JS 端调用以运行对应的测试
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("testSerializeBasicObject", TestSerializeBasicObject),
        DECLARE_NAPI_FUNCTION("testSerializeBooleanTrue", TestSerializeBooleanTrue),
        DECLARE_NAPI_FUNCTION("testSerializeBooleanFalse", TestSerializeBooleanFalse),
        DECLARE_NAPI_FUNCTION("testSerializeNumberInt", TestSerializeNumberInt),
        DECLARE_NAPI_FUNCTION("testSerializeNumberDouble", TestSerializeNumberDouble),
        DECLARE_NAPI_FUNCTION("testSerializeNull", TestSerializeNull),
        DECLARE_NAPI_FUNCTION("testSerializeUndefined", TestSerializeUndefined),
        DECLARE_NAPI_FUNCTION("testSerializeArray", TestSerializeArray),
        DECLARE_NAPI_FUNCTION("testSerializeCircularReference", TestSerializeCircularReference),
        DECLARE_NAPI_FUNCTION("testSerializeMap", TestSerializeMap),
        DECLARE_NAPI_FUNCTION("testSerializeSet", TestSerializeSet),
        DECLARE_NAPI_FUNCTION("testSerializeArrayBuffer", TestSerializeArrayBuffer),
        DECLARE_NAPI_FUNCTION("testSerializeTypedArray", TestSerializeTypedArray),
        DECLARE_NAPI_FUNCTION("testSerializeTransferArrayBuffer", TestSerializeTransferArrayBuffer),
        DECLARE_NAPI_FUNCTION("testSerializeCloneArrayBuffer", TestSerializeCloneArrayBuffer),
        DECLARE_NAPI_FUNCTION("testSerializeInvalidEnv", TestSerializeInvalidEnv),
        DECLARE_NAPI_FUNCTION("testSerializeNullObject", TestSerializeNullObject),
        DECLARE_NAPI_FUNCTION("testSerializeNullResult", TestSerializeNullResult),
        DECLARE_NAPI_FUNCTION("testDeserializeNullBuffer", TestDeserializeNullBuffer),
        DECLARE_NAPI_FUNCTION("testDeserializeNullObject", TestDeserializeNullObject),
        DECLARE_NAPI_FUNCTION("testDeleteSerializationDataNullBuffer", TestDeleteSerializationDataNullBuffer),
        DECLARE_NAPI_FUNCTION("testDeserializeCorruptedData", TestDeserializeCorruptedData),
        DECLARE_NAPI_FUNCTION("testSerializeRegExp", TestSerializeRegExp),
        DECLARE_NAPI_FUNCTION("testSerializeDate", TestSerializeDate),
        DECLARE_NAPI_FUNCTION("testSerializeMultiDeserialize", TestSerializeMultiDeserialize),
        DECLARE_NAPI_FUNCTION("testSerializeBigInt", TestSerializeBigInt),
        DECLARE_NAPI_FUNCTION("testSerializeEmptyObject", TestSerializeEmptyObject),
        DECLARE_NAPI_FUNCTION("testSerializeNestedObject", TestSerializeNestedObject),
        DECLARE_NAPI_FUNCTION("testSerializeUnsupportedTypeFunction", TestSerializeUnsupportedTypeFunction),
        DECLARE_NAPI_FUNCTION("testSerializeUnsupportedTypeSymbol", TestSerializeUnsupportedTypeSymbol),
        DECLARE_NAPI_FUNCTION("testSerializeEmptyString", TestSerializeEmptyString),
        DECLARE_NAPI_FUNCTION("testSerializeLongString", TestSerializeLongString),
        DECLARE_NAPI_FUNCTION("testSerializeInt8Array", TestSerializeInt8Array),
        DECLARE_NAPI_FUNCTION("testSerializeFloat32Array", TestSerializeFloat32Array),
        DECLARE_NAPI_FUNCTION("testSerializeFloat64Array", TestSerializeFloat64Array),
        DECLARE_NAPI_FUNCTION("testSerializeUint8ClampedArray", TestSerializeUint8ClampedArray),
        DECLARE_NAPI_FUNCTION("testSerializeArrayWithHoles", TestSerializeArrayWithHoles),
        DECLARE_NAPI_FUNCTION("testSerializeMapNested", TestSerializeMapNested),
        DECLARE_NAPI_FUNCTION("testSerializeSetNested", TestSerializeSetNested),
        DECLARE_NAPI_FUNCTION("testSerializeArrayBufferZeroLength", TestSerializeArrayBufferZeroLength),
        DECLARE_NAPI_FUNCTION("testSerializeObjectWithGetters", TestSerializeObjectWithGetters),
        DECLARE_NAPI_FUNCTION("testSerializeCircularDeep", TestSerializeCircularDeep),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module g_serializationModule = {
    .nm_version = MODULE_VERSION,
    .nm_flags = MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "serialization_suite",
    .nm_priv = nullptr,
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterModule()
{
    napi_module_register(&g_serializationModule);
}
