/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <chrono>
#include <thread>
#include <uv.h>

#include "event_runner.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"
#include "utils/log.h"

struct CallJsCbData_t {
    int32_t id = 0;
    bool secondaryThread = false;
    napi_threadsafe_function_call_mode blockOnFull = napi_tsfn_nonblocking;
};

struct FinalCbData_t {
    int32_t id = 0;
};

struct CallbackData {
    napi_threadsafe_function tsfn;
    napi_async_work work;
    napi_task_priority priority;
};

static constexpr int32_t SEND_DATA_TEST = 11;
static constexpr int32_t CALL_JS_CB_DATA_TEST_ID = 101;
static constexpr int32_t FINAL_CB_DATA_TEST_ID = 1001;
static constexpr int32_t SEND_DATAS_LENGTH = 10;
static constexpr int32_t THREAD_COUNT = 2;
static constexpr int32_t THREAD_COUNT_FOUR = 4;
static constexpr int32_t MAX_QUEUE_SIZE = 3;
static constexpr int32_t SUCCESS_COUNT_JS_FOUR = 4;
static constexpr int32_t CALL_THREAD_SAFE_SLEEP = 2; // 2s
static constexpr int32_t FIRST_TASK_SLEEP = 4; // 4s
static constexpr int32_t INVALID_NAPI_THREAD_SAFE_PRIORITY = -1;
static constexpr int32_t ADD_FIRST_NUMBER = 1;
static constexpr int32_t ADD_SECOND_NUMBER = 2;
static constexpr int32_t ADD_SUMMARY_RESULT = 3;
static constexpr int32_t ARGS_SIZE = 2;
static constexpr int32_t FIRST_RECEIVER = 1;
static constexpr int32_t SECOND_RECEIVER = 2;
static constexpr int32_t THIRD_RECEIVER = 3;
static constexpr int32_t FOURTH_RECEIVER = 4;
static constexpr int32_t FIFTH_RECEIVER = 5;
static constexpr int32_t DATA_LENGTH = 40;
static constexpr int32_t THREAD_SIZE = 5;
static constexpr int32_t FIRST_THREAD_INDEX = 0;
static constexpr int32_t SECOND_THREAD_INDEX = 1;
static constexpr int32_t THIRD_THREAD_INDEX = 2;
static constexpr int32_t FOURTH_THREAD_INDEX = 3;
static constexpr int32_t FIFTH_THREAD_INDEX = 4;

static pid_t g_mainTid = 0;
static CallJsCbData_t g_jsData;
static CallJsCbData_t g_jsDataInternal;
static FinalCbData_t g_finalData;
static int32_t g_sendData = 0;
static uv_thread_t g_uvThread;
static uv_thread_t g_uvThreadTest5;
static uv_thread_t g_uvThreadTest6;
static uv_thread_t g_uvThreadTest7;
static uv_thread_t g_uvThreadSecondary;
static uv_thread_t g_uvTheads2;
static uv_thread_t g_uvTheads3;
static int32_t g_sendDatas[SEND_DATAS_LENGTH];
static int32_t  g_callSuccessCount = 0;
static int32_t  g_callSuccessCountJS = 0;
static int32_t  g_callSuccessCountJSFour = 0;
static int32_t  g_callDepth = 4;
bool acquireFlag = false;
static int32_t g_receiveCnt = 0;
static bool g_isTailA = false;
static bool g_isTailB = false;

static void TsFuncCallJs(napi_env env, napi_value tsfn_cb, void* context, void* data)
{
    HILOG_INFO("TsFuncCallJs called");

    EXPECT_EQ(gettid(), g_mainTid);

    // expect context equal
    EXPECT_EQ(((CallJsCbData_t*)context)->id, CALL_JS_CB_DATA_TEST_ID);

    // expect data equal
    int* pData = (int32_t*)data;
    EXPECT_EQ((*pData), SEND_DATA_TEST);
}

static void TsFuncCallJsWithNewCall(napi_env env, napi_value tsfn_cb, void* context, void* data)
{
    HILOG_INFO("TsFuncCallJsWithNewCall called");
    EXPECT_EQ(gettid(), g_mainTid);

    // expect context equal
    EXPECT_EQ(((CallJsCbData_t*)context)->id, CALL_JS_CB_DATA_TEST_ID);

    napi_threadsafe_function tsFunc = (napi_threadsafe_function)data;

    if (g_callDepth > 0) {
        g_callDepth--;
        auto status = napi_call_threadsafe_function(tsFunc, data, napi_tsfn_nonblocking);
        EXPECT_EQ(status, napi_ok);
    }
    if (g_callDepth == 0) {
        auto status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
        EXPECT_EQ(status, napi_ok);
    }
}

static void TsFuncCallJsTwo(napi_env env, napi_value tsfn_cb, void* context, void* data)
{
    HILOG_INFO("TsFuncCallJsTwo called");
    TsFuncCallJs(env, tsfn_cb, context, data);
    g_callSuccessCountJS++;
}
static void TsFuncCallJsFour(napi_env env, napi_value tsfn_cb, void* context, void* data)
{
    HILOG_INFO("TsFuncCallJsFour called");

    TsFuncCallJs(env, tsfn_cb, context, data);
    g_callSuccessCountJSFour++;
}
static void TsFuncCallJsMulti(napi_env env,
                              napi_value tsfn_cb,
                              void* context,
                              void* data)
{
    HILOG_INFO("TsFuncCallJsMulti called");

    EXPECT_EQ(gettid(), g_mainTid);

    // expect context equal
    EXPECT_EQ(((CallJsCbData_t*)context)->id, CALL_JS_CB_DATA_TEST_ID);

    int* pData = ((int32_t*)data);

    HILOG_INFO("TsFuncCallJsMulti data %d", (*pData));
}

static void TsFuncFinal(napi_env env, void* finalizeData, void* hint)
{
    HILOG_INFO("TsFuncFinal called");

    // expect thread id equal
    EXPECT_EQ(gettid(), g_mainTid);

    // wait data source thread
    uv_thread_join(&g_uvThread);

    // expect context equal
    EXPECT_EQ(((CallJsCbData_t*)hint)->id, CALL_JS_CB_DATA_TEST_ID);

    // expect finalize data equal
    EXPECT_EQ(((FinalCbData_t*)finalizeData)->id, FINAL_CB_DATA_TEST_ID);
}
static void TsFuncFinalTest5(napi_env env, void* finalizeData, void* hint)
{
    HILOG_INFO("TsFuncFinalTest5 called");

    // expect thread id equal
    EXPECT_EQ(gettid(), g_mainTid);

    // wait data source thread
    uv_thread_join(&g_uvThreadTest5);

    // expect context equal
    EXPECT_EQ(((CallJsCbData_t*)hint)->id, CALL_JS_CB_DATA_TEST_ID);

    // expect finalize data equal
    EXPECT_EQ(((FinalCbData_t*)finalizeData)->id, FINAL_CB_DATA_TEST_ID);
}
static void TsFuncFinalTotal(napi_env env, void* finalizeData, void* hint)
{
    HILOG_INFO("TsFuncFinalTotal called");
    uv_thread_join(&g_uvThreadTest6);
    // when add thread,repair  g_callSuccessCountJS eq  SUCCESS_COUNT_JS_TWO
    EXPECT_EQ(g_callSuccessCountJS, SUCCESS_COUNT_JS_FOUR);
    HILOG_INFO("TsFuncFinalTotal end");
}
static void TsFuncFinalTotalFour(napi_env env, void* finalizeData, void* hint)
{
    HILOG_INFO("TsFuncFinalTotalFour called");
    uv_thread_join(&g_uvThreadTest7);
    EXPECT_EQ(g_callSuccessCountJSFour, SUCCESS_COUNT_JS_FOUR);
    HILOG_INFO("TsFuncFinalTotalFour end");
}
static void TsFuncFinalCallback(napi_env env, void* finalizeData, void* hint)
{
    HILOG_INFO("TsFuncFinalCallback called");
    EXPECT_EQ(g_callSuccessCountJSFour, SUCCESS_COUNT_JS_FOUR);
    HILOG_INFO("TsFuncFinalCallback end");
}

static void TsFuncFinalJoinThread(napi_env env, void* data, void* hint)
{
    HILOG_INFO("TsFuncFinalJoinThread called");

    uv_thread_t *uvThread = reinterpret_cast<uv_thread_t*>(data);
    CallJsCbData_t *jsData = reinterpret_cast<CallJsCbData_t*>(hint);

    uv_thread_join(uvThread);

    if (jsData->secondaryThread) {
        uv_thread_join(&g_uvThreadSecondary);
    }
}

static void TsFuncSecondaryThread(void* data)
{
    HILOG_INFO("TsFuncSecondaryThread called");

    // expect thread id not equal
    EXPECT_NE(gettid(), g_mainTid);

    napi_threadsafe_function func = (napi_threadsafe_function)data;

    auto status = napi_release_threadsafe_function(func, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
}

static void TsFuncDataSourceThread(void* data)
{
    HILOG_INFO("TsFuncDataSourceThread called");

    // expect thread id not equal
    EXPECT_NE(gettid(), g_mainTid);

    napi_threadsafe_function func = (napi_threadsafe_function)data;
    napi_threadsafe_function_call_mode blockMode = napi_tsfn_nonblocking;
    void* context = nullptr;

    auto status = napi_get_threadsafe_function_context(func, &context);
    EXPECT_EQ(status, napi_ok);

    // expect context equal
    EXPECT_EQ(((CallJsCbData_t*)context)->id, CALL_JS_CB_DATA_TEST_ID);

    // set send data
    g_sendData = SEND_DATA_TEST;

    // As main thread has set initial_thread_count to 1 and only this one secondary thread,
    // so no need to call `napi_acquire_threadsafe_function()`.
    status = napi_call_threadsafe_function(func, &g_sendData, blockMode);
    EXPECT_EQ(status, napi_ok);

    status = napi_release_threadsafe_function(func, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
}
static void TsFuncDataSourceThreadAbort(void* data)
{
    HILOG_INFO("TsFuncDataSourceThreadAbort called");

    // expect thread id not equal
    EXPECT_NE(gettid(), g_mainTid);

    napi_threadsafe_function func = (napi_threadsafe_function)data;
    napi_threadsafe_function_call_mode blockMode = napi_tsfn_nonblocking;
    void* context = nullptr;

    auto status = napi_get_threadsafe_function_context(func, &context);
    EXPECT_EQ(status, napi_ok);

    // expect context equal
    EXPECT_EQ(((CallJsCbData_t*)context)->id, CALL_JS_CB_DATA_TEST_ID);

    // set send data
    g_sendData = SEND_DATA_TEST;

    status = napi_call_threadsafe_function(func, &g_sendData, blockMode);
    EXPECT_EQ(status, napi_closing);
}

static void TsFuncDataSourceThreadCountTotal(void* data)
{
    HILOG_INFO("TsFuncDataSourceThreadCountTotal called");

    // expect thread id not equal
    EXPECT_NE(gettid(), g_mainTid);

    napi_threadsafe_function func = (napi_threadsafe_function)data;
    napi_threadsafe_function_call_mode blockMode = napi_tsfn_nonblocking;
    void* context = nullptr;

    auto status = napi_get_threadsafe_function_context(func, &context);
    EXPECT_EQ(status, napi_ok);

    // expect context equal
    EXPECT_EQ(((CallJsCbData_t*)context)->id, CALL_JS_CB_DATA_TEST_ID);
    // set send data
    g_sendData = SEND_DATA_TEST;
    if (acquireFlag) {
        std::cout<<"acquireFlag  is true"<<std::endl;
        status = napi_acquire_threadsafe_function(func);
        EXPECT_EQ(status, napi_ok);
        status = napi_call_threadsafe_function(func, &g_sendData, blockMode);
        if (status == napi_ok) {
            g_callSuccessCount++;
        }
        status = napi_release_threadsafe_function(func, napi_tsfn_release);
    } else {
        status = napi_call_threadsafe_function(func, &g_sendData, blockMode);
        if (status == napi_ok) {
            g_callSuccessCount++;
        }
    }
    status = napi_release_threadsafe_function(func, napi_tsfn_release);
}

static void TsFuncDataSourceThreadMulti(void* data)
{
    HILOG_INFO("TsFuncDataSourceThreadMulti called");

    // expect thread id not equal
    EXPECT_NE(gettid(), g_mainTid);

    napi_threadsafe_function func =  (napi_threadsafe_function)data;
    void* context = nullptr;

    auto status = napi_get_threadsafe_function_context(func, &context);
    EXPECT_EQ(status, napi_ok);
    CallJsCbData_t* jsData = nullptr;
    jsData = (CallJsCbData_t*)context;

    if (jsData->secondaryThread) {
        status = napi_acquire_threadsafe_function(func);
        EXPECT_EQ(status, napi_ok);

        if (uv_thread_create(&g_uvThreadSecondary, TsFuncSecondaryThread, func) != 0) {
            HILOG_ERROR("Failed to create uv thread!");
        }
    }

    bool queueClosing = false;
    bool queueFull = false;
    int32_t index = 0;
    for (index = SEND_DATAS_LENGTH - 1; index > -1 && !queueClosing; index--) {
        g_sendDatas[index] = index;
        status = napi_call_threadsafe_function(func, &g_sendDatas[index], jsData->blockOnFull);
        HILOG_INFO("napi_call_threadsafe_function index %d status %d", index, status);

        switch (status) {
            case napi_queue_full:
                queueFull = true;
                index++;
                [[fallthrough]];
            case napi_ok:
                continue;
            case napi_closing:
                queueClosing = true;
                break;
            default:
                HILOG_ERROR("Failed to call napi_call_threadsafe_function!");
        }
    }

    if (!queueClosing && (napi_release_threadsafe_function(func, napi_tsfn_release) != napi_ok)) {
        HILOG_ERROR("Failed to call napi_release_threadsafe_function!");
    }
}

static void TsFuncThreadInternal(napi_env env,
                                 napi_threadsafe_function_call_js cb,
                                 uv_thread_t& uvThread,
                                 bool secondary,
                                 bool block)
{
    HILOG_INFO("TsFuncThreadInternal start secondary %d block %d", secondary, block);

    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_mainTid = gettid();

    g_jsDataInternal.id = CALL_JS_CB_DATA_TEST_ID;
    g_jsDataInternal.secondaryThread = (secondary ? true : false);
    g_jsDataInternal.blockOnFull = (block ? napi_tsfn_blocking : napi_tsfn_nonblocking);

    auto status = napi_create_threadsafe_function(env,
                                                  nullptr,
                                                  nullptr,
                                                  resourceName,
                                                  MAX_QUEUE_SIZE,
                                                  THREAD_COUNT,
                                                  &uvThread,
                                                  TsFuncFinalJoinThread,
                                                  &g_jsDataInternal,
                                                  cb,
                                                  &tsFunc);
    EXPECT_EQ(status, napi_ok);

    if (uv_thread_create(&uvThread, TsFuncDataSourceThreadMulti, tsFunc) != 0) {
        HILOG_ERROR("Failed to create uv thread!");
    }

    HILOG_INFO("TsFuncThreadInternal end");
}

static napi_value JsCallback(napi_env env, napi_callback_info info)
{
    size_t agrc = ARGS_SIZE;
    napi_value argv[ARGS_SIZE] = { nullptr };
    napi_get_cb_info(env, info, &agrc, argv, nullptr, nullptr);

    int32_t number1 = 0;
    napi_get_value_int32(env, argv[0], &number1);

    int32_t number2 = 0;
    napi_get_value_int32(env, argv[1], &number2);

    napi_value result = nullptr;
    napi_create_int32(env, number1 + number2, &result);
    return result;
}

static void CallJsCallback(napi_env env, napi_value jsCb)
{
    napi_value undefined;
    napi_get_undefined(env, &undefined);
    napi_value numberOne = nullptr;
    napi_create_int32(env, ADD_FIRST_NUMBER, &numberOne);
    napi_value numberTwo = nullptr;
    napi_create_int32(env, ADD_SECOND_NUMBER, &numberTwo);
    napi_value argv[ARGS_SIZE] = { numberOne, numberTwo };

    napi_value result = nullptr;
    napi_call_function(env, undefined, jsCb, ARGS_SIZE, argv, &result);

    int32_t res = 0;
    napi_get_value_int32(env, result, &res);
    EXPECT_EQ(res, ADD_SUMMARY_RESULT);
}

static void StopCurrentRunner()
{
    auto runner = OHOS::AppExecFwk::EventRunner::Current();
    if (runner != nullptr) {
        HILOG_INFO("Stop the current runner!");
        runner->Stop();
    }
}

static void CallJs(napi_env env, napi_value jsCb, void *context, void *data)
{
    EXPECT_NE(env, nullptr);

    g_receiveCnt++;
    if (g_receiveCnt == FIRST_RECEIVER) {
        std::this_thread::sleep_for(std::chrono::seconds(FIRST_TASK_SLEEP));
        return;
    }
    // check data string
    char *testData = reinterpret_cast<char *>(data);
    EXPECT_NE(testData, nullptr) << "testData is nullptr";
    int32_t ret = 0;
    if (g_receiveCnt == SECOND_RECEIVER) {
        if (!g_isTailB) {
            ret = strcmp(testData, "hello world from B");
        } else {
            ret = strcmp(testData, "hello world from A");
        }
    }
    if (g_receiveCnt == THIRD_RECEIVER) {
        if (!g_isTailB) {
            ret = strcmp(testData, "hello world from A");
        } else {
            ret = strcmp(testData, "hello world from B");
        }
    }
    EXPECT_EQ(ret, 0);

    CallJsCallback(env, jsCb);
    if (g_receiveCnt == THIRD_RECEIVER) {
        StopCurrentRunner();
    }
}

static void CallJsWithDiffPriority(napi_env env, napi_value jsCb, void *context, void *data)
{
    EXPECT_NE(env, nullptr);

    g_receiveCnt++;
    if (g_receiveCnt == FIRST_RECEIVER) {
        std::this_thread::sleep_for(std::chrono::seconds(FIRST_TASK_SLEEP));
        return;
    }
    // check data string
    char *testData = reinterpret_cast<char *>(data);
    EXPECT_NE(testData, nullptr) << "testData is nullptr";
    int32_t ret = 0;
    if (g_receiveCnt == SECOND_RECEIVER) {
        ret = strcmp(testData, "hello world from A");
    } else if (g_receiveCnt == THIRD_RECEIVER) {
        ret = strcmp(testData, "hello world from B");
    } else if (g_receiveCnt == FOURTH_RECEIVER) {
        ret = strcmp(testData, "hello world from C");
    } else if (g_receiveCnt == FIFTH_RECEIVER) {
        ret = strcmp(testData, "hello world from D");
    }
    EXPECT_EQ(ret, 0);

    CallJsCallback(env, jsCb);
    if (g_receiveCnt == FIFTH_RECEIVER) {
        StopCurrentRunner();
    }
}

class UnitLoopHandler : public OHOS::AppExecFwk::FileDescriptorListener,
    public std::enable_shared_from_this<UnitLoopHandler> {
public:
    explicit UnitLoopHandler(uv_loop_t* uvLoop) : uvLoop_(uvLoop) {}
    void OnReadable(int32_t) override { uv_run(uvLoop_, UV_RUN_NOWAIT); }
    void OnWritable(int32_t) override { uv_run(uvLoop_, UV_RUN_NOWAIT); }

private:
    uv_loop_t* uvLoop_ = nullptr;
};

class NapiThreadsafeTest : public NativeEngineTest {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "NapiThreadsafeTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "NapiThreadsafeTest TearDownTestCase";
    }

    void SetUp() override { AttachEventHandler(); }
    void TearDown() override {}

    void CallThreadSafeWithSamePriorityTest(napi_task_priority priority);
    void CallThreadSafeWithDiffPriorityTest();
    void CallThreadSafeWithDiffPriorityMultipleThreadTest();
    void AttachEventHandler();
};

void NapiThreadsafeTest::AttachEventHandler()
{
    if (eventHandler_ != nullptr) {
        return;
    }
    auto uvLoop = engine_->GetUVLoop();
    auto fd = uvLoop != nullptr ? uv_backend_fd(uvLoop) : -1;
    EXPECT_GE(fd, 0);
    uv_run(uvLoop, UV_RUN_NOWAIT);
    auto runner = OHOS::AppExecFwk::EventRunner::Create(false);
    EXPECT_NE(runner, nullptr);
    eventHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    EXPECT_NE(eventHandler_, nullptr);
    uint32_t events = OHOS::AppExecFwk::FILE_DESCRIPTOR_INPUT_EVENT | OHOS::AppExecFwk::FILE_DESCRIPTOR_OUTPUT_EVENT;
    eventHandler_->AddFileDescriptorListener(fd, events, std::make_shared<UnitLoopHandler>(uvLoop), "uvLoopTask");
    HILOG_INFO("AttachEventHandler is completed!");
}

static void CallThreadSafeFunc(napi_threadsafe_function tsfn, napi_task_priority priority)
{
    char *testData = (char *)malloc(DATA_LENGTH);
    if (testData == nullptr) {
        return;
    }
    memset_s(testData, DATA_LENGTH, 0, DATA_LENGTH);
    if (priority == napi_priority_immediate) {
        strcpy_s(testData, DATA_LENGTH, "hello world from A");
    } else if (priority == napi_priority_high) {
        strcpy_s(testData, DATA_LENGTH, "hello world from B");
    } else if (priority == napi_priority_low) {
        strcpy_s(testData, DATA_LENGTH, "hello world from C");
    } else if (priority == napi_priority_idle) {
        strcpy_s(testData, DATA_LENGTH, "hello world from D");
    }
    auto status =
        napi_call_threadsafe_function_with_priority(tsfn, testData, priority, true);
    EXPECT_EQ(status, napi_ok);
}

void NapiThreadsafeTest::CallThreadSafeWithSamePriorityTest(napi_task_priority priority)
{
    napi_env env = (napi_env)engine_;
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    CallbackData *callbackData = new CallbackData();
    callbackData->priority = priority;
    napi_value testFunc = nullptr;
    napi_create_function(env, "jsCallback", NAPI_AUTO_LENGTH, JsCallback, nullptr, &testFunc);

    auto status = napi_create_threadsafe_function(env, testFunc, nullptr, resourceName,
        0, 1, callbackData, nullptr, callbackData, CallJs, &callbackData->tsfn);
    EXPECT_EQ(status, napi_ok);

    g_receiveCnt = 0;
    napi_create_async_work(
        env, nullptr, resourceName, [](napi_env env, void* data) {
            CallbackData* callbackData = (CallbackData*)data;
            char *testDataA = (char *)malloc(DATA_LENGTH);
            memset_s(testDataA, DATA_LENGTH, 0, DATA_LENGTH);
            strcpy_s(testDataA, DATA_LENGTH, "hello world from A");

            char *testDataB = (char *)malloc(DATA_LENGTH);
            memset_s(testDataB, DATA_LENGTH, 0, DATA_LENGTH);
            strcpy_s(testDataB, DATA_LENGTH, "hello world from B");
            // first call function to post a sleep task, then the next execution from event queue which
            // contains two tasks.
            auto status =
                napi_call_threadsafe_function_with_priority(callbackData->tsfn, testDataA, napi_priority_immediate,
                    true);
            EXPECT_EQ(status, napi_ok);
            std::this_thread::sleep_for(std::chrono::seconds(CALL_THREAD_SAFE_SLEEP));
            status = napi_call_threadsafe_function_with_priority(callbackData->tsfn, testDataA,
                callbackData->priority, g_isTailA);
            EXPECT_EQ(status, napi_ok);
            status = napi_call_threadsafe_function_with_priority(callbackData->tsfn, testDataB,
                callbackData->priority, g_isTailB);
            EXPECT_EQ(status, napi_ok);
        },
        [](napi_env env, napi_status status, void* data) {
            CallbackData* callbackData = (CallbackData*)data;
            napi_delete_async_work(env, callbackData->work);
            auto status1 = napi_release_threadsafe_function(callbackData->tsfn, napi_tsfn_release);
            EXPECT_EQ(status1, napi_ok);
            delete callbackData;
        },
        callbackData, &callbackData->work);
    napi_queue_async_work(env, callbackData->work);
}

void NapiThreadsafeTest::CallThreadSafeWithDiffPriorityTest()
{
    napi_env env = (napi_env)engine_;
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    CallbackData *callbackData = new CallbackData();
    napi_value testFunc = nullptr;
    napi_create_function(env, "jsCallback", NAPI_AUTO_LENGTH, JsCallback, nullptr, &testFunc);

    auto status = napi_create_threadsafe_function(env, testFunc, nullptr, resourceName,
        0, 1, callbackData, nullptr, callbackData, CallJsWithDiffPriority, &callbackData->tsfn);
    EXPECT_EQ(status, napi_ok);

    g_receiveCnt = 0;
    napi_create_async_work(
        env, nullptr, resourceName, [](napi_env env, void* data) {
            CallbackData* callbackData = (CallbackData*)data;
            // first call function to post a sleep task, then the next execution from event queue which
            // contains four different priority tasks.
            auto status =
                napi_call_threadsafe_function_with_priority(callbackData->tsfn, nullptr, napi_priority_immediate,
                    true);
            EXPECT_EQ(status, napi_ok);
            std::this_thread::sleep_for(std::chrono::seconds(CALL_THREAD_SAFE_SLEEP));
            CallThreadSafeFunc(callbackData->tsfn, napi_priority_immediate);
            CallThreadSafeFunc(callbackData->tsfn, napi_priority_high);
            CallThreadSafeFunc(callbackData->tsfn, napi_priority_low);
            CallThreadSafeFunc(callbackData->tsfn, napi_priority_idle);
        },
        [](napi_env env, napi_status status, void* data) {
            CallbackData* callbackData = (CallbackData*)data;
            napi_delete_async_work(env, callbackData->work);
            auto status1 = napi_release_threadsafe_function(callbackData->tsfn, napi_tsfn_release);
            EXPECT_EQ(status1, napi_ok);
            delete callbackData;
        },
        callbackData, &callbackData->work);
    napi_queue_async_work(env, callbackData->work);
}

void NapiThreadsafeTest::CallThreadSafeWithDiffPriorityMultipleThreadTest()
{
    napi_env env = (napi_env)engine_;
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    CallbackData *callbackData = new CallbackData();
    napi_value testFunc = nullptr;
    napi_create_function(env, "jsCallback", NAPI_AUTO_LENGTH, JsCallback, nullptr, &testFunc);

    auto status = napi_create_threadsafe_function(env, testFunc, nullptr, resourceName,
        0, 1, callbackData, nullptr, callbackData, CallJsWithDiffPriority, &callbackData->tsfn);
    EXPECT_EQ(status, napi_ok);

    g_receiveCnt = 0;
    auto runFunc = [callbackData](const napi_env &env, int32_t threadIndex) {
        if (threadIndex == FIRST_THREAD_INDEX) {
            auto status =
                napi_call_threadsafe_function_with_priority(callbackData->tsfn, nullptr, napi_priority_immediate,
                    true);
            EXPECT_EQ(status, napi_ok);
        } else if (threadIndex == SECOND_THREAD_INDEX) {
            CallThreadSafeFunc(callbackData->tsfn, napi_priority_immediate);
        } else if (threadIndex == THIRD_THREAD_INDEX) {
            CallThreadSafeFunc(callbackData->tsfn, napi_priority_high);
        } else if (threadIndex == FOURTH_THREAD_INDEX) {
            CallThreadSafeFunc(callbackData->tsfn, napi_priority_low);
        } else if (threadIndex == FIFTH_THREAD_INDEX) {
            CallThreadSafeFunc(callbackData->tsfn, napi_priority_idle);
        }
    };
    std::thread runFirstThread = std::thread(runFunc, std::ref(env), 0);
    runFirstThread.detach();
    std::this_thread::sleep_for(std::chrono::seconds(CALL_THREAD_SAFE_SLEEP));
    for (int32_t index = 1; index < THREAD_SIZE; ++index) {
        std::thread runThread = std::thread(runFunc, std::ref(env), index);
        runThread.detach();
    }
}

/**
 * @tc.name: ThreadsafeTest001
 * @tc.desc: Test LoadModule Func.
 * @tc.type: FUNC
 * @tc.require: I5K6KF
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeTest001, testing::ext::TestSize.Level1)
{
    HILOG_INFO("Threadsafe_Test_0100 start");
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_mainTid = gettid();
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;

    auto status = napi_create_threadsafe_function(env,
                                                  nullptr,
                                                  nullptr,
                                                  resourceName,
                                                  0,
                                                  1,
                                                  &g_finalData,
                                                  TsFuncFinal,
                                                  &g_jsData,
                                                  TsFuncCallJs,
                                                  &tsFunc);
    EXPECT_EQ(status, napi_ok);

    if (uv_thread_create(&g_uvThread, TsFuncDataSourceThread, tsFunc) != 0) {
        HILOG_ERROR("Failed to create uv thread!");
    }

    HILOG_INFO("Threadsafe_Test_0100 end");
}

/**
 * @tc.name: ThreadsafeTest002
 * @tc.desc: Test LoadModule Func.
 * @tc.type: FUNC
 * @tc.require: I5K6KF
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeTest002, testing::ext::TestSize.Level1)
{
    HILOG_INFO("Threadsafe_Test_0200 start");

    // start secondary thread, block on full
    TsFuncThreadInternal((napi_env)engine_, TsFuncCallJsMulti, g_uvTheads2, true, true);

    HILOG_INFO("Threadsafe_Test_0200 end");
}

/**
 * @tc.name: ThreadsafeTest003
 * @tc.desc: Test threadsafe Func, no js.
 * @tc.type: FUNC
 * @tc.require: I5K6KF
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeTest003, testing::ext::TestSize.Level1)
{
    HILOG_INFO("Threadsafe_Test_0300 start");

    // secondary thread, not block
    TsFuncThreadInternal((napi_env)engine_, TsFuncCallJsMulti, g_uvTheads3, false, false);

    HILOG_INFO("Threadsafe_Test_0300 end");
}

/**
 * @tc.name: ThreadsafeTest004
 * @tc.desc: Test napi_release_threadsafe_function, napi_tsfn_abort.
 * @tc.type: FUNC
 * @tc.require: I5K6KF
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeTest004, testing::ext::TestSize.Level1)
{
    HILOG_INFO("Threadsafe_Test_0400 start");
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_mainTid = gettid();
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;

    auto status = napi_create_threadsafe_function(env,
                                                  nullptr,
                                                  nullptr,
                                                  resourceName,
                                                  0,
                                                  10,
                                                  &g_finalData,
                                                  TsFuncFinalTest5,
                                                  &g_jsData,
                                                  TsFuncCallJs,
                                                  &tsFunc);
    EXPECT_EQ(status, napi_ok);
    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_abort);
    EXPECT_EQ(status, napi_ok);
    if (uv_thread_create(&g_uvThreadTest5, TsFuncDataSourceThreadAbort, tsFunc) != 0) {
        HILOG_ERROR("Failed to create uv thread!");
    }

    HILOG_INFO("Threadsafe_Test_0400 end");
}

/**
 * @tc.name: ThreadsafeTest005
 * @tc.desc: Test initial_thread_count not enough.
 * @tc.type: FUNC
 * @tc.require: I5K6KF
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeTest005, testing::ext::TestSize.Level1)
{
    HILOG_INFO("Threadsafe_Test_0500 start");
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;
    g_callSuccessCountJS = 0;
    g_callSuccessCount = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_mainTid = gettid();
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;

    auto status = napi_create_threadsafe_function(env,
                                                  nullptr,
                                                  nullptr,
                                                  resourceName,
                                                  0,
                                                  2,
                                                  &g_finalData,
                                                  TsFuncFinalTotal,
                                                  &g_jsData,
                                                  TsFuncCallJsTwo,
                                                  &tsFunc);
    EXPECT_EQ(status, napi_ok);
    int threadCount = THREAD_COUNT_FOUR;
    acquireFlag = false;

    for (int i = 0; i < threadCount; i++) {
        if (uv_thread_create(&g_uvThreadTest6, TsFuncDataSourceThreadCountTotal, tsFunc) != 0) {
            HILOG_ERROR("Failed to create uv thread!");
        }
    }

    usleep(200 * 1000);
    EXPECT_EQ(g_callSuccessCount, SUCCESS_COUNT_JS_FOUR);
    HILOG_INFO("Threadsafe_Test_0500 end");
}

/**
 * @tc.name: ThreadsafeTest006
 * @tc.desc: Test initial_thread_count not enough but acquire.
 * @tc.type: FUNC
 * @tc.require: I5K6KF
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeTest006, testing::ext::TestSize.Level1)
{
    HILOG_INFO("Threadsafe_Test_0600 start");
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;
    g_callSuccessCount = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_mainTid = gettid();
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;

    auto status = napi_create_threadsafe_function(env,
                                                  nullptr,
                                                  nullptr,
                                                  resourceName,
                                                  0,
                                                  1,
                                                  &g_finalData,
                                                  TsFuncFinalTotalFour,
                                                  &g_jsData,
                                                  TsFuncCallJsFour,
                                                  &tsFunc);
    EXPECT_EQ(status, napi_ok);
    int threadCount = THREAD_COUNT_FOUR;
    acquireFlag = true;

    for (int i = 0; i < threadCount; i++) {
        if (uv_thread_create(&g_uvThreadTest7, TsFuncDataSourceThreadCountTotal, tsFunc) != 0) {
            HILOG_ERROR("Failed to create uv thread!");
        }
    }

    usleep(200 * 1000);
    EXPECT_EQ(g_callSuccessCount, SUCCESS_COUNT_JS_FOUR);
    HILOG_INFO("Threadsafe_Test_0600 end");
}

/**
 * @tc.name: ThreadsafeTest007
 * @tc.desc: Test napi_ref_threadsafe_function.
 * @tc.type: FUNC
 * @tc.require: I5K6KF
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeTest007, testing::ext::TestSize.Level1)
{
    HILOG_INFO("Threadsafe_Test_0700 start");

    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;

    auto status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName,
        0, 1, &g_finalData, TsFuncFinalCallback, &g_jsData, TsFuncCallJsFour, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    status = napi_ref_threadsafe_function(env, tsFunc);
    EXPECT_EQ(status, napi_ok);

    HILOG_INFO("Threadsafe_Test_0700 end");
}

/**
 * @tc.name: ThreadsafeTest008
 * @tc.desc: Test napi_unref_threadsafe_function.
 * @tc.type: FUNC
 * @tc.require: I5K6KF
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeTest008, testing::ext::TestSize.Level1)
{
    HILOG_INFO("Threadsafe_Test_0800 start");
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;

    auto status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName,
        0, 1, &g_finalData, TsFuncFinalCallback, &g_jsData, TsFuncCallJsFour, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    status = napi_unref_threadsafe_function(env, tsFunc);
    EXPECT_EQ(status, napi_ok);

    HILOG_INFO("Threadsafe_Test_0800 end");
}

/**
 * @tc.name: ThreadsafeTest009
 * @tc.desc: Test napi_ref_threadsafe_function and napi_unref_threadsafe_function.
 * @tc.type: FUNC
 * @tc.require: I5K6KF
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeTest009, testing::ext::TestSize.Level1)
{
    HILOG_INFO("Threadsafe_Test_0900 start");
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;

    auto status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName,
        0, 1, &g_finalData, TsFuncFinalCallback, &g_jsData, TsFuncCallJsFour, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    status = napi_unref_threadsafe_function(env, tsFunc);
    EXPECT_EQ(status, napi_ok);

    status = napi_unref_threadsafe_function(env, tsFunc);
    EXPECT_EQ(status, napi_ok);

    HILOG_INFO("Threadsafe_Test_0900 end");
}

/**
 * @tc.name: ThreadsafeTest010
 * @tc.desc: Test napi_unref_threadsafe_function and napi_release_threadsafe_function.
 * @tc.type: FUNC
 * @tc.require: I5K6KF
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeTest010, testing::ext::TestSize.Level1)
{
    HILOG_INFO("Threadsafe_Test_1000 start");
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;
    napi_threadsafe_function_release_mode abort = napi_tsfn_abort;

    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;

    auto status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName,
        0, 1, &g_finalData, TsFuncFinalCallback, &g_jsData, TsFuncCallJsFour, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    status = napi_unref_threadsafe_function(env, tsFunc);
    EXPECT_EQ(status, napi_ok);

    status = napi_release_threadsafe_function(tsFunc, abort);
    EXPECT_EQ(status, napi_ok);

    HILOG_INFO("Threadsafe_Test_1000 end");
}

/**
 * @tc.name: ThreadsafeTest011
 * @tc.desc: Test napi_ref_threadsafe_function and napi_release_threadsafe_function.
 * @tc.type: FUNC
 * @tc.require: I5K6KF
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeTest011, testing::ext::TestSize.Level1)
{
    HILOG_INFO("Threadsafe_Test_1100 start");

    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;
    napi_threadsafe_function_release_mode abort = napi_tsfn_abort;

    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;

    auto status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName,
        0, 1, &g_finalData, TsFuncFinalCallback, &g_jsData, TsFuncCallJsFour, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    status = napi_ref_threadsafe_function(env, tsFunc);
    EXPECT_EQ(status, napi_ok);

    status = napi_release_threadsafe_function(tsFunc, abort);
    EXPECT_EQ(status, napi_ok);

    HILOG_INFO("Threadsafe_Test_1100 end");
}

/**
 * @tc.name: ThreadsafeWithPriorityArgsCheckTest001
 * @tc.desc: Test napi_call_threadsafe_function_with_priority.
 * @tc.type: FUNC
 * @tc.require: I99QUH
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeWithPriorityArgsCheckTest001, testing::ext::TestSize.Level1)
{
    HILOG_INFO("ThreadsafeWithPriorityArgsCheckTest001 start");
    auto status =
        napi_call_threadsafe_function_with_priority(nullptr, nullptr, napi_priority_immediate, true);
    EXPECT_EQ(status, napi_invalid_arg);
    HILOG_INFO("ThreadsafeWithPriorityArgsCheckTest001 end");
}

/**
 * @tc.name: ThreadsafeWithPriorityArgsCheckTest002
 * @tc.desc: Test napi_call_threadsafe_function_with_priority.
 * @tc.type: FUNC
 * @tc.require: I99QUH
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeWithPriorityArgsCheckTest002, testing::ext::TestSize.Level1)
{
    HILOG_INFO("ThreadsafeWithPriorityArgsCheckTest002 start");
    napi_env env = (napi_env)engine_;
    napi_value resourceName = 0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    CallbackData *callbackData = new CallbackData();
    auto status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName,
        0, 1, callbackData, nullptr, callbackData, CallJs, &callbackData->tsfn);
    EXPECT_EQ(status, napi_ok);
    status =
        napi_call_threadsafe_function_with_priority(callbackData->tsfn, nullptr,
            static_cast<napi_task_priority>(INVALID_NAPI_THREAD_SAFE_PRIORITY), true);
    delete callbackData;
    HILOG_INFO("ThreadsafeWithPriorityArgsCheckTest002 end");
}

/**
 * @tc.name: ThreadsafeWithPriorityTest001
 * @tc.desc: Test napi_call_threadsafe_function_with_priority.
 * @tc.type: FUNC
 * @tc.require: I99QUH
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeWithPriorityTest001, testing::ext::TestSize.Level1)
{
    HILOG_INFO("ThreadsafeWithPriorityTest001 start");
    g_isTailA = true;
    g_isTailB = true;
    auto task = [test = this]() {
        test->CallThreadSafeWithSamePriorityTest(napi_priority_immediate);
    };
    EXPECT_NE(eventHandler_, nullptr);
    eventHandler_->PostTask(task);
    auto runner = eventHandler_->GetEventRunner();
    EXPECT_NE(runner, nullptr);
    runner->Run();
    HILOG_INFO("ThreadsafeWithPriorityTest001 end");
}

/**
 * @tc.name: ThreadsafeWithPriorityTest002
 * @tc.desc: Test napi_call_threadsafe_function_with_priority.
 * @tc.type: FUNC
 * @tc.require: I99QUH
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeWithPriorityTest002, testing::ext::TestSize.Level1)
{
    HILOG_INFO("ThreadsafeWithPriorityTest002 start");
    g_isTailA = false;
    g_isTailB = false;
    auto task = [test = this]() {
        test->CallThreadSafeWithSamePriorityTest(napi_priority_immediate);
    };
    EXPECT_NE(eventHandler_, nullptr);
    eventHandler_->PostTask(task);
    auto runner = eventHandler_->GetEventRunner();
    EXPECT_NE(runner, nullptr);
    runner->Run();
    HILOG_INFO("ThreadsafeWithPriorityTest002 end");
}

/**
 * @tc.name: ThreadsafeWithPriorityTest003
 * @tc.desc: Test napi_call_threadsafe_function_with_priority.
 * @tc.type: FUNC
 * @tc.require: I99QUH
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeWithPriorityTest003, testing::ext::TestSize.Level1)
{
    HILOG_INFO("ThreadsafeWithPriorityTest003 start");
    g_isTailA = true;
    g_isTailB = false;
    auto task = [test = this]() {
        test->CallThreadSafeWithSamePriorityTest(napi_priority_immediate);
    };
    EXPECT_NE(eventHandler_, nullptr);
    eventHandler_->PostTask(task);
    auto runner = eventHandler_->GetEventRunner();
    EXPECT_NE(runner, nullptr);
    runner->Run();
    HILOG_INFO("ThreadsafeWithPriorityTest003 end");
}

/**
 * @tc.name: ThreadsafeWithPriorityTest004
 * @tc.desc: Test napi_call_threadsafe_function_with_priority.
 * @tc.type: FUNC
 * @tc.require: I99QUH
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeWithPriorityTest004, testing::ext::TestSize.Level1)
{
    HILOG_INFO("ThreadsafeWithPriorityTest004 start");
    g_isTailA = true;
    g_isTailB = true;
    auto task = [test = this]() {
        test->CallThreadSafeWithSamePriorityTest(napi_priority_high);
    };
    EXPECT_NE(eventHandler_, nullptr);
    eventHandler_->PostTask(task);
    auto runner = eventHandler_->GetEventRunner();
    EXPECT_NE(runner, nullptr);
    runner->Run();
    HILOG_INFO("ThreadsafeWithPriorityTest004 end");
}

/**
 * @tc.name: ThreadsafeWithPriorityTest005
 * @tc.desc: Test napi_call_threadsafe_function_with_priority.
 * @tc.type: FUNC
 * @tc.require: I99QUH
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeWithPriorityTest005, testing::ext::TestSize.Level1)
{
    HILOG_INFO("ThreadsafeWithPriorityTest005 start");
    g_isTailA = false;
    g_isTailB = false;
    auto task = [test = this]() {
        test->CallThreadSafeWithSamePriorityTest(napi_priority_high);
    };
    EXPECT_NE(eventHandler_, nullptr);
    eventHandler_->PostTask(task);
    auto runner = eventHandler_->GetEventRunner();
    EXPECT_NE(runner, nullptr);
    runner->Run();
    HILOG_INFO("ThreadsafeWithPriorityTest005 end");
}

/**
 * @tc.name: ThreadsafeWithPriorityTest006
 * @tc.desc: Test napi_call_threadsafe_function_with_priority.
 * @tc.type: FUNC
 * @tc.require: I99QUH
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeWithPriorityTest006, testing::ext::TestSize.Level1)
{
    HILOG_INFO("ThreadsafeWithPriorityTest006 start");
    g_isTailA = true;
    g_isTailB = false;
    auto task = [test = this]() {
        test->CallThreadSafeWithSamePriorityTest(napi_priority_high);
    };
    EXPECT_NE(eventHandler_, nullptr);
    eventHandler_->PostTask(task);
    auto runner = eventHandler_->GetEventRunner();
    EXPECT_NE(runner, nullptr);
    runner->Run();
    HILOG_INFO("ThreadsafeWithPriorityTest006 end");
}

/**
 * @tc.name: ThreadsafeWithPriorityTest007
 * @tc.desc: Test napi_call_threadsafe_function_with_priority.
 * @tc.type: FUNC
 * @tc.require: I99QUH
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeWithPriorityTest007, testing::ext::TestSize.Level1)
{
    HILOG_INFO("ThreadsafeWithPriorityTest007 start");
    g_isTailA = true;
    g_isTailB = true;
    auto task = [test = this]() {
        test->CallThreadSafeWithSamePriorityTest(napi_priority_low);
    };
    EXPECT_NE(eventHandler_, nullptr);
    eventHandler_->PostTask(task);
    auto runner = eventHandler_->GetEventRunner();
    EXPECT_NE(runner, nullptr);
    runner->Run();
    HILOG_INFO("ThreadsafeWithPriorityTest007 end");
}

/**
 * @tc.name: ThreadsafeWithPriorityTest008
 * @tc.desc: Test napi_call_threadsafe_function_with_priority.
 * @tc.type: FUNC
 * @tc.require: I99QUH
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeWithPriorityTest008, testing::ext::TestSize.Level1)
{
    HILOG_INFO("ThreadsafeWithPriorityTest008 start");
    g_isTailA = false;
    g_isTailB = false;
    auto task = [test = this]() {
        test->CallThreadSafeWithSamePriorityTest(napi_priority_low);
    };
    EXPECT_NE(eventHandler_, nullptr);
    eventHandler_->PostTask(task);
    auto runner = eventHandler_->GetEventRunner();
    EXPECT_NE(runner, nullptr);
    runner->Run();
    HILOG_INFO("ThreadsafeWithPriorityTest008 end");
}

/**
 * @tc.name: ThreadsafeWithPriorityTest009
 * @tc.desc: Test napi_call_threadsafe_function_with_priority.
 * @tc.type: FUNC
 * @tc.require: I99QUH
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeWithPriorityTest009, testing::ext::TestSize.Level1)
{
    HILOG_INFO("ThreadsafeWithPriorityTest009 start");
    g_isTailA = true;
    g_isTailB = false;
    auto task = [test = this]() {
        test->CallThreadSafeWithSamePriorityTest(napi_priority_low);
    };
    EXPECT_NE(eventHandler_, nullptr);
    eventHandler_->PostTask(task);
    auto runner = eventHandler_->GetEventRunner();
    EXPECT_NE(runner, nullptr);
    runner->Run();
    HILOG_INFO("ThreadsafeWithPriorityTest009 end");
}


/**
 * @tc.name: ThreadsafeWithPriorityTest010
 * @tc.desc: Test napi_call_threadsafe_function_with_priority.
 * @tc.type: FUNC
 * @tc.require: I99QUH
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeWithPriorityTest010, testing::ext::TestSize.Level1)
{
    HILOG_INFO("ThreadsafeWithPriorityTest010 start");
    g_isTailA = true;
    g_isTailB = true;
    auto task = [test = this]() {
        test->CallThreadSafeWithSamePriorityTest(napi_priority_idle);
    };
    EXPECT_NE(eventHandler_, nullptr);
    eventHandler_->PostTask(task);
    auto runner = eventHandler_->GetEventRunner();
    EXPECT_NE(runner, nullptr);
    runner->Run();
    HILOG_INFO("ThreadsafeWithPriorityTest010 end");
}

/**
 * @tc.name: ThreadsafeWithPriorityTest011
 * @tc.desc: Test napi_call_threadsafe_function_with_priority.
 * @tc.type: FUNC
 * @tc.require: I99QUH
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeWithPriorityTest011, testing::ext::TestSize.Level1)
{
    HILOG_INFO("ThreadsafeWithPriorityTest005 start");
    g_isTailA = false;
    g_isTailB = false;
    auto task = [test = this]() {
        test->CallThreadSafeWithSamePriorityTest(napi_priority_idle);
    };
    EXPECT_NE(eventHandler_, nullptr);
    eventHandler_->PostTask(task);
    auto runner = eventHandler_->GetEventRunner();
    EXPECT_NE(runner, nullptr);
    runner->Run();
    HILOG_INFO("ThreadsafeWithPriorityTest011 end");
}

/**
 * @tc.name: ThreadsafeWithPriorityTest012
 * @tc.desc: Test napi_call_threadsafe_function_with_priority.
 * @tc.type: FUNC
 * @tc.require: I99QUH
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeWithPriorityTest012, testing::ext::TestSize.Level1)
{
    HILOG_INFO("ThreadsafeWithPriorityTest012 start");
    g_isTailA = true;
    g_isTailB = false;
    auto task = [test = this]() {
        test->CallThreadSafeWithSamePriorityTest(napi_priority_idle);
    };
    EXPECT_NE(eventHandler_, nullptr);
    eventHandler_->PostTask(task);
    auto runner = eventHandler_->GetEventRunner();
    EXPECT_NE(runner, nullptr);
    runner->Run();
    HILOG_INFO("ThreadsafeWithPriorityTest012 end");
}

/**
 * @tc.name: ThreadsafeWithPriorityTest013
 * @tc.desc: Test napi_call_threadsafe_function_with_priority.
 * @tc.type: FUNC
 * @tc.require: I99QUH
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeWithPriorityTest013, testing::ext::TestSize.Level1)
{
    HILOG_INFO("ThreadsafeWithPriorityTest013 start");
    auto task = [test = this]() {
        test->CallThreadSafeWithDiffPriorityTest();
    };
    EXPECT_NE(eventHandler_, nullptr);
    eventHandler_->PostTask(task);
    auto runner = eventHandler_->GetEventRunner();
    EXPECT_NE(runner, nullptr);
    runner->Run();
    HILOG_INFO("ThreadsafeWithPriorityTest013 end");
}

/**
 * @tc.name: ThreadsafeWithPriorityTest014
 * @tc.desc: Test napi_call_threadsafe_function_with_priority.
 * @tc.type: FUNC
 * @tc.require: I99QUH
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeWithPriorityTest014, testing::ext::TestSize.Level1)
{
    HILOG_INFO("ThreadsafeWithPriorityTest014 start");
    auto task = [test = this]() {
        test->CallThreadSafeWithDiffPriorityMultipleThreadTest();
    };
    EXPECT_NE(eventHandler_, nullptr);
    eventHandler_->PostTask(task);
    auto runner = eventHandler_->GetEventRunner();
    EXPECT_NE(runner, nullptr);
    runner->Run();
    HILOG_INFO("ThreadsafeWithPriorityTest014 end");
}

/**
 * @tc.name: ThreadsafeTest012
 * @tc.desc: Test LoadModule Func, call napi_call_threadsafe_function in callback.
 * @tc.type: FUNC
 * @tc.require: I5K6KF
 */
HWTEST_F(NapiThreadsafeTest, ThreadsafeTest012, testing::ext::TestSize.Level1)
{
    HILOG_INFO("Threadsafe_Test_1200 start");
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_mainTid = gettid();
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;

    auto status = napi_create_threadsafe_function(env,
                                                  nullptr,
                                                  nullptr,
                                                  resourceName,
                                                  0,
                                                  1,
                                                  &g_finalData,
                                                  TsFuncFinal,
                                                  &g_jsData,
                                                  TsFuncCallJsWithNewCall,
                                                  &tsFunc);
    EXPECT_EQ(status, napi_ok);

    if (uv_thread_create(
            &g_uvThread,
            [](void *data) {
                napi_threadsafe_function func = (napi_threadsafe_function)data;
                auto status = napi_call_threadsafe_function(func, data, napi_tsfn_nonblocking);
                EXPECT_EQ(status, napi_ok);
            },
            tsFunc) != 0) {
        HILOG_ERROR("Failed to create uv thread!");
    }

    HILOG_INFO("Threadsafe_Test_1200 end");
}