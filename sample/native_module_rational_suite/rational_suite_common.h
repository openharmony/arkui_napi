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

#ifndef SAMPLE_NATIVE_MODULE_RATIONAL_SUITE_COMMON_H
#define SAMPLE_NATIVE_MODULE_RATIONAL_SUITE_COMMON_H

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <string>

#include "napi/native_api.h"
#include "napi/native_common.h"

struct RationalValue {
    int64_t num;
    int64_t den;
};

constexpr int REQUIRED_ARGS_ONE = 1;
constexpr int REQUIRED_ARGS_TWO = 2;
constexpr int ARG_INDEX_ZERO = 0;
constexpr int ARG_INDEX_ONE = 1;

constexpr int64_t RATIONAL_MAX_COMPONENT = 2147483647LL;
constexpr int64_t RATIONAL_MIN_COMPONENT = -2147483647LL;
constexpr int64_t DENOMINATOR_MIN = 1;


constexpr int64_t COMPARE_LESS = -1;
constexpr int64_t COMPARE_EQUAL = 0;
constexpr int64_t COMPARE_GREATER = 1;

constexpr int64_t POW_MIN_EXPONENT = 0;
constexpr int64_t POW_MAX_EXPONENT = 62;
constexpr int64_t HARMONIC_MIN_N = 1;
constexpr int64_t HARMONIC_MAX_N = 20;
constexpr int64_t FROM_DOUBLE_MIN_DEN = 1;
constexpr int64_t FROM_DOUBLE_MAX_DEN = 1000000;
constexpr int64_t FROM_DOUBLE_MAX_ITER = 32;
constexpr int64_t ROUND_HALF_DENOMINATOR = 2;

constexpr double DOUBLE_ZERO = 0.0;
constexpr double DOUBLE_ONE = 1.0;

const char* const NUMERATOR_PROPERTY_NAME = "num";
const char* const DENOMINATOR_PROPERTY_NAME = "den";
const char* const FRACTION_SLASH = "/";
const char* const MIXED_SEPARATOR = " ";

int64_t GcdInt64(int64_t a, int64_t b);
RationalValue NormalizeRational(RationalValue value);
int64_t ComputeFloorNum(const RationalValue& value);

napi_status ExtractRational(napi_env env, napi_value value, RationalValue* result);
napi_status ExtractInt64Arg(napi_env env, napi_value value, int64_t* result);
napi_status ExtractDoubleArg(napi_env env, napi_value value, double* result);

napi_value CreateRationalValue(napi_env env, const RationalValue& value);
napi_value CreateInt64Value(napi_env env, int64_t value);
napi_value CreateDoubleValue(napi_env env, double value);
napi_value CreateBoolValue(napi_env env, bool value);
napi_value CreateStringValue(napi_env env, const std::string& value);

napi_value Create(napi_env env, napi_callback_info info);
napi_value Add(napi_env env, napi_callback_info info);
napi_value Subtract(napi_env env, napi_callback_info info);
napi_value Multiply(napi_env env, napi_callback_info info);
napi_value Divide(napi_env env, napi_callback_info info);
napi_value Negate(napi_env env, napi_callback_info info);
napi_value Inverse(napi_env env, napi_callback_info info);
napi_value Abs(napi_env env, napi_callback_info info);
napi_value Reduce(napi_env env, napi_callback_info info);
napi_value Compare(napi_env env, napi_callback_info info);
napi_value IsEqual(napi_env env, napi_callback_info info);
napi_value IsZero(napi_env env, napi_callback_info info);
napi_value IsInteger(napi_env env, napi_callback_info info);
napi_value IsPositive(napi_env env, napi_callback_info info);
napi_value IsNegative(napi_env env, napi_callback_info info);

napi_value ToNumber(napi_env env, napi_callback_info info);
napi_value ToString(napi_env env, napi_callback_info info);
napi_value ToMixedString(napi_env env, napi_callback_info info);
napi_value Numerator(napi_env env, napi_callback_info info);
napi_value Denominator(napi_env env, napi_callback_info info);
napi_value Floor(napi_env env, napi_callback_info info);
napi_value Ceil(napi_env env, napi_callback_info info);
napi_value Round(napi_env env, napi_callback_info info);
napi_value Pow(napi_env env, napi_callback_info info);
napi_value Mediant(napi_env env, napi_callback_info info);
napi_value AddInt(napi_env env, napi_callback_info info);
napi_value MulInt(napi_env env, napi_callback_info info);
napi_value FromDouble(napi_env env, napi_callback_info info);
napi_value IsProper(napi_env env, napi_callback_info info);
napi_value HarmonicRational(napi_env env, napi_callback_info info);
napi_value Max(napi_env env, napi_callback_info info);
napi_value Min(napi_env env, napi_callback_info info);
napi_value SumArray(napi_env env, napi_callback_info info);
napi_value ProductArray(napi_env env, napi_callback_info info);
napi_value IsUnitFraction(napi_env env, napi_callback_info info);

napi_status RegisterRationalOpsFunctions(napi_env env, napi_value exports);

#endif // SAMPLE_NATIVE_MODULE_RATIONAL_SUITE_COMMON_H
