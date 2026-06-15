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

constexpr size_t K_EQUALITY_CASE_COUNT = 20;
constexpr size_t K_EQUALITY_ARG_COUNT = 2;
constexpr size_t K_FIRST_CASE_NUMBER = 1;
constexpr int K_CASE_NUMBER_WIDTH = 2;
constexpr size_t K_FIRST_ARG_INDEX = 0;
constexpr size_t K_SECOND_ARG_INDEX = 1;
constexpr int32_t K_MULTIPLIER_BASE = 2;
constexpr int32_t K_MULTIPLIER_CYCLE = 5;
constexpr int32_t K_DELTA_CYCLE = 7;
constexpr int32_t K_DELTA_OFFSET = 3;
constexpr int32_t K_CHECKSUM_CYCLE = 4;
constexpr uint32_t K_MODULE_VERSION = 1;
constexpr uint32_t K_NO_MODULE_FLAGS = 0;

struct EqualityCaseSpec {
    std::string name;
    int32_t multiplier;
    int32_t delta;
};

struct EqualityContext {
    int32_t firstValue;
    int32_t secondValue;
    int32_t computedValue;
    bool valuesEqual;
    bool computedMatchesFirst;
    bool computedMatchesSecond;
    const char* firstType;
    const char* secondType;
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

EqualityCaseSpec GetEqualityCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    return {
        BuildIndexedName("equalityCase", caseNumber),
        static_cast<int32_t>(caseNumber % K_MULTIPLIER_CYCLE) + K_MULTIPLIER_BASE,
        static_cast<int32_t>(caseNumber % K_DELTA_CYCLE) + K_DELTA_OFFSET,
    };
}

std::string BuildEqualityExportName(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    return BuildIndexedName("testEqualityCase", caseNumber);
}

bool FetchValueType(napi_env env, napi_value value, const char** typeLabel)
{
    napi_valuetype napiType = napi_undefined;
    if (napi_typeof(env, value, &napiType) != napi_ok) {
        return false;
    }
    switch (napiType) {
        case napi_number:
            *typeLabel = "number";
            break;
        case napi_string:
            *typeLabel = "string";
            break;
        case napi_boolean:
            *typeLabel = "boolean";
            break;
        case napi_object:
            *typeLabel = "object";
            break;
        case napi_undefined:
            *typeLabel = "undefined";
            break;
        case napi_null:
            *typeLabel = "null";
            break;
        case napi_function:
            *typeLabel = "function";
            break;
        default:
            *typeLabel = "unknown";
            break;
    }
    return true;
}

bool ReadTwoInt32(
    napi_env env, napi_value first, napi_value second, EqualityContext* eqCtx)
{
    if (!FetchValueType(env, first, &eqCtx->firstType)) {
        return false;
    }
    if (!FetchValueType(env, second, &eqCtx->secondType)) {
        return false;
    }
    napi_status firstStatus = napi_get_value_int32(env, first, &eqCtx->firstValue);
    napi_status secondStatus = napi_get_value_int32(env, second, &eqCtx->secondValue);
    if (firstStatus != napi_ok || secondStatus != napi_ok) {
        napi_throw_type_error(env, nullptr, "both arguments must be numbers");
        return false;
    }
    return true;
}

bool CheckStrictEquality(napi_env env, napi_value a, napi_value b, bool* result)
{
    return napi_strict_equals(env, a, b, result) == napi_ok;
}

bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int32(env, value, &napiValue) != napi_ok) {
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

bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value)
{
    napi_value napiValue = nullptr;
    if (napi_get_boolean(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

napi_value CreateEqualityResultObject(napi_env env, const EqualityCaseSpec& spec,
    const EqualityContext& eqCtx)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", spec.name);
    SetNamedInt32(env, result, "firstValue", eqCtx.firstValue);
    SetNamedInt32(env, result, "secondValue", eqCtx.secondValue);
    SetNamedInt32(env, result, "computedValue", eqCtx.computedValue);
    SetNamedBool(env, result, "valuesEqual", eqCtx.valuesEqual);
    SetNamedBool(env, result, "computedMatchesFirst", eqCtx.computedMatchesFirst);
    SetNamedBool(env, result, "computedMatchesSecond", eqCtx.computedMatchesSecond);
    SetNamedString(env, result, "firstType", eqCtx.firstType);
    SetNamedString(env, result, "secondType", eqCtx.secondType);
    SetNamedInt32(env, result, "checksum",
        (eqCtx.firstValue + eqCtx.secondValue) % K_CHECKSUM_CYCLE);
    return result;
}

static napi_value RunEqualityCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_EQUALITY_ARG_COUNT;
    napi_value args[K_EQUALITY_ARG_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));

    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_EQUALITY_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid equality case");
        return nullptr;
    }
    if (argc < K_EQUALITY_ARG_COUNT) {
        napi_throw_type_error(env, nullptr, "two arguments are required");
        return nullptr;
    }

    EqualityContext eqCtx = {};
    if (!ReadTwoInt32(env, args[K_FIRST_ARG_INDEX], args[K_SECOND_ARG_INDEX], &eqCtx)) {
        return nullptr;
    }

    const auto spec = GetEqualityCaseSpec(caseIndex);
    CheckStrictEquality(env, args[K_FIRST_ARG_INDEX], args[K_SECOND_ARG_INDEX],
        &eqCtx.valuesEqual);

    eqCtx.computedValue = eqCtx.firstValue * spec.multiplier + spec.delta;
    napi_value computedNapi = nullptr;
    NAPI_CALL(env, napi_create_int32(env, eqCtx.computedValue, &computedNapi));
    CheckStrictEquality(env, args[K_FIRST_ARG_INDEX], computedNapi,
        &eqCtx.computedMatchesFirst);
    CheckStrictEquality(env, args[K_SECOND_ARG_INDEX], computedNapi,
        &eqCtx.computedMatchesSecond);

    return CreateEqualityResultObject(env, spec, eqCtx);
}

}  // namespace

static napi_value InitEqualitySuite(napi_env env, napi_value exports)
{
    std::vector<std::string> exportNames;
    std::vector<napi_property_descriptor> descriptors(K_EQUALITY_CASE_COUNT);
    exportNames.reserve(K_EQUALITY_CASE_COUNT);
    for (size_t caseIndex = 0; caseIndex < K_EQUALITY_CASE_COUNT; caseIndex++) {
        exportNames.emplace_back(BuildEqualityExportName(caseIndex));
        descriptors[caseIndex] = napi_property_descriptor{
            exportNames.back().c_str(),
            nullptr,
            RunEqualityCase,
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

static napi_module g_equalitySuiteModule = {
    .nm_version = K_MODULE_VERSION,
    .nm_flags = K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitEqualitySuite,
    .nm_modname = "equality_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterEqualitySuiteModule(void)
{
    napi_module_register(&g_equalitySuiteModule);
}
