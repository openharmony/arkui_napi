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

#include "script_helper.h"

#include <sstream>

namespace ScriptSuite {

// ---------------------------------------------------------------------------
// Result object helpers
// ---------------------------------------------------------------------------

bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value)
{
    napi_value napiValue = nullptr;
    if (napi_get_boolean(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int32(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedDouble(napi_env env, napi_value object, const char* name, double value)
{
    napi_value napiValue = nullptr;
    if (napi_create_double(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedString(napi_env env, napi_value object, const char* name, const std::string& value)
{
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value.c_str(), value.size(), &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

napi_value CreateResultObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status = napi_create_object(env, &result);
    if (status != napi_ok) {
        return nullptr;
    }
    return result;
}

// ---------------------------------------------------------------------------
// Exception handling helpers
// ---------------------------------------------------------------------------

bool IsExceptionPending(napi_env env)
{
    bool isPending = false;
    napi_status status = napi_is_exception_pending(env, &isPending);
    return (status == napi_ok) && isPending;
}

napi_value ClearPendingException(napi_env env)
{
    napi_value exception = nullptr;
    napi_get_and_clear_last_exception(env, &exception);
    return exception;
}

// ---------------------------------------------------------------------------
// Script execution helpers
// ---------------------------------------------------------------------------

bool CreateScriptString(napi_env env, const std::string& source, napi_value* result)
{
    napi_status status = napi_create_string_utf8(env, source.c_str(), source.size(), result);
    return status == napi_ok;
}

bool RunScriptSource(napi_env env, const std::string& source, napi_value* result)
{
    napi_value script = nullptr;
    if (!CreateScriptString(env, source, &script)) {
        return false;
    }
    napi_status status = napi_run_script(env, script, result);
    return status == napi_ok;
}

// ---------------------------------------------------------------------------
// Value type checking helpers
// ---------------------------------------------------------------------------

bool CheckValueType(napi_env env, napi_value value, napi_valuetype expected)
{
    napi_valuetype actualType = GetValueType(env, value);
    return actualType == expected;
}

napi_valuetype GetValueType(napi_env env, napi_value value)
{
    napi_valuetype type = napi_undefined;
    napi_typeof(env, value, &type);
    return type;
}

// ---------------------------------------------------------------------------
// Value extraction helpers
// ---------------------------------------------------------------------------

bool GetInt32Value(napi_env env, napi_value value, int32_t* result)
{
    napi_status status = napi_get_value_int32(env, value, result);
    return status == napi_ok;
}

bool GetDoubleValue(napi_env env, napi_value value, double* result)
{
    napi_status status = napi_get_value_double(env, value, result);
    return status == napi_ok;
}

bool GetBoolValue(napi_env env, napi_value value, bool* result)
{
    napi_status status = napi_get_value_bool(env, value, result);
    return status == napi_ok;
}

bool GetStringValue(napi_env env, napi_value value, std::string& result)
{
    size_t length = 0;
    napi_status status = napi_get_value_string_utf8(env, value, nullptr, K_ARG_COUNT_ZERO, &length);
    if (status != napi_ok) {
        return false;
    }
    result.resize(length);
    status = napi_get_value_string_utf8(env, value, &result[0], length + K_NULL_TERMINATOR_SIZE, &length);
    return status == napi_ok;
}

// ---------------------------------------------------------------------------
// Value comparison helpers
// ---------------------------------------------------------------------------

bool StrictEquals(napi_env env, napi_value lhs, napi_value rhs)
{
    bool result = false;
    napi_status status = napi_strict_equals(env, lhs, rhs, &result);
    return (status == napi_ok) && result;
}

// ---------------------------------------------------------------------------
// Test case generation - arithmetic expressions
// ---------------------------------------------------------------------------

static std::string BuildArithmeticScript(size_t caseIndex)
{
    switch (caseIndex) {
        case 0: // Addition
            return std::to_string(K_NUM_ADD_A) + " + " + std::to_string(K_NUM_ADD_B);
        case K_SUB_CASE_INDEX_ONE: // Subtraction
            return std::to_string(K_NUM_SUB_A) + " - " + std::to_string(K_NUM_SUB_B);
        case K_SUB_CASE_INDEX_TWO: // Multiplication
            return std::to_string(K_NUM_MUL_A) + " * " + std::to_string(K_NUM_MUL_B);
        case K_SUB_CASE_INDEX_THREE: // Division
            return std::to_string(K_NUM_DIV_A) + " / " + std::to_string(K_NUM_DIV_B);
        case K_SUB_CASE_INDEX_FOUR: // Modulo
            return std::to_string(K_NUM_MOD_A) + " % " + std::to_string(K_NUM_MOD_B);
        case K_SUB_CASE_INDEX_FIVE: // Power (using multiplication chain)
            return std::to_string(K_NUM_POW_BASE) + " * " +
                   std::to_string(K_NUM_POW_BASE) + " * " +
                   std::to_string(K_NUM_POW_BASE) + " * " +
                   std::to_string(K_NUM_POW_BASE) + " * " +
                   std::to_string(K_NUM_POW_BASE) + " * " +
                   std::to_string(K_NUM_POW_BASE);
        case K_SUB_CASE_INDEX_SIX: // Negative addition
            return std::to_string(K_NUM_NEG_A) + " + " + std::to_string(K_NUM_NEG_B);
        case K_SUB_CASE_INDEX_SEVEN: // Compound expression
            return "(" + std::to_string(K_NUM_ADD_A) + " + " + std::to_string(K_NUM_ADD_B) + ") * " +
                   std::to_string(K_NUM_MUL_A);
        default:
            return "0";
    }
}

static int32_t GetArithmeticExpectedResult(size_t caseIndex)
{
    switch (caseIndex) {
        case 0: return K_NUM_ADD_RESULT;
        case K_SUB_CASE_INDEX_ONE: return K_NUM_SUB_RESULT;
        case K_SUB_CASE_INDEX_TWO: return K_NUM_MUL_RESULT;
        case K_SUB_CASE_INDEX_THREE: return K_NUM_DIV_RESULT;
        case K_SUB_CASE_INDEX_FOUR: return K_NUM_MOD_RESULT;
        case K_SUB_CASE_INDEX_FIVE: return K_NUM_POW_RESULT;
        case K_SUB_CASE_INDEX_SIX: return K_NUM_NEG_RESULT;
        case K_SUB_CASE_INDEX_SEVEN: return K_NUM_ADD_RESULT * K_NUM_MUL_A;
        default: return 0;
    }
}

// ---------------------------------------------------------------------------
// Test case generation - string concatenation
// ---------------------------------------------------------------------------

static std::string BuildStringConcatScript(size_t caseIndex)
{
    switch (caseIndex) {
        case 0: // Simple concatenation with space
            return std::string("'") + K_STR_HELLO + "' + '" + K_STR_SPACE + "' + '" + K_STR_WORLD + "'";
        case K_SUB_CASE_INDEX_ONE: // Direct concatenation
            return std::string("'") + K_STR_FOO + "' + '" + K_STR_BAR + "'";
        case K_SUB_CASE_INDEX_TWO: // Three-way concatenation
            return std::string("'") + K_STR_FOO + "' + '" + K_STR_BAR + "' + '" + K_STR_BAZ + "'";
        case K_SUB_CASE_INDEX_THREE: // Concatenation with empty string
            return std::string("'") + K_STR_HELLO + "' + ''";
        case K_SUB_CASE_INDEX_FOUR: // Template-style using concatenation
            return std::string("'") + K_STR_FOO + "' + ' ' + '" + K_STR_BAR + "'";
        case K_SUB_CASE_INDEX_FIVE: // Number to string
            return std::string("'value: ') + ") + std::to_string(K_NUM_ADD_RESULT);
        default:
            return "''";
    }
}

static std::string GetStringConcatExpectedResult(size_t caseIndex)
{
    switch (caseIndex) {
        case 0: return K_STR_RESULT_HELLO_WORLD;
        case K_SUB_CASE_INDEX_ONE: return K_STR_RESULT_FOOBAR;
        case K_SUB_CASE_INDEX_TWO: return std::string(K_STR_FOO) + K_STR_BAR + K_STR_BAZ;
        case K_SUB_CASE_INDEX_THREE: return K_STR_HELLO;
        case K_SUB_CASE_INDEX_FOUR: return std::string(K_STR_FOO) + " " + K_STR_BAR;
        case K_SUB_CASE_INDEX_FIVE: return std::string("value: ") + std::to_string(K_NUM_ADD_RESULT);
        default: return "";
    }
}

// Test case generation - object literals
static std::string BuildObjectLiteralScript(size_t caseIndex)
{
    switch (caseIndex) {
        case 0: // Simple object
            return std::string("({ ") + K_PROP_NAME_A + ": " + std::to_string(K_OBJ_VAL_A) + " })";
        case K_SUB_CASE_INDEX_ONE: // Two properties
            return std::string("({ ") + K_PROP_NAME_A + ": " + std::to_string(K_OBJ_VAL_A) + ", " +
                   K_PROP_NAME_B + ": " + std::to_string(K_OBJ_VAL_B) + " })";
        case K_SUB_CASE_INDEX_TWO: // Coordinate object
            return std::string("({ ") + K_PROP_NAME_X + ": " + std::to_string(K_OBJ_VAL_X) + ", " +
                   K_PROP_NAME_Y + ": " + std::to_string(K_OBJ_VAL_Y) + " })";
        case K_SUB_CASE_INDEX_THREE: // Nested object
            return std::string("({ outer: { inner: ") + std::to_string(K_OBJ_VAL_A) + " } })";
        default:
            return "({})";
    }
}

// Test case generation - array literals
static std::string BuildArrayLiteralScript(size_t caseIndex)
{
    switch (caseIndex) {
        case 0: // Simple array
            return "[" + std::to_string(K_ARR_ELEM_1) + "]";
        case K_SUB_CASE_INDEX_ONE: // Two elements
            return "[" + std::to_string(K_ARR_ELEM_1) + ", " + std::to_string(K_ARR_ELEM_2) + "]";
        case K_SUB_CASE_INDEX_TWO: // Three elements
            return "[" + std::to_string(K_ARR_ELEM_1) + ", " +
                   std::to_string(K_ARR_ELEM_2) + ", " +
                   std::to_string(K_ARR_ELEM_3) + "]";
        case K_SUB_CASE_INDEX_THREE: // Mixed types
            return "['" + std::string(K_STR_FOO) + "', " + std::to_string(K_OBJ_VAL_A) + "]";
        default:
            return "[]";
    }
}

// Test case generation - IIFE
static std::string BuildIIFEScript(size_t caseIndex)
{
    switch (caseIndex) {
        case 0: // Return integer
            return "(function() { return " + std::to_string(K_IIFE_VAL_42) + "; })()";
        case K_SUB_CASE_INDEX_ONE: // Return computed value
            return "(function() { return " + std::to_string(K_NUM_ADD_A) + " + " +
                   std::to_string(K_NUM_ADD_B) + "; })()";
        case K_SUB_CASE_INDEX_TWO: // Return another computed value
            return "(function(x) { return x * " + std::to_string(K_NUM_MUL_B) + "; })(" +
                   std::to_string(K_NUM_MUL_A) + ")";
        case K_SUB_CASE_INDEX_THREE: // Return larger value
            return "(function() { return " + std::to_string(K_IIFE_VAL_100) + "; })()";
        default:
            return "(function() { return 0; })()";
    }
}

static int32_t GetIIFEExpectedResult(size_t caseIndex)
{
    switch (caseIndex) {
        case 0: return K_IIFE_VAL_42;
        case K_SUB_CASE_INDEX_ONE: return K_NUM_ADD_RESULT;
        case K_SUB_CASE_INDEX_TWO: return K_NUM_MUL_RESULT;
        case K_SUB_CASE_INDEX_THREE: return K_IIFE_VAL_100;
        default: return 0;
    }
}

// Test case generation - typeof expressions
static std::string BuildTypeofScript(size_t caseIndex)
{
    switch (caseIndex) {
        case 0: return "typeof " + std::to_string(K_OBJ_VAL_A);
        case K_SUB_CASE_INDEX_ONE: return "typeof '" + std::string(K_STR_HELLO) + "'";
        case K_SUB_CASE_INDEX_TWO: return "typeof true";
        case K_SUB_CASE_INDEX_THREE: return "typeof undefined";
        case K_SUB_CASE_INDEX_FOUR: return "typeof null";
        case K_SUB_CASE_INDEX_FIVE: return "typeof []";
        default:
            return "typeof undefined";
    }
}

static std::string GetTypeofExpectedResult(size_t caseIndex)
{
    switch (caseIndex) {
        case 0: return "number";
        case K_SUB_CASE_INDEX_ONE: return "string";
        case K_SUB_CASE_INDEX_TWO: return "boolean";
        case K_SUB_CASE_INDEX_THREE: return "undefined";
        case K_SUB_CASE_INDEX_FOUR: return "object"; // typeof null === 'object'
        case K_SUB_CASE_INDEX_FIVE: return "object"; // typeof [] === 'object'
        default: return "undefined";
    }
}

// Test case generation - primitive values
static std::string BuildPrimitiveScript(size_t caseIndex)
{
    switch (caseIndex) {
        case 0: return "undefined";
        case K_SUB_CASE_INDEX_ONE: return "null";
        case K_SUB_CASE_INDEX_TWO: return "true";
        case K_SUB_CASE_INDEX_THREE: return "false";
        case K_SUB_CASE_INDEX_FOUR: return std::to_string(K_OBJ_VAL_A);
        case K_SUB_CASE_INDEX_FIVE: return std::to_string(K_NUM_MUL_RESULT) + "." + std::to_string(K_NUM_ADD_A);
        case K_SUB_CASE_INDEX_SIX: return "'" + std::string(K_STR_HELLO) + "'";
        case K_SUB_CASE_INDEX_SEVEN: return "'" + std::string(K_STR_FOO) + " " + std::string(K_STR_BAR) + "'";
        default:
            return "undefined";
    }
}

static napi_valuetype GetPrimitiveExpectedType(size_t caseIndex)
{
    switch (caseIndex) {
        case 0: return napi_undefined;
        case K_SUB_CASE_INDEX_ONE: return napi_null;
        case K_SUB_CASE_INDEX_TWO: return napi_boolean;
        case K_SUB_CASE_INDEX_THREE: return napi_boolean;
        case K_SUB_CASE_INDEX_FOUR: return napi_number;
        case K_SUB_CASE_INDEX_FIVE: return napi_number;
        case K_SUB_CASE_INDEX_SIX: return napi_string;
        case K_SUB_CASE_INDEX_SEVEN: return napi_string;
        default: return napi_undefined;
    }
}

// Test case generation - syntax errors
static std::string BuildSyntaxErrorScript(size_t caseIndex)
{
    switch (caseIndex) {
        case 0: return "invalid syntax here";
        case K_SUB_CASE_INDEX_ONE: return "function("; // Missing closing paren
        case K_SUB_CASE_INDEX_TWO: return "return"; // Return outside function
        case K_SUB_CASE_INDEX_THREE: return "1 + + 2"; // Invalid operator sequence
        default:
            return "syntax error";
    }
}

// Test case generation - strict equals
static std::string BuildStrictEqualsScript(size_t caseIndex)
{
    switch (caseIndex) {
        case 0: // Number equals
            return std::to_string(K_OBJ_VAL_A) + " === " + std::to_string(K_OBJ_VAL_A);
        case K_SUB_CASE_INDEX_ONE: // String equals
            return "'" + std::string(K_STR_HELLO) + "' === '" + std::string(K_STR_HELLO) + "'";
        case K_SUB_CASE_INDEX_TWO: // Boolean equals
            return "true === true";
        case K_SUB_CASE_INDEX_THREE: // Different types
            return std::to_string(K_OBJ_VAL_A) + " === '" + std::string(K_STR_HELLO) + "'";
        default:
            return "true === true";
    }
}

static bool GetStrictEqualsExpectedResult(size_t caseIndex)
{
    switch (caseIndex) {
        case 0: return true;
        case K_SUB_CASE_INDEX_ONE: return true;
        case K_SUB_CASE_INDEX_TWO: return true;
        case K_SUB_CASE_INDEX_THREE: return false;
        default: return true;
    }
}

static ScriptCaseSpec MakeArithmeticSpec(size_t caseIndex)
{
    ScriptCaseSpec spec;
    spec.category = ScriptTestCategory::ARITHMETIC;
    spec.scriptSource = BuildArithmeticScript(caseIndex);
    spec.expectedType = K_EXPECTED_NUMBER;
    spec.expectException = false;
    return spec;
}

static ScriptCaseSpec MakeStringConcatSpec(size_t localIndex)
{
    ScriptCaseSpec spec;
    spec.category = ScriptTestCategory::STRING_CONCAT;
    spec.scriptSource = BuildStringConcatScript(localIndex);
    spec.expectedType = K_EXPECTED_STRING;
    spec.expectException = false;
    return spec;
}

static ScriptCaseSpec MakeObjectLiteralSpec(size_t localIndex)
{
    ScriptCaseSpec spec;
    spec.category = ScriptTestCategory::OBJECT_LITERAL;
    spec.scriptSource = BuildObjectLiteralScript(localIndex);
    spec.expectedType = K_EXPECTED_OBJECT;
    spec.expectException = false;
    return spec;
}

static ScriptCaseSpec MakeArrayLiteralSpec(size_t localIndex)
{
    ScriptCaseSpec spec;
    spec.category = ScriptTestCategory::ARRAY_LITERAL;
    spec.scriptSource = BuildArrayLiteralScript(localIndex);
    spec.expectedType = K_EXPECTED_OBJECT;
    spec.expectException = false;
    return spec;
}

static ScriptCaseSpec MakeIIFESpec(size_t localIndex)
{
    ScriptCaseSpec spec;
    spec.category = ScriptTestCategory::IIFE;
    spec.scriptSource = BuildIIFEScript(localIndex);
    spec.expectedType = K_EXPECTED_NUMBER;
    spec.expectException = false;
    return spec;
}

static ScriptCaseSpec MakeTypeofSpec(size_t localIndex)
{
    ScriptCaseSpec spec;
    spec.category = ScriptTestCategory::TYPEOF;
    spec.scriptSource = BuildTypeofScript(localIndex);
    spec.expectedType = K_EXPECTED_STRING;
    spec.expectException = false;
    return spec;
}

static ScriptCaseSpec MakePrimitiveSpec(size_t localIndex)
{
    ScriptCaseSpec spec;
    spec.category = ScriptTestCategory::PRIMITIVE;
    spec.scriptSource = BuildPrimitiveScript(localIndex);
    spec.expectedType = GetPrimitiveExpectedType(localIndex);
    spec.expectException = false;
    return spec;
}

static ScriptCaseSpec MakeSyntaxErrorSpec(size_t localIndex)
{
    ScriptCaseSpec spec;
    spec.category = ScriptTestCategory::SYNTAX_ERROR;
    spec.scriptSource = BuildSyntaxErrorScript(localIndex);
    spec.expectedType = napi_undefined;
    spec.expectException = true;
    return spec;
}

static ScriptCaseSpec MakeStrictEqualsSpec(size_t localIndex)
{
    ScriptCaseSpec spec;
    spec.category = ScriptTestCategory::STRICT_EQUALS;
    spec.scriptSource = BuildStrictEqualsScript(localIndex);
    spec.expectedType = K_EXPECTED_BOOLEAN;
    spec.expectException = false;
    return spec;
}

// Main test case specification generator
ScriptCaseSpec GetScriptCaseSpec(size_t caseIndex)
{
    ScriptCaseSpec spec;
    spec.exportName = BuildExportName(caseIndex);
    size_t idx = caseIndex;

    if (idx < K_ARITHMETIC_CASE_COUNT) {
        return MakeArithmeticSpec(idx);
    }
    idx -= K_ARITHMETIC_CASE_COUNT;
    if (idx < K_STRING_CONCAT_CASE_COUNT) {
        return MakeStringConcatSpec(idx);
    }
    idx -= K_STRING_CONCAT_CASE_COUNT;
    if (idx < K_OBJECT_LITERAL_CASE_COUNT) {
        return MakeObjectLiteralSpec(idx);
    }
    idx -= K_OBJECT_LITERAL_CASE_COUNT;
    if (idx < K_ARRAY_LITERAL_CASE_COUNT) {
        return MakeArrayLiteralSpec(idx);
    }
    idx -= K_ARRAY_LITERAL_CASE_COUNT;
    if (idx < K_IIFE_CASE_COUNT) {
        return MakeIIFESpec(idx);
    }
    idx -= K_IIFE_CASE_COUNT;
    if (idx < K_TYPEOF_CASE_COUNT) {
        return MakeTypeofSpec(idx);
    }
    idx -= K_TYPEOF_CASE_COUNT;
    if (idx < K_PRIMITIVE_CASE_COUNT) {
        return MakePrimitiveSpec(idx);
    }
    idx -= K_PRIMITIVE_CASE_COUNT;
    if (idx < K_SYNTAX_ERROR_CASE_COUNT) {
        return MakeSyntaxErrorSpec(idx);
    }
    idx -= K_SYNTAX_ERROR_CASE_COUNT;
    if (idx < K_STRICT_EQUALS_CASE_COUNT) {
        return MakeStrictEqualsSpec(idx);
    }

    spec.scriptSource = "undefined";
    spec.expectedType = napi_undefined;
    spec.category = ScriptTestCategory::PRIMITIVE;
    return spec;
}

std::string BuildExportName(size_t caseIndex)
{
    return "testScript" + std::to_string(caseIndex);
}

} // namespace ScriptSuite