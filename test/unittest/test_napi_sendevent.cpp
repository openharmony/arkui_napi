/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http:// www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "test.h"
#include "js_native_api_types.h"
#include "js_native_api.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "native_engine.h"

static char g_defaultName[] = "defaultName";
static char g_defaultData[] = "testData";
static char g_testName[] = "testName";
static void Task(void* data)
{
}

class NapiSendEventTest : public NativeEngineTest {
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: SendEventTest001
 * @tc.desc: Test napi_send_event invalidParams
 * @tc.type:FUNC
 */
HWTEST_F(NapiSendEventTest, SendEventTest001, testing::ext::TestSize.Level1)
{
    const std::function<void()> cb = []() -> void {};
    auto status = napi_send_event(nullptr, cb, napi_eprio_high);
    EXPECT_EQ(status, napi_status::napi_invalid_arg);
}

/**
 * @tc.name: SendEventTest002
 * @tc.desc: Test napi_send_event invalidParams
 * @tc.type:FUNC
 */
HWTEST_F(NapiSendEventTest, SendEventTest002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    auto status = napi_send_event(env, nullptr, napi_eprio_high);
    EXPECT_EQ(status, napi_status::napi_invalid_arg);
}

/**
 * @tc.name: SendEventTest003
 * @tc.desc: Test napi_send_event invalidParams
 * @tc.type:FUNC
 */
HWTEST_F(NapiSendEventTest, SendEventTest003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    const std::function<void()> cb = []() -> void {};
    auto status = napi_send_event(env, cb, napi_event_priority(napi_eprio_vip - 1));
    EXPECT_EQ(status, napi_status::napi_invalid_arg);
    status = napi_send_event(env, cb, napi_event_priority(napi_eprio_idle + 1));
    EXPECT_EQ(status, napi_status::napi_invalid_arg);
}

/**
 * @tc.name: SendEventTest004
 * @tc.desc: Test napi_send_event sendEventByHandler
 * @tc.type:FUNC
 */
HWTEST_F(NapiSendEventTest, SendEventTest004, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    const std::function<void()> cb = []() -> void {};
    if (eventHandler_ == nullptr) {
        auto runner = OHOS::AppExecFwk::EventRunner::Create(false);
        EXPECT_NE(runner, nullptr);
        eventHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    }
    EXPECT_NE(eventHandler_, nullptr);
    auto status = napi_send_event(env, cb, napi_eprio_high);
    EXPECT_EQ(status, napi_status::napi_ok);

    engine_->SetInstanceId(1000);
    status = napi_send_event(env, cb, napi_eprio_high);
    EXPECT_EQ(status, napi_status::napi_ok);
}

/**
 * @tc.name: SendEventTest005
 * @tc.desc: Test napi_send_event sendEventByUv
 * @tc.type:FUNC
 */
HWTEST_F(NapiSendEventTest, SendEventTest005, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    const std::function<void()> cb = []() -> void {};
    eventHandler_ = nullptr;
    auto status = napi_send_event(env, cb, napi_eprio_high);
    EXPECT_EQ(status, napi_status::napi_ok);
}

/**
 * @tc.name: SendEventTest006
 * @tc.desc: Test napi_send_event defaultParams
 * @tc.type:FUNC
 */
HWTEST_F(NapiSendEventTest, SendEventTest006, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    const std::function<void()> cb = []() -> void {};
    auto status = napi_send_event(env, cb, napi_eprio_high);
    EXPECT_EQ(status, napi_status::napi_ok);
}

/**
 * @tc.name: SendEventTest007
 * @tc.desc: Test napi_send_event allParams
 * @tc.type:FUNC
 */
HWTEST_F(NapiSendEventTest, SendEventTest007, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    const std::function<void()> cb = []() -> void {};
    auto status = napi_send_event(env, cb, napi_eprio_high, g_testName);
    EXPECT_EQ(status, napi_status::napi_ok);
}

/**
 * @tc.name: SendCancelableEent001
 * @tc.desc: Test napi_send_cancelable_event invalidParams
 * @tc.type:FUNC
 */
HWTEST_F(NapiSendEventTest, SendCancelableEent001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    uint64_t handleId = 0;
    auto status = napi_send_cancelable_event(nullptr, Task, g_defaultData, napi_eprio_high, &handleId, g_defaultName);
    EXPECT_EQ(status, napi_status::napi_invalid_arg);
    status = napi_send_cancelable_event(env, nullptr, g_defaultData, napi_eprio_high, &handleId, g_defaultName);
    EXPECT_EQ(status, napi_status::napi_invalid_arg);
    status = napi_send_cancelable_event(env, Task, g_defaultData, napi_event_priority(napi_eprio_idle + 1),
                                            &handleId, g_defaultName);
    EXPECT_EQ(status, napi_status::napi_invalid_arg);
    status = napi_send_cancelable_event(env, Task, g_defaultData, napi_eprio_high, nullptr, g_defaultName);
    EXPECT_EQ(status, napi_status::napi_invalid_arg);
}

/**
 * @tc.name: SendCancelableEent002
 * @tc.desc: Test napi_send_cancelable_event sendEventByHandler
 * @tc.type:FUNC
 */
HWTEST_F(NapiSendEventTest, SendCancelableEent002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    uint64_t handleId = 0;
    if (eventHandler_ == nullptr) {
        auto runner = OHOS::AppExecFwk::EventRunner::Create(false);
        ASSERT_NE(runner, nullptr);
        eventHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
        ASSERT_NE(eventHandler_, nullptr);
    }
    auto status = napi_send_cancelable_event(env, Task, g_defaultData, napi_eprio_high, &handleId, g_defaultName);
    ASSERT_EQ(status, napi_status::napi_ok);
}

/**
 * @tc.name: SendCancelableEent003
 * @tc.desc: Test napi_send_cancelable_event sendEventByUv
 * @tc.type:FUNC
 */
HWTEST_F(NapiSendEventTest, SendCancelableEent003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    
    uint64_t handleId = 0;
    eventHandler_ = nullptr;
    auto status = napi_send_cancelable_event(env, Task, g_defaultData, napi_eprio_high, &handleId, g_defaultName);
    ASSERT_EQ(status, napi_status::napi_ok);
}

/**
 * @tc.name: CancelEvent001
 * @tc.desc: Test napi_cancel_event invalidParams
 * @tc.type:FUNC
 */
HWTEST_F(NapiSendEventTest, CancelEvent001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = reinterpret_cast<napi_env>(engine_);
    uint64_t handleId = 0;
    auto result = napi_cancel_event(env, handleId, g_defaultName);
    ASSERT_EQ(result, napi_status::napi_invalid_arg);
    auto status = napi_send_cancelable_event(env, Task, g_defaultData, napi_eprio_high, &handleId, g_defaultName);
    ASSERT_NE(handleId, 0);
    ASSERT_EQ(status, napi_status::napi_ok);
    result = napi_cancel_event(nullptr, handleId, g_defaultName);
    ASSERT_EQ(result, napi_status::napi_invalid_arg);
}

/**
 * @tc.name: CancelEvent002
 * @tc.desc: Test napi_cancel_event eventHandler_ -> RemoveTask
 * @tc.type:FUNC
 */
HWTEST_F(NapiSendEventTest, CancelEvent002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    uint64_t handleId = 0;
    if (eventHandler_ == nullptr) {
        auto runner = OHOS::AppExecFwk::EventRunner::Create(false);
        EXPECT_NE(runner, nullptr);
        eventHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
        ASSERT_NE(eventHandler_, nullptr);
    }
    auto status = napi_send_cancelable_event(env, Task, g_defaultData, napi_eprio_high, &handleId, g_defaultName);
    ASSERT_NE(handleId, 0);
    ASSERT_EQ(status, napi_status::napi_ok);
    auto result = napi_cancel_event(env, handleId, g_defaultName);
    ASSERT_EQ(result, napi_status::napi_ok);
}

/**
 * @tc.name: CancelEvent003
 * @tc.desc: Test napi_cancel_event sendEventByUv
 * @tc.type:FUNC
 */
HWTEST_F(NapiSendEventTest, CancelEvent003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(engine_, nullptr);
    napi_env env = (napi_env)engine_;
    uint64_t handleId = 0;
    eventHandler_ = nullptr;
    auto status = napi_send_cancelable_event(env, Task, g_defaultData, napi_eprio_high, &handleId, g_defaultName);
    ASSERT_EQ(status, napi_status::napi_ok);
    ASSERT_NE(handleId, 0);
    auto result = napi_cancel_event(env, handleId, g_defaultName);
    ASSERT_EQ(result, napi_status::napi_ok);
}