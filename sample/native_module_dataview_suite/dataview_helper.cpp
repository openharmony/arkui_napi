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

#include "dataview_helper.h"

#include <cstring>
#include "securec.h"

using namespace DataViewConst;

bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value)
{
    napi_value napiValue = nullptr;
    if (napi_get_boolean(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int32(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedUint32(napi_env env, napi_value object, const char* name, uint32_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_uint32(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedDouble(napi_env env, napi_value object, const char* name, double value)
{
    napi_value napiValue = nullptr;
    if (napi_create_double(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedString(napi_env env, napi_value object, const char* name, const std::string& value)
{
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value.c_str(), value.size(), &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedInt64(napi_env env, napi_value object, const char* name, int64_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int64(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

napi_value CreateResultObject(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    return result;
}

napi_value CreateArrayBufferWithSize(napi_env env, size_t byteLength, void** data)
{
    napi_value arrayBuffer = nullptr;
    NAPI_CALL(env, napi_create_arraybuffer(env, byteLength, data, &arrayBuffer));
    return arrayBuffer;
}

napi_value CreateDataViewWithParams(napi_env env, size_t byteLength, napi_value arrayBuffer,
    size_t byteOffset, napi_value* dataView)
{
    napi_status status = napi_create_dataview(env, byteLength, arrayBuffer, byteOffset, dataView);
    if (status != napi_ok) {
        *dataView = nullptr;
    }
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

void FillBufferWithPattern(void* data, size_t length, uint8_t value)
{
    if (data == nullptr || length == K_ZERO_LENGTH) {
        return;
    }
    (void)memset_s(data, length, value, length);
}

bool VerifyBufferPattern(void* data, size_t length, uint8_t expectedValue)
{
    if (data == nullptr || length == K_ZERO_LENGTH) {
        return false;
    }
    auto* bytes = static_cast<uint8_t*>(data);
    for (size_t i = K_ZERO_LENGTH; i < length; i++) {
        if (bytes[i] != expectedValue) {
            return false;
        }
    }
    return true;
}

void FillBufferPartial(void* data, size_t length, size_t start, size_t count, uint8_t value)
{
    if (data == nullptr || start >= length || count == K_ZERO_LENGTH) {
        return;
    }
    size_t endPos = start + count;
    if (endPos > length) {
        endPos = length;
    }
    auto* bytes = static_cast<uint8_t*>(data);
    for (size_t i = start; i < endPos; i++) {
        bytes[i] = value;
    }
}

bool VerifyBufferPartial(void* data, size_t length, size_t start, size_t count, uint8_t expectedValue)
{
    if (data == nullptr || start >= length || count == K_ZERO_LENGTH) {
        return false;
    }
    size_t endPos = start + count;
    if (endPos > length) {
        endPos = length;
    }
    auto* bytes = static_cast<uint8_t*>(data);
    for (size_t i = start; i < endPos; i++) {
        if (bytes[i] != expectedValue) {
            return false;
        }
    }
    return true;
}

napi_value BuildArrayBufferInfoResult(napi_env env, void* expectedData, size_t expectedLength,
    void* actualData, size_t actualLength)
{
    napi_value result = CreateResultObject(env);
    SetNamedBool(env, result, "dataMatch", expectedData == actualData);
    SetNamedBool(env, result, "lengthMatch", expectedLength == actualLength);
    SetNamedUint32(env, result, "byteLength", static_cast<uint32_t>(actualLength));
    return result;
}

napi_value BuildDataViewInfoResult(napi_env env, const DataViewInfoResult& result)
{
    napi_value obj = CreateResultObject(env);
    SetNamedBool(env, obj, "dataNotNull", result.dataNotNull);
    SetNamedBool(env, obj, "lengthMatch", result.lengthMatch);
    SetNamedBool(env, obj, "offsetMatch", result.offsetMatch);
    SetNamedBool(env, obj, "bufferNotNull", result.bufferNotNull);
    SetNamedUint32(env, obj, "byteLength", static_cast<uint32_t>(result.byteLength));
    SetNamedUint32(env, obj, "byteOffset", static_cast<uint32_t>(result.byteOffset));
    return obj;
}

napi_value BuildDataViewWriteResult(napi_env env, const DataViewWriteResult& result)
{
    napi_value obj = CreateResultObject(env);
    SetNamedBool(env, obj, "writeSuccess", result.writeSuccess);
    SetNamedBool(env, obj, "readBackMatch", result.readBackMatch);
    SetNamedUint32(env, obj, "writtenValue", static_cast<uint32_t>(result.writtenValue));
    SetNamedUint32(env, obj, "readValue", static_cast<uint32_t>(result.readValue));
    return obj;
}

napi_value BuildBooleanResult(napi_env env, bool value, const char* keyName)
{
    napi_value result = CreateResultObject(env);
    SetNamedBool(env, result, keyName, value);
    return result;
}

napi_value BuildMultiBooleanResult(napi_env env, bool* values, const char** keys, size_t count)
{
    napi_value result = CreateResultObject(env);
    for (size_t i = K_ZERO_LENGTH; i < count; i++) {
        SetNamedBool(env, result, keys[i], values[i]);
    }
    return result;
}

size_t GetSafeArgCount(napi_env env, napi_callback_info info, size_t expected)
{
    size_t argc = expected;
    napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
    return argc;
}

bool GetArgValueAsSize(napi_env env, napi_value arg, size_t* outValue)
{
    uint32_t value = static_cast<uint32_t>(K_ZERO_LENGTH);
    napi_status status = napi_get_value_uint32(env, arg, &value);
    if (status != napi_ok) {
        return false;
    }
    *outValue = static_cast<size_t>(value);
    return true;
}

bool GetArgValueAsInt32(napi_env env, napi_value arg, int32_t* outValue)
{
    int32_t value = 0;
    napi_status status = napi_get_value_int32(env, arg, &value);
    if (status != napi_ok) {
        return false;
    }
    *outValue = value;
    return true;
}

bool GetArgValueAsDouble(napi_env env, napi_value arg, double* outValue)
{
    double value = 0.0;
    napi_status status = napi_get_value_double(env, arg, &value);
    if (status != napi_ok) {
        return false;
    }
    *outValue = value;
    return true;
}

napi_value CreateArrayBufferAndFill(napi_env env, size_t byteLength, uint8_t fillValue, void** data)
{
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, byteLength, data);
    if (arrayBuffer != nullptr && *data != nullptr) {
        FillBufferWithPattern(*data, byteLength, fillValue);
    }
    return arrayBuffer;
}

napi_value CreateDataViewFromSpec(napi_env env, const DataViewTestSpec& spec, napi_value* dataView)
{
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, spec.bufferSize, &data);
    if (arrayBuffer == nullptr) {
        *dataView = nullptr;
        napi_value undefined = nullptr;
        napi_get_undefined(env, &undefined);
        return undefined;
    }
    napi_status status = napi_create_dataview(env, spec.dataViewLength, arrayBuffer, spec.dataViewOffset, dataView);
    if (status != napi_ok) {
        *dataView = nullptr;
    }
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    return undefined;
}

bool WriteByteToView(void* viewData, size_t index, uint8_t value)
{
    if (viewData == nullptr) {
        return false;
    }
    auto* bytes = static_cast<uint8_t*>(viewData);
    bytes[index] = value;
    return true;
}

bool ReadByteFromView(void* viewData, size_t index, uint8_t* value)
{
    if (viewData == nullptr || value == nullptr) {
        return false;
    }
    auto* bytes = static_cast<uint8_t*>(viewData);
    *value = bytes[index];
    return true;
}

bool WriteMultipleBytes(void* viewData, const uint8_t* values, size_t count)
{
    if (viewData == nullptr || values == nullptr || count == K_ZERO_LENGTH) {
        return false;
    }
    auto* bytes = static_cast<uint8_t*>(viewData);
    for (size_t i = K_ZERO_LENGTH; i < count; i++) {
        bytes[i] = values[i];
    }
    return true;
}

bool ReadMultipleBytes(void* viewData, uint8_t* values, size_t count)
{
    if (viewData == nullptr || values == nullptr || count == K_ZERO_LENGTH) {
        return false;
    }
    auto* bytes = static_cast<uint8_t*>(viewData);
    for (size_t i = K_ZERO_LENGTH; i < count; i++) {
        values[i] = bytes[i];
    }
    return true;
}

napi_value BuildTestResultWithMessage(napi_env env, bool success, const char* message)
{
    napi_value result = CreateResultObject(env);
    SetNamedBool(env, result, "success", success);
    SetNamedString(env, result, "message", std::string(message));
    return result;
}

napi_value BuildCompareResult(napi_env env, bool match, size_t expected, size_t actual)
{
    napi_value result = CreateResultObject(env);
    SetNamedBool(env, result, "match", match);
    SetNamedUint32(env, result, "expected", static_cast<uint32_t>(expected));
    SetNamedUint32(env, result, "actual", static_cast<uint32_t>(actual));
    return result;
}

bool IsPowerOfTwo(size_t value)
{
    if (value == K_ZERO_LENGTH) {
        return false;
    }
    return (value & (value - 1)) == K_ZERO_LENGTH;
}

size_t AlignToBoundary(size_t value, size_t boundary)
{
    if (boundary == 0) {
        return value;
    }
    size_t remainder = value % boundary;
    if (remainder == K_ZERO_LENGTH) {
        return value;
    }
    return value + (boundary - remainder);
}

uint8_t ComputeChecksum(uint8_t* data, size_t length)
{
    if (data == nullptr || length == K_ZERO_LENGTH) {
        return static_cast<uint8_t>(K_ZERO_LENGTH);
    }
    uint32_t sum = K_ZERO_LENGTH;
    for (size_t i = K_ZERO_LENGTH; i < length; i++) {
        sum += data[i];
    }
    return static_cast<uint8_t>(sum & K_CHECKSUM_BYTE_MASK);
}

bool ValidateChecksum(uint8_t* data, size_t length, uint8_t expectedChecksum)
{
    uint8_t computed = ComputeChecksum(data, length);
    return computed == expectedChecksum;
}