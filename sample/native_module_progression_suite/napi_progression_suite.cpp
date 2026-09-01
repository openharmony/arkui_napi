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
#include <cmath>
#include <vector>

#include "napi/native_node_api.h"
#include "progression_suite_common.h"

bool ExtractInt64Arg(napi_env env, napi_value value, int64_t& result)
{
    napi_valuetype type = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, value, &type), false);
    if (type != napi_number) {
        return false;
    }

    double number = 0.0;
    NAPI_CALL_BASE(env, napi_get_value_double(env, value, &number), false);
    if (std::isnan(number) || std::isinf(number)) {
        return false;
    }
    if (std::floor(number) != number) {
        return false;
    }

    result = static_cast<int64_t>(number);
    return true;
}

bool ExtractDoubleArg(napi_env env, napi_value value, double& result)
{
    napi_valuetype type = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, value, &type), false);
    if (type != napi_number) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_get_value_double(env, value, &result), false);
    if (std::isnan(result) || std::isinf(result)) {
        return false;
    }
    return true;
}

bool ExtractDoubleArray(napi_env env, napi_value value, std::vector<double>& result)
{
    bool isArray = false;
    NAPI_CALL_BASE(env, napi_is_array(env, value, &isArray), false);
    if (!isArray) {
        return false;
    }

    uint32_t length = 0;
    NAPI_CALL_BASE(env, napi_get_array_length(env, value, &length), false);

    result.clear();
    result.reserve(length);
    for (uint32_t i = 0; i < length; ++i) {
        napi_value element = nullptr;
        NAPI_CALL_BASE(env, napi_get_element(env, value, i, &element), false);
        double number = 0.0;
        if (!ExtractDoubleArg(env, element, number)) {
            return false;
        }
        result.push_back(number);
    }
    return true;
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

static bool IsPerfectSquareValue(int64_t value)
{
    if (value < 0) {
        return false;
    }
    int64_t root = static_cast<int64_t>(std::sqrt(static_cast<double>(value)));
    int64_t square = root * root;
    if (square == value) {
        return true;
    }
    return false;
}

napi_value ArithmeticTerm(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: first, diff, n");

    double first = 0.0;
    double diff = 0.0;
    int64_t n = 0;
    NAPI_ASSERT(env, ExtractDoubleArg(env, argv[ARG_INDEX_ZERO], first), "First term must be a number");
    NAPI_ASSERT(env, ExtractDoubleArg(env, argv[ARG_INDEX_ONE], diff), "Common difference must be a number");
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_TWO], n), "Term index must be an integer");
    NAPI_ASSERT(env, n >= FIRST_TERM_INDEX, "Term index must be at least 1");

    double offset = static_cast<double>(n - 1);
    double result = first + offset * diff;
    return CreateDoubleValue(env, result);
}

napi_value ArithmeticSum(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: first, diff, count");

    double first = 0.0;
    double diff = 0.0;
    int64_t count = 0;
    NAPI_ASSERT(env, ExtractDoubleArg(env, argv[ARG_INDEX_ZERO], first), "First term must be a number");
    NAPI_ASSERT(env, ExtractDoubleArg(env, argv[ARG_INDEX_ONE], diff), "Common difference must be a number");
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_TWO], count), "Count must be an integer");
    NAPI_ASSERT(env, count >= LIST_MIN_COUNT, "Count must be at least 1");

    double sum = 0.0;
    for (int64_t i = 0; i < count; ++i) {
        double term = first + static_cast<double>(i) * diff;
        sum += term;
    }
    return CreateDoubleValue(env, sum);
}

napi_value ArithmeticMeans(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: first, last, count");

    double first = 0.0;
    double last = 0.0;
    int64_t count = 0;
    NAPI_ASSERT(env, ExtractDoubleArg(env, argv[ARG_INDEX_ZERO], first), "First term must be a number");
    NAPI_ASSERT(env, ExtractDoubleArg(env, argv[ARG_INDEX_ONE], last), "Last term must be a number");
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_TWO], count), "Count must be an integer");
    NAPI_ASSERT(env, count >= LIST_MIN_COUNT, "Count must be at least 1");
    NAPI_ASSERT(env, count <= static_cast<int64_t>(LIST_MAX_COUNT), "Count must not exceed 1000");

    double span = static_cast<double>(count) + MEANS_ENDPOINT_COUNT - 1.0;
    double step = (last - first) / span;
    std::vector<double> means;
    means.reserve(static_cast<size_t>(count));
    for (int64_t k = FIRST_TERM_INDEX; k <= count; ++k) {
        means.push_back(first + static_cast<double>(k) * step);
    }
    return CreateDoubleArrayValue(env, means);
}

napi_value GeometricTerm(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: first, ratio, n");

    double first = 0.0;
    double ratio = 0.0;
    int64_t n = 0;
    NAPI_ASSERT(env, ExtractDoubleArg(env, argv[ARG_INDEX_ZERO], first), "First term must be a number");
    NAPI_ASSERT(env, ExtractDoubleArg(env, argv[ARG_INDEX_ONE], ratio), "Common ratio must be a number");
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_TWO], n), "Term index must be an integer");
    NAPI_ASSERT(env, n >= FIRST_TERM_INDEX, "Term index must be at least 1");

    double exponent = static_cast<double>(n - 1);
    double result = first * std::pow(ratio, exponent);
    return CreateDoubleValue(env, result);
}

napi_value GeometricSum(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: first, ratio, count");

    double first = 0.0;
    double ratio = 0.0;
    int64_t count = 0;
    NAPI_ASSERT(env, ExtractDoubleArg(env, argv[ARG_INDEX_ZERO], first), "First term must be a number");
    NAPI_ASSERT(env, ExtractDoubleArg(env, argv[ARG_INDEX_ONE], ratio), "Common ratio must be a number");
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_TWO], count), "Count must be an integer");
    NAPI_ASSERT(env, count >= LIST_MIN_COUNT, "Count must be at least 1");

    double sum = 0.0;
    double term = first;
    for (int64_t i = 0; i < count; ++i) {
        sum += term;
        term *= ratio;
    }
    return CreateDoubleValue(env, sum);
}

napi_value GeometricMean(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: a, b");

    double a = 0.0;
    double b = 0.0;
    NAPI_ASSERT(env, ExtractDoubleArg(env, argv[ARG_INDEX_ZERO], a), "First value must be a number");
    NAPI_ASSERT(env, ExtractDoubleArg(env, argv[ARG_INDEX_ONE], b), "Second value must be a number");
    NAPI_ASSERT(env, a > 0.0, "First value must be positive");
    NAPI_ASSERT(env, b > 0.0, "Second value must be positive");

    double product = a * b;
    double result = std::sqrt(product);
    return CreateDoubleValue(env, result);
}

napi_value InfiniteGeometricSum(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: first, ratio");

    double first = 0.0;
    double ratio = 0.0;
    NAPI_ASSERT(env, ExtractDoubleArg(env, argv[ARG_INDEX_ZERO], first), "First term must be a number");
    NAPI_ASSERT(env, ExtractDoubleArg(env, argv[ARG_INDEX_ONE], ratio), "Common ratio must be a number");
    NAPI_ASSERT(env, std::fabs(ratio) < INFINITE_SUM_RATIO_LIMIT, "Ratio absolute value must be less than 1");

    double denominator = INFINITE_SUM_RATIO_LIMIT - ratio;
    NAPI_ASSERT(env, std::fabs(denominator) > SEQUENCE_EPSILON, "Denominator must be non-zero");
    double result = first / denominator;
    return CreateDoubleValue(env, result);
}

napi_value IsArithmeticSequence(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: sequence");

    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[ARG_INDEX_ZERO], &isArray));
    NAPI_ASSERT(env, isArray, "Sequence must be an array");

    std::vector<double> values;
    NAPI_ASSERT(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), "Sequence must be an array of numbers");
    NAPI_ASSERT(env, values.size() >= SEQUENCE_MIN_TERMS_FOR_PATTERN, "Sequence must have at least 3 terms");

    double commonDiff = values[1] - values[0];
    bool result = true;
    for (size_t i = SEQUENCE_MIN_TERMS_FOR_PATTERN - 1; i < values.size(); ++i) {
        if (std::fabs(values[i] - values[i - 1] - commonDiff) > SEQUENCE_EPSILON) {
            result = false;
            break;
        }
    }
    return CreateBoolValue(env, result);
}

napi_value IsGeometricSequence(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: sequence");

    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[ARG_INDEX_ZERO], &isArray));
    NAPI_ASSERT(env, isArray, "Sequence must be an array");

    std::vector<double> values;
    NAPI_ASSERT(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), "Sequence must be an array of numbers");
    NAPI_ASSERT(env, values.size() >= SEQUENCE_MIN_TERMS_FOR_PATTERN, "Sequence must have at least 3 terms");

    bool hasZero = false;
    for (size_t i = 0; i < values.size(); ++i) {
        if (std::fabs(values[i]) <= SEQUENCE_EPSILON) {
            hasZero = true;
        }
    }
    NAPI_ASSERT(env, !hasZero, "Sequence must not contain zero terms");

    double firstRatio = values[1] / values[0];
    bool result = true;
    for (size_t i = SEQUENCE_MIN_TERMS_FOR_PATTERN - 1; i < values.size(); ++i) {
        if (std::fabs(values[i] / values[i - 1] - firstRatio) > SEQUENCE_EPSILON) {
            result = false;
            break;
        }
    }
    return CreateBoolValue(env, result);
}

napi_value NextTerm(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: sequence");

    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[ARG_INDEX_ZERO], &isArray));
    NAPI_ASSERT(env, isArray, "Sequence must be an array");

    std::vector<double> values;
    NAPI_ASSERT(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), "Sequence must be an array of numbers");
    NAPI_ASSERT(env, values.size() >= SEQUENCE_MIN_TERMS_FOR_PATTERN, "Sequence must have at least 3 terms");

    size_t lastIndex = values.size() - 1;
    double diff = values[lastIndex] - values[lastIndex - 1];
    double result = values[lastIndex] + diff;
    return CreateDoubleValue(env, result);
}

napi_value NextTerms(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: sequence, count");

    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[ARG_INDEX_ZERO], &isArray));
    NAPI_ASSERT(env, isArray, "Sequence must be an array");

    std::vector<double> values;
    int64_t count = 0;
    NAPI_ASSERT(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), "Sequence must be an array of numbers");
    NAPI_ASSERT(env, values.size() >= SEQUENCE_MIN_TERMS_FOR_PATTERN, "Sequence must have at least 3 terms");
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], count), "Count must be an integer");
    NAPI_ASSERT(env, count >= LIST_MIN_COUNT, "Count must be at least 1");
    NAPI_ASSERT(env, count <= static_cast<int64_t>(LIST_MAX_COUNT), "Count must not exceed 1000");

    size_t lastIndex = values.size() - 1;
    double diff = values[lastIndex] - values[lastIndex - 1];
    double current = values[lastIndex];
    std::vector<double> predictions;
    predictions.reserve(static_cast<size_t>(count));
    for (int64_t i = 0; i < count; ++i) {
        current += diff;
        predictions.push_back(current);
    }
    return CreateDoubleArrayValue(env, predictions);
}

napi_value SequenceDifferences(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: sequence");

    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[ARG_INDEX_ZERO], &isArray));
    NAPI_ASSERT(env, isArray, "Sequence must be an array");

    std::vector<double> values;
    NAPI_ASSERT(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), "Sequence must be an array of numbers");
    NAPI_ASSERT(env, values.size() >= DIFFERENCES_MIN_TERMS, "Sequence must have at least 2 terms");

    std::vector<double> differences;
    differences.reserve(values.size() - DIFFERENCES_MIN_TERMS + 1);
    for (size_t i = DIFFERENCES_MIN_TERMS - 1; i < values.size(); ++i) {
        differences.push_back(values[i] - values[i - 1]);
    }
    return CreateDoubleArrayValue(env, differences);
}

napi_value Fibonacci(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n), "Index must be an integer");
    NAPI_ASSERT(env, n >= FIBONACCI_SEED_ZERO, "Index must be at least 0");
    NAPI_ASSERT(env, n <= FIBONACCI_MAX_INDEX, "Index must not exceed 78");

    int64_t previous = FIBONACCI_SEED_ZERO;
    int64_t current = FIBONACCI_SEED_ONE;
    for (int64_t i = FIBONACCI_SEED_ZERO; i < n; ++i) {
        int64_t next = previous + current;
        previous = current;
        current = next;
    }

    int64_t result = previous;
    return CreateInt64Value(env, result);
}

napi_value FibonacciList(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: count");

    int64_t count = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], count), "Count must be an integer");
    NAPI_ASSERT(env, count >= LIST_MIN_COUNT, "Count must be at least 1");
    NAPI_ASSERT(env, count <= FIBONACCI_MAX_INDEX + 1, "Count must not exceed 79");

    std::vector<int64_t> values;
    values.reserve(static_cast<size_t>(count));
    int64_t previous = FIBONACCI_SEED_ZERO;
    int64_t current = FIBONACCI_SEED_ONE;
    for (int64_t i = FIBONACCI_SEED_ZERO; i < count; ++i) {
        values.push_back(previous);
        int64_t next = previous + current;
        previous = current;
        current = next;
    }
    return CreateInt64ArrayValue(env, values);
}

napi_value IsFibonacci(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    int64_t value = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], value), "Value must be an integer");
    NAPI_ASSERT(env, value >= 0, "Value must be non-negative");

    int64_t square = value * value;
    int64_t candidate = FIBONACCI_DISCRIMINANT * square;
    bool lowerMatch = IsPerfectSquareValue(candidate - DISCRIMINANT_OFFSET);
    bool upperMatch = IsPerfectSquareValue(candidate + DISCRIMINANT_OFFSET);
    bool result = lowerMatch || upperMatch;
    return CreateBoolValue(env, result);
}

napi_value LucasNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n), "Index must be an integer");
    NAPI_ASSERT(env, n >= FIBONACCI_SEED_ZERO, "Index must be at least 0");
    NAPI_ASSERT(env, n <= LUCAS_MAX_INDEX, "Index must not exceed 78");

    int64_t previous = LUCAS_SEED_ZERO;
    int64_t current = LUCAS_SEED_ONE;
    for (int64_t i = FIBONACCI_SEED_ZERO; i < n; ++i) {
        int64_t next = previous + current;
        previous = current;
        current = next;
    }

    int64_t result = previous;
    return CreateInt64Value(env, result);
}

napi_value LucasList(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: count");

    int64_t count = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], count), "Count must be an integer");
    NAPI_ASSERT(env, count >= LIST_MIN_COUNT, "Count must be at least 1");
    NAPI_ASSERT(env, count <= LUCAS_MAX_INDEX + 1, "Count must not exceed 79");

    std::vector<int64_t> values;
    values.reserve(static_cast<size_t>(count));
    int64_t previous = LUCAS_SEED_ZERO;
    int64_t current = LUCAS_SEED_ONE;
    for (int64_t i = FIBONACCI_SEED_ZERO; i < count; ++i) {
        values.push_back(previous);
        int64_t next = previous + current;
        previous = current;
        current = next;
    }
    return CreateInt64ArrayValue(env, values);
}

napi_value TriangularNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n), "Index must be an integer");
    NAPI_ASSERT(env, n >= FIRST_TERM_INDEX, "Index must be at least 1");

    double term = static_cast<double>(n);
    double successor = term + 1.0;
    double result = term * successor / TRIANGULAR_DIVISOR;
    return CreateDoubleValue(env, result);
}

napi_value SquareNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n), "Index must be an integer");
    NAPI_ASSERT(env, n >= FIRST_TERM_INDEX, "Index must be at least 1");

    double term = static_cast<double>(n);
    double result = term * term;
    return CreateDoubleValue(env, result);
}

napi_value PentagonalNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n), "Index must be an integer");
    NAPI_ASSERT(env, n >= FIRST_TERM_INDEX, "Index must be at least 1");

    double term = static_cast<double>(n);
    double factor = PENTAGONAL_MULTIPLIER * term - PENTAGONAL_SUBTRAHEND;
    double result = term * factor / PENTAGONAL_DIVISOR;
    return CreateDoubleValue(env, result);
}

napi_value HexagonalNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n), "Index must be an integer");
    NAPI_ASSERT(env, n >= FIRST_TERM_INDEX, "Index must be at least 1");

    double term = static_cast<double>(n);
    double factor = HEXAGONAL_MULTIPLIER * term - HEXAGONAL_SUBTRAHEND;
    double result = term * factor;
    return CreateDoubleValue(env, result);
}

napi_value HeptagonalNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n), "Index must be an integer");
    NAPI_ASSERT(env, n >= FIRST_TERM_INDEX, "Index must be at least 1");

    double term = static_cast<double>(n);
    double factor = HEPTAGONAL_MULTIPLIER * term - HEPTAGONAL_SUBTRAHEND;
    double result = term * factor / HEPTAGONAL_DIVISOR;
    return CreateDoubleValue(env, result);
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("arithmeticTerm", ArithmeticTerm),
        DECLARE_NAPI_FUNCTION("arithmeticSum", ArithmeticSum),
        DECLARE_NAPI_FUNCTION("arithmeticMeans", ArithmeticMeans),
        DECLARE_NAPI_FUNCTION("geometricTerm", GeometricTerm),
        DECLARE_NAPI_FUNCTION("geometricSum", GeometricSum),
        DECLARE_NAPI_FUNCTION("geometricMean", GeometricMean),
        DECLARE_NAPI_FUNCTION("infiniteGeometricSum", InfiniteGeometricSum),
        DECLARE_NAPI_FUNCTION("isArithmeticSequence", IsArithmeticSequence),
        DECLARE_NAPI_FUNCTION("isGeometricSequence", IsGeometricSequence),
        DECLARE_NAPI_FUNCTION("nextTerm", NextTerm),
        DECLARE_NAPI_FUNCTION("nextTerms", NextTerms),
        DECLARE_NAPI_FUNCTION("sequenceDifferences", SequenceDifferences),
        DECLARE_NAPI_FUNCTION("fibonacci", Fibonacci),
        DECLARE_NAPI_FUNCTION("fibonacciList", FibonacciList),
        DECLARE_NAPI_FUNCTION("isFibonacci", IsFibonacci),
        DECLARE_NAPI_FUNCTION("lucasNumber", LucasNumber),
        DECLARE_NAPI_FUNCTION("lucasList", LucasList),
        DECLARE_NAPI_FUNCTION("triangularNumber", TriangularNumber),
        DECLARE_NAPI_FUNCTION("squareNumber", SquareNumber),
        DECLARE_NAPI_FUNCTION("pentagonalNumber", PentagonalNumber),
        DECLARE_NAPI_FUNCTION("hexagonalNumber", HexagonalNumber),
        DECLARE_NAPI_FUNCTION("heptagonalNumber", HeptagonalNumber),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
                                          sizeof(desc) / sizeof(desc[0]), desc));
    NAPI_CALL_BASE(env, RegisterProgressionOpsFunctions(env, exports), nullptr);
    return exports;
}

static napi_module progressionSuiteModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "progressionSuite",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void ProgressionSuiteRegisterModule(void)
{
    napi_module_register(&progressionSuiteModule);
}
