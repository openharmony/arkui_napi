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

napi_value Percentile(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: numbers, percentile");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    double percentile = ExtractScalarValue(env, argv[ARG_INDEX_ONE]);
    NAPI_ASSERT(env, percentile >= PERCENTILE_MIN && percentile <= PERCENTILE_MAX,
                "Percentile must be between 0 and 100");
    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double result = ComputePercentileValue(SortedCopy(values), percentile);
    return CreateNumberValue(env, result);
}

napi_value Quartile(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: numbers, quartile");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    int quartile = static_cast<int>(ExtractScalarValue(env, argv[ARG_INDEX_ONE]));
    NAPI_ASSERT(env, quartile >= QUARTILE_LOW && quartile <= QUARTILE_HIGH,
                "Quartile must be 1, 2 or 3");
    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double percentile = static_cast<double>(quartile) * QUARTILE_FIRST;
    double result = ComputePercentileValue(SortedCopy(values), percentile);
    return CreateNumberValue(env, result);
}

napi_value InterquartileRange(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    std::vector<double> sorted = SortedCopy(values);
    double result = ComputePercentileValue(sorted, QUARTILE_THIRD)
                    - ComputePercentileValue(sorted, QUARTILE_FIRST);
    return CreateNumberValue(env, result);
}

napi_value TrimMean(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: numbers, percent");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    double percent = ExtractScalarValue(env, argv[ARG_INDEX_ONE]);
    NAPI_ASSERT(env, percent >= PERCENTILE_MIN && percent <= TRIM_PERCENT_MAX,
                "Trim percent must be between 0 and 50");
    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    std::vector<double> sorted = SortedCopy(values);
    double trimRatio = percent / PERCENTILE_SCALE / MIDPOINT_DIVISOR;
    size_t trimCount = static_cast<size_t>(static_cast<double>(sorted.size()) * trimRatio);
    NAPI_ASSERT(env, sorted.size() - trimCount > trimCount, "Trimmed array is empty");

    size_t endTrim = sorted.size() - trimCount;
    double total = 0.0;
    for (size_t i = trimCount; i < endTrim; ++i) {
        total += sorted[i];
    }
    double result = total / static_cast<double>(endTrim - trimCount);
    return CreateNumberValue(env, result);
}

napi_value Skewness(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, values.size() >= static_cast<size_t>(SKEWNESS_MIN_SAMPLES),
                "Requires at least 3 values");

    double mean = ComputeMeanValue(values);
    double secondMoment = 0.0;
    double thirdMoment = 0.0;
    for (size_t i = 0; i < values.size(); ++i) {
        double diff = values[i] - mean;
        double squared = diff * diff;
        secondMoment += squared;
        thirdMoment += squared * diff;
    }
    double count = static_cast<double>(values.size());
    secondMoment /= count;
    thirdMoment /= count;
    NAPI_ASSERT(env, secondMoment > STAT_EPSILON, "Variance is zero");

    double result = thirdMoment / std::sqrt(secondMoment * secondMoment * secondMoment);
    return CreateNumberValue(env, result);
}

napi_value Kurtosis(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, values.size() >= static_cast<size_t>(KURTOSIS_MIN_SAMPLES),
                "Requires at least 4 values");

    double mean = ComputeMeanValue(values);
    double secondMoment = 0.0;
    double fourthMoment = 0.0;
    for (size_t i = 0; i < values.size(); ++i) {
        double diff = values[i] - mean;
        double squared = diff * diff;
        secondMoment += squared;
        fourthMoment += squared * squared;
    }
    double count = static_cast<double>(values.size());
    secondMoment /= count;
    fourthMoment /= count;
    NAPI_ASSERT(env, secondMoment > STAT_EPSILON, "Variance is zero");

    double result = fourthMoment / (secondMoment * secondMoment) - EXCESS_KURTOSIS_OFFSET;
    return CreateNumberValue(env, result);
}

napi_value ZScore(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: numbers, value");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    double value = ExtractScalarValue(env, argv[ARG_INDEX_ONE]);
    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double mean = ComputeMeanValue(values);
    double stdDev = ComputePopulationStdDevValue(values);
    if (stdDev == 0.0 || stdDev <= STAT_EPSILON) {
        NAPI_CALL(env, napi_throw_error(env, nullptr, "Standard deviation is zero"));
        return nullptr;
    }

    double result = (value - mean) / stdDev;
    return CreateNumberValue(env, result);
}

napi_value Normalize(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double mean = ComputeMeanValue(values);
    double stdDev = ComputePopulationStdDevValue(values);
    if (stdDev == 0.0 || stdDev <= STAT_EPSILON) {
        NAPI_CALL(env, napi_throw_error(env, nullptr, "Standard deviation is zero"));
        return nullptr;
    }

    std::vector<double> normalized;
    for (size_t i = 0; i < values.size(); ++i) {
        normalized.push_back((values[i] - mean) / stdDev);
    }
    return CreateDoubleArrayValue(env, normalized);
}

napi_value MedianAbsDeviation(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double median = ComputeMedianValue(values);
    std::vector<double> deviations;
    for (size_t i = 0; i < values.size(); ++i) {
        deviations.push_back(std::fabs(values[i] - median));
    }
    double result = ComputeMedianValue(deviations);
    return CreateNumberValue(env, result);
}

napi_value CoefficientOfVariation(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double mean = ComputeMeanValue(values);
    NAPI_ASSERT(env, std::fabs(mean) > STAT_EPSILON, "Mean must not be zero");
    double result = ComputePopulationStdDevValue(values) / std::fabs(mean);
    return CreateNumberValue(env, result);
}

napi_value StandardError(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, values.size() >= static_cast<size_t>(MIN_SAMPLES_FOR_VARIANCE),
                "Requires at least 2 values");

    double sampleStdDev = ComputeSampleStdDevValue(values);
    double result = sampleStdDev / std::sqrt(static_cast<double>(values.size()));
    return CreateNumberValue(env, result);
}

napi_value DeviationArray(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    double mean = ComputeMeanValue(values);
    std::vector<double> deviations;
    for (size_t i = 0; i < values.size(); ++i) {
        deviations.push_back(values[i] - mean);
    }
    return CreateDoubleArrayValue(env, deviations);
}

napi_value DifferencesArray(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, values.size() >= static_cast<size_t>(MIN_SAMPLES_FOR_DIFFERENCE),
                "Requires at least 2 values");

    std::vector<double> differences;
    for (size_t i = 1; i < values.size(); ++i) {
        differences.push_back(values[i] - values[i - 1]);
    }
    return CreateDoubleArrayValue(env, differences);
}

napi_value IsMonotonicIncreasing(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    bool result = IsMonotonicValue(values, true);
    return CreateBooleanValue(env, result);
}

napi_value IsMonotonicDecreasing(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    bool result = IsMonotonicValue(values, false);
    return CreateBooleanValue(env, result);
}

napi_value MinIndex(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    size_t index = ExtremeIndexValue(values, false);
    return CreateUint32Value(env, static_cast<uint32_t>(index));
}

napi_value MaxIndex(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    size_t index = ExtremeIndexValue(values, true);
    return CreateUint32Value(env, static_cast<uint32_t>(index));
}

napi_value CountAbove(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: numbers, threshold");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    double threshold = ExtractScalarValue(env, argv[ARG_INDEX_ONE]);
    return CreateUint32Value(env, CountAboveThreshold(values, threshold));
}

napi_value CountBelow(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: numbers, threshold");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    double threshold = ExtractScalarValue(env, argv[ARG_INDEX_ONE]);
    return CreateUint32Value(env, CountBelowThreshold(values, threshold));
}

napi_value CountBetween(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: numbers, low, high");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    double low = ExtractScalarValue(env, argv[ARG_INDEX_ONE]);
    double high = ExtractScalarValue(env, argv[ARG_INDEX_TWO]);
    return CreateUint32Value(env, CountBetweenBounds(values, low, high));
}

napi_value FrequencyOf(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: numbers, value");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    double target = ExtractScalarValue(env, argv[ARG_INDEX_ONE]);
    return CreateUint32Value(env, CountExactMatches(values, target));
}

napi_value Describe(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: numbers");

    std::vector<double> values;
    NAPI_CALL_BASE(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), nullptr);

    NAPI_ASSERT(env, !values.empty(), "Array must not be empty");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    napi_value countValue = CreateUint32Value(env, static_cast<uint32_t>(values.size()));
    NAPI_CALL(env, napi_set_named_property(env, result, "count", countValue));
    napi_value sumValue = CreateNumberValue(env, ComputeSumValue(values));
    NAPI_CALL(env, napi_set_named_property(env, result, "sum", sumValue));
    napi_value meanValue = CreateNumberValue(env, ComputeMeanValue(values));
    NAPI_CALL(env, napi_set_named_property(env, result, "mean", meanValue));
    napi_value medianValue = CreateNumberValue(env, ComputeMedianValue(values));
    NAPI_CALL(env, napi_set_named_property(env, result, "median", medianValue));
    napi_value minValue = CreateNumberValue(env, ComputeMinValue(values));
    NAPI_CALL(env, napi_set_named_property(env, result, "min", minValue));
    napi_value maxValue = CreateNumberValue(env, ComputeMaxValue(values));
    NAPI_CALL(env, napi_set_named_property(env, result, "max", maxValue));
    return result;
}

napi_status RegisterStatisticsOpsFunctions(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("percentile", Percentile),
        DECLARE_NAPI_FUNCTION("quartile", Quartile),
        DECLARE_NAPI_FUNCTION("interquartileRange", InterquartileRange),
        DECLARE_NAPI_FUNCTION("trimMean", TrimMean),
        DECLARE_NAPI_FUNCTION("skewness", Skewness),
        DECLARE_NAPI_FUNCTION("kurtosis", Kurtosis),
        DECLARE_NAPI_FUNCTION("zScore", ZScore),
        DECLARE_NAPI_FUNCTION("normalize", Normalize),
        DECLARE_NAPI_FUNCTION("medianAbsDeviation", MedianAbsDeviation),
        DECLARE_NAPI_FUNCTION("coefficientOfVariation", CoefficientOfVariation),
        DECLARE_NAPI_FUNCTION("standardError", StandardError),
        DECLARE_NAPI_FUNCTION("deviationArray", DeviationArray),
        DECLARE_NAPI_FUNCTION("differencesArray", DifferencesArray),
        DECLARE_NAPI_FUNCTION("isMonotonicIncreasing", IsMonotonicIncreasing),
        DECLARE_NAPI_FUNCTION("isMonotonicDecreasing", IsMonotonicDecreasing),
        DECLARE_NAPI_FUNCTION("minIndex", MinIndex),
        DECLARE_NAPI_FUNCTION("maxIndex", MaxIndex),
        DECLARE_NAPI_FUNCTION("countAbove", CountAbove),
        DECLARE_NAPI_FUNCTION("countBelow", CountBelow),
        DECLARE_NAPI_FUNCTION("countBetween", CountBetween),
        DECLARE_NAPI_FUNCTION("frequencyOf", FrequencyOf),
        DECLARE_NAPI_FUNCTION("describe", Describe),
    };
    NAPI_CALL_BASE(env, napi_define_properties(env, exports,
                                               sizeof(desc) / sizeof(desc[0]), desc),
                   napi_generic_failure);
    return napi_ok;
}
