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
#include <ctime>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "binary_complex_js.h"

constexpr int ALPHABET_SIZE = 26;
constexpr int ASCII_CASE_DIFF = 32;
constexpr int YEAR_BASE = 1900;
constexpr int NUMBER_0 = 0;
constexpr int NUMBER_1 = 1;
constexpr int NUMBER_2 = 2;
constexpr int NUMBER_3 = 3;

// 使用带大小的数组声明
constexpr int2 = 2;
constexpr int NUMBER_3 = 3;

/*
 * String utilities - Advanced
 */
static napi_value StringToUpperCase(napi_env env, napi_callback_info info)
{
    size_t requireArgc = NUMBER_1;
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));

    NAPI_ASSERT(env, valuetype0 == napi_string, "Wrong argument type. String expected.");

    size_t strLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], nullptr, 0, &strLen));

    char* str = new char[strLen + 1];
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], str, strLen + 1, nullptr));

    // Convert to uppercase
    for (size_t i = 0; i < strLen; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] -= ASCII_CASE_DIFF;
        }
    }

    napi_value result;
    NAPI_CALL(env, napi_create_string_utf8(env, str, NAPI_AUTO_LENGTH, &result));

    delete[] str;
    return result;
}

static napi_value StringToLowerCase(napi_env env, napi_callback_info info)
{
    size_t requireArgc = NUMBER_1;
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));

    NAPI_ASSERT(env, valuetype0 == napi_string, "Wrong argument type. String expected.");

    size_t strLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], nullptr, 0, &strLen));

    char* str = new char[strLen + 1];
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], str, strLen + 1, nullptr));

    // Convert to lowercase
    for (size_t i = 0; i < strLen; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] += ASCII_CASE_DIFF;
        }
    }

    napi_value result;
    NAPI_CALL(env, napi_create_string_utf8(env, str, NAPI_AUTO_LENGTH, &result));

    delete[] str;
    return result;
}

static napi_value TrimString(napi_env env, napi_callback_info info)
{
    size_t requireArgc = NUMBER_1;
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));

    NAPI_ASSERT(env, valuetype0 == napi_string, "Wrong argument type. String expected.");

    size_t strLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], nullptr, 0, &strLen));

    char* str = new char[strLen + 1];
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], str, strLen + 1, nullptr));

    // Trim whitespace
    size_t start = 0;
    while (start < strLen && (str[start] == ' ' || str[start] == '\t' || str[start] == '\n' || str[start] == '\r')) {
        start++;
    }

    size_t end = strLen - 1;
    while (end >= start && (str[end] == ' ' || str[end] == '\t' || str[end] == '\n' || str[end] == '\r')) {
        end--;
    }

    size_t newLen = end - start + 1;
    char* trimmed = new char[newLen + 1];
    for (size_t j = 0; j < newLen; j++) {
        trimmed[j] = str[start + j];
    }
    trimmed[newLen] = '\0';

    napi_value result;
    NAPI_CALL(env, napi_create_string_utf8(env, trimmed, NAPI_AUTO_LENGTH, &result));

    delete[] str;
    delete[] trimmed;
    return result;
}

static napi_value SplitString(napi_env env, napi_callback_info info)
{
    size_t requireArgc = NUMBER_2;
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_string, "Wrong argument type. String expected.");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, args[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_string, "Wrong argument type. Delimiter expected.");

    size_t strLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], nullptr, 0, &strLen));

    char* str = new char[strLen + 1];
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], str, strLen + 1, nullptr));

    size_t delimiterLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[1], nullptr, 0, &delimiterLen));

    char* delimiter = new char[delimiterLen + 1];
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[1], delimiter, delimiterLen + 1, nullptr));

    // Split string
    std::vector<std::string> parts;
    char* token = strtok(str, delimiter);
    while (token != nullptr) {
        parts.push_back(token);
        token = strtok(nullptr, delimiter);
    }

    // Create result array
    napi_value result;
    NAPI_CALL(env, napi_create_array(env, &result));

    for (size_t i = 0; i < parts.size(); i++) {
        napi_value element;
        NAPI_CALL(env, napi_create_string_utf8(env, parts[i].c_str(), NAPI_AUTO_LENGTH, &element));
        NAPI_CALL(env, napi_set_element(env, result, i, element));
    }

    delete[] str;
    delete[] delimiter;
    return result;
}

static napi_value JoinStrings(napi_env env, napi_callback_info info)
{
    size_t requireArgc = NUMBER_2;
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_object, "Wrong argument type. Array expected.");

    bool isArray;
    NAPI_CALL(env, napi_is_array(env, args[0], &isArray));
    NAPI_ASSERT(env, isArray, "Wrong argument type. Array expected.");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, args[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_string, "Wrong argument type. Separator expected.");

    uint32_t length;
    NAPI_CALL(env, napi_get_array_length(env, args[0], &length));

    size_t separatorLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[1], nullptr, 0, &separatorLen));

    char* separator = new char[separatorLen + 1];
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[1], separator, separatorLen + 1, nullptr));

    // Join strings
    std::string resultStr;
    for (uint32_t i = 0; i < length; i++) {
        napi_value element;
        NAPI_CALL(env, napi_get_element(env, args[0], i, &element));

        size_t elemLen = 0;
        NAPI_CALL(env, napi_get_value_string_utf8(env, element, nullptr, 0, &elemLen));

        char* elemStr = new char[elemLen + 1];
        NAPI_CALL(env, napi_get_value_string_utf8(env, element, elemStr, elemLen + 1, nullptr));

        resultStr += elemStr;
        if (i < length - 1) {
            resultStr += separator;
        }

        delete[] elemStr;
    }

    napi_value result;
    NAPI_CALL(env, napi_create_string_utf8(env, resultStr.c_str(), NAPI_AUTO_LENGTH, &result));

    delete[] separator;
    return result;
}

/*
 * Math utilities - Advanced
 */
static napi_value GCD(napi_env env, napi_callback_info info)
{
    size_t requireArgc = NUMBER_2;
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_number, "Wrong argument type. Number expected.");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, args[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_number, "Wrong argument type. Number expected.");

    double value0;
    NAPI_CALL(env, napi_get_value_double(env, args[0], &value0));

    double value1;
    NAPI_CALL(env, napi_get_value_double(env, args[1], &value1));

    int a = static_cast<int>(value0);
    int b = static_cast<int>(value1);

    // Calculate GCD using Euclidean algorithm
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }

    napi_value result;
    NAPI_CALL(env, napi_create_number(env, a, &result));
    return result;
}

static napi_value LCM(napi_env env, napi_callback_info info)
{
    size_t requireArgc = NUMBER_2;
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_number, "Wrong argument type. Number expected.");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, args[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_number, "Wrong argument type. Number expected.");

    double value0;
    NAPI_CALL(env, napi_get_value_double(env, args[0], &value0));

    double value1;
    NAPI_CALL(env, napi_get_value_double(env, args[1], &value1));

    int a = static_cast<int>(value0);
    int b = static_cast<int>(value1);

    // Calculate GCD first
    int gcd = a;
    int temp = b;
    while (temp != 0) {
        int t = temp;
        temp = gcd % temp;
        gcd = t;
    }

    // Calculate LCM
    int lcm = (a * b) / gcd;

    napi_value result;
    NAPI_CALL(env, napi_create_number(env, lcm, &result));
    return result;
}

static napi_value IsPerfectNumber(napi_env env, napi_callback_info info)
{
    size_t requireArgc = NUMBER_1;
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_number, "Wrong argument type. Number expected.");

    double value;
    NAPI_CALL(env, napi_get_value_double(env, args[0], &value));

    int n = static_cast<int>(value);
    NAPI_ASSERT(env, n > 0, "Number must be positive");

    // Check if perfect number
    int sum = 0;
    for (int i = 1; i < n; i++) {
        if (n % i == 0) {
            sum += i;
        }
    }

    bool isPerfect = (sum == n);

    napi_value result;
    NAPI_CALL(env, napi_create_boolean(env, isPerfect, &result));
    return result;
}

static napi_value GeneratePrimeNumbers(napi_env env, napi_callback_info info)
{
    size_t requireArgc = NUMBER_1;
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_number, "Wrong argument type. Number expected.");

    double value;
    NAPI_CALL(env, napi_get_value_double(env, args[0], &value));

    int n = static_cast<int>(value);
    NAPI_ASSERT(env, n > 0, "Number must be positive");

    // Generate prime numbers up to n using Sieve of Eratosthenes
    std::vector<bool> isPrime(n + 1, true);
    isPrime[0] = isPrime[1] = false;

    for (int i = 2; i * i <= n; i++) {
        if (isPrime[i]) {
            for (int j = i * i; j <= n; j += i) {
                isPrime[j] = false;
            }
        }
    }

    // Collect prime numbers
    std::vector<int> primes;
    for (int i = 2; i <= n; i++) {
        if (isPrime[i]) {
            primes.push_back(i);
        }
    }

    // Create result array
    napi_value result;
    NAPI_CALL(env, napi_create_array(env, &result));

    for (size_t i = 0; i < primes.size(); i++) {
        napi_value element;
        NAPI_CALL(env, napi_create_number(env, primes[i], &element));
        NAPI_CALL(env, napi_set_element(env, result, i, element));
    }

    return result;
}

/*
 * Array utilities - Advanced
 */
static napi_value FilterArray(napi_env env, napi_callback_info info)
{
    size_t requireArgc = NUMBER_2;
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_object, "Wrong argument type. Array expected.");

    bool isArray;
    NAPI_CALL(env, napi_is_array(env, args[0], &isArray));
    NAPI_ASSERT(env, isArray, "Wrong argument type. Array expected.");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, args[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_function, "Wrong argument type. Function expected.");

    uint32_t length;
    NAPI_CALL(env, napi_get_array_length(env, args[0], &length));

    // Filter array
    std::vector<napi_value> filtered;
    for (uint32_t i = 0; i < length; i++) {
        napi_value element;
        NAPI_CALL(env, napi_get_element(env, args[0], i, &element));

        // Call filter function
        napi_value argv[1] = { element };
        napi_value result;
        NAPI_CALL(env, napi_call_function(env, nullptr, args[1], 1, argv, &result));

        bool keep;
        NAPI_CALL(env, napi_get_value_bool(env, result, &keep));

        if (keep) {
            filtered.push_back(element);
        }
    }

    // Create result array
    napi_value result;
    NAPI_CALL(env, napi_create_array(env, &result));

    for (size_t i = 0; i < filtered.size(); i++) {
        NAPI_CALL(env, napi_set_element(env, result, i, filtered[i]));
    }

    return result;
}

static napi_value MapArray(napi_env env, napi_callback_info info)
{
    size_t requireArgc = NUMBER_2;
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_object, "Wrong argument type. Array expected.");

    bool isArray;
    NAPI_CALL(env, napi_is_array(env, args[0], &isArray));
    NAPI_ASSERT(env, isArray, "Wrong argument type. Array expected.");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, args[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_function, "Wrong argument type. Function expected.");

    uint32_t length;
    NAPI_CALL(env, napi_get_array_length(env, args[0], &length));

    // Map array
    napi_value result;
    NAPI_CALL(env, napi_create_array(env, &result));

    for (uint32_t i = 0; i < length; i++) {
        napi_value element;
        NAPI_CALL(env, napi_get_element(env, args[0], i, &element));

        // Call map function
        napi_value argv[1] = { element };
        napi_value mappedValue;
        NAPI_CALL(env, napi_call_function(env, nullptr, args[1], 1, argv, &mappedValue));

        NAPI_CALL(env, napi_set_element(env, result, i, mappedValue));
    }

    return result;
}

static napi_value ReduceArray(napi_env env, napi_callback_info info)
{
    size_t requireArgc = NUMBER_2;
    size_t argc = NUMBER_3;
    napi_value args[NUMBER_3] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_object, "Wrong argument type. Array expected.");

    bool isArray;
    NAPI_CALL(env, napi_is_array(env, args[0], &isArray));
    NAPI_ASSERT(env, isArray, "Wrong argument type. Array expected.");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, args[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_function, "Wrong argument type. Function expected.");

    uint32_t length;
    NAPI_CALL(env, napi_get_array_length(env, args[0], &length));

    napi_value accumulator;
    if (argc &gt;= NUMBER_3) {
        accumulator = args[1];
    } else {
        napi_value element;
        NAPI(env, napi_get_element(env, args[0], 0, &element));
        accumulator = element;
    }

    return accumulator;
}

static napi_value RemoveDuplicates(napi_env env, napi_callback_info info)
{
    size_t requireArgc = NUMBER_1;
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_object, "Wrong argument type. Array expected.");

    bool isArray;
    NAPI_CALL(env, napi_is_array(env, args[0], &isArray));
    NAPI_ASSERT(env, isArray, "Wrong argument type. Array expected.");

    uint32_t length;
    NAPI_CALL(env, napi_get_array_length(env, args[0], &length));

    // Remove duplicates
    std::vector<napi_value> unique;
    for (uint32_t i = 0; i < length; i++) {
        napi_value element;
        NAPI_CALL(env, napi_get_element(env, args[0], i, &element));

        bool isDuplicate = false;
        for (size_t j = 0; j < unique.size(); j++) {
            bool equal;
            NAPI_CALL(env, napi_strict_equals(env, element, unique[j], &equal));
            if (equal) {
                isDuplicate = true;
                break;
            }
        }

        if (!isDuplicate) {
            unique.push_back(element);
        }
    }

    // Create result array
    napi_value result;
    NAPI_CALL(env, napi_create_array(env, &result));

    for (size_t i = 0; i < unique.size(); i++) {
        NAPI_CALL(env, napi_set_element(env, result, i, unique[i]));
    }

    return result;
}

/*
 * Date utilities - Advanced
 */

static napi_value CreateDateProperty(napi_env env, int value)
{
    napi_value result;
    NAPI_CALL(env, napi_create_number(env, value, &result));
    return result;
}

static void SetDateComponent(napi_env env, napi_value result, const char* name, int value)
{
    napi_value prop = CreateDateProperty(env, value);
    NAPI_CALL(env, napi_set_named_property(env, result, name, prop));
}

static void SetAllDateComponents(napi_env env, napi_value result, struct tm* localTime)
{
    SetDateComponent(env, result, "year", localTime ? localTime->tm_year + YEAR_BASE : 0);
    SetDateComponent(env, result, "month", localTime ? localTime->tm_mon + 1 : 0);
    SetDateComponent(env, result, "day", localTime ? localTime->tm_mday : 0);
    SetDateComponent(env, result, "hour", localTime ? localTime->tm_hour : 0);
    SetDateComponent(env, result, "minute", localTime ? localTime->tm_min : 0);
    SetDateComponent(env, result, "second", localTime ? localTime->tm_sec : 0);
    SetDateComponent(env, result, "weekday", localTime ? localTime->tm_wday : 0);
}

static napi_value GetDateComponents(napi_env env, napi_callback_info info)
{
    size_t requireArgc = NUMBER_0;
    size_t argc = NUMBER_0;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr));

    time_t now = time(nullptr);
    struct tm* localTime = localtime(&now);

    napi_value result;
    NAPI_CALL(env, napi_create_object(env, &result));

    SetAllDateComponents(env, result, localTime);

    return result;
}

static napi_value FormatDate(napi_env env, napi_callback_info info)
{
    size_t requireArgc = NUMBER_1;
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_string, "Wrong argument type. Format string expected.");

    size_t formatLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], nullptr, 0, &formatLen));

    char* format = new char[formatLen + 1];
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], format, formatLen + 1, nullptr));

    time_t now = time(nullptr);
    struct tm* localTime = localtime(&now);

    std::string resultStr;
    if (localTime != nullptr) {
        char buffer[256];
        strftime(buffer, sizeof(buffer), format, localTime);
        resultStr = buffer;
    } else {
        resultStr = "Invalid";
    }

    napi_value result;
    NAPI_CALL(env, napi_create_string_utf8(env, resultStr.c_str(), NAPI_AUTO_LENGTH, &result));

    delete[] format;
    return result;
}

/*
 * Simple data structures
 */
// Simple linked list implementation
struct ListNode {
    int value;
    ListNode* next;
    explicit ListNode(int val) : value(val), next(nullptr) {}
};

static ListNode* BuildLinkedList(napi_env env, napi_value array, uint32_t length)
{
    ListNode* head = nullptr;
    ListNode* tail = nullptr;

    for (uint32_t i = 0; i < length; i++) {
        napi_value element;
        NAPI_CALL(env, napi_get_element(env, array, i, &element));

        double value;
        NAPI_CALL(env, napi_get_value_double(env, element, &value));

        ListNode* newNode = new ListNode(static_cast<int>(value));
        if (!head) {
            head = newNode;
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
    }
    return head;
}

static std::vector<int> LinkedListToValues(ListNode* head)
{
    std::vector<int> values;
    ListNode* current = head;
    while (current) {
        values.push_back(current->value);
        ListNode* temp = current;
        current = current->next;
        delete temp;
    }
    return values;
}

static napi_value CreateResultArray(napi_env env, const std::vector<int>& values)
{
    napi_value result;
    NAPI_CALL(env, napi_create_array(env, &result));

    for (size_t i = 0; i < values.size(); i++) {
        napi_value element;
        NAPI_CALL(env, napi_create_number(env, values[i], &element));
        NAPI_CALL(env, napi_set_element(env, result, i, element));
    }
    return result;
}

static napi_value CreateLinkedList(napi_env env, napi_callback_info info)
{
    size_t requireArgc = NUMBER_1;
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_object, "Wrong argument type. Array expected.");

    bool isArray;
    NAPI_CALL(env, napi_is_array(env, args[0], &isArray));
    NAPI_ASSERT(env, isArray, "Wrong argument type. Array expected.");

    uint32_t length;
    NAPI_CALL(env, napi_get_array_length(env, args[0], &length));

    ListNode* head = BuildLinkedList(env, args[0], length);
    std::vector<int> values = LinkedListToValues(head);
    return CreateResultArray(env, values);
}

/*
 * Simple encryption utilities
 */
static napi_value CaesarCipher(napi_env env, napi_callback_info info)
{
    size_t requireArgc = NUMBER_2;
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_string, "Wrong argument type. String expected.");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, args[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_number, "Wrong argument type. Shift expected.");

    size_t strLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], nullptr, 0, &strLen));

    char* str = new char[strLen + 1];
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], str, strLen + 1, nullptr));

    double shiftValue;
    NAPI_CALL(env, napi_get_value_double(env, args[1], &shiftValue));
    int shift = static_cast<int>(shiftValue) % ALPHABET_SIZE;
    if (shift < 0) {
        shift += ALPHABET_SIZE;
    }

    // Apply Caesar cipher
    for (size_t i = 0; i < strLen; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] = 'a' + (str[i] - 'a' + shift) % ALPHABET_SIZE;
        } else if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = 'A' + (str[i] - 'A' + shift) % ALPHABET_SIZE;
        }
    }

    napi_value result;
    NAPI_CALL(env, napi_create_string_utf8(env, str, NAPI_AUTO_LENGTH, &result));

    delete[] str;
    return result;
}

static napi_value VigenereCipher(napi_env env, napi_callback_info info)
{
    size_t requireArgc = NUMBER_2;
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_string, "Wrong argument type. String expected.");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, args[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_string, "Wrong argument type. Key expected.");

    size_t strLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], nullptr, 0, &strLen));

    char* str = new char[strLen + 1];
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], str, strLen + 1, nullptr));

    size_t keyLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[1], nullptr, 0, &keyLen));

    char* key = new char[keyLen + 1];
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[1], key, keyLen + 1, nullptr));

    // Check if key length is zero to avoid division by zero
    if (keyLen == 0) {
        napi_throw_error(env, "ERR_INVALID_ARGS", "Key length cannot be zero");
        delete[] str;
        delete[] key;
        return nullptr;
    }

    // Apply Vigenere cipher
    for (size_t i = 0, j = 0; i < strLen; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            int keyShift = key[j % keyLen] - 'a';
            if (keyShift < 0) {
                keyShift += ALPHABET_SIZE;
            }
            str[i] = 'a' + (str[i] - 'a' + keyShift) % ALPHABET_SIZE;
            j++;
        } else if (str[i] >= 'A' && str[i] <= 'Z') {
            int keyShift = key[j % keyLen] - 'A';
            if (keyShift < 0) {
                keyShift += ALPHABET_SIZE;
            }
            str[i] = 'A' + (str[i] - 'A' + keyShift) % ALPHABET_SIZE;
            j++;
        }
    }

    napi_value result;
    NAPI_CALL(env, napi_create_string_utf8(env, str, NAPI_AUTO_LENGTH, &result));

    delete[] str;
    delete[] key;
    return result;
}

/*
 * function for module exports
 */
static napi_value Init(napi_env env, napi_value exports)
{
    /*
     * Properties define
     */
    napi_property_descriptor desc[] = {
        // String utilities
        DECLARE_NAPI_FUNCTION("toUpperCase", StringToUpperCase),
        DECLARE_NAPI_FUNCTION("toLowerCase", StringToLowerCase),
        DECLARE_NAPI_FUNCTION("trim", TrimString),
        DECLARE_NAPI_FUNCTION("split", SplitString),
        DECLARE_NAPI_FUNCTION("join", JoinStrings),
        // Math utilities
        DECLARE_NAPI_FUNCTION("gcd", GCD),
        DECLARE_NAPI_FUNCTION("lcm", LCM),
        DECLARE_NAPI_FUNCTION("isPerfectNumber", IsPerfectNumber),
        DECLARE_NAPI_FUNCTION("generatePrimes", GeneratePrimeNumbers),
        // Array utilities
        DECLARE_NAPI_FUNCTION("filter", FilterArray),
        DECLARE_NAPI_FUNCTION("map", MapArray),
        DECLARE_NAPI_FUNCTION("reduce", ReduceArray),
        DECLARE_NAPI_FUNCTION("removeDuplicates", RemoveDuplicates),
        // Date utilities
        DECLARE_NAPI_FUNCTION("getDateComponents", GetDateComponents),
        DECLARE_NAPI_FUNCTION("formatDate", FormatDate),
        // Data structures
        DECLARE_NAPI_FUNCTION("createLinkedList", CreateLinkedList),
        // Encryption utilities
        DECLARE_NAPI_FUNCTION("caesarCipher", CaesarCipher),
        DECLARE_NAPI_FUNCTION("vigenereCipher", VigenereCipher),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

extern "C" __attribute__((visibility("default"))) void NapiComplexGetJsCode(const char** buf, int* bufLen)
{
    if (buf != nullptr) {
        *buf = BINARY_COMPLEX_JS_START;
    }

    if (bufLen != nullptr) {
        *bufLen = BINARY_COMPLEX_JS_END - BINARY_COMPLEX_JS_START;
    }
}

/*
 * Module define
 */
static napi_module complexModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "complex",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};
/*
 * Module register function
 */
extern "C" __attribute__((constructor)) void ComplexRegisterModule(void)
{
    napi_module_register(&complexModule);
}