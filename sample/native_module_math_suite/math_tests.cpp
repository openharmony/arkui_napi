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

struct MathCaseSpec {
    const char* name;
    int32_t bias;
    int32_t multiplier;
    int32_t shift;
    int32_t loop;
};

static const MathCaseSpec g_mathCaseSpecs[] = {
    {
        "mathCase01",
        1,
        2,
        -9,
        3,
    },
    {
        "mathCase02",
        2,
        3,
        -7,
        4,
    },
    {
        "mathCase03",
        3,
        4,
        -5,
        5,
    },
    {
        "mathCase04",
        4,
        5,
        -3,
        2,
    },
    {
        "mathCase05",
        5,
        1,
        -1,
        3,
    },
    {
        "mathCase06",
        6,
        2,
        1,
        4,
    },
    {
        "mathCase07",
        7,
        3,
        3,
        5,
    },
    {
        "mathCase08",
        8,
        4,
        5,
        2,
    },
    {
        "mathCase09",
        9,
        5,
        7,
        3,
    },
    {
        "mathCase10",
        10,
        1,
        9,
        4,
    },
    {
        "mathCase11",
        11,
        2,
        11,
        5,
    },
    {
        "mathCase12",
        12,
        3,
        13,
        2,
    },
    {
        "mathCase13",
        13,
        4,
        15,
        3,
    },
    {
        "mathCase14",
        14,
        5,
        17,
        4,
    },
    {
        "mathCase15",
        15,
        1,
        19,
        5,
    },
    {
        "mathCase16",
        16,
        2,
        21,
        2,
    },
    {
        "mathCase17",
        17,
        3,
        23,
        3,
    },
    {
        "mathCase18",
        18,
        4,
        25,
        4,
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

napi_value CreateMathSummary(
    napi_env env, const char* name, int32_t left, int32_t right, int32_t actual, int32_t expected)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", name);
    SetNamedInt32(env, result, "left", left);
    SetNamedInt32(env, result, "right", right);
    SetNamedInt32(env, result, "actual", actual);
    SetNamedInt32(env, result, "expected", expected);
    SetNamedBool(env, result, "passed", actual == expected);
    return result;
}

static napi_value TestMathCase01(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t left = 0;
    int32_t right = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "left and right are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "left must be a number", &left)) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "right must be a number", &right)) {
        return nullptr;
    }

    const auto& spec = g_mathCaseSpecs[0];
    int32_t actual = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        actual += spec.bias + step;
    }
    actual = actual * spec.multiplier + right + spec.shift;

    int32_t expected = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        expected += spec.bias + step;
    }
    expected = expected * spec.multiplier + right + spec.shift;

    return CreateMathSummary(env, spec.name, left, right, actual, expected);
}

static napi_value TestMathCase02(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t left = 0;
    int32_t right = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "left and right are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "left must be a number", &left)) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "right must be a number", &right)) {
        return nullptr;
    }

    const auto& spec = g_mathCaseSpecs[1];
    int32_t actual = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        actual += spec.bias + step;
    }
    actual = actual * spec.multiplier + right + spec.shift;

    int32_t expected = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        expected += spec.bias + step;
    }
    expected = expected * spec.multiplier + right + spec.shift;

    return CreateMathSummary(env, spec.name, left, right, actual, expected);
}

static napi_value TestMathCase03(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t left = 0;
    int32_t right = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "left and right are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "left must be a number", &left)) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "right must be a number", &right)) {
        return nullptr;
    }

    const auto& spec = g_mathCaseSpecs[2];
    int32_t actual = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        actual += spec.bias + step;
    }
    actual = actual * spec.multiplier + right + spec.shift;

    int32_t expected = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        expected += spec.bias + step;
    }
    expected = expected * spec.multiplier + right + spec.shift;

    return CreateMathSummary(env, spec.name, left, right, actual, expected);
}

static napi_value TestMathCase04(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t left = 0;
    int32_t right = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "left and right are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "left must be a number", &left)) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "right must be a number", &right)) {
        return nullptr;
    }

    const auto& spec = g_mathCaseSpecs[3];
    int32_t actual = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        actual += spec.bias + step;
    }
    actual = actual * spec.multiplier + right + spec.shift;

    int32_t expected = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        expected += spec.bias + step;
    }
    expected = expected * spec.multiplier + right + spec.shift;

    return CreateMathSummary(env, spec.name, left, right, actual, expected);
}

static napi_value TestMathCase05(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t left = 0;
    int32_t right = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "left and right are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "left must be a number", &left)) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "right must be a number", &right)) {
        return nullptr;
    }

    const auto& spec = g_mathCaseSpecs[4];
    int32_t actual = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        actual += spec.bias + step;
    }
    actual = actual * spec.multiplier + right + spec.shift;

    int32_t expected = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        expected += spec.bias + step;
    }
    expected = expected * spec.multiplier + right + spec.shift;

    return CreateMathSummary(env, spec.name, left, right, actual, expected);
}

static napi_value TestMathCase06(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t left = 0;
    int32_t right = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "left and right are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "left must be a number", &left)) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "right must be a number", &right)) {
        return nullptr;
    }

    const auto& spec = g_mathCaseSpecs[5];
    int32_t actual = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        actual += spec.bias + step;
    }
    actual = actual * spec.multiplier + right + spec.shift;

    int32_t expected = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        expected += spec.bias + step;
    }
    expected = expected * spec.multiplier + right + spec.shift;

    return CreateMathSummary(env, spec.name, left, right, actual, expected);
}

static napi_value TestMathCase07(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t left = 0;
    int32_t right = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "left and right are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "left must be a number", &left)) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "right must be a number", &right)) {
        return nullptr;
    }

    const auto& spec = g_mathCaseSpecs[6];
    int32_t actual = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        actual += spec.bias + step;
    }
    actual = actual * spec.multiplier + right + spec.shift;

    int32_t expected = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        expected += spec.bias + step;
    }
    expected = expected * spec.multiplier + right + spec.shift;

    return CreateMathSummary(env, spec.name, left, right, actual, expected);
}

static napi_value TestMathCase08(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t left = 0;
    int32_t right = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "left and right are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "left must be a number", &left)) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "right must be a number", &right)) {
        return nullptr;
    }

    const auto& spec = g_mathCaseSpecs[7];
    int32_t actual = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        actual += spec.bias + step;
    }
    actual = actual * spec.multiplier + right + spec.shift;

    int32_t expected = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        expected += spec.bias + step;
    }
    expected = expected * spec.multiplier + right + spec.shift;

    return CreateMathSummary(env, spec.name, left, right, actual, expected);
}

static napi_value TestMathCase09(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t left = 0;
    int32_t right = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "left and right are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "left must be a number", &left)) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "right must be a number", &right)) {
        return nullptr;
    }

    const auto& spec = g_mathCaseSpecs[8];
    int32_t actual = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        actual += spec.bias + step;
    }
    actual = actual * spec.multiplier + right + spec.shift;

    int32_t expected = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        expected += spec.bias + step;
    }
    expected = expected * spec.multiplier + right + spec.shift;

    return CreateMathSummary(env, spec.name, left, right, actual, expected);
}

static napi_value TestMathCase10(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t left = 0;
    int32_t right = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "left and right are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "left must be a number", &left)) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "right must be a number", &right)) {
        return nullptr;
    }

    const auto& spec = g_mathCaseSpecs[9];
    int32_t actual = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        actual += spec.bias + step;
    }
    actual = actual * spec.multiplier + right + spec.shift;

    int32_t expected = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        expected += spec.bias + step;
    }
    expected = expected * spec.multiplier + right + spec.shift;

    return CreateMathSummary(env, spec.name, left, right, actual, expected);
}

static napi_value TestMathCase11(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t left = 0;
    int32_t right = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "left and right are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "left must be a number", &left)) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "right must be a number", &right)) {
        return nullptr;
    }

    const auto& spec = g_mathCaseSpecs[10];
    int32_t actual = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        actual += spec.bias + step;
    }
    actual = actual * spec.multiplier + right + spec.shift;

    int32_t expected = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        expected += spec.bias + step;
    }
    expected = expected * spec.multiplier + right + spec.shift;

    return CreateMathSummary(env, spec.name, left, right, actual, expected);
}

static napi_value TestMathCase12(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t left = 0;
    int32_t right = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "left and right are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "left must be a number", &left)) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "right must be a number", &right)) {
        return nullptr;
    }

    const auto& spec = g_mathCaseSpecs[11];
    int32_t actual = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        actual += spec.bias + step;
    }
    actual = actual * spec.multiplier + right + spec.shift;

    int32_t expected = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        expected += spec.bias + step;
    }
    expected = expected * spec.multiplier + right + spec.shift;

    return CreateMathSummary(env, spec.name, left, right, actual, expected);
}

static napi_value TestMathCase13(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t left = 0;
    int32_t right = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "left and right are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "left must be a number", &left)) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "right must be a number", &right)) {
        return nullptr;
    }

    const auto& spec = g_mathCaseSpecs[12];
    int32_t actual = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        actual += spec.bias + step;
    }
    actual = actual * spec.multiplier + right + spec.shift;

    int32_t expected = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        expected += spec.bias + step;
    }
    expected = expected * spec.multiplier + right + spec.shift;

    return CreateMathSummary(env, spec.name, left, right, actual, expected);
}

static napi_value TestMathCase14(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t left = 0;
    int32_t right = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "left and right are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "left must be a number", &left)) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "right must be a number", &right)) {
        return nullptr;
    }

    const auto& spec = g_mathCaseSpecs[13];
    int32_t actual = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        actual += spec.bias + step;
    }
    actual = actual * spec.multiplier + right + spec.shift;

    int32_t expected = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        expected += spec.bias + step;
    }
    expected = expected * spec.multiplier + right + spec.shift;

    return CreateMathSummary(env, spec.name, left, right, actual, expected);
}

static napi_value TestMathCase15(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t left = 0;
    int32_t right = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "left and right are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "left must be a number", &left)) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "right must be a number", &right)) {
        return nullptr;
    }

    const auto& spec = g_mathCaseSpecs[14];
    int32_t actual = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        actual += spec.bias + step;
    }
    actual = actual * spec.multiplier + right + spec.shift;

    int32_t expected = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        expected += spec.bias + step;
    }
    expected = expected * spec.multiplier + right + spec.shift;

    return CreateMathSummary(env, spec.name, left, right, actual, expected);
}

static napi_value TestMathCase16(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t left = 0;
    int32_t right = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "left and right are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "left must be a number", &left)) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "right must be a number", &right)) {
        return nullptr;
    }

    const auto& spec = g_mathCaseSpecs[15];
    int32_t actual = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        actual += spec.bias + step;
    }
    actual = actual * spec.multiplier + right + spec.shift;

    int32_t expected = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        expected += spec.bias + step;
    }
    expected = expected * spec.multiplier + right + spec.shift;

    return CreateMathSummary(env, spec.name, left, right, actual, expected);
}

static napi_value TestMathCase17(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t left = 0;
    int32_t right = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "left and right are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "left must be a number", &left)) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "right must be a number", &right)) {
        return nullptr;
    }

    const auto& spec = g_mathCaseSpecs[16];
    int32_t actual = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        actual += spec.bias + step;
    }
    actual = actual * spec.multiplier + right + spec.shift;

    int32_t expected = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        expected += spec.bias + step;
    }
    expected = expected * spec.multiplier + right + spec.shift;

    return CreateMathSummary(env, spec.name, left, right, actual, expected);
}

static napi_value TestMathCase18(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t left = 0;
    int32_t right = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "left and right are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "left must be a number", &left)) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "right must be a number", &right)) {
        return nullptr;
    }

    const auto& spec = g_mathCaseSpecs[17];
    int32_t actual = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        actual += spec.bias + step;
    }
    actual = actual * spec.multiplier + right + spec.shift;

    int32_t expected = left;
    for (int32_t step = 0; step < spec.loop; step++) {
        expected += spec.bias + step;
    }
    expected = expected * spec.multiplier + right + spec.shift;

    return CreateMathSummary(env, spec.name, left, right, actual, expected);
}

}  // namespace

static napi_value InitBranch01Math(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[] = {
        DECLARE_NAPI_FUNCTION("testMathCase01", TestMathCase01),
        DECLARE_NAPI_FUNCTION("testMathCase02", TestMathCase02),
        DECLARE_NAPI_FUNCTION("testMathCase03", TestMathCase03),
        DECLARE_NAPI_FUNCTION("testMathCase04", TestMathCase04),
        DECLARE_NAPI_FUNCTION("testMathCase05", TestMathCase05),
        DECLARE_NAPI_FUNCTION("testMathCase06", TestMathCase06),
        DECLARE_NAPI_FUNCTION("testMathCase07", TestMathCase07),
        DECLARE_NAPI_FUNCTION("testMathCase08", TestMathCase08),
        DECLARE_NAPI_FUNCTION("testMathCase09", TestMathCase09),
        DECLARE_NAPI_FUNCTION("testMathCase10", TestMathCase10),
        DECLARE_NAPI_FUNCTION("testMathCase11", TestMathCase11),
        DECLARE_NAPI_FUNCTION("testMathCase12", TestMathCase12),
        DECLARE_NAPI_FUNCTION("testMathCase13", TestMathCase13),
        DECLARE_NAPI_FUNCTION("testMathCase14", TestMathCase14),
        DECLARE_NAPI_FUNCTION("testMathCase15", TestMathCase15),
        DECLARE_NAPI_FUNCTION("testMathCase16", TestMathCase16),
        DECLARE_NAPI_FUNCTION("testMathCase17", TestMathCase17),
        DECLARE_NAPI_FUNCTION("testMathCase18", TestMathCase18),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(descriptors) / sizeof(descriptors[0]), descriptors));
    return exports;
}

static napi_module g_branch01MathModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitBranch01Math,
    .nm_modname = "math_suite",
    .nm_priv = nullptr,
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterBranch01MathModule(void)
{
    napi_module_register(&g_branch01MathModule);
}
