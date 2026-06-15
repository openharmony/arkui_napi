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

constexpr size_t K_DEFINE_CASE_COUNT = 20;
constexpr size_t K_DEFINE_ARG_COUNT = 1;
constexpr size_t K_FIRST_CASE_NUMBER = 1;
constexpr int K_CASE_NUMBER_WIDTH = 2;
constexpr size_t K_PROPERTY_COUNT = 3;
constexpr int32_t K_VALUE_BASE = 10;
constexpr int32_t K_VALUE_STEP = 3;
constexpr int32_t K_VALUE_CYCLE = 7;
constexpr int32_t K_WEIGHT_WRITABLE = 100;
constexpr int32_t K_WEIGHT_ENUMERABLE = 10;
constexpr int32_t K_WEIGHT_CONFIGURABLE = 1;
constexpr int32_t K_ATTRIBUTE_BIT_SHIFT = 2;
constexpr uint32_t K_MODULE_VERSION = 1;
constexpr uint32_t K_NO_MODULE_FLAGS = 0;

struct DefineCaseSpec {
    std::string name;
    int32_t valueFactor;
    bool writable;
    bool enumerable;
    bool configurable;
};

struct PropertyMeta {
    std::string name;
    bool writable;
    bool enumerable;
    bool configurable;
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

DefineCaseSpec GetDefineCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    const int32_t attrBits = static_cast<int32_t>(caseNumber >> K_ATTRIBUTE_BIT_SHIFT);
    return {
        BuildIndexedName("defineCase", caseNumber),
        static_cast<int32_t>(caseNumber % K_VALUE_CYCLE) * K_VALUE_STEP + K_VALUE_BASE,
        (attrBits & 1) != 0,
        (attrBits & 2) != 0,
        (attrBits & 4) != 0,
    };
}

std::string BuildDefineExportName(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    return BuildIndexedName("testDefineCase", caseNumber);
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

PropertyMeta GetPropertyMeta(size_t propIndex, const DefineCaseSpec& spec)
{
    // Rotate attributes so each property gets a different combination
    const bool props[K_PROPERTY_COUNT] = {spec.writable, spec.enumerable, spec.configurable};
    return {
        BuildIndexedName("prop", propIndex + K_FIRST_CASE_NUMBER),
        props[propIndex % K_PROPERTY_COUNT],
        props[(propIndex + 1) % K_PROPERTY_COUNT],
        props[(propIndex + 2) % K_PROPERTY_COUNT],
    };
}

uint32_t BuildPropertyAttribute(const PropertyMeta& meta)
{
    uint32_t attr = napi_default;
    if (!meta.writable) {
        attr |= napi_writable;
    }
    if (!meta.enumerable) {
        attr |= napi_enumerable;
    }
    if (!meta.configurable) {
        attr |= napi_configurable;
    }
    return attr;
}

bool DefinePropertiesOnObject(
    napi_env env, napi_value target, const DefineCaseSpec& spec, int32_t* resultValues)
{
    napi_property_descriptor descriptors[K_PROPERTY_COUNT] = {};
    for (size_t propIndex = 0; propIndex < K_PROPERTY_COUNT; propIndex++) {
        const auto meta = GetPropertyMeta(propIndex, spec);
        napi_value propValue = nullptr;
        const int32_t numericValue = spec.valueFactor *
            static_cast<int32_t>(propIndex + K_FIRST_CASE_NUMBER);
        if (napi_create_int32(env, numericValue, &propValue) != napi_ok) {
            return false;
        }
        resultValues[propIndex] = numericValue;
        descriptors[propIndex] = napi_property_descriptor{
            meta.name.c_str(),
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            propValue,
            BuildPropertyAttribute(meta),
            nullptr,
        };
    }
    return napi_define_properties(env, target, K_PROPERTY_COUNT, descriptors) == napi_ok;
}

int32_t ComputeAttributeScore(const DefineCaseSpec& spec)
{
    return (spec.writable ? K_WEIGHT_WRITABLE : 0) +
           (spec.enumerable ? K_WEIGHT_ENUMERABLE : 0) +
           (spec.configurable ? K_WEIGHT_CONFIGURABLE : 0);
}

napi_value CreateDefineResultObject(
    napi_env env, const DefineCaseSpec& spec, const int32_t* propValues)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", spec.name);
    SetNamedInt32(env, result, "valueFactor", spec.valueFactor);
    SetNamedBool(env, result, "writable", spec.writable);
    SetNamedBool(env, result, "enumerable", spec.enumerable);
    SetNamedBool(env, result, "configurable", spec.configurable);
    SetNamedInt32(env, result, "attributeScore", ComputeAttributeScore(spec));
    for (size_t propIndex = 0; propIndex < K_PROPERTY_COUNT; propIndex++) {
        const auto meta = GetPropertyMeta(propIndex, spec);
        napi_value propResults = nullptr;
        NAPI_CALL(env, napi_create_object(env, &propResults));
        SetNamedInt32(env, propResults, "value", propValues[propIndex]);
        SetNamedBool(env, propResults, "writable", meta.writable);
        SetNamedBool(env, propResults, "enumerable", meta.enumerable);
        SetNamedBool(env, propResults, "configurable", meta.configurable);
        napi_set_named_property(env, result, meta.name.c_str(), propResults);
    }
    return result;
}

static napi_value RunDefineCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_DEFINE_ARG_COUNT;
    napi_value args[K_DEFINE_ARG_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));

    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_DEFINE_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid define case");
        return nullptr;
    }
    if (argc < K_DEFINE_ARG_COUNT) {
        napi_throw_type_error(env, nullptr, "target object is required");
        return nullptr;
    }

    const auto spec = GetDefineCaseSpec(caseIndex);
    int32_t propValues[K_PROPERTY_COUNT] = {0};
    if (!DefinePropertiesOnObject(env, args[0], spec, propValues)) {
        return nullptr;
    }
    return CreateDefineResultObject(env, spec, propValues);
}

}  // namespace

static napi_value InitDefineSuite(napi_env env, napi_value exports)
{
    std::vector<std::string> exportNames;
    std::vector<napi_property_descriptor> descriptors(K_DEFINE_CASE_COUNT);
    exportNames.reserve(K_DEFINE_CASE_COUNT);
    for (size_t caseIndex = 0; caseIndex < K_DEFINE_CASE_COUNT; caseIndex++) {
        exportNames.emplace_back(BuildDefineExportName(caseIndex));
        descriptors[caseIndex] = napi_property_descriptor{
            exportNames.back().c_str(),
            nullptr,
            RunDefineCase,
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

static napi_module g_defineSuiteModule = {
    .nm_version = K_MODULE_VERSION,
    .nm_flags = K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitDefineSuite,
    .nm_modname = "define_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterDefineSuiteModule(void)
{
    napi_module_register(&g_defineSuiteModule);
}
