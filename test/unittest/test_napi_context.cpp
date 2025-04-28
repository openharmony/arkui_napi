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

class NapiContextTest : public NativeEngineTest {
public:
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
    napi_env env = (napi_env)engine_;

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
    napi_env env = (napi_env)engine_;

    // mock exception
    napi_status status = napi_throw_error(env, TEST_ERROR_CODE, TEST_ERROR_MESSAGE);
    EXPECT_EQ(status, napi_ok);

    NativeEngineProxy newEngine;
    napi_env newEnv = napi_env(newEngine);

    auto context = newEngine->context_;
    (newEngine->context_).Empty();
    status = napi_create_ark_context(env, &newEnv);
    EXPECT_EQ(status, napi_pending_exception);
    newEngine->context_ = context;
    //clear exception
    napi_value error = nullptr;
    status = napi_get_and_clear_last_exception(env, &error);
    EXPECT_EQ(status, napi_ok);
}

/**
 * @tc.name: NapiCreateContextTest004
 * @tc.desc: Test napi_create_ark_context when current env is not main env context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiCreateContextTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
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
 * @tc.desc: Test napi_create_ark_context when current thread is NATIVE_THREAD.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiCreateContextTest005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    EXPECT_TRUE(engine_->IsMainThread());
    engine_->jsThreadType_ = DataProtector(uintptr_t(NativeEngine::JSThreadType::NATIVE_THREAD));
    EXPECT_FALSE(engine_->IsMainThread());

    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    EXPECT_EQ(status, napi_invalid_arg);

    engine_->jsThreadType_ = DataProtector(uintptr_t(NativeEngine::JSThreadType::MAIN_THREAD));
    EXPECT_TRUE(engine_->IsMainThread());
}

/**
 * @tc.name: NapiCreateContextTest006
 * @tc.desc: Test napi_create_ark_context when current thread is FORM_THREAD.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiCreateContextTest006, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    EXPECT_TRUE(engine_->IsMainThread());
    engine_->jsThreadType_ = DataProtector(uintptr_t(NativeEngine::JSThreadType::FORM_THREAD));
    EXPECT_FALSE(engine_->IsMainThread());

    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    EXPECT_EQ(status, napi_invalid_arg);

    engine_->jsThreadType_ = DataProtector(uintptr_t(NativeEngine::JSThreadType::MAIN_THREAD));
    EXPECT_TRUE(engine_->IsMainThread());
}

/**
 * @tc.name: NapiCreateContextTest007
 * @tc.desc: Test napi_create_ark_context when context is not empty.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiCreateContextTest007, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;

    NativeEngineProxy newEngine;
    napi_env newEnv = napi_env(newEngine);

    napi_status status = napi_create_ark_context(env, &newEnv);
    EXPECT_EQ(status, napi_generic_failure);
}

/**
 * @tc.name: NapiCreateContextTest008
 * @tc.desc: Test napi_create_ark_context successfully.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiCreateContextTest008, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;

    NativeEngineProxy newEngine;
    napi_env newEnv = napi_env(newEngine);
    auto context = newEngine->context_;
    (newEngine->context_).Empty();
    napi_status status = napi_create_ark_context(env, &newEnv);
    EXPECT_EQ(status, napi_ok);
    newEngine->context_ = context;
}

/**
 * @tc.name: NapiSwitchContextTest001
 * @tc.desc: Test napi_switch_ark_context when the input argument env is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiSwitchContextTest001, testing::ext::TestSize.Level1)
{
    napi_status status = napi_switch_ark_context(nullptr);
    ASSERT_EQ(status, napi_invalid_arg);
}

/**
 * @tc.name: NapiSwitchContextTest002
 * @tc.desc: Test napi_switch_ark_context when some exception exists.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiSwitchContextTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;

    // mock exception
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &value));
    Local<panda::JSValueRef> nativeValue = LocalValueFromJsValue(value);
    engine_->lastException_ = Local<panda::ObjectRef>(nativeValue);

    napi_status status = napi_switch_ark_context(env);
    ASSERT_EQ(status, napi_pending_exception);

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
    napi_env env = (napi_env)engine_;

    // mock exception
    napi_status status = napi_throw_error(env, TEST_ERROR_CODE, TEST_ERROR_MESSAGE);
    EXPECT_EQ(status, napi_ok);

    status = napi_switch_ark_context(env);
    EXPECT_EQ(status, napi_pending_exception);

    //clear exception
    napi_value error = nullptr;
    status = napi_get_and_clear_last_exception(env, &error);
    EXPECT_EQ(status, napi_ok);
}

/**
 * @tc.name: NapiSwitchContextTest004
 * @tc.desc: Test napi_switch_ark_context when context is empty.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiSwitchContextTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    auto arkNativeEngine = reinterpret_cast<ArkNativeEngine* >(engine_);
    auto context = arkNativeEngine->context_;
    arkNativeEngine->context_ = panda::Global<panda::JSValueRef>();

    napi_status status = napi_switch_ark_context(env);
    EXPECT_EQ(status, napi_generic_failure);
    arkNativeEngine->context_ = context;
}

/**
 * @tc.name: NapiSwitchContextTest005
 * @tc.desc: Test napi_switch_ark_context when current thread is NATIVE_THREAD.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiSwitchContextTest005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    EXPECT_TRUE(engine_->IsMainThread());
    engine_->jsThreadType_ = DataProtector(uintptr_t(NativeEngine::JSThreadType::NATIVE_THREAD));
    EXPECT_FALSE(engine_->IsMainThread());

    napi_env env = (napi_env)engine_;
    napi_status status = napi_switch_ark_context(env);
    EXPECT_EQ(status, napi_invalid_arg);

    engine_->jsThreadType_ = DataProtector(uintptr_t(NativeEngine::JSThreadType::MAIN_THREAD));
    EXPECT_TRUE(engine_->IsMainThread());
}

/**
 * @tc.name: NapiSwitchContextTest006
 * @tc.desc: Test napi_switch_ark_context when current thread is FORM_THREAD.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiSwitchContextTest006, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    EXPECT_TRUE(engine_->IsMainThread());
    engine_->jsThreadType_ = DataProtector(uintptr_t(NativeEngine::JSThreadType::FORM_THREAD));
    EXPECT_FALSE(engine_->IsMainThread());

    napi_env env = (napi_env)engine_;
    napi_status status = napi_switch_ark_context(env);
    EXPECT_EQ(status, napi_invalid_arg);

    engine_->jsThreadType_ = DataProtector(uintptr_t(NativeEngine::JSThreadType::MAIN_THREAD));
    EXPECT_TRUE(engine_->IsMainThread());
}

/**
 * @tc.name: NapiSwitchContextTest007
 * @tc.desc: Test napi_switch_ark_context successfully.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiSwitchContextTest007, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
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
    ASSERT_EQ(status, napi_invalid_arg);
}

/**
 * @tc.name: NapiDestroyContextTest002
 * @tc.desc: Test napi_destroy_ark_context when context is empty.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiDestroyContextTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    auto arkNativeEngine = reinterpret_cast<ArkNativeEngine* >(engine_);
    auto context = arkNativeEngine->context_;
    arkNativeEngine->context_ = panda::Global<panda::JSValueRef>();

    napi_status status = napi_destroy_ark_context(env);
    EXPECT_EQ(status, napi_generic_failure);
    arkNativeEngine->context_ = context;
}

/**
 * @tc.name: NapiDestroyContextTest003
 * @tc.desc: Test napi_destroy_ark_context when some exception exists.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiDestroyContextTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;

    // mock exception
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &value));
    Local<panda::JSValueRef> nativeValue = LocalValueFromJsValue(value);
    engine_->lastException_ = Local<panda::ObjectRef>(nativeValue);

    napi_status status = napi_destroy_ark_context(env);
    ASSERT_EQ(status, napi_pending_exception);

    //clear exception
    engine_->lastException_.Empty();
}

/**
 * @tc.name: NapiDestroyContextTest004
 * @tc.desc: Test napi_destroy_ark_context when current thread is NATIVE_THREAD.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiDestroyContextTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    EXPECT_TRUE(engine_->IsMainThread());
    engine_->jsThreadType_ = DataProtector(uintptr_t(NativeEngine::JSThreadType::NATIVE_THREAD));
    EXPECT_FALSE(engine_->IsMainThread());

    napi_env env = (napi_env)engine_;
    napi_status status = napi_destroy_ark_context(env);
    EXPECT_EQ(status, napi_invalid_arg);

    engine_->jsThreadType_ = DataProtector(uintptr_t(NativeEngine::JSThreadType::MAIN_THREAD));
    EXPECT_TRUE(engine_->IsMainThread());
}

/**
 * @tc.name: NapiDestroyContextTest005
 * @tc.desc: Test napi_destroy_ark_context when current thread is FORM_THREAD.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiDestroyContextTest005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    EXPECT_TRUE(engine_->IsMainThread());
    engine_->jsThreadType_ = DataProtector(uintptr_t(NativeEngine::JSThreadType::FORM_THREAD));
    EXPECT_FALSE(engine_->IsMainThread());

    napi_env env = (napi_env)engine_;
    napi_status status = napi_destroy_ark_context(env);
    EXPECT_EQ(status, napi_invalid_arg);

    engine_->jsThreadType_ = DataProtector(uintptr_t(NativeEngine::JSThreadType::MAIN_THREAD));
    EXPECT_TRUE(engine_->IsMainThread());
}

/**
 * @tc.name: NapiDestroyContextTest006
 * @tc.desc: Test napi_create_ark_context when current env is main env context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiDestroyContextTest006, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    EXPECT_TRUE(engine_->IsMainEnvContext());
    napi_status status = napi_destroy_ark_context(env);
    EXPECT_EQ(status, napi_invalid_arg);
}

/**
 * @tc.name: NapiDestroyContextTest007
 * @tc.desc: Test napi_destroy_ark_context when current env is using.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiDestroyContextTest007, testing::ext::TestSize.Level1)
{
    NativeEngineProxy engine;
    napi_env env = napi_env(engine);
    auto arkNativeEngine = reinterpret_cast<ArkNativeEngine* >(env);
    EXPECT_TRUE(arkNativeEngine->IsMainEnvContext());
    arkNativeEngine->isMainEnvContext_ = false;
    EXPECT_FALSE(arkNativeEngine->IsMainEnvContext());
    napi_status status = napi_destroy_ark_context(env);
    EXPECT_EQ(status, napi_invalid_arg);
    arkNativeEngine->isMainEnvContext_ = true;
    EXPECT_TRUE(arkNativeEngine->IsMainEnvContext());
}
