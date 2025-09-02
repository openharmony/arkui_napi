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
#define protected public
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
constexpr const char TEST_KEY[] = "testKey";
constexpr const char TEST_VALUE[] = "testValue";
constexpr const char ENUMERABLE[] = "enumerable";
constexpr const char WRITEABLE[] = "writable";
constexpr const char CONFIGURABLE[] = "configurable";
constexpr const char TEST_CLASS[] = "TestClass";
constexpr const uint32_t REFERENCE_SIZE = 2;
constexpr const char MESSAGE[] = "message";
constexpr const char TEST_FUNC[] = "testFunc";
constexpr const char CONTEXT_NEW_PROP[] = "contextNewProp";
constexpr const char STR_ATTRIBUTE[] = "strAttribute";
constexpr const char STR_MODULE_PATH[] = "@ohos:xxxx";
constexpr const char TEST_STR_UTF8[] = "ut.latin1test.napi.!@#%^&*()6666";
constexpr const char16_t TEST_STR_UTF16[] = u"中文,English,123456,!@#$%$#^%&12345";
constexpr const char BUFFER_STR[] = "hello world";
constexpr const char PROPERTY_NAME[] = "toString";
constexpr const char MOD_NAME[] = "entry";
constexpr const char CONST_STRING_NUMBER[] = "number";
constexpr const char CONST_STRING_VALUE[] = "value";
constexpr const char CONST_STRING_DONE[] = "done";
constexpr const char CONST_STRING_PROP[] = "prop";

static constexpr int INT_ZERO = 0;
static constexpr int INT_ONE = 1;
static constexpr int INT_TWO = 2;
static constexpr int INT_THREE = 3;
static constexpr double TEST_DOUBLE = 1.1;
constexpr size_t TEST_ARRAY_LENGTH = 10;
constexpr uint32_t CONST_U32_1K = 1000;
constexpr uint32_t CONST_U32_2K = 2000;

static constexpr int32_t NAPI_BUFFER_SIZE = 5;
static constexpr int32_t TYPE_TAGS_SIZE = 5;
static constexpr size_t ARRAYBUFFER_SIZE = 1024;

static int g_hookTag = 0;
static int g_hookArgOne = 1;
static int g_hookArgTwo = 2;

static napi_value SendableFunc(napi_env env, napi_callback_info info)
{
    return nullptr;
}

static const napi_type_tag typeTags[TYPE_TAGS_SIZE] = { // 5:array element size is 5.
    {0xdaf987b3cc62481a, 0xb745b0497f299531},
    {0xbb7936c374084d9b, 0xa9548d0762eeedb9},
    {0xa5ed9ce2e4c00c38, 0},
    {0, 0},
    {0xa5ed9ce2e4c00c34, 0xdaf987b3cc62481a},
};

class NapiContextTest : public NativeEngineTest {
public:
    NapiContextTest() : NativeEngineTest()
    {
        napi_create_ark_context(reinterpret_cast<napi_env>(engine_), reinterpret_cast<napi_env*>(&multiContextEngine_));
    }

    ~NapiContextTest() override
    {
        napi_switch_ark_context(reinterpret_cast<napi_env>(engine_));
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

    inline void CheckContextEnv()
    {
        ASSERT_NE(multiContextEngine_, nullptr);
        bool isMainContextEnv = multiContextEngine_->IsMainEnvContext();
        ASSERT_FALSE(isMainContextEnv);
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
    std::thread t([]() {
        NativeEngineProxy engine;
        napi_env env = napi_env(engine);

        NativeEngineProxy newEngine;
        napi_env newEnv = napi_env(newEngine);
        auto context = newEngine->context_;
        (newEngine->context_).Empty();
        napi_status status = napi_create_ark_context(env, &newEnv);
        EXPECT_EQ(status, napi_ok);
        newEngine->context_ = context;
    });
    t.join();
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
 * @tc.name: SetModuleValidateCallbackTest001
 * @tc.desc: Test napi_set_module_validate_callback on context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, SetModuleValidateCallbackTest001, testing::ext::TestSize.Level1)
{
    ArkNativeEngine::moduleValidateCallback_ = nullptr;
    ASSERT_EQ(napi_set_module_validate_callback(nullptr), napi_invalid_arg);
    ArkNativeEngine::moduleValidateCallback_ = nullptr;
}

/**
 * @tc.name: SetModuleValidateCallbackTest002
 * @tc.desc: Test napi_set_module_validate_callback on context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, SetModuleValidateCallbackTest002, testing::ext::TestSize.Level1)
{
    ArkNativeEngine::moduleValidateCallback_ = nullptr;
    ASSERT_CHECK_CALL(napi_set_module_validate_callback([](const char*) { return false; }));
    ArkNativeEngine::moduleValidateCallback_ = nullptr;
}

/**
 * @tc.name: SetModuleValidateCallbackTest003
 * @tc.desc: Test napi_set_module_validate_callback on context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, SetModuleValidateCallbackTest003, testing::ext::TestSize.Level1)
{
    ArkNativeEngine::moduleValidateCallback_ = nullptr;
    ASSERT_CHECK_CALL(napi_set_module_validate_callback([](const char*) { return false; }));
    ASSERT_EQ(napi_set_module_validate_callback([](const char*) { return false; }), napi_generic_failure);
    ArkNativeEngine::moduleValidateCallback_ = nullptr;
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
 * @tc.name: RequireMethodTest001
 * @tc.desc: Test globalThis.requireNapi method.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, RequireMethodTest001, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);
    const char requireName[] = "requireInternal";
    napi_value require = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, requireName, NAPI_AUTO_LENGTH, &require));

    napi_value global = nullptr;
    ASSERT_CHECK_CALL(napi_get_global(env, &global));

    bool hasProp = false;
    ASSERT_CHECK_CALL(napi_has_property(env, global, require, &hasProp));
    ASSERT_TRUE(hasProp);

    napi_value requireFunc = nullptr;
    ASSERT_CHECK_CALL(napi_get_property(env, global, require, &requireFunc));
    ASSERT_CHECK_VALUE_TYPE(env, requireFunc, napi_function);
}

/**
 * @tc.name: RequireMethodTest002
 * @tc.desc: Test globalThis.requireNapi method.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, RequireMethodTest002, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);
    const char requireName[] = "requireNapi";
    napi_value require = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, requireName, NAPI_AUTO_LENGTH, &require));

    napi_value global = nullptr;
    ASSERT_CHECK_CALL(napi_get_global(env, &global));

    bool hasProp = false;
    ASSERT_CHECK_CALL(napi_has_property(env, global, require, &hasProp));
    ASSERT_TRUE(hasProp);

    napi_value requireFunc = nullptr;
    ASSERT_CHECK_CALL(napi_get_property(env, global, require, &requireFunc));
    ASSERT_CHECK_VALUE_TYPE(env, requireFunc, napi_function);
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
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;

    napi_value buffer = nullptr;
    size_t bufferSize = NAPI_BUFFER_SIZE;
    napi_status status = napi_create_external_buffer(
        env, bufferSize, (void*)TEST_STR, [](napi_env env, void* data, void* hint) {
            ASSERT_STREQ((const char*)data, (const char*)hint);
        }, (void*)TEST_STR, &buffer);
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
    CheckContextEnv();
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
    CheckContextEnv();
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
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    const unsigned int bufferSize = NAPI_BUFFER_SIZE;
    char* copyPtr = strdup(TEST_STR);
    napi_value result;
    ASSERT_CHECK_CALL(napi_create_external_buffer(env, bufferSize, copyPtr,
        [](napi_env env, void* data, void* hint) {
            free(reinterpret_cast<char*>(data));
        },
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
    CheckContextEnv();
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
    CheckContextEnv();
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
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);
    ASSERT_NE(engine_, nullptr);
    napi_env oldEnv = reinterpret_cast<napi_env>(engine_);

    napi_value global = nullptr;
    napi_value undefined = nullptr;
    ASSERT_CHECK_CALL(napi_get_global(env, &global));
    ASSERT_CHECK_CALL(napi_get_undefined(env, &undefined));
    ASSERT_CHECK_CALL(napi_set_named_property(env, global, CONTEXT_NEW_PROP, undefined));
    bool hasProp = false;
    ASSERT_CHECK_CALL(napi_has_named_property(env, global, CONTEXT_NEW_PROP, &hasProp));
    ASSERT_EQ(hasProp, true);

    hasProp = false;
    napi_value globalOld = nullptr;
    ASSERT_CHECK_CALL(napi_get_global(oldEnv, &globalOld));
    ASSERT_CHECK_CALL(napi_has_named_property(oldEnv, globalOld, CONTEXT_NEW_PROP, &hasProp));
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
    CheckContextEnv();
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
    CheckContextEnv();
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
    CheckContextEnv();
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
    CheckContextEnv();
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
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value value;
    napi_value result;
    napi_status status = napi_create_string_utf8(env, "", 0, &value);
    status = napi_coerce_to_number(env, value, &result);
    ASSERT_EQ(status, napi_ok);
}

/**
 * @tc.name: CoerceToObjectWithMultiContext001
 * @tc.desc: Test napi_coerce_to_object when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CoerceToObjectWithMultiContext001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
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
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value argument;
    napi_get_boolean(env, true, &argument);
    ASSERT_CHECK_VALUE_TYPE(env, argument, napi_boolean);

    napi_value result;
    ASSERT_CHECK_CALL(napi_coerce_to_string(env, argument, &result));
}

/**
 * @tc.name: NapiEnvReferenceManagerTest001
 * @tc.desc: Test code of engine constructor
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiEnvReferenceManagerTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_->GetReferenceManager(), nullptr);
}

/**
 * @tc.name: NapiEnvReferenceManagerTest002
 * @tc.desc: Test code of engine constructor
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiEnvReferenceManagerTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_->GetReferenceManager(), nullptr);
}

/**
 * @tc.name: NapiEnvCallbackScopeManagerTest001
 * @tc.desc: Test code of engine constructor
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiEnvCallbackScopeManagerTest001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_->GetCallbackScopeManager(), nullptr);
}

/**
 * @tc.name: NapiEnvCallbackScopeManagerTest002
 * @tc.desc: Test code of engine constructor
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiEnvCallbackScopeManagerTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_->GetCallbackScopeManager(), nullptr);
}

/**
 * @tc.name: FinalizersCallbackTest001
 * @tc.desc: Test finalize callback execution of napi_wrap
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, FinalizersCallbackTest001, testing::ext::TestSize.Level0)
{
    CheckContextEnv();
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);
    const EcmaVM *vm = reinterpret_cast<ArkNativeEngine*>(engine_)->GetEcmaVm();

    const char *str = GetTestCaseName();
    size_t size = 2 * ArkNativeEngine::FINALIZERS_PACK_PENDING_NATIVE_BINDING_SIZE_THRESHOLD;
    static bool finalizersCallbackDone[2] = {false, false};

    for (int i = 0; i < 2; ++i) {
        {
            panda::LocalScope scope(vm);
            napi_value object = nullptr;
            napi_create_object(env, &object);
            napi_wrap_with_size(env, object, (void*)str, [](napi_env env, void *data, void *hint) {
                bool *result = reinterpret_cast<bool*>(hint);
                ASSERT_FALSE(*result);
                *result = true;
            }, reinterpret_cast<void*>(&finalizersCallbackDone[i]), nullptr, size);
        }
        panda::JSNApi::TriggerGC(vm, panda::ecmascript::GCReason::OTHER, panda::JSNApi::TRIGGER_GC_TYPE::FULL_GC);
    }

    ASSERT_FALSE(finalizersCallbackDone[0]);
    ASSERT_TRUE(finalizersCallbackDone[1]);
}

class ContextAsyncWorkTestData final : public NapiAsyncWorkTestData {
public:
    ContextAsyncWorkTestData(napi_env env, const char* name) : NapiAsyncWorkTestData(env, name) {}
    ~ContextAsyncWorkTestData() {};

    void Execute(napi_env env) override
    {
        executeSucc_ = env == env_;
    }
    void Complete(napi_env env, napi_status status) override
    {
        completeSucc_ = env == env_;
    }

    bool executeSucc_ { false };
    bool completeSucc_ { false };
};

/**
 * @tc.name: NapiAsyncWorkWithContextEnv001
 * @tc.desc: Test interface of napi_queue_async_work
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiAsyncWorkWithContextEnv001, testing::ext::TestSize.Level1)
{
    UVLoopRunner runner(engine_);
    NativeEngineProxy contextEngine(engine_);
    ContextAsyncWorkTestData* work = new ContextAsyncWorkTestData(contextEngine, GetTestCaseName());
    ASSERT_CHECK_CALL(work->Queue());
    runner.Run();
    ASSERT_TRUE(work->executeSucc_);
    ASSERT_TRUE(work->completeSucc_);
    delete work;
}

/**
 * @tc.name: NapiAsyncWorkWithContextEnv002
 * @tc.desc: Test interface of napi_queue_async_work_with_qos
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiAsyncWorkWithContextEnv002, testing::ext::TestSize.Level1)
{
    UVLoopRunner runner(engine_);
    NativeEngineProxy contextEngine(engine_);
    ContextAsyncWorkTestData* work = new ContextAsyncWorkTestData(contextEngine, GetTestCaseName());
    ASSERT_CHECK_CALL(work->Queue(napi_qos_background));
    runner.Run();
    ASSERT_TRUE(work->executeSucc_);
    ASSERT_TRUE(work->completeSucc_);
    delete work;
}

/**
 * @tc.name: NapiAsyncWorkWithContextEnv003
 * @tc.desc: Test interface of napi_queue_async_work_with_qos
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiAsyncWorkWithContextEnv003, testing::ext::TestSize.Level1)
{
    UVLoopRunner runner(engine_);
    NativeEngineProxy contextEngine(engine_);
    ContextAsyncWorkTestData* work = new ContextAsyncWorkTestData(contextEngine, GetTestCaseName());
    ASSERT_CHECK_CALL(work->Queue(napi_qos_utility));
    runner.Run();
    ASSERT_TRUE(work->executeSucc_);
    ASSERT_TRUE(work->completeSucc_);
    delete work;
}

/**
 * @tc.name: NapiAsyncWorkWithContextEnv004
 * @tc.desc: Test interface of napi_queue_async_work_with_qos
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiAsyncWorkWithContextEnv004, testing::ext::TestSize.Level1)
{
    UVLoopRunner runner(engine_);
    NativeEngineProxy contextEngine(engine_);
    ContextAsyncWorkTestData* work = new ContextAsyncWorkTestData(contextEngine, GetTestCaseName());
    ASSERT_CHECK_CALL(work->Queue(napi_qos_default));
    runner.Run();
    ASSERT_TRUE(work->executeSucc_);
    ASSERT_TRUE(work->completeSucc_);
    delete work;
}

/**
 * @tc.name: NapiCreateTSFNWithContextEnv005
 * @tc.desc: Test interface of napi_queue_async_work_with_qos
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiCreateTSFNWithContextEnv005, testing::ext::TestSize.Level1)
{
    UVLoopRunner runner(engine_);
    NativeEngineProxy contextEngine(engine_);
    napi_threadsafe_function tsfn = nullptr;
    bool* executed = new bool { false };
    ASSERT_CHECK_CALL(napi_create_threadsafe_function(
        contextEngine, nullptr, nullptr, GetNapiTCName(contextEngine), 0, 1, nullptr, nullptr, nullptr,
        [](napi_env, napi_value, void*, void* data) { *reinterpret_cast<bool*>(data) = true; }, &tsfn));
    ASSERT_CHECK_CALL(napi_call_threadsafe_function(tsfn, executed, napi_tsfn_nonblocking));
    runner.Run(UV_RUN_ONCE);
    ASSERT_TRUE(*executed);
    ASSERT_CHECK_CALL(napi_release_threadsafe_function(tsfn, napi_tsfn_release));
    runner.Run(UV_RUN_ONCE);
    delete executed;
}

napi_value EmptyNapiCallback(napi_env, napi_callback_info)
{
    return nullptr;
}

/**
 * @tc.name: CreateObjWithPropsWithinMultiContext001
 * @tc.desc: Test napi_create_object_with_properties when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateObjWithPropsWithinMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value propJsName = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, CONST_STRING_PROP, NAPI_AUTO_LENGTH, &propJsName));
    napi_value result = nullptr;
    napi_property_descriptor desc[] = { {
        CONST_STRING_PROP,
        nullptr,
        EmptyNapiCallback,
        nullptr,
        nullptr,
        nullptr,
        napi_default,
        nullptr,
    } };
    ASSERT_CHECK_CALL(napi_create_object_with_properties(env, &result, sizeof(desc) / sizeof(desc[0]), desc));
    napi_value val = nullptr;
    ASSERT_CHECK_CALL(napi_get_property(env, result, propJsName, &val));
    ASSERT_CHECK_VALUE_TYPE(env, val, napi_function);
}

/**
 * @tc.name: CreateObjWithPropsWithinMultiContext002
 * @tc.desc: Test napi_create_object_with_properties when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateObjWithPropsWithinMultiContext002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value propJsName = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, CONST_STRING_PROP, NAPI_AUTO_LENGTH, &propJsName));
    napi_value result = nullptr;
    napi_property_descriptor desc[] = { {
        CONST_STRING_PROP,
        nullptr,
        nullptr,
        EmptyNapiCallback,
        nullptr,
        nullptr,
        napi_default,
        nullptr,
    } };
    ASSERT_CHECK_CALL(napi_create_object_with_properties(env, &result, sizeof(desc) / sizeof(desc[0]), desc));
    napi_value val = nullptr;
    ASSERT_CHECK_CALL(napi_get_property(env, result, propJsName, &val));
    ASSERT_CHECK_VALUE_TYPE(env, val, napi_undefined);
    bool hasProp = false;
    ASSERT_CHECK_CALL(napi_has_property(env, result, propJsName, &hasProp));
    ASSERT_TRUE(hasProp);
}

/**
 * @tc.name: CreateObjWithPropsWithinMultiContext003
 * @tc.desc: Test napi_create_object_with_properties when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateObjWithPropsWithinMultiContext003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value propJsName = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, CONST_STRING_PROP, NAPI_AUTO_LENGTH, &propJsName));
    napi_value result = nullptr;
    napi_property_descriptor desc[] = { {
        CONST_STRING_PROP,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        propJsName,
        napi_default,
        nullptr,
    } };
    ASSERT_CHECK_CALL(napi_create_object_with_properties(env, &result, sizeof(desc) / sizeof(desc[0]), desc));
    napi_value val = nullptr;
    ASSERT_CHECK_CALL(napi_get_property(env, result, propJsName, &val));
    ASSERT_CHECK_VALUE_TYPE(env, val, napi_string);
}

/**
 * @tc.name: CreateObjWithNamedPropsWithinMultiContext001
 * @tc.desc: Test napi_create_object_with_named_properties when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateObjWithNamedPropsWithinMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    const char* names[] = { CONST_STRING_PROP };
    napi_value propJsName = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, CONST_STRING_PROP, NAPI_AUTO_LENGTH, &propJsName));

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_object_with_named_properties(env, &result, 1, names, &propJsName));
    napi_value val = nullptr;
    ASSERT_CHECK_CALL(napi_get_property(env, result, propJsName, &val));
    ASSERT_CHECK_VALUE_TYPE(env, val, napi_string);
}

/**
 * @tc.name: CoerceToNativeBindingObjectWithMultiContext001
 * @tc.desc: Test napi_coerce_to_native_binding_object when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CoerceToNativeBindingObjectWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    bool* testResult = new bool[] { false, false }; // detached, attached
    ASSERT_FALSE(testResult[0]);
    ASSERT_FALSE(testResult[1]);

    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
    ASSERT_CHECK_CALL(napi_coerce_to_native_binding_object(
        env, object,
        [](napi_env, void* data, void*) -> void* {
            reinterpret_cast<bool*>(data)[0] = true;
            return data;
        },
        [](napi_env, void* data, void*) -> napi_value {
            reinterpret_cast<bool*>(data)[1] = true;
            return nullptr;
        },
        testResult, nullptr));
    napi_value undefined = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &undefined))
    void* serializeData = nullptr;
    // trigger detach callback
    ASSERT_CHECK_CALL(napi_serialize(env, object, undefined, undefined, &serializeData));
    ASSERT_TRUE(testResult[0]);
    napi_value result = nullptr;
    // trigger attach callback
    ASSERT_CHECK_CALL(napi_deserialize(env, serializeData, &result));
    ASSERT_TRUE(testResult[1]);
    // release serialize data
    ASSERT_CHECK_CALL(napi_delete_serialization_data(env, serializeData));
    serializeData = nullptr;
    delete [] testResult;
}

/**
 * @tc.name: AddDetachedFinalizerWithMultiContext001
 * @tc.desc: Test napi_add_detached_finalizer when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, AddDetachedFinalizerWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    bool* detachFinalizerRan = new bool(false);
    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
    ASSERT_CHECK_CALL(napi_coerce_to_native_binding_object(
        env, object,
        [](napi_env, void* data, void*) -> void* {
            reinterpret_cast<bool*>(data)[0] = true;
            return data;
        },
        [](napi_env, void* data, void*) -> napi_value {
            reinterpret_cast<bool*>(data)[1] = true;
            return nullptr;
        },
        detachFinalizerRan, nullptr));
    ASSERT_CHECK_CALL(napi_add_detached_finalizer(
        env, object, [](void* data, void*) { *reinterpret_cast<bool*>(data) = true; }, nullptr));

    napi_value undefined = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &undefined))
    void* serializeData = nullptr;
    // make serialize data
    ASSERT_CHECK_CALL(napi_serialize(env, object, undefined, undefined, &serializeData));
    // trigger detach finalize callback
    ASSERT_CHECK_CALL(napi_delete_serialization_data(env, serializeData));
    ASSERT_TRUE(*detachFinalizerRan);
    delete detachFinalizerRan;
}

/**
 * @tc.name: RunEventLoopWithMultiContext001
 * @tc.desc: Test napi_run_event_loop when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, RunEventLoopWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    ASSERT_EQ(napi_run_event_loop(env, napi_event_mode_default), napi_invalid_arg);
}

/**
 * @tc.name: RunEventLoopWithMultiContext002
 * @tc.desc: Test napi_run_event_loop when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, RunEventLoopWithMultiContext002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    ASSERT_EQ(napi_run_event_loop(env, napi_event_mode_default), napi_invalid_arg);
}

/**
 * @tc.name: StopEventLoopWithMultiContext001
 * @tc.desc: Test napi_stop_event_loop when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, StopEventLoopWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    ASSERT_EQ(napi_stop_event_loop(env), napi_invalid_arg);
}

/**
 * @tc.name: DeserializeWithMultiContext001
 * @tc.desc: Test napi_deserialize when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, DeserializeWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
        
    napi_value undefined = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &undefined));
    void* serializeData = nullptr;
    // make serialize data
    ASSERT_CHECK_CALL(napi_serialize(env, object, undefined, undefined, &serializeData));
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_deserialize(env, serializeData, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_object);
    ASSERT_CHECK_CALL(napi_delete_serialization_data(env, serializeData));
}

/**
 * @tc.name: DeleteDeserializeDataWithMultiContext001
 * @tc.desc: Test napi_delete_serialization_data when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, DeleteDeserializeDataWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
        
    napi_value undefined = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &undefined));
    void* serializeData = nullptr;
    // make serialize data
    ASSERT_CHECK_CALL(napi_serialize(env, object, undefined, undefined, &serializeData));
    ASSERT_CHECK_CALL(napi_delete_serialization_data(env, serializeData));
}

/**
 * @tc.name: IsConcurrentFunctionWithMultiContext001
 * @tc.desc: Test napi_is_concurrent_function when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, IsConcurrentFunctionWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value callback = nullptr;
    ASSERT_CHECK_CALL(napi_create_function(env, __func__, NAPI_AUTO_LENGTH, EmptyNapiCallback, nullptr, &callback));
    bool isConcurrent = true;
    ASSERT_CHECK_CALL(napi_is_concurrent_function(env, callback, &isConcurrent));
    ASSERT_FALSE(isConcurrent);
}

/**
 * @tc.name: CreateMapWithMultiContext001
 * @tc.desc: Test napi_create_map when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateMapWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &map));
    bool isMap = false;
    ASSERT_CHECK_CALL(napi_is_map(env, map, &isMap));
    ASSERT_TRUE(isMap);
}

/**
 * @tc.name: MapSetPropertyWithMultiContext001
 * @tc.desc: Test napi_map_set_property when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, MapSetPropertyWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &map));
    napi_value str = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, __func__, NAPI_AUTO_LENGTH, &str));
    ASSERT_CHECK_CALL(napi_map_set_property(env, map, str, str));
    bool hasProp = false;
    ASSERT_CHECK_CALL(napi_map_has_property(env, map, str, &hasProp));
    ASSERT_TRUE(hasProp);
}

/**
 * @tc.name: MapSetNamedPropertyWithMultiContext001
 * @tc.desc: Test napi_map_set_named_property when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, MapSetNamedPropertyWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &map));
    napi_value str = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, __func__, NAPI_AUTO_LENGTH, &str));
    ASSERT_CHECK_CALL(napi_map_set_named_property(env, map, __func__, str));
    bool hasProp = false;
    ASSERT_CHECK_CALL(napi_map_has_property(env, map, str, &hasProp));
    ASSERT_TRUE(hasProp);
}

/**
 * @tc.name: MapGetPropertyWithMultiContext001
 * @tc.desc: Test napi_map_get_property when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, MapGetPropertyWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &map));
    napi_value str = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, __func__, NAPI_AUTO_LENGTH, &str));
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_map_get_property(env, map, str, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_undefined);
}

/**
 * @tc.name: MapGetPropertyWithMultiContext002
 * @tc.desc: Test napi_map_get_property when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, MapGetPropertyWithMultiContext002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &map));
    napi_value str = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, __func__, NAPI_AUTO_LENGTH, &str));
    ASSERT_CHECK_CALL(napi_map_set_property(env, map, str, str));
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_map_get_property(env, map, str, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_string);
}

/*
 * @tc.name: MapGetNamedPropertyWithMultiContext001
 * @tc.desc: Test napi_map_get_named_property when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, MapGetNamedPropertyWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &map));
    napi_value str = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, __func__, NAPI_AUTO_LENGTH, &str));
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_map_get_named_property(env, map, __func__, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_undefined);
}

/**
 * @tc.name: MapGetNamedPropertyWithMultiContext002
 * @tc.desc: Test napi_map_get_named_property when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, MapGetNamedPropertyWithMultiContext002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &map));
    napi_value str = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, __func__, NAPI_AUTO_LENGTH, &str));
    ASSERT_CHECK_CALL(napi_map_set_property(env, map, str, str));
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_map_get_named_property(env, map, __func__, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_string);
}


/**
 * @tc.name: MapHasPropertyWithMultiContext001
 * @tc.desc: Test napi_map_get_property when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, MapHasPropertyWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &map));
    napi_value str = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, __func__, NAPI_AUTO_LENGTH, &str));
    bool result = true;
    ASSERT_CHECK_CALL(napi_map_has_property(env, map, str, &result));
    ASSERT_FALSE(result);
}

/**
 * @tc.name: MapHasPropertyWithMultiContext002
 * @tc.desc: Test napi_map_get_property when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, MapHasPropertyWithMultiContext002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &map));
    napi_value str = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, __func__, NAPI_AUTO_LENGTH, &str));
    ASSERT_CHECK_CALL(napi_map_set_property(env, map, str, str));
    bool result = false;
    ASSERT_CHECK_CALL(napi_map_has_property(env, map, str, &result));
    ASSERT_TRUE(result);
}

/**
 * @tc.name: MapHasNamedPropertyWithMultiContext001
 * @tc.desc: Test napi_map_has_named_property when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, MapHasNamedPropertyWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &map));
    napi_value str = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, __func__, NAPI_AUTO_LENGTH, &str));
    bool result = true;
    ASSERT_CHECK_CALL(napi_map_has_named_property(env, map, __func__, &result));
    ASSERT_FALSE(result);
}

/**
 * @tc.name: MapHasNamedPropertyWithMultiContext002
 * @tc.desc: Test napi_map_has_named_property when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, MapHasNamedPropertyWithMultiContext002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &map));
    napi_value str = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, __func__, NAPI_AUTO_LENGTH, &str));
    ASSERT_CHECK_CALL(napi_map_set_property(env, map, str, str));
    bool result = false;
    ASSERT_CHECK_CALL(napi_map_has_named_property(env, map, __func__, &result));
    ASSERT_TRUE(result);
}

/**
 * @tc.name: MapDeleteNamedPropertyWithMultiContext001
 * @tc.desc: Test napi_map_delete_property when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, MapDeleteNamedPropertyWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &map));
    napi_value str = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, __func__, NAPI_AUTO_LENGTH, &str));
    ASSERT_CHECK_CALL(napi_map_delete_property(env, map, str));
}

/**
 * @tc.name: MapDeleteNamedPropertyWithMultiContext002
 * @tc.desc: Test napi_map_delete_property when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, MapDeleteNamedPropertyWithMultiContext002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &map));
    napi_value str = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, __func__, NAPI_AUTO_LENGTH, &str));
    ASSERT_CHECK_CALL(napi_map_set_property(env, map, str, str));
    bool hasProp = false;
    ASSERT_CHECK_CALL(napi_map_has_property(env, map, str, &hasProp));
    ASSERT_TRUE(hasProp);
    ASSERT_CHECK_CALL(napi_map_delete_property(env, map, str));
    hasProp = true;
    ASSERT_CHECK_CALL(napi_map_has_property(env, map, str, &hasProp));
    ASSERT_FALSE(hasProp);
}

/**
 * @tc.name: MapClearWithMultiContext001
 * @tc.desc: Test napi_map_clear when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, MapClearWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &map));
    ASSERT_CHECK_CALL(napi_map_clear(env, map));
}

/**
 * @tc.name: MapClearWithMultiContext002
 * @tc.desc: Test napi_map_clear when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, MapClearWithMultiContext002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &map));
    napi_value str = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, __func__, NAPI_AUTO_LENGTH, &str));
    ASSERT_CHECK_CALL(napi_map_set_property(env, map, str, str));
    uint32_t size = 0;
    ASSERT_CHECK_CALL(napi_map_get_size(env, map, &size));
    ASSERT_GT(size, 0);
    ASSERT_CHECK_CALL(napi_map_clear(env, map));
    ASSERT_CHECK_CALL(napi_map_get_size(env, map, &size));
    ASSERT_EQ(size, 0);
}

/**
 * @tc.name: MapGetSizeWithMultiContext001
 * @tc.desc: Test napi_map_get_size when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, MapGetSizeWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &map));;
    uint32_t size = 0;
    ASSERT_CHECK_CALL(napi_map_get_size(env, map, &size));
    ASSERT_EQ(size, 0);
}

/**
 * @tc.name: MapGetSizeWithMultiContext002
 * @tc.desc: Test napi_map_get_size when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, MapGetSizeWithMultiContext002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value map = nullptr;
    ASSERT_CHECK_CALL(napi_create_map(env, &map));
    napi_value str = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, __func__, NAPI_AUTO_LENGTH, &str));
    ASSERT_CHECK_CALL(napi_map_set_property(env, map, str, str));
    uint32_t size = 0;
    ASSERT_CHECK_CALL(napi_map_get_size(env, map, &size));
    ASSERT_EQ(size, 1);
}

/**
 * @tc.name: MapGetEntriesWithMultiContext001
 * @tc.desc: Test napi_map_get_entries when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, MapGetEntriesWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

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
    ASSERT_CHECK_CALL(napi_get_named_property(env, entries0, CONST_STRING_VALUE, &entries0Value));
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
    ASSERT_CHECK_CALL(napi_get_named_property(env, end, CONST_STRING_DONE, &done));
    bool isDone;
    ASSERT_CHECK_CALL(napi_get_value_bool(env, done, &isDone));
    ASSERT_TRUE(isDone);
}

/**
 * @tc.name: MapGetKeysWithMultiContext001
 * @tc.desc: Test napi_map_get_keys when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, MapGetKeysWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

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
    ASSERT_CHECK_CALL(napi_get_named_property(env, keys0, CONST_STRING_VALUE, &key));
    int32_t nativeKey;
    ASSERT_CHECK_CALL(napi_get_value_int32(env, key, &nativeKey));
    ASSERT_EQ(nativeKey, 0);

    napi_value end;
    ASSERT_CHECK_CALL(napi_map_iterator_get_next(env, keys, &end));
    napi_value done = nullptr;
    ASSERT_CHECK_CALL(napi_get_named_property(env, end, CONST_STRING_DONE, &done));
    bool isDone;
    ASSERT_CHECK_CALL(napi_get_value_bool(env, done, &isDone));
    ASSERT_TRUE(isDone);
}

/**
 * @tc.name: MapGetValuesWithMultiContext001
 * @tc.desc: Test napi_map_get_values when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, MapGetValuesWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

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
    ASSERT_CHECK_CALL(napi_get_named_property(env, values0, CONST_STRING_VALUE, &value));
    int32_t nativeValue;
    ASSERT_CHECK_CALL(napi_get_value_int32(env, value, &nativeValue));
    ASSERT_EQ(nativeValue, 1);

    napi_value end;
    ASSERT_CHECK_CALL(napi_map_iterator_get_next(env, values, &end));
    napi_value done = nullptr;
    ASSERT_CHECK_CALL(napi_get_named_property(env, end, CONST_STRING_DONE, &done));
    bool isDone;
    ASSERT_CHECK_CALL(napi_get_value_bool(env, done, &isDone));
    ASSERT_TRUE(isDone);
}

/**
 * @tc.name: CreateLimitRuntimeWithMultiContext001
 * @tc.desc: Test napi_create_limit_runtime when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateLimitRuntimeWithMultiContext001, testing::ext::TestSize.Level1)
{
    std::thread t([]() {
        static bool executed = false;
        BasicDeathTest(
            []() {
                NativeEngineProxy rootEngine;
                NativeEngineProxy contextEngine(*rootEngine);
                napi_env limitEnv = nullptr;
                napi_create_limit_runtime(contextEngine, &limitEnv);
            },
            [](std::string, std::string err) {
                executed = true;
                ASSERT_NE(err.find("(napi_create_limit_runtime)] multi-context does not support this interface"),
                        std::string::npos);
            })
            .Run();
        ASSERT_TRUE(executed);
    });
    t.join();
}

/**
 * @tc.name: RegisterModWithJsWithinMultiContext001
 * @tc.desc: Test napi_module_with_js_register when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, RegisterModWithJsWithinMultiContext001, testing::ext::TestSize.Level1)
{
    napi_module_with_js mixedModule = {
        .nm_version = 0,
        .nm_flags = 0,
        .nm_filename = __func__,
        .nm_register_func = [](napi_env, napi_value) -> napi_value { return nullptr; },
        .nm_modname = __func__,
        .nm_priv = nullptr,
        .nm_get_abc_code = nullptr,
        .nm_get_js_code = nullptr,
    };
    // Fixme: Add LoggerCollector to watch logs, make sure module is registered succeed.
    napi_module_with_js_register(&mixedModule);
}

/**
 * @tc.name: IsCallableWithMultiContext001
 * @tc.desc: Test napi_is_callable when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, IsCallableWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value undefined = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &undefined));
    bool isCallable = false;
    ASSERT_CHECK_CALL(napi_is_callable(env, undefined, &isCallable));
    ASSERT_FALSE(isCallable);
}

/**
 * @tc.name: IsCallableWithMultiContext002
 * @tc.desc: Test napi_is_callable when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, IsCallableWithMultiContext002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value func = nullptr;
    ASSERT_CHECK_CALL(napi_create_function(env, __func__, NAPI_AUTO_LENGTH, EmptyNapiCallback, nullptr, &func));
    bool isCallable = false;
    ASSERT_CHECK_CALL(napi_is_callable(env, func, &isCallable));
    ASSERT_TRUE(isCallable);
}

static bool g_finalizer = false;
static void* TestDetachCallback(napi_env env, void* nativeObject, void* hint)
{
    return nativeObject;
}

static void TestDetachFinalizer(void* detachedObject, void* finalizeHint)
{
    ASSERT_FALSE(g_finalizer);
    g_finalizer = true;
}

static napi_value TestAttachCallback(napi_env env, void* nativeObject, void* hint)
{
    napi_value object = nullptr;
    napi_value number = nullptr;
    uint32_t data = 0;
    if (hint != nullptr) {
        object = reinterpret_cast<napi_value>(nativeObject);
        data = CONST_U32_2K;
        napi_create_uint32(env, data, &number);
    } else {
        napi_create_object(env, &object);
        data = CONST_U32_1K;
        napi_create_uint32(env, data, &number);
    }
    napi_set_named_property(env, object, CONST_STRING_NUMBER, number);
    return object;
}
/**
 * @tc.name: SerializeInnerWithMultiContext001
 * @tc.desc: Test napi_serialize_inner when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, SerializeInnerWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

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
    napi_get_named_property(env, result, CONST_STRING_NUMBER, &number);
    ASSERT_CHECK_VALUE_TYPE(env, number, napi_number);
    uint32_t numData = 0;
    napi_get_value_uint32(env, number, &numData);
    ASSERT_EQ(numData, CONST_U32_1K);
}

/**
 * @tc.name: SerializeInnerWithMultiContext002
 * @tc.desc: Test napi_serialize_inner when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, SerializeInnerWithMultiContext002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

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
    napi_get_named_property(env, result, CONST_STRING_NUMBER, &number);
    ASSERT_CHECK_VALUE_TYPE(env, number, napi_number);
    uint32_t numData = 0;
    napi_get_value_uint32(env, number, &numData);
    ASSERT_EQ(numData, CONST_U32_2K);
}

/**
 * @tc.name: SerializeInnerWithMultiContext003
 * @tc.desc: Test napi_serialize_inner when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, SerializeInnerWithMultiContext003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value num = nullptr;
    napi_create_uint32(env, CONST_U32_1K, &num);
    void* data = nullptr;
    napi_serialize_inner(env, num, undefined, undefined, false, true, &data);
    ASSERT_NE(data, nullptr);

    napi_value result = nullptr;
    napi_deserialize(env, data, &result);
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_number);
    napi_delete_serialization_data(env, data);
    int32_t resultData = 0;
    napi_get_value_int32(env, result, &resultData);
    ASSERT_EQ(resultData, CONST_U32_1K);
}

/**
 * @tc.name: SerializeInnerWithMultiContext004
 * @tc.desc: Test napi_serialize_inner when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, SerializeInnerWithMultiContext004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);


    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value num = nullptr;
    napi_create_uint32(env, CONST_U32_1K, &num);
    void* data = nullptr;
    napi_serialize_inner(env, num, undefined, undefined, false, true, &data);
    ASSERT_NE(data, nullptr);

    napi_value result1 = nullptr;
    napi_deserialize(env, data, &result1);
    ASSERT_CHECK_VALUE_TYPE(env, result1, napi_number);
    int32_t resultData1 = 0;
    napi_get_value_int32(env, result1, &resultData1);
    ASSERT_EQ(resultData1, CONST_U32_1K);

    napi_value result2 = nullptr;
    napi_deserialize(env, data, &result2);
    ASSERT_CHECK_VALUE_TYPE(env, result2, napi_number);
    int32_t resultData2 = 0;
    napi_get_value_int32(env, result2, &resultData2);
    ASSERT_EQ(resultData2, CONST_U32_1K);

    napi_delete_serialization_data(env, data);
}

/**
 * @tc.name: SerializeInnerWithMultiContext005
 * @tc.desc: Test napi_serialize_inner when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, SerializeInnerWithMultiContext005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

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
    napi_get_named_property(env, result1, CONST_STRING_NUMBER, &number1);
    ASSERT_CHECK_VALUE_TYPE(env, number1, napi_number);
    uint32_t numData1 = 0;
    napi_get_value_uint32(env, number1, &numData1);
    ASSERT_EQ(numData1, CONST_U32_2K);

    napi_value result2 = nullptr;
    napi_deserialize(env, data, &result2);
    ASSERT_CHECK_VALUE_TYPE(env, result2, napi_object);
    napi_value number2 = nullptr;
    napi_get_named_property(env, result2, CONST_STRING_NUMBER, &number2);
    ASSERT_CHECK_VALUE_TYPE(env, number2, napi_number);
    uint32_t numData2 = 0;
    napi_get_value_uint32(env, number2, &numData2);
    ASSERT_EQ(numData2, CONST_U32_2K);

    napi_delete_serialization_data(env, data);
}

/**
 * @tc.name: SerializeInnerWithMultiContext006
 * @tc.desc: Test napi_serialize_inner when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, SerializeInnerWithMultiContext006, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    const char objKey[] = "objKey";
    const char numKey[] = "numKey";

    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_value num = nullptr;
    napi_create_uint32(env, CONST_U32_1K, &num);
    napi_set_named_property(env, object, numKey, num);
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    napi_set_named_property(env, object, objKey, obj);

    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    void* data = nullptr;
    napi_serialize_inner(env, object, undefined, undefined, false, true, &data);
    ASSERT_NE(data, nullptr);

    napi_value result1 = nullptr;
    napi_deserialize(env, data, &result1);
    ASSERT_CHECK_VALUE_TYPE(env, result1, napi_object);
    napi_value obj1 = nullptr;
    napi_get_named_property(env, result1, objKey, &obj1);
    ASSERT_CHECK_VALUE_TYPE(env, obj1, napi_object);

    napi_value result2 = nullptr;
    napi_deserialize(env, data, &result2);
    ASSERT_CHECK_VALUE_TYPE(env, result2, napi_object);
    napi_value num1 = nullptr;
    napi_get_named_property(env, result2, numKey, &num1);
    uint32_t value1 = 0;
    napi_get_value_uint32(env, num1, &value1);
    ASSERT_EQ(value1, CONST_U32_1K);

    napi_delete_serialization_data(env, data);
}

/**
 * @tc.name: RunActorWithMultiContext001
 * @tc.desc: Test napi_run_actor when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, RunActorWithMultiContext001, testing::ext::TestSize.Level1)
{
    std::thread t([]() {
        static bool executed = false;
        BasicDeathTest(
            []() {
                NativeEngineProxy rootEngine;
                NativeEngineProxy contextEngine(*rootEngine);
                napi_value result = nullptr;
                char buf[16] { 0 };
                napi_run_actor(contextEngine, "", buf, &result);
            },
            [](std::string, std::string err) {
                executed = true;
                ASSERT_NE(err.find("(napi_run_actor)] multi-context does not support this interface"),
                          std::string::npos);
            })
            .Run();
        ASSERT_TRUE(executed);
    });
    t.join();
}

/**
 * @tc.name: WrapAsyncFinalizerWithinMultiContext001
 * @tc.desc: Test napi_wrap_with_size when context is sub context.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, WrapAsyncFinalizerWithinMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value object = nullptr;
    pid_t callbackTid = 0;
    const EcmaVM* vm = reinterpret_cast<ArkNativeEngine*>(engine_)->GetEcmaVm();
    {
        panda::LocalScope scope(vm);
        ASSERT_CHECK_CALL(napi_create_object(env, &object));
        ASSERT_CHECK_CALL(napi_wrap_async_finalizer(
            env, object, reinterpret_cast<void*>(&callbackTid),
            [](napi_env, void* data, void* hint) {
                std::cout << "finalize callback is executing at thread: " << gettid() << std::endl;
                *reinterpret_cast<pid_t*>(data) = gettid();
                STOP_EVENT_LOOP(reinterpret_cast<napi_env>(hint));
            }, engine_, nullptr, 0));
    }
    panda::JSNApi::TriggerGC(vm, panda::ecmascript::GCReason::OTHER, panda::JSNApi::TRIGGER_GC_TYPE::FULL_GC);
    RUN_EVENT_LOOP(reinterpret_cast<napi_env>(engine_));
    ASSERT_NE(callbackTid, 0);
    ASSERT_NE(callbackTid, gettid());
}

/**
 * @tc.name: NapiHasOwnPropertyTest001
 * @tc.desc: Test napi_has_own_property when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiHasOwnPropertyTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value obj = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));
    napi_value key = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, 1, &key));
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, 1, &value));

    ASSERT_CHECK_CALL(napi_set_property(env, obj, key, value));
    bool result = false;
    ASSERT_CHECK_CALL(napi_has_own_property(env, obj, key, &result));
    ASSERT_TRUE(result);
}

/**
 * @tc.name: NapiSetNamedPropertyTest001
 * @tc.desc: Test napi_set_named_property when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiSetNamedPropertyTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);
    napi_value obj = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));
    napi_value key = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_KEY, NAPI_AUTO_LENGTH, &key));
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_VALUE, NAPI_AUTO_LENGTH, &value));
    ASSERT_CHECK_CALL(napi_set_named_property(env, obj, TEST_KEY, value));
    bool result = false;
    ASSERT_CHECK_CALL(napi_has_own_property(env, obj, key, &result));
    ASSERT_TRUE(result);
}

/**
 * @tc.name: NapiHasNamedPropertyTest001
 * @tc.desc: Test napi_has_named_property when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiHasNamedPropertyTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);
    napi_value obj;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_VALUE, NAPI_AUTO_LENGTH, &value));
    ASSERT_CHECK_CALL(napi_set_named_property(env, obj, TEST_KEY, value));
    bool result = false;
    ASSERT_CHECK_CALL(napi_has_named_property(env, obj, TEST_KEY, &result));
    ASSERT_TRUE(result);
}

/**
 * @tc.name: NapiGetNamedPropertyTest001
 * @tc.desc: Test napi_get_named_property when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiGetNamedPropertyTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);
    napi_value obj;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_VALUE, NAPI_AUTO_LENGTH, &value));
    ASSERT_CHECK_CALL(napi_set_named_property(env, obj, TEST_KEY, value));
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_get_named_property(env, obj, TEST_KEY, &result));

    char* buffer = nullptr;
    size_t bufferSize = 0;
    size_t strLength = 0;
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, result, nullptr, 0, &bufferSize));
    ASSERT_GT(bufferSize, static_cast<size_t>(0));
    buffer = new char[bufferSize + 1]{ 0 };
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, result, buffer, bufferSize + 1, &strLength));
    ASSERT_STREQ(TEST_VALUE, buffer);
    ASSERT_EQ(strlen(TEST_VALUE), strLength);
    delete []buffer;
}

/**
 * @tc.name: NapiObjectElementTest001
 * @tc.desc: Test object element setting and getting capability when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiObjectElementTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);
    napi_value array = nullptr;
    ASSERT_CHECK_CALL(napi_create_array(env, &array));
    ASSERT_NE(array, nullptr);
    bool isArray = false;
    ASSERT_CHECK_CALL(napi_is_array(env, array, &isArray));
    ASSERT_TRUE(isArray);

    for (size_t i = 0; i < TEST_ARRAY_LENGTH; i++) {
        napi_value num = nullptr;
        ASSERT_CHECK_CALL(napi_create_uint32(env, i, &num));
        ASSERT_CHECK_CALL(napi_set_element(env, array, i, num));
    }

    uint32_t arrayLength = 0;
    ASSERT_CHECK_CALL(napi_get_array_length(env, array, &arrayLength));
    ASSERT_EQ(arrayLength, static_cast<uint32_t>(TEST_ARRAY_LENGTH));

    for (size_t i = 0; i < arrayLength; i++) {
        bool hasIndex = false;
        ASSERT_CHECK_CALL(napi_has_element(env, array, i, &hasIndex));
        ASSERT_TRUE(hasIndex);
        napi_value value = nullptr;
        ASSERT_CHECK_CALL(napi_get_element(env, array, i, &value));
        uint32_t number = 0;
        napi_get_value_uint32(env, value, &number);
        ASSERT_EQ(number, i);
    }

    for (size_t i = 0; i < arrayLength; i++) {
        bool isDelete = false;
        napi_delete_element(env, array, i, &isDelete);
        ASSERT_TRUE(isDelete);
    }
}

/**
 * @tc.name: NapiDefinePropertiesTest001
 * @tc.desc: Test napi_define_properties when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiDefinePropertiesTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value result = nullptr;
    // Create napi_values for 123, 456 and 789
    napi_value enumerable = nullptr;
    napi_value writAble = nullptr;
    napi_value configurAble = nullptr;
    napi_create_int32(env, 123, &enumerable);
    napi_create_int32(env, 456, &writAble);
    napi_create_int32(env, 789, &configurAble);

    napi_property_descriptor descriptors[] = {
        {ENUMERABLE, nullptr, nullptr, nullptr, nullptr, enumerable, napi_default_method, nullptr},
        {WRITEABLE, nullptr, nullptr, nullptr, nullptr, writAble,
            static_cast<napi_property_attributes>(napi_enumerable | napi_writable), nullptr},
        {CONFIGURABLE, nullptr, nullptr, nullptr, nullptr, configurAble,
            static_cast<napi_property_attributes>(napi_enumerable | napi_configurable), nullptr}
    };

    ASSERT_CHECK_CALL(napi_create_object(env, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_object);
    ASSERT_CHECK_CALL(napi_define_properties(env, result, sizeof(descriptors) / sizeof(descriptors[0]), descriptors));

    bool ret = false;
    ASSERT_CHECK_CALL(napi_has_named_property(env, result, ENUMERABLE, &ret));
    ASSERT_TRUE(ret);

    ret = false;
    ASSERT_CHECK_CALL(napi_has_named_property(env, result, WRITEABLE, &ret));
    ASSERT_TRUE(ret);

    ret = false;
    ASSERT_CHECK_CALL(napi_has_named_property(env, result, CONFIGURABLE, &ret));
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: NapiIsArrayTest001
 * @tc.desc: Test array when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiIsArrayTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);
    napi_value array = nullptr;
    ASSERT_CHECK_CALL(napi_create_array(env, &array));
    ASSERT_NE(array, nullptr);
    bool isArray = false;
    ASSERT_CHECK_CALL(napi_is_array(env, array, &isArray));
    ASSERT_TRUE(isArray);
}

/**
 * @tc.name: NapiGetArrayLengthTest001
 * @tc.desc: Test napi_get_array_length when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiGetArrayLengthTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);
    napi_value array = nullptr;
    ASSERT_CHECK_CALL(napi_create_array(env, &array));
    ASSERT_NE(array, nullptr);

    uint32_t arrayLength = 0;
    ASSERT_CHECK_CALL(napi_get_array_length(env, array, &arrayLength));
    ASSERT_EQ(arrayLength, 0);
}

/**
 * @tc.name: NapiStrictEqualsTest001
 * @tc.desc: Test napi_strict_equals when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiStrictEqualsTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value testString = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_STR, NAPI_AUTO_LENGTH, &testString));
    bool isStrictEquals = false;
    ASSERT_CHECK_CALL(napi_strict_equals(env, testString, testString, &isStrictEquals));
    ASSERT_TRUE(isStrictEquals);
}

/**
 * @tc.name: NapiCallFunctionTest001
 * @tc.desc: Test napi_call_function when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiCallFunctionTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

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
        napi_value result = nullptr;
        if (argv) {
            result = argv[0];
            delete[] argv;
        }
        return result;
    };

    napi_value fn = nullptr;
    napi_value funcResultValue = nullptr;
    napi_value recv = nullptr;
    napi_value jsNumber = nullptr;
    napi_status status = napi_create_function(env, nullptr, 0, func, (void*)TEST_STR, &fn);
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
}

/**
 * @tc.name: NapiNewInstanceTest001
 * @tc.desc: Test napi_new_instance when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiNewInstanceTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value testClass = nullptr;
    napi_define_class(env, TEST_CLASS, NAPI_AUTO_LENGTH,
       [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
            return thisVar;
        }, nullptr, 0, nullptr, &testClass);

    napi_value instanceValue = nullptr;
    ASSERT_CHECK_CALL(napi_new_instance(env, testClass, 0, nullptr, &instanceValue));
}

/**
 * @tc.name: NapiGetNewTargetTest001
 * @tc.desc: Test napi_new_instance when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiGetNewTargetTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value testClass = nullptr;
    napi_define_class(env, TEST_CLASS, NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
            napi_value constructor = nullptr;
            napi_get_new_target(env, info, &constructor);
            if (constructor == nullptr) {
                napi_throw_error(env, nullptr, "is not new instance");
            }
            return thisVar;
        }, nullptr, 0, nullptr, &testClass);

    napi_value instanceValue = nullptr;
    ASSERT_CHECK_CALL(napi_new_instance(env, testClass, 0, nullptr, &instanceValue));
    bool isInstanceOf = false;
    ASSERT_CHECK_CALL(napi_instanceof(env, instanceValue, testClass, &isInstanceOf));
    ASSERT_TRUE(isInstanceOf);
}

/**
 * @tc.name: NapiWrapTest001
 * @tc.desc: Test napi_wrap when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiWrapTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
    auto finalizer = [](napi_env, void* data, void*) {
        delete reinterpret_cast<uint8_t*>(data);
    };
    uint8_t* data0 = new uint8_t;
    ASSERT_CHECK_CALL(napi_wrap(env, object, data0, finalizer, nullptr, nullptr));
}

/**
 * @tc.name: NapiUnwrapTest001
 * @tc.desc: Test napi_unwrap when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiUnwrapTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
    auto finalizer = [](napi_env, void* data, void*) {
        delete reinterpret_cast<char*>(data);
    };

    char *str = strdup(TEST_STR);
    ASSERT_CHECK_CALL(napi_wrap(env, object, (void*)str, finalizer, nullptr, nullptr));
    void* result = nullptr;

    ASSERT_CHECK_CALL(napi_unwrap(env, object, &result));
    ASSERT_STREQ(reinterpret_cast<char*>(result), str);
}

/**
 * @tc.name: NapiRemoveWrapTest001
 * @tc.desc: Test napi_remove_wrap when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiRemoveWrapTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
    auto finalizer = [](napi_env, void* data, void* hint) {
        (void) data;
    };
    ASSERT_CHECK_CALL(napi_wrap(env, object, (void*)TEST_STR, finalizer, nullptr, nullptr));
    void* result = nullptr;

    ASSERT_CHECK_CALL(napi_remove_wrap(env, object, &result));
    ASSERT_STREQ(reinterpret_cast<char*>(result), TEST_STR);
}

/**
 * @tc.name: NapiCreateExternalTest001
 * @tc.desc: Test napi_create_external when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiCreateExternalTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value external = nullptr;
    ASSERT_CHECK_CALL(napi_create_external(env, (void*)TEST_STR,
        [](napi_env env, void* data, void* hint) {
            ASSERT_STREQ((const char*)data, (const char*)hint);
        }, (void*)TEST_STR, &external));

    ASSERT_CHECK_VALUE_TYPE(env, external, napi_external);
    void* tmpExternal = nullptr;
    ASSERT_CHECK_CALL(napi_get_value_external(env, external, &tmpExternal));
    ASSERT_TRUE(tmpExternal);
    ASSERT_EQ(tmpExternal, TEST_STR);
}

/**
 * @tc.name: NapiCreateReferenceTest001
 * @tc.desc: Test napi_create_reference when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiCreateReferenceTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &result));

    napi_ref resultRef = nullptr;
    ASSERT_CHECK_CALL(napi_create_reference(env, result, 1, &resultRef));

    uint32_t resultRefCount = 0;

    ASSERT_CHECK_CALL(napi_reference_ref(env, resultRef, &resultRefCount));
    ASSERT_EQ(resultRefCount, REFERENCE_SIZE);

    ASSERT_CHECK_CALL(napi_reference_unref(env, resultRef, &resultRefCount));
    ASSERT_EQ(resultRefCount, 1);

    napi_value refValue = nullptr;
    ASSERT_CHECK_CALL(napi_get_reference_value(env, resultRef, &refValue));
    ASSERT_NE(refValue, nullptr);
    bool isStrictEquals = false;
    ASSERT_CHECK_CALL(napi_strict_equals(env, result, refValue, &isStrictEquals));
    ASSERT_TRUE(isStrictEquals);
    ASSERT_CHECK_CALL(napi_delete_reference(env, resultRef));
}

/**
 * @tc.name: NapiScopeTest001
 * @tc.desc: Test napi scope when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiScopeTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

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
        napi_create_string_latin1(env, MESSAGE, NAPI_AUTO_LENGTH, &messageKey);
        napi_value messageValue = nullptr;
        napi_create_string_latin1(env, MESSAGE, NAPI_AUTO_LENGTH, &messageValue);
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

    ASSERT_CHECK_CALL(napi_create_function(env, TEST_FUNC, NAPI_AUTO_LENGTH, func, nullptr, &funcValue));
    ASSERT_NE(funcValue, nullptr);

    napi_handle_scope parentScope = nullptr;
    ASSERT_CHECK_CALL(napi_open_handle_scope(env, &parentScope));
    ASSERT_NE(parentScope, nullptr);

    napi_escapable_handle_scope childScope = nullptr;
    ASSERT_CHECK_CALL(napi_open_escapable_handle_scope(env, &childScope));
    ASSERT_NE(childScope, nullptr);

    napi_value funcResultValue = nullptr;
    napi_value newFuncResultValue = nullptr;
    ASSERT_CHECK_CALL(napi_call_function(env, recv, funcValue, 0, nullptr, &funcResultValue));
    ASSERT_NE(funcResultValue, nullptr);

    ASSERT_CHECK_CALL(napi_escape_handle(env, childScope, funcResultValue, &newFuncResultValue));
    ASSERT_CHECK_CALL(napi_close_escapable_handle_scope(env, childScope));
    ASSERT_TRUE(newFuncResultValue != nullptr);
    ASSERT_CHECK_VALUE_TYPE(env, newFuncResultValue, napi_object);
    ASSERT_CHECK_CALL(napi_close_handle_scope(env, parentScope));
}

/**
 * @tc.name: NapiThrowTest001
 * @tc.desc: Test napi_throw when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiThrowTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value code = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_ERROR_CODE, NAPI_AUTO_LENGTH, &code));

    napi_value message = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_ERROR_MESSAGE, NAPI_AUTO_LENGTH, &message));

    napi_value error = nullptr;
    ASSERT_CHECK_CALL(napi_create_error(env, code, message, &error));
    ASSERT_TRUE(error != nullptr);

    bool isError = false;
    ASSERT_CHECK_CALL(napi_is_error(env, error, &isError));
    ASSERT_TRUE(isError);
    ASSERT_CHECK_CALL(napi_throw(env, error));

    bool isExceptionPending = false;
    ASSERT_CHECK_CALL(napi_is_exception_pending(env, &isExceptionPending));
    ASSERT_TRUE(isExceptionPending);

    napi_value exception = nullptr;
    ASSERT_CHECK_CALL(napi_get_and_clear_last_exception(env, &exception));
    ASSERT_CHECK_CALL(napi_is_exception_pending(env, &isExceptionPending));
    ASSERT_FALSE(isExceptionPending);
}

/**
 * @tc.name: NapiThrowErrorTest001
 * @tc.desc: Test napi_throw_error when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiThrowErrorTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    ASSERT_CHECK_CALL(napi_throw_error(env, TEST_ERROR_CODE, TEST_ERROR_MESSAGE));

    bool isExceptionPending = false;
    ASSERT_CHECK_CALL(napi_is_exception_pending(env, &isExceptionPending));
    ASSERT_TRUE(isExceptionPending);
}

/**
 * @tc.name: NapiThrowTypeErrorTest001
 * @tc.desc: Test napi_throw_type_error when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiThrowTypeErrorTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    ASSERT_CHECK_CALL(napi_throw_type_error(env, TEST_ERROR_CODE, TEST_ERROR_MESSAGE));

    bool isExceptionPending = false;
    ASSERT_CHECK_CALL(napi_is_exception_pending(env, &isExceptionPending));
    ASSERT_TRUE(isExceptionPending);
}

/**
 * @tc.name: NapiThrowRangeErrorTest001
 * @tc.desc: Test napi_throw_range_error when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiThrowRangeErrorTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    ASSERT_CHECK_CALL(napi_throw_range_error(env, TEST_ERROR_CODE, TEST_ERROR_MESSAGE));

    bool isExceptionPending = false;
    ASSERT_CHECK_CALL(napi_is_exception_pending(env, &isExceptionPending));
    ASSERT_TRUE(isExceptionPending);
}

/**
 * @tc.name: NapiThrowRangeErrorTest001
 * @tc.desc: Test napi_throw_range_error when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiArrayBufferTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    ASSERT_CHECK_CALL(napi_create_arraybuffer(env, LENGTH, &arrayBufferPtr, &arrayBuffer));
    ASSERT_NE(arrayBuffer, nullptr);
    ASSERT_NE(arrayBufferPtr, nullptr);
    bool isArrayBuffer = false;
    ASSERT_CHECK_CALL(napi_is_arraybuffer(env, arrayBuffer, &isArrayBuffer));
    ASSERT_TRUE(isArrayBuffer);

    void* tmpArrayBufferPtr = nullptr;
    size_t arrayBufferLength = 0;
    ASSERT_CHECK_CALL(napi_get_arraybuffer_info(env, arrayBuffer, &tmpArrayBufferPtr, &arrayBufferLength));

    ASSERT_EQ(arrayBufferPtr, tmpArrayBufferPtr);
    ASSERT_EQ(LENGTH, arrayBufferLength);
}

/**
 * @tc.name: NapiCreateExternalArrayBufferTest001
 * @tc.desc: Test napi_create_external_arraybuffer when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiCreateExternalArrayBufferTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value external = nullptr;
    auto res = napi_create_external_arraybuffer(
        env, (void*)TEST_STR, strlen(TEST_STR),
        [](napi_env env, void* data, void* hint) { ASSERT_STREQ((const char*)data, (const char*)hint); },
        (void*)TEST_STR, &external);
    ASSERT_EQ(res, napi_ok);

    void* tmpArrayBufferPtr = nullptr;
    size_t arrayBufferLength = 0;
    ASSERT_CHECK_CALL(napi_get_arraybuffer_info(env, external, &tmpArrayBufferPtr, &arrayBufferLength));
    ASSERT_EQ(TEST_STR, reinterpret_cast<char*>(tmpArrayBufferPtr));
    ASSERT_EQ(arrayBufferLength, strlen(TEST_STR));
}

/**
 * @tc.name: NapiTypedArrayTest001
 * @tc.desc: Test napi_create_typedarray when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiTypedArrayTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    // create arraybuffer
    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    ASSERT_CHECK_CALL(napi_create_arraybuffer(env, LENGTH, &arrayBufferPtr, &arrayBuffer));
    ASSERT_NE(arrayBuffer, nullptr);
    ASSERT_NE(arrayBufferPtr, nullptr);

    // using arraybuffer to generate typedArray
    napi_value typedArray = nullptr;
    size_t byteOffsetCreate = 0;
    ASSERT_CHECK_CALL(napi_create_typedarray(env, napi_int8_array, LENGTH, arrayBuffer, byteOffsetCreate,
        &typedArray));
    ASSERT_NE(typedArray, nullptr);

    bool isTypedArray = false;
    ASSERT_CHECK_CALL(napi_is_typedarray(env, typedArray, &isTypedArray));
    ASSERT_TRUE(isTypedArray);

    napi_typedarray_type typedarrayType;
    size_t typedarrayLength = 0;
    void* typedarrayBufferPtr = nullptr;
    napi_value tmpArrayBuffer = nullptr;
    size_t byteOffset = 0;

    ASSERT_CHECK_CALL(napi_get_typedarray_info(env, typedArray, &typedarrayType, &typedarrayLength,
        &typedarrayBufferPtr, &tmpArrayBuffer, &byteOffset));

    ASSERT_EQ(typedarrayType, napi_int8_array);
    ASSERT_EQ(typedarrayBufferPtr, arrayBufferPtr);
    ASSERT_EQ(LENGTH, typedarrayLength);
    ASSERT_EQ(byteOffset, byteOffsetCreate);
}

/**
 * @tc.name: NapiDataViewTest001
 * @tc.desc: Test napi_create_dataview when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiDataViewTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    ASSERT_CHECK_CALL(napi_create_arraybuffer(env, LENGTH, &arrayBufferPtr, &arrayBuffer));
    ASSERT_NE(arrayBuffer, nullptr);
    ASSERT_NE(arrayBufferPtr, nullptr);
    bool isArrayBuffer = false;
    napi_is_arraybuffer(env, arrayBuffer, &isArrayBuffer);
    ASSERT_TRUE(isArrayBuffer);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_dataview(env, LENGTH, arrayBuffer, 0, &result));

    bool isDataView = false;
    ASSERT_CHECK_CALL(napi_is_dataview(env, result, &isDataView));

    napi_value retArrayBuffer = nullptr;
    void* data = nullptr;
    size_t byteLength = 0;
    size_t byteOffset = 0;
    ASSERT_CHECK_CALL(napi_get_dataview_info(env, result, &byteLength, &data, &retArrayBuffer, &byteOffset));

    bool retIsArrayBuffer = false;
    ASSERT_CHECK_CALL(napi_is_arraybuffer(env, retArrayBuffer, &retIsArrayBuffer));
    ASSERT_TRUE(retIsArrayBuffer);
    ASSERT_EQ(arrayBufferPtr, data);
    ASSERT_EQ(LENGTH, byteLength);
    ASSERT_EQ(static_cast<size_t>(0), byteOffset);
}

/**
 * @tc.name: NapiGetVersionTest001
 * @tc.desc: Test napi_get_version when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiGetVersionTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    uint32_t result = 0;
    napi_status status = napi_get_version(env, &result);
    ASSERT_EQ(status, napi_ok);
}

HWTEST_F(NapiContextTest, NapiPromiseTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

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
 * @tc.name: NapiRunScriptTest001
 * @tc.desc: Test napi_run_script when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiRunScriptTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value script = nullptr;
    napi_value res = nullptr;
    napi_status status = napi_run_script(env, script, &res);
    ASSERT_EQ(status, napi_invalid_arg);
}

/**
 * @tc.name: NapiDateTest001
 * @tc.desc: Test napi_create_date when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiDateTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    const double time = 202110181203150;
    napi_value date;
    napi_status status = napi_create_date(env, time, &date);
    ASSERT_EQ(status, napi_ok);

    bool isDate = false;
    napi_is_date(env, date, &isDate);
    ASSERT_TRUE(isDate);

    double result;
    status = napi_get_date_value(env, date, &result);
    ASSERT_EQ(status, napi_ok);
    ASSERT_EQ(time, result);
}

/**
 * @tc.name: NapiAddFinalizerTest001
 * @tc.desc: Test napi_add_finalizer when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiAddFinalizerTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value object;
    NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &object));

    static bool testValue = false;
    napi_ref ref = nullptr;
    napi_add_finalizer(
        env, object, (void*)TEST_STR, [](napi_env env, void* data, void* hint) {
            testValue = true;
        }, nullptr, &ref);

    napi_delete_reference(env, ref);
    ASSERT_TRUE(testValue);
}

/**
 * @tc.name: NapiBigIntTest001
 * @tc.desc: Test napi bigint when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiBigIntTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);
    // int64
    {
        int64_t testValue = INT64_MAX;
        napi_value result = nullptr;
        ASSERT_CHECK_CALL(napi_create_bigint_int64(env, testValue, &result));
        ASSERT_CHECK_VALUE_TYPE(env, result, napi_bigint);

        int64_t resultValue = 0;
        bool flag = false;
        ASSERT_CHECK_CALL(napi_get_value_bigint_int64(env, result, &resultValue, &flag));
        ASSERT_EQ(resultValue, INT64_MAX);
        ASSERT_TRUE(flag);
    }

    // uint64
    {
        uint64_t testValue = UINT64_MAX;
        napi_value result = nullptr;
        ASSERT_CHECK_CALL(napi_create_bigint_uint64(env, testValue, &result));
        ASSERT_CHECK_VALUE_TYPE(env, result, napi_bigint);

        bool flag = false;
        uint64_t resultValue = 0;
        ASSERT_CHECK_CALL(napi_get_value_bigint_uint64(env, result, &resultValue, &flag));
        ASSERT_EQ(resultValue, UINT64_MAX);
        ASSERT_TRUE(flag);
    }

    // word
    {
        int signBit = 0;
        size_t wordCount = 4;
        uint64_t words[] = { 0xFFFFFFFFFFFFFFFF, 34ULL, 56ULL, 0xFFFFFFFFFFFFFFFF };
        napi_value result = nullptr;
        ASSERT_CHECK_CALL(napi_create_bigint_words(env, signBit, wordCount, words, &result));

        uint64_t wordsOut[] = { 0ULL, 0ULL, 0ULL, 0ULL };
        ASSERT_CHECK_CALL(napi_get_value_bigint_words(env, result, &signBit, &wordCount, wordsOut));

        ASSERT_EQ(signBit, 0);
        ASSERT_EQ(wordCount, 4);
        for (auto index = 0; index < wordCount; ++index)
        {
            ASSERT_EQ(words[index], wordsOut[index]);
        }
    }
}

/**
 * @tc.name: NapiGetAllPropertyNamesTest001
 * @tc.desc: Test napi_get_all_property_names when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiGetAllPropertyNamesTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_key_collection_mode keyMode = napi_key_own_only;
    napi_key_filter keyFilter = napi_key_all_properties;
    napi_key_conversion keyConversion = napi_key_keep_numbers;
    napi_value result = nullptr;
    napi_value propNames = nullptr;

    ASSERT_CHECK_CALL(napi_create_object(env, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_object);

    napi_value strAttribute = nullptr;
    napi_create_string_utf8(env, TEST_STR, NAPI_AUTO_LENGTH, &strAttribute);
    napi_set_named_property(env, result, STR_ATTRIBUTE, strAttribute);

    int32_t testNumber = 1;
    napi_value numberAttribute = nullptr;
    napi_create_int32(env, testNumber, &numberAttribute);
    napi_set_named_property(env, result, STR_ATTRIBUTE, numberAttribute);

    ASSERT_CHECK_CALL(napi_get_all_property_names(env, result, keyMode, keyFilter, keyConversion, &propNames));

    ASSERT_CHECK_VALUE_TYPE(env, propNames, napi_object);
    bool isArray = false;
    ASSERT_CHECK_CALL(napi_is_array(env, propNames, &isArray));
    ASSERT_TRUE(isArray);
}

typedef struct {
    size_t value;
    bool print;
    napi_ref jsCallback;
} AddonData;

static void DeleteAddonData(napi_env env, void* rawData, void* hint)
{
    AddonData* data = (AddonData*)rawData;
    if (data->print) {
        printf("deleting addon data\n");
    }
    if (data->jsCallback != NULL) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, data->jsCallback));
    }
    delete data;
}

static napi_value SetPrintOnDelete(napi_env env, napi_callback_info info)
{
    AddonData* data;
    NAPI_CALL(env, napi_get_instance_data(env, (void**)&data));
    data->print = true;
    return NULL;
}

static void TestFinalizer(napi_env env, void* rawData, void* hint)
{
    (void)rawData;
    (void)hint;

    AddonData* data;
    napi_value jsResult;
    NAPI_CALL_RETURN_VOID(env, napi_get_instance_data(env, (void**)&data));
    napi_value js_cb = nullptr;
    napi_value undefined = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, data->jsCallback, &js_cb));
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, js_cb, 0, NULL, &jsResult));

    NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, data->jsCallback));
    data->jsCallback = NULL;
}

static napi_value ObjectWithFinalizer(napi_env env, napi_callback_info info)
{
    AddonData* data;

    napi_value value;
    napi_value jsCb;
    size_t argc = 1;

    auto func = [](napi_env env, napi_callback_info info) -> napi_value {
        return nullptr;
    };

    napi_create_function(env, TEST_FUNC, NAPI_AUTO_LENGTH, func, nullptr, &jsCb);

    NAPI_CALL(env, napi_get_instance_data(env, (void**)&data));
    NAPI_ASSERT(env, data->jsCallback == nullptr, "reference must be nullptr");
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &jsCb, nullptr, nullptr));
    NAPI_CALL(env, napi_create_object(env, &value));
    NAPI_CALL(env, napi_add_finalizer(env, value, nullptr, TestFinalizer, nullptr, nullptr));
    NAPI_CALL(env, napi_create_reference(env, jsCb, 1, &data->jsCallback));
    return nullptr;
}

/**
 * @tc.name: NapiInstanceDataTest001
 * @tc.desc: Test instance data when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiInstanceDataTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    // Set instance data
    AddonData* data = new AddonData();
    data->value = 41;
    data->print = false;
    data->jsCallback = NULL;
    ASSERT_CHECK_CALL(napi_set_instance_data(env, data, DeleteAddonData, NULL));

    // Test get instance data
    AddonData* addonData = nullptr;
    ASSERT_CHECK_CALL(napi_get_instance_data(env, (void**)&addonData));
    ++addonData->value;
    const size_t expectValue = 42;
    ASSERT_EQ(addonData->value, expectValue);

    // Test finalizer
    SetPrintOnDelete(env, nullptr);
    ObjectWithFinalizer(env, nullptr);
}

/**
 * @tc.name: NapiDetachArrayBufferTest001
 * @tc.desc: Test napi_is_detached_arraybuffer when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiDetachArrayBufferTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    static constexpr size_t arrayBufferSize = 1024;
    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    bool result = false;
    ASSERT_CHECK_CALL(napi_is_detached_arraybuffer(env, arrayBuffer, &result));
    ASSERT_FALSE(result);

    napi_status status = napi_detach_arraybuffer(env, arrayBuffer);
    if (status == napi_ok) {
        arrayBufferPtr = nullptr;
    }
    ASSERT_EQ(status, napi_ok);

    result = false;
    ASSERT_CHECK_CALL(napi_is_detached_arraybuffer(env, arrayBuffer, &result));
    ASSERT_TRUE(result);
}

/**
 * @tc.name: NapiTypeTagObjectTest001
 * @tc.desc: Test napi_check_object_type_tag when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiTypeTagObjectTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value instance = nullptr;
    bool result;
    for (size_t i = 0; i < TYPE_TAGS_SIZE; i++) {
        napi_create_object(env, &instance);
        napi_type_tag_object(env, instance, &typeTags[i]);
        napi_check_object_type_tag(env, instance, &typeTags[i], &result);
        ASSERT_TRUE(result);
    }
}

/**
 * @tc.name: NapiObjectFreezeTest001
 * @tc.desc: Test napi_object_freeze when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiObjectFreezeTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value object = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &object));
    int32_t testNumber = 0;
    napi_value numberAttribute = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, testNumber, &numberAttribute));

    ASSERT_CHECK_CALL(napi_object_freeze(env, object));
    // Set property after freezed will throw 'Cannot add property in prevent extensions'.
    napi_status status = napi_set_named_property(env, object, TEST_STR, numberAttribute);
    ASSERT_EQ(status, napi_pending_exception);
}

/**
 * @tc.name: NapiObjectSealTest001
 * @tc.desc: Test napi_object_seal when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiObjectSealTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value object = nullptr;
    napi_create_object(env, &object);
    napi_value strAttribute = nullptr;
    napi_create_string_utf8(env, TEST_VALUE, strlen(TEST_VALUE), &strAttribute);
    napi_set_named_property(env, object, TEST_KEY, strAttribute);

    ASSERT_CHECK_CALL(napi_object_seal(env, object));

    bool testDeleted = false;
    napi_value key = nullptr;
    napi_create_string_utf8(env, TEST_KEY, NAPI_AUTO_LENGTH, &key);
    ASSERT_CHECK_CALL(napi_delete_property(env, object, key, &testDeleted));
    ASSERT_FALSE(testDeleted);
}

/**
 * @tc.name: NapiObjectSealTest001
 * @tc.desc: Test napi_object_seal when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiRunScriptPathTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value result = nullptr;
    auto res = napi_run_script_path(env, TEST_STR, &result);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiLoadModuleTest001
 * @tc.desc: Test napi_load_module when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiLoadModuleTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value result = nullptr;
    auto res = napi_load_module(env, nullptr, &result);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiLoadModuleWithInfoTest001
 * @tc.desc: Test napi_load_module_with_info when env is a context env.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, NapiLoadModuleWithInfoTest001, testing::ext::TestSize.Level1)
{
    CheckContextEnv();
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value result = nullptr;
    napi_status res = napi_load_module_with_info(env, STR_MODULE_PATH, nullptr, &result);
    ASSERT_EQ(res, napi_generic_failure);
    const napi_extended_error_info* result2;
    napi_get_last_error_info(env, &result2);
    ASSERT_EQ(result2->error_code, napi_generic_failure);
}

static napi_value TestCallFunc(napi_env env, napi_callback_info info)
{
    return nullptr;
}

/**
 * @tc.name: MakeCallbackWithMultiContext001
 * @tc.desc: Test napi_make_callback when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, MakeCallbackWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value global;
    napi_get_global(env, &global);
    size_t argc = 0;
    napi_value result = nullptr;
    napi_value funcValue = nullptr;
    napi_create_function(env, TEST_STR, NAPI_AUTO_LENGTH, TestCallFunc, nullptr, &funcValue);
    ASSERT_CHECK_CALL(napi_make_callback(env, nullptr, global, funcValue, argc, nullptr, &result));
}

/**
 * @tc.name: GetPrototypeWithMultiContext001
 * @tc.desc: Test napi_get_prototype when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, GetPrototypeWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value obj = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));

    napi_value prototype = nullptr;
    ASSERT_CHECK_CALL(napi_get_prototype(env, obj, &prototype));
    ASSERT_CHECK_VALUE_TYPE(env, prototype, napi_object);
}

/**
 * @tc.name: GetPropertyNamesWithMultiContext001
 * @tc.desc: Test napi_get_property_names when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, GetPropertyNamesWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value obj = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));

    napi_value propNames = nullptr;
    ASSERT_CHECK_CALL(napi_get_property_names(env, obj, &propNames));
    ASSERT_CHECK_VALUE_TYPE(env, propNames, napi_object);
    bool isArray = false;
    ASSERT_CHECK_CALL(napi_is_array(env, propNames, &isArray));
    ASSERT_TRUE(isArray);
}

/**
 * @tc.name: SetPropertyWithMultiContext001
 * @tc.desc: Test napi_set_property when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, SetPropertyWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value obj = nullptr;
    napi_value key = nullptr;
    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));
    ASSERT_CHECK_CALL(napi_create_int32(env, INT_ZERO, &key));
    ASSERT_CHECK_CALL(napi_create_int32(env, INT_THREE, &value));
    ASSERT_CHECK_CALL(napi_set_property(env, obj, key, value));

    napi_value propVal = nullptr;
    int32_t resultValue = 0;
    ASSERT_CHECK_CALL(napi_get_property(env, obj, key, &propVal));
    ASSERT_CHECK_CALL(napi_get_value_int32(env, propVal, &resultValue));
    ASSERT_EQ(resultValue, INT_THREE);
}

/**
 * @tc.name: HasPropertyWithMultiContext001
 * @tc.desc: Test napi_has_property when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, HasPropertyWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value obj = nullptr;
    napi_value key = nullptr;
    bool result = false;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));
    ASSERT_CHECK_CALL(napi_create_int32(env, INT_ZERO, &key));
    ASSERT_CHECK_CALL(napi_has_property(env, obj, key, &result));
    ASSERT_FALSE(result);
}

/**
 * @tc.name: GetPropertyWithMultiContext001
 * @tc.desc: Test napi_get_property when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, GetPropertyWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value obj = nullptr;
    napi_value key = nullptr;
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, PROPERTY_NAME, NAPI_AUTO_LENGTH, &key));
    ASSERT_CHECK_CALL(napi_get_property(env, obj, key, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_function);
}

/**
 * @tc.name: DeletePropertyWithMultiContext001
 * @tc.desc: Test napi_delete_property when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, DeletePropertyWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value obj = nullptr;
    napi_value key = nullptr;
    napi_value value = nullptr;
    bool result;

    ASSERT_CHECK_CALL(napi_create_object(env, &obj));
    ASSERT_CHECK_CALL(napi_create_int32(env, INT_ZERO, &key));
    ASSERT_CHECK_CALL(napi_create_int32(env, INT_TWO, &value));
    ASSERT_CHECK_CALL(napi_set_property(env, obj, key, value));
    ASSERT_CHECK_CALL(napi_delete_property(env, obj, key, &result));
    ASSERT_TRUE(result);
}

/**
 * @tc.name: WrapEnhanceWithMultiContext001
 * @tc.desc: Test napi_wrap_enhance when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, WrapEnhanceWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value testWrapClass = nullptr;
    napi_define_class(
        env, SENDABLE_CLASS_NAME, NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_value thisVar = nullptr;
            napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);

            return thisVar;
        },
        nullptr, 0, nullptr, &testWrapClass);

    napi_value instanceValue = nullptr;
    napi_new_instance(env, testWrapClass, 0, nullptr, &instanceValue);

    size_t size = sizeof(TEST_WRAP_STRING) / sizeof(char);
    napi_wrap_enhance(
        env, instanceValue, (void*)TEST_WRAP_STRING, [](napi_env env, void* data, void* hint) {}, true, nullptr, size,
        nullptr);

    char* tempTestStr = nullptr;
    napi_unwrap(env, instanceValue, (void**)&tempTestStr);
    ASSERT_STREQ(TEST_WRAP_STRING, tempTestStr);

    char* tempTestStr1 = nullptr;
    napi_remove_wrap(env, instanceValue, (void**)&tempTestStr1);
    ASSERT_STREQ(TEST_WRAP_STRING, tempTestStr1);
}

/**
 * @tc.name: ModuleRegisterWithMultiContext001
 * @tc.desc: Test napi_module_register when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, ModuleRegisterWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    static napi_module demoModule = {
        .nm_version = 1,
        .nm_flags = 0,
        .nm_filename = nullptr,
        .nm_register_func = nullptr,
        .nm_modname = MOD_NAME,
        .nm_priv = ((void*)0),
        .reserved = { 0 },
    };
    napi_module_register(&demoModule);
    const napi_extended_error_info* errorInfo;
    ASSERT_CHECK_CALL(napi_get_last_error_info(env, &errorInfo));
    ASSERT_EQ(errorInfo->error_code, napi_ok);
}

/**
 * @tc.name: AsyncDestroyWithMultiContext001
 * @tc.desc: Test napi_async_destroy when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, AsyncDestroyWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    void* arrayBufferPtr = nullptr;
    napi_value arrayBuffer = nullptr;
    size_t arrayBufferSize = ARRAYBUFFER_SIZE;
    napi_value asyncResourceName = nullptr;
    napi_async_context result = nullptr;

    ASSERT_CHECK_CALL(napi_create_string_utf8(env, __func__, NAPI_AUTO_LENGTH, &asyncResourceName));
    ASSERT_CHECK_CALL(napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer));

    ASSERT_CHECK_CALL(napi_async_init(env, arrayBuffer, asyncResourceName, &result));
    EXPECT_NE(result, nullptr);

    ASSERT_CHECK_CALL(napi_async_destroy(env, result));
}

/**
 * @tc.name: IsBufferWithMultiContext001
 * @tc.desc: Test napi_is_buffer when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, IsBufferWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value buffer = nullptr;
    void* bufferPtr = nullptr;
    size_t bufferSize = ARRAYBUFFER_SIZE;

    ASSERT_CHECK_CALL(napi_create_buffer(env, bufferSize, &bufferPtr, &buffer));
    bool isBuffer = false;
    ASSERT_CHECK_CALL(napi_is_buffer(env, buffer, &isBuffer));
    EXPECT_TRUE(isBuffer);
}

/**
 * @tc.name: GetBufferInfoWithMultiContext001
 * @tc.desc: Test napi_get_buffer_info when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, GetBufferInfoWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value buffer = nullptr;
    void* resultData = nullptr;
    size_t bufferSize = ARRAYBUFFER_SIZE;

    ASSERT_CHECK_CALL(napi_create_buffer_copy(env, bufferSize, BUFFER_STR, &resultData, &buffer));
    void* tmpBufferPtr = nullptr;
    size_t bufferLength = 0;
    ASSERT_CHECK_CALL(napi_get_buffer_info(env, buffer, &tmpBufferPtr, &bufferLength));
    EXPECT_EQ(strcmp((char*)tmpBufferPtr, BUFFER_STR), 0);
    EXPECT_NE(tmpBufferPtr, nullptr);
    EXPECT_EQ(bufferLength, ARRAYBUFFER_SIZE);
}

/**
 * @tc.name: DeleteAsyncWorkWithMultiContext001
 * @tc.desc: Test napi_delete_async_work when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, DeleteAsyncWorkWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    struct AsyncWorkContext {
        napi_async_work work = nullptr;
    };
    std::unique_ptr<AsyncWorkContext> asyncWorkContext = std::make_unique<AsyncWorkContext>();
    napi_value resourceName = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_CHAR_ASYNCWORK, NAPI_AUTO_LENGTH, &resourceName));
    ASSERT_CHECK_CALL(napi_create_async_work(
        env, nullptr, resourceName, [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {},
        nullptr, &asyncWorkContext->work));
    ASSERT_CHECK_CALL(napi_delete_async_work(env, asyncWorkContext->work));
}

/**
 * @tc.name: GetNodeVersionWithMultiContext001
 * @tc.desc: Test napi_get_node_version when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, GetNodeVersionWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    const napi_node_version* version = nullptr;
    ASSERT_CHECK_CALL(napi_get_node_version(env, &version));
    ASSERT_EQ(version, nullptr);
}

/**
 * @tc.name: GetUvEventLoopWithMultiContext001
 * @tc.desc: Test napi_get_uv_event_loop when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, GetUvEventLoopWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    struct uv_loop_s* loop = nullptr;
    napi_status status = napi_get_uv_event_loop(env, &loop);
    ASSERT_EQ(status, napi_invalid_arg);
    ASSERT_EQ(loop, nullptr);
}

static napi_value TestFatalException(napi_env env, napi_callback_info info)
{
    napi_value err;
    size_t argc = 1;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &err, nullptr, nullptr));
    NAPI_CALL(env, napi_fatal_exception(env, err));
    return nullptr;
}

/**
 * @tc.name: FatalExceptionWithMultiContext001
 * @tc.desc: Test napi_fatal_exception when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, FatalExceptionWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    ASSERT_EQ(TestFatalException(env, nullptr), nullptr);
}

static void Cleanup(void* arg)
{
    g_hookTag += INT_ONE;
    if (arg != nullptr) {
    }
}
/**
 * @tc.name: AddEnvCleanupHookWithMultiContext001
 * @tc.desc: Test napi_add_env_cleanup_hook when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, AddEnvCleanupHookWithMultiContext001, testing::ext::TestSize.Level1)
{
    napi_env newEnv = nullptr;
    napi_create_ark_context(reinterpret_cast<napi_env>(engine_), &newEnv);
    g_hookTag = INT_ZERO;
    ASSERT_CHECK_CALL(napi_add_env_cleanup_hook(newEnv, Cleanup, nullptr));
    napi_destroy_ark_context(newEnv);
    ASSERT_EQ(g_hookTag, INT_ONE);
}

/**
 * @tc.name: RemoveEnvCleanupHookWithMultiContext001
 * @tc.desc: Test napi_remove_env_cleanup_hook when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, RemoveEnvCleanupHookWithMultiContext001, testing::ext::TestSize.Level1)
{
    napi_env newEnv = nullptr;
    napi_create_ark_context(reinterpret_cast<napi_env>(engine_), &newEnv);
    g_hookTag = INT_ZERO;
    ASSERT_CHECK_CALL(napi_add_env_cleanup_hook(newEnv, Cleanup, &g_hookArgOne));
    ASSERT_CHECK_CALL(napi_add_env_cleanup_hook(newEnv, Cleanup, &g_hookArgTwo));
    ASSERT_CHECK_CALL(napi_remove_env_cleanup_hook(newEnv, Cleanup, &g_hookArgTwo));
    napi_destroy_ark_context(newEnv);
    ASSERT_EQ(g_hookTag, INT_ONE);
}

/**
 * @tc.name: OpenCallbackScopeWithMultiContext001
 * @tc.desc: Test napi_open_callback_scope when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, OpenCallbackScopeWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value obj = nullptr;
    napi_async_context context = nullptr;
    napi_value name;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_STR, NAPI_AUTO_LENGTH, &name));
    ASSERT_CHECK_CALL(napi_async_init(env, nullptr, name, &context));
    EXPECT_NE(context, nullptr);
    napi_callback_scope result = nullptr;
    ASSERT_CHECK_CALL(napi_open_callback_scope(env, obj, context, &result));
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name: CloseCallbackScopeWithMultiContext001
 * @tc.desc: Test napi_close_callback_scope when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CloseCallbackScopeWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value obj = nullptr;
    napi_async_context context = nullptr;
    napi_value name;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_STR, NAPI_AUTO_LENGTH, &name));
    ASSERT_CHECK_CALL(napi_async_init(env, nullptr, name, &context));
    EXPECT_NE(context, nullptr);
    napi_callback_scope result = nullptr;
    ASSERT_CHECK_CALL(napi_open_callback_scope(env, obj, context, &result));
    EXPECT_NE(result, nullptr);
    ASSERT_CHECK_CALL(napi_close_callback_scope(env, result));
}

/**
 * @tc.name: GetThreadsafeFunctionContextWithMultiContext001
 * @tc.desc: Test napi_get_threadsafe_function_context when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, GetThreadsafeFunctionContextWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    UVLoopRunner runner(multiContextEngine_);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value resourceName;
    ASSERT_CHECK_CALL(napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_threadsafe_function tsfn;
    int32_t data = INT_ONE;
    ASSERT_CHECK_CALL(napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 0, 1, &data,
        [](napi_env env, void* context, void*) {}, &data,
        [](napi_env env, napi_value jsCb, void *context, void *data) {}, &tsfn));
    void* context = nullptr;
    ASSERT_CHECK_CALL(napi_get_threadsafe_function_context(tsfn, &context));
    ASSERT_EQ(*reinterpret_cast<int32_t*>(context), INT_ONE);
    ASSERT_CHECK_CALL(napi_release_threadsafe_function(tsfn, napi_tsfn_release));
    runner.Run();
}

/**
 * @tc.name: AcquireThreadsafeFunctionWithMultiContext001
 * @tc.desc: Test napi_acquire_threadsafe_function when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, AcquireThreadsafeFunctionWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    UVLoopRunner runner(multiContextEngine_);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value resourceName;
    ASSERT_CHECK_CALL(napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_threadsafe_function tsfn;
    ASSERT_CHECK_CALL(napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 0, 1, nullptr,
        [](napi_env env, void* context, void*) {}, nullptr,
        [](napi_env env, napi_value jsCb, void *context, void *data) {}, &tsfn));
    ASSERT_CHECK_CALL(napi_acquire_threadsafe_function(tsfn));
    ASSERT_EQ(reinterpret_cast<NativeSafeAsyncWork*>(tsfn)->threadCount_, INT_TWO);
    ASSERT_CHECK_CALL(napi_release_threadsafe_function(tsfn, napi_tsfn_abort));
    runner.Run();
}

/**
 * @tc.name: ReleaseThreadsafeFunctionWithMultiContext001
 * @tc.desc: Test napi_release_threadsafe_function when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, ReleaseThreadsafeFunctionWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    UVLoopRunner runner(multiContextEngine_);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value resourceName;
    ASSERT_CHECK_CALL(napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_threadsafe_function tsfn;
    ASSERT_CHECK_CALL(napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 0, 1, nullptr,
        [](napi_env env, void* context, void*) {}, nullptr,
        [](napi_env env, napi_value jsCb, void *context, void *data) {}, &tsfn));
    ASSERT_CHECK_CALL(napi_release_threadsafe_function(tsfn, napi_tsfn_abort));
    runner.Run();
}

/**
 * @tc.name: UnrefThreadsafeFunctionWithMultiContext001
 * @tc.desc: Test napi_unref_threadsafe_function when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, UnrefThreadsafeFunctionWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    UVLoopRunner runner(multiContextEngine_);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value resourceName;
    ASSERT_CHECK_CALL(napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_threadsafe_function tsfn;
    ASSERT_CHECK_CALL(napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 0, 1, nullptr,
        [](napi_env env, void* context, void*) {}, nullptr,
        [](napi_env env, napi_value jsCb, void *context, void *data) {}, &tsfn));
    ASSERT_CHECK_CALL(napi_unref_threadsafe_function(env, tsfn));
    ASSERT_CHECK_CALL(napi_ref_threadsafe_function(env, tsfn));
    ASSERT_CHECK_CALL(napi_release_threadsafe_function(tsfn, napi_tsfn_abort));
    runner.Run();
}

/**
 * @tc.name: RefThreadsafeFunctionWithMultiContext001
 * @tc.desc: Test napi_ref_threadsafe_function when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, RefThreadsafeFunctionWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    UVLoopRunner runner(multiContextEngine_);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value resourceName;
    ASSERT_CHECK_CALL(napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_threadsafe_function tsfn;
    ASSERT_CHECK_CALL(napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 0, 1, nullptr,
        [](napi_env env, void* context, void*) {}, nullptr,
        [](napi_env env, napi_value jsCb, void *context, void *data) {}, &tsfn));
    ASSERT_CHECK_CALL(napi_ref_threadsafe_function(env, tsfn));
    ASSERT_CHECK_CALL(napi_release_threadsafe_function(tsfn, napi_tsfn_abort));
    runner.Run();
}

/**
 * @tc.name: AddAsyncCleanupHookWithMultiContext001
 * @tc.desc: Test napi_add_async_cleanup_hook when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, AddAsyncCleanupHookWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_async_cleanup_hook_handle handle;
    napi_status res = napi_add_async_cleanup_hook(env, [](napi_async_cleanup_hook_handle handle, void* arg) {},
        nullptr, &handle);
    EXPECT_EQ(res, napi_ok);
    ASSERT_CHECK_CALL(napi_remove_async_cleanup_hook(handle));
}

/**
 * @tc.name: RemoveAsyncCleanupHookWithMultiContext001
 * @tc.desc: Test napi_add_async_cleanup_hook when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, RemoveAsyncCleanupHookWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_async_cleanup_hook_handle handle = nullptr;
    ASSERT_CHECK_CALL(napi_add_async_cleanup_hook(env, [](napi_async_cleanup_hook_handle handle, void* arg) {},
        nullptr, &handle));
    EXPECT_NE(handle, nullptr);
    ASSERT_CHECK_CALL(napi_remove_async_cleanup_hook(handle));
}

/**
 * @tc.name: ApiGetModuleFileNameWithMultiContext001
 * @tc.desc: Test node_api_get_module_file_name when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, ApiGetModuleFileNameWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    const char* result = nullptr;
    ASSERT_CHECK_CALL(node_api_get_module_file_name(env, &result));
    ASSERT_NE(result, nullptr);
}

/**
 * @tc.name: GetLastErrorInfoWithMultiContext001
 * @tc.desc: Test napi_get_last_error_info when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, GetLastErrorInfoWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    const napi_extended_error_info* result;
    ASSERT_CHECK_CALL(napi_get_last_error_info(env, &result));
}

/**
 * @tc.name: GetUndefinedWithMultiContext001
 * @tc.desc: Test napi_get_undefined when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, GetUndefinedWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_get_undefined(env, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_undefined);
}

/**
 * @tc.name: GetNullWithMultiContext001
 * @tc.desc: Test napi_get_null when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, GetNullWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_get_null(env, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_null);
}

/**
 * @tc.name: GetBooleanWithMultiContext001
 * @tc.desc: Test napi_get_boolean when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, GetBooleanWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_boolean);
}

/**
 * @tc.name: CreateObjectWithMultiContext001
 * @tc.desc: Test napi_create_object when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateObjectWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_object);
}

/**
 * @tc.name: CreateArraybufferWithMultiContext001
 * @tc.desc: Test napi_create_arraybuffer when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateArraybufferWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    size_t arrayBufferSize = ARRAYBUFFER_SIZE;
    ASSERT_CHECK_CALL(napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer));

    void* tmpArrayBufferPtr = nullptr;
    size_t arrayBufferLength = 0;
    napi_get_arraybuffer_info(env, arrayBuffer, &tmpArrayBufferPtr, &arrayBufferLength);

    ASSERT_EQ(arrayBufferPtr, tmpArrayBufferPtr);
    ASSERT_EQ(arrayBufferSize, arrayBufferLength);
}

/**
 * @tc.name: CreateArraybufferWithLengthWithMultiContext001
 * @tc.desc: Test napi_create_array_with_length when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateArraybufferWithLengthWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value array = nullptr;
    ASSERT_CHECK_CALL(napi_create_array_with_length(env, INT_THREE, &array));
    uint32_t result = 0;
    napi_get_array_length(env, array, &result);
    ASSERT_EQ(result, INT_THREE);
}

/**
 * @tc.name: CreateDoubleWithMultiContext001
 * @tc.desc: Test napi_create_double when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateDoubleWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_double(env, TEST_DOUBLE, &value));
}

/**
 * @tc.name: CreateInt32WithMultiContext001
 * @tc.desc: Test napi_create_int32 when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateInt32WithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, INT_ONE, &value));
}

/**
 * @tc.name: CreateUint32WithMultiContext001
 * @tc.desc: Test napi_create_uint32 when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateUint32WithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_uint32(env, INT_ONE, &value));
}

/**
 * @tc.name: CreateInt64WithMultiContext001
 * @tc.desc: Test napi_create_int64 when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateInt64WithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_int64(env, INT_ONE, &value));
}

/**
 * @tc.name: CreateStringLatin1WithMultiContext001
 * @tc.desc: Test napi_create_string_latin1 when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateStringLatin1WithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_latin1(env, TEST_STR_UTF8, NAPI_AUTO_LENGTH, &result));
}

/**
 * @tc.name: CreateStringUtf8WithMultiContext001
 * @tc.desc: Test napi_create_string_utf8 when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateStringUtf8WithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_STR_UTF8, NAPI_AUTO_LENGTH, &result));
}

/**
 * @tc.name: CreateStringUtf16WithMultiContext001
 * @tc.desc: Test napi_create_string_utf16 when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateStringUtf16WithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf16(env, TEST_STR_UTF16, NAPI_AUTO_LENGTH, &result));
}

/**
 * @tc.name: CreateSymbolWithMultiContext001
 * @tc.desc: Test napi_create_symbol when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CreateSymbolWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value result = nullptr;
    napi_create_string_latin1(env, TEST_STR, NAPI_AUTO_LENGTH, &result);

    napi_value symbolVal = nullptr;
    ASSERT_CHECK_CALL(napi_create_symbol(env, result, &symbolVal));
    ASSERT_CHECK_VALUE_TYPE(env, symbolVal, napi_symbol);
}

/**
 * @tc.name: TypeofWithMultiContext001
 * @tc.desc: Test napi_typeof when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, TypeofWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value value = nullptr;
    napi_valuetype result;
    ASSERT_CHECK_CALL(napi_create_double(env, TEST_DOUBLE, &value));
    ASSERT_CHECK_CALL(napi_typeof(env, value, &result));
    ASSERT_EQ(result, napi_number);
}

/**
 * @tc.name: GetValueDoubleWithMultiContext001
 * @tc.desc: Test napi_get_value_double when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, GetValueDoubleWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_double(env, TEST_DOUBLE, &value));
    double number;
    ASSERT_CHECK_CALL(napi_get_value_double(env, value, &number));
    ASSERT_EQ(number, TEST_DOUBLE);
}

/**
 * @tc.name: GetValueInt32WithMultiContext001
 * @tc.desc: Test napi_get_value_int32 when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, GetValueInt32WithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_int32(env, INT_ONE, &value));
    int32_t number;
    ASSERT_CHECK_CALL(napi_get_value_int32(env, value, &number));
    ASSERT_EQ(number, INT_ONE);
}

/**
 * @tc.name: GetValueUint32WithMultiContext001
 * @tc.desc: Test napi_get_value_uint32 when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, GetValueUint32WithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_uint32(env, INT_ONE, &value));
    uint32_t number;
    ASSERT_CHECK_CALL(napi_get_value_uint32(env, value, &number));
    ASSERT_EQ(number, INT_ONE);
}

/**
 * @tc.name: GetValueInt64WithMultiContext001
 * @tc.desc: Test napi_get_value_int64 when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, GetValueInt64WithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_create_int64(env, INT_ONE, &value));
    int64_t number;
    ASSERT_CHECK_CALL(napi_get_value_int64(env, value, &number));
    ASSERT_EQ(number, INT_ONE);
}

/**
 * @tc.name: GetValueBoolWithMultiContext001
 * @tc.desc: Test napi_get_value_bool when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, GetValueBoolWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value value = nullptr;
    ASSERT_CHECK_CALL(napi_get_boolean(env, true, &value));
    bool boolValue;
    ASSERT_CHECK_CALL(napi_get_value_bool(env, value, &boolValue));
    ASSERT_EQ(boolValue, true);
}

/**
 * @tc.name: GetValueStringLatin1WithMultiContext001
 * @tc.desc: Test napi_get_value_string_latin1 when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, GetValueStringLatin1WithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    size_t testStrLength = strlen(TEST_STR_UTF8);
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_latin1(env, TEST_STR_UTF8, testStrLength, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_string);

    char* buffer = nullptr;
    size_t bufferSize = 0;
    size_t strLength = 0;
    ASSERT_CHECK_CALL(napi_get_value_string_latin1(env, result, nullptr, 0, &bufferSize));
    ASSERT_GT(bufferSize, static_cast<size_t>(0));
    buffer = new char[bufferSize + 1]{ 0 };
    ASSERT_CHECK_CALL(napi_get_value_string_latin1(env, result, buffer, bufferSize + 1, &strLength));
    ASSERT_STREQ(TEST_STR_UTF8, buffer);
    ASSERT_EQ(testStrLength, strLength);
    delete []buffer;
    buffer = nullptr;
}

/**
 * @tc.name: GetValueStringUtf8WithMultiContext001
 * @tc.desc: Test napi_get_value_string_utf8 when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, GetValueStringUtf8WithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    size_t testStrLength = strlen(TEST_STR_UTF8);
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf8(env, TEST_STR_UTF8, testStrLength, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_string);

    char* buffer = nullptr;
    size_t bufferSize = 0;
    size_t strLength = 0;
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, result, nullptr, 0, &bufferSize));
    ASSERT_GT(bufferSize, static_cast<size_t>(0));
    buffer = new char[bufferSize + 1]{ 0 };
    ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, result, buffer, bufferSize + 1, &strLength));
    ASSERT_STREQ(TEST_STR_UTF8, buffer);
    ASSERT_EQ(testStrLength, strLength);
    delete []buffer;
    buffer = nullptr;
}

/**
 * @tc.name: GetValuStringUtf16WithMultiContext001
 * @tc.desc: Test napi_get_value_string_utf16 when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, GetValuStringUtf16WithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    size_t testStrLength = static_cast<size_t>(std::char_traits<char16_t>::length(TEST_STR_UTF16));
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf16(env, TEST_STR_UTF16, testStrLength, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_string);

    char16_t* buffer = nullptr;
    size_t bufferSize = 0;
    size_t strLength = 0;
    ASSERT_CHECK_CALL(napi_get_value_string_utf16(env, result, nullptr, 0, &bufferSize));
    ASSERT_GT(bufferSize, static_cast<size_t>(0));
    buffer = new char16_t[bufferSize + 1]{ 0 };
    ASSERT_CHECK_CALL(napi_get_value_string_utf16(env, result, buffer, bufferSize + 1, &strLength));
    for (int i = 0; i < testStrLength; i++) {
        ASSERT_EQ(TEST_STR_UTF16[i], buffer[i]);
    }
    ASSERT_EQ(testStrLength, strLength);
    delete []buffer;
    buffer = nullptr;
}

/**
 * @tc.name: CoerceToBoolWithMultiContext001
 * @tc.desc: Test napi_coerce_to_bool when context is sub context.
 *           interface.
 * @tc.type: FUNC
 */
HWTEST_F(NapiContextTest, CoerceToBoolWithMultiContext001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(multiContextEngine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(multiContextEngine_);

    napi_value value;
    napi_value result;
    ASSERT_CHECK_CALL(napi_create_double(env, TEST_DOUBLE, &value));
    ASSERT_CHECK_CALL(napi_coerce_to_bool(env, value, &result));
    bool ret = false;
    napi_get_value_bool(env, result, &ret);
    ASSERT_EQ(ret, true);
}