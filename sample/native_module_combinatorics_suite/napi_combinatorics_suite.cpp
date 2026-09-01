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

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "napi/native_node_api.h"

#include "combinatorics_suite_common.h"

napi_status ExtractInt64Arg(napi_env env, napi_value value, int64_t& result)
{
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, value, &type);
    if (status != napi_ok) {
        return status;
    }
    if (type != napi_number) {
        return napi_number_expected;
    }
    return napi_get_value_int64(env, value, &result);
}

napi_status ExtractInt64Array(napi_env env, napi_value value, std::vector<int64_t>& result)
{
    bool isArray = false;
    napi_status status = napi_is_array(env, value, &isArray);
    if (status != napi_ok) {
        return status;
    }
    if (!isArray) {
        return napi_array_expected;
    }
    uint32_t length = 0;
    status = napi_get_array_length(env, value, &length);
    if (status != napi_ok) {
        return status;
    }
    result.clear();
    result.reserve(length);
    for (uint32_t i = 0; i < length; ++i) {
        napi_value element = nullptr;
        status = napi_get_element(env, value, i, &element);
        if (status != napi_ok) {
            return status;
        }
        int64_t elementValue = 0;
        status = ExtractInt64Arg(env, element, elementValue);
        if (status != napi_ok) {
            return status;
        }
        if (elementValue < 0) {
            return napi_invalid_arg;
        }
        result.push_back(elementValue);
    }
    return napi_ok;
}

napi_value CreateInt64ArrayValue(napi_env env, const std::vector<int64_t>& values)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, values.size(), &result));
    for (size_t i = 0; i < values.size(); ++i) {
        napi_value element = nullptr;
        NAPI_CALL(env, napi_create_int64(env, values[i], &element));
        NAPI_CALL(env, napi_set_element(env, result, static_cast<uint32_t>(i), element));
    }
    return result;
}

napi_value CreateDoubleArrayValue(napi_env env, const std::vector<double>& values)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, values.size(), &result));
    for (size_t i = 0; i < values.size(); ++i) {
        napi_value element = nullptr;
        NAPI_CALL(env, napi_create_double(env, values[i], &element));
        NAPI_CALL(env, napi_set_element(env, result, static_cast<uint32_t>(i), element));
    }
    return result;
}

napi_value CreateDoubleMatrixValue(napi_env env, const std::vector<std::vector<double>>& values)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, values.size(), &result));
    for (size_t i = 0; i < values.size(); ++i) {
        napi_value row = CreateDoubleArrayValue(env, values[i]);
        NAPI_CALL(env, napi_set_element(env, result, static_cast<uint32_t>(i), row));
    }
    return result;
}

int64_t ComputeFactorial(int64_t n)
{
    int64_t result = 1;
    for (int64_t i = TWO_INPUT; i <= n; ++i) {
        result *= i;
    }
    return result;
}

int64_t ComputeBinomialInt64(int64_t n, int64_t k)
{
    if (k < 0 || k > n) {
        return 0;
    }
    int64_t kk = (k <= n - k) ? k : n - k;
    int64_t result = 1;
    for (int64_t i = 0; i < kk; ++i) {
        result = result * (n - i) / (i + 1);
    }
    return result;
}

static void BuildBinomialRowDoubles(int64_t n, std::vector<double>& row)
{
    row.clear();
    row.reserve(static_cast<size_t>(n) + 1);
    int64_t coefficient = 1;
    row.push_back(static_cast<double>(coefficient));
    for (int64_t k = 1; k <= n; ++k) {
        coefficient = coefficient * (n - k + 1) / k;
        row.push_back(static_cast<double>(coefficient));
    }
}

napi_value Factorial(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_ASSERT(env, n >= 0 && n <= FACTORIAL_MAX_INPUT, "n must be between 0 and 20");

    int64_t result = ComputeFactorial(n);

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value DoubleFactorial(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_ASSERT(env, n >= 0 && n <= DOUBLE_FACTORIAL_MAX_INPUT,
                "n must be between 0 and 20");

    int64_t result = 1;
    for (int64_t i = n; i >= TWO_INPUT; i -= TWO_INPUT) {
        result *= i;
    }

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value FallingFactorial(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: n, k");

    int64_t n = 0;
    int64_t k = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], k));
    NAPI_ASSERT(env, n >= 0 && k >= 0 && k <= n &&
                n <= FACTORIAL_MAX_INPUT, "requires 0 <= k <= n <= 20");

    int64_t result = 1;
    for (int64_t i = 0; i < k; ++i) {
        result *= (n - i);
    }

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value RisingFactorial(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: n, k");

    int64_t n = 0;
    int64_t k = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], k));
    NAPI_ASSERT(env, n >= 0 && k >= 0 && n + k <= FACTORIAL_MAX_INPUT,
                "n plus k must not exceed 20");

    int64_t result = 1;
    for (int64_t i = 0; i < k; ++i) {
        result *= (n + i);
    }

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value Permutation(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: n, k");

    int64_t n = 0;
    int64_t k = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], k));
    NAPI_ASSERT(env, n >= 0 && k >= 0 && k <= n &&
                n <= PERMUTATION_MAX_K, "requires 0 <= k <= n <= 20");

    int64_t result = 1;
    for (int64_t i = 0; i < k; ++i) {
        result *= (n - i);
    }

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value Combination(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: n, k");

    int64_t n = 0;
    int64_t k = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], k));
    NAPI_ASSERT(env, n >= 0 && k >= 0 && k <= n &&
                n <= BINOMIAL_MAX_N, "requires 0 <= k <= n <= 60");

    int64_t result = ComputeBinomialInt64(n, k);

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value CombinationsWithRepetition(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: n, k");

    int64_t n = 0;
    int64_t k = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], k));
    NAPI_ASSERT(env, n >= 1 && k >= 0 &&
                n + k - 1 <= BINOMIAL_MAX_N, "n plus k minus 1 must not exceed 60");

    int64_t result = ComputeBinomialInt64(n + k - 1, k);

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value PermutationsWithRepetition(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: n, k");

    int64_t n = 0;
    int64_t k = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], k));
    NAPI_ASSERT(env, n >= 0 && k >= 0, "n and k must be non-negative");

    int64_t result = 1;
    if (n >= TWO_INPUT) {
        for (int64_t i = 0; i < k; ++i) {
            NAPI_ASSERT(env, result <= INT64_LIMIT / n, "n power k exceeds int64 range");
            result *= n;
        }
    } else if (n == 0 && k > 0) {
        result = 0;
    }

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value MultisetPermutations(napi_env env, napi_callback_info info)
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

napi_value BinomialRow(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_ASSERT(env, n >= 0 && n <= PASCAL_MAX_ROWS, "n must be between 0 and 30");

    std::vector<double> row;
    BuildBinomialRowDoubles(n, row);
    return CreateDoubleArrayValue(env, row);
}

napi_value PascalTriangle(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: rows");

    int64_t rows = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], rows));
    NAPI_ASSERT(env, rows >= 0 && rows <= PASCAL_MAX_ROWS,
                "rows must be between 0 and 30");

    std::vector<std::vector<double>> triangle;
    triangle.reserve(static_cast<size_t>(rows));
    for (int64_t r = 0; r < rows; ++r) {
        std::vector<double> row;
        BuildBinomialRowDoubles(r, row);
        triangle.push_back(row);
    }
    return CreateDoubleMatrixValue(env, triangle);
}

napi_value CatalanNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_ASSERT(env, n >= 0 && n <= CATALAN_MAX_N, "n must be between 0 and 33");

    double binomial = 1.0;
    for (int64_t i = 1; i <= n; ++i) {
        binomial = binomial * (TWO_INPUT * n - i + 1) / i;
    }
    double result = binomial / (n + 1);

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_double(env, result, &resultValue));
    return resultValue;
}

napi_value CentralBinomial(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_ASSERT(env, n >= 0 && n <= CENTRAL_BINOMIAL_MAX_N,
                "n must be between 0 and 31");

    std::vector<double> row;
    BuildBinomialRowDoubles(TWO_INPUT * n, row);
    double result = row[n];

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_double(env, result, &resultValue));
    return resultValue;
}

napi_value Derangement(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_ASSERT(env, n >= 0 && n <= DERANGEMENT_MAX_N, "n must be between 0 and 20");

    int64_t previous = 1;
    int64_t current = 0;
    for (int64_t i = TWO_INPUT; i <= n; ++i) {
        int64_t next = (i - 1) * (previous + current);
        previous = current;
        current = next;
    }
    int64_t result = (n == 0) ? 1 : current;

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value BellNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_ASSERT(env, n >= 0 && n <= BELL_MAX_N, "n must be between 0 and 24");

    std::vector<int64_t> previous;
    std::vector<int64_t> current;
    previous.push_back(1);
    for (int64_t i = 1; i <= n; ++i) {
        current.clear();
        current.push_back(previous.back());
        for (size_t j = 0; j < previous.size(); ++j) {
            current.push_back(current[j] + previous[j]);
        }
        previous = current;
    }
    int64_t result = previous.front();

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value StirlingSecondKind(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: n, k");

    int64_t n = 0;
    int64_t k = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], k));
    NAPI_ASSERT(env, n >= 0 && k >= 0 && k <= n &&
                n <= STIRLING_MAX_N, "requires 0 <= k <= n <= 25");

    std::vector<int64_t> previous(n + 1, 0);
    std::vector<int64_t> current(n + 1, 0);
    previous[0] = 1;
    for (int64_t m = 1; m <= n; ++m) {
        current[0] = 0;
        for (int64_t j = 1; j <= m; ++j) {
            current[j] = j * previous[j] + previous[j - 1];
        }
        previous = current;
    }
    int64_t result = previous[k];

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value CircularPermutation(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_ASSERT(env, n >= 1 && n - 1 <= FACTORIAL_MAX_INPUT,
                "n minus 1 must not exceed 20");

    int64_t result = ComputeFactorial(n - 1);

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value SubsetCount(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_ASSERT(env, n >= 0 && n <= SUBSET_MAX_N, "n must be between 0 and 62");

    int64_t result = 1 << static_cast<int>(n);

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value HandshakeCount(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_ASSERT(env, n >= 0 && n <= HANDSHAKE_MAX_N, "n exceeds supported range");

    int64_t result = n * (n - 1) / TWO_INPUT;

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value IntegerPartitionCount(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_ASSERT(env, n >= 0 && n <= PARTITION_MAX_N, "n must be between 0 and 60");

    std::vector<int64_t> counts(n + 1, 0);
    counts[0] = 1;
    for (int64_t part = 1; part <= n; ++part) {
        for (int64_t sum = part; sum <= n; ++sum) {
            counts[sum] += counts[sum - part];
        }
    }
    int64_t result = counts[n];

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value SimplexNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: order, d");

    int64_t order = 0;
    int64_t d = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], order));
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], d));
    NAPI_ASSERT(env, order >= 1 && d >= 0 &&
                order + d - 1 <= SIMPLE_ORDER_MAX, "order plus d exceeds range");

    int64_t result = ComputeBinomialInt64(order + d - 1, d);

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value NextPermutation(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: values array");

    std::vector<int64_t> values;
    NAPI_CALL(env, ExtractInt64Array(env, argv[ARG_INDEX_ZERO], values));

    std::vector<int64_t> next(values);
    if (!std::next_permutation(next.begin(), next.end())) {
        next = values;
    }
    return CreateInt64ArrayValue(env, next);
}

napi_value PrevPermutation(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: values array");

    std::vector<int64_t> values;
    NAPI_CALL(env, ExtractInt64Array(env, argv[ARG_INDEX_ZERO], values));

    std::vector<int64_t> prev(values);
    if (!std::prev_permutation(prev.begin(), prev.end())) {
        prev = values;
    }
    return CreateInt64ArrayValue(env, prev);
}

napi_value StirlingFirstKind(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: n, k");

    int64_t n = 0;
    int64_t k = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], k));
    NAPI_ASSERT(env, n >= 0 && k >= 0 && k <= n &&
                n <= STIRLING_FIRST_MAX_N, "requires 0 <= k <= n <= 20");

    std::vector<int64_t> previous(n + 1, 0);
    std::vector<int64_t> current(n + 1, 0);
    previous[0] = 1;
    for (int64_t m = 1; m <= n; ++m) {
        current[0] = 0;
        for (int64_t j = 1; j <= m; ++j) {
            current[j] = previous[j - 1] + (m - 1) * previous[j];
        }
        previous = current;
    }
    int64_t result = previous[k];

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

napi_value EulerianNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: n, k");

    int64_t n = 0;
    int64_t k = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n));
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], k));
    NAPI_ASSERT(env, n >= 1 && k >= 0 && k < n && n <= EULERIAN_MAX_N,
                "requires 0 <= k < n <= 20");

    std::vector<int64_t> previous(k + 1, 0);
    std::vector<int64_t> current(k + 1, 0);
    previous[0] = 1;
    for (int64_t m = TWO_INPUT; m <= n; ++m) {
        current[0] = 1;
        for (int64_t j = 1; j <= k; ++j) {
            current[j] = (j + 1) * previous[j] + (m - j) * previous[j - 1];
        }
        previous = current;
    }
    int64_t result = previous[k];

    napi_value resultValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, result, &resultValue));
    return resultValue;
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("factorial", Factorial),
        DECLARE_NAPI_FUNCTION("doubleFactorial", DoubleFactorial),
        DECLARE_NAPI_FUNCTION("fallingFactorial", FallingFactorial),
        DECLARE_NAPI_FUNCTION("risingFactorial", RisingFactorial),
        DECLARE_NAPI_FUNCTION("permutation", Permutation),
        DECLARE_NAPI_FUNCTION("combination", Combination),
        DECLARE_NAPI_FUNCTION("combinationsWithRepetition", CombinationsWithRepetition),
        DECLARE_NAPI_FUNCTION("permutationsWithRepetition", PermutationsWithRepetition),
        DECLARE_NAPI_FUNCTION("multisetPermutations", MultisetPermutations),
        DECLARE_NAPI_FUNCTION("binomialRow", BinomialRow),
        DECLARE_NAPI_FUNCTION("pascalTriangle", PascalTriangle),
        DECLARE_NAPI_FUNCTION("catalanNumber", CatalanNumber),
        DECLARE_NAPI_FUNCTION("centralBinomial", CentralBinomial),
        DECLARE_NAPI_FUNCTION("derangement", Derangement),
        DECLARE_NAPI_FUNCTION("bellNumber", BellNumber),
        DECLARE_NAPI_FUNCTION("stirlingSecondKind", StirlingSecondKind),
        DECLARE_NAPI_FUNCTION("circularPermutation", CircularPermutation),
        DECLARE_NAPI_FUNCTION("subsetCount", SubsetCount),
        DECLARE_NAPI_FUNCTION("handshakeCount", HandshakeCount),
        DECLARE_NAPI_FUNCTION("integerPartitionCount", IntegerPartitionCount),
        DECLARE_NAPI_FUNCTION("simplexNumber", SimplexNumber),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
                                          sizeof(desc) / sizeof(desc[0]), desc));
    NAPI_CALL_BASE(env, RegisterCombinatoricsOpsFunctions(env, exports), nullptr);
    return exports;
}

static napi_module combinatoricsSuiteModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "combinatoricsSuite",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void CombinatoricsSuiteRegisterModule(void)
{
    napi_module_register(&combinatoricsSuiteModule);
}
