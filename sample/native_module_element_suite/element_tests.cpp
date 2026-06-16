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

constexpr size_t K_ELEMENT_CASE_COUNT = 20;
constexpr size_t K_ELEMENT_ARG_COUNT = 1;
constexpr size_t K_FIRST_CASE_NUMBER = 1;
constexpr int K_CASE_NUMBER_WIDTH = 2;
constexpr int32_t K_MULTIPLIER_BASE = 3;
constexpr int32_t K_DELTA_CYCLE = 5;
constexpr int32_t K_DELTA_STEP = 2;
constexpr int32_t K_TRANSFORM_SHIFT = 7;
constexpr uint32_t K_MODULE_VERSION = 1;
constexpr uint32_t K_NO_MODULE_FLAGS = 0;

struct ElementCaseSpec {
    std::string name;
    int32_t multiplier;
    int32_t delta;
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

ElementCaseSpec GetElementCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    return {
        BuildIndexedName("elementCase", caseNumber),
        static_cast<int32_t>(caseNumber % K_DELTA_CYCLE) + K_MULTIPLIER_BASE,
        static_cast<int32_t>(caseNumber) * K_DELTA_STEP,
    };
}

std::string BuildElementExportName(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    return BuildIndexedName("testElementCase", caseNumber);
}

bool ReadArrayElements(napi_env env, napi_value array, std::vector<int32_t>* elements)
{
    bool isArray = false;
    if (napi_is_array(env, array, &isArray) != napi_ok || !isArray) {
        napi_throw_type_error(env, nullptr, "value must be an array");
        return false;
    }
    uint32_t length = 0;
    if (napi_get_array_length(env, array, &length) != napi_ok) {
        return false;
    }
    elements->reserve(length);
    for (uint32_t index = 0; index < length; index++) {
        napi_value element = nullptr;
        if (napi_get_element(env, array, index, &element) != napi_ok) {
            return false;
        }
        int32_t value = 0;
        if (napi_get_value_int32(env, element, &value) != napi_ok) {
            return false;
        }
        elements->push_back(value);
    }
    return true;
}

bool WriteArrayElements(napi_env env, napi_value array, const std::vector<int32_t>& elements)
{
    for (size_t index = 0; index < elements.size(); index++) {
        napi_value element = nullptr;
        if (napi_create_int32(env, elements[index], &element) != napi_ok) {
            return false;
        }
        if (napi_set_element(env, array, static_cast<uint32_t>(index), element) != napi_ok) {
            return false;
        }
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

int32_t ComputeChecksum(const std::vector<int32_t>& elements, int32_t shift)
{
    int32_t checksum = 0;
    for (size_t index = 0; index < elements.size(); index++) {
        checksum += (elements[index] + static_cast<int32_t>(index) * shift);
    }
    return checksum;
}

napi_value CreateElementResultObject(
    napi_env env, const ElementCaseSpec& spec, const std::vector<int32_t>& original,
    const std::vector<int32_t>& transformed, napi_value outputArray)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", spec.name);
    SetNamedInt32(env, result, "originalCount",
        static_cast<int32_t>(original.size()));
    SetNamedInt32(env, result, "originalChecksum",
        ComputeChecksum(original, K_TRANSFORM_SHIFT));
    SetNamedInt32(env, result, "outputChecksum",
        ComputeChecksum(transformed, K_TRANSFORM_SHIFT));
    SetNamedBool(env, result, "isArray",
        static_cast<bool>(outputArray != nullptr));
    SetNamedInt32(env, result, "multiplier", spec.multiplier);
    SetNamedInt32(env, result, "delta", spec.delta);
    return result;
}

static napi_value RunElementCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_ELEMENT_ARG_COUNT;
    napi_value args[K_ELEMENT_ARG_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));

    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_ELEMENT_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid element case");
        return nullptr;
    }
    if (argc < K_ELEMENT_ARG_COUNT) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }

    std::vector<int32_t> original;
    if (!ReadArrayElements(env, args[0], &original)) {
        return nullptr;
    }

    const auto spec = GetElementCaseSpec(caseIndex);
    std::vector<int32_t> transformed;
    transformed.reserve(original.size());
    for (size_t index = 0; index < original.size(); index++) {
        transformed.push_back(original[index] * spec.multiplier + spec.delta);
    }

    napi_value outputArray = nullptr;
    NAPI_CALL(env, napi_create_array(env, &outputArray));
    if (WriteArrayElements(env, outputArray, transformed)) {
        return CreateElementResultObject(env, spec, original, transformed, outputArray);
    }
    return CreateElementResultObject(env, spec, original, transformed, nullptr);
}

}  // namespace

static napi_value InitElementSuite(napi_env env, napi_value exports)
{
    std::vector<std::string> exportNames;
    std::vector<napi_property_descriptor> descriptors(K_ELEMENT_CASE_COUNT);
    exportNames.reserve(K_ELEMENT_CASE_COUNT);
    for (size_t caseIndex = 0; caseIndex < K_ELEMENT_CASE_COUNT; caseIndex++) {
        exportNames.emplace_back(BuildElementExportName(caseIndex));
        descriptors[caseIndex] = napi_property_descriptor{
            exportNames.back().c_str(),
            nullptr,
            RunElementCase,
            nullptr,
            nullptr,
            nullptr,
            napi_default,
            reinterpret_cast<void*>(static_cast<uintptr_t>(caseIndex)),
        };
    }
    NAPI_CALL(env, napi_define_properties(env, exports, descriptors.size(), descriptors.data()));
    return exports;
}

static napi_module g_elementSuiteModule = {
    .nm_version = K_MODULE_VERSION,
    .nm_flags = K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitElementSuite,
    .nm_modname = "element_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterElementSuiteModule(void)
{
    napi_module_register(&g_elementSuiteModule);
}
