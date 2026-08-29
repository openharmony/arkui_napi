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

#include <string>
#include <vector>

namespace ScriptSuite {

// ---------------------------------------------------------------------------
// Test execution for arithmetic expressions
// ---------------------------------------------------------------------------

static napi_value RunArithmeticTest(napi_env env, size_t caseIndex, const ScriptCaseSpec& spec)
{
    napi_value result = CreateResultObject(env);
    SetNamedString(env, result, "category", "arithmetic");
    SetNamedString(env, result, "scriptSource", spec.scriptSource);

    napi_value scriptResult = nullptr;
    if (!RunScriptSource(env, spec.scriptSource, &scriptResult)) {
        SetNamedBool(env, result, "success", false);
        return result;
    }

    bool typeMatch = CheckValueType(env, scriptResult, K_EXPECTED_NUMBER);
    SetNamedBool(env, result, "typeMatch", typeMatch);

    int32_t actualValue = 0;
    if (GetInt32Value(env, scriptResult, &actualValue)) {
        int32_t expected = GetArithmeticExpectedResult(caseIndex);
        SetNamedInt32(env, result, "actualValue", actualValue);
        SetNamedInt32(env, result, "expectedValue", expected);
        SetNamedBool(env, result, "valueMatch", actualValue == expected);
    }

    SetNamedBool(env, result, "success", true);
    return result;
}

// ---------------------------------------------------------------------------
// Test execution for string concatenation
// ---------------------------------------------------------------------------

static napi_value RunStringConcatTest(napi_env env, size_t caseIndex, const ScriptCaseSpec& spec)
{
    napi_value result = CreateResultObject(env);
    SetNamedString(env, result, "category", "string_concat");
    SetNamedString(env, result, "scriptSource", spec.scriptSource);

    napi_value scriptResult = nullptr;
    if (!RunScriptSource(env, spec.scriptSource, &scriptResult)) {
        SetNamedBool(env, result, "success", false);
        return result;
    }

    bool typeMatch = CheckValueType(env, scriptResult, K_EXPECTED_STRING);
    SetNamedBool(env, result, "typeMatch", typeMatch);

    std::string actualValue;
    if (GetStringValue(env, scriptResult, actualValue)) {
        std::string expected = GetStringConcatExpectedResult(caseIndex);
        SetNamedString(env, result, "actualValue", actualValue);
        SetNamedString(env, result, "expectedValue", expected);
        SetNamedBool(env, result, "valueMatch", actualValue == expected);
    }

    SetNamedBool(env, result, "success", true);
    return result;
}

// ---------------------------------------------------------------------------
// Test execution for object literals
// ---------------------------------------------------------------------------

static napi_value RunObjectLiteralTest(napi_env env, const ScriptCaseSpec& spec)
{
    napi_value result = CreateResultObject(env);
    SetNamedString(env, result, "category", "object_literal");
    SetNamedString(env, result, "scriptSource", spec.scriptSource);

    napi_value scriptResult = nullptr;
    if (!RunScriptSource(env, spec.scriptSource, &scriptResult)) {
        SetNamedBool(env, result, "success", false);
        return result;
    }

    bool typeMatch = CheckValueType(env, scriptResult, K_EXPECTED_OBJECT);
    SetNamedBool(env, result, "typeMatch", typeMatch);

    // Verify object is not null
    napi_valuetype actualType = GetValueType(env, scriptResult);
    SetNamedBool(env, result, "isNonNullObject", actualType == napi_object);

    SetNamedBool(env, result, "success", typeMatch && (actualType == napi_object));
    return result;
}

// ---------------------------------------------------------------------------
// Test execution for array literals
// ---------------------------------------------------------------------------

static napi_value RunArrayLiteralTest(napi_env env, size_t caseIndex, const ScriptCaseSpec& spec)
{
    napi_value result = CreateResultObject(env);
    SetNamedString(env, result, "category", "array_literal");
    SetNamedString(env, result, "scriptSource", spec.scriptSource);

    napi_value scriptResult = nullptr;
    if (!RunScriptSource(env, spec.scriptSource, &scriptResult)) {
        SetNamedBool(env, result, "success", false);
        return result;
    }

    bool typeMatch = CheckValueType(env, scriptResult, K_EXPECTED_OBJECT);
    SetNamedBool(env, result, "typeMatch", typeMatch);

    // Check if it's actually an array
    bool isArray = false;
    napi_is_array(env, scriptResult, &isArray);
    SetNamedBool(env, result, "isArray", isArray);

    // Get array length
    uint32_t arrayLength = 0;
    napi_get_array_length(env, scriptResult, &arrayLength);
    SetNamedInt32(env, result, "arrayLength", static_cast<int32_t>(arrayLength));

    // Verify length based on case
    uint32_t expectedLength = K_ARR_LENGTH;
    if (caseIndex == 0) {
        expectedLength = K_ARR_LEN_SINGLE;
    } else if (caseIndex == K_SUB_CASE_INDEX_ONE) {
        expectedLength = K_ARR_LEN_TWO;
    } else if (caseIndex == K_SUB_CASE_INDEX_THREE) {
        expectedLength = K_ARR_LEN_TWO;
    }
    SetNamedBool(env, result, "lengthMatch", arrayLength == expectedLength);

    SetNamedBool(env, result, "success", isArray && typeMatch);
    return result;
}

// ---------------------------------------------------------------------------
// Test execution for IIFE
// ---------------------------------------------------------------------------

static napi_value RunIIFETest(napi_env env, size_t caseIndex, const ScriptCaseSpec& spec)
{
    napi_value result = CreateResultObject(env);
    SetNamedString(env, result, "category", "iife");
    SetNamedString(env, result, "scriptSource", spec.scriptSource);

    napi_value scriptResult = nullptr;
    if (!RunScriptSource(env, spec.scriptSource, &scriptResult)) {
        SetNamedBool(env, result, "success", false);
        return result;
    }

    bool typeMatch = CheckValueType(env, scriptResult, K_EXPECTED_NUMBER);
    SetNamedBool(env, result, "typeMatch", typeMatch);

    int32_t actualValue = 0;
    if (GetInt32Value(env, scriptResult, &actualValue)) {
        int32_t expected = GetIIFEExpectedResult(caseIndex);
        SetNamedInt32(env, result, "actualValue", actualValue);
        SetNamedInt32(env, result, "expectedValue", expected);
        SetNamedBool(env, result, "valueMatch", actualValue == expected);
    }

    SetNamedBool(env, result, "success", true);
    return result;
}

// ---------------------------------------------------------------------------
// Test execution for typeof expressions
// ---------------------------------------------------------------------------

static napi_value RunTypeofTest(napi_env env, size_t caseIndex, const ScriptCaseSpec& spec)
{
    napi_value result = CreateResultObject(env);
    SetNamedString(env, result, "category", "typeof");
    SetNamedString(env, result, "scriptSource", spec.scriptSource);

    napi_value scriptResult = nullptr;
    if (!RunScriptSource(env, spec.scriptSource, &scriptResult)) {
        SetNamedBool(env, result, "success", false);
        return result;
    }

    bool typeMatch = CheckValueType(env, scriptResult, K_EXPECTED_STRING);
    SetNamedBool(env, result, "typeMatch", typeMatch);

    std::string actualValue;
    if (GetStringValue(env, scriptResult, actualValue)) {
        std::string expected = GetTypeofExpectedResult(caseIndex);
        SetNamedString(env, result, "actualValue", actualValue);
        SetNamedString(env, result, "expectedValue", expected);
        SetNamedBool(env, result, "valueMatch", actualValue == expected);
    }

    SetNamedBool(env, result, "success", true);
    return result;
}

// ---------------------------------------------------------------------------
// Test execution for primitive values
// ---------------------------------------------------------------------------

static napi_value RunPrimitiveTest(napi_env env, size_t caseIndex, const ScriptCaseSpec& spec)
{
    napi_value result = CreateResultObject(env);
    SetNamedString(env, result, "category", "primitive");
    SetNamedString(env, result, "scriptSource", spec.scriptSource);

    napi_value scriptResult = nullptr;
    if (!RunScriptSource(env, spec.scriptSource, &scriptResult)) {
        SetNamedBool(env, result, "success", false);
        return result;
    }

    napi_valuetype actualType = GetValueType(env, scriptResult);
    napi_valuetype expectedType = GetPrimitiveExpectedType(caseIndex);

    bool typeMatch = (actualType == expectedType);
    SetNamedBool(env, result, "typeMatch", typeMatch);

    // Store type as string for visibility
    const char* typeNames[] = {
        "undefined", "null", "boolean", "number",
        "string", "object", "function", "external"
    };
    if (actualType >= napi_undefined && actualType <= napi_external) {
        SetNamedString(env, result, "actualType", typeNames[actualType]);
    }

    // For specific types, verify values
    if (actualType == napi_boolean) {
        bool boolValue = false;
        GetBoolValue(env, scriptResult, &boolValue);
        SetNamedBool(env, result, "boolValue", boolValue);
    } else if (actualType == napi_number) {
        int32_t numValue = 0;
        GetInt32Value(env, scriptResult, &numValue);
        SetNamedInt32(env, result, "numValue", numValue);
    } else if (actualType == napi_string) {
        std::string strValue;
        GetStringValue(env, scriptResult, strValue);
        SetNamedString(env, result, "strValue", strValue);
    }

    SetNamedBool(env, result, "success", typeMatch);
    return result;
}

// ---------------------------------------------------------------------------
// Test execution for syntax errors
// ---------------------------------------------------------------------------

static napi_value RunSyntaxErrorTest(napi_env env, const ScriptCaseSpec& spec)
{
    napi_value result = CreateResultObject(env);
    SetNamedString(env, result, "category", "syntax_error");
    SetNamedString(env, result, "scriptSource", spec.scriptSource);

    // Clear any prior exception
    ClearPendingException(env);

    // Run the script - it should fail
    napi_value scriptResult = nullptr;
    napi_status status = napi_create_string_utf8(env, spec.scriptSource.c_str(),
        spec.scriptSource.size(), &scriptResult);
    if (status != napi_ok) {
        SetNamedBool(env, result, "success", false);
        SetNamedBool(env, result, "createStringFailed", true);
        return result;
    }

    napi_value runResult = nullptr;
    status = napi_run_script(env, scriptResult, &runResult);

    // We expect an exception to be pending
    bool hasException = IsExceptionPending(env);
    SetNamedBool(env, result, "exceptionPending", hasException);

    // Clear the exception so we can continue
    napi_value exception = ClearPendingException(env);
    SetNamedBool(env, result, "exceptionCleared", true);

    // For syntax errors, success means we detected the exception
    SetNamedBool(env, result, "success", hasException);
    return result;
}

// ---------------------------------------------------------------------------
// Test execution for strict equals
// ---------------------------------------------------------------------------

static napi_value RunStrictEqualsTest(napi_env env, size_t caseIndex, const ScriptCaseSpec& spec)
{
    napi_value result = CreateResultObject(env);
    SetNamedString(env, result, "category", "strict_equals");
    SetNamedString(env, result, "scriptSource", spec.scriptSource);

    napi_value scriptResult = nullptr;
    if (!RunScriptSource(env, spec.scriptSource, &scriptResult)) {
        SetNamedBool(env, result, "success", false);
        return result;
    }

    bool typeMatch = CheckValueType(env, scriptResult, K_EXPECTED_BOOLEAN);
    SetNamedBool(env, result, "typeMatch", typeMatch);

    bool actualValue = false;
    if (GetBoolValue(env, scriptResult, &actualValue)) {
        bool expected = GetStrictEqualsExpectedResult(caseIndex);
        SetNamedBool(env, result, "actualValue", actualValue);
        SetNamedBool(env, result, "expectedValue", expected);
        SetNamedBool(env, result, "valueMatch", actualValue == expected);
    }

    SetNamedBool(env, result, "success", true);
    return result;
}

// ---------------------------------------------------------------------------
// Main test case dispatcher
// ---------------------------------------------------------------------------

static napi_value RunScriptTestCase(napi_env env, napi_callback_info info)
{
    // Extract case index from callback data
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data));

    size_t caseIndex = static_cast<size_t>(reinterpret_cast<uintptr_t>(data));
    if (caseIndex >= K_SCRIPT_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid script case index");
        return nullptr;
    }

    ScriptCaseSpec spec = GetScriptCaseSpec(caseIndex);

    // Dispatch to appropriate test handler based on category
    switch (spec.category) {
        case ScriptTestCategory::ARITHMETIC:
            return RunArithmeticTest(env, caseIndex, spec);
        case ScriptTestCategory::STRING_CONCAT:
            return RunStringConcatTest(env, caseIndex, spec);
        case ScriptTestCategory::OBJECT_LITERAL:
            return RunObjectLiteralTest(env, spec);
        case ScriptTestCategory::ARRAY_LITERAL:
            return RunArrayLiteralTest(env, caseIndex, spec);
        case ScriptTestCategory::IIFE:
            return RunIIFETest(env, caseIndex, spec);
        case ScriptTestCategory::TYPEOF:
            return RunTypeofTest(env, caseIndex, spec);
        case ScriptTestCategory::PRIMITIVE:
            return RunPrimitiveTest(env, caseIndex, spec);
        case ScriptTestCategory::SYNTAX_ERROR:
            return RunSyntaxErrorTest(env, spec);
        case ScriptTestCategory::STRICT_EQUALS:
            return RunStrictEqualsTest(env, caseIndex, spec);
        default:
            napi_throw_error(env, nullptr, "unknown script test category");
            return nullptr;
    }
}

// ---------------------------------------------------------------------------
// Additional test: run script with direct string creation
// ---------------------------------------------------------------------------

static napi_value TestRunScriptDirect(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    SetNamedString(env, result, "testName", "runScriptDirect");

    // Create script string directly
    std::string scriptText = std::to_string(K_NUM_ADD_A) + " + " + std::to_string(K_NUM_ADD_B);
    napi_value script = nullptr;
    if (!CreateScriptString(env, scriptText, &script)) {
        SetNamedBool(env, result, "createStringSuccess", false);
        return result;
    }
    SetNamedBool(env, result, "createStringSuccess", true);

    // Run the script
    napi_value scriptResult = nullptr;
    napi_status status = napi_run_script(env, script, &scriptResult);
    SetNamedBool(env, result, "runScriptSuccess", status == napi_ok);

    if (status == napi_ok) {
        int32_t value = 0;
        if (GetInt32Value(env, scriptResult, &value)) {
            SetNamedInt32(env, result, "resultValue", value);
            SetNamedBool(env, result, "valueCorrect", value == K_NUM_ADD_RESULT);
        }
    }

    return result;
}

// ---------------------------------------------------------------------------
// Additional test: verify exception handling
// ---------------------------------------------------------------------------

static napi_value TestExceptionHandling(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    SetNamedString(env, result, "testName", "exceptionHandling");

    // Initially no exception
    bool hasExceptionBefore = IsExceptionPending(env);
    SetNamedBool(env, result, "noExceptionInitially", !hasExceptionBefore);

    // Run a script that throws
    std::string throwScript = "throw new Error('test error')";
    napi_value script = nullptr;
    CreateScriptString(env, throwScript, &script);

    napi_value scriptResult = nullptr;
    napi_run_script(env, script, &scriptResult);

    // Should have exception now
    bool hasExceptionAfter = IsExceptionPending(env);
    SetNamedBool(env, result, "exceptionAfterThrow", hasExceptionAfter);

    // Clear the exception
    napi_value exception = ClearPendingException(env);
    SetNamedBool(env, result, "exceptionCleared", exception != nullptr);

    // No exception after clearing
    bool hasExceptionFinal = IsExceptionPending(env);
    SetNamedBool(env, result, "noExceptionAfterClear", !hasExceptionFinal);

    return result;
}

// ---------------------------------------------------------------------------
// Additional test: strict equals on different types
// ---------------------------------------------------------------------------

static napi_value TestStrictEqualsTypeDifference(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    SetNamedString(env, result, "testName", "strictEqualsTypeDiff");

    // Create values of different types
    napi_value numVal = nullptr;
    napi_value strVal = nullptr;
    napi_create_int32(env, K_OBJ_VAL_A, &numVal);
    napi_create_string_utf8(env, "1", NAPI_AUTO_LENGTH, &strVal);

    // Compare: 1 (number) === "1" (string) should be false
    bool areEqual = StrictEquals(env, numVal, strVal);
    SetNamedBool(env, result, "numberStringNotEqual", !areEqual);

    // Compare: 1 (number) === 1 (number) should be true
    napi_value numVal2 = nullptr;
    napi_create_int32(env, K_OBJ_VAL_A, &numVal2);
    areEqual = StrictEquals(env, numVal, numVal2);
    SetNamedBool(env, result, "sameNumberEqual", areEqual);

    // Compare: undefined === undefined should be true
    napi_value undef1 = nullptr;
    napi_value undef2 = nullptr;
    napi_get_undefined(env, &undef1);
    napi_get_undefined(env, &undef2);
    areEqual = StrictEquals(env, undef1, undef2);
    SetNamedBool(env, result, "undefinedEqual", areEqual);

    // Compare: null === null should be true
    napi_value null1 = nullptr;
    napi_value null2 = nullptr;
    napi_get_null(env, &null1);
    napi_get_null(env, &null2);
    areEqual = StrictEquals(env, null1, null2);
    SetNamedBool(env, result, "nullEqual", areEqual);

    // Compare: undefined === null should be false
    areEqual = StrictEquals(env, undef1, null1);
    SetNamedBool(env, result, "undefinedNullNotEqual", !areEqual);

    return result;
}

// ---------------------------------------------------------------------------
// Additional test: script returning complex objects
// ---------------------------------------------------------------------------

static napi_value TestComplexObjectReturn(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    SetNamedString(env, result, "testName", "complexObjectReturn");

    // Script that returns a complex nested object
    std::string complexScript = R"(
        ({
            data: {
                items: [
                    { id: 1, name: 'first' },
                    { id: 2, name: 'second' }
                ],
                count: 2
            },
            success: true
        })
    )";

    napi_value script = nullptr;
    if (!CreateScriptString(env, complexScript, &script)) {
        SetNamedBool(env, result, "createStringSuccess", false);
        return result;
    }

    napi_value scriptResult = nullptr;
    napi_status status = napi_run_script(env, script, &scriptResult);
    if (status != napi_ok) {
        SetNamedBool(env, result, "runScriptSuccess", false);
        return result;
    }
    SetNamedBool(env, result, "runScriptSuccess", true);

    // Verify result is an object
    bool isObject = CheckValueType(env, scriptResult, K_EXPECTED_OBJECT);
    SetNamedBool(env, result, "resultIsObject", isObject);

    // Check for 'success' property
    napi_value successProp = nullptr;
    bool hasSuccess = napi_get_named_property(env, scriptResult, "success", &successProp) == napi_ok;
    SetNamedBool(env, result, "hasSuccessProp", hasSuccess);

    if (hasSuccess) {
        bool successValue = false;
        GetBoolValue(env, successProp, &successValue);
        SetNamedBool(env, result, "successValue", successValue);
    }

    return result;
}

// ---------------------------------------------------------------------------
// Additional test: script array operations
// ---------------------------------------------------------------------------

static napi_value TestArrayOperations(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    SetNamedString(env, result, "testName", "arrayOperations");

    // Script that creates and manipulates an array
    std::string arrayScript = "[1, 2, 3, 4, 5].map(x => x * 2)";

    napi_value script = nullptr;
    CreateScriptString(env, arrayScript, &script);

    napi_value scriptResult = nullptr;
    if (napi_run_script(env, script, &scriptResult) != napi_ok) {
        SetNamedBool(env, result, "success", false);
        return result;
    }

    // Check if result is an array
    bool isArray = false;
    napi_is_array(env, scriptResult, &isArray);
    SetNamedBool(env, result, "resultIsArray", isArray);

    if (isArray) {
        uint32_t length = 0;
        napi_get_array_length(env, scriptResult, &length);
        SetNamedInt32(env, result, "arrayLength", static_cast<int32_t>(length));

        // Check first element (should be 2)
        napi_value firstElem = nullptr;
        napi_get_element(env, scriptResult, 0, &firstElem);
        int32_t firstValue = 0;
        GetInt32Value(env, firstElem, &firstValue);
        SetNamedInt32(env, result, "firstElement", firstValue);
    }

    SetNamedBool(env, result, "success", isArray);
    return result;
}

// ---------------------------------------------------------------------------
// Module initialization helpers
// ---------------------------------------------------------------------------

static void AddScriptTestDescriptors(std::vector<napi_property_descriptor>& descriptors,
    std::vector<std::string>& exportNames)
{
    for (size_t i = 0; i < K_SCRIPT_CASE_COUNT; i++) {
        exportNames.emplace_back(BuildExportName(i));
        napi_property_descriptor desc = {
            exportNames.back().c_str(),
            nullptr,
            RunScriptTestCase,
            nullptr,
            nullptr,
            nullptr,
            napi_default,
            reinterpret_cast<void*>(static_cast<uintptr_t>(i))
        };
        descriptors.push_back(desc);
    }
}

static void AddAdditionalTestDescriptors(std::vector<napi_property_descriptor>& descriptors)
{
    descriptors.push_back({ "testRunScriptDirect", nullptr, TestRunScriptDirect,
        nullptr, nullptr, nullptr, napi_default, nullptr });
    descriptors.push_back({ "testExceptionHandling", nullptr, TestExceptionHandling,
        nullptr, nullptr, nullptr, napi_default, nullptr });
    descriptors.push_back({ "testStrictEqualsTypeDiff", nullptr, TestStrictEqualsTypeDifference,
        nullptr, nullptr, nullptr, napi_default, nullptr });
    descriptors.push_back({ "testComplexObjectReturn", nullptr, TestComplexObjectReturn,
        nullptr, nullptr, nullptr, napi_default, nullptr });
    descriptors.push_back({ "testArrayOperations", nullptr, TestArrayOperations,
        nullptr, nullptr, nullptr, napi_default, nullptr });
}

// ---------------------------------------------------------------------------
// Module initialization
// ---------------------------------------------------------------------------

static napi_value InitScriptSuite(napi_env env, napi_value exports)
{
    std::vector<napi_property_descriptor> descriptors;
    std::vector<std::string> exportNames;
    exportNames.reserve(K_SCRIPT_CASE_COUNT + K_SUB_CASE_INDEX_FIVE);
    AddScriptTestDescriptors(descriptors, exportNames);
    AddAdditionalTestDescriptors(descriptors);
    NAPI_CALL(env, napi_define_properties(env, exports, descriptors.size(), descriptors.data()));
    return exports;
}

} // namespace ScriptSuite

// ---------------------------------------------------------------------------
// Module registration
// ---------------------------------------------------------------------------

static napi_module g_scriptSuiteModule = {
    .nm_version = ScriptSuite::K_MODULE_VERSION,
    .nm_flags = ScriptSuite::K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = ScriptSuite::InitScriptSuite,
    .nm_modname = "script_suite",
    .nm_priv = nullptr,
    .reserved = { nullptr }
};

extern "C" __attribute__((constructor)) void RegisterScriptSuiteModule(void)
{
    napi_module_register(&g_scriptSuiteModule);
}