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

#include "class_helper.h"

// ---------------------------------------------------------------------------
// Helper Functions - Value Creation
// ---------------------------------------------------------------------------
bool CreateInt32Value(napi_env env, int32_t value, napi_value* result)
{
    return napi_create_int32(env, value, result) == napi_ok;
}

bool CreateInt64Value(napi_env env, int64_t value, napi_value* result)
{
    return napi_create_int64(env, value, result) == napi_ok;
}

bool CreateUInt32Value(napi_env env, uint32_t value, napi_value* result)
{
    return napi_create_uint32(env, value, result) == napi_ok;
}

bool CreateDoubleValue(napi_env env, double value, napi_value* result)
{
    return napi_create_double(env, value, result) == napi_ok;
}

bool CreateStringValue(napi_env env, const std::string& value, napi_value* result)
{
    return napi_create_string_utf8(env, value.c_str(), value.size(), result) == napi_ok;
}

bool CreateBoolValue(napi_env env, bool value, napi_value* result)
{
    return napi_get_boolean(env, value, result) == napi_ok;
}

// ---------------------------------------------------------------------------
// Helper Functions - Value Extraction
// ---------------------------------------------------------------------------
bool GetInt32Value(napi_env env, napi_value value, int32_t* result)
{
    return napi_get_value_int32(env, value, result) == napi_ok;
}

bool GetInt64Value(napi_env env, napi_value value, int64_t* result)
{
    return napi_get_value_int64(env, value, result) == napi_ok;
}

bool GetUInt32Value(napi_env env, napi_value value, uint32_t* result)
{
    return napi_get_value_uint32(env, value, result) == napi_ok;
}

bool GetDoubleValue(napi_env env, napi_value value, double* result)
{
    return napi_get_value_double(env, value, result) == napi_ok;
}

bool GetStringValue(napi_env env, napi_value value, std::string& result)
{
    size_t length = 0;
    napi_status status = napi_get_value_string_utf8(env, value, nullptr, 0, &length);
    if (status != napi_ok) {
        return false;
    }
    result.resize(length);
    status = napi_get_value_string_utf8(env, value, &result[K_ARG_INDEX_FIRST], length + 1, &length);
    return status == napi_ok;
}

bool GetBoolValue(napi_env env, napi_value value, bool* result)
{
    return napi_get_value_bool(env, value, result) == napi_ok;
}

// ---------------------------------------------------------------------------
// Helper Functions - Object Property Operations
// ---------------------------------------------------------------------------
bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_value napiVal = nullptr;
    if (!CreateInt32Value(env, value, &napiVal)) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiVal) == napi_ok;
}

bool SetNamedInt64(napi_env env, napi_value object, const char* name, int64_t value)
{
    napi_value napiVal = nullptr;
    if (!CreateInt64Value(env, value, &napiVal)) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiVal) == napi_ok;
}

bool SetNamedDouble(napi_env env, napi_value object, const char* name, double value)
{
    napi_value napiVal = nullptr;
    if (!CreateDoubleValue(env, value, &napiVal)) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiVal) == napi_ok;
}

bool SetNamedString(napi_env env, napi_value object, const char* name, const std::string& value)
{
    napi_value napiVal = nullptr;
    if (!CreateStringValue(env, value, &napiVal)) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiVal) == napi_ok;
}

bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value)
{
    napi_value napiVal = nullptr;
    if (!CreateBoolValue(env, value, &napiVal)) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiVal) == napi_ok;
}

bool GetNamedInt32(napi_env env, napi_value object, const char* name, int32_t* value)
{
    napi_value napiVal = nullptr;
    if (napi_get_named_property(env, object, name, &napiVal) != napi_ok) {
        return false;
    }
    return GetInt32Value(env, napiVal, value);
}

bool GetNamedDouble(napi_env env, napi_value object, const char* name, double* value)
{
    napi_value napiVal = nullptr;
    if (napi_get_named_property(env, object, name, &napiVal) != napi_ok) {
        return false;
    }
    return GetDoubleValue(env, napiVal, value);
}

bool GetNamedBool(napi_env env, napi_value object, const char* name, bool* value)
{
    napi_value napiVal = nullptr;
    if (napi_get_named_property(env, object, name, &napiVal) != napi_ok) {
        return false;
    }
    return GetBoolValue(env, napiVal, value);
}

// ---------------------------------------------------------------------------
// Helper Functions - Instance Data Storage (via properties)
// ---------------------------------------------------------------------------
bool StoreInstanceData(napi_env env, napi_value instance, const InstanceData& data)
{
    if (!SetNamedInt32(env, instance, "_id", data.id)) {
        return false;
    }
    if (!SetNamedInt32(env, instance, "_value", data.value)) {
        return false;
    }
    if (!SetNamedDouble(env, instance, "_factor", data.factor)) {
        return false;
    }
    return true;
}

bool LoadInstanceData(napi_env env, napi_value instance, InstanceData& data)
{
    if (!GetNamedInt32(env, instance, "_id", &data.id)) {
        return false;
    }
    if (!GetNamedInt32(env, instance, "_value", &data.value)) {
        return false;
    }
    if (!GetNamedDouble(env, instance, "_factor", &data.factor)) {
        return false;
    }
    return true;
}

bool StoreVectorData(napi_env env, napi_value instance, const VectorData& data)
{
    if (!SetNamedDouble(env, instance, "_x", data.x)) {
        return false;
    }
    if (!SetNamedDouble(env, instance, "_y", data.y)) {
        return false;
    }
    if (!SetNamedDouble(env, instance, "_z", data.z)) {
        return false;
    }
    return true;
}

bool LoadVectorData(napi_env env, napi_value instance, VectorData& data)
{
    if (!GetNamedDouble(env, instance, "_x", &data.x)) {
        return false;
    }
    if (!GetNamedDouble(env, instance, "_y", &data.y)) {
        return false;
    }
    if (!GetNamedDouble(env, instance, "_z", &data.z)) {
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Helper Functions - Name Building
// ---------------------------------------------------------------------------
std::string BuildIndexedName(const char* prefix, size_t caseNumber)
{
    std::string suffix = std::to_string(caseNumber);
    if (suffix.size() < static_cast<size_t>(K_CASE_NUMBER_WIDTH)) {
        suffix.insert(0, static_cast<std::string::size_type>(
            K_CASE_NUMBER_WIDTH - suffix.size()), '0');
    }
    return std::string(prefix) + suffix;
}

// ---------------------------------------------------------------------------
// Helper Functions - Case Index Conversion
// ---------------------------------------------------------------------------
size_t GetCaseIndex(void* data)
{
    return static_cast<size_t>(reinterpret_cast<uintptr_t>(data));
}

void* MakeCaseData(size_t caseIndex)
{
    return reinterpret_cast<void*>(static_cast<uintptr_t>(caseIndex));
}

// ---------------------------------------------------------------------------
// Helper Functions - Case Specifications
// ---------------------------------------------------------------------------
ClassCaseSpec GetClassCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    const int32_t idValue = static_cast<int32_t>(
        K_INSTANCE_ID_BASE + caseNumber * K_INSTANCE_ID_STEP);
    const double factorValue = K_INSTANCE_FACTOR * static_cast<double>(caseNumber);
    return {
        BuildIndexedName("TestClass", caseNumber),
        idValue,
        factorValue,
        (caseIndex % K_ITER_COUNT_THREE) == 0
    };
}

InstanceCaseSpec GetInstanceCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    const int32_t inputVal = static_cast<int32_t>(caseNumber) * K_INT_VALUE_TEN;
    const int32_t outputVal = inputVal + K_INT_VALUE_FIVE;
    return {
        BuildIndexedName("method", caseNumber),
        inputVal,
        outputVal
    };
}

ArgCaseSpec GetArgCaseSpec(size_t caseIndex)
{
    const size_t argCount = (caseIndex % K_ITER_COUNT_FIVE) + K_ARGS_ONE;
    const int32_t argType = static_cast<int32_t>(caseIndex % K_ARGS_FIVE);
    const char* types[] = {"int32", "int64", "double", "string", "boolean"};
    const char* typeDesc = types[argType];
    return {
        argCount,
        argType,
        std::string("ArgTest with ") + typeDesc + " args"
    };
}

// ---------------------------------------------------------------------------
// Helper Functions - Result Object Creation
// ---------------------------------------------------------------------------
napi_value CreateResultObject(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    return result;
}

bool SetResultSuccess(napi_env env, napi_value result, const char* testName)
{
    return SetNamedBool(env, result, "success", true) &&
           SetNamedString(env, result, "testName", testName);
}

bool SetResultFailure(napi_env env, napi_value result, const char* testName, const char* reason)
{
    return SetNamedBool(env, result, "success", false) &&
           SetNamedString(env, result, "testName", testName) &&
           SetNamedString(env, result, "reason", reason);
}