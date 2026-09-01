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

#ifndef SAMPLE_NATIVE_MODULE_COMBINATORICS_SUITE_COMMON_H
#define SAMPLE_NATIVE_MODULE_COMBINATORICS_SUITE_COMMON_H

#include <cstddef>
#include <cstdint>
#include <vector>

#include "napi/native_api.h"

constexpr int REQUIRED_ARGS_ONE = 1;
constexpr int REQUIRED_ARGS_TWO = 2;
constexpr int ARG_INDEX_ZERO = 0;
constexpr int ARG_INDEX_ONE = 1;
constexpr int64_t FACTORIAL_MAX_INPUT = 20;
constexpr int64_t DOUBLE_FACTORIAL_MAX_INPUT = 20;
constexpr int64_t PERMUTATION_MAX_K = 20;
constexpr int64_t BINOMIAL_MAX_N = 60;
constexpr int64_t CATALAN_MAX_N = 33;
constexpr int64_t CENTRAL_BINOMIAL_MAX_N = 31;
constexpr int64_t DERANGEMENT_MAX_N = 20;
constexpr int64_t BELL_MAX_N = 24;
constexpr int64_t STIRLING_MAX_N = 25;
constexpr int64_t STIRLING_FIRST_MAX_N = 20;
constexpr int64_t MULTINOMIAL_MAX_SUM = 20;
constexpr int64_t PARTITION_MAX_N = 60;
constexpr int64_t LEGENDRE_MAX_INPUT = 1000000;
constexpr int64_t LEGENDRE_MIN_PRIME = 2;
constexpr int64_t PASCAL_MAX_ROWS = 30;
constexpr int64_t LATTICE_MAX_STEPS = 60;
constexpr int64_t BINARY_STRING_MAX_BITS = 60;
constexpr int64_t DOMINO_MAX_WIDTH = 60;
constexpr int64_t MOTZKIN_MAX_N = 41;
constexpr int64_t NARAYANA_MAX_N = 60;
constexpr int64_t DELANNOY_MAX_N = 30;
constexpr int64_t EULERIAN_MAX_N = 20;
constexpr int64_t INVERSION_LIMIT = 2048;
constexpr int64_t COMPOSITION_MIN_K = 1;
constexpr int64_t SUBSET_MAX_N = 62;
constexpr int64_t HANDSHAKE_MAX_N = 3037000499;
constexpr int64_t SIMPLE_ORDER_MAX = 60;
constexpr int64_t BELL_LIST_MAX_COUNT = 25;
constexpr int64_t CATALAN_LIST_MAX_COUNT = 34;
constexpr int64_t INT64_LIMIT = 9223372036854775807LL;
constexpr int64_t SIGN_NEGATIVE = -1;
constexpr int64_t SIGN_POSITIVE = 1;
constexpr int64_t TWO_INPUT = 2;
constexpr int64_t THREE_INPUT = 3;

napi_status ExtractInt64Arg(napi_env env, napi_value value, int64_t& result);

int64_t ComputeFactorial(int64_t n);
int64_t ComputeBinomialInt64(int64_t n, int64_t k);
napi_status ExtractInt64Array(napi_env env, napi_value value, std::vector<int64_t>& result);
napi_value CreateInt64ArrayValue(napi_env env, const std::vector<int64_t>& values);
napi_value CreateDoubleArrayValue(napi_env env, const std::vector<double>& values);
napi_value CreateDoubleMatrixValue(napi_env env, const std::vector<std::vector<double>>& values);

napi_value Factorial(napi_env env, napi_callback_info info);
napi_value DoubleFactorial(napi_env env, napi_callback_info info);
napi_value FallingFactorial(napi_env env, napi_callback_info info);
napi_value RisingFactorial(napi_env env, napi_callback_info info);
napi_value Permutation(napi_env env, napi_callback_info info);
napi_value Combination(napi_env env, napi_callback_info info);
napi_value CombinationsWithRepetition(napi_env env, napi_callback_info info);
napi_value PermutationsWithRepetition(napi_env env, napi_callback_info info);
napi_value MultisetPermutations(napi_env env, napi_callback_info info);
napi_value BinomialRow(napi_env env, napi_callback_info info);
napi_value PascalTriangle(napi_env env, napi_callback_info info);
napi_value CatalanNumber(napi_env env, napi_callback_info info);
napi_value CentralBinomial(napi_env env, napi_callback_info info);
napi_value Derangement(napi_env env, napi_callback_info info);
napi_value BellNumber(napi_env env, napi_callback_info info);
napi_value StirlingSecondKind(napi_env env, napi_callback_info info);
napi_value CircularPermutation(napi_env env, napi_callback_info info);
napi_value SubsetCount(napi_env env, napi_callback_info info);
napi_value HandshakeCount(napi_env env, napi_callback_info info);
napi_value IntegerPartitionCount(napi_env env, napi_callback_info info);
napi_value SimplexNumber(napi_env env, napi_callback_info info);
napi_value LegendreExponent(napi_env env, napi_callback_info info);
napi_value InversionCount(napi_env env, napi_callback_info info);
napi_value PermutationSign(napi_env env, napi_callback_info info);
napi_value IsValidPermutation(napi_env env, napi_callback_info info);
napi_value NextPermutation(napi_env env, napi_callback_info info);
napi_value PrevPermutation(napi_env env, napi_callback_info info);
napi_value CompositionsCount(napi_env env, napi_callback_info info);
napi_value BinomialSum(napi_env env, napi_callback_info info);
napi_value AlternatingBinomialSum(napi_env env, napi_callback_info info);
napi_value MultinomialCoefficient(napi_env env, napi_callback_info info);
napi_value LatticePaths(napi_env env, napi_callback_info info);
napi_value CountBinaryStringsNoAdjacentOnes(napi_env env, napi_callback_info info);
napi_value DominoTilings(napi_env env, napi_callback_info info);
napi_value PartitionsIntoParts(napi_env env, napi_callback_info info);
napi_value BellList(napi_env env, napi_callback_info info);
napi_value CatalanList(napi_env env, napi_callback_info info);
napi_value MotzkinNumber(napi_env env, napi_callback_info info);
napi_value NarayanaNumber(napi_env env, napi_callback_info info);
napi_value DelannoyNumber(napi_env env, napi_callback_info info);
napi_value EulerianNumber(napi_env env, napi_callback_info info);
napi_value StirlingFirstKind(napi_env env, napi_callback_info info);

napi_status RegisterCombinatoricsOpsFunctions(napi_env env, napi_value exports);

#endif  // SAMPLE_NATIVE_MODULE_COMBINATORICS_SUITE_COMMON_H
