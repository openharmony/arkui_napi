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

#include "rational_suite_common.h"

napi_value ToNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rational");

    RationalValue value { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &value), nullptr);

    RationalValue normalized = NormalizeRational(value);
    double num = static_cast<double>(normalized.num);
    double den = static_cast<double>(normalized.den);

    return CreateDoubleValue(env, num / den);
}

napi_value ToString(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rational");

    RationalValue value { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &value), nullptr);

    RationalValue normalized = NormalizeRational(value);
    std::string text = std::to_string(normalized.num);
    text += FRACTION_SLASH;
    text += std::to_string(normalized.den);

    return CreateStringValue(env, text);
}

napi_value ToMixedString(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rational");

    RationalValue value { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &value), nullptr);

    RationalValue normalized = NormalizeRational(value);
    int64_t absNum = (normalized.num < 0) ? -normalized.num : normalized.num;

    std::string text;
    if (absNum > normalized.den) {
        int64_t whole = ComputeFloorNum(normalized);
        int64_t remNum = normalized.num - whole * normalized.den;
        text = std::to_string(whole);
        text += MIXED_SEPARATOR;
        text += std::to_string(remNum);
        text += FRACTION_SLASH;
        text += std::to_string(normalized.den);
    } else {
        text = std::to_string(normalized.num);
        text += FRACTION_SLASH;
        text += std::to_string(normalized.den);
    }
    return CreateStringValue(env, text);
}

napi_value Numerator(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rational");

    RationalValue value { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &value), nullptr);

    RationalValue normalized = NormalizeRational(value);

    return CreateInt64Value(env, normalized.num);
}

napi_value Denominator(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rational");

    RationalValue value { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &value), nullptr);

    RationalValue normalized = NormalizeRational(value);

    return CreateInt64Value(env, normalized.den);
}

napi_value Floor(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rational");

    RationalValue value { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &value), nullptr);

    RationalValue normalized = NormalizeRational(value);
    int64_t whole = ComputeFloorNum(normalized);

    RationalValue result { whole, 1 };
    return CreateRationalValue(env, result);
}

napi_value Ceil(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rational");

    RationalValue value { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &value), nullptr);

    RationalValue normalized = NormalizeRational(value);
    RationalValue negated { -normalized.num, normalized.den };
    int64_t negatedFloor = ComputeFloorNum(negated);

    int64_t whole = -negatedFloor;
    RationalValue result { whole, 1 };
    return CreateRationalValue(env, result);
}

napi_value Round(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rational");

    RationalValue value { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &value), nullptr);

    RationalValue normalized = NormalizeRational(value);
    int64_t halfDen = normalized.den / ROUND_HALF_DENOMINATOR;

    int64_t shiftedNum = 0;
    bool overflow = __builtin_add_overflow(normalized.num, halfDen, &shiftedNum);
    NAPI_ASSERT(env, !overflow, "Rounding overflow");

    RationalValue shifted { shiftedNum, normalized.den };
    RationalValue shiftedNormalized = NormalizeRational(shifted);
    int64_t whole = ComputeFloorNum(shiftedNormalized);

    RationalValue result { whole, 1 };
    return CreateRationalValue(env, result);
}

napi_value Pow(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: rational, exponent");

    RationalValue base { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &base), nullptr);

    int64_t exponent = 0;
    NAPI_CALL_BASE(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], &exponent), nullptr);

    NAPI_ASSERT(env, exponent >= POW_MIN_EXPONENT, "Exponent must be non-negative");
    NAPI_ASSERT(env, exponent <= POW_MAX_EXPONENT, "Exponent too large");

    RationalValue normalizedBase = NormalizeRational(base);
    RationalValue acc { 1, 1 };
    for (int64_t i = 0; i < exponent; i += 1) {
        int64_t num = 0;
        bool numOverflow = __builtin_mul_overflow(acc.num, normalizedBase.num, &num);
        NAPI_ASSERT(env, !numOverflow, "Power overflow");

        int64_t den = 0;
        bool denOverflow = __builtin_mul_overflow(acc.den, normalizedBase.den, &den);
        NAPI_ASSERT(env, !denOverflow, "Power overflow");

        RationalValue product { num, den };
        acc = NormalizeRational(product);
    }
    return CreateRationalValue(env, acc);
}

napi_value Mediant(napi_env env, napi_callback_info info)
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
    bool numOverflow = __builtin_add_overflow(a.num, b.num, &num);
    NAPI_ASSERT(env, !numOverflow, "Mediant overflow");

    int64_t den = 0;
    bool denOverflow = __builtin_add_overflow(a.den, b.den, &den);
    NAPI_ASSERT(env, !denOverflow, "Mediant overflow");

    RationalValue sum { num, den };
    RationalValue result = NormalizeRational(sum);
    return CreateRationalValue(env, result);
}

napi_value AddInt(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: rational, n");

    RationalValue a { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &a), nullptr);

    int64_t n = 0;
    NAPI_CALL_BASE(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], &n), nullptr);

    NAPI_ASSERT(env, n <= RATIONAL_MAX_COMPONENT, "Integer too large");
    NAPI_ASSERT(env, n >= RATIONAL_MIN_COMPONENT, "Integer too small");

    int64_t scaled = 0;
    bool mulOverflow = __builtin_mul_overflow(n, a.den, &scaled);
    NAPI_ASSERT(env, !mulOverflow, "Addition overflow");

    int64_t sumNum = 0;
    bool addOverflow = __builtin_add_overflow(a.num, scaled, &sumNum);
    NAPI_ASSERT(env, !addOverflow, "Addition overflow");

    RationalValue sum { sumNum, a.den };
    RationalValue result = NormalizeRational(sum);
    return CreateRationalValue(env, result);
}

napi_value MulInt(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: rational, n");

    RationalValue a { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &a), nullptr);

    int64_t n = 0;
    NAPI_CALL_BASE(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], &n), nullptr);

    NAPI_ASSERT(env, n <= RATIONAL_MAX_COMPONENT, "Integer too large");
    NAPI_ASSERT(env, n >= RATIONAL_MIN_COMPONENT, "Integer too small");

    int64_t num = 0;
    bool overflow = __builtin_mul_overflow(a.num, n, &num);
    NAPI_ASSERT(env, !overflow, "Multiplication overflow");

    RationalValue product { num, a.den };
    RationalValue result = NormalizeRational(product);
    return CreateRationalValue(env, result);
}

static void ApproximateDouble(napi_env env, double value, int64_t maxDen, RationalValue* result)
{
    int64_t hOld = 0;
    int64_t hNew = 1;
    int64_t kOld = 1;
    int64_t kNew = 0;
    double fraction = std::fabs(value);

    for (int64_t iter = 0; iter < FROM_DOUBLE_MAX_ITER; iter += 1) {
        double floorValue = std::floor(fraction);
        int64_t whole = static_cast<int64_t>(floorValue);

        int64_t hNext = 0;
        bool hMulOverflow = __builtin_mul_overflow(whole, hNew, &hNext);
        bool hAddOverflow = __builtin_add_overflow(hNext, hOld, &hNext);
        NAPI_ASSERT_RETURN_VOID(env, !hMulOverflow && !hAddOverflow, "Approximation overflow");

        int64_t kNext = 0;
        bool kMulOverflow = __builtin_mul_overflow(whole, kNew, &kNext);
        bool kAddOverflow = __builtin_add_overflow(kNext, kOld, &kNext);
        NAPI_ASSERT_RETURN_VOID(env, !kMulOverflow && !kAddOverflow, "Approximation overflow");

        if (kNext > maxDen) {
            break;
        }

        hOld = hNew;
        hNew = hNext;
        kOld = kNew;
        kNew = kNext;

        double remainder = fraction - floorValue;
        if (remainder == DOUBLE_ZERO) {
            break;
        }
        fraction = DOUBLE_ONE / remainder;
    }

    int64_t sign = (value < DOUBLE_ZERO) ? -1 : 1;
    RationalValue approx { hNew * sign, kNew };
    *result = NormalizeRational(approx);
}

napi_value FromDouble(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: value, maxDen");

    double value = DOUBLE_ZERO;
    NAPI_CALL_BASE(env, ExtractDoubleArg(env, argv[ARG_INDEX_ZERO], &value), nullptr);

    int64_t maxDen = 0;
    NAPI_CALL_BASE(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], &maxDen), nullptr);

    NAPI_ASSERT(env, std::isfinite(value), "Value must be finite");
    double bound = static_cast<double>(RATIONAL_MAX_COMPONENT);
    NAPI_ASSERT(env, value >= -bound, "Value too small");
    NAPI_ASSERT(env, value <= bound, "Value too large");

    NAPI_ASSERT(env, maxDen >= FROM_DOUBLE_MIN_DEN, "maxDen too small");
    NAPI_ASSERT(env, maxDen <= FROM_DOUBLE_MAX_DEN, "maxDen too large");

    RationalValue approx { 0, 1 };
    ApproximateDouble(env, value, maxDen, &approx);
    return CreateRationalValue(env, approx);
}

napi_value IsProper(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rational");

    RationalValue value { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &value), nullptr);

    RationalValue normalized = NormalizeRational(value);

    int64_t absNum = (normalized.num < 0) ? -normalized.num : normalized.num;
    return CreateBoolValue(env, absNum < normalized.den);
}

napi_value HarmonicRational(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_CALL_BASE(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], &n), nullptr);

    NAPI_ASSERT(env, n >= HARMONIC_MIN_N, "n too small");
    NAPI_ASSERT(env, n <= HARMONIC_MAX_N, "n too large");

    RationalValue acc { 0, 1 };
    for (int64_t i = HARMONIC_MIN_N; i <= n; i += 1) {
        RationalValue term { 1, i };

        int64_t left = 0;
        int64_t right = 0;
        int64_t commonDen = 0;
        bool mulOverflow = __builtin_mul_overflow(acc.num, term.den, &left) ||
                           __builtin_mul_overflow(term.num, acc.den, &right) ||
                           __builtin_mul_overflow(acc.den, term.den, &commonDen);
        NAPI_ASSERT(env, !mulOverflow, "Harmonic overflow");

        int64_t sumNum = 0;
        bool addOverflow = __builtin_add_overflow(left, right, &sumNum);
        NAPI_ASSERT(env, !addOverflow, "Harmonic overflow");

        RationalValue sum { sumNum, commonDen };
        acc = NormalizeRational(sum);
    }
    return CreateRationalValue(env, acc);
}

napi_value Max(napi_env env, napi_callback_info info)
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

    RationalValue selected = a;
    if (right > left) {
        selected = b;
    }

    RationalValue result = NormalizeRational(selected);
    return CreateRationalValue(env, result);
}

napi_value Min(napi_env env, napi_callback_info info)
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

    RationalValue selected = a;
    if (right < left) {
        selected = b;
    }

    RationalValue result = NormalizeRational(selected);
    return CreateRationalValue(env, result);
}

napi_value SumArray(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rationals");

    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[ARG_INDEX_ZERO], &isArray));
    NAPI_ASSERT(env, isArray, "Argument must be an array");

    uint32_t length = 0;
    NAPI_CALL(env, napi_get_array_length(env, argv[ARG_INDEX_ZERO], &length));

    RationalValue acc { 0, 1 };
    for (uint32_t i = 0; i < length; ++i) {
        napi_value element = nullptr;
        NAPI_CALL(env, napi_get_element(env, argv[ARG_INDEX_ZERO], i, &element));

        RationalValue item { 0, 1 };
        NAPI_CALL_BASE(env, ExtractRational(env, element, &item), nullptr);

        int64_t left = 0;
        int64_t right = 0;
        int64_t commonDen = 0;
        bool mulOverflow = __builtin_mul_overflow(acc.num, item.den, &left) ||
                           __builtin_mul_overflow(item.num, acc.den, &right) ||
                           __builtin_mul_overflow(acc.den, item.den, &commonDen);
        NAPI_ASSERT(env, !mulOverflow, "Sum overflow");

        int64_t sumNum = 0;
        bool addOverflow = __builtin_add_overflow(left, right, &sumNum);
        NAPI_ASSERT(env, !addOverflow, "Sum overflow");

        RationalValue sum { sumNum, commonDen };
        acc = NormalizeRational(sum);
    }
    return CreateRationalValue(env, acc);
}

napi_value ProductArray(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rationals");

    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[ARG_INDEX_ZERO], &isArray));
    NAPI_ASSERT(env, isArray, "Argument must be an array");

    uint32_t length = 0;
    NAPI_CALL(env, napi_get_array_length(env, argv[ARG_INDEX_ZERO], &length));

    RationalValue acc { 1, 1 };
    for (uint32_t i = 0; i < length; ++i) {
        napi_value element = nullptr;
        NAPI_CALL(env, napi_get_element(env, argv[ARG_INDEX_ZERO], i, &element));

        RationalValue item { 0, 1 };
        NAPI_CALL_BASE(env, ExtractRational(env, element, &item), nullptr);

        int64_t num = 0;
        bool numOverflow = __builtin_mul_overflow(acc.num, item.num, &num);
        NAPI_ASSERT(env, !numOverflow, "Product overflow");

        int64_t den = 0;
        bool denOverflow = __builtin_mul_overflow(acc.den, item.den, &den);
        NAPI_ASSERT(env, !denOverflow, "Product overflow");

        RationalValue product { num, den };
        acc = NormalizeRational(product);
    }
    return CreateRationalValue(env, acc);
}

napi_value IsUnitFraction(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rational");

    RationalValue value { 0, 1 };
    NAPI_CALL_BASE(env, ExtractRational(env, argv[ARG_INDEX_ZERO], &value), nullptr);

    RationalValue normalized = NormalizeRational(value);
    return CreateBoolValue(env, normalized.num == 1);
}

napi_status RegisterRationalOpsFunctions(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("toNumber", ToNumber),
        DECLARE_NAPI_FUNCTION("toString", ToString),
        DECLARE_NAPI_FUNCTION("toMixedString", ToMixedString),
        DECLARE_NAPI_FUNCTION("numerator", Numerator),
        DECLARE_NAPI_FUNCTION("denominator", Denominator),
        DECLARE_NAPI_FUNCTION("floor", Floor),
        DECLARE_NAPI_FUNCTION("ceil", Ceil),
        DECLARE_NAPI_FUNCTION("round", Round),
        DECLARE_NAPI_FUNCTION("pow", Pow),
        DECLARE_NAPI_FUNCTION("mediant", Mediant),
        DECLARE_NAPI_FUNCTION("addInt", AddInt),
        DECLARE_NAPI_FUNCTION("mulInt", MulInt),
        DECLARE_NAPI_FUNCTION("fromDouble", FromDouble),
        DECLARE_NAPI_FUNCTION("isProper", IsProper),
        DECLARE_NAPI_FUNCTION("harmonicRational", HarmonicRational),
        DECLARE_NAPI_FUNCTION("max", Max),
        DECLARE_NAPI_FUNCTION("min", Min),
        DECLARE_NAPI_FUNCTION("sumArray", SumArray),
        DECLARE_NAPI_FUNCTION("productArray", ProductArray),
        DECLARE_NAPI_FUNCTION("isUnitFraction", IsUnitFraction),
    };
    return napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
}
