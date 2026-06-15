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

#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

namespace {

// ==================== 常量定义 ====================
static constexpr size_t ARG_COUNT_ZERO = 0;
static constexpr size_t ARG_COUNT_ONE = 1;
static constexpr size_t ARG_COUNT_TWO = 2;

static constexpr int32_t INT_VALUE_ZERO = 0;
static constexpr int32_t INT_VALUE_ONE = 1;
static constexpr int32_t INT_VALUE_TWO = 2;
static constexpr int32_t INT_VALUE_THREE = 3;
static constexpr int32_t INT_VALUE_FIVE = 5;
static constexpr int32_t INT_VALUE_TEN = 10;
static constexpr int32_t INT_VALUE_FORTY_TWO = 42;
static constexpr int32_t INT_VALUE_HUNDRED = 100;
static constexpr int32_t INT_VALUE_NEGATIVE = -99;

static constexpr double DOUBLE_PI = 3.14;
static constexpr double DOUBLE_EULER = 2.718;
static constexpr double DOUBLE_ZERO = 0.0;
static constexpr double DOUBLE_NEGATIVE = -1.5;
static constexpr double DOUBLE_LARGE = 1.0e15;
static constexpr double DATE_TIMESTAMP = 1717027200000.0;
static constexpr double DATE_TIMESTAMP_TWO = 1000000000000.0;
static constexpr double DATE_EPOCH = 0.0;
static constexpr double DATE_NEGATIVE = -86400000.0;

static constexpr int64_t BIGINT_POSITIVE = 9007199254740992LL;
static constexpr int64_t BIGINT_NEGATIVE = -9007199254740992LL;
static constexpr int64_t BIGINT_ZERO = 0LL;
static constexpr int64_t BIGINT_ONE = 1LL;
static constexpr int64_t BIGINT_MAX = 9223372036854775807LL;

static constexpr uint64_t BIGINT_UINT_LARGE = 18446744073709551615ULL;
static constexpr uint64_t BIGINT_UINT_ZERO = 0ULL;
static constexpr uint64_t BIGINT_UINT_ONE = 1ULL;

static constexpr size_t SYMBOL_DESC_BUF_SIZE = 256;
static constexpr size_t STRING_BUF_SIZE = 64;

static constexpr int32_t EXTERNAL_DATA_MARKER = 12345;

// ==================== 辅助结构体与回调 ====================
struct ExternalData {
    int32_t marker;
};

static void ExternalFinalizer(napi_env /* env */, void* data, void* /* hint */)
{
    auto* ext = static_cast<ExternalData*>(data);
    delete ext;
}

// ==================== 辅助函数 ====================
static napi_value CreateResult(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    return result;
}

static void SetNamedBool(napi_env env, napi_value obj, const char* key, bool val)
{
    napi_value boolVal = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, val, &boolVal));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, obj, key, boolVal));
}

static void SetNamedInt32(napi_env env, napi_value obj, const char* key, int32_t val)
{
    napi_value numVal = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, val, &numVal));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, obj, key, numVal));
}

static void SetNamedDouble(napi_env env, napi_value obj, const char* key, double val)
{
    napi_value numVal = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_create_double(env, val, &numVal));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, obj, key, numVal));
}

static void SetNamedString(napi_env env, napi_value obj, const char* key, const char* val)
{
    napi_value strVal = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, val, NAPI_AUTO_LENGTH, &strVal));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, obj, key, strVal));
}

// ==================== 全局对象测试 ====================
// Test 01: 获取全局对象
static napi_value TestGetGlobal(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));
    napi_valuetype globalType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, global, &globalType));
    SetNamedBool(env, result, "globalIsObject", globalType == napi_object);
    SetNamedString(env, result, "testName", "TestGetGlobal");
    return result;
}

// Test 02: 全局对象上设置和获取属性
static napi_value TestGlobalSetGetProperty(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT_VALUE_FORTY_TWO, &val));
    NAPI_CALL(env, napi_set_named_property(env, global, "__globalTestProp", val));
    napi_value got = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "__globalTestProp", &got));
    int32_t gotVal = INT_VALUE_ZERO;
    NAPI_CALL(env, napi_get_value_int32(env, got, &gotVal));
    SetNamedBool(env, result, "valueMatches", gotVal == INT_VALUE_FORTY_TWO);
    SetNamedString(env, result, "testName", "TestGlobalSetGetProperty");
    return result;
}

// Test 03: 获取 undefined
static napi_value TestGetUndefined(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value undef = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undef));
    napi_valuetype type = napi_object;
    NAPI_CALL(env, napi_typeof(env, undef, &type));
    SetNamedBool(env, result, "isUndefined", type == napi_undefined);
    SetNamedString(env, result, "testName", "TestGetUndefined");
    return result;
}

// Test 04: 获取 null
static napi_value TestGetNull(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value nullVal = nullptr;
    NAPI_CALL(env, napi_get_null(env, &nullVal));
    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, nullVal, &type));
    SetNamedBool(env, result, "isNull", type == napi_null);
    SetNamedString(env, result, "testName", "TestGetNull");
    return result;
}

// Test 05: 获取 boolean true/false
static napi_value TestGetBoolean(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value trueVal = nullptr;
    napi_value falseVal = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, true, &trueVal));
    NAPI_CALL(env, napi_get_boolean(env, false, &falseVal));
    bool gotTrue = false;
    bool gotFalse = true;
    NAPI_CALL(env, napi_get_value_bool(env, trueVal, &gotTrue));
    NAPI_CALL(env, napi_get_value_bool(env, falseVal, &gotFalse));
    SetNamedBool(env, result, "trueCorrect", gotTrue == true);
    SetNamedBool(env, result, "falseCorrect", gotFalse == false);
    SetNamedString(env, result, "testName", "TestGetBoolean");
    return result;
}

// ==================== Symbol 测试 ====================
// Test 06: 创建无描述的 Symbol
static napi_value TestCreateSymbolNoDesc(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value sym = nullptr;
    NAPI_CALL(env, napi_create_symbol(env, nullptr, &sym));
    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, sym, &type));
    SetNamedBool(env, result, "isSymbol", type == napi_symbol);
    SetNamedString(env, result, "testName", "TestCreateSymbolNoDesc");
    return result;
}

// Test 07: 创建有描述的 Symbol
static napi_value TestCreateSymbolWithDesc(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value desc = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "mySymbol", NAPI_AUTO_LENGTH, &desc));
    napi_value sym = nullptr;
    NAPI_CALL(env, napi_create_symbol(env, desc, &sym));
    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, sym, &type));
    SetNamedBool(env, result, "isSymbol", type == napi_symbol);
    SetNamedString(env, result, "testName", "TestCreateSymbolWithDesc");
    return result;
}

// Test 08: Symbol 作为属性 key
static napi_value TestSymbolAsPropertyKey(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value sym = nullptr;
    NAPI_CALL(env, napi_create_symbol(env, nullptr, &sym));
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT_VALUE_TEN, &val));
    NAPI_CALL(env, napi_set_property(env, obj, sym, val));
    napi_value got = nullptr;
    NAPI_CALL(env, napi_get_property(env, obj, sym, &got));
    int32_t gotVal = INT_VALUE_ZERO;
    NAPI_CALL(env, napi_get_value_int32(env, got, &gotVal));
    SetNamedBool(env, result, "symbolKeyWorks", gotVal == INT_VALUE_TEN);
    SetNamedString(env, result, "testName", "TestSymbolAsPropertyKey");
    return result;
}

// Test 09: 两个 Symbol 不相等
static napi_value TestSymbolUniqueness(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value desc = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "same", NAPI_AUTO_LENGTH, &desc));
    napi_value sym1 = nullptr;
    napi_value sym2 = nullptr;
    NAPI_CALL(env, napi_create_symbol(env, desc, &sym1));
    NAPI_CALL(env, napi_create_symbol(env, desc, &sym2));
    bool isEqual = true;
    NAPI_CALL(env, napi_strict_equals(env, sym1, sym2, &isEqual));
    SetNamedBool(env, result, "symbolsUnique", !isEqual);
    SetNamedString(env, result, "testName", "TestSymbolUniqueness");
    return result;
}

// Test 10: Symbol 有 has_property 检查
static napi_value TestSymbolHasProperty(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value sym = nullptr;
    NAPI_CALL(env, napi_create_symbol(env, nullptr, &sym));
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    bool hasBefore = true;
    NAPI_CALL(env, napi_has_property(env, obj, sym, &hasBefore));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT_VALUE_FIVE, &val));
    NAPI_CALL(env, napi_set_property(env, obj, sym, val));
    bool hasAfter = false;
    NAPI_CALL(env, napi_has_property(env, obj, sym, &hasAfter));
    SetNamedBool(env, result, "notBefore", !hasBefore);
    SetNamedBool(env, result, "hasAfter", hasAfter);
    SetNamedString(env, result, "testName", "TestSymbolHasProperty");
    return result;
}

// ==================== Date 测试 ====================
// Test 11: 创建 Date 对象
static napi_value TestCreateDate(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value dateVal = nullptr;
    NAPI_CALL(env, napi_create_date(env, DATE_TIMESTAMP, &dateVal));
    bool isDate = false;
    NAPI_CALL(env, napi_is_date(env, dateVal, &isDate));
    SetNamedBool(env, result, "isDate", isDate);
    SetNamedString(env, result, "testName", "TestCreateDate");
    return result;
}

// Test 12: 获取 Date 的时间戳值
static napi_value TestGetDateValue(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value dateVal = nullptr;
    NAPI_CALL(env, napi_create_date(env, DATE_TIMESTAMP, &dateVal));
    double timestamp = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_date_value(env, dateVal, &timestamp));
    SetNamedBool(env, result, "timestampMatches", timestamp == DATE_TIMESTAMP);
    SetNamedDouble(env, result, "timestamp", timestamp);
    SetNamedString(env, result, "testName", "TestGetDateValue");
    return result;
}

// Test 13: Date epoch (时间戳为0)
static napi_value TestDateEpoch(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value dateVal = nullptr;
    NAPI_CALL(env, napi_create_date(env, DATE_EPOCH, &dateVal));
    double timestamp = DOUBLE_NEGATIVE;
    NAPI_CALL(env, napi_get_date_value(env, dateVal, &timestamp));
    SetNamedBool(env, result, "isEpoch", timestamp == DATE_EPOCH);
    SetNamedString(env, result, "testName", "TestDateEpoch");
    return result;
}

// Test 14: 负时间戳的 Date
static napi_value TestDateNegativeTimestamp(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value dateVal = nullptr;
    NAPI_CALL(env, napi_create_date(env, DATE_NEGATIVE, &dateVal));
    double timestamp = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_date_value(env, dateVal, &timestamp));
    SetNamedBool(env, result, "negativeOk", timestamp == DATE_NEGATIVE);
    SetNamedString(env, result, "testName", "TestDateNegativeTimestamp");
    return result;
}

// Test 15: 非 Date 对象的 is_date 检查
static napi_value TestIsDateOnNonDate(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    bool isDate = true;
    NAPI_CALL(env, napi_is_date(env, obj, &isDate));
    napi_value num = nullptr;
    NAPI_CALL(env, napi_create_double(env, DATE_TIMESTAMP, &num));
    bool numIsDate = true;
    NAPI_CALL(env, napi_is_date(env, num, &numIsDate));
    SetNamedBool(env, result, "objNotDate", !isDate);
    SetNamedBool(env, result, "numNotDate", !numIsDate);
    SetNamedString(env, result, "testName", "TestIsDateOnNonDate");
    return result;
}

// ==================== BigInt 测试 ====================
// Test 16: 创建 int64 BigInt
static napi_value TestCreateBigintInt64(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value bigint = nullptr;
    NAPI_CALL(env, napi_create_bigint_int64(env, BIGINT_POSITIVE, &bigint));
    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, bigint, &type));
    SetNamedBool(env, result, "isBigint", type == napi_bigint);
    SetNamedString(env, result, "testName", "TestCreateBigintInt64");
    return result;
}

// Test 17: int64 BigInt 往返
static napi_value TestBigintInt64Roundtrip(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value bigint = nullptr;
    NAPI_CALL(env, napi_create_bigint_int64(env, BIGINT_POSITIVE, &bigint));
    int64_t gotVal = BIGINT_ZERO;
    bool lossless = false;
    NAPI_CALL(env, napi_get_value_bigint_int64(env, bigint, &gotVal, &lossless));
    SetNamedBool(env, result, "valueMatches", gotVal == BIGINT_POSITIVE);
    SetNamedBool(env, result, "lossless", lossless);
    SetNamedString(env, result, "testName", "TestBigintInt64Roundtrip");
    return result;
}

// Test 18: 负值 int64 BigInt
static napi_value TestBigintNegativeInt64(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value bigint = nullptr;
    NAPI_CALL(env, napi_create_bigint_int64(env, BIGINT_NEGATIVE, &bigint));
    int64_t gotVal = BIGINT_ZERO;
    bool lossless = false;
    NAPI_CALL(env, napi_get_value_bigint_int64(env, bigint, &gotVal, &lossless));
    SetNamedBool(env, result, "valueMatches", gotVal == BIGINT_NEGATIVE);
    SetNamedBool(env, result, "lossless", lossless);
    SetNamedString(env, result, "testName", "TestBigintNegativeInt64");
    return result;
}

// Test 19: uint64 BigInt
static napi_value TestCreateBigintUint64(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value bigint = nullptr;
    NAPI_CALL(env, napi_create_bigint_uint64(env, BIGINT_UINT_LARGE, &bigint));
    uint64_t gotVal = BIGINT_UINT_ZERO;
    bool lossless = false;
    NAPI_CALL(env, napi_get_value_bigint_uint64(env, bigint, &gotVal, &lossless));
    SetNamedBool(env, result, "valueMatches", gotVal == BIGINT_UINT_LARGE);
    SetNamedBool(env, result, "lossless", lossless);
    SetNamedString(env, result, "testName", "TestCreateBigintUint64");
    return result;
}

// Test 20: BigInt 零值
static napi_value TestBigintZero(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value bigint = nullptr;
    NAPI_CALL(env, napi_create_bigint_int64(env, BIGINT_ZERO, &bigint));
    int64_t gotVal = BIGINT_ONE;
    bool lossless = false;
    NAPI_CALL(env, napi_get_value_bigint_int64(env, bigint, &gotVal, &lossless));
    SetNamedBool(env, result, "isZero", gotVal == BIGINT_ZERO);
    SetNamedBool(env, result, "lossless", lossless);
    SetNamedString(env, result, "testName", "TestBigintZero");
    return result;
}

// Test 21: int64 最大值 BigInt
static napi_value TestBigintMaxInt64(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value bigint = nullptr;
    NAPI_CALL(env, napi_create_bigint_int64(env, BIGINT_MAX, &bigint));
    int64_t gotVal = BIGINT_ZERO;
    bool lossless = false;
    NAPI_CALL(env, napi_get_value_bigint_int64(env, bigint, &gotVal, &lossless));
    SetNamedBool(env, result, "valueMatches", gotVal == BIGINT_MAX);
    SetNamedBool(env, result, "lossless", lossless);
    SetNamedString(env, result, "testName", "TestBigintMaxInt64");
    return result;
}

// Test 22: uint64 BigInt 零值和一
static napi_value TestBigintUint64Boundary(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value zeroVal = nullptr;
    NAPI_CALL(env, napi_create_bigint_uint64(env, BIGINT_UINT_ZERO, &zeroVal));
    uint64_t gotZero = BIGINT_UINT_ONE;
    bool losslessZero = false;
    NAPI_CALL(env, napi_get_value_bigint_uint64(env, zeroVal, &gotZero, &losslessZero));
    napi_value oneVal = nullptr;
    NAPI_CALL(env, napi_create_bigint_uint64(env, BIGINT_UINT_ONE, &oneVal));
    uint64_t gotOne = BIGINT_UINT_ZERO;
    bool losslessOne = false;
    NAPI_CALL(env, napi_get_value_bigint_uint64(env, oneVal, &gotOne, &losslessOne));
    SetNamedBool(env, result, "zeroOk", gotZero == BIGINT_UINT_ZERO && losslessZero);
    SetNamedBool(env, result, "oneOk", gotOne == BIGINT_UINT_ONE && losslessOne);
    SetNamedString(env, result, "testName", "TestBigintUint64Boundary");
    return result;
}

// ==================== Promise 测试 ====================
// Test 23: 创建 Promise
static napi_value TestCreatePromise(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    bool isPromise = false;
    NAPI_CALL(env, napi_is_promise(env, promise, &isPromise));
    SetNamedBool(env, result, "isPromise", isPromise);
    SetNamedString(env, result, "testName", "TestCreatePromise");
    // 解析以避免未处理的 promise
    napi_value undef = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undef));
    NAPI_CALL(env, napi_resolve_deferred(env, deferred, undef));
    return result;
}

// Test 24: resolve Promise
static napi_value TestResolvePromise(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    napi_value resolveVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT_VALUE_FORTY_TWO, &resolveVal));
    NAPI_CALL(env, napi_resolve_deferred(env, deferred, resolveVal));
    SetNamedBool(env, result, "resolved", true);
    SetNamedString(env, result, "testName", "TestResolvePromise");
    return result;
}

// Test 25: reject Promise
static napi_value TestRejectPromise(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    napi_value errMsg = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "TestError", NAPI_AUTO_LENGTH, &errMsg));
    napi_value errObj = nullptr;
    NAPI_CALL(env, napi_create_error(env, nullptr, errMsg, &errObj));
    NAPI_CALL(env, napi_reject_deferred(env, deferred, errObj));
    SetNamedBool(env, result, "rejected", true);
    SetNamedString(env, result, "testName", "TestRejectPromise");
    return result;
}

// Test 26: 非 Promise 的 is_promise 检查
static napi_value TestIsPromiseOnNonPromise(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    bool objIsPromise = true;
    NAPI_CALL(env, napi_is_promise(env, obj, &objIsPromise));
    napi_value num = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT_VALUE_ONE, &num));
    bool numIsPromise = true;
    NAPI_CALL(env, napi_is_promise(env, num, &numIsPromise));
    SetNamedBool(env, result, "objNotPromise", !objIsPromise);
    SetNamedBool(env, result, "numNotPromise", !numIsPromise);
    SetNamedString(env, result, "testName", "TestIsPromiseOnNonPromise");
    return result;
}

// Test 27: 多个独立 Promise
static napi_value TestMultiplePromises(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_deferred d1 = nullptr;
    napi_value p1 = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &d1, &p1));
    napi_deferred d2 = nullptr;
    napi_value p2 = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &d2, &p2));
    bool isP1 = false;
    bool isP2 = false;
    NAPI_CALL(env, napi_is_promise(env, p1, &isP1));
    NAPI_CALL(env, napi_is_promise(env, p2, &isP2));
    bool notEqual = true;
    NAPI_CALL(env, napi_strict_equals(env, p1, p2, &notEqual));
    SetNamedBool(env, result, "bothPromises", isP1 && isP2);
    SetNamedBool(env, result, "different", !notEqual);
    napi_value undef = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undef));
    NAPI_CALL(env, napi_resolve_deferred(env, d1, undef));
    NAPI_CALL(env, napi_resolve_deferred(env, d2, undef));
    SetNamedString(env, result, "testName", "TestMultiplePromises");
    return result;
}

// ==================== External 测试 ====================
// Test 28: 创建 External 值
static napi_value TestCreateExternal(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    auto* data = new ExternalData { EXTERNAL_DATA_MARKER };
    napi_value external = nullptr;
    NAPI_CALL(env, napi_create_external(env, data, ExternalFinalizer, nullptr, &external));
    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, external, &type));
    SetNamedBool(env, result, "isExternal", type == napi_external);
    SetNamedString(env, result, "testName", "TestCreateExternal");
    return result;
}

// Test 29: External 数据往返
static napi_value TestExternalRoundtrip(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    auto* data = new ExternalData { EXTERNAL_DATA_MARKER };
    napi_value external = nullptr;
    NAPI_CALL(env, napi_create_external(env, data, ExternalFinalizer, nullptr, &external));
    void* gotData = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, external, &gotData));
    auto* gotExt = static_cast<ExternalData*>(gotData);
    SetNamedBool(env, result, "dataValid", gotExt != nullptr);
    SetNamedBool(env, result, "markerMatches", gotExt->marker == EXTERNAL_DATA_MARKER);
    SetNamedString(env, result, "testName", "TestExternalRoundtrip");
    return result;
}

// Test 30: External 的 typeof 和 is 检查
static napi_value TestExternalTypeChecks(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    auto* data = new ExternalData { INT_VALUE_ONE };
    napi_value external = nullptr;
    NAPI_CALL(env, napi_create_external(env, data, ExternalFinalizer, nullptr, &external));
    bool isArray = true;
    NAPI_CALL(env, napi_is_array(env, external, &isArray));
    bool isPromise = true;
    NAPI_CALL(env, napi_is_promise(env, external, &isPromise));
    SetNamedBool(env, result, "notArray", !isArray);
    SetNamedBool(env, result, "notPromise", !isPromise);
    SetNamedString(env, result, "testName", "TestExternalTypeChecks");
    return result;
}

// ==================== 数值边界测试 ====================
// Test 31: 整数创建和获取
static napi_value TestIntRoundtrip(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT_VALUE_NEGATIVE, &val));
    int32_t got = INT_VALUE_ZERO;
    NAPI_CALL(env, napi_get_value_int32(env, val, &got));
    SetNamedBool(env, result, "negativeOk", got == INT_VALUE_NEGATIVE);
    SetNamedInt32(env, result, "value", got);
    SetNamedString(env, result, "testName", "TestIntRoundtrip");
    return result;
}

// Test 32: double 创建和获取
static napi_value TestDoubleRoundtrip(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_double(env, DOUBLE_PI, &val));
    double got = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_value_double(env, val, &got));
    SetNamedBool(env, result, "piOk", got == DOUBLE_PI);
    SetNamedDouble(env, result, "value", got);
    SetNamedString(env, result, "testName", "TestDoubleRoundtrip");
    return result;
}

// Test 33: 特殊浮点值 NaN
static napi_value TestNanValue(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value nanVal = nullptr;
    NAPI_CALL(env, napi_create_double(env, NAN, &nanVal));
    double got = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_value_double(env, nanVal, &got));
    SetNamedBool(env, result, "isNan", std::isnan(got));
    SetNamedString(env, result, "testName", "TestNanValue");
    return result;
}

// Test 34: Infinity 值
static napi_value TestInfinityValue(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value infVal = nullptr;
    NAPI_CALL(env, napi_create_double(env, INFINITY, &infVal));
    double got = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_value_double(env, infVal, &got));
    SetNamedBool(env, result, "isInf", std::isinf(got));
    SetNamedBool(env, result, "isPositive", got > DOUBLE_ZERO);
    SetNamedString(env, result, "testName", "TestInfinityValue");
    return result;
}

// Test 35: 负 Infinity
static napi_value TestNegativeInfinity(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value negInfVal = nullptr;
    NAPI_CALL(env, napi_create_double(env, -INFINITY, &negInfVal));
    double got = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_value_double(env, negInfVal, &got));
    SetNamedBool(env, result, "isInf", std::isinf(got));
    SetNamedBool(env, result, "isNegative", got < DOUBLE_ZERO);
    SetNamedString(env, result, "testName", "TestNegativeInfinity");
    return result;
}

// ==================== 类型强制转换测试 ====================
// Test 36: number 强制转 string
static napi_value TestCoerceNumberToString(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value num = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT_VALUE_FORTY_TWO, &num));
    napi_value strVal = nullptr;
    NAPI_CALL(env, napi_coerce_to_string(env, num, &strVal));
    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, strVal, &type));
    char buf[STRING_BUF_SIZE] = { 0 };
    size_t len = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, strVal, buf, STRING_BUF_SIZE, &len));
    SetNamedBool(env, result, "isString", type == napi_string);
    SetNamedString(env, result, "value", buf);
    SetNamedString(env, result, "testName", "TestCoerceNumberToString");
    return result;
}

// Test 37: boolean 强制转 number
static napi_value TestCoerceBoolToNumber(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value trueVal = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, true, &trueVal));
    napi_value numFromTrue = nullptr;
    NAPI_CALL(env, napi_coerce_to_number(env, trueVal, &numFromTrue));
    double gotTrue = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_value_double(env, numFromTrue, &gotTrue));
    napi_value falseVal = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, false, &falseVal));
    napi_value numFromFalse = nullptr;
    NAPI_CALL(env, napi_coerce_to_number(env, falseVal, &numFromFalse));
    double gotFalse = DOUBLE_NEGATIVE;
    NAPI_CALL(env, napi_get_value_double(env, numFromFalse, &gotFalse));
    SetNamedBool(env, result, "trueIsOne", gotTrue == 1.0);
    SetNamedBool(env, result, "falseIsZero", gotFalse == DOUBLE_ZERO);
    SetNamedString(env, result, "testName", "TestCoerceBoolToNumber");
    return result;
}

// Test 38: 强制转 object
static napi_value TestCoerceToObject(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value num = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT_VALUE_HUNDRED, &num));
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_coerce_to_object(env, num, &obj));
    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, obj, &type));
    SetNamedBool(env, result, "isObject", type == napi_object);
    SetNamedString(env, result, "testName", "TestCoerceToObject");
    return result;
}

// Test 39: 强制转 bool
static napi_value TestCoerceToBool(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value num = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT_VALUE_ONE, &num));
    napi_value boolVal = nullptr;
    NAPI_CALL(env, napi_coerce_to_bool(env, num, &boolVal));
    bool got = false;
    NAPI_CALL(env, napi_get_value_bool(env, boolVal, &got));
    napi_value zero = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT_VALUE_ZERO, &zero));
    napi_value boolFromZero = nullptr;
    NAPI_CALL(env, napi_coerce_to_bool(env, zero, &boolFromZero));
    bool gotZero = true;
    NAPI_CALL(env, napi_get_value_bool(env, boolFromZero, &gotZero));
    SetNamedBool(env, result, "oneIsTrue", got);
    SetNamedBool(env, result, "zeroIsFalse", !gotZero);
    SetNamedString(env, result, "testName", "TestCoerceToBool");
    return result;
}

// ==================== strict_equals 和 typeof 测试 ====================
// Test 40: strict_equals 同值同类型
static napi_value TestStrictEqualsSameValue(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value a = nullptr;
    napi_value b = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT_VALUE_FIVE, &a));
    NAPI_CALL(env, napi_create_int32(env, INT_VALUE_FIVE, &b));
    bool isEqual = false;
    NAPI_CALL(env, napi_strict_equals(env, a, b, &isEqual));
    SetNamedBool(env, result, "equal", isEqual);
    SetNamedString(env, result, "testName", "TestStrictEqualsSameValue");
    return result;
}

// Test 41: strict_equals 不同值
static napi_value TestStrictEqualsDiffValue(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value a = nullptr;
    napi_value b = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT_VALUE_ONE, &a));
    NAPI_CALL(env, napi_create_int32(env, INT_VALUE_TWO, &b));
    bool isEqual = true;
    NAPI_CALL(env, napi_strict_equals(env, a, b, &isEqual));
    SetNamedBool(env, result, "notEqual", !isEqual);
    SetNamedString(env, result, "testName", "TestStrictEqualsDiffValue");
    return result;
}

// Test 42: strict_equals null vs undefined
static napi_value TestStrictEqualsNullUndef(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value nullVal = nullptr;
    napi_value undefVal = nullptr;
    NAPI_CALL(env, napi_get_null(env, &nullVal));
    NAPI_CALL(env, napi_get_undefined(env, &undefVal));
    bool isEqual = true;
    NAPI_CALL(env, napi_strict_equals(env, nullVal, undefVal, &isEqual));
    SetNamedBool(env, result, "notStrictEqual", !isEqual);
    SetNamedString(env, result, "testName", "TestStrictEqualsNullUndef");
    return result;
}

// Test 43: 多个 Date 对象比较
static napi_value TestDateComparison(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value d1 = nullptr;
    napi_value d2 = nullptr;
    NAPI_CALL(env, napi_create_date(env, DATE_TIMESTAMP, &d1));
    NAPI_CALL(env, napi_create_date(env, DATE_TIMESTAMP_TWO, &d2));
    double ts1 = DOUBLE_ZERO;
    double ts2 = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_date_value(env, d1, &ts1));
    NAPI_CALL(env, napi_get_date_value(env, d2, &ts2));
    SetNamedBool(env, result, "d1Correct", ts1 == DATE_TIMESTAMP);
    SetNamedBool(env, result, "d2Correct", ts2 == DATE_TIMESTAMP_TWO);
    SetNamedBool(env, result, "different", ts1 != ts2);
    SetNamedString(env, result, "testName", "TestDateComparison");
    return result;
}

// Test 44: Symbol 删除属性
static napi_value TestSymbolDeleteProperty(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value sym = nullptr;
    NAPI_CALL(env, napi_create_symbol(env, nullptr, &sym));
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT_VALUE_THREE, &val));
    NAPI_CALL(env, napi_set_property(env, obj, sym, val));
    bool hasBefore = false;
    NAPI_CALL(env, napi_has_property(env, obj, sym, &hasBefore));
    bool deleted = false;
    NAPI_CALL(env, napi_delete_property(env, obj, sym, &deleted));
    bool hasAfter = true;
    NAPI_CALL(env, napi_has_property(env, obj, sym, &hasAfter));
    SetNamedBool(env, result, "hadBefore", hasBefore);
    SetNamedBool(env, result, "wasDeleted", deleted);
    SetNamedBool(env, result, "goneAfter", !hasAfter);
    SetNamedString(env, result, "testName", "TestSymbolDeleteProperty");
    return result;
}

// Test 45: 大数 double 精度检查
static napi_value TestLargeDoubleValue(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_double(env, DOUBLE_LARGE, &val));
    double got = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_value_double(env, val, &got));
    SetNamedBool(env, result, "largeOk", got == DOUBLE_LARGE);
    SetNamedDouble(env, result, "value", got);
    SetNamedString(env, result, "testName", "TestLargeDoubleValue");
    return result;
}

// Test 46: 负 double
static napi_value TestNegativeDouble(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_double(env, DOUBLE_NEGATIVE, &val));
    double got = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_value_double(env, val, &got));
    SetNamedBool(env, result, "negativeOk", got == DOUBLE_NEGATIVE);
    SetNamedString(env, result, "testName", "TestNegativeDouble");
    return result;
}

// Test 47: Euler 常数 double
static napi_value TestEulerDouble(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_double(env, DOUBLE_EULER, &val));
    double got = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_value_double(env, val, &got));
    SetNamedBool(env, result, "eulerOk", got == DOUBLE_EULER);
    SetNamedString(env, result, "testName", "TestEulerDouble");
    return result;
}

// ==================== 测试注册表 ====================
struct TestEntry {
    const char* name;
    napi_callback callback;
};

static const TestEntry GLOBAL_TESTS[] = {
    { "testGetGlobal", TestGetGlobal },
    { "testGlobalSetGetProperty", TestGlobalSetGetProperty },
    { "testGetUndefined", TestGetUndefined },
    { "testGetNull", TestGetNull },
    { "testGetBoolean", TestGetBoolean },
    { "testCreateSymbolNoDesc", TestCreateSymbolNoDesc },
    { "testCreateSymbolWithDesc", TestCreateSymbolWithDesc },
    { "testSymbolAsPropertyKey", TestSymbolAsPropertyKey },
    { "testSymbolUniqueness", TestSymbolUniqueness },
    { "testSymbolHasProperty", TestSymbolHasProperty },
    { "testCreateDate", TestCreateDate },
    { "testGetDateValue", TestGetDateValue },
    { "testDateEpoch", TestDateEpoch },
    { "testDateNegativeTimestamp", TestDateNegativeTimestamp },
    { "testIsDateOnNonDate", TestIsDateOnNonDate },
    { "testCreateBigintInt64", TestCreateBigintInt64 },
    { "testBigintInt64Roundtrip", TestBigintInt64Roundtrip },
    { "testBigintNegativeInt64", TestBigintNegativeInt64 },
    { "testCreateBigintUint64", TestCreateBigintUint64 },
    { "testBigintZero", TestBigintZero },
    { "testBigintMaxInt64", TestBigintMaxInt64 },
    { "testBigintUint64Boundary", TestBigintUint64Boundary },
    { "testCreatePromise", TestCreatePromise },
    { "testResolvePromise", TestResolvePromise },
    { "testRejectPromise", TestRejectPromise },
    { "testIsPromiseOnNonPromise", TestIsPromiseOnNonPromise },
    { "testMultiplePromises", TestMultiplePromises },
    { "testCreateExternal", TestCreateExternal },
    { "testExternalRoundtrip", TestExternalRoundtrip },
    { "testExternalTypeChecks", TestExternalTypeChecks },
    { "testIntRoundtrip", TestIntRoundtrip },
    { "testDoubleRoundtrip", TestDoubleRoundtrip },
    { "testNanValue", TestNanValue },
    { "testInfinityValue", TestInfinityValue },
    { "testNegativeInfinity", TestNegativeInfinity },
    { "testCoerceNumberToString", TestCoerceNumberToString },
    { "testCoerceBoolToNumber", TestCoerceBoolToNumber },
    { "testCoerceToObject", TestCoerceToObject },
    { "testCoerceToBool", TestCoerceToBool },
    { "testStrictEqualsSameValue", TestStrictEqualsSameValue },
    { "testStrictEqualsDiffValue", TestStrictEqualsDiffValue },
    { "testStrictEqualsNullUndef", TestStrictEqualsNullUndef },
    { "testDateComparison", TestDateComparison },
    { "testSymbolDeleteProperty", TestSymbolDeleteProperty },
    { "testLargeDoubleValue", TestLargeDoubleValue },
    { "testNegativeDouble", TestNegativeDouble },
    { "testEulerDouble", TestEulerDouble },
};

static constexpr size_t GLOBAL_TEST_COUNT = sizeof(GLOBAL_TESTS) / sizeof(GLOBAL_TESTS[0]);

// ==================== 模块初始化 ====================
static napi_value GlobalSuiteInit(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[GLOBAL_TEST_COUNT];
    for (size_t i = 0; i < GLOBAL_TEST_COUNT; i++) {
        descriptors[i] = DECLARE_NAPI_FUNCTION(GLOBAL_TESTS[i].name, GLOBAL_TESTS[i].callback);
    }
    NAPI_CALL(env, napi_define_properties(env, exports, GLOBAL_TEST_COUNT, descriptors));
    return exports;
}

} // namespace

static napi_module g_globalSuiteModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = GlobalSuiteInit,
    .nm_modname = "global_suite",
    .nm_priv = nullptr,
    .reserved = { 0 },
};

static void GlobalSuiteRegister() __attribute__((constructor));
static void GlobalSuiteRegister()
{
    napi_module_register(&g_globalSuiteModule);
}
