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

#include <cmath>
#include <cstdint>
#include <limits>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

static constexpr int REQUIRED_ARGS_ONE = 1;
static constexpr int REQUIRED_ARGS_TWO = 2;
static constexpr int REQUIRED_ARGS_THREE = 3;
static constexpr int ARG_INDEX_ZERO = 0;
static constexpr int ARG_INDEX_ONE = 1;
static constexpr int ARG_INDEX_TWO = 2;
static constexpr double SAFE_INTEGER_MAX = 9007199254740991.0;
static constexpr double SAFE_INTEGER_MIN = -9007199254740991.0;

static napi_value IsInteger(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));

    bool isInt = (valueType == napi_number);
    if (isInt) {
        double value = 0;
        NAPI_CALL(env, napi_get_value_double(env, argv[0], &value));
        isInt = (std::trunc(value) == value) &&
                (value >= std::numeric_limits<int64_t>::min()) &&
                (value <= std::numeric_limits<int64_t>::max());
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, isInt, &result));
    return result;
}

static napi_value IsSafeInteger(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));

    bool isSafe = (valueType == napi_number);
    if (isSafe) {
        double value = 0;
        NAPI_CALL(env, napi_get_value_double(env, argv[0], &value));
        isSafe = (std::trunc(value) == value) &&
                (value >= SAFE_INTEGER_MIN) &&
                (value <= SAFE_INTEGER_MAX);
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, isSafe, &result));
    return result;
}

static napi_value ToInteger(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));

    napi_value result = nullptr;
    if (valueType == napi_number) {
        double value = 0;
        NAPI_CALL(env, napi_get_value_double(env, argv[0], &value));
        NAPI_CALL(env, napi_create_int64(env, static_cast<int64_t>(std::trunc(value)),
                                         &result));
    } else {
        NAPI_CALL(env, napi_create_int64(env, 0, &result));
    }
    return result;
}

static napi_value ToSafeInteger(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));

    napi_value result = nullptr;
    if (valueType == napi_number) {
        double value = 0;
        NAPI_CALL(env, napi_get_value_double(env, argv[0], &value));
        
        if (!std::isfinite(value)) {
            NAPI_CALL(env, napi_create_int64(env, 0, &result));
        } else {
            int64_t intValue = static_cast<int64_t>(std::trunc(value));
            if (intValue > SAFE_INTEGER_MAX) {
                NAPI_CALL(env, napi_create_int64(env, SAFE_INTEGER_MAX, &result));
            } else if (intValue < SAFE_INTEGER_MIN) {
                NAPI_CALL(env, napi_create_int64(env, SAFE_INTEGER_MIN, &result));
            } else {
                NAPI_CALL(env, napi_create_int64(env, intValue, &result));
            }
        }
    } else {
        NAPI_CALL(env, napi_create_int64(env, 0, &result));
    }
    return result;
}

static napi_value Clamp(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value argv[3] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: value, min, max");

    double value = 0;
    double min = 0;
    double max = 0;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ZERO], &value));
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ONE], &min));
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_TWO], &max));

    NAPI_ASSERT(env, min <= max, "Min must be less than or equal to max");

    double clampedValue = std::min(std::max(value, min), max);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_double(env, clampedValue, &result));
    return result;
}

static napi_value Max(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments");

    double value1 = 0;
    double value2 = 0;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ZERO], &value1));
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ONE], &value2));

    double maxValue = std::max(value1, value2);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_double(env, maxValue, &result));
    return result;
}

static napi_value Min(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments");

    double value1 = 0;
    double value2 = 0;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ZERO], &value1));
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ONE], &value2));

    double minValue = std::min(value1, value2);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_double(env, minValue, &result));
    return result;
}

static napi_value Round(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    double value = 0;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ZERO], &value));

    double rounded = std::round(value);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_double(env, rounded, &result));
    return result;
}

static napi_value Floor(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    double value = 0;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ZERO], &value));

    double floored = std::floor(value);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_double(env, floored, &result));
    return result;
}

static napi_value Ceil(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    double value = 0;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ZERO], &value));

    double ceiled = std::ceil(value);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_double(env, ceiled, &result));
    return result;
}

static napi_value Abs(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    double value = 0;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ZERO], &value));

    double absValue = std::abs(value);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_double(env, absValue, &result));
    return result;
}

static napi_value Sign(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    double value = 0;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ZERO], &value));

    double signValue = 0;
    if (value > 0) {
        signValue = 1;
    } else if (value < 0) {
        signValue = -1;
    } else {
        signValue = 0;
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_double(env, signValue, &result));
    return result;
}

static napi_value IsFinite(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));

    bool isFinite = false;
    if (valueType == napi_number) {
        double value = 0;
        NAPI_CALL(env, napi_get_value_double(env, argv[0], &value));
        isFinite = std::isfinite(value);
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, isFinite, &result));
    return result;
}

static napi_value IsNaN(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));

    bool isNaN = false;
    if (valueType == napi_number) {
        double value = 0;
        NAPI_CALL(env, napi_get_value_double(env, argv[0], &value));
        isNaN = std::isnan(value);
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, isNaN, &result));
    return result;
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("isInteger", IsInteger),
        DECLARE_NAPI_FUNCTION("isSafeInteger", IsSafeInteger),
        DECLARE_NAPI_FUNCTION("toInteger", ToInteger),
        DECLARE_NAPI_FUNCTION("toSafeInteger", ToSafeInteger),
        DECLARE_NAPI_FUNCTION("clamp", Clamp),
        DECLARE_NAPI_FUNCTION("max", Max),
        DECLARE_NAPI_FUNCTION("min", Min),
        DECLARE_NAPI_FUNCTION("round", Round),
        DECLARE_NAPI_FUNCTION("floor", Floor),
        DECLARE_NAPI_FUNCTION("ceil", Ceil),
        DECLARE_NAPI_FUNCTION("abs", Abs),
        DECLARE_NAPI_FUNCTION("sign", Sign),
        DECLARE_NAPI_FUNCTION("isFinite", IsFinite),
        DECLARE_NAPI_FUNCTION("isNaN", IsNaN),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
                                      sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module numberModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "number",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void NumberRegisterModule(void)
{
    napi_module_register(&numberModule);
}
