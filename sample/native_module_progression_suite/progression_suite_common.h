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

#ifndef SAMPLE_NATIVE_MODULE_PROGRESSION_SUITE_COMMON_H
#define SAMPLE_NATIVE_MODULE_PROGRESSION_SUITE_COMMON_H

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_common.h"

constexpr int REQUIRED_ARGS_ONE = 1;
constexpr int REQUIRED_ARGS_TWO = 2;
constexpr int REQUIRED_ARGS_THREE = 3;
constexpr int ARG_INDEX_ZERO = 0;
constexpr int ARG_INDEX_ONE = 1;
constexpr int ARG_INDEX_TWO = 2;
constexpr int64_t FIBONACCI_SEED_ZERO = 0;
constexpr int64_t FIBONACCI_SEED_ONE = 1;
constexpr int64_t FIBONACCI_MAX_INDEX = 78;
constexpr int64_t FIBONACCI_DISCRIMINANT = 5;
constexpr int64_t DISCRIMINANT_OFFSET = 4;
constexpr int64_t LUCAS_SEED_ZERO = 2;
constexpr int64_t LUCAS_SEED_ONE = 1;
constexpr int64_t LUCAS_MAX_INDEX = 78;
constexpr int64_t PADOVAN_SEED = 1;
constexpr int64_t PADOVAN_MAX_INDEX = 78;
constexpr size_t PADOVAN_WINDOW = 3;
constexpr int64_t JACOBSTHAL_SEED_ZERO = 0;
constexpr int64_t JACOBSTHAL_SEED_ONE = 1;
constexpr int64_t JACOBSTHAL_MAX_INDEX = 62;
constexpr int64_t JACOBSTHAL_MULTIPLIER = 2;
constexpr int64_t COLLATZ_ODD_MULTIPLIER = 3;
constexpr int64_t COLLATZ_DIVISOR = 2;
constexpr int64_t COLLATZ_MAX_STEPS = 100000;
constexpr int64_t PRODUCT_RANGE_MAX_SPAN = 60;
constexpr int64_t HARMONIC_MIN_N = 1;
constexpr int64_t HARMONIC_MAX_N = 1000;
constexpr int64_t DIGIT_BASE = 10;
constexpr int64_t FIRST_TERM_INDEX = 1;
constexpr int64_t LIST_MIN_COUNT = 1;
constexpr int64_t POWER_SUM_SEED = 1;
constexpr size_t SEQUENCE_MIN_TERMS_FOR_PATTERN = 3;
constexpr size_t DIFFERENCES_MIN_TERMS = 2;
constexpr size_t LIST_MAX_COUNT = 1000;
constexpr size_t POWERS_OF_TWO_MAX_COUNT = 63;
constexpr int64_t POWERS_OF_TWO_BASE = 2;
constexpr double INFINITE_SUM_RATIO_LIMIT = 1.0;
constexpr double SEQUENCE_EPSILON = 1e-12;
constexpr double MEANS_ENDPOINT_COUNT = 2.0;
constexpr double TRIANGULAR_DIVISOR = 2.0;
constexpr double PENTAGONAL_MULTIPLIER = 3.0;
constexpr double PENTAGONAL_SUBTRAHEND = 1.0;
constexpr double PENTAGONAL_DIVISOR = 2.0;
constexpr double HEXAGONAL_MULTIPLIER = 2.0;
constexpr double HEXAGONAL_SUBTRAHEND = 1.0;
constexpr double HEPTAGONAL_MULTIPLIER = 5.0;
constexpr double HEPTAGONAL_SUBTRAHEND = 3.0;
constexpr double HEPTAGONAL_DIVISOR = 2.0;
constexpr double OCTAGONAL_MULTIPLIER = 3.0;
constexpr double OCTAGONAL_SUBTRAHEND = 2.0;
constexpr double TETRAHEDRAL_FACTOR_ONE = 1.0;
constexpr double TETRAHEDRAL_FACTOR_TWO = 2.0;
constexpr double TETRAHEDRAL_DIVISOR = 6.0;
constexpr double SQUARE_PYRAMID_FACTOR_ONE = 1.0;
constexpr double SQUARE_PYRAMID_FACTOR_TWO = 2.0;
constexpr double SQUARE_PYRAMID_DIVISOR = 6.0;
constexpr double CENTERED_SQUARE_MULTIPLIER = 2.0;
constexpr size_t ALTERNATING_SIGN_PERIOD = 2;

bool ExtractInt64Arg(napi_env env, napi_value value, int64_t& result);
bool ExtractDoubleArg(napi_env env, napi_value value, double& result);
bool ExtractDoubleArray(napi_env env, napi_value value, std::vector<double>& result);
napi_value CreateDoubleArrayValue(napi_env env, const std::vector<double>& values);
napi_value CreateInt64ArrayValue(napi_env env, const std::vector<int64_t>& values);
napi_value CreateInt64Value(napi_env env, int64_t value);
napi_value CreateDoubleValue(napi_env env, double value);
napi_value CreateBoolValue(napi_env env, bool value);

napi_value ArithmeticTerm(napi_env env, napi_callback_info info);
napi_value ArithmeticSum(napi_env env, napi_callback_info info);
napi_value ArithmeticMeans(napi_env env, napi_callback_info info);
napi_value GeometricTerm(napi_env env, napi_callback_info info);
napi_value GeometricSum(napi_env env, napi_callback_info info);
napi_value GeometricMean(napi_env env, napi_callback_info info);
napi_value InfiniteGeometricSum(napi_env env, napi_callback_info info);
napi_value IsArithmeticSequence(napi_env env, napi_callback_info info);
napi_value IsGeometricSequence(napi_env env, napi_callback_info info);
napi_value NextTerm(napi_env env, napi_callback_info info);
napi_value NextTerms(napi_env env, napi_callback_info info);
napi_value SequenceDifferences(napi_env env, napi_callback_info info);
napi_value Fibonacci(napi_env env, napi_callback_info info);
napi_value FibonacciList(napi_env env, napi_callback_info info);
napi_value IsFibonacci(napi_env env, napi_callback_info info);
napi_value LucasNumber(napi_env env, napi_callback_info info);
napi_value LucasList(napi_env env, napi_callback_info info);
napi_value TriangularNumber(napi_env env, napi_callback_info info);
napi_value SquareNumber(napi_env env, napi_callback_info info);
napi_value PentagonalNumber(napi_env env, napi_callback_info info);
napi_value HexagonalNumber(napi_env env, napi_callback_info info);
napi_value HeptagonalNumber(napi_env env, napi_callback_info info);

napi_value OctagonalNumber(napi_env env, napi_callback_info info);
napi_value TetrahedralNumber(napi_env env, napi_callback_info info);
napi_value SquarePyramidalNumber(napi_env env, napi_callback_info info);
napi_value CenteredSquareNumber(napi_env env, napi_callback_info info);
napi_value SumRange(napi_env env, napi_callback_info info);
napi_value ProductRange(napi_env env, napi_callback_info info);
napi_value SumOfSquaresRange(napi_env env, napi_callback_info info);
napi_value SumOfCubesRange(napi_env env, napi_callback_info info);
napi_value PowersOfTwo(napi_env env, napi_callback_info info);
napi_value PowerSum(napi_env env, napi_callback_info info);
napi_value GeometricSequenceList(napi_env env, napi_callback_info info);
napi_value ArithmeticSequenceList(napi_env env, napi_callback_info info);
napi_value CollatzSteps(napi_env env, napi_callback_info info);
napi_value CollatzSequence(napi_env env, napi_callback_info info);
napi_value CollatzMaxValue(napi_env env, napi_callback_info info);
napi_value PadovanNumber(napi_env env, napi_callback_info info);
napi_value JacobsthalNumber(napi_env env, napi_callback_info info);
napi_value HarmonicNumber(napi_env env, napi_callback_info info);
napi_value AlternatingSum(napi_env env, napi_callback_info info);
napi_value DigitSum(napi_env env, napi_callback_info info);
napi_value DigitProduct(napi_env env, napi_callback_info info);
napi_value IsPalindromicNumber(napi_env env, napi_callback_info info);

napi_status RegisterProgressionOpsFunctions(napi_env env, napi_value exports);

#endif // SAMPLE_NATIVE_MODULE_PROGRESSION_SUITE_COMMON_H
