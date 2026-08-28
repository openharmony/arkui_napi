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

#ifndef FOUNDATION_ACE_NAPI_SAMPLE_NATIVE_MODULE_PROTOTYPE_SUITE_PROTOTYPE_HELPER_H
#define FOUNDATION_ACE_NAPI_SAMPLE_NATIVE_MODULE_PROTOTYPE_SUITE_PROTOTYPE_HELPER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include <cstdint>
#include <string>
#include <vector>

namespace PrototypeConst {
    constexpr size_t K_PROTOTYPE_CHAIN_MAX_DEPTH = 8;
    constexpr size_t K_CASE_COUNT = 20;
    constexpr size_t K_FIRST_CASE_NUMBER = 1;
    constexpr int K_CASE_NUMBER_WIDTH = 2;
    constexpr int32_t K_INSTANCE_VALUE_BASE = 100;
    constexpr int32_t K_INSTANCE_VALUE_STEP = 7;
    constexpr int32_t K_INSTANCE_VALUE_CYCLE = 13;
    constexpr int32_t K_CHECKSUM_CYCLE = 7;
    constexpr int32_t K_PROPERTY_COUNT = 3;
    constexpr int32_t K_ARRAY_SIZE = 5;
    constexpr uint32_t K_MODULE_VERSION = 1;
    constexpr uint32_t K_NO_MODULE_FLAGS = 0;
    constexpr size_t K_FIRST_ARG_INDEX = 0;
    constexpr size_t K_SECOND_ARG_INDEX = 1;
    constexpr size_t K_ARG_COUNT_TWO = 2;
    constexpr size_t K_ARG_COUNT_ONE = 1;
    constexpr size_t K_ARG_COUNT_THREE = 3;
    constexpr size_t K_ARG_COUNT_FOUR = 4;
    constexpr int32_t K_DEFAULT_INSTANCE_VALUE = 42;
    constexpr int32_t K_ALT_INSTANCE_VALUE = 99;
    constexpr int32_t K_INSTANCE_VALUE_OFFSET = 10;
    constexpr int32_t K_CHECKSUM_MULTIPLIER = 17;
    constexpr int32_t K_ARRAY_ELEMENT_BASE = 10;
    constexpr int32_t K_ARRAY_ELEMENT_STEP = 10;
    constexpr int32_t K_TYPEOF_UNKNOWN = 0;
    constexpr int32_t K_TYPEOF_NUMBER = 1;
    constexpr int32_t K_TYPEOF_STRING = 2;
    constexpr int32_t K_TYPEOF_BOOLEAN = 3;
    constexpr int32_t K_TYPEOF_OBJECT = 4;
    constexpr int32_t K_TYPEOF_FUNCTION = 5;
    constexpr int32_t K_TYPEOF_UNDEFINED = 6;
    constexpr int32_t K_TYPEOF_NULL = 7;
    constexpr size_t K_MAX_TYPE_CHAIN_LENGTH = 16;
    constexpr int32_t K_TEST_CLASS_COUNT = 4;
    constexpr int32_t K_PROTO_DEPTH_MIN = 3;
    constexpr int32_t K_PROTO_DEPTH_MAX = 6;
    constexpr int32_t K_WEIGHT_BASE = 100;
    constexpr int32_t K_WEIGHT_STEP = 50;
    constexpr int32_t K_RESULT_KEY_COUNT = 8;
    constexpr int32_t K_CHAIN_TYPE_COUNT = 5;
    constexpr int32_t K_CHECKSUM_BASE = 1000;
    constexpr int32_t K_INSTANCE_COUNT_MIN = 1;
    constexpr int32_t K_INSTANCE_COUNT_MAX = 10;
    constexpr size_t K_MAX_EXPORT_COUNT = 32;
    constexpr int32_t K_DEPTH_INCREMENT = 1;
    constexpr int32_t K_VALUE_MULTIPLIER = 3;
    constexpr int32_t K_CHECKSUM_OFFSET = 5;
    constexpr int32_t K_CHAIN_LENGTH_BASE = 1;
    constexpr int32_t K_TYPE_MASK = 0xF;
    constexpr int32_t K_TYPE_CODE_BASE = 10;
    constexpr int32_t K_CHAIN_STEP = 1;
    constexpr int32_t K_PROTO_CHECK_BASE = 0;
    constexpr int32_t K_INSTANCE_ID_BASE = 1000;
    constexpr int32_t K_CHECKSUM_FACTOR = 7;
    constexpr int32_t K_DEPTH_FACTOR = 3;
    constexpr int32_t K_VALUE_OFFSET = 50;
    constexpr int32_t K_TYPE_OFFSET = 1;
    constexpr int32_t K_RESULT_COUNT_BASE = 1;
    constexpr int32_t K_PROPERTY_INDEX_BASE = 0;
    constexpr int32_t K_ARRAY_INDEX_BASE = 0;
    constexpr int32_t K_ELEMENT_COUNT_BASE = 1;
    constexpr int32_t K_ITERATION_BASE = 1;
    constexpr int32_t K_ACCUMULATOR_BASE = 0;
    constexpr int32_t K_ALT_INSTANCE_VALUE_TWO = 50;
    constexpr int32_t K_PLAIN_OBJECT_VALUE = 77;
    constexpr int32_t K_DEPTH_LIMIT_CYCLE = 4;
    constexpr int32_t K_DEPTH_LIMIT_OFFSET = 3;
    constexpr int32_t K_CHAIN_WALKING_CYCLE = 5;
    constexpr int32_t K_INSTANCEOF_CYCLE = 3;
    constexpr int32_t K_INSTANCEOF_OFFSET = 1;
    constexpr int32_t K_TYPEOF_CYCLE = 7;
    constexpr int32_t K_TYPEOF_OFFSET = 2;
    constexpr int32_t K_STRICT_EQUALS_CYCLE = 11;
}

enum PrototypeArgIndex {
    PROTO_FIRST_ARG = 0,
    PROTO_SECOND_ARG,
    PROTO_THIRD_ARG,
    PROTO_FOURTH_ARG,
};

enum PrototypeTestType {
    PROTO_TEST_PLAIN_OBJECT = 0,
    PROTO_TEST_ARRAY,
    PROTO_TEST_FUNCTION,
    PROTO_TEST_INSTANCE,
    PROTO_TEST_CHAIN_WALKING,
    PROTO_TEST_INSTANCEOF_POSITIVE,
    PROTO_TEST_INSTANCEOF_NEGATIVE_UNRELATED,
    PROTO_TEST_INSTANCEOF_NEGATIVE_PLAIN,
    PROTO_TEST_SAME_CONSTRUCTOR,
    PROTO_TEST_FUNCTION_PROTO_PROP,
    PROTO_TEST_TYPEOF_CHAIN,
    PROTO_TEST_STRICT_EQUALS,
    PROTO_TEST_GENERIC_CASE,
};

struct PrototypeCaseSpec {
    std::string name;
    int32_t valueFactor;
    int32_t depthLimit;
    int32_t testType;
    bool testChainWalking;
    bool testInstanceof;
    bool testTypeof;
    bool testStrictEquals;
};

struct PrototypeChainResult {
    int32_t depth;
    int32_t chainChecksum;
    bool reachedNull;
    bool exceededDepth;
    bool walkSuccessful;
    std::vector<int32_t> typeofCodes;
    std::vector<std::string> typeLabels;
};

struct InstanceofResult {
    bool positiveMatch;
    bool negativeMatchUnrelated;
    bool negativeMatchPlain;
    int32_t matchCount;
};

struct PrototypeCompareResult {
    bool prototypesEqual;
    bool sameConstructor;
    int32_t instanceIdA;
    int32_t instanceIdB;
};

struct TypeChainInfo {
    int32_t length;
    int32_t checksum;
    bool hasObject;
    bool hasFunction;
    bool hasNull;
};

struct TestClassInfo {
    std::string className;
    int32_t instanceCount;
    bool hasConstructor;
};

struct PrototypeTestContext {
    int32_t caseIndex;
    int32_t iteration;
    int32_t accumulator;
    bool success;
};

std::string BuildIndexedName(const char* prefix, size_t caseNumber);

size_t GetCaseIndex(void* data);

int32_t GetTypeCode(napi_valuetype type);

PrototypeCaseSpec GetPrototypeCaseSpec(size_t caseIndex);

std::string BuildPrototypeExportName(size_t caseIndex);

bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value);

bool SetNamedInt64(napi_env env, napi_value object, const char* name, int64_t value);

bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value);

bool SetNamedString(napi_env env, napi_value object, const char* name, const std::string& value);

bool FetchValueType(napi_env env, napi_value value, napi_valuetype* type);

const char* ValuetypeToLabel(napi_valuetype type);

bool CheckStrictEquality(napi_env env, napi_value a, napi_value b, bool* result);

bool GetPrototypeSafely(napi_env env, napi_value object, napi_value* prototype);

bool WalkPrototypeChain(napi_env env, napi_value start, PrototypeChainResult* result);

bool CheckInstanceof(napi_env env, napi_value object, napi_value constructor, bool* result);

bool CreateTestClass(napi_env env, const char* className, napi_value* constructor);

bool CreateTestInstance(napi_env env, napi_value constructor, int32_t value, napi_value* instance);

bool GetFunctionPrototypeProperty(napi_env env, napi_value func, napi_value* prototypeProp);

bool ComparePrototypeOfInstances(
    napi_env env, napi_value constructor, napi_value instanceA, napi_value instanceB, bool* equal);

#endif  // FOUNDATION_ACE_NAPI_SAMPLE_NATIVE_MODULE_PROTOTYPE_SUITE_PROTOTYPE_HELPER_H
