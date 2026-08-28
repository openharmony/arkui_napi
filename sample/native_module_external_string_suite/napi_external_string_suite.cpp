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

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"
#include <vector>
#include <string>
#include <cstring>

namespace {
// 在匿名空间里定义全部的常量，以彻底规避魔鬼数字
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;
constexpr size_t INDEX_THREE = 3;
constexpr size_t INDEX_FOUR = 4;
constexpr size_t INDEX_FIVE = 5;

constexpr size_t STR_LEN_5 = 5;
constexpr size_t STR_LEN_10 = 10;

constexpr size_t BUF_SIZE_8 = 8;
constexpr size_t BUF_SIZE_16 = 16;
constexpr size_t BUF_SIZE_32 = 32;
constexpr size_t BUF_SIZE_64 = 64;
constexpr size_t BUF_SIZE_128 = 128;
constexpr size_t BUF_SIZE_256 = 256;
constexpr size_t BUF_SIZE_512 = 512;

constexpr int32_t FINALIZE_CALLED_FLAG = 1;
constexpr int32_t FINALIZE_INIT_FLAG = 0;
constexpr size_t MAX_CALLBACK_TRACKERS = 100;
constexpr int64_t ONE_MB = 1024 * 1024;

// 跟踪析构状态的全局数组
static int32_t g_finalizeTrackers[MAX_CALLBACK_TRACKERS] = {0};
}

static void FinalizeAsciiCallback(void* data, void* hint)
{
    if (data != nullptr) {
        char* str = static_cast<char*>(data);
        delete[] str;
    }
    if (hint != nullptr) {
        size_t index = *static_cast<size_t*>(hint);
        if (index < MAX_CALLBACK_TRACKERS) {
            g_finalizeTrackers[index] = FINALIZE_CALLED_FLAG;
        }
        delete static_cast<size_t*>(hint);
    }
}

static void FinalizeUtf16Callback(void* data, void* hint)
{
    if (data != nullptr) {
        char16_t* str = static_cast<char16_t*>(data);
        delete[] str;
    }
    if (hint != nullptr) {
        size_t index = *static_cast<size_t*>(hint);
        if (index < MAX_CALLBACK_TRACKERS) {
            g_finalizeTrackers[index] = FINALIZE_CALLED_FLAG;
        }
        delete static_cast<size_t*>(hint);
    }
}

static void FinalizeOffsetCallback(void* /* data */, void* hint)
{
    if (hint != nullptr) {
        char* originalPtr = static_cast<char*>(hint);
        delete[] originalPtr;
    }
}

static void FinalizeUtf16OffsetCallback(void* /* data */, void* hint)
{
    if (hint != nullptr) {
        char16_t* originalPtr = static_cast<char16_t*>(hint);
        delete[] originalPtr;
    }
}

static void SetNamedBool(napi_env env, napi_value obj, const char* name, bool value)
{
    napi_value napiValue = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, value, &napiValue));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, obj, name, napiValue));
}

static bool ValidateAsciiContent(napi_env env, napi_value jsStr, const char* expectedStr)
{
    size_t copiedLength = 0;
    napi_status status = napi_get_value_string_utf8(env, jsStr, nullptr, 0, &copiedLength);
    if (status != napi_ok) {
        return false;
    }
    std::vector<char> buffer(copiedLength + 1, '\0');
    status = napi_get_value_string_utf8(env, jsStr, buffer.data(), buffer.size(), &copiedLength);
    if (status != napi_ok) {
        return false;
    }
    return strcmp(buffer.data(), expectedStr) == 0;
}

static bool ValidateUtf16Content(napi_env env, napi_value jsStr, const char16_t* expectedStr)
{
    size_t copiedLength = 0;
    napi_status status = napi_get_value_string_utf16(env, jsStr, nullptr, 0, &copiedLength);
    if (status != napi_ok) {
        return false;
    }
    std::vector<char16_t> buffer(copiedLength + 1, u'\0');
    status = napi_get_value_string_utf16(env, jsStr, buffer.data(), buffer.size(), &copiedLength);
    if (status != napi_ok) {
        return false;
    }
    return std::char_traits<char16_t>::compare(buffer.data(), expectedStr, copiedLength) == 0;
}

static napi_value TestCreateExternalStringAsciiAutoLength(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char* rawStr = "Hello NAPI!";
    size_t strLen = strlen(rawStr);
    char* extStr = new char[strLen + INDEX_ONE];
    if (strcpy_s(extStr, strLen + INDEX_ONE, rawStr) != EOK) {
        SetNamedBool(env, result, "success", false);
        delete[] extStr;
        return result;
    }
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_ascii(
        env, extStr, NAPI_AUTO_LENGTH, FinalizeAsciiCallback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateAsciiContent(env, jsStr, rawStr);
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringAsciiExplicitLength(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char* rawStr = "Hello Explicit!";
    size_t strLen = strlen(rawStr);
    char* extStr = new char[strLen + INDEX_ONE];
    if (strcpy_s(extStr, strLen + INDEX_ONE, rawStr) != EOK) {
        SetNamedBool(env, result, "success", false);
        delete[] extStr;
        return result;
    }
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_ascii(
        env, extStr, strLen, FinalizeAsciiCallback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateAsciiContent(env, jsStr, rawStr);
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringUtf16AutoLength(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char16_t* rawStr = u"Hello UTF16 Auto!";
    size_t strLen = std::char_traits<char16_t>::length(rawStr);
    char16_t* extStr = new char16_t[strLen + INDEX_ONE];
    for (size_t index = INDEX_ZERO; index <= strLen; ++index) {
        extStr[index] = rawStr[index];
    }
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_utf16(
        env, extStr, NAPI_AUTO_LENGTH, FinalizeUtf16Callback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateUtf16Content(env, jsStr, rawStr);
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringUtf16ExplicitLength(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char16_t* rawStr = u"Hello UTF16 Explicit!";
    size_t strLen = std::char_traits<char16_t>::length(rawStr);
    char16_t* extStr = new char16_t[strLen + INDEX_ONE];
    for (size_t index = INDEX_ZERO; index <= strLen; ++index) {
        extStr[index] = rawStr[index];
    }
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_utf16(
        env, extStr, strLen, FinalizeUtf16Callback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateUtf16Content(env, jsStr, rawStr);
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringAsciiEmpty(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    char* extStr = new char[INDEX_ONE];
    extStr[INDEX_ZERO] = '\0';
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_ascii(
        env, extStr, INDEX_ZERO, FinalizeAsciiCallback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateAsciiContent(env, jsStr, "");
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringUtf16Empty(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    char16_t* extStr = new char16_t[INDEX_ONE];
    extStr[INDEX_ZERO] = u'\0';
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_utf16(
        env, extStr, INDEX_ZERO, FinalizeUtf16Callback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateUtf16Content(env, jsStr, u"");
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringAsciiOffset(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char* rawStr = "abcdefghijklmnopqrstuvwxyz";
    size_t strLen = strlen(rawStr);
    char* extStr = new char[strLen + INDEX_ONE];
    if (strcpy_s(extStr, strLen + INDEX_ONE, rawStr) != EOK) {
        SetNamedBool(env, result, "success", false);
        delete[] extStr;
        return result;
    }
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_ascii(
        env, extStr + INDEX_FIVE, STR_LEN_10, FinalizeOffsetCallback, extStr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateAsciiContent(env, jsStr, "fghijklmno");
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringUtf16Offset(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char16_t* rawStr = u"abcdefghijklmnopqrstuvwxyz";
    size_t strLen = std::char_traits<char16_t>::length(rawStr);
    char16_t* extStr = new char16_t[strLen + INDEX_ONE];
    for (size_t index = INDEX_ZERO; index <= strLen; ++index) {
        extStr[index] = rawStr[index];
    }
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_utf16(
        env, extStr + INDEX_FIVE, STR_LEN_10, FinalizeUtf16OffsetCallback, extStr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateUtf16Content(env, jsStr, u"fghijklmno");
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringAsciiNullEnv(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char* rawStr = "Null Env Test";
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_ascii(
        nullptr, rawStr, NAPI_AUTO_LENGTH, nullptr, nullptr, &jsStr);
    SetNamedBool(env, result, "success", status == napi_invalid_arg);
    return result;
}

static napi_value TestCreateExternalStringUtf16NullEnv(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char16_t* rawStr = u"Null Env Test UTF16";
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_utf16(
        nullptr, rawStr, NAPI_AUTO_LENGTH, nullptr, nullptr, &jsStr);
    SetNamedBool(env, result, "success", status == napi_invalid_arg);
    return result;
}

static napi_value TestCreateExternalStringAsciiNullStr(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_ascii(
        env, nullptr, STR_LEN_10, nullptr, nullptr, &jsStr);
    SetNamedBool(env, result, "success", status == napi_invalid_arg);
    return result;
}

static napi_value TestCreateExternalStringUtf16NullStr(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_utf16(
        env, nullptr, STR_LEN_10, nullptr, nullptr, &jsStr);
    SetNamedBool(env, result, "success", status == napi_invalid_arg);
    return result;
}

static napi_value TestCreateExternalStringAsciiNullResult(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char* rawStr = "Null Result Test";
    napi_status status = napi_create_external_string_ascii(
        env, rawStr, NAPI_AUTO_LENGTH, nullptr, nullptr, nullptr);
    SetNamedBool(env, result, "success", status == napi_invalid_arg);
    return result;
}

static napi_value TestCreateExternalStringUtf16NullResult(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char16_t* rawStr = u"Null Result Test UTF16";
    napi_status status = napi_create_external_string_utf16(
        env, rawStr, NAPI_AUTO_LENGTH, nullptr, nullptr, nullptr);
    SetNamedBool(env, result, "success", status == napi_invalid_arg);
    return result;
}

static napi_value TestCreateExternalStringAsciiNullCallback(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char* rawStr = "Static Constant ASCII String";
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_ascii(
        env, rawStr, NAPI_AUTO_LENGTH, nullptr, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateAsciiContent(env, jsStr, rawStr);
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringUtf16NullCallback(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char16_t* rawStr = u"Static Constant UTF16 String";
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_utf16(
        env, rawStr, NAPI_AUTO_LENGTH, nullptr, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateUtf16Content(env, jsStr, rawStr);
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringAsciiNullHint(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char* rawStr = "Null Hint Test String";
    size_t strLen = strlen(rawStr);
    char* extStr = new char[strLen + INDEX_ONE];
    if (strcpy_s(extStr, strLen + INDEX_ONE, rawStr) != EOK) {
        SetNamedBool(env, result, "success", false);
        delete[] extStr;
        return result;
    }
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_ascii(
        env, extStr, NAPI_AUTO_LENGTH, FinalizeAsciiCallback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateAsciiContent(env, jsStr, rawStr);
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringUtf16NullHint(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char16_t* rawStr = u"Null Hint UTF16 Test String";
    size_t strLen = std::char_traits<char16_t>::length(rawStr);
    char16_t* extStr = new char16_t[strLen + INDEX_ONE];
    for (size_t index = INDEX_ZERO; index <= strLen; ++index) {
        extStr[index] = rawStr[index];
    }
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_utf16(
        env, extStr, NAPI_AUTO_LENGTH, FinalizeUtf16Callback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateUtf16Content(env, jsStr, rawStr);
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringUtf16InvalidSurrogate(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    char16_t* extStr = new char16_t[INDEX_THREE];
    extStr[INDEX_ZERO] = static_cast<char16_t>(0xD800);
    extStr[INDEX_ONE] = static_cast<char16_t>(0x0020);
    extStr[INDEX_TWO] = u'\0';
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_utf16(
        env, extStr, NAPI_AUTO_LENGTH, FinalizeUtf16Callback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        size_t copiedLength = 0;
        napi_get_value_string_utf16(env, jsStr, nullptr, 0, &copiedLength);
        isSuccess = (copiedLength > 0);
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringAsciiSpecialChars(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char* rawStr = "Line1\nLine2\tTabbed\rCarriage!";
    size_t strLen = strlen(rawStr);
    char* extStr = new char[strLen + INDEX_ONE];
    if (strcpy_s(extStr, strLen + INDEX_ONE, rawStr) != EOK) {
        SetNamedBool(env, result, "success", false);
        delete[] extStr;
        return result;
    }
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_ascii(
        env, extStr, NAPI_AUTO_LENGTH, FinalizeAsciiCallback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateAsciiContent(env, jsStr, rawStr);
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringUtf16SpecialChars(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char16_t* rawStr = u"测试中文字符串\uD83D\uDE00笑脸";
    size_t strLen = std::char_traits<char16_t>::length(rawStr);
    char16_t* extStr = new char16_t[strLen + INDEX_ONE];
    for (size_t index = INDEX_ZERO; index <= strLen; ++index) {
        extStr[index] = rawStr[index];
    }
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_utf16(
        env, extStr, NAPI_AUTO_LENGTH, FinalizeUtf16Callback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateUtf16Content(env, jsStr, rawStr);
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value CreateExternalStringAsciiWithTracker(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    size_t argc = INDEX_ONE;
    napi_value argv[INDEX_ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    int32_t trackerIndex = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[INDEX_ZERO], &trackerIndex));
    if (trackerIndex < 0 || trackerIndex >= static_cast<int32_t>(MAX_CALLBACK_TRACKERS)) {
        SetNamedBool(env, result, "success", false);
        return result;
    }
    const char* rawStr = "Tracked ASCII External String";
    size_t strLen = strlen(rawStr);
    char* extStr = new char[strLen + INDEX_ONE];
    if (strcpy_s(extStr, strLen + INDEX_ONE, rawStr) != EOK) {
        delete[] extStr;
        SetNamedBool(env, result, "success", false);
        return result;
    }
    size_t* hint = new size_t(static_cast<size_t>(trackerIndex));
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_ascii(
        env, extStr, NAPI_AUTO_LENGTH, FinalizeAsciiCallback, hint, &jsStr);
    if (status == napi_ok) {
        g_finalizeTrackers[trackerIndex] = FINALIZE_INIT_FLAG;
        NAPI_CALL(env, napi_set_named_property(env, result, "string", jsStr));
        SetNamedBool(env, result, "success", true);
    } else {
        delete[] extStr;
        delete hint;
        SetNamedBool(env, result, "success", false);
    }
    return result;
}

static napi_value CreateExternalStringUtf16WithTracker(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    size_t argc = INDEX_ONE;
    napi_value argv[INDEX_ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    int32_t trackerIndex = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[INDEX_ZERO], &trackerIndex));
    if (trackerIndex < 0 || trackerIndex >= static_cast<int32_t>(MAX_CALLBACK_TRACKERS)) {
        SetNamedBool(env, result, "success", false);
        return result;
    }
    const char16_t* rawStr = u"Tracked UTF16 External String";
    size_t strLen = std::char_traits<char16_t>::length(rawStr);
    char16_t* extStr = new char16_t[strLen + INDEX_ONE];
    for (size_t index = INDEX_ZERO; index <= strLen; ++index) {
        extStr[index] = rawStr[index];
    }
    size_t* hint = new size_t(static_cast<size_t>(trackerIndex));
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_utf16(
        env, extStr, NAPI_AUTO_LENGTH, FinalizeUtf16Callback, hint, &jsStr);
    if (status == napi_ok) {
        g_finalizeTrackers[trackerIndex] = FINALIZE_INIT_FLAG;
        NAPI_CALL(env, napi_set_named_property(env, result, "string", jsStr));
        SetNamedBool(env, result, "success", true);
    } else {
        delete[] extStr;
        delete hint;
        SetNamedBool(env, result, "success", false);
    }
    return result;
}

static napi_value CheckTracker(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    size_t argc = INDEX_ONE;
    napi_value argv[INDEX_ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    int32_t trackerIndex = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[INDEX_ZERO], &trackerIndex));
    bool called = false;
    if (trackerIndex >= 0 && trackerIndex < static_cast<int32_t>(MAX_CALLBACK_TRACKERS)) {
        called = (g_finalizeTrackers[trackerIndex] == FINALIZE_CALLED_FLAG);
    }
    SetNamedBool(env, result, "called", called);
    return result;
}

static napi_value ResetTrackers(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    for (size_t index = INDEX_ZERO; index < MAX_CALLBACK_TRACKERS; ++index) {
        g_finalizeTrackers[index] = FINALIZE_INIT_FLAG;
    }
    SetNamedBool(env, result, "success", true);
    return result;
}

static napi_value TestAdjustExternalMemory(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    int64_t change = ONE_MB;
    int64_t adjustedValue = 0;
    napi_status status = napi_adjust_external_memory(env, change, &adjustedValue);
    SetNamedBool(env, result, "success", status == napi_ok);
    return result;
}

static napi_value TestAdjustExternalMemoryNegative(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    int64_t change = -ONE_MB;
    int64_t adjustedValue = 0;
    napi_status status = napi_adjust_external_memory(env, change, &adjustedValue);
    SetNamedBool(env, result, "success", status == napi_ok);
    return result;
}

static napi_value TestCreateExternalStringAsciiLen1(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    char* extStr = new char[INDEX_TWO];
    extStr[INDEX_ZERO] = 'A';
    extStr[INDEX_ONE] = '\0';
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_ascii(
        env, extStr, INDEX_ONE, FinalizeAsciiCallback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateAsciiContent(env, jsStr, "A");
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringAsciiLen2(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    char* extStr = new char[INDEX_THREE];
    extStr[INDEX_ZERO] = 'H';
    extStr[INDEX_ONE] = 'i';
    extStr[INDEX_TWO] = '\0';
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_ascii(
        env, extStr, INDEX_TWO, FinalizeAsciiCallback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateAsciiContent(env, jsStr, "Hi");
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringAsciiLen8(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char* rawStr = "12345678";
    char* extStr = new char[BUF_SIZE_16];
    if (strcpy_s(extStr, BUF_SIZE_16, rawStr) != EOK) {
        delete[] extStr;
        SetNamedBool(env, result, "success", false);
        return result;
    }
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_ascii(
        env, extStr, BUF_SIZE_8, FinalizeAsciiCallback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateAsciiContent(env, jsStr, rawStr);
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringAsciiLen32(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char* rawStr = "12345678901234567890123456789012";
    char* extStr = new char[BUF_SIZE_64];
    if (strcpy_s(extStr, BUF_SIZE_64, rawStr) != EOK) {
        delete[] extStr;
        SetNamedBool(env, result, "success", false);
        return result;
    }
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_ascii(
        env, extStr, BUF_SIZE_32, FinalizeAsciiCallback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateAsciiContent(env, jsStr, rawStr);
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringAsciiLen128(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    std::string rawStr(BUF_SIZE_128, 'a');
    char* extStr = new char[BUF_SIZE_256];
    if (strcpy_s(extStr, BUF_SIZE_256, rawStr.c_str()) != EOK) {
        delete[] extStr;
        SetNamedBool(env, result, "success", false);
        return result;
    }
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_ascii(
        env, extStr, BUF_SIZE_128, FinalizeAsciiCallback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateAsciiContent(env, jsStr, rawStr.c_str());
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringUtf16Len1(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    char16_t* extStr = new char16_t[INDEX_TWO];
    extStr[INDEX_ZERO] = u'A';
    extStr[INDEX_ONE] = u'\0';
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_utf16(
        env, extStr, INDEX_ONE, FinalizeUtf16Callback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateUtf16Content(env, jsStr, u"A");
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringUtf16Len2(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    char16_t* extStr = new char16_t[INDEX_THREE];
    extStr[INDEX_ZERO] = u'H';
    extStr[INDEX_ONE] = u'i';
    extStr[INDEX_TWO] = u'\0';
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_utf16(
        env, extStr, INDEX_TWO, FinalizeUtf16Callback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateUtf16Content(env, jsStr, u"Hi");
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringUtf16Len8(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char16_t* rawStr = u"12345678";
    char16_t* extStr = new char16_t[BUF_SIZE_16];
    for (size_t index = INDEX_ZERO; index < BUF_SIZE_8; ++index) {
        extStr[index] = rawStr[index];
    }
    extStr[BUF_SIZE_8] = u'\0';
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_utf16(
        env, extStr, BUF_SIZE_8, FinalizeUtf16Callback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateUtf16Content(env, jsStr, rawStr);
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringUtf16Len32(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char16_t* rawStr = u"12345678901234567890123456789012";
    char16_t* extStr = new char16_t[BUF_SIZE_64];
    for (size_t index = INDEX_ZERO; index < BUF_SIZE_32; ++index) {
        extStr[index] = rawStr[index];
    }
    extStr[BUF_SIZE_32] = u'\0';
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_utf16(
        env, extStr, BUF_SIZE_32, FinalizeUtf16Callback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateUtf16Content(env, jsStr, rawStr);
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringUtf16Len128(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    std::u16string rawStr(BUF_SIZE_128, u'a');
    char16_t* extStr = new char16_t[BUF_SIZE_256];
    for (size_t index = INDEX_ZERO; index < BUF_SIZE_128; ++index) {
        extStr[index] = rawStr[index];
    }
    extStr[BUF_SIZE_128] = u'\0';
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_utf16(
        env, extStr, BUF_SIZE_128, FinalizeUtf16Callback, nullptr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateUtf16Content(env, jsStr, rawStr.c_str());
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringAsciiOffset1(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char* rawStr = "abcdef";
    size_t strLen = strlen(rawStr);
    char* extStr = new char[strLen + INDEX_ONE];
    if (strcpy_s(extStr, strLen + INDEX_ONE, rawStr) != EOK) {
        delete[] extStr;
        SetNamedBool(env, result, "success", false);
        return result;
    }
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_ascii(
        env, extStr + INDEX_ONE, STR_LEN_5, FinalizeOffsetCallback, extStr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateAsciiContent(env, jsStr, "bcdef");
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringAsciiOffset2(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char* rawStr = "abcdef";
    size_t strLen = strlen(rawStr);
    char* extStr = new char[strLen + INDEX_ONE];
    if (strcpy_s(extStr, strLen + INDEX_ONE, rawStr) != EOK) {
        delete[] extStr;
        SetNamedBool(env, result, "success", false);
        return result;
    }
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_ascii(
        env, extStr + INDEX_TWO, INDEX_FOUR, FinalizeOffsetCallback, extStr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateAsciiContent(env, jsStr, "cdef");
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringUtf16Offset1(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char16_t* rawStr = u"abcdef";
    size_t strLen = std::char_traits<char16_t>::length(rawStr);
    char16_t* extStr = new char16_t[strLen + INDEX_ONE];
    for (size_t index = INDEX_ZERO; index <= strLen; ++index) {
        extStr[index] = rawStr[index];
    }
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_utf16(
        env, extStr + INDEX_ONE, STR_LEN_5, FinalizeUtf16OffsetCallback, extStr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateUtf16Content(env, jsStr, u"bcdef");
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static napi_value TestCreateExternalStringUtf16Offset2(napi_env env, napi_callback_info /* info */)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    const char16_t* rawStr = u"abcdef";
    size_t strLen = std::char_traits<char16_t>::length(rawStr);
    char16_t* extStr = new char16_t[strLen + INDEX_ONE];
    for (size_t index = INDEX_ZERO; index <= strLen; ++index) {
        extStr[index] = rawStr[index];
    }
    napi_value jsStr = nullptr;
    napi_status status = napi_create_external_string_utf16(
        env, extStr + INDEX_TWO, INDEX_FOUR, FinalizeUtf16OffsetCallback, extStr, &jsStr);
    bool isSuccess = (status == napi_ok);
    if (isSuccess) {
        isSuccess = ValidateUtf16Content(env, jsStr, u"cdef");
    } else {
        delete[] extStr;
    }
    SetNamedBool(env, result, "success", isSuccess);
    return result;
}

static const napi_property_descriptor EXTERNAL_STRING_TEST_DESCRIPTORS[] = {
    DECLARE_NAPI_FUNCTION("testCreateExternalStringAsciiAutoLength",
        TestCreateExternalStringAsciiAutoLength),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringAsciiExplicitLength",
        TestCreateExternalStringAsciiExplicitLength),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringUtf16AutoLength",
        TestCreateExternalStringUtf16AutoLength),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringUtf16ExplicitLength",
        TestCreateExternalStringUtf16ExplicitLength),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringAsciiEmpty",
        TestCreateExternalStringAsciiEmpty),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringUtf16Empty",
        TestCreateExternalStringUtf16Empty),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringAsciiOffset",
        TestCreateExternalStringAsciiOffset),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringUtf16Offset",
        TestCreateExternalStringUtf16Offset),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringAsciiNullEnv",
        TestCreateExternalStringAsciiNullEnv),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringUtf16NullEnv",
        TestCreateExternalStringUtf16NullEnv),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringAsciiNullStr",
        TestCreateExternalStringAsciiNullStr),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringUtf16NullStr",
        TestCreateExternalStringUtf16NullStr),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringAsciiNullResult",
        TestCreateExternalStringAsciiNullResult),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringUtf16NullResult",
        TestCreateExternalStringUtf16NullResult),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringAsciiNullCallback",
        TestCreateExternalStringAsciiNullCallback),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringUtf16NullCallback",
        TestCreateExternalStringUtf16NullCallback),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringAsciiNullHint",
        TestCreateExternalStringAsciiNullHint),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringUtf16NullHint",
        TestCreateExternalStringUtf16NullHint),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringUtf16InvalidSurrogate",
        TestCreateExternalStringUtf16InvalidSurrogate),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringAsciiSpecialChars",
        TestCreateExternalStringAsciiSpecialChars),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringUtf16SpecialChars",
        TestCreateExternalStringUtf16SpecialChars),
    DECLARE_NAPI_FUNCTION("createExternalStringAsciiWithTracker",
        CreateExternalStringAsciiWithTracker),
    DECLARE_NAPI_FUNCTION("createExternalStringUtf16WithTracker",
        CreateExternalStringUtf16WithTracker),
    DECLARE_NAPI_FUNCTION("checkTracker",
        CheckTracker),
    DECLARE_NAPI_FUNCTION("resetTrackers",
        ResetTrackers),
    DECLARE_NAPI_FUNCTION("testAdjustExternalMemory",
        TestAdjustExternalMemory),
    DECLARE_NAPI_FUNCTION("testAdjustExternalMemoryNegative",
        TestAdjustExternalMemoryNegative),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringAsciiLen1",
        TestCreateExternalStringAsciiLen1),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringAsciiLen2",
        TestCreateExternalStringAsciiLen2),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringAsciiLen8",
        TestCreateExternalStringAsciiLen8),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringAsciiLen32",
        TestCreateExternalStringAsciiLen32),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringAsciiLen128",
        TestCreateExternalStringAsciiLen128),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringUtf16Len1",
        TestCreateExternalStringUtf16Len1),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringUtf16Len2",
        TestCreateExternalStringUtf16Len2),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringUtf16Len8",
        TestCreateExternalStringUtf16Len8),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringUtf16Len32",
        TestCreateExternalStringUtf16Len32),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringUtf16Len128",
        TestCreateExternalStringUtf16Len128),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringAsciiOffset1",
        TestCreateExternalStringAsciiOffset1),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringAsciiOffset2",
        TestCreateExternalStringAsciiOffset2),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringUtf16Offset1",
        TestCreateExternalStringUtf16Offset1),
    DECLARE_NAPI_FUNCTION("testCreateExternalStringUtf16Offset2",
        TestCreateExternalStringUtf16Offset2)
};

static napi_value Init(napi_env env, napi_value exports)
{
    NAPI_CALL(env, napi_define_properties(env, exports,
        sizeof(EXTERNAL_STRING_TEST_DESCRIPTORS) / sizeof(EXTERNAL_STRING_TEST_DESCRIPTORS[0]),
        EXTERNAL_STRING_TEST_DESCRIPTORS));
    return exports;
}

static napi_module g_externalStringModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "external_string_suite",
    .nm_priv = nullptr,
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterModule()
{
    napi_module_register(&g_externalStringModule);
}
