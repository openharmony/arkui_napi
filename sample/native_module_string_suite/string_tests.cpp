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

constexpr size_t K_STRING_CASE_COUNT = 21;
constexpr size_t K_STRING_ARG_COUNT = 1;
constexpr size_t K_FIRST_CASE_NUMBER = 1;
constexpr int K_CASE_NUMBER_WIDTH = 2;
constexpr size_t K_NULL_TERMINATOR_SIZE = 1;
constexpr size_t K_MARKER_CYCLE = 7;
constexpr size_t K_REPEAT_CYCLE = 4;
constexpr int32_t K_REPEAT_BASE = 1;
constexpr int32_t K_JOINER_LENGTH = 1;
constexpr uint32_t K_MODULE_VERSION = 1;
constexpr uint32_t K_NO_MODULE_FLAGS = 0;

struct StringCaseSpec {
    std::string name;
    std::string prefix;
    std::string suffix;
    std::string marker;
    int32_t repeat;
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

StringCaseSpec GetStringCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    return {
        BuildIndexedName("stringCase", caseNumber),
        BuildIndexedName("pre", caseNumber),
        BuildIndexedName("suf", caseNumber),
        std::string("M") + std::to_string(caseNumber % K_MARKER_CYCLE),
        static_cast<int32_t>(caseNumber % K_REPEAT_CYCLE) + K_REPEAT_BASE,
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

napi_value CreateStringSummary(napi_env env, const std::string& name, const std::string& text, int32_t measuredLength,
    int32_t expectedLength, bool containsMarker)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", name);
    SetNamedString(env, result, "text", text);
    SetNamedInt32(env, result, "measuredLength", measuredLength);
    SetNamedInt32(env, result, "expectedLength", expectedLength);
    SetNamedBool(env, result, "containsMarker", containsMarker);
    SetNamedBool(env, result, "passed", measuredLength == expectedLength && containsMarker);
    return result;
}

static napi_value RunStringCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_STRING_ARG_COUNT;
    napi_value args[K_STRING_ARG_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));

    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_STRING_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid string case");
        return nullptr;
    }

    std::string source;
    if (argc < K_STRING_ARG_COUNT) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto spec = GetStringCaseSpec(caseIndex);
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    const bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    const int32_t measuredLength = static_cast<int32_t>(transformed.size());
    const int32_t expectedLength =
        static_cast<int32_t>(spec.prefix.size() + source.size() + K_JOINER_LENGTH + spec.suffix.size() +
                             spec.repeat * (K_JOINER_LENGTH + static_cast<int32_t>(spec.marker.size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

}  // namespace

static napi_value InitStringSuite(napi_env env, napi_value exports)
{
    std::vector<std::string> exportNames;
    std::vector<napi_property_descriptor> descriptors(K_STRING_CASE_COUNT);
    exportNames.reserve(K_STRING_CASE_COUNT);
    for (size_t caseIndex = 0; caseIndex < K_STRING_CASE_COUNT; caseIndex++) {
        exportNames.emplace_back(BuildIndexedName("testStringCase", caseIndex + K_FIRST_CASE_NUMBER));
        descriptors[caseIndex] = napi_property_descriptor{exportNames.back().c_str(), nullptr, RunStringCase, nullptr,
            nullptr, nullptr, napi_default, reinterpret_cast<void*>(static_cast<uintptr_t>(caseIndex))};
    }
    NAPI_CALL(env, napi_define_properties(env, exports, descriptors.size(), descriptors.data()));
    return exports;
}

static napi_module g_stringSuiteModule = {
    .nm_version = K_MODULE_VERSION,
    .nm_flags = K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitStringSuite,
    .nm_modname = "string_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterStringSuiteModule(void)
{
    napi_module_register(&g_stringSuiteModule);
}
