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

#include <cstddef>
#include <cmath>
#include <algorithm>
#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

static constexpr int REQUIRED_ARGS_ONE = 1;
static constexpr int REQUIRED_ARGS_TWO = 2;
static constexpr int REQUIRED_ARGS_THREE = 3;
static constexpr int ARG_INDEX_ZERO = 0;
static constexpr int ARG_INDEX_ONE = 1;
static constexpr int ARG_INDEX_TWO = 2;
static constexpr int PRECISION_DIGITS = 6;
static constexpr int BUFFER_SIZE = 50;
static constexpr int MIN_PRIME_NUMBER = 2;
static constexpr int PRIME_START = 2;

static double ExtractNumberValue(napi_env env, napi_value value)
{
    napi_valuetype type = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, value, &type), 0.0);

    switch (type) {
        case napi_number: {
            double result = 0.0;
            NAPI_CALL_BASE(env, napi_get_value_double(env, value, &result), 0.0);
            return result;
        }
        case napi_string: {
            size_t bufferSize = 0;
            NAPI_CALL_BASE(env, napi_get_value_string_utf8(env, value, nullptr, 0,
                                                         &bufferSize), 0.0);

            std::string buffer(bufferSize + 1, '\0');
            size_t copiedLength = 0;
            NAPI_CALL_BASE(env, napi_get_value_string_utf8(env, value, &buffer[0],
                                                         bufferSize, &copiedLength), 0.0);
            buffer.resize(copiedLength);

            size_t pos = 0;
            double result = std::stod(buffer, &pos);
            return result;
        }
        default:
            return 0.0;
    }
}

static napi_value CreateNumber(napi_env env, double value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_double(env, value, &result));
    return result;
}

static napi_value AddNumbers(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: num1, num2");

    double num1 = ExtractNumberValue(env, argv[ARG_INDEX_ZERO]);
    double num2 = ExtractNumberValue(env, argv[ARG_INDEX_ONE]);

    double result = num1 + num2;
    return CreateNumber(env, result);
}

static napi_value SubtractNumbers(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: num1, num2");

    double num1 = ExtractNumberValue(env, argv[ARG_INDEX_ZERO]);
    double num2 = ExtractNumberValue(env, argv[ARG_INDEX_ONE]);

    double result = num1 - num2;
    return CreateNumber(env, result);
}

static napi_value MultiplyNumbers(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: num1, num2");

    double num1 = ExtractNumberValue(env, argv[ARG_INDEX_ZERO]);
    double num2 = ExtractNumberValue(env, argv[ARG_INDEX_ONE]);

    double result = num1 * num2;
    return CreateNumber(env, result);
}

static napi_value DivideNumbers(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: num1, num2");

    double num1 = ExtractNumberValue(env, argv[ARG_INDEX_ZERO]);
    double num2 = ExtractNumberValue(env, argv[ARG_INDEX_ONE]);

    NAPI_ASSERT(env, num2 != 0.0, "Cannot divide by zero");

    double result = num1 / num2;
    return CreateNumber(env, result);
}

static napi_value SquareRoot(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: number");

    double number = ExtractNumberValue(env, argv[ARG_INDEX_ZERO]);

    NAPI_ASSERT(env, number >= 0.0, "Cannot calculate square root of negative number");

    double result = sqrt(number);
    return CreateNumber(env, result);
}

static napi_value AbsoluteValue(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: number");

    double number = ExtractNumberValue(env, argv[ARG_INDEX_ZERO]);

    double result = fabs(number);
    return CreateNumber(env, result);
}

static napi_value RoundNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: number");

    double number = ExtractNumberValue(env, argv[ARG_INDEX_ZERO]);

    double result = round(number);
    return CreateNumber(env, result);
}

static napi_value FloorNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: number");

    double number = ExtractNumberValue(env, argv[ARG_INDEX_ZERO]);

    double result = floor(number);
    return CreateNumber(env, result);
}

static napi_value CeilingNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: number");

    double number = ExtractNumberValue(env, argv[ARG_INDEX_ZERO]);

    double result = ceil(number);
    return CreateNumber(env, result);
}

static napi_value Power(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: base, exponent");

    double base = ExtractNumberValue(env, argv[ARG_INDEX_ZERO]);
    double exponent = ExtractNumberValue(env, argv[ARG_INDEX_ONE]);

    double result = pow(base, exponent);
    return CreateNumber(env, result);
}

static napi_value IsEven(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: number");

    double number = ExtractNumberValue(env, argv[ARG_INDEX_ZERO]);

    bool result = (fmod(number, 2.0) == 0.0);

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, result, &resultValue));
    return resultValue;
}

static napi_value IsOdd(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: number");

    double number = ExtractNumberValue(env, argv[ARG_INDEX_ZERO]);

    bool result = (fmod(number, 2.0) != 0.0);

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, result, &resultValue));
    return resultValue;
}

static napi_value IsPrime(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: number");

    double number = ExtractNumberValue(env, argv[ARG_INDEX_ZERO]);
    if (number < MIN_PRIME_NUMBER || fmod(number, 1.0) != 0.0) {
        bool result = false;
        napi_value resultValue = nullptr;
        NAPI_CALL(env, napi_get_boolean(env, result, &resultValue));
        return resultValue;
    }

    int64_t intNumber = static_cast<int64_t>(number);
    for (int64_t i = PRIME_START; i <= sqrt(number); ++i) {
        if (intNumber % i == 0) {
            bool result = false;
            napi_value resultValue = nullptr;
            NAPI_CALL(env, napi_get_boolean(env, result, &resultValue));
            return resultValue;
        }
    }

    bool result = true;
    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, result, &resultValue));
    return resultValue;
}

static napi_value Clamp(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: value, min, max");

    double value = ExtractNumberValue(env, argv[ARG_INDEX_ZERO]);
    double minVal = ExtractNumberValue(env, argv[ARG_INDEX_ONE]);
    double maxVal = ExtractNumberValue(env, argv[ARG_INDEX_TWO]);

    NAPI_ASSERT(env, minVal <= maxVal, "Minimum value must be less than or equal to maximum value");

    double result = std::max(minVal, std::min(value, maxVal));
    return CreateNumber(env, result);
}

static napi_value ToPrecision(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: number, precision");

    double number = ExtractNumberValue(env, argv[ARG_INDEX_ZERO]);

    int32_t precision = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[ARG_INDEX_ONE], &precision));
    NAPI_ASSERT(env, precision >= 0 && precision <= PRECISION_DIGITS,
                "Precision must be between 0 and 6");

    char format[20];
    int retFormat = snprintf_s(format, sizeof(format), sizeof(format) - 1, "%%.%df", precision);
    if (retFormat < 0 || retFormat >= static_cast<int>(sizeof(format))) {
        NAPI_CALL(env, napi_throw_error(env, "FORMAT_ERROR", "Failed to format precision"));
        return nullptr;
    }
    char buffer[BUFFER_SIZE];
    int ret = snprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1, format, number);
    if (ret < 0 || ret >= static_cast<int>(sizeof(buffer))) {
        NAPI_CALL(env, napi_throw_error(env, "FORMAT_ERROR", "Failed to format number"));
        return nullptr;
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, buffer, strlen(buffer), &result));
    return result;
}

static napi_value ParseNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: string");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ZERO], &type));
    NAPI_ASSERT(env, type == napi_string, "Argument must be a string");

    size_t bufferSize = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[ARG_INDEX_ZERO], nullptr, 0,
                                                &bufferSize));

    std::string buffer(bufferSize + 1, '\0');
    size_t copiedLength = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[ARG_INDEX_ZERO], &buffer[0],
                                                bufferSize, &copiedLength));
    buffer.resize(copiedLength);

    size_t pos = 0;
    double result = std::stod(buffer, &pos);
    return CreateNumber(env, result);
}

static napi_value FormatNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: number");

    double number = ExtractNumberValue(env, argv[ARG_INDEX_ZERO]);

    char buffer[BUFFER_SIZE];
    int ret = snprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1, "%.6f", number);
    if (ret < 0 || ret >= static_cast<int>(sizeof(buffer))) {
        NAPI_CALL(env, napi_throw_error(env, "FORMAT_ERROR", "Failed to format number"));
        return nullptr;
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, buffer, strlen(buffer), &result));
    return result;
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("add", AddNumbers),
        DECLARE_NAPI_FUNCTION("subtract", SubtractNumbers),
        DECLARE_NAPI_FUNCTION("multiply", MultiplyNumbers),
        DECLARE_NAPI_FUNCTION("divide", DivideNumbers),
        DECLARE_NAPI_FUNCTION("sqrt", SquareRoot),
        DECLARE_NAPI_FUNCTION("abs", AbsoluteValue),
        DECLARE_NAPI_FUNCTION("round", RoundNumber),
        DECLARE_NAPI_FUNCTION("floor", FloorNumber),
        DECLARE_NAPI_FUNCTION("ceil", CeilingNumber),
        DECLARE_NAPI_FUNCTION("power", Power),
        DECLARE_NAPI_FUNCTION("isEven", IsEven),
        DECLARE_NAPI_FUNCTION("isOdd", IsOdd),
        DECLARE_NAPI_FUNCTION("isPrime", IsPrime),
        DECLARE_NAPI_FUNCTION("clamp", Clamp),
        DECLARE_NAPI_FUNCTION("toPrecision", ToPrecision),
        DECLARE_NAPI_FUNCTION("parse", ParseNumber),
        DECLARE_NAPI_FUNCTION("format", FormatNumber),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
                                      sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module numberSuiteModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "numberSuite",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void NumberSuiteRegisterModule(void)
{
    napi_module_register(&numberSuiteModule);
}
