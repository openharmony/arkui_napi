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

#ifndef FOUNDATION_ACE_NAPI_SAMPLE_NATIVE_MODULE_SCRIPT_SUITE_SCRIPT_HELPER_H
#define FOUNDATION_ACE_NAPI_SAMPLE_NATIVE_MODULE_SCRIPT_SUITE_SCRIPT_HELPER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include <cstdint>
#include <string>

namespace ScriptSuite {

// ---------------------------------------------------------------------------
// Named constants (no magic numbers)
// ---------------------------------------------------------------------------

// Module metadata
constexpr uint32_t K_MODULE_VERSION = 1;
constexpr uint32_t K_NO_MODULE_FLAGS = 0;

// String buffer sizes
constexpr size_t K_STRING_BUFFER_SIZE = 256;
constexpr size_t K_NULL_TERMINATOR_SIZE = 1;

// Test case counts per category
constexpr size_t K_ARITHMETIC_CASE_COUNT = 8;
constexpr size_t K_STRING_CONCAT_CASE_COUNT = 6;
constexpr size_t K_OBJECT_LITERAL_CASE_COUNT = 4;
constexpr size_t K_ARRAY_LITERAL_CASE_COUNT = 4;
constexpr size_t K_IIFE_CASE_COUNT = 4;
constexpr size_t K_TYPEOF_CASE_COUNT = 6;
constexpr size_t K_PRIMITIVE_CASE_COUNT = 8;
constexpr size_t K_SYNTAX_ERROR_CASE_COUNT = 4;
constexpr size_t K_STRICT_EQUALS_CASE_COUNT = 4;

// Total test case count
constexpr size_t K_SCRIPT_CASE_COUNT = K_ARITHMETIC_CASE_COUNT +
    K_STRING_CONCAT_CASE_COUNT +
    K_OBJECT_LITERAL_CASE_COUNT +
    K_ARRAY_LITERAL_CASE_COUNT +
    K_IIFE_CASE_COUNT +
    K_TYPEOF_CASE_COUNT +
    K_PRIMITIVE_CASE_COUNT +
    K_SYNTAX_ERROR_CASE_COUNT +
    K_STRICT_EQUALS_CASE_COUNT;

// Argument counts
constexpr size_t K_ARG_COUNT_ONE = 1;
constexpr size_t K_ARG_COUNT_ZERO = 0;

// Sub-case index selectors for category helper functions
constexpr size_t K_SUB_CASE_INDEX_ONE = 1;
constexpr size_t K_SUB_CASE_INDEX_TWO = 2;
constexpr size_t K_SUB_CASE_INDEX_THREE = 3;
constexpr size_t K_SUB_CASE_INDEX_FOUR = 4;
constexpr size_t K_SUB_CASE_INDEX_FIVE = 5;
constexpr size_t K_SUB_CASE_INDEX_SIX = 6;
constexpr size_t K_SUB_CASE_INDEX_SEVEN = 7;

// Numeric values for test cases
constexpr int32_t K_NUM_ADD_A = 1;
constexpr int32_t K_NUM_ADD_B = 2;
constexpr int32_t K_NUM_ADD_RESULT = 3;
constexpr int32_t K_NUM_SUB_A = 10;
constexpr int32_t K_NUM_SUB_B = 4;
constexpr int32_t K_NUM_SUB_RESULT = 6;
constexpr int32_t K_NUM_MUL_A = 7;
constexpr int32_t K_NUM_MUL_B = 8;
constexpr int32_t K_NUM_MUL_RESULT = 56;
constexpr int32_t K_NUM_DIV_A = 20;
constexpr int32_t K_NUM_DIV_B = 4;
constexpr int32_t K_NUM_DIV_RESULT = 5;
constexpr int32_t K_NUM_MOD_A = 17;
constexpr int32_t K_NUM_MOD_B = 5;
constexpr int32_t K_NUM_MOD_RESULT = 2;
constexpr int32_t K_NUM_POW_BASE = 2;
constexpr int32_t K_NUM_POW_EXP = 6;
constexpr int32_t K_NUM_POW_RESULT = 64;
constexpr int32_t K_NUM_NEG_A = -5;
constexpr int32_t K_NUM_NEG_B = 3;
constexpr int32_t K_NUM_NEG_RESULT = -8;

// String values for test cases
constexpr const char* K_STR_HELLO = "hello";
constexpr const char* K_STR_WORLD = "world";
constexpr const char* K_STR_SPACE = " ";
constexpr const char* K_STR_FOO = "foo";
constexpr const char* K_STR_BAR = "bar";
constexpr const char* K_STR_BAZ = "baz";
constexpr const char* K_STR_RESULT_HELLO_WORLD = "hello world";
constexpr const char* K_STR_RESULT_FOOBAR = "foobar";

// Object property names
constexpr const char* K_PROP_NAME_A = "a";
constexpr const char* K_PROP_NAME_B = "b";
constexpr const char* K_PROP_NAME_X = "x";
constexpr const char* K_PROP_NAME_Y = "y";

// Object property values
constexpr int32_t K_OBJ_VAL_A = 1;
constexpr int32_t K_OBJ_VAL_B = 2;
constexpr int32_t K_OBJ_VAL_X = 10;
constexpr int32_t K_OBJ_VAL_Y = 20;

// Array element values
constexpr int32_t K_ARR_ELEM_1 = 1;
constexpr int32_t K_ARR_ELEM_2 = 2;
constexpr int32_t K_ARR_ELEM_3 = 3;
constexpr size_t K_ARR_LENGTH = 3;
constexpr size_t K_ARR_LEN_SINGLE = 1;
constexpr size_t K_ARR_LEN_TWO = 2;

// IIFE return values
constexpr int32_t K_IIFE_VAL_42 = 42;
constexpr int32_t K_IIFE_VAL_100 = 100;
constexpr double K_IIFE_VAL_PI = 3.14159;

// Expected result types (napi_valuetype values)
constexpr napi_valuetype K_EXPECTED_NUMBER = napi_number;
constexpr napi_valuetype K_EXPECTED_STRING = napi_string;
constexpr napi_valuetype K_EXPECTED_BOOLEAN = napi_boolean;
constexpr napi_valuetype K_EXPECTED_OBJECT = napi_object;
constexpr napi_valuetype K_EXPECTED_UNDEFINED = napi_undefined;
constexpr napi_valuetype K_EXPECTED_NULL = napi_null;

// ---------------------------------------------------------------------------
// Script test case specification
// ---------------------------------------------------------------------------

enum class ScriptTestCategory {
    ARITHMETIC,
    STRING_CONCAT,
    OBJECT_LITERAL,
    ARRAY_LITERAL,
    IIFE,
    TYPEOF,
    PRIMITIVE,
    SYNTAX_ERROR,
    STRICT_EQUALS
};

struct ScriptCaseSpec {
    std::string exportName;
    std::string scriptSource;
    napi_valuetype expectedType;
    ScriptTestCategory category;
    bool expectException;
};

// ---------------------------------------------------------------------------
// Result object helpers
// ---------------------------------------------------------------------------

bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value);
bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value);
bool SetNamedDouble(napi_env env, napi_value object, const char* name, double value);
bool SetNamedString(napi_env env, napi_value object, const char* name, const std::string& value);

napi_value CreateResultObject(napi_env env);

// ---------------------------------------------------------------------------
// Exception handling helpers
// ---------------------------------------------------------------------------

bool IsExceptionPending(napi_env env);
napi_value ClearPendingException(napi_env env);

// ---------------------------------------------------------------------------
// Script execution helpers
// ---------------------------------------------------------------------------

bool CreateScriptString(napi_env env, const std::string& source, napi_value* result);
bool RunScriptSource(napi_env env, const std::string& source, napi_value* result);

// ---------------------------------------------------------------------------
// Value type checking helpers
// ---------------------------------------------------------------------------

bool CheckValueType(napi_env env, napi_value value, napi_valuetype expected);
napi_valuetype GetValueType(napi_env env, napi_value value);

// ---------------------------------------------------------------------------
// Value extraction helpers
// ---------------------------------------------------------------------------

bool GetInt32Value(napi_env env, napi_value value, int32_t* result);
bool GetDoubleValue(napi_env env, napi_value value, double* result);
bool GetBoolValue(napi_env env, napi_value value, bool* result);
bool GetStringValue(napi_env env, napi_value value, std::string& result);

// ---------------------------------------------------------------------------
// Value comparison helpers
// ---------------------------------------------------------------------------

bool StrictEquals(napi_env env, napi_value lhs, napi_value rhs);

// ---------------------------------------------------------------------------
// Test case generation
// ---------------------------------------------------------------------------

ScriptCaseSpec GetScriptCaseSpec(size_t caseIndex);
std::string BuildExportName(size_t caseIndex);

} // namespace ScriptSuite

#endif // FOUNDATION_ACE_NAPI_SAMPLE_NATIVE_MODULE_SCRIPT_SUITE_SCRIPT_HELPER_H