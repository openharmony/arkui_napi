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
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

constexpr size_t K_ARRAY_CASE_COUNT = 20;
constexpr size_t K_ARRAY_ARG_COUNT = 1;
constexpr size_t K_FIRST_CASE_NUMBER = 1;
constexpr int K_CASE_NUMBER_WIDTH = 2;
constexpr int32_t K_WEIGHT_CYCLE = 5;
constexpr int32_t K_OFFSET_BASE = 10;
constexpr int32_t K_THRESHOLD_STEP = 8;
constexpr int32_t K_EVEN_VALUE_MASK = 1;
constexpr uint32_t K_MODULE_VERSION = 1;
constexpr uint32_t K_NO_MODULE_FLAGS = 0;

struct ArrayCaseSpec {
    std::string name;
    int32_t weight;
    int32_t offset;
    int32_t threshold;
};

std::string BuildIndexedName(const char* prefix, size_t caseNumber)
{
    std::string suffix = std::to_string(caseNumber);
    if (suffix.size() < static_cast<size_t>(K_CASE_NUMBER_WIDTH)) {
        suffix.insert(0, static_cast<std::string::size_type>(K_CASE_NUMBER_WIDTH - suffix.size()), '0');
    }
    return std::string(prefix) + suffix;
}

size_t GetCaseIndex(void* data) { return static_cast<size_t>(reinterpret_cast<uintptr_t>(data)); }

ArrayCaseSpec GetArrayCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    return {
        BuildIndexedName("arrayCase", caseNumber),
        static_cast<int32_t>(caseNumber % K_WEIGHT_CYCLE) + 1,
        static_cast<int32_t>(caseNumber) - K_OFFSET_BASE,
        static_cast<int32_t>(caseNumber) * K_THRESHOLD_STEP,
    };
}

bool ReadInt32(napi_env env, napi_value value, int32_t* result)
{
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, value, &type) != napi_ok) {
        return false;
    }
    if (type != napi_number) {
        return false;
    }
    return napi_get_value_int32(env, value, result) == napi_ok;
}

bool ReadInt32Array(napi_env env, napi_value value, std::vector<int32_t>* result)
{
    bool isArray = false;
    if (napi_is_array(env, value, &isArray) != napi_ok || !isArray) {
        napi_throw_type_error(env, nullptr, "value must be an array");
        return false;
    }

    uint32_t length = 0;
    if (napi_get_array_length(env, value, &length) != napi_ok) {
        return false;
    }

    result->clear();
    result->reserve(length);
    for (uint32_t index = 0; index < length; index++) {
        napi_value element = nullptr;
        int32_t current = 0;
        if (napi_get_element(env, value, index, &element) != napi_ok || !ReadInt32(env, element, &current)) {
            napi_throw_type_error(env, nullptr, "array elements must be numbers");
            return false;
        }
        result->push_back(current);
    }
    return true;
}

bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int32(env, value, &napiValue) != napi_ok) {
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

bool SetNamedString(napi_env env, napi_value object, const char* name, const std::string& value)
{
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value.c_str(), value.size(), &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

napi_value CreateArraySummary(napi_env env, const std::string& name, int32_t length, int32_t weightedSum,
    int32_t expected, int32_t evenCount, bool thresholdPassed)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", name);
    SetNamedInt32(env, result, "length", length);
    SetNamedInt32(env, result, "weightedSum", weightedSum);
    SetNamedInt32(env, result, "expected", expected);
    SetNamedInt32(env, result, "evenCount", evenCount);
    SetNamedBool(env, result, "thresholdPassed", thresholdPassed);
    SetNamedBool(env, result, "passed", weightedSum == expected);
    return result;
}

static napi_value RunArrayCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_ARRAY_ARG_COUNT;
    napi_value args[K_ARRAY_ARG_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));

    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_ARRAY_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid array case");
        return nullptr;
    }

    std::vector<int32_t> values;
    if (argc < K_ARRAY_ARG_COUNT) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto spec = GetArrayCaseSpec(caseIndex);
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & K_EVEN_VALUE_MASK) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

}  // namespace

static napi_value InitArraySuite(napi_env env, napi_value exports)
{
    std::vector<std::string> exportNames;
    std::vector<napi_property_descriptor> descriptors(K_ARRAY_CASE_COUNT);
    exportNames.reserve(K_ARRAY_CASE_COUNT);
    for (size_t caseIndex = 0; caseIndex < K_ARRAY_CASE_COUNT; caseIndex++) {
        exportNames.emplace_back(BuildIndexedName("testArrayCase", caseIndex + K_FIRST_CASE_NUMBER));
        descriptors[caseIndex] = napi_property_descriptor{exportNames.back().c_str(), nullptr, RunArrayCase, nullptr,
            nullptr, nullptr, napi_default, reinterpret_cast<void*>(static_cast<uintptr_t>(caseIndex))};
    }
    NAPI_CALL(env, napi_define_properties(env, exports, descriptors.size(), descriptors.data()));
    return exports;
}

static napi_module g_arraySuiteModule = {
    .nm_version = K_MODULE_VERSION,
    .nm_flags = K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitArraySuite,
    .nm_modname = "array_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterArraySuiteModule(void)
{
    napi_module_register(&g_arraySuiteModule);
}
