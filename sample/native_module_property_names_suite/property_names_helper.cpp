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

#include "property_names_helper.h"

#include <algorithm>
#include <sstream>

std::string BuildIndexedName(const char* prefix, size_t caseNumber)
{
    std::string suffix = std::to_string(caseNumber);
    if (suffix.size() < static_cast<size_t>(PropertyNamesConst::K_CASE_NUMBER_WIDTH)) {
        suffix.insert(0, PropertyNamesConst::K_CASE_NUMBER_WIDTH - suffix.size(), '0');
    }
    return std::string(prefix) + suffix;
}

size_t GetCaseIndex(void* data)
{
    return static_cast<size_t>(reinterpret_cast<uintptr_t>(data));
}

bool CreateInt32Value(napi_env env, int32_t value, napi_value* result)
{
    if (env == nullptr || result == nullptr) {
        return false;
    }
    return napi_create_int32(env, value, result) == napi_ok;
}

bool CreateStringValue(napi_env env, const std::string& value, napi_value* result)
{
    if (env == nullptr || result == nullptr) {
        return false;
    }
    return napi_create_string_utf8(env, value.c_str(), value.size(), result) == napi_ok;
}

bool GetInt32Value(napi_env env, napi_value value, int32_t* result)
{
    if (env == nullptr || value == nullptr || result == nullptr) {
        return false;
    }
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, value, &type) != napi_ok || type != napi_number) {
        return false;
    }
    return napi_get_value_int32(env, value, result) == napi_ok;
}

bool GetStringValue(napi_env env, napi_value value, std::string* result)
{
    if (env == nullptr || value == nullptr || result == nullptr) {
        return false;
    }
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, value, &type) != napi_ok || type != napi_string) {
        return false;
    }
    size_t length = 0;
    if (napi_get_value_string_utf8(env, value, nullptr, 0, &length) != napi_ok) {
        return false;
    }
    std::string buffer(length + PropertyNamesConst::K_NULL_TERMINATOR_SIZE, '\0');
    if (napi_get_value_string_utf8(env, value, buffer.data(), buffer.size(), &length) != napi_ok) {
        return false;
    }
    buffer.resize(length);
    *result = buffer;
    return true;
}

bool SetNamedPropertyInt32(napi_env env, napi_value object, const char* name, int32_t value)
{
    if (env == nullptr || object == nullptr || name == nullptr) {
        return false;
    }
    napi_value napiValue = nullptr;
    if (!CreateInt32Value(env, value, &napiValue)) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedPropertyString(napi_env env, napi_value object, const char* name, const std::string& value)
{
    if (env == nullptr || object == nullptr || name == nullptr) {
        return false;
    }
    napi_value napiValue = nullptr;
    if (!CreateStringValue(env, value, &napiValue)) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool DefinePropertyWithAttributes(napi_env env, napi_value object, const char* name, int32_t value,
    const PropertyAttributes& attributes)
{
    if (env == nullptr || object == nullptr || name == nullptr) {
        return false;
    }
    napi_value propValue = nullptr;
    if (!CreateInt32Value(env, value, &propValue)) {
        return false;
    }
    uint32_t attrFlags = napi_default;
    if (!attributes.writable) {
        attrFlags |= napi_writable;
    }
    if (!attributes.enumerable) {
        attrFlags |= napi_enumerable;
    }
    if (!attributes.configurable) {
        attrFlags |= napi_configurable;
    }
    napi_property_descriptor descriptor = {
        name,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        propValue,
        attrFlags,
        nullptr,
    };
    return napi_define_properties(env, object, PropertyNamesArgCount::ONE, &descriptor) == napi_ok;
}

napi_value CreateTestObject(napi_env env, const std::vector<std::string>& keys, const std::vector<int32_t>& values)
{
    if (env == nullptr) {
        return nullptr;
    }
    napi_value object = nullptr;
    if (napi_create_object(env, &object) != napi_ok) {
        return nullptr;
    }
    const size_t count = std::min(keys.size(), values.size());
    for (size_t i = 0; i < count; i++) {
        if (!SetNamedPropertyInt32(env, object, keys[i].c_str(), values[i])) {
            return nullptr;
        }
    }
    return object;
}

bool EnumeratePropertyNames(napi_env env, napi_value object, const EnumOptions& options, EnumResult* result)
{
    if (env == nullptr || object == nullptr || result == nullptr) {
        return false;
    }
    napi_value namesArray = nullptr;
    napi_status status = napi_get_all_property_names(env, object, options.collectionMode,
        options.keyFilter, options.conversion, &namesArray);
    if (status != napi_ok) {
        return false;
    }
    if (napi_get_array_length(env, namesArray, &result->keyCount) != napi_ok) {
        return false;
    }
    return CollectArrayKeys(env, namesArray, &result->foundKeys);
}

bool CheckPropertyMembership(napi_env env, napi_value object, const std::string& key, MembershipResult* result)
{
    if (env == nullptr || object == nullptr || result == nullptr) {
        return false;
    }
    napi_value keyStr = nullptr;
    if (!CreateStringValue(env, key, &keyStr)) {
        return false;
    }
    return napi_has_own_property(env, object, keyStr, &result->hasProperty) == napi_ok;
}

bool CollectArrayKeys(napi_env env, napi_value array, std::vector<std::string>* keys)
{
    if (env == nullptr || array == nullptr || keys == nullptr) {
        return false;
    }
    bool isArray = false;
    if (napi_is_array(env, array, &isArray) != napi_ok || !isArray) {
        return false;
    }
    uint32_t length = 0;
    if (napi_get_array_length(env, array, &length) != napi_ok) {
        return false;
    }
    keys->clear();
    keys->reserve(length);
    for (uint32_t i = 0; i < length; i++) {
        napi_value element = nullptr;
        if (napi_get_element(env, array, i, &element) != napi_ok) {
            return false;
        }
        std::string keyStr;
        if (!GetStringValue(env, element, &keyStr)) {
            return false;
        }
        keys->push_back(keyStr);
    }
    return true;
}

napi_value CreateResultObject(napi_env env, const std::string& caseName, bool passed,
    uint32_t keyCount, const std::vector<std::string>& keys)
{
    if (env == nullptr) {
        return nullptr;
    }
    napi_value result = nullptr;
    if (napi_create_object(env, &result) != napi_ok) {
        return nullptr;
    }
    if (!SetNamedPropertyString(env, result, "caseName", caseName)) {
        return nullptr;
    }
    napi_value passedValue = nullptr;
    if (napi_get_boolean(env, passed, &passedValue) != napi_ok) {
        return nullptr;
    }
    if (napi_set_named_property(env, result, "passed", passedValue) != napi_ok) {
        return nullptr;
    }
    if (!SetNamedPropertyInt32(env, result, "keyCount", static_cast<int32_t>(keyCount))) {
        return nullptr;
    }
    napi_value keysArray = nullptr;
    if (napi_create_array_with_length(env, keys.size(), &keysArray) != napi_ok) {
        return nullptr;
    }
    for (size_t i = 0; i < keys.size(); i++) {
        napi_value keyStr = nullptr;
        if (!CreateStringValue(env, keys[i], &keyStr)) {
            return nullptr;
        }
        if (napi_set_element(env, keysArray, static_cast<uint32_t>(i), keyStr) != napi_ok) {
            return nullptr;
        }
    }
    if (napi_set_named_property(env, result, "keys", keysArray) != napi_ok) {
        return nullptr;
    }
    return result;
}

bool VerifyKeyPresence(napi_env env, napi_value object, const std::vector<KeySpec>& expectedKeys)
{
    if (env == nullptr || object == nullptr) {
        return false;
    }
    for (const auto& spec : expectedKeys) {
        MembershipResult result = {false, false};
        if (!CheckPropertyMembership(env, object, spec.name, &result)) {
            return false;
        }
        if (result.hasProperty != spec.shouldExist) {
            return false;
        }
        if (result.hasProperty && spec.expectedValue != PropertyNamesConst::K_INVALID_INDEX) {
            napi_value value = nullptr;
            if (napi_get_named_property(env, object, spec.name.c_str(), &value) != napi_ok) {
                return false;
            }
            int32_t actualValue = PropertyNamesConst::K_INVALID_INDEX;
            if (!GetInt32Value(env, value, &actualValue)) {
                return false;
            }
            if (actualValue != spec.expectedValue) {
                return false;
            }
        }
    }
    return true;
}

bool CreateObjectWithProperties(napi_env env, napi_value* result, const std::vector<std::string>& keys)
{
    if (env == nullptr || result == nullptr) {
        return false;
    }
    napi_value object = nullptr;
    if (napi_create_object(env, &object) != napi_ok) {
        return false;
    }
    for (size_t i = 0; i < keys.size(); i++) {
        int32_t value = PropertyNamesConst::K_BASE_INT_VALUE + static_cast<int32_t>(i);
        if (!SetNamedPropertyInt32(env, object, keys[i].c_str(), value)) {
            return false;
        }
    }
    *result = object;
    return true;
}

bool HasAllExpectedKeys(napi_env env, napi_value object, const std::vector<std::string>& expectedKeys)
{
    if (env == nullptr || object == nullptr) {
        return false;
    }
    for (const auto& key : expectedKeys) {
        MembershipResult result = {false, false};
        if (!CheckPropertyMembership(env, object, key, &result)) {
            return false;
        }
        if (!result.hasProperty) {
            return false;
        }
    }
    return true;
}

bool VerifyEnumeratedKeyCount(napi_env env, napi_value array, uint32_t expectedCount, bool* result)
{
    if (env == nullptr || array == nullptr || result == nullptr) {
        return false;
    }
    uint32_t length = 0;
    if (napi_get_array_length(env, array, &length) != napi_ok) {
        return false;
    }
    *result = (length == expectedCount);
    return true;
}

bool FindKeyInArray(napi_env env, napi_value array, const std::string& key, bool* found)
{
    if (env == nullptr || array == nullptr || found == nullptr) {
        return false;
    }
    uint32_t length = 0;
    if (napi_get_array_length(env, array, &length) != napi_ok) {
        return false;
    }
    for (uint32_t i = 0; i < length; i++) {
        napi_value element = nullptr;
        if (napi_get_element(env, array, i, &element) != napi_ok) {
            return false;
        }
        std::string elementStr;
        if (!GetStringValue(env, element, &elementStr)) {
            return false;
        }
        if (elementStr == key) {
            *found = true;
            return true;
        }
    }
    *found = false;
    return true;
}

bool CreateEmptyObject(napi_env env, napi_value* result)
{
    if (env == nullptr || result == nullptr) {
        return false;
    }
    return napi_create_object(env, result) == napi_ok;
}

bool CreateEmptyArray(napi_env env, napi_value* result)
{
    if (env == nullptr || result == nullptr) {
        return false;
    }
    return napi_create_array(env, result) == napi_ok;
}

bool GetUndefinedValue(napi_env env, napi_value* result)
{
    if (env == nullptr || result == nullptr) {
        return false;
    }
    return napi_get_undefined(env, result) == napi_ok;
}

bool GetNullValue(napi_env env, napi_value* result)
{
    if (env == nullptr || result == nullptr) {
        return false;
    }
    return napi_get_null(env, result) == napi_ok;
}