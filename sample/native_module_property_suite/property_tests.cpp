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

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

constexpr size_t K_PROPERTY_CASE_COUNT = 20;
constexpr size_t K_PROPERTY_ARG_COUNT = 2;
constexpr size_t K_FIRST_ARG_INDEX = 0;
constexpr size_t K_SECOND_ARG_INDEX = 1;
constexpr size_t K_FIRST_CASE_NUMBER = 1;
constexpr int K_CASE_NUMBER_WIDTH = 2;
constexpr uint32_t K_MODULE_VERSION = 1;
constexpr uint32_t K_NO_MODULE_FLAGS = 0;
constexpr size_t K_NULL_TERMINATOR_SIZE = 1;
constexpr int32_t K_INVALID_VALUE = -1;
constexpr int32_t K_BASE_PROPERTY_COUNT = 1;
constexpr size_t K_OPERATION_COUNT = 4;
constexpr size_t K_CASES_PER_OPERATION = 5;
constexpr size_t K_KEY_NAME_COUNT = 5;

constexpr std::array<const char*, K_KEY_NAME_COUNT> K_KEY_NAMES = {
    "alpha",
    "beta",
    "gamma",
    "delta",
    "epsilon",
};

constexpr std::array<const char*, K_OPERATION_COUNT> K_OPERATION_LABELS = {
    "setAndGet",
    "hasProperty",
    "deleteProperty",
    "enumerate",
};

enum class PropertyOp : int32_t {
    SetGet = 0,
    Has = 1,
    Delete = 2,
    Enumerate = 3,
};

struct PropertyCaseSpec {
    std::string name;
    PropertyOp operation;
    const char* keyName;
    int32_t extraPropCount;
};

struct OperationResult {
    bool passed;
    int32_t value;
};

std::string BuildIndexedName(const char* prefix, size_t caseNumber)
{
    std::string suffix = std::to_string(caseNumber);
    if (suffix.size() < static_cast<size_t>(K_CASE_NUMBER_WIDTH)) {
        suffix.insert(
            0, static_cast<std::string::size_type>(K_CASE_NUMBER_WIDTH - suffix.size()), '0');
    }
    return std::string(prefix) + suffix;
}

size_t GetCaseIndex(void* data)
{
    return static_cast<size_t>(reinterpret_cast<uintptr_t>(data));
}

PropertyCaseSpec GetPropertyCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    const size_t opIndex = caseIndex / K_CASES_PER_OPERATION;
    const size_t keyIndex = caseIndex % K_KEY_NAMES.size();
    const int32_t extraProps = static_cast<int32_t>(caseIndex % K_CASES_PER_OPERATION);
    return {
        BuildIndexedName("propertyCase", caseNumber),
        static_cast<PropertyOp>(opIndex),
        K_KEY_NAMES[keyIndex],
        extraProps,
    };
}

bool ReadString(napi_env env, napi_value value, const char* message, std::string* result)
{
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, value, &type) != napi_ok) {
        return false;
    }
    if (type != napi_string) {
        napi_throw_type_error(env, nullptr, message);
        return false;
    }
    size_t length = 0;
    if (napi_get_value_string_utf8(env, value, nullptr, 0, &length) != napi_ok) {
        return false;
    }
    std::string buffer(length + K_NULL_TERMINATOR_SIZE, '\0');
    if (napi_get_value_string_utf8(env, value, buffer.data(), buffer.size(), &length) != napi_ok) {
        return false;
    }
    buffer.resize(length);
    *result = buffer;
    return true;
}

bool ReadInt32(napi_env env, napi_value value, const char* message, int32_t* result)
{
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, value, &type) != napi_ok) {
        return false;
    }
    if (type != napi_number) {
        napi_throw_type_error(env, nullptr, message);
        return false;
    }
    return napi_get_value_int32(env, value, result) == napi_ok;
}

bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int32(env, value, &napiValue) != napi_ok) {
        return false;
    }
    if (napi_set_named_property(env, object, name, napiValue) != napi_ok) {
        return false;
    }
    napi_value retrieved = nullptr;
    if (napi_get_named_property(env, object, name, &retrieved) != napi_ok) {
        return false;
    }
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, retrieved, &type) != napi_ok || type != napi_number) {
        return false;
    }
    int32_t readBack = K_INVALID_VALUE;
    if (napi_get_value_int32(env, retrieved, &readBack) != napi_ok || readBack != value) {
        return false;
    }
    return true;
}

bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value)
{
    napi_value napiValue = nullptr;
    if (napi_get_boolean(env, value, &napiValue) != napi_ok) {
        return false;
    }
    if (napi_set_named_property(env, object, name, napiValue) != napi_ok) {
        return false;
    }
    napi_value retrieved = nullptr;
    if (napi_get_named_property(env, object, name, &retrieved) != napi_ok) {
        return false;
    }
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, retrieved, &type) != napi_ok || type != napi_boolean) {
        return false;
    }
    bool readBack = false;
    if (napi_get_value_bool(env, retrieved, &readBack) != napi_ok || readBack != value) {
        return false;
    }
    return true;
}

bool SetNamedString(napi_env env, napi_value object, const char* name, const std::string& value)
{
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value.c_str(), value.size(), &napiValue) != napi_ok) {
        return false;
    }
    if (napi_set_named_property(env, object, name, napiValue) != napi_ok) {
        return false;
    }
    napi_value retrieved = nullptr;
    if (napi_get_named_property(env, object, name, &retrieved) != napi_ok) {
        return false;
    }
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, retrieved, &type) != napi_ok || type != napi_string) {
        return false;
    }
    size_t readLen = 0;
    if (napi_get_value_string_utf8(env, retrieved, nullptr, 0, &readLen) != napi_ok || readLen != value.size()) {
        return false;
    }
    return true;
}

napi_value CreatePropertySummary(
    napi_env env, const std::string& name, const char* operation, int32_t retrievedValue, bool passed)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    if (!SetNamedString(env, result, "name", name)) {
        napi_throw_error(env, nullptr, "failed to set name property");
        return nullptr;
    }
    if (!SetNamedString(env, result, "operation", std::string(operation))) {
        napi_throw_error(env, nullptr, "failed to set operation property");
        return nullptr;
    }
    if (!SetNamedInt32(env, result, "value", retrievedValue)) {
        napi_throw_error(env, nullptr, "failed to set value property");
        return nullptr;
    }
    if (!SetNamedBool(env, result, "passed", passed)) {
        napi_throw_error(env, nullptr, "failed to set passed property");
        return nullptr;
    }
    return result;
}

napi_value CreateTestObject(napi_env env)
{
    napi_value obj = nullptr;
    if (napi_create_object(env, &obj) != napi_ok) {
        return nullptr;
    }
    return obj;
}

bool SetPropertyInt32(napi_env env, napi_value obj, const char* key, int32_t val)
{
    napi_value napiVal = nullptr;
    if (napi_create_int32(env, val, &napiVal) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, obj, key, napiVal) == napi_ok;
}

bool ExecuteSetGet(napi_env env, const char* key, int32_t inputVal, int32_t* outVal)
{
    napi_value obj = CreateTestObject(env);
    if (obj == nullptr) {
        return false;
    }
    if (!SetPropertyInt32(env, obj, key, inputVal)) {
        return false;
    }
    napi_value retrieved = nullptr;
    if (napi_get_named_property(env, obj, key, &retrieved) != napi_ok) {
        return false;
    }
    return napi_get_value_int32(env, retrieved, outVal) == napi_ok;
}

bool ExecuteHas(napi_env env, const char* key, int32_t inputVal, bool* outHas)
{
    napi_value obj = CreateTestObject(env);
    if (obj == nullptr) {
        return false;
    }
    if (!SetPropertyInt32(env, obj, key, inputVal)) {
        return false;
    }
    return napi_has_named_property(env, obj, key, outHas) == napi_ok;
}

bool ExecuteDelete(napi_env env, const char* key, int32_t inputVal, bool* outDeleted)
{
    napi_value obj = CreateTestObject(env);
    if (obj == nullptr) {
        return false;
    }
    if (!SetPropertyInt32(env, obj, key, inputVal)) {
        return false;
    }
    napi_value keyStr = nullptr;
    if (napi_create_string_utf8(env, key, NAPI_AUTO_LENGTH, &keyStr) != napi_ok) {
        return false;
    }
    napi_value deleteResult = nullptr;
    if (napi_delete_property(env, obj, keyStr, &deleteResult) != napi_ok) {
        return false;
    }
    return napi_get_value_bool(env, deleteResult, outDeleted) == napi_ok;
}

bool ExecuteEnumerate(
    napi_env env, const char* key, int32_t inputVal, int32_t extraCount, uint32_t* outCount)
{
    napi_value obj = CreateTestObject(env);
    if (obj == nullptr) {
        return false;
    }
    if (!SetPropertyInt32(env, obj, key, inputVal)) {
        return false;
    }
    for (int32_t i = 0; i < extraCount; i++) {
        std::string extraKey = "extra_" + std::to_string(i);
        if (!SetPropertyInt32(env, obj, extraKey.c_str(), inputVal + i)) {
            return false;
        }
    }
    napi_value names = nullptr;
    if (napi_get_property_names(env, obj, &names) != napi_ok) {
        return false;
    }
    return napi_get_array_length(env, names, outCount) == napi_ok;
}

OperationResult ExecuteOperation(napi_env env, const PropertyCaseSpec& spec, int32_t inputVal)
{
    OperationResult result = {false, K_INVALID_VALUE};
    switch (spec.operation) {
        case PropertyOp::SetGet: {
            int32_t val = K_INVALID_VALUE;
            result.passed = ExecuteSetGet(env, spec.keyName, inputVal, &val);
            if (result.passed) {
                result.passed = (val == inputVal);
            }
            result.value = val;
            break;
        }
        case PropertyOp::Has: {
            bool hasProp = false;
            result.passed = ExecuteHas(env, spec.keyName, inputVal, &hasProp);
            if (result.passed) {
                result.passed = hasProp;
            }
            result.value = hasProp ? inputVal : K_INVALID_VALUE;
            break;
        }
        case PropertyOp::Delete: {
            bool deleted = false;
            result.passed = ExecuteDelete(env, spec.keyName, inputVal, &deleted);
            if (result.passed) {
                result.passed = deleted;
            }
            result.value = deleted ? inputVal : K_INVALID_VALUE;
            break;
        }
        case PropertyOp::Enumerate: {
            uint32_t count = 0;
            const int32_t expected = K_BASE_PROPERTY_COUNT + spec.extraPropCount;
            result.passed = ExecuteEnumerate(
                env, spec.keyName, inputVal, spec.extraPropCount, &count);
            if (result.passed) {
                result.passed = (static_cast<int32_t>(count) == expected);
            }
            result.value = static_cast<int32_t>(count);
            break;
        }
    }
    return result;
}

static napi_value RunPropertyCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_PROPERTY_ARG_COUNT;
    napi_value args[K_PROPERTY_ARG_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_PROPERTY_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid property case");
        return nullptr;
    }
    if (argc < K_PROPERTY_ARG_COUNT) {
        napi_throw_type_error(env, nullptr, "key and value are required");
        return nullptr;
    }
    std::string key;
    if (!ReadString(env, args[K_FIRST_ARG_INDEX], "key must be a string", &key)) {
        return nullptr;
    }
    int32_t inputVal = 0;
    if (!ReadInt32(env, args[K_SECOND_ARG_INDEX], "value must be a number", &inputVal)) {
        return nullptr;
    }
    const auto spec = GetPropertyCaseSpec(caseIndex);
    const auto opResult = ExecuteOperation(env, spec, inputVal);
    const char* opLabel = K_OPERATION_LABELS[static_cast<size_t>(spec.operation)];

    return CreatePropertySummary(env, spec.name, opLabel, opResult.value, opResult.passed);
}

}  // namespace

static napi_value InitPropertySuite(napi_env env, napi_value exports)
{
    std::vector<std::string> exportNames;
    std::vector<napi_property_descriptor> descriptors(K_PROPERTY_CASE_COUNT);
    exportNames.reserve(K_PROPERTY_CASE_COUNT);
    for (size_t caseIndex = 0; caseIndex < K_PROPERTY_CASE_COUNT; caseIndex++) {
        exportNames.emplace_back(
            BuildIndexedName("testPropertyCase", caseIndex + K_FIRST_CASE_NUMBER));
        descriptors[caseIndex] = napi_property_descriptor{exportNames.back().c_str(), nullptr,
            RunPropertyCase, nullptr, nullptr, nullptr, napi_default,
            reinterpret_cast<void*>(static_cast<uintptr_t>(caseIndex))};
    }
    NAPI_CALL(env, napi_define_properties(env, exports, descriptors.size(), descriptors.data()));
    return exports;
}

static napi_module g_propertySuiteModule = {
    .nm_version = K_MODULE_VERSION,
    .nm_flags = K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitPropertySuite,
    .nm_modname = "property_suite",
    .nm_priv = nullptr,
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterPropertySuiteModule(void)
{
    napi_module_register(&g_propertySuiteModule);
}
