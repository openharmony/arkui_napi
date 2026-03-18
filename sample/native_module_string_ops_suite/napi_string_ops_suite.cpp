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
#include <cstring>
#include <string>
#include <algorithm>
#include <cctype>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

static constexpr int REQUIRED_ARGS_ONE = 1;
static constexpr int REQUIRED_ARGS_TWO = 2;
static constexpr int REQUIRED_ARGS_THREE = 3;
static constexpr int ARG_INDEX_ZERO = 0;
static constexpr int ARG_INDEX_ONE = 1;
static constexpr int ARG_INDEX_TWO = 2;
static constexpr int MAX_STRING_LENGTH = 1024;
static constexpr int MAX_SPLIT_PARTS = 100;
static constexpr int MAX_REPEAT_COUNT = 100;

static std::string ExtractUtf8String(napi_env env, napi_value value)
{
    size_t bufferSize = 0;
    NAPI_CALL_BASE(env, napi_get_value_string_utf8(env, value, nullptr, 0,
                                                     &bufferSize), std::string());

    if (bufferSize == 0) {
        return std::string();
    }

    if (bufferSize > MAX_STRING_LENGTH) {
        bufferSize = MAX_STRING_LENGTH;
    }

    std::string result(bufferSize, '\0');
    size_t copiedLength = 0;
    NAPI_CALL_BASE(env, napi_get_value_string_utf8(env, value, &result[0],
                                                     bufferSize, &copiedLength), std::string());
    result.resize(copiedLength);
    return result;
}

static napi_value CreateUtf8String(napi_env env, const std::string& str)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, str.c_str(), str.length(),
                                             &result));
    return result;
}

static napi_value ConcatStrings(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: str1, str2");

    napi_valuetype type1 = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ZERO], &type1));
    NAPI_ASSERT(env, type1 == napi_string, "First argument must be a string");

    napi_valuetype type2 = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ONE], &type2));
    NAPI_ASSERT(env, type2 == napi_string, "Second argument must be a string");

    std::string str1 = ExtractUtf8String(env, argv[ARG_INDEX_ZERO]);
    std::string str2 = ExtractUtf8String(env, argv[ARG_INDEX_ONE]);

    return CreateUtf8String(env, str1 + str2);
}

static napi_value ToUpperCase(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: string");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ZERO], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Argument must be a string");

    std::string str = ExtractUtf8String(env, argv[ARG_INDEX_ZERO]);
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::toupper(c); });

    return CreateUtf8String(env, str);
}

static napi_value ToLowerCase(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: string");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ZERO], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Argument must be a string");

    std::string str = ExtractUtf8String(env, argv[ARG_INDEX_ZERO]);
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    return CreateUtf8String(env, str);
}

static napi_value TrimString(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: string");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ZERO], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Argument must be a string");

    std::string str = ExtractUtf8String(env, argv[ARG_INDEX_ZERO]);

    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) {
        return CreateUtf8String(env, "");
    }

    size_t end = str.find_last_not_of(" \t\n\r");
    return CreateUtf8String(env, str.substr(start, end - start + 1));
}

static napi_value Substring(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires at least 2 arguments");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ZERO], &type));
    NAPI_ASSERT(env, type == napi_string, "First argument must be a string");

    int32_t startIndex = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[ARG_INDEX_ONE],
                                         &startIndex));

    std::string str = ExtractUtf8String(env, argv[ARG_INDEX_ZERO]);

    int32_t length = static_cast<int32_t>(str.length());
    if (argc >= REQUIRED_ARGS_THREE) {
        NAPI_CALL(env, napi_get_value_int32(env, argv[ARG_INDEX_TWO], &length));
    }

    if (startIndex < 0) {
        startIndex = 0;
    }

    if (startIndex >= static_cast<int32_t>(str.length())) {
        return CreateUtf8String(env, "");
    }

    int32_t maxLength = static_cast<int32_t>(str.length()) - startIndex;
    if (length > maxLength) {
        length = maxLength;
    }

    return CreateUtf8String(env, str.substr(startIndex, length));
}

static napi_value SplitString(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: str, separator");

    napi_valuetype type1 = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ZERO], &type1));
    NAPI_ASSERT(env, type1 == napi_string, "First argument must be a string");

    napi_valuetype type2 = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ONE], &type2));
    NAPI_ASSERT(env, type2 == napi_string, "Second argument must be a string");

    std::string str = ExtractUtf8String(env, argv[ARG_INDEX_ZERO]);
    std::string separator = ExtractUtf8String(env, argv[ARG_INDEX_ONE]);
    if (separator.empty()) {
        return CreateUtf8String(env, "");
    }

    napi_value array = nullptr;
    NAPI_CALL(env, napi_create_array(env, &array));

    size_t index = 0;
    size_t pos = 0;
    size_t sepLen = separator.length();

    while (pos < str.length() && index < MAX_SPLIT_PARTS) {
        size_t foundPos = str.find(separator, pos);
        if (foundPos == std::string::npos) {
            break;
        }

        std::string part = str.substr(pos, foundPos - pos);
        napi_value element = CreateUtf8String(env, part);
        NAPI_CALL(env, napi_set_element(env, array, index++, element));
        pos = foundPos + sepLen;
    }

    if (pos < str.length() && index < MAX_SPLIT_PARTS) {
        std::string part = str.substr(pos);
        napi_value element = CreateUtf8String(env, part);
        NAPI_CALL(env, napi_set_element(env, array, index++, element));
    }

    return array;
}

static napi_value ReplaceString(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE,
                "Requires 3 arguments: str, search, replace");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ZERO], &type));
    NAPI_ASSERT(env, type == napi_string, "First argument must be a string");

    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ONE], &type));
    NAPI_ASSERT(env, type == napi_string, "Second argument must be a string");

    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_TWO], &type));
    NAPI_ASSERT(env, type == napi_string, "Third argument must be a string");

    std::string str = ExtractUtf8String(env, argv[ARG_INDEX_ZERO]);
    std::string search = ExtractUtf8String(env, argv[ARG_INDEX_ONE]);
    std::string replace = ExtractUtf8String(env, argv[ARG_INDEX_TWO]);

    if (search.empty()) {
        return CreateUtf8String(env, str);
    }

    size_t pos = 0;
    while ((pos = str.find(search, pos)) != std::string::npos) {
        str.replace(pos, search.length(), replace);
        pos += replace.length();
    }

    return CreateUtf8String(env, str);
}

static napi_value StartsWith(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO,
                "Requires 2 arguments: str, prefix");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ZERO], &type));
    NAPI_ASSERT(env, type == napi_string, "First argument must be a string");

    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ONE], &type));
    NAPI_ASSERT(env, type == napi_string, "Second argument must be a string");

    std::string str = ExtractUtf8String(env, argv[ARG_INDEX_ZERO]);
    std::string prefix = ExtractUtf8String(env, argv[ARG_INDEX_ONE]);

    bool result = (str.length() >= prefix.length()) &&
                  (str.compare(0, prefix.length(), prefix) == 0);

    napi_value napiResult = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, result, &napiResult));
    return napiResult;
}

static napi_value EndsWith(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO,
                "Requires 2 arguments: str, suffix");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ZERO], &type));
    NAPI_ASSERT(env, type == napi_string, "First argument must be a string");

    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ONE], &type));
    NAPI_ASSERT(env, type == napi_string, "Second argument must be a string");

    std::string str = ExtractUtf8String(env, argv[ARG_INDEX_ZERO]);
    std::string suffix = ExtractUtf8String(env, argv[ARG_INDEX_ONE]);

    bool result = (str.length() >= suffix.length()) &&
                  (str.compare(str.length() - suffix.length(),
                               suffix.length(), suffix) == 0);

    napi_value napiResult = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, result, &napiResult));
    return napiResult;
}

static napi_value IndexOf(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO,
                "Requires 2 arguments: str, search");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ZERO], &type));
    NAPI_ASSERT(env, type == napi_string, "First argument must be a string");

    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ONE], &type));
    NAPI_ASSERT(env, type == napi_string, "Second argument must be a string");

    std::string str = ExtractUtf8String(env, argv[ARG_INDEX_ZERO]);
    std::string search = ExtractUtf8String(env, argv[ARG_INDEX_ONE]);

    size_t pos = str.find(search);
    int32_t result = (pos != std::string::npos) ?
                     static_cast<int32_t>(pos) : -1;

    napi_value napiResult = nullptr;
    NAPI_CALL(env, napi_create_int32(env, result, &napiResult));
    return napiResult;
}

static napi_value LastIndexOf(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO,
                "Requires 2 arguments: str, search");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ZERO], &type));
    NAPI_ASSERT(env, type == napi_string, "First argument must be a string");

    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ONE], &type));
    NAPI_ASSERT(env, type == napi_string, "Second argument must be a string");

    std::string str = ExtractUtf8String(env, argv[ARG_INDEX_ZERO]);
    std::string search = ExtractUtf8String(env, argv[ARG_INDEX_ONE]);

    size_t pos = str.rfind(search);
    int32_t result = (pos != std::string::npos) ?
                     static_cast<int32_t>(pos) : -1;

    napi_value napiResult = nullptr;
    NAPI_CALL(env, napi_create_int32(env, result, &napiResult));
    return napiResult;
}

static napi_value RepeatString(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO,
                "Requires 2 arguments: str, count");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ZERO], &type));
    NAPI_ASSERT(env, type == napi_string, "First argument must be a string");

    int32_t count = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[ARG_INDEX_ONE], &count));
    NAPI_ASSERT(env, count >= 0, "Count must be non-negative");
    NAPI_ASSERT(env, count <= MAX_REPEAT_COUNT, "Count must be less than or equal to 100");

    std::string str = ExtractUtf8String(env, argv[ARG_INDEX_ZERO]);
    std::string result;

    for (int32_t i = 0; i < count; ++i) {
        result += str;
    }

    return CreateUtf8String(env, result);
}

static napi_value PadString(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO,
                "Requires at least 2 arguments: str, targetLength");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ZERO], &type));
    NAPI_ASSERT(env, type == napi_string, "First argument must be a string");

    int32_t targetLength = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[ARG_INDEX_ONE], &targetLength));
    NAPI_ASSERT(env, targetLength >= 0, "Target length must be non-negative");

    std::string str = ExtractUtf8String(env, argv[ARG_INDEX_ZERO]);
    std::string padString = " ";

    if (argc >= REQUIRED_ARGS_THREE) {
        NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_TWO], &type));
        if (type == napi_string) {
            padString = ExtractUtf8String(env, argv[ARG_INDEX_TWO]);
        }
    }

    if (padString.empty()) {
        return CreateUtf8String(env, str);
    }

    while (static_cast<int32_t>(str.length()) < targetLength) {
        str += padString;
    }

    if (static_cast<int32_t>(str.length()) > targetLength) {
        str.resize(targetLength);
    }

    return CreateUtf8String(env, str);
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("concat", ConcatStrings),
        DECLARE_NAPI_FUNCTION("toUpperCase", ToUpperCase),
        DECLARE_NAPI_FUNCTION("toLowerCase", ToLowerCase),
        DECLARE_NAPI_FUNCTION("trim", TrimString),
        DECLARE_NAPI_FUNCTION("substring", Substring),
        DECLARE_NAPI_FUNCTION("split", SplitString),
        DECLARE_NAPI_FUNCTION("replace", ReplaceString),
        DECLARE_NAPI_FUNCTION("startsWith", StartsWith),
        DECLARE_NAPI_FUNCTION("endsWith", EndsWith),
        DECLARE_NAPI_FUNCTION("indexOf", IndexOf),
        DECLARE_NAPI_FUNCTION("lastIndexOf", LastIndexOf),
        DECLARE_NAPI_FUNCTION("repeat", RepeatString),
        DECLARE_NAPI_FUNCTION("padEnd", PadString),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
                                      sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module stringOpsModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "stringOpsSuite",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void StringOpsRegisterModule(void)
{
    napi_module_register(&stringOpsModule);
}
