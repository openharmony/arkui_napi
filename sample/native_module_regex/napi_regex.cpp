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
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

static constexpr int REQUIRED_ARGS_ONE = 1;
static constexpr int REQUIRED_ARGS_TWO = 2;
static constexpr int REQUIRED_ARGS_THREE = 3;
static constexpr int ARG_INDEX_ZERO = 0;
static constexpr int ARG_INDEX_ONE = 1;
static constexpr int ARG_INDEX_TWO = 2;

struct RegexMatch {
    std::string match;
    size_t index;
    size_t length;
};

static bool IsDigit(char c)
{
    return c >= '0' && c <= '9';
}

static bool IsLetter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool IsWhitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static bool IsDigitPattern(const std::string& str)
{
    if (str.empty()) {
        return false;
    }
    for (char c : str) {
        if (!IsDigit(c)) {
            return false;
        }
    }
    return true;
}

static bool IsLetterPattern(const std::string& str)
{
    if (str.empty()) {
        return false;
    }
    for (char c : str) {
        if (!IsLetter(c)) {
            return false;
        }
    }
    return true;
}

static bool IsWhitespacePattern(const std::string& str)
{
    if (str.empty()) {
        return false;
    }
    for (char c : str) {
        if (!IsWhitespace(c)) {
            return false;
        }
    }
    return true;
}

static bool MatchesPattern(const std::string& str, const std::string& pattern)
{
    if (pattern.empty()) {
        return str.empty();
    }

    if (pattern == "\\d+") {
        return IsDigitPattern(str);
    }

    if (pattern == "\\w+") {
        if (str.empty()) {
            return false;
        }
        for (char c : str) {
            if (!IsLetter(c) && !IsDigit(c) && c != '_') {
                return false;
            }
        }
        return true;
    }

    if (pattern == "\\s+") {
        return IsWhitespacePattern(str);
    }

    if (pattern == "[0-9]+") {
        return IsDigitPattern(str);
    }

    if (pattern == "[a-zA-Z]+") {
        return IsLetterPattern(str);
    }

    return str == pattern;
}

static napi_value Test(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: string, pattern");

    napi_valuetype type1 = napi_undefined;
    napi_valuetype type2 = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &type1));
    NAPI_CALL(env, napi_typeof(env, argv[1], &type2));
    NAPI_ASSERT(env, type1 == napi_string, "First argument must be a string");
    NAPI_ASSERT(env, type2 == napi_string, "Second argument must be a string");

    size_t strLen = 0;
    size_t patternLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strLen));
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[1], nullptr, 0, &patternLen));

    std::string str(strLen, '\0');
    std::string pattern(patternLen, '\0');
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], &str[0], strLen + 1, &strLen));
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[1], &pattern[0], patternLen + 1, &patternLen));

    bool result = MatchesPattern(str, pattern);

    napi_value returnValue = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, result, &returnValue));
    return returnValue;
}

static void FindDigitMatches(const std::string& str, std::vector<RegexMatch>& matches)
{
    size_t i = 0;
    while (i < str.length()) {
        if (!IsDigit(str[i])) {
            i++;
            continue;
        }
        size_t start = i;
        while (i < str.length() && IsDigit(str[i])) {
            i++;
        }
        matches.push_back({str.substr(start, i - start), start, i - start});
    }
}

static void FindLetterMatches(const std::string& str, std::vector<RegexMatch>& matches)
{
    size_t i = 0;
    while (i < str.length()) {
        if (!IsLetter(str[i])) {
            i++;
            continue;
        }
        size_t start = i;
        while (i < str.length() && IsLetter(str[i])) {
            i++;
        }
        matches.push_back({str.substr(start, i - start), start, i - start});
    }
}

static bool IsWordChar(char c)
{
    return IsLetter(c) || IsDigit(c) || c == '_';
}

static void FindWordMatches(const std::string& str, std::vector<RegexMatch>& matches)
{
    size_t i = 0;
    while (i < str.length()) {
        if (!IsWordChar(str[i])) {
            i++;
            continue;
        }
        size_t start = i;
        while (i < str.length() && IsWordChar(str[i])) {
            i++;
        }
        matches.push_back({str.substr(start, i - start), start, i - start});
    }
}

static void FindSimpleMatches(const std::string& str, const std::string& pattern, std::vector<RegexMatch>& matches)
{
    size_t pos = str.find(pattern);
    while (pos != std::string::npos) {
        matches.push_back({pattern, pos, pattern.length()});
        pos = str.find(pattern, pos + 1);
    }
}

static std::vector<RegexMatch> FindAllMatches(const std::string& str, const std::string& pattern)
{
    std::vector<RegexMatch> matches;

    if (pattern == "\\d+") {
        FindDigitMatches(str, matches);
    } else if (pattern == "\\w+") {
        FindWordMatches(str, matches);
    } else if (pattern == "[0-9]+") {
        FindDigitMatches(str, matches);
    } else if (pattern == "[a-zA-Z]+") {
        FindLetterMatches(str, matches);
    } else {
        FindSimpleMatches(str, pattern, matches);
    }

    return matches;
}

static napi_value CreateMatchObject(napi_env env, const RegexMatch& match)
{
    napi_value matchObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &matchObj));

    napi_value matchStr = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, match.match.c_str(),
                                           match.match.length(), &matchStr));
    NAPI_CALL(env, napi_set_named_property(env, matchObj, "match", matchStr));

    napi_value indexValue = nullptr;
    NAPI_CALL(env, napi_create_uint32(env, match.index, &indexValue));
    NAPI_CALL(env, napi_set_named_property(env, matchObj, "index", indexValue));

    napi_value lengthValue = nullptr;
    NAPI_CALL(env, napi_create_uint32(env, match.length, &lengthValue));
    NAPI_CALL(env, napi_set_named_property(env, matchObj, "length", lengthValue));

    return matchObj;
}

static napi_value MatchAll(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: string, pattern");

    napi_valuetype type1 = napi_undefined;
    napi_valuetype type2 = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &type1));
    NAPI_CALL(env, napi_typeof(env, argv[1], &type2));
    NAPI_ASSERT(env, type1 == napi_string, "First argument must be a string");
    NAPI_ASSERT(env, type2 == napi_string, "Second argument must be a string");

    size_t strLen = 0;
    size_t patternLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strLen));
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[1], nullptr, 0, &patternLen));

    std::string str(strLen, '\0');
    std::string pattern(patternLen, '\0');
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], &str[0], strLen + 1, &strLen));
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[1], &pattern[0], patternLen + 1, &patternLen));

    std::vector<RegexMatch> matches = FindAllMatches(str, pattern);

    napi_value resultArray = nullptr;
    NAPI_CALL(env, napi_create_array(env, &resultArray));

    for (size_t i = 0; i < matches.size(); i++) {
        napi_value matchObj = CreateMatchObject(env, matches[i]);
        NAPI_CALL(env, napi_set_element(env, resultArray, i, matchObj));
    }

    return resultArray;
}

static std::string ReplaceDigit(const std::string& str, const std::string& replacement)
{
    std::string result = str;
    size_t i = 0;
    while (i < result.length()) {
        if (!IsDigit(result[i])) {
            i++;
            continue;
        }
        size_t start = i;
        while (i < result.length() && IsDigit(result[i])) {
            i++;
        }
        result.replace(start, i - start, replacement);
        i = start + replacement.length();
    }
    return result;
}

static std::string ReplaceWord(const std::string& str, const std::string& replacement)
{
    std::string result = str;
    size_t i = 0;
    while (i < result.length()) {
        if (!IsWordChar(result[i])) {
            i++;
            continue;
        }
        size_t start = i;
        while (i < result.length() && IsWordChar(result[i])) {
            i++;
        }
        result.replace(start, i - start, replacement);
        i = start + replacement.length();
    }
    return result;
}

static std::string ReplaceSimple(const std::string& str, const std::string& pattern,
                                 const std::string& replacement)
{
    std::string result = str;
    size_t pos = 0;
    while ((pos = result.find(pattern, pos)) != std::string::npos) {
        result.replace(pos, pattern.length(), replacement);
        pos += replacement.length();
    }
    return result;
}

static std::string ReplacePattern(const std::string& str, const std::string& pattern,
                                  const std::string& replacement)
{
    if (pattern == "\\d+") {
        return ReplaceDigit(str, replacement);
    } else if (pattern == "\\w+") {
        return ReplaceWord(str, replacement);
    } else {
        return ReplaceSimple(str, pattern, replacement);
    }
}

static napi_value Replace(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: string, pattern, replacement");

    napi_valuetype type1 = napi_undefined;
    napi_valuetype type2 = napi_undefined;
    napi_valuetype type3 = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ZERO], &type1));
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ONE], &type2));
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_TWO], &type3));
    NAPI_ASSERT(env, type1 == napi_string, "First argument must be a string");
    NAPI_ASSERT(env, type2 == napi_string, "Second argument must be a string");
    NAPI_ASSERT(env, type3 == napi_string, "Third argument must be a string");

    size_t strLen = 0;
    size_t patternLen = 0;
    size_t replacementLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[ARG_INDEX_ZERO], nullptr, 0, &strLen));
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[ARG_INDEX_ONE], nullptr, 0, &patternLen));
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[ARG_INDEX_TWO], nullptr, 0, &replacementLen));

    std::string str(strLen, '\0');
    std::string pattern(patternLen, '\0');
    std::string replacement(replacementLen, '\0');
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[ARG_INDEX_ZERO], &str[0], strLen + 1, &strLen));
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[ARG_INDEX_ONE], &pattern[0], patternLen + 1, &patternLen));
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[ARG_INDEX_TWO], &replacement[0],
                                              replacementLen + 1, &replacementLen));

    std::string result = ReplacePattern(str, pattern, replacement);

    napi_value returnValue = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, result.c_str(), result.length(), &returnValue));
    return returnValue;
}

static std::vector<std::string> SplitString(const std::string& str, const std::string& pattern)
{
    std::vector<std::string> parts;

    if (pattern.empty()) {
        for (char c : str) {
            parts.push_back(std::string(1, c));
        }
        return parts;
    }

    size_t start = 0;
    size_t pos = str.find(pattern);

    while (pos != std::string::npos) {
        parts.push_back(str.substr(start, pos - start));
        start = pos + pattern.length();
        pos = str.find(pattern, start);
    }

    parts.push_back(str.substr(start));
    return parts;
}

static napi_value Split(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: string, pattern");

    napi_valuetype type1 = napi_undefined;
    napi_valuetype type2 = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &type1));
    NAPI_CALL(env, napi_typeof(env, argv[1], &type2));
    NAPI_ASSERT(env, type1 == napi_string, "First argument must be a string");
    NAPI_ASSERT(env, type2 == napi_string, "Second argument must be a string");

    size_t strLen = 0;
    size_t patternLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strLen));
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[1], nullptr, 0, &patternLen));

    std::string str(strLen, '\0');
    std::string pattern(patternLen, '\0');
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], &str[0], strLen + 1, &strLen));
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[1], &pattern[0], patternLen + 1, &patternLen));

    std::vector<std::string> parts = SplitString(str, pattern);

    napi_value resultArray = nullptr;
    NAPI_CALL(env, napi_create_array(env, &resultArray));

    for (size_t i = 0; i < parts.size(); i++) {
        napi_value partStr = nullptr;
        NAPI_CALL(env, napi_create_string_utf8(env, parts[i].c_str(),
                                               parts[i].length(), &partStr));
        NAPI_CALL(env, napi_set_element(env, resultArray, i, partStr));
    }

    return resultArray;
}

static napi_value Escape(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: string");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &type));
    NAPI_ASSERT(env, type == napi_string, "Argument must be a string");

    size_t strLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strLen));

    std::string str(strLen, '\0');
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], &str[0], strLen + 1, &strLen));

    std::string result;
    for (char c : str) {
        switch (c) {
            case '.':
            case '^':
            case '$':
            case '*':
            case '+':
            case '?':
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}':
            case '|':
            case '\\':
                result += '\\';
                result += c;
                break;
            default:
                result += c;
                break;
        }
    }

    napi_value returnValue = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, result.c_str(), result.length(), &returnValue));
    return returnValue;
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("test", Test),
        DECLARE_NAPI_FUNCTION("matchAll", MatchAll),
        DECLARE_NAPI_FUNCTION("replace", Replace),
        DECLARE_NAPI_FUNCTION("split", Split),
        DECLARE_NAPI_FUNCTION("escape", Escape),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
                                      sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module regexModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "regex",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegexRegisterModule(void)
{
    napi_module_register(&regexModule);
}