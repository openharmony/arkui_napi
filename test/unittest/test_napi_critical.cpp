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

#include "gtest/gtest.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "native_utils.h"
#include "uv.h"
#define private public
#define protected public
#include "test.h"
#undef private
#include "test_common.h"
#include "utils/log.h"

// use macro instead of constexpr variable, due to need concat with other string
#define TEST_NAPI_UNCLOSED_CRITICAL_LOG "napi cannot invoke under critical scope, id: "
#define TEST_UNCLOSED_CRITICAL_CALLBACK_LOG "critical scope still open after user callback"

class NapiCriticalTest : public NativeEngineTest {
public:
    NapiCriticalTest() : NativeEngineTest(), runner_(engine_) {}
    ~NapiCriticalTest()
    {
        if (uv_loop_alive(runner_)) {
        }
        runner_.Run(UV_RUN_NOWAIT);
    }
    void SetUp() override
    {
        napi_open_critical_scope(reinterpret_cast<napi_env>(engine_), &scope_);
    }
    void TearDown() override
    {
        napi_close_critical_scope(reinterpret_cast<napi_env>(engine_), scope_);
    }

private:
public:
    void TriggerFailed()
    {
        GTEST_LOG_(ERROR) << "NapiCriticalTest failed: " << GetTestCaseName();
        ASSERT_TRUE(false);
    }
    UVLoopRunner runner_;
    napi_critical_scope scope_ = nullptr;
};

napi_value EmptyNapiCallback(napi_env env, napi_callback_info info)
{
    return nullptr;
}

template<typename T>
void FinalizeCallback([[maybe_unused]] napi_env env, void* data, [[maybe_unused]] void* hint)
{
    if (data == nullptr) {
        return;
    }
    delete reinterpret_cast<T*>(data);
}

void EmptyFinalizeCallback([[maybe_unused]] napi_env env, void* data, [[maybe_unused]] void* hint) {}

/**
 * @tc.name: NapiUnclosedCriticalTest001
 * @tc.desc: Test check of unclosed critical scope after user callback.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiUnclosedCriticalTest001, testing::ext::TestSize.Level0)
{
    BasicDeathTest deathTest([]() {
        NativeEngineProxy env;
        static napi_critical_scope scope {};
        napi_value fn {};
        napi_create_function(
            env, "testFunc", NAPI_AUTO_LENGTH,
            [](napi_env env, [[maybe_unused]] napi_callback_info info) -> napi_value {
                napi_open_critical_scope(env, &scope);
                return nullptr;
            },
            nullptr, &fn);
        napi_call_function(env, nullptr, fn, 0, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError("[ArkNativeFunctionCallBack] " TEST_UNCLOSED_CRITICAL_CALLBACK_LOG
                                                " 'testFunc'");
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiUnclosedCriticalTest002
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiUnclosedCriticalTest002, testing::ext::TestSize.Level0)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        UVLoopRunner runner(*env);
        napi_value name {};
        napi_create_string_utf8(env, "taskName", NAPI_AUTO_LENGTH, &name);
        napi_async_work work {};
        napi_create_async_work(
            env, nullptr, name, []([[maybe_unused]] napi_env env, [[maybe_unused]] void* data) {},
            [](napi_env env, [[maybe_unused]] napi_status status, [[maybe_unused]] void* data) {
                static napi_critical_scope scope {};
                napi_open_critical_scope(env, &scope);
            },
            nullptr, &work);
        NativeAsyncWork::AsyncAfterWorkCallback(&reinterpret_cast<NativeAsyncWork*>(work)->work_, 0);
    });
    deathTest.AssertSignal(SIGABRT).AssertError("[AsyncAfterWorkCallback] " TEST_UNCLOSED_CRITICAL_CALLBACK_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiUnclosedCriticalTest003
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiUnclosedCriticalTest003, testing::ext::TestSize.Level0)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        UVLoopRunner runner(*env);
        static napi_critical_scope scope {};
        napi_value undefined {};
        napi_get_undefined(env, &undefined);
        napi_threadsafe_function tsfn {};
        napi_create_threadsafe_function(
            env, nullptr, nullptr, undefined, 0, 1, nullptr, nullptr, nullptr,
            [](napi_env env, [[maybe_unused]] napi_value func, [[maybe_unused]] void* context,
               [[maybe_unused]] void* data) { napi_open_critical_scope(env, &scope); },
            &tsfn);
        napi_call_threadsafe_function(tsfn, nullptr, napi_tsfn_blocking);
        runner.Run(UV_RUN_ONCE);
        napi_release_threadsafe_function(tsfn, napi_tsfn_release);
        runner.Run(UV_RUN_ONCE);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_UNCLOSED_CRITICAL_CALLBACK_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiUnclosedCriticalTest004
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiUnclosedCriticalTest004, testing::ext::TestSize.Level0)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        UVLoopRunner runner(*env);
        auto eventRunner = OHOS::AppExecFwk::EventRunner::Create(false);
        auto eventHandler = std::make_shared<OHOS::AppExecFwk::EventHandler>(eventRunner);

        static napi_critical_scope scope {};
        napi_threadsafe_function tsfn {};
        eventHandler->PostTask([&env, &tsfn] {
            napi_value undefined {};
            napi_get_undefined(env, &undefined);
            napi_create_threadsafe_function(
                env, nullptr, nullptr, undefined, 0, 1, nullptr, nullptr, nullptr,
                [](napi_env env, [[maybe_unused]] napi_value func, [[maybe_unused]] void* context,
                   [[maybe_unused]] void* data) {
                    OHOS::AppExecFwk::EventRunner::Current()->Stop();
                    napi_open_critical_scope(env, &scope);
                },
                &tsfn);
            napi_call_threadsafe_function_with_priority(tsfn, nullptr, napi_priority_immediate, false);
            napi_release_threadsafe_function(tsfn, napi_tsfn_release);
        });
        eventRunner->Run();
        runner.Run(UV_RUN_ONCE);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_UNCLOSED_CRITICAL_CALLBACK_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest001
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest001, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_env result {};
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_create_limit_runtime(env, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest002
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest002, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_fatal_exception(env, object);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest003
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest003, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value name {};
        napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &name);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_async_work work {};
        napi_create_async_work(
            env, nullptr, name, []([[maybe_unused]] napi_env env, [[maybe_unused]] void* data) {},
            []([[maybe_unused]] napi_env env, [[maybe_unused]] napi_status status, [[maybe_unused]] void* data) {},
            nullptr, &work);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest004
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest004, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value name {};
        napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &name);
        napi_threadsafe_function tsfn {};
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        // use non-nullptr napi_value for func, skip check of call_js_cb
        napi_create_threadsafe_function(env, name, nullptr, name, 0, 1, nullptr, nullptr, nullptr, nullptr, &tsfn);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest005
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest005, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value name {};
        napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &name);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_async_context result {};
        napi_async_init(env, nullptr, name, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest006
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest006, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        // type check is after env check
        napi_value undefined {};
        napi_get_undefined(env, &undefined);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_make_callback(env, nullptr, undefined, undefined, 0, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest007
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest007, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_get_undefined(env, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest008
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest008, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_get_null(env, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest009
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest009, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_get_global(env, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest010
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest010, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_get_boolean(env, true, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest011
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest011, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_create_object(env, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest012
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest012, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_create_object_with_properties(env, &result, 0, nullptr);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest013
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest013, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_create_object_with_named_properties(env, &result, 0, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest014
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest014, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_create_array(env, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest015
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest015, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_create_array_with_length(env, 0, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest016
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest016, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_create_sendable_array(env, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest017
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest017, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_create_sendable_array_with_length(env, 0, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest018
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest018, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_create_double(env, 0, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest019
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest019, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_create_int32(env, 0, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest020
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest020, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_create_uint32(env, 0, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest021
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest021, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_create_int64(env, 0, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest022
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest022, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_create_string_latin1(env, "", 0, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest023
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest023, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_create_string_utf8(env, "", 0, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest024
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest024, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_create_string_utf16(env, u"", 0, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest025
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest025, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_create_symbol(env, nullptr, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest026
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest026, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_create_function(env, "", 0, EmptyNapiCallback, nullptr, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest027
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest027, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value name {};
        napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &name);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_create_error(env, name, name, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest028
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest028, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value name {};
        napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &name);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_create_type_error(env, name, name, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest029
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest029, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value name {};
        napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &name);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_create_range_error(env, name, name, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest030
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest030, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value undefined {};
        napi_get_undefined(env, &undefined);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_valuetype type {};
        napi_typeof(env, undefined, &type);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest031
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest031, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value name {};
        napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &name);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_get_value_string_latin1(env, name, nullptr, 0, nullptr);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest032
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest032, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value name {};
        napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &name);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_get_value_string_utf8(env, name, nullptr, 0, nullptr);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest033
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest033, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value name {};
        napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &name);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_get_value_string_utf16(env, name, nullptr, 0, nullptr);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest034
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest034, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_critical_scope result {};
        napi_open_critical_scope(env, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest035
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest035, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value undefined {};
        napi_get_undefined(env, &undefined);
        napi_value result {};
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_coerce_to_bool(env, undefined, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest036
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest036, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value undefined {};
        napi_get_undefined(env, &undefined);
        napi_value result {};
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_coerce_to_number(env, undefined, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest037
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest037, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value undefined {};
        napi_get_undefined(env, &undefined);
        napi_value result {};
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_coerce_to_object(env, undefined, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest038
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest038, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value undefined {};
        napi_get_undefined(env, &undefined);
        napi_value result {};
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_coerce_to_string(env, undefined, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest039
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest039, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_get_prototype(env, object, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest040
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest040, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_get_property_names(env, object, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest041
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest041, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_set_property(env, object, object, object);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest042
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest042, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        bool result {};
        napi_has_property(env, object, object, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest043
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest043, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_get_property(env, object, object, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest044
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest044, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        bool result {};
        napi_delete_property(env, object, object, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest045
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest045, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        bool result {};
        napi_has_own_property(env, object, object, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest046
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest046, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_set_named_property(env, object, "", object);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest047
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest047, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        bool result {};
        napi_has_named_property(env, object, "", &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest048
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest048, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_get_named_property(env, object, "", &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest049
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest049, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_get_own_property_descriptor(env, object, "", &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest050
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest050, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value array {};
        napi_create_array(env, &array);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_set_element(env, array, 0, array);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest051
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest051, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value array {};
        napi_create_array(env, &array);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        bool result {};
        napi_has_element(env, array, 0, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest052
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest052, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value array {};
        napi_create_array(env, &array);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_get_element(env, array, 0, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest053
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest053, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value array {};
        napi_create_array(env, &array);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        bool result {};
        napi_delete_element(env, array, 0, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest054
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest054, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        std::vector<napi_property_descriptor> properties { { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                             napi_default, nullptr } };
        napi_define_properties(env, object, properties.size(), properties.data());
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest055
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest055, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value array {};
        napi_create_array(env, &array);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        bool result {};
        napi_is_array(env, array, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest056
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest056, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value undefined {};
        napi_get_undefined(env, &undefined);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        bool result {};
        napi_is_sendable(env, undefined, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest057
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest057, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value undefined {};
        napi_get_undefined(env, &undefined);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        bool result {};
        napi_strict_equals(env, undefined, undefined, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest058
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest058, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value undefined {};
        napi_get_undefined(env, &undefined);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_call_function(env, undefined, undefined, 0, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest059
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest059, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value undefined {};
        napi_get_undefined(env, &undefined);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_new_instance(env, undefined, 0, nullptr, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest060
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest060, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value undefined {};
        napi_get_undefined(env, &undefined);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        bool result {};
        napi_instanceof(env, undefined, undefined, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest061
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest061, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value fn {};
        napi_create_function(
            env, "", 0,
            [](napi_env env, napi_callback_info info) -> napi_value {
                napi_critical_scope scope {};
                napi_open_critical_scope(env, &scope);

                napi_value newTarget;
                napi_get_new_target(env, info, &newTarget);
                napi_close_critical_scope(env, scope);
                return nullptr;
            },
            nullptr, &fn);
        napi_call_function(env, nullptr, fn, 0, nullptr, nullptr);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest062
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest062, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_value result {};
        napi_define_class(env, "", 0, EmptyNapiCallback, nullptr, 0, nullptr, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest063
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest063, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_value result {};
        napi_define_sendable_class(env, "", 0, EmptyNapiCallback, nullptr, 0, nullptr, nullptr, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest064
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest064, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_value result {};
        napi_create_sendable_object_with_properties(env, 0, nullptr, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest065
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest065, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_value result {};
        napi_create_map(env, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest066
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest066, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_value result {};
        napi_create_sendable_map(env, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest067
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest067, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value map {};
        napi_create_map(env, &map);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_map_set_property(env, map, map, map);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest068
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest068, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value map {};
        napi_create_map(env, &map);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_map_set_named_property(env, map, "", map);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest069
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest069, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value map {};
        napi_create_map(env, &map);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_map_get_property(env, map, map, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest070
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest070, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value map {};
        napi_create_map(env, &map);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_map_get_named_property(env, map, "", &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest071
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest071, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value map {};
        napi_create_map(env, &map);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        bool result {};
        napi_map_has_property(env, map, map, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest072
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest072, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value map {};
        napi_create_map(env, &map);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        bool result {};
        napi_map_has_named_property(env, map, "", &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest073
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest073, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value map {};
        napi_create_map(env, &map);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_map_delete_property(env, map, map);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest074
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest074, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value map {};
        napi_create_map(env, &map);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_map_clear(env, map);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest075
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest075, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value map {};
        napi_create_map(env, &map);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        uint32_t size {};
        napi_map_get_size(env, map, &size);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest076
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest076, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value map {};
        napi_create_map(env, &map);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_map_get_entries(env, map, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest077
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest077, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value map {};
        napi_create_map(env, &map);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_map_get_keys(env, map, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest078
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest078, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value map {};
        napi_create_map(env, &map);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_map_get_values(env, map, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest079
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest079, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value map {};
        napi_create_map(env, &map);
        napi_value it {};
        napi_map_get_entries(env, map, &it);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_map_iterator_get_next(env, it, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest080
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest080, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        // test case would crash, no need release after test
        auto data = new int { 1 };
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_wrap(env, object, data, FinalizeCallback<int>, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest081
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest081, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        // test case would crash, no need release after test
        auto data = new int { 1 };
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_wrap_enhance(env, object, data, FinalizeCallback<int>, true, nullptr, 0, nullptr);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest082
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest082, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        // test case would crash, no need release after test
        auto data = new int { 1 };
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_wrap_async_finalizer(env, object, data, FinalizeCallback<int>, nullptr, nullptr, 0);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest083
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest083, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        // test case would crash, no need release after test
        auto data = new int { 1 };
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_wrap_with_size(env, object, data, FinalizeCallback<int>, nullptr, nullptr, 0);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest084
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest084, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        void* result {};
        napi_unwrap(env, object, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest085
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest085, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        void* result {};
        napi_remove_wrap(env, object, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest086
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest086, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_value object {};
        napi_create_external_with_size(env, nullptr, EmptyFinalizeCallback, nullptr, &object, 0);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest087
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest087, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        void* result {};
        napi_get_value_external(env, object, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest088
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest088, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_ref result {};
        napi_create_reference(env, object, 1, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest089
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest089, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        napi_ref ref {};
        napi_create_reference(env, object, 1, &ref);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_value result {};
        napi_get_reference_value(env, ref, &result);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest090
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest090, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_handle_scope scope2nd {};
        napi_open_handle_scope(env, &scope2nd);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest091
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest091, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_escapable_handle_scope scope2nd {};
        napi_open_escapable_handle_scope(env, &scope2nd);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}

/**
 * @tc.name: NapiNonCriticalTest092
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest092, testing::ext::TestSize.Level1)
{
    BasicDeathTest deathTest([] {
        NativeEngineProxy env;
        napi_value object {};
        napi_create_object(env, &object);
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);

        napi_throw(env, object);
        napi_close_critical_scope(env, scope);
    });
    deathTest.AssertSignal(SIGABRT).AssertError(TEST_NAPI_UNCLOSED_CRITICAL_LOG);
    ASSERT_TRUE(deathTest.GetResult());
}
