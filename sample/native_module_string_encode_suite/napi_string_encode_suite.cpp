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
#include <cstring>
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

namespace {

static constexpr size_t UTF8_EXACT_BUF_EXPECTED = 4;
static constexpr size_t UTF16_EXACT_BUF_EXPECTED = 4;

// Buffer and length constants
static constexpr size_t BUFFER_SIZE_SMALL = 4;
static constexpr size_t BUFFER_SIZE_MEDIUM = 64;
static constexpr size_t BUFFER_SIZE_LARGE = 2048;
static constexpr size_t EMPTY_STRING_LENGTH = 0;
static constexpr size_t LONG_STRING_LENGTH = 1024;
static constexpr size_t LONG_STRING_CHAR = 65; // 'A'

// UTF-8 test data lengths
static constexpr size_t UTF8_HELLO_LENGTH = 5;
static constexpr size_t UTF8_ACCENTED_BYTE_LENGTH = 6; // "éñü" in UTF-8 is 2 bytes each
static constexpr size_t UTF8_CJK_BYTE_LENGTH = 12;     // "你好世界" in UTF-8 is 3 bytes each

// UTF-16 test data
static constexpr char16_t UTF16_HELLO[] = u"Hello";
static constexpr size_t UTF16_HELLO_LENGTH = 5;
static constexpr char16_t UTF16_EMPTY[] = u"";
static constexpr size_t UTF16_EMPTY_LENGTH = 0;
static constexpr char16_t UTF16_ACCENTED[] = u"\u00E9\u00F1\u00FC"; // éñü
static constexpr size_t UTF16_ACCENTED_LENGTH = 3;
static constexpr char16_t UTF16_CJK[] = u"\u4F60\u597D\u4E16\u754C"; // 你好世界
static constexpr size_t UTF16_CJK_LENGTH = 4;
static constexpr char16_t UTF16_SNOWMAN[] = u"\u2603"; // ☃ BMP emoji-like
static constexpr size_t UTF16_SNOWMAN_LENGTH = 1;
// Surrogate pair for U+1F600 (😀): high=0xD83D, low=0xDE00
static constexpr char16_t UTF16_SURROGATE_PAIR[] = { 0xD83D, 0xDE00, 0x0000 };
static constexpr size_t UTF16_SURROGATE_PAIR_LENGTH = 2;

// Latin-1 test data
static constexpr const char* LATIN1_HELLO = "Hello";
static constexpr size_t LATIN1_HELLO_LENGTH = 5;
static constexpr const char* LATIN1_EMPTY = "";
static constexpr const char LATIN1_HIGH_BYTES[] = { static_cast<char>(0x80), static_cast<char>(0xA0),
                                                    static_cast<char>(0xBF), static_cast<char>(0xFF), '\0' };
static constexpr size_t LATIN1_HIGH_BYTES_LENGTH = 4;
static constexpr const char* LATIN1_SPECIAL = "\xE9\xF1\xFC"; // éñü in Latin-1
static constexpr size_t LATIN1_SPECIAL_LENGTH = 3;

// Coercion expected strings
static constexpr const char* COERCE_TRUE_STR = "true";
static constexpr const char* COERCE_FALSE_STR = "false";
static constexpr const char* COERCE_NULL_STR = "null";
static constexpr const char* COERCE_UNDEFINED_STR = "undefined";
static constexpr const char* COERCE_FORTY_TWO_STR = "42";
static constexpr int32_t COERCE_INT_VALUE = 42;

// String comparison constants
static constexpr int32_t STRCMP_EQUAL = 0;

// Truncation test buffer size
static constexpr size_t TRUNCATION_BUFFER_SIZE = 3;
static constexpr size_t TRUNCATION_EXPECTED_COPIED = 2;

// Concatenation test inputs
static constexpr const char* CONCAT_PART_A = "Hello";
static constexpr const char* CONCAT_PART_B = "World";
static constexpr const char* CONCAT_EXPECTED = "HelloWorld";
static constexpr size_t CONCAT_EXPECTED_LENGTH = 10;

// Property key test
static constexpr const char* PROPERTY_KEY = "testKey";
static constexpr const char* PROPERTY_VALUE = "testValue";

// Module registration
static constexpr uint32_t MODULE_VERSION = 1;
static constexpr uint32_t MODULE_FLAGS_NONE = 0;

// Test count
static constexpr size_t STRING_ENCODE_TEST_COUNT = 40;

// ============================================================
// Helper: set a boolean property on a result object
// ============================================================
static bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value)
{
    napi_value napiValue = nullptr;
    if (napi_get_boolean(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

// ============================================================
// Helper: set a string property on a result object
// ============================================================
static bool SetNamedString(napi_env env, napi_value object, const char* name, const char* value)
{
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

// ============================================================
// Helper: set an int32 property on a result object
// ============================================================
static bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int32(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

// ============================================================
// Helper: set a size_t property as uint32 on a result object
// ============================================================
static bool SetNamedSize(napi_env env, napi_value object, const char* name, size_t value)
{
    return SetNamedInt32(env, object, name, static_cast<int32_t>(value));
}

// ============================================================
// Helper: build a long string of repeated characters
// ============================================================
static std::string BuildLongString(size_t length)
{
    return std::string(length, static_cast<char>(LONG_STRING_CHAR));
}

// ============================================================
// Test 01: UTF-8 basic round-trip ("Hello")
// ============================================================
static napi_value TestUtf8BasicRoundTrip(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "Hello", UTF8_HELLO_LENGTH, &str));

    char buf[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, str, buf, sizeof(buf), &len));

    SetNamedBool(env, result, "lengthMatch", len == UTF8_HELLO_LENGTH);
    SetNamedBool(env, result, "contentMatch", strcmp(buf, "Hello") == STRCMP_EQUAL);
    return result;
}

// ============================================================
// Test 02: UTF-8 empty string round-trip
// ============================================================
static napi_value TestUtf8EmptyString(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "", EMPTY_STRING_LENGTH, &str));

    char buf[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len = BUFFER_SIZE_MEDIUM;
    NAPI_CALL(env, napi_get_value_string_utf8(env, str, buf, sizeof(buf), &len));

    SetNamedBool(env, result, "lengthIsZero", len == EMPTY_STRING_LENGTH);
    SetNamedBool(env, result, "contentEmpty", buf[0] == '\0');
    return result;
}

// ============================================================
// Test 03: UTF-8 accented characters (éñü)
// ============================================================
static napi_value TestUtf8AccentedChars(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    const char* input = "\xC3\xA9\xC3\xB1\xC3\xBC"; // éñü in UTF-8
    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, input, UTF8_ACCENTED_BYTE_LENGTH, &str));

    char buf[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, str, buf, sizeof(buf), &len));

    SetNamedBool(env, result, "lengthMatch", len == UTF8_ACCENTED_BYTE_LENGTH);
    SetNamedBool(env, result, "contentMatch", memcmp(buf, input, UTF8_ACCENTED_BYTE_LENGTH) == STRCMP_EQUAL);
    return result;
}

// ============================================================
// Test 04: UTF-8 CJK characters (你好世界)
// ============================================================
static napi_value TestUtf8CJKChars(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    const char* input = "\xE4\xBD\xA0\xE5\xA5\xBD\xE4\xB8\x96\xE7\x95\x8C"; // 你好世界
    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, input, UTF8_CJK_BYTE_LENGTH, &str));

    char buf[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, str, buf, sizeof(buf), &len));

    SetNamedBool(env, result, "lengthMatch", len == UTF8_CJK_BYTE_LENGTH);
    SetNamedBool(env, result, "contentMatch", memcmp(buf, input, UTF8_CJK_BYTE_LENGTH) == STRCMP_EQUAL);
    return result;
}

// ============================================================
// Test 05: UTF-8 long string (1000+ chars)
// ============================================================
static napi_value TestUtf8LongString(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    std::string longStr = BuildLongString(LONG_STRING_LENGTH);
    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, longStr.c_str(), longStr.size(), &str));

    size_t queryLen = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, str, nullptr, EMPTY_STRING_LENGTH, &queryLen));

    std::string readBuf(queryLen + 1, '\0');
    size_t copiedLen = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, str, &readBuf[0], readBuf.size(), &copiedLen));
    readBuf.resize(copiedLen);

    SetNamedBool(env, result, "lengthMatch", copiedLen == LONG_STRING_LENGTH);
    SetNamedBool(env, result, "contentMatch", readBuf == longStr);
    return result;
}

// ============================================================
// Test 06: UTF-8 length query (nullptr buffer)
// ============================================================
static napi_value TestUtf8LengthQuery(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "Hello", UTF8_HELLO_LENGTH, &str));

    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, str, nullptr, EMPTY_STRING_LENGTH, &len));

    SetNamedBool(env, result, "queryLengthMatch", len == UTF8_HELLO_LENGTH);
    return result;
}

// ============================================================
// Test 07: UTF-8 buffer too small (truncation)
// ============================================================
static napi_value TestUtf8BufferTooSmall(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "Hello", UTF8_HELLO_LENGTH, &str));

    // Buffer of size 3 can hold 2 chars + null terminator
    char buf[TRUNCATION_BUFFER_SIZE] = { 0 };
    size_t copied = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, str, buf, TRUNCATION_BUFFER_SIZE, &copied));

    SetNamedBool(env, result, "truncated", copied == TRUNCATION_EXPECTED_COPIED);
    SetNamedBool(env, result, "partialMatch", buf[0] == 'H' && buf[1] == 'e');
    return result;
}

// ============================================================
// Test 08: UTF-8 NAPI_AUTO_LENGTH
// ============================================================
static napi_value TestUtf8AutoLength(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "Hello", NAPI_AUTO_LENGTH, &str));

    char buf[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, str, buf, sizeof(buf), &len));

    SetNamedBool(env, result, "autoLengthOk", len == UTF8_HELLO_LENGTH);
    SetNamedBool(env, result, "contentMatch", strcmp(buf, "Hello") == STRCMP_EQUAL);
    return result;
}

// ============================================================
// Test 09: UTF-8 exact buffer size (no room for null terminator)
// ============================================================
static napi_value TestUtf8ExactBufferSize(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "Hello", UTF8_HELLO_LENGTH, &str));

    // Buffer size == string length (5), so only 4 chars + null can fit
    char buf[UTF8_HELLO_LENGTH] = { 0 };
    size_t copied = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, str, buf, UTF8_HELLO_LENGTH, &copied));

    SetNamedBool(env, result, "copiedLessThanFull", copied == UTF8_EXACT_BUF_EXPECTED);
    return result;
}

// ============================================================
// Test 10: UTF-16 basic round-trip ("Hello")
// ============================================================
static napi_value TestUtf16BasicRoundTrip(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf16(env, UTF16_HELLO, UTF16_HELLO_LENGTH, &str));

    char16_t buf[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf16(env, str, buf, BUFFER_SIZE_MEDIUM, &len));

    SetNamedBool(env, result, "lengthMatch", len == UTF16_HELLO_LENGTH);
    bool match = (memcmp(buf, UTF16_HELLO, UTF16_HELLO_LENGTH * sizeof(char16_t)) == STRCMP_EQUAL);
    SetNamedBool(env, result, "contentMatch", match);
    return result;
}

// ============================================================
// Test 11: UTF-16 empty string
// ============================================================
static napi_value TestUtf16EmptyString(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf16(env, UTF16_EMPTY, UTF16_EMPTY_LENGTH, &str));

    char16_t buf[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len = BUFFER_SIZE_MEDIUM;
    NAPI_CALL(env, napi_get_value_string_utf16(env, str, buf, BUFFER_SIZE_MEDIUM, &len));

    SetNamedBool(env, result, "lengthIsZero", len == UTF16_EMPTY_LENGTH);
    return result;
}

// ============================================================
// Test 12: UTF-16 accented characters (éñü)
// ============================================================
static napi_value TestUtf16AccentedChars(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf16(env, UTF16_ACCENTED, UTF16_ACCENTED_LENGTH, &str));

    char16_t buf[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf16(env, str, buf, BUFFER_SIZE_MEDIUM, &len));

    SetNamedBool(env, result, "lengthMatch", len == UTF16_ACCENTED_LENGTH);
    bool match = (memcmp(buf, UTF16_ACCENTED, UTF16_ACCENTED_LENGTH * sizeof(char16_t)) == STRCMP_EQUAL);
    SetNamedBool(env, result, "contentMatch", match);
    return result;
}

// ============================================================
// Test 13: UTF-16 CJK characters (你好世界)
// ============================================================
static napi_value TestUtf16CJKChars(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf16(env, UTF16_CJK, UTF16_CJK_LENGTH, &str));

    char16_t buf[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf16(env, str, buf, BUFFER_SIZE_MEDIUM, &len));

    SetNamedBool(env, result, "lengthMatch", len == UTF16_CJK_LENGTH);
    bool match = (memcmp(buf, UTF16_CJK, UTF16_CJK_LENGTH * sizeof(char16_t)) == STRCMP_EQUAL);
    SetNamedBool(env, result, "contentMatch", match);
    return result;
}

// ============================================================
// Test 14: UTF-16 BMP emoji-like char (snowman ☃)
// ============================================================
static napi_value TestUtf16BMPEmoji(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf16(env, UTF16_SNOWMAN, UTF16_SNOWMAN_LENGTH, &str));

    char16_t buf[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf16(env, str, buf, BUFFER_SIZE_MEDIUM, &len));

    SetNamedBool(env, result, "lengthMatch", len == UTF16_SNOWMAN_LENGTH);
    SetNamedBool(env, result, "contentMatch", buf[0] == UTF16_SNOWMAN[0]);
    return result;
}

// ============================================================
// Test 15: UTF-16 surrogate pair (U+1F600)
// ============================================================
static napi_value TestUtf16SurrogatePair(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf16(env, UTF16_SURROGATE_PAIR, UTF16_SURROGATE_PAIR_LENGTH, &str));

    char16_t buf[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf16(env, str, buf, BUFFER_SIZE_MEDIUM, &len));

    SetNamedBool(env, result, "lengthMatch", len == UTF16_SURROGATE_PAIR_LENGTH);
    SetNamedBool(env, result, "highSurrogate", buf[0] == UTF16_SURROGATE_PAIR[0]);
    SetNamedBool(env, result, "lowSurrogate", buf[1] == UTF16_SURROGATE_PAIR[1]);
    return result;
}

// ============================================================
// Test 16: UTF-16 long string (1000+ chars)
// ============================================================
static napi_value TestUtf16LongString(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    std::vector<char16_t> longData(LONG_STRING_LENGTH, u'B');
    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf16(env, longData.data(), longData.size(), &str));

    size_t queryLen = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf16(env, str, nullptr, EMPTY_STRING_LENGTH, &queryLen));

    SetNamedBool(env, result, "lengthMatch", queryLen == LONG_STRING_LENGTH);
    return result;
}

// ============================================================
// Test 17: UTF-16 length query (nullptr buffer)
// ============================================================
static napi_value TestUtf16LengthQuery(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf16(env, UTF16_HELLO, UTF16_HELLO_LENGTH, &str));

    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf16(env, str, nullptr, EMPTY_STRING_LENGTH, &len));

    SetNamedBool(env, result, "queryLengthMatch", len == UTF16_HELLO_LENGTH);
    return result;
}

// ============================================================
// Test 18: UTF-16 buffer too small (truncation)
// ============================================================
static napi_value TestUtf16BufferTooSmall(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf16(env, UTF16_HELLO, UTF16_HELLO_LENGTH, &str));

    char16_t buf[TRUNCATION_BUFFER_SIZE] = { 0 };
    size_t copied = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf16(env, str, buf, TRUNCATION_BUFFER_SIZE, &copied));

    SetNamedBool(env, result, "truncated", copied == TRUNCATION_EXPECTED_COPIED);
    SetNamedBool(env, result, "firstCharMatch", buf[0] == u'H');
    return result;
}

// ============================================================
// Test 19: Latin-1 basic round-trip ("Hello")
// ============================================================
static napi_value TestLatin1BasicRoundTrip(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, LATIN1_HELLO, NAPI_AUTO_LENGTH, &str));

    char buf[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_latin1(env, str, buf, sizeof(buf), &len));

    SetNamedBool(env, result, "lengthMatch", len == LATIN1_HELLO_LENGTH);
    SetNamedBool(env, result, "contentMatch", strcmp(buf, LATIN1_HELLO) == STRCMP_EQUAL);
    return result;
}

// ============================================================
// Test 20: Latin-1 empty string
// ============================================================
static napi_value TestLatin1EmptyString(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, LATIN1_EMPTY, EMPTY_STRING_LENGTH, &str));

    char buf[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len = BUFFER_SIZE_MEDIUM;
    NAPI_CALL(env, napi_get_value_string_latin1(env, str, buf, sizeof(buf), &len));

    SetNamedBool(env, result, "lengthIsZero", len == EMPTY_STRING_LENGTH);
    return result;
}

// ============================================================
// Test 21: Latin-1 high-byte characters (0x80-0xFF range)
// ============================================================
static napi_value TestLatin1HighBytes(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, LATIN1_HIGH_BYTES, LATIN1_HIGH_BYTES_LENGTH, &str));

    char buf[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_latin1(env, str, buf, sizeof(buf), &len));

    SetNamedBool(env, result, "lengthMatch", len == LATIN1_HIGH_BYTES_LENGTH);
    return result;
}

// ============================================================
// Test 22: Latin-1 special characters (éñü)
// ============================================================
static napi_value TestLatin1SpecialChars(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, LATIN1_SPECIAL, LATIN1_SPECIAL_LENGTH, &str));

    char buf[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_latin1(env, str, buf, sizeof(buf), &len));

    SetNamedBool(env, result, "lengthMatch", len == LATIN1_SPECIAL_LENGTH);
    return result;
}

// ============================================================
// Test 23: Latin-1 length query (nullptr buffer)
// ============================================================
static napi_value TestLatin1LengthQuery(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, LATIN1_HELLO, LATIN1_HELLO_LENGTH, &str));

    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_latin1(env, str, nullptr, EMPTY_STRING_LENGTH, &len));

    SetNamedBool(env, result, "queryLengthMatch", len == LATIN1_HELLO_LENGTH);
    return result;
}

// ============================================================
// Test 24: Latin-1 buffer too small (truncation)
// ============================================================
static napi_value TestLatin1BufferTooSmall(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, LATIN1_HELLO, NAPI_AUTO_LENGTH, &str));

    char buf[TRUNCATION_BUFFER_SIZE] = { 0 };
    size_t copied = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_latin1(env, str, buf, TRUNCATION_BUFFER_SIZE, &copied));

    SetNamedBool(env, result, "truncated", copied == TRUNCATION_EXPECTED_COPIED);
    SetNamedBool(env, result, "partialMatch", buf[0] == 'H' && buf[1] == 'e');
    return result;
}

// ============================================================
// Test 25: napi_coerce_to_string from number
// ============================================================
static napi_value TestCoerceStringFromNumber(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value numVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, COERCE_INT_VALUE, &numVal));

    napi_value coerced = nullptr;
    NAPI_CALL(env, napi_coerce_to_string(env, numVal, &coerced));

    char buf[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, coerced, buf, sizeof(buf), &len));

    SetNamedBool(env, result, "coerceOk", strcmp(buf, COERCE_FORTY_TWO_STR) == STRCMP_EQUAL);
    SetNamedString(env, result, "coercedValue", buf);
    return result;
}

// ============================================================
// Test 26: napi_coerce_to_string from booleans
// ============================================================
static napi_value TestCoerceStringFromBooleans(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value trueVal = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, true, &trueVal));
    napi_value coercedTrue = nullptr;
    NAPI_CALL(env, napi_coerce_to_string(env, trueVal, &coercedTrue));
    char bufTrue[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t lenTrue = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, coercedTrue, bufTrue, sizeof(bufTrue), &lenTrue));
    SetNamedBool(env, result, "trueOk", strcmp(bufTrue, COERCE_TRUE_STR) == STRCMP_EQUAL);

    napi_value falseVal = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, false, &falseVal));
    napi_value coercedFalse = nullptr;
    NAPI_CALL(env, napi_coerce_to_string(env, falseVal, &coercedFalse));
    char bufFalse[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t lenFalse = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, coercedFalse, bufFalse, sizeof(bufFalse), &lenFalse));
    SetNamedBool(env, result, "falseOk", strcmp(bufFalse, COERCE_FALSE_STR) == STRCMP_EQUAL);

    return result;
}

// ============================================================
// Test 27: napi_coerce_to_string from null and undefined
// ============================================================
static napi_value TestCoerceStringFromNullUndef(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value nullVal = nullptr;
    NAPI_CALL(env, napi_get_null(env, &nullVal));
    napi_value coercedNull = nullptr;
    NAPI_CALL(env, napi_coerce_to_string(env, nullVal, &coercedNull));
    char bufNull[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t lenNull = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, coercedNull, bufNull, sizeof(bufNull), &lenNull));
    SetNamedBool(env, result, "nullOk", strcmp(bufNull, COERCE_NULL_STR) == STRCMP_EQUAL);

    napi_value undefVal = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undefVal));
    napi_value coercedUndef = nullptr;
    NAPI_CALL(env, napi_coerce_to_string(env, undefVal, &coercedUndef));
    char bufUndef[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t lenUndef = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, coercedUndef, bufUndef, sizeof(bufUndef), &lenUndef));
    SetNamedBool(env, result, "undefinedOk", strcmp(bufUndef, COERCE_UNDEFINED_STR) == STRCMP_EQUAL);

    return result;
}

// ============================================================
// Test 28: napi_coerce_to_string from object
// ============================================================
static napi_value TestCoerceStringFromObject(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));

    napi_value coerced = nullptr;
    NAPI_CALL(env, napi_coerce_to_string(env, obj, &coerced));

    napi_valuetype coercedType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, coerced, &coercedType));
    SetNamedBool(env, result, "isString", coercedType == napi_string);

    char buf[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, coerced, buf, sizeof(buf), &len));
    SetNamedBool(env, result, "notEmpty", len > EMPTY_STRING_LENGTH);

    return result;
}

// ============================================================
// Test 29: napi_typeof on string values
// ============================================================
static napi_value TestTypeofString(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value strUtf8 = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "test", NAPI_AUTO_LENGTH, &strUtf8));
    napi_valuetype type1 = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, strUtf8, &type1));
    SetNamedBool(env, result, "utf8IsString", type1 == napi_string);

    napi_value strUtf16 = nullptr;
    NAPI_CALL(env, napi_create_string_utf16(env, UTF16_HELLO, UTF16_HELLO_LENGTH, &strUtf16));
    napi_valuetype type2 = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, strUtf16, &type2));
    SetNamedBool(env, result, "utf16IsString", type2 == napi_string);

    napi_value strLatin1 = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, LATIN1_HELLO, NAPI_AUTO_LENGTH, &strLatin1));
    napi_valuetype type3 = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, strLatin1, &type3));
    SetNamedBool(env, result, "latin1IsString", type3 == napi_string);

    return result;
}

// ============================================================
// Test 30: napi_strict_equals for identical and different strings
// ============================================================
static napi_value TestStrictEqualsStrings(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value strA = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "Hello", NAPI_AUTO_LENGTH, &strA));
    napi_value strB = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "Hello", NAPI_AUTO_LENGTH, &strB));
    napi_value strC = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "World", NAPI_AUTO_LENGTH, &strC));

    bool equalAB = false;
    NAPI_CALL(env, napi_strict_equals(env, strA, strB, &equalAB));
    SetNamedBool(env, result, "identicalEqual", equalAB);

    bool equalAC = true;
    NAPI_CALL(env, napi_strict_equals(env, strA, strC, &equalAC));
    SetNamedBool(env, result, "differentNotEqual", !equalAC);

    return result;
}

// ============================================================
// Test 31: String as property key via napi_set/get_property
// ============================================================
static napi_value TestStringAsPropertyKey(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));

    napi_value key = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, PROPERTY_KEY, NAPI_AUTO_LENGTH, &key));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, PROPERTY_VALUE, NAPI_AUTO_LENGTH, &val));

    NAPI_CALL(env, napi_set_property(env, obj, key, val));

    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_property(env, obj, key, &retrieved));

    char buf[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, retrieved, buf, sizeof(buf), &len));

    SetNamedBool(env, result, "valueMatch", strcmp(buf, PROPERTY_VALUE) == STRCMP_EQUAL);
    return result;
}

// ============================================================
// Test 32: Concatenation pattern — create, get, concat, create
// ============================================================
static napi_value TestStringConcatenation(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value strA = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, CONCAT_PART_A, NAPI_AUTO_LENGTH, &strA));
    napi_value strB = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, CONCAT_PART_B, NAPI_AUTO_LENGTH, &strB));

    char bufA[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t lenA = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, strA, bufA, sizeof(bufA), &lenA));

    char bufB[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t lenB = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, strB, bufB, sizeof(bufB), &lenB));

    std::string concatenated = std::string(bufA) + std::string(bufB);
    napi_value strConcat = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, concatenated.c_str(), concatenated.size(), &strConcat));

    char bufConcat[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t lenConcat = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, strConcat, bufConcat, sizeof(bufConcat), &lenConcat));

    SetNamedBool(env, result, "concatLengthOk", lenConcat == CONCAT_EXPECTED_LENGTH);
    SetNamedBool(env, result, "concatContentOk", strcmp(bufConcat, CONCAT_EXPECTED) == STRCMP_EQUAL);
    return result;
}

// ============================================================
// Test 33: Multiple sequential string operations
// ============================================================
static napi_value TestMultipleSequentialOps(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    // Step 1: Create UTF-8 string
    napi_value str1 = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "Step1", NAPI_AUTO_LENGTH, &str1));
    napi_valuetype type1 = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, str1, &type1));

    // Step 2: Create UTF-16 string
    constexpr char16_t step2Data[] = u"Step2";
    // length of u"Step2" (excluding null terminator)
    constexpr size_t step2Length = 5;
    napi_value str2 = nullptr;
    NAPI_CALL(env, napi_create_string_utf16(env, step2Data, step2Length, &str2));
    napi_valuetype type2 = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, str2, &type2));

    // Step 3: Create Latin-1 string
    napi_value str3 = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, "Step3", NAPI_AUTO_LENGTH, &str3));
    napi_valuetype type3 = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, str3, &type3));

    // Step 4: Read back UTF-8
    char buf1[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len1 = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, str1, buf1, sizeof(buf1), &len1));

    // Step 5: Verify all are strings
    bool allStrings = (type1 == napi_string) && (type2 == napi_string) && (type3 == napi_string);
    SetNamedBool(env, result, "allStrings", allStrings);

    // Step 6: Verify first readback
    SetNamedBool(env, result, "step1ContentOk", strcmp(buf1, "Step1") == STRCMP_EQUAL);

    // Step 7: Compare str1 with itself
    bool selfEqual = false;
    NAPI_CALL(env, napi_strict_equals(env, str1, str1, &selfEqual));
    SetNamedBool(env, result, "selfEqual", selfEqual);

    // Step 8: Verify str1 != str2
    bool crossNotEqual = true;
    NAPI_CALL(env, napi_strict_equals(env, str1, str2, &crossNotEqual));
    SetNamedBool(env, result, "crossNotEqual", !crossNotEqual);

    return result;
}

// ============================================================
// Test 34: Latin-1 with explicit length (not NAPI_AUTO_LENGTH)
// ============================================================
static napi_value TestLatin1ExplicitLength(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, LATIN1_HELLO, LATIN1_HELLO_LENGTH, &str));

    char buf[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_latin1(env, str, buf, sizeof(buf), &len));

    SetNamedBool(env, result, "lengthMatch", len == LATIN1_HELLO_LENGTH);
    SetNamedBool(env, result, "contentMatch", strcmp(buf, LATIN1_HELLO) == STRCMP_EQUAL);
    return result;
}

// ============================================================
// Test 35: UTF-8 explicit length vs NAPI_AUTO_LENGTH equivalence
// ============================================================
static napi_value TestUtf8ExplicitVsAutoLength(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value strExplicit = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "Hello", UTF8_HELLO_LENGTH, &strExplicit));

    napi_value strAuto = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "Hello", NAPI_AUTO_LENGTH, &strAuto));

    bool areEqual = false;
    NAPI_CALL(env, napi_strict_equals(env, strExplicit, strAuto, &areEqual));
    SetNamedBool(env, result, "explicitEqualsAuto", areEqual);

    size_t lenExplicit = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, strExplicit, nullptr, EMPTY_STRING_LENGTH, &lenExplicit));
    size_t lenAuto = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, strAuto, nullptr, EMPTY_STRING_LENGTH, &lenAuto));

    SetNamedBool(env, result, "lengthsEqual", lenExplicit == lenAuto);
    return result;
}

// ============================================================
// Test 36: UTF-16 exact buffer size edge case
// ============================================================
static napi_value TestUtf16ExactBufferSize(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf16(env, UTF16_HELLO, UTF16_HELLO_LENGTH, &str));

    // Buffer size == string length (5), so only 4 code units + null can fit
    char16_t buf[UTF16_HELLO_LENGTH] = { 0 };
    size_t copied = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf16(env, str, buf, UTF16_HELLO_LENGTH, &copied));

    SetNamedBool(env, result, "copiedLessThanFull", copied == UTF16_EXACT_BUF_EXPECTED);
    SetNamedBool(env, result, "firstCharOk", buf[0] == u'H');
    return result;
}

// ============================================================
// Test 37: Latin-1 long string (1000+ chars)
// ============================================================
static napi_value TestLatin1LongString(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    std::string longStr = BuildLongString(LONG_STRING_LENGTH);
    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, longStr.c_str(), longStr.size(), &str));

    size_t queryLen = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_latin1(env, str, nullptr, EMPTY_STRING_LENGTH, &queryLen));

    std::string readBuf(queryLen + 1, '\0');
    size_t copiedLen = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_latin1(env, str, &readBuf[0], readBuf.size(), &copiedLen));
    readBuf.resize(copiedLen);

    SetNamedBool(env, result, "lengthMatch", copiedLen == LONG_STRING_LENGTH);
    SetNamedBool(env, result, "contentMatch", readBuf == longStr);
    return result;
}

// ============================================================
// Test 38: UTF-8 single character round-trip
// ============================================================
static napi_value TestUtf8SingleChar(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    constexpr size_t singleCharLength = 1;
    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "X", singleCharLength, &str));

    char buf[BUFFER_SIZE_SMALL] = { 0 };
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, str, buf, sizeof(buf), &len));

    SetNamedBool(env, result, "lengthIsOne", len == singleCharLength);
    SetNamedBool(env, result, "charMatch", buf[0] == 'X');
    return result;
}

// ============================================================
// Test 39: UTF-16 string read back as UTF-8 (cross-encoding)
// ============================================================
static napi_value TestUtf16ReadAsUtf8(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    // Create string via UTF-16 "Hello"
    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf16(env, UTF16_HELLO, UTF16_HELLO_LENGTH, &str));

    // Read it back as UTF-8
    char buf[BUFFER_SIZE_MEDIUM] = { 0 };
    size_t len = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, str, buf, sizeof(buf), &len));

    SetNamedBool(env, result, "utf8LengthMatch", len == UTF8_HELLO_LENGTH);
    SetNamedBool(env, result, "utf8ContentMatch", strcmp(buf, "Hello") == STRCMP_EQUAL);

    // Verify it's still the same JS string type
    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, str, &type));
    SetNamedBool(env, result, "isString", type == napi_string);

    return result;
}

// ============================================================
// Test 40: UTF-8 NAPI_AUTO_LENGTH with empty string
// ============================================================
static napi_value TestUtf8AutoLengthEmpty(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &str));

    size_t queryLen = EMPTY_STRING_LENGTH;
    NAPI_CALL(env, napi_get_value_string_utf8(env, str, nullptr, EMPTY_STRING_LENGTH, &queryLen));
    SetNamedBool(env, result, "emptyAutoLenIsZero", queryLen == EMPTY_STRING_LENGTH);

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, str, &type));
    SetNamedBool(env, result, "isString", type == napi_string);

    // Strict equals with explicitly empty string
    napi_value strExplicit = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "", EMPTY_STRING_LENGTH, &strExplicit));
    bool areEqual = false;
    NAPI_CALL(env, napi_strict_equals(env, str, strExplicit, &areEqual));
    SetNamedBool(env, result, "equalsExplicitEmpty", areEqual);

    return result;
}

// ============================================================
// Test registration table
// ============================================================
static napi_property_descriptor STRING_ENCODE_TESTS[] = {
    DECLARE_NAPI_FUNCTION("testUtf8BasicRoundTrip", TestUtf8BasicRoundTrip),
    DECLARE_NAPI_FUNCTION("testUtf8EmptyString", TestUtf8EmptyString),
    DECLARE_NAPI_FUNCTION("testUtf8AccentedChars", TestUtf8AccentedChars),
    DECLARE_NAPI_FUNCTION("testUtf8CJKChars", TestUtf8CJKChars),
    DECLARE_NAPI_FUNCTION("testUtf8LongString", TestUtf8LongString),
    DECLARE_NAPI_FUNCTION("testUtf8LengthQuery", TestUtf8LengthQuery),
    DECLARE_NAPI_FUNCTION("testUtf8BufferTooSmall", TestUtf8BufferTooSmall),
    DECLARE_NAPI_FUNCTION("testUtf8AutoLength", TestUtf8AutoLength),
    DECLARE_NAPI_FUNCTION("testUtf8ExactBufferSize", TestUtf8ExactBufferSize),
    DECLARE_NAPI_FUNCTION("testUtf16BasicRoundTrip", TestUtf16BasicRoundTrip),
    DECLARE_NAPI_FUNCTION("testUtf16EmptyString", TestUtf16EmptyString),
    DECLARE_NAPI_FUNCTION("testUtf16AccentedChars", TestUtf16AccentedChars),
    DECLARE_NAPI_FUNCTION("testUtf16CJKChars", TestUtf16CJKChars),
    DECLARE_NAPI_FUNCTION("testUtf16BMPEmoji", TestUtf16BMPEmoji),
    DECLARE_NAPI_FUNCTION("testUtf16SurrogatePair", TestUtf16SurrogatePair),
    DECLARE_NAPI_FUNCTION("testUtf16LongString", TestUtf16LongString),
    DECLARE_NAPI_FUNCTION("testUtf16LengthQuery", TestUtf16LengthQuery),
    DECLARE_NAPI_FUNCTION("testUtf16BufferTooSmall", TestUtf16BufferTooSmall),
    DECLARE_NAPI_FUNCTION("testLatin1BasicRoundTrip", TestLatin1BasicRoundTrip),
    DECLARE_NAPI_FUNCTION("testLatin1EmptyString", TestLatin1EmptyString),
    DECLARE_NAPI_FUNCTION("testLatin1HighBytes", TestLatin1HighBytes),
    DECLARE_NAPI_FUNCTION("testLatin1SpecialChars", TestLatin1SpecialChars),
    DECLARE_NAPI_FUNCTION("testLatin1LengthQuery", TestLatin1LengthQuery),
    DECLARE_NAPI_FUNCTION("testLatin1BufferTooSmall", TestLatin1BufferTooSmall),
    DECLARE_NAPI_FUNCTION("testCoerceStringFromNumber", TestCoerceStringFromNumber),
    DECLARE_NAPI_FUNCTION("testCoerceStringFromBooleans", TestCoerceStringFromBooleans),
    DECLARE_NAPI_FUNCTION("testCoerceStringFromNullUndef", TestCoerceStringFromNullUndef),
    DECLARE_NAPI_FUNCTION("testCoerceStringFromObject", TestCoerceStringFromObject),
    DECLARE_NAPI_FUNCTION("testTypeofString", TestTypeofString),
    DECLARE_NAPI_FUNCTION("testStrictEqualsStrings", TestStrictEqualsStrings),
    DECLARE_NAPI_FUNCTION("testStringAsPropertyKey", TestStringAsPropertyKey),
    DECLARE_NAPI_FUNCTION("testStringConcatenation", TestStringConcatenation),
    DECLARE_NAPI_FUNCTION("testMultipleSequentialOps", TestMultipleSequentialOps),
    DECLARE_NAPI_FUNCTION("testLatin1ExplicitLength", TestLatin1ExplicitLength),
    DECLARE_NAPI_FUNCTION("testUtf8ExplicitVsAutoLength", TestUtf8ExplicitVsAutoLength),
    DECLARE_NAPI_FUNCTION("testUtf16ExactBufferSize", TestUtf16ExactBufferSize),
    DECLARE_NAPI_FUNCTION("testLatin1LongString", TestLatin1LongString),
    DECLARE_NAPI_FUNCTION("testUtf8SingleChar", TestUtf8SingleChar),
    DECLARE_NAPI_FUNCTION("testUtf16ReadAsUtf8", TestUtf16ReadAsUtf8),
    DECLARE_NAPI_FUNCTION("testUtf8AutoLengthEmpty", TestUtf8AutoLengthEmpty),
};
} // namespace

static napi_value InitStringEncodeSuite(napi_env env, napi_value exports)
{
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(STRING_ENCODE_TESTS) / sizeof(STRING_ENCODE_TESTS[0]),
                                          STRING_ENCODE_TESTS));
    return exports;
}

static napi_module g_stringEncodeSuiteModule = {
    .nm_version = MODULE_VERSION,
    .nm_flags = MODULE_FLAGS_NONE,
    .nm_filename = nullptr,
    .nm_register_func = InitStringEncodeSuite,
    .nm_modname = "string_encode_suite",
    .nm_priv = nullptr,
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterStringEncodeSuiteModule(void)
{
    napi_module_register(&g_stringEncodeSuiteModule);
}
