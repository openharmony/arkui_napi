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

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

struct BooleanCaseSpec {
    const char* name;
    bool invertFirst;
    bool invertSecond;
    bool invertThird;
    const char* route;
};

static const BooleanCaseSpec g_booleanCaseSpecs[] = {
    {
        "booleanCase01",
        true,
        false,
        false,
        "beta",
    },
    {
        "booleanCase02",
        false,
        false,
        false,
        "gamma",
    },
    {
        "booleanCase03",
        true,
        true,
        false,
        "delta",
    },
    {
        "booleanCase04",
        false,
        false,
        true,
        "alpha",
    },
    {
        "booleanCase05",
        true,
        false,
        false,
        "beta",
    },
    {
        "booleanCase06",
        false,
        true,
        false,
        "gamma",
    },
    {
        "booleanCase07",
        true,
        false,
        false,
        "delta",
    },
    {
        "booleanCase08",
        false,
        false,
        true,
        "alpha",
    },
    {
        "booleanCase09",
        true,
        true,
        false,
        "beta",
    },
    {
        "booleanCase10",
        false,
        false,
        false,
        "gamma",
    },
    {
        "booleanCase11",
        true,
        false,
        false,
        "delta",
    },
    {
        "booleanCase12",
        false,
        true,
        true,
        "alpha",
    },
    {
        "booleanCase13",
        true,
        false,
        false,
        "beta",
    },
    {
        "booleanCase14",
        false,
        false,
        false,
        "gamma",
    },
    {
        "booleanCase15",
        true,
        true,
        false,
        "delta",
    },
    {
        "booleanCase16",
        false,
        false,
        true,
        "alpha",
    },
    {
        "booleanCase17",
        true,
        false,
        false,
        "beta",
    },
    {
        "booleanCase18",
        false,
        true,
        false,
        "gamma",
    },
    {
        "booleanCase19",
        true,
        false,
        false,
        "delta",
    },
    {
        "booleanCase20",
        false,
        false,
        true,
        "alpha",
    },
};

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

static napi_value TestBooleanCase01(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[0], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[1], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[2], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto& spec = g_booleanCaseSpecs[0];
    bool normalizedFirst = spec.invertFirst ? !first : first;
    bool normalizedSecond = spec.invertSecond ? !second : second;
    bool normalizedThird = spec.invertThird ? !third : third;
    int32_t score = static_cast<int32_t>(normalizedFirst) * 4 + static_cast<int32_t>(normalizedSecond) * 2 +
                    static_cast<int32_t>(normalizedThird);
    bool passed = score >= 3;

    return CreateBooleanSummary(env, spec.name, spec.route, score, passed);
}

static napi_value TestBooleanCase02(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[0], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[1], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[2], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto& spec = g_booleanCaseSpecs[1];
    bool normalizedFirst = spec.invertFirst ? !first : first;
    bool normalizedSecond = spec.invertSecond ? !second : second;
    bool normalizedThird = spec.invertThird ? !third : third;
    int32_t score = static_cast<int32_t>(normalizedFirst) * 4 + static_cast<int32_t>(normalizedSecond) * 2 +
                    static_cast<int32_t>(normalizedThird);
    bool passed = score >= 3;

    return CreateBooleanSummary(env, spec.name, spec.route, score, passed);
}

static napi_value TestBooleanCase03(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[0], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[1], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[2], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto& spec = g_booleanCaseSpecs[2];
    bool normalizedFirst = spec.invertFirst ? !first : first;
    bool normalizedSecond = spec.invertSecond ? !second : second;
    bool normalizedThird = spec.invertThird ? !third : third;
    int32_t score = static_cast<int32_t>(normalizedFirst) * 4 + static_cast<int32_t>(normalizedSecond) * 2 +
                    static_cast<int32_t>(normalizedThird);
    bool passed = score >= 3;

    return CreateBooleanSummary(env, spec.name, spec.route, score, passed);
}

static napi_value TestBooleanCase04(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[0], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[1], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[2], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto& spec = g_booleanCaseSpecs[3];
    bool normalizedFirst = spec.invertFirst ? !first : first;
    bool normalizedSecond = spec.invertSecond ? !second : second;
    bool normalizedThird = spec.invertThird ? !third : third;
    int32_t score = static_cast<int32_t>(normalizedFirst) * 4 + static_cast<int32_t>(normalizedSecond) * 2 +
                    static_cast<int32_t>(normalizedThird);
    bool passed = score >= 3;

    return CreateBooleanSummary(env, spec.name, spec.route, score, passed);
}

static napi_value TestBooleanCase05(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[0], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[1], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[2], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto& spec = g_booleanCaseSpecs[4];
    bool normalizedFirst = spec.invertFirst ? !first : first;
    bool normalizedSecond = spec.invertSecond ? !second : second;
    bool normalizedThird = spec.invertThird ? !third : third;
    int32_t score = static_cast<int32_t>(normalizedFirst) * 4 + static_cast<int32_t>(normalizedSecond) * 2 +
                    static_cast<int32_t>(normalizedThird);
    bool passed = score >= 3;

    return CreateBooleanSummary(env, spec.name, spec.route, score, passed);
}

static napi_value TestBooleanCase06(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[0], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[1], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[2], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto& spec = g_booleanCaseSpecs[5];
    bool normalizedFirst = spec.invertFirst ? !first : first;
    bool normalizedSecond = spec.invertSecond ? !second : second;
    bool normalizedThird = spec.invertThird ? !third : third;
    int32_t score = static_cast<int32_t>(normalizedFirst) * 4 + static_cast<int32_t>(normalizedSecond) * 2 +
                    static_cast<int32_t>(normalizedThird);
    bool passed = score >= 3;

    return CreateBooleanSummary(env, spec.name, spec.route, score, passed);
}

static napi_value TestBooleanCase07(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[0], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[1], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[2], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto& spec = g_booleanCaseSpecs[6];
    bool normalizedFirst = spec.invertFirst ? !first : first;
    bool normalizedSecond = spec.invertSecond ? !second : second;
    bool normalizedThird = spec.invertThird ? !third : third;
    int32_t score = static_cast<int32_t>(normalizedFirst) * 4 + static_cast<int32_t>(normalizedSecond) * 2 +
                    static_cast<int32_t>(normalizedThird);
    bool passed = score >= 3;

    return CreateBooleanSummary(env, spec.name, spec.route, score, passed);
}

static napi_value TestBooleanCase08(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[0], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[1], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[2], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto& spec = g_booleanCaseSpecs[7];
    bool normalizedFirst = spec.invertFirst ? !first : first;
    bool normalizedSecond = spec.invertSecond ? !second : second;
    bool normalizedThird = spec.invertThird ? !third : third;
    int32_t score = static_cast<int32_t>(normalizedFirst) * 4 + static_cast<int32_t>(normalizedSecond) * 2 +
                    static_cast<int32_t>(normalizedThird);
    bool passed = score >= 3;

    return CreateBooleanSummary(env, spec.name, spec.route, score, passed);
}

static napi_value TestBooleanCase09(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[0], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[1], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[2], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto& spec = g_booleanCaseSpecs[8];
    bool normalizedFirst = spec.invertFirst ? !first : first;
    bool normalizedSecond = spec.invertSecond ? !second : second;
    bool normalizedThird = spec.invertThird ? !third : third;
    int32_t score = static_cast<int32_t>(normalizedFirst) * 4 + static_cast<int32_t>(normalizedSecond) * 2 +
                    static_cast<int32_t>(normalizedThird);
    bool passed = score >= 3;

    return CreateBooleanSummary(env, spec.name, spec.route, score, passed);
}

static napi_value TestBooleanCase10(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[0], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[1], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[2], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto& spec = g_booleanCaseSpecs[9];
    bool normalizedFirst = spec.invertFirst ? !first : first;
    bool normalizedSecond = spec.invertSecond ? !second : second;
    bool normalizedThird = spec.invertThird ? !third : third;
    int32_t score = static_cast<int32_t>(normalizedFirst) * 4 + static_cast<int32_t>(normalizedSecond) * 2 +
                    static_cast<int32_t>(normalizedThird);
    bool passed = score >= 3;

    return CreateBooleanSummary(env, spec.name, spec.route, score, passed);
}

static napi_value TestBooleanCase11(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[0], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[1], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[2], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto& spec = g_booleanCaseSpecs[10];
    bool normalizedFirst = spec.invertFirst ? !first : first;
    bool normalizedSecond = spec.invertSecond ? !second : second;
    bool normalizedThird = spec.invertThird ? !third : third;
    int32_t score = static_cast<int32_t>(normalizedFirst) * 4 + static_cast<int32_t>(normalizedSecond) * 2 +
                    static_cast<int32_t>(normalizedThird);
    bool passed = score >= 3;

    return CreateBooleanSummary(env, spec.name, spec.route, score, passed);
}

static napi_value TestBooleanCase12(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[0], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[1], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[2], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto& spec = g_booleanCaseSpecs[11];
    bool normalizedFirst = spec.invertFirst ? !first : first;
    bool normalizedSecond = spec.invertSecond ? !second : second;
    bool normalizedThird = spec.invertThird ? !third : third;
    int32_t score = static_cast<int32_t>(normalizedFirst) * 4 + static_cast<int32_t>(normalizedSecond) * 2 +
                    static_cast<int32_t>(normalizedThird);
    bool passed = score >= 3;

    return CreateBooleanSummary(env, spec.name, spec.route, score, passed);
}

static napi_value TestBooleanCase13(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[0], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[1], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[2], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto& spec = g_booleanCaseSpecs[12];
    bool normalizedFirst = spec.invertFirst ? !first : first;
    bool normalizedSecond = spec.invertSecond ? !second : second;
    bool normalizedThird = spec.invertThird ? !third : third;
    int32_t score = static_cast<int32_t>(normalizedFirst) * 4 + static_cast<int32_t>(normalizedSecond) * 2 +
                    static_cast<int32_t>(normalizedThird);
    bool passed = score >= 3;

    return CreateBooleanSummary(env, spec.name, spec.route, score, passed);
}

static napi_value TestBooleanCase14(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[0], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[1], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[2], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto& spec = g_booleanCaseSpecs[13];
    bool normalizedFirst = spec.invertFirst ? !first : first;
    bool normalizedSecond = spec.invertSecond ? !second : second;
    bool normalizedThird = spec.invertThird ? !third : third;
    int32_t score = static_cast<int32_t>(normalizedFirst) * 4 + static_cast<int32_t>(normalizedSecond) * 2 +
                    static_cast<int32_t>(normalizedThird);
    bool passed = score >= 3;

    return CreateBooleanSummary(env, spec.name, spec.route, score, passed);
}

static napi_value TestBooleanCase15(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[0], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[1], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[2], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto& spec = g_booleanCaseSpecs[14];
    bool normalizedFirst = spec.invertFirst ? !first : first;
    bool normalizedSecond = spec.invertSecond ? !second : second;
    bool normalizedThird = spec.invertThird ? !third : third;
    int32_t score = static_cast<int32_t>(normalizedFirst) * 4 + static_cast<int32_t>(normalizedSecond) * 2 +
                    static_cast<int32_t>(normalizedThird);
    bool passed = score >= 3;

    return CreateBooleanSummary(env, spec.name, spec.route, score, passed);
}

static napi_value TestBooleanCase16(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[0], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[1], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[2], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto& spec = g_booleanCaseSpecs[15];
    bool normalizedFirst = spec.invertFirst ? !first : first;
    bool normalizedSecond = spec.invertSecond ? !second : second;
    bool normalizedThird = spec.invertThird ? !third : third;
    int32_t score = static_cast<int32_t>(normalizedFirst) * 4 + static_cast<int32_t>(normalizedSecond) * 2 +
                    static_cast<int32_t>(normalizedThird);
    bool passed = score >= 3;

    return CreateBooleanSummary(env, spec.name, spec.route, score, passed);
}

static napi_value TestBooleanCase17(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[0], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[1], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[2], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto& spec = g_booleanCaseSpecs[16];
    bool normalizedFirst = spec.invertFirst ? !first : first;
    bool normalizedSecond = spec.invertSecond ? !second : second;
    bool normalizedThird = spec.invertThird ? !third : third;
    int32_t score = static_cast<int32_t>(normalizedFirst) * 4 + static_cast<int32_t>(normalizedSecond) * 2 +
                    static_cast<int32_t>(normalizedThird);
    bool passed = score >= 3;

    return CreateBooleanSummary(env, spec.name, spec.route, score, passed);
}

static napi_value TestBooleanCase18(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[0], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[1], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[2], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto& spec = g_booleanCaseSpecs[17];
    bool normalizedFirst = spec.invertFirst ? !first : first;
    bool normalizedSecond = spec.invertSecond ? !second : second;
    bool normalizedThird = spec.invertThird ? !third : third;
    int32_t score = static_cast<int32_t>(normalizedFirst) * 4 + static_cast<int32_t>(normalizedSecond) * 2 +
                    static_cast<int32_t>(normalizedThird);
    bool passed = score >= 3;

    return CreateBooleanSummary(env, spec.name, spec.route, score, passed);
}

static napi_value TestBooleanCase19(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[0], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[1], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[2], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto& spec = g_booleanCaseSpecs[18];
    bool normalizedFirst = spec.invertFirst ? !first : first;
    bool normalizedSecond = spec.invertSecond ? !second : second;
    bool normalizedThird = spec.invertThird ? !third : third;
    int32_t score = static_cast<int32_t>(normalizedFirst) * 4 + static_cast<int32_t>(normalizedSecond) * 2 +
                    static_cast<int32_t>(normalizedThird);
    bool passed = score >= 3;

    return CreateBooleanSummary(env, spec.name, spec.route, score, passed);
}

static napi_value TestBooleanCase20(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    bool first = false;
    bool second = false;
    bool third = false;
    if (argc < 3) {
        napi_throw_type_error(env, nullptr, "three boolean values are required");
        return nullptr;
    }
    if (!ReadBool(env, args[0], "first must be a boolean", &first)) {
        return nullptr;
    }
    if (!ReadBool(env, args[1], "second must be a boolean", &second)) {
        return nullptr;
    }
    if (!ReadBool(env, args[2], "third must be a boolean", &third)) {
        return nullptr;
    }

    const auto& spec = g_booleanCaseSpecs[19];
    bool normalizedFirst = spec.invertFirst ? !first : first;
    bool normalizedSecond = spec.invertSecond ? !second : second;
    bool normalizedThird = spec.invertThird ? !third : third;
    int32_t score = static_cast<int32_t>(normalizedFirst) * 4 + static_cast<int32_t>(normalizedSecond) * 2 +
                    static_cast<int32_t>(normalizedThird);
    bool passed = score >= 3;

    return CreateBooleanSummary(env, spec.name, spec.route, score, passed);
}

}  // namespace

static napi_value InitBranch05Boolean(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[] = {
        DECLARE_NAPI_FUNCTION("testBooleanCase01", TestBooleanCase01),
        DECLARE_NAPI_FUNCTION("testBooleanCase02", TestBooleanCase02),
        DECLARE_NAPI_FUNCTION("testBooleanCase03", TestBooleanCase03),
        DECLARE_NAPI_FUNCTION("testBooleanCase04", TestBooleanCase04),
        DECLARE_NAPI_FUNCTION("testBooleanCase05", TestBooleanCase05),
        DECLARE_NAPI_FUNCTION("testBooleanCase06", TestBooleanCase06),
        DECLARE_NAPI_FUNCTION("testBooleanCase07", TestBooleanCase07),
        DECLARE_NAPI_FUNCTION("testBooleanCase08", TestBooleanCase08),
        DECLARE_NAPI_FUNCTION("testBooleanCase09", TestBooleanCase09),
        DECLARE_NAPI_FUNCTION("testBooleanCase10", TestBooleanCase10),
        DECLARE_NAPI_FUNCTION("testBooleanCase11", TestBooleanCase11),
        DECLARE_NAPI_FUNCTION("testBooleanCase12", TestBooleanCase12),
        DECLARE_NAPI_FUNCTION("testBooleanCase13", TestBooleanCase13),
        DECLARE_NAPI_FUNCTION("testBooleanCase14", TestBooleanCase14),
        DECLARE_NAPI_FUNCTION("testBooleanCase15", TestBooleanCase15),
        DECLARE_NAPI_FUNCTION("testBooleanCase16", TestBooleanCase16),
        DECLARE_NAPI_FUNCTION("testBooleanCase17", TestBooleanCase17),
        DECLARE_NAPI_FUNCTION("testBooleanCase18", TestBooleanCase18),
        DECLARE_NAPI_FUNCTION("testBooleanCase19", TestBooleanCase19),
        DECLARE_NAPI_FUNCTION("testBooleanCase20", TestBooleanCase20),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(descriptors) / sizeof(descriptors[0]), descriptors));
    return exports;
}

static napi_module g_branch05BooleanModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitBranch05Boolean,
    .nm_modname = "boolean_suite",
    .nm_priv = nullptr,
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterBranch05BooleanModule(void)
{
    napi_module_register(&g_branch05BooleanModule);
}
