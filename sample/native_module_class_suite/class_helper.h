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

#ifndef FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_CLASS_CLASS_HELPER_H
#define FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_CLASS_CLASS_HELPER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include <cstdint>
#include <string>

// ---------------------------------------------------------------------------
// Constants - Module Configuration
// ---------------------------------------------------------------------------
constexpr uint32_t K_MODULE_VERSION = 1;
constexpr uint32_t K_NO_MODULE_FLAGS = 0;

// ---------------------------------------------------------------------------
// Constants - Argument Counts
// ---------------------------------------------------------------------------
constexpr size_t K_ARGS_ZERO = 0;
constexpr size_t K_ARGS_ONE = 1;
constexpr size_t K_ARGS_TWO = 2;
constexpr size_t K_ARGS_THREE = 3;
constexpr size_t K_ARGS_FOUR = 4;
constexpr size_t K_ARGS_FIVE = 5;
constexpr size_t K_ARGS_SIX = 6;

// ---------------------------------------------------------------------------
// Constants - Argument Indices
// ---------------------------------------------------------------------------
constexpr size_t K_ARG_INDEX_FIRST = 0;
constexpr size_t K_ARG_INDEX_SECOND = 1;
constexpr size_t K_ARG_INDEX_THIRD = 2;

// ---------------------------------------------------------------------------
// Constants - Property Counts
// ---------------------------------------------------------------------------
constexpr size_t K_PROP_COUNT_ZERO = 0;
constexpr size_t K_PROP_COUNT_ONE = 1;
constexpr size_t K_PROP_COUNT_TWO = 2;
constexpr size_t K_PROP_COUNT_THREE = 3;
constexpr size_t K_PROP_COUNT_FOUR = 4;
constexpr size_t K_PROP_COUNT_FIVE = 5;
constexpr size_t K_PROP_COUNT_SIX = 6;
constexpr size_t K_PROP_COUNT_SEVEN = 7;
constexpr size_t K_PROP_COUNT_EIGHT = 8;

// ---------------------------------------------------------------------------
// Constants - Test Values (Integers)
// ---------------------------------------------------------------------------
constexpr int32_t K_INT_VALUE_ZERO = 0;
constexpr int32_t K_INT_VALUE_ONE = 1;
constexpr int32_t K_INT_VALUE_TWO = 2;
constexpr int32_t K_INT_VALUE_THREE = 3;
constexpr int32_t K_INT_VALUE_FOUR = 4;
constexpr int32_t K_INT_VALUE_FIVE = 5;
constexpr int32_t K_INT_VALUE_TEN = 10;
constexpr int32_t K_INT_VALUE_TWENTY = 20;
constexpr int32_t K_INT_VALUE_FORTY_TWO = 42;
constexpr int32_t K_INT_VALUE_ONE_HUNDRED = 100;
constexpr int32_t K_INT_VALUE_TWO_HUNDRED = 200;
constexpr int32_t K_INT_VALUE_THREE_HUNDRED = 300;
constexpr int32_t K_INT_VALUE_NEG_TEN = -10;
constexpr int32_t K_INT_VALUE_NEG_FIFTY = -50;

// ---------------------------------------------------------------------------
// Constants - Test Values (Doubles)
// ---------------------------------------------------------------------------
constexpr double K_DOUBLE_ZERO = 0.0;
constexpr double K_DOUBLE_ONE = 1.0;
constexpr double K_DOUBLE_TWO = 2.0;
constexpr double K_DOUBLE_THREE = 3.0;
constexpr double K_DOUBLE_PI = 3.14159265358979;
constexpr double K_DOUBLE_E = 2.71828182845904;
constexpr double K_DOUBLE_NEG_ONE = -1.0;
constexpr double K_DOUBLE_LARGE = 123456.789;

// ---------------------------------------------------------------------------
// Constants - Test Values (Int64)
// ---------------------------------------------------------------------------
constexpr int64_t K_INT64_ZERO = 0LL;
constexpr int64_t K_INT64_ONE = 1LL;
constexpr int64_t K_INT64_LARGE = 9876543210LL;
constexpr int64_t K_INT64_NEG_LARGE = -9876543210LL;

// ---------------------------------------------------------------------------
// Constants - Test Values (Uint32)
// ---------------------------------------------------------------------------
constexpr uint32_t K_UINT32_ZERO = 0U;
constexpr uint32_t K_UINT32_ONE = 1U;
constexpr uint32_t K_UINT32_LARGE = 4294967290U;

// ---------------------------------------------------------------------------
// Constants - Test Strings
// ---------------------------------------------------------------------------
constexpr size_t K_STRING_SHORT_LEN = 5;
constexpr size_t K_STRING_MEDIUM_LEN = 20;
constexpr size_t K_STRING_LONG_LEN = 100;

// ---------------------------------------------------------------------------
// Constants - Array and Loop
// ---------------------------------------------------------------------------
constexpr size_t K_ARRAY_SIZE_FOUR = 4;
constexpr size_t K_ITER_COUNT_THREE = 3;
constexpr size_t K_ITER_COUNT_FIVE = 5;

// ---------------------------------------------------------------------------
// Constants - Instance Data
// ---------------------------------------------------------------------------
constexpr int32_t K_INSTANCE_ID_BASE = 1000;
constexpr int32_t K_INSTANCE_ID_STEP = 7;
constexpr double K_INSTANCE_FACTOR = 1.5;

// ---------------------------------------------------------------------------
// Constants - Method Result Codes
// ---------------------------------------------------------------------------
constexpr int32_t K_RESULT_SUCCESS = 0;
constexpr int32_t K_RESULT_ERROR = -1;

// ---------------------------------------------------------------------------
// Constants - Test Function Count
// ---------------------------------------------------------------------------
constexpr size_t K_TEST_FUNCTION_COUNT = 20;

// ---------------------------------------------------------------------------
// Constants - Case Index
// ---------------------------------------------------------------------------
constexpr size_t K_FIRST_CASE_NUMBER = 1;
constexpr int K_CASE_NUMBER_WIDTH = 2;

// ---------------------------------------------------------------------------
// Spec Structures - Instance Data (stored via instance properties, NOT napi_wrap)
// ---------------------------------------------------------------------------
struct InstanceData {
    int32_t id;
    int32_t value;
    double factor;
};

struct VectorData {
    double x;
    double y;
    double z;
};

struct EntityData {
    std::string name;
    int32_t health;
    int32_t level;
};

struct ConfigData {
    bool enabled;
    int32_t timeout;
    double threshold;
};

// ---------------------------------------------------------------------------
// Spec Structures - Test Case Specification
// ---------------------------------------------------------------------------
struct ClassCaseSpec {
    std::string className;
    int32_t expectedId;
    double expectedFactor;
    bool hasMethods;
};

struct InstanceCaseSpec {
    std::string methodName;
    int32_t inputValue;
    int32_t expectedOutput;
};

struct ArgCaseSpec {
    size_t argCount;
    int32_t argType; // 0=int32, 1=int64, 2=double, 3=string, 4=boolean
    std::string description;
};

// ---------------------------------------------------------------------------
// Helper Functions - Value Creation
// ---------------------------------------------------------------------------
bool CreateInt32Value(napi_env env, int32_t value, napi_value* result);
bool CreateInt64Value(napi_env env, int64_t value, napi_value* result);
bool CreateUInt32Value(napi_env env, uint32_t value, napi_value* result);
bool CreateDoubleValue(napi_env env, double value, napi_value* result);
bool CreateStringValue(napi_env env, const std::string& value, napi_value* result);
bool CreateBoolValue(napi_env env, bool value, napi_value* result);

// ---------------------------------------------------------------------------
// Helper Functions - Value Extraction
// ---------------------------------------------------------------------------
bool GetInt32Value(napi_env env, napi_value value, int32_t* result);
bool GetInt64Value(napi_env env, napi_value value, int64_t* result);
bool GetUInt32Value(napi_env env, napi_value value, uint32_t* result);
bool GetDoubleValue(napi_env env, napi_value value, double* result);
bool GetStringValue(napi_env env, napi_value value, std::string& result);
bool GetBoolValue(napi_env env, napi_value value, bool* result);

// ---------------------------------------------------------------------------
// Helper Functions - Object Property Operations
// ---------------------------------------------------------------------------
bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value);
bool SetNamedInt64(napi_env env, napi_value object, const char* name, int64_t value);
bool SetNamedDouble(napi_env env, napi_value object, const char* name, double value);
bool SetNamedString(napi_env env, napi_value object, const char* name, const std::string& value);
bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value);

bool GetNamedInt32(napi_env env, napi_value object, const char* name, int32_t* value);
bool GetNamedDouble(napi_env env, napi_value object, const char* name, double* value);
bool GetNamedBool(napi_env env, napi_value object, const char* name, bool* value);

// ---------------------------------------------------------------------------
// Helper Functions - Instance Data Storage (via properties)
// ---------------------------------------------------------------------------
bool StoreInstanceData(napi_env env, napi_value instance, const InstanceData& data);
bool LoadInstanceData(napi_env env, napi_value instance, InstanceData& data);

bool StoreVectorData(napi_env env, napi_value instance, const VectorData& data);
bool LoadVectorData(napi_env env, napi_value instance, VectorData& data);

// ---------------------------------------------------------------------------
// Helper Functions - Name Building
// ---------------------------------------------------------------------------
std::string BuildIndexedName(const char* prefix, size_t caseNumber);

// ---------------------------------------------------------------------------
// Helper Functions - Case Index Conversion
// ---------------------------------------------------------------------------
size_t GetCaseIndex(void* data);
void* MakeCaseData(size_t caseIndex);

// ---------------------------------------------------------------------------
// Helper Functions - Case Specifications
// ---------------------------------------------------------------------------
ClassCaseSpec GetClassCaseSpec(size_t caseIndex);
InstanceCaseSpec GetInstanceCaseSpec(size_t caseIndex);
ArgCaseSpec GetArgCaseSpec(size_t caseIndex);

// ---------------------------------------------------------------------------
// Helper Functions - Result Object Creation
// ---------------------------------------------------------------------------
napi_value CreateResultObject(napi_env env);
bool SetResultSuccess(napi_env env, napi_value result, const char* testName);
bool SetResultFailure(napi_env env, napi_value result, const char* testName, const char* reason);

#endif  // FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_CLASS_CLASS_HELPER_H