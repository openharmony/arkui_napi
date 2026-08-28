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

#include "external_helper.h"

#include <cstring>

// ============================================================
// Payload initialization implementations
// ============================================================

void InitSimplePayload(SimplePayload* payload, int32_t id, double value, const char* tag)
{
    if (payload == nullptr || tag == nullptr) {
        return;
    }
    payload->id = id;
    payload->value = value;
    size_t tagLen = std::strlen(tag);
    size_t copyLen = tagLen < ExternalConst::PAYLOAD_SIZE_SMALL - ExternalConst::INT32_ONE
        ? tagLen : ExternalConst::PAYLOAD_SIZE_SMALL - ExternalConst::INT32_ONE;
    for (size_t i = ExternalConst::INT32_ZERO; i < copyLen; i++) {
        payload->tag[i] = tag[i];
    }
    payload->tag[copyLen] = '\0';
}

void InitArrayPayload(ArrayPayload* payload, int32_t start, int32_t count)
{
    if (payload == nullptr) {
        return;
    }
    if (count > static_cast<int32_t>(ExternalConst::ARRAY_SIZE_LARGE)) {
        count = static_cast<int32_t>(ExternalConst::ARRAY_SIZE_LARGE);
    }
    if (count < ExternalConst::INT32_ZERO) {
        count = ExternalConst::INT32_ZERO;
    }
    payload->count = count;
    for (int32_t i = ExternalConst::INT32_ZERO; i < count; i++) {
        payload->values[i] = start + i;
    }
}

void InitStringPayload(StringPayload* payload, const char* data)
{
    if (payload == nullptr || data == nullptr) {
        return;
    }
    size_t dataLen = std::strlen(data);
    size_t copyLen = dataLen < ExternalConst::PAYLOAD_SIZE_LARGE - ExternalConst::INT32_ONE
        ? dataLen : ExternalConst::PAYLOAD_SIZE_LARGE - ExternalConst::INT32_ONE;
    for (size_t i = ExternalConst::INT32_ZERO; i < copyLen; i++) {
        payload->utf8Data[i] = data[i];
    }
    payload->utf8Data[copyLen] = '\0';
    payload->length = copyLen;
}

void InitNestedPayload(NestedPayload* payload, int32_t id, double value, const char* tag, int32_t extra)
{
    if (payload == nullptr) {
        return;
    }
    InitSimplePayload(&payload->inner, id, value, tag);
    payload->extra = extra;
}

void InitVectorPayload(VectorPayload* payload, double x, double y, double z, double w)
{
    if (payload == nullptr) {
        return;
    }
    payload->x = x;
    payload->y = y;
    payload->z = z;
    payload->w = w;
}

void InitMatrixPayload(MatrixPayload* payload, size_t rows, size_t cols, double fillValue)
{
    if (payload == nullptr) {
        return;
    }
    if (rows > ExternalConst::ARRAY_SIZE_MEDIUM) {
        rows = ExternalConst::ARRAY_SIZE_MEDIUM;
    }
    if (cols > ExternalConst::ARRAY_SIZE_MEDIUM) {
        cols = ExternalConst::ARRAY_SIZE_MEDIUM;
    }
    payload->rows = rows;
    payload->cols = cols;
    size_t totalElements = rows * cols;
    if (totalElements > ExternalConst::ARRAY_SIZE_MEDIUM) {
        totalElements = ExternalConst::ARRAY_SIZE_MEDIUM;
    }
    for (size_t i = ExternalConst::INT32_ZERO; i < totalElements; i++) {
        payload->elements[i] = fillValue;
    }
}

void InitBufferPayload(BufferPayload* payload, const uint8_t* data, size_t length)
{
    if (payload == nullptr || data == nullptr) {
        return;
    }
    if (length > ExternalConst::PAYLOAD_SIZE_LARGE) {
        length = ExternalConst::PAYLOAD_SIZE_LARGE;
    }
    for (size_t i = ExternalConst::INT32_ZERO; i < length; i++) {
        payload->data[i] = data[i];
    }
    payload->length = length;
    payload->checksum = ExternalConst::INT32_ZERO;
    for (size_t i = ExternalConst::INT32_ZERO; i < length; i++) {
        payload->checksum += static_cast<int32_t>(data[i]);
    }
}

void InitMetadataPayload(MetadataPayload* payload, int32_t version, int32_t flags, const char* name)
{
    if (payload == nullptr || name == nullptr) {
        return;
    }
    payload->version = version;
    payload->flags = flags;
    payload->timestamp = static_cast<int64_t>(ExternalConst::INT32_ZERO);
    size_t nameLen = std::strlen(name);
    size_t copyLen = nameLen < ExternalConst::PAYLOAD_SIZE_SMALL - ExternalConst::INT32_ONE
        ? nameLen : ExternalConst::PAYLOAD_SIZE_SMALL - ExternalConst::INT32_ONE;
    for (size_t i = ExternalConst::INT32_ZERO; i < copyLen; i++) {
        payload->name[i] = name[i];
    }
    payload->name[copyLen] = '\0';
}

void InitConfigPayload(ConfigPayload* payload, bool enabled, int32_t priority, double threshold)
{
    if (payload == nullptr) {
        return;
    }
    payload->enabled = enabled;
    payload->priority = priority;
    payload->threshold = threshold;
    payload->label[ExternalConst::INT32_ZERO] = '\0';
}

void InitStatePayload(StatePayload* payload, int32_t current, int32_t previous)
{
    if (payload == nullptr) {
        return;
    }
    payload->currentState = current;
    payload->previousState = previous;
    payload->transitionCount = static_cast<int64_t>(ExternalConst::INT32_ONE);
    payload->lastUpdate = ExternalConst::DOUBLE_ZERO;
}

// ============================================================
// Finalizer callback implementations
// ============================================================

void SimpleFinalizer(napi_env env, void* data, void* hint)
{
    (void)env;
    (void)hint;
    if (data != nullptr) {
        delete static_cast<SimplePayload*>(data);
    }
}

void ContextFinalizer(napi_env env, void* data, void* hint)
{
    (void)env;
    FinalizerContext* ctx = static_cast<FinalizerContext*>(data);
    if (ctx != nullptr) {
        if (ctx->callCount != nullptr) {
            (*ctx->callCount)++;
        }
        if (ctx->originalData != nullptr) {
            delete static_cast<SimplePayload*>(ctx->originalData);
        }
        delete ctx;
    }
    (void)hint;
}

void ArrayPayloadFinalizer(napi_env env, void* data, void* hint)
{
    (void)env;
    (void)hint;
    if (data != nullptr) {
        delete static_cast<ArrayPayload*>(data);
    }
}

void StringPayloadFinalizer(napi_env env, void* data, void* hint)
{
    (void)env;
    (void)hint;
    if (data != nullptr) {
        delete static_cast<StringPayload*>(data);
    }
}

// ============================================================
// Utility function implementations
// ============================================================

bool ComparePayloadPointers(void* ptrA, void* ptrB)
{
    return ptrA == ptrB;
}

napi_value CreateResultObject(napi_env env, bool status)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    SetNamedBool(env, result, ExternalConst::PROP_STATUS, status);
    return result;
}

napi_value CreateExternalFromPayload(napi_env env, SimplePayload* payload)
{
    if (payload == nullptr) {
        return nullptr;
    }
    napi_value external = nullptr;
    napi_status status = napi_create_external(env, payload, SimpleFinalizer, nullptr, &external);
    if (status != napi_ok) {
        return nullptr;
    }
    return external;
}

napi_value CreateExternalWithSizeFromPayload(napi_env env, SimplePayload* payload, size_t size)
{
    if (payload == nullptr) {
        return nullptr;
    }
    napi_value external = nullptr;
    napi_status status = napi_create_external_with_size(
        env, payload, SimpleFinalizer, nullptr, size, &external);
    if (status != napi_ok) {
        return nullptr;
    }
    return external;
}

bool ValidateSimplePayload(const SimplePayload* payload, int32_t expectedId, double expectedValue)
{
    if (payload == nullptr) {
        return false;
    }
    return payload->id == expectedId && payload->value == expectedValue;
}

bool ValidateArrayPayload(const ArrayPayload* payload, int32_t expectedStart, int32_t expectedCount)
{
    if (payload == nullptr) {
        return false;
    }
    if (payload->count != expectedCount) {
        return false;
    }
    for (int32_t i = ExternalConst::INT32_ZERO; i < expectedCount; i++) {
        if (payload->values[i] != expectedStart + i) {
            return false;
        }
    }
    return true;
}

bool ValidateStringPayload(const StringPayload* payload, const char* expectedData)
{
    if (payload == nullptr || expectedData == nullptr) {
        return false;
    }
    return std::strcmp(payload->utf8Data, expectedData) == ExternalConst::INT32_ZERO;
}

// ============================================================
// Set named property helper implementations
// ============================================================

bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value)
{
    if (name == nullptr) {
        return false;
    }
    napi_value napiValue = nullptr;
    if (napi_create_int32(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedDouble(napi_env env, napi_value object, const char* name, double value)
{
    if (name == nullptr) {
        return false;
    }
    napi_value napiValue = nullptr;
    if (napi_create_double(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value)
{
    if (name == nullptr) {
        return false;
    }
    napi_value napiValue = nullptr;
    if (napi_get_boolean(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedString(napi_env env, napi_value object, const char* name, const char* value)
{
    if (name == nullptr || value == nullptr) {
        return false;
    }
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedInt64(napi_env env, napi_value object, const char* name, int64_t value)
{
    if (name == nullptr) {
        return false;
    }
    napi_value napiValue = nullptr;
    if (napi_create_int64(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedUint32(napi_env env, napi_value object, const char* name, uint32_t value)
{
    if (name == nullptr) {
        return false;
    }
    napi_value napiValue = nullptr;
    if (napi_create_uint32(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedSizeT(napi_env env, napi_value object, const char* name, size_t value)
{
    if (name == nullptr) {
        return false;
    }
    napi_value napiValue = nullptr;
    uint32_t uintValue = static_cast<uint32_t>(value);
    if (napi_create_uint32(env, uintValue, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}