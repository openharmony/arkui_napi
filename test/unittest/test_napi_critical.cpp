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
#include "native_engine/native_async_work.h"
#include "test.h"
#undef private
#include "test_common.h"
#include "utils/log.h"

// use macro instead of constexpr variable, due to need concat with other string
#define TEST_NAPI_UNCLOSED_CRITICAL_LOG " current interface cannot invoke under critical scope"
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

/**
 * @tc.name: NapiUnclosedCriticalTest001
 * @tc.desc: Test check of unclosed critical scope after user callback.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiUnclosedCriticalTest001, testing::ext::TestSize.Level0)
{
    BasicDeathTest([]() {
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
    })
        .AssertSignal(SIGABRT)
        .AssertError("[ArkNativeFunctionCallBack] " TEST_UNCLOSED_CRITICAL_CALLBACK_LOG " 'testFunc'");
}

/**
 * @tc.name: NapiUnclosedCriticalTest002
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiUnclosedCriticalTest002, testing::ext::TestSize.Level0)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        UVLoopRunner runner(*env);
        napi_value name {};
        napi_create_string_utf8(env, "taskName", NAPI_AUTO_LENGTH, &name);
        static napi_critical_scope scope {};
        napi_async_work work {};
        napi_create_async_work(
            env, nullptr, name, []([[maybe_unused]] napi_env env, [[maybe_unused]] void* data) {},
            [](napi_env env, [[maybe_unused]] napi_status status, [[maybe_unused]] void* data) {
                napi_open_critical_scope(env, &scope);
            },
            nullptr, &work);
        NativeAsyncWork::AsyncAfterWorkCallback(&reinterpret_cast<NativeAsyncWork*>(work)->work_, 0);
        napi_delete_async_work(env, work);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[AsyncAfterWorkCallback] " TEST_UNCLOSED_CRITICAL_CALLBACK_LOG);
}

/**
 * @tc.name: NapiUnclosedCriticalTest003
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiUnclosedCriticalTest003, testing::ext::TestSize.Level0)
{
    BasicDeathTest([] {
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
    })
        .AssertSignal(SIGABRT)
        .AssertError("[ProcessAsyncHandle] " TEST_UNCLOSED_CRITICAL_CALLBACK_LOG);
}

/**
 * @tc.name: NapiUnclosedCriticalTest004
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiUnclosedCriticalTest004, testing::ext::TestSize.Level0)
{
    BasicDeathTest([] {
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
    })
        .AssertSignal(SIGABRT)
        .AssertError(TEST_UNCLOSED_CRITICAL_CALLBACK_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest001
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest001, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_limit_runtime(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_limit_runtime]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest002
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest002, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_fatal_exception(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_fatal_exception]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest003
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest003, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_async_work(env, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_async_work]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest004
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest004, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_delete_async_work(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_delete_async_work]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest005
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest005, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_queue_async_work(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_queue_async_work]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest006
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest006, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_cancel_async_work(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_cancel_async_work]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest007
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest007, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_uv_event_loop(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_uv_event_loop]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest008
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest008, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_add_async_cleanup_hook(env, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_add_async_cleanup_hook]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest009
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest009, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_threadsafe_function(env, nullptr, nullptr, nullptr, 0, 0, nullptr, nullptr, nullptr, nullptr,
                                        nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_threadsafe_function]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest010
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest010, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_ref_threadsafe_function(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_ref_threadsafe_function]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest011
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest011, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_unref_threadsafe_function(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_unref_threadsafe_function]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest012
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest012, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_async_init(env, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_async_init]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest013
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest013, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_async_destroy(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_async_destroy]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest014
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest014, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_open_callback_scope(env, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_open_callback_scope]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest015
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest015, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_close_callback_scope(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_close_callback_scope]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest016
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest016, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_set_instance_data(env, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_set_instance_data]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest017
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest017, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_instance_data(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_instance_data]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest018
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest018, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        node_api_get_module_file_name(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[node_api_get_module_file_name]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest019
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest019, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_make_callback(env, nullptr, nullptr, nullptr, 0, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_make_callback]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest020
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest020, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_undefined(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_undefined]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest021
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest021, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_null(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_null]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest022
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest022, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_global(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_global]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest023
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest023, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_boolean(env, true, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_boolean]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest024
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest024, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_object(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_object]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest025
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest025, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_object_with_properties(env, nullptr, 0, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_object_with_properties]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest026
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest026, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_object_with_named_properties(env, nullptr, 0, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_object_with_named_properties]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest027
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest027, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_array(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_array]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest028
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest028, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_array_with_length(env, 0, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_array_with_length]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest029
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest029, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_sendable_array(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_sendable_array]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest030
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest030, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_sendable_array_with_length(env, 0, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_sendable_array_with_length]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest031
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest031, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_double(env, 0, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_double]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest032
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest032, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_int32(env, 0, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_int32]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest033
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest033, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_uint32(env, 0, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_uint32]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest034
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest034, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_int64(env, 0, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_int64]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest035
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest035, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_string_latin1(env, "", 0, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_string_latin1]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest036
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest036, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_string_utf8(env, "", 0, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_string_utf8]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest037
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest037, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_string_utf16(env, u"", 0, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_string_utf16]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest038
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest038, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_symbol(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_symbol]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest039
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest039, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_function(env, "", 0, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_function]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest040
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest040, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_error(env, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_error]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest041
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest041, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_type_error(env, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_type_error]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest042
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest042, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_range_error(env, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_range_error]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest043
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest043, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_typeof(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_typeof]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest044
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest044, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_value_double(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_value_double]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest045
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest045, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_value_int32(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_value_int32]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest046
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest046, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_value_uint32(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_value_uint32]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest047
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest047, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_value_int64(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_value_int64]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest048
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest048, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_value_bool(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_value_bool]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest049
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest049, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_value_string_latin1(env, nullptr, nullptr, 0, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_value_string_latin1]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest050
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest050, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_value_string_utf8(env, nullptr, nullptr, 0, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_value_string_utf8]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest051
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest051, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_value_string_utf16(env, nullptr, nullptr, 0, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_value_string_utf16]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest052
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest052, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_open_critical_scope(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_open_critical_scope]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest053
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest053, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_coerce_to_bool(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_coerce_to_bool]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest054
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest054, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_coerce_to_number(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_coerce_to_number]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest055
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest055, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_coerce_to_object(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_coerce_to_object]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest056
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest056, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_coerce_to_string(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_coerce_to_string]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest057
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest057, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_prototype(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_prototype]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest058
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest058, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_property_names(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_property_names]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest059
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest059, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_set_property(env, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_set_property]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest060
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest060, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_has_property(env, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_has_property]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest061
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest061, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_property(env, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_property]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest062
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest062, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_delete_property(env, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_delete_property]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest063
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest063, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_has_own_property(env, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_has_own_property]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest064
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest064, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_set_named_property(env, nullptr, "", nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_set_named_property]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest065
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest065, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_has_named_property(env, nullptr, "", nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_has_named_property]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest066
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest066, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_named_property(env, nullptr, "", nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_named_property]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest067
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest067, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_own_property_descriptor(env, nullptr, "", nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_own_property_descriptor]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest068
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest068, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_set_element(env, nullptr, 0, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_set_element]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest069
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest069, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_has_element(env, nullptr, 0, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_has_element]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest070
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest070, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_element(env, nullptr, 0, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_element]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest071
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest071, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_delete_element(env, nullptr, 0, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_delete_element]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest072
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest072, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_define_properties(env, nullptr, 0, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_define_properties]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest073
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest073, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_is_array(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_is_array]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest074
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest074, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_is_sendable(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_is_sendable]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest075
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest075, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_strict_equals(env, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_strict_equals]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest076
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest076, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_call_function(env, nullptr, nullptr, 0, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_call_function]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest077
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest077, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_new_instance(env, nullptr, 0, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_new_instance]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest078
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest078, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_instanceof(env, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_instanceof]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest079
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest079, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_get_new_target(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_get_new_target]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest080
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest080, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_define_class(env, "", 0, nullptr, nullptr, 0, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_define_class]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest081
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest081, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_define_sendable_class(env, "", 0, nullptr, nullptr, 0, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_define_sendable_class]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest082
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest082, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_sendable_object_with_properties(env, 0, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_sendable_object_with_properties]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest083
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest083, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_map(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_map]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest084
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest084, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_create_sendable_map(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_create_sendable_map]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest085
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest085, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_map_set_property(env, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_map_set_property]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest086
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest086, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_map_set_named_property(env, nullptr, "", nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_map_set_named_property]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest087
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest087, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_map_get_property(env, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_map_get_property]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest088
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest088, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_map_get_named_property(env, nullptr, "", nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_map_get_named_property]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest089
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest089, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_map_has_property(env, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_map_has_property]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest090
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest090, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_map_has_named_property(env, nullptr, "", nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_map_has_named_property]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest091
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest091, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_map_delete_property(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_map_delete_property]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest092
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest092, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_map_clear(env, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_map_clear]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest093
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest093, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_map_get_size(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_map_get_size]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest094
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest094, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_map_get_entries(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_map_get_entries]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest095
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest095, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_map_get_keys(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_map_get_keys]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest096
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest096, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_map_get_values(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_map_get_values]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest097
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest097, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_map_iterator_get_next(env, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_map_iterator_get_next]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest098
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest098, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_wrap(env, nullptr, nullptr, nullptr, nullptr, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_wrap]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest099
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest099, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_wrap_enhance(env, nullptr, nullptr, nullptr, true, nullptr, 0, nullptr);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_wrap_enhance]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}

/**
 * @tc.name: NapiNonCriticalTest100
 * @tc.desc: Test interface cannot invoke while critical scope is opening.
 * @tc.type: FUNC
 */
HWTEST_F(NapiCriticalTest, NapiNonCriticalTest100, testing::ext::TestSize.Level1)
{
    BasicDeathTest([] {
        NativeEngineProxy env;
        napi_critical_scope scope {};
        napi_open_critical_scope(env, &scope);
        napi_wrap_async_finalizer(env, nullptr, nullptr, nullptr, nullptr, nullptr, 0);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[napi_wrap_async_finalizer]" TEST_NAPI_UNCLOSED_CRITICAL_LOG);
}
