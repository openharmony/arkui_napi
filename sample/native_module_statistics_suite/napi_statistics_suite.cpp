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

#include "statistics_suite_common.h"

napi_status ExtractDoubleArray(napi_env env, napi_value value, std::vector<double>& result)
{
    bool isArray = false;
    NAPI_CALL_BASE(env, napi_is_array(env, value, &isArray), napi_invalid_arg);
    if (!isArray) {
        napi_throw_error(env, nullptr, "Argument must be an array");
        return napi_invalid_arg;
    }

    uint32_t length = 0;
    NAPI_CALL_BASE(env, napi_get_array_length(env, value, &length), napi_invalid_arg);

    result.clear();
    result.reserve(length);
    for (uint32_t i = 0; i < length; ++i) {
        napi_value element = nullptr;
        NAPI_CALL_BASE(env, napi_get_element(env, value, i, &element), napi_invalid_arg);
        napi_valuetype elementType = napi_undefined;
        NAPI_CALL_BASE(env, napi_typeof(env, element, &elementType), napi_invalid_arg);
        if (elementType != napi_number) {
            NAPI_CALL_BASE(env, napi_throw_error(env, nullptr, "Array elements must be numbers"),
                           napi_invalid_arg);
            return napi_invalid_arg;
        }
        double number = 0.0;
        NAPI_CALL_BASE(env, napi_get_value_double(env, element, &number), napi_invalid_arg);
        result.push_back(number);
    }
    return napi_ok;
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

std::vector<double> SortedCopy(const std::vector<double>& values)
{
    std::vector<double> sorted = values;
    std::sort(sorted.begin(), sorted.end());
    return sorted;
}

double ExtractScalarValue(napi_env env, napi_value value)
{
    napi_valuetype type = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, value, &type), 0.0);
    if (type != napi_number) {
        NAPI_CALL_BASE(env, napi_throw_error(env, nullptr, "Argument must be a number"), 0.0);
        return 0.0;
    }

    double result = 0.0;
    NAPI_CALL_BASE(env, napi_get_value_double(env, value, &result), 0.0);
    return result;
}

napi_value CreateNumberValue(napi_env env, double value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_double(env, value, &result));
    return result;
}

napi_value CreateBooleanValue(napi_env env, bool value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, value, &result));
    return result;
}

napi_value CreateUint32Value(napi_env env, uint32_t value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_uint32(env, value, &result));
    return result;
}

double ComputeSumValue(const std::vector<double>& values)
{
    double total = 0.0;
    for (size_t i = 0; i < values.size(); ++i) {
        total += values[i];
    }
    return total;
}

double ComputeSumSquaresValue(const std::vector<double>& values)
{
    double total = 0.0;
    for (size_t i = 0; i < values.size(); ++i) {
        total += values[i] * values[i];
    }
    return total;
}

double ComputeMeanValue(const std::vector<double>& values)
{
    return ComputeSumValue(values) / static_cast<double>(values.size());
}

double ComputeMedianValue(const std::vector<double>& values)
{
    std::vector<double> sorted = SortedCopy(values);
    size_t middle = sorted.size() / static_cast<size_t>(MIDPOINT_DIVISOR);
    double result = sorted[middle];
    if (sorted.size() % static_cast<size_t>(MIDPOINT_DIVISOR) == 0) {
        result = (sorted[middle - 1] + sorted[middle]) / MIDPOINT_DIVISOR;
    }
    return result;
}

double ComputeMinValue(const std::vector<double>& values)
{
    double result = values[0];
    for (size_t i = 1; i < values.size(); ++i) {
        if (values[i] < result) {
            result = values[i];
        }
    }
    return result;
}

double ComputeMaxValue(const std::vector<double>& values)
{
    double result = values[0];
    for (size_t i = 1; i < values.size(); ++i) {
        if (values[i] > result) {
            result = values[i];
        }
    }
    return result;
}

double ComputePopulationVariance(const std::vector<double>& values)
{
    double mean = ComputeMeanValue(values);
    double total = 0.0;
    for (size_t i = 0; i < values.size(); ++i) {
        double diff = values[i] - mean;
        total += diff * diff;
    }
    return total / static_cast<double>(values.size());
}

double ComputeSampleVariance(const std::vector<double>& values)
{
    double mean = ComputeMeanValue(values);
    double total = 0.0;
    for (size_t i = 0; i < values.size(); ++i) {
        double diff = values[i] - mean;
        total += diff * diff;
    }
    return total / static_cast<double>(values.size() - 1);
}

double ComputePopulationStdDevValue(const std::vector<double>& values)
{
    return std::sqrt(ComputePopulationVariance(values));
}

double ComputeSampleStdDevValue(const std::vector<double>& values)
{
    return std::sqrt(ComputeSampleVariance(values));
}

double ComputePercentileValue(const std::vector<double>& sortedValues, double percentile)
{
    if (sortedValues.size() == 1) {
        return sortedValues[0];
    }

    double position = (percentile / PERCENTILE_SCALE) * static_cast<double>(sortedValues.size() - 1);
    size_t lower = static_cast<size_t>(position);
    if (lower >= sortedValues.size() - 1) {
        return sortedValues[sortedValues.size() - 1];
    }

    double fraction = position - static_cast<double>(lower);
    double difference = sortedValues[lower + 1] - sortedValues[lower];
    return sortedValues[lower] + fraction * difference;
}

uint32_t CountAboveThreshold(const std::vector<double>& values, double threshold)
{
    uint32_t result = 0;
    for (size_t i = 0; i < values.size(); ++i) {
        if (values[i] > threshold) {
            ++result;
        }
    }
    return result;
}

uint32_t CountBelowThreshold(const std::vector<double>& values, double threshold)
{
    uint32_t result = 0;
    for (size_t i = 0; i < values.size(); ++i) {
        if (values[i] < threshold) {
            ++result;
        }
    }
    return result;
}

uint32_t CountBetweenBounds(const std::vector<double>& values, double low, double high)
{
    uint32_t result = 0;
    for (size_t i = 0; i < values.size(); ++i) {
        if (values[i] >= low && values[i] <= high) {
            ++result;
        }
    }
    return result;
}

uint32_t CountExactMatches(const std::vector<double>& values, double target)
{
    uint32_t result = 0;
    for (size_t i = 0; i < values.size(); ++i) {
        if (values[i] == target) {
            ++result;
        }
    }
    return result;
}

bool IsMonotonicValue(const std::vector<double>& values, bool increasing)
{
    for (size_t i = 1; i < values.size(); ++i) {
        if (increasing ? (values[i] < values[i - 1]) : (values[i] > values[i - 1])) {
            return false;
        }
    }
    return true;
}

size_t ExtremeIndexValue(const std::vector<double>& values, bool findMax)
{
    size_t index = 0;
    for (size_t i = 1; i < values.size(); ++i) {
        if (findMax ? (values[i] > values[index]) : (values[i] < values[index])) {
            index = i;
        }
    }
    return index;
}

napi_value Sum(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    double result = ComputeSumValue(values);
    return CreateNumberValue(env, result);
}

napi_value Mean(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double result = ComputeMeanValue(values);
    return CreateNumberValue(env, result);
}

napi_value Median(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double result = ComputeMedianValue(values);
    return CreateNumberValue(env, result);
}

napi_value Min(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double result = ComputeMinValue(values);
    return CreateNumberValue(env, result);
}

napi_value Max(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double result = ComputeMaxValue(values);
    return CreateNumberValue(env, result);
}

napi_value Range(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double result = ComputeMaxValue(values) - ComputeMinValue(values);
    return CreateNumberValue(env, result);
}

napi_value Count(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    return CreateUint32Value(env, static_cast<uint32_t>(values.size()));
}

napi_value Product(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    double result = 1.0;
    for (size_t i = 0; i < values.size(); ++i) {
        result *= values[i];
    }

    return CreateNumberValue(env, result);
}

napi_value Variance(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double result = ComputePopulationVariance(values);
    return CreateNumberValue(env, result);
}

napi_value SampleVariance(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, values.size() >= static_cast<size_t>(MIN_SAMPLES_FOR_VARIANCE),
                "Requires at least 2 values");

    double result = ComputeSampleVariance(values);
    return CreateNumberValue(env, result);
}

napi_value StdDev(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double result = ComputePopulationStdDevValue(values);
    return CreateNumberValue(env, result);
}

napi_value SampleStdDev(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, values.size() >= static_cast<size_t>(MIN_SAMPLES_FOR_VARIANCE),
                "Requires at least 2 values");

    double result = ComputeSampleStdDevValue(values);
    return CreateNumberValue(env, result);
}

napi_value GeometricMean(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double logSum = 0.0;
    for (size_t i = 0; i < values.size(); ++i) {
        NAPI_ASSERT(env, values[i] > 0.0, "All values must be positive");
        logSum += std::log(values[i]);
    }

    double result = std::exp(logSum / static_cast<double>(values.size()));
    return CreateNumberValue(env, result);
}

napi_value HarmonicMean(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double reciprocalSum = 0.0;
    for (size_t i = 0; i < values.size(); ++i) {
        NAPI_ASSERT(env, values[i] > 0.0, "All values must be positive");
        reciprocalSum += 1.0 / values[i];
    }

    double result = static_cast<double>(values.size()) / reciprocalSum;
    return CreateNumberValue(env, result);
}

napi_value MeanAbsDeviation(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double mean = ComputeMeanValue(values);
    double total = 0.0;
    for (size_t i = 0; i < values.size(); ++i) {
        total += std::fabs(values[i] - mean);
    }

    double result = total / static_cast<double>(values.size());
    return CreateNumberValue(env, result);
}

napi_value SumSquares(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    double result = ComputeSumSquaresValue(values);
    return CreateNumberValue(env, result);
}

napi_value RootMeanSquare(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double result = std::sqrt(ComputeSumSquaresValue(values) / static_cast<double>(values.size()));
    return CreateNumberValue(env, result);
}

napi_value Midrange(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double result = (ComputeMinValue(values) + ComputeMaxValue(values)) / MIDPOINT_DIVISOR;
    return CreateNumberValue(env, result);
}

napi_value Mode(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double result = values[0];
    size_t bestCount = 0;
    for (size_t i = 0; i < values.size(); ++i) {
        size_t occurrences = 0;
        for (size_t j = 0; j < values.size(); ++j) {
            if (values[j] == values[i]) {
                ++occurrences;
            }
        }
        if (occurrences > bestCount) {
            bestCount = occurrences;
            result = values[i];
        }
    }

    return CreateNumberValue(env, result);
}

napi_value CumulativeSum(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    std::vector<double> cumulative;
    double runningTotal = 0.0;
    for (size_t i = 0; i < values.size(); ++i) {
        runningTotal += values[i];
        cumulative.push_back(runningTotal);
    }

    return CreateDoubleArrayValue(env, cumulative);
}

napi_value MovingAverage(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: numbers, window");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    size_t window = static_cast<size_t>(ExtractScalarValue(env, argv[ARG_INDEX_ONE]));
    NAPI_ASSERT(env, window >= static_cast<size_t>(MIN_MOVING_WINDOW) && window <= values.size(),
                "Window size out of range");

    std::vector<double> averages;
    for (size_t i = window - 1; i < values.size(); ++i) {
        double total = 0.0;
        for (size_t j = i + 1 - window; j <= i; ++j) {
            total += values[j];
        }
        averages.push_back(total / static_cast<double>(window));
    }

    return CreateDoubleArrayValue(env, averages);
}

napi_value WeightedMean(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: values, weights");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    std::vector<double> weights;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ONE], weights), nullptr);

    NAPI_ASSERT(env, values.size() == weights.size(), "Values and weights must have same length");
    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double weightedTotal = 0.0;
    double weightSum = 0.0;
    for (size_t i = 0; i < values.size(); ++i) {
        weightedTotal += values[i] * weights[i];
        weightSum += weights[i];
    }

    NAPI_ASSERT(env, weightSum > 0.0, "Weights sum must be positive");

    double result = weightedTotal / weightSum;
    return CreateNumberValue(env, result);
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("sum", Sum),
        DECLARE_NAPI_FUNCTION("mean", Mean),
        DECLARE_NAPI_FUNCTION("median", Median),
        DECLARE_NAPI_FUNCTION("min", Min),
        DECLARE_NAPI_FUNCTION("max", Max),
        DECLARE_NAPI_FUNCTION("range", Range),
        DECLARE_NAPI_FUNCTION("count", Count),
        DECLARE_NAPI_FUNCTION("product", Product),
        DECLARE_NAPI_FUNCTION("variance", Variance),
        DECLARE_NAPI_FUNCTION("sampleVariance", SampleVariance),
        DECLARE_NAPI_FUNCTION("stdDev", StdDev),
        DECLARE_NAPI_FUNCTION("sampleStdDev", SampleStdDev),
        DECLARE_NAPI_FUNCTION("geometricMean", GeometricMean),
        DECLARE_NAPI_FUNCTION("harmonicMean", HarmonicMean),
        DECLARE_NAPI_FUNCTION("meanAbsDeviation", MeanAbsDeviation),
        DECLARE_NAPI_FUNCTION("sumSquares", SumSquares),
        DECLARE_NAPI_FUNCTION("rootMeanSquare", RootMeanSquare),
        DECLARE_NAPI_FUNCTION("midrange", Midrange),
        DECLARE_NAPI_FUNCTION("mode", Mode),
        DECLARE_NAPI_FUNCTION("cumulativeSum", CumulativeSum),
        DECLARE_NAPI_FUNCTION("movingAverage", MovingAverage),
        DECLARE_NAPI_FUNCTION("weightedMean", WeightedMean),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
                                          sizeof(desc) / sizeof(desc[0]), desc));
    NAPI_CALL_BASE(env, RegisterStatisticsOpsFunctions(env, exports), nullptr);
    return exports;
}

static napi_module statisticsSuiteModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "statisticsSuite",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void StatisticsSuiteRegisterModule(void)
{
    napi_module_register(&statisticsSuiteModule);
}
