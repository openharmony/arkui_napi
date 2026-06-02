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

#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstring>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

// ---------------------------------------------------------------------------
// Named constants – no magic numbers
// ---------------------------------------------------------------------------
static constexpr uint32_t MODULE_VERSION = 1;
static constexpr uint32_t NO_MODULE_FLAGS = 0;
static constexpr size_t ARRAYBUFFER_BYTE_LENGTH = 16;
static constexpr size_t TYPED_ARRAY_LENGTH = 4;
static constexpr size_t TYPED_ARRAY_OFFSET = 0;
static constexpr size_t DATAVIEW_LENGTH = 8;
static constexpr size_t DATAVIEW_OFFSET = 0;
static constexpr int32_t INT32_TEST_VALUE = 42;
static constexpr uint32_t UINT32_TEST_VALUE = 100;
static constexpr int64_t INT64_TEST_VALUE = 9007199254740992LL;
static constexpr double DOUBLE_TEST_VALUE = 3.14;
static constexpr double DOUBLE_ZERO = 0.0;
static constexpr double DOUBLE_NEGATIVE_ZERO = -0.0;
static constexpr double DOUBLE_MAX_SAFE_INTEGER = 9007199254740991.0;
static constexpr double DOUBLE_MIN_SAFE_INTEGER = -9007199254740991.0;
static constexpr double DATE_TIMESTAMP_MS = 1700000000000.0;
static constexpr size_t ARRAY_ELEMENT_COUNT = 3;
static constexpr size_t BUFFER_BYTE_LENGTH = 8;
static constexpr size_t ZERO_INDEX = 0;

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
// Helper: check typeof and compare
// ---------------------------------------------------------------------------
static bool CheckTypeOf(napi_env env, napi_value value, napi_valuetype expected)
{
    napi_valuetype actual = napi_undefined;
    if (napi_typeof(env, value, &actual) != napi_ok) {
        return false;
    }
    return actual == expected;
}

// ---------------------------------------------------------------------------
// Test 01: napi_typeof – undefined
// ---------------------------------------------------------------------------
static napi_value TestTypeofUndefined(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value undefined = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undefined));
    SetNamedBool(env, result, "isUndefined", CheckTypeOf(env, undefined, napi_undefined));
    return result;
}

// ---------------------------------------------------------------------------
// Test 02: napi_typeof – null
// ---------------------------------------------------------------------------
static napi_value TestTypeofNull(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value null = nullptr;
    NAPI_CALL(env, napi_get_null(env, &null));
    SetNamedBool(env, result, "isNull", CheckTypeOf(env, null, napi_null));
    return result;
}

// ---------------------------------------------------------------------------
// Test 03: napi_typeof – boolean
// ---------------------------------------------------------------------------
static napi_value TestTypeofBoolean(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value boolVal = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, true, &boolVal));
    SetNamedBool(env, result, "isBoolean", CheckTypeOf(env, boolVal, napi_boolean));
    return result;
}

// ---------------------------------------------------------------------------
// Test 04: napi_typeof – number
// ---------------------------------------------------------------------------
static napi_value TestTypeofNumber(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value numVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_TEST_VALUE, &numVal));
    SetNamedBool(env, result, "isNumber", CheckTypeOf(env, numVal, napi_number));
    return result;
}

// ---------------------------------------------------------------------------
// Test 05: napi_typeof – string
// ---------------------------------------------------------------------------
static napi_value TestTypeofString(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value strVal = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "hello", NAPI_AUTO_LENGTH, &strVal));
    SetNamedBool(env, result, "isString", CheckTypeOf(env, strVal, napi_string));
    return result;
}

// ---------------------------------------------------------------------------
// Test 06: napi_typeof – symbol
// ---------------------------------------------------------------------------
static napi_value TestTypeofSymbol(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value desc = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "testSym", NAPI_AUTO_LENGTH, &desc));
    napi_value sym = nullptr;
    NAPI_CALL(env, napi_create_symbol(env, desc, &sym));
    SetNamedBool(env, result, "isSymbol", CheckTypeOf(env, sym, napi_symbol));
    return result;
}

// ---------------------------------------------------------------------------
// Test 07: napi_typeof – object
// ---------------------------------------------------------------------------
static napi_value TestTypeofObject(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedBool(env, result, "isObject", CheckTypeOf(env, obj, napi_object));
    return result;
}

// ---------------------------------------------------------------------------
// Test 08: napi_typeof – function
// ---------------------------------------------------------------------------
static napi_value DummyCallback(napi_env env, napi_callback_info info)
{
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    return undefined;
}

static napi_value TestTypeofFunction(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value fn = nullptr;
    NAPI_CALL(env, napi_create_function(env, "dummy", NAPI_AUTO_LENGTH, DummyCallback, nullptr, &fn));
    SetNamedBool(env, result, "isFunction", CheckTypeOf(env, fn, napi_function));
    return result;
}

// ---------------------------------------------------------------------------
// Test 09: napi_typeof – external
// ---------------------------------------------------------------------------
static napi_value TestTypeofExternal(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    int dummy = 0;
    napi_value ext = nullptr;
    NAPI_CALL(env, napi_create_external(env, &dummy, nullptr, nullptr, &ext));
    SetNamedBool(env, result, "isExternal", CheckTypeOf(env, ext, napi_external));
    return result;
}

// ---------------------------------------------------------------------------
// Test 10: napi_typeof – bigint
// ---------------------------------------------------------------------------
static napi_value TestTypeofBigint(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value bigint = nullptr;
    NAPI_CALL(env, napi_create_bigint_int64(env, INT64_TEST_VALUE, &bigint));
    SetNamedBool(env, result, "isBigint", CheckTypeOf(env, bigint, napi_bigint));
    return result;
}

// ---------------------------------------------------------------------------
// Test 11: napi_is_array – positive
// ---------------------------------------------------------------------------
static napi_value TestIsArrayPositive(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value arr = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, ARRAY_ELEMENT_COUNT, &arr));
    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, arr, &isArray));
    SetNamedBool(env, result, "isArray", isArray);
    return result;
}

// ---------------------------------------------------------------------------
// Test 12: napi_is_array – negative
// ---------------------------------------------------------------------------
static napi_value TestIsArrayNegative(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    bool isArray = true;
    NAPI_CALL(env, napi_is_array(env, obj, &isArray));
    SetNamedBool(env, result, "isNotArray", !isArray);
    return result;
}

// ---------------------------------------------------------------------------
// Test 13: napi_is_arraybuffer – positive
// ---------------------------------------------------------------------------
static napi_value TestIsArraybufferPositive(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value ab = nullptr;
    NAPI_CALL(env, napi_create_arraybuffer(env, ARRAYBUFFER_BYTE_LENGTH, &data, &ab));
    bool isAB = false;
    NAPI_CALL(env, napi_is_arraybuffer(env, ab, &isAB));
    SetNamedBool(env, result, "isArrayBuffer", isAB);
    return result;
}

// ---------------------------------------------------------------------------
// Test 14: napi_is_arraybuffer – negative
// ---------------------------------------------------------------------------
static napi_value TestIsArraybufferNegative(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value num = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_TEST_VALUE, &num));
    bool isAB = true;
    NAPI_CALL(env, napi_is_arraybuffer(env, num, &isAB));
    SetNamedBool(env, result, "isNotArrayBuffer", !isAB);
    return result;
}

// ---------------------------------------------------------------------------
// Test 15: napi_is_buffer – positive
// ---------------------------------------------------------------------------
static napi_value TestIsBufferPositive(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value buf = nullptr;
    NAPI_CALL(env, napi_create_buffer(env, BUFFER_BYTE_LENGTH, &data, &buf));
    bool isBuf = false;
    NAPI_CALL(env, napi_is_buffer(env, buf, &isBuf));
    SetNamedBool(env, result, "isBuffer", isBuf);
    return result;
}

// ---------------------------------------------------------------------------
// Test 16: napi_is_buffer – negative
// ---------------------------------------------------------------------------
static napi_value TestIsBufferNegative(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "notbuf", NAPI_AUTO_LENGTH, &str));
    bool isBuf = true;
    NAPI_CALL(env, napi_is_buffer(env, str, &isBuf));
    SetNamedBool(env, result, "isNotBuffer", !isBuf);
    return result;
}

// ---------------------------------------------------------------------------
// Test 17: napi_is_date – positive
// ---------------------------------------------------------------------------
static napi_value TestIsDatePositive(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value date = nullptr;
    NAPI_CALL(env, napi_create_date(env, DATE_TIMESTAMP_MS, &date));
    bool isDate = false;
    NAPI_CALL(env, napi_is_date(env, date, &isDate));
    SetNamedBool(env, result, "isDate", isDate);
    return result;
}

// ---------------------------------------------------------------------------
// Test 18: napi_is_date – negative
// ---------------------------------------------------------------------------
static napi_value TestIsDateNegative(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    bool isDate = true;
    NAPI_CALL(env, napi_is_date(env, obj, &isDate));
    SetNamedBool(env, result, "isNotDate", !isDate);
    return result;
}

// ---------------------------------------------------------------------------
// Test 19: napi_is_error – positive
// ---------------------------------------------------------------------------
static napi_value TestIsErrorPositive(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value msg = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "test error", NAPI_AUTO_LENGTH, &msg));
    napi_value error = nullptr;
    NAPI_CALL(env, napi_create_error(env, nullptr, msg, &error));
    bool isError = false;
    NAPI_CALL(env, napi_is_error(env, error, &isError));
    SetNamedBool(env, result, "isError", isError);
    return result;
}

// ---------------------------------------------------------------------------
// Test 20: napi_is_error – negative
// ---------------------------------------------------------------------------
static napi_value TestIsErrorNegative(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    bool isError = true;
    NAPI_CALL(env, napi_is_error(env, obj, &isError));
    SetNamedBool(env, result, "isNotError", !isError);
    return result;
}

// ---------------------------------------------------------------------------
// Test 21: napi_is_typedarray – positive
// ---------------------------------------------------------------------------
static napi_value TestIsTypedarrayPositive(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value ab = nullptr;
    NAPI_CALL(env, napi_create_arraybuffer(env, ARRAYBUFFER_BYTE_LENGTH, &data, &ab));
    napi_value ta = nullptr;
    NAPI_CALL(env, napi_create_typedarray(env, napi_int32_array, TYPED_ARRAY_LENGTH, ab, TYPED_ARRAY_OFFSET, &ta));
    bool isTA = false;
    NAPI_CALL(env, napi_is_typedarray(env, ta, &isTA));
    SetNamedBool(env, result, "isTypedArray", isTA);
    return result;
}

// ---------------------------------------------------------------------------
// Test 22: napi_is_typedarray – negative
// ---------------------------------------------------------------------------
static napi_value TestIsTypedarrayNegative(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value arr = nullptr;
    NAPI_CALL(env, napi_create_array(env, &arr));
    bool isTA = true;
    NAPI_CALL(env, napi_is_typedarray(env, arr, &isTA));
    SetNamedBool(env, result, "isNotTypedArray", !isTA);
    return result;
}

// ---------------------------------------------------------------------------
// Test 23: napi_is_dataview – positive
// ---------------------------------------------------------------------------
static napi_value TestIsDataviewPositive(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value ab = nullptr;
    NAPI_CALL(env, napi_create_arraybuffer(env, ARRAYBUFFER_BYTE_LENGTH, &data, &ab));
    napi_value dv = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, DATAVIEW_LENGTH, ab, DATAVIEW_OFFSET, &dv));
    bool isDV = false;
    NAPI_CALL(env, napi_is_dataview(env, dv, &isDV));
    SetNamedBool(env, result, "isDataView", isDV);
    return result;
}

// ---------------------------------------------------------------------------
// Test 24: napi_is_dataview – negative
// ---------------------------------------------------------------------------
static napi_value TestIsDataviewNegative(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    bool isDV = true;
    NAPI_CALL(env, napi_is_dataview(env, obj, &isDV));
    SetNamedBool(env, result, "isNotDataView", !isDV);
    return result;
}

// ---------------------------------------------------------------------------
// Test 25: napi_is_promise – positive
// ---------------------------------------------------------------------------
static napi_value TestIsPromisePositive(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    bool isPromise = false;
    NAPI_CALL(env, napi_is_promise(env, promise, &isPromise));
    SetNamedBool(env, result, "isPromise", isPromise);
    // Resolve deferred to avoid leak
    napi_value undefined = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undefined));
    NAPI_CALL(env, napi_resolve_deferred(env, deferred, undefined));
    return result;
}

// ---------------------------------------------------------------------------
// Test 26: napi_is_promise – negative
// ---------------------------------------------------------------------------
static napi_value TestIsPromiseNegative(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    bool isPromise = true;
    NAPI_CALL(env, napi_is_promise(env, obj, &isPromise));
    SetNamedBool(env, result, "isNotPromise", !isPromise);
    return result;
}

// ---------------------------------------------------------------------------
// Test 27: napi_strict_equals – same type same value
// ---------------------------------------------------------------------------
static napi_value TestStrictEqualsSameValue(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value a = nullptr;
    napi_value b = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_TEST_VALUE, &a));
    NAPI_CALL(env, napi_create_int32(env, INT32_TEST_VALUE, &b));
    bool equal = false;
    NAPI_CALL(env, napi_strict_equals(env, a, b, &equal));
    SetNamedBool(env, result, "strictEqual", equal);
    return result;
}

// ---------------------------------------------------------------------------
// Test 28: napi_strict_equals – same type different value
// ---------------------------------------------------------------------------
static napi_value TestStrictEqualsDiffValue(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value a = nullptr;
    napi_value b = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_TEST_VALUE, &a));
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(UINT32_TEST_VALUE), &b));
    bool equal = true;
    NAPI_CALL(env, napi_strict_equals(env, a, b, &equal));
    SetNamedBool(env, result, "notEqual", !equal);
    return result;
}

// ---------------------------------------------------------------------------
// Test 29: napi_strict_equals – different types
// ---------------------------------------------------------------------------
static napi_value TestStrictEqualsDiffTypes(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value num = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_TEST_VALUE, &num));
    napi_value str = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "42", NAPI_AUTO_LENGTH, &str));
    bool equal = true;
    NAPI_CALL(env, napi_strict_equals(env, num, str, &equal));
    SetNamedBool(env, result, "notEqual", !equal);
    return result;
}

// ---------------------------------------------------------------------------
// Test 30: napi_strict_equals – null vs undefined
// ---------------------------------------------------------------------------
static napi_value TestStrictEqualsNullUndefined(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value nullVal = nullptr;
    NAPI_CALL(env, napi_get_null(env, &nullVal));
    napi_value undefVal = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undefVal));
    bool equal = true;
    NAPI_CALL(env, napi_strict_equals(env, nullVal, undefVal, &equal));
    SetNamedBool(env, result, "notEqual", !equal);
    return result;
}

// ---------------------------------------------------------------------------
// Test 31: napi_strict_equals – NaN vs NaN
// ---------------------------------------------------------------------------
static napi_value TestStrictEqualsNanNan(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value nan1 = nullptr;
    napi_value nan2 = nullptr;
    NAPI_CALL(env, napi_create_double(env, NAN, &nan1));
    NAPI_CALL(env, napi_create_double(env, NAN, &nan2));
    bool equal = true;
    NAPI_CALL(env, napi_strict_equals(env, nan1, nan2, &equal));
    // NaN !== NaN per JS spec
    SetNamedBool(env, result, "nanNotEqualNan", !equal);
    return result;
}

// ---------------------------------------------------------------------------
// Test 32: napi_get_value_bool roundtrip
// ---------------------------------------------------------------------------
static napi_value TestGetValueBoolTrue(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value boolTrue = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, true, &boolTrue));
    bool retrieved = false;
    NAPI_CALL(env, napi_get_value_bool(env, boolTrue, &retrieved));
    SetNamedBool(env, result, "valueIsTrue", retrieved);
    return result;
}

// ---------------------------------------------------------------------------
// Test 33: napi_get_value_bool – false
// ---------------------------------------------------------------------------
static napi_value TestGetValueBoolFalse(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value boolFalse = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, false, &boolFalse));
    bool retrieved = true;
    NAPI_CALL(env, napi_get_value_bool(env, boolFalse, &retrieved));
    SetNamedBool(env, result, "valueIsFalse", !retrieved);
    return result;
}

// ---------------------------------------------------------------------------
// Test 34: napi_get_value_int32 roundtrip
// ---------------------------------------------------------------------------
static napi_value TestGetValueInt32(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value num = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_TEST_VALUE, &num));
    int32_t retrieved = 0;
    NAPI_CALL(env, napi_get_value_int32(env, num, &retrieved));
    SetNamedBool(env, result, "roundtrip", retrieved == INT32_TEST_VALUE);
    SetNamedInt32(env, result, "value", retrieved);
    return result;
}

// ---------------------------------------------------------------------------
// Test 35: napi_get_value_uint32 roundtrip
// ---------------------------------------------------------------------------
static napi_value TestGetValueUint32(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value num = nullptr;
    NAPI_CALL(env, napi_create_uint32(env, UINT32_TEST_VALUE, &num));
    uint32_t retrieved = 0;
    NAPI_CALL(env, napi_get_value_uint32(env, num, &retrieved));
    SetNamedBool(env, result, "roundtrip", retrieved == UINT32_TEST_VALUE);
    SetNamedInt32(env, result, "value", static_cast<int32_t>(retrieved));
    return result;
}

// ---------------------------------------------------------------------------
// Test 36: napi_get_value_int64 roundtrip
// ---------------------------------------------------------------------------
static napi_value TestGetValueInt64(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value num = nullptr;
    NAPI_CALL(env, napi_create_int64(env, INT64_TEST_VALUE, &num));
    int64_t retrieved = 0;
    NAPI_CALL(env, napi_get_value_int64(env, num, &retrieved));
    SetNamedBool(env, result, "roundtrip", retrieved == INT64_TEST_VALUE);
    return result;
}

// ---------------------------------------------------------------------------
// Test 37: napi_get_value_double roundtrip
// ---------------------------------------------------------------------------
static napi_value TestGetValueDouble(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value num = nullptr;
    NAPI_CALL(env, napi_create_double(env, DOUBLE_TEST_VALUE, &num));
    double retrieved = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_value_double(env, num, &retrieved));
    SetNamedBool(env, result, "roundtrip", retrieved == DOUBLE_TEST_VALUE);
    SetNamedDouble(env, result, "value", retrieved);
    return result;
}

// ---------------------------------------------------------------------------
// Test 38: napi_is_object – various types
// ---------------------------------------------------------------------------
static napi_value TestIsObject(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_valuetype objType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, obj, &objType));
    SetNamedBool(env, result, "objectIsObject", objType == napi_object);

    napi_value arr = nullptr;
    NAPI_CALL(env, napi_create_array(env, &arr));
    napi_valuetype arrType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, arr, &arrType));
    SetNamedBool(env, result, "arrayIsObject", arrType == napi_object);

    napi_value num = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_TEST_VALUE, &num));
    napi_valuetype numType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, num, &numType));
    SetNamedBool(env, result, "numberNotObject", numType != napi_object);

    return result;
}

// ---------------------------------------------------------------------------
// Test 39: napi_coerce_to_string + typeof check
// ---------------------------------------------------------------------------
static napi_value TestCoerceToString(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value num = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_TEST_VALUE, &num));
    napi_value coerced = nullptr;
    NAPI_CALL(env, napi_coerce_to_string(env, num, &coerced));
    SetNamedBool(env, result, "coercedIsString", CheckTypeOf(env, coerced, napi_string));
    return result;
}

// ---------------------------------------------------------------------------
// Test 40: napi_instanceof with custom constructor
// ---------------------------------------------------------------------------
static napi_value ConstructorCallback(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr));
    return thisArg;
}

static napi_value TestInstanceof(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value ctor = nullptr;
    NAPI_CALL(env, napi_define_class(
        env, "TestClass", NAPI_AUTO_LENGTH, ConstructorCallback,
        nullptr, ZERO_INDEX, nullptr, &ctor));
    napi_value instance = nullptr;
    NAPI_CALL(env, napi_new_instance(env, ctor, ZERO_INDEX, nullptr, &instance));
    bool isInstance = false;
    NAPI_CALL(env, napi_instanceof(env, instance, ctor, &isInstance));
    SetNamedBool(env, result, "isInstance", isInstance);

    napi_value plain = nullptr;
    NAPI_CALL(env, napi_create_object(env, &plain));
    bool isNotInstance = true;
    NAPI_CALL(env, napi_instanceof(env, plain, ctor, &isNotInstance));
    SetNamedBool(env, result, "plainNotInstance", !isNotInstance);

    return result;
}

// ---------------------------------------------------------------------------
// Test 41: napi_is_detached_arraybuffer
// ---------------------------------------------------------------------------
static napi_value TestIsDetachedArraybuffer(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value ab = nullptr;
    NAPI_CALL(env, napi_create_arraybuffer(env, ARRAYBUFFER_BYTE_LENGTH, &data, &ab));

    bool beforeDetach = true;
    NAPI_CALL(env, napi_is_detached_arraybuffer(env, ab, &beforeDetach));
    SetNamedBool(env, result, "notDetachedBefore", !beforeDetach);

    NAPI_CALL(env, napi_detach_arraybuffer(env, ab));
    bool afterDetach = false;
    NAPI_CALL(env, napi_is_detached_arraybuffer(env, ab, &afterDetach));
    SetNamedBool(env, result, "detachedAfter", afterDetach);

    return result;
}

// ---------------------------------------------------------------------------
// Test 42: Comprehensive typeof matrix
// ---------------------------------------------------------------------------
static napi_value TestTypeofMatrix(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_value undef = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undef));
    SetNamedBool(env, result, "undefinedOk", CheckTypeOf(env, undef, napi_undefined));

    napi_value nullVal = nullptr;
    NAPI_CALL(env, napi_get_null(env, &nullVal));
    SetNamedBool(env, result, "nullOk", CheckTypeOf(env, nullVal, napi_null));

    napi_value boolVal = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, false, &boolVal));
    SetNamedBool(env, result, "boolOk", CheckTypeOf(env, boolVal, napi_boolean));

    napi_value numVal = nullptr;
    NAPI_CALL(env, napi_create_double(env, DOUBLE_TEST_VALUE, &numVal));
    SetNamedBool(env, result, "numberOk", CheckTypeOf(env, numVal, napi_number));

    napi_value strVal = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "matrix", NAPI_AUTO_LENGTH, &strVal));
    SetNamedBool(env, result, "stringOk", CheckTypeOf(env, strVal, napi_string));

    napi_value objVal = nullptr;
    NAPI_CALL(env, napi_create_object(env, &objVal));
    SetNamedBool(env, result, "objectOk", CheckTypeOf(env, objVal, napi_object));

    napi_value fnVal = nullptr;
    NAPI_CALL(env, napi_create_function(env, "fn", NAPI_AUTO_LENGTH, DummyCallback, nullptr, &fnVal));
    SetNamedBool(env, result, "functionOk", CheckTypeOf(env, fnVal, napi_function));

    return result;
}

// ---------------------------------------------------------------------------
// Test 43: Number edge cases – zero and negative zero
// ---------------------------------------------------------------------------
static napi_value TestNumberZero(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_value zero = nullptr;
    NAPI_CALL(env, napi_create_double(env, DOUBLE_ZERO, &zero));
    double zeroVal = -1.0;
    NAPI_CALL(env, napi_get_value_double(env, zero, &zeroVal));
    SetNamedBool(env, result, "zeroOk", zeroVal == DOUBLE_ZERO);

    napi_value negZero = nullptr;
    NAPI_CALL(env, napi_create_double(env, DOUBLE_NEGATIVE_ZERO, &negZero));
    double negZeroVal = 1.0;
    NAPI_CALL(env, napi_get_value_double(env, negZero, &negZeroVal));
    SetNamedBool(env, result, "negZeroIsZero", negZeroVal == DOUBLE_ZERO);
    SetNamedBool(env, result, "negZeroSign", std::signbit(negZeroVal));

    return result;
}

// ---------------------------------------------------------------------------
// Test 44: Number edge cases – Infinity and -Infinity
// ---------------------------------------------------------------------------
static napi_value TestNumberInfinity(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_value posInf = nullptr;
    NAPI_CALL(env, napi_create_double(env, INFINITY, &posInf));
    double posInfVal = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_value_double(env, posInf, &posInfVal));
    SetNamedBool(env, result, "posInfOk", std::isinf(posInfVal) && posInfVal > DOUBLE_ZERO);

    napi_value negInf = nullptr;
    NAPI_CALL(env, napi_create_double(env, -INFINITY, &negInf));
    double negInfVal = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_value_double(env, negInf, &negInfVal));
    SetNamedBool(env, result, "negInfOk", std::isinf(negInfVal) && negInfVal < DOUBLE_ZERO);

    SetNamedBool(env, result, "bothAreNumbers", CheckTypeOf(env, posInf, napi_number) &&
                                                 CheckTypeOf(env, negInf, napi_number));
    return result;
}

// ---------------------------------------------------------------------------
// Test 45: Number edge cases – NaN
// ---------------------------------------------------------------------------
static napi_value TestNumberNan(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value nanVal = nullptr;
    NAPI_CALL(env, napi_create_double(env, NAN, &nanVal));
    double retrieved = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_value_double(env, nanVal, &retrieved));
    SetNamedBool(env, result, "isNan", std::isnan(retrieved));
    SetNamedBool(env, result, "isNumber", CheckTypeOf(env, nanVal, napi_number));
    return result;
}

// ---------------------------------------------------------------------------
// Test 46: Number edge cases – MAX_SAFE_INTEGER / MIN_SAFE_INTEGER
// ---------------------------------------------------------------------------
static napi_value TestNumberSafeIntegers(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);

    napi_value maxSafe = nullptr;
    NAPI_CALL(env, napi_create_double(env, DOUBLE_MAX_SAFE_INTEGER, &maxSafe));
    double maxVal = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_value_double(env, maxSafe, &maxVal));
    SetNamedBool(env, result, "maxSafeOk", maxVal == DOUBLE_MAX_SAFE_INTEGER);

    napi_value minSafe = nullptr;
    NAPI_CALL(env, napi_create_double(env, DOUBLE_MIN_SAFE_INTEGER, &minSafe));
    double minVal = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_value_double(env, minSafe, &minVal));
    SetNamedBool(env, result, "minSafeOk", minVal == DOUBLE_MIN_SAFE_INTEGER);

    return result;
}

// ---------------------------------------------------------------------------
// Test 47: napi_get_undefined and typeof
// ---------------------------------------------------------------------------
static napi_value TestGetUndefined(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value undef = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undef));
    SetNamedBool(env, result, "notNull", undef != nullptr);
    SetNamedBool(env, result, "typeofUndefined", CheckTypeOf(env, undef, napi_undefined));
    return result;
}

// ---------------------------------------------------------------------------
// Test 48: napi_get_null and typeof
// ---------------------------------------------------------------------------
static napi_value TestGetNull(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value nullVal = nullptr;
    NAPI_CALL(env, napi_get_null(env, &nullVal));
    SetNamedBool(env, result, "notNull", nullVal != nullptr);
    SetNamedBool(env, result, "typeofNull", CheckTypeOf(env, nullVal, napi_null));
    return result;
}

// ---------------------------------------------------------------------------
// Module descriptor table (UPPER_CASE for global constant array)
// ---------------------------------------------------------------------------
struct TestEntry {
    const char* name;
    napi_callback callback;
};

static const TestEntry VALUE_CHECK_TESTS[] = {
    { "testTypeofUndefined",         TestTypeofUndefined },
    { "testTypeofNull",              TestTypeofNull },
    { "testTypeofBoolean",           TestTypeofBoolean },
    { "testTypeofNumber",            TestTypeofNumber },
    { "testTypeofString",            TestTypeofString },
    { "testTypeofSymbol",            TestTypeofSymbol },
    { "testTypeofObject",            TestTypeofObject },
    { "testTypeofFunction",          TestTypeofFunction },
    { "testTypeofExternal",          TestTypeofExternal },
    { "testTypeofBigint",            TestTypeofBigint },
    { "testIsArrayPositive",         TestIsArrayPositive },
    { "testIsArrayNegative",         TestIsArrayNegative },
    { "testIsArraybufferPositive",   TestIsArraybufferPositive },
    { "testIsArraybufferNegative",   TestIsArraybufferNegative },
    { "testIsBufferPositive",        TestIsBufferPositive },
    { "testIsBufferNegative",        TestIsBufferNegative },
    { "testIsDatePositive",          TestIsDatePositive },
    { "testIsDateNegative",          TestIsDateNegative },
    { "testIsErrorPositive",         TestIsErrorPositive },
    { "testIsErrorNegative",         TestIsErrorNegative },
    { "testIsTypedarrayPositive",    TestIsTypedarrayPositive },
    { "testIsTypedarrayNegative",    TestIsTypedarrayNegative },
    { "testIsDataviewPositive",      TestIsDataviewPositive },
    { "testIsDataviewNegative",      TestIsDataviewNegative },
    { "testIsPromisePositive",       TestIsPromisePositive },
    { "testIsPromiseNegative",       TestIsPromiseNegative },
    { "testStrictEqualsSameValue",   TestStrictEqualsSameValue },
    { "testStrictEqualsDiffValue",   TestStrictEqualsDiffValue },
    { "testStrictEqualsDiffTypes",   TestStrictEqualsDiffTypes },
    { "testStrictEqualsNullUndef",   TestStrictEqualsNullUndefined },
    { "testStrictEqualsNanNan",      TestStrictEqualsNanNan },
    { "testGetValueBoolTrue",        TestGetValueBoolTrue },
    { "testGetValueBoolFalse",       TestGetValueBoolFalse },
    { "testGetValueInt32",           TestGetValueInt32 },
    { "testGetValueUint32",          TestGetValueUint32 },
    { "testGetValueInt64",           TestGetValueInt64 },
    { "testGetValueDouble",          TestGetValueDouble },
    { "testIsObject",                TestIsObject },
    { "testCoerceToString",          TestCoerceToString },
    { "testInstanceof",              TestInstanceof },
    { "testIsDetachedArraybuffer",   TestIsDetachedArraybuffer },
    { "testTypeofMatrix",            TestTypeofMatrix },
    { "testNumberZero",              TestNumberZero },
    { "testNumberInfinity",          TestNumberInfinity },
    { "testNumberNan",               TestNumberNan },
    { "testNumberSafeIntegers",      TestNumberSafeIntegers },
    { "testGetUndefined",            TestGetUndefined },
    { "testGetNull",                 TestGetNull },
};

static constexpr size_t VALUE_CHECK_TEST_COUNT = sizeof(VALUE_CHECK_TESTS) / sizeof(VALUE_CHECK_TESTS[0]);

}  // namespace

// ---------------------------------------------------------------------------
// Module initialization
// ---------------------------------------------------------------------------
static napi_value InitValueCheckSuite(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[VALUE_CHECK_TEST_COUNT];
    for (size_t i = 0; i < VALUE_CHECK_TEST_COUNT; i++) {
        descriptors[i] = napi_property_descriptor{
            VALUE_CHECK_TESTS[i].name, nullptr, VALUE_CHECK_TESTS[i].callback,
            nullptr, nullptr, nullptr, napi_default, nullptr
        };
    }
    NAPI_CALL(env, napi_define_properties(env, exports, VALUE_CHECK_TEST_COUNT, descriptors));
    return exports;
}

static napi_module g_valueCheckSuiteModule = {
    .nm_version = MODULE_VERSION,
    .nm_flags = NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitValueCheckSuite,
    .nm_modname = "value_check_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterValueCheckSuiteModule(void)
{
    napi_module_register(&g_valueCheckSuiteModule);
}
