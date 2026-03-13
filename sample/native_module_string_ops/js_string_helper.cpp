/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "js_string_helper.h"

using namespace StringBufferSize;

std::string StringHelper::ToUpper(const std::string& input)
{
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::toupper(c); });
    return result;
}

std::string StringHelper::ToLower(const std::string& input)
{
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string StringHelper::Trim(const std::string& input)
{
    size_t start = input.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = input.find_last_not_of(" \t\n\r\f\v");
    return input.substr(start, end - start + 1);
}

std::string StringHelper::TrimStart(const std::string& input)
{
    size_t start = input.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
        return "";
    }
    return input.substr(start);
}

std::string StringHelper::TrimEnd(const std::string& input)
{
    size_t end = input.find_last_not_of(" \t\n\r\f\v");
    if (end == std::string::npos) {
        return "";
    }
    return input.substr(0, end + 1);
}

std::string StringHelper::Reverse(const std::string& input)
{
    std::string result = input;
    std::reverse(result.begin(), result.end());
    return result;
}

std::string StringHelper::Repeat(const std::string& input, int count)
{
    if (count <= 0) {
        return "";
    }
    std::string result;
    result.reserve(input.size() * count);
    for (int i = 0; i < count; i++) {
        result += input;
    }
    return result;
}

std::string StringHelper::PadStart(const std::string& input, int targetLength, const std::string& padStr)
{
    if (static_cast<int>(input.size()) >= targetLength || padStr.empty()) {
        return input;
    }
    int padNeeded = targetLength - static_cast<int>(input.size());
    std::string padding;
    padding.reserve(padNeeded);
    while (static_cast<int>(padding.size()) < padNeeded) {
        padding += padStr;
    }
    padding = padding.substr(0, padNeeded);
    return padding + input;
}

std::string StringHelper::PadEnd(const std::string& input, int targetLength, const std::string& padStr)
{
    if (static_cast<int>(input.size()) >= targetLength || padStr.empty()) {
        return input;
    }
    int padNeeded = targetLength - static_cast<int>(input.size());
    std::string padding;
    padding.reserve(padNeeded);
    while (static_cast<int>(padding.size()) < padNeeded) {
        padding += padStr;
    }
    padding = padding.substr(0, padNeeded);
    return input + padding;
}

std::vector<std::string> StringHelper::Split(const std::string& input, const std::string& delimiter)
{
    std::vector<std::string> result;
    if (delimiter.empty()) {
        for (char c : input) {
            result.push_back(std::string(1, c));
        }
        return result;
    }
    size_t start = 0;
    size_t end = input.find(delimiter);
    while (end != std::string::npos) {
        result.push_back(input.substr(start, end - start));
        start = end + delimiter.size();
        end = input.find(delimiter, start);
    }
    result.push_back(input.substr(start));
    return result;
}

std::string StringHelper::Replace(const std::string& input, const std::string& search, const std::string& replace)
{
    if (search.empty()) {
        return input;
    }
    std::string result = input;
    size_t pos = result.find(search);
    if (pos != std::string::npos) {
        result.replace(pos, search.size(), replace);
    }
    return result;
}

std::string StringHelper::ReplaceAll(const std::string& input, const std::string& search, const std::string& replace)
{
    if (search.empty()) {
        return input;
    }
    std::string result = input;
    size_t pos = 0;
    while ((pos = result.find(search, pos)) != std::string::npos) {
        result.replace(pos, search.size(), replace);
        pos += replace.size();
    }
    return result;
}

bool StringHelper::Contains(const std::string& input, const std::string& search)
{
    return input.find(search) != std::string::npos;
}

bool StringHelper::StartsWith(const std::string& input, const std::string& prefix)
{
    if (prefix.size() > input.size()) {
        return false;
    }
    return input.compare(0, prefix.size(), prefix) == 0;
}

bool StringHelper::EndsWith(const std::string& input, const std::string& suffix)
{
    if (suffix.size() > input.size()) {
        return false;
    }
    return input.compare(input.size() - suffix.size(), suffix.size(), suffix) == 0;
}

int StringHelper::IndexOf(const std::string& input, const std::string& search)
{
    size_t pos = input.find(search);
    if (pos == std::string::npos) {
        return -1;
    }
    return static_cast<int>(pos);
}

int StringHelper::LastIndexOf(const std::string& input, const std::string& search)
{
    size_t pos = input.rfind(search);
    if (pos == std::string::npos) {
        return -1;
    }
    return static_cast<int>(pos);
}

std::string StringHelper::Substring(const std::string& input, int start, int length)
{
    if (start < 0) {
        start = 0;
    }
    if (start >= static_cast<int>(input.size())) {
        return "";
    }
    if (length < 0 || start + length > static_cast<int>(input.size())) {
        length = static_cast<int>(input.size()) - start;
    }
    return input.substr(start, length);
}

int StringHelper::CountOccurrences(const std::string& input, const std::string& search)
{
    if (search.empty()) {
        return 0;
    }
    int count = 0;
    size_t pos = 0;
    while ((pos = input.find(search, pos)) != std::string::npos) {
        count++;
        pos += search.size();
    }
    return count;
}

/***********************************************
 * NAPI Sync Functions
 ***********************************************/

static napi_value JSStringToUpper(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "argument must be string");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    std::string result = StringHelper::ToUpper(std::string(input, inputLen));
    napi_value output = nullptr;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

static napi_value JSStringToLower(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "argument must be string");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    std::string result = StringHelper::ToLower(std::string(input, inputLen));
    napi_value output = nullptr;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

static napi_value JSStringTrim(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "argument must be string");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    std::string result = StringHelper::Trim(std::string(input, inputLen));
    napi_value output = nullptr;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

static napi_value JSStringTrimStart(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "argument must be string");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    std::string result = StringHelper::TrimStart(std::string(input, inputLen));
    napi_value output = nullptr;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

static napi_value JSStringTrimEnd(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "argument must be string");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    std::string result = StringHelper::TrimEnd(std::string(input, inputLen));
    napi_value output = nullptr;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

static napi_value JSStringReverse(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "argument must be string");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    std::string result = StringHelper::Reverse(std::string(input, inputLen));
    napi_value output = nullptr;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

static napi_value JSStringRepeat(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[STRING_THIRD_ARG] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= StringArgCount::TWO, "requires 2 parameters");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_string, "first argument must be string");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_number, "second argument must be number");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    int32_t count = 0;
    napi_get_value_int32(env, argv[1], &count);

    std::string result = StringHelper::Repeat(std::string(input, inputLen), count);
    napi_value output = nullptr;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

static napi_value JSStringPadStart(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value argv[3] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= StringArgCount::TWO, "requires at least 2 parameters");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_string, "first argument must be string");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_number, "second argument must be number");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    int32_t targetLength = 0;
    napi_get_value_int32(env, argv[1], &targetLength);

    std::string padStr = " ";
    if (argc > StringArgCount::TWO) {
        napi_valuetype valuetype2;
        NAPI_CALL(env, napi_typeof(env, argv[STRING_THIRD_ARG], &valuetype2));
        if (valuetype2 == napi_string) {
            char padBuf[DELIMITER_BUFFER_SIZE] = { 0 };
            size_t padLen = 0;
            napi_get_value_string_utf8(env, argv[STRING_THIRD_ARG], padBuf, DELIMITER_BUFFER_SIZE, &padLen);
            padStr = std::string(padBuf, padLen);
        }
    }

    std::string result = StringHelper::PadStart(std::string(input, inputLen), targetLength, padStr);
    napi_value output = nullptr;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

static napi_value JSStringPadEnd(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value argv[3] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= StringArgCount::TWO, "requires at least 2 parameters");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_string, "first argument must be string");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_number, "second argument must be number");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    int32_t targetLength = 0;
    napi_get_value_int32(env, argv[1], &targetLength);

    std::string padStr = " ";
    if (argc > StringArgCount::TWO) {
        napi_valuetype valuetype2;
        NAPI_CALL(env, napi_typeof(env, argv[STRING_THIRD_ARG], &valuetype2));
        if (valuetype2 == napi_string) {
            char padBuf[DELIMITER_BUFFER_SIZE] = { 0 };
            size_t padLen = 0;
            napi_get_value_string_utf8(env, argv[STRING_THIRD_ARG], padBuf, DELIMITER_BUFFER_SIZE, &padLen);
            padStr = std::string(padBuf, padLen);
        }
    }

    std::string result = StringHelper::PadEnd(std::string(input, inputLen), targetLength, padStr);

    napi_value output = nullptr;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

static napi_value JSStringSplit(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[STRING_THIRD_ARG] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= StringArgCount::TWO, "requires 2 parameters");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_string, "first argument must be string");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_string, "second argument must be string");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    char delimiter[DELIMITER_BUFFER_SIZE] = { 0 };
    size_t delimiterLen = 0;
    napi_get_value_string_utf8(env, argv[1], delimiter, DELIMITER_BUFFER_SIZE, &delimiterLen);

    std::vector<std::string> parts = StringHelper::Split(
        std::string(input, inputLen), std::string(delimiter, delimiterLen));

    napi_value result = nullptr;
    napi_create_array_with_length(env, parts.size(), &result);
    for (size_t i = 0; i < parts.size(); i++) {
        napi_value element = nullptr;
        napi_create_string_utf8(env, parts[i].c_str(), parts[i].length(), &element);
        napi_set_element(env, result, i, element);
    }
    return result;
}

static napi_value JSStringReplace(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value argv[3] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= StringArgCount::THREE, "requires 3 parameters");

    for (int i = 0; i < StringArgCount::THREE; i++) {
        napi_valuetype valuetype;
        NAPI_CALL(env, napi_typeof(env, argv[i], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "all arguments must be string");
    }

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    char search[SEARCH_BUFFER_SIZE] = { 0 };
    size_t searchLen = 0;
    napi_get_value_string_utf8(env, argv[1], search, SEARCH_BUFFER_SIZE, &searchLen);

    char replace[REPLACE_BUFFER_SIZE] = { 0 };
    size_t replaceLen = 0;
    napi_get_value_string_utf8(env, argv[STRING_THIRD_ARG], replace, REPLACE_BUFFER_SIZE, &replaceLen);

    std::string result = StringHelper::Replace(
        std::string(input, inputLen), std::string(search, searchLen), std::string(replace, replaceLen));

    napi_value output = nullptr;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

static napi_value JSStringReplaceAll(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value argv[3] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= StringArgCount::THREE, "requires 3 parameters");

    for (int i = 0; i < StringArgCount::THREE; i++) {
        napi_valuetype valuetype;
        NAPI_CALL(env, napi_typeof(env, argv[i], &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "all arguments must be string");
    }

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    char search[SEARCH_BUFFER_SIZE] = { 0 };
    size_t searchLen = 0;
    napi_get_value_string_utf8(env, argv[1], search, SEARCH_BUFFER_SIZE, &searchLen);

    char replace[REPLACE_BUFFER_SIZE] = { 0 };
    size_t replaceLen = 0;
    napi_get_value_string_utf8(env, argv[STRING_THIRD_ARG], replace, REPLACE_BUFFER_SIZE, &replaceLen);

    std::string result = StringHelper::ReplaceAll(
        std::string(input, inputLen), std::string(search, searchLen), std::string(replace, replaceLen));

    napi_value output = nullptr;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

static napi_value JSStringContains(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[STRING_THIRD_ARG] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= StringArgCount::TWO, "requires 2 parameters");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_string, "first argument must be string");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_string, "second argument must be string");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    char search[SEARCH_BUFFER_SIZE] = { 0 };
    size_t searchLen = 0;
    napi_get_value_string_utf8(env, argv[1], search, SEARCH_BUFFER_SIZE, &searchLen);

    bool result = StringHelper::Contains(std::string(input, inputLen), std::string(search, searchLen));

    napi_value output = nullptr;
    napi_get_boolean(env, result, &output);
    return output;
}

static napi_value JSStringStartsWith(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[STRING_THIRD_ARG] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= StringArgCount::TWO, "requires 2 parameters");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_string, "first argument must be string");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_string, "second argument must be string");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    char prefix[SEARCH_BUFFER_SIZE] = { 0 };
    size_t prefixLen = 0;
    napi_get_value_string_utf8(env, argv[1], prefix, SEARCH_BUFFER_SIZE, &prefixLen);

    bool result = StringHelper::StartsWith(std::string(input, inputLen), std::string(prefix, prefixLen));

    napi_value output = nullptr;
    napi_get_boolean(env, result, &output);
    return output;
}

static napi_value JSStringEndsWith(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[STRING_THIRD_ARG] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= StringArgCount::TWO, "requires 2 parameters");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_string, "first argument must be string");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_string, "second argument must be string");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    char suffix[SEARCH_BUFFER_SIZE] = { 0 };
    size_t suffixLen = 0;
    napi_get_value_string_utf8(env, argv[1], suffix, SEARCH_BUFFER_SIZE, &suffixLen);

    bool result = StringHelper::EndsWith(std::string(input, inputLen), std::string(suffix, suffixLen));

    napi_value output = nullptr;
    napi_get_boolean(env, result, &output);
    return output;
}

static napi_value JSStringIndexOf(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[STRING_THIRD_ARG] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= StringArgCount::TWO, "requires 2 parameters");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_string, "first argument must be string");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_string, "second argument must be string");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    char search[SEARCH_BUFFER_SIZE] = { 0 };
    size_t searchLen = 0;
    napi_get_value_string_utf8(env, argv[1], search, SEARCH_BUFFER_SIZE, &searchLen);

    int result = StringHelper::IndexOf(std::string(input, inputLen), std::string(search, searchLen));

    napi_value output = nullptr;
    napi_create_int32(env, result, &output);
    return output;
}

static napi_value JSStringLastIndexOf(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[STRING_THIRD_ARG] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= StringArgCount::TWO, "requires 2 parameters");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_string, "first argument must be string");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_string, "second argument must be string");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    char search[SEARCH_BUFFER_SIZE] = { 0 };
    size_t searchLen = 0;
    napi_get_value_string_utf8(env, argv[1], search, SEARCH_BUFFER_SIZE, &searchLen);

    int result = StringHelper::LastIndexOf(std::string(input, inputLen), std::string(search, searchLen));

    napi_value output = nullptr;
    napi_create_int32(env, result, &output);
    return output;
}

static napi_value JSStringSubstring(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value argv[3] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= StringArgCount::TWO, "requires at least 2 parameters");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_string, "first argument must be string");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_number, "second argument must be number");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    int32_t start = 0;
    napi_get_value_int32(env, argv[1], &start);

    int32_t length = -1;
    if (argc > StringArgCount::TWO) {
        napi_valuetype valuetype2;
        NAPI_CALL(env, napi_typeof(env, argv[STRING_THIRD_ARG], &valuetype2));
        if (valuetype2 == napi_number) {
            napi_get_value_int32(env, argv[STRING_THIRD_ARG], &length);
        }
    }

    std::string result = StringHelper::Substring(std::string(input, inputLen), start, length);

    napi_value output = nullptr;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

static napi_value JSStringCountOccurrences(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[STRING_THIRD_ARG] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= StringArgCount::TWO, "requires 2 parameters");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_string, "first argument must be string");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_string, "second argument must be string");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    char search[SEARCH_BUFFER_SIZE] = { 0 };
    size_t searchLen = 0;
    napi_get_value_string_utf8(env, argv[1], search, SEARCH_BUFFER_SIZE, &searchLen);

    int result = StringHelper::CountOccurrences(std::string(input, inputLen), std::string(search, searchLen));

    napi_value output = nullptr;
    napi_create_int32(env, result, &output);
    return output;
}

/***********************************************
 * Module export and register
 ***********************************************/
static napi_value StringExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("toUpper", JSStringToUpper),
        DECLARE_NAPI_FUNCTION("toLower", JSStringToLower),
        DECLARE_NAPI_FUNCTION("trim", JSStringTrim),
        DECLARE_NAPI_FUNCTION("trimStart", JSStringTrimStart),
        DECLARE_NAPI_FUNCTION("trimEnd", JSStringTrimEnd),
        DECLARE_NAPI_FUNCTION("reverse", JSStringReverse),
        DECLARE_NAPI_FUNCTION("repeat", JSStringRepeat),
        DECLARE_NAPI_FUNCTION("padStart", JSStringPadStart),
        DECLARE_NAPI_FUNCTION("padEnd", JSStringPadEnd),
        DECLARE_NAPI_FUNCTION("split", JSStringSplit),
        DECLARE_NAPI_FUNCTION("replace", JSStringReplace),
        DECLARE_NAPI_FUNCTION("replaceAll", JSStringReplaceAll),
        DECLARE_NAPI_FUNCTION("contains", JSStringContains),
        DECLARE_NAPI_FUNCTION("startsWith", JSStringStartsWith),
        DECLARE_NAPI_FUNCTION("endsWith", JSStringEndsWith),
        DECLARE_NAPI_FUNCTION("indexOf", JSStringIndexOf),
        DECLARE_NAPI_FUNCTION("lastIndexOf", JSStringLastIndexOf),
        DECLARE_NAPI_FUNCTION("substring", JSStringSubstring),
        DECLARE_NAPI_FUNCTION("countOccurrences", JSStringCountOccurrences),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module g_stringModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = StringExport,
    .nm_modname = "string_ops",
    .nm_priv = nullptr,
    .reserved = { nullptr },
};

extern "C" __attribute__((constructor)) void StringRegister()
{
    napi_module_register(&g_stringModule);
}
