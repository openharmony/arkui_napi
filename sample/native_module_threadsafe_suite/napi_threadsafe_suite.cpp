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

#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <cstring>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

// 匿名命名空间，用于定义所有的全局静态常量和相关的结构体
namespace {
constexpr size_t THREAD_COUNT_ONE = 1;
constexpr size_t THREAD_COUNT_TWO = 2;
constexpr size_t THREAD_COUNT_FOUR = 4;

constexpr size_t MAX_QUEUE_SIZE = 10;
constexpr size_t MAX_QUEUE_SIZE_ONE = 1;

constexpr int32_t DATA_VAL_HUNDRED = 100;
constexpr int32_t DATA_VAL_TEN = 10;
constexpr int32_t DATA_VAL_NINE = 9;

constexpr int32_t LOOP_LIMIT_FIVE = 5;
constexpr int32_t LOOP_LIMIT_TEN = 10;
constexpr int32_t LOOP_LIMIT_FIFTY = 50;

constexpr size_t BUFFER_SIZE_HUNDRED = 100;

constexpr uint32_t SLEEP_DURATION_MS_TEN = 10;
constexpr uint32_t SLEEP_DURATION_MS_FIFTY = 50;

// 数据结构体用于多线程传递
struct TestData {
    int32_t value = 0;
    char message[BUFFER_SIZE_HUNDRED] = {0};
};

// 正常线程上下文
struct NormalThreadCtx {
    napi_threadsafe_function tsfn = nullptr;
    int32_t count = 0;
};

// Promise并发上下文
struct PromiseContext {
    napi_deferred deferred = nullptr;
    napi_threadsafe_function tsfn = nullptr;
    std::atomic<int32_t> activeThreads{0};
};

// 非阻塞场景上下文
struct NonBlockingContext {
    napi_threadsafe_function tsfn = nullptr;
    napi_deferred deferred = nullptr;
    std::atomic<bool> hasQueueFull{false};
    std::atomic<bool> isFinished{false};
};

// 阻塞场景上下文
struct BlockingContext {
    napi_threadsafe_function tsfn = nullptr;
    napi_deferred deferred = nullptr;
    std::atomic<int32_t> callSuccessCount{0};
};

// 多工作线程上下文
struct MultiThreadCtx {
    napi_threadsafe_function tsfn = nullptr;
    napi_deferred deferred = nullptr;
    std::atomic<int32_t> completedCount{0};
};

// 数据通信上下文
struct DataPassCtx {
    napi_threadsafe_function tsfn = nullptr;
};
}

// 辅助函数：将布尔属性设置到JS对象中
static bool SetNamedPropertyBool(napi_env env, napi_value object, const char* propertyName, bool value)
{
    napi_value napiValue = nullptr;
    napi_status status = napi_get_boolean(env, value, &napiValue);
    if (status != napi_ok) {
        return false;
    }
    status = napi_set_named_property(env, object, propertyName, napiValue);
    return (status == napi_ok);
}

// 辅助函数：将Int32属性设置到JS对象中
static bool SetNamedPropertyInt32(napi_env env, napi_value object, const char* propertyName, int32_t value)
{
    napi_value napiValue = nullptr;
    napi_status status = napi_create_int32(env, value, &napiValue);
    if (status != napi_ok) {
        return false;
    }
    status = napi_set_named_property(env, object, propertyName, napiValue);
    return (status == napi_ok);
}

// 辅助函数：将字符串属性设置到JS对象中
static bool SetNamedPropertyString(napi_env env, napi_value object, const char* propertyName, const char* value)
{
    napi_value napiValue = nullptr;
    napi_status status = napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &napiValue);
    if (status != napi_ok) {
        return false;
    }
    status = napi_set_named_property(env, object, propertyName, napiValue);
    return (status == napi_ok);
}

// 辅助函数：创建标准的返回结果对象
static napi_value CreateReturnObject(napi_env env, bool success, const char* message)
{
    napi_value resultObj = nullptr;
    napi_status status = napi_create_object(env, &resultObj);
    if (status != napi_ok) {
        return nullptr;
    }
    if (!SetNamedPropertyBool(env, resultObj, "success", success)) {
        return nullptr;
    }
    if (!SetNamedPropertyString(env, resultObj, "message", message)) {
        return nullptr;
    }
    return resultObj;
}

// 线程安全函数JS回调的默认包装器
static void CallJsCallback(napi_env env, napi_value jsCallback, void* context, void* data)
{
    (void)context;
    if (jsCallback == nullptr) {
        return;
    }
    napi_value undefinedVal = nullptr;
    napi_status status = napi_get_undefined(env, &undefinedVal);
    if (status != napi_ok) {
        return;
    }
    napi_value jsData = nullptr;
    if (data != nullptr) {
        int32_t* intData = static_cast<int32_t*>(data);
        status = napi_create_int32(env, *intData, &jsData);
        delete intData;
    } else {
        status = napi_get_undefined(env, &jsData);
    }
    if (status != napi_ok) {
        return;
    }
    napi_value returnVal = nullptr;
    napi_value args[] = { jsData };
    constexpr size_t argCount = sizeof(args) / sizeof(args[0]);
    status = napi_call_function(
        env,
        undefinedVal,
        jsCallback,
        argCount,
        args,
        &returnVal);
    (void)status;
}

// 线程安全函数JS回调：支持复杂结构体
static void CallJsCallbackWithStruct(napi_env env, napi_value jsCallback, void* context, void* data)
{
    (void)context;
    if (jsCallback == nullptr || data == nullptr) {
        return;
    }
    TestData* testData = static_cast<TestData*>(data);
    napi_value undefinedVal = nullptr;
    napi_status status = napi_get_undefined(env, &undefinedVal);
    if (status != napi_ok) {
        delete testData;
        return;
    }
    napi_value jsObj = nullptr;
    status = napi_create_object(env, &jsObj);
    if (status != napi_ok) {
        delete testData;
        return;
    }
    if (!SetNamedPropertyInt32(env, jsObj, "value", testData->value) ||
        !SetNamedPropertyString(env, jsObj, "message", testData->message)) {
        delete testData;
        return;
    }
    napi_value returnVal = nullptr;
    napi_value args[] = { jsObj };
    constexpr size_t argCount = sizeof(args) / sizeof(args[0]);
    status = napi_call_function(
        env,
        undefinedVal,
        jsCallback,
        argCount,
        args,
        &returnVal);
    (void)status;
    delete testData;
}

// 线程安全函数JS回调：支持Context和Data的联动校验
static void CallJsCallbackWithContext(napi_env env, napi_value jsCallback, void* context, void* data)
{
    if (jsCallback == nullptr || context == nullptr || data == nullptr) {
        return;
    }
    int32_t* ctxVal = static_cast<int32_t*>(context);
    int32_t* dataVal = static_cast<int32_t*>(data);
    napi_value undefinedVal = nullptr;
    napi_status status = napi_get_undefined(env, &undefinedVal);
    if (status != napi_ok) {
        delete dataVal;
        return;
    }
    napi_value jsObj = nullptr;
    status = napi_create_object(env, &jsObj);
    if (status != napi_ok) {
        delete dataVal;
        return;
    }
    if (!SetNamedPropertyInt32(env, jsObj, "context", *ctxVal) ||
        !SetNamedPropertyInt32(env, jsObj, "data", *dataVal)) {
        delete dataVal;
        return;
    }
    napi_value returnVal = nullptr;
    napi_value args[] = { jsObj };
    constexpr size_t argCount = sizeof(args) / sizeof(args[0]);
    status = napi_call_function(
        env,
        undefinedVal,
        jsCallback,
        argCount,
        args,
        &returnVal);
    (void)status;
    delete dataVal;
}

// 正常测试子线程的工作逻辑
static void NormalWorker(NormalThreadCtx* ctx)
{
    if (ctx == nullptr) {
        return;
    }
    for (int32_t i = 0; i < LOOP_LIMIT_FIVE; ++i) {
        int32_t* val = new int32_t(i);
        napi_status status = napi_call_threadsafe_function(
            ctx->tsfn,
            val,
            napi_tsfn_blocking);
        if (status != napi_ok) {
            delete val;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DURATION_MS_TEN));
    }
    napi_release_threadsafe_function(ctx->tsfn, napi_tsfn_release);
    delete ctx;
}

// 并发Acquire/Release子线程工作逻辑
static void ConcurrentWorker(PromiseContext* ctx)
{
    if (ctx == nullptr) {
        return;
    }
    napi_status status = napi_acquire_threadsafe_function(ctx->tsfn);
    if (status == napi_ok) {
        ctx->activeThreads.fetch_add(1);
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DURATION_MS_FIFTY));
        ctx->activeThreads.fetch_sub(1);
        napi_release_threadsafe_function(ctx->tsfn, napi_tsfn_release);
    }
}

// 非阻塞队列溢出测试工作逻辑
static void NonBlockingWorker(NonBlockingContext* ctx)
{
    if (ctx == nullptr) {
        return;
    }
    for (int32_t i = 0; i < LOOP_LIMIT_FIFTY; ++i) {
        int32_t* val = new int32_t(i);
        napi_status status = napi_call_threadsafe_function(
            ctx->tsfn,
            val,
            napi_tsfn_nonblocking);
        if (status == napi_queue_full) {
            ctx->hasQueueFull.store(true);
            delete val;
            break;
        } else if (status != napi_ok) {
            delete val;
        }
    }
    ctx->isFinished.store(true);
    napi_release_threadsafe_function(ctx->tsfn, napi_tsfn_release);
}

// 阻塞调用测试工作逻辑
static void BlockingWorker(BlockingContext* ctx)
{
    if (ctx == nullptr) {
        return;
    }
    for (int32_t i = 0; i < LOOP_LIMIT_FIVE; ++i) {
        int32_t* val = new int32_t(i);
        napi_status status = napi_call_threadsafe_function(
            ctx->tsfn,
            val,
            napi_tsfn_blocking);
        if (status == napi_ok) {
            ctx->callSuccessCount.fetch_add(1);
        } else {
            delete val;
        }
    }
    napi_release_threadsafe_function(ctx->tsfn, napi_tsfn_release);
}

// 多线程调用测试工作逻辑
static void MultiWorker(MultiThreadCtx* ctx)
{
    if (ctx == nullptr) {
        return;
    }
    for (int32_t i = 0; i < LOOP_LIMIT_TEN; ++i) {
        int32_t* val = new int32_t(i);
        napi_status status = napi_call_threadsafe_function(
            ctx->tsfn,
            val,
            napi_tsfn_blocking);
        if (status != napi_ok) {
            delete val;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DURATION_MS_TEN));
    }
    ctx->completedCount.fetch_add(1);
    napi_release_threadsafe_function(ctx->tsfn, napi_tsfn_release);
}

// 复杂数据通信测试工作逻辑
static void DataPassWorker(DataPassCtx* ctx)
{
    if (ctx == nullptr) {
        return;
    }
    TestData* data = new TestData();
    data->value = DATA_VAL_HUNDRED;
    errno_t err = strcpy_s(data->message, sizeof(data->message), "Hello from background thread");
    if (err == 0) {
        napi_status status = napi_call_threadsafe_function(
            ctx->tsfn,
            data,
            napi_tsfn_blocking);
        if (status != napi_ok) {
            delete data;
        }
    } else {
        delete data;
    }
    napi_release_threadsafe_function(ctx->tsfn, napi_tsfn_release);
    delete ctx;
}

// 带优先级的多线程异步调用工作逻辑
static void PriorityWorker(napi_threadsafe_function tsfn, napi_task_priority priority, bool isTail)
{
    int32_t* data = new int32_t(DATA_VAL_HUNDRED);
    napi_status status = napi_call_threadsafe_function_with_priority(
        tsfn,
        data,
        priority,
        isTail);
    if (status != napi_ok) {
        delete data;
    }
    napi_release_threadsafe_function(tsfn, napi_tsfn_release);
}

// 传递空指针作为data的工作逻辑
static void NullDataWorker(napi_threadsafe_function tsfn)
{
    napi_status status = napi_call_threadsafe_function(
        tsfn,
        nullptr,
        napi_tsfn_blocking);
    (void)status;
    napi_release_threadsafe_function(tsfn, napi_tsfn_release);
}

// Context测试专用工作逻辑
static void ContextWorker(napi_threadsafe_function tsfn)
{
    int32_t* val = new int32_t(DATA_VAL_TEN);
    napi_status status = napi_call_threadsafe_function(
        tsfn,
        val,
        napi_tsfn_blocking);
    if (status != napi_ok) {
        delete val;
    }
    napi_release_threadsafe_function(tsfn, napi_tsfn_release);
}

// 释放回调解析 Promise
static void FinalizePromiseCb(napi_env env, void* finalizeData, void* hint)
{
    (void)hint;
    PromiseContext* ctx = static_cast<PromiseContext*>(finalizeData);
    if (ctx == nullptr) {
        return;
    }
    napi_value result = nullptr;
    napi_status status = napi_create_string_utf8(env, "Concurrent test done", NAPI_AUTO_LENGTH, &result);
    if (status == napi_ok) {
        napi_resolve_deferred(env, ctx->deferred, result);
    }
    delete ctx;
}

// 释放回调解析非阻塞溢出状态
static void FinalizeNonBlockingCb(napi_env env, void* finalizeData, void* hint)
{
    (void)hint;
    NonBlockingContext* ctx = static_cast<NonBlockingContext*>(finalizeData);
    if (ctx == nullptr) {
        return;
    }
    napi_value result = nullptr;
    bool success = ctx->hasQueueFull.load();
    napi_status status = napi_get_boolean(env, success, &result);
    if (status == napi_ok) {
        napi_resolve_deferred(env, ctx->deferred, result);
    }
    delete ctx;
}

// 释放回调解析阻塞状态
static void FinalizeBlockingCb(napi_env env, void* finalizeData, void* hint)
{
    (void)hint;
    BlockingContext* ctx = static_cast<BlockingContext*>(finalizeData);
    if (ctx == nullptr) {
        return;
    }
    napi_value result = nullptr;
    bool success = (ctx->callSuccessCount.load() == LOOP_LIMIT_FIVE);
    napi_status status = napi_get_boolean(env, success, &result);
    if (status == napi_ok) {
        napi_resolve_deferred(env, ctx->deferred, result);
    }
    delete ctx;
}

// 释放回调解析多工作线程状态
static void FinalizeMultiCb(napi_env env, void* finalizeData, void* hint)
{
    (void)hint;
    MultiThreadCtx* ctx = static_cast<MultiThreadCtx*>(finalizeData);
    if (ctx == nullptr) {
        return;
    }
    napi_value result = nullptr;
    bool success = (ctx->completedCount.load() == THREAD_COUNT_FOUR);
    napi_status status = napi_get_boolean(env, success, &result);
    if (status == napi_ok) {
        napi_resolve_deferred(env, ctx->deferred, result);
    }
    delete ctx;
}

// 异常退出的回调空实现
static void FinalizeCleanupCb(napi_env env, void* finalizeData, void* hint)
{
    (void)env;
    (void)finalizeData;
    (void)hint;
}

// 用例 1: 正常创建与释放线程安全方法
static napi_value TestCreateThreadSafeFunctionNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value resourceName = nullptr;
    napi_status status = napi_create_string_utf8(env, "NormalTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create string failed");
    }
    napi_value jsFunc = nullptr;
    status = napi_create_function(
        env,
        "dummy",
        NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
        nullptr,
        &jsFunc);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create function failed");
    }
    napi_threadsafe_function tsfn = nullptr;
    status = napi_create_threadsafe_function(
        env,
        jsFunc,
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_ONE,
        nullptr,
        nullptr,
        nullptr,
        CallJsCallback,
        &tsfn);
    if (status != napi_ok || tsfn == nullptr) {
        return CreateReturnObject(env, false, "Create threadsafe function failed");
    }
    status = napi_release_threadsafe_function(tsfn, napi_tsfn_release);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Release threadsafe function failed");
    }
    return CreateReturnObject(env, true, "Success");
}

// 用例 2: 创建线程安全方法传入空 env 时的行为
static napi_value TestCreateThreadSafeFunctionNullEnv(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value resourceName = nullptr;
    napi_status status = napi_create_string_utf8(env, "NullEnvTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create resource name failed");
    }
    napi_value jsFunc = nullptr;
    status = napi_create_function(
        env,
        "dummy",
        NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
        nullptr,
        &jsFunc);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create function failed");
    }
    napi_threadsafe_function tsfn = nullptr;
    napi_status errStatus = napi_create_threadsafe_function(
        nullptr,
        jsFunc,
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_ONE,
        nullptr,
        nullptr,
        nullptr,
        CallJsCallback,
        &tsfn);
    if (errStatus == napi_ok) {
        if (tsfn != nullptr) {
            napi_release_threadsafe_function(tsfn, napi_tsfn_release);
        }
        return CreateReturnObject(env, false, "Should return invalid arg when env is null");
    }
    return CreateReturnObject(env, true, "Success");
}

// 用例 3: 回调方法（js_func）为空时的容错
static napi_value TestCreateThreadSafeFunctionNullFunc(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value resourceName = nullptr;
    napi_status status = napi_create_string_utf8(env, "NullFuncTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create resource name failed");
    }
    napi_threadsafe_function tsfn = nullptr;
    status = napi_create_threadsafe_function(
        env,
        nullptr,
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_ONE,
        nullptr,
        nullptr,
        nullptr,
        CallJsCallback,
        &tsfn);
    if (status != napi_ok || tsfn == nullptr) {
        return CreateReturnObject(env, false, "Failed to create TSFN with null function");
    }
    status = napi_release_threadsafe_function(tsfn, napi_tsfn_release);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Release failed");
    }
    return CreateReturnObject(env, true, "Success");
}

// 用例 4: 资源标识名为空时的错误处理
static napi_value TestCreateThreadSafeFunctionNullResourceName(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsFunc = nullptr;
    napi_status status = napi_create_function(
        env,
        "dummy",
        NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
        nullptr,
        &jsFunc);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create function failed");
    }
    napi_threadsafe_function tsfn = nullptr;
    napi_status errStatus = napi_create_threadsafe_function(
        env,
        jsFunc,
        nullptr,
        nullptr,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_ONE,
        nullptr,
        nullptr,
        nullptr,
        CallJsCallback,
        &tsfn);
    if (errStatus == napi_ok) {
        if (tsfn != nullptr) {
            napi_release_threadsafe_function(tsfn, napi_tsfn_release);
        }
        return CreateReturnObject(env, false, "Should fail when resource name is null");
    }
    return CreateReturnObject(env, true, "Success");
}

// 用例 5: 接收指针为空时的错误处理
static napi_value TestCreateThreadSafeFunctionNullResult(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value resourceName = nullptr;
    napi_status status = napi_create_string_utf8(env, "NullResultTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create resource name failed");
    }
    napi_value jsFunc = nullptr;
    status = napi_create_function(
        env,
        "dummy",
        NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
        nullptr,
        &jsFunc);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create function failed");
    }
    status = napi_create_threadsafe_function(
        env,
        jsFunc,
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_ONE,
        nullptr,
        nullptr,
        nullptr,
        CallJsCallback,
        nullptr);
    if (status == napi_ok) {
        return CreateReturnObject(env, false, "Should fail when result pointer is null");
    }
    return CreateReturnObject(env, true, "Success");
}

// 用例 6: 多线程任务调度与主线程与后台工作线程的数据通信生命周期
static napi_value TestCallThreadsafeNormal(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        return CreateReturnObject(env, false, "Get callback info failed");
    }
    napi_value resourceName = nullptr;
    status = napi_create_string_utf8(env, "NormalCallTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create string failed");
    }
    napi_threadsafe_function tsfn = nullptr;
    status = napi_create_threadsafe_function(
        env,
        argv[0],
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_ONE,
        nullptr,
        nullptr,
        nullptr,
        CallJsCallback,
        &tsfn);
    if (status != napi_ok || tsfn == nullptr) {
        return CreateReturnObject(env, false, "Create tsfn failed");
    }
    NormalThreadCtx* ctx = new NormalThreadCtx();
    ctx->tsfn = tsfn;
    ctx->count = LOOP_LIMIT_FIVE;
    std::thread t(NormalWorker, ctx);
    t.detach();
    return CreateReturnObject(env, true, "Success");
}

// 用例 7: 正常的获取计数和释放操作流程
static napi_value TestAcquireReleaseNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value resourceName = nullptr;
    napi_status status = napi_create_string_utf8(env, "AcquireReleaseTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create string failed");
    }
    napi_value jsFunc = nullptr;
    status = napi_create_function(
        env,
        "dummy",
        NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
        nullptr,
        &jsFunc);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create function failed");
    }
    napi_threadsafe_function tsfn = nullptr;
    status = napi_create_threadsafe_function(
        env,
        jsFunc,
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_ONE,
        nullptr,
        nullptr,
        nullptr,
        CallJsCallback,
        &tsfn);
    if (status != napi_ok || tsfn == nullptr) {
        return CreateReturnObject(env, false, "Create tsfn failed");
    }
    status = napi_acquire_threadsafe_function(tsfn);
    if (status != napi_ok) {
        napi_release_threadsafe_function(tsfn, napi_tsfn_release);
        return CreateReturnObject(env, false, "Acquire failed");
    }
    status = napi_release_threadsafe_function(tsfn, napi_tsfn_release);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "First release failed");
    }
    status = napi_release_threadsafe_function(tsfn, napi_tsfn_release);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Second release failed");
    }
    return CreateReturnObject(env, true, "Success");
}

// 用例 8: 传入空 tsfn 进行获取计数时的安全防护
static napi_value TestAcquireNullTsfn(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_status status = napi_acquire_threadsafe_function(nullptr);
    if (status == napi_ok) {
        return CreateReturnObject(env, false, "Acquire with null tsfn should fail");
    }
    return CreateReturnObject(env, true, "Success");
}

// 用例 9: 传入空 tsfn 进行释放时的安全防护
static napi_value TestReleaseNullTsfn(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_status status = napi_release_threadsafe_function(nullptr, napi_tsfn_release);
    if (status == napi_ok) {
        return CreateReturnObject(env, false, "Release with null tsfn should fail");
    }
    return CreateReturnObject(env, true, "Success");
}

// 用例 10: 传入空 tsfn 发起异步调用时的安全防护
static napi_value TestCallThreadsafeNullTsfn(napi_env env, napi_callback_info info)
{
    (void)info;
    int32_t val = DATA_VAL_HUNDRED;
    napi_status status = napi_call_threadsafe_function(nullptr, &val, napi_tsfn_nonblocking);
    if (status == napi_ok) {
        return CreateReturnObject(env, false, "Call with null tsfn should fail");
    }
    return CreateReturnObject(env, true, "Success");
}

// 用例 11: 多个工作线程并发获取/释放计数的正确性验证
static napi_value TestConcurrentAcquireRelease(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value promise = nullptr;
    napi_deferred deferred = nullptr;
    napi_status status = napi_create_promise(env, &deferred, &promise);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_value resourceName = nullptr;
    status = napi_create_string_utf8(env, "ConcurrentTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_value jsFunc = nullptr;
    status = napi_create_function(
        env,
        "dummy",
        NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
        nullptr,
        &jsFunc);
    if (status != napi_ok) {
        return nullptr;
    }
    PromiseContext* ctx = new PromiseContext();
    ctx->deferred = deferred;
    status = napi_create_threadsafe_function(
        env,
        jsFunc,
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_TWO,
        ctx,
        FinalizePromiseCb,
        nullptr,
        CallJsCallback,
        &(ctx->tsfn));
    if (status != napi_ok) {
        delete ctx;
        return nullptr;
    }
    std::thread t1(ConcurrentWorker, ctx);
    std::thread t2(ConcurrentWorker, ctx);
    t1.detach();
    t2.detach();
    napi_release_threadsafe_function(ctx->tsfn, napi_tsfn_release);
    return promise;
}

// 用例 12: 紧急优先级立即异步调用
static napi_value TestCallWithPriorityImmediate(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        return CreateReturnObject(env, false, "Get callback info failed");
    }
    napi_value resourceName = nullptr;
    status = napi_create_string_utf8(env, "PriorityImmediateTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create string failed");
    }
    napi_threadsafe_function tsfn = nullptr;
    status = napi_create_threadsafe_function(
        env,
        argv[0],
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_ONE,
        nullptr,
        nullptr,
        nullptr,
        CallJsCallback,
        &tsfn);
    if (status != napi_ok || tsfn == nullptr) {
        return CreateReturnObject(env, false, "Create tsfn failed");
    }
    status = napi_acquire_threadsafe_function(tsfn);
    if (status != napi_ok) {
        napi_release_threadsafe_function(tsfn, napi_tsfn_release);
        return CreateReturnObject(env, false, "Acquire failed");
    }
    std::thread t(PriorityWorker, tsfn, napi_priority_immediate, false);
    t.detach();
    napi_release_threadsafe_function(tsfn, napi_tsfn_release);
    return CreateReturnObject(env, true, "Success");
}

// 用例 13: 高优先级异步调用
static napi_value TestCallWithPriorityHigh(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        return CreateReturnObject(env, false, "Get callback info failed");
    }
    napi_value resourceName = nullptr;
    status = napi_create_string_utf8(env, "PriorityHighTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create string failed");
    }
    napi_threadsafe_function tsfn = nullptr;
    status = napi_create_threadsafe_function(
        env,
        argv[0],
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_ONE,
        nullptr,
        nullptr,
        nullptr,
        CallJsCallback,
        &tsfn);
    if (status != napi_ok || tsfn == nullptr) {
        return CreateReturnObject(env, false, "Create tsfn failed");
    }
    status = napi_acquire_threadsafe_function(tsfn);
    if (status != napi_ok) {
        napi_release_threadsafe_function(tsfn, napi_tsfn_release);
        return CreateReturnObject(env, false, "Acquire failed");
    }
    std::thread t(PriorityWorker, tsfn, napi_priority_high, false);
    t.detach();
    napi_release_threadsafe_function(tsfn, napi_tsfn_release);
    return CreateReturnObject(env, true, "Success");
}

// 用例 14: 低优先级异步调用
static napi_value TestCallWithPriorityLow(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        return CreateReturnObject(env, false, "Get callback info failed");
    }
    napi_value resourceName = nullptr;
    status = napi_create_string_utf8(env, "PriorityLowTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create string failed");
    }
    napi_threadsafe_function tsfn = nullptr;
    status = napi_create_threadsafe_function(
        env,
        argv[0],
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_ONE,
        nullptr,
        nullptr,
        nullptr,
        CallJsCallback,
        &tsfn);
    if (status != napi_ok || tsfn == nullptr) {
        return CreateReturnObject(env, false, "Create tsfn failed");
    }
    status = napi_acquire_threadsafe_function(tsfn);
    if (status != napi_ok) {
        napi_release_threadsafe_function(tsfn, napi_tsfn_release);
        return CreateReturnObject(env, false, "Acquire failed");
    }
    std::thread t(PriorityWorker, tsfn, napi_priority_low, false);
    t.detach();
    napi_release_threadsafe_function(tsfn, napi_tsfn_release);
    return CreateReturnObject(env, true, "Success");
}

// 用例 15: 空闲优先级异步调用
static napi_value TestCallWithPriorityIdle(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        return CreateReturnObject(env, false, "Get callback info failed");
    }
    napi_value resourceName = nullptr;
    status = napi_create_string_utf8(env, "PriorityIdleTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create string failed");
    }
    napi_threadsafe_function tsfn = nullptr;
    status = napi_create_threadsafe_function(
        env,
        argv[0],
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_ONE,
        nullptr,
        nullptr,
        nullptr,
        CallJsCallback,
        &tsfn);
    if (status != napi_ok || tsfn == nullptr) {
        return CreateReturnObject(env, false, "Create tsfn failed");
    }
    status = napi_acquire_threadsafe_function(tsfn);
    if (status != napi_ok) {
        napi_release_threadsafe_function(tsfn, napi_tsfn_release);
        return CreateReturnObject(env, false, "Acquire failed");
    }
    std::thread t(PriorityWorker, tsfn, napi_priority_idle, false);
    t.detach();
    napi_release_threadsafe_function(tsfn, napi_tsfn_release);
    return CreateReturnObject(env, true, "Success");
}

// 用例 16: 带尾调用的优先级异步调用
static napi_value TestCallWithPriorityTail(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        return CreateReturnObject(env, false, "Get callback info failed");
    }
    napi_value resourceName = nullptr;
    status = napi_create_string_utf8(env, "PriorityTailTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create string failed");
    }
    napi_threadsafe_function tsfn = nullptr;
    status = napi_create_threadsafe_function(
        env,
        argv[0],
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_ONE,
        nullptr,
        nullptr,
        nullptr,
        CallJsCallback,
        &tsfn);
    if (status != napi_ok || tsfn == nullptr) {
        return CreateReturnObject(env, false, "Create tsfn failed");
    }
    status = napi_acquire_threadsafe_function(tsfn);
    if (status != napi_ok) {
        napi_release_threadsafe_function(tsfn, napi_tsfn_release);
        return CreateReturnObject(env, false, "Acquire failed");
    }
    std::thread t(PriorityWorker, tsfn, napi_priority_high, true);
    t.detach();
    napi_release_threadsafe_function(tsfn, napi_tsfn_release);
    return CreateReturnObject(env, true, "Success");
}

// 用例 17: 无效优先级值传入时的异常分支校验
static napi_value TestCallWithPriorityInvalid(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value resourceName = nullptr;
    napi_status status = napi_create_string_utf8(env, "PriorityInvalidTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create string failed");
    }
    napi_value jsFunc = nullptr;
    status = napi_create_function(
        env,
        "dummy",
        NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
        nullptr,
        &jsFunc);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create function failed");
    }
    napi_threadsafe_function tsfn = nullptr;
    status = napi_create_threadsafe_function(
        env,
        jsFunc,
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_ONE,
        nullptr,
        nullptr,
        nullptr,
        CallJsCallback,
        &tsfn);
    if (status != napi_ok || tsfn == nullptr) {
        return CreateReturnObject(env, false, "Create tsfn failed");
    }
    napi_task_priority invalidPriority = static_cast<napi_task_priority>(INVALID_PRIORITY_VAL);
    int32_t val = DATA_VAL_HUNDRED;
    napi_status callStatus = napi_call_threadsafe_function_with_priority(
        tsfn,
        &val,
        invalidPriority,
        false);
    napi_release_threadsafe_function(tsfn, napi_tsfn_release);
    if (callStatus == napi_ok) {
        return CreateReturnObject(env, false, "Call with invalid priority should fail");
    }
    return CreateReturnObject(env, true, "Success");
}

// 用例 18: 主线程非阻塞调用遇到队列满的行为
static napi_value TestCallThreadsafeNonBlocking(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value resourceName = nullptr;
    napi_status status = napi_create_string_utf8(env, "NonBlockingFullTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create string failed");
    }
    napi_value jsFunc = nullptr;
    status = napi_create_function(
        env,
        "dummy",
        NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
        nullptr,
        &jsFunc);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create function failed");
    }
    napi_threadsafe_function tsfn = nullptr;
    status = napi_create_threadsafe_function(
        env,
        jsFunc,
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE_ONE,
        THREAD_COUNT_ONE,
        nullptr,
        nullptr,
        nullptr,
        CallJsCallback,
        &tsfn);
    if (status != napi_ok || tsfn == nullptr) {
        return CreateReturnObject(env, false, "Create tsfn failed");
    }
    int32_t val1 = DATA_VAL_HUNDRED;
    int32_t val2 = DATA_VAL_HUNDRED;
    napi_status status1 = napi_call_threadsafe_function(tsfn, &val1, napi_tsfn_nonblocking);
    napi_status status2 = napi_call_threadsafe_function(tsfn, &val2, napi_tsfn_nonblocking);
    napi_release_threadsafe_function(tsfn, napi_tsfn_release);
    if (status1 != napi_ok) {
        return CreateReturnObject(env, false, "First call failed");
    }
    (void)status2;
    return CreateReturnObject(env, true, "Success");
}

// 用例 19: 工作线程中非阻塞调用发生队列溢出时的状态测试
static napi_value TestCallThreadsafeNonBlockingThread(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value resourceName = nullptr;
    napi_status status = napi_create_string_utf8(env, "NonBlockingThreadTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_value jsFunc = nullptr;
    status = napi_create_function(
        env,
        "dummy",
        NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
        nullptr,
        &jsFunc);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    status = napi_create_promise(env, &deferred, &promise);
    if (status != napi_ok) {
        return nullptr;
    }
    NonBlockingContext* ctx = new NonBlockingContext();
    ctx->deferred = deferred;
    status = napi_create_threadsafe_function(
        env,
        jsFunc,
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE_ONE,
        THREAD_COUNT_ONE,
        ctx,
        FinalizeNonBlockingCb,
        nullptr,
        CallJsCallback,
        &(ctx->tsfn));
    if (status != napi_ok) {
        delete ctx;
        return nullptr;
    }
    std::thread t(NonBlockingWorker, ctx);
    t.detach();
    return promise;
}

// 用例 20: 阻塞队列调用验证
static napi_value TestCallThreadsafeBlocking(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value resourceName = nullptr;
    napi_status status = napi_create_string_utf8(env, "BlockingThreadTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_value jsFunc = nullptr;
    status = napi_create_function(
        env,
        "dummy",
        NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
        nullptr,
        &jsFunc);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    status = napi_create_promise(env, &deferred, &promise);
    if (status != napi_ok) {
        return nullptr;
    }
    BlockingContext* ctx = new BlockingContext();
    ctx->deferred = deferred;
    status = napi_create_threadsafe_function(
        env,
        jsFunc,
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE_ONE,
        THREAD_COUNT_ONE,
        ctx,
        FinalizeBlockingCb,
        nullptr,
        CallJsCallback,
        &(ctx->tsfn));
    if (status != napi_ok) {
        delete ctx;
        return nullptr;
    }
    std::thread t(BlockingWorker, ctx);
    t.detach();
    return promise;
}

// 用例 21: 带有 abort 模式的释放流程校验
static napi_value TestReleaseWithAbort(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value resourceName = nullptr;
    napi_status status = napi_create_string_utf8(env, "AbortTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create string failed");
    }
    napi_value jsFunc = nullptr;
    status = napi_create_function(
        env,
        "dummy",
        NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
        nullptr,
        &jsFunc);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create function failed");
    }
    napi_threadsafe_function tsfn = nullptr;
    status = napi_create_threadsafe_function(
        env,
        jsFunc,
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_ONE,
        nullptr,
        nullptr,
        nullptr,
        CallJsCallback,
        &tsfn);
    if (status != napi_ok || tsfn == nullptr) {
        return CreateReturnObject(env, false, "Create tsfn failed");
    }
    status = napi_release_threadsafe_function(tsfn, napi_tsfn_abort);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Release with abort failed");
    }
    return CreateReturnObject(env, true, "Success");
}

// 用例 22: 并发多个后台工作线程向主线程通信调度
static napi_value TestConcurrentMultipleThreads(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        return nullptr;
    }
    napi_value promise = nullptr;
    napi_deferred deferred = nullptr;
    status = napi_create_promise(env, &deferred, &promise);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_value resourceName = nullptr;
    status = napi_create_string_utf8(env, "MultiThreadTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return nullptr;
    }
    MultiThreadCtx* ctx = new MultiThreadCtx();
    ctx->deferred = deferred;
    status = napi_create_threadsafe_function(
        env,
        argv[0],
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_FOUR,
        ctx,
        FinalizeMultiCb,
        nullptr,
        CallJsCallback,
        &(ctx->tsfn));
    if (status != napi_ok) {
        delete ctx;
        return nullptr;
    }
    for (size_t i = 0; i < THREAD_COUNT_FOUR; ++i) {
        napi_acquire_threadsafe_function(ctx->tsfn);
        std::thread t(MultiWorker, ctx);
        t.detach();
    }
    napi_release_threadsafe_function(ctx->tsfn, napi_tsfn_release);
    return promise;
}

// 用例 23: 复杂结构体传递与生命周期拷贝校验
static napi_value TestThreadSafeFunctionDataPass(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        return CreateReturnObject(env, false, "Get callback info failed");
    }
    napi_value resourceName = nullptr;
    status = napi_create_string_utf8(env, "DataPassTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create string failed");
    }
    napi_threadsafe_function tsfn = nullptr;
    status = napi_create_threadsafe_function(
        env,
        argv[0],
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_ONE,
        nullptr,
        nullptr,
        nullptr,
        CallJsCallbackWithStruct,
        &tsfn);
    if (status != napi_ok || tsfn == nullptr) {
        return CreateReturnObject(env, false, "Create tsfn failed");
    }
    DataPassCtx* ctx = new DataPassCtx();
    ctx->tsfn = tsfn;
    std::thread t(DataPassWorker, ctx);
    t.detach();
    return CreateReturnObject(env, true, "Success");
}

// 用例 24: 异步调用传递空指针数据时的容错校验
static napi_value TestCallThreadsafeWithNullData(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        return CreateReturnObject(env, false, "Get callback info failed");
    }
    napi_value resourceName = nullptr;
    status = napi_create_string_utf8(env, "NullDataTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create string failed");
    }
    napi_threadsafe_function tsfn = nullptr;
    status = napi_create_threadsafe_function(
        env,
        argv[0],
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_ONE,
        nullptr,
        nullptr,
        nullptr,
        CallJsCallback,
        &tsfn);
    if (status != napi_ok || tsfn == nullptr) {
        return CreateReturnObject(env, false, "Create tsfn failed");
    }
    status = napi_acquire_threadsafe_function(tsfn);
    if (status != napi_ok) {
        napi_release_threadsafe_function(tsfn, napi_tsfn_release);
        return CreateReturnObject(env, false, "Acquire failed");
    }
    std::thread t(NullDataWorker, tsfn);
    t.detach();
    napi_release_threadsafe_function(tsfn, napi_tsfn_release);
    return CreateReturnObject(env, true, "Success");
}

// 用例 25: 队列限制为零（即无限制）时的调度行为
static napi_value TestCreateThreadsafeFunctionZeroQueue(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value resourceName = nullptr;
    napi_status status = napi_create_string_utf8(env, "ZeroQueueTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create string failed");
    }
    napi_value jsFunc = nullptr;
    status = napi_create_function(
        env,
        "dummy",
        NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
        nullptr,
        &jsFunc);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create function failed");
    }
    napi_threadsafe_function tsfn = nullptr;
    status = napi_create_threadsafe_function(
        env,
        jsFunc,
        nullptr,
        resourceName,
        0,
        THREAD_COUNT_ONE,
        nullptr,
        nullptr,
        nullptr,
        CallJsCallback,
        &tsfn);
    if (status != napi_ok || tsfn == nullptr) {
        return CreateReturnObject(env, false, "Create tsfn with zero queue failed");
    }
    status = napi_release_threadsafe_function(tsfn, napi_tsfn_release);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Release failed");
    }
    return CreateReturnObject(env, true, "Success");
}

// 用例 26: 上下文参数传递与 JS 回调解析校验
static napi_value TestCreateThreadsafeWithContext(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        return CreateReturnObject(env, false, "Get callback info failed");
    }
    napi_value resourceName = nullptr;
    status = napi_create_string_utf8(env, "ContextTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create string failed");
    }
    static int32_t globalContext = DATA_VAL_HUNDRED;
    napi_threadsafe_function tsfn = nullptr;
    status = napi_create_threadsafe_function(
        env,
        argv[0],
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_ONE,
        &globalContext,
        nullptr,
        nullptr,
        CallJsCallbackWithContext,
        &tsfn);
    if (status != napi_ok || tsfn == nullptr) {
        return CreateReturnObject(env, false, "Create tsfn failed");
    }
    status = napi_acquire_threadsafe_function(tsfn);
    if (status != napi_ok) {
        napi_release_threadsafe_function(tsfn, napi_tsfn_release);
        return CreateReturnObject(env, false, "Acquire failed");
    }
    std::thread t(ContextWorker, tsfn);
    t.detach();
    napi_release_threadsafe_function(tsfn, napi_tsfn_release);
    return CreateReturnObject(env, true, "Success");
}

// 用例 27: 从主线程直接同步发起异步调用
static napi_value TestCallThreadsafeFromMainThread(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        return CreateReturnObject(env, false, "Get callback info failed");
    }
    napi_value resourceName = nullptr;
    status = napi_create_string_utf8(env, "MainThreadCallTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create string failed");
    }
    napi_threadsafe_function tsfn = nullptr;
    status = napi_create_threadsafe_function(
        env,
        argv[0],
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_ONE,
        nullptr,
        nullptr,
        nullptr,
        CallJsCallback,
        &tsfn);
    if (status != napi_ok || tsfn == nullptr) {
        return CreateReturnObject(env, false, "Create tsfn failed");
    }
    int32_t* val = new int32_t(DATA_VAL_HUNDRED);
    status = napi_call_threadsafe_function(tsfn, val, napi_tsfn_nonblocking);
    if (status != napi_ok) {
        delete val;
        napi_release_threadsafe_function(tsfn, napi_tsfn_release);
        return CreateReturnObject(env, false, "Call failed");
    }
    status = napi_release_threadsafe_function(tsfn, napi_tsfn_release);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Release failed");
    }
    return CreateReturnObject(env, true, "Success");
}

static napi_value CreateDummyFunction(napi_env env)
{
    napi_value jsFunc = nullptr;
    napi_status status = napi_create_function(
        env,
        "dummy",
        NAPI_AUTO_LENGTH,
        [](napi_env /* env */, napi_callback_info /* info */) -> napi_value { return nullptr; },
        nullptr,
        &jsFunc);
    return (status == napi_ok) ? jsFunc : nullptr;
}

// 用例 28: 连续多次 acquire/release 计数验证行为
static napi_value TestAcquireReleaseMultiTimes(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value resourceName = nullptr;
    napi_status status = napi_create_string_utf8(env, "MultiAcquireReleaseTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create string failed");
    }
    napi_value jsFunc = CreateDummyFunction(env);
    if (jsFunc == nullptr) {
        return CreateReturnObject(env, false, "Create dummy function failed");
    }
    napi_threadsafe_function tsfn = nullptr;
    status = napi_create_threadsafe_function(
        env,
        jsFunc,
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_ONE,
        nullptr,
        nullptr,
        nullptr,
        CallJsCallback,
        &tsfn);
    if (status != napi_ok || tsfn == nullptr) {
        return CreateReturnObject(env, false, "Create tsfn failed");
    }
    for (int32_t i = 0; i < LOOP_LIMIT_TEN; ++i) {
        status = napi_acquire_threadsafe_function(tsfn);
        if (status != napi_ok) {
            napi_release_threadsafe_function(tsfn, napi_tsfn_release);
            return CreateReturnObject(env, false, "Acquire failed in loop");
        }
    }
    for (int32_t i = 0; i < LOOP_LIMIT_TEN; ++i) {
        status = napi_release_threadsafe_function(tsfn, napi_tsfn_release);
        if (status != napi_ok) {
            return CreateReturnObject(env, false, "Release failed in loop");
        }
    }
    status = napi_release_threadsafe_function(tsfn, napi_tsfn_release);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Final release failed");
    }
    return CreateReturnObject(env, true, "Success");
}

// 用例 29: 在未释放 (release) 的情况下，异常退出模块生命周期
static napi_value TestEnvCleanupWithoutRelease(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value resourceName = nullptr;
    napi_status status = napi_create_string_utf8(env, "CleanupTSFN", NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create string failed");
    }
    napi_value jsFunc = nullptr;
    status = napi_create_function(
        env,
        "dummy",
        NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; },
        nullptr,
        &jsFunc);
    if (status != napi_ok) {
        return CreateReturnObject(env, false, "Create function failed");
    }
    napi_threadsafe_function tsfn = nullptr;
    status = napi_create_threadsafe_function(
        env,
        jsFunc,
        nullptr,
        resourceName,
        MAX_QUEUE_SIZE,
        THREAD_COUNT_ONE,
        nullptr,
        FinalizeCleanupCb,
        nullptr,
        CallJsCallback,
        &tsfn);
    if (status != napi_ok || tsfn == nullptr) {
        return CreateReturnObject(env, false, "Create tsfn failed");
    }
    return CreateReturnObject(env, true, "Success");
}

// 模块初始化导出与用例注册
static const napi_property_descriptor THREADSAFE_TEST_DESCRIPTORS[] = {
    DECLARE_NAPI_FUNCTION("testCreateThreadSafeFunctionNormal",
        TestCreateThreadSafeFunctionNormal),
    DECLARE_NAPI_FUNCTION("testCreateThreadSafeFunctionNullEnv",
        TestCreateThreadSafeFunctionNullEnv),
    DECLARE_NAPI_FUNCTION("testCreateThreadSafeFunctionNullFunc",
        TestCreateThreadSafeFunctionNullFunc),
    DECLARE_NAPI_FUNCTION("testCreateThreadSafeFunctionNullResourceName",
        TestCreateThreadSafeFunctionNullResourceName),
    DECLARE_NAPI_FUNCTION("testCreateThreadSafeFunctionNullResult",
        TestCreateThreadSafeFunctionNullResult),
    DECLARE_NAPI_FUNCTION("testCallThreadsafeNormal",
        TestCallThreadsafeNormal),
    DECLARE_NAPI_FUNCTION("testAcquireReleaseNormal",
        TestAcquireReleaseNormal),
    DECLARE_NAPI_FUNCTION("testAcquireNullTsfn",
        TestAcquireNullTsfn),
    DECLARE_NAPI_FUNCTION("testReleaseNullTsfn",
        TestReleaseNullTsfn),
    DECLARE_NAPI_FUNCTION("testCallThreadsafeNullTsfn",
        TestCallThreadsafeNullTsfn),
    DECLARE_NAPI_FUNCTION("testConcurrentAcquireRelease",
        TestConcurrentAcquireRelease),
    DECLARE_NAPI_FUNCTION("testCallWithPriorityImmediate",
        TestCallWithPriorityImmediate),
    DECLARE_NAPI_FUNCTION("testCallWithPriorityHigh",
        TestCallWithPriorityHigh),
    DECLARE_NAPI_FUNCTION("testCallWithPriorityLow",
        TestCallWithPriorityLow),
    DECLARE_NAPI_FUNCTION("testCallWithPriorityIdle",
        TestCallWithPriorityIdle),
    DECLARE_NAPI_FUNCTION("testCallWithPriorityTail",
        TestCallWithPriorityTail),
    DECLARE_NAPI_FUNCTION("testCallWithPriorityInvalid",
        TestCallWithPriorityInvalid),
    DECLARE_NAPI_FUNCTION("testCallThreadsafeNonBlocking",
        TestCallThreadsafeNonBlocking),
    DECLARE_NAPI_FUNCTION("testCallThreadsafeNonBlockingThread",
        TestCallThreadsafeNonBlockingThread),
    DECLARE_NAPI_FUNCTION("testCallThreadsafeBlocking",
        TestCallThreadsafeBlocking),
    DECLARE_NAPI_FUNCTION("testReleaseWithAbort",
        TestReleaseWithAbort),
    DECLARE_NAPI_FUNCTION("testConcurrentMultipleThreads",
        TestConcurrentMultipleThreads),
    DECLARE_NAPI_FUNCTION("testThreadSafeFunctionDataPass",
        TestThreadSafeFunctionDataPass),
    DECLARE_NAPI_FUNCTION("testCallThreadsafeWithNullData",
        TestCallThreadsafeWithNullData),
    DECLARE_NAPI_FUNCTION("testCreateThreadsafeFunctionZeroQueue",
        TestCreateThreadsafeFunctionZeroQueue),
    DECLARE_NAPI_FUNCTION("testCreateThreadsafeWithContext",
        TestCreateThreadsafeWithContext),
    DECLARE_NAPI_FUNCTION("testCallThreadsafeFromMainThread",
        TestCallThreadsafeFromMainThread),
    DECLARE_NAPI_FUNCTION("testAcquireReleaseMultiTimes",
        TestAcquireReleaseMultiTimes),
    DECLARE_NAPI_FUNCTION("testEnvCleanupWithoutRelease",
        TestEnvCleanupWithoutRelease)
};

// 模块初始化导出与用例注册
static napi_value Init(napi_env env, napi_value exports)
{
    NAPI_CALL(env, napi_define_properties(env, exports,
        sizeof(THREADSAFE_TEST_DESCRIPTORS) / sizeof(THREADSAFE_TEST_DESCRIPTORS[0]),
        THREADSAFE_TEST_DESCRIPTORS));
    return exports;
}

// 模块注册定义
static napi_module g_threadsafeModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "threadsafe_suite",
    .nm_priv = nullptr,
    .reserved = { 0 },
};

// 使用 __attribute__((constructor)) 方式自动注册模块
extern "C" __attribute__((constructor)) void RegisterModule()
{
    napi_module_register(&g_threadsafeModule);
}
