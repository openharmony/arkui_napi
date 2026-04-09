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

#include "gtest/gtest.h"
#include "ecmascript/napi/include/jsnapi_expo.h"
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
    auto &value = std::get<std::string>(str);

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
    auto &value = std::get<std::u16string>(str);

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
    auto &value = std::get<std::u16string>(str);

    ASSERT_EQ(value, testStr);
    ASSERT_EQ(value.length(), testStrLength);
}