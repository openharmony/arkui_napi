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

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

struct StringCaseSpec {
    const char* name;
    const char* prefix;
    const char* suffix;
    const char* marker;
    int32_t repeat;
};

static const StringCaseSpec g_stringCaseSpecs[] = {
    {
        "stringCase01",
        "pre01",
        "suf01",
        "M1",
        2,
    },
    {
        "stringCase02",
        "pre02",
        "suf02",
        "M2",
        3,
    },
    {
        "stringCase03",
        "pre03",
        "suf03",
        "M3",
        4,
    },
    {
        "stringCase04",
        "pre04",
        "suf04",
        "M4",
        1,
    },
    {
        "stringCase05",
        "pre05",
        "suf05",
        "M5",
        2,
    },
    {
        "stringCase06",
        "pre06",
        "suf06",
        "M6",
        3,
    },
    {
        "stringCase07",
        "pre07",
        "suf07",
        "M0",
        4,
    },
    {
        "stringCase08",
        "pre08",
        "suf08",
        "M1",
        1,
    },
    {
        "stringCase09",
        "pre09",
        "suf09",
        "M2",
        2,
    },
    {
        "stringCase10",
        "pre10",
        "suf10",
        "M3",
        3,
    },
    {
        "stringCase11",
        "pre11",
        "suf11",
        "M4",
        4,
    },
    {
        "stringCase12",
        "pre12",
        "suf12",
        "M5",
        1,
    },
    {
        "stringCase13",
        "pre13",
        "suf13",
        "M6",
        2,
    },
    {
        "stringCase14",
        "pre14",
        "suf14",
        "M0",
        3,
    },
    {
        "stringCase15",
        "pre15",
        "suf15",
        "M1",
        4,
    },
    {
        "stringCase16",
        "pre16",
        "suf16",
        "M2",
        1,
    },
    {
        "stringCase17",
        "pre17",
        "suf17",
        "M3",
        2,
    },
    {
        "stringCase18",
        "pre18",
        "suf18",
        "M4",
        3,
    },
    {
        "stringCase19",
        "pre19",
        "suf19",
        "M5",
        4,
    },
    {
        "stringCase20",
        "pre20",
        "suf20",
        "M6",
        1,
    },
    {
        "stringCase21",
        "pre21",
        "suf21",
        "M0",
        2,
    },
};

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

    std::string buffer(length + 1, '\0');
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

napi_value CreateStringSummary(napi_env env, const char* name, const std::string& text, int32_t measuredLength,
    int32_t expectedLength, bool containsMarker)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", std::string(name));
    SetNamedString(env, result, "text", text);
    SetNamedInt32(env, result, "measuredLength", measuredLength);
    SetNamedInt32(env, result, "expectedLength", expectedLength);
    SetNamedBool(env, result, "containsMarker", containsMarker);
    SetNamedBool(env, result, "passed", measuredLength == expectedLength && containsMarker);
    return result;
}

static napi_value TestStringCase01(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[0];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

static napi_value TestStringCase02(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[1];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

static napi_value TestStringCase03(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[2];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

static napi_value TestStringCase04(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[3];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

static napi_value TestStringCase05(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[4];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

static napi_value TestStringCase06(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[5];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

static napi_value TestStringCase07(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[6];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

static napi_value TestStringCase08(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[7];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

static napi_value TestStringCase09(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[8];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

static napi_value TestStringCase10(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[9];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

static napi_value TestStringCase11(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[10];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

static napi_value TestStringCase12(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[11];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

static napi_value TestStringCase13(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[12];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

static napi_value TestStringCase14(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[13];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

static napi_value TestStringCase15(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[14];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

static napi_value TestStringCase16(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[15];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

static napi_value TestStringCase17(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[16];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

static napi_value TestStringCase18(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[17];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

static napi_value TestStringCase19(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[18];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

static napi_value TestStringCase20(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[19];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

static napi_value TestStringCase21(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::string source;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "source is required");
        return nullptr;
    }
    if (!ReadUtf8(env, args[0], "source must be a string", &source)) {
        return nullptr;
    }

    const auto& spec = g_stringCaseSpecs[20];
    std::string transformed = spec.prefix + source + ":" + spec.suffix;
    for (int32_t step = 0; step < spec.repeat; step++) {
        transformed += "|";
        transformed += spec.marker;
    }

    bool containsMarker = transformed.find(spec.marker) != std::string::npos;
    int32_t measuredLength = static_cast<int32_t>(transformed.size());
    int32_t expectedLength =
        static_cast<int32_t>(std::string(spec.prefix).size() + source.size() + 1 + std::string(spec.suffix).size() +
                             spec.repeat * (1 + static_cast<int32_t>(std::string(spec.marker).size())));

    return CreateStringSummary(env, spec.name, transformed, measuredLength, expectedLength, containsMarker);
}

}  // namespace

static napi_value InitBranch02String(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[] = {
        DECLARE_NAPI_FUNCTION("testStringCase01", TestStringCase01),
        DECLARE_NAPI_FUNCTION("testStringCase02", TestStringCase02),
        DECLARE_NAPI_FUNCTION("testStringCase03", TestStringCase03),
        DECLARE_NAPI_FUNCTION("testStringCase04", TestStringCase04),
        DECLARE_NAPI_FUNCTION("testStringCase05", TestStringCase05),
        DECLARE_NAPI_FUNCTION("testStringCase06", TestStringCase06),
        DECLARE_NAPI_FUNCTION("testStringCase07", TestStringCase07),
        DECLARE_NAPI_FUNCTION("testStringCase08", TestStringCase08),
        DECLARE_NAPI_FUNCTION("testStringCase09", TestStringCase09),
        DECLARE_NAPI_FUNCTION("testStringCase10", TestStringCase10),
        DECLARE_NAPI_FUNCTION("testStringCase11", TestStringCase11),
        DECLARE_NAPI_FUNCTION("testStringCase12", TestStringCase12),
        DECLARE_NAPI_FUNCTION("testStringCase13", TestStringCase13),
        DECLARE_NAPI_FUNCTION("testStringCase14", TestStringCase14),
        DECLARE_NAPI_FUNCTION("testStringCase15", TestStringCase15),
        DECLARE_NAPI_FUNCTION("testStringCase16", TestStringCase16),
        DECLARE_NAPI_FUNCTION("testStringCase17", TestStringCase17),
        DECLARE_NAPI_FUNCTION("testStringCase18", TestStringCase18),
        DECLARE_NAPI_FUNCTION("testStringCase19", TestStringCase19),
        DECLARE_NAPI_FUNCTION("testStringCase20", TestStringCase20),
        DECLARE_NAPI_FUNCTION("testStringCase21", TestStringCase21),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(descriptors) / sizeof(descriptors[0]), descriptors));
    return exports;
}

static napi_module g_branch02StringModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitBranch02String,
    .nm_modname = "string_suite",
    .nm_priv = nullptr,
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterBranch02StringModule(void)
{
    napi_module_register(&g_branch02StringModule);
}
