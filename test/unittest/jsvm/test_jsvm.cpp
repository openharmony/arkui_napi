/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <cstring>
#include <deque>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include "jsvm.h"
#include "jsvm_types.h"
#include "jsvm_utils.h"

using namespace std;
using namespace testing;
using namespace testing::ext;

JSVM_Env jsvm_env = nullptr;

static string srcProf = R"JS(
function sleep(delay) {
    var start = (new Date()).getTime();
    while ((new Date()).getTime() - start < delay) {
        continue;
    }
}
function work9() {
    sleep(100);
}
function work8() {
    work9();
    sleep(100);
}
function work7() {
    work8();
    sleep(100);
}
function work6() {
    work7();
    sleep(100);
}
function work5() {
    work6();
    sleep(100);
}
function work4() {
    work5();
    sleep(100);
}

function work3() {
    work4();
    sleep(100);
}

function work2() {
    work3();
    sleep(100);
}

function work1() {
    work2();
    sleep(100);
}

work1();
)JS";

static JSVM_Value hello_fn(JSVM_Env env, JSVM_CallbackInfo info)
{
    JSVM_Value output;
    void* data = nullptr;
    OH_JSVM_GetCbInfo(env, info, nullptr, nullptr, nullptr, &data);
    OH_JSVM_CreateStringUtf8(env, (char*)data, strlen((char*)data), &output);
    return output;
}

class Task {
public:
    virtual ~Task() = default;
    virtual void Run() = 0;
};

static deque<Task*> task_queue;

static JSVM_Value read_fn(JSVM_Env env, JSVM_CallbackInfo info)
{
    JSVM_Value promise;
    JSVM_Deferred deferred;
    OH_JSVM_CreatePromise(env, &deferred, &promise);
    class ReadTask : public Task {
    public:
        ReadTask(JSVM_Env env, JSVM_Deferred deferred) : env_(env), deferred_(deferred) {}
        void Run() override
        {
            string str;
            getline(cin, str);
            JSVM_Value result;
            OH_JSVM_CreateStringUtf8(env_, str.c_str(), str.size(), &result);
            OH_JSVM_ResolveDeferred(env_, deferred_, result);
        }
    private:
        JSVM_Env env_;
        JSVM_Deferred deferred_;
    };
    task_queue.push_back(new ReadTask(env, deferred));
    return promise;
}

#define BUF_SIZE1 102400
static JSVM_Value print_fn(JSVM_Env env, JSVM_CallbackInfo info)
{
    size_t argc = 1;
    JSVM_Value argv[1];
    OH_JSVM_GetCbInfo(env, info, &argc, argv, nullptr, nullptr);
    if (argc > 0) {
        char buf[BUF_SIZE1];
        OH_JSVM_GetValueStringUtf8(env, argv[0], buf, BUF_SIZE1, nullptr);
        std::cout << buf << std::endl;
    }
    return nullptr;
}

static JSVM_CallbackStruct hello_cb = { hello_fn, (void *)"Hello" };
static JSVM_CallbackStruct read_cb = { read_fn, nullptr };
static JSVM_CallbackStruct print_cb = { print_fn, nullptr };

static JSVM_PropertyDescriptor property_descriptors[] = {
    {
        "hello",
        NULL,
        &hello_cb,
        NULL,
        NULL,
        NULL,
        JSVM_DEFAULT
    },
    {
        "read",
        NULL,
        &read_cb,
        NULL,
        NULL,
        NULL,
        JSVM_DEFAULT
    },
    {
        "print",
        NULL,
        &print_cb,
        NULL,
        NULL,
        NULL,
        JSVM_DEFAULT
    },
};

class JSVMTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "JSVMTest SetUpTestCase";
        JSVM_InitOptions init_options{};
        OH_JSVM_Init(&init_options);
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "JSVMTest TearDownTestCase";
    }

    void SetUp() override
    {
        GTEST_LOG_(INFO) << "JSVMTest SetUp";
        OH_JSVM_CreateVM(nullptr, &vm);
        // propertyCount is 3
        OH_JSVM_CreateEnv(vm, 3, property_descriptors, &env);
        OH_JSVM_OpenVMScope(vm, &vm_scope);
        OH_JSVM_OpenEnvScope(env, &env_scope);
        OH_JSVM_OpenHandleScope(env, &handleScope);
        jsvm_env = env;
    }
    void TearDown() override
    {
        GTEST_LOG_(INFO) << "JSVMTest TearDown";
        OH_JSVM_CloseHandleScope(env, handleScope);
        OH_JSVM_CloseEnvScope(env, env_scope);
        OH_JSVM_CloseVMScope(vm, vm_scope);
        OH_JSVM_DestroyEnv(env);
        OH_JSVM_DestroyVM(vm);
    }

protected:
    JSVM_Env env = nullptr;
    JSVM_VM vm = nullptr;
    JSVM_EnvScope env_scope = nullptr;
    JSVM_VMScope vm_scope = nullptr;
    JSVM_HandleScope handleScope;
};

HWTEST_F(JSVMTest, JSVMGetVersion001, TestSize.Level1)
{
    uint32_t versionId = 0;
    JSVMTEST_CALL(OH_JSVM_GetVersion(env, &versionId));
    ASSERT_EQ(versionId, 9);
}

HWTEST_F(JSVMTest, JSVMEquals001, TestSize.Level1)
{
    JSVM_Value lhs = nullptr;
    bool x = true;
    JSVMTEST_CALL(OH_JSVM_GetBoolean(env, x, &lhs));
    JSVM_Value rhs = nullptr;
    bool y = true;
    JSVMTEST_CALL(OH_JSVM_GetBoolean(env, y, &rhs));
    bool isEquals = false;
    JSVMTEST_CALL(OH_JSVM_Equals(env, lhs, rhs, &isEquals));
    ASSERT_TRUE(isEquals);
}

HWTEST_F(JSVMTest, JSVMCreateCodeCache001, TestSize.Level1)
{
    JSVM_Value jsSrc;
    JSVMTEST_CALL(OH_JSVM_CreateStringUtf8(env, srcProf.c_str(), srcProf.size(), &jsSrc));

    const uint8_t x1 = 34;
    const uint8_t* x2 = &x1;
    const uint8_t** dataPtr = &x2;
    size_t x3 = 1;
    size_t* lengthPtr = &x3;
    JSVM_Script script = nullptr;
    JSVMTEST_CALL(OH_JSVM_CompileScript(env, jsSrc, nullptr, 0, true, nullptr, &script));
    JSVMTEST_CALL(OH_JSVM_CreateCodeCache(env, script, dataPtr, lengthPtr));
}

HWTEST_F(JSVMTest, JSVMAcquire001, TestSize.Level1)
{
    JSVMTEST_CALL(OH_JSVM_AcquireLock(env));
}

HWTEST_F(JSVMTest, JSVMIsObject001, TestSize.Level1)
{
    JSVM_Value obj;
    JSVMTEST_CALL(OH_JSVM_CreateMap(env, &obj));
    bool result = false;
    JSVMTEST_CALL(OH_JSVM_IsObject(env, obj, &result));
    ASSERT_TRUE(result);
    
    JSVMTEST_CALL(OH_JSVM_CreateSymbol(env, nullptr, &obj));
    result = false;
    JSVMTEST_CALL(OH_JSVM_IsObject(env, obj, &result));
    ASSERT_FALSE(result);
}

HWTEST_F(JSVMTest, JSVMIsBoolean001, TestSize.Level1)
{
    JSVM_Value obj;
    JSVMTEST_CALL(OH_JSVM_CreateArray(env, &obj));
    bool result = false;
    JSVMTEST_CALL(OH_JSVM_IsBoolean(env, obj, &result));
    ASSERT_FALSE(result);

    bool boolvalue = true;
    JSVMTEST_CALL(OH_JSVM_GetBoolean(env, boolvalue, &obj));
    result = false;
    JSVMTEST_CALL(OH_JSVM_IsBoolean(env, obj, &result));
    ASSERT_TRUE(result);
}

HWTEST_F(JSVMTest, JSVMIsString001, TestSize.Level1)
{
    JSVM_Value createString;
    char str[12] = "hello world";
    JSVMTEST_CALL(OH_JSVM_CreateStringUtf8(env, str, 12, &createString));
    bool result = false;
    JSVMTEST_CALL(OH_JSVM_IsString(env, createString, &result));
    ASSERT_TRUE(result);

    JSVM_Value obj = nullptr;
    JSVMTEST_CALL(OH_JSVM_CreateSet(env, &obj));
    result = false;
    JSVMTEST_CALL(OH_JSVM_IsString(env, obj, &result));
    ASSERT_FALSE(result);
}

HWTEST_F(JSVMTest, JSVMIsFunction001, TestSize.Level1)
{
    JSVM_Value function;
    JSVM_CallbackStruct param;
    param.data = nullptr;
    param.callback = nullptr;
    JSVMTEST_CALL(OH_JSVM_CreateFunction(env, "func", JSVM_AUTO_LENGTH, &param, &function));
    bool result = false;
    JSVMTEST_CALL(OH_JSVM_IsFunction(env, function, &result));
    ASSERT_TRUE(result);

    JSVM_Value obj;
    JSVMTEST_CALL(OH_JSVM_CreateObject(env, &obj));
    result = false;
    JSVMTEST_CALL(OH_JSVM_IsFunction(env, obj, &result));
    ASSERT_FALSE(result);
}

HWTEST_F(JSVMTest, JSVMIsSymbol001, TestSize.Level1)
{
    JSVM_Value obj;
    JSVMTEST_CALL(OH_JSVM_CreateSymbol(env, nullptr, &obj));
    bool result = false;
    JSVMTEST_CALL(OH_JSVM_IsSymbol(env, obj, &result));
    ASSERT_TRUE(result);

    JSVMTEST_CALL(OH_JSVM_CreateObject(env, &obj));
    result = false;
    JSVMTEST_CALL(OH_JSVM_IsSymbol(env, obj, &result));
    ASSERT_FALSE(result);
}

HWTEST_F(JSVMTest, JSVMIsNumber001, TestSize.Level1)
{
    JSVM_Value obj;
    JSVMTEST_CALL(OH_JSVM_CreateObject(env, &obj));
    bool result = false;
    JSVMTEST_CALL(OH_JSVM_IsNumber(env, obj, &result));
    ASSERT_FALSE(result);

    JSVM_Value value;
    int intValue = 2;
    JSVMTEST_CALL(OH_JSVM_CreateInt32(env, intValue, &value));
    result = false;
    JSVMTEST_CALL(OH_JSVM_IsNumber(env, value, &result));
    ASSERT_TRUE(result);
}

HWTEST_F(JSVMTest, JSVMIsBigInt001, TestSize.Level1)
{
    JSVM_Value obj;
    JSVMTEST_CALL(OH_JSVM_CreateObject(env, &obj));
    bool result = false;
    JSVMTEST_CALL(OH_JSVM_IsBigInt(env, obj, &result));
    ASSERT_FALSE(result);

    JSVM_Value bigint;
    int intValue = 2;
    JSVMTEST_CALL(OH_JSVM_CreateBigintInt64(env, intValue, &bigint));
    result = false;
    JSVMTEST_CALL(OH_JSVM_IsBigInt(env, bigint, &result));
    ASSERT_TRUE(result);
}

HWTEST_F(JSVMTest, JSVMIsNull001, TestSize.Level1)
{
    JSVM_Value input;
    bool result = false;
    JSVMTEST_CALL(OH_JSVM_CreateArray(env, &input));
    JSVMTEST_CALL(OH_JSVM_IsNull(env, input, &result));
    ASSERT_FALSE(result);

    JSVM_Value input2;
    bool result2;
    JSVMTEST_CALL(OH_JSVM_GetNull(env, &input2));
    JSVMTEST_CALL(OH_JSVM_IsNull(env, input2, &result2));
    ASSERT_TRUE(result2);
}

HWTEST_F(JSVMTest, JSVMIsUndefined001, TestSize.Level1)
{
    JSVM_Value input;
    bool result = false;
    JSVMTEST_CALL(OH_JSVM_CreateArray(env, &input));
    JSVMTEST_CALL(OH_JSVM_IsUndefined(env, input, &result));
    ASSERT_FALSE(result);

    JSVM_Value input2;
    bool result2;
    JSVMTEST_CALL(OH_JSVM_GetUndefined(env, &input2));
    JSVMTEST_CALL(OH_JSVM_IsUndefined(env, input2, &result2));
    ASSERT_TRUE(result2);
}

HWTEST_F(JSVMTest, OH_JSVM_IsNullOrUndefined001, TestSize.Level1)
{
    JSVM_Value input;
    bool result = false;
    JSVMTEST_CALL(OH_JSVM_CreateArray(env, &input));
    JSVMTEST_CALL(OH_JSVM_IsNullOrUndefined(env, input, &result));
    ASSERT_FALSE(result);

    JSVM_Value input2;
    bool result2 = false;
    JSVMTEST_CALL(OH_JSVM_GetNull(env, &input2));
    JSVMTEST_CALL(OH_JSVM_IsNullOrUndefined(env, input2, &result2));
    ASSERT_TRUE(result2);

    result2 = false;
    JSVMTEST_CALL(OH_JSVM_GetUndefined(env, &input2));
    JSVMTEST_CALL(OH_JSVM_IsNullOrUndefined(env, input2, &result2));
    ASSERT_TRUE(result2);
}

HWTEST_F(JSVMTest, JSVMIsLocked001, TestSize.Level1)
{
    bool isLocked = false;
    JSVMTEST_CALL(OH_JSVM_IsLocked(env, &isLocked));
}

HWTEST_F(JSVMTest, JSVMReleaseLock001, TestSize.Level1)
{
    bool isLocked = false;
    JSVMTEST_CALL(OH_JSVM_IsLocked(env, &isLocked));
    JSVMTEST_CALL(OH_JSVM_ReleaseLock(env));
}

HWTEST_F(JSVMTest, JSVMCompileScriptWithOrigin001, TestSize.Level1)
{
    JSVM_Value jsSrc;
    JSVMTEST_CALL(OH_JSVM_CreateStringUtf8(env, srcProf.c_str(), srcProf.size(), &jsSrc));
    JSVM_Script script;
    JSVM_ScriptOrigin origin {
        .sourceMapUrl = "/data/local/tmp/workload/index.js.map",
        // 源文件名字
        .resourceName = "index.js",
        // scirpt 在源文件中的起始行列号
        .resourceLineOffset = 0,
        .resourceColumnOffset = 0,
    };
    JSVMTEST_CALL(OH_JSVM_CompileScriptWithOrigin(env, jsSrc, nullptr, 0, true, nullptr, &origin, &script));
}

HWTEST_F(JSVMTest, JSVMCompileScriptWithOrigin002, TestSize.Level1)
{
    JSVM_Value jsSrc = nullptr;
    bool x = true;
    JSVMTEST_CALL(OH_JSVM_GetBoolean(env, x, &jsSrc));
    JSVM_Script script;
    JSVM_ScriptOrigin origin {
        .sourceMapUrl = "/data/local/tmp/workload/index.js.map",
        // 源文件名字
        .resourceName = "index.js",
        // scirpt 在源文件中的起始行列号
        .resourceLineOffset = 0,
        .resourceColumnOffset = 0,
    };
    JSVM_Status status = OH_JSVM_CompileScriptWithOrigin(env, jsSrc, nullptr, 0, true, nullptr, &origin, &script);
    ASSERT_EQ(status, 3);
}

HWTEST_F(JSVMTest, JSVMCompileScriptWithOrigin003, TestSize.Level1)
{
    JSVM_Value jsSrc = nullptr;
    bool x = true;
    JSVMTEST_CALL(OH_JSVM_GetBoolean(env, x, &jsSrc));
    JSVM_ScriptOrigin origin {
        .sourceMapUrl = "/data/local/tmp/workload/index.js.map",
        // 源文件名字
        .resourceName = "index.js",
        // scirpt 在源文件中的起始行列号
        .resourceLineOffset = 0,
        .resourceColumnOffset = 0,
    };
    JSVM_Status status = OH_JSVM_CompileScriptWithOrigin(env, jsSrc, nullptr, 0, true, nullptr, &origin, nullptr);
    ASSERT_EQ(status, 1);
}

static JSVM_PropertyHandlerConfigurationStruct propertyCfg{
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

HWTEST_F(JSVMTest, JSVMDefineClassWithPropertyHandler001, TestSize.Level1)
{
    JSVM_CallbackStruct param;
    param.callback = [](JSVM_Env env, JSVM_CallbackInfo info) -> JSVM_Value {
        JSVM_Value thisVar = nullptr;
        JSVMTEST_CALL(OH_JSVM_GetCbInfo(env, info, nullptr, nullptr, &thisVar, nullptr));
        return thisVar;
    };
    param.data = nullptr;
    JSVM_Value testWrapClass = nullptr;
    JSVMTEST_CALL(OH_JSVM_DefineClassWithPropertyHandler(env, "Test2", 5, &param, 0, nullptr,
                                                    &propertyCfg, nullptr, &testWrapClass));
}

HWTEST_F(JSVMTest, JSVMDefineClassWithPropertyHandler002, TestSize.Level1)
{
    JSVM_CallbackStruct param;
    param.callback = [](JSVM_Env env, JSVM_CallbackInfo info) -> JSVM_Value {
        JSVM_Value thisVar = nullptr;
        JSVMTEST_CALL(OH_JSVM_GetCbInfo(env, info, nullptr, nullptr, &thisVar, nullptr));
        return thisVar;
    };
    param.data = nullptr;
    JSVM_Value testWrapClass = nullptr;
    JSVMTEST_CALL(OH_JSVM_DefineClassWithPropertyHandler(env, "Test2", JSVM_AUTO_LENGTH, &param, 0,
                                                    nullptr, &propertyCfg, nullptr, &testWrapClass));
}

HWTEST_F(JSVMTest, JSVMDefineClassWithPropertyHandler003, TestSize.Level1)
{
    JSVM_CallbackStruct param;
    param.callback = [](JSVM_Env env, JSVM_CallbackInfo info) -> JSVM_Value {
        JSVM_Value thisVar = nullptr;
        JSVMTEST_CALL(OH_JSVM_GetCbInfo(env, info, nullptr, nullptr, &thisVar, nullptr));
        return thisVar;
    };
    param.data = nullptr;
    JSVM_Value testWrapClass = nullptr;
    JSVMTEST_CALL(OH_JSVM_DefineClassWithPropertyHandler(env, "Test2", 4, &param, 0, nullptr, &propertyCfg,
                                                    nullptr, &testWrapClass));
}

HWTEST_F(JSVMTest, JSVMDefineClassWithPropertyHandler004, TestSize.Level1)
{
    JSVM_CallbackStruct param;
    param.callback = [](JSVM_Env env, JSVM_CallbackInfo info) -> JSVM_Value {
        JSVM_Value thisVar = nullptr;
        JSVMTEST_CALL(OH_JSVM_GetCbInfo(env, info, nullptr, nullptr, &thisVar, nullptr));
        return thisVar;
    };
    param.data = nullptr;
    JSVM_Value testWrapClass = nullptr;
    JSVMTEST_CALL(OH_JSVM_DefineClassWithPropertyHandler(env, "Test2", 6, &param, 0, nullptr,
                                                    &propertyCfg, nullptr, &testWrapClass));
}

HWTEST_F(JSVMTest, JSVMDefineClassWithPropertyHandler005, TestSize.Level1)
{
    JSVM_CallbackStruct param;
    param.callback = [](JSVM_Env env, JSVM_CallbackInfo info) -> JSVM_Value {
        JSVM_Value thisVar = nullptr;
        JSVMTEST_CALL(OH_JSVM_GetCbInfo(env, info, nullptr, nullptr, &thisVar, nullptr));
        return thisVar;
    };
    param.data = nullptr;
    JSVM_Status status = OH_JSVM_DefineClassWithPropertyHandler(env, "Test2", JSVM_AUTO_LENGTH, &param, 0,
                                                                nullptr, &propertyCfg, nullptr, nullptr);
    ASSERT_EQ(status, JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, JSVMDefineClassWithPropertyHandler006, TestSize.Level1)
{
    JSVM_Value testWrapClass = nullptr;
    JSVM_Status status = OH_JSVM_DefineClassWithPropertyHandler(env, "Test2", JSVM_AUTO_LENGTH, nullptr, 0,
                                                                nullptr, &propertyCfg, nullptr, &testWrapClass);
    ASSERT_EQ(status, JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, JSVMDefineClassWithPropertyHandler007, TestSize.Level1)
{
    JSVM_CallbackStruct param;
    param.callback = [](JSVM_Env env, JSVM_CallbackInfo info) -> JSVM_Value {
        JSVM_Value thisVar = nullptr;
        JSVMTEST_CALL(OH_JSVM_GetCbInfo(env, info, nullptr, nullptr, &thisVar, nullptr));
        return thisVar;
    };
    param.data = nullptr;
    JSVM_Value testWrapClass = nullptr;
    JSVM_Status status = OH_JSVM_DefineClassWithPropertyHandler(env, "Test2", JSVM_AUTO_LENGTH, &param, 0,
                                                                nullptr, nullptr, nullptr, &testWrapClass);
    ASSERT_EQ(status, JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, JSVMCreateSnapshot001, TestSize.Level1)
{
    const char *blobData = nullptr;
    size_t blobSize = 0;
    JSVM_Env envs[1] = {env};
    JSVM_Status status = OH_JSVM_CreateSnapshot(vm, 1, envs, &blobData, &blobSize);
    ASSERT_EQ(status, JSVM_GENERIC_FAILURE);
}

HWTEST_F(JSVMTest, JSVMCreateEnvFromSnapshot001, TestSize.Level1)
{
    JSVM_Env env2 = nullptr;
    JSVM_Status status = OH_JSVM_CreateEnvFromSnapshot(vm, 0, &env2);
    ASSERT_EQ(status, JSVM_GENERIC_FAILURE);
}

HWTEST_F(JSVMTest, JSVMTraceStart001, TestSize.Level1)
{
    JSVM_Status status = OH_JSVM_TraceStart(0, nullptr, "default", 0);
    ASSERT_EQ(status, JSVM_OK);
}

HWTEST_F(JSVMTest, JSVMTraceStart002, TestSize.Level1)
{
    std::vector<JSVM_TraceCategory> category = {JSVM_TRACE_WASM};
    JSVM_Status status = OH_JSVM_TraceStart(category.size(), category.data(), "custom", 100);
    ASSERT_EQ(status, JSVM_OK);
}

HWTEST_F(JSVMTest, JSVMTraceStart003, TestSize.Level1)
{
    std::vector<JSVM_TraceCategory> category(1);
    *(int *)&category[0] = 100;
    JSVM_Status status = OH_JSVM_TraceStart(category.size(), category.data(), "invalid", 0);
    ASSERT_EQ(status, JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, JSVMTraceStart004, TestSize.Level1)
{
    std::vector<JSVM_TraceCategory> category = {JSVM_TRACE_WASM};
    JSVM_Status status = OH_JSVM_TraceStart(0, category.data(), "invalid", 0);
    ASSERT_EQ(status, JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, JSVMTraceStart005, TestSize.Level1)
{
    JSVM_Status status = OH_JSVM_TraceStart(1, nullptr, "invalid", 0);
    ASSERT_EQ(status, JSVM_INVALID_ARG);
}

bool OutputStream(const char *data, int size, void *streamData)
{
    std::string value(data, size);
    *(std::string *)streamData = std::string(data, size);
    return true;
}

HWTEST_F(JSVMTest, JSVMTraceStop001, TestSize.Level1)
{
    JSVM_Status status = OH_JSVM_TraceStart(0, nullptr, "default", 0);
    ASSERT_EQ(status, JSVM_OK);
    std::string data;
    status = OH_JSVM_TraceStop(OutputStream, (void *)&data);
    ASSERT_EQ(status, JSVM_OK);
}

HWTEST_F(JSVMTest, JSVMTraceStop002, TestSize.Level1)
{
    std::string data;
    JSVM_Status status = OH_JSVM_TraceStop(OutputStream, (void *)&data);
    ASSERT_EQ(status, JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, JSVMTraceStop003, TestSize.Level1)
{
    JSVM_Status status = OH_JSVM_TraceStart(0, nullptr, "default", 0);
    ASSERT_EQ(status, JSVM_OK);
    status = OH_JSVM_TraceStop(OutputStream, nullptr);
    ASSERT_EQ(status, JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, JSVMTraceStop004, TestSize.Level1)
{
    JSVM_Status status = OH_JSVM_TraceStart(0, nullptr, "default", 0);
    ASSERT_EQ(status, JSVM_OK);
    std::string data;
    status = OH_JSVM_TraceStop(nullptr, (void *)&data);
    ASSERT_EQ(status, JSVM_INVALID_ARG);
}

HWTEST_F(JSVMTest, JSVMIsNumberObject001, TestSize.Level1)
{
    JSVM_Value result = jsvm::Run("new Number(42)");
    bool isNumberObject = false;
    JSVMTEST_CALL(OH_JSVM_IsNumberObject(env, result, &isNumberObject));
    ASSERT_TRUE(isNumberObject);
}

HWTEST_F(JSVMTest, JSVMIsBooleanObject001, TestSize.Level1)
{
    JSVM_Value result = jsvm::Run("new Boolean(true)");
    bool isBooleanObject = false;
    JSVMTEST_CALL(OH_JSVM_IsBooleanObject(env, result, &isBooleanObject));
    ASSERT_TRUE(isBooleanObject);
}

HWTEST_F(JSVMTest, JSVMIsBigIntObject001, TestSize.Level1)
{
    JSVM_Value result = jsvm::Run("new Object(42n)");
    bool isBigIntObject = false;
    JSVMTEST_CALL(OH_JSVM_IsBigIntObject(env, result, &isBigIntObject));
    ASSERT_TRUE(isBigIntObject);
}

HWTEST_F(JSVMTest, JSVMIsStringObject001, TestSize.Level1)
{
    JSVM_Value result = jsvm::Run("new String(\"test\")");
    bool isStringObject = false;
    JSVMTEST_CALL(OH_JSVM_IsStringObject(env, result, &isStringObject));
    ASSERT_TRUE(isStringObject);
}

HWTEST_F(JSVMTest, JSVMIsSymbolObject001, TestSize.Level1)
{
    JSVM_Value result = jsvm::Run("Object(Symbol('foo'))");
    bool isSymbolObject = false;
    JSVMTEST_CALL(OH_JSVM_IsSymbolObject(env, result, &isSymbolObject));
    ASSERT_TRUE(isSymbolObject);
}

HWTEST_F(JSVMTest, JSVMToStringTag001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolToStringTag(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.toStringTag")));
}

HWTEST_F(JSVMTest, JSVMToPrimitive001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolToPrimitive(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.toPrimitive")));
}

HWTEST_F(JSVMTest, JSVMSplit001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolSplit(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.split")));
}

HWTEST_F(JSVMTest, JSVMSearch001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolSearch(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.search")));
}

HWTEST_F(JSVMTest, JSVMReplace001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolReplace(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.replace")));
}

HWTEST_F(JSVMTest, JSVMMatch001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolMatch(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.match")));
}

HWTEST_F(JSVMTest, JSVMIsConcatSpreadable001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolIsConcatSpreadable(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.isConcatSpreadable")));
}

HWTEST_F(JSVMTest, JSVMHasInstance001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolHasInstance(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.hasInstance")));
}

HWTEST_F(JSVMTest, JSVMUnscopables001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolUnscopables(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.unscopables")));
}

HWTEST_F(JSVMTest, JSVMAsyncIterator001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolAsyncIterator(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.asyncIterator")));
}

HWTEST_F(JSVMTest, JSVMIterator001, TestSize.Level1)
{
    JSVM_Value result;
    JSVMTEST_CALL(OH_JSVM_GetSymbolIterator(env, &result));
    ASSERT_TRUE(jsvm::StrictEquals(result, jsvm::Run("Symbol.iterator")));
}
