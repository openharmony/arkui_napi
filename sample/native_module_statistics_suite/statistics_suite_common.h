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

#ifndef SAMPLE_NATIVE_MODULE_STATISTICS_SUITE_COMMON_H
#define SAMPLE_NATIVE_MODULE_STATISTICS_SUITE_COMMON_H

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <vector>

#include "napi/native_api.h"

constexpr int REQUIRED_ARGS_ONE = 1;
constexpr int REQUIRED_ARGS_TWO = 2;
constexpr int REQUIRED_ARGS_THREE = 3;
constexpr int ARG_INDEX_ZERO = 0;
constexpr int ARG_INDEX_ONE = 1;
constexpr int ARG_INDEX_TWO = 2;

constexpr double PERCENTILE_MIN = 0.0;
constexpr double PERCENTILE_MAX = 100.0;
constexpr double PERCENTILE_SCALE = 100.0;
constexpr double QUARTILE_FIRST = 25.0;
constexpr double QUARTILE_THIRD = 75.0;
constexpr int QUARTILE_LOW = 1;
constexpr int QUARTILE_HIGH = 3;
constexpr double TRIM_PERCENT_MAX = 50.0;

constexpr int MIN_MOVING_WINDOW = 1;
constexpr int SKEWNESS_MIN_SAMPLES = 3;
constexpr int KURTOSIS_MIN_SAMPLES = 4;
constexpr int MIN_SAMPLES_FOR_VARIANCE = 2;
constexpr int MIN_SAMPLES_FOR_DIFFERENCE = 2;

constexpr double MIDPOINT_DIVISOR = 2.0;
constexpr double EXCESS_KURTOSIS_OFFSET = 3.0;
constexpr double STAT_EPSILON = 1e-12;

napi_status ExtractDoubleArray(napi_env env, napi_value value, std::vector<double>& result);
napi_value CreateDoubleArrayValue(napi_env env, const std::vector<double>& values);
std::vector<double> SortedCopy(const std::vector<double>& values);
double ExtractScalarValue(napi_env env, napi_value value);
napi_value CreateNumberValue(napi_env env, double value);
napi_value CreateBooleanValue(napi_env env, bool value);
napi_value CreateUint32Value(napi_env env, uint32_t value);
double ComputeSumValue(const std::vector<double>& values);
double ComputeSumSquaresValue(const std::vector<double>& values);
double ComputeMeanValue(const std::vector<double>& values);
double ComputeMedianValue(const std::vector<double>& values);
double ComputeMinValue(const std::vector<double>& values);
double ComputeMaxValue(const std::vector<double>& values);
double ComputePopulationVariance(const std::vector<double>& values);
double ComputeSampleVariance(const std::vector<double>& values);
double ComputePopulationStdDevValue(const std::vector<double>& values);
double ComputeSampleStdDevValue(const std::vector<double>& values);
double ComputePercentileValue(const std::vector<double>& sortedValues, double percentile);
uint32_t CountAboveThreshold(const std::vector<double>& values, double threshold);
uint32_t CountBelowThreshold(const std::vector<double>& values, double threshold);
uint32_t CountBetweenBounds(const std::vector<double>& values, double low, double high);
uint32_t CountExactMatches(const std::vector<double>& values, double target);
bool IsMonotonicValue(const std::vector<double>& values, bool increasing);
size_t ExtremeIndexValue(const std::vector<double>& values, bool findMax);

napi_value Sum(napi_env env, napi_callback_info info);
napi_value Mean(napi_env env, napi_callback_info info);
napi_value Median(napi_env env, napi_callback_info info);
napi_value Min(napi_env env, napi_callback_info info);
napi_value Max(napi_env env, napi_callback_info info);
napi_value Range(napi_env env, napi_callback_info info);
napi_value Count(napi_env env, napi_callback_info info);
napi_value Product(napi_env env, napi_callback_info info);
napi_value Variance(napi_env env, napi_callback_info info);
napi_value SampleVariance(napi_env env, napi_callback_info info);
napi_value StdDev(napi_env env, napi_callback_info info);
napi_value SampleStdDev(napi_env env, napi_callback_info info);
napi_value GeometricMean(napi_env env, napi_callback_info info);
napi_value HarmonicMean(napi_env env, napi_callback_info info);
napi_value MeanAbsDeviation(napi_env env, napi_callback_info info);
napi_value SumSquares(napi_env env, napi_callback_info info);
napi_value RootMeanSquare(napi_env env, napi_callback_info info);
napi_value Midrange(napi_env env, napi_callback_info info);
napi_value Mode(napi_env env, napi_callback_info info);
napi_value CumulativeSum(napi_env env, napi_callback_info info);
napi_value MovingAverage(napi_env env, napi_callback_info info);
napi_value WeightedMean(napi_env env, napi_callback_info info);

napi_value Percentile(napi_env env, napi_callback_info info);
napi_value Quartile(napi_env env, napi_callback_info info);
napi_value InterquartileRange(napi_env env, napi_callback_info info);
napi_value TrimMean(napi_env env, napi_callback_info info);
napi_value Skewness(napi_env env, napi_callback_info info);
napi_value Kurtosis(napi_env env, napi_callback_info info);
napi_value ZScore(napi_env env, napi_callback_info info);
napi_value Normalize(napi_env env, napi_callback_info info);
napi_value MedianAbsDeviation(napi_env env, napi_callback_info info);
napi_value CoefficientOfVariation(napi_env env, napi_callback_info info);
napi_value StandardError(napi_env env, napi_callback_info info);
napi_value DeviationArray(napi_env env, napi_callback_info info);
napi_value DifferencesArray(napi_env env, napi_callback_info info);
napi_value IsMonotonicIncreasing(napi_env env, napi_callback_info info);
napi_value IsMonotonicDecreasing(napi_env env, napi_callback_info info);
napi_value MinIndex(napi_env env, napi_callback_info info);
napi_value MaxIndex(napi_env env, napi_callback_info info);
napi_value CountAbove(napi_env env, napi_callback_info info);
napi_value CountBelow(napi_env env, napi_callback_info info);
napi_value CountBetween(napi_env env, napi_callback_info info);
napi_value FrequencyOf(napi_env env, napi_callback_info info);
napi_value Describe(napi_env env, napi_callback_info info);

napi_status RegisterStatisticsOpsFunctions(napi_env env, napi_value exports);

#endif  // SAMPLE_NATIVE_MODULE_STATISTICS_SUITE_COMMON_H
