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

#ifndef FOUNDATION_ACE_NAPI_SAMPLE_NATIVE_MODULE_PROPERTY_NAMES_SUITE_HELPER_H
#define FOUNDATION_ACE_NAPI_SAMPLE_NATIVE_MODULE_PROPERTY_NAMES_SUITE_HELPER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace PropertyNamesConst {
    constexpr size_t K_MAX_STRING_LENGTH = 256;
    constexpr size_t K_NULL_TERMINATOR_SIZE = 1;
    constexpr int32_t K_INVALID_INDEX = -1;
    constexpr int32_t K_BASE_KEY_COUNT = 1;
    constexpr size_t K_FIRST_INDEX = 0;
    constexpr size_t K_SECOND_INDEX = 1;
    constexpr size_t K_THIRD_INDEX = 2;
    constexpr size_t K_FOURTH_INDEX = 3;
    constexpr size_t K_FIFTH_INDEX = 4;
    constexpr size_t K_SIXTH_INDEX = 5;
    constexpr size_t K_SEVENTH_INDEX = 6;
    constexpr size_t K_EIGHTH_INDEX = 7;
    constexpr int K_CASE_NUMBER_WIDTH = 2;
    constexpr int32_t K_VALUE_MULTIPLIER = 10;
    constexpr int32_t K_ATTRIBUTE_SHIFT = 3;
    constexpr uint32_t K_MODULE_VERSION = 1;
    constexpr uint32_t K_NO_MODULE_FLAGS = 0;
    constexpr size_t K_KEY_NAME_COUNT = 6;
    constexpr size_t K_FILTER_COUNT = 6;
    constexpr size_t K_COLLECTION_MODE_COUNT = 2;
    constexpr size_t K_CONVERSION_MODE_COUNT = 2;
    constexpr int32_t K_BASE_INT_VALUE = 100;
    constexpr int32_t K_VALUE_INCREMENT_STEP = 7;
    constexpr int32_t K_MAX_EXTRA_KEYS = 5;
    constexpr size_t K_MIN_ARG_COUNT = 1;
    constexpr size_t K_ARG_COUNT_TWO = 2;
    constexpr int32_t K_EXPECTED_KEY_COUNT_BASE = 3;
    constexpr size_t K_RESERVED_SIZE = 4;
    constexpr int32_t K_WITNESS_VALUE_A = 42;
    constexpr int32_t K_WITNESS_VALUE_B = 73;
    constexpr int32_t K_WITNESS_VALUE_C = 256;
    constexpr double K_DOUBLE_VALUE_A = 3.14159;
    constexpr double K_DOUBLE_VALUE_B = 2.71828;
    constexpr double K_DOUBLE_VALUE_C = 1.41421;
    constexpr int32_t K_NEGATIVE_VALUE = -100;
    constexpr int64_t K_LARGE_INT_VALUE = 1234567890LL;
    constexpr uint32_t K_UINT_VALUE_MAX = 4294967295U;
    constexpr size_t K_MAX_ARRAY_LENGTH = 1024;
    constexpr int32_t K_ITERATION_LIMIT = 100;
    constexpr size_t K_STRING_BUFFER_SIZE = 512;
    constexpr int32_t K_ERROR_CODE_BASE = 1000;
    constexpr int32_t K_ERROR_INVALID_ARG = 1001;
    constexpr int32_t K_ERROR_NULL_PTR = 1002;
    constexpr int32_t K_ERROR_TYPE_MISMATCH = 1003;
    constexpr int32_t K_ERROR_OUT_OF_RANGE = 1004;
    constexpr int32_t K_SUCCESS_CODE = 0;
    constexpr int32_t K_PROP_COUNT_MIN = 1;
    constexpr int32_t K_PROP_COUNT_MAX = 10;
    constexpr size_t K_MAX_KEY_LENGTH = 64;
    constexpr size_t K_MAX_DESCRIPTOR_COUNT = 20;
    constexpr int32_t K_ATTRIBUTE_WRITABLE_BIT = 1;
    constexpr int32_t K_ATTRIBUTE_ENUMERABLE_BIT = 2;
    constexpr int32_t K_ATTRIBUTE_CONFIGURABLE_BIT = 4;
    constexpr size_t K_ENUMERATION_RETRY_COUNT = 3;
    constexpr int32_t K_DEFAULT_PROPERTY_VALUE = 0;
    constexpr size_t K_ITERATION_THRESHOLD = 50;
};

namespace PropertyNamesArgCount {
    constexpr size_t ONE = 1;
    constexpr size_t TWO = 2;
    constexpr size_t THREE = 3;
    constexpr size_t FOUR = 4;
};

constexpr std::array<const char*, PropertyNamesConst::K_KEY_NAME_COUNT> K_KEY_NAMES = {
    "alpha",
    "beta",
    "gamma",
    "delta",
    "epsilon",
    "zeta",
};

constexpr std::array<const char*, PropertyNamesConst::K_KEY_NAME_COUNT> K_EXTRA_KEY_NAMES = {
    "extra_one",
    "extra_two",
    "extra_three",
    "extra_four",
    "extra_five",
    "extra_six",
};

struct KeySpec {
    std::string name;
    bool shouldExist;
    int32_t expectedValue;
};

struct PropertyNamesCaseSpec {
    std::string name;
    napi_key_collection_mode collectionMode;
    napi_key_filter keyFilter;
    napi_key_conversion keyConversion;
    int32_t extraKeyCount;
    std::vector<KeySpec> expectedKeys;
};

struct EnumResult {
    bool passed;
    uint32_t keyCount;
    std::vector<std::string> foundKeys;
};

struct MembershipResult {
    bool passed;
    bool hasProperty;
};

struct EnumerationTestCase {
    std::string name;
    napi_key_collection_mode mode;
    napi_key_filter filter;
    napi_key_conversion conversion;
    uint32_t expectedCount;
};

struct PropertyDescriptorSpec {
    std::string name;
    int32_t value;
    uint32_t attributes;
};

struct ValidationContext {
    napi_env env;
    napi_value object;
    std::vector<std::string> expectedKeys;
    bool strictMode;
};

struct EnumerationResultData {
    std::vector<std::string> keys;
    uint32_t totalCount;
    bool success;
};

struct MembershipCheckData {
    std::string keyName;
    bool exists;
    bool verified;
};

struct PropertyTestConfig {
    int32_t keyCount;
    int32_t valueBase;
    bool includeExtraKeys;
    int32_t extraKeyCount;
};

struct FilterTestData {
    napi_key_filter filter;
    std::string filterName;
    uint32_t expectedMinCount;
    uint32_t expectedMaxCount;
};

struct ConversionTestData {
    napi_key_conversion conversion;
    std::string conversionName;
    bool expectNumberConversion;
};

struct ArrayTestData {
    uint32_t arraySize;
    bool useNumericKeys;
    int32_t valueOffset;
};

struct MixedKeyTestData {
    uint32_t stringKeyCount;
    uint32_t numericKeyCount;
    int32_t baseValue;
};

struct PropertyAttributes {
    bool writable;
    bool enumerable;
    bool configurable;
};

struct EnumOptions {
    napi_key_collection_mode collectionMode;
    napi_key_filter keyFilter;
    napi_key_conversion keyConversion;
};

std::string BuildIndexedName(const char* prefix, size_t caseNumber);
size_t GetCaseIndex(void* data);
bool CreateInt32Value(napi_env env, int32_t value, napi_value* result);
bool CreateStringValue(napi_env env, const std::string& value, napi_value* result);
bool GetInt32Value(napi_env env, napi_value value, int32_t* result);
bool GetStringValue(napi_env env, napi_value value, std::string* result);
bool SetNamedPropertyInt32(napi_env env, napi_value object, const char* name, int32_t value);
bool SetNamedPropertyString(napi_env env, napi_value object, const char* name, const std::string& value);
bool DefinePropertyWithAttributes(napi_env env, napi_value object, const char* name, int32_t value,
    const PropertyAttributes& attributes);
napi_value CreateTestObject(napi_env env, const std::vector<std::string>& keys, const std::vector<int32_t>& values);
bool EnumeratePropertyNames(napi_env env, napi_value object, const EnumOptions& options, EnumResult* result);
bool CheckPropertyMembership(napi_env env, napi_value object, const std::string& key, MembershipResult* result);
bool CollectArrayKeys(napi_env env, napi_value array, std::vector<std::string>* keys);
napi_value CreateResultObject(napi_env env, const std::string& caseName, bool passed,
    uint32_t keyCount, const std::vector<std::string>& keys);
bool VerifyKeyPresence(napi_env env, napi_value object, const std::vector<KeySpec>& expectedKeys);
bool CreateObjectWithProperties(napi_env env, napi_value* result, const std::vector<std::string>& keys);
bool HasAllExpectedKeys(napi_env env, napi_value object, const std::vector<std::string>& expectedKeys);
bool VerifyEnumeratedKeyCount(napi_env env, napi_value array, uint32_t expectedCount, bool* result);
bool FindKeyInArray(napi_env env, napi_value array, const std::string& key, bool* found);
bool CreateEmptyObject(napi_env env, napi_value* result);
bool CreateEmptyArray(napi_env env, napi_value* result);
bool GetUndefinedValue(napi_env env, napi_value* result);
bool GetNullValue(napi_env env, napi_value* result);

#endif  // FOUNDATION_ACE_NAPI_SAMPLE_NATIVE_MODULE_PROPERTY_NAMES_SUITE_HELPER_H