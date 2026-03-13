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

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARG_FIRST = 0;
constexpr size_t ARG_SECOND = 1;
}

static napi_value Add(napi_env env, napi_callback_info info)
{
    size_t requireArgc = ARGC_TWO;
    size_t argc = ARGC_TWO;
    napi_value args[ARGC_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[ARG_FIRST], &valuetype0));

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, args[ARG_SECOND], &valuetype1));

    NAPI_ASSERT(env, valuetype0 == napi_number && valuetype1 == napi_number,
                "Wrong argument type. Numbers expected.");

    double value0;
    NAPI_CALL(env, napi_get_value_double(env, args[ARG_FIRST], &value0));

    double value1;
    NAPI_CALL(env, napi_get_value_double(env, args[ARG_SECOND], &value1));

    napi_value sum;
    NAPI_CALL(env, napi_create_double(env, value0 + value1, &sum));

    return sum;
}

static napi_value Subtract(napi_env env, napi_callback_info info)
{
    size_t requireArgc = ARGC_TWO;
    size_t argc = ARGC_TWO;
    napi_value args[ARGC_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[ARG_FIRST], &valuetype0));

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, args[ARG_SECOND], &valuetype1));

    NAPI_ASSERT(env, valuetype0 == napi_number && valuetype1 == napi_number,
                "Wrong argument type. Numbers expected.");

    double value0;
    NAPI_CALL(env, napi_get_value_double(env, args[ARG_FIRST], &value0));

    double value1;
    NAPI_CALL(env, napi_get_value_double(env, args[ARG_SECOND], &value1));

    napi_value result;
    NAPI_CALL(env, napi_create_double(env, value0 - value1, &result));

    return result;
}

static napi_value Multiply(napi_env env, napi_callback_info info)
{
    size_t requireArgc = ARGC_TWO;
    size_t argc = ARGC_TWO;
    napi_value args[ARGC_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[ARG_FIRST], &valuetype0));

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, args[ARG_SECOND], &valuetype1));

    NAPI_ASSERT(env, valuetype0 == napi_number && valuetype1 == napi_number,
                "Wrong argument type. Numbers expected.");

    double value0;
    NAPI_CALL(env, napi_get_value_double(env, args[ARG_FIRST], &value0));

    double value1;
    NAPI_CALL(env, napi_get_value_double(env, args[ARG_SECOND], &value1));

    napi_value result;
    NAPI_CALL(env, napi_create_double(env, value0 * value1, &result));

    return result;
}

static napi_value Divide(napi_env env, napi_callback_info info)
{
    size_t requireArgc = ARGC_TWO;
    size_t argc = ARGC_TWO;
    napi_value args[ARGC_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[ARG_FIRST], &valuetype0));

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, args[ARG_SECOND], &valuetype1));

    NAPI_ASSERT(env, valuetype0 == napi_number && valuetype1 == napi_number,
                "Wrong argument type. Numbers expected.");

    double value0;
    NAPI_CALL(env, napi_get_value_double(env, args[ARG_FIRST], &value0));

    double value1;
    NAPI_CALL(env, napi_get_value_double(env, args[ARG_SECOND], &value1));

    constexpr double divisorZero = 0.0;
    NAPI_ASSERT(env, value1 != divisorZero, "Division by zero is not allowed.");

    napi_value result;

    if (value1 != 0) {
        NAPI_CALL(env, napi_create_double(env, value0 / value1, &result));
    }

    return result;
}

static napi_value Sqrt(napi_env env, napi_callback_info info)
{
    size_t requireArgc = ARGC_ONE;
    size_t argc = ARGC_ONE;
    napi_value args[ARGC_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args[ARG_FIRST], &valuetype));

    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");

    double value;
    NAPI_CALL(env, napi_get_value_double(env, args[ARG_FIRST], &value));

    constexpr double minSqrtValue = 0.0;
    NAPI_ASSERT(env, value >= minSqrtValue, "Cannot calculate square root of negative number.");

    napi_value result;
    NAPI_CALL(env, napi_create_double(env, sqrt(value), &result));

    return result;
}

static napi_value Pow(napi_env env, napi_callback_info info)
{
    size_t requireArgc = ARGC_TWO;
    size_t argc = ARGC_TWO;
    napi_value args[ARGC_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[ARG_FIRST], &valuetype0));

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, args[ARG_SECOND], &valuetype1));

    NAPI_ASSERT(env, valuetype0 == napi_number && valuetype1 == napi_number,
                "Wrong argument type. Numbers expected.");

    double base;
    NAPI_CALL(env, napi_get_value_double(env, args[ARG_FIRST], &base));

    double exponent;
    NAPI_CALL(env, napi_get_value_double(env, args[ARG_SECOND], &exponent));

    napi_value result;
    NAPI_CALL(env, napi_create_double(env, pow(base, exponent), &result));

    return result;
}

static napi_value Abs(napi_env env, napi_callback_info info)
{
    size_t requireArgc = ARGC_ONE;
    size_t argc = ARGC_ONE;
    napi_value args[ARGC_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args[ARG_FIRST], &valuetype));

    NAPI_ASSERT(env, valuetype == napi_number, "Wrong argument type. Number expected.");

    double value;
    NAPI_CALL(env, napi_get_value_double(env, args[ARG_FIRST], &value));

    napi_value result;
    NAPI_CALL(env, napi_create_double(env, fabs(value), &result));

    return result;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("add", Add),
        DECLARE_NAPI_FUNCTION("subtract", Subtract),
        DECLARE_NAPI_FUNCTION("multiply", Multiply),
        DECLARE_NAPI_FUNCTION("divide", Divide),
        DECLARE_NAPI_FUNCTION("sqrt", Sqrt),
        DECLARE_NAPI_FUNCTION("pow", Pow),
        DECLARE_NAPI_FUNCTION("abs", Abs),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    return exports;
}
EXTERN_C_END

static napi_module mathModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "math",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void MathRegisterModule(void)
{
    napi_module_register(&mathModule);
}