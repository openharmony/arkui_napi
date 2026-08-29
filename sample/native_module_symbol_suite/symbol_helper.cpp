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

#include "symbol_helper.h"

#include <vector>

namespace SymbolSuite {

std::string BuildIndexedName(const char* prefix, size_t caseNumber)
{
    std::string suffix = std::to_string(caseNumber);
    if (suffix.size() < static_cast<size_t>(K_CASE_NUMBER_WIDTH)) {
        suffix.insert(0, K_CASE_NUMBER_WIDTH - suffix.size(), '0');
    }
    return std::string(prefix) + suffix;
}

size_t GetCaseIndex(void* data)
{
    return static_cast<size_t>(reinterpret_cast<uintptr_t>(data));
}

CreateSymbolCaseSpec GetCreateSymbolCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    bool hasDesc = (caseIndex % K_ARG_COUNT_TWO) == 0;
    std::string desc = hasDesc ? BuildIndexedName("symbolDesc", caseNumber) : "";
    return {BuildIndexedName("createSymbolCase", caseNumber), desc, hasDesc,
        static_cast<int32_t>(desc.size())};
}

TypeofCaseSpec GetTypeofCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    size_t typeIndex = caseIndex % K_ARG_COUNT_FIVE;
    std::string valueType;
    bool expectSymbol = false;
    int32_t typeCode = 0;
    switch (typeIndex) {
        case K_FIRST_ARG_INDEX: valueType = "symbol"; expectSymbol = true; typeCode = K_TYPE_CODE_SYMBOL; break;
        case K_SECOND_ARG_INDEX: valueType = "number"; typeCode = K_TYPE_CODE_NUMBER; break;
        case K_THIRD_ARG_INDEX: valueType = "string"; typeCode = K_TYPE_CODE_STRING; break;
        case K_FOURTH_ARG_INDEX: valueType = "object"; typeCode = K_TYPE_CODE_OBJECT; break;
        default: valueType = "boolean"; typeCode = K_TYPE_CODE_BOOLEAN; break;
    }
    return {BuildIndexedName("typeofCase", caseNumber), valueType, expectSymbol, typeCode};
}

IdentityCaseSpec GetIdentityCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    int32_t seedA = K_IDENTITY_SEED_BASE + static_cast<int32_t>(caseIndex % K_IDENTITY_SEED_CYCLE);
    bool sameSeeds = (caseIndex % K_ARG_COUNT_TWO) == 0;
    int32_t seedB = sameSeeds ? seedA : seedA + K_IDENTITY_SEED_STEP;
    return {BuildIndexedName("identityCase", caseNumber), seedA, seedB, sameSeeds,
        "symA" + std::to_string(seedA), "symB" + std::to_string(seedB)};
}

PropertyCaseSpec GetPropertyCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    bool useSymbolKey = (caseIndex % K_ARG_COUNT_TWO) == 0;
    bool shouldDelete = (caseIndex % K_ARG_COUNT_THREE) == 0;
    int32_t propertyValue = K_PROPERTY_VALUE_BASE +
        static_cast<int32_t>(caseIndex % K_PROPERTY_VALUE_CYCLE) * K_PROPERTY_VALUE_STEP;
    return {BuildIndexedName("propertyCase", caseNumber), BuildIndexedName("propKey", caseNumber),
        static_cast<int32_t>(caseNumber), useSymbolKey, propertyValue, true, shouldDelete};
}

BoxingCaseSpec GetBoxingCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    bool shouldBox = (caseIndex % K_ARG_COUNT_TWO) == 0;
    return {BuildIndexedName("boxingCase", caseNumber), shouldBox, shouldBox,
        static_cast<int32_t>(caseNumber)};
}

MultiPropertyCaseSpec GetMultiPropertyCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    bool useSymbolKeys = (caseIndex % K_ARG_COUNT_TWO) == 0;
    std::vector<int32_t> values;
    for (int32_t i = 0; i < K_MULTI_PROPERTY_COUNT; i++) {
        values.push_back(static_cast<int32_t>(caseNumber) * K_VALUE_STRIDE + i);
    }
    return {BuildIndexedName("multiPropertyCase", caseNumber), K_MULTI_PROPERTY_COUNT,
        useSymbolKeys, values};
}

SymbolObjectCaseSpec GetSymbolObjectCaseSpec(size_t caseIndex)
{
    bool createBoxed = (caseIndex % K_ARG_COUNT_TWO) == 0;
    return {BuildIndexedName("symbolObjectCase", caseIndex + K_FIRST_CASE_NUMBER),
        createBoxed, true, createBoxed};
}

NestedPropertyCaseSpec GetNestedPropertyCaseSpec(size_t caseIndex)
{
    bool useSymbolKeys = (caseIndex % K_ARG_COUNT_TWO) == 0;
    return {BuildIndexedName("nestedPropertyCase", caseIndex + K_FIRST_CASE_NUMBER),
        K_NESTED_DEPTH, useSymbolKeys, static_cast<int32_t>(caseIndex + K_FIRST_CASE_NUMBER) * K_VALUE_SCALE};
}

DeleteCycleCaseSpec GetDeleteCycleCaseSpec(size_t caseIndex)
{
    bool useSymbolKey = (caseIndex % K_ARG_COUNT_TWO) == 0;
    return {BuildIndexedName("deleteCycleCase", caseIndex + K_FIRST_CASE_NUMBER),
        K_DELETE_CYCLE_COUNT, useSymbolKey};
}

std::string BuildCreateSymbolExportName(size_t caseIndex)
{
    return BuildIndexedName("testCreateSymbol", caseIndex + K_FIRST_CASE_NUMBER);
}

std::string BuildTypeofExportName(size_t caseIndex)
{
    return BuildIndexedName("testTypeof", caseIndex + K_FIRST_CASE_NUMBER);
}

std::string BuildIdentityExportName(size_t caseIndex)
{
    return BuildIndexedName("testIdentity", caseIndex + K_FIRST_CASE_NUMBER);
}

std::string BuildPropertyExportName(size_t caseIndex)
{
    return BuildIndexedName("testProperty", caseIndex + K_FIRST_CASE_NUMBER);
}

std::string BuildBoxingExportName(size_t caseIndex)
{
    return BuildIndexedName("testBoxing", caseIndex + K_FIRST_CASE_NUMBER);
}

std::string BuildMultiPropertyExportName(size_t caseIndex)
{
    return BuildIndexedName("testMultiProperty", caseIndex + K_FIRST_CASE_NUMBER);
}

std::string BuildSymbolObjectExportName(size_t caseIndex)
{
    return BuildIndexedName("testSymbolObject", caseIndex + K_FIRST_CASE_NUMBER);
}

std::string BuildNestedPropertyExportName(size_t caseIndex)
{
    return BuildIndexedName("testNestedProperty", caseIndex + K_FIRST_CASE_NUMBER);
}

std::string BuildDeleteCycleExportName(size_t caseIndex)
{
    return BuildIndexedName("testDeleteCycle", caseIndex + K_FIRST_CASE_NUMBER);
}

bool CreateSymbolWithDesc(napi_env env, const std::string& desc, napi_value* symbol)
{
    napi_value descValue = nullptr;
    if (napi_create_string_utf8(env, desc.c_str(), desc.size(), &descValue) != napi_ok) {
        return false;
    }
    return napi_create_symbol(env, descValue, symbol) == napi_ok;
}

bool CreateSymbolWithoutDesc(napi_env env, napi_value* symbol)
{
    return napi_create_symbol(env, nullptr, symbol) == napi_ok;
}

bool CheckSymbolTypeof(napi_env env, napi_value value, bool* isSymbol)
{
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, value, &valueType) != napi_ok) {
        return false;
    }
    *isSymbol = (valueType == napi_symbol);
    return true;
}

bool CheckStrictEquals(napi_env env, napi_value a, napi_value b, bool* result)
{
    return napi_strict_equals(env, a, b, result) == napi_ok;
}

bool SetPropertyWithKey(napi_env env, napi_value object, napi_value key, napi_value value)
{
    return napi_set_property(env, object, key, value) == napi_ok;
}

bool GetPropertyWithKey(napi_env env, napi_value object, napi_value key, napi_value* value)
{
    return napi_get_property(env, object, key, value) == napi_ok;
}

bool HasPropertyWithKey(napi_env env, napi_value object, napi_value key, bool* result)
{
    return napi_has_property(env, object, key, result) == napi_ok;
}

bool DeletePropertyWithKey(napi_env env, napi_value object, napi_value key, bool* result)
{
    return napi_delete_property(env, object, key, result) == napi_ok;
}

bool CoerceToObject(napi_env env, napi_value value, napi_value* object)
{
    return napi_coerce_to_object(env, value, object) == napi_ok;
}

bool IsSymbolObject(napi_env env, napi_value value, bool* result)
{
    return napi_is_symbol_object(env, value, result) == napi_ok;
}

bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int32(env, value, &napiValue) != napi_ok) {
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

bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value)
{
    napi_value napiValue = nullptr;
    if (napi_get_boolean(env, value, &napiValue) != napi_ok) {
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

bool SetNamedInt64(napi_env env, napi_value object, const char* name, int64_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int64(env, value, &napiValue) != napi_ok) {
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

bool GetInt32Property(napi_env env, napi_value object, const char* name, int32_t* value)
{
    napi_value napiValue = nullptr;
    if (napi_get_named_property(env, object, name, &napiValue) != napi_ok) {
        return false;
    }
    return napi_get_value_int32(env, napiValue, value) == napi_ok;
}

bool GetStringProperty(napi_env env, napi_value object, const char* name, std::string* value)
{
    napi_value napiValue = nullptr;
    if (napi_get_named_property(env, object, name, &napiValue) != napi_ok) {
        return false;
    }
    size_t length = 0;
    if (napi_get_value_string_utf8(env, napiValue, nullptr, 0, &length) != napi_ok) {
        return false;
    }
    value->resize(length);
    return napi_get_value_string_utf8(env, napiValue, &value[0], length + 1, &length) == napi_ok;
}

bool GetBoolProperty(napi_env env, napi_value object, const char* name, bool* value)
{
    napi_value napiValue = nullptr;
    if (napi_get_named_property(env, object, name, &napiValue) != napi_ok) {
        return false;
    }
    return napi_get_value_bool(env, napiValue, value) == napi_ok;
}

bool GetDoubleProperty(napi_env env, napi_value object, const char* name, double* value)
{
    napi_value napiValue = nullptr;
    if (napi_get_named_property(env, object, name, &napiValue) != napi_ok) {
        return false;
    }
    return napi_get_value_double(env, napiValue, value) == napi_ok;
}

bool GetInt64Property(napi_env env, napi_value object, const char* name, int64_t* value)
{
    napi_value napiValue = nullptr;
    if (napi_get_named_property(env, object, name, &napiValue) != napi_ok) {
        return false;
    }
    return napi_get_value_int64(env, napiValue, value) == napi_ok;
}

bool GetUint32Property(napi_env env, napi_value object, const char* name, uint32_t* value)
{
    napi_value napiValue = nullptr;
    if (napi_get_named_property(env, object, name, &napiValue) != napi_ok) {
        return false;
    }
    return napi_get_value_uint32(env, napiValue, value) == napi_ok;
}

int32_t ComputeSymbolHash(const std::string& desc)
{
    int32_t hash = 0;
    for (size_t i = 0; i < desc.size(); i++) {
        hash = (hash * K_SYMBOL_HASH_BASE + static_cast<int32_t>(desc[i])) % K_SYMBOL_HASH_MODULO;
    }
    return hash;
}

bool CreateSymbolKey(napi_env env, int32_t keyIndex, napi_value* key)
{
    return CreateSymbolWithDesc(env, "symKey" + std::to_string(keyIndex), key);
}

bool CreateMultipleSymbolKeys(napi_env env, int32_t count, std::vector<napi_value>& keys)
{
    keys.clear();
    keys.reserve(static_cast<size_t>(count));
    for (int32_t i = 0; i < count; i++) {
        napi_value key = nullptr;
        if (!CreateSymbolKey(env, i, &key)) {
            return false;
        }
        keys.push_back(key);
    }
    return true;
}

}  // namespace SymbolSuite