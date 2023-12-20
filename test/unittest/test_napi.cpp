/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "test.h"
#include "test_common.h"
#include "gtest/gtest.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi/native_common.h"
#include "securec.h"
#include "utils/log.h"

static constexpr int THREAD_PAUSE_THREE = 300 * 1000; // 300 ms
static constexpr int THREAD_PAUSE_ONE = 100 * 1000; // 100 ms
static constexpr int MAX_BUFFER_SIZE = 2;
static constexpr int BUFFER_SIZE_FIVE = 5;
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

    void SetUp() override {}
    void TearDown() override {}
};

static const napi_type_tag typeTags[5] = { // 5:array element size is 5.
    {0xdaf987b3cc62481a, 0xb745b0497f299531},
    {0xbb7936c374084d9b, 0xa9548d0762eeedb9},
    {0xa5ed9ce2e4c00c38, 0},
    {0, 0},
    {0xa5ed9ce2e4c00c34, 0xdaf987b3cc62481a},
};

static void* TestDetachCallback(napi_env env, void* nativeObject, void* hint)
{
    HILOG_INFO("this is detach callback");
    return nativeObject;
}

static napi_value TestAttachCallback(napi_env env, void* nativeObject, void* hint)
{
    HILOG_INFO("this is attach callback");
    napi_value object = nullptr;
    napi_value number = nullptr;
    uint32_t data = 0;
    if (hint != nullptr) {
        object = reinterpret_cast<napi_value>(nativeObject);
        data = 2000;
        napi_create_uint32(env, data, &number);
    } else {
        napi_create_object(env, &object);
        data = 1000;
        napi_create_uint32(env, data, &number);
    }
    napi_set_named_property(env, object, "number", number);
    return object;
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
    napi_status status = napi_coerce_to_native_binding_object(
        env, object, TestDetachCallback, TestAttachCallback, reinterpret_cast<void*>(object), nullptr);
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
    napi_coerce_to_native_binding_object(
        env, object, TestDetachCallback, TestAttachCallback, reinterpret_cast<void*>(object), nullptr);
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    napi_value data = nullptr;
    napi_serialize(env, object, undefined, &data);
    ASSERT_NE(data, nullptr);
    napi_value result = nullptr;
    napi_deserialize(env, data, &result);
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_object);
    napi_delete_serialization_data(env, data);
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
    status = napi_coerce_to_native_binding_object(
        env, object, nullptr, nullptr, reinterpret_cast<void*>(object), nullptr);
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
    napi_status status = napi_coerce_to_native_binding_object(env, object,
        TestDetachCallback, TestAttachCallback, reinterpret_cast<void*>(object), reinterpret_cast<void*>(hint));
    ASSERT_EQ(status, napi_status::napi_ok);
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    napi_value data = nullptr;
    napi_serialize(env, object, undefined, &data);
    ASSERT_NE(data, nullptr);
    napi_value result = nullptr;
    napi_deserialize(env, data, &result);
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_object);
    napi_delete_serialization_data(env, data);
    napi_value number = nullptr;
    napi_get_named_property(env, result, "number", &number);
    ASSERT_CHECK_VALUE_TYPE(env, number, napi_number);
    uint32_t numData = 0;
    napi_get_value_uint32(env, number, &numData);
    ASSERT_EQ(numData, 2000);
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
    ASSERT_GT(bufferSize, (size_t)0);
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
    size_t testStrLength = sizeof(testStr) / sizeof(char16_t);
    napi_value res = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf16(env, testStr, testStrLength, &res));
    ASSERT_CHECK_VALUE_TYPE(env, res, napi_string);

    char16_t* buffer = nullptr;
    size_t bufSize = 0;
    size_t copied = 0;
    ASSERT_CHECK_CALL(napi_get_value_string_utf16(env, res, nullptr, 0, &bufSize));
    ASSERT_EQ(bufSize, 6);
    buffer = new char16_t[bufSize]{ 0 };
    ASSERT_CHECK_CALL(napi_get_value_string_utf16(env, res, buffer, bufSize, &copied));
    for (size_t i = 0; i < copied; i++) {
        ASSERT_TRUE(testStr[i] == buffer[i]);
    }
    ASSERT_EQ(testStrLength - 1, copied);
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
    size_t testStrLength = sizeof(testStr) / sizeof(char16_t);
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
        napi_create_typedarray(env, napi_int8_array, arrayBufferSize, arrayBuffer, 0, &typedarray);
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

        ASSERT_EQ(typedarrayBufferPtr, arrayBufferPtr);
        ASSERT_EQ(arrayBufferSize, typedarrayLength);
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
    ASSERT_EQ((size_t)0, byteOffset);
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
    {
        auto asyncWorkContext = new AsyncWorkContext();
        napi_value resourceName = nullptr;
        napi_create_string_utf8(env, "AsyncWorkTest", NAPI_AUTO_LENGTH, &resourceName);
        ASSERT_CHECK_CALL(napi_create_async_work(
            env, nullptr, resourceName, [](napi_env value, void* data) {},
            [](napi_env env, napi_status status, void* data) {
                AsyncWorkContext* asyncWorkContext = (AsyncWorkContext*)data;
                ASSERT_CHECK_CALL(napi_delete_async_work(env, asyncWorkContext->work));
                delete asyncWorkContext;
            },
            asyncWorkContext, &asyncWorkContext->work));
        ASSERT_CHECK_CALL(napi_queue_async_work(env, asyncWorkContext->work));
    }
    {
        auto asyncWorkContext = new AsyncWorkContext();
        napi_value resourceName = nullptr;
        napi_create_string_utf8(env, "AsyncWorkTest", NAPI_AUTO_LENGTH, &resourceName);
        napi_create_async_work(
            env, nullptr, resourceName, [](napi_env value, void* data) {},
            [](napi_env env, napi_status status, void* data) {
                AsyncWorkContext* asyncWorkContext = (AsyncWorkContext*)data;
                napi_delete_async_work(env, asyncWorkContext->work);
                delete asyncWorkContext;
            },
            asyncWorkContext, &asyncWorkContext->work);
        napi_queue_async_work(env, asyncWorkContext->work);
        ASSERT_CHECK_CALL(napi_cancel_async_work(env, asyncWorkContext->work));
    }
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
    napi_create_string_utf8(env, "AsyncWorkTest", NAPI_AUTO_LENGTH, &resourceName);
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
    napi_create_string_utf8(env, "AsyncWorkTest", NAPI_AUTO_LENGTH, &resourceName);
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
    napi_create_string_utf8(env, "AsyncWorkTest", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        env, nullptr, resourceName, [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {
            AsyncWorkContext* asyncWorkContext = reinterpret_cast<AsyncWorkContext*>(data);
            ASSERT_NE(asyncWorkContext, nullptr);
            delete asyncWorkContext;
        },
        asyncWorkContext, &asyncWorkContext->work);
    napi_status status = napi_queue_async_work(env, asyncWorkContext->work);
    ASSERT_EQ(status, napi_ok);
    status = napi_queue_async_work(env, nullptr);
    ASSERT_EQ(status, napi_invalid_arg);
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
 * @tc.name: CreateRuntimeTest001
 * @tc.desc: Test create runtime.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, CreateRuntimeTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_env newEnv = nullptr;
    napi_create_runtime(env, &newEnv);
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
    napi_value data = nullptr;
    napi_serialize(env, num, undefined, &data);
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
    napi_value data = nullptr;
    napi_serialize(env, num, undefined, &data);
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
    napi_status status = napi_coerce_to_native_binding_object(env, object,
        TestDetachCallback, TestAttachCallback, reinterpret_cast<void*>(object), reinterpret_cast<void*>(hint));
    ASSERT_EQ(status, napi_status::napi_ok);
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    napi_value data = nullptr;
    napi_serialize(env, object, undefined, &data);
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
    napi_value data = nullptr;
    napi_serialize(env, object, undefined, &data);
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
    int32_t written = 0;
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
 * @tc.name: SharedArrayBufferTest001
 * @tc.desc: Test is shared array buffer.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, SharedArrayBufferTest001, testing::ext::TestSize.Level1) {
    napi_env env = (napi_env) engine_;
    
    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    size_t arrayBufferSize = 1024;
    napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    bool isSharedArrayBuffer = true;
    napi_is_shared_array_buffer(env, arrayBuffer, &isSharedArrayBuffer);
    ASSERT_EQ(isSharedArrayBuffer, false);
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
 * @tc.name: OpenCallbackScopeTest001
 * @tc.desc: Test napi_open_callback_scope, napi_close_callback_scope.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, OpenCallbackScopeTest001, testing::ext::TestSize.Level1)
{
    napi_env envOne = reinterpret_cast<napi_env>(engine_);

    auto callbackScopeManager = engine_->GetCallbackScopeManager();
    ASSERT_NE(callbackScopeManager, nullptr);

    int openCallbackScopesBefore = callbackScopeManager->GetOpenCallbackScopes();
    int asyncCallbackScopeDepthBefore = callbackScopeManager->GetAsyncCallbackScopeDepth();

    napi_value resourceName;
    NAPI_CALL_RETURN_VOID(envOne, napi_create_string_utf8(envOne, "test", NAPI_AUTO_LENGTH, &resourceName));

    napi_async_context context;
    NAPI_CALL_RETURN_VOID(envOne, napi_async_init(envOne, nullptr, resourceName, &context));

    napi_callback_scope scope = nullptr;
    napi_status ret = napi_open_callback_scope(envOne, nullptr, context, &scope);
    EXPECT_EQ(ret, napi_ok);
    EXPECT_NE(scope, nullptr);

    int openCallbackScopes = callbackScopeManager->GetOpenCallbackScopes();
    int asyncCallbackScopeDepth = callbackScopeManager->GetAsyncCallbackScopeDepth();
    EXPECT_EQ(openCallbackScopes, (openCallbackScopesBefore + 1));
    EXPECT_EQ(asyncCallbackScopeDepth, (asyncCallbackScopeDepthBefore + 1));

    ret = napi_close_callback_scope(envOne, scope);
    EXPECT_EQ(ret, napi_ok);

    int openCallbackScopesAfter = callbackScopeManager->GetOpenCallbackScopes();
    int asyncCallbackScopeDepthAfter = callbackScopeManager->GetAsyncCallbackScopeDepth();
    EXPECT_EQ(openCallbackScopesAfter, openCallbackScopesBefore);
    EXPECT_EQ(asyncCallbackScopeDepthAfter, asyncCallbackScopeDepthBefore);

    NAPI_CALL_RETURN_VOID(envOne, napi_async_destroy(envOne, context));
}

/**
 * @tc.name: OpenCallbackScopeTest002
 * @tc.desc: Test napi_open_callback_scope, napi_close_callback_scope.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, OpenCallbackScopeTest002, testing::ext::TestSize.Level1)
{
    napi_env envOne = reinterpret_cast<napi_env>(engine_);

    auto callbackScopeManager = engine_->GetCallbackScopeManager();
    ASSERT_NE(callbackScopeManager, nullptr);

    int openCallbackScopesBefore = callbackScopeManager->GetOpenCallbackScopes();
    int asyncCallbackScopeDepthBefore = callbackScopeManager->GetAsyncCallbackScopeDepth();

    napi_value resourceName;
    NAPI_CALL_RETURN_VOID(envOne, napi_create_string_utf8(envOne, "test", NAPI_AUTO_LENGTH, &resourceName));

    napi_async_context context;
    NAPI_CALL_RETURN_VOID(envOne, napi_async_init(envOne, nullptr, resourceName, &context));

    napi_callback_scope scope = nullptr;
    napi_status res = napi_open_callback_scope(envOne, nullptr, context, &scope);
    EXPECT_EQ(res, napi_ok);
    EXPECT_NE(scope, nullptr);

    int openCallbackScopesOne = callbackScopeManager->GetOpenCallbackScopes();
    int asyncCallbackScopeDepthOne = callbackScopeManager->GetAsyncCallbackScopeDepth();

    // Open a internal callback scope
    auto scopeTwo = callbackScopeManager->Open(engine_);
    int openCallbackScopesTwo = callbackScopeManager->GetOpenCallbackScopes();
    int asyncCallbackScopeDepthTwo = callbackScopeManager->GetAsyncCallbackScopeDepth();

    EXPECT_NE(scopeTwo, nullptr);
    EXPECT_EQ(openCallbackScopesTwo, openCallbackScopesOne);
    EXPECT_EQ(asyncCallbackScopeDepthTwo, (asyncCallbackScopeDepthOne + 1));

    callbackScopeManager->Close(scopeTwo);
    int openCallbackScopesAfterTwo = callbackScopeManager->GetOpenCallbackScopes();
    int asyncCallbackScopeDepthAfterTwo = callbackScopeManager->GetAsyncCallbackScopeDepth();

    EXPECT_EQ(openCallbackScopesAfterTwo, openCallbackScopesOne);
    EXPECT_EQ(asyncCallbackScopeDepthAfterTwo, asyncCallbackScopeDepthOne);

    res = napi_close_callback_scope(envOne, scope);
    EXPECT_EQ(res, napi_ok);

    int openCallbackScopesAfter = callbackScopeManager->GetOpenCallbackScopes();
    int asyncCallbackScopeDepthAfter = callbackScopeManager->GetAsyncCallbackScopeDepth();

    EXPECT_EQ(openCallbackScopesAfter, openCallbackScopesBefore);
    EXPECT_EQ(asyncCallbackScopeDepthAfter, asyncCallbackScopeDepthBefore);

    NAPI_CALL_RETURN_VOID(envOne, napi_async_destroy(envOne, context));
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
 * @tc.name: AddEnvCleanupHook001
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
HWTEST_F(NapiBasicTest, AddEnvCleanupHook007, testing::ext::TestSize.Level2)
{
    napi_env testEnv = reinterpret_cast<napi_env>(engine_);
    g_hookTag = INT_ZERO;
    ExpectCheckCall(napi_add_env_cleanup_hook(testEnv, Cleanup, &g_hookArgOne));
    ExpectCheckCall(napi_add_env_cleanup_hook(testEnv, Cleanup, &g_hookArgOne));
    engine_->RunCleanup();
    EXPECT_EQ(g_hookTag, INT_ONE);
}

/**
 * @tc.name: AddEnvCleanupHook008
 * @tc.desc: Test napi_add_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AddEnvCleanupHook008, testing::ext::TestSize.Level1)
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
 * @tc.name: EnvCleanupHook009
 * @tc.desc: Test napi_add_env_cleanup_hook napi_remove_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, EnvCleanupHook009, testing::ext::TestSize.Level1)
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
 * @tc.name: EnvCleanupHook0010
 * @tc.desc: Test napi_add_env_cleanup_hook napi_remove_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, EnvCleanupHook0010, testing::ext::TestSize.Level2)
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
    res = napi_remove_env_cleanup_hook(env, nullptr, &g_hookArgTwo);
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
 * @tc.name: EnvCleanupHook0013
 * @tc.desc: Test napi_add_env_cleanup_hook napi_remove_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, EnvCleanupHook0013, testing::ext::TestSize.Level2)
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
 * @tc.name: EnvCleanupHook0014
 * @tc.desc: Test napi_add_env_cleanup_hook napi_remove_env_cleanup_hook
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, EnvCleanupHook0014, testing::ext::TestSize.Level1)
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
    sleep(1);
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
    sleep(1);
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
    sleep(1);
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
    sleep(1);
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
    sleep(1);
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
    sleep(1);
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
    sleep(1);
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
    sleep(1);
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
    sleep(2);
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
    sleep(1);
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
    sleep(1);
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
 * @tc.name: AsyncWorkTest002
 * @tc.desc: Test async work.
 * @tc.type: FUNC
 */
HWTEST_F(NapiBasicTest, AsyncWorkTest002, testing::ext::TestSize.Level1)
{
    struct AsyncWorkContext {
        napi_async_work work = nullptr;
    };
    napi_env env = reinterpret_cast<napi_env>(engine_);
    auto asyncWorkContext = new AsyncWorkContext();
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, "AsyncWorkTest", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(
        env, nullptr, resourceName, [](napi_env value, void* data) {
            // Simulate long-term running tasks.
            usleep(THREAD_PAUSE_THREE);
        },
        [](napi_env env, napi_status status, void* data) {
            AsyncWorkContext* asyncWorkContext = (AsyncWorkContext*)data;
            ASSERT_EQ(status, napi_status::napi_cancelled);
            napi_delete_async_work(env, asyncWorkContext->work);
            delete asyncWorkContext;
        },
        asyncWorkContext, &asyncWorkContext->work);
    napi_queue_async_work(env, asyncWorkContext->work);

    // Sleep for a short duration to allow the async work to start executing.
    usleep(THREAD_PAUSE_ONE);
    napi_cancel_async_work(env, asyncWorkContext->work);
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
    napi_value val_false;
    napi_value val_true;
    ASSERT_CHECK_CALL(napi_get_boolean(env, false, &val_false));
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &val_true));
    napi_property_descriptor desc1[] = {
        DECLARE_NAPI_PROPERTY("x", val_true),
    };
    napi_value obj1;
    ASSERT_CHECK_CALL(napi_create_object_with_properties(env, &obj1, 1, desc1));
    napi_value obj2;
    napi_property_descriptor desc2[] = {
        DECLARE_NAPI_PROPERTY("a", val_false),
        DECLARE_NAPI_GETTER_SETTER("b", CreateWithPropertiesTestGetter, CreateWithPropertiesTestSetter),
        DECLARE_NAPI_PROPERTY("c", obj1),
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