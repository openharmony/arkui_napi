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

#include "gtest/gtest.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "native_utils.h"
#define private public
#include "test.h"
#undef private
#include "test_common.h"
#include "utils/log.h"

constexpr const char TEST_ERROR_CODE[] = "500";
constexpr const char TEST_ERROR_MESSAGE[] = "Common error";
constexpr const char SENDABLE_CLASS_NAME[] = "MY_CLASS";
constexpr const char SENDABLE_FUNCTION_NAME[] = "func";
constexpr const char STRING_TEST[] = "a";
constexpr const char TEST_WRAP_STRING[] = "testWrapStr";
constexpr const int32_t LENGTH = 1024;
constexpr const char TEST_STR[] = "test";
constexpr const char TEST_CHAR_ASYNCWORK[] = "AsyncWorkTest";

static constexpr int32_t NAPI_BUFFER_SIZE = 64;

static napi_value SendableFunc(napi_env env, napi_callback_info info)
{
    return nullptr;
}

class NapiContextTest : public NativeEngineTest {
public:
    NapiContextTest() : NativeEngineTest()
    {
        napi_create_ark_context(reinterpret_cast<napi_env>(engine_), reinterpret_cast<napi_env*>(&multiContextEngine_));
    }

    ~NapiContextTest() override
    {
        if (multiContextEngine_ != nullptr) {
            napi_destroy_ark_context(reinterpret_cast<napi_env>(multiContextEngine_));
            multiContextEngine_ = nullptr;
        }
    }

    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "NapiContextTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "NapiContextTest TearDownTestCase";
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

    inline void SetMainEnvContext(bool isMainContext)
    {
        auto arkNativeEngine = reinterpret_cast<ArkNativeEngine* >(engine_);
        arkNativeEngine->isMainEnvContext_ = isMainContext;
        ASSERT_EQ(arkNativeEngine->IsMainEnvContext(), isMainContext);
    }

private:
    napi_handle_scope scope_ = nullptr;
};

/**
 * @tc.name: NapiCreateContextTest001
 * @tc.desc: Test napi_create_ark_context when the input argument env is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiCreateContextTest001, testing::ext::TestSize.Level1)
{
    napi_env newEnv = nullptr;
    napi_status res = napi_create_ark_context(nullptr, &newEnv);
    EXPECT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiCreateContextTest002
 * @tc.desc: Test napi_create_ark_context when some exception exists.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiCreateContextTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    // mock exception
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &value));
    Local<panda::JSValueRef> nativeValue = LocalValueFromJsValue(value);
    engine_->lastException_ = Local<panda::ObjectRef>(nativeValue);

    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    EXPECT_EQ(status, napi_pending_exception);

    //clear exception
    engine_->lastException_.Empty();
}

/**
 * @tc.name: NapiCreateContextTest003
 * @tc.desc: Test napi_create_ark_context when some exception exists.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiCreateContextTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    // mock exception
    NAPI_THROW_ERROR(env, TEST_ERROR_CODE, TEST_ERROR_MESSAGE);

    napi_env newEnv = nullptr;
    EXPECT_EQ(napi_create_ark_context(env, &newEnv), napi_pending_exception);
    //clear exception
    napi_value error = nullptr;
    EXPECT_EQ(napi_get_and_clear_last_exception(env, &error), napi_ok);
}

/**
 * @tc.name: NapiCreateContextTest004
 * @tc.desc: Test napi_create_ark_context when current env is not main env context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiCreateContextTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    auto arkNativeEngine = reinterpret_cast<ArkNativeEngine* >(engine_);
    EXPECT_TRUE(arkNativeEngine->IsMainEnvContext());
    arkNativeEngine->isMainEnvContext_ = false;
    EXPECT_FALSE(arkNativeEngine->IsMainEnvContext());

    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    EXPECT_EQ(status, napi_invalid_arg);

    arkNativeEngine->isMainEnvContext_ = true;
    EXPECT_TRUE(arkNativeEngine->IsMainEnvContext());
}

/**
 * @tc.name: NapiCreateContextTest005
 * @tc.desc: Test napi_create_ark_context when context is not empty.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiCreateContextTest005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_env newEnv = nullptr;

    napi_status status = napi_create_ark_context(env, &newEnv);
    EXPECT_EQ(status, napi_ok);
    EXPECT_EQ(napi_destroy_ark_context(newEnv), napi_ok);
}

/**
 * @tc.name: NapiCreateContextTest006
 * @tc.desc: Test napi_create_ark_context successfully.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiCreateContextTest006, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    NativeEngineProxy newEngine;
    napi_env newEnv = napi_env(newEngine);
    auto context = newEngine->context_;
    (newEngine->context_).Empty();
    napi_status status = napi_create_ark_context(env, &newEnv);
    EXPECT_EQ(status, napi_ok);
    newEngine->context_ = context;
}

/**
 * @tc.name: NapiCreateContextTest007
 * @tc.desc: Test napi_create_ark_context with nullptr out parameter.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiCreateContextTest007, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    ASSERT_EQ(napi_create_ark_context(reinterpret_cast<napi_env>(engine_), nullptr), napi_invalid_arg);
}

/**
 * @tc.name: NapiSwitchContextTest001
 * @tc.desc: Test napi_switch_ark_context when the input argument env is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiSwitchContextTest001, testing::ext::TestSize.Level1)
{
    napi_status status = napi_switch_ark_context(nullptr);
    EXPECT_EQ(status, napi_invalid_arg);
}

/**
 * @tc.name: NapiSwitchContextTest002
 * @tc.desc: Test napi_switch_ark_context when some exception exists.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiSwitchContextTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    // mock exception
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &value));
    Local<panda::JSValueRef> nativeValue = LocalValueFromJsValue(value);
    engine_->lastException_ = Local<panda::ObjectRef>(nativeValue);

    napi_status status = napi_switch_ark_context(env);
    EXPECT_EQ(status, napi_pending_exception);

    //clear exception
    engine_->lastException_.Empty();
}

/**
 * @tc.name: NapiSwitchContextTest003
 * @tc.desc: Test napi_switch_ark_context when some exception exists.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiSwitchContextTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    NAPI_THROW_ERROR(env, TEST_ERROR_CODE, TEST_ERROR_MESSAGE);

    EXPECT_EQ(napi_switch_ark_context(env), napi_pending_exception);

    //clear exception
    napi_value error = nullptr;
    EXPECT_EQ(napi_get_and_clear_last_exception(env, &error), napi_ok);
}

/**
 * @tc.name: NapiSwitchContextTest004
 * @tc.desc: Test napi_switch_ark_context successfully.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiSwitchContextTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status status = napi_switch_ark_context(env);
    EXPECT_EQ(status, napi_ok);
}

/**
 * @tc.name: NapiDestroyContextTest001
 * @tc.desc: Test napi_destroy_ark_context when the input argument env is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiDestroyContextTest001, testing::ext::TestSize.Level1)
{
    napi_status status = napi_destroy_ark_context(nullptr);
    EXPECT_EQ(status, napi_invalid_arg);
}


/**
 * @tc.name: NapiDestroyContextTest002
 * @tc.desc: Test napi_destroy_ark_context when current env is using.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiDestroyContextTest002, testing::ext::TestSize.Level1)
{
    NativeEngineProxy engine(engine_);
    napi_env env = napi_env(engine);
    EXPECT_EQ(napi_switch_ark_context(env), napi_ok);
    napi_status status = napi_destroy_ark_context(env);
    EXPECT_EQ(status, napi_invalid_arg);
    EXPECT_EQ(napi_switch_ark_context(reinterpret_cast<napi_env>(engine_)), napi_ok);
}

/**
 * @tc.name: NapiDestroyContextTest003
 * @tc.desc: Test napi_destroy_ark_context when some exception exists.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiDestroyContextTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    // mock exception
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &value));
    Local<panda::JSValueRef> nativeValue = LocalValueFromJsValue(value);
    engine_->lastException_ = Local<panda::ObjectRef>(nativeValue);

    napi_status status = napi_destroy_ark_context(env);
    EXPECT_EQ(status, napi_pending_exception);

    //clear exception
    engine_->lastException_.Empty();
}

/**
 * @tc.name: NapiDestroyContextTest004
 * @tc.desc: Test napi_create_ark_context when current env is main env context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiDestroyContextTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    EXPECT_TRUE(engine_->IsMainEnvContext());
    napi_status status = napi_destroy_ark_context(env);
    EXPECT_EQ(status, napi_invalid_arg);
}

/**
 * @tc.name: NapiGetUvEventLoopWithMultiContextTest001
 * @tc.desc: Test napi_get_uv_event_loop when designated env is generated by napi_create_ark_context interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiGetUvEventLoopWithMultiContextTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    SetMainEnvContext(false);
    struct uv_loop_s** loop = nullptr;
    napi_status status = napi_get_uv_event_loop(env, loop);
    EXPECT_EQ(status, napi_invalid_arg);
    SetMainEnvContext(true);
}

/**
 * @tc.name: CreateSendableArrayWithMultiContext001
 * @tc.desc: Test napi_create_sendable_array when designated env is generated by napi_create_ark_context interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateSendableArrayWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    SetMainEnvContext(false);
    napi_value result = nullptr;
    napi_status res = napi_ok;
    res = napi_create_sendable_array(env, &result);
    EXPECT_EQ(res, napi_invalid_arg);
    SetMainEnvContext(true);
}

/**
 * @tc.name: CreateSendableArrayWithLengthWithMultiContext001
 * @tc.desc: Test napi_create_sendable_array_with_length when designated env is generated by napi_create_ark_context
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateSendableArrayWithLengthWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    SetMainEnvContext(false);
    napi_value result = nullptr;
    napi_status res = napi_ok;
    res = napi_create_sendable_array_with_length(env, 0, &result);
    EXPECT_EQ(res, napi_invalid_arg);
    SetMainEnvContext(true);
}

/**
 * @tc.name: IsSendableWithMultiContext001
 * @tc.desc: Test napi_is_sendable when designated env is generated by napi_create_ark_context interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, IsSendableWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    napi_value result = nullptr;
    res = napi_create_sendable_array_with_length(env, LENGTH, &result);
    EXPECT_EQ(res, napi_ok);

    SetMainEnvContext(false);
    bool isShared = false;
    res = napi_is_sendable(env, result, &isShared);
    EXPECT_EQ(res, napi_invalid_arg);
    SetMainEnvContext(true);
}

/**
 * @tc.name: SendableClassWithMultiContextTest001
 * @tc.desc: Test napi_define_sendable_class when designated env is generated by napi_create_ark_context interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, SendableClassWithMultiContextTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    SetMainEnvContext(false);
    napi_value myClass = nullptr;
    napi_status status = napi_define_sendable_class(
        env, SENDABLE_CLASS_NAME, NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
            return thisVar;
        },
        nullptr, 0, nullptr, nullptr, &myClass);
    EXPECT_EQ(status, napi_invalid_arg);
    SetMainEnvContext(true);
}

/**
 * @tc.name: CreateSendableObjectWithPropertiesWithMultiContextTest001
 * @tc.desc: Test napi_create_sendable_object_with_properties when designated env is generated by
 *           napi_create_ark_context interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateSendableObjectWithPropertiesWithMultiContextTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_value exception = nullptr;
    ASSERT_CHECK_CALL(napi_get_and_clear_last_exception(env, &exception));

    napi_value obj1 = nullptr;
    napi_property_descriptor desc1[] = {
        DECLARE_NAPI_FUNCTION(SENDABLE_FUNCTION_NAME, SendableFunc),
    };

    SetMainEnvContext(false);
    napi_status status = napi_create_sendable_object_with_properties(env, 1, desc1, &obj1);
    EXPECT_EQ(status, napi_invalid_arg);
    SetMainEnvContext(true);
}

/**
 * @tc.name: CreateSendableMapWithMultiContext001
 * @tc.desc: Test napi_create_sendable_map when designated env is generated by napi_create_ark_context interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateSendableMapWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    SetMainEnvContext(false);
    napi_value result = nullptr;
    res = napi_create_sendable_map(env, &result);
    EXPECT_EQ(res, napi_invalid_arg);
    SetMainEnvContext(true);
}

/**
 * @tc.name: WrapSendableWithMultiContextTest001
 * @tc.desc: Test napi_wrap_sendable when designated env is generated by napi_create_ark_context interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, WrapSendableWithMultiContextTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value exception = nullptr;
    ASSERT_CHECK_CALL(napi_get_and_clear_last_exception(env, &exception));

    napi_value obj1 = nullptr;
    napi_property_descriptor desc1[] = {
        DECLARE_NAPI_FUNCTION(SENDABLE_FUNCTION_NAME, SendableFunc),
    };
    ASSERT_CHECK_CALL(napi_create_sendable_object_with_properties(env, 1, desc1, &obj1));
    bool isShared = false;
    napi_is_sendable(env, obj1, &isShared);
    EXPECT_EQ(isShared, true);

    SetMainEnvContext(false);
    napi_status status = napi_wrap_sendable(env, obj1, (void*)STRING_TEST, nullptr, nullptr);
    EXPECT_EQ(status, napi_invalid_arg);
    SetMainEnvContext(true);
}

/**
 * @tc.name: WrapSendableWithMultiContextTest002
 * @tc.desc: Test napi_wrap_sendable_with_size when designated env is generated by napi_create_ark_context interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, WrapSendableWithMultiContextTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value testWrapClass = nullptr;
    napi_define_sendable_class(
        env, SENDABLE_CLASS_NAME, NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
            return thisVar;
        },
        nullptr, 0, nullptr, nullptr, &testWrapClass);

    napi_value instanceValue = nullptr;
    napi_new_instance(env, testWrapClass, 0, nullptr, &instanceValue);

    size_t size = sizeof(STRING_TEST) / sizeof(char);
    SetMainEnvContext(false);
    napi_status status = napi_wrap_sendable_with_size(
        env, instanceValue, (void*)STRING_TEST, [](napi_env env, void* data, void* hint) {}, nullptr, size);
    EXPECT_EQ(status, napi_invalid_arg);
    SetMainEnvContext(true);
}

/**
 * @tc.name: WrapSendableWithMultiContextTest003
 * @tc.desc: Test napi_unwrap_sendable when designated env is generated by napi_create_ark_context interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, WrapSendableWithMultiContextTest003, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;

    napi_value testWrapClass = nullptr;
    napi_define_sendable_class(
        env, SENDABLE_CLASS_NAME, NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
            return thisVar;
        },
        nullptr, 0, nullptr, nullptr, &testWrapClass);

    napi_value instanceValue = nullptr;
    napi_new_instance(env, testWrapClass, 0, nullptr, &instanceValue);

    size_t size = sizeof(TEST_WRAP_STRING) / sizeof(char);
    napi_wrap_sendable_with_size(
        env, instanceValue, (void*)TEST_WRAP_STRING, [](napi_env env, void* data, void* hint) {}, nullptr, size);

    char* tempTestStr = nullptr;
    SetMainEnvContext(false);
    napi_status status = napi_unwrap_sendable(env, instanceValue, (void**)&tempTestStr);
    EXPECT_EQ(status, napi_invalid_arg);
    SetMainEnvContext(true);
}

/**
 * @tc.name: WrapSendableWithMultiContextTest004
 * @tc.desc: Test napi_remove_wrap_sendable when designated env is generated by napi_create_ark_context interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, WrapSendableWithMultiContextTest004, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_status res = napi_ok;

    napi_value testClass = nullptr;
    res = napi_define_sendable_class(
        env, SENDABLE_CLASS_NAME, NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
            return thisVar;
        },
        nullptr, 0, nullptr, nullptr, &testClass);
    ASSERT_EQ(res, napi_ok);

    napi_value instanceValue = nullptr;
    res = napi_new_instance(env, testClass, 0, nullptr, &instanceValue);
    ASSERT_EQ(res, napi_ok);

    res = napi_wrap_sendable(
        env, instanceValue, (void*)STRING_TEST, [](napi_env env, void* data, void* hint) {}, nullptr);
    ASSERT_EQ(res, napi_ok);

    char* tmpTestStr = nullptr;
    res = napi_unwrap_sendable(env, instanceValue, (void**)&tmpTestStr);
    ASSERT_EQ(res, napi_ok);
    ASSERT_STREQ(STRING_TEST, tmpTestStr);

    char* tmpTestStr1 = nullptr;
    SetMainEnvContext(false);
    res = napi_remove_wrap_sendable(env, instanceValue, (void**)&tmpTestStr1);
    EXPECT_EQ(res, napi_invalid_arg);
    SetMainEnvContext(true);
}

/**
 * @tc.name: CreateSendableArrayBuffer002
 * @tc.desc: Test napi_create_sendable_arraybuffer when designated env is generated by napi_create_ark_context
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateSendableArrayBuffer002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value result = nullptr;
    SetMainEnvContext(false);
    res = napi_create_sendable_arraybuffer(env, LENGTH, &data, &result);
    ASSERT_EQ(res, napi_invalid_arg);
    SetMainEnvContext(true);
}

/**
 * @tc.name: CreateSendableTypedArrayWithMultiContext001
 * @tc.desc: Test napi_create_sendable_typedarray when designated env is generated by napi_create_ark_context
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateSendableTypedArrayWithMultiContext001, testing::ext::TestSize.Level1)
{
    static size_t LENGTH = 1024;
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status res = napi_ok;

    void *data;
    napi_value arraybuffer = nullptr;
    napi_value result = nullptr;
    res = napi_create_sendable_arraybuffer(env, LENGTH, &data, &arraybuffer);
    ASSERT_EQ(res, napi_ok);

    bool isShared = false;
    res = napi_is_sendable(env, arraybuffer, &isShared);
    ASSERT_EQ(res, napi_ok);
    ASSERT_EQ(isShared, true);
    SetMainEnvContext(false);
    res = napi_create_sendable_typedarray(env, napi_uint8_clamped_array, LENGTH / 2, arraybuffer, 1, &result);
    ASSERT_EQ(res, napi_invalid_arg);
    SetMainEnvContext(true);
}

/**
 * @tc.name: ExternalTestWithMultiContext001
 * @tc.desc: Test external type.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, ExternalTestWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    // mock not main env context case
    EXPECT_TRUE(engine_->IsMainEnvContext());
    auto arkNativeEngine = reinterpret_cast<ArkNativeEngine* >(engine_);
    arkNativeEngine->isMainEnvContext_ = false;

    napi_value external = nullptr;
    napi_create_external(
        env, (void*)TEST_STR,
        [](napi_env env, void* data, void* hint) { ASSERT_STREQ((const char*)data, (const char*)hint); },
        (void*)TEST_STR, &external);

    ASSERT_CHECK_VALUE_TYPE(env, external, napi_external);
    void* tmpExternal = nullptr;
    napi_get_value_external(env, external, &tmpExternal);
    ASSERT_TRUE(tmpExternal);
    ASSERT_EQ(tmpExternal, TEST_STR);
    arkNativeEngine->isMainEnvContext_ = true;
}

/**
 * @tc.name: CreateExternalWithSizeAndMultiContextTest001
 * @tc.desc: Test create external with size.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateExternalWithSizeTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    // mock not main env context case
    EXPECT_TRUE(engine_->IsMainEnvContext());
    auto arkNativeEngine = reinterpret_cast<ArkNativeEngine* >(engine_);
    arkNativeEngine->isMainEnvContext_ = false;

    size_t size = sizeof(TEST_STR) / sizeof(char);
    napi_value external = nullptr;
    napi_create_external_with_size(
        env, (void*)TEST_STR,
        [](napi_env env, void* data, void* hint) { ASSERT_STREQ((const char*)data, (const char*)hint); },
        (void*)TEST_STR, &external, size);

    ASSERT_CHECK_VALUE_TYPE(env, external, napi_external);
    void* tempExternal = nullptr;
    napi_get_value_external(env, external, &tempExternal);
    ASSERT_TRUE(tempExternal);
    ASSERT_EQ(tempExternal, TEST_STR);
    arkNativeEngine->isMainEnvContext_ = true;
}

/**
 * @tc.name: NapiCreateExternalArraybufferWithMultiContextTest001
 * @tc.desc: Test interface of napi_create_external_arraybuffer
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiCreateExternalArraybufferWithMultiContextTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    // mock not main env context case
    EXPECT_TRUE(engine_->IsMainEnvContext());
    auto arkNativeEngine = reinterpret_cast<ArkNativeEngine* >(engine_);
    arkNativeEngine->isMainEnvContext_ = false;

    napi_value external = nullptr;
    auto res = napi_create_external_arraybuffer(
        env, (void*)TEST_STR, strlen(TEST_STR),
        [](napi_env env, void* data, void* hint) { ASSERT_STREQ((const char*)data, (const char*)hint); },
        (void*)TEST_STR, &external);
    ASSERT_EQ(res, napi_ok);

    arkNativeEngine->isMainEnvContext_ = true;
}

/**
 * @tc.name: NapiCreateExternalBufferWithMultiContextTest001
 * @tc.desc: Test interface of napi_create_external_buffer
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiCreateExternalBufferWithMultiContextTest001, testing::ext::TestSize.Level1)
{
    static constexpr int32_t NAPI_BUFFER_SIZE = 64;
    napi_env env = (napi_env)engine_;

    napi_value buffer = nullptr;
    char testStr[] = "test";
    void* bufferPtr = testStr;

    size_t bufferSize = NAPI_BUFFER_SIZE;
    napi_status status = napi_create_external_buffer(
        env, bufferSize, bufferPtr, [](napi_env env, void* data, void* hint) {
            ASSERT_STREQ((const char*)data, (const char*)hint);
        }, (void*)testStr, &buffer);
    ASSERT_EQ(status, napi_ok);
    void* tmpBufferPtr = nullptr;
    size_t bufferLength = 0;
    status = napi_get_buffer_info(env, buffer, &tmpBufferPtr, &bufferLength);
    ASSERT_EQ(status, napi_ok);
    bool isBuffer = false;
    status = napi_is_buffer(env, buffer, &isBuffer);
    ASSERT_EQ(status, napi_ok);
    ASSERT_EQ(bufferSize, bufferLength);
}

/**
 * @tc.name: AsyncInitWithMultiContext001
 * @tc.desc: Test napi_async_init when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, AsyncInitWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value name;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_STR, NAPI_AUTO_LENGTH, &name));

    napi_async_context context = nullptr;
    ASSERT_CHECK_CALL(napi_async_init(env, nullptr, name, &context));
}

/**
 * @tc.name: CreateBufferWithMultiContext001
 * @tc.desc: Test napi_create_buffer when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateBufferWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value buffer = nullptr;
    void* bufferPtr = nullptr;
    size_t bufferSize = NAPI_BUFFER_SIZE;
    ASSERT_CHECK_CALL(napi_create_buffer(env, bufferSize, &bufferPtr, &buffer));
}

/**
 * @tc.name: CreateExternalBufferWithMultiContext001
 * @tc.desc: Test napi_create_external_buffer when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateExternalBufferWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    const unsigned int bufferSize = NAPI_BUFFER_SIZE;
    char* copyPtr = strdup(TEST_STR);
    napi_value result;
    ASSERT_CHECK_CALL(napi_create_external_buffer(env, bufferSize, copyPtr,
        [](napi_env env, void* data, void* hint) { return; },
        nullptr, &result));
}

/**
 * @tc.name: CreateBufferCopyWithMultiContext001
 * @tc.desc: Test napi_create_buffer_copy when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateBufferCopyWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value Buffer = nullptr;
    void* resultData = nullptr;
    size_t BufferSize = sizeof(TEST_STR);
    ASSERT_CHECK_CALL(napi_create_buffer_copy(env, BufferSize, TEST_STR, &resultData, &Buffer));
}

/**
 * @tc.name: CreateAsyncWorkWithMultiContext001
 * @tc.desc: Test napi_create_buffer_copy when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateAsyncWorkWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    struct AsyncWorkContext {
        napi_async_work work = nullptr;
    };
    std::unique_ptr<AsyncWorkContext> asyncWorkContext = std::make_unique<AsyncWorkContext>();
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, TEST_CHAR_ASYNCWORK, NAPI_AUTO_LENGTH, &resourceName);
    napi_status status = napi_create_async_work(
        env, nullptr, resourceName, [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {},
        nullptr, &asyncWorkContext->work);
    ASSERT_EQ(status, napi_ok);
}

/**
 * @tc.name: GetGlobalWithMultiContext001
 * @tc.desc: Test napi_get_global when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, GetGlobalWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);

    NativeEngineProxy envProxy;
    napi_env env = napi_env(envProxy);
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    EXPECT_EQ(status, napi_ok);

    napi_value global = nullptr;
    napi_value undefined = nullptr;
    ASSERT_CHECK_CALL(napi_get_global(newEnv, &global));
    ASSERT_CHECK_CALL(napi_get_undefined(newEnv, &undefined));
    ASSERT_CHECK_CALL(napi_set_named_property(newEnv, global, "contextNewProp", undefined));
    bool hasProp = false;
    ASSERT_CHECK_CALL(napi_has_named_property(newEnv, global, "contextNewProp", &hasProp));
    ASSERT_EQ(hasProp, true);

    hasProp = false;
    napi_value globalOld = nullptr;
    ASSERT_CHECK_CALL(napi_get_global(env, &globalOld));
    ASSERT_CHECK_CALL(napi_has_named_property(env, globalOld, "contextNewProp", &hasProp));
    ASSERT_EQ(hasProp, false);
}

/**
 * @tc.name: CreateFunctionWithMultiContext001
 * @tc.desc: Test napi_create_function when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateFunctionWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    auto func = [](napi_env env, napi_callback_info info) -> napi_value {
        return nullptr;
    };
    napi_value fn;
    const char data[] = "data";
    ASSERT_CHECK_CALL(napi_create_function(env, nullptr, 0, func, (void*)data, &fn));
}

/**
 * @tc.name: CreateErrorWithMultiContext001
 * @tc.desc: Test napi_create_error when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateErrorWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value code = nullptr;
    napi_value message = nullptr;
    napi_create_string_latin1(env, TEST_ERROR_CODE, NAPI_AUTO_LENGTH, &code);
    napi_create_string_latin1(env, TEST_ERROR_MESSAGE, NAPI_AUTO_LENGTH, &message);

    napi_value error = nullptr;
    ASSERT_CHECK_CALL(napi_create_error(env, code, message, &error));
}

/**
 * @tc.name: CreateTypeErrorWithMultiContext001
 * @tc.desc: Test napi_create_type_error when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateTypeErrorWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value code = nullptr;
    napi_value message = nullptr;
    napi_create_string_latin1(env, TEST_ERROR_CODE, NAPI_AUTO_LENGTH, &code);
    napi_create_string_latin1(env, TEST_ERROR_MESSAGE, NAPI_AUTO_LENGTH, &message);

    napi_value error = nullptr;
    ASSERT_CHECK_CALL(napi_create_type_error(env, code, message, &error));
}

/**
 * @tc.name: CreateRangeErrorWithMultiContext001
 * @tc.desc: Test napi_create_range_error when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateRangeErrorWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value code = nullptr;
    napi_value message = nullptr;
    napi_create_string_latin1(env, TEST_ERROR_CODE, NAPI_AUTO_LENGTH, &code);
    napi_create_string_latin1(env, TEST_ERROR_MESSAGE, NAPI_AUTO_LENGTH, &message);

    napi_value error = nullptr;
    ASSERT_CHECK_CALL(napi_create_range_error(env, code, message, &error));
}

/**
 * @tc.name: CoerceToNumberWithMultiContext001
 * @tc.desc: Test napi_coerce_to_number when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CoerceToNumberWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value value;
    napi_value result;
    napi_status status = napi_create_string_utf8(env, "", 0, &value);
    status = napi_coerce_to_number(env, value, &result);
}

/**
 * @tc.name: CoerceToObjectWithMultiContext001
 * @tc.desc: Test napi_coerce_to_object when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CoerceToObjectWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value value;
    napi_value result;
    napi_status status = napi_get_undefined(env, &value);
    status = napi_coerce_to_object(env, value, &result);
    ASSERT_EQ(status, napi_ok);
}

/**
 * @tc.name: CoerceToStringWithMultiContext001
 * @tc.desc: Test napi_coerce_to_string when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CoerceToStringWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value argument;
    napi_get_boolean(env, true, &argument);
    ASSERT_CHECK_VALUE_TYPE(env, argument, napi_boolean);

    napi_value result;
    ASSERT_CHECK_CALL(napi_coerce_to_string(env, argument, &result));
}
