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

struct ErrorCaseSpec {
    const char* name;
    int32_t minValue;
    int32_t parity;
};

static const ErrorCaseSpec g_errorCaseSpecs[] = {
    {
        "errorCase01",
        -17,
        1,
    },
    {
        "errorCase02",
        -16,
        0,
    },
    {
        "errorCase03",
        -15,
        1,
    },
    {
        "errorCase04",
        -14,
        0,
    },
    {
        "errorCase05",
        -13,
        1,
    },
    {
        "errorCase06",
        -12,
        0,
    },
    {
        "errorCase07",
        -11,
        1,
    },
    {
        "errorCase08",
        -10,
        0,
    },
    {
        "errorCase09",
        -9,
        1,
    },
    {
        "errorCase10",
        -8,
        0,
    },
    {
        "errorCase11",
        -7,
        1,
    },
    {
        "errorCase12",
        -6,
        0,
    },
    {
        "errorCase13",
        -5,
        1,
    },
    {
        "errorCase14",
        -4,
        0,
    },
    {
        "errorCase15",
        -3,
        1,
    },
    {
        "errorCase16",
        -2,
        0,
    },
    {
        "errorCase17",
        -1,
        1,
    },
    {
        "errorCase18",
        0,
        0,
    },
    {
        "errorCase19",
        1,
        1,
    },
    {
        "errorCase20",
        2,
        0,
    },
};

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

bool SetNamedString(napi_env env, napi_value object, const char* name, const std::string& value)
{
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value.c_str(), value.size(), &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

napi_value CreateErrorSummary(napi_env env, const char* name, int32_t value, const std::string& text)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", std::string(name));
    SetNamedInt32(env, result, "value", value);
    SetNamedString(env, result, "text", text);
    return result;
}

static napi_value TestErrorCase01(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t value = 0;
    std::string text;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "value and text are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "value must be a number", &value)) {
        return nullptr;
    }
    if (!ReadUtf8(env, args[1], "text must be a string", &text)) {
        return nullptr;
    }

    const auto& spec = g_errorCaseSpecs[0];
    if (value < spec.minValue) {
        napi_throw_range_error(env, nullptr, spec.name);
        return nullptr;
    }
    if ((value & 1) != spec.parity) {
        napi_throw_error(env, nullptr, "parity check failed");
        return nullptr;
    }
    if (text.empty()) {
        napi_throw_error(env, nullptr, "text must not be empty");
        return nullptr;
    }

    return CreateErrorSummary(env, spec.name, value, text);
}

static napi_value TestErrorCase02(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t value = 0;
    std::string text;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "value and text are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "value must be a number", &value)) {
        return nullptr;
    }
    if (!ReadUtf8(env, args[1], "text must be a string", &text)) {
        return nullptr;
    }

    const auto& spec = g_errorCaseSpecs[1];
    if (value < spec.minValue) {
        napi_throw_range_error(env, nullptr, spec.name);
        return nullptr;
    }
    if ((value & 1) != spec.parity) {
        napi_throw_error(env, nullptr, "parity check failed");
        return nullptr;
    }
    if (text.empty()) {
        napi_throw_error(env, nullptr, "text must not be empty");
        return nullptr;
    }

    return CreateErrorSummary(env, spec.name, value, text);
}

static napi_value TestErrorCase03(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t value = 0;
    std::string text;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "value and text are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "value must be a number", &value)) {
        return nullptr;
    }
    if (!ReadUtf8(env, args[1], "text must be a string", &text)) {
        return nullptr;
    }

    const auto& spec = g_errorCaseSpecs[2];
    if (value < spec.minValue) {
        napi_throw_range_error(env, nullptr, spec.name);
        return nullptr;
    }
    if ((value & 1) != spec.parity) {
        napi_throw_error(env, nullptr, "parity check failed");
        return nullptr;
    }
    if (text.empty()) {
        napi_throw_error(env, nullptr, "text must not be empty");
        return nullptr;
    }

    return CreateErrorSummary(env, spec.name, value, text);
}

static napi_value TestErrorCase04(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t value = 0;
    std::string text;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "value and text are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "value must be a number", &value)) {
        return nullptr;
    }
    if (!ReadUtf8(env, args[1], "text must be a string", &text)) {
        return nullptr;
    }

    const auto& spec = g_errorCaseSpecs[3];
    if (value < spec.minValue) {
        napi_throw_range_error(env, nullptr, spec.name);
        return nullptr;
    }
    if ((value & 1) != spec.parity) {
        napi_throw_error(env, nullptr, "parity check failed");
        return nullptr;
    }
    if (text.empty()) {
        napi_throw_error(env, nullptr, "text must not be empty");
        return nullptr;
    }

    return CreateErrorSummary(env, spec.name, value, text);
}

static napi_value TestErrorCase05(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t value = 0;
    std::string text;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "value and text are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "value must be a number", &value)) {
        return nullptr;
    }
    if (!ReadUtf8(env, args[1], "text must be a string", &text)) {
        return nullptr;
    }

    const auto& spec = g_errorCaseSpecs[4];
    if (value < spec.minValue) {
        napi_throw_range_error(env, nullptr, spec.name);
        return nullptr;
    }
    if ((value & 1) != spec.parity) {
        napi_throw_error(env, nullptr, "parity check failed");
        return nullptr;
    }
    if (text.empty()) {
        napi_throw_error(env, nullptr, "text must not be empty");
        return nullptr;
    }

    return CreateErrorSummary(env, spec.name, value, text);
}

static napi_value TestErrorCase06(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t value = 0;
    std::string text;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "value and text are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "value must be a number", &value)) {
        return nullptr;
    }
    if (!ReadUtf8(env, args[1], "text must be a string", &text)) {
        return nullptr;
    }

    const auto& spec = g_errorCaseSpecs[5];
    if (value < spec.minValue) {
        napi_throw_range_error(env, nullptr, spec.name);
        return nullptr;
    }
    if ((value & 1) != spec.parity) {
        napi_throw_error(env, nullptr, "parity check failed");
        return nullptr;
    }
    if (text.empty()) {
        napi_throw_error(env, nullptr, "text must not be empty");
        return nullptr;
    }

    return CreateErrorSummary(env, spec.name, value, text);
}

static napi_value TestErrorCase07(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t value = 0;
    std::string text;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "value and text are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "value must be a number", &value)) {
        return nullptr;
    }
    if (!ReadUtf8(env, args[1], "text must be a string", &text)) {
        return nullptr;
    }

    const auto& spec = g_errorCaseSpecs[6];
    if (value < spec.minValue) {
        napi_throw_range_error(env, nullptr, spec.name);
        return nullptr;
    }
    if ((value & 1) != spec.parity) {
        napi_throw_error(env, nullptr, "parity check failed");
        return nullptr;
    }
    if (text.empty()) {
        napi_throw_error(env, nullptr, "text must not be empty");
        return nullptr;
    }

    return CreateErrorSummary(env, spec.name, value, text);
}

static napi_value TestErrorCase08(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t value = 0;
    std::string text;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "value and text are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "value must be a number", &value)) {
        return nullptr;
    }
    if (!ReadUtf8(env, args[1], "text must be a string", &text)) {
        return nullptr;
    }

    const auto& spec = g_errorCaseSpecs[7];
    if (value < spec.minValue) {
        napi_throw_range_error(env, nullptr, spec.name);
        return nullptr;
    }
    if ((value & 1) != spec.parity) {
        napi_throw_error(env, nullptr, "parity check failed");
        return nullptr;
    }
    if (text.empty()) {
        napi_throw_error(env, nullptr, "text must not be empty");
        return nullptr;
    }

    return CreateErrorSummary(env, spec.name, value, text);
}

static napi_value TestErrorCase09(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t value = 0;
    std::string text;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "value and text are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "value must be a number", &value)) {
        return nullptr;
    }
    if (!ReadUtf8(env, args[1], "text must be a string", &text)) {
        return nullptr;
    }

    const auto& spec = g_errorCaseSpecs[8];
    if (value < spec.minValue) {
        napi_throw_range_error(env, nullptr, spec.name);
        return nullptr;
    }
    if ((value & 1) != spec.parity) {
        napi_throw_error(env, nullptr, "parity check failed");
        return nullptr;
    }
    if (text.empty()) {
        napi_throw_error(env, nullptr, "text must not be empty");
        return nullptr;
    }

    return CreateErrorSummary(env, spec.name, value, text);
}

static napi_value TestErrorCase10(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t value = 0;
    std::string text;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "value and text are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "value must be a number", &value)) {
        return nullptr;
    }
    if (!ReadUtf8(env, args[1], "text must be a string", &text)) {
        return nullptr;
    }

    const auto& spec = g_errorCaseSpecs[9];
    if (value < spec.minValue) {
        napi_throw_range_error(env, nullptr, spec.name);
        return nullptr;
    }
    if ((value & 1) != spec.parity) {
        napi_throw_error(env, nullptr, "parity check failed");
        return nullptr;
    }
    if (text.empty()) {
        napi_throw_error(env, nullptr, "text must not be empty");
        return nullptr;
    }

    return CreateErrorSummary(env, spec.name, value, text);
}

static napi_value TestErrorCase11(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t value = 0;
    std::string text;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "value and text are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "value must be a number", &value)) {
        return nullptr;
    }
    if (!ReadUtf8(env, args[1], "text must be a string", &text)) {
        return nullptr;
    }

    const auto& spec = g_errorCaseSpecs[10];
    if (value < spec.minValue) {
        napi_throw_range_error(env, nullptr, spec.name);
        return nullptr;
    }
    if ((value & 1) != spec.parity) {
        napi_throw_error(env, nullptr, "parity check failed");
        return nullptr;
    }
    if (text.empty()) {
        napi_throw_error(env, nullptr, "text must not be empty");
        return nullptr;
    }

    return CreateErrorSummary(env, spec.name, value, text);
}

static napi_value TestErrorCase12(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t value = 0;
    std::string text;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "value and text are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "value must be a number", &value)) {
        return nullptr;
    }
    if (!ReadUtf8(env, args[1], "text must be a string", &text)) {
        return nullptr;
    }

    const auto& spec = g_errorCaseSpecs[11];
    if (value < spec.minValue) {
        napi_throw_range_error(env, nullptr, spec.name);
        return nullptr;
    }
    if ((value & 1) != spec.parity) {
        napi_throw_error(env, nullptr, "parity check failed");
        return nullptr;
    }
    if (text.empty()) {
        napi_throw_error(env, nullptr, "text must not be empty");
        return nullptr;
    }

    return CreateErrorSummary(env, spec.name, value, text);
}

static napi_value TestErrorCase13(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t value = 0;
    std::string text;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "value and text are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "value must be a number", &value)) {
        return nullptr;
    }
    if (!ReadUtf8(env, args[1], "text must be a string", &text)) {
        return nullptr;
    }

    const auto& spec = g_errorCaseSpecs[12];
    if (value < spec.minValue) {
        napi_throw_range_error(env, nullptr, spec.name);
        return nullptr;
    }
    if ((value & 1) != spec.parity) {
        napi_throw_error(env, nullptr, "parity check failed");
        return nullptr;
    }
    if (text.empty()) {
        napi_throw_error(env, nullptr, "text must not be empty");
        return nullptr;
    }

    return CreateErrorSummary(env, spec.name, value, text);
}

static napi_value TestErrorCase14(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t value = 0;
    std::string text;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "value and text are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "value must be a number", &value)) {
        return nullptr;
    }
    if (!ReadUtf8(env, args[1], "text must be a string", &text)) {
        return nullptr;
    }

    const auto& spec = g_errorCaseSpecs[13];
    if (value < spec.minValue) {
        napi_throw_range_error(env, nullptr, spec.name);
        return nullptr;
    }
    if ((value & 1) != spec.parity) {
        napi_throw_error(env, nullptr, "parity check failed");
        return nullptr;
    }
    if (text.empty()) {
        napi_throw_error(env, nullptr, "text must not be empty");
        return nullptr;
    }

    return CreateErrorSummary(env, spec.name, value, text);
}

static napi_value TestErrorCase15(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t value = 0;
    std::string text;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "value and text are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "value must be a number", &value)) {
        return nullptr;
    }
    if (!ReadUtf8(env, args[1], "text must be a string", &text)) {
        return nullptr;
    }

    const auto& spec = g_errorCaseSpecs[14];
    if (value < spec.minValue) {
        napi_throw_range_error(env, nullptr, spec.name);
        return nullptr;
    }
    if ((value & 1) != spec.parity) {
        napi_throw_error(env, nullptr, "parity check failed");
        return nullptr;
    }
    if (text.empty()) {
        napi_throw_error(env, nullptr, "text must not be empty");
        return nullptr;
    }

    return CreateErrorSummary(env, spec.name, value, text);
}

static napi_value TestErrorCase16(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t value = 0;
    std::string text;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "value and text are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "value must be a number", &value)) {
        return nullptr;
    }
    if (!ReadUtf8(env, args[1], "text must be a string", &text)) {
        return nullptr;
    }

    const auto& spec = g_errorCaseSpecs[15];
    if (value < spec.minValue) {
        napi_throw_range_error(env, nullptr, spec.name);
        return nullptr;
    }
    if ((value & 1) != spec.parity) {
        napi_throw_error(env, nullptr, "parity check failed");
        return nullptr;
    }
    if (text.empty()) {
        napi_throw_error(env, nullptr, "text must not be empty");
        return nullptr;
    }

    return CreateErrorSummary(env, spec.name, value, text);
}

static napi_value TestErrorCase17(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t value = 0;
    std::string text;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "value and text are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "value must be a number", &value)) {
        return nullptr;
    }
    if (!ReadUtf8(env, args[1], "text must be a string", &text)) {
        return nullptr;
    }

    const auto& spec = g_errorCaseSpecs[16];
    if (value < spec.minValue) {
        napi_throw_range_error(env, nullptr, spec.name);
        return nullptr;
    }
    if ((value & 1) != spec.parity) {
        napi_throw_error(env, nullptr, "parity check failed");
        return nullptr;
    }
    if (text.empty()) {
        napi_throw_error(env, nullptr, "text must not be empty");
        return nullptr;
    }

    return CreateErrorSummary(env, spec.name, value, text);
}

static napi_value TestErrorCase18(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t value = 0;
    std::string text;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "value and text are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "value must be a number", &value)) {
        return nullptr;
    }
    if (!ReadUtf8(env, args[1], "text must be a string", &text)) {
        return nullptr;
    }

    const auto& spec = g_errorCaseSpecs[17];
    if (value < spec.minValue) {
        napi_throw_range_error(env, nullptr, spec.name);
        return nullptr;
    }
    if ((value & 1) != spec.parity) {
        napi_throw_error(env, nullptr, "parity check failed");
        return nullptr;
    }
    if (text.empty()) {
        napi_throw_error(env, nullptr, "text must not be empty");
        return nullptr;
    }

    return CreateErrorSummary(env, spec.name, value, text);
}

static napi_value TestErrorCase19(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t value = 0;
    std::string text;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "value and text are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "value must be a number", &value)) {
        return nullptr;
    }
    if (!ReadUtf8(env, args[1], "text must be a string", &text)) {
        return nullptr;
    }

    const auto& spec = g_errorCaseSpecs[18];
    if (value < spec.minValue) {
        napi_throw_range_error(env, nullptr, spec.name);
        return nullptr;
    }
    if ((value & 1) != spec.parity) {
        napi_throw_error(env, nullptr, "parity check failed");
        return nullptr;
    }
    if (text.empty()) {
        napi_throw_error(env, nullptr, "text must not be empty");
        return nullptr;
    }

    return CreateErrorSummary(env, spec.name, value, text);
}

static napi_value TestErrorCase20(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t value = 0;
    std::string text;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "value and text are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "value must be a number", &value)) {
        return nullptr;
    }
    if (!ReadUtf8(env, args[1], "text must be a string", &text)) {
        return nullptr;
    }

    const auto& spec = g_errorCaseSpecs[19];
    if (value < spec.minValue) {
        napi_throw_range_error(env, nullptr, spec.name);
        return nullptr;
    }
    if ((value & 1) != spec.parity) {
        napi_throw_error(env, nullptr, "parity check failed");
        return nullptr;
    }
    if (text.empty()) {
        napi_throw_error(env, nullptr, "text must not be empty");
        return nullptr;
    }

    return CreateErrorSummary(env, spec.name, value, text);
}

}  // namespace

static napi_value InitBranch10Error(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[] = {
        DECLARE_NAPI_FUNCTION("testErrorCase01", TestErrorCase01),
        DECLARE_NAPI_FUNCTION("testErrorCase02", TestErrorCase02),
        DECLARE_NAPI_FUNCTION("testErrorCase03", TestErrorCase03),
        DECLARE_NAPI_FUNCTION("testErrorCase04", TestErrorCase04),
        DECLARE_NAPI_FUNCTION("testErrorCase05", TestErrorCase05),
        DECLARE_NAPI_FUNCTION("testErrorCase06", TestErrorCase06),
        DECLARE_NAPI_FUNCTION("testErrorCase07", TestErrorCase07),
        DECLARE_NAPI_FUNCTION("testErrorCase08", TestErrorCase08),
        DECLARE_NAPI_FUNCTION("testErrorCase09", TestErrorCase09),
        DECLARE_NAPI_FUNCTION("testErrorCase10", TestErrorCase10),
        DECLARE_NAPI_FUNCTION("testErrorCase11", TestErrorCase11),
        DECLARE_NAPI_FUNCTION("testErrorCase12", TestErrorCase12),
        DECLARE_NAPI_FUNCTION("testErrorCase13", TestErrorCase13),
        DECLARE_NAPI_FUNCTION("testErrorCase14", TestErrorCase14),
        DECLARE_NAPI_FUNCTION("testErrorCase15", TestErrorCase15),
        DECLARE_NAPI_FUNCTION("testErrorCase16", TestErrorCase16),
        DECLARE_NAPI_FUNCTION("testErrorCase17", TestErrorCase17),
        DECLARE_NAPI_FUNCTION("testErrorCase18", TestErrorCase18),
        DECLARE_NAPI_FUNCTION("testErrorCase19", TestErrorCase19),
        DECLARE_NAPI_FUNCTION("testErrorCase20", TestErrorCase20),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(descriptors) / sizeof(descriptors[0]), descriptors));
    return exports;
}

static napi_module g_branch10ErrorModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitBranch10Error,
    .nm_modname = "error_suite",
    .nm_priv = nullptr,
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterBranch10ErrorModule(void)
{
    napi_module_register(&g_branch10ErrorModule);
}
