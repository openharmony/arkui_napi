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

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

constexpr size_t K_CLAMP_CASE_COUNT = 20;
constexpr size_t K_CLAMP_ARG_COUNT = 1;
constexpr size_t K_FIRST_CASE_NUMBER = 1;
constexpr int K_CASE_NUMBER_WIDTH = 2;
constexpr int32_t K_SHIFT_CYCLE = 7;
constexpr int32_t K_SHIFT_FACTOR = 5;
constexpr int32_t K_SHIFT_OFFSET = -15;
constexpr int32_t K_BOUND_STEP = 8;
constexpr int32_t K_BOUND_BASE = 50;
constexpr int32_t K_BOUND_SHRINK_STEP = 4;
constexpr uint32_t K_MODULE_VERSION = 1;
constexpr uint32_t K_NO_MODULE_FLAGS = 0;

struct ClampCaseSpec {
    std::string name;
    int32_t minBound;
    int32_t maxBound;
    int32_t shift;
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

ClampCaseSpec GetClampCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    const int32_t shiftValue = static_cast<int32_t>(caseNumber % K_SHIFT_CYCLE) *
        K_SHIFT_FACTOR + K_SHIFT_OFFSET;
    const int32_t minVal = -(K_BOUND_BASE +
        static_cast<int32_t>(caseNumber) * K_BOUND_STEP);
    const int32_t maxVal = K_BOUND_BASE +
        static_cast<int32_t>(K_CLAMP_CASE_COUNT - caseNumber) * K_BOUND_SHRINK_STEP;
    return {
        BuildIndexedName("clampCase", caseNumber),
        std::min(minVal, maxVal),
        std::max(minVal, maxVal),
        shiftValue,
    };
}

std::string BuildClampExportName(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    return BuildIndexedName("testClampCase", caseNumber);
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

int32_t ComputeShiftedValue(int32_t input, int32_t shift)
{
    const int64_t wide = static_cast<int64_t>(input) + static_cast<int64_t>(shift);
    if (wide > static_cast<int64_t>(INT32_MAX)) {
        return INT32_MAX;
    }
    if (wide < static_cast<int64_t>(INT32_MIN)) {
        return INT32_MIN;
    }
    return static_cast<int32_t>(wide);
}

int32_t ClampToBounds(int32_t value, int32_t minBound, int32_t maxBound)
{
    if (value < minBound) {
        return minBound;
    }
    if (value > maxBound) {
        return maxBound;
    }
    return value;
}

napi_value CreateClampResultObject(
    napi_env env, const ClampCaseSpec& spec, int32_t input,
    int32_t shifted, int32_t clamped)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", spec.name);
    SetNamedInt32(env, result, "original", input);
    SetNamedInt32(env, result, "shifted", shifted);
    SetNamedInt32(env, result, "clamped", clamped);
    SetNamedInt32(env, result, "minBound", spec.minBound);
    SetNamedInt32(env, result, "maxBound", spec.maxBound);
    SetNamedInt32(env, result, "shift", spec.shift);
    SetNamedBool(env, result, "wasClamped", (clamped != shifted));
    SetNamedBool(env, result, "wasOverflow", (shifted != input + spec.shift));
    return result;
}

static napi_value RunClampCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_CLAMP_ARG_COUNT;
    napi_value args[K_CLAMP_ARG_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));

    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_CLAMP_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid clamp case");
        return nullptr;
    }
    if (argc < K_CLAMP_ARG_COUNT) {
        napi_throw_type_error(env, nullptr, "input number is required");
        return nullptr;
    }

    int32_t input = 0;
    if (!ReadInt32(env, args[0], "input must be a number", &input)) {
        return nullptr;
    }

    const auto spec = GetClampCaseSpec(caseIndex);
    const int32_t shifted = ComputeShiftedValue(input, spec.shift);
    const int32_t clamped = ClampToBounds(shifted, spec.minBound, spec.maxBound);
    return CreateClampResultObject(env, spec, input, shifted, clamped);
}

}  // namespace

static napi_value InitClampSuite(napi_env env, napi_value exports)
{
    std::vector<std::string> exportNames;
    std::vector<napi_property_descriptor> descriptors(K_CLAMP_CASE_COUNT);
    exportNames.reserve(K_CLAMP_CASE_COUNT);
    for (size_t caseIndex = 0; caseIndex < K_CLAMP_CASE_COUNT; caseIndex++) {
        exportNames.emplace_back(BuildClampExportName(caseIndex));
        descriptors[caseIndex] = napi_property_descriptor{
            exportNames.back().c_str(),
            nullptr,
            RunClampCase,
            nullptr,
            nullptr,
            nullptr,
            napi_default,
            reinterpret_cast<void*>(static_cast<uintptr_t>(caseIndex)),
        };
    }
    NAPI_CALL(
        env, napi_define_properties(env, exports, descriptors.size(), descriptors.data()));
    return exports;
}

static napi_module g_clampSuiteModule = {
    .nm_version = K_MODULE_VERSION,
    .nm_flags = K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitClampSuite,
    .nm_modname = "clamp_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterClampSuiteModule(void)
{
    napi_module_register(&g_clampSuiteModule);
}
