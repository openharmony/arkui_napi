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

constexpr size_t K_BOOLEAN_CASE_COUNT = 20;
constexpr size_t K_BOOLEAN_ARG_COUNT = 3;
constexpr size_t K_FIRST_CASE_NUMBER = 1;
constexpr size_t K_FIRST_BOOLEAN_ARG_INDEX = 0;
constexpr size_t K_SECOND_BOOLEAN_ARG_INDEX = 1;
constexpr size_t K_THIRD_BOOLEAN_ARG_INDEX = 2;
constexpr int K_CASE_NUMBER_WIDTH = 2;
constexpr int32_t K_FIRST_BOOL_WEIGHT = 4;
constexpr int32_t K_SECOND_BOOL_WEIGHT = 2;
constexpr int32_t K_THIRD_BOOL_WEIGHT = 1;
constexpr int32_t K_PASS_THRESHOLD = 3;
constexpr uint32_t K_MODULE_VERSION = 1;
constexpr uint32_t K_NO_MODULE_FLAGS = 0;

constexpr std::array<const char*, 4> K_ROUTE_NAMES = {
    "beta",
    "gamma",
    "delta",
    "alpha",
};

struct BooleanCaseSpec {
    std::string name;
    bool invertFirst;
    bool invertSecond;
    bool invertThird;
    const char* route;
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

BooleanCaseSpec GetBooleanCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    return {
        BuildIndexedName("booleanCase", caseNumber),
        caseIndex % 2 == 0,
        caseIndex % 3 == 2,
        caseIndex % K_ROUTE_NAMES.size() == K_ROUTE_NAMES.size() - 1,
        K_ROUTE_NAMES[caseIndex % K_ROUTE_NAMES.size()],
    };
}

bool ReadBool(napi_env env, napi_value value, const char* message, bool* result)
{
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, value, &type) != napi_ok) {
        return false;
    }
    if (type != napi_boolean) {
        napi_throw_type_error(env, nullptr, message);
        return false;
    }
    return napi_get_value_bool(env, value, result) == napi_ok;
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

bool SetNamedString(napi_env env, napi_value object, const char* name, const char* value)
{
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

napi_value CreateBooleanSummary(napi_env env, const char* name, const char* route, int32_t score, bool passed)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", name);
    SetNamedString(env, result, "route", route);
    SetNamedInt32(env, result, "score", score);
    SetNamedBool(env, result, "passed", passed);
    return result;
}

static napi_value RunBooleanCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_BOOLEAN_ARG_COUNT;
    napi_value args[K_BOOLEAN_ARG_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));

    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_BOOLEAN_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid boolean case");
        return nullptr;
    }

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < K_BOOLEAN_ARG_COUNT) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[K_FIRST_BOOLEAN_ARG_INDEX], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[K_SECOND_BOOLEAN_ARG_INDEX], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[K_THIRD_BOOLEAN_ARG_INDEX], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto spec = GetBooleanCaseSpec(caseIndex);
    const bool normalizedFirst = spec.invertFirst ? !first : first;
    const bool normalizedSecond = spec.invertSecond ? !second : second;
    const bool normalizedThird = spec.invertThird ? !third : third;
    const int32_t score = static_cast<int32_t>(normalizedFirst) * K_FIRST_BOOL_WEIGHT +
                          static_cast<int32_t>(normalizedSecond) * K_SECOND_BOOL_WEIGHT +
                          static_cast<int32_t>(normalizedThird) * K_THIRD_BOOL_WEIGHT;

    return CreateBooleanSummary(env, spec.name.c_str(), spec.route, score, score >= K_PASS_THRESHOLD);
}

}  // namespace

static napi_value InitBooleanSuite(napi_env env, napi_value exports)
{
    std::vector<std::string> exportNames;
    std::vector<napi_property_descriptor> descriptors(K_BOOLEAN_CASE_COUNT);
    exportNames.reserve(K_BOOLEAN_CASE_COUNT);
    for (size_t caseIndex = 0; caseIndex < K_BOOLEAN_CASE_COUNT; caseIndex++) {
        exportNames.emplace_back(BuildIndexedName("testBooleanCase", caseIndex + K_FIRST_CASE_NUMBER));
        descriptors[caseIndex] = napi_property_descriptor{exportNames.back().c_str(), nullptr, RunBooleanCase, nullptr,
            nullptr, nullptr, napi_default, reinterpret_cast<void*>(static_cast<uintptr_t>(caseIndex))};
    }
    NAPI_CALL(env, napi_define_properties(env, exports, descriptors.size(), descriptors.data()));
    return exports;
}

static napi_module g_booleanSuiteModule = {
    .nm_version = K_MODULE_VERSION,
    .nm_flags = K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitBooleanSuite,
    .nm_modname = "boolean_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterBooleanSuiteModule(void)
{
    napi_module_register(&g_booleanSuiteModule);
}
