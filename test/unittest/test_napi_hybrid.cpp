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

#include "ecmascript/napi/include/jsnapi_expo.h"
#include "gtest/gtest.h"
#include "napi/native_node_hybrid_api.h"
#include "test.h"
#include "test_common.h"

class NapiHybridTest : public NativeEngineTest {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "NapiHybridTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "NapiHybridTest TearDownTestCase";
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

static void NoopFinalizer([[maybe_unused]] napi_env env, [[maybe_unused]] void* data, [[maybe_unused]] void* hint) {}

static const napi_type_tag hybridTypeTag = {
    0x1234567890abcdef, // lower
    0xfedcba0987654321  // upper
};

/**
 * @tc.name: NapiLoadModuleWithInfoForHybridAppTest
 * @tc.desc: Test interface of napi_load_module_with_info_hybrid
 * @tc.type: FUNC
 */
HWTEST_F(NapiHybridTest, NapiLoadModuleWithInfoForHybridAppTest001, testing::ext::TestSize.Level1)
{
    auto res = napi_load_module_with_info_hybrid(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiLoadModuleWithInfoForHybridAppTest
 * @tc.desc: Test interface of napi_load_module_with_info_hybrid
 * @tc.type: FUNC
 */
HWTEST_F(NapiHybridTest, NapiLoadModuleWithInfoForHybridAppTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    auto res = napi_load_module_with_info_hybrid(env, nullptr, nullptr, nullptr, nullptr, nullptr);
    ASSERT_EQ(res, napi_invalid_arg);
}

/**
 * @tc.name: NapiLoadModuleWithInfoForHybridAppTest
 * @tc.desc: Test interface of napi_load_module_with_info_hybrid
 * @tc.type: FUNC
 */
HWTEST_F(NapiHybridTest, NapiLoadModuleWithInfoForHybridAppTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);

    napi_value result = nullptr;
    auto res = napi_load_module_with_info_hybrid(env, nullptr, nullptr, nullptr, nullptr, &result);
    ASSERT_EQ(res, napi_ok);
}

/**
 * @tc.name: NapiGetStringUtf8HybridTest001
 * @tc.desc: Test napi_get_value_string_utf8_hybrid with basic ASCII string
 * @tc.type: FUNC
 */
HWTEST_F(NapiHybridTest, NapiGetStringUtf8HybridTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    const char testStr[] = "Hello, World!";
    size_t testStrLength = strlen(testStr);
    napi_value result = nullptr;

    ASSERT_CHECK_CALL(napi_create_string_utf8(env, testStr, testStrLength, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_string);

    std::variant<std::string, std::u16string> str;

    ASSERT_CHECK_CALL(napi_get_value_string_utf8_hybrid(env, result, &str));

    ASSERT_TRUE(std::holds_alternative<std::string>(str));
    auto& value = std::get<std::string>(str);

    ASSERT_EQ(value, testStr);
    ASSERT_EQ(value.length(), testStrLength);
}

/**
 * @tc.name: NapiGetStringUtf8HybridTest002
 * @tc.desc: Test napi_get_value_string_utf8_hybrid with Chinese characters
 * @tc.type: FUNC
 */
HWTEST_F(NapiHybridTest, NapiGetStringUtf8HybridTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    const char16_t testStr[] = u"中文字符测试";
    size_t testStrLength = std::char_traits<char16_t>::length(testStr);
    napi_value result = nullptr;

    ASSERT_CHECK_CALL(napi_create_string_utf16(env, testStr, testStrLength, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_string);

    std::variant<std::string, std::u16string> str;

    ASSERT_CHECK_CALL(napi_get_value_string_utf8_hybrid(env, result, &str));

    ASSERT_TRUE(std::holds_alternative<std::u16string>(str));
    auto& value = std::get<std::u16string>(str);

    ASSERT_EQ(value, testStr);
    ASSERT_EQ(value.length(), testStrLength);
}

/**
 * @tc.name: NapiGetStringUtf8HybridTest003
 * @tc.desc: Test napi_get_value_string_utf8_hybrid with emoji characters
 * @tc.type: FUNC
 */
HWTEST_F(NapiHybridTest, NapiGetStringUtf8HybridTest003, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    const char16_t testStr[] = u"😊😂🤣❤️😍😒👌😘";
    size_t testStrLength = std::char_traits<char16_t>::length(testStr);
    napi_value result = nullptr;

    ASSERT_CHECK_CALL(napi_create_string_utf16(env, testStr, testStrLength, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_string);

    std::variant<std::string, std::u16string> str;
    ASSERT_CHECK_CALL(napi_get_value_string_utf8_hybrid(env, result, &str));
    ASSERT_TRUE(std::holds_alternative<std::u16string>(str));
    auto& value = std::get<std::u16string>(str);

    ASSERT_EQ(value, testStr);
    ASSERT_EQ(value.length(), testStrLength);
}

/**
 * @tc.name: NapiHybridWrapUnwrapTest001
 * @tc.desc: Test basic wrap and unwrap for hybrid API.
 * @tc.type: FUNC
 */
HWTEST_F(NapiHybridTest, NapiHybridWrapUnwrapTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value jsObject = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &jsObject));

    int nativeValue = 42;
    void* resultValue = nullptr;

    // 1. Wrap
    ASSERT_CHECK_CALL(napi_wrap_hybrid_s(env, jsObject, &nativeValue, NoopFinalizer, nullptr, &hybridTypeTag, nullptr));

    // 2. Unwrap with same tag
    ASSERT_CHECK_CALL(napi_unwrap_hybrid_s(env, jsObject, &hybridTypeTag, &resultValue));
    ASSERT_EQ(resultValue, &nativeValue);
    ASSERT_EQ(*(static_cast<int*>(resultValue)), 42);
}

/**
 * @tc.name: NapiHybridWrapUnwrapTest002
 * @tc.desc: Test unwrap with mismatched type tag for hybrid API.
 * @tc.type: FUNC
 */
HWTEST_F(NapiHybridTest, NapiHybridWrapUnwrapTest002, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value jsObject = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &jsObject));

    int nativeValue = 100;
    napi_type_tag wrongTag = { 0x1, 0x2 };

    ASSERT_CHECK_CALL(napi_wrap_hybrid_s(env, jsObject, &nativeValue, NoopFinalizer, nullptr, &hybridTypeTag, nullptr));

    // Unwrap with mismatched tag should return napi_invalid_arg
    void* resultValue = nullptr;
    napi_status status = napi_unwrap_hybrid_s(env, jsObject, &wrongTag, &resultValue);
    ASSERT_EQ(status, napi_invalid_arg);
    ASSERT_EQ(resultValue, nullptr);
}

/**
 * @tc.name: NapiHybridWrapUnwrapTest003
 * @tc.desc: Test repeated wrap on the same object.
 * @tc.type: FUNC
 */
HWTEST_F(NapiHybridTest, NapiHybridWrapUnwrapTest003, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value jsObject = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &jsObject));

    int data1 = 1;
    int data2 = 2;

    ASSERT_CHECK_CALL(napi_wrap_hybrid_s(env, jsObject, &data1, NoopFinalizer, nullptr, &hybridTypeTag, nullptr));

    // Repeated wrap should fail
    napi_status status = napi_wrap_hybrid_s(env, jsObject, &data2, NoopFinalizer, nullptr, &hybridTypeTag, nullptr);
    ASSERT_EQ(status, napi_invalid_arg);
}

/**
 * @tc.name: NapiRefGetVmTest001
 * @tc.desc: Test napi_ref_get_vm with valid reference.
 * @tc.type: FUNC
 */
HWTEST_F(NapiHybridTest, NapiRefGetVmTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value obj = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));

    napi_ref ref = nullptr;
    ASSERT_CHECK_CALL(napi_create_reference(env, obj, 1, &ref));

    uintptr_t vm = 0;
    napi_status status = napi_ref_get_vm(ref, vm);
    ASSERT_EQ(status, napi_ok);
    ASSERT_NE(vm, 0UL);

    ASSERT_CHECK_CALL(napi_delete_reference(env, ref));
}

/**
 * @tc.name: NapiRefGetVmAndValueTest002
 * @tc.desc: Test napi_ref_get_vm and napi_ref_get_value with nullptr ref.
 * @tc.type: FUNC
 */
HWTEST_F(NapiHybridTest, NapiRefGetVmAndValueTest002, testing::ext::TestSize.Level1)
{
    uintptr_t vm = 0;
    napi_status status = napi_ref_get_vm(nullptr, vm);
    ASSERT_EQ(status, napi_invalid_arg);

    uintptr_t value = 0;
    status = napi_ref_get_value(nullptr, value);
    ASSERT_EQ(status, napi_invalid_arg);
}

/**
 * @tc.name: NapiRefGetValueTest001
 * @tc.desc: Test napi_ref_get_value with valid reference.
 * @tc.type: FUNC
 */
HWTEST_F(NapiHybridTest, NapiRefGetValueTest001, testing::ext::TestSize.Level1)
{
    napi_env env = (napi_env)engine_;
    napi_value obj = nullptr;
    ASSERT_CHECK_CALL(napi_create_object(env, &obj));

    napi_ref ref = nullptr;
    ASSERT_CHECK_CALL(napi_create_reference(env, obj, 1, &ref));

    uintptr_t value = 0;
    napi_status status = napi_ref_get_value(ref, value);
    ASSERT_EQ(status, napi_ok);
    ASSERT_NE(value, 0UL);

    ASSERT_CHECK_CALL(napi_delete_reference(env, ref));
}