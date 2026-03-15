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

#ifndef FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_STRING_JS_STRING_HELPER_H
#define FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_STRING_JS_STRING_HELPER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "securec.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace StringBufferSize {
    constexpr size_t INPUT_BUFFER_SIZE = 1024;
    constexpr size_t OUTPUT_BUFFER_SIZE = 2048;
    constexpr size_t DELIMITER_BUFFER_SIZE = 64;
    constexpr size_t SEARCH_BUFFER_SIZE = 256;
    constexpr size_t REPLACE_BUFFER_SIZE = 256;
};

enum StringArgIndex {
    STRING_FIRST_ARG = 0,
    STRING_SECOND_ARG,
    STRING_THIRD_ARG,
    STRING_FOURTH_ARG,
};

namespace StringArgCount {
    constexpr size_t TWO = 2;
    constexpr size_t THREE = 3;
};

enum StringOperation {
    STRING_OP_UNKNOWN = -1,
    STRING_OP_UPPER,
    STRING_OP_LOWER,
    STRING_OP_TRIM,
    STRING_OP_TRIM_START,
    STRING_OP_TRIM_END,
    STRING_OP_REVERSE,
    STRING_OP_REPEAT,
    STRING_OP_PAD_START,
    STRING_OP_PAD_END,
};

struct StringAsyncContext {
    napi_env env = nullptr;
    napi_async_work work = nullptr;

    char input[StringBufferSize::INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    char output[StringBufferSize::OUTPUT_BUFFER_SIZE] = { 0 };
    size_t outputLen = 0;
    char delimiter[StringBufferSize::DELIMITER_BUFFER_SIZE] = { 0 };
    size_t delimiterLen = 0;
    char search[StringBufferSize::SEARCH_BUFFER_SIZE] = { 0 };
    size_t searchLen = 0;
    char replace[StringBufferSize::REPLACE_BUFFER_SIZE] = { 0 };
    size_t replaceLen = 0;

    int32_t repeatCount = 0;
    int32_t padLength = 0;

    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;

    int status = 0;
};

class StringHelper {
public:
    static std::string ToUpper(const std::string& input);
    static std::string ToLower(const std::string& input);
    static std::string Trim(const std::string& input);
    static std::string TrimStart(const std::string& input);
    static std::string TrimEnd(const std::string& input);
    static std::string Reverse(const std::string& input);
    static std::string Repeat(const std::string& input, int count);
    static std::string PadStart(const std::string& input, int targetLength, const std::string& padStr);
    static std::string PadEnd(const std::string& input, int targetLength, const std::string& padStr);
    static std::vector<std::string> Split(const std::string& input, const std::string& delimiter);
    static std::string Replace(const std::string& input, const std::string& search, const std::string& replace);
    static std::string ReplaceAll(const std::string& input, const std::string& search, const std::string& replace);
    static bool Contains(const std::string& input, const std::string& search);
    static bool StartsWith(const std::string& input, const std::string& prefix);
    static bool EndsWith(const std::string& input, const std::string& suffix);
    static int IndexOf(const std::string& input, const std::string& search);
    static int LastIndexOf(const std::string& input, const std::string& search);
    static std::string Substring(const std::string& input, int start, int length);
    static int CountOccurrences(const std::string& input, const std::string& search);
};

#endif
