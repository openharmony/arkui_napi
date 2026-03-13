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

constexpr size_t K_OBJECT_CASE_COUNT = 19;
constexpr size_t K_OBJECT_ARG_COUNT = 1;
constexpr size_t K_FIRST_CASE_NUMBER = 1;
constexpr int K_CASE_NUMBER_WIDTH = 2;
constexpr size_t K_NULL_TERMINATOR_SIZE = 1;
constexpr int32_t K_DELTA_STEP = 3;
constexpr int32_t K_DELTA_OFFSET = 20;
constexpr int32_t K_MULTIPLIER_CYCLE = 4;
constexpr int32_t K_THRESHOLD_OFFSET = 40;
constexpr uint32_t K_MODULE_VERSION = 1;
constexpr uint32_t K_NO_MODULE_FLAGS = 0;

struct ObjectCaseSpec {
    std::string name;
    int32_t delta;
    int32_t multiplier;
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

ObjectCaseSpec GetObjectCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    return {
        BuildIndexedName("objectCase", caseNumber),
        static_cast<int32_t>(caseNumber) * K_DELTA_STEP - K_DELTA_OFFSET,
        static_cast<int32_t>(caseNumber % K_MULTIPLIER_CYCLE) + 1,
        static_cast<int32_t>(caseNumber) + K_THRESHOLD_OFFSET,
    };
}

bool ReadUtf8(napi_env env, napi_value value, const char* message, std::string* result)
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

bool ReadNamedString(napi_env env, napi_value object, const char* name, std::string* result)
{
    napi_value property = nullptr;
    bool hasProperty = false;
    if (napi_has_named_property(env, object, name, &hasProperty) != napi_ok || !hasProperty) {
        napi_throw_error(env, nullptr, "required string property is missing");
        return false;
    }
    if (napi_get_named_property(env, object, name, &property) != napi_ok) {
        return false;
    }
    return ReadUtf8(env, property, "property must be a string", result);
}

bool ReadNamedInt32(napi_env env, napi_value object, const char* name, int32_t* result)
{
    napi_value property = nullptr;
    bool hasProperty = false;
    if (napi_has_named_property(env, object, name, &hasProperty) != napi_ok || !hasProperty) {
        napi_throw_error(env, nullptr, "required number property is missing");
        return false;
    }
    if (napi_get_named_property(env, object, name, &property) != napi_ok) {
        return false;
    }
    if (napi_get_value_int32(env, property, result) != napi_ok) {
        napi_throw_type_error(env, nullptr, "property must be a number");
        return false;
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

napi_value CreateObjectSummary(
    napi_env env, const std::string& name, const std::string& label, int32_t actual, int32_t expected, bool elevated)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", name);
    SetNamedString(env, result, "label", label);
    SetNamedInt32(env, result, "actual", actual);
    SetNamedInt32(env, result, "expected", expected);
    SetNamedBool(env, result, "elevated", elevated);
    SetNamedBool(env, result, "passed", actual == expected);
    return result;
}

static napi_value RunObjectCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_OBJECT_ARG_COUNT;
    napi_value args[K_OBJECT_ARG_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));

    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_OBJECT_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid object case");
        return nullptr;
    }

    std::string label;
    int32_t score = 0;
    if (argc < K_OBJECT_ARG_COUNT) {
        napi_throw_type_error(env, nullptr, "record is required");
        return nullptr;
    }
    if (!ReadNamedString(env, args[0], "label", &label)) {
        return nullptr;
    }
    if (!ReadNamedInt32(env, args[0], "score", &score)) {
        return nullptr;
    }

    const auto spec = GetObjectCaseSpec(caseIndex);
    const int32_t actual = score * spec.multiplier + spec.delta;
    int32_t expected = score;
    for (int32_t step = 0; step < spec.multiplier; step++) {
        expected += score;
    }
    expected -= score;
    expected += spec.delta;

    return CreateObjectSummary(env, spec.name, label, actual, expected, actual >= spec.threshold);
}

}  // namespace

static napi_value InitObjectSuite(napi_env env, napi_value exports)
{
    std::vector<std::string> exportNames;
    std::vector<napi_property_descriptor> descriptors(K_OBJECT_CASE_COUNT);
    exportNames.reserve(K_OBJECT_CASE_COUNT);
    for (size_t caseIndex = 0; caseIndex < K_OBJECT_CASE_COUNT; caseIndex++) {
        exportNames.emplace_back(BuildIndexedName("testObjectCase", caseIndex + K_FIRST_CASE_NUMBER));
        descriptors[caseIndex] = napi_property_descriptor{exportNames.back().c_str(), nullptr, RunObjectCase, nullptr,
            nullptr, nullptr, napi_default, reinterpret_cast<void*>(static_cast<uintptr_t>(caseIndex))};
    }
    NAPI_CALL(env, napi_define_properties(env, exports, descriptors.size(), descriptors.data()));
    return exports;
}

static napi_module g_objectSuiteModule = {
    .nm_version = K_MODULE_VERSION,
    .nm_flags = K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitObjectSuite,
    .nm_modname = "object_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterObjectSuiteModule(void)
{
    napi_module_register(&g_objectSuiteModule);
}
