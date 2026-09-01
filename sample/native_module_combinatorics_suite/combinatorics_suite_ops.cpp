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
#include <cstdint>
#include <vector>

#include "napi/native_node_api.h"

#include "combinatorics_suite_common.h"

static int64_t ComputeFibonacci(int64_t index)
{
    int64_t current = 1;
    int64_t next = 1;
    for (int64_t i = 0; i < index; ++i) {
        int64_t sum = current + next;
        current = next;
        next = sum;
    }
    return current;
}

napi_value LegendreExponent(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: n, prime");

    int64_t n = 0;
    int64_t prime = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], prime));
    NAPI_ASSERT(env, n >= 0 && n <= LEGENDRE_MAX_INPUT && prime >= LEGENDRE_MIN_PRIME,
                "requires n <= 1000000 and prime >= 2");

    int64_t exponent = 0;
    int64_t power = prime;
    while (power <= n) {
        exponent += n / power;
        NAPI_ASSERT(env, power <= INT64_LIMIT / prime, "prime power exceeds int64 range");
        power *= prime;
    }

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, exponent, &resultValue));
    return resultValue;
}

napi_value InversionCount(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: values array");

    std::vector<int64_t> values;
    NAPI_CALL(env, ExtractInt64Array(env, argv[ARG_INDEX_ZERO], values));
    NAPI_ASSERT(env, static_cast<int64_t>(values.size()) <= INVERSION_LIMIT, "array length must not exceed 2048");

    int64_t count = 0;
    for (size_t i = 0; i < values.size(); ++i) {
        for (size_t j = i + 1; j < values.size(); ++j) {
            if (values[i] > values[j]) {
                ++count;
            }
        }
    }

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, count, &resultValue));
    return resultValue;
}

napi_value PermutationSign(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: values array");

    std::vector<int64_t> values;
    NAPI_CALL(env, ExtractInt64Array(env, argv[ARG_INDEX_ZERO], values));
    NAPI_ASSERT(env, static_cast<int64_t>(values.size()) <= INVERSION_LIMIT, "array length must not exceed 2048");

    int64_t count = 0;
    for (size_t i = 0; i < values.size(); ++i) {
        for (size_t j = i + 1; j < values.size(); ++j) {
            if (values[i] > values[j]) {
                ++count;
            }
        }
    }
    int64_t sign = (count % TWO_INPUT == 0) ? SIGN_POSITIVE : SIGN_NEGATIVE;

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, sign, &resultValue));
    return resultValue;
}

napi_value IsValidPermutation(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: values array, n");

    std::vector<int64_t> values;
    NAPI_CALL(env, ExtractInt64Array(env, argv[ARG_INDEX_ZERO], values));

    int64_t n = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], n));
    NAPI_ASSERT(env, n >= 0, "n must be non-negative");

    bool valid = (static_cast<int64_t>(values.size()) == n);
    if (valid && n > 0) {
        std::vector<bool> seen(static_cast<size_t>(n), false);
        for (size_t i = 0; valid && i < values.size(); ++i) {
            valid = values[i] >= 1 && values[i] <= n;
            if (valid) {
                size_t slot = static_cast<size_t>(values[i] - 1);
                valid = !seen[slot];
                seen[slot] = true;
            }
        }
    }

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, valid, &resultValue));
    return resultValue;
}

napi_value CompositionsCount(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: n, k");

    int64_t n = 0;
    int64_t k = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], k));
    NAPI_ASSERT(env, k >= COMPOSITION_MIN_K && n >= k && n - 1 <= BINOMIAL_MAX_N,
                "requires k >= 1, n >= k, n <= 61");

    int64_t result = ComputeBinomialInt64(n - 1, k - 1);

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value BinomialSum(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_ASSERT(env, n >= 0 && n <= BINOMIAL_MAX_N, "n must be between 0 and 60");

    double sum = 1.0;
    double term = 1.0;
    for (int64_t k = 0; k < n; ++k) {
        term = term * (n - k) / (k + 1);
        sum += term;
    }

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_double(env, sum, &resultValue));
    return resultValue;
}

napi_value AlternatingBinomialSum(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_ASSERT(env, n >= 0 && n <= BINOMIAL_MAX_N, "n must be between 0 and 60");

    double sum = 1.0;
    double term = 1.0;
    double sign = -1.0;
    for (int64_t k = 0; k < n; ++k) {
        term = term * (n - k) / (k + 1);
        sum += sign * term;
        sign = -sign;
    }

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_double(env, sum, &resultValue));
    return resultValue;
}

napi_value MultinomialCoefficient(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: parts array");

    std::vector<int64_t> parts;
    NAPI_CALL(env, ExtractInt64Array(env, argv[ARG_INDEX_ZERO], parts));

    int64_t total = 0;
    for (size_t i = 0; i < parts.size(); ++i) {
        total += parts[i];
    }
    NAPI_ASSERT(env, total <= MULTINOMIAL_MAX_SUM, "sum of parts must not exceed 20");

    int64_t result = ComputeFactorial(total);
    for (size_t i = 0; i < parts.size(); ++i) {
        result /= ComputeFactorial(parts[i]);
    }

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value LatticePaths(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: m, n");

    int64_t m = 0;
    int64_t n = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], m));
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], n));
    NAPI_ASSERT(env, m >= 0 && n >= 0 && m + n <= LATTICE_MAX_STEPS,
                "m plus n must not exceed 60");

    int64_t total = m + n;
    int64_t kk = (m <= n) ? m : n;
    double result = 1.0;
    for (int64_t i = 0; i < kk; ++i) {
        result = result * (total - i) / (i + 1);
    }

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_double(env, result, &resultValue));
    return resultValue;
}

napi_value CountBinaryStringsNoAdjacentOnes(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_ASSERT(env, n >= 0 && n <= BINARY_STRING_MAX_BITS, "n must be between 0 and 60");

    int64_t result = ComputeFibonacci(n + 1);

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value DominoTilings(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: width");

    int64_t width = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], width));
    NAPI_ASSERT(env, width >= 0 && width <= DOMINO_MAX_WIDTH, "width must be between 0 and 60");

    int64_t result = ComputeFibonacci(width);

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value PartitionsIntoParts(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: n, maxPart");

    int64_t n = 0;
    int64_t maxPart = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], maxPart));
    NAPI_ASSERT(env, n >= 0 && n <= PARTITION_MAX_N && maxPart >= 1,
                "requires 0 <= n <= 60 and maxPart >= 1");

    int64_t bound = (maxPart <= n) ? maxPart : n;
    std::vector<int64_t> counts(n + 1, 0);
    counts[0] = 1;
    for (int64_t part = 1; part <= bound; ++part) {
        for (int64_t sum = part; sum <= n; ++sum) {
            counts[sum] += counts[sum - part];
        }
    }
    int64_t result = counts[n];

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value BellList(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: count");

    int64_t count = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], count));
    NAPI_ASSERT(env, count >= 1 && count <= BELL_LIST_MAX_COUNT,
                "count must be between 1 and 25");

    std::vector<int64_t> previous;
    std::vector<int64_t> current;
    std::vector<int64_t> bells;
    bells.reserve(static_cast<size_t>(count));
    previous.push_back(1);
    bells.push_back(1);
    for (int64_t i = 1; i < count; ++i) {
        current.clear();
        current.push_back(previous.back());
        for (size_t j = 0; j < previous.size(); ++j) {
            current.push_back(current[j] + previous[j]);
        }
        bells.push_back(current.front());
        previous = current;
    }
    return CreateInt64ArrayValue(env, bells);
}

napi_value CatalanList(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: count");

    int64_t count = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], count));
    NAPI_ASSERT(env, count >= 1 && count <= CATALAN_LIST_MAX_COUNT,
                "count must be between 1 and 34");

    std::vector<double> catalans;
    catalans.reserve(static_cast<size_t>(count));
    double current = 1.0;
    catalans.push_back(current);
    for (int64_t i = 1; i < count; ++i) {
        current = current * (TWO_INPUT * (TWO_INPUT * (i - 1) + 1)) /
                  (i + 1);
        catalans.push_back(current);
    }
    return CreateDoubleArrayValue(env, catalans);
}

napi_value MotzkinNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_ASSERT(env, n >= 0 && n <= MOTZKIN_MAX_N, "n must be between 0 and 41");

    std::vector<int64_t> motzkin(n + 1, 0);
    motzkin[0] = 1;
    if (n >= 1) {
        motzkin[1] = 1;
    }
    for (int64_t i = TWO_INPUT; i <= n; ++i) {
        motzkin[i] = ((TWO_INPUT * i + 1) * motzkin[i - 1] +
                      THREE_INPUT * (i - 1) * motzkin[i - TWO_INPUT]) /
                     (i + TWO_INPUT);
    }
    int64_t result = motzkin[n];

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value NarayanaNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: n, k");

    int64_t n = 0;
    int64_t k = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], k));
    NAPI_ASSERT(env, n >= 1 && k >= 1 && k <= n && n <= NARAYANA_MAX_N, "requires 1 <= k <= n <= 60");

    double first = 1.0;
    double second = 1.0;
    for (int64_t i = 0; i < k; ++i) {
        first = first * (n - i) / (i + 1);
    }
    for (int64_t i = 0; i < k - 1; ++i) {
        second = second * (n - i) / (i + 1);
    }
    double result = first * second / n;

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_double(env, result, &resultValue));
    return resultValue;
}

napi_value DelannoyNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: m, n");

    int64_t m = 0;
    int64_t n = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], m));
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], n));
    NAPI_ASSERT(env, m >= 0 && n >= 0 && m + n <= DELANNOY_MAX_N,
                "m plus n must not exceed 30");

    std::vector<double> previous(n + 1, 1.0);
    std::vector<double> current(n + 1, 0.0);
    for (int64_t i = 1; i <= m; ++i) {
        current[0] = 1.0;
        for (int64_t j = 1; j <= n; ++j) {
            current[j] = current[j - 1] + previous[j] + previous[j - 1];
        }
        previous = current;
    }
    double result = previous[n];

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_double(env, result, &resultValue));
    return resultValue;
}

napi_status RegisterCombinatoricsOpsFunctions(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("legendreExponent", LegendreExponent),
        DECLARE_NAPI_FUNCTION("inversionCount", InversionCount),
        DECLARE_NAPI_FUNCTION("permutationSign", PermutationSign),
        DECLARE_NAPI_FUNCTION("isValidPermutation", IsValidPermutation),
        DECLARE_NAPI_FUNCTION("nextPermutation", NextPermutation),
        DECLARE_NAPI_FUNCTION("prevPermutation", PrevPermutation),
        DECLARE_NAPI_FUNCTION("compositionsCount", CompositionsCount),
        DECLARE_NAPI_FUNCTION("binomialSum", BinomialSum),
        DECLARE_NAPI_FUNCTION("alternatingBinomialSum", AlternatingBinomialSum),
        DECLARE_NAPI_FUNCTION("multinomialCoefficient", MultinomialCoefficient),
        DECLARE_NAPI_FUNCTION("latticePaths", LatticePaths),
        DECLARE_NAPI_FUNCTION("countBinaryStringsNoAdjacentOnes", CountBinaryStringsNoAdjacentOnes),
        DECLARE_NAPI_FUNCTION("dominoTilings", DominoTilings),
        DECLARE_NAPI_FUNCTION("partitionsIntoParts", PartitionsIntoParts),
        DECLARE_NAPI_FUNCTION("bellList", BellList),
        DECLARE_NAPI_FUNCTION("catalanList", CatalanList),
        DECLARE_NAPI_FUNCTION("motzkinNumber", MotzkinNumber),
        DECLARE_NAPI_FUNCTION("narayanaNumber", NarayanaNumber),
        DECLARE_NAPI_FUNCTION("delannoyNumber", DelannoyNumber),
        DECLARE_NAPI_FUNCTION("eulerianNumber", EulerianNumber),
        DECLARE_NAPI_FUNCTION("stirlingFirstKind", StirlingFirstKind),
    };
    return napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
}
