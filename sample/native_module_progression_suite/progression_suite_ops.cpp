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

#include "progression_suite_common.h"

static int64_t AdvanceCollatz(int64_t current)
{
    if (current % COLLATZ_DIVISOR == 0) {
        return current / COLLATZ_DIVISOR;
    }
    return COLLATZ_ODD_MULTIPLIER * current + 1;
}

napi_value OctagonalNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n), "Index must be an integer");
    NAPI_ASSERT(env, n >= FIRST_TERM_INDEX, "Index must be at least 1");

    double term = static_cast<double>(n);
    double result = term * (OCTAGONAL_MULTIPLIER * term - OCTAGONAL_SUBTRAHEND);
    return CreateDoubleValue(env, result);
}

napi_value TetrahedralNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n), "Index must be an integer");
    NAPI_ASSERT(env, n >= FIRST_TERM_INDEX, "Index must be at least 1");

    double term = static_cast<double>(n);
    double result = term * (term + TETRAHEDRAL_FACTOR_ONE) * (term + TETRAHEDRAL_FACTOR_TWO) / TETRAHEDRAL_DIVISOR;
    return CreateDoubleValue(env, result);
}

napi_value SquarePyramidalNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n), "Index must be an integer");
    NAPI_ASSERT(env, n >= FIRST_TERM_INDEX, "Index must be at least 1");

    double term = static_cast<double>(n);
    double result = term * (term + SQUARE_PYRAMID_FACTOR_ONE) *
                    (SQUARE_PYRAMID_FACTOR_TWO * term + SQUARE_PYRAMID_FACTOR_ONE) / SQUARE_PYRAMID_DIVISOR;
    return CreateDoubleValue(env, result);
}

napi_value CenteredSquareNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n), "Index must be an integer");
    NAPI_ASSERT(env, n >= 0, "Index must be at least 0");

    double term = static_cast<double>(n);
    double result = CENTERED_SQUARE_MULTIPLIER * term * term + CENTERED_SQUARE_MULTIPLIER * term + 1.0;
    return CreateDoubleValue(env, result);
}

napi_value SumRange(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: from, to");

    int64_t from = 0;
    int64_t to = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], from), "From must be an integer");
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], to), "To must be an integer");
    NAPI_ASSERT(env, from <= to, "From must be less than or equal to to");

    int64_t sum = 0;
    for (int64_t i = from; i <= to; ++i) {
        sum += i;
    }
    return CreateInt64Value(env, sum);
}

napi_value ProductRange(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: from, to");

    int64_t from = 0;
    int64_t to = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], from), "From must be an integer");
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], to), "To must be an integer");
    NAPI_ASSERT(env, from <= to, "From must be less than or equal to to");
    NAPI_ASSERT(env, to - from <= PRODUCT_RANGE_MAX_SPAN, "Range span must not exceed 60");

    int64_t product = 1;
    for (int64_t i = from; i <= to; ++i) {
        product *= i;
    }
    return CreateInt64Value(env, product);
}

napi_value SumOfSquaresRange(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: from, to");

    int64_t from = 0;
    int64_t to = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], from), "From must be an integer");
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], to), "To must be an integer");
    NAPI_ASSERT(env, from <= to, "From must be less than or equal to to");

    int64_t sum = 0;
    for (int64_t i = from; i <= to; ++i) {
        sum += i * i;
    }
    return CreateInt64Value(env, sum);
}

napi_value SumOfCubesRange(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: from, to");

    int64_t from = 0;
    int64_t to = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], from), "From must be an integer");
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], to), "To must be an integer");
    NAPI_ASSERT(env, from <= to, "From must be less than or equal to to");

    int64_t sum = 0;
    for (int64_t i = from; i <= to; ++i) {
        sum += i * i * i;
    }
    return CreateInt64Value(env, sum);
}

napi_value PowersOfTwo(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: count");

    int64_t count = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], count), "Count must be an integer");
    NAPI_ASSERT(env, count >= LIST_MIN_COUNT && count <= static_cast<int64_t>(POWERS_OF_TWO_MAX_COUNT),
                "Count must be between 1 and 63");

    std::vector<int64_t> values;
    int64_t power = 1;
    for (int64_t i = 0; i < count; ++i) {
        values.push_back(power);
        power *= POWERS_OF_TWO_BASE;
    }
    return CreateInt64ArrayValue(env, values);
}

napi_value PowerSum(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: base, count");

    int64_t base = 0;
    int64_t count = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], base), "Base must be an integer");
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], count), "Count must be an integer");
    NAPI_ASSERT(env, count >= LIST_MIN_COUNT && count <= static_cast<int64_t>(LIST_MAX_COUNT),
                "Count must be between 1 and 1000");

    int64_t sum = 0;
    int64_t term = POWER_SUM_SEED;
    for (int64_t i = 0; i < count; ++i) {
        sum += term;
        term *= base;
    }
    return CreateInt64Value(env, sum);
}

napi_value GeometricSequenceList(napi_env env, napi_callback_info info)
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
    NAPI_ASSERT(env, count >= LIST_MIN_COUNT && count <= static_cast<int64_t>(LIST_MAX_COUNT),
                "Count must be between 1 and 1000");

    std::vector<double> values;
    double term = first;
    for (int64_t i = 0; i < count; ++i) {
        values.push_back(term);
        term *= ratio;
    }
    return CreateDoubleArrayValue(env, values);
}

napi_value ArithmeticSequenceList(napi_env env, napi_callback_info info)
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
    NAPI_ASSERT(env, count >= LIST_MIN_COUNT && count <= static_cast<int64_t>(LIST_MAX_COUNT),
                "Count must be between 1 and 1000");

    std::vector<double> values;
    for (int64_t i = 0; i < count; ++i) {
        values.push_back(first + static_cast<double>(i) * diff);
    }
    return CreateDoubleArrayValue(env, values);
}

napi_value CollatzSteps(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n), "Value must be an integer");
    NAPI_ASSERT(env, n >= FIRST_TERM_INDEX, "Value must be at least 1");

    int64_t steps = 0;
    for (int64_t current = n; current != 1; ++steps) {
        NAPI_ASSERT(env, steps < COLLATZ_MAX_STEPS, "Collatz step limit exceeded");
        current = AdvanceCollatz(current);
    }
    return CreateInt64Value(env, steps);
}

napi_value CollatzSequence(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n), "Value must be an integer");
    NAPI_ASSERT(env, n >= FIRST_TERM_INDEX, "Value must be at least 1");

    std::vector<int64_t> values;
    int64_t current = n;
    values.push_back(current);
    while (current != 1) {
        NAPI_ASSERT(env, static_cast<int64_t>(values.size()) <= COLLATZ_MAX_STEPS, "Collatz step limit exceeded");
        current = AdvanceCollatz(current);
        values.push_back(current);
    }
    return CreateInt64ArrayValue(env, values);
}

napi_value CollatzMaxValue(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n), "Value must be an integer");
    NAPI_ASSERT(env, n >= FIRST_TERM_INDEX, "Value must be at least 1");

    int64_t current = n;
    int64_t maxValue = n;
    for (int64_t steps = 0; current != 1; ++steps) {
        NAPI_ASSERT(env, steps < COLLATZ_MAX_STEPS, "Collatz step limit exceeded");
        current = AdvanceCollatz(current);
        if (current > maxValue) {
            maxValue = current;
        }
    }
    return CreateInt64Value(env, maxValue);
}

napi_value PadovanNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n), "Index must be an integer");
    NAPI_ASSERT(env, n >= 0 && n <= PADOVAN_MAX_INDEX, "Index must be between 0 and 78");

    int64_t seedA = PADOVAN_SEED;
    int64_t seedB = PADOVAN_SEED;
    int64_t seedC = PADOVAN_SEED;
    for (int64_t i = static_cast<int64_t>(PADOVAN_WINDOW); i <= n; ++i) {
        int64_t next = seedA + seedB;
        seedA = seedB;
        seedB = seedC;
        seedC = next;
    }
    return CreateInt64Value(env, seedC);
}

napi_value JacobsthalNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n), "Index must be an integer");
    NAPI_ASSERT(env, n >= 0 && n <= JACOBSTHAL_MAX_INDEX, "Index must be between 0 and 62");

    int64_t previous = JACOBSTHAL_SEED_ZERO;
    int64_t current = JACOBSTHAL_SEED_ONE;
    for (int64_t i = JACOBSTHAL_SEED_ONE + 1; i <= n; ++i) {
        int64_t next = current + JACOBSTHAL_MULTIPLIER * previous;
        previous = current;
        current = next;
    }
    return CreateInt64Value(env, previous);
}

napi_value HarmonicNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: n");

    int64_t n = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], n), "Index must be an integer");
    NAPI_ASSERT(env, n >= HARMONIC_MIN_N && n <= HARMONIC_MAX_N, "Index must be between 1 and 1000");

    double sum = 0.0;
    for (int64_t k = HARMONIC_MIN_N; k <= n; ++k) {
        sum += 1.0 / static_cast<double>(k);
    }
    return CreateDoubleValue(env, sum);
}

napi_value AlternatingSum(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: sequence");

    std::vector<double> values;
    NAPI_ASSERT(env, ExtractDoubleArray(env, argv[ARG_INDEX_ZERO], values), "Sequence must be an array of numbers");
    NAPI_ASSERT(env, values.size() >= static_cast<size_t>(LIST_MIN_COUNT), "Sequence must have at least 1 term");

    double sum = 0.0;
    for (size_t i = 0; i < values.size(); ++i) {
        double sign = (i % ALTERNATING_SIGN_PERIOD == 0) ? 1.0 : -1.0;
        sum += sign * values[i];
    }
    return CreateDoubleValue(env, sum);
}

napi_value DigitSum(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    int64_t value = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], value), "Value must be an integer");
    NAPI_ASSERT(env, value >= 0, "Value must be non-negative");

    int64_t sum = 0;
    int64_t current = value;
    while (current > 0) {
        sum += current % DIGIT_BASE;
        current /= DIGIT_BASE;
    }
    return CreateInt64Value(env, sum);
}

napi_value DigitProduct(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    int64_t value = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], value), "Value must be an integer");
    NAPI_ASSERT(env, value >= 0, "Value must be non-negative");

    int64_t product = 1;
    int64_t current = value;
    while (current > 0) {
        product *= current % DIGIT_BASE;
        current /= DIGIT_BASE;
    }
    return CreateInt64Value(env, product);
}

napi_value IsPalindromicNumber(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    int64_t value = 0;
    NAPI_ASSERT(env, ExtractInt64Arg(env, argv[ARG_INDEX_ZERO], value), "Value must be an integer");
    NAPI_ASSERT(env, value >= 0, "Value must be non-negative");

    int64_t reversed = 0;
    int64_t current = value;
    while (current > 0) {
        reversed = reversed * DIGIT_BASE + current % DIGIT_BASE;
        current /= DIGIT_BASE;
    }

    bool result = reversed == value;
    return CreateBoolValue(env, result);
}

napi_status RegisterProgressionOpsFunctions(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("octagonalNumber", OctagonalNumber),
        DECLARE_NAPI_FUNCTION("tetrahedralNumber", TetrahedralNumber),
        DECLARE_NAPI_FUNCTION("squarePyramidalNumber", SquarePyramidalNumber),
        DECLARE_NAPI_FUNCTION("centeredSquareNumber", CenteredSquareNumber),
        DECLARE_NAPI_FUNCTION("sumRange", SumRange),
        DECLARE_NAPI_FUNCTION("productRange", ProductRange),
        DECLARE_NAPI_FUNCTION("sumOfSquaresRange", SumOfSquaresRange),
        DECLARE_NAPI_FUNCTION("sumOfCubesRange", SumOfCubesRange),
        DECLARE_NAPI_FUNCTION("powersOfTwo", PowersOfTwo),
        DECLARE_NAPI_FUNCTION("powerSum", PowerSum),
        DECLARE_NAPI_FUNCTION("geometricSequenceList", GeometricSequenceList),
        DECLARE_NAPI_FUNCTION("arithmeticSequenceList", ArithmeticSequenceList),
        DECLARE_NAPI_FUNCTION("collatzSteps", CollatzSteps),
        DECLARE_NAPI_FUNCTION("collatzSequence", CollatzSequence),
        DECLARE_NAPI_FUNCTION("collatzMaxValue", CollatzMaxValue),
        DECLARE_NAPI_FUNCTION("padovanNumber", PadovanNumber),
        DECLARE_NAPI_FUNCTION("jacobsthalNumber", JacobsthalNumber),
        DECLARE_NAPI_FUNCTION("harmonicNumber", HarmonicNumber),
        DECLARE_NAPI_FUNCTION("alternatingSum", AlternatingSum),
        DECLARE_NAPI_FUNCTION("digitSum", DigitSum),
        DECLARE_NAPI_FUNCTION("digitProduct", DigitProduct),
        DECLARE_NAPI_FUNCTION("isPalindromicNumber", IsPalindromicNumber),
    };
    return napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
}
