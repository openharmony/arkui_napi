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
        .AssertError("[ArkNativeFunctionCallBack] critical scope still open after user callback 'testFunc'");
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
        napi_queue_async_work(env, work);
        runner.Run(UV_RUN_ONCE);
        napi_close_critical_scope(env, scope);
    })
        .AssertSignal(SIGABRT)
        .AssertError("[AsyncAfterWorkCallback] critical scope still open after user callback");
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
        .AssertError("critical scope still open after user callback");
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
        .AssertError("critical scope still open after user callback");
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
        .AssertError("[napi_create_limit_runtime] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_fatal_exception] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_create_async_work] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_delete_async_work] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_queue_async_work] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_cancel_async_work] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_get_uv_event_loop] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_add_async_cleanup_hook] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_create_threadsafe_function] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_ref_threadsafe_function] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_unref_threadsafe_function] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_async_init] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_async_destroy] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_open_callback_scope] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_close_callback_scope] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_set_instance_data] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_get_instance_data] current interface cannot invoke under critical scope");
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
        .AssertError("[node_api_get_module_file_name] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_make_callback] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_get_undefined] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_get_null] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_get_global] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_get_boolean] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_create_object] current interface cannot invoke under critical scope");
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
        .AssertError("[napi_create_object_with_properties] current interface cannot invoke under critical scope");
}
