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

#ifndef FOUNDATION_ACE_NAPI_SAMPLE_NATIVE_MODULE_FROZEN_FROZEN_HELPER_H
#define FOUNDATION_ACE_NAPI_SAMPLE_NATIVE_MODULE_FROZEN_FROZEN_HELPER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include <cstdint>
#include <string>

namespace FrozenConst {
    constexpr size_t K_FROZEN_CASE_COUNT = 20;
    constexpr size_t K_OPERATION_COUNT = 5;
    constexpr size_t K_CASES_PER_OPERATION = 4;
    constexpr size_t K_PROPERTY_COUNT = 3;
    constexpr size_t K_KEY_NAMES_COUNT = 5;
    constexpr size_t K_FIRST_CASE_NUMBER = 1;
    constexpr int K_CASE_NUMBER_WIDTH = 2;
    constexpr int K_ARG_COUNT_TWO = 2;
    constexpr int K_ARG_COUNT_ONE = 1;
    constexpr int K_ARG_COUNT_THREE = 3;
    constexpr int K_ARG_COUNT_FOUR = 4;
    constexpr uint32_t K_MODULE_VERSION = 1;
    constexpr uint32_t K_NO_MODULE_FLAGS = 0;
    constexpr size_t K_NULL_TERMINATOR_SIZE = 1;
    constexpr int32_t K_INVALID_VALUE = -1;
    constexpr int32_t K_INITIAL_VALUE_BASE = 10;
    constexpr int32_t K_INITIAL_VALUE_STEP = 5;
    constexpr int32_t K_MUTATION_OFFSET = 100;
    constexpr int32_t K_ATTR_BIT_SHIFT = 2;
    constexpr int32_t K_VALUE_CYCLE = 7;
    constexpr int32_t K_STRING_LEN_SHORT = 16;
    constexpr int32_t K_STRING_LEN_LONG = 32;
    constexpr int32_t K_DOUBLE_SCALE = 2;
    constexpr int32_t K_UINT32_BASE = 1000;
    constexpr int64_t K_INT64_BASE = 10000LL;
    constexpr double K_DOUBLE_PI = 3.141592653589793;
    constexpr double K_DOUBLE_E = 2.718281828459045;
    constexpr int32_t K_SUCCESS_CODE = 0;
    constexpr int32_t K_FAILURE_CODE = -1;
    constexpr int32_t K_EXCEPTION_CODE = -2;
    constexpr size_t K_DESCRIPTOR_COUNT = 3;
    constexpr int32_t K_BOOL_TRUE_VALUE = 1;
    constexpr int32_t K_BOOL_FALSE_VALUE = 0;
    constexpr int32_t K_MAX_ATTR_PATTERN = 8;
    constexpr int32_t K_MIN_ATTR_PATTERN = 0;
    constexpr int32_t K_VALUE_MULTIPLIER = 10;
    constexpr size_t K_ARRAY_INIT_SIZE = 0;
    constexpr size_t K_MAX_PROPERTY_KEYS = 10;
    constexpr int32_t K_DESCRIPTOR_HAS_VALUE = 1;
    constexpr int32_t K_DESCRIPTOR_NO_VALUE = 0;
    constexpr int32_t K_TEST_ARRAY_LENGTH = 5;
    constexpr int32_t K_TEST_ELEMENT_VALUE = 42;
    constexpr int32_t K_NEW_ELEMENT_VALUE = 99;
    constexpr size_t K_FIRST_ELEMENT_INDEX = 0;
    constexpr size_t K_SECOND_ELEMENT_INDEX = 1;
    constexpr int32_t K_MIN_VALID_ATTR_PATTERN = 0;
    constexpr int32_t K_MAX_VALID_ATTR_PATTERN = 7;
    constexpr double K_TEST_DOUBLE_VALUE = 123.456;
    constexpr uint32_t K_TEST_UINT_VALUE = 1000U;
    constexpr int64_t K_TEST_INT64_VALUE = 1000000LL;
    constexpr bool K_DEFAULT_WRITABLE = true;
    constexpr bool K_DEFAULT_ENUMERABLE = true;
    constexpr bool K_DEFAULT_CONFIGURABLE = true;
    constexpr bool K_FROZEN_WRITABLE = false;
    constexpr bool K_FROZEN_CONFIGURABLE = false;
    constexpr bool K_SEALED_CONFIGURABLE = false;
    constexpr int32_t K_PROPERTY_NAME_MAX_LEN = 32;
    constexpr size_t K_MAX_TEST_ITERATIONS = 100;
    constexpr int32_t K_ZERO_VALUE = 0;
    constexpr int32_t K_ONE_VALUE = 1;
    constexpr int32_t K_NEGATIVE_ONE = -1;
    constexpr double K_ZERO_DOUBLE = 0.0;
    constexpr double K_ONE_DOUBLE = 1.0;
    constexpr int32_t K_TEST_INT_VALUE_42 = 42;
    constexpr int32_t K_TEST_INT_VALUE_100 = 100;
    constexpr int32_t K_TEST_INT_VALUE_999 = 999;
    constexpr double K_TEST_DOUBLE_PI_APPROX = 3.14159;
    constexpr int32_t K_BIT_MODULUS = 2;
    constexpr int32_t K_TWO_BIT_SHIFT = 4;
    constexpr int32_t K_EXTRA_EXPORT_COUNT = 17;
    constexpr int64_t K_TEST_INT64_VALUE_LARGE = 9999999999LL;
}

namespace FrozenOp {
    constexpr int32_t BUILD_AND_FREEZE = 0;
    constexpr int32_t BUILD_AND_SEAL = 1;
    constexpr int32_t MUTATE_FROZEN = 2;
    constexpr int32_t MUTATE_SEALED = 3;
    constexpr int32_t DESCRIPTOR_CHECK = 4;
}

struct PropertyMeta {
    std::string name;
    bool writable;
    bool enumerable;
    bool configurable;
};

struct FrozenCaseSpec {
    std::string name;
    int32_t operation;
    int32_t attrPattern;
    int32_t initialValue;
};

struct DescriptorInfo {
    bool hasDescriptor;
    bool writable;
    bool enumerable;
    bool configurable;
    int32_t value;
};

struct MutationResult {
    bool success;
    int32_t finalValue;
    bool exceptionRaised;
};

std::string BuildIndexedName(const char* prefix, size_t caseNumber);

size_t GetCaseIndex(void* data);

FrozenCaseSpec GetFrozenCaseSpec(size_t caseIndex);

PropertyMeta GetPropertyMeta(size_t propIndex, int32_t attrPattern);

uint32_t BuildPropertyAttribute(const PropertyMeta& meta);

bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value);

bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value);

bool SetNamedString(napi_env env, napi_value object, const char* name, const std::string& value);

bool SetNamedDouble(napi_env env, napi_value object, const char* name, double value);

bool SetNamedUint32(napi_env env, napi_value object, const char* name, uint32_t value);

bool SetNamedInt64(napi_env env, napi_value object, const char* name, int64_t value);

bool GetNamedInt32(napi_env env, napi_value object, const char* name, int32_t* value);

bool GetNamedBool(napi_env env, napi_value object, const char* name, bool* value);

bool ReadInt32Arg(napi_env env, napi_value arg, int32_t* value);

bool ReadStringArg(napi_env env, napi_value arg, std::string* value);

napi_value CreateTestObjectWithProperties(
    napi_env env, int32_t attrPattern, int32_t baseValue, int32_t* propValues);

napi_value GetDescriptorInfo(napi_env env, napi_value obj, const char* key, DescriptorInfo* info);

napi_value CreateResultObject(
    napi_env env, const std::string& caseName, const char* operation, bool passed, int32_t value);

napi_value CreateDetailedResultObject(
    napi_env env, const FrozenCaseSpec& spec, bool passed, const int32_t* propValues,
    const DescriptorInfo* descriptors);

MutationResult TryMutateProperty(napi_env env, napi_value obj, const char* key, int32_t newValue);

MutationResult TryDeleteProperty(napi_env env, napi_value obj, const char* key);

bool FreezeObject(napi_env env, napi_value obj);

bool SealObject(napi_env env, napi_value obj);

bool IsObjectFrozen(napi_env env, napi_value obj);

bool IsObjectSealed(napi_env env, napi_value obj);

#endif  // FOUNDATION_ACE_NAPI_SAMPLE_NATIVE_MODULE_FROZEN_FROZEN_HELPER_H