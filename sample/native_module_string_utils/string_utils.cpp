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

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr size_t ARG_FIRST = 0;
constexpr size_t ARG_SECOND = 1;
constexpr size_t ARG_THIRD = 2;

bool GetStringArg(napi_env env, napi_value arg, std::string& result)
{
    size_t length = 0;
    napi_status status = napi_get_value_string_utf8(env, arg, nullptr, 0, &length);
    if (status != napi_ok || length == 0) {
        return false;
    }
    result.resize(length);
    status = napi_get_value_string_utf8(env, arg, &result[0], length + 1, &length);
    return status == napi_ok;
}

bool GetStringArgs(napi_env env, napi_callback_info info, std::string* str1, std::string* str2 = nullptr,
    std::string* str3 = nullptr)
{
    size_t argc = str2 ? (str3 ? ARGC_THREE : ARGC_TWO) : ARGC_ONE;
    napi_value args[ARGC_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    if (argc < (str2 ? (str3 ? ARGC_THREE : ARGC_TWO) : ARGC_ONE)) {
        napi_throw_type_error(env, nullptr, "Wrong number of arguments");
        return false;
    }

    if (!GetStringArg(env, args[ARG_FIRST], *str1)) {
        napi_throw_type_error(env, nullptr, "First argument must be a string");
        return false;
    }

    if (str2 && !GetStringArg(env, args[ARG_SECOND], *str2)) {
        napi_throw_type_error(env, nullptr, "Second argument must be a string");
        return false;
    }

    if (str3 && !GetStringArg(env, args[ARG_THIRD], *str3)) {
        napi_throw_type_error(env, nullptr, "Third argument must be a string");
        return false;
    }

    return true;
}

}

static napi_value ToUpperCase(napi_env env, napi_callback_info info)
{
    std::string input;
    if (!GetStringArgs(env, info, &input)) {
        return nullptr;
    }

    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::toupper(c); });

    napi_value output = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, result.c_str(), result.length(), &output));

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, output, &valueType));
    if (valueType != napi_string) {
        napi_throw_type_error(env, nullptr, "Output is not a string");
        return nullptr;
    }

    return output;
}

static napi_value ToLowerCase(napi_env env, napi_callback_info info)
{
    std::string input;
    if (!GetStringArgs(env, info, &input)) {
        return nullptr;
    }

    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });

    napi_value output = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, result.c_str(), result.length(), &output));

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, output, &valueType));
    if (valueType != napi_string) {
        napi_throw_type_error(env, nullptr, "Output is not a string");
        return nullptr;
    }

    return output;
}

static napi_value Reverse(napi_env env, napi_callback_info info)
{
    std::string input;
    if (!GetStringArgs(env, info, &input)) {
        return nullptr;
    }

    std::string result = input;
    std::reverse(result.begin(), result.end());

    napi_value output = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, result.c_str(), result.length(), &output));

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, output, &valueType));
    if (valueType != napi_string) {
        napi_throw_type_error(env, nullptr, "Output is not a string");
        return nullptr;
    }

    return output;
}

static napi_value Trim(napi_env env, napi_callback_info info)
{
    std::string input;
    if (!GetStringArgs(env, info, &input)) {
        return nullptr;
    }

    std::string result = input;
    auto start = std::find_if_not(result.begin(), result.end(),
        [](unsigned char c) { return std::isspace(c); });
    auto end = std::find_if_not(result.rbegin(), result.rend(),
        [](unsigned char c) { return std::isspace(c); }).base();

    result = (start < end) ? std::string(start, end) : std::string();

    napi_value output = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, result.c_str(), result.length(), &output));

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, output, &valueType));
    if (valueType != napi_string) {
        napi_throw_type_error(env, nullptr, "Output is not a string");
        return nullptr;
    }

    return output;
}

static napi_value Concat(napi_env env, napi_callback_info info)
{
    std::string str1;
    std::string str2;
    if (!GetStringArgs(env, info, &str1, &str2)) {
        return nullptr;
    }

    std::string result = str1 + str2;

    napi_value output = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, result.c_str(), result.length(), &output));

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, output, &valueType));
    if (valueType != napi_string) {
        napi_throw_type_error(env, nullptr, "Output is not a string");
        return nullptr;
    }

    return output;
}

static napi_value Contains(napi_env env, napi_callback_info info)
{
    std::string str;
    std::string substr;
    if (!GetStringArgs(env, info, &str, &substr)) {
        return nullptr;
    }

    bool found = str.find(substr) != std::string::npos;

    napi_value result;
    NAPI_CALL(env, napi_get_boolean(env, found, &result));
    return result;
}

static napi_value StartsWith(napi_env env, napi_callback_info info)
{
    std::string str;
    std::string prefix;
    if (!GetStringArgs(env, info, &str, &prefix)) {
        return nullptr;
    }

    bool result = str.rfind(prefix, 0) == 0;

    napi_value output = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, result, &output));
    return output;
}

static napi_value EndsWith(napi_env env, napi_callback_info info)
{
    std::string str;
    std::string suffix;
    if (!GetStringArgs(env, info, &str, &suffix)) {
        return nullptr;
    }

    if (suffix.length() > str.length()) {
        napi_value result;
        NAPI_CALL(env, napi_get_boolean(env, false, &result));
        return result;
    }

    bool result = str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;

    napi_value output = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, result, &output));
    return output;
}

static napi_value Split(napi_env env, napi_callback_info info)
{
    std::string str;
    std::string delimiter;
    if (!GetStringArgs(env, info, &str, &delimiter)) {
        return nullptr;
    }

    if (delimiter.empty()) {
        napi_throw_type_error(env, nullptr, "Delimiter cannot be empty");
        return nullptr;
    }

    std::vector<std::string> parts;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos) {
        parts.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    parts.push_back(str.substr(start));

    napi_value array;
    NAPI_CALL(env, napi_create_array_with_length(env, parts.size(), &array));

    for (size_t i = 0; i < parts.size(); ++i) {
        napi_value element;
        NAPI_CALL(env, napi_create_string_utf8(env, parts[i].c_str(), parts[i].length(), &element));
        NAPI_CALL(env, napi_set_element(env, array, i, element));
    }

    uint32_t arrayLength = 0;
    NAPI_CALL(env, napi_get_array_length(env, array, &arrayLength));
    if (arrayLength != parts.size()) {
        napi_throw_type_error(env, nullptr, "Array length does not match parts size");
        return nullptr;
    }

    return array;
}

static napi_value Replace(napi_env env, napi_callback_info info)
{
    std::string str;
    std::string oldStr;
    std::string newStr;
    if (!GetStringArgs(env, info, &str, &oldStr, &newStr)) {
        return nullptr;
    }

    if (oldStr.empty()) {
        napi_throw_type_error(env, nullptr, "Old string cannot be empty");
        return nullptr;
    }

    std::string result = str;
    size_t pos = 0;
    while ((pos = result.find(oldStr, pos)) != std::string::npos) {
        result.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }

    napi_value output = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, result.c_str(), result.length(), &output));

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, output, &valueType));
    if (valueType != napi_string) {
        napi_throw_type_error(env, nullptr, "Output is not a string");
        return nullptr;
    }

    return output;
}

static napi_value PadStart(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value args[ARGC_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    if (argc < ARGC_ONE) {
        napi_throw_type_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }

    std::string str;
    if (!GetStringArg(env, args[ARG_FIRST], str)) {
        napi_throw_type_error(env, nullptr, "First argument must be a string");
        return nullptr;
    }

    int32_t targetLength = 0;
    if (argc >= ARGC_TWO) {
        napi_valuetype type;
        NAPI_CALL(env, napi_typeof(env, args[ARG_SECOND], &type));
        if (type != napi_number) {
            napi_throw_type_error(env, nullptr, "Second argument must be a number");
            return nullptr;
        }
        NAPI_CALL(env, napi_get_value_int32(env, args[ARG_SECOND], &targetLength));
    }

    std::string result = str;
    if (static_cast<int32_t>(result.length()) < targetLength) {
        result = std::string(targetLength - result.length(), ' ') + result;
    }

    napi_value output = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, result.c_str(), result.length(), &output));

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, output, &valueType));
    if (valueType != napi_string) {
        napi_throw_type_error(env, nullptr, "Output is not a string");
        return nullptr;
    }

    return output;
}

static napi_value PadEnd(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value args[ARGC_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    if (argc < ARGC_ONE) {
        napi_throw_type_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }

    std::string str;
    if (!GetStringArg(env, args[ARG_FIRST], str)) {
        napi_throw_type_error(env, nullptr, "First argument must be a string");
        return nullptr;
    }

    int32_t targetLength = 0;
    if (argc >= ARGC_TWO) {
        napi_valuetype type;
        NAPI_CALL(env, napi_typeof(env, args[ARG_SECOND], &type));
        if (type != napi_number) {
            napi_throw_type_error(env, nullptr, "Second argument must be a number");
            return nullptr;
        }
        NAPI_CALL(env, napi_get_value_int32(env, args[ARG_SECOND], &targetLength));
    }

    std::string result = str;
    if (static_cast<int32_t>(result.length()) < targetLength) {
        result += std::string(targetLength - result.length(), ' ');
    }

    napi_value output = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, result.c_str(), result.length(), &output));

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, output, &valueType));
    if (valueType != napi_string) {
        napi_throw_type_error(env, nullptr, "Output is not a string");
        return nullptr;
    }

    return output;
}

static napi_value Substring(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value args[ARGC_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    if (argc < ARGC_ONE) {
        napi_throw_type_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }

    std::string str;
    if (!GetStringArg(env, args[ARG_FIRST], str)) {
        napi_throw_type_error(env, nullptr, "First argument must be a string");
        return nullptr;
    }

    int32_t start = 0;
    if (argc >= ARGC_TWO) {
        napi_valuetype type;
        NAPI_CALL(env, napi_typeof(env, args[ARG_SECOND], &type));
        if (type == napi_number) {
            NAPI_CALL(env, napi_get_value_int32(env, args[ARG_SECOND], &start));
        }
    }

    int32_t length = str.length();
    if (argc >= ARGC_THREE) {
        napi_valuetype type;
        NAPI_CALL(env, napi_typeof(env, args[ARG_THIRD], &type));
        if (type == napi_number) {
            NAPI_CALL(env, napi_get_value_int32(env, args[ARG_THIRD], &length));
        }
    }

    if (start < 0 || start > length) {
        napi_throw_type_error(env, nullptr, "Start index out of range");
        return nullptr;
    }

    std::string result = str.substr(start, length);

    napi_value output = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, result.c_str(), result.length(), &output));

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, output, &valueType));
    if (valueType != napi_string) {
        napi_throw_type_error(env, nullptr, "Output is not a string");
        return nullptr;
    }

    return output;
}

static napi_value GetLength(napi_env env, napi_callback_info info)
{
    std::string str;
    if (!GetStringArgs(env, info, &str)) {
        return nullptr;
    }

    napi_value result;
    NAPI_CALL(env, napi_create_uint32(env, str.length(), &result));
    return result;
}

static napi_value Repeat(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TWO;
    napi_value args[ARGC_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    if (argc < ARGC_ONE) {
        napi_throw_type_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }

    std::string str;
    if (!GetStringArg(env, args[ARG_FIRST], str)) {
        napi_throw_type_error(env, nullptr, "First argument must be a string");
        return nullptr;
    }

    int32_t count = 1;
    if (argc >= ARGC_TWO) {
        napi_valuetype type;
        NAPI_CALL(env, napi_typeof(env, args[ARG_SECOND], &type));
        if (type != napi_number) {
            napi_throw_type_error(env, nullptr, "Second argument must be a number");
            return nullptr;
        }
        NAPI_CALL(env, napi_get_value_int32(env, args[ARG_SECOND], &count));
    }

    if (count < 0) {
        napi_throw_type_error(env, nullptr, "Count cannot be negative");
        return nullptr;
    }

    std::string result;
    for (int32_t i = 0; i < count; ++i) {
        result += str;
    }

    napi_value output = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, result.c_str(), result.length(), &output));

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, output, &valueType));
    if (valueType != napi_string) {
        napi_throw_type_error(env, nullptr, "Output is not a string");
        return nullptr;
    }

    return output;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("toUpperCase", ToUpperCase),
        DECLARE_NAPI_FUNCTION("toLowerCase", ToLowerCase),
        DECLARE_NAPI_FUNCTION("reverse", Reverse),
        DECLARE_NAPI_FUNCTION("trim", Trim),
        DECLARE_NAPI_FUNCTION("concat", Concat),
        DECLARE_NAPI_FUNCTION("contains", Contains),
        DECLARE_NAPI_FUNCTION("startsWith", StartsWith),
        DECLARE_NAPI_FUNCTION("endsWith", EndsWith),
        DECLARE_NAPI_FUNCTION("split", Split),
        DECLARE_NAPI_FUNCTION("replace", Replace),
        DECLARE_NAPI_FUNCTION("padStart", PadStart),
        DECLARE_NAPI_FUNCTION("padEnd", PadEnd),
        DECLARE_NAPI_FUNCTION("substring", Substring),
        DECLARE_NAPI_FUNCTION("getLength", GetLength),
        DECLARE_NAPI_FUNCTION("repeat", Repeat),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    return exports;
}
EXTERN_C_END

static napi_module stringUtilsModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "stringutils",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void StringutilsRegisterModule(void)
{
    napi_module_register(&stringUtilsModule);
}
