/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include <cstring>
#include <sstream>
#define private public
#define protected public

#include <chrono>
#include <thread>

#include "ark_native_reference.h"
#include "gtest/gtest.h"
#include "hilog/log.h"
#include "ecmascript/napi/include/jsnapi_expo.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "native_create_env.h"
#include "native_utils.h"
#include "reference_manager/native_reference_manager.h"
#include "securec.h"
#include "test.h"

#include "test_common.h"
#include "utils/log.h"

static constexpr int MAX_BUFFER_SIZE = 2;
static constexpr int BUFFER_SIZE_FIVE = 5;
static constexpr int BUFFER_SIZE_TEN = 10;
static constexpr size_t TEST_STR_LENGTH = 30;
static int g_hookTagcp = 0;
static int g_hookTag = 0;
static int g_hookArgOne = 1;
static int g_hookArgTwo = 2;
static int g_hookArgThree = 3;
static constexpr int INT_ZERO = 0;
static constexpr int INT_ONE = 1;
static constexpr int INT_TWO = 2;
static constexpr int INT_THREE = 3;
static constexpr int INT_FORTYTWO = 42;
static constexpr int INT_HUNDRED = 100;
static constexpr int ASYNC_WORK_NUM = 10;
static constexpr uint64_t TASKID = 0;
static int g_currentExeId = 0;
static int g_currentCptId = 0;

static constexpr double TEST_DOUBLE = 1.1;
static constexpr char TEST_STRING[5] = "test";
static constexpr char TEST_PROP[9] = "property";
static constexpr char TEST_PROP_TEST[11] = "property99";
static constexpr size_t MAX_BYTE_LENGTH = 2097152;
static constexpr int32_t TEST_INT32_MINUS_1 = -1;
static constexpr int32_t TEST_INT32_1 = 1;
static constexpr int32_t TEST_INT32_10 = 10;
static constexpr int32_t TEST_INT32_20 = 20;
static constexpr int32_t TEST_INT32_500 = 500;
static constexpr uint32_t TEST_UINT32_1000 = 1000;
static constexpr int64_t MAX_SAFE_INTEGER = 9007199254740991;
static constexpr int64_t TEST_INT64_NEGATIVE = -987654321;
static constexpr int TEST_NUM = 95;
static constexpr const char TEST_CHAR_STRING[] = "TestString";
static constexpr const char TEST_PROP_STRING_1[] = "prop1";
static constexpr const char TEST_PROP_STRING_2[] = "prop2";
static constexpr const char TEST_CHAR_ASYNCWORK[] = "AsyncWorkTest";
static constexpr const char16_t TEST_CHAR16_STRING[] = u"TestString";
static constexpr const char TEST_CHAR_ERROR_CODE[] = "500";
static constexpr const char TEST_CHAR_ERROR_MESSAGE[] = "Common error";
static constexpr const char TEST_CHAR_TEST_FUNC[] = "testFunc";

class NapiBasicTest : public NativeEngineTest {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "NapiBasicTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "NapiBasicTest TearDownTestCase";
    }

    void SetUp() override
    {
        napi_env env = reinterpret_cast<napi_env>(engine_);
        napi_open_handle_scope(env, &scope_);
    }

    void TearDown() override
    {
        napi_env env = reinterpret_cast<napi_env>(engine_);
        napi_value exception = nullptr;
        napi_get_and_clear_last_exception(env, &exception);
        napi_close_handle_scope(env, scope_);
    }
private:
    napi_handle_scope scope_ = nullptr;
};

static const napi_type_tag typeTags[5] = { // 5:array element size is 5.
    {0xdaf987b3cc62481a, 0xb745b0497f299531},
    {0xbb7936c374084d9b, 0xa9548d0762eeedb9},
    {0xa5ed9ce2e4c00c38, 0},
    {0, 0},
    {0xa5ed9ce2e4c00c34, 0xdaf987b3cc62481a},
};

static bool g_finalizer = false;

void AssertFalse()
{
    ASSERT_TRUE(false);
}

static void* TestDetachCallback(napi_env env, void* nativeObject, void* hint)
{
    HILOG_INFO("this is detach callback");
    return nativeObject;
}

static void TestDetachFinalizer(void* detachedObject, void* finalizeHint)
{
    HILOG_INFO("this is detach finalizer");
    if (g_finalizer) {
        AssertFalse();
    }
    g_finalizer = true;
}

static napi_value TestAttachCallback(napi_env env, void* nativeObject, void* hint)
{
    HILOG_INFO("this is attach callback");
    napi_value object = nullptr;
    napi_value number = nullptr;
    uint32_t data = 0;
    if (hint != nullptr) {
        object = reinterpret_cast<napi_value>(nativeObject);
        data = 2000; // 2000 : test number
        napi_create_uint32(env, data, &number);
    } else {
        napi_create_object(env, &object);
        data = 1000; // 1000 : test number
        napi_create_uint32(env, data, &number);
    }
    napi_set_named_property(env, object, "number", number);
    return object;
}

napi_value TestAbort(napi_env env, napi_callback_info info)
{
    AssertFalse();
    return nullptr;
}

/**
 * @tc.name: IsolateRequireMethodTest001
 * @tc.desc: Test globalThis.requireNapi and globalThis.requireInternal method.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, IsolateRequireMethodTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    NativeEngineProxy ctxEnv(engine_);

    const char requireNapiName[] = "requireNapi";
    const char requireInternalName[] = "requireInternal";
    napi_value requireNapi = nullptr;
    napi_value requireInternal = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, requireNapiName, NAPI_AUTO_LENGTH, &requireNapi));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, requireInternalName, NAPI_AUTO_LENGTH, &requireInternal));

    napi_value originGlobal = nullptr;
    napi_value originRequire[] = { nullptr, nullptr };
    ASSERT_CHECK_CALL(napi_get_global(env, &originGlobal));
    ASSERT_CHECK_CALL(napi_get_property(env, originGlobal, requireNapi, &originRequire[0]));
    ASSERT_CHECK_CALL(napi_get_property(env, originGlobal, requireInternal, &originRequire[1]));

    napi_value newGlobal = nullptr;
    napi_value newRequire[] = { nullptr, nullptr };
    ASSERT_CHECK_CALL(napi_get_global(ctxEnv, &newGlobal));
    ASSERT_CHECK_CALL(napi_get_property(ctxEnv, newGlobal, requireNapi, &newRequire[0]));
    ASSERT_CHECK_CALL(napi_get_property(ctxEnv, newGlobal, requireInternal, &newRequire[1]));

    bool isStrictEqual = false;
    ASSERT_CHECK_CALL(napi_strict_equals(env, originRequire[0], newRequire[0], &isStrictEqual));
    ASSERT_FALSE(isStrictEqual);
    ASSERT_CHECK_CALL(napi_strict_equals(env, originRequire[1], newRequire[1], &isStrictEqual));
    ASSERT_FALSE(isStrictEqual);
}

/**
 * @tc.name: ToNativeBindingObjectTest001
 * @tc.desc: Test nativeBinding object type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ToNativeBindingObjectTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_value object1 = nullptr;
    napi_create_object(env, &object1);
    napi_status status = napi_coerce_to_native_binding_object(
        env, object, TestDetachCallback, TestAttachCallback, reinterpret_cast<void*>(object1), nullptr);
    ASSERT_EQ(status, napi_status::napi_ok);
    status = napi_add_detached_finalizer(env, object, TestDetachFinalizer, nullptr);
    ASSERT_EQ(status, napi_status::napi_ok);
}

/**
 * @tc.name: ToNativeBindingObjectTest002
 * @tc.desc: Test nativeBinding object type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ToNativeBindingObjectTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_value object1 = nullptr;
    napi_create_object(env, &object1);
    napi_status status = napi_coerce_to_native_binding_object(
        env, object, TestDetachCallback, TestAttachCallback, reinterpret_cast<void*>(object1), nullptr);
    ASSERT_EQ(status, napi_status::napi_ok);
    status = napi_add_detached_finalizer(env, object, TestDetachFinalizer, nullptr);
    ASSERT_EQ(status, napi_status::napi_ok);
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    void* data = nullptr;
    napi_serialize_inner(env, object, undefined, undefined, false, true, &data);
    ASSERT_NE(data, nullptr);
    napi_value result = nullptr;
    napi_deserialize(env, data, &result);
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_object);
    g_finalizer = false;
    napi_delete_serialization_data(env, data);
    ASSERT_TRUE(g_finalizer);
    napi_value number = nullptr;
    napi_get_named_property(env, result, "number", &number);
    ASSERT_CHECK_VALUE_TYPE(env, number, napi_number);
    uint32_t numData = 0;
    napi_get_value_uint32(env, number, &numData);
    ASSERT_EQ(numData, 1000);
}

/**
 * @tc.name: ToNativeBindingObjectTest003
 * @tc.desc: Test nativeBinding object type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ToNativeBindingObjectTest003, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_status status = napi_coerce_to_native_binding_object(
        env, object, TestDetachCallback, TestAttachCallback, nullptr, nullptr);
    ASSERT_EQ(status, napi_status::napi_invalid_arg);
    napi_value object1 = nullptr;
    napi_create_object(env, &object1);
    status = napi_coerce_to_native_binding_object(
        env, object, nullptr, nullptr, reinterpret_cast<void*>(object1), nullptr);
    ASSERT_EQ(status, napi_status::napi_invalid_arg);
}

/**
 * @tc.name: ToNativeBindingObjectTest004
 * @tc.desc: Test nativeBinding object type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ToNativeBindingObjectTest004, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_value hint = nullptr;
    napi_create_object(env, &hint);
    napi_value object1 = nullptr;
    napi_create_object(env, &object1);
    napi_status status = napi_coerce_to_native_binding_object(env, object,
        TestDetachCallback, TestAttachCallback, reinterpret_cast<void*>(object1), reinterpret_cast<void*>(hint));
    ASSERT_EQ(status, napi_status::napi_ok);
    status = napi_add_detached_finalizer(env, object, TestDetachFinalizer, nullptr);
    ASSERT_EQ(status, napi_status::napi_ok);
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    void* data = nullptr;
    napi_serialize_inner(env, object, undefined, undefined, false, true, &data);
    ASSERT_NE(data, nullptr);
    napi_value result = nullptr;
    napi_deserialize(env, data, &result);
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_object);
    g_finalizer = false;
    napi_delete_serialization_data(env, data);
    ASSERT_TRUE(g_finalizer);
    napi_value number = nullptr;
    napi_get_named_property(env, result, "number", &number);
    ASSERT_CHECK_VALUE_TYPE(env, number, napi_number);
    uint32_t numData = 0;
    napi_get_value_uint32(env, number, &numData);
    ASSERT_EQ(numData, 2000);
}

/**
 * @tc.name: ToNativeBindingObjectTest005
 * @tc.desc: Test nativeBinding object type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ToNativeBindingObjectTest005, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_status status = napi_create_object(env, &object);
    ASSERT_EQ(status, napi_status::napi_ok);
    status = napi_add_detached_finalizer(env, object, TestDetachFinalizer, nullptr);
    ASSERT_EQ(status, napi_status::napi_object_expected);
}

/**
 * @tc.name: UndefinedTest001
 * @tc.desc: Test undefined type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, UndefinedTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_undefined);
}

/**
 * @tc.name: NullTest001
 * @tc.desc: Test null type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NullTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_get_null(env, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_null);
}

/**
 * @tc.name: BooleanTest001
 * @tc.desc: Test boolean type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, BooleanTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_boolean);

    bool resultValue = false;
    ASSERT_CHECK_CALL(napi_get_value_bool(env, result, &resultValue));
    ASSERT_TRUE(resultValue);
}

/**
 * @tc.name: BooleanTest002
 * @tc.desc: Test boolean type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, BooleanTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, false, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_boolean);

    bool resultValue = true;
    ASSERT_CHECK_CALL(napi_get_value_bool(env, result, &resultValue));
    ASSERT_FALSE(resultValue);
}

/**
 * @tc.name: NumberTest001
 * @tc.desc: Test number type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NumberTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    {
        int32_t testValue = INT32_MAX;
        napi_value result = nullptr;
        ASSERT_CHECK_CALL(napi_create_int32(env, testValue, &result));
        ASSERT_CHECK_VALUE_TYPE(env, result, napi_number);

        int32_t resultValue = 0;
        ASSERT_CHECK_CALL(napi_get_value_int32(env, result, &resultValue));
        ASSERT_EQ(resultValue, INT32_MAX);
    }
    {
        uint32_t testValue = UINT32_MAX;
        napi_value result = nullptr;
        ASSERT_CHECK_CALL(napi_create_uint32(env, testValue, &result));
        ASSERT_CHECK_VALUE_TYPE(env, result, napi_number);

        uint32_t resultValue = 0;
        ASSERT_CHECK_CALL(napi_get_value_uint32(env, result, &resultValue));
        ASSERT_EQ(resultValue, UINT32_MAX);
    }
    {
        int64_t testValue = 9007199254740991;
        napi_value result = nullptr;
        ASSERT_CHECK_CALL(napi_create_int64(env, testValue, &result));
        ASSERT_CHECK_VALUE_TYPE(env, result, napi_number);

        int64_t resultValue = 0;
        ASSERT_CHECK_CALL(napi_get_value_int64(env, result, &resultValue));
        ASSERT_EQ(resultValue, testValue);
    }
    {
        double testValue = DBL_MAX;
        napi_value result = nullptr;
        ASSERT_CHECK_CALL(napi_create_double(env, testValue, &result));
        ASSERT_CHECK_VALUE_TYPE(env, result, napi_number);

        double resultValue = 0;
        ASSERT_CHECK_CALL(napi_get_value_double(env, result, &resultValue));
        ASSERT_EQ(resultValue, DBL_MAX);
    }
}

/**
 * @tc.name: StringTest001
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    const char testStr[] = "中文,English,123456,!@#$%$#^%&";
    size_t testStrLength = strlen(testStr);
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, testStr, testStrLength, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_string);

    char* buffer = nullptr;
    size_t bufferSize = 0;
    size_t strLength = 0;
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, result, nullptr, 0, &bufferSize));
    ASSERT_GT(bufferSize, static_cast<size_t>(0));
    buffer = new char[bufferSize + 1]{ 0 };
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, result, buffer, bufferSize + 1, &strLength));
    ASSERT_STREQ(testStr, buffer);
    ASSERT_EQ(testStrLength, strLength);
    delete []buffer;
    buffer = nullptr;
}

/**
 * @tc.name: StringTest002
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    const char testStr[] = "中测";
    size_t testStrLength = strlen(testStr);
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, testStr, testStrLength, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_string);

    std::string str = "";
    size_t strSize = 0;
    napi_get_value_string_latin1(env, result, nullptr, 0, &strSize);
    str.reserve(strSize + 1);
    str.resize(strSize);
    napi_get_value_string_latin1(env, result, str.data(), strSize + 1, &strSize);

    ASSERT_EQ(str, "-K");
}

/**
 * @tc.name: StringTest003
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringTest003, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    const char16_t testStr[] = u"abc56";
    size_t testStrLength = std::char_traits<char16_t>::length(testStr);
    napi_value res = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf16(env, testStr, testStrLength, &res));
    ASSERT_CHECK_VALUE_TYPE(env, res, napi_string);

    char16_t* buffer = nullptr;
    size_t bufSize = 0;
    size_t copied = 0;
    ASSERT_CHECK_CALL(napi_get_value_string_utf16(env, res, nullptr, 0, &bufSize));
    ASSERT_EQ(bufSize, testStrLength);
    buffer = new char16_t[bufSize + 1]{ 0 };
    ASSERT_CHECK_CALL(napi_get_value_string_utf16(env, res, buffer, bufSize + 1, &copied));
    for (size_t i = 0; i < copied; i++) {
        ASSERT_TRUE(testStr[i] == buffer[i]);
    }
    ASSERT_EQ(testStrLength, copied);
    delete []buffer;
    buffer = nullptr;
}

/**
 * @tc.name: StringTest004
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringTest004, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    const char16_t testStr[] = u"abc56";
    size_t testStrLength = std::char_traits<char16_t>::length(testStr);
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf16(env, testStr, testStrLength, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_string);

    char16_t buffer[4]; // 4: char16_t type of array size
    size_t bufferSize = 4; // 4: char16_t type of array size
    size_t copied;

    ASSERT_CHECK_CALL(napi_get_value_string_utf16(env, result, buffer, bufferSize, &copied));
    for (size_t i = 0; i < copied; i++) {
        ASSERT_TRUE(testStr[i] == buffer[i]);
    }
    ASSERT_EQ(copied, 3);
}

/**
 * @tc.name: StringTest005
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringTest005, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    const char16_t testStr[] = u"123456"; // 123456: test numbers
    size_t testStrLength = std::char_traits<char16_t>::length(testStr);
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf16(env, testStr, testStrLength, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_string);

    char16_t buffer[4]; // 4: char16_t type of array size
    size_t bufferSize = 4; // 4: char16_t type of array size
    size_t copied;

    ASSERT_CHECK_CALL(napi_get_value_string_utf16(env, result, buffer, bufferSize, &copied));
    for (size_t i = 0; i < copied; i++) {
        ASSERT_TRUE(testStr[i] == buffer[i]);
    }
    ASSERT_EQ(copied, 3);
}

/**
 * @tc.name: TypetagTest001
 * @tc.desc: Test typetag type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, TypetagTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value instance = nullptr;
    bool result;
    for (size_t i = 0; i < 5; i++) {
        napi_create_object(env, &instance);
        napi_type_tag_object(env, instance, &typeTags[i]);
        napi_check_object_type_tag(env, instance, &typeTags[i], &result);
        ASSERT_TRUE(result);
    }
}

/**
 * @tc.name: TypetagTest002
 * @tc.desc: Test typetag type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, TypetagTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    uint32_t typeIndex = 0;
    napi_value instance = nullptr;
    bool result;
    napi_create_object(env, &instance);

    napi_type_tag_object(env, instance, &typeTags[typeIndex]);
    napi_check_object_type_tag(env, instance, &typeTags[typeIndex + 1], &result);

    ASSERT_FALSE(result);
}

/**
 * @tc.name: TypetagTest003
 * @tc.desc: Test typetag type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, TypetagTest003, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value instance = nullptr;
    bool result;
    for (size_t i = BUFFER_SIZE_FIVE; i < BUFFER_SIZE_TEN; i++) {
        napi_create_object(env, &instance);
        napi_type_tag_object(env, instance, &typeTags[i]);
        napi_check_object_type_tag(env, instance, &typeTags[i], &result);
        ASSERT_TRUE(result);
    }
}

/**
 * @tc.name: SymbolTest001
 * @tc.desc: Test symbol type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, SymbolTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    const char testStr[] = "testSymbol";
    napi_value result = nullptr;

    napi_create_string_latin1(env, testStr, strlen(testStr), &result);

    napi_value symbolVal = nullptr;
    napi_create_symbol(env, result, &symbolVal);

    ASSERT_CHECK_VALUE_TYPE(env, symbolVal, napi_symbol);
}

/**
 * @tc.name: SymbolTest002
 * @tc.desc: Test symbol type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, SymbolTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    const char testStr[] = "中测";
    napi_value result = nullptr;

    napi_create_string_latin1(env, testStr, strlen(testStr), &result);

    napi_value symbolVal = nullptr;
    napi_create_symbol(env, result, &symbolVal);

    ASSERT_CHECK_VALUE_TYPE(env, symbolVal, napi_symbol);
}

/**
 * @tc.name: ExternalTest001
 * @tc.desc: Test external type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ExternalTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    const char testStr[] = "test";
    napi_value external = nullptr;
    napi_create_external(
        env, (void*)testStr,
        [](napi_env env, void* data, void* hint) { ASSERT_STREQ((const char*)data, (const char*)hint); },
        (void*)testStr, &external);

    ASSERT_CHECK_VALUE_TYPE(env, external, napi_external);
    void* tmpExternal = nullptr;
    napi_get_value_external(env, external, &tmpExternal);
    ASSERT_TRUE(tmpExternal);
    ASSERT_EQ(tmpExternal, testStr);
}

/**
 * @tc.name: ExternalTest002
 * @tc.desc: Test external type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ExternalTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    const char testStr[] = "中测";
    napi_value external = nullptr;
    napi_create_external(
        env, (void*)testStr,
        [](napi_env env, void* data, void* hint) { ASSERT_STREQ((const char*)data, (const char*)hint); },
        (void*)testStr, &external);

    ASSERT_CHECK_VALUE_TYPE(env, external, napi_external);
    void* tmpExternal = nullptr;
    napi_get_value_external(env, external, &tmpExternal);
    ASSERT_TRUE(tmpExternal);
    ASSERT_EQ(tmpExternal, testStr);
}

/**
 * @tc.name: ExternalTest003
 * @tc.desc: Test external type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ExternalTest003, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    const char testStr[] = "123456"; // 123456: test numbers
    napi_value external = nullptr;
    napi_create_external(
        env, (void*)testStr,
        [](napi_env env, void* data, void* hint) { ASSERT_STREQ((const char*)data, (const char*)hint); },
        (void*)testStr, &external);

    ASSERT_CHECK_VALUE_TYPE(env, external, napi_external);
    void* tmpExternal = nullptr;
    napi_get_value_external(env, external, &tmpExternal);
    ASSERT_TRUE(tmpExternal);
    ASSERT_EQ(tmpExternal, testStr);
}

/**
 * @tc.name: ObjectTest001
 * @tc.desc: Test object type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ObjectTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_object);

    const char testStr[] = "1234567";
    napi_value strAttribute = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, testStr, strlen(testStr), &strAttribute));
    ASSERT_CHECK_VALUE_TYPE(env, strAttribute, napi_string);
    ASSERT_CHECK_CALL(napi_set_named_property(env, result, "strAttribute", strAttribute));

    napi_value retStrAttribute = nullptr;
    ASSERT_CHECK_CALL(napi_get_named_property(env, result, "strAttribute", &retStrAttribute));
    ASSERT_CHECK_VALUE_TYPE(env, retStrAttribute, napi_string);

    int32_t testNumber = 12345;
    napi_value numberAttribute = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, testNumber, &numberAttribute));
    ASSERT_CHECK_VALUE_TYPE(env, numberAttribute, napi_number);
    ASSERT_CHECK_CALL(napi_set_named_property(env, result, "numberAttribute", numberAttribute));

    napi_value propNames = nullptr;
    ASSERT_CHECK_CALL(napi_get_property_names(env, result, &propNames));
    ASSERT_CHECK_VALUE_TYPE(env, propNames, napi_object);
    bool isArray = false;
    ASSERT_CHECK_CALL(napi_is_array(env, propNames, &isArray));
    ASSERT_TRUE(isArray);
    uint32_t arrayLength = 0;
    ASSERT_CHECK_CALL(napi_get_array_length(env, propNames, &arrayLength));
    ASSERT_EQ(arrayLength, static_cast<uint32_t>(2));

    for (uint32_t i = 0; i < arrayLength; i++) {
        bool hasElement = false;
        ASSERT_CHECK_CALL(napi_has_element(env, propNames, i, &hasElement));

        napi_value propName = nullptr;
        ASSERT_CHECK_CALL(napi_get_element(env, propNames, i, &propName));
        ASSERT_CHECK_VALUE_TYPE(env, propName, napi_string);

        bool hasProperty = false;
        napi_has_property(env, result, propName, &hasProperty);
        ASSERT_TRUE(hasProperty);

        napi_value propValue = nullptr;
        napi_get_property(env, result, propName, &propValue);
        ASSERT_TRUE(propValue != nullptr);
    }
}

/**
 * @tc.name: ObjectTest002
 * @tc.desc: Test Object Type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ObjectTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value result = nullptr;
    napi_create_object(env, &result);
    napi_value messageKey = nullptr;
    const char* messageKeyStr = "message";
    napi_create_string_latin1(env, messageKeyStr, strlen(messageKeyStr), &messageKey);
    napi_value messageValue = nullptr;
    const char* messageValueStr = "OK";
    napi_create_string_latin1(env, messageValueStr, strlen(messageValueStr), &messageValue);
    napi_set_property(env, result, messageKey, messageValue);

    napi_value propValue = nullptr;
    napi_get_property(env, result, messageKey, &propValue);
    ASSERT_TRUE(propValue != nullptr);

    napi_delete_property(env, result, messageKey, nullptr);
    bool resultVal = true;
    napi_has_property(env, result, messageKey, &resultVal);
    ASSERT_FALSE(resultVal);

    napi_value newKey = nullptr;
    const char* newKeyStr = "new";
    napi_create_string_latin1(env, newKeyStr, strlen(newKeyStr), &newKey);
    int32_t testnumber = 12345;
    napi_value numberValue = nullptr;
    napi_create_int32(env, testnumber, &numberValue);
    napi_set_property(env, result, newKey, numberValue);

    napi_value propNames = nullptr;
    napi_get_property_names(env, result, &propNames);
    uint32_t arrayLength = 0;
    napi_get_array_length(env, propNames, &arrayLength);
    ASSERT_EQ(arrayLength, static_cast<uint32_t>(1));
}

/**
 * @tc.name: ObjectTest003
 * @tc.desc: Test Object Type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ObjectTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value result = nullptr;
    napi_create_object(env, &result);

    auto func = [](napi_env env, napi_callback_info info) -> napi_value {
        return nullptr;
    };
    napi_value funcAttribute = nullptr;
    napi_create_function(env, "testFunc", NAPI_AUTO_LENGTH, func, nullptr, &funcAttribute);

    napi_value funcKey = nullptr;
    const char* funcKeyStr = "func";
    napi_create_string_latin1(env, funcKeyStr, strlen(funcKeyStr), &funcKey);
    napi_status status = napi_set_property(env, result, funcKey, funcAttribute);
    ASSERT_EQ(status, napi_status::napi_ok);

    bool isFuncExist = false;
    ASSERT_CHECK_CALL(napi_has_property(env, result, funcKey, &isFuncExist));
    ASSERT_TRUE(isFuncExist);

    napi_value propFuncValue = nullptr;
    napi_get_property_names(env, result, &propFuncValue);
    uint32_t arrayLength = 0;
    napi_get_array_length(env, propFuncValue, &arrayLength);
    ASSERT_EQ(arrayLength, static_cast<uint32_t>(1));

    bool isFuncDelete = false;
    ASSERT_CHECK_CALL(napi_delete_property(env, result, funcKey, &isFuncDelete));
    ASSERT_TRUE(isFuncDelete);
}

/**
 * @tc.name: FunctionTest001
 * @tc.desc: Test function type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, FunctionTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    auto func = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value thisVar;
        napi_value* argv = nullptr;
        size_t argc = 0;
        void* data = nullptr;

        napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
        if (argc > 0) {
            argv = new napi_value[argc];
        }
        napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

        napi_value result = nullptr;
        napi_create_object(env, &result);

        napi_value messageKey = nullptr;
        const char* messageKeyStr = "message";
        napi_create_string_latin1(env, messageKeyStr, strlen(messageKeyStr), &messageKey);
        napi_value messageValue = nullptr;
        const char* messageValueStr = "OK";
        napi_create_string_latin1(env, messageValueStr, strlen(messageValueStr), &messageValue);
        napi_set_property(env, result, messageKey, messageValue);

        if (argv != nullptr) {
            delete []argv;
        }

        return result;
    };

    napi_value recv = nullptr;
    napi_value funcValue = nullptr;
    napi_get_undefined(env, &recv);
    ASSERT_NE(recv, nullptr);

    napi_create_function(env, "testFunc", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    ASSERT_NE(funcValue, nullptr);

    napi_handle_scope parentScope = nullptr;
    napi_open_handle_scope(env, &parentScope);
    ASSERT_NE(parentScope, nullptr);

    napi_escapable_handle_scope childScope = nullptr;
    napi_open_escapable_handle_scope(env, &childScope);
    ASSERT_NE(childScope, nullptr);

    napi_value funcResultValue = nullptr;
    napi_value newFuncResultValue = nullptr;
    napi_call_function(env, recv, funcValue, 0, nullptr, &funcResultValue);
    ASSERT_NE(funcResultValue, nullptr);

    napi_escape_handle(env, childScope, funcResultValue, &newFuncResultValue);
    napi_close_escapable_handle_scope(env, childScope);
    ASSERT_TRUE(newFuncResultValue != nullptr);
    ASSERT_CHECK_VALUE_TYPE(env, newFuncResultValue, napi_object);
    napi_close_handle_scope(env, parentScope);
}

/**
 * @tc.name: FunctionTest002
 * @tc.desc: Test function type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, FunctionTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto func = [](napi_env env, napi_callback_info info) -> napi_value {
        return nullptr;
    };
    napi_value fn;
    const char data[] = "data";
    napi_status status = napi_create_function(nullptr, nullptr, 0, nullptr, nullptr, &fn);
    ASSERT_EQ(status, napi_invalid_arg);
    status = napi_create_function(env, nullptr, 0, nullptr, nullptr, &fn);
    ASSERT_EQ(status, napi_invalid_arg);
    status = napi_create_function(env, nullptr, 0, func, (void*)data, nullptr);
    ASSERT_EQ(status, napi_invalid_arg);
    status = napi_create_function(env, nullptr, 0, func, nullptr, &fn);
    ASSERT_EQ(status, napi_ok);
    status = napi_create_function(env, nullptr, 0, func, (void*)data, &fn);
    ASSERT_EQ(status, napi_ok);
}

/**
 * @tc.name: FunctionTest003
 * @tc.desc: Test function type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, FunctionTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    auto func = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value thisVar;
        napi_value* argv = nullptr;
        size_t argc = 0;
        void* innerData = nullptr;

        napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
        if (argc > 0) {
            argv = new napi_value[argc];
        }
        napi_get_cb_info(env, info, &argc, argv, &thisVar, &innerData);
        napi_value result;
        if (argv) {
            result = argv[0];
            delete[] argv;
        } else {
            napi_get_undefined(env, &result);
        }
        return result;
    };

    napi_value fn;
    napi_value funcResultValue;
    napi_value recv;
    napi_value jsNumber;
    const static char data[] = "data";
    napi_status status = napi_create_function(env, nullptr, 0, func, (void*)data, &fn);
    ASSERT_EQ(napi_ok, status);

    const int32_t testNumber = 1;
    napi_create_int32(env, testNumber, &jsNumber);
    napi_value argv[] = { jsNumber };
    napi_get_undefined(env, &recv);
    status = napi_call_function(env, recv, fn, 1, argv, &funcResultValue);
    ASSERT_EQ(status, napi_ok);

    int32_t cNumber;
    napi_get_value_int32(env, funcResultValue, &cNumber);
    ASSERT_EQ(cNumber, testNumber);

    status = napi_call_function(env, nullptr, fn, 1, argv, &funcResultValue);
    ASSERT_EQ(status, napi_ok);

    status = napi_call_function(env, nullptr, nullptr, 1, argv, &funcResultValue);
    ASSERT_EQ(status, napi_invalid_arg);

    status = napi_call_function(env, nullptr, nullptr, 0, nullptr, &funcResultValue);
    ASSERT_EQ(status, napi_invalid_arg);

    status = napi_call_function(env, nullptr, fn, 1, argv, nullptr);
    ASSERT_EQ(status, napi_ok);
}

static napi_value TestCreateFunc(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    return result;
}

/**
 * @tc.name: FunctionTest004
 * @tc.desc: Test the second parameter as null
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, FunctionTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value funcValue = nullptr;
    napi_create_function(env, nullptr, NAPI_AUTO_LENGTH, TestCreateFunc, nullptr, &funcValue);
    ASSERT_NE(funcValue, nullptr);

    napi_value recv = nullptr;
    napi_get_undefined(env, &recv);
    ASSERT_NE(recv, nullptr);
    napi_value funcResultValue = nullptr;
    napi_call_function(env, recv, funcValue, 0, nullptr, &funcResultValue);
    ASSERT_NE(funcResultValue, nullptr);
}

static napi_value TestCallFunc(napi_env env, napi_callback_info info)
{
    napi_value error = nullptr;
    napi_throw_error(env, "500", "Common error");
    return error;
}

/**
 * @tc.name: FunctionTest005
 * @tc.desc: Test callfunction throw error
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, FunctionTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value funcValue = nullptr;
    napi_value exception = nullptr;
    napi_create_function(env, "testFunc", NAPI_AUTO_LENGTH, TestCallFunc, nullptr, &funcValue);
    ASSERT_NE(funcValue, nullptr);

    napi_value recv = nullptr;
    napi_get_undefined(env, &recv);
    ASSERT_NE(recv, nullptr);
    napi_value funcResultValue = nullptr;
    bool isExceptionPending = false;
    napi_call_function(env, recv, funcValue, 0, nullptr, &funcResultValue);
    napi_is_exception_pending(env, &isExceptionPending);
    ASSERT_TRUE(isExceptionPending);

    napi_get_and_clear_last_exception(env, &exception);
}

/**
 * @tc.name: ArrayTest001
 * @tc.desc: Test array type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ArrayTest001, testing::ext::TestSize.Level1) {
    napi_env env = (napi_env) engine_;

    napi_value array = nullptr;
    napi_create_array(env, &array);
    ASSERT_NE(array, nullptr);
    bool isArray = false;
    napi_is_array(env, array, &isArray);
    ASSERT_TRUE(isArray);

    for (size_t i = 0; i < 10; i++) {
        napi_value num = nullptr;
        napi_create_uint32(env, i, &num);
        napi_set_element(env, array, i, num);
    }

    uint32_t arrayLength = 0;
    napi_get_array_length(env, array, &arrayLength);

    ASSERT_EQ(arrayLength, static_cast<uint32_t>(10));

    for (size_t i = 0; i < arrayLength; i++) {
        bool hasIndex = false;
        napi_has_element(env, array, i, &hasIndex);
        ASSERT_TRUE(hasIndex);
    }

    for (size_t i = 0; i < arrayLength; i++) {
        bool isDelete = false;
        napi_delete_element(env, array, i, &isDelete);
        ASSERT_TRUE(isDelete);
    }
}

/**
 * @tc.name: ArrayBufferTest001
 * @tc.desc: Test array buffer type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ArrayBufferTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    size_t arrayBufferSize = 1024;
    napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    void* tmpArrayBufferPtr = nullptr;
    size_t arrayBufferLength = 0;
    napi_get_arraybuffer_info(env, arrayBuffer, &tmpArrayBufferPtr, &arrayBufferLength);

    ASSERT_EQ(arrayBufferPtr, tmpArrayBufferPtr);
    ASSERT_EQ(arrayBufferSize, arrayBufferLength);
}

/**
 * @tc.name: TypedArrayTest001
 * @tc.desc: Test typed array type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, TypedArrayTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    {
        napi_value arrayBuffer = nullptr;
        void* arrayBufferPtr = nullptr;
        size_t arrayBufferSize = 1024;
        napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

        void* tmpArrayBufferPtr = nullptr;
        size_t arrayBufferLength = 0;
        napi_get_arraybuffer_info(env, arrayBuffer, &tmpArrayBufferPtr, &arrayBufferLength);

        ASSERT_EQ(arrayBufferPtr, tmpArrayBufferPtr);
        ASSERT_EQ(arrayBufferSize, arrayBufferLength);

        napi_value typedarray = nullptr;
        size_t byteOffsetCreate = 0;
        napi_create_typedarray(env, napi_int8_array, arrayBufferSize, arrayBuffer, byteOffsetCreate, &typedarray);
        ASSERT_NE(typedarray, nullptr);
        bool isTypedArray = false;
        napi_is_typedarray(env, typedarray, &isTypedArray);
        ASSERT_TRUE(isTypedArray);

        napi_typedarray_type typedarrayType;
        size_t typedarrayLength = 0;
        void* typedarrayBufferPtr = nullptr;
        napi_value tmpArrayBuffer = nullptr;
        size_t byteOffset = 0;

        napi_get_typedarray_info(env, typedarray, &typedarrayType, &typedarrayLength, &typedarrayBufferPtr,
                                 &tmpArrayBuffer, &byteOffset);

        ASSERT_EQ(typedarrayType, napi_int8_array);
        ASSERT_EQ(typedarrayBufferPtr, arrayBufferPtr);
        ASSERT_EQ(arrayBufferSize, typedarrayLength);
        ASSERT_EQ(byteOffset, byteOffsetCreate);
    }
}

/**
 * @tc.name: TypedArrayTest002
 * @tc.desc: Test typed array type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, TypedArrayTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    {
        napi_value arrayBuffer = nullptr;
        void* arrayBufferPtr = nullptr;
        size_t arrayBufferSize = 1024;
        napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

        void* tmpArrayBufferPtr = nullptr;
        size_t arrayBufferLength = 0;
        napi_get_arraybuffer_info(env, arrayBuffer, &tmpArrayBufferPtr, &arrayBufferLength);

        ASSERT_EQ(arrayBufferPtr, tmpArrayBufferPtr);
        ASSERT_EQ(arrayBufferSize, arrayBufferLength);

        napi_value typedarray = nullptr;
        size_t byteOffsetCreate = 0;
        napi_create_typedarray(env, napi_uint8_array, arrayBufferSize, arrayBuffer, byteOffsetCreate, &typedarray);
        ASSERT_NE(typedarray, nullptr);
        bool isTypedArray = false;
        napi_is_typedarray(env, typedarray, &isTypedArray);
        ASSERT_TRUE(isTypedArray);

        napi_typedarray_type typedarrayType;
        size_t typedarrayLength = 0;
        void* typedarrayBufferPtr = nullptr;
        napi_value tmpArrayBuffer = nullptr;
        size_t byteOffset = 0;

        napi_get_typedarray_info(env, typedarray, &typedarrayType, &typedarrayLength, &typedarrayBufferPtr,
                                 &tmpArrayBuffer, &byteOffset);

        ASSERT_EQ(typedarrayType, napi_uint8_array);
        ASSERT_EQ(typedarrayBufferPtr, arrayBufferPtr);
        ASSERT_EQ(arrayBufferSize, typedarrayLength);
        ASSERT_EQ(byteOffset, byteOffsetCreate);
    }
}

/**
 * @tc.name: TypedArrayTest003
 * @tc.desc: Test typed array type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, TypedArrayTest003, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    {
        napi_value arrayBuffer = nullptr;
        void* arrayBufferPtr = nullptr;
        size_t arrayBufferSize = 1024;
        napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

        void* tmpArrayBufferPtr = nullptr;
        size_t arrayBufferLength = 0;
        napi_get_arraybuffer_info(env, arrayBuffer, &tmpArrayBufferPtr, &arrayBufferLength);

        ASSERT_EQ(arrayBufferPtr, tmpArrayBufferPtr);
        ASSERT_EQ(arrayBufferSize, arrayBufferLength);

        napi_value typedarray = nullptr;
        size_t byteOffsetCreate = 0;
        napi_create_typedarray(env, napi_uint8_clamped_array, arrayBufferSize, arrayBuffer, byteOffsetCreate, &typedarray);
        ASSERT_NE(typedarray, nullptr);
        bool isTypedArray = false;
        napi_is_typedarray(env, typedarray, &isTypedArray);
        ASSERT_TRUE(isTypedArray);

        napi_typedarray_type typedarrayType;
        size_t typedarrayLength = 0;
        void* typedarrayBufferPtr = nullptr;
        napi_value tmpArrayBuffer = nullptr;
        size_t byteOffset = 0;

        napi_get_typedarray_info(env, typedarray, &typedarrayType, &typedarrayLength, &typedarrayBufferPtr,
                                 &tmpArrayBuffer, &byteOffset);

        ASSERT_EQ(typedarrayType, napi_uint8_clamped_array);
        ASSERT_EQ(typedarrayBufferPtr, arrayBufferPtr);
        ASSERT_EQ(arrayBufferSize, typedarrayLength);
        ASSERT_EQ(byteOffset, byteOffsetCreate);
    }
}

/**
 * @tc.name: DataViewTest001
 * @tc.desc: Test data view type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, DataViewTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    size_t arrayBufferSize = 1024;
    napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);
    ASSERT_NE(arrayBuffer, nullptr);
    ASSERT_NE(arrayBufferPtr, nullptr);
    bool isArrayBuffer = false;
    napi_is_arraybuffer(env, arrayBuffer, &isArrayBuffer);
    ASSERT_TRUE(isArrayBuffer);

    napi_value result = nullptr;
    napi_create_dataview(env, arrayBufferSize, arrayBuffer, 0, &result);

    bool isDataView = false;
    napi_is_dataview(env, result, &isDataView);

    napi_value retArrayBuffer = nullptr;
    void* data = nullptr;
    size_t byteLength = 0;
    size_t byteOffset = 0;
    napi_get_dataview_info(env, result, &byteLength, &data, &retArrayBuffer, &byteOffset);

    bool retIsArrayBuffer = false;
    napi_is_arraybuffer(env, arrayBuffer, &retIsArrayBuffer);
    ASSERT_TRUE(retIsArrayBuffer);
    ASSERT_EQ(arrayBufferPtr, data);
    ASSERT_EQ(arrayBufferSize, byteLength);
    ASSERT_EQ(static_cast<size_t>(0), byteOffset);
}

/**
 * @tc.name: PromiseTest001
 * @tc.desc: Test promise type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, PromiseTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    {
        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        ASSERT_CHECK_CALL(napi_create_promise(env, &deferred, &promise));
        ASSERT_NE(deferred, nullptr);
        ASSERT_NE(promise, nullptr);

        bool isPromise = false;
        ASSERT_CHECK_CALL(napi_is_promise(env, promise, &isPromise));
        ASSERT_TRUE(isPromise);

        napi_value undefined = nullptr;
        napi_get_undefined(env, &undefined);
        ASSERT_CHECK_CALL(napi_resolve_deferred(env, deferred, undefined));
    }
    {
        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        ASSERT_CHECK_CALL(napi_create_promise(env, &deferred, &promise));
        ASSERT_NE(deferred, nullptr);
        ASSERT_NE(promise, nullptr);

        bool isPromise = false;
        ASSERT_CHECK_CALL(napi_is_promise(env, promise, &isPromise));
        ASSERT_TRUE(isPromise);

        napi_value undefined = nullptr;
        napi_get_undefined(env, &undefined);
        ASSERT_CHECK_CALL(napi_reject_deferred(env, deferred, undefined));
    }
    {
        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        ASSERT_CHECK_CALL(napi_throw_error(env, "500", "common error"));
        ASSERT_NE(napi_create_promise(env, &deferred, &promise), napi_ok);
        ASSERT_EQ(deferred, nullptr);
        ASSERT_EQ(promise, nullptr);
        napi_value error = nullptr;
        napi_get_and_clear_last_exception(env, &error);
    }
}

/**
 * @tc.name: PromiseTest002
 * @tc.desc: Test promise type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, PromiseTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    {
        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        napi_status status = napi_create_promise(nullptr, &deferred, &promise);
        ASSERT_EQ(status, napi_status::napi_invalid_arg);
        status = napi_create_promise(env, nullptr, &promise);
        ASSERT_EQ(status, napi_status::napi_invalid_arg);
        status = napi_create_promise(env, &deferred, nullptr);
        ASSERT_EQ(status, napi_status::napi_invalid_arg);
    }
    {
        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        ASSERT_CHECK_CALL(napi_create_promise(env, &deferred, &promise));

        bool isPromise = false;
        napi_status status = napi_is_promise(nullptr, promise, &isPromise);
        ASSERT_EQ(status, napi_status::napi_invalid_arg);
        status = napi_is_promise(env, nullptr, &isPromise);
        ASSERT_EQ(status, napi_status::napi_invalid_arg);
        status = napi_is_promise(env, promise, nullptr);
        ASSERT_EQ(status, napi_status::napi_invalid_arg);
    }
    {
        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        ASSERT_CHECK_CALL(napi_create_promise(env, &deferred, &promise));

        napi_value undefined = nullptr;
        napi_get_undefined(env, &undefined);
        napi_status status = napi_resolve_deferred(nullptr, deferred, undefined);
        ASSERT_EQ(status, napi_status::napi_invalid_arg);
        status = napi_resolve_deferred(env, nullptr, undefined);
        ASSERT_EQ(status, napi_status::napi_invalid_arg);
        status = napi_resolve_deferred(env, deferred, nullptr);
        ASSERT_EQ(status, napi_status::napi_invalid_arg);
    }
    {
        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        ASSERT_CHECK_CALL(napi_create_promise(env, &deferred, &promise));

        napi_value undefined = nullptr;
        napi_get_undefined(env, &undefined);
        napi_status status = napi_reject_deferred(nullptr, deferred, undefined);
        ASSERT_EQ(status, napi_status::napi_invalid_arg);
        status = napi_reject_deferred(env, nullptr, undefined);
        ASSERT_EQ(status, napi_status::napi_invalid_arg);
        status = napi_reject_deferred(env, deferred, nullptr);
        ASSERT_EQ(status, napi_status::napi_invalid_arg);
    }
}

/**
 * @tc.name: PromiseTest003
 * @tc.desc: Test promise type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, PromiseTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    ASSERT_CHECK_CALL(napi_create_promise(env, &deferred, &promise));

    napi_value key, thenFun, result;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "then", NAPI_AUTO_LENGTH, &key));
    ASSERT_CHECK_CALL(napi_get_property(env, promise, key, &thenFun));

    napi_value cb = nullptr;
    auto func = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 1;
        napi_value argv[1] = {0};

        napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

        int32_t num;
        napi_get_value_int32(env, argv[0], &num);
        if (num != TEST_NUM) {
            AssertFalse();
        }
        return nullptr;
    };
    ASSERT_CHECK_CALL(napi_create_function(env, "testFunc", NAPI_AUTO_LENGTH, func, nullptr, &cb));
    ASSERT_CHECK_CALL(napi_call_function(env, promise, thenFun, 1, &cb, &result));

    napi_value resolution = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, TEST_NUM, &resolution));
    ASSERT_CHECK_CALL(napi_resolve_deferred(env, deferred, resolution));
}

/**
 * @tc.name: PromiseTest004
 * @tc.desc: Test promise type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, PromiseTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    ASSERT_CHECK_CALL(napi_create_promise(env, &deferred, &promise));

    napi_value key, catchFun, result;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "catch", NAPI_AUTO_LENGTH, &key));
    ASSERT_CHECK_CALL(napi_get_property(env, promise, key, &catchFun));

    napi_value cb = nullptr;
    auto func = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 1;
        napi_value argv[1] = {0};

        napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

        int32_t num;
        napi_get_value_int32(env, argv[0], &num);
        if (num != TEST_NUM) {
            AssertFalse();
        }
        return nullptr;
    };
    ASSERT_CHECK_CALL(napi_create_function(env, "testFunc", NAPI_AUTO_LENGTH, func, nullptr, &cb));
    ASSERT_CHECK_CALL(napi_call_function(env, promise, catchFun, 1, &cb, &result));

    napi_value resolution = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, TEST_NUM, &resolution));
    ASSERT_CHECK_CALL(napi_reject_deferred(env, deferred, resolution));
}

/**
 * @tc.name: PromiseTest005
 * @tc.desc: Test promise type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, PromiseTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    ASSERT_CHECK_CALL(napi_create_promise(env, &deferred, &promise));

    napi_value thenFun, result;
    ASSERT_CHECK_CALL(napi_get_named_property(env, promise, "then", &thenFun));

    napi_value cb = nullptr;
    auto func = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 1;
        napi_value argv[1] = {0};

        napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

        int32_t num;
        napi_get_value_int32(env, argv[0], &num);
        if (num != TEST_NUM) {
            AssertFalse();
        }
        return nullptr;
    };
    ASSERT_CHECK_CALL(napi_create_function(env, "testFunc", NAPI_AUTO_LENGTH, func, nullptr, &cb));
    ASSERT_CHECK_CALL(napi_call_function(env, promise, thenFun, 1, &cb, &result));

    napi_value resolution = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, TEST_NUM, &resolution));
    ASSERT_CHECK_CALL(napi_resolve_deferred(env, deferred, resolution));
}

/**
 * @tc.name: PromiseTest006
 * @tc.desc: Test promise type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, PromiseTest006, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    ASSERT_CHECK_CALL(napi_create_promise(env, &deferred, &promise));

    napi_value catchFun, result;
    ASSERT_CHECK_CALL(napi_get_named_property(env, promise, "catch", &catchFun));

    napi_value cb = nullptr;
    auto func = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 1;
        napi_value argv[1] = {0};

        napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

        int32_t num;
        napi_get_value_int32(env, argv[0], &num);
        if (num != TEST_NUM) {
            AssertFalse();
        }
        return nullptr;
    };
    ASSERT_CHECK_CALL(napi_create_function(env, "testFunc", NAPI_AUTO_LENGTH, func, nullptr, &cb));
    ASSERT_CHECK_CALL(napi_call_function(env, promise, catchFun, 1, &cb, &result));

    napi_value resolution = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, TEST_NUM, &resolution));
    ASSERT_CHECK_CALL(napi_reject_deferred(env, deferred, resolution));
}

struct CallbackDeferred {
    napi_deferred deferred;
    napi_threadsafe_function tsfn;
    napi_async_work work;
};

static void callBackPromiseTest007(napi_env env, napi_value tsfn_cb, void* context, void* data)
{
    CallbackDeferred *callbackData = reinterpret_cast<CallbackDeferred *>(data);
    napi_value resolution = nullptr;
    napi_status stat = napi_create_int32(env, TEST_NUM, &resolution);
    if (stat != napi_ok) {
        AssertFalse();
    }
    stat = napi_resolve_deferred(env, callbackData->deferred, resolution);
    if (stat != napi_ok) {
        AssertFalse();
    }
    napi_release_threadsafe_function(callbackData->tsfn, napi_tsfn_release);
}

static void finalizeCbPromiseTest007(napi_env env, void* finalizeData, void* hint)
{
    CallbackDeferred *callbackData = reinterpret_cast<CallbackDeferred *>(finalizeData);
    callbackData->tsfn = nullptr;
    if(callbackData->work == nullptr) {
        delete callbackData;
    }
}

/**
 * @tc.name: PromiseTest007
 * @tc.desc: Test promise type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, PromiseTest007, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    ASSERT_CHECK_CALL(napi_create_promise(env, &deferred, &promise));
    napi_value thenFun, result;
    ASSERT_CHECK_CALL(napi_get_named_property(env, promise, "then", &thenFun));
    napi_value cb = nullptr;
    auto func = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 1;
        napi_value argv[1] = {0};
        napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

        int32_t num;
        napi_get_value_int32(env, argv[0], &num);
        if (num != TEST_NUM) {
            AssertFalse();
        }
        return nullptr;
    };
    ASSERT_CHECK_CALL(napi_create_function(env, "testFunc", NAPI_AUTO_LENGTH, func, nullptr, &cb));
    ASSERT_CHECK_CALL(napi_call_function(env, promise, thenFun, 1, &cb, &result));

    CallbackDeferred *callbackData = new CallbackDeferred();
    callbackData->deferred = deferred;
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    auto status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName,
        0, 1, callbackData, finalizeCbPromiseTest007, callbackData, callBackPromiseTest007, &callbackData->tsfn);
    EXPECT_EQ(status, napi_ok);

    status = napi_create_async_work(env, nullptr, resourceName,
        [](napi_env env, void *data) {
            CallbackDeferred *callbackData = reinterpret_cast<CallbackDeferred *>(data);
            napi_call_threadsafe_function(callbackData->tsfn, (void *)callbackData, napi_tsfn_nonblocking);
        },
        [](napi_env env, napi_status status, void* data) {
            STOP_EVENT_LOOP(env);
            CallbackDeferred *callbackData = reinterpret_cast<CallbackDeferred *>(data);
            napi_delete_async_work(env, callbackData->work);
            callbackData->work = nullptr;
            if (callbackData->tsfn == nullptr) {
                delete callbackData;
            }
        },
        callbackData, &callbackData->work);
    EXPECT_EQ(status, napi_ok);
    napi_queue_async_work(env, callbackData->work);
    RUN_EVENT_LOOP(env);
}

/**
 * @tc.name: PromiseTest008
 * @tc.desc: Test promise type.
 * @tc.type: FUNC
 */
static void callBackPromiseTest008(napi_env env, napi_value tsfn_cb, void* context, void* data)
{
    CallbackDeferred *callbackData = reinterpret_cast<CallbackDeferred *>(data);
    napi_value resolution = nullptr;
    napi_status stat = napi_create_int32(env, TEST_NUM, &resolution);
    if (stat != napi_ok) {
        AssertFalse();
    }
    stat = napi_resolve_deferred(env, callbackData->deferred, resolution);
    if (stat != napi_ok) {
        AssertFalse();
    }
    napi_release_threadsafe_function(callbackData->tsfn, napi_tsfn_release);
}

static void finalizeCbPromiseTest008(napi_env env, void* finalizeData, void* hint)
{
    CallbackDeferred *callbackData = reinterpret_cast<CallbackDeferred *>(finalizeData);
    callbackData->tsfn = nullptr;
    if(callbackData->work == nullptr) {
        delete callbackData;
    }
}

HWTEST_F(NapiBasicTest, PromiseTest008, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    ASSERT_CHECK_CALL(napi_create_promise(env, &deferred, &promise));
    napi_value catchFun, result;
    ASSERT_CHECK_CALL(napi_get_named_property(env, promise, "catch", &catchFun));
    napi_value cb = nullptr;
    auto func = [](napi_env env, napi_callback_info info) -> napi_value {
        size_t argc = 1;
        napi_value argv[1] = {0};
        napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

        int32_t num;
        napi_get_value_int32(env, argv[0], &num);
        if (num != TEST_NUM) {
            AssertFalse();
        }
        return nullptr;
    };
    ASSERT_CHECK_CALL(napi_create_function(env, "testFunc", NAPI_AUTO_LENGTH, func, nullptr, &cb));
    ASSERT_CHECK_CALL(napi_call_function(env, promise, catchFun, 1, &cb, &result));

    CallbackDeferred *callbackData = new CallbackDeferred();
    callbackData->deferred = deferred;
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    auto status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName,
        0, 1, callbackData, finalizeCbPromiseTest008, callbackData, callBackPromiseTest008, &callbackData->tsfn);
    EXPECT_EQ(status, napi_ok);

    status = napi_create_async_work(env, nullptr, resourceName,
        [](napi_env env, void *data) {
            CallbackDeferred *callbackData = reinterpret_cast<CallbackDeferred *>(data);
            napi_call_threadsafe_function(callbackData->tsfn, (void *)callbackData, napi_tsfn_nonblocking);
        },
        [](napi_env env, napi_status status, void* data) {
            STOP_EVENT_LOOP(env);
            CallbackDeferred *callbackData = reinterpret_cast<CallbackDeferred *>(data);
            napi_delete_async_work(env, callbackData->work);
            callbackData->work = nullptr;
            if (callbackData->tsfn == nullptr) {
                delete callbackData;
            }
        },
        callbackData, &callbackData->work);
    EXPECT_EQ(status, napi_ok);
    napi_queue_async_work(env, callbackData->work);
    RUN_EVENT_LOOP(env);
}

/**
 * @tc.name: ErrorTest001
 * @tc.desc: Test error type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ErrorTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    bool isExceptionPending = false;
    napi_value exception = nullptr;

    {
        napi_value code = nullptr;
        napi_value message = nullptr;

        napi_create_string_latin1(env, "500", NAPI_AUTO_LENGTH, &code);
        napi_create_string_latin1(env, "common error", NAPI_AUTO_LENGTH, &message);

        napi_value error = nullptr;
        napi_create_error(env, code, message, &error);
        ASSERT_TRUE(error != nullptr);
        bool isError = false;
        napi_is_error(env, error, &isError);
        ASSERT_TRUE(isError);
        napi_throw(env, error);
        napi_is_exception_pending(env, &isExceptionPending);
        ASSERT_TRUE(isExceptionPending);
        napi_get_and_clear_last_exception(env, &exception);
        napi_is_exception_pending(env, &isExceptionPending);
        ASSERT_FALSE(isExceptionPending);
    }

    {
        napi_value code = nullptr;
        napi_value message = nullptr;
        napi_create_string_latin1(env, "500", NAPI_AUTO_LENGTH, &code);
        napi_create_string_latin1(env, "range error", NAPI_AUTO_LENGTH, &message);
        napi_value error = nullptr;
        napi_create_range_error(env, code, message, &error);
        ASSERT_TRUE(error != nullptr);
        bool isError = false;
        napi_is_error(env, error, &isError);
        ASSERT_TRUE(isError);

        napi_throw_range_error(env, "500", "Range error");
        napi_is_exception_pending(env, &isExceptionPending);
        ASSERT_TRUE(isExceptionPending);
        napi_get_and_clear_last_exception(env, &exception);
        napi_is_exception_pending(env, &isExceptionPending);
        ASSERT_FALSE(isExceptionPending);
    }

    {
        napi_value code = nullptr;
        napi_value message = nullptr;
        napi_create_string_latin1(env, "500", NAPI_AUTO_LENGTH, &code);
        napi_create_string_latin1(env, "type error", NAPI_AUTO_LENGTH, &message);
        napi_value error = nullptr;
        napi_create_type_error(env, code, message, &error);
        ASSERT_TRUE(error != nullptr);
        bool isError = false;
        napi_is_error(env, error, &isError);
        ASSERT_TRUE(isError);

        napi_throw_type_error(env, "500", "Type error");
        napi_is_exception_pending(env, &isExceptionPending);
        ASSERT_TRUE(isExceptionPending);
        napi_get_and_clear_last_exception(env, &exception);
        napi_is_exception_pending(env, &isExceptionPending);
        ASSERT_FALSE(isExceptionPending);
    }

    napi_throw_error(env, "500", "Common error");
    napi_is_exception_pending(env, &isExceptionPending);
    ASSERT_TRUE(isExceptionPending);
    napi_get_and_clear_last_exception(env, &exception);
    napi_is_exception_pending(env, &isExceptionPending);
    ASSERT_FALSE(isExceptionPending);
}

/**
 * @tc.name: ReferenceTest001
 * @tc.desc: Test reference type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ReferenceTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    napi_ref resultRef = nullptr;

    napi_create_object(env, &result);
    napi_create_reference(env, result, 1, &resultRef);

    uint32_t resultRefCount = 0;

    napi_reference_ref(env, resultRef, &resultRefCount);
    ASSERT_EQ(resultRefCount, static_cast<uint32_t>(2));

    napi_reference_unref(env, resultRef, &resultRefCount);
    ASSERT_EQ(resultRefCount, static_cast<uint32_t>(1));

    napi_value refValue = nullptr;
    napi_get_reference_value(env, resultRef, &refValue);

    ASSERT_NE(refValue, nullptr);

    napi_delete_reference(env, resultRef);
}

/**
 * @tc.name: CustomClassTest001
 * @tc.desc: Test define class.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, CustomClassTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    auto constructor = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value thisVar = nullptr;
        napi_value* argv = nullptr;
        size_t argc = 0;
        void* data = nullptr;
        napi_value constructor = nullptr;
        napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
        if (argc > 0) {
            argv = new napi_value[argc];
        }
        napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
        napi_get_new_target(env, info, &constructor);
        if (constructor == nullptr) {
            napi_throw_error(env, nullptr, "is not new instance");
        }
        if (argv != nullptr) {
            delete []argv;
        }
        return thisVar;
    };

    napi_value ln2 = nullptr;
    napi_value e = nullptr;

    napi_create_double(env, 2.718281828459045, &e);
    napi_create_double(env, 0.6931471805599453, &ln2);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("add", [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }),
        DECLARE_NAPI_FUNCTION("sub", [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }),
        DECLARE_NAPI_FUNCTION("mul", [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }),
        DECLARE_NAPI_FUNCTION("div", [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }),
        DECLARE_NAPI_STATIC_FUNCTION("getTime",
                                     [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }),
        DECLARE_NAPI_GETTER_SETTER(
            "pi", [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
            [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }),

    };

    napi_value customClass = nullptr;

    ASSERT_CHECK_CALL(napi_define_class(env, "CustomClass", NAPI_AUTO_LENGTH, constructor, nullptr,
                                        sizeof(desc) / sizeof(desc[0]), desc, &customClass));
    ASSERT_CHECK_VALUE_TYPE(env, customClass, napi_function);
    napi_value customClassPrototype = nullptr;
    napi_get_prototype(env, customClass, &customClassPrototype);
    ASSERT_CHECK_VALUE_TYPE(env, customClassPrototype, napi_function);

    napi_value customInstance = nullptr;
    ASSERT_CHECK_CALL(napi_new_instance(env, customClass, 0, nullptr, &customInstance));

    bool isInstanceOf = false;
    ASSERT_CHECK_CALL(napi_instanceof(env, customInstance, customClass, &isInstanceOf));
    ASSERT_TRUE(isInstanceOf);
}

/**
 * @tc.name: CreateMap001
 * @tc.desc: Test napi_create_map.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, CreateMap001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    res = napi_create_map(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &result));

    bool isMap = false;
    ASSERT_CHECK_CALL(napi_is_map(env, result, &isMap));
    ASSERT_EQ(isMap, true);
}

/**
 * @tc.name: CreateMap002
 * @tc.desc: Test napi_create_map.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, CreateMap002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &result));

    uint32_t length = 0;
    napi_value value = nullptr;
    bool hasKey = false;

    napi_value key = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, "null", NAPI_AUTO_LENGTH, &key));
    napi_value null = nullptr;
    ASSERT_CHECK_CALL(napi_get_null(env, &null));
    napi_value undefined = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &undefined));

    ASSERT_CHECK_CALL(napi_map_set_property(env, result, key, null));
    ASSERT_CHECK_CALL(napi_map_get_size(env, result, &length));
    ASSERT_EQ(length, 1);
    ASSERT_CHECK_CALL(napi_map_has_property(env, result, key, &hasKey));
    ASSERT_TRUE(hasKey);
    ASSERT_CHECK_CALL(napi_map_get_property(env, result, key, &value));
    ASSERT_CHECK_VALUE_TYPE(env, value, napi_null);

    ASSERT_CHECK_CALL(napi_map_delete_property(env, result, key));
    ASSERT_CHECK_CALL(napi_map_get_size(env, result, &length));
    ASSERT_EQ(length, 0);
    ASSERT_CHECK_CALL(napi_map_has_property(env, result, key, &hasKey));
    ASSERT_FALSE(hasKey);
    ASSERT_CHECK_CALL(napi_map_get_property(env, result, key, &value));
    ASSERT_CHECK_VALUE_TYPE(env, value, napi_undefined);
}

/**
 * @tc.name: CreateMap003
 * @tc.desc: Test napi_create_map.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, CreateMap003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &result));

    uint32_t length = 0;
    napi_value value = nullptr;
    bool hasKey = false;

    const char* key = "null";
    napi_value null = nullptr;
    ASSERT_CHECK_CALL(napi_get_null(env, &null));
    napi_value undefined = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &undefined));

    ASSERT_CHECK_CALL(napi_map_set_named_property(env, result, key, null));
    ASSERT_CHECK_CALL(napi_map_get_size(env, result, &length));
    ASSERT_EQ(length, 1);
    ASSERT_CHECK_CALL(napi_map_has_named_property(env, result, key, &hasKey));
    ASSERT_TRUE(hasKey);
    ASSERT_CHECK_CALL(napi_map_get_named_property(env, result, key, &value));
    ASSERT_CHECK_VALUE_TYPE(env, value, napi_null);

    ASSERT_CHECK_CALL(napi_map_clear(env, result));
    ASSERT_CHECK_CALL(napi_map_get_size(env, result, &length));
    ASSERT_EQ(length, 0);
    ASSERT_CHECK_CALL(napi_map_has_named_property(env, result, key, &hasKey));
    ASSERT_FALSE(hasKey);
    ASSERT_CHECK_CALL(napi_map_get_named_property(env, result, key, &value));
    ASSERT_CHECK_VALUE_TYPE(env, value, napi_undefined);

    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
    ASSERT_CHECK_CALL(napi_map_set_named_property(env, result, key, object));
    ASSERT_CHECK_CALL(napi_map_get_named_property(env, result, key, &value));
    ASSERT_CHECK_VALUE_TYPE(env, value, napi_object);
}

/**
 * @tc.name: CreateMap004
 * @tc.desc: Test napi_create_map.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, CreateMap004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &map));

    napi_value zero = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, 0, &zero));
    napi_value one = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, 1, &one));

    ASSERT_CHECK_CALL(napi_map_set_property(env, map, zero, one));

    napi_value entries;
    ASSERT_CHECK_CALL(napi_map_get_entries(env, map, &entries));

    napi_value entries0;
    ASSERT_CHECK_CALL(napi_map_iterator_get_next(env, entries, &entries0));
    napi_value entries0Value = nullptr;
    ASSERT_CHECK_CALL(napi_get_named_property(env, entries0, "value", &entries0Value));
    napi_value key = nullptr;
    ASSERT_CHECK_CALL(napi_get_element(env, entries0Value, 0, &key));
    int32_t nativeKey;
    ASSERT_CHECK_CALL(napi_get_value_int32(env, key, &nativeKey));
    ASSERT_EQ(nativeKey, 0);
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_get_element(env, entries0Value, 1, &value));
    int32_t nativeValue;
    ASSERT_CHECK_CALL(napi_get_value_int32(env, value, &nativeValue));
    ASSERT_EQ(nativeValue, 1);

    napi_value end;
    ASSERT_CHECK_CALL(napi_map_iterator_get_next(env, entries, &end));
    napi_value done = nullptr;
    ASSERT_CHECK_CALL(napi_get_named_property(env, end, "done", &done));
    bool isDone;
    ASSERT_CHECK_CALL(napi_get_value_bool(env, done, &isDone));
    ASSERT_TRUE(isDone);
}

/**
 * @tc.name: CreateMap005
 * @tc.desc: Test napi_create_map.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, CreateMap005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &map));

    napi_value zero = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, 0, &zero));
    napi_value one = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, 1, &one));

    ASSERT_CHECK_CALL(napi_map_set_property(env, map, zero, one));

    napi_value keys;
    ASSERT_CHECK_CALL(napi_map_get_keys(env, map, &keys));

    napi_value keys0;
    ASSERT_CHECK_CALL(napi_map_iterator_get_next(env, keys, &keys0));
    napi_value key = nullptr;
    ASSERT_CHECK_CALL(napi_get_named_property(env, keys0, "value", &key));
    int32_t nativeKey;
    ASSERT_CHECK_CALL(napi_get_value_int32(env, key, &nativeKey));
    ASSERT_EQ(nativeKey, 0);

    napi_value end;
    ASSERT_CHECK_CALL(napi_map_iterator_get_next(env, keys, &end));
    napi_value done = nullptr;
    ASSERT_CHECK_CALL(napi_get_named_property(env, end, "done", &done));
    bool isDone;
    ASSERT_CHECK_CALL(napi_get_value_bool(env, done, &isDone));
    ASSERT_TRUE(isDone);
}

/**
 * @tc.name: CreateMap006
 * @tc.desc: Test napi_create_map.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, CreateMap006, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &map));

    napi_value zero = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, 0, &zero));
    napi_value one = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, 1, &one));

    ASSERT_CHECK_CALL(napi_map_set_property(env, map, zero, one));

    napi_value values;
    ASSERT_CHECK_CALL(napi_map_get_values(env, map, &values));

    napi_value values0;
    ASSERT_CHECK_CALL(napi_map_iterator_get_next(env, values, &values0));
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_get_named_property(env, values0, "value", &value));
    int32_t nativeValue;
    ASSERT_CHECK_CALL(napi_get_value_int32(env, value, &nativeValue));
    ASSERT_EQ(nativeValue, 1);

    napi_value end;
    ASSERT_CHECK_CALL(napi_map_iterator_get_next(env, values, &end));
    napi_value done = nullptr;
    ASSERT_CHECK_CALL(napi_get_named_property(env, end, "done", &done));
    bool isDone;
    ASSERT_CHECK_CALL(napi_get_value_bool(env, done, &isDone));
    ASSERT_TRUE(isDone);
}

/**
 * @tc.name: AsyncWorkTest001
 * @tc.desc: Test async work.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AsyncWorkTest001, testing::ext::TestSize.Level1)
{
    struct AsyncWorkContext {
        napi_async_work work = nullptr;
    };
    napi_env env = (napi_env)engine_;
    auto asyncWorkContext = new AsyncWorkContext();
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, "AsyncWorkTest", NAPI_AUTO_LENGTH, &resourceName);
    ASSERT_CHECK_CALL(napi_create_async_work(
        env, nullptr, resourceName, [](napi_env value, void* data) {},
        [](napi_env env, napi_status status, void* data) {
            STOP_EVENT_LOOP(env);
            AsyncWorkContext* asyncWorkContext = (AsyncWorkContext*)data;
            napi_status deleteStatus = napi_delete_async_work(env, asyncWorkContext->work);
            delete asyncWorkContext;
            ASSERT_EQ(deleteStatus, napi_ok);
        },
        asyncWorkContext, &asyncWorkContext->work));
    ASSERT_CHECK_CALL(napi_queue_async_work(env, asyncWorkContext->work));
    RUN_EVENT_LOOP(env);
}

/**
 * @tc.name: AsyncWorkTest003
 * @tc.desc: Test async work.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AsyncWorkTest003, testing::ext::TestSize.Level1)
{
    struct AsyncWorkContext {
        napi_async_work work = nullptr;
    };
    napi_env env = reinterpret_cast<napi_env>(engine_);
    std::unique_ptr<AsyncWorkContext> asyncWorkContext = std::make_unique<AsyncWorkContext>();
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, TEST_CHAR_ASYNCWORK, NAPI_AUTO_LENGTH, &resourceName);
    napi_status status = napi_create_async_work(
        env, nullptr, nullptr, [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {},
        asyncWorkContext.get(), &asyncWorkContext->work);
    ASSERT_EQ(status, napi_invalid_arg);

    status = napi_create_async_work(
        env, nullptr, resourceName, nullptr,
        [](napi_env env, napi_status status, void* data) {},
        asyncWorkContext.get(), &asyncWorkContext->work);
    ASSERT_EQ(status, napi_invalid_arg);

    status = napi_create_async_work(
        env, nullptr, resourceName, [](napi_env env, void* data) {},
        nullptr,
        asyncWorkContext.get(), &asyncWorkContext->work);
    ASSERT_EQ(status, napi_invalid_arg);

    status = napi_create_async_work(
        env, nullptr, resourceName, [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {},
        nullptr, &asyncWorkContext->work);
    ASSERT_EQ(status, napi_ok);

    status = napi_create_async_work(
        env, nullptr, resourceName, [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {},
        asyncWorkContext.get(), nullptr);
    ASSERT_EQ(status, napi_invalid_arg);
}

/**
 * @tc.name: AsyncWorkTest004
 * @tc.desc: Test async work.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AsyncWorkTest004, testing::ext::TestSize.Level1)
{
    struct AsyncWorkContext {
        napi_async_work work = nullptr;
    };
    napi_env env = reinterpret_cast<napi_env>(engine_);
    auto asyncWorkContext = new AsyncWorkContext();
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, TEST_CHAR_ASYNCWORK, NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        env, nullptr, resourceName, [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {
            AsyncWorkContext* asyncWorkContext = reinterpret_cast<AsyncWorkContext*>(data);
            ASSERT_NE(asyncWorkContext, nullptr);
            delete asyncWorkContext;
        },
        nullptr, &asyncWorkContext->work);
    napi_delete_async_work(env, asyncWorkContext->work);
}

/**
 * @tc.name: AsyncWorkTest005
 * @tc.desc: Test async work.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AsyncWorkTest005, testing::ext::TestSize.Level1)
{
    struct AsyncWorkContext {
        napi_async_work work = nullptr;
    };
    napi_env env = reinterpret_cast<napi_env>(engine_);
    auto asyncWorkContext = new AsyncWorkContext();
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, TEST_CHAR_ASYNCWORK, NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        env, nullptr, resourceName, [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {
            STOP_EVENT_LOOP(env);
            AsyncWorkContext* asyncWorkContext = reinterpret_cast<AsyncWorkContext*>(data);
            ASSERT_NE(asyncWorkContext, nullptr);
            delete asyncWorkContext;
        },
        asyncWorkContext, &asyncWorkContext->work);
    napi_status status = napi_queue_async_work(env, asyncWorkContext->work);
    ASSERT_EQ(status, napi_ok);
    status = napi_queue_async_work(env, nullptr);
    ASSERT_EQ(status, napi_invalid_arg);
    RUN_EVENT_LOOP(env);
}

/**
 * @tc.name: AsyncWorkTest006
 * @tc.desc: Test async work.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AsyncWorkTest006, testing::ext::TestSize.Level1)
{
    struct AsyncWorkContext {
        napi_async_work work = nullptr;
    };
    napi_env env = (napi_env)engine_;
    auto asyncWorkContext = new AsyncWorkContext();
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, "中测", NAPI_AUTO_LENGTH, &resourceName);
    ASSERT_CHECK_CALL(napi_create_async_work(
        env, nullptr, resourceName, [](napi_env value, void* data) {},
        [](napi_env env, napi_status status, void* data) {
            STOP_EVENT_LOOP(env);
            AsyncWorkContext* asyncWorkContext = (AsyncWorkContext*)data;
            napi_status deleteStatus = napi_delete_async_work(env, asyncWorkContext->work);
            delete asyncWorkContext;
            ASSERT_EQ(deleteStatus, napi_ok);
        },
        asyncWorkContext, &asyncWorkContext->work));
    ASSERT_CHECK_CALL(napi_queue_async_work(env, asyncWorkContext->work));
    RUN_EVENT_LOOP(env);
}

/**
 * @tc.name: ObjectWrapperTest001
 * @tc.desc: Test object wrapper.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ObjectWrapperTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value testClass = nullptr;
    napi_define_class(
        env, "TestClass", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);

            return thisVar;
        },
        nullptr, 0, nullptr, &testClass);

    napi_value instanceValue = nullptr;
    napi_new_instance(env, testClass, 0, nullptr, &instanceValue);

    const char* testStr = "test";
    napi_wrap(
        env, instanceValue, (void*)testStr, [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);

    char* tmpTestStr = nullptr;
    napi_unwrap(env, instanceValue, (void**)&tmpTestStr);
    ASSERT_STREQ(testStr, tmpTestStr);

    char* tmpTestStr1 = nullptr;
    napi_remove_wrap(env, instanceValue, (void**)&tmpTestStr1);
    ASSERT_STREQ(testStr, tmpTestStr1);
}

/**
 * @tc.name: ObjectWrapperTest002
 * @tc.desc: Test object wrapper.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ObjectWrapperTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value testClass = nullptr;
    napi_define_class(
        env, "TestClass", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);

            return thisVar;
        },
        nullptr, 0, nullptr, &testClass);

    napi_value instanceValue = nullptr;
    napi_new_instance(env, testClass, 0, nullptr, &instanceValue);

    const char* testStr = "中测";
    napi_wrap(
        env, instanceValue, (void*)testStr, [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);

    char* tmpTestStr = nullptr;
    napi_unwrap(env, instanceValue, (void**)&tmpTestStr);
    ASSERT_STREQ(testStr, tmpTestStr);

    char* tmpTestStr1 = nullptr;
    napi_remove_wrap(env, instanceValue, (void**)&tmpTestStr1);
    ASSERT_STREQ(testStr, tmpTestStr1);
}

/**
 * @tc.name: ObjectWrapperTest003
 * @tc.desc: Test object wrapper.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ObjectWrapperTest003, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value testClass = nullptr;
    napi_define_class(
        env, "TestClass", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);

            return thisVar;
        },
        nullptr, 0, nullptr, &testClass);

    napi_value instanceValue = nullptr;
    napi_new_instance(env, testClass, 0, nullptr, &instanceValue);

    const char* testStr = "123456"; // 123456: test numbers
    napi_wrap(
        env, instanceValue, (void*)testStr, [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);

    char* tmpTestStr = nullptr;
    napi_unwrap(env, instanceValue, (void**)&tmpTestStr);
    ASSERT_STREQ(testStr, tmpTestStr);

    char* tmpTestStr1 = nullptr;
    napi_remove_wrap(env, instanceValue, (void**)&tmpTestStr1);
    ASSERT_STREQ(testStr, tmpTestStr1);
}

/**
 * @tc.name: ObjectWrapperTest004
 * @tc.desc: Test object wrapper.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ObjectWrapperTest004, testing::ext::TestSize.Level1)
{
    std::thread t([]() {
        NativeEngineProxy engine;

        napi_value object = nullptr;
        ASSERT_EQ(napi_create_object(engine, &object), napi_ok);
        auto finalizer = [](napi_env, void* data, void*) {
            delete reinterpret_cast<uint8_t*>(data);
        };
        uint8_t* data0 = new uint8_t;
        ASSERT_EQ(napi_wrap(engine, object, data0, finalizer, nullptr, nullptr), napi_ok);

        LoggerCollector collector(LogLevel::LOG_WARN, LOG_DOMAIN);
        uint8_t* data1 = new uint8_t;
        ASSERT_EQ(napi_wrap(engine, object, data1, finalizer, nullptr, nullptr), napi_ok);
        ASSERT_TRUE(collector.Includes("napi_wrap: current js_object has been wrapped."));
    });
    t.join();
}

/**
 * @tc.name: ObjectWrapperTest005
 * @tc.desc: Test object wrapper.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ObjectWrapperTest005, testing::ext::TestSize.Level1)
{
    std::thread t([]() {
        NativeEngineProxy engine;

        napi_value object = nullptr;
        ASSERT_EQ(napi_create_object(engine, &object), napi_ok);
        auto finalizer = [](napi_env, void* data, void*) {
            delete reinterpret_cast<uint8_t*>(data);
        };
        uint8_t* data0 = new uint8_t;
        ASSERT_EQ(napi_wrap_async_finalizer(engine, object, data0, finalizer, nullptr, nullptr, sizeof(data0)),
                  napi_ok);

        LoggerCollector collector(LogLevel::LOG_WARN, LOG_DOMAIN);
        uint8_t* data1 = new uint8_t;
        ASSERT_EQ(napi_wrap_async_finalizer(engine, object, data1, finalizer, nullptr, nullptr, sizeof(data0)),
                  napi_ok);
        ASSERT_TRUE(collector.Includes("napi_wrap_async_finalizer: current js_object has been wrapped."));
    });
    t.join();
}

/**
 * @tc.name: ObjectWrapperTest006
 * @tc.desc: Test object wrapper.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ObjectWrapperTest006, testing::ext::TestSize.Level1)
{
    std::thread t([]() {
        NativeEngineProxy engine;

        napi_value object = nullptr;
        ASSERT_EQ(napi_create_object(engine, &object), napi_ok);
        auto finalizer = [](napi_env, void* data, void*) {
            delete reinterpret_cast<uint8_t*>(data);
        };
        uint8_t* data0 = new uint8_t;
        ASSERT_EQ(napi_wrap_with_size(engine, object, data0, finalizer, nullptr, nullptr, sizeof(data0)), napi_ok);

        LoggerCollector collector(LogLevel::LOG_WARN, LOG_DOMAIN);
        uint8_t* data1 = new uint8_t;
        ASSERT_EQ(napi_wrap_with_size(engine, object, data1, finalizer, nullptr, nullptr, sizeof(data1)), napi_ok);
        ASSERT_TRUE(collector.Includes("napi_wrap_with_size: current js_object has been wrapped."));
    });
    t.join();
}

/**
 * @tc.name: StrictEqualsTest001
 * @tc.desc: Test date type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StrictEqualsTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    const char* testStringStr = "test";
    napi_value testString = nullptr;
    napi_create_string_utf8(env, testStringStr, strlen(testStringStr), &testString);
    bool isStrictEquals = false;
    napi_strict_equals(env, testString, testString, &isStrictEquals);
    ASSERT_TRUE(isStrictEquals);

    napi_value testObject = nullptr;
    napi_create_object(env, &testObject);
    isStrictEquals = false;
    napi_strict_equals(env, testObject, testObject, &isStrictEquals);
    ASSERT_TRUE(isStrictEquals);
}

/**
 * @tc.name: StrictEqualsTest002
 * @tc.desc: Test date type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StrictEqualsTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    const char* testStringStr = "中测";
    napi_value testString = nullptr;
    napi_create_string_utf8(env, testStringStr, strlen(testStringStr), &testString);
    bool isStrictEquals = false;
    napi_strict_equals(env, testString, testString, &isStrictEquals);
    ASSERT_TRUE(isStrictEquals);

    napi_value testObject = nullptr;
    napi_create_object(env, &testObject);
    isStrictEquals = false;
    napi_strict_equals(env, testObject, testObject, &isStrictEquals);
    ASSERT_TRUE(isStrictEquals);
}

/**
 * @tc.name: StrictEqualsTest003
 * @tc.desc: Test date type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StrictEqualsTest003, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    const char* testStringStr = "123456"; // 123456: test numbers
    napi_value testString = nullptr;
    napi_create_string_utf8(env, testStringStr, strlen(testStringStr), &testString);
    bool isStrictEquals = false;
    napi_strict_equals(env, testString, testString, &isStrictEquals);
    ASSERT_TRUE(isStrictEquals);

    napi_value testObject = nullptr;
    napi_create_object(env, &testObject);
    isStrictEquals = false;
    napi_strict_equals(env, testObject, testObject, &isStrictEquals);
    ASSERT_TRUE(isStrictEquals);
}

/**
 * @tc.name: CreateRuntimeTest001
 * @tc.desc: Test create runtime.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, CreateRuntimeTest001, testing::ext::TestSize.Level1)
{
    std::thread t([this]() {
        napi_env env = (napi_env)engine_;

        napi_env newEnv = nullptr;
        napi_status status = napi_create_runtime(env, &newEnv);
        ASSERT_EQ(status, napi_ok);
        if (newEnv != nullptr) {
            delete reinterpret_cast<NativeEngine *>(newEnv);
        }
    });
    t.join();
}

/**
 * @tc.name: SerializeDeSerializeTest001
 * @tc.desc: Test serialize & deserialize.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, SerializeDeSerializeTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value num = nullptr;
    uint32_t value = 1000;
    napi_create_uint32(env, value, &num);
    void* data = nullptr;
    napi_serialize_inner(env, num, undefined, undefined, false, true, &data);
    ASSERT_NE(data, nullptr);

    napi_value result = nullptr;
    napi_deserialize(env, data, &result);
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_number);
    napi_delete_serialization_data(env, data);
    int32_t resultData = 0;
    napi_get_value_int32(env, result, &resultData);
    ASSERT_EQ(resultData, 1000);
}

/**
 * @tc.name: SerializeDeSerializeTest002
 * @tc.desc: Test serialize & deserialize.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, SerializeDeSerializeTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value num = nullptr;
    uint32_t value = 1000;
    napi_create_uint32(env, value, &num);
    void* data = nullptr;
    napi_serialize_inner(env, num, undefined, undefined, false, true, &data);
    ASSERT_NE(data, nullptr);

    napi_value result1 = nullptr;
    napi_deserialize(env, data, &result1);
    ASSERT_CHECK_VALUE_TYPE(env, result1, napi_number);
    int32_t resultData1 = 0;
    napi_get_value_int32(env, result1, &resultData1);
    ASSERT_EQ(resultData1, 1000);

    napi_value result2 = nullptr;
    napi_deserialize(env, data, &result2);
    ASSERT_CHECK_VALUE_TYPE(env, result2, napi_number);
    int32_t resultData2 = 0;
    napi_get_value_int32(env, result2, &resultData2);
    ASSERT_EQ(resultData2, 1000);

    napi_delete_serialization_data(env, data);
}

/**
 * @tc.name: SerializeDeSerializeTest003
 * @tc.desc: Test nativeBinding object type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, SerializeDeSerializeTest003, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_value hint = nullptr;
    napi_create_object(env, &hint);
    napi_value object1 = nullptr;
    napi_create_object(env, &object1);
    napi_status status = napi_coerce_to_native_binding_object(env, object,
        TestDetachCallback, TestAttachCallback, reinterpret_cast<void*>(object1), reinterpret_cast<void*>(hint));
    ASSERT_EQ(status, napi_status::napi_ok);
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    void* data = nullptr;
    napi_serialize_inner(env, object, undefined, undefined, false, true, &data);
    ASSERT_NE(data, nullptr);

    napi_value result1 = nullptr;
    napi_deserialize(env, data, &result1);
    ASSERT_CHECK_VALUE_TYPE(env, result1, napi_object);
    napi_value number1 = nullptr;
    napi_get_named_property(env, result1, "number", &number1);
    ASSERT_CHECK_VALUE_TYPE(env, number1, napi_number);
    uint32_t numData1 = 0;
    napi_get_value_uint32(env, number1, &numData1);
    ASSERT_EQ(numData1, 2000);

    napi_value result2 = nullptr;
    napi_deserialize(env, data, &result2);
    ASSERT_CHECK_VALUE_TYPE(env, result2, napi_object);
    napi_value number2 = nullptr;
    napi_get_named_property(env, result2, "number", &number2);
    ASSERT_CHECK_VALUE_TYPE(env, number2, napi_number);
    uint32_t numData2 = 0;
    napi_get_value_uint32(env, number2, &numData2);
    ASSERT_EQ(numData2, 2000);

    napi_delete_serialization_data(env, data);
}

/**
 * @tc.name: SerializeDeSerializeTest004
 * @tc.desc: Test nativeBinding object type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, SerializeDeSerializeTest004, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_value num = nullptr;
    uint32_t value = 1000;
    napi_create_uint32(env, value, &num);
    napi_set_named_property(env, object, "numKey", num);
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    napi_set_named_property(env, object, "objKey", obj);

    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    void* data = nullptr;
    napi_serialize_inner(env, object, undefined, undefined, false, true, &data);
    ASSERT_NE(data, nullptr);

    napi_value result1 = nullptr;
    napi_deserialize(env, data, &result1);
    ASSERT_CHECK_VALUE_TYPE(env, result1, napi_object);
    napi_value obj1 = nullptr;
    napi_get_named_property(env, result1, "objKey", &obj1);
    ASSERT_CHECK_VALUE_TYPE(env, obj1, napi_object);

    napi_value result2 = nullptr;
    napi_deserialize(env, data, &result2);
    ASSERT_CHECK_VALUE_TYPE(env, result2, napi_object);
    napi_value num1 = nullptr;
    napi_get_named_property(env, result2, "numKey", &num1);
    uint32_t value1 = 0;
    napi_get_value_uint32(env, num1, &value1);
    ASSERT_EQ(value1, 1000);

    napi_delete_serialization_data(env, data);
}

/**
 * @tc.name: SerializeDeSerializeTest005
 * @tc.desc: Test serialize & deserialize.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, SerializeDeSerializeTest005, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value num = nullptr;
    uint32_t value = 1000;
    napi_create_uint32(env, value, &num);
    void* data = nullptr;
    napi_serialize(env, num, undefined, undefined, &data);
    ASSERT_NE(data, nullptr);

    napi_value result = nullptr;
    napi_deserialize(env, data, &result);
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_number);
    napi_delete_serialization_data(env, data);
    int32_t resultData = 0;
    napi_get_value_int32(env, result, &resultData);
    ASSERT_EQ(resultData, 1000);
}

/**
 * @tc.name: SerializeDeSerializeTest006
 * @tc.desc: Test serialize & deserialize.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, SerializeDeSerializeTest006, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value num = nullptr;
    uint32_t value = 1000;
    napi_create_uint32(env, value, &num);
    void* data = nullptr;
    napi_serialize(env, num, undefined, undefined, &data);
    ASSERT_NE(data, nullptr);

    napi_value result1 = nullptr;
    napi_deserialize(env, data, &result1);
    ASSERT_CHECK_VALUE_TYPE(env, result1, napi_number);
    int32_t resultData1 = 0;
    napi_get_value_int32(env, result1, &resultData1);
    ASSERT_EQ(resultData1, 1000);

    napi_value result2 = nullptr;
    napi_deserialize(env, data, &result2);
    ASSERT_CHECK_VALUE_TYPE(env, result2, napi_number);
    int32_t resultData2 = 0;
    napi_get_value_int32(env, result2, &resultData2);
    ASSERT_EQ(resultData2, 1000);

    napi_delete_serialization_data(env, data);
}

/**
 * @tc.name: SerializeDeSerializeTest007
 * @tc.desc: Test nativeBinding object type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, SerializeDeSerializeTest007, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_value hint = nullptr;
    napi_create_object(env, &hint);
    napi_value object1 = nullptr;
    napi_create_object(env, &object1);
    napi_status status = napi_coerce_to_native_binding_object(env, object,
        TestDetachCallback, TestAttachCallback, reinterpret_cast<void*>(object1), reinterpret_cast<void*>(hint));
    ASSERT_EQ(status, napi_status::napi_ok);
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    void* data = nullptr;
    napi_serialize(env, object, undefined, undefined, &data);
    ASSERT_NE(data, nullptr);

    napi_value result1 = nullptr;
    napi_deserialize(env, data, &result1);
    ASSERT_CHECK_VALUE_TYPE(env, result1, napi_object);
    napi_value number1 = nullptr;
    napi_get_named_property(env, result1, "number", &number1);
    ASSERT_CHECK_VALUE_TYPE(env, number1, napi_number);
    uint32_t numData1 = 0;
    napi_get_value_uint32(env, number1, &numData1);
    ASSERT_EQ(numData1, 2000);

    napi_value result2 = nullptr;
    napi_deserialize(env, data, &result2);
    ASSERT_CHECK_VALUE_TYPE(env, result2, napi_object);
    napi_value number2 = nullptr;
    napi_get_named_property(env, result2, "number", &number2);
    ASSERT_CHECK_VALUE_TYPE(env, number2, napi_number);
    uint32_t numData2 = 0;
    napi_get_value_uint32(env, number2, &numData2);
    ASSERT_EQ(numData2, 2000);

    napi_delete_serialization_data(env, data);
}

/**
 * @tc.name: SerializeDeSerializeTest008
 * @tc.desc: Test nativeBinding object type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, SerializeDeSerializeTest008, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_value num = nullptr;
    uint32_t value = 1000;
    napi_create_uint32(env, value, &num);
    napi_set_named_property(env, object, "numKey", num);
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    napi_set_named_property(env, object, "objKey", obj);

    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    void* data = nullptr;
    napi_serialize(env, object, undefined, undefined, &data);
    ASSERT_NE(data, nullptr);

    napi_value result1 = nullptr;
    napi_deserialize(env, data, &result1);
    ASSERT_CHECK_VALUE_TYPE(env, result1, napi_object);
    napi_value obj1 = nullptr;
    napi_get_named_property(env, result1, "objKey", &obj1);
    ASSERT_CHECK_VALUE_TYPE(env, obj1, napi_object);

    napi_value result2 = nullptr;
    napi_deserialize(env, data, &result2);
    ASSERT_CHECK_VALUE_TYPE(env, result2, napi_object);
    napi_value num1 = nullptr;
    napi_get_named_property(env, result2, "numKey", &num1);
    uint32_t value1 = 0;
    napi_get_value_uint32(env, num1, &value1);
    ASSERT_EQ(value1, 1000);

    napi_delete_serialization_data(env, data);
}

/**
 * @tc.name: SerializeDeSerializeTest009
 * @tc.desc: Test serialize & deserialize.
 * @tc.type: FUNC
 */
 HWTEST_F(NapiBasicTest, SerializeDeSerializeTest009, testing::ext::TestSize.Level1)
 {
    napi_env env = (napi_env)engine_;

    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_value num = nullptr;
    uint32_t value = 1000;
    napi_create_uint32(env, value, &num);
    napi_set_named_property(env, object, "numKey", num);
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    napi_set_named_property(env, object, "objKey", obj);

    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    void* data = nullptr;
    std::string err = "";
    napi_serialize_inner_with_error(env, object, undefined, undefined, false, true, &data, err);
    ASSERT_NE(data, nullptr);

    napi_value result1 = nullptr;
    napi_deserialize(env, data, &result1);
    ASSERT_CHECK_VALUE_TYPE(env, result1, napi_object);
    napi_value obj1 = nullptr;
    napi_get_named_property(env, result1, "objKey", &obj1);
    ASSERT_CHECK_VALUE_TYPE(env, obj1, napi_object);

    napi_value result2 = nullptr;
    napi_deserialize(env, data, &result2);
    ASSERT_CHECK_VALUE_TYPE(env, result2, napi_object);
    napi_value num1 = nullptr;
    napi_get_named_property(env, result2, "numKey", &num1);
    uint32_t value1 = 0;
    napi_get_value_uint32(env, num1, &value1);
    ASSERT_EQ(value1, 1000);

    napi_delete_serialization_data(env, data);
 }

/**
 * @tc.name: IsCallableTest001
 * @tc.desc: Test is callable.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, IsCallableTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    auto func = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value thisVar;
        napi_value* argv = nullptr;
        size_t argc = 0;
        void* data = nullptr;

        napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
        if (argc > 0) {
            argv = new napi_value[argc];
        }
        napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

        napi_value result = nullptr;
        napi_create_object(env, &result);

        napi_value messageKey = nullptr;
        const char* messageKeyStr = "message";
        napi_create_string_latin1(env, messageKeyStr, strlen(messageKeyStr), &messageKey);
        napi_value messageValue = nullptr;
        const char* messageValueStr = "OK";
        napi_create_string_latin1(env, messageValueStr, strlen(messageValueStr), &messageValue);
        napi_set_property(env, result, messageKey, messageValue);

        if (argv != nullptr) {
            delete []argv;
        }
        return result;
    };

    napi_value funcValue = nullptr;
    napi_create_function(env, "testFunc", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    ASSERT_NE(funcValue, nullptr);

    bool result = false;
    napi_is_callable(env, funcValue, &result);
    ASSERT_TRUE(result);
}

/**
 * @tc.name: EncodeToUtf8Test001
 * @tc.desc: Test EncodeToUtf8 Func.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, EncodeToUtf8Test001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    std::string str = "encode";
    napi_value testStr = nullptr;
    napi_create_string_utf8(env, str.c_str(), str.length(), &testStr);
    char* buffer = new char[str.length()];
    size_t bufferSize = str.length();
    uint32_t written = 0;
    int32_t nchars = 0;
    ASSERT_EQ(memset_s(buffer, str.length(), 0, str.length()), EOK);
    engine_->EncodeToUtf8(testStr, buffer, &written, bufferSize, &nchars);
    ASSERT_EQ(written, 6);
    ASSERT_EQ(nchars, 6);
    delete[] buffer;

    str = "encode\xc2\xab\xe2\x98\x80";
    testStr = nullptr;
    napi_create_string_utf8(env, str.c_str(), str.length(), &testStr);
    buffer = new char[str.length()];
    bufferSize = str.length();
    ASSERT_EQ(memset_s(buffer, str.length(), 0, str.length()), EOK);
    engine_->EncodeToUtf8(testStr, buffer, &written, bufferSize, &nchars);
    ASSERT_EQ(written, 11);
    ASSERT_EQ(nchars, 8);
    delete[] buffer;

    buffer = new char[str.length()];
    bufferSize = str.length();
    ASSERT_EQ(memset_s(buffer, str.length(), 0, str.length()), EOK);
    bufferSize--;
    engine_->EncodeToUtf8(testStr, buffer, &written, bufferSize, &nchars);
    ASSERT_EQ(written, 8);
    ASSERT_EQ(nchars, 7);
    delete[] buffer;

    buffer = new char[str.length()];
    bufferSize = str.length();
    ASSERT_EQ(memset_s(buffer, str.length(), 0, str.length()), EOK);
    bufferSize -= 4;
    engine_->EncodeToUtf8(testStr, buffer, &written, bufferSize, &nchars);
    ASSERT_EQ(written, 6);
    ASSERT_EQ(nchars, 6);
    delete[] buffer;

    str = "encode\xc2\xab\xe2\x98\x80t";
    testStr = nullptr;
    napi_create_string_utf8(env, str.c_str(), str.length(), &testStr);
    buffer = new char[str.length()];
    bufferSize = str.length();
    ASSERT_EQ(memset_s(buffer, str.length(), 0, str.length()), EOK);
    bufferSize--;
    engine_->EncodeToUtf8(testStr, buffer, &written, bufferSize, &nchars);
    ASSERT_EQ(written, 11);
    ASSERT_EQ(nchars, 8);
    delete[] buffer;

    str = "";
    testStr = nullptr;
    napi_create_string_utf8(env, str.c_str(), str.length(), &testStr);
    buffer = new char[str.length() + 1];
    bufferSize = str.length() + 1;
    ASSERT_EQ(memset_s(buffer, str.length(), 0, str.length()), EOK);
    engine_->EncodeToUtf8(testStr, buffer, &written, bufferSize, &nchars);
    ASSERT_EQ(written, 0);
    ASSERT_EQ(nchars, 0);
    delete[] buffer;
}

/**
 * @tc.name: EncodeToUtf8Test002
 * @tc.desc: Test EncodeToUtf8 Func.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, EncodeToUtf8Test002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    std::string str = "中测";
    napi_value testStr = nullptr;
    napi_create_string_utf8(env, str.c_str(), str.length(), &testStr);
    char* buffer = new char[str.length()];
    size_t bufferSize = str.length();
    uint32_t written = 0;
    int32_t nchars = 0;
    ASSERT_EQ(memset_s(buffer, str.length(), 0, str.length()), EOK);
    engine_->EncodeToUtf8(testStr, buffer, &written, bufferSize, &nchars);
    ASSERT_EQ(written, 6);
    ASSERT_EQ(nchars, 2);
    delete[] buffer;

    str = "中测\xc2\xab\xe2\x98\x80";
    testStr = nullptr;
    napi_create_string_utf8(env, str.c_str(), str.length(), &testStr);
    buffer = new char[str.length()];
    bufferSize = str.length();
    ASSERT_EQ(memset_s(buffer, str.length(), 0, str.length()), EOK);
    engine_->EncodeToUtf8(testStr, buffer, &written, bufferSize, &nchars);
    ASSERT_EQ(written, 11);
    ASSERT_EQ(nchars, 4);
    delete[] buffer;

    buffer = new char[str.length()];
    bufferSize = str.length();
    ASSERT_EQ(memset_s(buffer, str.length(), 0, str.length()), EOK);
    bufferSize--;
    engine_->EncodeToUtf8(testStr, buffer, &written, bufferSize, &nchars);
    ASSERT_EQ(written, 8);
    ASSERT_EQ(nchars, 3);
    delete[] buffer;

    buffer = new char[str.length()];
    bufferSize = str.length();
    ASSERT_EQ(memset_s(buffer, str.length(), 0, str.length()), EOK);
    bufferSize -= 4;
    engine_->EncodeToUtf8(testStr, buffer, &written, bufferSize, &nchars);
    ASSERT_EQ(written, 6);
    ASSERT_EQ(nchars, 2);
    delete[] buffer;

    str = "中测\xc2\xab\xe2\x98\x80t";
    testStr = nullptr;
    napi_create_string_utf8(env, str.c_str(), str.length(), &testStr);
    buffer = new char[str.length()];
    bufferSize = str.length();
    ASSERT_EQ(memset_s(buffer, str.length(), 0, str.length()), EOK);
    bufferSize--;
    engine_->EncodeToUtf8(testStr, buffer, &written, bufferSize, &nchars);
    ASSERT_EQ(written, 11);
    ASSERT_EQ(nchars, 4);
    delete[] buffer;

    str = "";
    testStr = nullptr;
    napi_create_string_utf8(env, str.c_str(), str.length(), &testStr);
    buffer = new char[str.length() + 1];
    bufferSize = str.length() + 1;
    ASSERT_EQ(memset_s(buffer, str.length(), 0, str.length()), EOK);
    engine_->EncodeToUtf8(testStr, buffer, &written, bufferSize, &nchars);
    ASSERT_EQ(written, 0);
    ASSERT_EQ(nchars, 0);
    delete[] buffer;
}

/**
 * @tc.name: WrapWithSizeTest001
 * @tc.desc: Test wrap with size.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, WrapWithSizeTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value testWrapClass = nullptr;
    napi_define_class(
        env, "TestWrapClass", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);

            return thisVar;
        },
        nullptr, 0, nullptr, &testWrapClass);

    napi_value instanceValue = nullptr;
    napi_new_instance(env, testWrapClass, 0, nullptr, &instanceValue);

    const char* testWrapStr = "testWrapStr";
    size_t size = sizeof(*testWrapStr) / sizeof(char);
    napi_wrap_with_size(
        env, instanceValue, (void*)testWrapStr, [](napi_env env, void* data, void* hint) {}, nullptr, nullptr, size);

    char* tempTestStr = nullptr;
    napi_unwrap(env, instanceValue, (void**)&tempTestStr);
    ASSERT_STREQ(testWrapStr, tempTestStr);

    char* tempTestStr1 = nullptr;
    napi_remove_wrap(env, instanceValue, (void**)&tempTestStr1);
    ASSERT_STREQ(testWrapStr, tempTestStr1);

}

/**
 * @tc.name: WrapWithSizeTest002
 * @tc.desc: Test wrap with size.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, WrapWithSizeTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value testWrapClass = nullptr;
    napi_define_class(
        env, "TestWrapClass", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);

            return thisVar;
        },
        nullptr, 0, nullptr, &testWrapClass);

    napi_value instanceValue = nullptr;
    napi_new_instance(env, testWrapClass, 0, nullptr, &instanceValue);

    const char* testWrapStr = "中测";
    size_t size = sizeof(*testWrapStr) / sizeof(char);
    napi_wrap_with_size(
        env, instanceValue, (void*)testWrapStr, [](napi_env env, void* data, void* hint) {}, nullptr, nullptr, size);

    char* tempTestStr = nullptr;
    napi_unwrap(env, instanceValue, (void**)&tempTestStr);
    ASSERT_STREQ(testWrapStr, tempTestStr);

    char* tempTestStr1 = nullptr;
    napi_remove_wrap(env, instanceValue, (void**)&tempTestStr1);
    ASSERT_STREQ(testWrapStr, tempTestStr1);
}

/**
 * @tc.name: WrapEnhanceTest001
 * @tc.desc: Test wrap with size and finalize_cb execute sync.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, WrapEnhanceTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value testWrapClass = nullptr;
    napi_define_class(
        env, "TestWrapClass", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);

            return thisVar;
        },
        nullptr, 0, nullptr, &testWrapClass);

    napi_value instanceValue = nullptr;
    napi_new_instance(env, testWrapClass, 0, nullptr, &instanceValue);

    const char* testWrapStr = "WrapEnhance";
    size_t size = sizeof(*testWrapStr) / sizeof(char);
    napi_wrap_enhance(
        env, instanceValue, (void*)testWrapStr, [](napi_env env, void* data, void* hint) {}, false, nullptr, size,
        nullptr);

    char* tempTestStr = nullptr;
    napi_unwrap(env, instanceValue, (void**)&tempTestStr);
    ASSERT_STREQ(testWrapStr, tempTestStr);

    char* tempTestStr1 = nullptr;
    napi_remove_wrap(env, instanceValue, (void**)&tempTestStr1);
    ASSERT_STREQ(testWrapStr, tempTestStr1);
}

/**
 * @tc.name: WrapEnhanceTest002
 * @tc.desc: Test wrap with size and finalize_cb execute async.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, WrapEnhanceTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value testWrapClass = nullptr;
    napi_define_class(
        env, "TestWrapClass", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);

            return thisVar;
        },
        nullptr, 0, nullptr, &testWrapClass);

    napi_value instanceValue = nullptr;
    napi_new_instance(env, testWrapClass, 0, nullptr, &instanceValue);

    const char* testWrapStr = "WrapEnhance";
    size_t size = sizeof(*testWrapStr) / sizeof(char);
    napi_wrap_enhance(
        env, instanceValue, (void*)testWrapStr, [](napi_env env, void* data, void* hint) {}, true, nullptr, size,
        nullptr);

    char* tempTestStr = nullptr;
    napi_unwrap(env, instanceValue, (void**)&tempTestStr);
    ASSERT_STREQ(testWrapStr, tempTestStr);

    char* tempTestStr1 = nullptr;
    napi_remove_wrap(env, instanceValue, (void**)&tempTestStr1);
    ASSERT_STREQ(testWrapStr, tempTestStr1);
}

/**
 * @tc.name: CreateExternalWithSizeTest001
 * @tc.desc: Test create external with size.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, CreateExternalWithSizeTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    const char testStr[] = "test";
    size_t size = sizeof(testStr) / sizeof(char);
    napi_value external = nullptr;
    napi_create_external_with_size(
        env, (void*)testStr,
        [](napi_env env, void* data, void* hint) { ASSERT_STREQ((const char*)data, (const char*)hint); },
        (void*)testStr, &external, size);

    ASSERT_CHECK_VALUE_TYPE(env, external, napi_external);
    void* tempExternal = nullptr;
    napi_get_value_external(env, external, &tempExternal);
    ASSERT_TRUE(tempExternal);
    ASSERT_EQ(tempExternal, testStr);
}

/**
 * @tc.name: CreateExternalWithSizeTest002
 * @tc.desc: Test create external with size.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, CreateExternalWithSizeTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    const char testStr[] = "中测";
    size_t size = sizeof(testStr) / sizeof(char);
    napi_value external = nullptr;
    napi_create_external_with_size(
        env, (void*)testStr,
        [](napi_env env, void* data, void* hint) { ASSERT_STREQ((const char*)data, (const char*)hint); },
        (void*)testStr, &external, size);

    ASSERT_CHECK_VALUE_TYPE(env, external, napi_external);
    void* tempExternal = nullptr;
    napi_get_value_external(env, external, &tempExternal);
    ASSERT_TRUE(tempExternal);
    ASSERT_EQ(tempExternal, testStr);
}

/**
 * @tc.name: BigArrayTest001
 * @tc.desc: Test is big int64 array and big uint64 array.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, BigArrayTest001, testing::ext::TestSize.Level1) {
    napi_env env = (napi_env) engine_;

    napi_value array = nullptr;
    napi_create_array(env, &array);
    ASSERT_NE(array, nullptr);
    bool isArray = false;
    napi_is_array(env, array, &isArray);
    ASSERT_TRUE(isArray);

    bool isBigInt64Array = true;
    napi_is_big_int64_array(env, array, &isBigInt64Array);
    ASSERT_EQ(isBigInt64Array, false);

    bool isBigUInt64Array = true;
    napi_is_big_uint64_array(env, array, &isBigUInt64Array);
    ASSERT_EQ(isBigUInt64Array, false);
}

/**
 * @tc.name: CreateBufferTest001
 * @tc.desc: Test is CreateBuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, CreateBufferTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value buffer = nullptr;
    void* bufferPtr = nullptr;
    size_t bufferSize = -1;
    napi_status creatresult = napi_create_buffer(env, bufferSize, &bufferPtr, &buffer);

    ASSERT_EQ(creatresult, napi_status::napi_invalid_arg);
    ASSERT_EQ(bufferPtr, nullptr);
}

/**
 * @tc.name: CreateBufferTest002
 * @tc.desc: Test is CreateBuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, CreateBufferTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value buffer = nullptr;
    void* bufferPtr = nullptr;
    const char* data = nullptr;
    size_t bufferSize = -1;
    napi_status creatresult = napi_create_buffer_copy(env, bufferSize, data, &bufferPtr, &buffer);

    ASSERT_EQ(creatresult, napi_status::napi_invalid_arg);
    ASSERT_EQ(bufferPtr, nullptr);
}

/**
 * @tc.name: CreateBufferTest003
 * @tc.desc: Test is CreateBuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, CreateBufferTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value buffer = nullptr;
    void* bufferPtr = nullptr;
    size_t bufferSize = 1;
    napi_status creatresult = napi_create_buffer(env, bufferSize, &bufferPtr, &buffer);
    ASSERT_EQ(creatresult, napi_status::napi_ok);
    creatresult = napi_create_buffer(env, bufferSize, nullptr, &buffer);
    ASSERT_EQ(creatresult, napi_status::napi_invalid_arg);
}

/**
 * @tc.name: CreateBufferTest004
 * @tc.desc: Test is CreateBufferCopy.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, CreateBufferTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value buffer = nullptr;
    void* bufferPtr = nullptr;
    const char* data = nullptr;
    size_t bufferSize = 1;
    napi_status creatresult = napi_create_buffer_copy(env, bufferSize, nullptr, &bufferPtr, &buffer);
    ASSERT_EQ(creatresult, napi_status::napi_invalid_arg);
    creatresult = napi_create_buffer_copy(env, bufferSize, data, &bufferPtr, nullptr);
    ASSERT_EQ(creatresult, napi_status::napi_invalid_arg);
}

/**
 * @tc.name: IsDetachedArrayBufferTest001
 * @tc.desc: Test is DetachedArrayBuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, IsDetachedArrayBufferTest001, testing::ext::TestSize.Level1)
{
    static constexpr size_t arrayBufferSize = 1024;
    napi_env env = (napi_env)engine_;
    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    bool result = false;
    ASSERT_CHECK_CALL(napi_is_detached_arraybuffer(env, arrayBuffer, &result));

    auto out = napi_detach_arraybuffer(env, arrayBuffer);
    if (out == napi_ok) {
        arrayBufferPtr = nullptr;
    }
    ASSERT_EQ(out, napi_ok);

    result = false;
    ASSERT_CHECK_CALL(napi_is_detached_arraybuffer(env, arrayBuffer, &result));
    ASSERT_TRUE(result);
}

/**
 * @tc.name: IsDetachedArrayBufferTest002
 * @tc.desc: Test is DetachedArrayBuffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, IsDetachedArrayBufferTest002, testing::ext::TestSize.Level1)
{
    static constexpr size_t arrayBufferSize = 102400; // 102400: test big numbers
    napi_env env = (napi_env)engine_;
    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    bool result = false;
    ASSERT_CHECK_CALL(napi_is_detached_arraybuffer(env, arrayBuffer, &result));

    auto out = napi_detach_arraybuffer(env, arrayBuffer);
    if (out == napi_ok) {
        arrayBufferPtr = nullptr;
    }
    ASSERT_EQ(out, napi_ok);

    result = false;
    ASSERT_CHECK_CALL(napi_is_detached_arraybuffer(env, arrayBuffer, &result));
    ASSERT_TRUE(result);
}

/**
 * @tc.name: FreezeObjectTest001
 * @tc.desc: Test is FreezeObject.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, FreezeObjectTest001, testing::ext::TestSize.Level1)
{
    constexpr int dataSize = 60;
    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_create_object(env, &object);

    const char testStr[] = "1234567";
    napi_value strAttribute = nullptr;
    napi_create_string_utf8(env, testStr, strlen(testStr), &strAttribute);
    napi_set_named_property(env, object, "strAttribute", strAttribute);

    int32_t testNumber = 1;
    napi_value numberAttribute = nullptr;
    napi_create_int32(env, testNumber, &numberAttribute);
    napi_set_named_property(env, object, "numberAttribute", numberAttribute);

    ASSERT_CHECK_CALL(napi_object_freeze(env, object));

    int32_t testNumber2 = 0;
    napi_value numberAttribute2 = nullptr;
    napi_create_int32(env, testNumber2, &numberAttribute2);
    // Set property after freezed will throw 'Cannot add property in prevent extensions'.
    napi_status status = napi_set_named_property(env, object, "test", numberAttribute2);
    ASSERT_EQ(status, napi_pending_exception);

    napi_value ex;
    napi_get_and_clear_last_exception(env, &ex);

    napi_key_collection_mode keyMode = napi_key_own_only;
    napi_key_filter keyFilter = napi_key_all_properties;
    napi_key_conversion keyConversion = napi_key_keep_numbers;
    napi_value propNames = nullptr;
    ASSERT_CHECK_CALL(napi_get_all_property_names(env, object, keyMode, keyFilter, keyConversion, &propNames));

    uint32_t arrayLength = 0;
    ASSERT_CHECK_CALL(napi_get_array_length(env, propNames, &arrayLength));
    ASSERT_EQ(arrayLength, MAX_BUFFER_SIZE);

    char names[2][30];
    memset_s(names, dataSize, 0, dataSize);
    auto ret = memcpy_s(names[0], strlen("strAttribute"), "strAttribute", strlen("strAttribute"));
    ASSERT_EQ(ret, EOK);
    ret = memcpy_s(names[1], strlen("numberAttribute"), "numberAttribute", strlen("numberAttribute"));
    ASSERT_EQ(ret, EOK);
    for (uint32_t i = 0; i < arrayLength; i++) {
        bool hasElement = false;
        ASSERT_CHECK_CALL(napi_has_element(env, propNames, i, &hasElement));

        napi_value propName = nullptr;
        ASSERT_CHECK_CALL(napi_get_element(env, propNames, i, &propName));
        ASSERT_CHECK_VALUE_TYPE(env, propName, napi_string);

        size_t testStrLength = TEST_STR_LENGTH;
        char testStrInner[TEST_STR_LENGTH + 1];
        size_t outStrLength = 0;
        memset_s(testStrInner, testStrLength + 1, 0, testStrLength + 1);
        ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, propName, testStrInner, testStrLength, &outStrLength));

        int ret = strcmp(testStrInner, names[i]);
        ASSERT_EQ(ret, 0);
    }
}

/**
 * @tc.name: FreezeObjectTest002
 * @tc.desc: Test is FreezeObject.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, FreezeObjectTest002, testing::ext::TestSize.Level1)
{
    constexpr int dataSize = INT_HUNDRED;
    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_create_object(env, &object);

    const char testStr[] = "中测";
    napi_value strAttribute = nullptr;
    napi_create_string_utf8(env, testStr, strlen(testStr), &strAttribute);
    napi_set_named_property(env, object, "strAttribute", strAttribute);

    int32_t testNumber = TEST_INT32_10;
    napi_value numberAttribute = nullptr;
    napi_create_int32(env, testNumber, &numberAttribute);
    napi_set_named_property(env, object, "numberAttribute", numberAttribute);

    ASSERT_CHECK_CALL(napi_object_freeze(env, object));

    int32_t testNumber2 = TEST_INT32_20;
    napi_value numberAttribute2 = nullptr;
    napi_create_int32(env, testNumber2, &numberAttribute2);
    // Set property after freezed will throw 'Cannot add property in prevent extensions'.
    napi_status status = napi_set_named_property(env, object, "test", numberAttribute2);
    ASSERT_EQ(status, napi_pending_exception);

    napi_value ex;
    napi_get_and_clear_last_exception(env, &ex);

    napi_key_collection_mode keyMode = napi_key_own_only;
    napi_key_filter keyFilter = napi_key_all_properties;
    napi_key_conversion keyConversion = napi_key_keep_numbers;
    napi_value propNames = nullptr;
    ASSERT_CHECK_CALL(napi_get_all_property_names(env, object, keyMode, keyFilter, keyConversion, &propNames));

    uint32_t arrayLength = TEST_INT32_10;
    ASSERT_CHECK_CALL(napi_get_array_length(env, propNames, &arrayLength));
    ASSERT_EQ(arrayLength, MAX_BUFFER_SIZE);

    char names[2][50];
    memset_s(names, dataSize, 0, dataSize);
    auto ret = memcpy_s(names[0], strlen("strAttribute"), "strAttribute", strlen("strAttribute"));
    ASSERT_EQ(ret, EOK);
    ret = memcpy_s(names[1], strlen("numberAttribute"), "numberAttribute", strlen("numberAttribute"));
    ASSERT_EQ(ret, EOK);
    for (uint32_t i = 0; i < arrayLength; i++) {
        bool hasElement = false;
        ASSERT_CHECK_CALL(napi_has_element(env, propNames, i, &hasElement));

        napi_value propName = nullptr;
        ASSERT_CHECK_CALL(napi_get_element(env, propNames, i, &propName));
        ASSERT_CHECK_VALUE_TYPE(env, propName, napi_string);

        size_t testStrLength = TEST_STR_LENGTH;
        char testStrInner[TEST_STR_LENGTH + 1];
        size_t outStrLength = 0; // 0: define initial length
        memset_s(testStrInner, testStrLength + 1, 0, testStrLength + 1);
        ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, propName, testStrInner, testStrLength, &outStrLength));

        int ret = strcmp(testStrInner, names[i]);
        ASSERT_EQ(ret, 0);
    }
}

/**
 * @tc.name: SealObjectTest001
 * @tc.desc: Test is SealObject.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, SealObjectTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;

    napi_create_object(env, &object);

    const char testStr[] = "1234567";
    napi_value strAttribute = nullptr;
    napi_create_string_utf8(env, testStr, strlen(testStr), &strAttribute);
    napi_set_named_property(env, object, "strAttribute", strAttribute);

    int32_t testNumber = 1;
    napi_value numberAttribute = nullptr;
    napi_create_int32(env, testNumber, &numberAttribute);
    napi_set_named_property(env, object, "numberAttribute", numberAttribute);

    ASSERT_CHECK_CALL(napi_object_seal(env, object));

    bool testDeleted = false;
    ASSERT_CHECK_CALL(napi_delete_property(env, object, strAttribute, &testDeleted));
    ASSERT_TRUE(testDeleted);

    const char modifiedStr[] = "modified";
    napi_value modifiedValue = nullptr;
    napi_create_string_utf8(env, modifiedStr, strlen(modifiedStr), &modifiedValue);
    ASSERT_CHECK_CALL(napi_set_named_property(env, object, "strAttribute", modifiedValue));

    napi_value strAttribute2 = nullptr;
    napi_get_named_property(env, object, "strAttribute", &strAttribute2);
    char buffer[TEST_STR_LENGTH] = {0};
    size_t length = 0;
    napi_status status = napi_get_value_string_utf8(env, strAttribute2, buffer, sizeof(buffer) - 1, &length);
    ASSERT_EQ(status, napi_ok);
    ASSERT_EQ(length, strlen(modifiedStr));
    ASSERT_EQ(strcmp(buffer, modifiedStr), 0);

    napi_key_collection_mode keyMode = napi_key_own_only;
    napi_key_filter keyFilter = napi_key_all_properties;
    napi_key_conversion keyConversion = napi_key_keep_numbers;
    napi_value propNames = nullptr;
    ASSERT_CHECK_CALL(napi_get_all_property_names(env, object, keyMode, keyFilter, keyConversion, &propNames));

    uint32_t arrayLength = 0;
    ASSERT_CHECK_CALL(napi_get_array_length(env, propNames, &arrayLength));
    ASSERT_EQ(arrayLength, MAX_BUFFER_SIZE);

    char names[2][TEST_STR_LENGTH];
    // There are 2 elements in the string array,
    // so the parameter is set to TEST_STR_LENGTH * 2 to clear the entire array.
    memset_s(names, TEST_STR_LENGTH * 2, 0, TEST_STR_LENGTH * 2);
    auto ret = memcpy_s(names[0], strlen("strAttribute"), "strAttribute", strlen("strAttribute"));
    ASSERT_EQ(ret, EOK);
    ret = memcpy_s(names[1], strlen("numberAttribute"), "numberAttribute", strlen("numberAttribute"));
    ASSERT_EQ(ret, EOK);

    for (uint32_t i = 0; i < arrayLength; i++) {
        bool hasElement = false;
        ASSERT_CHECK_CALL(napi_has_element(env, propNames, i, &hasElement));

        napi_value propName = nullptr;
        ASSERT_CHECK_CALL(napi_get_element(env, propNames, i, &propName));
        ASSERT_CHECK_VALUE_TYPE(env, propName, napi_string);

        size_t testStrLength = TEST_STR_LENGTH;
        char testStrInner[TEST_STR_LENGTH + 1];
        size_t outStrLength = 0;
        memset_s(testStrInner, testStrLength + 1, 0, testStrLength + 1);
        ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, propName, testStrInner, testStrLength, &outStrLength));

        int ret = strcmp(testStrInner, names[i]);
        ASSERT_EQ(ret, 0);
    }
}

/**
 * @tc.name: SealObjectTest002
 * @tc.desc: Test is SealObject.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, SealObjectTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_create_object(env, &object);
    const char testStr[] = "中测";
    napi_value strAttribute = nullptr;
    napi_create_string_utf8(env, testStr, strlen(testStr), &strAttribute);
    napi_set_named_property(env, object, "strAttribute", strAttribute);
    int32_t testNumber = TEST_INT32_1;
    napi_value numberAttribute = nullptr;
    napi_create_int32(env, testNumber, &numberAttribute);
    napi_set_named_property(env, object, "numberAttribute", numberAttribute);
    ASSERT_CHECK_CALL(napi_object_seal(env, object));
    bool testDeleted = false;
    ASSERT_CHECK_CALL(napi_delete_property(env, object, strAttribute, &testDeleted));
    ASSERT_TRUE(testDeleted);
    const char modifiedStr[] = "中测修改";
    napi_value modifiedValue = nullptr;
    napi_create_string_utf8(env, modifiedStr, strlen(modifiedStr), &modifiedValue);
    ASSERT_CHECK_CALL(napi_set_named_property(env, object, "strAttribute", modifiedValue));
    napi_value strAttribute2 = nullptr;
    napi_get_named_property(env, object, "strAttribute", &strAttribute2);
    char buffer[TEST_STR_LENGTH] = {1};
    size_t length = 1; // 0: define initial length
    napi_status status = napi_get_value_string_utf8(env, strAttribute2, buffer, sizeof(buffer) - 1, &length);
    ASSERT_EQ(status, napi_ok);
    ASSERT_EQ(length, strlen(modifiedStr));
    ASSERT_EQ(strcmp(buffer, modifiedStr), 0);

    napi_key_collection_mode keyMode = napi_key_own_only;
    napi_key_filter keyFilter = napi_key_all_properties;
    napi_key_conversion keyConversion = napi_key_keep_numbers;
    napi_value propNames = nullptr;
    ASSERT_CHECK_CALL(napi_get_all_property_names(env, object, keyMode, keyFilter, keyConversion, &propNames));

    uint32_t arrayLength = 1; // 0: define initial length
    ASSERT_CHECK_CALL(napi_get_array_length(env, propNames, &arrayLength));
    ASSERT_EQ(arrayLength, MAX_BUFFER_SIZE);

    char names[2][TEST_STR_LENGTH];
    // There are 2 elements in the string array,
    // so the parameter is set to TEST_STR_LENGTH * 2 to clear the entire array.
    memset_s(names, TEST_STR_LENGTH * 2, 0, TEST_STR_LENGTH * 2);
    auto ret = memcpy_s(names[0], strlen("strAttribute"), "strAttribute", strlen("strAttribute"));
    ASSERT_EQ(ret, EOK);
    ret = memcpy_s(names[1], strlen("numberAttribute"), "numberAttribute", strlen("numberAttribute"));
    ASSERT_EQ(ret, EOK);

    for (uint32_t i = 0; i < arrayLength; i++) {
        bool hasElement = false;
        ASSERT_CHECK_CALL(napi_has_element(env, propNames, i, &hasElement));

        napi_value propName = nullptr;
        ASSERT_CHECK_CALL(napi_get_element(env, propNames, i, &propName));
        ASSERT_CHECK_VALUE_TYPE(env, propName, napi_string);

        size_t testStrLength = TEST_STR_LENGTH;
        char testStrInner[TEST_STR_LENGTH + 1];
        size_t outStrLength = 0; // 0: define initial size
        memset_s(testStrInner, testStrLength + 1, 0, testStrLength + 1);
        ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, propName, testStrInner, testStrLength, &outStrLength));

        int ret = strcmp(testStrInner, names[i]);
        ASSERT_EQ(ret, 0);
    }
}

/**
 * @tc.name: AllPropertyNamesTest001
 * @tc.desc: Test is AllPropertyNames.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AllPropertyNamesTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_key_collection_mode keyMode = napi_key_own_only;
    napi_key_filter keyFilter = napi_key_all_properties;
    napi_key_conversion keyConversion = napi_key_keep_numbers;
    napi_value result = nullptr;
    napi_value propNames = nullptr;

    ASSERT_CHECK_CALL(napi_create_object(env, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_object);

    const char testStr[] = "1234567";
    napi_value strAttribute = nullptr;
    napi_create_string_utf8(env, testStr, strlen(testStr), &strAttribute);
    napi_set_named_property(env, result, "strAttribute", strAttribute);

    int32_t testNumber = 1;
    napi_value numberAttribute = nullptr;
    napi_create_int32(env, testNumber, &numberAttribute);
    napi_set_named_property(env, result, "numberAttribute", numberAttribute);

    ASSERT_CHECK_CALL(napi_get_all_property_names(env, result, keyMode, keyFilter, keyConversion, &propNames));

    ASSERT_CHECK_VALUE_TYPE(env, propNames, napi_object);
    bool isArray = false;
    ASSERT_CHECK_CALL(napi_is_array(env, propNames, &isArray));
    ASSERT_TRUE(isArray);
    uint32_t arrayLength = 0;
    ASSERT_CHECK_CALL(napi_get_array_length(env, propNames, &arrayLength));
    ASSERT_EQ(arrayLength, MAX_BUFFER_SIZE);

    char names[2][TEST_STR_LENGTH];
    // There are 2 elements in the string array,
    // so the parameter is set to TEST_STR_LENGTH * 2 to clear the entire array.
    memset_s(names, TEST_STR_LENGTH * 2, 0, TEST_STR_LENGTH * 2);
    auto ret = memcpy_s(names[0], strlen("strAttribute"), "strAttribute", strlen("strAttribute"));
    ASSERT_EQ(ret, EOK);
    ret = memcpy_s(names[1], strlen("numberAttribute"), "numberAttribute", strlen("numberAttribute"));
    ASSERT_EQ(ret, EOK);

    for (uint32_t i = 0; i < arrayLength; i++) {
        bool hasElement = false;
        ASSERT_CHECK_CALL(napi_has_element(env, propNames, i, &hasElement));

        napi_value propName = nullptr;
        ASSERT_CHECK_CALL(napi_get_element(env, propNames, i, &propName));
        ASSERT_CHECK_VALUE_TYPE(env, propName, napi_string);

        size_t testStrLength = TEST_STR_LENGTH;
        char testStrInner[TEST_STR_LENGTH + 1];
        size_t outStrLength = 0;
        memset_s(testStrInner, testStrLength + 1, 0, testStrLength + 1);
        ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, propName, testStrInner, testStrLength, &outStrLength));

        int ret = strcmp(testStrInner, names[i]);
        ASSERT_EQ(ret, 0);
    }
}

/**
 * @tc.name: AllPropertyNamesTest002
 * @tc.desc: Test is AllPropertyNames.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AllPropertyNamesTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_key_collection_mode keyMode = napi_key_own_only;
    napi_key_filter keyFilter = napi_key_writable;
    napi_key_conversion keyConversion = napi_key_keep_numbers;
    napi_value result = nullptr;
    napi_value propNames = nullptr;
    // Create napi_values for 123, 456 and 789
    napi_value unenumerAble, writAble, configurAble;
    napi_create_int32(env, 123, &unenumerAble);
    napi_create_int32(env, 456, &writAble);
    napi_create_int32(env, 789, &configurAble);

    napi_property_descriptor descriptors[] = {
        {"unenumerable",
         nullptr, nullptr, nullptr, nullptr, unenumerAble,
         napi_default_method, nullptr},
        {"writable",
         nullptr, nullptr, nullptr, nullptr, writAble,
         static_cast<napi_property_attributes>(napi_enumerable | napi_writable), nullptr},
        {"configurable",
         nullptr, nullptr, nullptr, nullptr, configurAble,
         static_cast<napi_property_attributes>(napi_enumerable | napi_configurable), nullptr}
    };

    ASSERT_CHECK_CALL(napi_create_object(env, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_object);
    ASSERT_CHECK_CALL(napi_define_properties(env, result, sizeof(descriptors) / sizeof(descriptors[0]), descriptors));

    const char testStr[] = "1234567";
    napi_value strAttribute = nullptr;
    napi_create_string_utf8(env, testStr, strlen(testStr), &strAttribute);
    napi_set_named_property(env, result, "strAttribute", strAttribute);

    int32_t testNumber = 1;
    napi_value numberAttribute = nullptr;
    napi_create_int32(env, testNumber, &numberAttribute);
    napi_set_named_property(env, result, "numberAttribute", numberAttribute);

    ASSERT_CHECK_CALL(napi_get_all_property_names(env, result, keyMode, keyFilter, keyConversion, &propNames));

    ASSERT_CHECK_VALUE_TYPE(env, propNames, napi_object);
    bool isArray = false;
    ASSERT_CHECK_CALL(napi_is_array(env, propNames, &isArray));
    ASSERT_TRUE(isArray);
    uint32_t arrayLength = 0;
    ASSERT_CHECK_CALL(napi_get_array_length(env, propNames, &arrayLength));
    ASSERT_EQ(arrayLength, 4); // 4 means array length.

    char names[4][TEST_STR_LENGTH];
    // There are 4 elements in the string array,
    // so the parameter is set to TEST_STR_LENGTH * 4 to clear the entire array.
    memset_s(names, TEST_STR_LENGTH * 4, 0, TEST_STR_LENGTH * 4);
    auto ret = memcpy_s(names[0], strlen("unenumerable"), "unenumerable", strlen("unenumerable"));
    ASSERT_EQ(ret, EOK);
    ret = memcpy_s(names[1], strlen("writable"), "writable", strlen("writable"));
    ASSERT_EQ(ret, EOK);
    ret = memcpy_s(names[2], strlen("strAttribute"), "strAttribute", strlen("strAttribute"));
    ASSERT_EQ(ret, EOK);
    ret = memcpy_s(names[3], strlen("numberAttribute"), "numberAttribute", strlen("numberAttribute"));
    ASSERT_EQ(ret, EOK);

    for (uint32_t i = 0; i < arrayLength; i++) {
        bool hasElement = false;
        ASSERT_CHECK_CALL(napi_has_element(env, propNames, i, &hasElement));

        napi_value propName = nullptr;
        ASSERT_CHECK_CALL(napi_get_element(env, propNames, i, &propName));
        ASSERT_CHECK_VALUE_TYPE(env, propName, napi_string);

        size_t testStrLength = TEST_STR_LENGTH;
        char testStrInner[TEST_STR_LENGTH + 1];
        size_t outStrLength = 0;
        memset_s(testStrInner, testStrLength + 1, 0, testStrLength + 1);
        ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, propName, testStrInner, testStrLength, &outStrLength));

        int ret = strcmp(testStrInner, names[i]);
        ASSERT_EQ(ret, 0);
    }
}

/**
 * @tc.name: StringUtf16Test001
 * @tc.desc: Test is Chinese space character special character truncation.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringUtf16Test001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const char16_t testStr[] = u"中文,English,123456,！@#$%$#^%&12345     ";
    int testStrLength = static_cast<int>(std::char_traits<char16_t>::length(testStr));
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf16(env, testStr, testStrLength, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_string);

    char16_t* buffer = nullptr;
    size_t bufferSize = 0;
    size_t strLength = 0;
    ASSERT_CHECK_CALL(napi_get_value_string_utf16(env, result, nullptr, 0, &bufferSize));
    ASSERT_GT(bufferSize, 0);
    buffer = new char16_t[bufferSize + 1] { 0 };
    ASSERT_CHECK_CALL(napi_get_value_string_utf16(env, result, buffer, bufferSize + 1, &strLength));
    for (int i = 0; i < testStrLength; i++) {
        ASSERT_EQ(testStr[i], buffer[i]);
    }
    ASSERT_EQ(testStrLength, strLength);
    delete[] buffer;
    buffer = nullptr;

    char16_t* bufferShort = nullptr;
    int bufferShortSize = 3;
    bufferShort = new char16_t[bufferShortSize] { 0 };
    ASSERT_CHECK_CALL(napi_get_value_string_utf16(env, result, bufferShort, bufferShortSize, &strLength));
    for (int i = 0; i < bufferShortSize; i++) {
        if (i == (bufferShortSize - 1)) {
            ASSERT_EQ(0, bufferShort[i]);
        } else {
            ASSERT_EQ(testStr[i], bufferShort[i]);
        }
    }
    ASSERT_EQ(strLength, MAX_BUFFER_SIZE);
    delete[] bufferShort;
    bufferShort = nullptr;
}

/**
 * @tc.name: StringUtf16Test002
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringUtf16Test002, testing::ext::TestSize.Level2)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    char16_t testStr[] = u"ut.utf16test.napi.!@#%中^&*()6666";
    int testStrLength = static_cast<int>(std::char_traits<char16_t>::length(testStr));
    napi_value result = nullptr;
    {
        napi_status ret = napi_create_string_utf16(env, nullptr, testStrLength, &result);
        ASSERT_EQ(ret, napi_status::napi_invalid_arg);
    }
    {
        napi_status ret = napi_create_string_utf16(env, testStr, (size_t)INT_MAX + 1, &result);
        ASSERT_EQ(ret, napi_status::napi_invalid_arg);
    }
}

/**
 * @tc.name: StringUtf16Test003
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringUtf16Test003, testing::ext::TestSize.Level2)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    char16_t testStr[] = u"ut.utf16test.napi.!@#$%^&*123";
    size_t testStrLength = static_cast<size_t>(std::char_traits<char16_t>::length(testStr));
    char16_t buffer[] = u"12345";
    size_t bufferSize = 0;
    size_t copied = 0;
    napi_value result = nullptr;

    ASSERT_CHECK_CALL(napi_create_string_utf16(env, testStr, testStrLength, &result));
    ASSERT_CHECK_CALL(napi_get_value_string_utf16(env, result, buffer, bufferSize, &copied));

    for (size_t i = 0; i < MAX_BUFFER_SIZE; i++) {
        ASSERT_NE(buffer[i], testStr[i]);
    }
}

/**
 * @tc.name: StringUtf16Test004
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringUtf16Test004, testing::ext::TestSize.Level2)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    char16_t buffer[BUFFER_SIZE_FIVE];
    int testStrLength = static_cast<int>(std::char_traits<char16_t>::length(buffer));
    size_t copied;
    int64_t testValue = INT64_MAX;
    napi_value result = nullptr;

    ASSERT_CHECK_CALL(napi_create_bigint_int64(env, testValue, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_bigint);

    napi_status ret = napi_get_value_string_utf16(env, result, buffer, testStrLength, &copied);
    ASSERT_EQ(ret, napi_status::napi_string_expected);
}

/**
 * @tc.name: StringUtf16Test005
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringUtf16Test005, testing::ext::TestSize.Level2)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    char16_t testStr[] = u"ut.utf16test.napi.!@#$%^&*123";
    int testStrLength = static_cast<int>(std::char_traits<char16_t>::length(testStr));
    char16_t buffer[testStrLength];
    size_t copied;
    napi_value result = nullptr;

    napi_status ret = napi_get_value_string_utf16(env, result, buffer, testStrLength, &copied);
    ASSERT_EQ(ret, napi_status::napi_invalid_arg);
}

/**
 * @tc.name: StringUtf16Test006
 * @tc.desc: Test string length.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringUtf16Test006, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    char16_t testStr[] = u"ut.utf16test.napi.!@#$%^&*123";
    size_t testStrLength = static_cast<size_t>(std::char_traits<char16_t>::length(testStr));
    size_t copied = 0;
    napi_value result = nullptr;

    ASSERT_CHECK_CALL(napi_create_string_utf16(env, testStr, testStrLength, &result));
    ASSERT_CHECK_CALL(napi_get_value_string_utf16(env, result, nullptr, testStrLength, &copied));

    ASSERT_EQ(testStrLength, copied);
}

/**
 * @tc.name: StringUtf16Test007
 * @tc.desc: Test string length.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringUtf16Test007, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    char16_t testStr[] = u"中测ut.utf16test.napi.!@#$%^&*123456";
    size_t testStrLength = static_cast<size_t>(std::char_traits<char16_t>::length(testStr));
    size_t copied = 0;
    napi_value result = nullptr;

    ASSERT_CHECK_CALL(napi_create_string_utf16(env, testStr, testStrLength, &result));
    ASSERT_CHECK_CALL(napi_get_value_string_utf16(env, result, nullptr, testStrLength, &copied));

    ASSERT_EQ(testStrLength, copied);
}

/**
 * @tc.name: StringUtf8Test001
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringUtf8Test001, testing::ext::TestSize.Level2)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const char testStr[] = "ut.utf8test.napi.!@#%中^&*()6666";
    size_t testStrLength = strlen(testStr);

    napi_status ret = napi_create_string_utf8(env, testStr, testStrLength, nullptr);
    ASSERT_EQ(ret, napi_status::napi_invalid_arg);
}

/**
 * @tc.name: StringUtf8Test002
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringUtf8Test002, testing::ext::TestSize.Level2)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    char buffer[BUFFER_SIZE_FIVE] = { 0 };
    size_t testStrLength = strlen(buffer);
    size_t copied;
    napi_value result = nullptr;
    napi_get_boolean(env, true, &result);
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_boolean);

    napi_status ret = napi_get_value_string_utf8(env, result, buffer, testStrLength, &copied);
    ASSERT_EQ(ret, napi_status::napi_string_expected);
}

/**
 * @tc.name: StringUtf8Test003
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringUtf8Test003, testing::ext::TestSize.Level2)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const char testStr[] = "ut.utf8test.napi.!@#$%^&*123";
    size_t testStrLength = strlen(testStr);
    char buffer[testStrLength];
    size_t copied;
    napi_value result = nullptr;

    napi_status ret = napi_get_value_string_utf8(env, result, buffer, testStrLength, &copied);
    ASSERT_EQ(ret, napi_status::napi_invalid_arg);
}

/**
 * @tc.name: StringUtf8Test004
 * @tc.desc: Test string length.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringUtf8Test004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const char testStr[] = "ut.utf8test.napi.!@#$%^&*123";
    size_t testStrLength = strlen(testStr);
    size_t copied = 0;
    napi_value result = nullptr;

    ASSERT_CHECK_CALL(napi_create_string_utf8(env, testStr, testStrLength, &result));
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, result, nullptr, testStrLength, &copied));

    ASSERT_EQ(testStrLength, copied);
}

/**
 * @tc.name: StringLatin1Test001
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringLatin1Test001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const char testStr[] = "ut.latin1test.napi.!@#%^&*()6666";
    size_t testStrLength = strlen(testStr);
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_latin1(env, testStr, testStrLength, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_string);

    char* buffer = nullptr;
    size_t bufferSize = 0;
    size_t strLength = 0;
    ASSERT_CHECK_CALL(napi_get_value_string_latin1(env, result, nullptr, 0, &bufferSize));
    ASSERT_GT(bufferSize, 0);
    buffer = new char[bufferSize + 1]{ 0 };
    ASSERT_CHECK_CALL(napi_get_value_string_latin1(env, result, buffer, bufferSize + 1, &strLength));
    ASSERT_STREQ(testStr, buffer);
    ASSERT_EQ(testStrLength, strLength);
    delete []buffer;
    buffer = nullptr;
}

/**
 * @tc.name: StringLatin1Test002
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringLatin1Test002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const char testStr[] = "ut.latin1test.中文测试";
    size_t testStrLength = strlen(testStr);
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_latin1(env, testStr, testStrLength, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_string);

    char* buffer = nullptr;
    size_t bufferSize = 0;
    size_t strLength = 0;
    ASSERT_CHECK_CALL(napi_get_value_string_latin1(env, result, nullptr, 0, &bufferSize));
    ASSERT_GT(bufferSize, 0);
    buffer = new char[bufferSize + 1]{ 0 };
    ASSERT_CHECK_CALL(napi_get_value_string_latin1(env, result, buffer, bufferSize + 1, &strLength));
    ASSERT_STRNE(testStr, buffer);
    ASSERT_GT(testStrLength, strLength);
    delete []buffer;
    buffer = nullptr;
}

/**
 * @tc.name: StringLatin1Test003
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringLatin1Test003, testing::ext::TestSize.Level2)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value result = nullptr;

    const char testStr[] = "ut.latin1test.napi.!@#%^&*()6666";
    size_t testStrLength = strlen(testStr);

    napi_status ret = napi_create_string_latin1(env, nullptr, testStrLength, &result);
    ASSERT_EQ(ret, napi_status::napi_invalid_arg);
}

/**
 * @tc.name: StringLatin1Test004
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringLatin1Test004, testing::ext::TestSize.Level2)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value result = nullptr;

    const char testStr[] = "ut.latin1test.napi.!@#%^&*()6666";

    napi_status ret = napi_create_string_latin1(env, testStr, 0, &result);
    ASSERT_EQ(ret, napi_status::napi_ok);

    size_t bufferSize = 0;
    ASSERT_CHECK_CALL(napi_get_value_string_latin1(env, result, nullptr, 0, &bufferSize));
    ASSERT_EQ(bufferSize, 0);
}

/**
 * @tc.name: StringLatin1Test005
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringLatin1Test005, testing::ext::TestSize.Level2)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    char buffer[BUFFER_SIZE_FIVE] = { 0 };
    size_t testStrLength = strlen(buffer);
    size_t copied;
    napi_value result = nullptr;
    napi_get_boolean(env, true, &result);
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_boolean);

    napi_status ret = napi_get_value_string_latin1(env, result, buffer, testStrLength, &copied);
    ASSERT_EQ(ret, napi_status::napi_string_expected);
}

/**
 * @tc.name: StringLatin1Test006
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringLatin1Test006, testing::ext::TestSize.Level2)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const char testStr[] = "ut.latin1test.napi.!@#$%^&*123";
    size_t testStrLength = strlen(testStr);
    char buffer[testStrLength];
    size_t copied;
    napi_value result = nullptr;

    napi_status ret = napi_get_value_string_latin1(env, result, buffer, testStrLength, &copied);
    ASSERT_EQ(ret, napi_status::napi_invalid_arg);
}

/**
 * @tc.name: StringLatin1Test007
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, StringLatin1Test007, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const char testStr[] = "ut.latin1test.napi.!@#$%^&*123";
    size_t testStrLength = strlen(testStr);
    size_t copied = 0;
    napi_value result = nullptr;

    ASSERT_CHECK_CALL(napi_create_string_latin1(env, testStr, testStrLength, &result));
    ASSERT_CHECK_CALL(napi_get_value_string_latin1(env, result, nullptr, testStrLength, &copied));

    ASSERT_EQ(testStrLength, copied);
}

/**
 * @tc.name: ToStringTest001
 * @tc.desc: Test string type of str.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ToStringTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const char testStr[] = "中文,English,123456,!@#$%$#^%&";
    size_t testStrLength = strlen(testStr);
    napi_value str = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, testStr, testStrLength, &str));
    ASSERT_CHECK_VALUE_TYPE(env, str, napi_string);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_coerce_to_string(env, str, &result));
    char* buffer = nullptr;
    size_t bufferSize = 0;
    size_t strLength = 0;
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, result, nullptr, 0, &bufferSize));
    ASSERT_GT(bufferSize, 0);
    buffer = new char[bufferSize + 1]{ 0 };
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, result, buffer, bufferSize + 1, &strLength));
    ASSERT_STREQ(testStr, buffer);
    ASSERT_EQ(testStrLength, strLength);
    delete []buffer;
    buffer = nullptr;
}

/**
 * @tc.name: ToStringTest002
 * @tc.desc: Test string type of undefined.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ToStringTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value argument;
    napi_get_undefined(env, &argument);
    ASSERT_CHECK_VALUE_TYPE(env, argument, napi_undefined);

    napi_value result;
    ASSERT_CHECK_CALL(napi_coerce_to_string(env, argument, &result));

    const char expected[] = "undefined";
    size_t expectedLength = strlen(expected);
    char* buffer = nullptr;
    size_t bufferSize = 0;
    size_t strLength = 0;
    napi_get_value_string_utf8(env, result, nullptr, 0, &bufferSize);
    ASSERT_GT(bufferSize, 0);
    buffer = new char[bufferSize + 1]{ 0 };
    napi_get_value_string_utf8(env, result, buffer, bufferSize + 1, &strLength);
    ASSERT_EQ(expectedLength, strLength);
    ASSERT_STREQ(expected, buffer);
    delete []buffer;
    buffer = nullptr;
}

/**
 * @tc.name: ToStringTest003
 * @tc.desc: Test string type of null.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ToStringTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value argument;
    napi_get_null(env, &argument);
    ASSERT_CHECK_VALUE_TYPE(env, argument, napi_null);

    napi_value result;
    ASSERT_CHECK_CALL(napi_coerce_to_string(env, argument, &result));

    const char expected[] = "null";
    size_t expectedLength = strlen(expected);
    char* buffer = nullptr;
    size_t bufferSize = 0;
    size_t strLength = 0;
    napi_get_value_string_utf8(env, result, nullptr, 0, &bufferSize);
    ASSERT_GT(bufferSize, 0);
    buffer = new char[bufferSize + 1]{ 0 };
    napi_get_value_string_utf8(env, result, buffer, bufferSize + 1, &strLength);
    ASSERT_EQ(expectedLength, strLength);
    ASSERT_STREQ(expected, buffer);
    delete []buffer;
    buffer = nullptr;
}

/**
 * @tc.name: ToStringTest004
 * @tc.desc: Test string type of bool.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ToStringTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value argument;
    napi_get_boolean(env, true, &argument);
    ASSERT_CHECK_VALUE_TYPE(env, argument, napi_boolean);

    napi_value result;
    ASSERT_CHECK_CALL(napi_coerce_to_string(env, argument, &result));

    const char expected[] = "true";
    size_t expectedLength = strlen(expected);
    char* buffer = nullptr;
    size_t bufferSize = 0;
    size_t strLength = 0;
    napi_get_value_string_utf8(env, result, nullptr, 0, &bufferSize);
    ASSERT_GT(bufferSize, 0);
    buffer = new char[bufferSize + 1]{ 0 };
    napi_get_value_string_utf8(env, result, buffer, bufferSize + 1, &strLength);
    ASSERT_EQ(expectedLength, strLength);
    ASSERT_STREQ(expected, buffer);
    delete []buffer;
    buffer = nullptr;
}

/**
 * @tc.name: ToStringTest005
 * @tc.desc: Test string type of number.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ToStringTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value argument;
    double number = 0.1;
    napi_create_double(env, number, &argument);
    ASSERT_CHECK_VALUE_TYPE(env, argument, napi_number);

    napi_value result;
    ASSERT_CHECK_CALL(napi_coerce_to_string(env, argument, &result));

    double numberValue;
    napi_get_value_double(env, argument, &numberValue);
    std::string expected = std::to_string(numberValue);
    // Remove excess '0' after delimiter
    while (!expected.empty() && expected.back() == '0')
    {
        expected.pop_back();
    }

    size_t expectedLength = expected.length();
    char* buffer = nullptr;
    size_t bufferSize = 0;
    size_t strLength = 0;
    napi_get_value_string_utf8(env, result, nullptr, 0, &bufferSize);
    ASSERT_GT(bufferSize, 0);
    buffer = new char[bufferSize + 1]{ 0 };
    napi_get_value_string_utf8(env, result, buffer, bufferSize + 1, &strLength);
    ASSERT_EQ(expectedLength, strLength);
    ASSERT_STREQ(expected.c_str(), buffer);
    delete []buffer;
    buffer = nullptr;
}

/**
 * @tc.name: ToStringTest006
 * @tc.desc: Test string type of bigint.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ToStringTest006, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    int64_t testValue = INT64_MAX;
    napi_value argument;
    bool flag = false;
    ASSERT_CHECK_CALL(napi_create_bigint_int64(env, testValue, &argument));
    ASSERT_CHECK_VALUE_TYPE(env, argument, napi_bigint);

    napi_value result;
    ASSERT_CHECK_CALL(napi_coerce_to_string(env, argument, &result));

    int64_t numberValue = 0;
    ASSERT_CHECK_CALL(napi_get_value_bigint_int64(env, argument, &numberValue, &flag));
    ASSERT_EQ(numberValue, INT64_MAX);
    ASSERT_TRUE(flag);
    std::string expected = std::to_string(numberValue);

    size_t expectedLength = expected.length();
    char* buffer = nullptr;
    size_t bufferSize = 0;
    size_t strLength = 0;
    napi_get_value_string_utf8(env, result, nullptr, 0, &bufferSize);
    ASSERT_GT(bufferSize, 0);
    buffer = new char[bufferSize + 1]{ 0 };
    napi_get_value_string_utf8(env, result, buffer, bufferSize + 1, &strLength);
    ASSERT_EQ(expectedLength, strLength);
    ASSERT_STREQ(expected.c_str(), buffer);
    delete []buffer;
    buffer = nullptr;
}

/**
 * @tc.name: ToStringTest007
 * @tc.desc: Test string type of symbol.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ToStringTest007, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const char testStr[] = "testSymbol";
    size_t testStrLength = strlen(testStr);
    napi_value testSymbol = nullptr;
    napi_create_string_utf8(env, testStr, testStrLength, &testSymbol);
    napi_value symbolVal = nullptr;
    napi_create_symbol(env, testSymbol, &symbolVal);
    ASSERT_CHECK_VALUE_TYPE(env, symbolVal, napi_symbol);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_coerce_to_string(env, symbolVal, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_undefined);
}

/**
 * @tc.name: ToStringTest001
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ToStringTest008, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value result;
    napi_status status = napi_coerce_to_string(env, nullptr, &result);
    ASSERT_EQ(status, napi_status::napi_invalid_arg);
}

/**
 * @tc.name: ToStringTest009
 * @tc.desc: Test string type of symbol.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ToStringTest009, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const char testStr[] = "中测";
    size_t testStrLength = strlen(testStr);
    napi_value testSymbol = nullptr;
    napi_create_string_utf8(env, testStr, testStrLength, &testSymbol);
    napi_value symbolVal = nullptr;
    napi_create_symbol(env, testSymbol, &symbolVal);
    ASSERT_CHECK_VALUE_TYPE(env, symbolVal, napi_symbol);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_coerce_to_string(env, symbolVal, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_undefined);
}

/**
 * @tc.name: InstanceDataTest_001
 * @tc.desc: Test instance type.
 * @tc.type: FUNC
 */
struct AddonDataTest {
    size_t value;
    bool print;
    napi_ref jsCbRef;
};

static void DeleteAddonData(napi_env env, void* rawData, void* hint)
{
    AddonDataTest* data = reinterpret_cast<AddonDataTest*>(rawData);
    if (data->print) {
        printf("deleting addon data\n");
    }
    if (data->jsCbRef != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, data->jsCbRef));
    }
    free(data);
}

static napi_value SetPrintOnDelete(napi_env env, napi_callback_info info)
{
    AddonDataTest* data;
    NAPI_CALL(env, napi_get_instance_data(env, (void**)&data));
    data->print = true;
    return nullptr;
}

static void TestFinalizer(napi_env env, void* rawData, void* hint)
{
    (void)rawData;
    (void)hint;

    AddonDataTest* data;
    napi_value jsResult;
    NAPI_CALL_RETURN_VOID(env, napi_get_instance_data(env, (void**)&data));
    napi_value jsCb;
    napi_value value;
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, data->jsCbRef, &jsCb));
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &value));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, value, jsCb, 0, nullptr, &jsResult));

    NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, data->jsCbRef));
    data->jsCbRef = nullptr;
}

static napi_value ObjectWithFinalizer(napi_env env, napi_callback_info info)
{
    AddonDataTest* data;

    napi_value value;
    napi_value jsCb;
    size_t argc = 1;

    auto func = [](napi_env env, napi_callback_info info) -> napi_value {
        return nullptr;
    };

    napi_create_function(env, "testFunc", NAPI_AUTO_LENGTH, func, nullptr, &jsCb);

    NAPI_CALL(env, napi_get_instance_data(env, (void**)&data));
    NAPI_ASSERT(env, data->jsCbRef == nullptr, "reference must be nullptr");
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &jsCb, nullptr, nullptr));
    NAPI_CALL(env, napi_create_object(env, &value));
    NAPI_CALL(env, napi_add_finalizer(env, value, nullptr, TestFinalizer, nullptr, nullptr));
    NAPI_CALL(env, napi_create_reference(env, jsCb, 1, &data->jsCbRef));
    return nullptr;
}

HWTEST_F(NapiBasicTest, InstanceDataTest_001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    // Set instance data
    AddonDataTest* data = new AddonDataTest();
    data->value = 41;
    data->print = false;
    data->jsCbRef = nullptr;
    ASSERT_CHECK_CALL(napi_set_instance_data(env, data, DeleteAddonData, nullptr));

    // Test get instance data
    AddonDataTest* getData = nullptr;
    ASSERT_CHECK_CALL(napi_get_instance_data(env, (void**)&getData));
    ++getData->value;
    const size_t expectValue = 42;
    ASSERT_EQ(getData->value, expectValue);

    // Test finalizer
    SetPrintOnDelete(env, nullptr);
    ObjectWithFinalizer(env, nullptr);
}

HWTEST_F(NapiBasicTest, InstanceDataTest_002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    // Set instance data
    AddonDataTest* data = new AddonDataTest();
    data->value = 0;
    data->print = false;
    data->jsCbRef = nullptr;
    ASSERT_CHECK_CALL(napi_set_instance_data(env, data, DeleteAddonData, nullptr));

    // Test get instance data
    AddonDataTest* getData = nullptr;
    ASSERT_CHECK_CALL(napi_get_instance_data(env, (void**)&getData));
    ++getData->value;
    const size_t expectValue = 1;
    ASSERT_EQ(getData->value, expectValue);

    // Test finalizer
    SetPrintOnDelete(env, nullptr);
    ObjectWithFinalizer(env, nullptr);
}

/**
 * @tc.name: AsyncInitTest001.
 * @tc.desc: Test napi_async_init, napi_async_destroy.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AsyncInitTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value name;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "ACE_napi_async_init_Test_001",
        NAPI_AUTO_LENGTH, &name));

    napi_async_context context = nullptr;
    napi_status ret = napi_async_init(env, nullptr, name, &context);
    ASSERT_EQ(ret, napi_ok);
    EXPECT_NE(context, nullptr);

    ret = napi_async_destroy(env, context);
    ASSERT_EQ(ret, napi_ok);
}

/**
 * @tc.name: AsyncInitTest002
 * @tc.desc: Test napi_async_init with invalid arguments.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AsyncInitTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value resourceName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "test", NAPI_AUTO_LENGTH, &resourceName));

    napi_async_context* contextPtr = nullptr;
    napi_status status = napi_async_init(env, nullptr, resourceName, contextPtr);
    EXPECT_EQ(status, napi_invalid_arg);
}

/**
 * @tc.name: AsyncInitTest003
 * @tc.desc: Test napi_async_init with invalid arguments.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AsyncInitTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value resourceName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "中测", NAPI_AUTO_LENGTH, &resourceName));

    napi_async_context* contextPtr = nullptr;
    napi_status status = napi_async_init(env, nullptr, resourceName, contextPtr);
    EXPECT_EQ(status, napi_invalid_arg);
}

static void ExpectCheckCall(napi_status call)
{
    EXPECT_EQ(call, napi_ok);
}

static void Cleanup(void* arg)
{
    g_hookTag += INT_ONE;
    if (arg != nullptr) {
    }
}

static void CleanupCopy(void* arg)
{
    g_hookTagcp += INT_ONE;
    if (arg != nullptr) {
    }
}

/**
 * @tc.name: AddEnvCleanupHook001
 * @tc.desc: Test napi_add_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AddEnvCleanupHook001, testing::ext::TestSize.Level1)
{
    napi_env testEnv = reinterpret_cast<napi_env>(engine_);
    g_hookTag = INT_ZERO;
    ExpectCheckCall(napi_add_env_cleanup_hook(testEnv, Cleanup, &g_hookArgOne));
    engine_->RunCleanup();
    EXPECT_EQ(g_hookTag, INT_ONE);
}

/**
 * @tc.name: AddEnvCleanupHook002
 * @tc.desc: Test napi_add_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AddEnvCleanupHook002, testing::ext::TestSize.Level2)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_invalid_arg;
    res = napi_add_env_cleanup_hook(env, Cleanup, nullptr);
    engine_->RunCleanup();
    EXPECT_EQ(res, napi_ok);
}

/**
 * @tc.name: AddEnvCleanupHook003
 * @tc.desc: Test napi_add_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AddEnvCleanupHook003, testing::ext::TestSize.Level2)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;
    res = napi_add_env_cleanup_hook(env, nullptr, &g_hookArgOne);

    EXPECT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: AddEnvCleanupHook004
 * @tc.desc: Test napi_add_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AddEnvCleanupHook004, testing::ext::TestSize.Level2)
{
    napi_status res = napi_ok;
    res = napi_add_env_cleanup_hook(nullptr, Cleanup, &g_hookArgOne);
    engine_->RunCleanup();
    EXPECT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: AddEnvCleanupHook005
 * @tc.desc: Test napi_add_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AddEnvCleanupHook005, testing::ext::TestSize.Level1)
{
    napi_env testEnv = reinterpret_cast<napi_env>(engine_);
    g_hookTag = INT_ZERO;
    ExpectCheckCall(napi_add_env_cleanup_hook(testEnv, Cleanup, &g_hookArgOne));
    ExpectCheckCall(napi_remove_env_cleanup_hook(testEnv, Cleanup, &g_hookArgOne));
    ExpectCheckCall(napi_add_env_cleanup_hook(testEnv, Cleanup, &g_hookArgOne));
    engine_->RunCleanup();
    EXPECT_EQ(g_hookTag, INT_ONE);
}

/**
 * @tc.name: AddEnvCleanupHook006
 * @tc.desc: Test napi_add_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AddEnvCleanupHook006, testing::ext::TestSize.Level1)
{
    g_hookTag = INT_ZERO;
    napi_env testEnv = reinterpret_cast<napi_env>(engine_);
    ExpectCheckCall(napi_add_env_cleanup_hook(testEnv, Cleanup, &g_hookArgOne));
    ExpectCheckCall(napi_add_env_cleanup_hook(testEnv, Cleanup, &g_hookArgTwo));
    engine_->RunCleanup();
    EXPECT_EQ(g_hookTag, INT_TWO);
}

/**
 * @tc.name: AddEnvCleanupHook007
 * @tc.desc: Test napi_add_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AddEnvCleanupHook007, testing::ext::TestSize.Level1)
{
    napi_env testEnv = reinterpret_cast<napi_env>(engine_);
    g_hookTag = INT_ZERO;
    ExpectCheckCall(napi_add_env_cleanup_hook(testEnv, Cleanup, &g_hookArgOne));
    ExpectCheckCall(napi_add_env_cleanup_hook(testEnv, Cleanup, &g_hookArgTwo));
    ExpectCheckCall(napi_add_env_cleanup_hook(testEnv, Cleanup, &g_hookArgThree));
    engine_->RunCleanup();
    EXPECT_EQ(g_hookTag, INT_THREE);
}

/**
 * @tc.name: EnvCleanupHook008
 * @tc.desc: Test napi_add_env_cleanup_hook napi_remove_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, EnvCleanupHook008, testing::ext::TestSize.Level1)
{
    napi_env testEnv = reinterpret_cast<napi_env>(engine_);
    g_hookTag = INT_ZERO;
    ExpectCheckCall(napi_add_env_cleanup_hook(testEnv, Cleanup, &g_hookArgOne));
    ExpectCheckCall(napi_add_env_cleanup_hook(testEnv, Cleanup, &g_hookArgTwo));
    ExpectCheckCall(napi_remove_env_cleanup_hook(testEnv, Cleanup, &g_hookArgTwo));
    engine_->RunCleanup();
    EXPECT_EQ(g_hookTag, INT_ONE);
}

/**
 * @tc.name: EnvCleanupHook0009
 * @tc.desc: Test napi_add_env_cleanup_hook napi_remove_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, EnvCleanupHook0009, testing::ext::TestSize.Level2)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    g_hookTag = INT_ZERO;
    napi_status res = napi_invalid_arg;
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &g_hookArgOne));
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &g_hookArgTwo));
    res = napi_remove_env_cleanup_hook(env, Cleanup, nullptr);
    engine_->RunCleanup();
    EXPECT_EQ(g_hookTag, INT_TWO);
    EXPECT_EQ(res, napi_ok);
}

/**
 * @tc.name: EnvCleanupHook0010
 * @tc.desc: Test napi_add_env_cleanup_hook napi_remove_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, EnvCleanupHook0010, testing::ext::TestSize.Level2)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    g_hookTag = INT_ZERO;
    napi_status res = napi_ok;
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &g_hookArgOne));
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &g_hookArgTwo));
    res = napi_remove_env_cleanup_hook(env, nullptr, &g_hookArgTwo);
    engine_->RunCleanup();
    EXPECT_EQ(g_hookTag, INT_TWO);
    EXPECT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: EnvCleanupHook0011
 * @tc.desc: Test napi_add_env_cleanup_hook napi_remove_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, EnvCleanupHook0011, testing::ext::TestSize.Level2)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    g_hookTag = INT_ZERO;
    napi_status res = napi_ok;
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &g_hookArgOne));
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &g_hookArgTwo));
    res = napi_remove_env_cleanup_hook(nullptr, Cleanup, &g_hookArgTwo);
    engine_->RunCleanup();
    EXPECT_EQ(g_hookTag, INT_TWO);
    EXPECT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: EnvCleanupHook0012
 * @tc.desc: Test napi_add_env_cleanup_hook napi_remove_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, EnvCleanupHook0012, testing::ext::TestSize.Level2)
{
    napi_env testEnv = reinterpret_cast<napi_env>(engine_);
    g_hookTag = INT_ZERO;
    g_hookTagcp = INT_ZERO;
    ExpectCheckCall(napi_add_env_cleanup_hook(testEnv, Cleanup, &g_hookArgOne));
    ExpectCheckCall(napi_add_env_cleanup_hook(testEnv, CleanupCopy, &g_hookArgTwo));
    ExpectCheckCall(napi_remove_env_cleanup_hook(testEnv, Cleanup, &g_hookArgTwo));
    ExpectCheckCall(napi_remove_env_cleanup_hook(testEnv, CleanupCopy, &g_hookArgOne));
    engine_->RunCleanup();
    EXPECT_EQ(g_hookTag, INT_ONE);
    EXPECT_EQ(g_hookTagcp, INT_ONE);
}

/**
 * @tc.name: EnvCleanupHook0013
 * @tc.desc: Test napi_add_env_cleanup_hook napi_remove_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, EnvCleanupHook0013, testing::ext::TestSize.Level1)
{
    napi_env testEnv = reinterpret_cast<napi_env>(engine_);
    g_hookTag = INT_ZERO;
    g_hookTagcp = INT_ZERO;
    ExpectCheckCall(napi_add_env_cleanup_hook(testEnv, Cleanup, &g_hookArgOne));
    ExpectCheckCall(napi_add_env_cleanup_hook(testEnv, CleanupCopy, &g_hookArgTwo));
    ExpectCheckCall(napi_remove_env_cleanup_hook(testEnv, Cleanup, &g_hookArgOne));
    ExpectCheckCall(napi_remove_env_cleanup_hook(testEnv, CleanupCopy, &g_hookArgTwo));
    engine_->RunCleanup();
    EXPECT_EQ(g_hookTag, INT_ZERO);
    EXPECT_EQ(g_hookTagcp, INT_ZERO);
}

static void CleanupFirst(void* arg)
{
    if (g_hookTag != INT_TWO) {
        AssertFalse();
    }
    g_hookTag += INT_ONE;
}

static void CleanupSecond(void* arg)
{
    if (g_hookTag != INT_ONE) {
        AssertFalse();
    }
    g_hookTag += INT_ONE;
}

static void CleanupThird(void* arg)
{
    if (g_hookTag != INT_ZERO) {
        AssertFalse();
    }
    g_hookTag += INT_ONE;
}

/**
 * @tc.name: EnvCleanupHook0014
 * @tc.desc: Test napi_add_env_cleanup_hook napi_remove_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, EnvCleanupHook0014, testing::ext::TestSize.Level1)
{
    napi_env testEnv = reinterpret_cast<napi_env>(engine_);
    g_hookTag = INT_ZERO;
    ExpectCheckCall(napi_add_env_cleanup_hook(testEnv, CleanupFirst, &g_hookArgOne));
    ExpectCheckCall(napi_add_env_cleanup_hook(testEnv, CleanupSecond, &g_hookArgTwo));
    ExpectCheckCall(napi_add_env_cleanup_hook(testEnv, CleanupThird, &g_hookArgThree));
    engine_->RunCleanup();
    EXPECT_EQ(g_hookTag, INT_THREE);
}

struct AsyncData {
    uv_async_t async;
    napi_env env;
    napi_async_cleanup_hook_handle handle;
};

static void MustNotCall(napi_async_cleanup_hook_handle hook, void* arg)
{
    EXPECT_EQ(1, 0);
}

static struct AsyncData* CreateAsyncData()
{
    AsyncData* data = static_cast<AsyncData*>(malloc(sizeof(AsyncData)));
    if (data == nullptr) {
        return nullptr;
    }
    data->handle = nullptr;
    return data;
}

static void AfterCleanupHookTwo(uv_handle_t* handle)
{
    AsyncData* data = static_cast<AsyncData*>(handle->data);
    ExpectCheckCall(napi_remove_async_cleanup_hook(data->handle));
    g_hookTag += INT_ONE;
    free(data);
}

static void AfterCleanupHookOne(uv_async_t* async)
{
    uv_close((uv_handle_t*)async, AfterCleanupHookTwo);
}

static void AsyncCleanupHook(napi_async_cleanup_hook_handle handle, void* arg)
{
    AsyncData* data = static_cast<AsyncData*>(arg);
    uv_loop_t* loop;
    ExpectCheckCall(napi_get_uv_event_loop(data->env, &loop));
    int res = uv_async_init(loop, &data->async, AfterCleanupHookOne);
    EXPECT_EQ(res, 0);

    data->async.data = data;
    data->handle = handle;
    uv_async_send(&data->async);
}

/**
 * @tc.name: AsyncCleanupHook001
 * @tc.desc: Test napi_add_async_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AsyncCleanupHook001, testing::ext::TestSize.Level1)
{
    napi_env testEnv = reinterpret_cast<napi_env>(engine_);
    AsyncData* data = CreateAsyncData();
    if (data == nullptr) {
        return;
    }
    g_hookTag = INT_ZERO;
    data->env = testEnv;
    napi_status res = napi_add_async_cleanup_hook(testEnv, AsyncCleanupHook, data, &data->handle);
    engine_->RunCleanup();
    EXPECT_EQ(res, napi_ok);
    EXPECT_EQ(g_hookTag, INT_ONE);
}

/**
 * @tc.name: AsyncCleanupHook002
 * @tc.desc: Test napi_add_async_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AsyncCleanupHook002, testing::ext::TestSize.Level1)
{
    napi_env testEnv = reinterpret_cast<napi_env>(engine_);
    AsyncData* data = CreateAsyncData();
    if (data == nullptr) {
        return;
    }
    g_hookTag = INT_ZERO;
    data->env = testEnv;
    napi_status res = napi_add_async_cleanup_hook(testEnv, AsyncCleanupHook, data, nullptr);
    engine_->RunCleanup();
    EXPECT_EQ(g_hookTag, INT_ONE);
    EXPECT_EQ(res, napi_ok);
}

/**
 * @tc.name: AsyncCleanupHook003
 * @tc.desc: Test napi_add_async_cleanup_hook napi_remove_async_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ACE_Napi_Add_Async_Cleanup_Hook_0300, testing::ext::TestSize.Level2)
{
    napi_env testEnv = reinterpret_cast<napi_env>(engine_);
    napi_async_cleanup_hook_handle mustNotCallHandle;
    g_hookTag = INT_ZERO;
    ExpectCheckCall(napi_add_async_cleanup_hook(testEnv, MustNotCall, nullptr, &mustNotCallHandle));
    ExpectCheckCall(napi_remove_async_cleanup_hook(mustNotCallHandle));
    engine_->RunCleanup();
    EXPECT_EQ(g_hookTag, INT_ZERO);
}

/**
 * @tc.name: AsyncCleanupHook004
 * @tc.desc: Test napi_add_async_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ACE_Napi_Add_Async_Cleanup_Hook_0400, testing::ext::TestSize.Level2)
{
    napi_status res = napi_ok;
    napi_async_cleanup_hook_handle mustNotCallHandle;
    g_hookTag = INT_ZERO;
    res = napi_add_async_cleanup_hook(nullptr, MustNotCall, nullptr, &mustNotCallHandle);
    engine_->RunCleanup();
    EXPECT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: AsyncCleanupHook005
 * @tc.desc: Test napi_add_async_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ACE_Napi_Add_Async_Cleanup_Hook_0500, testing::ext::TestSize.Level2)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;
    napi_async_cleanup_hook_handle mustNotCallHandle;
    res = napi_add_async_cleanup_hook(env, nullptr, nullptr, &mustNotCallHandle);
    engine_->RunCleanup();
    EXPECT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: AsyncCleanupHook006
 * @tc.desc: Test napi_add_async_cleanup_hook napi_remove_async_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ACE_Napi_Add_Async_Cleanup_Hook_0600, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    AsyncData* data = CreateAsyncData();
    if (data == nullptr) {
        return;
    }
    data->env = env;
    g_hookTag = INT_ZERO;
    napi_status res = napi_invalid_arg;
    res = napi_add_async_cleanup_hook(env, AsyncCleanupHook, data, &data->handle);
    ASSERT_EQ(res, napi_ok);
    res = napi_remove_async_cleanup_hook(data->handle);
    ASSERT_EQ(res, napi_ok);
    res = napi_add_async_cleanup_hook(env, AsyncCleanupHook, data, &data->handle);
    ASSERT_EQ(res, napi_ok);
    engine_->RunCleanup();
    EXPECT_EQ(g_hookTag, INT_ONE);
    EXPECT_EQ(res, napi_ok);
}

/**
 * @tc.name: AsyncCleanupHook007
 * @tc.desc: Test napi_add_async_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AsyncCleanupHook007, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_env envTwo = reinterpret_cast<napi_env>(engine_);
    g_hookTag = INT_ZERO;
    AsyncData* data = CreateAsyncData();
    if (data == nullptr) {
        return;
    }
    data->env = env;
    napi_status res = napi_invalid_arg;
    res = napi_add_async_cleanup_hook(env, AsyncCleanupHook, data, &data->handle);
    EXPECT_EQ(res, napi_ok);
    AsyncData* dataTwo = CreateAsyncData();
    if (dataTwo == nullptr) {
        return;
    }
    dataTwo->env = envTwo;
    res = napi_add_async_cleanup_hook(env, AsyncCleanupHook, dataTwo, &dataTwo->handle);
    EXPECT_EQ(res, napi_ok);
    engine_->RunCleanup();
    EXPECT_EQ(g_hookTag, INT_TWO);
}

/**
 * @tc.name: AsyncCleanupHook008
 * @tc.desc: Test napi_add_async_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AsyncCleanupHook008, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_env envTwo = reinterpret_cast<napi_env>(engine_);
    napi_env envThree = reinterpret_cast<napi_env>(engine_);
    AsyncData* data = CreateAsyncData();
    if (data == nullptr) {
        return;
    }
    g_hookTag = INT_ZERO;
    data->env = env;
    napi_status res = napi_invalid_arg;
    res = napi_add_async_cleanup_hook(env, AsyncCleanupHook, data, &data->handle);
    EXPECT_EQ(res, napi_ok);

    AsyncData* dataTwo = CreateAsyncData();
    if (dataTwo == nullptr) {
        return;
    }
    dataTwo->env = envTwo;
    res = napi_add_async_cleanup_hook(env, AsyncCleanupHook, dataTwo, &dataTwo->handle);
    EXPECT_EQ(res, napi_ok);

    AsyncData* dataThree = CreateAsyncData();
    if (dataThree == nullptr) {
        return;
    }
    dataThree->env = envThree;
    res = napi_add_async_cleanup_hook(env, AsyncCleanupHook, dataThree, &dataThree->handle);
    EXPECT_EQ(res, napi_ok);
    engine_->RunCleanup();
    EXPECT_EQ(g_hookTag, INT_THREE);
}

/**
 * @tc.name: AsyncCleanupHook009
 * @tc.desc: Test napi_add_async_cleanup_hook napi_remove_async_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AsyncCleanupHook009, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    AsyncData* data = CreateAsyncData();
    if (data == nullptr) {
        return;
    }
    napi_status res = napi_invalid_arg;
    g_hookTag = INT_ZERO;
    data->env = env;
    res = napi_add_async_cleanup_hook(env, AsyncCleanupHook, data, &data->handle);
    EXPECT_EQ(res, napi_ok);
    res = napi_remove_async_cleanup_hook(data->handle);
    EXPECT_EQ(res, napi_ok);
    engine_->RunCleanup();
    EXPECT_EQ(g_hookTag, INT_ZERO);
}

/**
 * @tc.name: AsyncCleanupHook0010
 * @tc.desc: Test napi_remove_async_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AsyncCleanupHook0010, testing::ext::TestSize.Level2)
{
    napi_status res = napi_ok;
    res = napi_remove_async_cleanup_hook(nullptr);
    EXPECT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: AsyncCleanupHook0011
 * @tc.desc: Test napi_add_async_cleanup_hook napi_remove_async_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AsyncCleanupHook0011, testing::ext::TestSize.Level2)
{

    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_env envTwo = reinterpret_cast<napi_env>(engine_);
    AsyncData* data = CreateAsyncData();
    if (data == nullptr) {
        return;
    }
    napi_status res = napi_invalid_arg;
    data->env = env;
    res = napi_add_async_cleanup_hook(env, AsyncCleanupHook, data, nullptr);
    EXPECT_EQ(res, napi_ok);
    AsyncData* dataTwo = CreateAsyncData();
    if (dataTwo == nullptr) {
        return;
    }
    dataTwo->env = envTwo;
    res = napi_add_async_cleanup_hook(env, AsyncCleanupHook, dataTwo, &dataTwo->handle);
    EXPECT_EQ(res, napi_ok);
    res = napi_remove_async_cleanup_hook(dataTwo->handle);
    EXPECT_EQ(res, napi_ok);
    engine_->RunCleanup();
}

/**
 * @tc.name: nodeApiGetModuleFileName0001
 * @tc.desc: Test node_api_get_module_file_name.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, nodeApiGetModuleFileName0001, testing::ext::TestSize.Level1)
{
    const char *fileName;
    napi_env testEnv = reinterpret_cast<napi_env>(engine_);
    napi_value result;
    node_api_get_module_file_name(testEnv, &fileName);
    napi_create_string_utf8(testEnv, fileName, NAPI_AUTO_LENGTH, &result);
    ASSERT_TRUE(strcmp(fileName, "") == 0);
}

/**
 * @tc.name: nodeApiGetModuleFileName0002
 * @tc.desc: Test node_api_get_module_file_name.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, nodeApiGetModuleFileName0002, testing::ext::TestSize.Level1)
{
    const char *fileName;
    napi_env testEnv = reinterpret_cast<napi_env>(engine_);
    napi_value result;
    node_api_get_module_file_name(testEnv, &fileName);
    napi_create_string_utf8(testEnv, fileName, NAPI_AUTO_LENGTH, &result);
    ASSERT_FALSE(strcmp(fileName, "中测") == 2); // 2: test fileName length
}

/**
 * @tc.name: AsyncWorkTest002
 * @tc.desc: Test async work.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AsyncWorkTest002, testing::ext::TestSize.Level1)
{
    struct AsyncWorkContext {
        napi_async_work work = nullptr;
        bool executed = false;
    };
    napi_env env = reinterpret_cast<napi_env>(engine_);
    auto asyncWorkContext = new AsyncWorkContext();
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, TEST_CHAR_ASYNCWORK, NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        env, nullptr, resourceName, [](napi_env value, void* data) {
            AsyncWorkContext* asyncWorkContext = (AsyncWorkContext*)data;
            asyncWorkContext->executed = true;
        },
        [](napi_env env, napi_status status, void* data) {
            STOP_EVENT_LOOP(env);
            AsyncWorkContext* asyncWorkContext = (AsyncWorkContext*)data;
            std::cout << "status of task is: " << status << std::endl;
            napi_delete_async_work(env, asyncWorkContext->work);
            bool executed = asyncWorkContext->executed;
            delete asyncWorkContext;
            if (executed) {
                ASSERT_EQ(status, napi_status::napi_ok);
            } else {
                ASSERT_EQ(status, napi_status::napi_cancelled);
            }
        },
        asyncWorkContext, &asyncWorkContext->work);
    napi_queue_async_work(env, asyncWorkContext->work);
    RUN_EVENT_LOOP(env);
}

static napi_value CreateWithPropertiesTestGetter(napi_env env, napi_callback_info info)
{
    napi_value res;
    napi_get_boolean(env, false, &res);
    return res;
}

static napi_value CreateWithPropertiesTestSetter(napi_env env, napi_callback_info info)
{
    napi_value res;
    napi_get_boolean(env, true, &res);
    return res;
}

/**
 * @tc.name: CreateObjectWithPropertiesTest001
 * @tc.desc: Test napi_create_object_with_properteis.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, CreateObjectWithPropertiesTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value excep;
    ASSERT_CHECK_CALL(napi_get_and_clear_last_exception(env, &excep));
    napi_value val_false;
    napi_value val_true;
    ASSERT_CHECK_CALL(napi_get_boolean(env, false, &val_false));
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &val_true));
    napi_property_descriptor desc1[] = {
        DECLARE_NAPI_DEFAULT_PROPERTY("x", val_true),
    };
    napi_value obj1;
    ASSERT_CHECK_CALL(napi_create_object_with_properties(env, &obj1, 1, desc1));
    napi_value obj2;
    napi_property_descriptor desc2[] = {
        DECLARE_NAPI_DEFAULT_PROPERTY("a", val_false),
        DECLARE_NAPI_GETTER_SETTER("b", CreateWithPropertiesTestGetter, CreateWithPropertiesTestSetter),
        DECLARE_NAPI_DEFAULT_PROPERTY("c", obj1),
    };
    ASSERT_CHECK_CALL(napi_create_object_with_properties(env, &obj2, 3, desc2));
    ASSERT_CHECK_VALUE_TYPE(env, obj1, napi_object);
    ASSERT_CHECK_VALUE_TYPE(env, obj2, napi_object);
    auto checkPropertyEqualsTo = [env] (napi_value obj, const char *keyStr, napi_value expect) -> bool {
        napi_value result;
        napi_get_named_property(env, obj, keyStr, &result);
        bool equal = false;
        napi_strict_equals(env, result, expect, &equal);
        return equal;
    };
    // get obj1.x == true
    ASSERT_TRUE(checkPropertyEqualsTo(obj1, "x", val_true));
    // set obj1.x = false
    ASSERT_CHECK_CALL(napi_set_named_property(env, obj1, "x", val_false));
    // get obj1.x == false
    ASSERT_TRUE(checkPropertyEqualsTo(obj1, "x", val_false));
    // get obj2.a == false
    ASSERT_TRUE(checkPropertyEqualsTo(obj2, "a", val_false));
    // get obj2.b == false
    ASSERT_TRUE(checkPropertyEqualsTo(obj2, "b", val_false));
    // set obj2.b = true (useless)
    ASSERT_CHECK_CALL(napi_set_named_property(env, obj2, "b", val_true));
    // get obj2.b == false
    ASSERT_TRUE(checkPropertyEqualsTo(obj2, "b", val_false));
    // get obj2.c == obj1
    ASSERT_TRUE(checkPropertyEqualsTo(obj2, "c", obj1));
    // get obj2.c.x == false
    napi_value val_res;
    ASSERT_CHECK_CALL(napi_get_named_property(env, obj2, "c", &val_res));
    ASSERT_TRUE(checkPropertyEqualsTo(val_res, "x", val_false));
}

/**
 * @tc.name: CreateObjectWithNamedPropertiesTest001
 * @tc.desc: Test napi_create_object_with_named_properteis.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, CreateObjectWithNamedPropertiesTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value excep;
    ASSERT_CHECK_CALL(napi_get_and_clear_last_exception(env, &excep));
    napi_value val_false;
    napi_value val_true;
    ASSERT_CHECK_CALL(napi_get_boolean(env, false, &val_false));
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &val_true));
    const char *keys1[] = {
        "x",
    };
    const napi_value values1[] = {
        val_true,
    };
    napi_value obj1;
    ASSERT_CHECK_CALL(napi_create_object_with_named_properties(env, &obj1, 1, keys1, values1));
    napi_value obj2;
    const char *keys2[] = {
        "a",
        "b",
    };
    const napi_value values2[] = {
        val_false,
        obj1,
    };
    ASSERT_CHECK_CALL(napi_create_object_with_named_properties(env, &obj2, 2, keys2, values2));
    ASSERT_CHECK_VALUE_TYPE(env, obj1, napi_object);
    ASSERT_CHECK_VALUE_TYPE(env, obj2, napi_object);
    auto checkPropertyEqualsTo = [env] (napi_value obj, const char *keyStr, napi_value expect) -> bool {
        napi_value result;
        napi_get_named_property(env, obj, keyStr, &result);
        bool equal = false;
        napi_strict_equals(env, result, expect, &equal);
        return equal;
    };
    // get obj1.x == true
    ASSERT_TRUE(checkPropertyEqualsTo(obj1, "x", val_true));
    // set obj1.x = false
    ASSERT_CHECK_CALL(napi_set_named_property(env, obj1, "x", val_false));
    // get obj1.x == false
    ASSERT_TRUE(checkPropertyEqualsTo(obj1, "x", val_false));
    // get obj2.a == false
    ASSERT_TRUE(checkPropertyEqualsTo(obj2, "a", val_false));
    // get obj2.b == obj1
    ASSERT_TRUE(checkPropertyEqualsTo(obj2, "b", obj1));
    // get obj2.b.x == false
    napi_value val_res;
    ASSERT_CHECK_CALL(napi_get_named_property(env, obj2, "b", &val_res));
    ASSERT_TRUE(checkPropertyEqualsTo(val_res, "x", val_false));
}

/**
 * @tc.name: loadModuleWithInfo001
 * @tc.desc: Test napi_load_module_with_info with nullptr env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, loadModuleWithInfo001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_value result;
    napi_status res = napi_load_module_with_info(nullptr, nullptr, nullptr, &result);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: loadModuleWithInfo002
 * @tc.desc: Test napi_load_module_with_info with nullptr result.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, loadModuleWithInfo002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    napi_status res = napi_load_module_with_info(env, "@ohos.hilog", nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: loadModuleWithInfo003
 * @tc.desc: Test napi_load_module_with_info with nullptr result.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, loadModuleWithInfo003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    napi_status status = napi_load_module_with_info(env, "@ohos.hilog", "nullptr", nullptr);
    ASSERT_EQ(status, napi_invalid_arg);
}

/**
 * @tc.name: runEventLoopTest001
 * @tc.desc: Test napi_run_event_loop with nullptr env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, runEventLoopTest001, testing::ext::TestSize.Level1)
{
    napi_status res = napi_run_event_loop(nullptr, napi_event_mode_default);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: runEventLoopTest002
 * @tc.desc: Test napi_run_event_loop with nullptr env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, runEventLoopTest002, testing::ext::TestSize.Level1)
{
    napi_status res = napi_run_event_loop(nullptr, napi_event_mode_nowait);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: runEventLoopTest003
 * @tc.desc: Test napi_run_event_loop with nullptr loop
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, runEventLoopTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    std::thread t([]() {
        NativeEngineProxy engine;
        engine->Deinit();
        napi_status res = napi_run_event_loop(napi_env(engine), napi_event_mode_nowait);
        ASSERT_EQ(res, napi_invalid_arg);
        engine->Init();
    });
    t.join();
}

/**
 * @tc.name: runEventLoopTest004
 * @tc.desc: Test napi_run_event_loop with nullptr loop
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, runEventLoopTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    std::thread t([]() {
        NativeEngineProxy engine;
        engine->Deinit();
        napi_status res = napi_run_event_loop(napi_env(engine), napi_event_mode_default);
        engine->Init();
        ASSERT_EQ(res, napi_invalid_arg);
    });
    t.join();
}

/**
 * @tc.name: runEventLoopTest005
 * @tc.desc: Test napi_run_event_loop with main thread.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, runEventLoopTest005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    // main thread does not support napi_run_event_loop func
    napi_status res = napi_run_event_loop(env, napi_event_mode_default);
    ASSERT_EQ(res, napi_generic_failure);
}

/**
 * @tc.name: runEventLoopTest006
 * @tc.desc: Test napi_run_event_loop with main thread.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, runEventLoopTest006, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    // main thread does not support napi_run_event_loop func
    napi_status res = napi_run_event_loop(env, napi_event_mode_nowait);
    ASSERT_EQ(res, napi_generic_failure);
}

/**
 * @tc.name: runEventLoopTest007
 * @tc.desc: Test napi_run_event_loop with worker thread.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, runEventLoopTest007, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    engine_->MarkWorkerThread();
    napi_env env = (napi_env)engine_;
    // worker thread does not support napi_run_event_loop func
    napi_status res = napi_run_event_loop(env, napi_event_mode_nowait);
    ASSERT_EQ(res, napi_generic_failure);
    engine_->jsThreadType_ = DataProtector(uintptr_t(NativeEngine::JSThreadType::MAIN_THREAD));
}

/**
 * @tc.name: runEventLoopTest008
 * @tc.desc: Test napi_run_event_loop with worker thread.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, runEventLoopTest008, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    engine_->MarkWorkerThread();
    napi_env env = (napi_env)engine_;
    // worker thread does not support napi_run_event_loop func
    napi_status res = napi_run_event_loop(env, napi_event_mode_default);
    ASSERT_EQ(res, napi_generic_failure);
    engine_->jsThreadType_ = DataProtector(uintptr_t(NativeEngine::JSThreadType::MAIN_THREAD));
}

/**
 * @tc.name: runEventLoopTest009
 * @tc.desc: Test napi_run_event_loop with taskpool thread.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, runEventLoopTest009, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    engine_->MarkTaskPoolThread();
    napi_env env = (napi_env)engine_;
    // taskpool thread does not support napi_run_event_loop func
    napi_status res = napi_run_event_loop(env, napi_event_mode_nowait);
    ASSERT_EQ(res, napi_generic_failure);
    engine_->jsThreadType_ = DataProtector(uintptr_t(NativeEngine::JSThreadType::MAIN_THREAD));
}

/**
 * @tc.name: runEventLoopTest010
 * @tc.desc: Test napi_run_event_loop with taskpool thread.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, runEventLoopTest010, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    engine_->MarkTaskPoolThread();
    napi_env env = (napi_env)engine_;
    // taskpool thread does not support napi_run_event_loop func
    napi_status res = napi_run_event_loop(env, napi_event_mode_default);
    ASSERT_EQ(res, napi_generic_failure);
    engine_->jsThreadType_ = DataProtector(uintptr_t(NativeEngine::JSThreadType::MAIN_THREAD));
}

/**
 * @tc.name: stopEventLoopTest001
 * @tc.desc: Test napi_stop_event_loop with nullptr env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, stopEventLoopTest001, testing::ext::TestSize.Level1)
{
    napi_status res = napi_stop_event_loop(nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: stopEventLoopTest002
 * @tc.desc: Test napi_stop_event_loop with nullptr loop.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, stopEventLoopTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    std::thread t([]() {
        NativeEngineProxy engine;
        engine->Deinit();
        napi_status res = napi_stop_event_loop(napi_env(engine));
        engine->Init();
        ASSERT_EQ(res, napi_invalid_arg);
    });
    t.join();
}

/**
 * @tc.name: stopEventLoopTest003
 * @tc.desc: Test napi_stop_event_loop with main thread.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, stopEventLoopTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    // main thread does not support napi_run_event_loop func
    napi_status res = napi_stop_event_loop(env);
    ASSERT_EQ(res, napi_generic_failure);
}

/**
 * @tc.name: stopEventLoopTest004
 * @tc.desc: Test napi_stop_event_loop with worker thread.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, stopEventLoopTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    engine_->MarkWorkerThread();
    napi_env env = (napi_env)engine_;
    // worker thread does not support napi_run_event_loop func
    napi_status res = napi_stop_event_loop(env);
    ASSERT_EQ(res, napi_generic_failure);
    engine_->jsThreadType_ = DataProtector(uintptr_t(NativeEngine::JSThreadType::MAIN_THREAD));
}

/**
 * @tc.name: stopEventLoopTest005
 * @tc.desc: Test napi_stop_event_loop with taskpool thread.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, stopEventLoopTest005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    engine_->MarkTaskPoolThread();
    napi_env env = (napi_env)engine_;
    // taskpool thread does not support napi_run_event_loop func
    napi_status res = napi_stop_event_loop(env);
    ASSERT_EQ(res, napi_generic_failure);
    engine_->jsThreadType_ = DataProtector(uintptr_t(NativeEngine::JSThreadType::MAIN_THREAD));
}

/**
 * @tc.name: stopEventLoopTest006
 * @tc.desc: Test napi_stop_event_loop before running the loop.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, stopEventLoopTest006, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    engine_->MarkNativeThread();
    napi_env env = (napi_env)engine_;
    napi_status res = napi_stop_event_loop(env);
    ASSERT_EQ(res, napi_ok);
    engine_->jsThreadType_ = DataProtector(uintptr_t(NativeEngine::JSThreadType::MAIN_THREAD));
}

/**
 * @tc.name: stopEventLoopTest007
 * @tc.desc: Test napi_stop_event_loop before running the loop.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, stopEventLoopTest007, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    engine_->MarkTaskPoolThread();
    napi_env env = (napi_env)engine_;
    // taskpool thread does not support napi_run_event_loop func
    napi_status res = napi_run_event_loop(env, napi_event_mode_default);
    ASSERT_EQ(res, napi_generic_failure);
    res = napi_stop_event_loop(env);
    ASSERT_EQ(res, napi_generic_failure);
    engine_->jsThreadType_ = DataProtector(uintptr_t(NativeEngine::JSThreadType::MAIN_THREAD));
}

/**
 * @tc.name: NapiFatalExceptionTest
 * @tc.desc: Test interface of napi_fatal_exception
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiFatalExceptionTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    // create error object
    napi_value code = nullptr;
    constexpr char codeStr[] = "test code";
    napi_status res = napi_create_string_utf8(env, codeStr, NAPI_AUTO_LENGTH, &code);
    ASSERT_EQ(res, napi_ok);

    napi_value msg = nullptr;
    constexpr char msgStr[] = "test message";
    res = napi_create_string_utf8(env, msgStr, NAPI_AUTO_LENGTH, &msg);
    ASSERT_EQ(res, napi_ok);

    napi_value error = nullptr;
    res = napi_create_error(env, code, msg, &error);
    ASSERT_EQ(res, napi_ok);

    // call napi_fatal_exception interface with nullptr env
    res = napi_fatal_exception(nullptr, error);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiFatalExceptionTest
 * @tc.desc: Test interface of napi_fatal_exception
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiFatalExceptionTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    // create error object
    napi_value code = nullptr;
    constexpr char codeStr[] = "test code";
    napi_status res = napi_create_string_utf8(env, codeStr, NAPI_AUTO_LENGTH, &code);
    ASSERT_EQ(res, napi_ok);

    // call napi_fatal_exception interface with non-JSError object
    res = napi_fatal_exception(env, code);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiFatalExceptionTest
 * @tc.desc: Test interface of napi_fatal_exception
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiFatalExceptionTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    // call napi_fatal_exception interface with nullptr error
    auto res = napi_fatal_exception(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiFatalExceptionTest4
 * @tc.desc: Test interface of napi_fatal_exception
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiFatalExceptionTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    // create error object
    napi_value code = nullptr;
    constexpr char codeStr[] = "中测";
    napi_status res = napi_create_string_utf8(env, codeStr, NAPI_AUTO_LENGTH, &code);
    ASSERT_EQ(res, napi_ok);

    napi_value msg = nullptr;
    constexpr char msgStr[] = "中文测试";
    res = napi_create_string_utf8(env, msgStr, NAPI_AUTO_LENGTH, &msg);
    ASSERT_EQ(res, napi_ok);

    napi_value error = nullptr;
    res = napi_create_error(env, code, msg, &error);
    ASSERT_EQ(res, napi_ok);

    // call napi_fatal_exception interface with nullptr env
    res = napi_fatal_exception(nullptr, error);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiFatalExceptionTest
 * @tc.desc: Test interface of napi_coerce_to_bool
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCoerceToBoolTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    napi_value result;
    napi_status status = napi_coerce_to_bool(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCoerceToBoolTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value* result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_coerce_to_bool(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

/**
 * @tc.name: NapiFatalExceptionTest
 * @tc.desc: Test interface of napi_coerce_to_bool
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCoerceToBoolTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value result;
    napi_status status = napi_create_double(env, NAN, &value);
    status = napi_coerce_to_bool(env, value, &result);
    bool ret = true;
    napi_get_value_bool(env, result, &ret);
    ASSERT_EQ(ret, false);
    ASSERT_EQ(status, napi_ok);
}
/**
 * @tc.name: NapiFatalExceptionTest
 * @tc.desc: Test interface of napi_coerce_to_bool
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCoerceToBoolTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value result;
    napi_status status = napi_get_undefined(env, &value);
    status = napi_coerce_to_bool(env, value, &result);
    bool ret = true;
    napi_get_value_bool(env, result, &ret);
    ASSERT_EQ(ret, false);
    ASSERT_EQ(status, napi_ok);
}

/**
 * @tc.name: NapiFatalExceptionTest
 * @tc.desc: Test interface of napi_coerce_to_bool
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCoerceToBoolTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value result;
    napi_status status = napi_get_null(env, &value);
    status = napi_coerce_to_bool(env, value, &result);
    bool ret = true;
    napi_get_value_bool(env, result, &ret);
    ASSERT_EQ(ret, false);
    ASSERT_EQ(status, napi_ok);
}

HWTEST_F(NapiBasicTest, NapiCoerceToNumberTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    napi_value result;
    napi_status status = napi_coerce_to_number(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCoerceToNumberTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value* result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_coerce_to_number(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCoerceToNumberTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value result;
    napi_status status = napi_create_string_utf8(env, "", 0, &value);
    status = napi_coerce_to_number(env, value, &result);
    ASSERT_EQ(status, napi_ok);
    int32_t num;
    status = napi_get_value_int32(env, result, &num);
    ASSERT_EQ(num, 0);
}

HWTEST_F(NapiBasicTest, NapiCoerceToNumberTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value result;
    napi_status status = napi_create_string_utf8(env, TEST_STRING, 4, &value);
    status = napi_coerce_to_number(env, value, &result);
    ASSERT_EQ(status, napi_ok);
    double db;
    status = napi_get_value_double(env, result, &db);
    ASSERT_EQ(std::isnan(db), true);
}

HWTEST_F(NapiBasicTest, NapiCoerceToNumberTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value result;
    napi_status status = napi_get_undefined(env, &value);
    status = napi_coerce_to_number(env, value, &result);
    ASSERT_EQ(status, napi_ok);
    double db;
    status = napi_get_value_double(env, result, &db);
    ASSERT_EQ(std::isnan(db), true);
}

HWTEST_F(NapiBasicTest, NapiCoerceToNumberTest006, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value result;
    napi_status status = napi_get_null(env, &value);
    status = napi_coerce_to_number(env, value, &result);
    ASSERT_EQ(status, napi_ok);
    int32_t num;
    status = napi_get_value_int32(env, result, &num);
    ASSERT_EQ(num, 0);
}

HWTEST_F(NapiBasicTest, NapiCoerceToNumberTest007, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value result;
    napi_status status = napi_create_object(env, &value);
    status = napi_coerce_to_number(env, value, &result);
    ASSERT_EQ(status, napi_ok);
    double db;
    status = napi_get_value_double(env, result, &db);
    ASSERT_EQ(std::isnan(db), true);
}

HWTEST_F(NapiBasicTest, NapiCoerceToObjectTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    napi_value result;
    napi_status status = napi_coerce_to_object(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCoerceToObjectTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value* result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_coerce_to_object(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCoerceToObjectTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value result;
    napi_status status = napi_get_undefined(env, &value);
    status = napi_coerce_to_object(env, value, &result);
    ASSERT_EQ(status, napi_ok);
    napi_valuetype type = napi_undefined;
    status = napi_typeof(env, result, &type);
    ASSERT_EQ(status, napi_ok);
    ASSERT_EQ(type, napi_undefined);
}

HWTEST_F(NapiBasicTest, NapiCoerceToObjectTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value result;
    napi_status status = napi_get_null(env, &value);
    status = napi_coerce_to_object(env, value, &result);
    ASSERT_EQ(status, napi_ok);
    napi_valuetype type = napi_undefined;
    status = napi_typeof(env, result, &type);
    ASSERT_EQ(status, napi_ok);
    ASSERT_EQ(type, napi_undefined);
}

HWTEST_F(NapiBasicTest, NapiCoerceToStringTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    napi_value result;
    napi_status status = napi_coerce_to_string(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCoerceToStringTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value* result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_coerce_to_string(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCoerceToStringTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    constexpr char codeStr[] = "test code";
    napi_status status = napi_create_string_utf8(env, codeStr, NAPI_AUTO_LENGTH, &value);
    ASSERT_EQ(status, napi_ok);
    napi_value* result = nullptr;
    status = napi_coerce_to_string(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiTypeofTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    napi_valuetype result;
    napi_status status = napi_typeof(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiTypeofTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_valuetype* result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_typeof(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiInstanceofTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    napi_value constructor;
    bool result;
    napi_status status = napi_create_object(env, &constructor);
    status = napi_instanceof(env, value, constructor, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiInstanceofTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value constructor = nullptr;
    bool result;
    napi_status status = napi_create_object(env, &value);
    status = napi_instanceof(env, value, constructor, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiInstanceofTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value constructor;
    bool* result = nullptr;
    napi_status status = napi_create_object(env, &value);
    status = napi_create_object(env, &constructor);
    status = napi_instanceof(env, value, constructor, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiInstanceofTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value constructor;
    bool result;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_create_object(env, &constructor);
    status = napi_instanceof(env, value, constructor, &result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiInstanceofTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value constructor;
    bool result;
    napi_status status = napi_create_object(env, &value);
    status = napi_create_double(env, TEST_DOUBLE, &constructor);
    status = napi_instanceof(env, value, constructor, &result);
    ASSERT_EQ(status, napi_function_expected);
}

HWTEST_F(NapiBasicTest, NapiIsArrayTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    bool result;
    napi_status status = napi_is_array(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiIsArrayTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    bool* result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_is_array(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiIsArrayTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    bool* result = nullptr;
    napi_status status = napi_create_array(env, &value);
    status = napi_is_array(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiIsArrayTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    bool* result = nullptr;
    napi_status status = napi_create_array_with_length(env, INT_THREE, &value);
    status = napi_is_array(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiIsArrayBufferTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    bool result;
    napi_status status = napi_is_arraybuffer(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiIsArrayBufferTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    bool* result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_is_arraybuffer(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiIsArrayBufferTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    bool* result = nullptr;
    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    size_t arrayBufferSize = 0; // 0: define initial length
    napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);
    napi_status status = napi_is_arraybuffer(env, arrayBuffer, result);
    ASSERT_FALSE(result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiIsTypeBufferTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    bool result;
    napi_status status = napi_is_typedarray(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiIsTypeBufferTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    bool* result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_is_typedarray(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiIsTypeBufferTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    bool* result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_is_typedarray(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiIsDataViewTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    bool result;
    napi_status status = napi_is_dataview(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiIsDataViewTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    bool* result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_is_dataview(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiIsDateTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    bool result;
    napi_status status = napi_is_date(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiIsDateTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    bool* result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_is_date(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiStrictEqualsTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value lhs = nullptr;
    napi_value rhs;
    bool result;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &rhs);
    status = napi_strict_equals(env, lhs, rhs, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiStrictEqualsTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value lhs;
    napi_value rhs = nullptr;
    bool result;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &lhs);
    status = napi_strict_equals(env, lhs, rhs, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiStrictEqualsTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value lhs;
    napi_value rhs;
    bool* result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &lhs);
    status = napi_create_double(env, TEST_DOUBLE, &rhs);
    status = napi_strict_equals(env, lhs, rhs, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiStrictEqualsTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value lhs;
    napi_value rhs;
    bool result;
    napi_status status = napi_create_double(env, NAN, &lhs);
    status = napi_create_double(env, NAN, &rhs);
    status = napi_strict_equals(env, lhs, rhs, &result);
    ASSERT_EQ(status, false);
}

HWTEST_F(NapiBasicTest, NapiStrictEqualsTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value lhs;
    napi_value rhs;
    bool result;
    napi_create_int32(env, INT_ZERO, &lhs);
    napi_create_int32(env, INT_ONE, &rhs);
    napi_strict_equals(env, lhs, rhs, &result);
    ASSERT_FALSE(result);
    napi_create_int32(env, INT_ZERO, &rhs);
    napi_strict_equals(env, lhs, rhs, &result);
    ASSERT_TRUE(result);
}

HWTEST_F(NapiBasicTest, NapiGetPropertyNamesTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    napi_value result;
    napi_status status = napi_get_property_names(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetPropertyNamesTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value* result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_get_property_names(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetPropertyNamesTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value result;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_get_property_names(env, value, &result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiGetPropertyNamesTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value result;
    napi_status status = napi_create_int32(env, INT_ZERO, &value);
    status = napi_get_property_names(env, value, &result);
    ASSERT_NE(value, result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiSetPropertyTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_value key;
    napi_value value;

    napi_create_int32(env, INT_ONE, &key);
    napi_create_int32(env, INT_TWO, &value);
    napi_status status = napi_set_property(env, obj, key, value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiSetPropertyTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key = nullptr;
    napi_value value;

    napi_create_object(env, &obj);
    napi_create_int32(env, INT_TWO, &value);
    napi_status status = napi_set_property(env, obj, key, value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiSetPropertyTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key;
    napi_value value = nullptr;

    napi_create_object(env, &obj);
    napi_create_int32(env, INT_ONE, &key);
    napi_status status = napi_set_property(env, obj, key, value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiSetPropertyTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key;
    napi_value value;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    napi_create_int32(env, INT_ONE, &key);
    napi_create_int32(env, INT_TWO, &value);
    status = napi_set_property(env, obj, key, value);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiSetPropertyTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key;
    napi_value value;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    napi_create_int32(env, INT_ZERO, &key);
    napi_create_int32(env, INT_THREE, &value);
    status = napi_set_property(env, obj, key, value);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiGetPropertyTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_value key;
    napi_value result;

    napi_create_int32(env, INT_ONE, &key);
    napi_status status = napi_get_property(env, obj, key, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetPropertyTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key = nullptr;
    napi_value result;

    napi_create_object(env, &obj);
    napi_status status = napi_get_property(env, obj, key, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetPropertyTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key;
    napi_value* result = nullptr;

    napi_create_object(env, &obj);
    napi_create_int32(env, INT_ONE, &key);
    napi_status status = napi_get_property(env, obj, key, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetPropertyTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key;
    napi_value result;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    napi_create_int32(env, INT_ONE, &key);
    status = napi_get_property(env, obj, key, &result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiGetPropertyTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key;
    napi_value value;
    napi_value result;
    napi_create_double(env, TEST_DOUBLE, &obj);
    napi_create_int32(env, INT_ZERO, &key);
    napi_create_int32(env, INT_THREE, &value);
    napi_set_property(env, obj, key, value);
    napi_status status = napi_get_property(env, obj, key, &result);
    ASSERT_NE(value, result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiHasPropertyTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_value key;
    bool result;

    napi_create_int32(env, INT_ONE, &key);
    napi_status status = napi_has_property(env, obj, key, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiHasPropertyTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key = nullptr;
    bool result;

    napi_create_object(env, &obj);
    napi_status status = napi_has_property(env, obj, key, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiHasPropertyTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key;
    bool* result = nullptr;

    napi_create_object(env, &obj);
    napi_create_int32(env, INT_ONE, &key);
    napi_status status = napi_has_property(env, obj, key, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiHasPropertyTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key;
    bool result;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    napi_create_int32(env, INT_ONE, &key);
    status = napi_has_property(env, obj, key, &result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiHasPropertyTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key;
    napi_value value;
    bool result;

    napi_create_double(env, TEST_DOUBLE, &obj);
    napi_create_int32(env, INT_ZERO, &key);
    napi_create_int32(env, INT_ONE, &value);
    napi_set_property(env, obj, key, value);
    napi_has_property(env, obj, key, &result);
    ASSERT_FALSE(result);
}

HWTEST_F(NapiBasicTest, NapiDeletePropertyTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_value key;
    bool result;

    napi_create_int32(env, INT_ONE, &key);
    napi_status status = napi_delete_property(env, obj, key, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiDeletePropertyTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key = nullptr;
    bool result;

    napi_create_object(env, &obj);
    napi_status status = napi_delete_property(env, obj, key, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiDeletePropertyTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key;
    bool result;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    napi_create_int32(env, INT_ONE, &key);
    status = napi_delete_property(env, obj, key, &result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiDeletePropertyTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key;
    napi_value value;
    bool result;

    napi_create_double(env, TEST_DOUBLE, &obj);
    napi_create_int32(env, INT_ZERO, &key);
    napi_create_int32(env, INT_TWO, &value);
    napi_set_property(env, obj, key, value);
    napi_status status = napi_delete_property(env, obj, key, &result);
    ASSERT_FALSE(result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiHasOwnPropertyTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_value key;
    bool result;

    napi_create_int32(env, INT_ONE, &key);
    napi_status status = napi_has_own_property(env, obj, key, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiHasOwnPropertyTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key = nullptr;
    bool result;

    napi_create_object(env, &obj);
    napi_status status = napi_has_own_property(env, obj, key, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiHasOwnPropertyTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key;
    bool* result = nullptr;

    napi_create_object(env, &obj);
    napi_create_int32(env, INT_ONE, &key);
    napi_status status = napi_has_own_property(env, obj, key, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiHasOwnPropertyTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key;
    bool result;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    napi_create_int32(env, INT_ONE, &key);
    status = napi_has_own_property(env, obj, key, &result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiHasOwnPropertyTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key;
    napi_value value;
    bool result;

    napi_create_double(env, TEST_DOUBLE, &obj);
    napi_create_int32(env, INT_ONE, &key);
    napi_create_int32(env, INT_ONE, &value);
    napi_set_property(env, obj, key, value);
    napi_status status = napi_has_own_property(env, obj, key, &result);
    ASSERT_FALSE(result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiSetNamedPropertyTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_value value;

    napi_create_int32(env, INT_TWO, &value);
    napi_status status = napi_set_named_property(env, obj, TEST_STRING, value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiSetNamedPropertyTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    char* utf8name = nullptr;
    napi_value value;

    napi_create_object(env, &obj);
    napi_create_int32(env, INT_TWO, &value);
    napi_status status = napi_set_named_property(env, obj, utf8name, value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiSetNamedPropertyTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value value = nullptr;

    napi_create_object(env, &obj);
    napi_status status = napi_set_named_property(env, obj, TEST_STRING, value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiSetNamedPropertyTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value value;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    napi_create_int32(env, INT_TWO, &value);
    status = napi_set_named_property(env, obj, TEST_STRING, value);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiSetNamedPropertyTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value value;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    napi_create_int32(env, INT_THREE, &value);
    status = napi_set_named_property(env, obj, TEST_STRING, value);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiGetNamedPropertyTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_value value;

    napi_status status = napi_get_named_property(env, obj, TEST_STRING, &value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetNamedPropertyTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    char* utf8name = nullptr;
    napi_value value;

    napi_create_object(env, &obj);
    napi_status status = napi_get_named_property(env, obj, utf8name, &value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetNamedPropertyTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value* value = nullptr;

    napi_create_object(env, &obj);
    napi_status status = napi_get_named_property(env, obj, TEST_STRING, value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetNamedPropertyTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value value;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    status = napi_get_named_property(env, obj, TEST_STRING, &value);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiGetNamedPropertyTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value value;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    napi_create_int32(env, INT_THREE, &value);
    status = napi_set_named_property(env, obj, TEST_STRING, value);
    napi_value res;
    status = napi_get_named_property(env, obj, TEST_STRING, &res);
    ASSERT_NE(res, value);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiHasNamedPropertyTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    bool result;

    napi_status status = napi_has_named_property(env, obj, TEST_STRING, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiHasNamedPropertyTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    char* utf8name = nullptr;
    bool result;

    napi_create_object(env, &obj);
    napi_status status = napi_has_named_property(env, obj, utf8name, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiHasNamedPropertyTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    bool* result = nullptr;

    napi_create_object(env, &obj);
    napi_status status = napi_has_named_property(env, obj, TEST_STRING, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiHasNamedPropertyTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    bool result;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    status = napi_has_named_property(env, obj, TEST_STRING, &result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiHasNamedPropertyTest006, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value global, globalThis, proxy_constructor, proxy;
    napi_get_global(env, &global);
    ASSERT_CHECK_CALL(napi_get_named_property(env, global, "globalThis", &globalThis));
    ASSERT_CHECK_CALL(napi_get_named_property(env, global, "Proxy", &proxy_constructor));
    napi_value obj;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));
    napi_value args[2] = { obj, obj };
    ASSERT_CHECK_CALL(napi_new_instance(env, proxy_constructor, 2, args, &proxy));

    bool result = false;
    napi_status status = napi_has_named_property(env, proxy, "toString", &result);
    ASSERT_EQ(result, true);
    ASSERT_EQ(status, napi_ok);
}

HWTEST_F(NapiBasicTest, NapiHasNamedPropertyTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value value;

    napi_create_double(env, TEST_DOUBLE, &obj);
    napi_create_int32(env, INT_THREE, &value);
    napi_set_named_property(env, obj, TEST_STRING, value);
    bool result;
    napi_status status = napi_has_named_property(env, obj, TEST_STRING, &result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiSetElementTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    uint32_t index = 1;
    napi_value value;

    napi_create_int32(env, INT_TWO, &value);
    napi_status status = napi_set_element(env, obj, index, value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiSetElementTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    uint32_t index = 1;
    napi_value value = nullptr;

    napi_create_object(env, &obj);
    napi_status status = napi_set_element(env, obj, index, value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiSetElementTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    uint32_t index = 1;
    napi_value value;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    napi_create_int32(env, INT_TWO, &value);
    status = napi_set_element(env, obj, index, value);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiSetElementTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value value;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    napi_create_int32(env, INT_TWO, &value);
    status = napi_set_element(env, obj, INT_ONE, value);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiGetElementTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    uint32_t index = 1;
    napi_value value;

    napi_status status = napi_get_element(env, obj, index, &value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetElementTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    uint32_t index = 1;
    napi_value* value = nullptr;

    napi_create_object(env, &obj);
    napi_status status = napi_get_element(env, obj, index, value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetElementTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    uint32_t index = 1;
    napi_value value;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    status = napi_get_element(env, obj, index, &value);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiGetElementTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value value;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    ASSERT_EQ(status, napi_ok);
    napi_create_int32(env, INT_TWO, &value);
    napi_set_element(env, obj, INT_ONE, value);
    napi_value res;
    status = napi_get_element(env, obj, INT_ONE, &res);
    ASSERT_NE(value, res);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiHasElementTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    uint32_t index = 1;
    bool result;

    napi_status status = napi_has_element(env, obj, index, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiHasElementTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    uint32_t index = 1;
    bool* result = nullptr;

    napi_create_object(env, &obj);
    napi_status status = napi_has_element(env, obj, index, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiHasElementTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    uint32_t index = 1;
    bool result;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    status = napi_has_element(env, obj, index, &result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiHasElementTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value value;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    ASSERT_EQ(status, napi_ok);
    napi_create_int32(env, INT_TWO, &value);
    napi_set_element(env, obj, INT_ONE, value);
    bool res;
    status = napi_has_element(env, obj, INT_ONE, &res);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiDeleteElementTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    uint32_t index = 1;
    bool result;

    napi_status status = napi_delete_element(env, obj, index, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiDeleteElementTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    uint32_t index = 1;
    bool result;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    status = napi_delete_element(env, obj, index, &result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiDeleteElementTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value value;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    ASSERT_EQ(status, napi_ok);
    napi_create_int32(env, INT_THREE, &value);
    napi_set_element(env, obj, INT_ONE, value);
    bool res;
    status = napi_delete_element(env, obj, INT_ONE, &res);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiDefinePropertiesTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_property_descriptor desc[] = {
        { "testMethod", nullptr, [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }, nullptr,
          nullptr, nullptr, napi_default, nullptr },
    };
    napi_value result = nullptr;

    napi_status status = napi_define_properties(env, result, sizeof(desc) / sizeof(desc[0]), desc);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiDefinePropertiesTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_property_descriptor* desc = nullptr;
    napi_value result;
    napi_create_object(env, &result);

    napi_status status = napi_define_properties(env, result, INT_ONE, desc);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiDefinePropertiesTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_property_descriptor desc[] = {
        { "testMethod", nullptr, [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }, nullptr,
          nullptr, nullptr, napi_default, nullptr },
    };
    napi_value result;
    napi_create_double(env, TEST_DOUBLE, &result);

    napi_status status = napi_define_properties(env, result, INT_ONE, desc);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiDefinePropertiesTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_property_descriptor desc[] = {
        { nullptr, nullptr, [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }, nullptr,
          nullptr, nullptr, napi_default, nullptr },
    };
    napi_value result;
    napi_create_object(env, &result);

    napi_status status = napi_define_properties(env, result, INT_ONE, desc);
    ASSERT_EQ(status, napi_name_expected);
}

HWTEST_F(NapiBasicTest, NapiDefinePropertiesTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value name;
    napi_create_object(env, &name);
    napi_property_descriptor desc[] = {
        { nullptr, name, [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }, nullptr, nullptr,
          nullptr, napi_default, nullptr },
    };
    napi_value result;
    napi_create_object(env, &result);

    napi_status status = napi_define_properties(env, result, INT_ONE, desc);
    ASSERT_EQ(status, napi_name_expected);
}

HWTEST_F(NapiBasicTest, NapiDefinePropertiesTest006, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value func1 = nullptr;
    napi_status status = napi_create_function(env, "testAbort", NAPI_AUTO_LENGTH, TestAbort, nullptr, &func1);
    ASSERT_TRUE(status == napi_ok);
    napi_property_descriptor properties[] = {
        // napi_default_jsproperty = napi_writable | napi_enumerable | napi_configurable
        { "testAbort", nullptr, nullptr, nullptr, nullptr, func1, napi_default, nullptr }
    };
    napi_value globalObj = nullptr;
    napi_get_global(env, &globalObj);
    status = napi_define_properties(env, globalObj, sizeof(properties) / sizeof(properties[0]), properties);
    ASSERT_TRUE(status == napi_ok);
}

HWTEST_F(NapiBasicTest, NapiTypeTagObjectTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_type_tag tag;

    napi_status status = napi_type_tag_object(env, obj, &tag);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiTypeTagObjectTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_type_tag* tag = nullptr;
    napi_create_object(env, &obj);

    napi_status status = napi_type_tag_object(env, obj, tag);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiTypeTagObjectTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_type_tag tag;
    napi_create_double(env, TEST_DOUBLE, &obj);

    napi_status status = napi_type_tag_object(env, obj, &tag);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiTypeTagObjectTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_type_tag tag;
    napi_create_int32(env, INT_ONE, &obj);

    napi_status status = napi_type_tag_object(env, obj, &tag);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiCheckObjectTypeTagTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_type_tag tag;
    bool result;

    napi_status status = napi_check_object_type_tag(env, obj, &tag, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCheckObjectTypeTagTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_type_tag* tag = nullptr;
    bool result;
    napi_create_object(env, &obj);

    napi_status status = napi_check_object_type_tag(env, obj, tag, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCheckObjectTypeTagTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_type_tag tag;
    bool* result = nullptr;
    napi_create_object(env, &obj);

    napi_status status = napi_check_object_type_tag(env, obj, &tag, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCheckObjectTypeTagTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_type_tag tag;
    napi_create_int32(env, INT_ONE, &obj);

    napi_status status = napi_type_tag_object(env, obj, &tag);
    bool result;
    status = napi_check_object_type_tag(env, obj, &tag, &result);
    ASSERT_FALSE(result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiCallFunctionTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value funcValue = nullptr;
    napi_value recv = nullptr;
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_value funcResultValue = nullptr;

    napi_status status = napi_call_function(env, recv, funcValue, argc, args, &funcResultValue);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCallFunctionTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value funcValue = nullptr;
    napi_value recv = nullptr;
    size_t argc = 1;
    napi_value* args = nullptr;
    napi_value funcResultValue = nullptr;

    napi_create_function(
        env, "testFunc", NAPI_AUTO_LENGTH, [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
        nullptr, &funcValue);
    napi_status status = napi_call_function(env, recv, funcValue, argc, args, &funcResultValue);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCallFunctionTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value funcValue = nullptr;
    napi_value recv = nullptr;
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_value funcResultValue = nullptr;

    napi_create_object(env, &funcValue);
    napi_status status = napi_call_function(env, recv, funcValue, argc, args, &funcResultValue);
    ASSERT_EQ(status, napi_function_expected);
}

HWTEST_F(NapiBasicTest, NapiCallFunctionTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value funcValue = nullptr;
    napi_value recv = nullptr;
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_value funcResultValue = nullptr;

    napi_create_function(
        env, "testFunc", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_throw_error(env, "500", "Common error");
            return nullptr;
        },
        nullptr, &funcValue);
    napi_status status = napi_call_function(env, recv, funcValue, argc, args, &funcResultValue);
    ASSERT_EQ(status, napi_pending_exception);
}

HWTEST_F(NapiBasicTest, NapiCreateFunctionTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value funcValue = nullptr;

    napi_status status = napi_create_function(env, "testFunc", NAPI_AUTO_LENGTH, nullptr, nullptr, &funcValue);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateFunctionTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value* funcValue = nullptr;

    napi_status status = napi_create_function(
        env, "testFunc", NAPI_AUTO_LENGTH, [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
        nullptr, funcValue);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetCbInfoTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_callback_info info = nullptr;
    size_t argc = 0;
    napi_value* argv = nullptr;
    napi_value thisVar;
    void* data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetCbInfoTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_callback_info info = nullptr;
    size_t argc = 0; // 0: define initial length
    napi_value* argv = nullptr;
    napi_value thisVar;
    void* data = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    napi_value sendableArray = nullptr;
    napi_create_sendable_array(env, &sendableArray);
    ASSERT_NE(sendableArray, nullptr);
    napi_status status = napi_serialize(env, sendableArray, undefined, undefined, &data);
    ASSERT_NE(data, nullptr);
    ASSERT_EQ(status, napi_ok);

    status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetNewTargetTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_callback_info info = nullptr;
    napi_value result;

    napi_status status = napi_get_new_target(env, info, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetNewTargetTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_callback_info info = napi_callback_info(nullptr);
    ;
    napi_value* result = nullptr;

    napi_status status = napi_get_new_target(env, info, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetNewTargetTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_callback_info info = napi_callback_info(nullptr);
    ;
    napi_value result = nullptr;
    napi_create_int32(env, INT_ONE, &result);

    napi_status status = napi_get_new_target(env, info, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetNewTargetTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_callback_info info = napi_callback_info(nullptr);
    ;
    napi_value result = nullptr;
    napi_create_double(env, TEST_DOUBLE, &result);

    napi_status status = napi_get_new_target(env, info, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetNewTargetTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_callback_info info = napi_callback_info(nullptr);
    ;
    napi_value result = nullptr;
    napi_create_string_utf8(env, TEST_STRING, 4, &result);

    napi_status status = napi_get_new_target(env, info, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiNewInstanceTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value constructor = nullptr;
    size_t argc = 0;
    napi_value args[1] = { nullptr };
    napi_value result;

    napi_status status = napi_new_instance(env, constructor, argc, args, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiNewInstanceTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value constructor;
    size_t argc = 1;
    napi_value* args = nullptr;
    napi_value result;

    napi_create_object(env, &constructor);
    napi_status status = napi_new_instance(env, constructor, argc, args, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiNewInstanceTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value constructor;
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_value* result = nullptr;

    napi_create_object(env, &constructor);
    napi_status status = napi_new_instance(env, constructor, argc, args, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiNewInstanceTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value constructor;
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    napi_value result;

    napi_create_object(env, &constructor);
    napi_status status = napi_new_instance(env, constructor, argc, args, &result);
    ASSERT_EQ(status, napi_function_expected);
}

HWTEST_F(NapiBasicTest, NapiDefineClassTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value result;
    napi_status status = napi_define_class(
        env, nullptr, NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);

            return thisVar;
        },
        nullptr, 0, nullptr, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiDefineClassTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value result;
    napi_status status = napi_define_class(env, "TestClass", NAPI_AUTO_LENGTH, nullptr, nullptr, 0, nullptr, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiDefineClassTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value* result = nullptr;
    napi_status status = napi_define_class(
        env, "TestClass", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);

            return thisVar;
        },
        nullptr, 0, nullptr, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiDefineClassTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value result;
    napi_status status = napi_define_class(
        env, "TestClass", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);

            return thisVar;
        },
        nullptr, 1, nullptr, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiDefineClassTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value result;
    napi_status status = napi_define_class(
        env, "TestClass", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
            return thisVar;
        },
        nullptr, 0, nullptr, &result);

    const EcmaVM* vm = reinterpret_cast<ArkNativeEngine*>(engine_)->GetEcmaVm();
    Local<panda::FunctionRef> classFunc = LocalValueFromJsValue(result);
    Local<panda::StringRef> className = classFunc->GetName(vm);
    ASSERT_EQ(className->ToString(vm), "TestClass");
    ASSERT_EQ(status, napi_ok);
}

HWTEST_F(NapiBasicTest, NapiDefineClassTest006, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_property_descriptor properties[10];
    const std::string keyNames[10] = { "key0", "key1", "key2", "key3", "key4", "key5", "key6", "key7", "key8", "key9" };
    const int expectedVals[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int defaultAttributes = napi_default | napi_writable | napi_configurable;
    for (size_t i = 0; i < 10; ++i) {
        properties[i].utf8name = keyNames[i].c_str();
        properties[i].name = nullptr;
        properties[i].method = nullptr;
        properties[i].getter = nullptr;
        properties[i].setter = nullptr;
        if (i <= 5) {
            properties[i].attributes = static_cast<napi_property_attributes>(defaultAttributes | napi_static);
        } else {
            properties[i].attributes = static_cast<napi_property_attributes>(defaultAttributes);
        }
        napi_create_int32(env, expectedVals[i], &(properties[i].value));
    }

    napi_value result;
    napi_status status = napi_define_class(
        env, "TestClass", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
            return thisVar;
        },
        nullptr, 10, &properties[0], &result);

    const EcmaVM* vm = reinterpret_cast<ArkNativeEngine*>(engine_)->GetEcmaVm();
    Local<panda::FunctionRef> classFunc = LocalValueFromJsValue(result);
    Local<panda::ObjectRef> clsFuncProto = classFunc->GetFunctionPrototype(vm);
    for (size_t i = 0; i < 10; ++i) {
        Local<panda::JSValueRef> curProp;
        if (i <= 5) {
            curProp = classFunc->Get(vm, properties[i].utf8name);
        } else {
            curProp = clsFuncProto->Get(vm, properties[i].utf8name);
        }
        bool tmpBool = true;
        ASSERT_EQ(curProp->GetValueInt32(tmpBool), expectedVals[i]);
    }
    ASSERT_EQ(status, napi_ok);
}

static napi_value NativeCallBackForTest(napi_env env, napi_callback_info info)
{
    return nullptr;
}

HWTEST_F(NapiBasicTest, NapiDefineClassTest007, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_property_descriptor properties[panda::ObjectRef::MAX_PROPERTIES_ON_STACK + 1];
    const std::string keyNames[10] = { "key0", "key1", "key2", "key3", "key4", "key5", "key6", "key7", "key8", "key9" };
    const std::string lastKeys[4] = { "lastKey0", "lastKey1", "lastKey2", "lastKey3" };
    int expectedVals[panda::ObjectRef::MAX_PROPERTIES_ON_STACK + 1];
    for (size_t i = 0; i < panda::ObjectRef::MAX_PROPERTIES_ON_STACK + 1; ++i) {
        expectedVals[i] = i;
    }
    int defaultAttributes = napi_default | napi_writable | napi_configurable | napi_static;
    for (size_t i = 0; i < panda::ObjectRef::MAX_PROPERTIES_ON_STACK + 1; ++i) {
        if (i >= panda::ObjectRef::MAX_PROPERTIES_ON_STACK - 3) {
            properties[i].utf8name = lastKeys[i - (panda::ObjectRef::MAX_PROPERTIES_ON_STACK - 3)].c_str();
        } else {
            properties[i].utf8name = keyNames[i % 10].c_str();
        }
        properties[i].name = nullptr;
        properties[i].method = nullptr;
        properties[i].getter = nullptr;
        properties[i].setter = nullptr;
        properties[i].attributes = static_cast<napi_property_attributes>(defaultAttributes);
        napi_create_int32(env, expectedVals[i], &(properties[i].value));
    }
    properties[panda::ObjectRef::MAX_PROPERTIES_ON_STACK - 1].getter = NativeCallBackForTest;
    properties[panda::ObjectRef::MAX_PROPERTIES_ON_STACK - 2].setter = NativeCallBackForTest;
    properties[panda::ObjectRef::MAX_PROPERTIES_ON_STACK - 3].method = NativeCallBackForTest;

    napi_value result;
    napi_status status = napi_define_class(
        env, "TestClass", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
            return thisVar;
        },
        nullptr, panda::ObjectRef::MAX_PROPERTIES_ON_STACK + 1, &properties[0], &result);

    const EcmaVM* vm = reinterpret_cast<ArkNativeEngine*>(engine_)->GetEcmaVm();
    Local<panda::FunctionRef> classFunc = LocalValueFromJsValue(result);
    Local<panda::JSValueRef> curProp =
        classFunc->Get(vm, properties[panda::ObjectRef::MAX_PROPERTIES_ON_STACK].utf8name);
    bool tmpBool = true;
    ASSERT_EQ(curProp->GetValueInt32(tmpBool), expectedVals[panda::ObjectRef::MAX_PROPERTIES_ON_STACK]);

    ASSERT_EQ(status, napi_ok);
}

HWTEST_F(NapiBasicTest, NapiWrapTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_ref result;

    napi_status status = napi_wrap(
        env, obj, (void*)TEST_STRING, [](napi_env, void* data, void* hint) {}, nullptr, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiWrapTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_ref result;

    napi_create_object(env, &obj);
    napi_status status = napi_wrap(
        env, obj, nullptr, [](napi_env, void* data, void* hint) {}, nullptr, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiWrapTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_ref result;

    napi_create_object(env, &obj);
    napi_status status = napi_wrap(env, obj, (void*)TEST_STRING, nullptr, nullptr, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiWrapTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_ref result;

    napi_create_double(env, TEST_DOUBLE, &obj);
    napi_status status = napi_wrap(
        env, obj, (void*)TEST_STRING, [](napi_env, void* data, void* hint) {}, nullptr, &result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiWrapTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_ref result;

    napi_create_int32(env, INT_ONE, &obj);
    napi_status status = napi_wrap(
        env, obj, (void*)TEST_STRING, [](napi_env, void* data, void* hint) {}, nullptr, &result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiWrapTest006, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_ref result;

    napi_create_int64(env, INT_TWO, &obj);
    napi_status status = napi_wrap(
        env, obj, (void*)TEST_STRING, [](napi_env, void* data, void* hint) {}, nullptr, &result);
    ASSERT_EQ(status, napi_object_expected);
}

#ifdef PANDA_JS_ETS_HYBRID_MODE_TEST
HWTEST_F(NapiBasicTest, NapiWrapWithXRefTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_ref result;

    napi_create_object(env, &obj);
    napi_status status = napi_wrap_with_xref(
        env, obj, (void*)TEST_STRING, [](napi_env, void* data, void* hint) {}, &result);
    ASSERT_EQ(status, napi_ok);
}

HWTEST_F(NapiBasicTest, NapiWrapWithXRefTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_ref result;

    napi_create_object(env, &obj);
    napi_status status = napi_wrap_with_xref(
        env, obj, (void*)TEST_STRING, [](napi_env, void* data, void* hint) {}, &result);
    ASSERT_EQ(status, napi_ok);
}

HWTEST_F(NapiBasicTest, NapiWrapWithXRefTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_ref result;

    napi_status status = napi_wrap_with_xref(
        env, obj, (void*)TEST_STRING, [](napi_env, void* data, void* hint) {}, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiWrapWithXRefTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_ref result;

    napi_create_object(env, &obj);
    napi_status status = napi_wrap_with_xref(
        env, obj, nullptr, [](napi_env, void* data, void* hint) {}, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiWrapWithXRefTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_ref result;

    napi_create_object(env, &obj);
    napi_status status = napi_wrap_with_xref(env, obj, (void*)TEST_STRING, nullptr, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiWrapWithXRefTest006, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_ref result;

    napi_create_object(env, &obj);
    napi_status status = napi_wrap_with_xref(env, obj, (void*)TEST_STRING, nullptr, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiMarkFromObjectTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_ref result = nullptr;

    napi_create_object(env, &obj);
    napi_status status = napi_wrap_with_xref(
        env, obj, (void*)TEST_STRING, [](napi_env, void* data, void* hint) {}, &result);
    ASSERT_EQ(status, napi_ok);
    status = napi_mark_from_object(env, result);
    ASSERT_EQ(status, napi_ok);
}

HWTEST_F(NapiBasicTest, NapiCreateXRefTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    napi_ref booleanRef = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));

    auto res = napi_create_xref(env, boolean, 1, &booleanRef);
    ASSERT_EQ(res, napi_ok);
    ASSERT_CHECK_CALL(napi_delete_reference(env, booleanRef));
}

HWTEST_F(NapiBasicTest, NapiCreateXRefTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));

    auto res = napi_create_xref(env, boolean, 1, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateXRefTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_xref(env, nullptr, 1, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}
#endif

HWTEST_F(NapiBasicTest, NapiRegisterAppStateCallbakcTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_status status = napi_register_appstate_callback(env, nullptr);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiRegisterAppStateCallbakcTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_status status = napi_register_appstate_callback(env, [](int state, int64_t timestamp) {});
    ASSERT_EQ(status, napi_ok);
}

HWTEST_F(NapiBasicTest, NapiUnwrapTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    char* testStr = nullptr;

    napi_status status = napi_unwrap(env, obj, (void**)&testStr);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiUnwrapTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    char** testStr = nullptr;

    napi_create_object(env, &obj);
    napi_status status = napi_unwrap(env, obj, (void**)testStr);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiUnwrapTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    char* testStr = nullptr;

    napi_create_double(env, TEST_DOUBLE, &obj);
    napi_status status = napi_unwrap(env, obj, (void**)&testStr);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiUnwrapTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    char* testStr = nullptr;

    napi_create_int32(env, INT_ONE, &obj);
    napi_status status = napi_unwrap(env, obj, (void**)&testStr);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiUnwrapTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    char* testStr = nullptr;

    napi_create_int64(env, INT_TWO, &obj);
    napi_status status = napi_unwrap(env, obj, (void**)&testStr);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiUnwrapTest006, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    char** testStr = nullptr;
    auto func = [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; };

    napi_create_function(env, nullptr, 0, func, nullptr, &obj);
    napi_status status = napi_unwrap(env, obj, (void**)testStr);
    ASSERT_EQ(status, napi_invalid_arg);
}

#ifdef PANDA_JS_ETS_HYBRID_MODE_TEST
HWTEST_F(NapiBasicTest, NapiXrefUnwrapTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    char* testStr = nullptr;

    napi_create_object(env, &obj);
    napi_status status = napi_xref_unwrap(env, obj, (void**)&testStr);
    ASSERT_EQ(status, napi_ok);
}
#endif

HWTEST_F(NapiBasicTest, NapiRemoveWrapTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    char* testStr = nullptr;

    napi_status status = napi_remove_wrap(env, obj, (void**)&testStr);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiRemoveWrapTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    char** testStr = nullptr;

    napi_create_object(env, &obj);
    napi_status status = napi_remove_wrap(env, obj, (void**)testStr);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiRemoveWrapTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    char* testStr = nullptr;

    napi_create_double(env, TEST_DOUBLE, &obj);
    napi_status status = napi_remove_wrap(env, obj, (void**)&testStr);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiRemoveWrapTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    char *testStr = nullptr;

    napi_create_int32(env, INT_TWO, &obj);
    napi_status status = napi_remove_wrap(env, obj, (void **)&testStr);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiRemoveWrapTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    char *testStr = nullptr;

    napi_create_int64(env, INT_THREE, &obj);
    napi_status status = napi_remove_wrap(env, obj, (void **)&testStr);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiBasicTest, NapiRemoveWrapTest006, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    char **testStr = nullptr;
    auto func = [](napi_env env, napi_callback_info info) -> napi_value {
        return nullptr;
    };

    napi_create_function(env, nullptr, 0, func, nullptr, &obj);
    napi_status status = napi_remove_wrap(env, obj, (void **)testStr);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateAsyncWorkTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_work work = nullptr;
    napi_value resourceName = nullptr;

    napi_status status = napi_create_async_work(env, nullptr, resourceName, [](napi_env value, void* data) {},
        [](napi_env env, napi_status status, void* data) {}, nullptr, &work);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateAsyncWorkTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_work work = nullptr;
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, TEST_CHAR_ASYNCWORK, NAPI_AUTO_LENGTH, &resourceName);
    napi_status status = napi_create_async_work(env, nullptr, resourceName, nullptr,
                            [](napi_env env, napi_status status, void* data) {}, nullptr, &work);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateAsyncWorkTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_work work = nullptr;
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, TEST_CHAR_ASYNCWORK, NAPI_AUTO_LENGTH, &resourceName);
    napi_status status = napi_create_async_work(env, nullptr, resourceName, [](napi_env value, void* data) {},
                            nullptr, nullptr, &work);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateAsyncWorkTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_work* work = nullptr;
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, TEST_CHAR_ASYNCWORK, NAPI_AUTO_LENGTH, &resourceName);
    napi_status status = napi_create_async_work(env, nullptr, resourceName, [](napi_env value, void* data) {},
                            nullptr, nullptr, work);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateAsyncWorkTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_work work = nullptr;
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, TEST_CHAR_ASYNCWORK, NAPI_AUTO_LENGTH, &resourceName);
    napi_status status = napi_create_async_work(
        env, nullptr, resourceName,
        [](napi_env env, void *data) {},
        [](napi_env env, napi_status status, void *data) {},
        nullptr, &work);
    ASSERT_EQ(status, napi_ok);

    auto asyncWork = reinterpret_cast<NativeAsyncWork*>(work);
    auto asyncWorkTraceDesc = asyncWork->GetTraceDescription();
    ASSERT_TRUE(asyncWorkTraceDesc.find(TEST_CHAR_ASYNCWORK) != std::string::npos);
}

HWTEST_F(NapiBasicTest, NapiCreateAsyncWorkTest006, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_work work = nullptr;
    napi_value resourceName = nullptr;
    napi_get_null(env, &resourceName);
    napi_status status = napi_create_async_work(
        env, nullptr, resourceName,
        [](napi_env env, void *data) {},
        [](napi_env env, napi_status status, void *data) {},
        nullptr, &work);
    ASSERT_EQ(status, napi_ok);

    auto asyncWork = reinterpret_cast<NativeAsyncWork*>(work);
    auto asyncWorkTraceDesc = asyncWork->GetTraceDescription();
    ASSERT_FALSE(asyncWorkTraceDesc.find(TEST_CHAR_ASYNCWORK) != std::string::npos);
}

HWTEST_F(NapiBasicTest, NapiCreateAsyncWorkTest007, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_work work = nullptr;
    napi_value resourceName = nullptr;
    napi_get_undefined(env, &resourceName);
    napi_status status = napi_create_async_work(
        env, nullptr, resourceName,
        [](napi_env env, void *data) {},
        [](napi_env env, napi_status status, void *data) {},
        nullptr, &work);
    ASSERT_EQ(status, napi_ok);

    auto asyncWork = reinterpret_cast<NativeAsyncWork*>(work);
    auto asyncWorkTraceDesc = asyncWork->GetTraceDescription();
    ASSERT_FALSE(asyncWorkTraceDesc.find(TEST_CHAR_ASYNCWORK) != std::string::npos);
}

HWTEST_F(NapiBasicTest, NapiDeleteAsyncWorkTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_work work = nullptr;

    napi_status status = napi_delete_async_work(env, work);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiDeleteAsyncWorkTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_work work = nullptr;
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, TEST_STRING, NAPI_AUTO_LENGTH, &resourceName);
    napi_status status = napi_create_async_work(env, nullptr, resourceName, [](napi_env value, void* data) {},
                            nullptr, nullptr, &work);
    ASSERT_EQ(status, napi_invalid_arg);
    status = napi_delete_async_work(env, work);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiQueueAsyncWorkTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_work work = nullptr;

    napi_status status = napi_queue_async_work(env, work);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiQueueAsyncWorkTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_work work = nullptr;
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, TEST_STRING, NAPI_AUTO_LENGTH, &resourceName);
    napi_status status = napi_create_async_work(env, nullptr, resourceName, [](napi_env value, void* data) {},
                            nullptr, nullptr, &work);
    ASSERT_EQ(status, napi_invalid_arg);
    status = napi_queue_async_work(env, work);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCancelAsyncWorkTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_work work = nullptr;

    napi_status status = napi_cancel_async_work(env, work);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCancelAsyncWorkTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_work work = nullptr;
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, TEST_STRING, NAPI_AUTO_LENGTH, &resourceName);
    napi_status status = napi_create_async_work(env, nullptr, resourceName, [](napi_env value, void* data) {},
                            nullptr, nullptr, &work);
    ASSERT_EQ(status, napi_invalid_arg);
    status = napi_cancel_async_work(env, work);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiAsyncInitTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value resourceName = nullptr;
    napi_async_context context;

    napi_status status = napi_async_init(env, nullptr, resourceName, &context);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiAsyncInitTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value resourceName;
    napi_async_context* context = nullptr;

    napi_create_string_utf8(env, "test", NAPI_AUTO_LENGTH, &resourceName);
    napi_status status = napi_async_init(env, nullptr, resourceName, context);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiAsyncInitTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_work* work = nullptr;
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, TEST_STRING, NAPI_AUTO_LENGTH, &resourceName);
    napi_status status = napi_create_async_work(env, nullptr, resourceName, [](napi_env value, void* data) {},
                            nullptr, nullptr, work);
    ASSERT_EQ(status, napi_invalid_arg);
    napi_async_context* context = nullptr;
    status = napi_async_init(env, nullptr, resourceName, context);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiMakeCallbackTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_context context = nullptr;
    napi_value recv = nullptr;
    napi_value func;
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_value result = nullptr;

    napi_create_double(env, TEST_DOUBLE, &func);
    napi_status status = napi_make_callback(env, context, recv, func, argc, args, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiMakeCallbackTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_context context = nullptr;
    napi_value recv;
    napi_value func = nullptr;
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_value result = nullptr;

    napi_create_double(env, TEST_DOUBLE, &recv);
    napi_status status = napi_make_callback(env, context, recv, func, argc, args, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiAsyncDestroyTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_context context = nullptr;

    napi_status status = napi_async_destroy(env, context);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiAsyncDestroyTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_work* work = nullptr;
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, TEST_STRING, NAPI_AUTO_LENGTH, &resourceName);
    napi_status status = napi_create_async_work(env, nullptr, resourceName, [](napi_env value, void* data) {},
                           nullptr, nullptr, work);
    ASSERT_EQ(status, napi_invalid_arg);
    napi_async_context context = nullptr;
    napi_async_init(env, nullptr, resourceName, &context);
    status = napi_async_destroy(env, context);
    ASSERT_EQ(status, napi_ok);
}

HWTEST_F(NapiBasicTest, NapiOpenCallbackScopeTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_async_context context = nullptr;
    napi_callback_scope* result = nullptr;

    napi_status status = napi_open_callback_scope(env, obj, context, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiOpenCallbackScopeTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_async_context context = nullptr;
    napi_value name;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "test", NAPI_AUTO_LENGTH, &name));
    napi_status ret = napi_async_init(env, nullptr, name, &context);
    ASSERT_EQ(ret, napi_ok);
    EXPECT_NE(context, nullptr);
    napi_callback_scope* result = nullptr;

    napi_status status = napi_open_callback_scope(env, obj, context, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCloseCallbackScopeTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_callback_scope result = nullptr;

    napi_status status = napi_close_callback_scope(env, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCloseCallbackScopeTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_async_context context = nullptr;
    napi_value name;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "test", NAPI_AUTO_LENGTH, &name));
    napi_status ret = napi_async_init(env, nullptr, name, &context);
    ASSERT_EQ(ret, napi_ok);
    EXPECT_NE(context, nullptr);
    napi_callback_scope result = nullptr;
    napi_status status = napi_open_callback_scope(env, obj, context, &result);
    ASSERT_EQ(status, napi_ok);
    status = napi_close_callback_scope(env, result);
    ASSERT_EQ(status, napi_ok);
}

HWTEST_F(NapiBasicTest, NapiGetVersionTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    uint32_t* result = nullptr;

    napi_status status = napi_get_version(env, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetVersionTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    uint32_t result = 0; // 0: define initial res

    napi_status status = napi_get_version(env, &result);
    ASSERT_EQ(status, napi_ok);
}

HWTEST_F(NapiBasicTest, NapiCreatePromiseTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value* promise = nullptr;
    napi_deferred deferred = nullptr;

    napi_status status = napi_create_promise(env, &deferred, promise);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreatePromiseTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value promise = nullptr;
    napi_deferred* deferred = nullptr;

    napi_status status = napi_create_promise(env, deferred, &promise);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreatePromiseTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value promise = nullptr;
    napi_deferred deferred = nullptr;
    napi_value resolution = nullptr;
    napi_create_double(env, TEST_DOUBLE, &resolution);
    napi_status status = napi_resolve_deferred(env, deferred, resolution);
    ASSERT_EQ(status, napi_invalid_arg);
    status = napi_create_promise(env, &deferred, &promise);
    ASSERT_EQ(status, napi_ok);
}

HWTEST_F(NapiBasicTest, NapiResolveDeferredTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_deferred deferred = nullptr;

    napi_value resolution = nullptr;
    napi_create_double(env, TEST_DOUBLE, &resolution);
    napi_status status = napi_resolve_deferred(env, deferred, resolution);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiResolveDeferredTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);

    napi_value resolution = nullptr;
    napi_status status = napi_resolve_deferred(env, deferred, resolution);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiRejectDeferredTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_deferred deferred = nullptr;

    napi_value resolution = nullptr;
    napi_create_double(env, TEST_DOUBLE, &resolution);
    napi_status status = napi_reject_deferred(env, deferred, resolution);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiRejectDeferredTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);

    napi_value resolution = nullptr;
    napi_status status = napi_reject_deferred(env, deferred, resolution);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiIsPromiseTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value promise = nullptr;
    bool result;

    napi_status status = napi_is_promise(env, promise, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiIsPromiseTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);
    bool* result = nullptr;

    napi_status status = napi_is_promise(env, promise, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiIsPromiseTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);
    bool result = false;

    napi_status status = napi_is_promise(env, promise, &result);
    ASSERT_TRUE(result);
    ASSERT_EQ(status, napi_ok);
}

HWTEST_F(NapiBasicTest, NapiGetUvEventLoopTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    struct uv_loop_s** loop = nullptr;

    napi_status status = napi_get_uv_event_loop(env, loop);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateThreadsafeFunctionTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value jsCb = 0;
    napi_create_object(env, &jsCb);
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = nullptr;
    int32_t callJsCbDataTestId = 101;
    int32_t finalCbDataTestId = 1001;
    napi_status status = napi_create_threadsafe_function(env, jsCb, nullptr, resourceName,
                                                         0, 1, &callJsCbDataTestId,
                                                         nullptr, &finalCbDataTestId, nullptr, &tsFunc);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateThreadsafeFunctionTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value jsCb = 0;
    napi_create_object(env, &jsCb);
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    int32_t callJsCbDataTestId = 101;
    int32_t finalCbDataTestId = 1001;
    napi_status status = napi_create_threadsafe_function(env, jsCb, nullptr, resourceName,
                                                         0, 129, &callJsCbDataTestId,
                                                         nullptr, &finalCbDataTestId, nullptr, &tsFunc);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetThreadsafeFunctionContextTest001, testing::ext::TestSize.Level1)
{
    napi_threadsafe_function tsFunc = nullptr;
    void** result = nullptr;
    napi_status status = napi_get_threadsafe_function_context(tsFunc, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

struct CallbackData {
    napi_threadsafe_function tsfn;
    napi_async_work work;
};

HWTEST_F(NapiBasicTest, NapiGetThreadsafeFunctionContextTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value resourceName = 0; // 0: define initial res
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    CallbackData *callbackData = new CallbackData();

    napi_create_threadsafe_function(env, nullptr, nullptr, resourceName,
        0, 1, callbackData, nullptr, callbackData, nullptr, &callbackData->tsfn);
    void** result = nullptr;
    napi_status status = napi_get_threadsafe_function_context(callbackData->tsfn, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCallThreadsafeFunctionTest001, testing::ext::TestSize.Level1)
{
    napi_threadsafe_function tsFunc = nullptr;
    void* result = nullptr;
    napi_status status = napi_call_threadsafe_function(tsFunc, result, napi_tsfn_blocking);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCallThreadsafeFunctionTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value resourceName = 0; // 0: define initial res
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    CallbackData *callbackData = new CallbackData();

    napi_create_threadsafe_function(env, nullptr, nullptr, resourceName,
        0, 1, callbackData, nullptr, callbackData, nullptr, &callbackData->tsfn);
    void* result = nullptr;
    napi_status status = napi_call_threadsafe_function(callbackData->tsfn, result, napi_tsfn_blocking);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiAcquireThreadsafeFunctionTest001, testing::ext::TestSize.Level1)
{
    napi_threadsafe_function tsFunc = nullptr;
    napi_status status = napi_acquire_threadsafe_function(tsFunc);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiAcquireThreadsafeFunctionTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value resourceName = 0; // 0: define initial res
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    CallbackData *callbackData = new CallbackData();

    napi_create_threadsafe_function(env, nullptr, nullptr, resourceName,
        0, 1, callbackData, nullptr, callbackData, nullptr, &callbackData->tsfn);
    napi_status status = napi_acquire_threadsafe_function(callbackData->tsfn);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiReleaseThreadsafeFunctionTest001, testing::ext::TestSize.Level1)
{
    napi_threadsafe_function tsFunc = nullptr;
    napi_status status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiReleaseThreadsafeFunctionTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value resourceName = 0; // 0: define initial res
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    CallbackData *callbackData = new CallbackData();

    napi_create_threadsafe_function(env, nullptr, nullptr, resourceName,
        0, 1, callbackData, nullptr, callbackData, nullptr, &callbackData->tsfn);
    napi_status status = napi_release_threadsafe_function(callbackData->tsfn, napi_tsfn_release);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiReleaseThreadsafeFunctionTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value resourceName = 0; // 0: define initial res
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    CallbackData *callbackData = new CallbackData();

    napi_create_threadsafe_function(env, nullptr, nullptr, resourceName,
        0, 1, callbackData, nullptr, callbackData, nullptr, &callbackData->tsfn);
    napi_status status = napi_release_threadsafe_function(callbackData->tsfn, napi_tsfn_abort);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiRefThreadsafeFunctionTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_threadsafe_function tsFunc = nullptr;
    napi_status status = napi_ref_threadsafe_function(env, tsFunc);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiRefThreadsafeFunctionTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value resourceName = 0; // 0: define initial res
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    CallbackData *callbackData = new CallbackData();

    napi_create_threadsafe_function(env, nullptr, nullptr, resourceName,
        0, 1, callbackData, nullptr, callbackData, nullptr, &callbackData->tsfn);
    napi_status status = napi_ref_threadsafe_function(env, callbackData->tsfn);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiUnrefThreadsafeFunctionTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_threadsafe_function tsFunc = nullptr;
    napi_status status = napi_unref_threadsafe_function(env, tsFunc);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiUnrefThreadsafeFunctionTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value resourceName = 0; // 0: define initial res
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    CallbackData *callbackData = new CallbackData();

    napi_create_threadsafe_function(env, nullptr, nullptr, resourceName,
        0, 1, callbackData, nullptr, callbackData, nullptr, &callbackData->tsfn);
    napi_status status = napi_unref_threadsafe_function(env, callbackData->tsfn);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateDateTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value* result = nullptr;

    napi_status status = napi_create_date(env, TEST_DOUBLE, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetDateValueTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value date = nullptr;
    double result;

    napi_status status = napi_get_date_value(env, date, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetDateValueTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    double time = 202110181203150;
    napi_value date;
    double* result = nullptr;

    napi_status status = napi_create_date(env, time, &date);
    status = napi_get_date_value(env, date, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetDateValueTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value date;
    double result;

    napi_status status = napi_create_object(env, &date);
    status = napi_get_date_value(env, date, &result);
    ASSERT_EQ(status, napi_date_expected);
}

HWTEST_F(NapiBasicTest, NapiGetDateValueTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    double time = 202110181203150;
    napi_value date;
    double result;

    napi_status status = napi_create_date(env, time, &date);
    status = napi_get_date_value(env, date, &result);
    ASSERT_EQ(status, napi_ok);
    ASSERT_EQ(time, result);
}

HWTEST_F(NapiBasicTest, NapiCreateBigintInt64Test001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    int64_t value = INT_ONE;
    napi_value* result = nullptr;

    napi_status status = napi_create_bigint_int64(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateBigintInt64Test002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    int64_t value = INT_TWO;
    napi_value result = nullptr;

    napi_status status = napi_create_bigint_int64(env, value, &result);
    ASSERT_EQ(status, napi_ok);
}

HWTEST_F(NapiBasicTest, NapiCreateBigintUint64Test001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    int64_t value = INT_ONE;
    napi_value* result = nullptr;

    napi_status status = napi_create_bigint_uint64(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateBigintUint64Test002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    int64_t value = INT_TWO;
    napi_value result = nullptr;

    napi_status status = napi_create_bigint_uint64(env, value, &result);
    ASSERT_EQ(status, napi_ok);
}

HWTEST_F(NapiBasicTest, NapiCreateBigintWordsTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    int signBit = 0;
    size_t wordCount = 4;
    uint64_t* words = nullptr;
    napi_value result = nullptr;

    napi_status status = napi_create_bigint_words(env, signBit, wordCount, words, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateBigintWordsTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    int signBit = 0;
    size_t wordCount = 4;
    uint64_t words[] = {0ULL, 34ULL, 56ULL, 2ULL};
    napi_value* result = nullptr;

    napi_status status = napi_create_bigint_words(env, signBit, wordCount, words, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateBigintWordsTest003, testing::ext::TestSize.Level1)
{
    int signBit = 0;
    size_t wordCount = 4;
    uint64_t words[] = {12ULL, 34ULL, 56ULL, 78ULL};
    uint64_t wordsOut[] = {0ULL, 0ULL, 0ULL, 0ULL};
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    napi_status status = napi_create_bigint_words(env, signBit, wordCount, words, &value);
    ASSERT_EQ(status, napi_ok);

    {
        int retSignBit = -1;
        size_t retWordCount = wordCount;
        status = napi_get_value_bigint_words(env, value, &retSignBit, &retWordCount, wordsOut);
        ASSERT_EQ(status, napi_ok);
        ASSERT_EQ(retSignBit, 0);
        ASSERT_EQ(wordCount, retWordCount);
        ASSERT_EQ(words[0], wordsOut[0]);
        ASSERT_EQ(words[1], wordsOut[1]);
        ASSERT_EQ(words[2], wordsOut[2]);
        ASSERT_EQ(words[3], wordsOut[3]);
    }

    {
        int64_t result = 0;
        bool lossless = false;
        status = napi_get_value_bigint_int64(env, value, &result, &lossless);
        ASSERT_EQ(status, napi_ok);
        ASSERT_EQ(lossless, false);
        ASSERT_EQ(result, 12);
    }

    {
        uint64_t result = 0;
        bool lossless = false;
        status = napi_get_value_bigint_uint64(env, value, &result, &lossless);
        ASSERT_EQ(status, napi_ok);
        ASSERT_EQ(lossless, false);
        ASSERT_EQ(result, 12);
    }
}

HWTEST_F(NapiBasicTest, NapiCreateBigintWordsTest004, testing::ext::TestSize.Level1)
{
    int signBit = 3;
    size_t wordCount = 4;
    uint64_t words[] = {0xFFFFFFFFFFFFFFFF, 34ULL, 56ULL, 78ULL};
    uint64_t wordsOut[] = {0ULL, 0ULL, 0ULL, 0ULL};
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    napi_status status = napi_create_bigint_words(env, signBit, wordCount, words, &value);
    ASSERT_EQ(status, napi_ok);

    {
        int retSignBit = -1;
        size_t retWordCount = wordCount;
        status = napi_get_value_bigint_words(env, value, &retSignBit, &retWordCount, wordsOut);
        ASSERT_EQ(status, napi_ok);
        ASSERT_EQ(retSignBit, 1);
        ASSERT_EQ(wordCount, retWordCount);
        ASSERT_EQ(words[0], wordsOut[0]);
        ASSERT_EQ(words[1], wordsOut[1]);
        ASSERT_EQ(words[2], wordsOut[2]);
        ASSERT_EQ(words[3], wordsOut[3]);
    }

    {
        int64_t result = 0;
        bool lossless = false;
        status = napi_get_value_bigint_int64(env, value, &result, &lossless);
        ASSERT_EQ(status, napi_ok);
        ASSERT_EQ(lossless, false);
        ASSERT_EQ(result, 1);
    }

    {
        uint64_t result = 0;
        bool lossless = false;
        status = napi_get_value_bigint_uint64(env, value, &result, &lossless);
        ASSERT_EQ(status, napi_ok);
        ASSERT_EQ(lossless, false);
        ASSERT_EQ(result, 1);
    }
}

HWTEST_F(NapiBasicTest, NapiCreateBigintWordsTest005, testing::ext::TestSize.Level1)
{
    int signBit = 3;
    size_t wordCount = 4;
    uint64_t words[] = {0xFFFFFFFFFFFFFFFF, 34ULL, 56ULL, 78ULL};
    uint64_t wordsOut[] = {0ULL, 0ULL, 0ULL, 0ULL};
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    napi_status status = napi_create_bigint_words(env, signBit, wordCount, words, &value);
    ASSERT_EQ(status, napi_ok);
    signBit = 4;
    {
        int retSignBit = -1;
        size_t retWordCount = wordCount;
        status = napi_get_value_bigint_words(env, value, &retSignBit, &retWordCount, wordsOut);
        ASSERT_EQ(status, napi_ok);
        ASSERT_EQ(retSignBit, 1);
        ASSERT_EQ(wordCount, retWordCount);
        ASSERT_EQ(words[0], wordsOut[0]);
        ASSERT_EQ(words[1], wordsOut[1]);
        ASSERT_EQ(words[2], wordsOut[2]);
        ASSERT_EQ(words[3], wordsOut[3]);
    }

    {
        int64_t result = 0;
        bool lossless = false;
        status = napi_get_value_bigint_int64(env, value, &result, &lossless);
        ASSERT_EQ(status, napi_ok);
        ASSERT_EQ(lossless, false);
        ASSERT_EQ(result, 1);
    }

    {
        uint64_t result = 0;
        bool lossless = false;
        status = napi_get_value_bigint_uint64(env, value, &result, &lossless);
        ASSERT_EQ(status, napi_ok);
        ASSERT_EQ(lossless, false);
        ASSERT_EQ(result, 1);
    }
}

HWTEST_F(NapiBasicTest, NapiCreateBigintWordsTest006, testing::ext::TestSize.Level1)
{
    size_t wordCount = 4;
    uint64_t wordsOut[] = {0ULL, 0ULL, 0ULL, 0ULL};
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    int64_t testValue = INT64_MIN;
    napi_status status = napi_create_bigint_int64(env, testValue, &value);
    ASSERT_EQ(status, napi_ok);

    {
        int retSignBit = -1;
        size_t retWordCount = wordCount;
        status = napi_get_value_bigint_words(env, value, &retSignBit, &retWordCount, wordsOut);
        ASSERT_EQ(status, napi_ok);
        ASSERT_EQ(retSignBit, 1);
        ASSERT_EQ(retWordCount, 1);
        ASSERT_EQ(wordsOut[0], INT64_MIN);
        ASSERT_EQ(wordsOut[1], 0);
        ASSERT_EQ(wordsOut[2], 0);
        ASSERT_EQ(wordsOut[3], 0);
    }

    {
        int64_t result = 0;
        bool lossless = false;
        status = napi_get_value_bigint_int64(env, value, &result, &lossless);
        ASSERT_EQ(status, napi_ok);
        ASSERT_EQ(lossless, true);
        ASSERT_EQ(result, INT64_MIN);
    }

    {
        uint64_t result = 0;
        bool lossless = false;
        status = napi_get_value_bigint_uint64(env, value, &result, &lossless);
        ASSERT_EQ(status, napi_ok);
        ASSERT_EQ(lossless, false);
        ASSERT_EQ(result, INT64_MAX + 1);
    }
}

HWTEST_F(NapiBasicTest, NapiCreateBigintWordsTest007, testing::ext::TestSize.Level1)
{
    size_t wordCount = 4;
    uint64_t wordsOut[] = {0ULL, 0ULL, 0ULL, 0ULL};
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    uint64_t testValue = UINT64_MAX;
    napi_status status = napi_create_bigint_uint64(env, testValue, &value);
    ASSERT_EQ(status, napi_ok);

    {
        int retSignBit = -1;
        size_t retWordCount = wordCount;
        status = napi_get_value_bigint_words(env, value, &retSignBit, &retWordCount, wordsOut);
        ASSERT_EQ(status, napi_ok);
        ASSERT_EQ(retSignBit, 0);
        ASSERT_EQ(retWordCount, 1);
        ASSERT_EQ(wordsOut[0], UINT64_MAX);
        ASSERT_EQ(wordsOut[1], 0);
        ASSERT_EQ(wordsOut[2], 0);
        ASSERT_EQ(wordsOut[3], 0);
    }

    {
        int64_t result = 0;
        bool lossless = false;
        status = napi_get_value_bigint_int64(env, value, &result, &lossless);
        ASSERT_EQ(status, napi_ok);
        ASSERT_EQ(lossless, false);
        ASSERT_EQ(result, -1);
    }

    {
        uint64_t result = 0;
        bool lossless = false;
        status = napi_get_value_bigint_uint64(env, value, &result, &lossless);
        ASSERT_EQ(status, napi_ok);
        ASSERT_EQ(lossless, true);
        ASSERT_EQ(result, UINT64_MAX);
    }
}

HWTEST_F(NapiBasicTest, NapiGetValueBigintInt64Test001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    int64_t result = 0;
    bool lossless = false;

    napi_status status = napi_get_value_bigint_int64(env, value, &result, &lossless);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetValueBigintInt64Test002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    int64_t testValue = INT64_MAX;
    napi_value value = nullptr;
    napi_create_bigint_int64(env, testValue, &value);
    int64_t* result = nullptr;
    bool lossless = false;

    napi_status status = napi_get_value_bigint_int64(env, value, result, &lossless);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetValueBigintInt64Test003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    int64_t testValue = INT64_MAX;
    napi_value value = nullptr;
    napi_create_bigint_int64(env, testValue, &value);
    int64_t result = 0;
    bool* lossless = nullptr;

    napi_status status = napi_get_value_bigint_int64(env, value, &result, lossless);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetValueBigintInt64Test004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    napi_create_object(env, &value);
    int64_t result = 0;
    bool lossless = false;

    napi_status status = napi_get_value_bigint_int64(env, value, &result, &lossless);
    ASSERT_EQ(status, napi_bigint_expected);
}

HWTEST_F(NapiBasicTest, NapiGetValueBigintInt64Test005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    uint64_t testValue = UINT64_MAX;
    napi_value value = nullptr;
    napi_create_bigint_uint64(env, testValue, &value);
    int64_t result = 0;
    bool lossless = false;

    napi_status status = napi_get_value_bigint_int64(env, value, &result, &lossless);
    ASSERT_EQ(status, napi_ok);
    ASSERT_EQ(lossless, false);
}

HWTEST_F(NapiBasicTest, NapiGetValueBigintUint64Test001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    uint64_t result = 0;
    bool lossless = false;

    napi_status status = napi_get_value_bigint_uint64(env, value, &result, &lossless);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetValueBigintUint64Test002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    uint64_t testValue = UINT64_MAX;
    napi_value value = nullptr;
    napi_create_bigint_uint64(env, testValue, &value);
    uint64_t* result = nullptr;
    bool lossless = false;

    napi_status status = napi_get_value_bigint_uint64(env, value, result, &lossless);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetValueBigintUint64Test003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    uint64_t testValue = UINT64_MAX;
    napi_value value = nullptr;
    napi_create_bigint_uint64(env, testValue, &value);
    uint64_t result = 0;
    bool* lossless = nullptr;

    napi_status status = napi_get_value_bigint_uint64(env, value, &result, lossless);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetValueBigintUint64Test004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    napi_create_object(env, &value);
    uint64_t result = 0;
    bool lossless = false;

    napi_status status = napi_get_value_bigint_uint64(env, value, &result, &lossless);
    ASSERT_EQ(status, napi_bigint_expected);
}

#ifdef PANDA_JS_ETS_HYBRID_MODE_TEST
HWTEST_F(NapiBasicTest, NapiIsAliveObjectTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_ref result = nullptr;
    bool res = false;

    napi_create_object(env, &obj);
    napi_status status =
        napi_wrap_with_xref(env, obj, (void*)TEST_STRING, [](napi_env, void* data, void* hint) {}, &result);
    ASSERT_EQ(status, napi_ok);
    status = napi_is_alive_object(env, result, &res);
    ASSERT_EQ(status, napi_ok);
}

HWTEST_F(NapiBasicTest, NapiIsValidHeapObjectTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_ref result = nullptr;
    bool res = false;

    napi_create_object(env, &obj);
    napi_status status =
        napi_wrap_with_xref(env, obj, (void*)TEST_STRING, [](napi_env, void* data, void* hint) {}, &result);
    ASSERT_EQ(status, napi_ok);
    status = napi_is_contain_object(env, result, &res);
    ASSERT_EQ(status, napi_ok);
}

HWTEST_F(NapiBasicTest, NapiIsXrefTypeTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    bool res = false;

    napi_create_object(env, &obj);
    napi_status status = napi_is_xref_type(env, obj, &res);
    ASSERT_EQ(status, napi_ok);
}
#endif

HWTEST_F(NapiBasicTest, NapiGetValueBigintWordsTest001, testing::ext::TestSize.Level1)
{
    uint64_t wordsOut[] = {0ULL, 0ULL, 0ULL, 0ULL};
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;

    int retSignBit = -1;
    size_t retWordCount = 4;
    napi_status status = napi_get_value_bigint_words(env, value, &retSignBit, &retWordCount, wordsOut);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetValueBigintWordsTest002, testing::ext::TestSize.Level1)
{
    int signBit = 0;
    size_t wordCount = 4;
    uint64_t words[] = {0ULL, 34ULL, 56ULL, 2ULL};
    uint64_t wordsOut[] = {0ULL, 0ULL, 0ULL, 0ULL};
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    napi_status status = napi_create_bigint_words(env, signBit, wordCount, words, &value);
    ASSERT_EQ(status, napi_ok);

    int retSignBit = -1;
    size_t* retWordCount = nullptr;
    status = napi_get_value_bigint_words(env, value, &retSignBit, retWordCount, wordsOut);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateBufferTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const unsigned int bufferSize = sizeof(TEST_STRING);
    char** data = nullptr;
    napi_value result;
    napi_status status = napi_create_buffer(env, bufferSize, (void**)(data), &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateBufferTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const unsigned int bufferSize = sizeof(TEST_STRING);
    char* data;
    napi_value* result = nullptr;
    napi_status status = napi_create_buffer(env, bufferSize, (void**)(&data), result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateBufferTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const unsigned int bufferSize = 0;
    char* data;
    napi_value result;
    napi_status status = napi_create_buffer(env, bufferSize, (void**)(&data), &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateBufferTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const unsigned int bufferSize = MAX_BYTE_LENGTH + 1;
    char* data;
    napi_value result;
    napi_status status = napi_create_buffer(env, bufferSize, (void**)(&data), &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateBufferCopyTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const unsigned int bufferSize = 0;
    char* data;
    napi_value result;
    napi_status status = napi_create_buffer_copy(env, bufferSize, TEST_STRING, (void**)(&data), &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateBufferCopyTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const unsigned int bufferSize = MAX_BYTE_LENGTH + 1;
    char* data;
    napi_value result;
    napi_status status = napi_create_buffer_copy(env, bufferSize, TEST_STRING, (void**)(&data), &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateBufferCopyTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const unsigned int bufferSize = sizeof(TEST_STRING);
    char* data;
    napi_value result;
    napi_status status = napi_create_buffer_copy(env, bufferSize, nullptr, (void**)(&data), &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateBufferCopyTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const unsigned int bufferSize = sizeof(TEST_STRING);
    char** data = nullptr;
    napi_value result;
    napi_status status = napi_create_buffer_copy(env, bufferSize, TEST_STRING, (void**)(data), &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateBufferCopyTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const unsigned int bufferSize = sizeof(TEST_STRING);
    char* data;
    napi_value* result = nullptr;
    napi_status status = napi_create_buffer_copy(env, bufferSize, TEST_STRING, (void**)(&data), result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateExternalBufferTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const unsigned int bufferSize = 0;
    char* copyPtr = strdup(TEST_STRING);
    napi_value result;
    napi_status status = napi_create_external_buffer(env, bufferSize, copyPtr,
        [](napi_env env, void* data, void* hint) { return; },
        nullptr, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateExternalBufferTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const unsigned int bufferSize = MAX_BYTE_LENGTH + 1;
    char* copyPtr = strdup(TEST_STRING);
    napi_value result;
    napi_status status = napi_create_external_buffer(env, bufferSize, copyPtr,
        [](napi_env env, void* data, void* hint) { return; },
        nullptr, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateExternalBufferTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const unsigned int bufferSize = sizeof(TEST_STRING);
    char* copyPtr = nullptr;
    napi_value result;
    napi_status status = napi_create_external_buffer(env, bufferSize, copyPtr,
        [](napi_env env, void* data, void* hint) { return; },
        nullptr, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateExternalBufferTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const unsigned int bufferSize = sizeof(TEST_STRING);
    char* copyPtr = strdup(TEST_STRING);
    napi_value* result = nullptr;
    napi_status status = napi_create_external_buffer(env, bufferSize, copyPtr,
        [](napi_env env, void* data, void* hint) { return; },
        nullptr, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetBufferInfoTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    char *data;
    size_t length;

    napi_status status = napi_get_buffer_info(env, value, (void**)&data, &length);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiGetBufferInfoTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    char *data;
    size_t length;

    napi_create_double(env, TEST_DOUBLE, &value);
    napi_status status = napi_get_buffer_info(env, value, (void**)&data, &length);
    ASSERT_EQ(status, napi_arraybuffer_expected);
}

HWTEST_F(NapiBasicTest, NapiIsBufferTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    bool result;

    napi_status status = napi_is_buffer(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiIsBufferTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    bool* result = nullptr;

    napi_create_object(env, &value);
    napi_status status = napi_is_buffer(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiDeserializeTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    void* buffer = nullptr;
    napi_value result = nullptr;

    napi_status status = napi_deserialize(env, buffer, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiDeserializeTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    int buffer = 0;
    napi_value* result = nullptr;

    napi_status status = napi_deserialize(env, (void*)&buffer, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiDeleteSerializationDataTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    void* buffer = nullptr;

    napi_status status = napi_delete_serialization_data(env, buffer);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCallThreadsafeFunctionWithPriorityTest001, testing::ext::TestSize.Level1)
{
    napi_threadsafe_function func = nullptr;

    napi_status status = napi_call_threadsafe_function_with_priority(func, nullptr, napi_priority_idle, true);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiIsSendableTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    bool result;

    napi_status status = napi_is_sendable(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiIsSendableTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    bool* result = nullptr;

    napi_create_object(env, &value);
    napi_status status = napi_is_sendable(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiDefineSendableClassTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value testClass = nullptr;
    napi_status status = napi_define_sendable_class(
        env, nullptr, NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);

            return thisVar;
        },
        nullptr, 0, nullptr, nullptr, &testClass);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiDefineSendableClassTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value testClass = nullptr;
    napi_status status = napi_define_sendable_class(
        env, "TestClass", NAPI_AUTO_LENGTH,
        nullptr, nullptr, 0, nullptr, nullptr, &testClass);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiDefineSendableClassTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value testClass = nullptr;
    napi_status status = napi_define_sendable_class(
        env, "TestClass", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);

            return thisVar;
        },
        nullptr, 1, nullptr, nullptr, &testClass);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiDefineSendableClassTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value* testClass = nullptr;
    napi_status status = napi_define_sendable_class(
        env, "TestClass", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);

            return thisVar;
        },
        nullptr, 0, nullptr, nullptr, testClass);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateSendableObjectWithPropertiesTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value val_true;
    napi_get_boolean(env, true, &val_true);
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_DEFAULT_PROPERTY("x", val_true),
    };
    napi_value* result = nullptr;

    napi_status status = napi_create_sendable_object_with_properties(env, 1, desc, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateSendableArrayTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value* result = nullptr;

    napi_status status = napi_create_sendable_array(env, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateSendableArrayWithLengthTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    size_t length = INT_ONE;
    napi_value* result = nullptr;

    napi_status status = napi_create_sendable_array_with_length(env, length, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateSendableArrayWithLengthTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    size_t length = INT_THREE;
    napi_value array = nullptr;

    napi_status status = napi_create_sendable_array_with_length(env, length, &array);
    uint32_t result = 0;
    napi_get_array_length(env, array, &result);
    ASSERT_EQ(result, INT_THREE);
    ASSERT_EQ(status, napi_ok);
}

HWTEST_F(NapiBasicTest, NapiCreateSendableArraybufferTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    size_t length = INT_ONE;
    void** data = nullptr;
    napi_value result;

    napi_status status = napi_create_sendable_arraybuffer(env, length, data, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateSendableArraybufferTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    size_t length = INT_ONE;
    void* data;
    napi_value* result = nullptr;

    napi_status status = napi_create_sendable_arraybuffer(env, length, &data, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateSendableTypedarrayTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value arraybuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    size_t arrayBufferSize = 16;
    size_t typedArrayLength = 4;
    napi_status status = napi_create_sendable_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arraybuffer);
    ASSERT_EQ(status, napi_ok);

    napi_value* result = nullptr;
    status = napi_create_sendable_typedarray(env, napi_int32_array, typedArrayLength, arraybuffer, 0, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiCreateSendableTypedarrayTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value arraybuffer = nullptr;
    size_t typedArrayLength = 4;

    napi_value result;
    napi_status status = napi_create_sendable_typedarray(env, napi_int32_array, typedArrayLength,
        arraybuffer, 0, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiWrapSendableTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value js_obj = nullptr;

    napi_status status = napi_wrap_sendable(
        env, js_obj, (void*)TEST_STRING, [](napi_env env, void* data, void* hint) {}, nullptr);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiWrapSendableTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value js_obj = nullptr;

    napi_status status = napi_create_object(env, &js_obj);
    status = napi_wrap_sendable(
        env, js_obj, nullptr, [](napi_env env, void* data, void* hint) {}, nullptr);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiWrapSendableWithSizeTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value js_obj = nullptr;

    napi_status status = napi_wrap_sendable_with_size(
        env, js_obj, (void*)TEST_STRING, [](napi_env env, void* data, void* hint) {}, nullptr, INT_ONE);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiWrapSendableWithSizeTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value js_obj = nullptr;

    napi_status status = napi_create_object(env, &js_obj);
    status = napi_wrap_sendable_with_size(
        env, js_obj, nullptr, [](napi_env env, void* data, void* hint) {}, nullptr, INT_ONE);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiUnwrapSendableTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value js_obj = nullptr;
    void* result;

    napi_status status = napi_unwrap_sendable(env, js_obj, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiUnwrapSendableTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value js_obj;
    void** result = nullptr;

    napi_status status = napi_create_object(env, &js_obj);
    status = napi_unwrap_sendable(env, js_obj, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiRemoveWrapSendableTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value js_obj = nullptr;
    void* result;

    napi_status status = napi_remove_wrap_sendable(env, js_obj, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiBasicTest, NapiRemoveWrapSendableTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value js_obj;
    void** result = nullptr;

    napi_status status = napi_create_object(env, &js_obj);
    status = napi_remove_wrap_sendable(env, js_obj, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

/**
 * @tc.name: NapiModuleRegisterTest
 * @tc.desc: Test interface of napi_module_register
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiModuleRegisterTest001, testing::ext::TestSize.Level1)
{
    // call napi_module_register interface with nullptr error
    LoggerCollector collector;
    napi_module_register(nullptr);
    ASSERT_TRUE(collector.Includes("mod is nullptr"));
}

/**
 * @tc.name: NapiGetLastErrorInfoTest
 * @tc.desc: Test interface of napi_get_last_error_info
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetLastErrorInfoTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    // call napi_get_last_error_info interface with nullptr error
    auto res = napi_get_last_error_info(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiThrowTest
 * @tc.desc: Test interface of napi_throw
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiThrowTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    // call napi_throw interface with nullptr error
    auto res = napi_throw(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiThrowErrorTest
 * @tc.desc: Test interface of napi_throw_error
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiThrowErrorTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    // call napi_throw_error interface with nullptr msg
    auto res = napi_throw_error(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiThrowTypeErrorTest
 * @tc.desc: Test interface of napi_throw_type_error
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiThrowTypeErrorTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    // call napi_throw_type_error interface with nullptr msg
    auto res = napi_throw_type_error(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiThrowRangeErrorTest
 * @tc.desc: Test interface of napi_throw_range_error
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiThrowRangeErrorTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    // call napi_throw_range_error interface with nullptr msg
    auto res = napi_throw_range_error(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiIsErrorTest
 * @tc.desc: Test interface of napi_is_error
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiIsErrorTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value code = nullptr;
    napi_value message = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_CHAR_STRING, NAPI_AUTO_LENGTH, &code));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_CHAR_STRING, NAPI_AUTO_LENGTH, &message));

    napi_value error = nullptr;
    ASSERT_CHECK_CALL(napi_create_error(env, code, message, &error));
    ASSERT_TRUE(error != nullptr);

    bool isError = false;
    // call napi_is_error interface with correct input
    auto res = napi_is_error(env, error, &isError);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiIsErrorTest
 * @tc.desc: Test interface of napi_is_error
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiIsErrorTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    // call napi_is_error interface with nullptr value and nullptr result
    auto res = napi_is_error(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiIsErrorTest
 * @tc.desc: Test interface of napi_is_error
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiIsErrorTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    bool isError = false;
    // call napi_is_error interface with nullptr value
    auto res = napi_is_error(env, nullptr, &isError);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiIsErrorTest
 * @tc.desc: Test interface of napi_is_error
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiIsErrorTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value code = nullptr;
    napi_value message = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_CHAR_STRING, NAPI_AUTO_LENGTH, &code));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_CHAR_STRING, NAPI_AUTO_LENGTH, &message));

    napi_value error = nullptr;
    ASSERT_CHECK_CALL(napi_create_error(env, code, message, &error));
    ASSERT_TRUE(error != nullptr);

    // call napi_is_error interface with nullptr result
    auto res = napi_is_error(env, error, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateErrorTest
 * @tc.desc: Test interface of napi_create_error
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateErrorTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value code = nullptr;
    napi_value message = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_CHAR_STRING, NAPI_AUTO_LENGTH, &code));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_CHAR_STRING, NAPI_AUTO_LENGTH, &message));

    napi_value error = nullptr;
    auto res = napi_create_error(env, code, message, &error);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCreateErrorTest
 * @tc.desc: Test interface of napi_create_error
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateErrorTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value message = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, TEST_INT32_500, &message));

    napi_value error = nullptr;
    auto res = napi_create_error(env, nullptr, message, &error);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateErrorTest
 * @tc.desc: Test interface of napi_create_error
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateErrorTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value code = nullptr;
    napi_value message = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &code));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_CHAR_STRING, NAPI_AUTO_LENGTH, &message));

    napi_value error = nullptr;
    auto res = napi_create_error(env, code, message, &error);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateErrorTest
 * @tc.desc: Test interface of napi_create_error
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateErrorTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_error(env, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateTypeErrorTest
 * @tc.desc: Test interface of napi_create_type_error
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateTypeErrorTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value code = nullptr;
    napi_value message = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_CHAR_STRING, NAPI_AUTO_LENGTH, &code));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_CHAR_STRING, NAPI_AUTO_LENGTH, &message));

    napi_value error = nullptr;
    auto res = napi_create_type_error(env, code, message, &error);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCreateTypeErrorTest
 * @tc.desc: Test interface of napi_create_type_error
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateTypeErrorTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value message = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, TEST_INT32_500, &message));

    napi_value error = nullptr;
    auto res = napi_create_type_error(env, nullptr, message, &error);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateTypeErrorTest
 * @tc.desc: Test interface of napi_create_type_error
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateTypeErrorTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value code = nullptr;
    napi_value message = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &code));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_CHAR_STRING, NAPI_AUTO_LENGTH, &message));

    napi_value error = nullptr;
    auto res = napi_create_type_error(env, code, message, &error);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateTypeErrorTest
 * @tc.desc: Test interface of napi_create_type_error
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateTypeErrorTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_type_error(env, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateRangeErrorTest
 * @tc.desc: Test interface of napi_create_range_error
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateRangeErrorTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value code = nullptr;
    napi_value message = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_CHAR_STRING, NAPI_AUTO_LENGTH, &code));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_CHAR_STRING, NAPI_AUTO_LENGTH, &message));

    napi_value error = nullptr;
    auto res = napi_create_range_error(env, code, message, &error);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCreateRangeErrorTest
 * @tc.desc: Test interface of napi_create_range_error
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateRangeErrorTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value message = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, TEST_INT32_500, &message));

    napi_value error = nullptr;
    auto res = napi_create_range_error(env, nullptr, message, &error);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateRangeErrorTest
 * @tc.desc: Test interface of napi_create_range_error
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateRangeErrorTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value code = nullptr;
    napi_value message = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &code));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_CHAR_STRING, NAPI_AUTO_LENGTH, &message));

    napi_value error = nullptr;
    auto res = napi_create_range_error(env, code, message, &error);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateRangeErrorTest
 * @tc.desc: Test interface of napi_create_range_error
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateRangeErrorTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_range_error(env, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetAndClearLastExceptionTest
 * @tc.desc: Test interface of napi_get_and_clear_last_exception
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetAndClearLastExceptionTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    auto res = napi_get_and_clear_last_exception(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiIsExceptionPendingTest
 * @tc.desc: Test interface of napi_is_exception_pending
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiIsExceptionPendingTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_is_exception_pending(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiOpenHandleScopeTest
 * @tc.desc: Test interface of napi_open_handle_scope
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiOpenHandleScopeTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_open_handle_scope(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCloseHandleScopeTest
 * @tc.desc: Test interface of napi_close_handle_scope
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCloseHandleScopeTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_close_handle_scope(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCloseHandleScopeTest
 * @tc.desc: Test interface of napi_close_handle_scope
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCloseHandleScopeTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_handle_scope scope = nullptr;
    ASSERT_CHECK_CALL(napi_open_handle_scope(env, &scope));
    auto res = napi_close_handle_scope(env, scope);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCloseHandleScopeTest
 * @tc.desc: Test interface of napi_close_handle_scope
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCloseHandleScopeTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_handle_scope scope = reinterpret_cast<napi_handle_scope>(scope_);

    auto tempScope = engine_->openHandleScopes_;
    engine_->openHandleScopes_ = 0;
    auto res = napi_close_handle_scope(env, scope);
    engine_->openHandleScopes_ = tempScope;
    ASSERT_EQ(res, napi_handle_scope_mismatch);
}

/**
 * @tc.name: NapiOpenEscapableHandleScopeTest
 * @tc.desc: Test interface of napi_open_escapable_handle_scope
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiOpenEscapableHandleScopeTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_open_escapable_handle_scope(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCloseEscapableHandleScopeTest
 * @tc.desc: Test interface of napi_close_escapable_handle_scope
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCloseEscapableHandleScopeTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_close_escapable_handle_scope(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCloseEscapableHandleScopeTest
 * @tc.desc: Test interface of napi_close_escapable_handle_scope
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCloseEscapableHandleScopeTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_escapable_handle_scope scope = nullptr;
    ASSERT_CHECK_CALL(napi_open_escapable_handle_scope(env, &scope));

    auto res = napi_close_escapable_handle_scope(env, scope);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCloseEscapableHandleScopeTest
 * @tc.desc: Test interface of napi_close_escapable_handle_scope
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCloseEscapableHandleScopeTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_handle_scope scope = reinterpret_cast<napi_handle_scope>(scope_);

    auto tempScope = engine_->openHandleScopes_;
    engine_->openHandleScopes_ = 0;
    auto res = napi_close_escapable_handle_scope(env, (napi_escapable_handle_scope)scope);
    engine_->openHandleScopes_ = tempScope;
    ASSERT_EQ(res, napi_handle_scope_mismatch);
}

/**
 * @tc.name: NapiEscapeHandleTest
 * @tc.desc: Test interface of napi_escape_handle
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiEscapeHandleTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_escapable_handle_scope escapableScope = nullptr;
    ASSERT_CHECK_CALL(napi_open_escapable_handle_scope(env, &escapableScope));
    napi_value boolean, booleanNew = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));

    auto res = napi_escape_handle(env, escapableScope, boolean, &booleanNew);
    ASSERT_EQ(res, napi_ok);

    ASSERT_CHECK_CALL(napi_close_escapable_handle_scope(env, escapableScope));
}

/**
 * @tc.name: NapiEscapeHandleTest
 * @tc.desc: Test interface of napi_escape_handle
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiEscapeHandleTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_escapable_handle_scope escapableScope = nullptr;
    ASSERT_CHECK_CALL(napi_open_escapable_handle_scope(env, &escapableScope));
    napi_value boolean, booleanNew = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    ASSERT_CHECK_CALL(napi_escape_handle(env, escapableScope, boolean, &booleanNew));

    auto res = napi_escape_handle(env, escapableScope, boolean, &booleanNew);
    ASSERT_EQ(res, napi_escape_called_twice);

    ASSERT_CHECK_CALL(napi_close_escapable_handle_scope(env, escapableScope));
}

/**
 * @tc.name: NapiEscapeHandleTest
 * @tc.desc: Test interface of napi_escape_handle
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiEscapeHandleTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_escapable_handle_scope escapableScope = nullptr;
    ASSERT_CHECK_CALL(napi_open_escapable_handle_scope(env, &escapableScope));
    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));

    auto res = napi_escape_handle(env, escapableScope, boolean, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);

    ASSERT_CHECK_CALL(napi_close_escapable_handle_scope(env, escapableScope));
}

/**
 * @tc.name: NapiEscapeHandleTest
 * @tc.desc: Test interface of napi_escape_handle
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiEscapeHandleTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_escapable_handle_scope escapableScope = nullptr;
    ASSERT_CHECK_CALL(napi_open_escapable_handle_scope(env, &escapableScope));

    auto res = napi_escape_handle(env, escapableScope, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);

    ASSERT_CHECK_CALL(napi_close_escapable_handle_scope(env, escapableScope));
}

/**
 * @tc.name: NapiEscapeHandleTest
 * @tc.desc: Test interface of napi_escape_handle
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiEscapeHandleTest005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_escape_handle(env, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateReferenceTest
 * @tc.desc: Test interface of napi_create_reference
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateReferenceTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    napi_ref booleanRef = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));

    auto res = napi_create_reference(env, boolean, 1, &booleanRef);
    ASSERT_EQ(res, napi_ok);
    ASSERT_CHECK_CALL(napi_delete_reference(env, booleanRef));
}

/**
 * @tc.name: NapiCreateReferenceTest
 * @tc.desc: Test interface of napi_create_reference
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateReferenceTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));

    auto res = napi_create_reference(env, boolean, 1, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateReferenceTest
 * @tc.desc: Test interface of napi_create_reference
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateReferenceTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_reference(env, nullptr, 1, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateReferenceTest
 * @tc.desc: Test interface of napi_create_reference
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateReferenceTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    napi_ref booleanRef = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));

    auto res = napi_create_reference(env, boolean, 1, &booleanRef);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiDeleteReferenceTest
 * @tc.desc: Test interface of napi_delete_reference
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiDeleteReferenceTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_delete_reference(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiDeleteReferenceTest
 * @tc.desc: Test interface of napi_delete_reference
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiDeleteReferenceTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));

    napi_ref booleanRef = nullptr;
    ASSERT_CHECK_CALL(napi_create_reference(env, boolean, 1, &booleanRef));
    auto res = napi_delete_reference(env, booleanRef);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiReferenceRefTest
 * @tc.desc: Test interface of napi_reference_ref
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiReferenceRefTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_reference_ref(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiReferenceUnrefTest
 * @tc.desc: Test interface of napi_reference_unref
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiReferenceUnrefTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_reference_unref(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetReferenceValueTest
 * @tc.desc: Test interface of napi_get_reference_value
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetReferenceValueTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_get_reference_value(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetReferenceValueTest
 * @tc.desc: Test interface of napi_get_reference_value
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetReferenceValueTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));

    napi_ref booleanRef = nullptr;
    ASSERT_CHECK_CALL(napi_create_reference(env, boolean, 1, &booleanRef));

    auto res = napi_get_reference_value(env, booleanRef, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetReferenceValueTest
 * @tc.desc: Test interface of napi_get_reference_value
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetReferenceValueTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));

    napi_ref booleanRef = nullptr;
    ASSERT_CHECK_CALL(napi_create_reference(env, boolean, 1, &booleanRef));

    napi_value refValue = nullptr;
    auto res = napi_get_reference_value(env, booleanRef, &refValue);
    ASSERT_EQ(res, napi_ok);
    ASSERT_NE(refValue, nullptr);
}

/**
 * @tc.name: NapiCreateArrayTest
 * @tc.desc: Test interface of napi_create_array
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateArrayTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value array = nullptr;
    auto res = napi_create_array(env, &array);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCreateArrayTest
 * @tc.desc: Test interface of napi_create_array
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateArrayTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_array(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateArrayWithLengthTest
 * @tc.desc: Test interface of napi_create_array_with_length
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateArrayWithLengthTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value array = nullptr;
    auto res = napi_create_array_with_length(env, 0, &array);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCreateArrayWithLengthTest
 * @tc.desc: Test interface of napi_create_array_with_length
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateArrayWithLengthTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_array_with_length(env, 0, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateArrayWithLengthTest
 * @tc.desc: Test interface of napi_create_array_with_length
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateArrayWithLengthTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value array = nullptr;
    auto res = napi_create_array_with_length(env, INT_THREE, &array);
    uint32_t result = 0;
    napi_get_array_length(env, array, &result);
    ASSERT_EQ(result, INT_THREE);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCreateArraybufferTest
 * @tc.desc: Test interface of napi_create_arraybuffer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateArraybufferTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    size_t arrayBufferSize = 0;
    auto res = napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCreateArraybufferTest
 * @tc.desc: Test interface of napi_create_arraybuffer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateArraybufferTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    size_t arrayBufferSize = 0;
    auto res = napi_create_arraybuffer(env, arrayBufferSize, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateArraybufferTest
 * @tc.desc: Test interface of napi_create_arraybuffer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateArraybufferTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    void* arrayBufferPtr = nullptr;
    size_t arrayBufferSize = 0;
    auto res = napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateExternalTest
 * @tc.desc: Test interface of napi_create_external
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateExternalTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_external(
        env, (void*)TEST_CHAR_STRING,
        [](napi_env env, void* data, void* hint) { ASSERT_STREQ((const char*)data, (const char*)hint); },
        (void*)TEST_CHAR_STRING, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateExternalTest
 * @tc.desc: Test interface of napi_create_external
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateExternalTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value external = nullptr;
    auto res = napi_create_external(
        env, (void*)TEST_CHAR_STRING,
        [](napi_env env, void* data, void* hint) { ASSERT_STREQ((const char*)data, (const char*)hint); },
        (void*)TEST_CHAR_STRING, &external);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCreateExternalArraybufferTest
 * @tc.desc: Test interface of napi_create_external_arraybuffer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateExternalArraybufferTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_external_arraybuffer(
        env, nullptr, strlen(TEST_CHAR_STRING),
        nullptr,
        (void*)TEST_CHAR_STRING, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateExternalArraybufferTest
 * @tc.desc: Test interface of napi_create_external_arraybuffer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateExternalArraybufferTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_external_arraybuffer(
        env, (void*)TEST_CHAR_STRING, strlen(TEST_CHAR_STRING),
        nullptr,
        (void*)TEST_CHAR_STRING, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateExternalArraybufferTest
 * @tc.desc: Test interface of napi_create_external_arraybuffer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateExternalArraybufferTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_external_arraybuffer(
        env, (void*)TEST_CHAR_STRING, strlen(TEST_CHAR_STRING),
        [](napi_env env, void* data, void* hint) { ASSERT_STREQ((const char*)data, (const char*)hint); },
        (void*)TEST_CHAR_STRING, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateExternalArraybufferTest
 * @tc.desc: Test interface of napi_create_external_arraybuffer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateExternalArraybufferTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value external = nullptr;
    auto res = napi_create_external_arraybuffer(
        env, (void*)TEST_CHAR_STRING, strlen(TEST_CHAR_STRING),
        [](napi_env env, void* data, void* hint) { ASSERT_STREQ((const char*)data, (const char*)hint); },
        (void*)TEST_CHAR_STRING, &external);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCreateObjectTest
 * @tc.desc: Test interface of napi_create_object
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateObjectTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_object(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &result));
}

/**
 * @tc.name: NapiCreateSymbolTest
 * @tc.desc: Test interface of napi_create_symbol
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateSymbolTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_symbol(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateSymbolTest
 * @tc.desc: Test interface of napi_create_symbol
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateSymbolTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));

    napi_value result = nullptr;
    auto res = napi_create_symbol(env, boolean, &result);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateTypedarrayTest
 * @tc.desc: Test interface of napi_create_typedarray
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateTypedarrayTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_typedarray(env, napi_int8_array, 0, nullptr, 0, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);

    napi_value arraybuffer = nullptr;
    res = napi_create_typedarray(env, napi_int8_array, 0, arraybuffer, 0, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateTypedarrayTest
 * @tc.desc: Test interface of napi_create_typedarray
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateTypedarrayTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    napi_value typedarray = nullptr;
    auto res = napi_create_typedarray(env, napi_int8_array, 0, boolean, 0, &typedarray);
    ASSERT_EQ(res, napi_arraybuffer_expected);
}

/**
 * @tc.name: NapiCreateTypedarrayTest
 * @tc.desc: Test interface of napi_create_typedarray
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateTypedarrayTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    size_t arrayBufferSize = 1024;
    ASSERT_CHECK_CALL(napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer));

    napi_value typedarray = nullptr;
    auto res = napi_create_typedarray(env, (napi_typedarray_type)(napi_int8_array - 1), arrayBufferSize,
        arrayBuffer, 0, &typedarray);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateTypedarrayTest
 * @tc.desc: Test interface of napi_create_typedarray
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateTypedarrayTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    size_t arrayBufferSize = 1024;
    ASSERT_CHECK_CALL(napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer));

    napi_value typedarray = nullptr;
    auto res = napi_create_typedarray(env, (napi_typedarray_type)(napi_biguint64_array + 1), arrayBufferSize,
        arrayBuffer, 0, &typedarray);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateDataviewTest
 * @tc.desc: Test interface of napi_create_dataview
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateDataviewTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_dataview(env, 0, nullptr, 0, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);

    napi_value arraybuffer = nullptr;
    res = napi_create_dataview(env, 0, arraybuffer, 0, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateDataviewTest
 * @tc.desc: Test interface of napi_create_dataview
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateDataviewTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    napi_value result = nullptr;
    auto res = napi_create_dataview(env, 0, boolean, 0, &result);
    ASSERT_EQ(res, napi_arraybuffer_expected);
}

/**
 * @tc.name: NapiCreateDataviewTest
 * @tc.desc: Test interface of napi_create_dataview
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateDataviewTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    size_t arrayBufferSize = 1024;
    napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);
    ASSERT_NE(arrayBuffer, nullptr);
    ASSERT_NE(arrayBufferPtr, nullptr);
    bool isArrayBuffer = false;
    napi_is_arraybuffer(env, arrayBuffer, &isArrayBuffer);
    ASSERT_TRUE(isArrayBuffer);

    napi_value result = nullptr;
    auto res = napi_create_dataview(env, arrayBufferSize, arrayBuffer, arrayBufferSize + 1, &result);
    ASSERT_EQ(res, napi_pending_exception);
}

/**
 * @tc.name: NapiCreateInt32Test
 * @tc.desc: Test interface of napi_create_int32
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateInt32Test001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_int32(env, TEST_INT32_MINUS_1, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateInt32Test
 * @tc.desc: Test interface of napi_create_int32
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateInt32Test002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value numberValue = nullptr;
    auto res = napi_create_int32(env, TEST_INT32_MINUS_1, &numberValue);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCreateUint32Test
 * @tc.desc: Test interface of napi_create_uint32
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateUint32Test001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_uint32(env, TEST_UINT32_1000, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateUint32Test
 * @tc.desc: Test interface of napi_create_uint32
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateUint32Test002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value numberValue = nullptr;
    auto res = napi_create_uint32(env, TEST_UINT32_1000, &numberValue);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCreateInt64Test
 * @tc.desc: Test interface of napi_create_int64
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateInt64Test001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_status res = napi_create_int64(env, MAX_SAFE_INTEGER, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateInt64Test
 * @tc.desc: Test interface of napi_create_int64
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateInt64Test002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value numberValue = nullptr;
    napi_status res = napi_create_int64(env, MAX_SAFE_INTEGER, &numberValue);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCreateDoubleTest
 * @tc.desc: Test interface of napi_create_double
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateDoubleTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_double(env, TEST_DOUBLE, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateDoubleTest
 * @tc.desc: Test interface of napi_create_double
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateDoubleTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value numberValue = nullptr;
    auto res = napi_create_double(env, TEST_DOUBLE, &numberValue);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCreateStringLatin1Test
 * @tc.desc: Test interface of napi_create_string_latin1
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateStringLatin1Test001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_string_latin1(env, nullptr, NAPI_AUTO_LENGTH, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateStringLatin1Test
 * @tc.desc: Test interface of napi_create_string_latin1
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateStringLatin1Test002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_string_latin1(env, TEST_CHAR_STRING, NAPI_AUTO_LENGTH, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateStringUtf8Test
 * @tc.desc: Test interface of napi_create_string_utf8
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateStringUtf8Test001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_string_utf8(env, nullptr, NAPI_AUTO_LENGTH, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateStringUtf8Test
 * @tc.desc: Test interface of napi_create_string_utf8
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateStringUtf8Test002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_string_utf8(env, TEST_CHAR_STRING, NAPI_AUTO_LENGTH, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateStringUtf8Test003
 * @tc.desc: Test napi_create_string_utf8 with a valid string and auto length
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateStringUtf8Test003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    const char* testString = TEST_CHAR_STRING;
    napi_value result = nullptr;
    napi_status res = napi_create_string_utf8(env, testString, NAPI_AUTO_LENGTH, &result);
    ASSERT_EQ(res, napi_ok);

    size_t strLength = strlen(testString);
    char* buffer = new char[strLength + 1];
    size_t resultLength = 0;
    res = napi_get_value_string_utf8(env, result, buffer, strLength + 1, &resultLength);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(resultLength, strLength);
    ASSERT_STREQ(buffer, testString);

    delete[] buffer;
}

/**
 * @tc.name: NapiCreateStringUtf8Test004
 * @tc.desc: Test napi_create_string_utf8 with a valid string and explicit length
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateStringUtf8Test004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    const char* testString = TEST_STRING;
    size_t strLength = strlen(testString);
    napi_value result;
    napi_status res = napi_create_string_utf8(env, testString, strLength, &result);
    ASSERT_EQ(res, napi_ok);

    char* buffer = new char[strLength + 1];
    size_t resultLength = 0;
    res = napi_get_value_string_utf8(env, result, buffer, strLength + 1, &resultLength);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(resultLength, strLength);
    ASSERT_STREQ(buffer, testString);
}

/**
 * @tc.name: NapiCreateStringUtf16Test
 * @tc.desc: Test interface of napi_create_string_utf16
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateStringUtf16Test001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_string_utf16(env, nullptr, NAPI_AUTO_LENGTH, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateStringUtf16Test
 * @tc.desc: Test interface of napi_create_string_utf16
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateStringUtf16Test002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_string_utf16(env, TEST_CHAR16_STRING, NAPI_AUTO_LENGTH, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateStringUtf16Test
 * @tc.desc: Test interface of napi_create_string_utf16
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateStringUtf16Test003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value stringValue = nullptr;
    auto res = napi_create_string_utf16(env, TEST_CHAR16_STRING, (NAPI_AUTO_LENGTH - 1), &stringValue);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetArrayLengthTest
 * @tc.desc: Test interface of napi_get_array_length
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetArrayLengthTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_get_array_length(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetArrayLengthTest
 * @tc.desc: Test interface of napi_get_array_length
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetArrayLengthTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    uint32_t arrayLength = 0;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    auto res = napi_get_array_length(env, boolean, &arrayLength);
    ASSERT_EQ(res, napi_array_expected);
}

/**
 * @tc.name: NapiGetArrayLengthTest
 * @tc.desc: Test interface of napi_get_array_length
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetArrayLengthTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value arr = nullptr;
    uint32_t arrayLength = TEST_UINT32_1000;
    ASSERT_CHECK_CALL(napi_create_array_with_length(env, INT_THREE, &arr));
    napi_status status = napi_get_array_length(env, arr, &arrayLength);
    ASSERT_EQ(status, napi_ok);
    ASSERT_EQ(arrayLength, INT_THREE);
}

/**
 * @tc.name: NapiGetArrayLengthTest
 * @tc.desc: Test interface of napi_get_array_length
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetArrayLengthTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value arr = nullptr;
    uint32_t arrayLength = TEST_UINT32_1000;
    ASSERT_CHECK_CALL(napi_create_array_with_length(env, INT_THREE, &arr));
    ASSERT_CHECK_CALL(napi_throw_error(env, TEST_CHAR_ERROR_CODE, TEST_CHAR_ERROR_MESSAGE));
    bool isPendingException = false;
    ASSERT_CHECK_CALL(napi_is_exception_pending(env, &isPendingException));
    ASSERT_EQ(isPendingException, true);
    napi_status status = napi_get_array_length(env, arr, &arrayLength);
    ASSERT_EQ(status, napi_pending_exception);
    ASSERT_EQ(arrayLength, 0);
}

/**
 * @tc.name: NapiGetArrayLengthTest
 * @tc.desc: Test interface of napi_get_array_length
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetArrayLengthTest005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value arr = nullptr;
    ASSERT_CHECK_CALL(napi_create_array_with_length(env, INT_THREE, &arr));

    // throw error by function-call
    napi_value funcValue = nullptr;
    ASSERT_CHECK_CALL(napi_create_function(env, TEST_CHAR_TEST_FUNC, NAPI_AUTO_LENGTH, TestCallFunc, nullptr, &funcValue));
    ASSERT_NE(funcValue, nullptr);
    napi_status status = napi_call_function(env, nullptr, funcValue, 0, nullptr, nullptr);
    ASSERT_EQ(status, napi_pending_exception);
    status = napi_ok; // reset status

    bool isPendingException = false;
    ASSERT_CHECK_CALL(napi_is_exception_pending(env, &isPendingException));
    ASSERT_EQ(isPendingException, true);

    uint32_t arrayLength = TEST_UINT32_1000;
    status = napi_get_array_length(env, arr, &arrayLength);
    ASSERT_EQ(status, napi_pending_exception);
    ASSERT_EQ(arrayLength, TEST_UINT32_1000);
}

/**
 * @tc.name: NapiGetArrayLengthTest
 * @tc.desc: Test interface of napi_get_array_length
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetArrayLengthTest006, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    ASSERT_EQ(engine_->lastException_.IsEmpty(), true);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value arr = nullptr;
    ASSERT_CHECK_CALL(napi_create_array_with_length(env, INT_THREE, &arr));
    ASSERT_CHECK_CALL(napi_throw_error(env, TEST_CHAR_ERROR_CODE, TEST_CHAR_ERROR_MESSAGE));
    bool isPendingException = false;
    ASSERT_CHECK_CALL(napi_is_exception_pending(env, &isPendingException));
    ASSERT_EQ(isPendingException, true);
    ASSERT_EQ(engine_->lastException_.IsEmpty(), true);
    napi_status status = napi_get_array_length(env, arr, nullptr);
    ASSERT_EQ(status, napi_invalid_arg);
    ASSERT_EQ(engine_->lastException_.IsEmpty(), false);
}

/**
 * @tc.name: NapiGetArrayLengthTest
 * @tc.desc: Test interface of napi_get_array_length
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetArrayLengthTest007, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    ASSERT_EQ(engine_->lastException_.IsEmpty(), true);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value arr = nullptr;
    ASSERT_CHECK_CALL(napi_create_array_with_length(env, INT_THREE, &arr));
    ASSERT_CHECK_CALL(napi_throw_error(env, TEST_CHAR_ERROR_CODE, TEST_CHAR_ERROR_MESSAGE));
    bool isPendingException = false;
    ASSERT_CHECK_CALL(napi_is_exception_pending(env, &isPendingException));
    ASSERT_EQ(isPendingException, true);
    ASSERT_EQ(engine_->lastException_.IsEmpty(), true);
    uint32_t arrayLength = TEST_UINT32_1000;
    napi_status status = napi_get_array_length(env, nullptr, &arrayLength);
    ASSERT_EQ(status, napi_invalid_arg);
    ASSERT_EQ(engine_->lastException_.IsEmpty(), false);
}

/**
 * @tc.name: NapiGetArrayLengthTest
 * @tc.desc: Test interface of napi_get_array_length
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetArrayLengthTest008, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    ASSERT_EQ(engine_->lastException_.IsEmpty(), true);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value noArr = nullptr;
    ASSERT_CHECK_CALL(napi_create_uint32(env, TEST_UINT32_1000, &noArr));
    ASSERT_CHECK_CALL(napi_throw_error(env, TEST_CHAR_ERROR_CODE, TEST_CHAR_ERROR_MESSAGE));
    bool isPendingException = false;
    ASSERT_CHECK_CALL(napi_is_exception_pending(env, &isPendingException));
    ASSERT_EQ(isPendingException, true);
    ASSERT_EQ(engine_->lastException_.IsEmpty(), true);
    uint32_t arrayLength = TEST_UINT32_1000;
    napi_status status = napi_get_array_length(env, noArr, &arrayLength);
    ASSERT_EQ(status, napi_array_expected);
    ASSERT_EQ(engine_->lastException_.IsEmpty(), false);
}

/**
 * @tc.name: NapiGetArraybufferInfoTest
 * @tc.desc: Test interface of napi_get_arraybuffer_info
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetArraybufferInfoTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_get_arraybuffer_info(env, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetArraybufferInfoTest
 * @tc.desc: Test interface of napi_get_arraybuffer_info
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetArraybufferInfoTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    size_t arrayBufferLength = 0;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    auto res = napi_get_arraybuffer_info(env, boolean, nullptr, &arrayBufferLength);
    ASSERT_EQ(res, napi_arraybuffer_expected);
}

/**
 * @tc.name: NapiGetPrototypeTest
 * @tc.desc: Test interface of napi_get_prototype
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetPrototypeTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_get_prototype(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetPrototypeTest
 * @tc.desc: Test interface of napi_get_prototype
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetPrototypeTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value result = nullptr;
    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    auto res = napi_get_prototype(env, boolean, &result);
    ASSERT_EQ(res, napi_object_expected);
}

/**
 * @tc.name: NapiGetTypedarrayInfoTest
 * @tc.desc: Test interface of napi_get_typedarray_info
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetTypedarrayInfoTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_get_typedarray_info(env, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetTypedarrayInfoTest
 * @tc.desc: Test interface of napi_get_typedarray_info
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetTypedarrayInfoTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    auto res = napi_get_typedarray_info(env, boolean, nullptr, nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetDataviewInfoTest
 * @tc.desc: Test interface of napi_get_dataview_info
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetDataviewInfoTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_get_dataview_info(env, nullptr, nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetDataviewInfoTest
 * @tc.desc: Test interface of napi_get_dataview_info
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetDataviewInfoTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    auto res = napi_get_dataview_info(env, boolean, nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetValueBoolTest
 * @tc.desc: Test interface of napi_get_value_bool
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueBoolTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_get_value_bool(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetValueBoolTest
 * @tc.desc: Test interface of napi_get_value_bool
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueBoolTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value stringUtf8 = nullptr;
    const char testStr[] = "errorType";
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, testStr, NAPI_AUTO_LENGTH, &stringUtf8));
    bool boolean;
    auto res = napi_get_value_bool(env, stringUtf8, &boolean);
    ASSERT_EQ(res, napi_boolean_expected);
}

/**
 * @tc.name: NapiGetValueDoubleTest
 * @tc.desc: Test interface of napi_get_value_double
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueDoubleTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_get_value_double(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetValueDoubleTest
 * @tc.desc: Test interface of napi_get_value_double
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueDoubleTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    double number;
    auto res = napi_get_value_double(env, boolean, &number);
    ASSERT_EQ(res, napi_number_expected);
}

/**
 * @tc.name: NapiGetValueExternalTest
 * @tc.desc: Test interface of napi_get_value_external
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueExternalTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_get_value_external(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetValueExternalTest
 * @tc.desc: Test interface of napi_get_value_external
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueExternalTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    void* external;
    auto res = napi_get_value_external(env, boolean, &external);
    ASSERT_EQ(res, napi_object_expected);
}

/**
 * @tc.name: NapiGetValueInt32Test
 * @tc.desc: Test interface of napi_get_value_int32
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueInt32Test001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_get_value_int32(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetValueInt32Test
 * @tc.desc: Test interface of napi_get_value_int32
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueInt32Test002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    int32_t number;
    auto res = napi_get_value_int32(env, boolean, &number);
    ASSERT_EQ(res, napi_number_expected);
}

/**
 * @tc.name: NapiGetValueInt32Test003
 * @tc.desc: Test napi_get_value_int32 with a valid positive int32 number
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueInt32Test003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    int32_t testNumber = TEST_INT32_500;
    napi_value numberValue = nullptr;
    ASSERT_EQ(napi_create_int32(env, testNumber, &numberValue), napi_ok);

    int32_t result = 0;
    napi_status res = napi_get_value_int32(env, numberValue, &result);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(result, testNumber);
}

/**
 * @tc.name: NapiGetValueInt32Test004
 * @tc.desc: Test napi_get_value_int32 with a valid negative int32 number
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueInt32Test004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    int32_t negativeNumber = TEST_INT64_NEGATIVE;
    napi_value numberValue = nullptr;
    ASSERT_EQ(napi_create_int32(env, negativeNumber, &numberValue), napi_ok);

    int32_t result = 0;
    napi_status res = napi_get_value_int32(env, numberValue, &result);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(result, negativeNumber);
}

/**
 * @tc.name: NapiGetValueInt32Test005
 * @tc.desc: Test napi_get_value_int32 with the maximum int32 number
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueInt32Test005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    int32_t maxNumber = INT32_MAX;
    napi_value numberValue = nullptr;
    ASSERT_EQ(napi_create_int32(env, maxNumber, &numberValue), napi_ok);

    int32_t result = 0;
    napi_status res = napi_get_value_int32(env, numberValue, &result);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(result, maxNumber);
}

/**
 * @tc.name: NapiGetValueInt32Test006
 * @tc.desc: Test napi_get_value_int32 with the minimum int32 number
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueInt32Test006, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    int32_t minNumber = INT32_MIN;
    napi_value numberValue = nullptr;
    ASSERT_EQ(napi_create_int32(env, minNumber, &numberValue), napi_ok);

    int32_t result = 0;
    napi_status res = napi_get_value_int32(env, numberValue, &result);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(result, minNumber);
}

/**
 * @tc.name: NapiGetValueInt64Test
 * @tc.desc: Test interface of napi_get_value_int64
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueInt64Test001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_get_value_int64(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetValueInt64Test
 * @tc.desc: Test interface of napi_get_value_int64
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueInt64Test002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    int64_t number;
    auto res = napi_get_value_int64(env, boolean, &number);
    ASSERT_EQ(res, napi_number_expected);
}

/**
 * @tc.name: NapiGetValueInt64Test003
 * @tc.desc: Test napi_get_value_int64 with a valid positive int64 number
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueInt64Test003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    int64_t testNumber = MAX_SAFE_INTEGER;
    napi_value numberValue = nullptr;
    ASSERT_EQ(napi_create_int64(env, testNumber, &numberValue), napi_ok);

    int64_t result = 0;
    napi_status res = napi_get_value_int64(env, numberValue, &result);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(result, testNumber);
}

/**
 * @tc.name: NapiGetValueInt64Test004
 * @tc.desc: Test napi_get_value_int64 with a valid negative int64 number
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueInt64Test004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    int64_t negativeNumber = TEST_INT64_NEGATIVE;
    napi_value numberValue = nullptr;
    ASSERT_EQ(napi_create_int64(env, negativeNumber, &numberValue), napi_ok);

    int64_t result = 0;
    napi_status res = napi_get_value_int64(env, numberValue, &result);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(result, negativeNumber);
}

/**
 * @tc.name: NapiGetValueInt64Test005
 * @tc.desc: Test napi_get_value_int64 with the maximum int64 number
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueInt64Test005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    int64_t maxNumber = INT64_MAX;
    napi_value numberValue = nullptr;
    ASSERT_EQ(napi_create_int64(env, maxNumber, &numberValue), napi_ok);

    int64_t result = 0;
    napi_status res = napi_get_value_int64(env, numberValue, &result);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(result, maxNumber);
}

/**
 * @tc.name: NapiGetValueInt64Test006
 * @tc.desc: Test napi_get_value_int64 with the minimum int64 number
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueInt64Test006, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    int64_t minNumber = INT64_MIN;
    napi_value numberValue = nullptr;
    ASSERT_EQ(napi_create_int64(env, minNumber, &numberValue), napi_ok);

    int64_t result = 0;
    napi_status res = napi_get_value_int64(env, numberValue, &result);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(result, minNumber);
}

/**
 * @tc.name: NapiGetValueInt64Test007
 * @tc.desc: Test napi_get_value_int64 with a non - number napi_value
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueInt64Test007, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value stringValue = nullptr;
    size_t strLength = strlen(TEST_STRING);
    ASSERT_EQ(napi_create_string_utf8(env, TEST_STRING, strLength, &stringValue), napi_ok);

    int64_t result = 0;
    napi_status res = napi_get_value_int64(env, stringValue, &result);
    ASSERT_EQ(res, napi_number_expected);
}

/**
 * @tc.name: NapiGetValueStringLatin1Test
 * @tc.desc: Test interface of napi_get_value_string_latin1
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueStringLatin1Test001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_get_value_string_latin1(env, nullptr, nullptr, 0, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetValueStringLatin1Test
 * @tc.desc: Test interface of napi_get_value_string_latin1
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueStringLatin1Test002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    auto res = napi_get_value_string_latin1(env, boolean, nullptr, 0, nullptr);
    ASSERT_EQ(res, napi_string_expected);
}

/**
 * @tc.name: NapiGetValueStringLatin1Test
 * @tc.desc: Test interface of napi_get_value_string_latin1
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueStringLatin1Test003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value stringValue = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_latin1(env, TEST_CHAR_STRING, NAPI_AUTO_LENGTH, &stringValue));
    auto res = napi_get_value_string_latin1(env, stringValue, nullptr, 0, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);

    size_t strSize = 0;
    res = napi_get_value_string_latin1(env, stringValue, nullptr, 0, &strSize);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiGetValueStringUtf8Test
 * @tc.desc: Test interface of napi_get_value_string_utf8
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueStringUtf8Test001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_get_value_string_utf8(env, nullptr, nullptr, 0, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetValueStringUtf8Test
 * @tc.desc: Test interface of napi_get_value_string_utf8
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueStringUtf8Test002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    auto res = napi_get_value_string_utf8(env, boolean, nullptr, 0, nullptr);
    ASSERT_EQ(res, napi_string_expected);
}

/**
 * @tc.name: NapiGetValueStringUtf8Test
 * @tc.desc: Test interface of napi_get_value_string_utf8
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueStringUtf8Test003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value stringValue = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_CHAR_STRING, NAPI_AUTO_LENGTH, &stringValue));
    auto res = napi_get_value_string_utf8(env, stringValue, nullptr, 0, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);

    size_t strSize = 0;
    res = napi_get_value_string_utf8(env, stringValue, nullptr, 0, &strSize);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiGetValueStringUtf16Test
 * @tc.desc: Test interface of napi_get_value_string_utf16
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueStringUtf16Test001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_get_value_string_utf16(env, nullptr, nullptr, 0, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetValueStringUtf16Test
 * @tc.desc: Test interface of napi_get_value_string_utf16
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueStringUtf16Test002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    auto res = napi_get_value_string_utf16(env, boolean, nullptr, 0, nullptr);
    ASSERT_EQ(res, napi_string_expected);
}

/**
 * @tc.name: NapiGetValueStringUtf16Test
 * @tc.desc: Test interface of napi_get_value_string_utf16
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueStringUtf16Test003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value stringValue = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf16(env, TEST_CHAR16_STRING, NAPI_AUTO_LENGTH, &stringValue));
    auto res = napi_get_value_string_utf16(env, stringValue, nullptr, 0, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);

    size_t strSize = 0;
    res = napi_get_value_string_utf16(env, stringValue, nullptr, 0, &strSize);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiGetValueUint32Test
 * @tc.desc: Test interface of napi_get_value_uint32
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueUint32Test001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_get_value_uint32(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetValueUint32Test
 * @tc.desc: Test interface of napi_get_value_uint32
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueUint32Test002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    uint32_t number;
    auto res = napi_get_value_uint32(env, boolean, &number);
    ASSERT_EQ(res, napi_number_expected);
}

/**
 * @tc.name: NapiGetValueUint32Test003
 * @tc.desc: Test napi_get_value_uint32 with a valid uint32 number
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueUint32Test003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    uint32_t testNumber = TEST_UINT32_1000;
    napi_value numberValue = nullptr;
    ASSERT_EQ(napi_create_uint32(env, testNumber, &numberValue), napi_ok);

    uint32_t result = 0;
    napi_status res = napi_get_value_uint32(env, numberValue, &result);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(result, testNumber);
}

/**
 * @tc.name: NapiGetValueUint32Test004
 * @tc.desc: Test napi_get_value_uint32 with a null environment
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueUint32Test004, testing::ext::TestSize.Level1)
{
    napi_value value = nullptr;
    napi_get_boolean(reinterpret_cast<napi_env>(engine_), true, &value);
    uint32_t result = 0;
    napi_status res = napi_get_value_uint32(nullptr, value, &result);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetValueUint32Test005
 * @tc.desc: Test napi_get_value_uint32 with the maximum uint32 number
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueUint32Test005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    uint32_t maxNumber = UINT32_MAX;
    napi_value numberValue = nullptr;
    ASSERT_EQ(napi_create_uint32(env, maxNumber, &numberValue), napi_ok);

    uint32_t result = 0;
    napi_status res = napi_get_value_uint32(env, numberValue, &result);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(result, maxNumber);
}

/**
 * @tc.name: NapiGetValueUint32Test006
 * @tc.desc: Test napi_get_value_uint32 with a negative number represented as double (should fail)
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetValueUint32Test006, testing::ext::TestSize.Level1)
{
    NAPI_EXTERN napi_status napi_create_double(napi_env env, double value, napi_value* result);

    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    double negativeNumber = -10.0; // test num -10.0
    napi_value numberValue = nullptr;
    ASSERT_EQ(napi_create_double(env, negativeNumber, &numberValue), napi_ok);

    uint32_t result = 0;
    napi_status res = napi_get_value_uint32(env, numberValue, &result);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiGetBooleanTest
 * @tc.desc: Test interface of napi_get_boolean
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetBooleanTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_status res = napi_get_boolean(env, true, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &result));
    ASSERT_CHECK_CALL(napi_get_boolean(env, false, &result));
}

bool checkBooleanValue(napi_env env, napi_value result, bool expected) {
    bool actual;
    napi_status status = napi_get_value_bool(env, result, &actual);
    if (status != napi_ok) {
        return false;
    }
    return actual == expected;
}

/**
 * @tc.name: NapiGetBooleanTest002
 * @tc.desc: Test napi_get_boolean with null environment
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetBooleanTest002, testing::ext::TestSize.Level1)
{
    bool value = true;
    napi_value result = nullptr;

    napi_status res = napi_get_boolean(nullptr, value, &result);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetBooleanTest003
 * @tc.desc: Test napi_get_boolean with true value
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetBooleanTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    bool value = true;
    napi_value result = nullptr;

    napi_status res = napi_get_boolean(env, value, &result);
    ASSERT_EQ(res, napi_ok);
    ASSERT_TRUE(checkBooleanValue(env, result, value));
}

/**
 * @tc.name: NapiGetBooleanTest004
 * @tc.desc: Test napi_get_boolean with false value
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetBooleanTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    bool value = false;
    napi_value result = nullptr;

    napi_status res = napi_get_boolean(env, value, &result);
    ASSERT_EQ(res, napi_ok);
    ASSERT_TRUE(checkBooleanValue(env, result, value));
}

/**
 * @tc.name: NapiGetBooleanTest005
 * @tc.desc: Test napi_get_boolean multiple times with different values
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetBooleanTest005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value result1 = nullptr;
    napi_value result2 = nullptr;

    napi_status res1 = napi_get_boolean(env, true, &result1);
    ASSERT_EQ(res1, napi_ok);
    ASSERT_TRUE(checkBooleanValue(env, result1, true));

    napi_status res2 = napi_get_boolean(env, false, &result2);
    ASSERT_EQ(res2, napi_ok);
    ASSERT_TRUE(checkBooleanValue(env, result2, false));
    bool res3 = false;
    napi_status res4 = napi_strict_equals(env, result1, result2, &res3);
    ASSERT_EQ(res4, napi_ok);
    ASSERT_FALSE(res3);
}

/**
 * @tc.name: NapiGetGlobalTest
 * @tc.desc: Test interface of napi_get_global
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetGlobalTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_status res = napi_get_global(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetGlobalTest002
 * @tc.desc: Test napi_get_global with valid environment and result pointer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetGlobalTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value result = nullptr;

    napi_status res = napi_get_global(env, &result);
    ASSERT_EQ(res, napi_ok);
    ASSERT_NE(result, nullptr);
}

/**
 * @tc.name: NapiGetGlobalTest003
 * @tc.desc: Test napi_get_global with null environment
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetGlobalTest003, testing::ext::TestSize.Level1)
{
    napi_value result = nullptr;

    napi_status res = napi_get_global(nullptr, &result);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetGlobalTest004
 * @tc.desc: Test napi_get_global multiple times with the same valid environment
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetGlobalTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value result1 = nullptr;
    napi_value result2 = nullptr;

    napi_status res1 = napi_get_global(env, &result1);
    ASSERT_EQ(res1, napi_ok);
    ASSERT_NE(result1, nullptr);

    napi_status res2 = napi_get_global(env, &result2);
    ASSERT_EQ(res2, napi_ok);
    ASSERT_NE(result2, nullptr);
    bool res3 = false;
    napi_strict_equals(env, result1, result2, &res3);
    ASSERT_TRUE(res3);
}

/**
 * @tc.name: NapiGetNullTest
 * @tc.desc: Test interface of napi_get_null
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetNullTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_status res = napi_get_null(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetNullTest002
 * @tc.desc: Test napi_get_null with valid environment and result pointer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetNullTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value result = nullptr;

    napi_status res = napi_get_null(env, &result);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiGetNullTest003
 * @tc.desc: Test napi_get_null with null environment
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetNullTest003, testing::ext::TestSize.Level1)
{
    napi_value result = nullptr;

    napi_status res = napi_get_null(nullptr, &result);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetNullTest004
 * @tc.desc: Test napi_get_null multiple times with the same valid environment
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetNullTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value result1 = nullptr;
    napi_value result2 = nullptr;

    napi_status res1 = napi_get_null(env, &result1);
    ASSERT_EQ(res1, napi_ok);

    napi_status res2 = napi_get_null(env, &result2);
    ASSERT_EQ(res2, napi_ok);
    bool res3 = false;
    napi_strict_equals(env, result1, result2, &res3);
    ASSERT_TRUE(res3);
}

/**
 * @tc.name: NapiGetNullTest005
 * @tc.desc: Test napi_get_null in different environments
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetNullTest005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    std::thread t([]() {
        NativeEngineProxy env1;
        env1->Deinit();
        NativeEngineProxy env2;
        env2->Deinit();

        napi_value result1 = nullptr;
        napi_value result2 = nullptr;

        napi_status res1 = napi_get_null(env1, &result1);
        ASSERT_EQ(res1, napi_ok);

        napi_status res2 = napi_get_null(env2, &result2);
        ASSERT_EQ(res2, napi_ok);
        bool res3 = false;
        napi_strict_equals(env1, result1, result2, &res3);
        ASSERT_TRUE(res3);
        env1->Init();
        env2->Init();
    });
    t.join();
}

/**
 * @tc.name: NapiGetUndefinedTest
 * @tc.desc: Test interface of napi_get_undefined
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetUndefinedTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_status res = napi_get_undefined(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetUndefinedTest002
 * @tc.desc: Test napi_get_undefined with valid environment and result pointer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetUndefinedTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value result = nullptr;

    napi_status res = napi_get_undefined(env, &result);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiGetUndefinedTest003
 * @tc.desc: Test napi_get_undefined with null environment
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetUndefinedTest003, testing::ext::TestSize.Level1)
{
    napi_value result = nullptr;

    napi_status res = napi_get_undefined(nullptr, &result);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetUndefinedTest004
 * @tc.desc: Test napi_get_undefined multiple times with the same valid environment
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetUndefinedTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value result1 = nullptr;
    napi_value result2 = nullptr;

    napi_status res1 = napi_get_undefined(env, &result1);
    ASSERT_EQ(res1, napi_ok);

    napi_status res2 = napi_get_undefined(env, &result2);
    ASSERT_EQ(res2, napi_ok);
    bool res3 = false;
    napi_strict_equals(env, result1, result2, &res3);
    ASSERT_TRUE(res3);
}

/**
 * @tc.name: NapiObjectFreezeTest
 * @tc.desc: Test interface of napi_object_freeze
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiObjectFreezeTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_status res = napi_object_freeze(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiObjectFreezeTest
 * @tc.desc: Test interface of napi_object_freeze
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiObjectFreezeTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    auto res = napi_object_freeze(env, boolean);
    ASSERT_EQ(res, napi_object_expected);
}

/**
 * @tc.name: NapiObjectFreezeTest
 * @tc.desc: Test interface of napi_object_freeze
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiObjectFreezeTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value str = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_latin1(env, "test string", NAPI_AUTO_LENGTH, &str));
    napi_status res = napi_object_freeze(env, str);
    ASSERT_EQ(res, napi_object_expected);
}

/**
 * @tc.name: NapiObjectFreezeTest
 * @tc.desc: Test interface of napi_object_freeze
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiObjectFreezeTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    size_t arrayBufferSize = 1024;
    ASSERT_CHECK_CALL(napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer));

    napi_value typedarray = nullptr;
    ASSERT_CHECK_CALL(napi_create_typedarray(env, napi_int8_array, arrayBufferSize, arrayBuffer, 0, &typedarray));

    napi_status res = napi_object_freeze(env, typedarray);
    ASSERT_EQ(res, napi_pending_exception);
}

/**
 * @tc.name: NapiObjectFreezeTest
 * @tc.desc: Test interface of napi_object_freeze
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiObjectFreezeTest005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value date = nullptr;
    ASSERT_CHECK_CALL(napi_create_date(env, 20250123, &date));
    napi_status res = napi_object_freeze(env, date);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiObjectSealTest
 * @tc.desc: Test interface of napi_object_seal
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiObjectSealTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_object_seal(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiObjectSealTest
 * @tc.desc: Test interface of napi_object_seal
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiObjectSealTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    napi_status res = napi_object_seal(env, boolean);
    ASSERT_EQ(res, napi_object_expected);
}

/**
 * @tc.name: NapiObjectSealTest003
 * @tc.desc: Test napi_object_seal with a valid object
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiObjectSealTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value object = nullptr;
    ASSERT_EQ(napi_create_object(env, &object), napi_ok);

    napi_status res = napi_object_seal(env, object);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiObjectSealTest004
 * @tc.desc: Test napi_object_seal with a null environment
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiObjectSealTest004, testing::ext::TestSize.Level1)
{
    napi_value object = nullptr;
    napi_create_object(reinterpret_cast<napi_env>(engine_), &object);

    napi_status res = napi_object_seal(nullptr, object);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiObjectSealTest005
 * @tc.desc: Test napi_object_seal on an already sealed object
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiObjectSealTest005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value object = nullptr;
    ASSERT_EQ(napi_create_object(env, &object), napi_ok);

    napi_status firstSealRes = napi_object_seal(env, object);
    ASSERT_EQ(firstSealRes, napi_ok);

    napi_status secondSealRes = napi_object_seal(env, object);
    ASSERT_EQ(secondSealRes, napi_ok);
}

/**
 * @tc.name: NapiGetAllPropertyNamesTest
 * @tc.desc: Test interface of napi_get_all_property_names
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetAllPropertyNamesTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_get_all_property_names(env, nullptr, napi_key_include_prototypes, napi_key_all_properties,
        napi_key_keep_numbers, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetAllPropertyNamesTest
 * @tc.desc: Test interface of napi_get_all_property_names
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetAllPropertyNamesTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
    auto res = napi_get_all_property_names(env, object, napi_key_include_prototypes, napi_key_all_properties,
        napi_key_keep_numbers, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetAllPropertyNamesTest
 * @tc.desc: Test interface of napi_get_all_property_names
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetAllPropertyNamesTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value result = nullptr;
    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    auto res = napi_get_all_property_names(env, boolean, napi_key_include_prototypes, napi_key_all_properties,
        napi_key_keep_numbers, &result);
    ASSERT_EQ(res, napi_object_expected);
}

/**
 * @tc.name: NapiGetAllPropertyNamesTest
 * @tc.desc: Test interface of napi_get_all_property_names
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetAllPropertyNamesTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value result = nullptr;
    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
    auto res = napi_get_all_property_names(env, object, (napi_key_collection_mode)(napi_key_include_prototypes - 1),
        napi_key_all_properties, napi_key_keep_numbers, &result);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetAllPropertyNamesTest
 * @tc.desc: Test interface of napi_get_all_property_names
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetAllPropertyNamesTest005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value result = nullptr;
    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
    auto res = napi_get_all_property_names(env, object, napi_key_include_prototypes, napi_key_all_properties,
        (napi_key_conversion)(napi_key_keep_numbers - 1), &result);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiDetachArraybufferTest
 * @tc.desc: Test interface of napi_detach_arraybuffer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiDetachArraybufferTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_detach_arraybuffer(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiDetachArraybufferTest
 * @tc.desc: Test interface of napi_detach_arraybuffer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiDetachArraybufferTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    auto res = napi_detach_arraybuffer(env, boolean);
    ASSERT_EQ(res, napi_object_expected);
}

/**
 * @tc.name: NapiDetachArraybufferTest
 * @tc.desc: Test interface of napi_detach_arraybuffer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiDetachArraybufferTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
    auto res = napi_detach_arraybuffer(env, object);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiIsDetachedArraybufferTest
 * @tc.desc: Test interface of napi_is_detached_arraybuffer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiIsDetachedArraybufferTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_is_detached_arraybuffer(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiIsDetachedArraybufferTest
 * @tc.desc: Test interface of napi_is_detached_arraybuffer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiIsDetachedArraybufferTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    static constexpr size_t arrayBufferSize = 1024;
    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    ASSERT_CHECK_CALL(napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer));

    auto res = napi_is_detached_arraybuffer(env, arrayBuffer, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiIsDetachedArraybufferTest003
 * @tc.desc: Test napi_is_detached_arraybuffer with a valid non - detached arraybuffer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiIsDetachedArraybufferTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    static constexpr size_t arrayBufferSize = 0;
    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    ASSERT_EQ(napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer), napi_ok);

    bool isDetached = false;
    napi_status res = napi_is_detached_arraybuffer(env, arrayBuffer, &isDetached);
    ASSERT_EQ(res, napi_ok);
    ASSERT_FALSE(isDetached);
}

/**
 * @tc.name: NapiIsDetachedArraybufferTest004
 * @tc.desc: Test napi_is_detached_arraybuffer with null result pointer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiIsDetachedArraybufferTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    static constexpr size_t arrayBufferSize = 0;
    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    ASSERT_EQ(napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer), napi_ok);

    napi_status res = napi_is_detached_arraybuffer(env, arrayBuffer, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiSetInstanceDataTest
 * @tc.desc: Test interface of napi_set_instance_data
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiSetInstanceDataTest001, testing::ext::TestSize.Level1)
{
    auto res = napi_set_instance_data(nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

void emptyFinalizeCb(napi_env env, void* data, void* hint) {}

/**
 * @tc.name: NapiSetInstanceDataTest002
 * @tc.desc: Test napi_set_instance_data with valid environment and null data
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiSetInstanceDataTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    std::thread t([]() {
        NativeEngineProxy env;
        void* data = nullptr;

        napi_status res = napi_set_instance_data(env, data, emptyFinalizeCb, nullptr);
        ASSERT_EQ(res, napi_ok);
    });
    t.join();
}

/**
 * @tc.name: NapiSetInstanceDataTest003
 * @tc.desc: Test napi_set_instance_data with valid environment, data, and finalize callback
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiSetInstanceDataTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    std::thread t([]() {
        NativeEngineProxy env;
        int dataValue = INT_FORTYTWO;
        void* data = &dataValue;

        napi_status res = napi_set_instance_data(env, data, emptyFinalizeCb, nullptr);
        ASSERT_EQ(res, napi_ok);
    });
    t.join();
}

/**
 * @tc.name: NapiSetInstanceDataTest004
 * @tc.desc: Test napi_set_instance_data with valid environment, data, null finalize callback, and non - null hint
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiSetInstanceDataTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    std::thread t([]() {
        NativeEngineProxy env;
        int dataValue = INT_FORTYTWO;
        void* data = &dataValue;
        int hintValue = INT_HUNDRED;

        napi_status res = napi_set_instance_data(env, data, nullptr, &hintValue);
        ASSERT_EQ(res, napi_ok);
    });
    t.join();
}

/**
 * @tc.name: NapiGetInstanceDataTest
 * @tc.desc: Test interface of napi_get_instance_data
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetInstanceDataTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_get_instance_data(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetInstanceDataTest002
 * @tc.desc: Test napi_get_instance_data after setting valid instance data
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetInstanceDataTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    std::thread t([]() {
        NativeEngineProxy env;

        int dataValue = INT_FORTYTWO;
        void* dataToSet = &dataValue;

        napi_status setRes = napi_set_instance_data(env, dataToSet, emptyFinalizeCb, nullptr);
        ASSERT_EQ(setRes, napi_ok);

        void* retrievedData;
        napi_status getRes = napi_get_instance_data(env, &retrievedData);
        ASSERT_EQ(getRes, napi_ok);
        ASSERT_EQ(retrievedData, dataToSet);
    });
    t.join();
}

/**
 * @tc.name: NapiGetInstanceDataTest003
 * @tc.desc: Test napi_get_instance_data with null environment
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetInstanceDataTest003, testing::ext::TestSize.Level1)
{
    void* retrievedData = nullptr;
    napi_status res = napi_get_instance_data(nullptr, &retrievedData);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetInstanceDataTest004
 * @tc.desc: Test napi_get_instance_data before setting any instance data
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiGetInstanceDataTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    std::thread t([]() {
        NativeEngineProxy env;
        env->Deinit();
        void* retrievedData = nullptr;
        napi_status res = napi_get_instance_data(env, &retrievedData);
        ASSERT_EQ(res, napi_ok);
        ASSERT_EQ(retrievedData, nullptr);
        env->Init();
    });
    t.join();
}

/**
 * @tc.name: NapiAddEnvCleanupHookTest
 * @tc.desc: Test interface of napi_add_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiAddEnvCleanupHookTest001, testing::ext::TestSize.Level1)
{
    auto res = napi_add_env_cleanup_hook(nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiAddEnvCleanupHookTest
 * @tc.desc: Test interface of napi_add_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiAddEnvCleanupHookTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_add_env_cleanup_hook(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiRemoveEnvCleanupHookTest
 * @tc.desc: Test interface of napi_remove_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiRemoveEnvCleanupHookTest001, testing::ext::TestSize.Level1)
{
    auto res = napi_remove_env_cleanup_hook(nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiRemoveEnvCleanupHookTest
 * @tc.desc: Test interface of napi_remove_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiRemoveEnvCleanupHookTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_remove_env_cleanup_hook(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiAddAsyncCleanupHookTest
 * @tc.desc: Test interface of napi_add_async_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiAddAsyncCleanupHookTest001, testing::ext::TestSize.Level1)
{
    auto res = napi_add_async_cleanup_hook(nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiAddAsyncCleanupHookTest
 * @tc.desc: Test interface of napi_add_async_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiAddAsyncCleanupHookTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_add_async_cleanup_hook(env, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiRemoveAsyncCleanupHookTest
 * @tc.desc: Test interface of napi_remove_async_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiRemoveAsyncCleanupHookTest001, testing::ext::TestSize.Level1)
{
    auto res = napi_remove_async_cleanup_hook(nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NodeApiGetModuleFileNameTest
 * @tc.desc: Test interface of node_api_get_module_file_name
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NodeApiGetModuleFileNameTest001, testing::ext::TestSize.Level1)
{
    auto res = node_api_get_module_file_name(nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NodeApiGetModuleFileNameTest
 * @tc.desc: Test interface of node_api_get_module_file_name
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NodeApiGetModuleFileNameTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = node_api_get_module_file_name(env, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NodeApiGetModuleFileNameTest003
 * @tc.desc: Test node_api_get_module_file_name with valid environment and valid result pointer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NodeApiGetModuleFileNameTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const char* result = nullptr;

    napi_status res = node_api_get_module_file_name(env, &result);
    ASSERT_EQ(res, napi_ok);
    ASSERT_NE(result, nullptr);
}

/**
 * @tc.name: NodeApiGetModuleFileNameTest004
 * @tc.desc: Test node_api_get_module_file_name with same valid environment multiple times
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NodeApiGetModuleFileNameTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const char* result = nullptr;

    for (int i = 0; i < BUFFER_SIZE_FIVE; ++i) {
        napi_status res = node_api_get_module_file_name(env, &result);
        ASSERT_EQ(res, napi_ok);
        ASSERT_NE(result, nullptr);
    }
}

/**
 * @tc.name: NapiAddFinalizerTest
 * @tc.desc: Test interface of napi_add_finalizer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiAddFinalizerTest001, testing::ext::TestSize.Level1)
{
    auto res = napi_add_finalizer(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiAddFinalizerTest
 * @tc.desc: Test interface of napi_add_finalizer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiAddFinalizerTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_add_finalizer(env, nullptr, nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiAddFinalizerTest
 * @tc.desc: Test interface of napi_add_finalizer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiAddFinalizerTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    auto res = napi_add_finalizer(env, boolean, nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiAddFinalizerTest
 * @tc.desc: Test interface of napi_add_finalizer
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiAddFinalizerTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    auto res = napi_add_finalizer(env, boolean, nullptr, TestFinalizer, nullptr, nullptr);
    ASSERT_EQ(res, napi_object_expected);
}

/**
 * @tc.name: NapiQueueAsyncWorkWithQosTest
 * @tc.desc: Test interface of napi_queue_async_work_with_qos
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiQueueAsyncWorkWithQosTest001, testing::ext::TestSize.Level1)
{
    auto res = napi_queue_async_work_with_qos(nullptr, nullptr, napi_qos_default);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiQueueAsyncWorkWithQosTest
 * @tc.desc: Test interface of napi_queue_async_work_with_qos
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiQueueAsyncWorkWithQosTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_queue_async_work_with_qos(env, nullptr, napi_qos_default);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiQueueAsyncWorkWithQosTest
 * @tc.desc: Test interface of napi_queue_async_work_with_qos
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiQueueAsyncWorkWithQosTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    struct AsyncWorkContext {
        napi_async_work work = nullptr;
    };
    auto asyncWorkContext = new AsyncWorkContext();
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, TEST_CHAR_STRING, NAPI_AUTO_LENGTH, &resourceName);
    ASSERT_CHECK_CALL(napi_create_async_work(
        env, nullptr, resourceName, [](napi_env value, void* data) {},
        [](napi_env env, napi_status status, void* data) {
            STOP_EVENT_LOOP(env);
            AsyncWorkContext* asyncWorkContext = (AsyncWorkContext*)data;
            napi_status deleteStatus = napi_delete_async_work(env, asyncWorkContext->work);
            delete asyncWorkContext;
            ASSERT_EQ(deleteStatus, napi_ok);
        },
        asyncWorkContext, &asyncWorkContext->work));

    auto res = napi_queue_async_work_with_qos(env, asyncWorkContext->work, napi_qos_default);
    ASSERT_EQ(res, napi_ok);
    RUN_EVENT_LOOP(env);
}

/**
 * @tc.name: NapiRunScriptPathTest
 * @tc.desc: Test interface of napi_run_script_path
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiRunScriptPathTest001, testing::ext::TestSize.Level1)
{
    auto res = napi_run_script_path(nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiRunScriptPathTest
 * @tc.desc: Test interface of napi_run_script_path
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiRunScriptPathTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_run_script_path(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiRunScriptPathTest
 * @tc.desc: Test interface of napi_run_script_path
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiRunScriptPathTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value result = nullptr;
    auto res = napi_run_script_path(env, TEST_CHAR_STRING, &result);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiLoadModuleTest
 * @tc.desc: Test interface of napi_load_module
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiLoadModuleTest001, testing::ext::TestSize.Level1)
{
    auto res = napi_load_module(nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiLoadModuleTest
 * @tc.desc: Test interface of napi_load_module
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiLoadModuleTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_load_module(env, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiLoadModuleTest
 * @tc.desc: Test interface of napi_load_module
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiLoadModuleTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value result = nullptr;
    auto res = napi_load_module(env, nullptr, &result);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiLoadModuleTest
 * @tc.desc: Test interface of napi_load_module
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiLoadModuleTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value result = nullptr;
    auto res = napi_load_module(env, "@ohos:xxx", &result);
    ASSERT_EQ(res, napi_pending_exception);
}

/**
 * @tc.name: NapiCreateObjectWithPropertiesTest
 * @tc.desc: Test interface of napi_create_object_with_properties
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateObjectWithPropertiesTest001, testing::ext::TestSize.Level1)
{
    auto res = napi_create_object_with_properties(nullptr, nullptr, 0, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateObjectWithPropertiesTest
 * @tc.desc: Test interface of napi_create_object_with_properties
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateObjectWithPropertiesTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_object_with_properties(env, nullptr, 0, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateObjectWithPropertiesTest003
 * @tc.desc: Test napi_create_object_with_properties with valid environment and one property
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateObjectWithPropertiesTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    size_t propertyCount = 0;
    napi_value func1 = nullptr;
    napi_property_descriptor properties[] = {
        {TEST_PROP_STRING_1, nullptr, nullptr, nullptr, nullptr, func1, napi_default, nullptr}
    };
    napi_value result = nullptr;

    napi_status res = napi_create_object_with_properties(env, &result, propertyCount, properties);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCreateObjectWithPropertiesTest004
 * @tc.desc: Test napi_create_object_with_properties with multiple properties
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateObjectWithPropertiesTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    size_t propertyCount = INT_TWO;
    napi_value func1 = nullptr;
    napi_property_descriptor properties[] = {
        {TEST_PROP_STRING_1, nullptr, nullptr, nullptr, nullptr, func1, napi_default, nullptr},
        {TEST_PROP_STRING_2, nullptr, nullptr, nullptr, nullptr, func1, napi_default, nullptr}
    };
    napi_value result = nullptr;

    napi_status res = napi_create_object_with_properties(env, &result, propertyCount, properties);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCreateObjectWithPropertiesTest005
 * @tc.desc: Test napi_create_object_with_properties with valid environment but zero property count
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateObjectWithPropertiesTest005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    size_t propertyCount = 0;
    napi_value result = nullptr;

    napi_status res = napi_create_object_with_properties(env, &result, propertyCount, nullptr);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCreateObjectWithPropertiesTest006
 * @tc.desc: Test napi_create_object_with_properties with null environment
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateObjectWithPropertiesTest006, testing::ext::TestSize.Level1)
{
    size_t propertyCount = INT_ONE;
    napi_value func1 = nullptr;

    napi_property_descriptor properties[] = {
        {TEST_CHAR_STRING, nullptr, nullptr, nullptr, nullptr, func1, napi_default, nullptr}
    };
    napi_value result = nullptr;

    napi_status res = napi_create_object_with_properties(nullptr, &result, propertyCount, properties);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateObjectWithNamedPropertiesTest
 * @tc.desc: Test interface of napi_create_object_with_named_properties
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateObjectWithNamedPropertiesTest001, testing::ext::TestSize.Level1)
{
    auto res = napi_create_object_with_named_properties(nullptr, nullptr, 0, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateObjectWithNamedPropertiesTest
 * @tc.desc: Test interface of napi_create_object_with_named_properties
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateObjectWithNamedPropertiesTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_create_object_with_named_properties(env, nullptr, 0, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateObjectWithNamedPropertiesTest003
 * @tc.desc: Test napi_create_object_with_named_properties with valid environment and one property
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateObjectWithNamedPropertiesTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    size_t propertyCount = INT_ONE;
    const char* keys[] = { TEST_CHAR_STRING };
    napi_value values[1];
    values[0] = nullptr;
    napi_value result = nullptr;

    napi_status res = napi_create_object_with_named_properties(env, &result, propertyCount, keys, values);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCreateObjectWithNamedPropertiesTest004
 * @tc.desc: Test napi_create_object_with_named_properties with multiple properties
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateObjectWithNamedPropertiesTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    size_t propertyCount = INT_TWO;
    const char* keys[] = { TEST_PROP_STRING_1, TEST_PROP_STRING_2 };
    napi_value values[2];
    values[0] = nullptr;
    values[1] = nullptr;
    napi_value result = nullptr;

    napi_status res = napi_create_object_with_named_properties(env, &result, propertyCount, keys, values);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCreateObjectWithNamedPropertiesTest005
 * @tc.desc: Test napi_create_object_with_named_properties with valid environment but zero property count
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateObjectWithNamedPropertiesTest005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    size_t propertyCount = INT_ZERO;
    const char** keys = nullptr;
    const napi_value* values = nullptr;
    napi_value result = nullptr;

    napi_status res = napi_create_object_with_named_properties(env, &result, propertyCount, keys, values);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCreateObjectWithNamedPropertiesTest006
 * @tc.desc: Test napi_create_object_with_named_properties with null environment
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateObjectWithNamedPropertiesTest006, testing::ext::TestSize.Level1)
{
    size_t propertyCount = INT_ONE;
    const char* keys[] = { TEST_PROP_STRING_1 };
    napi_value values[1];
    values[0] = nullptr;
    napi_value result = nullptr;

    napi_status res = napi_create_object_with_named_properties(nullptr, &result, propertyCount, keys, values);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCoerceToNativeBindingObjectTest
 * @tc.desc: Test interface of napi_coerce_to_native_binding_object
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCoerceToNativeBindingObjectTest001, testing::ext::TestSize.Level1)
{
    auto res = napi_coerce_to_native_binding_object(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCoerceToNativeBindingObjectTest
 * @tc.desc: Test interface of napi_coerce_to_native_binding_object
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCoerceToNativeBindingObjectTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_coerce_to_native_binding_object(env, nullptr, nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCoerceToNativeBindingObjectTest
 * @tc.desc: Test interface of napi_coerce_to_native_binding_object
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCoerceToNativeBindingObjectTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
    auto res = napi_coerce_to_native_binding_object(env, object, nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCoerceToNativeBindingObjectTest
 * @tc.desc: Test interface of napi_coerce_to_native_binding_object
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCoerceToNativeBindingObjectTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
    auto res = napi_coerce_to_native_binding_object(env, object, TestDetachCallback, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCoerceToNativeBindingObjectTest
 * @tc.desc: Test interface of napi_coerce_to_native_binding_object
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCoerceToNativeBindingObjectTest005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
    auto res = napi_coerce_to_native_binding_object(env, object, TestDetachCallback, TestAttachCallback,
        nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCoerceToNativeBindingObjectTest
 * @tc.desc: Test interface of napi_coerce_to_native_binding_object
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCoerceToNativeBindingObjectTest006, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
    auto res = napi_coerce_to_native_binding_object(env, object, TestDetachCallback, TestAttachCallback,
        reinterpret_cast<void*>(object), nullptr);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiCreateArkRuntimeTest
 * @tc.desc: Test interface of napi_create_ark_runtime
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateArkRuntimeTest001, testing::ext::TestSize.Level1)
{
    std::thread t([]() {
        auto res = napi_create_ark_runtime(nullptr);
        ASSERT_EQ(res, napi_invalid_arg);
    });
    t.join();
}

/**
 * @tc.name: NapiCreateArkRuntimeTest
 * @tc.desc: Test interface of napi_create_ark_runtime
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiCreateArkRuntimeTest002, testing::ext::TestSize.Level1)
{
    std::thread t([]() {
        auto temp = NativeCreateEnv::g_createNapiEnvCallback;
        NativeCreateEnv::g_createNapiEnvCallback = nullptr;
        auto res = napi_create_ark_runtime(nullptr);
        NativeCreateEnv::g_createNapiEnvCallback = temp;
        ASSERT_EQ(res, napi_invalid_arg);
    });
    t.join();
}

/**
 * @tc.name: NapiDestroyArkRuntimeTest
 * @tc.desc: Test interface of napi_destroy_ark_runtime
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiDestroyArkRuntimeTest001, testing::ext::TestSize.Level1)
{
    auto res = napi_destroy_ark_runtime(nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiDestroyArkRuntimeTest
 * @tc.desc: Test interface of napi_destroy_ark_runtime
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiDestroyArkRuntimeTest002, testing::ext::TestSize.Level1)
{
    auto temp = NativeCreateEnv::g_destroyNapiEnvCallback;
    NativeCreateEnv::g_destroyNapiEnvCallback = nullptr;
    auto res = napi_destroy_ark_runtime(nullptr);
    NativeCreateEnv::g_destroyNapiEnvCallback = temp;
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiRunEventLoopTest
 * @tc.desc: Test interface of napi_run_event_loop
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiRunEventLoopTest001, testing::ext::TestSize.Level1)
{
    auto res = napi_run_event_loop(nullptr, napi_event_mode_default);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiRunEventLoopTest
 * @tc.desc: Test interface of napi_run_event_loop
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiRunEventLoopTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_run_event_loop(env, (napi_event_mode)(napi_event_mode_default - 1));
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiStopEventLoopTest
 * @tc.desc: Test interface of napi_stop_event_loop
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiStopEventLoopTest001, testing::ext::TestSize.Level1)
{
    auto res = napi_stop_event_loop(nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiLoadModuleWithInfoTest
 * @tc.desc: Test interface of napi_load_module_with_info
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiLoadModuleWithInfoTest001, testing::ext::TestSize.Level1)
{
    auto res = napi_load_module_with_info(nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiLoadModuleWithInfoTest
 * @tc.desc: Test interface of napi_load_module_with_info
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiLoadModuleWithInfoTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_load_module_with_info(env, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiLoadModuleWithInfoTest
 * @tc.desc: Test interface of napi_load_module_with_info
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiLoadModuleWithInfoTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value result = nullptr;
    auto res = napi_load_module_with_info(env, nullptr, nullptr, &result);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiLoadModuleWithInfoTest
 * @tc.desc: Test interface of napi_load_module_with_info
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiLoadModuleWithInfoTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value result1 = nullptr;
    napi_status res = napi_load_module_with_info(env, "@ohos:xxxx", nullptr, &result1);
    ASSERT_EQ(res, napi_generic_failure);
    const napi_extended_error_info* result2;
    napi_get_last_error_info(env, &result2);
    ASSERT_EQ(result2->error_code, napi_generic_failure);
}

/**
 * @tc.name: NapiLoadModuleWithInfoForHybridAppTest
 * @tc.desc: Test interface of napi_load_module_with_info_hybrid
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiLoadModuleWithInfoForHybridAppTest001, testing::ext::TestSize.Level1)
{
    auto res = napi_load_module_with_info_hybrid(nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiLoadModuleWithInfoForHybridAppTest
 * @tc.desc: Test interface of napi_load_module_with_info_hybrid
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiLoadModuleWithInfoForHybridAppTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_load_module_with_info_hybrid(env, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiLoadModuleWithInfoForHybridAppTest
 * @tc.desc: Test interface of napi_load_module_with_info_hybrid
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiLoadModuleWithInfoForHybridAppTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value result = nullptr;
    auto res = napi_load_module_with_info_hybrid(env, nullptr, nullptr, &result);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiSerializeTest
 * @tc.desc: Test interface of napi_serialize
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiSerializeTest001, testing::ext::TestSize.Level1)
{
    auto res = napi_serialize(nullptr, nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiSerializeTest
 * @tc.desc: Test interface of napi_serialize
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiSerializeTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_serialize(env, nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiSerializeTest
 * @tc.desc: Test interface of napi_serialize
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiSerializeTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value num = nullptr;
    ASSERT_CHECK_CALL(napi_create_uint32(env, TEST_UINT32_1000, &num));
    auto res = napi_serialize(env, num, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiSerializeTest
 * @tc.desc: Test interface of napi_serialize
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiSerializeTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value num = nullptr;
    ASSERT_CHECK_CALL(napi_create_uint32(env, TEST_UINT32_1000, &num));
    napi_value undefined = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &undefined));
    auto res = napi_serialize(env, num, undefined, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiSerializeTest
 * @tc.desc: Test interface of napi_serialize
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiSerializeTest005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value num = nullptr;
    ASSERT_CHECK_CALL(napi_create_uint32(env, TEST_UINT32_1000, &num));
    napi_value undefined = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &undefined));
    auto res = napi_serialize(env, num, undefined, undefined, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiSerializeTest
 * @tc.desc: Test interface of napi_serialize
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiSerializeTest006, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value num = nullptr;
    ASSERT_CHECK_CALL(napi_create_uint32(env, TEST_UINT32_1000, &num));
    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    void* data = nullptr;
    auto res = napi_serialize(env, num, boolean, boolean, &data);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiSerializeTest
 * @tc.desc: Test interface of napi_serialize
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiSerializeTest007, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value num = nullptr;
    ASSERT_CHECK_CALL(napi_create_uint32(env, TEST_UINT32_1000, &num));
    napi_value undefined = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &undefined));
    napi_value boolean = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &boolean));
    void* data = nullptr;
    auto res = napi_serialize(env, num, undefined, boolean, &data);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiSerializeTest
 * @tc.desc: Test interface of napi_serialize
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiSerializeTest008, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value num = nullptr;
    ASSERT_CHECK_CALL(napi_create_uint32(env, TEST_UINT32_1000, &num));
    napi_value undefined = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &undefined));
    void* data = nullptr;
    auto res = napi_serialize(env, num, undefined, undefined, &data);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiEncodeTest
 * @tc.desc: Test interface of napi_encode
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiEncodeTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    const char testStr[] = "中测_Eng_123";
    size_t testStrLength = strlen(testStr);
    napi_value src = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, testStr, testStrLength, &src));

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_encode(env, src, &result));
    char expected[15] = {0xe4, 0xb8, 0xad, 0xe6, 0xb5, 0x8b, 0x5f, 0x45, 0x6e, 0x67, 0x5f, 0x31, 0x32, 0x33, 0};

    napi_typedarray_type type;
    size_t srcLength = 0;
    void* srcData = nullptr;
    napi_value srcBuffer = nullptr;
    size_t byteOffset = 0;

    napi_get_typedarray_info(env, result, &type, &srcLength, &srcData, &srcBuffer, &byteOffset);

    ASSERT_EQ(srcLength, 14); // 14:string length
    char* res = reinterpret_cast<char*>(srcData);

    res[srcLength] = 0;
    ASSERT_STREQ(res, expected);
}

/**
 * @tc.name: NapiEncodeTest
 * @tc.desc: Test interface of napi_encode
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiEncodeTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value undefined = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &undefined));

    napi_value result = nullptr;
    auto ret = napi_encode(env, undefined, &result);
    ASSERT_EQ(ret, napi_string_expected);
}

struct CallbackDataAsyncWork007 {
    // napi_threadsafe_function tsfn;
    napi_async_work work1;
    napi_async_work work2;
    std::vector<napi_property_descriptor>* desc;
    napi_ref objref;
    std::map<int, napi_value>* input;
};

static void executeWork1(napi_env env, void *data)
{
    CallbackDataAsyncWork007 *callbackData = reinterpret_cast<CallbackDataAsyncWork007 *>(data);
    int count = 50;
    for(int i = 0; i < count; i++) {
        std::string str = TEST_PROP + std::to_string(i);
        char* prop = new char[str.size() + 1];
        errno_t ret = strncpy_s(prop, str.size() + 1, str.c_str(), str.size());
        ASSERT_EQ(ret, EOK);
        napi_property_descriptor property = {
            prop,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            (*callbackData->input)[i],
            napi_default,
            nullptr
        };
        callbackData->desc->push_back(property);
    }
}

static void executeWork2(napi_env env, void *data)
{
    CallbackDataAsyncWork007 *callbackData = reinterpret_cast<CallbackDataAsyncWork007 *>(data);
    int count = 100;
    for(int i = 50; i < count; i++) {
        std::string str = TEST_PROP + std::to_string(i);
        char* prop = new char[str.size() + 1];
        errno_t ret = strncpy_s(prop, str.size() + 1, str.c_str(), str.size());
        ASSERT_EQ(ret, EOK);
        napi_property_descriptor property = {
            prop,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            (*callbackData->input)[i],
            napi_default,
            nullptr
        };
        callbackData->desc->push_back(property);
    }
}

static void completeWork1(napi_env env, napi_status status, void* data)
{
    CallbackDataAsyncWork007 *callbackData = reinterpret_cast<CallbackDataAsyncWork007 *>(data);
    napi_value object = nullptr;
    napi_get_reference_value(env, callbackData->objref, &object);
    ASSERT_CHECK_CALL(napi_define_properties(env, object, callbackData->desc->size(), callbackData->desc->data()));

    napi_delete_async_work(env, callbackData->work1);
    callbackData->work1 = nullptr;
    if(callbackData->work2 == nullptr) {
        napi_delete_reference(env, callbackData->objref);
        delete callbackData->desc;
        delete callbackData->input;
        delete callbackData;
        STOP_EVENT_LOOP(env);
    }
}

static void completeWork2(napi_env env, napi_status status, void* data)
{
    CallbackDataAsyncWork007 *callbackData = reinterpret_cast<CallbackDataAsyncWork007 *>(data);
    napi_value object = nullptr;
    napi_get_reference_value(env, callbackData->objref, &object);
    ASSERT_CHECK_CALL(napi_define_properties(env, object, callbackData->desc->size(), callbackData->desc->data()));

    napi_delete_async_work(env, callbackData->work2);
    callbackData->work2 = nullptr;
    if (callbackData->work1 == nullptr) {
        napi_delete_reference(env, callbackData->objref);
        delete callbackData->desc;
        delete callbackData->input;
        delete callbackData;
        STOP_EVENT_LOOP(env);
    }
}

/**
 * @tc.name: AsyncWorkTest007
 * @tc.desc: Test async work.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AsyncWorkTest007, testing::ext::TestSize.Level1)
{
    HILOG_INFO("AsyncWorkTest007 start");
    napi_env env = (napi_env)engine_;
    std::vector<napi_property_descriptor>* desc = new std::vector<napi_property_descriptor>{};
    desc->reserve(100);
    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_ref objref;
    napi_create_reference(env, object, 1, &objref);

    CallbackDataAsyncWork007 *callbackData = new CallbackDataAsyncWork007();
    callbackData->desc = desc;
    callbackData->objref = objref;
    std::map<int, napi_value>* input = new std::map<int, napi_value>();
    for (int i = 1; i < 100; i++) {
        napi_value value = nullptr;
        napi_create_int32(env, i, &value);
        (*input)[i] = value;
    }
    callbackData->input = input;

    napi_value resourceName1 = nullptr;
    napi_create_string_utf8(env, "AsyncWorkTest007-1", NAPI_AUTO_LENGTH, &resourceName1);
    ASSERT_CHECK_CALL(napi_create_async_work(env, nullptr, resourceName1, executeWork1, completeWork1,
        callbackData, &callbackData->work1));

    napi_value resourceName2 = nullptr;
    napi_create_string_utf8(env, "AsyncWorkTest007-2", NAPI_AUTO_LENGTH, &resourceName2);
    ASSERT_CHECK_CALL(napi_create_async_work(env, nullptr, resourceName2, executeWork2, completeWork2,
        callbackData, &callbackData->work2));
    ASSERT_CHECK_CALL(napi_queue_async_work(env, callbackData->work1));
    ASSERT_CHECK_CALL(napi_queue_async_work(env, callbackData->work2));
    RUN_EVENT_LOOP(env);

    bool hasProperty = false;
    napi_has_named_property(env, object, TEST_PROP_TEST, &hasProperty);
    ASSERT_EQ(hasProperty, true);

    napi_value prop = nullptr;
    napi_get_named_property(env, object, TEST_PROP_TEST, &prop);
    int32_t value = -1;
    napi_get_value_int32(env, prop, &value);
    ASSERT_EQ(value, 99);
    HILOG_INFO("AsyncWorkTest007 end");
}

class FinalizerChecker {
public:
    FinalizerChecker() : called_(false), data_(nullptr) {}

    static void Callback(void* arg) {
        FinalizerChecker* checker = static_cast<FinalizerChecker*>(arg);
        checker->called_ = true;
        checker->data_ = arg;
    }

    bool Called() const { return called_; }
    void* Data() const { return data_; }

private:
    bool called_;
    void* data_;
};

/**
* @tc.name: AddCleanupFinalizerTest001
* @tc.desc: Test AddCleanupFinalizer.
* @tc.type: FUNC
*/
HWTEST_F(NapiBasicTest, AddCleanupFinalizerTest001, testing::ext::TestSize.Level1)
{
    std::thread t([]() {
        NativeEngineProxy env;
        FinalizerChecker* checker = new FinalizerChecker();
        void* testData = checker;

        napi_status status = napi_add_cleanup_finalizer(env, FinalizerChecker::Callback, testData);
        EXPECT_EQ(status, napi_ok);
        delete checker;
    });
    t.join();
}

/**
* @tc.name: AddCleanupFinalizerTest002
* @tc.desc: Test AddCleanupFinalizer.
* @tc.type: FUNC
*/
HWTEST_F(NapiBasicTest, AddCleanupFinalizerTest002, testing::ext::TestSize.Level1)
{
    std::thread t([]() {
        NativeEngineProxy engine;
        void* testData = new char[10];
        CleanupFinalizerCallBack callback = [](void* arg) {
            delete[] reinterpret_cast<char*>(arg);
        };

        napi_status addStatus = engine->AddCleanupFinalizer(callback, testData);
        EXPECT_EQ(addStatus, napi_ok);
    });
    t.join();
}

/**
* @tc.name: RemoveCleanupFinalizerTest001
* @tc.desc: Test RemoveCleanupFinalizer.
* @tc.type: FUNC
*/
HWTEST_F(NapiBasicTest, RemoveCleanupFinalizerTest001, testing::ext::TestSize.Level1)
{
    std::thread t([]() {
        NativeEngineProxy env;
        FinalizerChecker* checker = new FinalizerChecker();
        void* testData = checker;

        napi_status addStatus = napi_add_cleanup_finalizer(env, FinalizerChecker::Callback, testData);
        EXPECT_EQ(addStatus, napi_ok);

        napi_status removeStatus = napi_remove_cleanup_finalizer(env, FinalizerChecker::Callback, testData);
        EXPECT_EQ(removeStatus, napi_ok);
        delete checker;
    });
    t.join();
}

/**
* @tc.name: RemoveCleanupFinalizerTest002
* @tc.desc: Test RemoveCleanupFinalizer.
* @tc.type: FUNC
*/
HWTEST_F(NapiBasicTest, RemoveCleanupFinalizerTest002, testing::ext::TestSize.Level1)
{
    std::thread t([]() {
        NativeEngineProxy engine;
        void* testData = new char[10];
        CleanupFinalizerCallBack callback = [](void* arg) {
            delete[] reinterpret_cast<char*>(arg);
        };

        napi_status addStatus = engine->AddCleanupFinalizer(callback, testData);
        EXPECT_EQ(addStatus, napi_ok);

        napi_status removeStatus = engine->RemoveCleanupFinalizer(callback, testData);
        EXPECT_EQ(removeStatus, napi_ok);
        delete[] reinterpret_cast<char*>(testData);
    });
    t.join();
}

/**
* @tc.name: RunInstanceFinalizerTest001
* @tc.desc: Test RunInstanceFinalizer.
* @tc.type: FUNC
*/
HWTEST_F(NapiBasicTest, RunInstanceFinalizerTest001, testing::ext::TestSize.Level1)
{
    std::thread t([]() {
        NativeEngineProxy engine;
        FinalizerChecker* checker = new FinalizerChecker();
        void* testData = checker;
        CleanupFinalizerCallBack callback = FinalizerChecker::Callback;

        napi_status addStatus = engine->AddCleanupFinalizer(callback, testData);
        EXPECT_EQ(addStatus, napi_ok);

        engine->RunInstanceFinalizer();
        EXPECT_TRUE(checker->Called());
        delete checker;
    });
    t.join();
}

/**
* @tc.name: RunInstanceFinalizer002
* @tc.desc: Test RunInstanceFinalizer.
* @tc.type: FUNC
*/
HWTEST_F(NapiBasicTest, RunInstanceFinalizer002, testing::ext::TestSize.Level1)
{
    std::thread t([]() {
        NativeEngineProxy engine;
        FinalizerChecker* checker = new FinalizerChecker();
        void* testData = checker;
        CleanupFinalizerCallBack callback = FinalizerChecker::Callback;

        napi_status addStatus = engine->AddCleanupFinalizer(callback, testData);
        EXPECT_EQ(addStatus, napi_ok);

        napi_status removeStatus = engine->RemoveCleanupFinalizer(callback, testData);
        EXPECT_EQ(removeStatus, napi_ok);

        engine->RunInstanceFinalizer();
        EXPECT_FALSE(checker->Called());
        delete checker;
    });
    t.join();
}

/**
 * @tc.name: NapiEnvCleanupTest001
 * @tc.desc: Test code of cleanup
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiEnvCleanupTest001, testing::ext::TestSize.Level1)
{
    std::thread t([]() {
        NativeEngineProxy engine;
        static bool workDone = false;
        // make sure async work is done after cleanup hook
        ASSERT_CHECK_CALL(napi_add_env_cleanup_hook(
            engine,
            [](void* data) {
                NativeEngineProxy* proxy = reinterpret_cast<NativeEngineProxy*>(data);
                napi_env env = *proxy;
                napi_value taskName = nullptr;
                ASSERT_CHECK_CALL(napi_create_string_utf8(env, __FUNCTION__, NAPI_AUTO_LENGTH, &taskName));
                napi_async_work* work = new napi_async_work;
                static std::mutex cleanupMutex;
                static std::condition_variable cond;
                ASSERT_CHECK_CALL(napi_create_async_work(
                    env, nullptr, taskName,
                    [](napi_env, void*) {
                        std::unique_lock<std::mutex> lock(cleanupMutex);
                        cond.notify_one();
                    },
                    [](napi_env env, napi_status, void* data) {
                        napi_async_work* work = reinterpret_cast<napi_async_work*>(data);
                        ASSERT_CHECK_CALL(napi_delete_async_work(env, *work));
                        delete work;
                        workDone = true;
                    },
                    work, work));
                std::unique_lock<std::mutex> lock(cleanupMutex);
                ASSERT_CHECK_CALL(napi_queue_async_work(env, *work));
                cond.wait(lock);
            },
            &engine));
        LoggerCollector collector(LogLevel::LOG_DEBUG);
        engine->RunCleanup();
        ASSERT_TRUE(collector.Includes("CleanupHandles, request waiting:"));
        ASSERT_TRUE(workDone);
    });
    t.join();
}

/**
 * @tc.name: ArkNativeReferenceTest001
 * @tc.desc: Test code of ArkNativeReference
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ArkNativeReferenceTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &value));
    ArkNativeReference *ref = nullptr;
    ASSERT_CHECK_CALL(napi_create_reference(env, value, 0, reinterpret_cast<napi_ref*>(&ref)));
    ASSERT_EQ(ref->properties_ & ArkNativeReference::DELETE_SELF_MASK, 0);
    ASSERT_EQ(ref->properties_ & ArkNativeReference::IS_ASYNC_CALL_MASK, 0);
    ASSERT_EQ(ref->properties_ & ArkNativeReference::HAS_DELETE_MASK, 0);
    ASSERT_EQ(ref->properties_ & ArkNativeReference::FINAL_RAN_MASK, 0);
    ASSERT_CHECK_CALL(napi_delete_reference(env, reinterpret_cast<napi_ref>(ref)));
}

/**
 * @tc.name: ArkNativeReferenceTest002
 * @tc.desc: Test code of ArkNativeReference
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ArkNativeReferenceTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &value));
    ArkNativeReference *ref = nullptr;
    ASSERT_CHECK_CALL(napi_create_reference(env, value, 0, reinterpret_cast<napi_ref*>(&ref)));
    ASSERT_CHECK_CALL(napi_delete_reference(env, reinterpret_cast<napi_ref>(ref)));
    // Weak ref will mark to delete self instead delete directly
    ASSERT_NE(ref->properties_ & ArkNativeReference::DELETE_SELF_MASK, 0);
    ASSERT_EQ(ref->properties_ & ArkNativeReference::IS_ASYNC_CALL_MASK, 0);
    ASSERT_EQ(ref->properties_ & ArkNativeReference::HAS_DELETE_MASK, 0);
    ASSERT_EQ(ref->properties_ & ArkNativeReference::FINAL_RAN_MASK, 0);
}

/**
 * @tc.name: ArkNativeReferenceTest003
 * @tc.desc: Test code of ArkNativeReference
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ArkNativeReferenceTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &value));
    ArkNativeReference** ref = new ArkNativeReference* { nullptr };
    ASSERT_CHECK_CALL(napi_add_finalizer(
        env, value, ref,
        // This callback is execution under deconstructor of ArkNativeReference
        [](napi_env env, void* data, void*) {
            ArkNativeReference** secondData = reinterpret_cast<ArkNativeReference**>(data);
            ArkNativeReference* ref = *secondData;
            ASSERT_EQ(ref->properties_ & ArkNativeReference::DELETE_SELF_MASK, 0);
            ASSERT_EQ(ref->properties_ & ArkNativeReference::IS_ASYNC_CALL_MASK, 0);
            ASSERT_NE(ref->properties_ & ArkNativeReference::HAS_DELETE_MASK, 0);
            ASSERT_EQ(ref->properties_ & ArkNativeReference::FINAL_RAN_MASK, 0);
            delete secondData;
        },
        nullptr, reinterpret_cast<napi_ref*>(ref)));
    ASSERT_EQ((*ref)->properties_ & ArkNativeReference::DELETE_SELF_MASK, 0);
    ASSERT_EQ((*ref)->properties_ & ArkNativeReference::IS_ASYNC_CALL_MASK, 0);
    ASSERT_EQ((*ref)->properties_ & ArkNativeReference::HAS_DELETE_MASK, 0);
    ASSERT_EQ((*ref)->properties_ & ArkNativeReference::FINAL_RAN_MASK, 0);
    ASSERT_CHECK_CALL(napi_delete_reference(env, reinterpret_cast<napi_ref>(*ref)));
}

/**
 * @tc.name: ArkNativeReferenceTest004
 * @tc.desc: Test code of ArkNativeReference
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ArkNativeReferenceTest004, testing::ext::TestSize.Level1)
{
    UVLoopRunner runner(engine_);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    ArkNativeReference** ref = new ArkNativeReference* { nullptr };
    {
        panda::LocalScope scope(engine_->GetEcmaVm());
        napi_value value = nullptr;
        ASSERT_CHECK_CALL(napi_create_object(env, &value));
        ASSERT_CHECK_CALL(napi_add_finalizer(
            env, value, ref,
            // This callback is execution under deconstructor of ArkNativeReference
            [](napi_env env, void* data, void*) {
                *reinterpret_cast<ArkNativeReference**>(data) = nullptr;
            },
            nullptr, nullptr));
        // Head is last reference which created above. 
        *ref = reinterpret_cast<ArkNativeReference*>(engine_->GetReferenceManager()->references_);
        ASSERT_NE((*ref)->properties_ & ArkNativeReference::DELETE_SELF_MASK, 0);
        ASSERT_EQ((*ref)->properties_ & ArkNativeReference::IS_ASYNC_CALL_MASK, 0);
        ASSERT_EQ((*ref)->properties_ & ArkNativeReference::HAS_DELETE_MASK, 0);
        ASSERT_EQ((*ref)->properties_ & ArkNativeReference::FINAL_RAN_MASK, 0);
    }
    ASSERT_NE(ref, nullptr);
    panda::JSNApi::TriggerGC(engine_->GetEcmaVm(), panda::ecmascript::GCReason::OTHER, panda::JSNApi::TRIGGER_GC_TYPE::FULL_GC);
    runner.Run();
    ASSERT_EQ(*ref, nullptr);
    delete ref;
}

/**
 * @tc.name: ArkNativeReferenceTest005
 * @tc.desc: Test code of ~ArkNativeReference in ~NativeEngine.
 *           This test case would crash if tests failed.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, ArkNativeReferenceTest005, testing::ext::TestSize.Level1)
{
    struct RefTestData {
        napi_env env_ { nullptr };
        napi_ref ref_ { nullptr };
    };

    RefTestData* testData = new RefTestData;
    {
        std::thread t([&testData]() {
            NativeEngineProxy env;
            testData->env_ = env;
            napi_value val = nullptr;
            ASSERT_CHECK_CALL(napi_create_object(env, &val));
            ASSERT_CHECK_CALL(napi_create_reference(env, val, 0, &testData->ref_));
            ASSERT_CHECK_CALL(napi_add_env_cleanup_hook(
                env,
                [](void* arg) {
                    RefTestData* data = reinterpret_cast<RefTestData*>(arg);
                    ASSERT_CHECK_CALL(napi_delete_reference(data->env_, data->ref_));
                    data->ref_ = nullptr;
                },
                testData));
        });
        t.join();
    }
    testData->env_ = nullptr;
    ASSERT_EQ(testData->ref_, nullptr);
    delete testData;
}

void TestQueueAsyncWorkWithQueue(NativeEngine* engine, napi_qos_t qos)
{
    UVLoopRunner runner(engine);
    napi_env env = (napi_env)engine;

    g_currentExeId = 0;
    g_currentCptId = 0;
    struct AsyncWorkContext {
        napi_async_work work = nullptr;
        int workid = 0;
    };
    std::unique_ptr<AsyncWorkContext> asyncWorkContext[ASYNC_WORK_NUM];
    for (int i = 0; i < ASYNC_WORK_NUM; ++i) {
        asyncWorkContext[i] = std::make_unique<AsyncWorkContext>();
        asyncWorkContext[i]->workid = i;
    }
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, TEST_CHAR_ASYNCWORK, NAPI_AUTO_LENGTH, &resourceName);
    for (int i = 0; i < ASYNC_WORK_NUM; ++i) {
        ASSERT_CHECK_CALL(napi_create_async_work(env, nullptr, resourceName,
            [](napi_env env, void *data) {
                auto asyncWorkContext = reinterpret_cast<AsyncWorkContext*>(data);
                ASSERT_EQ(g_currentExeId, asyncWorkContext->workid);
                g_currentExeId++;
            },
            [](napi_env env, napi_status status, void* data) {
                auto asyncWorkContext = reinterpret_cast<AsyncWorkContext*>(data);
                ASSERT_EQ(g_currentCptId, asyncWorkContext->workid);
                g_currentCptId++;
            },
            asyncWorkContext[i].get(), &asyncWorkContext[i]->work));
    }
    for (int i = 0; i < ASYNC_WORK_NUM; ++i) {
        ASSERT_CHECK_CALL(napi_queue_async_work_with_queue(env, asyncWorkContext[i]->work, qos, reinterpret_cast<uintptr_t>(&TASKID)));
    }

    runner.Run();
}

/**
 * @tc.name: NapiQueueAsyncWorkWithQueueTest001
 * @tc.desc: Test interface of napi_queue_async_work_with_queue
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiQueueAsyncWorkWithQueueTest001, testing::ext::TestSize.Level1)
{
    TestQueueAsyncWorkWithQueue(engine_, napi_qos_background);
}

/**
 * @tc.name: NapiQueueAsyncWorkWithQueueTest002
 * @tc.desc: Test interface of napi_queue_async_work_with_queue
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiQueueAsyncWorkWithQueueTest002, testing::ext::TestSize.Level1)
{
    TestQueueAsyncWorkWithQueue(engine_, napi_qos_utility);
}

/**
 * @tc.name: NapiQueueAsyncWorkWithQueueTest003
 * @tc.desc: Test interface of napi_queue_async_work_with_queue
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiQueueAsyncWorkWithQueueTest003, testing::ext::TestSize.Level1)
{
    TestQueueAsyncWorkWithQueue(engine_, napi_qos_default);
}

/**
 * @tc.name: NapiQueueAsyncWorkWithQueueTest004
 * @tc.desc: Test interface of napi_queue_async_work_with_queue
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiQueueAsyncWorkWithQueueTest004, testing::ext::TestSize.Level1)
{
    TestQueueAsyncWorkWithQueue(engine_, napi_qos_user_initiated);
}

/**
 * @tc.name: NapiQueueAsyncWorkWithQueueTest005
 * @tc.desc: Test napi_queue_async_work_with_queue when env is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiQueueAsyncWorkWithQueueTest005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_work work = nullptr;
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, TEST_CHAR_ASYNCWORK, NAPI_AUTO_LENGTH, &resourceName);
    ASSERT_CHECK_CALL(napi_create_async_work(env, nullptr, resourceName,
            [](napi_env env, void *data) {},
            [](napi_env env, napi_status status, void* data) {},
            nullptr, &work));

    auto res = napi_queue_async_work_with_queue(nullptr, work, napi_qos_default, reinterpret_cast<uintptr_t>(&TASKID));
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiQueueAsyncWorkWithQueueTest006
 * @tc.desc: Test napi_queue_async_work_with_queue when work is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiQueueAsyncWorkWithQueueTest006, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_queue_async_work_with_queue(env, nullptr, napi_qos_default, reinterpret_cast<uintptr_t>(&TASKID));
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiQueueAsyncWorkWithQueueTest007
 * @tc.desc: Test napi_queue_async_work_with_queue when taskId is 0
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiQueueAsyncWorkWithQueueTest007, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_work work = nullptr;
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, TEST_CHAR_ASYNCWORK, NAPI_AUTO_LENGTH, &resourceName);
    ASSERT_CHECK_CALL(napi_create_async_work(env, nullptr, resourceName,
            [](napi_env env, void *data) {},
            [](napi_env env, napi_status status, void* data) {},
            nullptr, &work));

    auto res = napi_queue_async_work_with_queue(env, work, napi_qos_default, 0);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiQueueAsyncWorkWithQueueTest008
 * @tc.desc: Test napi_queue_async_work_with_queue when napi_qos_t is invalid
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiQueueAsyncWorkWithQueueTest008, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_work work = nullptr;
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, TEST_CHAR_ASYNCWORK, NAPI_AUTO_LENGTH, &resourceName);
    ASSERT_CHECK_CALL(napi_create_async_work(env, nullptr, resourceName,
            [](napi_env env, void *data) {},
            [](napi_env env, napi_status status, void* data) {},
            nullptr, &work));

    auto res = napi_queue_async_work_with_queue(env, work, static_cast<napi_qos_t>(TEST_INT32_10), reinterpret_cast<uintptr_t>(&TASKID));
    ASSERT_EQ(res, napi_generic_failure);
}

/**
 * @tc.name: NapiQueueAsyncWorkWithQueueTest009
 * @tc.desc: Test interface of napi_queue_async_work_with_queue
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, NapiQueueAsyncWorkWithQueueTest009, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_async_work work = nullptr;
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, TEST_CHAR_ASYNCWORK, NAPI_AUTO_LENGTH, &resourceName);
    ASSERT_CHECK_CALL(napi_create_async_work(env, nullptr, resourceName,
            [](napi_env env, void *data) {},
            [](napi_env env, napi_status status, void* data) {},
            nullptr, &work));

    auto res = napi_queue_async_work_with_queue(env, work, napi_qos_default, reinterpret_cast<uintptr_t>(&TASKID));
    ASSERT_EQ(res, napi_ok);
}