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
#include <cstdint>
#include <cstring>
#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

namespace {

// ---------------------------------------------------------------------------
// Named constants (no magic numbers)
// ---------------------------------------------------------------------------
static constexpr size_t STRING_BUFFER_SIZE = 256;
static constexpr size_t NULL_TERMINATOR_SIZE = 1;
static constexpr int32_t INT32_TEST_VALUE = 42;
static constexpr int32_t SEQUENTIAL_ERROR_COUNT = 3;
static constexpr uint32_t MODULE_VERSION = 1;
static constexpr uint32_t NO_MODULE_FLAGS = 0;
static constexpr size_t EXCEPTION_TEST_COUNT = 32;

// ---------------------------------------------------------------------------
// Helper: set named boolean property
// ---------------------------------------------------------------------------
static bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value)
{
    napi_value napiValue = nullptr;
    if (napi_get_boolean(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

// ---------------------------------------------------------------------------
// Helper: set named int32 property
// ---------------------------------------------------------------------------
static bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int32(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

// ---------------------------------------------------------------------------
// Helper: set named string property
// ---------------------------------------------------------------------------
static bool SetNamedString(napi_env env, napi_value object, const char* name, const char* value)
{
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

// ---------------------------------------------------------------------------
// Helper: set named double property
// ---------------------------------------------------------------------------
static bool SetNamedDouble(napi_env env, napi_value object, const char* name, double value)
{
    napi_value napiValue = nullptr;
    if (napi_create_double(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

// ---------------------------------------------------------------------------
// Helper: create a napi_value string
// ---------------------------------------------------------------------------
static napi_value CreateString(napi_env env, const char* str)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, str, NAPI_AUTO_LENGTH, &result));
    return result;
}

// ---------------------------------------------------------------------------
// Helper: extract string from a napi_value into buffer
// ---------------------------------------------------------------------------
static bool ExtractString(napi_env env, napi_value value, char* buffer, size_t bufferSize)
{
    size_t length = 0;
    if (napi_get_value_string_utf8(env, value, buffer, bufferSize, &length) != napi_ok) {
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Helper: create result object
// ---------------------------------------------------------------------------
static napi_value CreateResultObject(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    return result;
}

// ---------------------------------------------------------------------------
// Helper: check if an error has a specific message property
// ---------------------------------------------------------------------------
static bool CheckErrorMessage(napi_env env, napi_value error, const char* expected)
{
    napi_value msgProp = nullptr;
    if (napi_get_named_property(env, error, "message", &msgProp) != napi_ok) {
        return false;
    }
    char buffer[STRING_BUFFER_SIZE] = {0};
    if (!ExtractString(env, msgProp, buffer, sizeof(buffer))) {
        return false;
    }
    return strcmp(buffer, expected) == 0;
}

// ---------------------------------------------------------------------------
// Helper: check if an error has a specific code property
// ---------------------------------------------------------------------------
static bool CheckErrorCode(napi_env env, napi_value error, const char* expected)
{
    napi_value codeProp = nullptr;
    if (napi_get_named_property(env, error, "code", &codeProp) != napi_ok) {
        return false;
    }
    char buffer[STRING_BUFFER_SIZE] = {0};
    if (!ExtractString(env, codeProp, buffer, sizeof(buffer))) {
        return false;
    }
    return strcmp(buffer, expected) == 0;
}

// ---------------------------------------------------------------------------
// Helper: clear any pending exception and return it
// ---------------------------------------------------------------------------
static napi_value ClearPendingException(napi_env env)
{
    napi_value exception = nullptr;
    napi_get_and_clear_last_exception(env, &exception);
    return exception;
}

// =========================================================================
// Test 01: napi_throw_error - basic error throw
// =========================================================================
static napi_value TestThrowError(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_status status = napi_throw_error(env, nullptr, "test error message");
    SetNamedBool(env, result, "throwStatus", status == napi_ok);

    bool isPending = false;
    napi_is_exception_pending(env, &isPending);
    SetNamedBool(env, result, "isPending", isPending);

    napi_value exception = ClearPendingException(env);
    bool hasMessage = (exception != nullptr) && CheckErrorMessage(env, exception, "test error message");
    SetNamedBool(env, result, "messageMatch", hasMessage);

    return result;
}

// =========================================================================
// Test 02: napi_throw_error with code
// =========================================================================
static napi_value TestThrowErrorWithCode(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_status status = napi_throw_error(env, "ERR_CUSTOM", "error with code");
    SetNamedBool(env, result, "throwStatus", status == napi_ok);

    napi_value exception = ClearPendingException(env);
    bool hasCode = (exception != nullptr) && CheckErrorCode(env, exception, "ERR_CUSTOM");
    SetNamedBool(env, result, "codeMatch", hasCode);

    bool hasMessage = (exception != nullptr) && CheckErrorMessage(env, exception, "error with code");
    SetNamedBool(env, result, "messageMatch", hasMessage);

    return result;
}

// =========================================================================
// Test 03: napi_throw_type_error - basic type error throw
// =========================================================================
static napi_value TestThrowTypeError(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_status status = napi_throw_type_error(env, nullptr, "type error message");
    SetNamedBool(env, result, "throwStatus", status == napi_ok);

    bool isPending = false;
    napi_is_exception_pending(env, &isPending);
    SetNamedBool(env, result, "isPending", isPending);

    napi_value exception = ClearPendingException(env);
    bool hasMessage = (exception != nullptr) && CheckErrorMessage(env, exception, "type error message");
    SetNamedBool(env, result, "messageMatch", hasMessage);

    return result;
}

// =========================================================================
// Test 04: napi_throw_type_error with code
// =========================================================================
static napi_value TestThrowTypeErrorWithCode(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_status status = napi_throw_type_error(env, "ERR_TYPE", "typed error");
    SetNamedBool(env, result, "throwStatus", status == napi_ok);

    napi_value exception = ClearPendingException(env);
    bool hasCode = (exception != nullptr) && CheckErrorCode(env, exception, "ERR_TYPE");
    SetNamedBool(env, result, "codeMatch", hasCode);

    bool hasMessage = (exception != nullptr) && CheckErrorMessage(env, exception, "typed error");
    SetNamedBool(env, result, "messageMatch", hasMessage);

    return result;
}

// =========================================================================
// Test 05: napi_throw_range_error - basic range error throw
// =========================================================================
static napi_value TestThrowRangeError(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_status status = napi_throw_range_error(env, nullptr, "range error message");
    SetNamedBool(env, result, "throwStatus", status == napi_ok);

    bool isPending = false;
    napi_is_exception_pending(env, &isPending);
    SetNamedBool(env, result, "isPending", isPending);

    napi_value exception = ClearPendingException(env);
    bool hasMessage = (exception != nullptr) && CheckErrorMessage(env, exception, "range error message");
    SetNamedBool(env, result, "messageMatch", hasMessage);

    return result;
}

// =========================================================================
// Test 06: napi_throw_range_error with code
// =========================================================================
static napi_value TestThrowRangeErrorWithCode(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_status status = napi_throw_range_error(env, "ERR_RANGE", "out of range");
    SetNamedBool(env, result, "throwStatus", status == napi_ok);

    napi_value exception = ClearPendingException(env);
    bool hasCode = (exception != nullptr) && CheckErrorCode(env, exception, "ERR_RANGE");
    SetNamedBool(env, result, "codeMatch", hasCode);

    bool hasMessage = (exception != nullptr) && CheckErrorMessage(env, exception, "out of range");
    SetNamedBool(env, result, "messageMatch", hasMessage);

    return result;
}

// =========================================================================
// Test 07: napi_is_exception_pending - no exception pending
// =========================================================================
static napi_value TestNoExceptionPending(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    bool isPending = true;
    napi_status status = napi_is_exception_pending(env, &isPending);
    SetNamedBool(env, result, "checkStatus", status == napi_ok);
    SetNamedBool(env, result, "notPending", !isPending);

    return result;
}

// =========================================================================
// Test 08: napi_is_exception_pending - exception is pending
// =========================================================================
static napi_value TestExceptionIsPending(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_throw_error(env, nullptr, "pending check");

    bool isPending = false;
    napi_is_exception_pending(env, &isPending);
    SetNamedBool(env, result, "isPending", isPending);

    ClearPendingException(env);

    bool isPendingAfterClear = true;
    napi_is_exception_pending(env, &isPendingAfterClear);
    SetNamedBool(env, result, "notPendingAfterClear", !isPendingAfterClear);

    return result;
}

// =========================================================================
// Test 09: napi_get_and_clear_last_exception
// =========================================================================
static napi_value TestGetAndClearException(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_throw_error(env, "ERR_TEST", "clear test");

    napi_value exception = nullptr;
    napi_status status = napi_get_and_clear_last_exception(env, &exception);
    SetNamedBool(env, result, "clearStatus", status == napi_ok);
    SetNamedBool(env, result, "hasException", exception != nullptr);

    bool isPending = true;
    napi_is_exception_pending(env, &isPending);
    SetNamedBool(env, result, "clearedSuccessfully", !isPending);

    if (exception != nullptr) {
        bool msgOk = CheckErrorMessage(env, exception, "clear test");
        SetNamedBool(env, result, "messageMatch", msgOk);
    }

    return result;
}

// =========================================================================
// Test 10: napi_create_error - basic
// =========================================================================
static napi_value TestCreateError(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_value code = CreateString(env, "ERR_CREATE");
    napi_value msg = CreateString(env, "created error");
    napi_value error = nullptr;

    napi_status status = napi_create_error(env, code, msg, &error);
    SetNamedBool(env, result, "createStatus", status == napi_ok);
    SetNamedBool(env, result, "errorNotNull", error != nullptr);

    if (error != nullptr) {
        bool isError = false;
        napi_is_error(env, error, &isError);
        SetNamedBool(env, result, "isError", isError);
    }

    return result;
}

// =========================================================================
// Test 11: napi_create_type_error
// =========================================================================
static napi_value TestCreateTypeError(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_value code = CreateString(env, "ERR_TYPE_CREATE");
    napi_value msg = CreateString(env, "type error created");
    napi_value error = nullptr;

    napi_status status = napi_create_type_error(env, code, msg, &error);
    SetNamedBool(env, result, "createStatus", status == napi_ok);
    SetNamedBool(env, result, "errorNotNull", error != nullptr);

    if (error != nullptr) {
        bool isError = false;
        napi_is_error(env, error, &isError);
        SetNamedBool(env, result, "isError", isError);
        SetNamedBool(env, result, "messageMatch", CheckErrorMessage(env, error, "type error created"));
    }

    return result;
}

// =========================================================================
// Test 12: napi_create_range_error
// =========================================================================
static napi_value TestCreateRangeError(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_value code = CreateString(env, "ERR_RANGE_CREATE");
    napi_value msg = CreateString(env, "range error created");
    napi_value error = nullptr;

    napi_status status = napi_create_range_error(env, code, msg, &error);
    SetNamedBool(env, result, "createStatus", status == napi_ok);
    SetNamedBool(env, result, "errorNotNull", error != nullptr);

    if (error != nullptr) {
        bool isError = false;
        napi_is_error(env, error, &isError);
        SetNamedBool(env, result, "isError", isError);
        SetNamedBool(env, result, "messageMatch", CheckErrorMessage(env, error, "range error created"));
    }

    return result;
}

// =========================================================================
// Test 13: napi_is_error - with error object
// =========================================================================
static napi_value TestIsErrorWithError(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_value code = CreateString(env, "CODE");
    napi_value msg = CreateString(env, "test");
    napi_value error = nullptr;
    NAPI_CALL(env, napi_create_error(env, code, msg, &error));

    bool isError = false;
    napi_status status = napi_is_error(env, error, &isError);
    SetNamedBool(env, result, "checkStatus", status == napi_ok);
    SetNamedBool(env, result, "isError", isError);

    return result;
}

// =========================================================================
// Test 14: napi_is_error - with non-error object
// =========================================================================
static napi_value TestIsErrorWithNonError(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_value plainObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &plainObj));

    bool isError = true;
    napi_status status = napi_is_error(env, plainObj, &isError);
    SetNamedBool(env, result, "checkStatus", status == napi_ok);
    SetNamedBool(env, result, "isNotError", !isError);

    // Also check with number
    napi_value num = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_TEST_VALUE, &num));

    bool isNumError = true;
    napi_is_error(env, num, &isNumError);
    SetNamedBool(env, result, "numberIsNotError", !isNumError);

    // Also check with string
    napi_value str = CreateString(env, "not an error");
    bool isStrError = true;
    napi_is_error(env, str, &isStrError);
    SetNamedBool(env, result, "stringIsNotError", !isStrError);

    return result;
}

// =========================================================================
// Test 15: napi_get_last_error_info - after successful call
// =========================================================================
static napi_value TestErrorInfoAfterSuccess(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    // Perform a successful operation first
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));

    const napi_extended_error_info* errorInfo = nullptr;
    napi_status status = napi_get_last_error_info(env, &errorInfo);
    SetNamedBool(env, result, "infoStatus", status == napi_ok);
    SetNamedBool(env, result, "hasInfo", errorInfo != nullptr);

    if (errorInfo != nullptr) {
        SetNamedInt32(env, result, "errorCode", errorInfo->error_code);
        SetNamedBool(env, result, "isClean", errorInfo->error_code == napi_ok);
    }

    return result;
}

// =========================================================================
// Test 16: napi_get_last_error_info - after failed call
// =========================================================================
static napi_value TestErrorInfoAfterFailure(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    // Deliberately cause an error by passing nullptr
    napi_status badStatus = napi_get_value_int32(env, nullptr, nullptr);
    SetNamedBool(env, result, "badCallFailed", badStatus != napi_ok);

    const napi_extended_error_info* errorInfo = nullptr;
    napi_status status = napi_get_last_error_info(env, &errorInfo);
    SetNamedBool(env, result, "infoStatus", status == napi_ok);

    if (errorInfo != nullptr) {
        SetNamedBool(env, result, "errorCodeNotOk", errorInfo->error_code != napi_ok);
        SetNamedInt32(env, result, "errorCode", errorInfo->error_code);
    }

    return result;
}

// =========================================================================
// Test 17: Error message property inspection
// =========================================================================
static napi_value TestErrorMessageProperty(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_value msg = CreateString(env, "inspectable message");
    napi_value error = nullptr;
    NAPI_CALL(env, napi_create_error(env, nullptr, msg, &error));

    // Read message property
    napi_value msgProp = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, error, "message", &msgProp));

    char buffer[STRING_BUFFER_SIZE] = {0};
    bool extracted = ExtractString(env, msgProp, buffer, sizeof(buffer));
    SetNamedBool(env, result, "extracted", extracted);
    SetNamedString(env, result, "message", buffer);
    SetNamedBool(env, result, "messageMatch", strcmp(buffer, "inspectable message") == 0);

    return result;
}

// =========================================================================
// Test 18: Error code property inspection
// =========================================================================
static napi_value TestErrorCodeProperty(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_value code = CreateString(env, "CUSTOM_CODE_001");
    napi_value msg = CreateString(env, "code test");
    napi_value error = nullptr;
    NAPI_CALL(env, napi_create_error(env, code, msg, &error));

    // Read code property
    napi_value codeProp = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, error, "code", &codeProp));

    char buffer[STRING_BUFFER_SIZE] = {0};
    bool extracted = ExtractString(env, codeProp, buffer, sizeof(buffer));
    SetNamedBool(env, result, "extracted", extracted);
    SetNamedString(env, result, "code", buffer);
    SetNamedBool(env, result, "codeMatch", strcmp(buffer, "CUSTOM_CODE_001") == 0);

    return result;
}

// =========================================================================
// Test 19: napi_create_error with null code
// =========================================================================
static napi_value TestCreateErrorNullCode(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_value msg = CreateString(env, "null code error");
    napi_value error = nullptr;

    napi_status status = napi_create_error(env, nullptr, msg, &error);
    SetNamedBool(env, result, "createStatus", status == napi_ok);
    SetNamedBool(env, result, "errorNotNull", error != nullptr);

    if (error != nullptr) {
        bool isError = false;
        napi_is_error(env, error, &isError);
        SetNamedBool(env, result, "isError", isError);
        SetNamedBool(env, result, "messageMatch", CheckErrorMessage(env, error, "null code error"));
    }

    return result;
}

// =========================================================================
// Test 20: napi_create_error with empty message
// =========================================================================
static napi_value TestCreateErrorEmptyMessage(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_value code = CreateString(env, "ERR_EMPTY");
    napi_value msg = CreateString(env, "");
    napi_value error = nullptr;

    napi_status status = napi_create_error(env, code, msg, &error);
    SetNamedBool(env, result, "createStatus", status == napi_ok);

    if (error != nullptr) {
        bool isError = false;
        napi_is_error(env, error, &isError);
        SetNamedBool(env, result, "isError", isError);
        SetNamedBool(env, result, "messageMatch", CheckErrorMessage(env, error, ""));
    }

    return result;
}

// =========================================================================
// Test 21: napi_throw with created error object
// =========================================================================
static napi_value TestThrowCreatedError(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_value code = CreateString(env, "THROW_CODE");
    napi_value msg = CreateString(env, "thrown error");
    napi_value error = nullptr;
    NAPI_CALL(env, napi_create_error(env, code, msg, &error));

    napi_status status = napi_throw(env, error);
    SetNamedBool(env, result, "throwStatus", status == napi_ok);

    bool isPending = false;
    napi_is_exception_pending(env, &isPending);
    SetNamedBool(env, result, "isPending", isPending);

    napi_value exception = ClearPendingException(env);
    SetNamedBool(env, result, "hasException", exception != nullptr);

    return result;
}

// =========================================================================
// Test 22: Throw and catch pattern
// =========================================================================
static napi_value TestThrowAndCatchPattern(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    // Phase 1: Throw
    napi_throw_error(env, "CATCH_CODE", "catch me");

    // Phase 2: Verify pending
    bool isPending = false;
    napi_is_exception_pending(env, &isPending);
    SetNamedBool(env, result, "pendingAfterThrow", isPending);

    // Phase 3: Catch (clear)
    napi_value exception = ClearPendingException(env);
    SetNamedBool(env, result, "caught", exception != nullptr);

    // Phase 4: Verify cleared
    bool isPendingAfter = true;
    napi_is_exception_pending(env, &isPendingAfter);
    SetNamedBool(env, result, "clearedAfterCatch", !isPendingAfter);

    // Phase 5: Inspect caught exception
    if (exception != nullptr) {
        SetNamedBool(env, result, "messageOk", CheckErrorMessage(env, exception, "catch me"));
        SetNamedBool(env, result, "codeOk", CheckErrorCode(env, exception, "CATCH_CODE"));
    }

    return result;
}

// =========================================================================
// Test 23: Multiple sequential error creation
// =========================================================================
static napi_value TestSequentialErrorCreation(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    bool allCreated = true;
    bool allAreErrors = true;

    for (int32_t i = 0; i < SEQUENTIAL_ERROR_COUNT; i++) {
        char codeStr[STRING_BUFFER_SIZE] = {0};
        int ret = snprintf_s(codeStr, sizeof(codeStr), sizeof(codeStr) - NULL_TERMINATOR_SIZE,
                             "SEQ_%d", i);
        if (ret < 0) {
            allCreated = false;
            break;
        }

        char msgStr[STRING_BUFFER_SIZE] = {0};
        ret = snprintf_s(msgStr, sizeof(msgStr), sizeof(msgStr) - NULL_TERMINATOR_SIZE,
                         "sequential error %d", i);
        if (ret < 0) {
            allCreated = false;
            break;
        }

        napi_value code = CreateString(env, codeStr);
        napi_value msg = CreateString(env, msgStr);
        napi_value error = nullptr;

        if (napi_create_error(env, code, msg, &error) != napi_ok) {
            allCreated = false;
            break;
        }

        bool isError = false;
        napi_is_error(env, error, &isError);
        if (!isError) {
            allAreErrors = false;
        }
    }

    SetNamedBool(env, result, "allCreated", allCreated);
    SetNamedBool(env, result, "allAreErrors", allAreErrors);
    SetNamedInt32(env, result, "count", SEQUENTIAL_ERROR_COUNT);

    return result;
}

// =========================================================================
// Test 24: Error with custom code strings
// =========================================================================
static napi_value TestErrorCustomCodeStrings(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    const char* customCode = "MODULE_EXCEPTION_SUITE_ERR_001";
    napi_value code = CreateString(env, customCode);
    napi_value msg = CreateString(env, "custom code test");
    napi_value error = nullptr;
    NAPI_CALL(env, napi_create_error(env, code, msg, &error));

    bool codeOk = CheckErrorCode(env, error, customCode);
    SetNamedBool(env, result, "codeMatch", codeOk);
    SetNamedString(env, result, "codeValue", customCode);

    return result;
}

// =========================================================================
// Test 25: Exception pending state management
// =========================================================================
static napi_value TestPendingStateManagement(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    // Initially not pending
    bool step1 = false;
    napi_is_exception_pending(env, &step1);
    SetNamedBool(env, result, "initiallyClean", !step1);

    // Throw -> pending
    napi_throw_error(env, nullptr, "state test");
    bool step2 = false;
    napi_is_exception_pending(env, &step2);
    SetNamedBool(env, result, "pendingAfterThrow", step2);

    // Clear -> not pending
    ClearPendingException(env);
    bool step3 = true;
    napi_is_exception_pending(env, &step3);
    SetNamedBool(env, result, "cleanAfterClear", !step3);

    // Throw again -> pending again
    napi_throw_type_error(env, nullptr, "state test 2");
    bool step4 = false;
    napi_is_exception_pending(env, &step4);
    SetNamedBool(env, result, "pendingAfterSecondThrow", step4);

    // Final cleanup
    ClearPendingException(env);

    return result;
}

// =========================================================================
// Test 26: Stack trace property on error
// =========================================================================
static napi_value TestStackTraceProperty(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_value code = CreateString(env, "STACK_TEST");
    napi_value msg = CreateString(env, "stack trace test");
    napi_value error = nullptr;
    NAPI_CALL(env, napi_create_error(env, code, msg, &error));

    // Check for "stack" property existence
    bool hasStack = false;
    napi_has_named_property(env, error, "stack", &hasStack);
    SetNamedBool(env, result, "hasStackProperty", hasStack);

    if (hasStack) {
        napi_value stackProp = nullptr;
        napi_get_named_property(env, error, "stack", &stackProp);

        napi_valuetype stackType = napi_undefined;
        napi_typeof(env, stackProp, &stackType);
        SetNamedBool(env, result, "stackIsString", stackType == napi_string);
    }

    return result;
}

// =========================================================================
// Test 27: Create error with all error types comparison
// =========================================================================
static napi_value TestCreateAllErrorTypes(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    // Create Error
    napi_value errCode = CreateString(env, "E1");
    napi_value errMsg = CreateString(env, "error");
    napi_value error1 = nullptr;
    napi_status s1 = napi_create_error(env, errCode, errMsg, &error1);
    SetNamedBool(env, result, "errorCreated", s1 == napi_ok);

    // Create TypeError
    napi_value typeCode = CreateString(env, "E2");
    napi_value typeMsg = CreateString(env, "type error");
    napi_value error2 = nullptr;
    napi_status s2 = napi_create_type_error(env, typeCode, typeMsg, &error2);
    SetNamedBool(env, result, "typeErrorCreated", s2 == napi_ok);

    // Create RangeError
    napi_value rangeCode = CreateString(env, "E3");
    napi_value rangeMsg = CreateString(env, "range error");
    napi_value error3 = nullptr;
    napi_status s3 = napi_create_range_error(env, rangeCode, rangeMsg, &error3);
    SetNamedBool(env, result, "rangeErrorCreated", s3 == napi_ok);

    // All should be errors
    bool isErr1 = false;
    bool isErr2 = false;
    bool isErr3 = false;
    napi_is_error(env, error1, &isErr1);
    napi_is_error(env, error2, &isErr2);
    napi_is_error(env, error3, &isErr3);
    SetNamedBool(env, result, "allAreErrors", isErr1 && isErr2 && isErr3);

    return result;
}

// =========================================================================
// Test 28: napi_create_type_error with null code
// =========================================================================
static napi_value TestCreateTypeErrorNullCode(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_value msg = CreateString(env, "type error no code");
    napi_value error = nullptr;

    napi_status status = napi_create_type_error(env, nullptr, msg, &error);
    SetNamedBool(env, result, "createStatus", status == napi_ok);

    if (error != nullptr) {
        bool isError = false;
        napi_is_error(env, error, &isError);
        SetNamedBool(env, result, "isError", isError);
        SetNamedBool(env, result, "messageMatch",
                     CheckErrorMessage(env, error, "type error no code"));
    }

    return result;
}

// =========================================================================
// Test 29: napi_create_range_error with null code
// =========================================================================
static napi_value TestCreateRangeErrorNullCode(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_value msg = CreateString(env, "range error no code");
    napi_value error = nullptr;

    napi_status status = napi_create_range_error(env, nullptr, msg, &error);
    SetNamedBool(env, result, "createStatus", status == napi_ok);

    if (error != nullptr) {
        bool isError = false;
        napi_is_error(env, error, &isError);
        SetNamedBool(env, result, "isError", isError);
        SetNamedBool(env, result, "messageMatch",
                     CheckErrorMessage(env, error, "range error no code"));
    }

    return result;
}

// =========================================================================
// Test 30: Throw created error and inspect after catch
// =========================================================================
static napi_value TestThrowCreatedAndInspect(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    // Create an error with code and message
    napi_value code = CreateString(env, "INSPECT_CODE");
    napi_value msg = CreateString(env, "inspect after catch");
    napi_value error = nullptr;
    NAPI_CALL(env, napi_create_error(env, code, msg, &error));

    // Throw it
    napi_throw(env, error);

    // Catch it
    napi_value caught = ClearPendingException(env);
    SetNamedBool(env, result, "caught", caught != nullptr);

    if (caught != nullptr) {
        SetNamedBool(env, result, "msgOk",
                     CheckErrorMessage(env, caught, "inspect after catch"));
        SetNamedBool(env, result, "codeOk",
                     CheckErrorCode(env, caught, "INSPECT_CODE"));

        bool isError = false;
        napi_is_error(env, caught, &isError);
        SetNamedBool(env, result, "isError", isError);
    }

    return result;
}

// =========================================================================
// Test 31: Error creation pattern for fatal_error (no actual call)
// =========================================================================
static napi_value TestFatalErrorPattern(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    // napi_fatal_error terminates the process, so we only test the error
    // creation patterns that would be used alongside it. We verify that
    // errors can be created and inspected without calling fatal_error.

    napi_value fatalCode = CreateString(env, "FATAL_SIMULATION");
    napi_value fatalMsg = CreateString(env, "simulated fatal error");
    napi_value fatalError = nullptr;
    NAPI_CALL(env, napi_create_error(env, fatalCode, fatalMsg, &fatalError));

    bool isError = false;
    napi_is_error(env, fatalError, &isError);
    SetNamedBool(env, result, "isError", isError);
    SetNamedBool(env, result, "messageOk",
                 CheckErrorMessage(env, fatalError, "simulated fatal error"));
    SetNamedBool(env, result, "codeOk",
                 CheckErrorCode(env, fatalError, "FATAL_SIMULATION"));

    // Verify error has standard properties
    bool hasMessage = false;
    napi_has_named_property(env, fatalError, "message", &hasMessage);
    SetNamedBool(env, result, "hasMessage", hasMessage);

    bool hasStack = false;
    napi_has_named_property(env, fatalError, "stack", &hasStack);
    SetNamedBool(env, result, "hasStack", hasStack);

    return result;
}

// =========================================================================
// Test 32: Error with long message and special characters
// =========================================================================
static napi_value TestErrorLongMessage(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    const char* longMsg = "This is a very long error message designed to test "
                          "the handling of longer strings in the NAPI error "
                          "creation APIs. It includes special chars: <>&\"'";

    napi_value code = CreateString(env, "ERR_LONG");
    napi_value msg = CreateString(env, longMsg);
    napi_value error = nullptr;

    napi_status status = napi_create_error(env, code, msg, &error);
    SetNamedBool(env, result, "createStatus", status == napi_ok);

    if (error != nullptr) {
        bool isError = false;
        napi_is_error(env, error, &isError);
        SetNamedBool(env, result, "isError", isError);
        SetNamedBool(env, result, "messageMatch", CheckErrorMessage(env, error, longMsg));
    }

    return result;
}

// =========================================================================
// Test function descriptor table
// =========================================================================
struct ExceptionTestEntry {
    const char* name;
    napi_callback callback;
};

static const ExceptionTestEntry EXCEPTION_TESTS[] = {
    {"testThrowError", TestThrowError},
    {"testThrowErrorWithCode", TestThrowErrorWithCode},
    {"testThrowTypeError", TestThrowTypeError},
    {"testThrowTypeErrorWithCode", TestThrowTypeErrorWithCode},
    {"testThrowRangeError", TestThrowRangeError},
    {"testThrowRangeErrorWithCode", TestThrowRangeErrorWithCode},
    {"testNoExceptionPending", TestNoExceptionPending},
    {"testExceptionIsPending", TestExceptionIsPending},
    {"testGetAndClearException", TestGetAndClearException},
    {"testCreateError", TestCreateError},
    {"testCreateTypeError", TestCreateTypeError},
    {"testCreateRangeError", TestCreateRangeError},
    {"testIsErrorWithError", TestIsErrorWithError},
    {"testIsErrorWithNonError", TestIsErrorWithNonError},
    {"testErrorInfoAfterSuccess", TestErrorInfoAfterSuccess},
    {"testErrorInfoAfterFailure", TestErrorInfoAfterFailure},
    {"testErrorMessageProperty", TestErrorMessageProperty},
    {"testErrorCodeProperty", TestErrorCodeProperty},
    {"testCreateErrorNullCode", TestCreateErrorNullCode},
    {"testCreateErrorEmptyMessage", TestCreateErrorEmptyMessage},
    {"testThrowCreatedError", TestThrowCreatedError},
    {"testThrowAndCatchPattern", TestThrowAndCatchPattern},
    {"testSequentialErrorCreation", TestSequentialErrorCreation},
    {"testErrorCustomCodeStrings", TestErrorCustomCodeStrings},
    {"testPendingStateManagement", TestPendingStateManagement},
    {"testStackTraceProperty", TestStackTraceProperty},
    {"testCreateAllErrorTypes", TestCreateAllErrorTypes},
    {"testCreateTypeErrorNullCode", TestCreateTypeErrorNullCode},
    {"testCreateRangeErrorNullCode", TestCreateRangeErrorNullCode},
    {"testThrowCreatedAndInspect", TestThrowCreatedAndInspect},
    {"testFatalErrorPattern", TestFatalErrorPattern},
    {"testErrorLongMessage", TestErrorLongMessage},
};

}  // namespace

// ---------------------------------------------------------------------------
// Module initialization
// ---------------------------------------------------------------------------
static napi_value InitExceptionSuite(napi_env env, napi_value exports)
{
    static constexpr size_t testCount = sizeof(EXCEPTION_TESTS) / sizeof(EXCEPTION_TESTS[0]);

    napi_property_descriptor descriptors[EXCEPTION_TEST_COUNT];
    for (size_t i = 0; i < testCount && i < EXCEPTION_TEST_COUNT; i++) {
        descriptors[i] = DECLARE_NAPI_FUNCTION(EXCEPTION_TESTS[i].name,
            EXCEPTION_TESTS[i].callback);
    }

    NAPI_CALL(env, napi_define_properties(env, exports, testCount, descriptors));
    return exports;
}

// ---------------------------------------------------------------------------
// Module descriptor
// ---------------------------------------------------------------------------
static napi_module g_exceptionSuiteModule = {
    .nm_version = MODULE_VERSION,
    .nm_flags = NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitExceptionSuite,
    .nm_modname = "exception_suite",
    .nm_priv = nullptr,
};

// ---------------------------------------------------------------------------
// Module registration
// ---------------------------------------------------------------------------
extern "C" __attribute__((constructor)) void RegisterExceptionSuiteModule(void)
{
    napi_module_register(&g_exceptionSuiteModule);
}
