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

#include "test.h"
#include "napi/native_api.h"

static constexpr int INT_ZERO = 0;
static constexpr int INT_ONE = 1;
static constexpr int INT_TWO = 2;
static constexpr int INT_THREE = 3;
static constexpr double TEST_DOUBLE = 1.1;
static constexpr char TEST_STRING[5] = "test";

class NapiErrorCodeTest : public NativeEngineTest {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "NapiErrorCodeTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "NapiErrorCodeTest TearDownTestCase";
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

void AssertFalse()
{
    ASSERT_TRUE(false);
}

napi_value TestAbort(napi_env env, napi_callback_info info)
{
    AssertFalse();
    return nullptr;
}

/**
 * @tc.name: NapiFatalExceptionTest
 * @tc.desc: Test interface of napi_coerce_to_bool
 * @tc.type: FUNC
 */
HWTEST_F(NapiErrorCodeTest, NapiCoerceToBoolTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    napi_value result;
    napi_status status = napi_coerce_to_bool(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiCoerceToBoolTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value *result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_coerce_to_bool(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

/**
 * @tc.name: NapiFatalExceptionTest
 * @tc.desc: Test interface of napi_coerce_to_bool
 * @tc.type: FUNC
 */
HWTEST_F(NapiErrorCodeTest, NapiCoerceToBoolTest003, testing::ext::TestSize.Level1)
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
HWTEST_F(NapiErrorCodeTest, NapiCoerceToBoolTest004, testing::ext::TestSize.Level1)
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
HWTEST_F(NapiErrorCodeTest, NapiCoerceToBoolTest005, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiCoerceToNumberTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    napi_value result;
    napi_status status = napi_coerce_to_number(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiCoerceToNumberTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value *result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_coerce_to_number(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiCoerceToNumberTest003, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiCoerceToNumberTest004, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiCoerceToNumberTest005, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiCoerceToNumberTest006, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiCoerceToNumberTest007, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiCoerceToObjectTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    napi_value result;
    napi_status status = napi_coerce_to_object(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiCoerceToObjectTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value *result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_coerce_to_object(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiCoerceToObjectTest003, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiCoerceToObjectTest004, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiCoerceToStringTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    napi_value result;
    napi_status status = napi_coerce_to_string(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiCoerceToStringTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value *result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_coerce_to_string(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiCoerceToStringTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    constexpr char codeStr[] = "test code";
    napi_status status = napi_create_string_utf8(env, codeStr, NAPI_AUTO_LENGTH, &value);
    ASSERT_EQ(status, napi_ok);
    napi_value *result = nullptr;
    status = napi_coerce_to_string(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiTypeofTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    napi_valuetype result;
    napi_status status = napi_typeof(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiTypeofTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_valuetype *result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_typeof(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiInstanceofTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    napi_value constructor;
    bool result;
    napi_status status = napi_create_object(env, &constructor);
    status = napi_instanceof(env, value, constructor, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiInstanceofTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value constructor = nullptr;
    bool result;
    napi_status status = napi_create_object(env, &value);
    status = napi_instanceof(env, value, constructor, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiInstanceofTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value constructor;
    bool *result = nullptr;
    napi_status status = napi_create_object(env, &value);
    status = napi_create_object(env, &constructor);
    status = napi_instanceof(env, value, constructor, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiInstanceofTest004, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiInstanceofTest005, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiIsArrayTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    bool result;
    napi_status status = napi_is_array(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiIsArrayTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    bool *result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_is_array(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiIsArrayTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    bool *result = nullptr;
    napi_status status = napi_create_array(env, &value);
    status = napi_is_array(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiIsArrayTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    bool *result = nullptr;
    napi_status status = napi_create_array_with_length(env, INT_THREE, &value);
    status = napi_is_array(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiIsArrayBufferTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    bool result;
    napi_status status = napi_is_arraybuffer(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiIsArrayBufferTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    bool *result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_is_arraybuffer(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiIsArrayBufferTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    bool *result = nullptr;
    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    size_t arrayBufferSize = 0; // 0: define initial length
    napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);
    napi_status status = napi_is_arraybuffer(env, arrayBuffer, result);
    ASSERT_FALSE(result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiIsTypeBufferTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    bool result;
    napi_status status = napi_is_typedarray(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiIsTypeBufferTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    bool *result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_is_typedarray(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiIsTypeBufferTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    bool *result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_is_typedarray(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiIsDataViewTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    bool result;
    napi_status status = napi_is_dataview(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiIsDataViewTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    bool *result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_is_dataview(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiIsDateTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    bool result;
    napi_status status = napi_is_date(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiIsDateTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    bool *result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_is_date(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiStrictEqualsTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value lhs = nullptr;
    napi_value rhs;
    bool result;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &rhs);
    status = napi_strict_equals(env, lhs, rhs, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiStrictEqualsTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value lhs;
    napi_value rhs = nullptr;
    bool result;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &lhs);
    status = napi_strict_equals(env, lhs, rhs, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiStrictEqualsTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value lhs;
    napi_value rhs;
    bool *result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &lhs);
    status = napi_create_double(env, TEST_DOUBLE, &rhs);
    status = napi_strict_equals(env, lhs, rhs, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiStrictEqualsTest004, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiStrictEqualsTest005, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiGetPropertyNamesTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value = nullptr;
    napi_value result;
    napi_status status = napi_get_property_names(env, value, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiGetPropertyNamesTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value *result = nullptr;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_get_property_names(env, value, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiGetPropertyNamesTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value result;
    napi_status status = napi_create_double(env, TEST_DOUBLE, &value);
    status = napi_get_property_names(env, value, &result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiGetPropertyNamesTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value value;
    napi_value result;
    napi_status status = napi_create_int32(env, INT_ZERO, &value);
    status = napi_get_property_names(env, value, &result);
    ASSERT_NE(value, result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiSetPropertyTest001, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiSetPropertyTest002, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiSetPropertyTest003, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiSetPropertyTest004, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiSetPropertyTest005, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiGetPropertyTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_value key;
    napi_value result;

    napi_create_int32(env, INT_ONE, &key);
    napi_status status = napi_get_property(env, obj, key, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiGetPropertyTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key = nullptr;
    napi_value result;

    napi_create_object(env, &obj);
    napi_status status = napi_get_property(env, obj, key, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiGetPropertyTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key;
    napi_value *result = nullptr;

    napi_create_object(env, &obj);
    napi_create_int32(env, INT_ONE, &key);
    napi_status status = napi_get_property(env, obj, key, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiGetPropertyTest004, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiGetPropertyTest005, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiHasPropertyTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_value key;
    bool result;

    napi_create_int32(env, INT_ONE, &key);
    napi_status status = napi_has_property(env, obj, key, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiHasPropertyTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key = nullptr;
    bool result;

    napi_create_object(env, &obj);
    napi_status status = napi_has_property(env, obj, key, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiHasPropertyTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key;
    bool *result = nullptr;

    napi_create_object(env, &obj);
    napi_create_int32(env, INT_ONE, &key);
    napi_status status = napi_has_property(env, obj, key, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiHasPropertyTest004, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiHasPropertyTest005, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiDeletePropertyTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_value key;
    bool result;

    napi_create_int32(env, INT_ONE, &key);
    napi_status status = napi_delete_property(env, obj, key, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiDeletePropertyTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key = nullptr;
    bool result;

    napi_create_object(env, &obj);
    napi_status status = napi_delete_property(env, obj, key, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiDeletePropertyTest004, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiDeletePropertyTest005, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiHasOwnPropertyTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_value key;
    bool result;

    napi_create_int32(env, INT_ONE, &key);
    napi_status status = napi_has_own_property(env, obj, key, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiHasOwnPropertyTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key = nullptr;
    bool result;

    napi_create_object(env, &obj);
    napi_status status = napi_has_own_property(env, obj, key, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiHasOwnPropertyTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value key;
    bool *result = nullptr;

    napi_create_object(env, &obj);
    napi_create_int32(env, INT_ONE, &key);
    napi_status status = napi_has_own_property(env, obj, key, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiHasOwnPropertyTest004, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiHasOwnPropertyTest005, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiSetNamedPropertyTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_value value;

    napi_create_int32(env, INT_TWO, &value);
    napi_status status = napi_set_named_property(env, obj, TEST_STRING, value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiSetNamedPropertyTest002, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiSetNamedPropertyTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value value = nullptr;

    napi_create_object(env, &obj);
    napi_status status = napi_set_named_property(env, obj, TEST_STRING, value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiSetNamedPropertyTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value value;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    napi_create_int32(env, INT_TWO, &value);
    status = napi_set_named_property(env, obj, TEST_STRING, value);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiSetNamedPropertyTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value value;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    napi_create_int32(env, INT_THREE, &value);
    status = napi_set_named_property(env, obj, TEST_STRING, value);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiGetNamedPropertyTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_value value;

    napi_status status = napi_get_named_property(env, obj, TEST_STRING, &value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiGetNamedPropertyTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    char* utf8name = nullptr;
    napi_value value;

    napi_create_object(env, &obj);
    napi_status status = napi_get_named_property(env, obj, utf8name, &value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiGetNamedPropertyTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value *value = nullptr;

    napi_create_object(env, &obj);
    napi_status status = napi_get_named_property(env, obj, TEST_STRING, value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiGetNamedPropertyTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value value;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    status = napi_get_named_property(env, obj, TEST_STRING, &value);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiGetNamedPropertyTest005, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiHasNamedPropertyTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    bool result;

    napi_status status = napi_has_named_property(env, obj, TEST_STRING, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiHasNamedPropertyTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    char* utf8name = nullptr;
    bool result;

    napi_create_object(env, &obj);
    napi_status status = napi_has_named_property(env, obj, utf8name, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiHasNamedPropertyTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    bool *result = nullptr;

    napi_create_object(env, &obj);
    napi_status status = napi_has_named_property(env, obj, TEST_STRING, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiHasNamedPropertyTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    bool result;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    status = napi_has_named_property(env, obj, TEST_STRING, &result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiHasNamedPropertyTest006, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value global, globalThis, proxy_constructor, proxy;
    napi_get_global(env, &global);
    ASSERT_CHECK_CALL(napi_get_named_property(env, global, "globalThis", &globalThis));
    ASSERT_CHECK_CALL(napi_get_named_property(env, global, "Proxy", &proxy_constructor));
    napi_value obj;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));
    napi_value args[2] = {obj, obj};
    ASSERT_CHECK_CALL(napi_new_instance(env, proxy_constructor, 2, args, &proxy));

    bool result = false;
    napi_status status = napi_has_named_property(env, proxy, "toString", &result);
    ASSERT_EQ(result, true);
    ASSERT_EQ(status, napi_ok);
}

HWTEST_F(NapiErrorCodeTest, NapiHasNamedPropertyTest005, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiSetElementTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    uint32_t index = 1;
    napi_value value;

    napi_create_int32(env, INT_TWO, &value);
    napi_status status = napi_set_element(env, obj, index, value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiSetElementTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    uint32_t index = 1;
    napi_value value = nullptr;

    napi_create_object(env, &obj);
    napi_status status = napi_set_element(env, obj, index, value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiSetElementTest003, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiSetElementTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_value value;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    napi_create_int32(env, INT_TWO, &value);
    status = napi_set_element(env, obj, INT_ONE, value);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiGetElementTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    uint32_t index = 1;
    napi_value value;

    napi_status status = napi_get_element(env, obj, index, &value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiGetElementTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    uint32_t index = 1;
    napi_value *value = nullptr;

    napi_create_object(env, &obj);
    napi_status status = napi_get_element(env, obj, index, value);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiGetElementTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    uint32_t index = 1;
    napi_value value;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    status = napi_get_element(env, obj, index, &value);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiGetElementTest004, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiHasElementTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    uint32_t index = 1;
    bool result;

    napi_status status = napi_has_element(env, obj, index, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiHasElementTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    uint32_t index = 1;
    bool *result = nullptr;

    napi_create_object(env, &obj);
    napi_status status = napi_has_element(env, obj, index, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiHasElementTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    uint32_t index = 1;
    bool result;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    status = napi_has_element(env, obj, index, &result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiHasElementTest004, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiDeleteElementTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    uint32_t index = 1;
    bool result;

    napi_status status = napi_delete_element(env, obj, index, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiDeleteElementTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    uint32_t index = 1;
    bool result;

    napi_status status = napi_create_double(env, TEST_DOUBLE, &obj);
    status = napi_delete_element(env, obj, index, &result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiDeleteElementTest003, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiDefinePropertiesTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_property_descriptor desc[] = {
        {"testMethod", nullptr, [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
         nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    napi_value result = nullptr;

    napi_status status = napi_define_properties(env, result, sizeof(desc)/sizeof(desc[0]), desc);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiDefinePropertiesTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_property_descriptor *desc = nullptr;
    napi_value result;
    napi_create_object(env, &result);

    napi_status status = napi_define_properties(env, result, INT_ONE, desc);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiDefinePropertiesTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_property_descriptor desc[] = {
        {"testMethod", nullptr, [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
         nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    napi_value result;
    napi_create_double(env, TEST_DOUBLE, &result);

    napi_status status = napi_define_properties(env, result, INT_ONE, desc);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiDefinePropertiesTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_property_descriptor desc[] = {
        {nullptr, nullptr, [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
         nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    napi_value result;
    napi_create_object(env, &result);

    napi_status status = napi_define_properties(env, result, INT_ONE, desc);
    ASSERT_EQ(status, napi_name_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiDefinePropertiesTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value name;
    napi_create_object(env, &name);
    napi_property_descriptor desc[] = {
        {nullptr, name, [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
         nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    napi_value result;
    napi_create_object(env, &result);

    napi_status status = napi_define_properties(env, result, INT_ONE, desc);
    ASSERT_EQ(status, napi_name_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiDefinePropertiesTest006, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value func1 = nullptr;
    napi_status status = napi_create_function(env, "testAbort", NAPI_AUTO_LENGTH,
                                              TestAbort, nullptr, &func1);
    ASSERT_TRUE(status == napi_ok);
    napi_property_descriptor properties[] = {
        // napi_default_jsproperty = napi_writable | napi_enumerable | napi_configurable
        {"testAbort", nullptr, nullptr, nullptr, nullptr, func1, napi_default, nullptr}
    };
    napi_value globalObj = nullptr;
    napi_get_global(env, &globalObj);
    status = napi_define_properties(env, globalObj, sizeof(properties) / sizeof(properties[0]), properties);
    ASSERT_TRUE(status == napi_ok);
}

HWTEST_F(NapiErrorCodeTest, NapiTypeTagObjectTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_type_tag tag;

    napi_status status = napi_type_tag_object(env, obj, &tag);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiTypeTagObjectTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_type_tag* tag = nullptr;
    napi_create_object(env, &obj);

    napi_status status = napi_type_tag_object(env, obj, tag);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiTypeTagObjectTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_type_tag tag;
    napi_create_double(env, TEST_DOUBLE, &obj);

    napi_status status = napi_type_tag_object(env, obj, &tag);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiTypeTagObjectTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_type_tag tag;
    napi_create_int32(env, INT_ONE, &obj);

    napi_status status = napi_type_tag_object(env, obj, &tag);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiCheckObjectTypeTagTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_type_tag tag;
    bool result;

    napi_status status = napi_check_object_type_tag(env, obj, &tag, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiCheckObjectTypeTagTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_type_tag *tag = nullptr;
    bool result;
    napi_create_object(env, &obj);

    napi_status status = napi_check_object_type_tag(env, obj, tag, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiCheckObjectTypeTagTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_type_tag tag;
    bool *result = nullptr;
    napi_create_object(env, &obj);

    napi_status status = napi_check_object_type_tag(env, obj, &tag, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiCheckObjectTypeTagTest004, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiCallFunctionTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value funcValue = nullptr;
    napi_value recv = nullptr;
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_value funcResultValue = nullptr;

    napi_status status = napi_call_function(env, recv, funcValue, argc, args, &funcResultValue);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiCallFunctionTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value funcValue = nullptr;
    napi_value recv = nullptr;
    size_t argc = 1;
    napi_value* args = nullptr;
    napi_value funcResultValue = nullptr;

    napi_create_function(env, "testFunc", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }, nullptr, &funcValue);
    napi_status status = napi_call_function(env, recv, funcValue, argc, args, &funcResultValue);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiCallFunctionTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value funcValue = nullptr;
    napi_value recv = nullptr;
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_value funcResultValue = nullptr;

    napi_create_object(env, &funcValue);
    napi_status status = napi_call_function(env, recv, funcValue, argc, args, &funcResultValue);
    ASSERT_EQ(status, napi_function_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiCallFunctionTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value funcValue = nullptr;
    napi_value recv = nullptr;
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_value funcResultValue = nullptr;

    napi_create_function(env, "testFunc", NAPI_AUTO_LENGTH, [](napi_env env, napi_callback_info info) -> napi_value {
            napi_throw_error(env, "500", "Common error");
            return nullptr;
        }, nullptr, &funcValue);
    napi_status status = napi_call_function(env, recv, funcValue, argc, args, &funcResultValue);
    ASSERT_EQ(status, napi_pending_exception);
}

HWTEST_F(NapiErrorCodeTest, NapiCreateFunctionTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value funcValue = nullptr;

    napi_status status = napi_create_function(env, "testFunc", NAPI_AUTO_LENGTH, nullptr, nullptr, &funcValue);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiCreateFunctionTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value *funcValue = nullptr;

    napi_status status = napi_create_function(env, "testFunc", NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; }, nullptr, funcValue);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiGetCbInfoTest001, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiGetCbInfoTest002, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiGetNewTargetTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_callback_info info = nullptr;
    napi_value result;

    napi_status status = napi_get_new_target(env, info, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiGetNewTargetTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_callback_info info = napi_callback_info(nullptr);;
    napi_value* result = nullptr;

    napi_status status = napi_get_new_target(env, info, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiGetNewTargetTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_callback_info info = napi_callback_info(nullptr);;
    napi_value result = nullptr;
    napi_create_int32(env, INT_ONE, &result);

    napi_status status = napi_get_new_target(env, info, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiGetNewTargetTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_callback_info info = napi_callback_info(nullptr);;
    napi_value result = nullptr;
    napi_create_double(env, TEST_DOUBLE, &result);

    napi_status status = napi_get_new_target(env, info, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiGetNewTargetTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_callback_info info = napi_callback_info(nullptr);;
    napi_value result = nullptr;
    napi_create_string_utf8(env, TEST_STRING, 4, &result);

    napi_status status = napi_get_new_target(env, info, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiNewInstanceTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value constructor = nullptr;
    size_t argc = 0;
    napi_value args[1] = {nullptr};
    napi_value result;

    napi_status status = napi_new_instance(env, constructor, argc, args, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiNewInstanceTest002, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiNewInstanceTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value constructor;
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_value* result = nullptr;

    napi_create_object(env, &constructor);
    napi_status status = napi_new_instance(env, constructor, argc, args, result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiNewInstanceTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value constructor;
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_value result;

    napi_create_object(env, &constructor);
    napi_status status = napi_new_instance(env, constructor, argc, args, &result);
    ASSERT_EQ(status, napi_function_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiDefineClassTest001, testing::ext::TestSize.Level1)
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


HWTEST_F(NapiErrorCodeTest, NapiDefineClassTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value result;
    napi_status status = napi_define_class(
        env, "TestClass", NAPI_AUTO_LENGTH,
        nullptr, nullptr, 0, nullptr, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiDefineClassTest003, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiDefineClassTest004, testing::ext::TestSize.Level1)
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

HWTEST_F(NapiErrorCodeTest, NapiWrapTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    napi_ref result;

    napi_status status = napi_wrap(env, obj, (void *)TEST_STRING,
        [](napi_env, void* data, void* hint) {}, nullptr, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiWrapTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_ref result;

    napi_create_object(env, &obj);
    napi_status status = napi_wrap(env, obj, nullptr, [](napi_env, void* data, void* hint) {}, nullptr, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiWrapTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_ref result;

    napi_create_object(env, &obj);
    napi_status status = napi_wrap(env, obj, (void *)TEST_STRING, nullptr, nullptr, &result);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiWrapTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_ref result;

    napi_create_double(env, TEST_DOUBLE, &obj);
    napi_status status = napi_wrap(env, obj, (void *)TEST_STRING,
        [](napi_env, void* data, void* hint) {}, nullptr, &result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiWrapTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_ref result;

    napi_create_int32(env, INT_ONE, &obj);
    napi_status status = napi_wrap(env, obj, (void *)TEST_STRING,
        [](napi_env, void* data, void* hint) {}, nullptr, &result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiWrapTest006, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    napi_ref result;

    napi_create_int64(env, INT_TWO, &obj);
    napi_status status = napi_wrap(env, obj, (void *)TEST_STRING,
        [](napi_env, void* data, void* hint) {}, nullptr, &result);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiUnwrapTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    char *testStr = nullptr;

    napi_status status = napi_unwrap(env, obj, (void **)&testStr);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiUnwrapTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    char **testStr = nullptr;

    napi_create_object(env, &obj);
    napi_status status = napi_unwrap(env, obj, (void **)testStr);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiUnwrapTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    char *testStr = nullptr;

    napi_create_double(env, TEST_DOUBLE, &obj);
    napi_status status = napi_unwrap(env, obj, (void **)&testStr);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiUnwrapTest004, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    char *testStr = nullptr;

    napi_create_int32(env, INT_ONE, &obj);
    napi_status status = napi_unwrap(env, obj, (void **)&testStr);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiUnwrapTest005, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    char *testStr = nullptr;

    napi_create_int64(env, INT_TWO, &obj);
    napi_status status = napi_unwrap(env, obj, (void **)&testStr);
    ASSERT_EQ(status, napi_object_expected);
}

HWTEST_F(NapiErrorCodeTest, NapiUnwrapTest006, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    char **testStr = nullptr;
    auto func = [](napi_env env, napi_callback_info info) -> napi_value {
        return nullptr;
    };

    napi_create_function(env, nullptr, 0, func, nullptr, &obj);
    napi_status status = napi_unwrap(env, obj, (void **)testStr);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiRemoveWrapTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    char *testStr = nullptr;

    napi_status status = napi_remove_wrap(env, obj, (void **)&testStr);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiRemoveWrapTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj;
    char **testStr = nullptr;

    napi_create_object(env, &obj);
    napi_status status = napi_remove_wrap(env, obj, (void **)testStr);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiErrorCodeTest, NapiRemoveWrapTest003, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value obj = nullptr;
    char *testStr = nullptr;

    napi_create_double(env, TEST_DOUBLE, &obj);
    napi_status status = napi_remove_wrap(env, obj, (void **)&testStr);
    ASSERT_EQ(status, napi_object_expected);
}