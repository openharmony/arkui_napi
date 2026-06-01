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

#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

namespace {

// ============================================================
// Named constants — no magic numbers
// ============================================================

// Numeric boundary values
static constexpr int32_t INT32_MIN_VAL = INT32_MIN;            // -2147483648
static constexpr int32_t INT32_MAX_VAL = INT32_MAX;            //  2147483647
static constexpr uint32_t UINT32_MAX_VAL = UINT32_MAX;         //  4294967295
static constexpr int64_t INT64_MIN_VAL = INT64_MIN;
static constexpr int64_t INT64_MAX_VAL = INT64_MAX;
static constexpr uint64_t UINT64_MAX_VAL = UINT64_MAX;
static constexpr double DOUBLE_PI = 3.14159265358979323846;
static constexpr double DOUBLE_NEGATIVE_PI = -3.14159265358979323846;
static constexpr double DOUBLE_ZERO = 0.0;
static constexpr double DOUBLE_ONE = 1.0;
static constexpr double DOUBLE_NEGATIVE_ONE = -1.0;
static constexpr double DOUBLE_LARGE = 1.0e+15;
static constexpr double DOUBLE_SMALL = 1.0e-15;

// Integer test values
static constexpr int32_t INT32_ZERO = 0;
static constexpr int32_t INT32_ONE = 1;
static constexpr int32_t INT32_NEGATIVE_ONE = -1;
static constexpr int32_t INT32_FORTY_TWO = 42;
static constexpr uint32_t UINT32_ZERO = 0;
static constexpr uint32_t UINT32_ONE = 1;
static constexpr uint32_t UINT32_HUNDRED = 100;

// BigInt test values
static constexpr int64_t BIGINT_POSITIVE_SMALL = 42;
static constexpr int64_t BIGINT_NEGATIVE_SMALL = -42;
static constexpr int64_t BIGINT_ZERO = 0;
static constexpr uint64_t BIGINT_UINT_LARGE = 18446744073709551615ULL;  // UINT64_MAX
static constexpr uint64_t BIGINT_UINT_ZERO = 0;
static constexpr uint64_t BIGINT_UINT_ONE = 1;
static constexpr size_t BIGINT_WORD_COUNT_ONE = 1;
static constexpr size_t BIGINT_WORD_COUNT_TWO = 2;
static constexpr int BIGINT_SIGN_POSITIVE = 0;
static constexpr int BIGINT_SIGN_NEGATIVE = 1;
static constexpr uint64_t BIGINT_WORD_LOW = 0xFFFFFFFFFFFFFFFFULL;
static constexpr uint64_t BIGINT_WORD_HIGH = 0x1ULL;

// Date test values (milliseconds since epoch)
static constexpr double DATE_EPOCH = 0.0;
static constexpr double DATE_Y2K = 946684800000.0;       // 2000-01-01T00:00:00Z
static constexpr double DATE_UNIX_100K = 100000000000.0;  // ~1973
static constexpr double DATE_NEGATIVE = -86400000.0;      // 1969-12-31
static constexpr double DATE_TOLERANCE = 0.001;

// String test constants
static constexpr size_t STRING_BUFFER_SIZE = 256;
static constexpr size_t EMPTY_STRING_LENGTH = 0;
static constexpr size_t LATIN1_TEST_LENGTH = 5;

// UTF-16 test data
static constexpr char16_t UTF16_HELLO[] = u"Hello";
static constexpr size_t UTF16_HELLO_LENGTH = 5;
static constexpr char16_t UTF16_EMPTY[] = u"";
static constexpr size_t UTF16_EMPTY_LENGTH = 0;
static constexpr char16_t UTF16_SPECIAL[] = u"\u00E9\u00F1\u00FC";  // éñü
static constexpr size_t UTF16_SPECIAL_LENGTH = 3;
static constexpr char16_t UTF16_CJK[] = u"\u4F60\u597D";  // 你好
static constexpr size_t UTF16_CJK_LENGTH = 2;

// Latin-1 test data
static constexpr const char* LATIN1_HELLO = "Hello";
static constexpr const char* LATIN1_EMPTY = "";
static constexpr const char* LATIN1_SPECIAL = "\xE9\xF1\xFC";  // éñü in Latin-1
static constexpr size_t LATIN1_SPECIAL_LENGTH = 3;

// Coercion expected values
static constexpr int32_t COERCE_NUM_FROM_TRUE = 1;
static constexpr int32_t COERCE_NUM_FROM_FALSE = 0;
static constexpr int32_t COERCE_NUM_FROM_NULL = 0;

// Module registration
static constexpr uint32_t MODULE_VERSION = 1;
static constexpr uint32_t MODULE_FLAGS_NONE = 0;

// ============================================================
// Helper functions for setting named properties
// ============================================================

bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value)
{
    napi_value napiValue = nullptr;
    if (napi_get_boolean(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int32(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedDouble(napi_env env, napi_value object, const char* name, double value)
{
    napi_value napiValue = nullptr;
    if (napi_create_double(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedString(napi_env env, napi_value object, const char* name, const char* value)
{
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedInt64(napi_env env, napi_value object, const char* name, int64_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int64(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

// Helper: get a boolean value from napi_value
bool GetBoolValue(napi_env env, napi_value value)
{
    bool result = false;
    napi_get_value_bool(env, value, &result);
    return result;
}

// Helper: get coerced boolean result from a napi_value
bool CoerceToBoolValue(napi_env env, napi_value input)
{
    napi_value coerced = nullptr;
    napi_coerce_to_bool(env, input, &coerced);
    return GetBoolValue(env, coerced);
}

// Helper: get coerced number result from a napi_value
double CoerceToNumberValue(napi_env env, napi_value input)
{
    napi_value coerced = nullptr;
    napi_coerce_to_number(env, input, &coerced);
    double result = DOUBLE_ZERO;
    napi_get_value_double(env, coerced, &result);
    return result;
}

// ============================================================
// Test 1: napi_coerce_to_bool — boolean coercion of number zero
// ============================================================
static napi_value TestCoerceBoolFromZero(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value zeroVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_ZERO, &zeroVal));
    bool coerced = CoerceToBoolValue(env, zeroVal);
    SetNamedBool(env, result, "zeroIsFalse", !coerced);

    napi_value oneVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_ONE, &oneVal));
    bool coercedOne = CoerceToBoolValue(env, oneVal);
    SetNamedBool(env, result, "oneIsTrue", coercedOne);

    return result;
}

// ============================================================
// Test 2: napi_coerce_to_bool — empty string and non-empty string
// ============================================================
static napi_value TestCoerceBoolFromString(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value emptyStr = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "", EMPTY_STRING_LENGTH, &emptyStr));
    SetNamedBool(env, result, "emptyStringIsFalse", !CoerceToBoolValue(env, emptyStr));

    napi_value nonEmptyStr = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "hello", NAPI_AUTO_LENGTH, &nonEmptyStr));
    SetNamedBool(env, result, "nonEmptyStringIsTrue", CoerceToBoolValue(env, nonEmptyStr));

    return result;
}

// ============================================================
// Test 3: napi_coerce_to_bool — null and undefined
// ============================================================
static napi_value TestCoerceBoolFromNullUndefined(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value nullVal = nullptr;
    NAPI_CALL(env, napi_get_null(env, &nullVal));
    SetNamedBool(env, result, "nullIsFalse", !CoerceToBoolValue(env, nullVal));

    napi_value undefVal = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undefVal));
    SetNamedBool(env, result, "undefinedIsFalse", !CoerceToBoolValue(env, undefVal));

    return result;
}

// ============================================================
// Test 4: napi_coerce_to_bool — object is always truthy
// ============================================================
static napi_value TestCoerceBoolFromObject(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedBool(env, result, "objectIsTrue", CoerceToBoolValue(env, obj));

    napi_value arr = nullptr;
    NAPI_CALL(env, napi_create_array(env, &arr));
    SetNamedBool(env, result, "arrayIsTrue", CoerceToBoolValue(env, arr));

    return result;
}

// ============================================================
// Test 5: napi_coerce_to_number — from boolean
// ============================================================
static napi_value TestCoerceNumberFromBool(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value trueVal = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, true, &trueVal));
    double numTrue = CoerceToNumberValue(env, trueVal);
    SetNamedBool(env, result, "trueIsOne", numTrue == static_cast<double>(COERCE_NUM_FROM_TRUE));

    napi_value falseVal = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, false, &falseVal));
    double numFalse = CoerceToNumberValue(env, falseVal);
    SetNamedBool(env, result, "falseIsZero", numFalse == static_cast<double>(COERCE_NUM_FROM_FALSE));

    return result;
}

// ============================================================
// Test 6: napi_coerce_to_number — from null
// ============================================================
static napi_value TestCoerceNumberFromNull(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value nullVal = nullptr;
    NAPI_CALL(env, napi_get_null(env, &nullVal));
    double numNull = CoerceToNumberValue(env, nullVal);
    SetNamedBool(env, result, "nullIsZero", numNull == static_cast<double>(COERCE_NUM_FROM_NULL));

    napi_value undefVal = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undefVal));
    double numUndef = CoerceToNumberValue(env, undefVal);
    SetNamedBool(env, result, "undefinedIsNaN", std::isnan(numUndef));

    return result;
}

// ============================================================
// Test 7: napi_coerce_to_string — from number
// ============================================================
static napi_value TestCoerceStringFromNumber(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value numVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_FORTY_TWO, &numVal));

    napi_value coerced = nullptr;
    NAPI_CALL(env, napi_coerce_to_string(env, numVal, &coerced));

    char buf[STRING_BUFFER_SIZE] = {0};
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, coerced, buf, sizeof(buf), &len));
    SetNamedBool(env, result, "numberToStringOk", strcmp(buf, "42") == INT32_ZERO);
    SetNamedString(env, result, "coercedValue", buf);

    return result;
}

// ============================================================
// Test 8: napi_coerce_to_string — from boolean
// ============================================================
static napi_value TestCoerceStringFromBool(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value trueVal = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, true, &trueVal));
    napi_value coercedTrue = nullptr;
    NAPI_CALL(env, napi_coerce_to_string(env, trueVal, &coercedTrue));

    char bufTrue[STRING_BUFFER_SIZE] = {0};
    size_t lenTrue = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, coercedTrue, bufTrue, sizeof(bufTrue), &lenTrue));
    SetNamedBool(env, result, "trueToString", strcmp(bufTrue, "true") == INT32_ZERO);

    napi_value falseVal = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, false, &falseVal));
    napi_value coercedFalse = nullptr;
    NAPI_CALL(env, napi_coerce_to_string(env, falseVal, &coercedFalse));

    char bufFalse[STRING_BUFFER_SIZE] = {0};
    size_t lenFalse = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, coercedFalse, bufFalse, sizeof(bufFalse), &lenFalse));
    SetNamedBool(env, result, "falseToString", strcmp(bufFalse, "false") == INT32_ZERO);

    return result;
}

// ============================================================
// Test 9: napi_coerce_to_object — from number
// ============================================================
static napi_value TestCoerceObjectFromNumber(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value numVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_FORTY_TWO, &numVal));

    napi_value coerced = nullptr;
    NAPI_CALL(env, napi_coerce_to_object(env, numVal, &coerced));

    napi_valuetype objType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, coerced, &objType));
    SetNamedBool(env, result, "isObject", objType == napi_object);

    return result;
}

// ============================================================
// Test 10: napi_coerce_to_object — from string
// ============================================================
static napi_value TestCoerceObjectFromString(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value strVal = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "test", NAPI_AUTO_LENGTH, &strVal));

    napi_value coerced = nullptr;
    NAPI_CALL(env, napi_coerce_to_object(env, strVal, &coerced));

    napi_valuetype objType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, coerced, &objType));
    SetNamedBool(env, result, "stringToObject", objType == napi_object);

    return result;
}

// ============================================================
// Test 11: napi_create_bigint_int64 / napi_get_value_bigint_int64 round-trip
// ============================================================
static napi_value TestBigintInt64RoundTrip(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    // Positive value
    napi_value bigintPos = nullptr;
    NAPI_CALL(env, napi_create_bigint_int64(env, BIGINT_POSITIVE_SMALL, &bigintPos));
    int64_t valPos = BIGINT_ZERO;
    bool losslessPos = false;
    NAPI_CALL(env, napi_get_value_bigint_int64(env, bigintPos, &valPos, &losslessPos));
    SetNamedBool(env, result, "positiveMatch", valPos == BIGINT_POSITIVE_SMALL);
    SetNamedBool(env, result, "positiveLossless", losslessPos);

    // Negative value
    napi_value bigintNeg = nullptr;
    NAPI_CALL(env, napi_create_bigint_int64(env, BIGINT_NEGATIVE_SMALL, &bigintNeg));
    int64_t valNeg = BIGINT_ZERO;
    bool losslessNeg = false;
    NAPI_CALL(env, napi_get_value_bigint_int64(env, bigintNeg, &valNeg, &losslessNeg));
    SetNamedBool(env, result, "negativeMatch", valNeg == BIGINT_NEGATIVE_SMALL);
    SetNamedBool(env, result, "negativeLossless", losslessNeg);

    return result;
}

// ============================================================
// Test 12: BigInt int64 boundary values
// ============================================================
static napi_value TestBigintInt64Boundary(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    // INT64_MIN
    napi_value bigintMin = nullptr;
    NAPI_CALL(env, napi_create_bigint_int64(env, INT64_MIN_VAL, &bigintMin));
    int64_t valMin = BIGINT_ZERO;
    bool lossMin = false;
    NAPI_CALL(env, napi_get_value_bigint_int64(env, bigintMin, &valMin, &lossMin));
    SetNamedBool(env, result, "int64MinMatch", valMin == INT64_MIN_VAL);

    // INT64_MAX
    napi_value bigintMax = nullptr;
    NAPI_CALL(env, napi_create_bigint_int64(env, INT64_MAX_VAL, &bigintMax));
    int64_t valMax = BIGINT_ZERO;
    bool lossMax = false;
    NAPI_CALL(env, napi_get_value_bigint_int64(env, bigintMax, &valMax, &lossMax));
    SetNamedBool(env, result, "int64MaxMatch", valMax == INT64_MAX_VAL);

    // Zero
    napi_value bigintZero = nullptr;
    NAPI_CALL(env, napi_create_bigint_int64(env, BIGINT_ZERO, &bigintZero));
    int64_t valZero = INT64_MAX_VAL;
    bool lossZero = false;
    NAPI_CALL(env, napi_get_value_bigint_int64(env, bigintZero, &valZero, &lossZero));
    SetNamedBool(env, result, "zeroMatch", valZero == BIGINT_ZERO);

    return result;
}

// ============================================================
// Test 13: napi_create_bigint_uint64 / napi_get_value_bigint_uint64 round-trip
// ============================================================
static napi_value TestBigintUint64RoundTrip(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    // UINT64_MAX
    napi_value bigintMax = nullptr;
    NAPI_CALL(env, napi_create_bigint_uint64(env, BIGINT_UINT_LARGE, &bigintMax));
    uint64_t valMax = BIGINT_UINT_ZERO;
    bool lossMax = false;
    NAPI_CALL(env, napi_get_value_bigint_uint64(env, bigintMax, &valMax, &lossMax));
    SetNamedBool(env, result, "uint64MaxMatch", valMax == BIGINT_UINT_LARGE);
    SetNamedBool(env, result, "uint64MaxLossless", lossMax);

    // Zero
    napi_value bigintZero = nullptr;
    NAPI_CALL(env, napi_create_bigint_uint64(env, BIGINT_UINT_ZERO, &bigintZero));
    uint64_t valZero = BIGINT_UINT_ONE;
    bool lossZero = false;
    NAPI_CALL(env, napi_get_value_bigint_uint64(env, bigintZero, &valZero, &lossZero));
    SetNamedBool(env, result, "uint64ZeroMatch", valZero == BIGINT_UINT_ZERO);

    // One
    napi_value bigintOne = nullptr;
    NAPI_CALL(env, napi_create_bigint_uint64(env, BIGINT_UINT_ONE, &bigintOne));
    uint64_t valOne = BIGINT_UINT_ZERO;
    bool lossOne = false;
    NAPI_CALL(env, napi_get_value_bigint_uint64(env, bigintOne, &valOne, &lossOne));
    SetNamedBool(env, result, "uint64OneMatch", valOne == BIGINT_UINT_ONE);

    return result;
}

// ============================================================
// Test 14: napi_create_bigint_words / napi_get_value_bigint_words — single word
// ============================================================
static napi_value TestBigintWordsSingleWord(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    uint64_t wordsIn[BIGINT_WORD_COUNT_ONE] = {BIGINT_WORD_LOW};
    napi_value bigint = nullptr;
    NAPI_CALL(env, napi_create_bigint_words(env, BIGINT_SIGN_POSITIVE,
                                             BIGINT_WORD_COUNT_ONE, wordsIn, &bigint));

    int signBit = BIGINT_SIGN_NEGATIVE;
    size_t wordCount = BIGINT_WORD_COUNT_ONE;
    NAPI_CALL(env, napi_get_value_bigint_words(env, bigint, &signBit, &wordCount, nullptr));

    uint64_t wordsOut[BIGINT_WORD_COUNT_ONE] = {BIGINT_UINT_ZERO};
    NAPI_CALL(env, napi_get_value_bigint_words(env, bigint, &signBit, &wordCount, wordsOut));

    SetNamedBool(env, result, "signIsPositive", signBit == BIGINT_SIGN_POSITIVE);
    SetNamedBool(env, result, "wordCountIsOne", wordCount == BIGINT_WORD_COUNT_ONE);
    SetNamedBool(env, result, "wordValueMatch", wordsOut[0] == BIGINT_WORD_LOW);

    return result;
}

// ============================================================
// Test 15: napi_create_bigint_words — two words (128-bit)
// ============================================================
static napi_value TestBigintWordsTwoWords(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    uint64_t wordsIn[BIGINT_WORD_COUNT_TWO] = {BIGINT_WORD_LOW, BIGINT_WORD_HIGH};
    napi_value bigint = nullptr;
    NAPI_CALL(env, napi_create_bigint_words(env, BIGINT_SIGN_NEGATIVE,
                                             BIGINT_WORD_COUNT_TWO, wordsIn, &bigint));

    int signBit = BIGINT_SIGN_POSITIVE;
    size_t wordCount = BIGINT_WORD_COUNT_TWO;
    NAPI_CALL(env, napi_get_value_bigint_words(env, bigint, &signBit, &wordCount, nullptr));

    uint64_t wordsOut[BIGINT_WORD_COUNT_TWO] = {BIGINT_UINT_ZERO, BIGINT_UINT_ZERO};
    NAPI_CALL(env, napi_get_value_bigint_words(env, bigint, &signBit, &wordCount, wordsOut));

    SetNamedBool(env, result, "signIsNegative", signBit == BIGINT_SIGN_NEGATIVE);
    SetNamedBool(env, result, "wordCountIsTwo", wordCount == BIGINT_WORD_COUNT_TWO);
    SetNamedBool(env, result, "lowWordMatch", wordsOut[0] == BIGINT_WORD_LOW);
    SetNamedBool(env, result, "highWordMatch", wordsOut[1] == BIGINT_WORD_HIGH);

    return result;
}

// ============================================================
// Test 16: napi_create_string_utf16 / napi_get_value_string_utf16 — basic
// ============================================================
static napi_value TestStringUtf16Basic(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf16(env, UTF16_HELLO, UTF16_HELLO_LENGTH, &str));

    char16_t buf[STRING_BUFFER_SIZE] = {0};
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf16(env, str, buf, STRING_BUFFER_SIZE, &len));

    SetNamedBool(env, result, "lengthMatch", len == UTF16_HELLO_LENGTH);
    bool contentMatch = (memcmp(buf, UTF16_HELLO, UTF16_HELLO_LENGTH * sizeof(char16_t)) == INT32_ZERO);
    SetNamedBool(env, result, "contentMatch", contentMatch);

    return result;
}

// ============================================================
// Test 17: napi_create_string_utf16 — empty string
// ============================================================
static napi_value TestStringUtf16Empty(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf16(env, UTF16_EMPTY, UTF16_EMPTY_LENGTH, &str));

    char16_t buf[STRING_BUFFER_SIZE] = {0};
    size_t len = UINT32_ONE;
    NAPI_CALL(env, napi_get_value_string_utf16(env, str, buf, STRING_BUFFER_SIZE, &len));

    SetNamedBool(env, result, "emptyLengthZero", len == UTF16_EMPTY_LENGTH);

    return result;
}

// ============================================================
// Test 18: napi_create_string_utf16 — special characters (accented)
// ============================================================
static napi_value TestStringUtf16Special(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf16(env, UTF16_SPECIAL, UTF16_SPECIAL_LENGTH, &str));

    char16_t buf[STRING_BUFFER_SIZE] = {0};
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf16(env, str, buf, STRING_BUFFER_SIZE, &len));

    SetNamedBool(env, result, "specialLengthMatch", len == UTF16_SPECIAL_LENGTH);
    bool contentMatch = (memcmp(buf, UTF16_SPECIAL, UTF16_SPECIAL_LENGTH * sizeof(char16_t)) == INT32_ZERO);
    SetNamedBool(env, result, "specialContentMatch", contentMatch);

    return result;
}

// ============================================================
// Test 19: napi_create_string_utf16 — CJK characters
// ============================================================
static napi_value TestStringUtf16Cjk(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf16(env, UTF16_CJK, UTF16_CJK_LENGTH, &str));

    char16_t buf[STRING_BUFFER_SIZE] = {0};
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf16(env, str, buf, STRING_BUFFER_SIZE, &len));

    SetNamedBool(env, result, "cjkLengthMatch", len == UTF16_CJK_LENGTH);
    bool contentMatch = (memcmp(buf, UTF16_CJK, UTF16_CJK_LENGTH * sizeof(char16_t)) == INT32_ZERO);
    SetNamedBool(env, result, "cjkContentMatch", contentMatch);

    return result;
}

// ============================================================
// Test 20: napi_create_string_latin1 / napi_get_value_string_latin1 — basic
// ============================================================
static napi_value TestStringLatin1Basic(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, LATIN1_HELLO, NAPI_AUTO_LENGTH, &str));

    char buf[STRING_BUFFER_SIZE] = {0};
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_latin1(env, str, buf, sizeof(buf), &len));

    SetNamedBool(env, result, "latin1LengthMatch", len == LATIN1_TEST_LENGTH);
    SetNamedBool(env, result, "latin1ContentMatch", strcmp(buf, LATIN1_HELLO) == INT32_ZERO);

    return result;
}

// ============================================================
// Test 21: napi_create_string_latin1 — empty string
// ============================================================
static napi_value TestStringLatin1Empty(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, LATIN1_EMPTY, EMPTY_STRING_LENGTH, &str));

    char buf[STRING_BUFFER_SIZE] = {0};
    size_t len = UINT32_ONE;
    NAPI_CALL(env, napi_get_value_string_latin1(env, str, buf, sizeof(buf), &len));

    SetNamedBool(env, result, "emptyLatin1LenZero", len == EMPTY_STRING_LENGTH);

    return result;
}

// ============================================================
// Test 22: napi_create_string_latin1 — special chars (éñü)
// ============================================================
static napi_value TestStringLatin1Special(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, LATIN1_SPECIAL, LATIN1_SPECIAL_LENGTH, &str));

    char buf[STRING_BUFFER_SIZE] = {0};
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_latin1(env, str, buf, sizeof(buf), &len));

    SetNamedBool(env, result, "latin1SpecialLenMatch", len == LATIN1_SPECIAL_LENGTH);

    return result;
}

// ============================================================
// Test 23: napi_create_date / napi_get_date_value / napi_is_date — epoch
// ============================================================
static napi_value TestDateEpoch(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value dateVal = nullptr;
    NAPI_CALL(env, napi_create_date(env, DATE_EPOCH, &dateVal));

    bool isDate = false;
    NAPI_CALL(env, napi_is_date(env, dateVal, &isDate));
    SetNamedBool(env, result, "isDate", isDate);

    double retrieved = DOUBLE_NEGATIVE_ONE;
    NAPI_CALL(env, napi_get_date_value(env, dateVal, &retrieved));
    SetNamedBool(env, result, "epochMatch", fabs(retrieved - DATE_EPOCH) < DATE_TOLERANCE);

    return result;
}

// ============================================================
// Test 24: Date — Y2K and negative timestamps
// ============================================================
static napi_value TestDateY2kAndNegative(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    // Y2K
    napi_value y2kDate = nullptr;
    NAPI_CALL(env, napi_create_date(env, DATE_Y2K, &y2kDate));
    double y2kVal = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_date_value(env, y2kDate, &y2kVal));
    SetNamedBool(env, result, "y2kMatch", fabs(y2kVal - DATE_Y2K) < DATE_TOLERANCE);

    // Negative (before epoch)
    napi_value negDate = nullptr;
    NAPI_CALL(env, napi_create_date(env, DATE_NEGATIVE, &negDate));
    double negVal = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_date_value(env, negDate, &negVal));
    SetNamedBool(env, result, "negativeMatch", fabs(negVal - DATE_NEGATIVE) < DATE_TOLERANCE);

    return result;
}

// ============================================================
// Test 25: napi_is_date — non-date objects
// ============================================================
static napi_value TestIsDateNonDate(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    // Number is not a date
    napi_value numVal = nullptr;
    NAPI_CALL(env, napi_create_double(env, DATE_UNIX_100K, &numVal));
    bool isDateNum = true;
    NAPI_CALL(env, napi_is_date(env, numVal, &isDateNum));
    SetNamedBool(env, result, "numberIsNotDate", !isDateNum);

    // Object is not a date
    napi_value objVal = nullptr;
    NAPI_CALL(env, napi_create_object(env, &objVal));
    bool isDateObj = true;
    NAPI_CALL(env, napi_is_date(env, objVal, &isDateObj));
    SetNamedBool(env, result, "objectIsNotDate", !isDateObj);

    // String is not a date
    napi_value strVal = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "2000-01-01", NAPI_AUTO_LENGTH, &strVal));
    bool isDateStr = true;
    NAPI_CALL(env, napi_is_date(env, strVal, &isDateStr));
    SetNamedBool(env, result, "stringIsNotDate", !isDateStr);

    return result;
}

// ============================================================
// Test 26: Int32 round-trip with boundary values
// ============================================================
static napi_value TestInt32RoundTrip(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    // INT32_MIN
    napi_value minVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_MIN_VAL, &minVal));
    int32_t gotMin = INT32_ZERO;
    NAPI_CALL(env, napi_get_value_int32(env, minVal, &gotMin));
    SetNamedBool(env, result, "int32MinMatch", gotMin == INT32_MIN_VAL);

    // INT32_MAX
    napi_value maxVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_MAX_VAL, &maxVal));
    int32_t gotMax = INT32_ZERO;
    NAPI_CALL(env, napi_get_value_int32(env, maxVal, &gotMax));
    SetNamedBool(env, result, "int32MaxMatch", gotMax == INT32_MAX_VAL);

    // Zero
    napi_value zeroVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_ZERO, &zeroVal));
    int32_t gotZero = INT32_ONE;
    NAPI_CALL(env, napi_get_value_int32(env, zeroVal, &gotZero));
    SetNamedBool(env, result, "int32ZeroMatch", gotZero == INT32_ZERO);

    return result;
}

// ============================================================
// Test 27: Uint32 round-trip with boundary values
// ============================================================
static napi_value TestUint32RoundTrip(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    // UINT32_MAX
    napi_value maxVal = nullptr;
    NAPI_CALL(env, napi_create_uint32(env, UINT32_MAX_VAL, &maxVal));
    uint32_t gotMax = UINT32_ZERO;
    NAPI_CALL(env, napi_get_value_uint32(env, maxVal, &gotMax));
    SetNamedBool(env, result, "uint32MaxMatch", gotMax == UINT32_MAX_VAL);

    // Zero
    napi_value zeroVal = nullptr;
    NAPI_CALL(env, napi_create_uint32(env, UINT32_ZERO, &zeroVal));
    uint32_t gotZero = UINT32_ONE;
    NAPI_CALL(env, napi_get_value_uint32(env, zeroVal, &gotZero));
    SetNamedBool(env, result, "uint32ZeroMatch", gotZero == UINT32_ZERO);

    // Hundred
    napi_value hundredVal = nullptr;
    NAPI_CALL(env, napi_create_uint32(env, UINT32_HUNDRED, &hundredVal));
    uint32_t gotHundred = UINT32_ZERO;
    NAPI_CALL(env, napi_get_value_uint32(env, hundredVal, &gotHundred));
    SetNamedBool(env, result, "uint32HundredMatch", gotHundred == UINT32_HUNDRED);

    return result;
}

// ============================================================
// Test 28: Int64 round-trip with boundary values
// ============================================================
static napi_value TestInt64RoundTrip(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    // INT64_MIN
    napi_value minVal = nullptr;
    NAPI_CALL(env, napi_create_int64(env, INT64_MIN_VAL, &minVal));
    int64_t gotMin = BIGINT_ZERO;
    NAPI_CALL(env, napi_get_value_int64(env, minVal, &gotMin));
    SetNamedBool(env, result, "int64MinMatch", gotMin == INT64_MIN_VAL);

    // INT64_MAX
    napi_value maxVal = nullptr;
    NAPI_CALL(env, napi_create_int64(env, INT64_MAX_VAL, &maxVal));
    int64_t gotMax = BIGINT_ZERO;
    NAPI_CALL(env, napi_get_value_int64(env, maxVal, &gotMax));
    SetNamedBool(env, result, "int64MaxMatch", gotMax == INT64_MAX_VAL);

    return result;
}

// ============================================================
// Test 29: Double round-trip — PI, large, small values
// ============================================================
static napi_value TestDoubleRoundTrip(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    // PI
    napi_value piVal = nullptr;
    NAPI_CALL(env, napi_create_double(env, DOUBLE_PI, &piVal));
    double gotPi = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_value_double(env, piVal, &gotPi));
    SetNamedBool(env, result, "piMatch", gotPi == DOUBLE_PI);

    // Negative PI
    napi_value negPiVal = nullptr;
    NAPI_CALL(env, napi_create_double(env, DOUBLE_NEGATIVE_PI, &negPiVal));
    double gotNegPi = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_value_double(env, negPiVal, &gotNegPi));
    SetNamedBool(env, result, "negativePiMatch", gotNegPi == DOUBLE_NEGATIVE_PI);

    // Large value
    napi_value largeVal = nullptr;
    NAPI_CALL(env, napi_create_double(env, DOUBLE_LARGE, &largeVal));
    double gotLarge = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_value_double(env, largeVal, &gotLarge));
    SetNamedBool(env, result, "largeMatch", gotLarge == DOUBLE_LARGE);

    return result;
}

// ============================================================
// Test 30: Double — small value and zero
// ============================================================
static napi_value TestDoubleSmallAndZero(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    // Small value
    napi_value smallVal = nullptr;
    NAPI_CALL(env, napi_create_double(env, DOUBLE_SMALL, &smallVal));
    double gotSmall = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_value_double(env, smallVal, &gotSmall));
    SetNamedBool(env, result, "smallMatch", gotSmall == DOUBLE_SMALL);

    // Zero
    napi_value zeroVal = nullptr;
    NAPI_CALL(env, napi_create_double(env, DOUBLE_ZERO, &zeroVal));
    double gotZero = DOUBLE_ONE;
    NAPI_CALL(env, napi_get_value_double(env, zeroVal, &gotZero));
    SetNamedBool(env, result, "zeroMatch", gotZero == DOUBLE_ZERO);

    // Negative one
    napi_value negOneVal = nullptr;
    NAPI_CALL(env, napi_create_double(env, DOUBLE_NEGATIVE_ONE, &negOneVal));
    double gotNegOne = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_value_double(env, negOneVal, &gotNegOne));
    SetNamedBool(env, result, "negOneMatch", gotNegOne == DOUBLE_NEGATIVE_ONE);

    return result;
}

// ============================================================
// Test 31: napi_strict_equals — same value comparisons
// ============================================================
static napi_value TestStrictEqualsSameValue(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    // Same number
    napi_value num1 = nullptr;
    napi_value num2 = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_FORTY_TWO, &num1));
    NAPI_CALL(env, napi_create_int32(env, INT32_FORTY_TWO, &num2));
    bool eqNum = false;
    NAPI_CALL(env, napi_strict_equals(env, num1, num2, &eqNum));
    SetNamedBool(env, result, "sameNumberEqual", eqNum);

    // Same string
    napi_value str1 = nullptr;
    napi_value str2 = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "hello", NAPI_AUTO_LENGTH, &str1));
    NAPI_CALL(env, napi_create_string_utf8(env, "hello", NAPI_AUTO_LENGTH, &str2));
    bool eqStr = false;
    NAPI_CALL(env, napi_strict_equals(env, str1, str2, &eqStr));
    SetNamedBool(env, result, "sameStringEqual", eqStr);

    // Same boolean
    napi_value boolA = nullptr;
    napi_value boolB = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, true, &boolA));
    NAPI_CALL(env, napi_get_boolean(env, true, &boolB));
    bool eqBool = false;
    NAPI_CALL(env, napi_strict_equals(env, boolA, boolB, &eqBool));
    SetNamedBool(env, result, "sameBoolEqual", eqBool);

    return result;
}

// ============================================================
// Test 32: napi_strict_equals — different type comparisons
// ============================================================
static napi_value TestStrictEqualsDiffType(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    // Number vs String "42"
    napi_value numVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_FORTY_TWO, &numVal));
    napi_value strVal = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "42", NAPI_AUTO_LENGTH, &strVal));
    bool eqNumStr = true;
    NAPI_CALL(env, napi_strict_equals(env, numVal, strVal, &eqNumStr));
    SetNamedBool(env, result, "numberStringNotEqual", !eqNumStr);

    // null vs undefined
    napi_value nullVal = nullptr;
    NAPI_CALL(env, napi_get_null(env, &nullVal));
    napi_value undefVal = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undefVal));
    bool eqNullUndef = true;
    NAPI_CALL(env, napi_strict_equals(env, nullVal, undefVal, &eqNullUndef));
    SetNamedBool(env, result, "nullUndefinedNotEqual", !eqNullUndef);

    // true vs 1
    napi_value trueVal = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, true, &trueVal));
    napi_value oneVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_ONE, &oneVal));
    bool eqBoolNum = true;
    NAPI_CALL(env, napi_strict_equals(env, trueVal, oneVal, &eqBoolNum));
    SetNamedBool(env, result, "boolNumberNotEqual", !eqBoolNum);

    return result;
}

// ============================================================
// Test 33: napi_strict_equals — object identity
// ============================================================
static napi_value TestStrictEqualsObjectIdentity(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    // Same object reference
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    bool eqSame = false;
    NAPI_CALL(env, napi_strict_equals(env, obj, obj, &eqSame));
    SetNamedBool(env, result, "sameObjectEqual", eqSame);

    // Different object references (content-identical but not same)
    napi_value objA = nullptr;
    napi_value objB = nullptr;
    NAPI_CALL(env, napi_create_object(env, &objA));
    NAPI_CALL(env, napi_create_object(env, &objB));
    bool eqDiff = true;
    NAPI_CALL(env, napi_strict_equals(env, objA, objB, &eqDiff));
    SetNamedBool(env, result, "diffObjectsNotEqual", !eqDiff);

    return result;
}

// ============================================================
// Test 34: napi_coerce_to_bool — negative number and NaN
// ============================================================
static napi_value TestCoerceBoolFromNegativeAndNaN(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    // Negative number is truthy
    napi_value negVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_NEGATIVE_ONE, &negVal));
    SetNamedBool(env, result, "negativeIsTrue", CoerceToBoolValue(env, negVal));

    // NaN is falsy
    napi_value nanVal = nullptr;
    NAPI_CALL(env, napi_create_double(env, NAN, &nanVal));
    SetNamedBool(env, result, "nanIsFalse", !CoerceToBoolValue(env, nanVal));

    // Infinity is truthy
    napi_value infVal = nullptr;
    NAPI_CALL(env, napi_create_double(env, INFINITY, &infVal));
    SetNamedBool(env, result, "infinityIsTrue", CoerceToBoolValue(env, infVal));

    return result;
}

// ============================================================
// Module export descriptors table
// ============================================================
static constexpr size_t CONVERT_TEST_COUNT = 34;

struct ConvertTestEntry {
    const char* name;
    napi_callback callback;
};

static const ConvertTestEntry CONVERT_TESTS[] = {
    {"testCoerceBoolFromZero", TestCoerceBoolFromZero},
    {"testCoerceBoolFromString", TestCoerceBoolFromString},
    {"testCoerceBoolFromNullUndefined", TestCoerceBoolFromNullUndefined},
    {"testCoerceBoolFromObject", TestCoerceBoolFromObject},
    {"testCoerceNumberFromBool", TestCoerceNumberFromBool},
    {"testCoerceNumberFromNull", TestCoerceNumberFromNull},
    {"testCoerceStringFromNumber", TestCoerceStringFromNumber},
    {"testCoerceStringFromBool", TestCoerceStringFromBool},
    {"testCoerceObjectFromNumber", TestCoerceObjectFromNumber},
    {"testCoerceObjectFromString", TestCoerceObjectFromString},
    {"testBigintInt64RoundTrip", TestBigintInt64RoundTrip},
    {"testBigintInt64Boundary", TestBigintInt64Boundary},
    {"testBigintUint64RoundTrip", TestBigintUint64RoundTrip},
    {"testBigintWordsSingleWord", TestBigintWordsSingleWord},
    {"testBigintWordsTwoWords", TestBigintWordsTwoWords},
    {"testStringUtf16Basic", TestStringUtf16Basic},
    {"testStringUtf16Empty", TestStringUtf16Empty},
    {"testStringUtf16Special", TestStringUtf16Special},
    {"testStringUtf16Cjk", TestStringUtf16Cjk},
    {"testStringLatin1Basic", TestStringLatin1Basic},
    {"testStringLatin1Empty", TestStringLatin1Empty},
    {"testStringLatin1Special", TestStringLatin1Special},
    {"testDateEpoch", TestDateEpoch},
    {"testDateY2kAndNegative", TestDateY2kAndNegative},
    {"testIsDateNonDate", TestIsDateNonDate},
    {"testInt32RoundTrip", TestInt32RoundTrip},
    {"testUint32RoundTrip", TestUint32RoundTrip},
    {"testInt64RoundTrip", TestInt64RoundTrip},
    {"testDoubleRoundTrip", TestDoubleRoundTrip},
    {"testDoubleSmallAndZero", TestDoubleSmallAndZero},
    {"testStrictEqualsSameValue", TestStrictEqualsSameValue},
    {"testStrictEqualsDiffType", TestStrictEqualsDiffType},
    {"testStrictEqualsObjectIdentity", TestStrictEqualsObjectIdentity},
    {"testCoerceBoolFromNegativeAndNaN", TestCoerceBoolFromNegativeAndNaN},
};

}  // namespace

static napi_value InitConvertSuite(napi_env env, napi_value exports)
{
    std::vector<napi_property_descriptor> descriptors(CONVERT_TEST_COUNT);
    for (size_t i = 0; i < CONVERT_TEST_COUNT; i++) {
        descriptors[i] = napi_property_descriptor{
            CONVERT_TESTS[i].name,
            nullptr,
            CONVERT_TESTS[i].callback,
            nullptr,
            nullptr,
            nullptr,
            napi_default,
            nullptr,
        };
    }
    NAPI_CALL(env, napi_define_properties(env, exports, descriptors.size(), descriptors.data()));
    return exports;
}

static napi_module g_convertSuiteModule = {
    .nm_version = MODULE_VERSION,
    .nm_flags = MODULE_FLAGS_NONE,
    .nm_filename = nullptr,
    .nm_register_func = InitConvertSuite,
    .nm_modname = "convert_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterConvertSuiteModule(void)
{
    napi_module_register(&g_convertSuiteModule);
}
