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

#include "napi/native_node_api.h"

#include "rational_suite_common.h"

static int64_t NegateInt64(int64_t value)
{
    return static_cast<int64_t>(static_cast<uint64_t>(0) - static_cast<uint64_t>(value));
}

int64_t GcdInt64(int64_t a, int64_t b)
{
    if (a < 0) {
        a = NegateInt64(a);
    }
    if (b < 0) {
        b = NegateInt64(b);
    }
    if (b == 0) {
        return a;
    }
    return GcdInt64(b, a % b);
}

RationalValue NormalizeRational(RationalValue value)
{
    if (value.num == 0) {
        value.den = 1;
        return value;
    }
    if (value.den < 0) {
        value.num = -value.num;
        value.den = -value.den;
    }

    int64_t common = GcdInt64(value.num, value.den);
    if (common == 0) {
        return value;
    }
    if (common > 1) {
        value.num /= common;
        value.den /= common;
    }
    return value;
}

int64_t ComputeFloorNum(const RationalValue& value)
{
    int64_t quotient = value.num / value.den;
    int64_t remainder = value.num % value.den;
    if (remainder < 0) {
        quotient += -1;
    }
    return quotient;
}

napi_status ExtractRational(napi_env env, napi_value value, RationalValue* result)
{
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, value, &type);
    if (status != napi_ok) {
        return status;
    }
    if (type != napi_object) {
        return napi_invalid_arg;
    }

    napi_value prop = nullptr;
    status = napi_get_named_property(env, value, NUMERATOR_PROPERTY_NAME, &prop);
    if (status != napi_ok) {
        return status;
    }
    int64_t num = 0;
    status = napi_get_value_int64(env, prop, &num);
    if (status != napi_ok) {
        return status;
    }

    status = napi_get_named_property(env, value, DENOMINATOR_PROPERTY_NAME, &prop);
    if (status != napi_ok) {
        return status;
    }
    int64_t den = 0;
    status = napi_get_value_int64(env, prop, &den);
    if (status != napi_ok) {
        return status;
    }

    if (num > RATIONAL_MAX_COMPONENT) {
        return napi_invalid_arg;
    }
    if (num < RATIONAL_MIN_COMPONENT) {
        return napi_invalid_arg;
    }
    if (den > RATIONAL_MAX_COMPONENT) {
        return napi_invalid_arg;
    }
    if (den < DENOMINATOR_MIN) {
        return napi_invalid_arg;
    }

    result->num = num;
    result->den = den;
    return napi_ok;
}

napi_status ExtractInt64Arg(napi_env env, napi_value value, int64_t* result)
{
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, value, &type);
    if (status != napi_ok) {
        return status;
    }

    bool isNumber = (type == napi_number);
    if (!isNumber) {
        return napi_invalid_arg;
    }
    return napi_get_value_int64(env, value, result);
}

napi_status ExtractDoubleArg(napi_env env, napi_value value, double* result)
{
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, value, &type);
    if (status != napi_ok) {
        return status;
    }

    bool isNumber = (type == napi_number);
    if (!isNumber) {
        return napi_invalid_arg;
    }
    return napi_get_value_double(env, value, result);
}

napi_value CreateRationalValue(napi_env env, const RationalValue& value)
{
    napi_value object = nullptr;
    NAPI_CALL(env, napi_create_object(env, &object));

    napi_value numValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, value.num, &numValue));
    NAPI_CALL(env, napi_set_named_property(env, object, NUMERATOR_PROPERTY_NAME, numValue));

    napi_value denValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, value.den, &denValue));
    NAPI_CALL(env, napi_set_named_property(env, object, DENOMINATOR_PROPERTY_NAME, denValue));
    return object;
}

napi_value CreateInt64Value(napi_env env, int64_t value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int64(env, value, &result));
    return result;
}

napi_value CreateDoubleValue(napi_env env, double value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_double(env, value, &result));
    return result;
}

napi_value CreateBoolValue(napi_env env, bool value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, value, &result));
    return result;
}

napi_value CreateStringValue(napi_env env, const std::string& value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.size(), &result));
    return result;
}

napi_value Create(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: num, den");

    int64_t num = 0;
    NAPI_CALL_BASE(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], &num), nullptr);

    int64_t den = 0;
    NAPI_CALL_BASE(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], &den), nullptr);

    NAPI_ASSERT(env, num <= RATIONAL_MAX_COMPONENT, "Numerator too large");
    NAPI_ASSERT(env, num >= RATIONAL_MIN_COMPONENT, "Numerator too small");
    NAPI_ASSERT(env, den >= DENOMINATOR_MIN, "Denominator must be positive");
    NAPI_ASSERT(env, den <= RATIONAL_MAX_COMPONENT, "Denominator too large");

    RationalValue value { num, den };
    RationalValue result = NormalizeRational(value);
    return CreateRationalValue(env, result);
}

napi_value Add(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: a, b");

    RationalValue a { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &a), nullptr);

    RationalValue b { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ONE], &b), nullptr);

    int64_t left = 0;
    int64_t right = 0;
    int64_t commonDen = 0;
    bool mulOverflow = __builtin_mul_overflow(a.num, b.den, &left) ||
                       __builtin_mul_overflow(b.num, a.den, &right) ||
                       __builtin_mul_overflow(a.den, b.den, &commonDen);
    NAPI_ASSERT(env, !mulOverflow, "Addition overflow");

    int64_t sumNum = 0;
    bool addOverflow = __builtin_add_overflow(left, right, &sumNum);
    NAPI_ASSERT(env, !addOverflow, "Addition overflow");

    RationalValue sum { sumNum, commonDen };
    RationalValue result = NormalizeRational(sum);
    return CreateRationalValue(env, result);
}

napi_value Subtract(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: a, b");

    RationalValue a { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &a), nullptr);

    RationalValue b { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ONE], &b), nullptr);

    RationalValue negated { -b.num, b.den };
    int64_t left = 0;
    int64_t right = 0;
    int64_t commonDen = 0;
    bool mulOverflow = __builtin_mul_overflow(a.num, negated.den, &left) ||
                       __builtin_mul_overflow(negated.num, a.den, &right) ||
                       __builtin_mul_overflow(a.den, negated.den, &commonDen);
    NAPI_ASSERT(env, !mulOverflow, "Subtraction overflow");

    int64_t diffNum = 0;
    bool addOverflow = __builtin_add_overflow(left, right, &diffNum);
    NAPI_ASSERT(env, !addOverflow, "Subtraction overflow");

    RationalValue difference { diffNum, commonDen };
    RationalValue result = NormalizeRational(difference);
    return CreateRationalValue(env, result);
}

napi_value Multiply(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: a, b");

    RationalValue a { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &a), nullptr);

    RationalValue b { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ONE], &b), nullptr);

    int64_t num = 0;
    bool numOverflow = __builtin_mul_overflow(a.num, b.num, &num);
    NAPI_ASSERT(env, !numOverflow, "Multiplication overflow");

    int64_t den = 0;
    bool denOverflow = __builtin_mul_overflow(a.den, b.den, &den);
    NAPI_ASSERT(env, !denOverflow, "Multiplication overflow");

    RationalValue product { num, den };
    RationalValue result = NormalizeRational(product);
    return CreateRationalValue(env, result);
}

napi_value Divide(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: a, b");

    RationalValue a { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &a), nullptr);

    RationalValue b { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ONE], &b), nullptr);

    NAPI_ASSERT(env, b.num != 0, "Cannot divide by zero rational");

    RationalValue inverted { b.den, b.num };
    RationalValue inverse = NormalizeRational(inverted);

    int64_t num = 0;
    bool numOverflow = __builtin_mul_overflow(a.num, inverse.num, &num);
    NAPI_ASSERT(env, !numOverflow, "Division overflow");

    int64_t den = 0;
    bool denOverflow = __builtin_mul_overflow(a.den, inverse.den, &den);
    NAPI_ASSERT(env, !denOverflow, "Division overflow");

    RationalValue quotient { num, den };
    RationalValue result = NormalizeRational(quotient);
    return CreateRationalValue(env, result);
}

napi_value Negate(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rational");

    RationalValue value { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &value), nullptr);

    RationalValue negated { -value.num, value.den };
    RationalValue result = NormalizeRational(negated);
    return CreateRationalValue(env, result);
}

napi_value Inverse(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rational");

    RationalValue value { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &value), nullptr);

    NAPI_ASSERT(env, value.num != 0, "Cannot invert zero rational");

    RationalValue inverted { value.den, value.num };
    RationalValue result = NormalizeRational(inverted);
    return CreateRationalValue(env, result);
}

napi_value Abs(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rational");

    RationalValue value { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &value), nullptr);

    int64_t num = (value.num < 0) ? -value.num : value.num;

    RationalValue absolute { num, value.den };
    RationalValue result = NormalizeRational(absolute);
    return CreateRationalValue(env, result);
}

napi_value Reduce(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rational");

    RationalValue value { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &value), nullptr);

    RationalValue result = NormalizeRational(value);
    return CreateRationalValue(env, result);
}

napi_value Compare(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: a, b");

    RationalValue a { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &a), nullptr);

    RationalValue b { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ONE], &b), nullptr);

    int64_t left = 0;
    bool leftOverflow = __builtin_mul_overflow(a.num, b.den, &left);
    NAPI_ASSERT(env, !leftOverflow, "Comparison overflow");

    int64_t right = 0;
    bool rightOverflow = __builtin_mul_overflow(b.num, a.den, &right);
    NAPI_ASSERT(env, !rightOverflow, "Comparison overflow");

    int64_t result = COMPARE_EQUAL;
    if (left < right) {
        result = COMPARE_LESS;
    } else if (left > right) {
        result = COMPARE_GREATER;
    }
    return CreateInt64Value(env, result);
}

napi_value IsEqual(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: a, b");

    RationalValue a { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &a), nullptr);

    RationalValue b { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ONE], &b), nullptr);

    int64_t left = 0;
    bool leftOverflow = __builtin_mul_overflow(a.num, b.den, &left);
    NAPI_ASSERT(env, !leftOverflow, "Comparison overflow");

    int64_t right = 0;
    bool rightOverflow = __builtin_mul_overflow(b.num, a.den, &right);
    NAPI_ASSERT(env, !rightOverflow, "Comparison overflow");

    return CreateBoolValue(env, left == right);
}

napi_value IsZero(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rational");

    RationalValue value { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &value), nullptr);

    RationalValue normalized = NormalizeRational(value);
    return CreateBoolValue(env, normalized.num == 0);
}

napi_value IsInteger(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rational");

    RationalValue value { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &value), nullptr);

    RationalValue normalized = NormalizeRational(value);
    return CreateBoolValue(env, normalized.den == DENOMINATOR_MIN);
}

napi_value IsPositive(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rational");

    RationalValue value { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &value), nullptr);

    RationalValue normalized = NormalizeRational(value);
    return CreateBoolValue(env, normalized.num > 0);
}

napi_value IsNegative(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rational");

    RationalValue value { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &value), nullptr);

    RationalValue normalized = NormalizeRational(value);
    return CreateBoolValue(env, normalized.num < 0);
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("create", Create),
        DECLARE_NAPI_FUNCTION("add", Add),
        DECLARE_NAPI_FUNCTION("subtract", Subtract),
        DECLARE_NAPI_FUNCTION("multiply", Multiply),
        DECLARE_NAPI_FUNCTION("divide", Divide),
        DECLARE_NAPI_FUNCTION("negate", Negate),
        DECLARE_NAPI_FUNCTION("inverse", Inverse),
        DECLARE_NAPI_FUNCTION("abs", Abs),
        DECLARE_NAPI_FUNCTION("reduce", Reduce),
        DECLARE_NAPI_FUNCTION("compare", Compare),
        DECLARE_NAPI_FUNCTION("isEqual", IsEqual),
        DECLARE_NAPI_FUNCTION("isZero", IsZero),
        DECLARE_NAPI_FUNCTION("isInteger", IsInteger),
        DECLARE_NAPI_FUNCTION("isPositive", IsPositive),
        DECLARE_NAPI_FUNCTION("isNegative", IsNegative),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    NAPI_CALL_BASE(env, RegisterRationalOpsFunctions(env, exports), nullptr);
    return exports;
}

static napi_module rationalSuiteModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "rationalSuite",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RationalSuiteRegisterModule(void)
{
    napi_module_register(&rationalSuiteModule);
}
