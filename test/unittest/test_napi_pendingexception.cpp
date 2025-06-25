/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "napi/native_api.h"
#include "native_utils.h"
#include "test.h"

constexpr const int32_t LENGTH = 1024;
static constexpr int INT_ONE = 1;
static constexpr int INT_TWO = 2;
static constexpr int INT_FOUR = 4;
static constexpr int BUFF_SIZE = 20;
static constexpr const char TEST_CHAR_STRING[] = "TestString";
static constexpr const char TEST_CHAR_ERROR_MESSAGE[] = "Common error";
static constexpr const char TEST_CHAR_ERROR_CODE[] = "500";
static constexpr const char TEST_CHAR_ERROR_CODE_KEY[] = "code";
static constexpr const char TEST_CHAR_ERROR_MESSAGE_KEY[] = "message";

class NapiPendingExceptionTest : public NativeEngineTest {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "NapiPendingExceptionTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "NapiPendingExceptionTest TearDownTestCase";
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

    inline void ConstructionException()
    {
        napi_env env = reinterpret_cast<napi_env>(engine_);
        bool isExceptionPending = false;
        ASSERT_CHECK_CALL(napi_throw_error(env, nullptr, TEST_CHAR_ERROR_MESSAGE));
        ASSERT_CHECK_CALL(napi_is_exception_pending(env, &isExceptionPending));
        ASSERT_TRUE(isExceptionPending);
        engine_->lastException_ = panda::JSNApi::GetUncaughtException(engine_->GetEcmaVm());
    }

    inline void DestructionException()
    {
        napi_env env = reinterpret_cast<napi_env>(engine_);
        napi_value result = nullptr;
        ASSERT_CHECK_CALL(napi_get_and_clear_last_exception(env, &result));
    }

private:
    napi_handle_scope scope_ = nullptr;
};

HWTEST_F(NapiPendingExceptionTest, NapiLoadModuleWithInfoTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    ConstructionException();
    napi_value result = nullptr;
    napi_status res = napi_load_module_with_info(env, nullptr, nullptr, &result);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiCreateArraybufferTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    ConstructionException();
    napi_value arrayBuffer = nullptr;
    void *arrayBufferPtr = nullptr;
    napi_status res = napi_create_arraybuffer(env, LENGTH, &arrayBufferPtr, &arrayBuffer);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiCreateExternalArraybufferTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    ConstructionException();
    napi_value external = nullptr;
    napi_status res = napi_create_external_arraybuffer(
        env,
        (void *)TEST_CHAR_STRING,
        strlen(TEST_CHAR_STRING),
        [](napi_env env, void *data, void *hint) { ASSERT_STREQ((const char *)data, (const char *)hint); },
        (void *)TEST_CHAR_STRING,
        &external);
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiCreateTypedarrayTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value arrayBuffer = nullptr;
    void *arrayBufferPtr = nullptr;
    ASSERT_CHECK_CALL(napi_create_arraybuffer(env, LENGTH, &arrayBufferPtr, &arrayBuffer));

    ConstructionException();
    napi_value typedArray = nullptr;
    size_t byteOffsetCreate = 0;
    napi_status res = napi_create_typedarray(env, napi_int8_array, LENGTH, arrayBuffer, byteOffsetCreate, &typedArray);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiCreateDataviewTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value arrayBuffer = nullptr;
    void *arrayBufferPtr = nullptr;
    ASSERT_CHECK_CALL(napi_create_arraybuffer(env, LENGTH, &arrayBufferPtr, &arrayBuffer));

    ConstructionException();
    napi_value result = nullptr;
    napi_status res = napi_create_dataview(env, LENGTH, arrayBuffer, 0, &result);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiGetArrayLengthTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value array = nullptr;
    ASSERT_CHECK_CALL(napi_create_array(env, &array));

    ConstructionException();
    uint32_t arrayLength = 0;
    napi_status res = napi_get_array_length(env, array, &arrayLength);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiGetPrototypeTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));

    ConstructionException();
    napi_value result = nullptr;
    napi_status res = napi_get_prototype(env, object, &result);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiSetPropertyTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value obj = nullptr;
    napi_value key = nullptr;
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));
    ASSERT_CHECK_CALL(napi_create_int32(env, INT_ONE, &key));
    ASSERT_CHECK_CALL(napi_create_int32(env, INT_TWO, &value));

    ConstructionException();
    napi_status res = napi_set_property(env, obj, key, value);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiGetPropertyTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value obj = nullptr;
    napi_value key = nullptr;
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));
    ASSERT_CHECK_CALL(napi_create_int32(env, INT_ONE, &key));
    ASSERT_CHECK_CALL(napi_create_int32(env, INT_TWO, &value));
    ASSERT_CHECK_CALL(napi_set_property(env, obj, key, value));

    ConstructionException();
    napi_value result = nullptr;
    napi_status res = napi_get_property(env, obj, key, &result);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiHasPropertyTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value obj = nullptr;
    napi_value key = nullptr;
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));
    ASSERT_CHECK_CALL(napi_create_int32(env, INT_ONE, &key));
    ASSERT_CHECK_CALL(napi_create_int32(env, INT_TWO, &value));
    ASSERT_CHECK_CALL(napi_set_property(env, obj, key, value));

    ConstructionException();
    bool result = false;
    napi_status res = napi_has_property(env, obj, key, &result);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiDeletePropertyTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value obj = nullptr;
    napi_value key = nullptr;
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));
    ASSERT_CHECK_CALL(napi_create_int32(env, INT_ONE, &key));
    ASSERT_CHECK_CALL(napi_create_int32(env, INT_TWO, &value));
    ASSERT_CHECK_CALL(napi_set_property(env, obj, key, value));

    ConstructionException();
    bool result = false;
    napi_status res = napi_delete_property(env, obj, key, &result);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiHasOwnPropertyTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value obj = nullptr;
    napi_value key = nullptr;
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));
    ASSERT_CHECK_CALL(napi_create_int32(env, INT_ONE, &key));
    ASSERT_CHECK_CALL(napi_create_int32(env, INT_TWO, &value));
    ASSERT_CHECK_CALL(napi_set_property(env, obj, key, value));

    ConstructionException();
    bool result = false;
    napi_status res = napi_has_own_property(env, obj, key, &result);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiSetNamedPropertyTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value obj = nullptr;
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));
    ASSERT_CHECK_CALL(napi_create_int32(env, INT_TWO, &value));

    ConstructionException();
    napi_status res = napi_set_named_property(env, obj, TEST_CHAR_STRING, value);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiGetNamedPropertyTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value obj = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));

    ConstructionException();
    napi_value value = nullptr;
    napi_status res = napi_get_named_property(env, obj, TEST_CHAR_STRING, &value);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiHasNamedPropertyTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value obj = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));

    ConstructionException();
    bool value = false;
    napi_status res = napi_has_named_property(env, obj, TEST_CHAR_STRING, &value);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiSetElementTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value array = nullptr;
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_array(env, &array));
    ASSERT_CHECK_CALL(napi_create_int32(env, INT_ONE, &value));

    ConstructionException();
    napi_status res = napi_set_element(env, array, 0, value);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiGetElementTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value array = nullptr;
    ASSERT_CHECK_CALL(napi_create_array(env, &array));

    ConstructionException();
    napi_value value = nullptr;
    napi_status res = napi_get_element(env, array, 0, &value);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiHasElementTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value array = nullptr;
    ASSERT_CHECK_CALL(napi_create_array(env, &array));

    ConstructionException();
    bool value = false;
    napi_status res = napi_has_element(env, array, 0, &value);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiDeleteElementTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value array = nullptr;
    ASSERT_CHECK_CALL(napi_create_array(env, &array));

    ConstructionException();
    bool value = false;
    napi_status res = napi_delete_element(env, array, 0, &value);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiCreateFunctionTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto func = [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; };

    ConstructionException();
    napi_value jsCallback = nullptr;
    napi_status res = napi_create_function(env, TEST_CHAR_STRING, NAPI_AUTO_LENGTH, func, nullptr, &jsCallback);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiNewInstanceTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value testClass = nullptr;
    ASSERT_CHECK_CALL(napi_define_class(
        env,
        TEST_CHAR_STRING,
        NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
            return thisVar;
        },
        nullptr,
        0,
        nullptr,
        &testClass));

    ConstructionException();
    napi_value instanceValue = nullptr;
    napi_status res = napi_new_instance(env, testClass, 0, nullptr, &instanceValue);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiDefineClassTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    ConstructionException();
    napi_value testClass = nullptr;
    napi_status res = napi_define_class(
        env,
        TEST_CHAR_STRING,
        NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
            return thisVar;
        },
        nullptr,
        0,
        nullptr,
        &testClass);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiWrapTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
    auto finalizer = [](napi_env, void *data, void *) { delete reinterpret_cast<uint8_t *>(data); };
    uint8_t *data0 = new uint8_t;

    ConstructionException();
    napi_status res = napi_wrap(env, object, data0, finalizer, nullptr, nullptr);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiUnwrapTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
    auto finalizer = [](napi_env, void *data, void *) { delete reinterpret_cast<uint8_t *>(data); };
    uint8_t *data0 = new uint8_t;
    ASSERT_CHECK_CALL(napi_wrap(env, object, data0, finalizer, nullptr, nullptr));

    ConstructionException();
    void *result = nullptr;
    napi_status res = napi_unwrap(env, object, &result);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiRemoveWrapTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
    auto finalizer = [](napi_env, void *data, void *) { delete reinterpret_cast<uint8_t *>(data); };
    uint8_t *data0 = new uint8_t;
    ASSERT_CHECK_CALL(napi_wrap(env, object, data0, finalizer, nullptr, nullptr));

    ConstructionException();
    void *result = nullptr;
    napi_status res = napi_remove_wrap(env, object, &result);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiCreateBigintWordsTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    int signBit = 0;
    size_t wordCount = INT_FOUR;
    uint64_t words[] = {12ULL, 34ULL, 56ULL, 78ULL};

    ConstructionException();
    napi_value result = nullptr;
    napi_status res = napi_create_bigint_words(env, signBit, wordCount, words, &result);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiObjectFreezeTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value obj = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));

    ConstructionException();
    napi_status res = napi_object_freeze(env, obj);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

HWTEST_F(NapiPendingExceptionTest, NapiObjectSealTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value obj = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));

    ConstructionException();
    napi_status res = napi_object_seal(env, obj);
    DestructionException();
    ASSERT_EQ(res, napi_pending_exception);
}

/**
 * @tc.name: NapiGetAndClearLastException001
 * @tc.desc: Test napi_get_and_clear_last_exception without exception.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiPendingExceptionTest, NapiGetAndClearLastException001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value exceptionObj = nullptr;
    napi_status status = napi_get_and_clear_last_exception(env, &exceptionObj);
    ASSERT_EQ(status, napi_ok);
    ASSERT_EQ(exceptionObj, nullptr);
}

/**
 * @tc.name: NapiGetAndClearLastException002
 * @tc.desc: Test napi_get_and_clear_last_exception with exception.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiPendingExceptionTest, NapiGetAndClearLastException002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_status status = napi_throw_error(env, TEST_CHAR_ERROR_CODE, TEST_CHAR_ERROR_MESSAGE);
    ASSERT_EQ(status, napi_ok);

    napi_value exceptionObj = nullptr;
    status = napi_get_and_clear_last_exception(env, &exceptionObj);
    ASSERT_EQ(status, napi_ok);
    ASSERT_NE(exceptionObj, nullptr);

    bool hasProperty = false;
    ASSERT_CHECK_CALL(napi_has_named_property(env, exceptionObj, TEST_CHAR_ERROR_CODE_KEY, &hasProperty));
    ASSERT_TRUE(hasProperty);
    napi_value codeValue = nullptr;
    ASSERT_CHECK_CALL(napi_get_named_property(env, exceptionObj, TEST_CHAR_ERROR_CODE_KEY, &codeValue));
    char testCode[BUFF_SIZE] = {0};
    size_t length = 0;
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, codeValue, testCode, BUFF_SIZE, &length));
    ASSERT_EQ(length, strlen(TEST_CHAR_ERROR_CODE));
    ASSERT_STREQ(TEST_CHAR_ERROR_CODE, testCode);

    hasProperty = false;
    ASSERT_CHECK_CALL(napi_has_named_property(env, exceptionObj, TEST_CHAR_ERROR_MESSAGE_KEY, &hasProperty));
    ASSERT_TRUE(hasProperty);
    napi_value messageValue = nullptr;
    ASSERT_CHECK_CALL(napi_get_named_property(env, exceptionObj, TEST_CHAR_ERROR_MESSAGE_KEY, &messageValue));
    char testMessage[BUFF_SIZE] = {0};
    length = 0;
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, messageValue, testMessage, BUFF_SIZE, &length));
    ASSERT_EQ(length, strlen(TEST_CHAR_ERROR_MESSAGE));
    ASSERT_STREQ(TEST_CHAR_ERROR_MESSAGE, testMessage);
}