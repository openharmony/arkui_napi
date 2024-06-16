/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "ark_interop_external.h"
#include "ark_interop_helper.h"
#include "ark_interop_hitrace.h"
#include "ark_interop_internal.h"
#include "ark_interop_log.h"
#include "ark_interop_napi.h"
#include "gtest/gtest.h"
#include "uv_loop_handler.h"

using namespace testing;
using namespace testing::ext;

struct ARKTS_ModuleCallbacks {
    ARKTS_Value (*exportModule)(ARKTS_Env env, const char* dllName, ARKTS_Value exports) = nullptr;
    bool (*hasModuleHandle)(const char* dllName) = nullptr;
    void (*throwJSError)(ARKTS_Env env, ARKTS_Value) = nullptr;
    void (*throwNativeError)(const char*) = nullptr;
    void (*deleteArrayBufferRawData)(void* buffer, int64_t lambdaId) = nullptr;
    void (*deleteExternal)(int64_t id, ARKTS_Env env) = nullptr;
    ARKTS_Value (*invokerLambda)(ARKTS_CallInfo, int64_t lambdaId) = nullptr;
    void (*deleteLambda)(ARKTS_Env env, int64_t lambdaId) = nullptr;
    void (*invokeAsyncLambda)(ARKTS_Env env, int64_t lambdaId) = nullptr;
    void (*deleteJSContext)(ARKTS_Env env) = nullptr;
};

namespace {
ARKTS_Engine engine_ = nullptr;

} // namespace

class ArkInteropTest : public testing::Test {};

HWTEST_F(ArkInteropTest, ArkTSInteropNapiAsync001, TestSize.Level1)
{
    ARKTS_Env env = ARKTS_GetContext(engine_);
    ARKTS_CreateAsyncTask(env, 0);
    ARKTS_CreateAsyncTask(nullptr, 0);
    ARKTS_CreateAsyncTask(env, 0);
}

HWTEST_F(ArkInteropTest, ArkTSInteropNapi001, TestSize.Level1)
{
    ARKTS_Env env = ARKTS_GetContext(engine_);
    ARKTS_GetGlobalConstant(env);
    ARKTS_InitEventHandle(env);
}

void TestComplexType(ARKTS_Env env)
{
    auto glbConst = ARKTS_GetGlobalConstant(env);
    ARKTS_GetValueType(env, glbConst);
    auto numv = ARKTS_CreateF64(12.34);
    auto numv1 = ARKTS_CreateF64(12.34);
    EXPECT_FALSE(ARKTS_StrictEqual(env, glbConst, numv));
    EXPECT_TRUE(ARKTS_StrictEqual(env, numv, numv1));
    EXPECT_TRUE(ARKTS_IsNumber(numv));

    // string
    char origeStr[] = "ut test ArkInteropNapi004";
    auto strValue = ARKTS_CreateUtf8(env, origeStr, strlen(origeStr));
    EXPECT_EQ(ARKTS_GetValueUtf8(env, strValue, strlen(origeStr), origeStr), strlen(origeStr) + 1);
    EXPECT_TRUE(ARKTS_IsString(env, strValue));
    EXPECT_EQ(ARKTS_GetValueUtf8Size(env, strValue), strlen(origeStr) + 1);
    const char* transStr = ARKTS_GetValueCString(env, strValue);
    EXPECT_EQ(strcmp(transStr, origeStr), 0);
    ARKTS_FreeCString(transStr);
    transStr = nullptr;

    // func
    ARKTS_Value func = ARKTS_CreateFunc(env, 0);
    EXPECT_TRUE(ARKTS_IsCallable(env, func));
    ARKTS_Call(env, func, ARKTS_CreateUndefined(), 0, nullptr);

    // object
    ARKTS_Value objv = ARKTS_CreateObject(env);
    EXPECT_TRUE(ARKTS_IsHeapObject(objv));
    EXPECT_TRUE(ARKTS_IsObject(env, objv));

    // class
    ARKTS_Value cls = ARKTS_CreateClass(env, 0, ARKTS_CreateUndefined());
    EXPECT_TRUE(ARKTS_IsClass(env, cls));
    ARKTS_GetPrototype(env, cls);
    auto clsObj = ARKTS_New(env, cls, 0, nullptr);
    EXPECT_FALSE(ARKTS_InstanceOf(env, clsObj, cls));
    ARKTS_New(env, cls, 0, nullptr);

    // null
    auto nullv = ARKTS_CreateNull();
    EXPECT_TRUE(ARKTS_IsNull(nullv));

    // undefined
    EXPECT_TRUE(ARKTS_IsUndefined(ARKTS_CreateUndefined()));

    // bool
    auto boolv = ARKTS_CreateBool(false);
    EXPECT_TRUE(ARKTS_IsBool(boolv));
    EXPECT_FALSE(ARKTS_GetValueBool(boolv));

    // array
    auto arrv = ARKTS_CreateArray(env, 3);
    EXPECT_TRUE(ARKTS_IsArray(env, arrv));
    ARKTS_Value elements[] = { numv, numv, numv };
    auto arrv2 = ARKTS_CreateArrayWithInit(env, 3, elements);
    EXPECT_EQ(ARKTS_GetArrayLength(env, arrv), ARKTS_GetArrayLength(env, arrv2));
    ARKTS_SetElement(env, arrv, 1, numv);
    EXPECT_EQ(
        ARKTS_GetValueNumber(ARKTS_GetElement(env, arrv, 1)), ARKTS_GetValueNumber(ARKTS_GetElement(env, arrv2, 1)));

    // Recreate the engine to clear the exception.
    ARKTS_DestroyEngine(engine_);
    engine_ = ARKTS_CreateEngine();
}

void TestBasicType(ARKTS_Env env)
{
    // global value
    char origeStr[] = "ut test ArkInteropNapi005";
    auto strValue = ARKTS_CreateUtf8(env, origeStr, strlen(origeStr));
    auto glb = ARKTS_CreateGlobal(env, strValue);
    ARKTS_GetGlobalValue(glb);
    ARKTS_DisposeGlobal(env, glb);
    ARKTS_GetGlobalValue(glb);

    // external
    auto extv = ARKTS_CreateExternal(env, nullptr);
    EXPECT_TRUE(ARKTS_IsExternal(env, extv));
    EXPECT_EQ(ARKTS_GetExternalData(env, extv), nullptr);

    // symbol
    char des[] = "symbol test";
    auto symv = ARKTS_CreateSymbol(env, des, strlen(des));
    ARKTS_CreateSymbol(env, nullptr, 0);
    EXPECT_TRUE(ARKTS_IsSymbol(env, symv));
    EXPECT_FALSE(ARKTS_IsSymbol(env, ARKTS_CreateBool(true)));
    EXPECT_EQ(strcmp(des, ARKTS_GetSymbolDesc(env, symv)), 0);

    // promise capability
    auto prom = ARKTS_CreatePromiseCapability(env);
    auto promv = ARKTS_GetPromiseFromCapability(env, prom);
    ARKTS_PromiseCapabilityResolve(env, prom, promv);
    ARKTS_PromiseCapabilityReject(env, prom, promv);
    EXPECT_TRUE(ARKTS_IsPromise(env, promv));
    ARKTS_PromiseThen(env, promv, ARKTS_CreateFunc(env, 0), promv);
    ARKTS_PromiseCatch(env, promv, ARKTS_CreateFunc(env, 0));

    // array buffer
    auto abv1 = ARKTS_CreateArrayBuffer(env, 1024);
    EXPECT_TRUE(ARKTS_IsArrayBuffer(env, abv1));
    auto rawPtr = (int8_t*)ARKTS_GetArrayBufferRawPtr(env, abv1);
    rawPtr[0] = 1;

    void* buf = malloc(1024);
    EXPECT_NE(buf, nullptr);
    auto abv2 = ARKTS_CreateArrayBufferWithData(env, buf, 1024, 0);
    auto rawPtr2 = (int8_t*)ARKTS_GetArrayBufferRawPtr(env, abv2);
    rawPtr2[0] = 1;

    EXPECT_EQ(ARKTS_GetArrayBufferLength(env, abv1), ARKTS_GetArrayBufferLength(env, abv2));
    auto data0 = (int8_t*)malloc(1);
    EXPECT_NE(data0, nullptr);
    ARKTS_ArrayBufferReadBytes(env, abv1, data0, 1);
    EXPECT_EQ(*data0, 1);
    ARKTS_ArrayBufferReadBytes(env, abv2, data0, 1);
    EXPECT_EQ(*data0, 1);

    // property
    ARKTS_Value objv = ARKTS_CreateObject(env);
    ARKTS_DefineOwnProperty(env, objv, strValue, ARKTS_CreateBool(false), ARKTS_PropertyFlag::N_ENUMERABLE);
    EXPECT_TRUE(ARKTS_HasOwnProperty(env, objv, strValue));
    EXPECT_NE(ARKTS_EnumOwnProperties(env, objv), nullptr);
    auto v = ARKTS_GetProperty(env, objv, strValue);
    ARKTS_SetProperty(env, objv, strValue, ARKTS_CreateBool(true));
    EXPECT_NE(ARKTS_GetProperty(env, objv, strValue), v);
}

#ifdef CLANG_COVERAGE
HWTEST_F(ArkInteropTest, ArkTSInteropNapi003, TestSize.Level1)
{
    // Test for no callback is registered.
    ARKTS_Env env = ARKTS_GetContext(engine_);
    ARKTSInner_CJAsyncCallback(env, nullptr);

    TestComplexType(env);
    TestBasicType(env);

    char dllName[] = "123";
    ARKTS_LoadModule(env, dllName);
}

HWTEST_F(ArkInteropTest, ArkTSInteropNapi004, TestSize.Level1)
{
    // Test registered callback
    ARKTS_Env env = ARKTS_GetContext(engine_);
    auto callback = GetCallBack();
    ARKTS_SetCJModuleCallback(callback.get());
    ARKTSInner_CJAsyncCallback(env, nullptr);
    // Recreate the engine to clear the exception.
    ARKTS_DestroyEngine(engine_);
    engine_ = ARKTS_CreateEngine();
    env = ARKTS_GetContext(engine_);
    TestComplexType(env);
    TestBasicType(env);

    char dllName[] = "123";
    ARKTS_LoadModule(env, dllName);

    char origeStr[] = "ut test ArkInteropNapi007";
    auto strValue = ARKTS_CreateUtf8(env, origeStr, strlen(origeStr));
    ARKTSInner_FormatJSError(env, strValue);

    auto result = JSNApi::GetAndClearUncaughtException(P_CAST(env, EcmaVM*));
    auto jError = ARKTS_FromHandle(result);
    ARKTSInner_FormatJSError(env, jError);
    ARKTS_Throw(env, jError);
}

#endif

HWTEST_F(ArkInteropTest, ArkTSInteropNapi005, TestSize.Level1)
{
    ARKTS_Env env = ARKTS_GetContext(engine_);
    EXPECT_NE(ARKTS_GetNAPIEnv(engine_), nullptr);

    auto scope = ARKTS_OpenScope(env);
    auto subscope = ARKTS_OpenScope(env);
    ARKTS_CloseScope(env, subscope);
    auto val = ARKTS_CreateF64(12.34);
    EXPECT_NE(ARKTS_Return(env, scope, val), nullptr);
}

HWTEST_F(ArkInteropTest, ArkTSInteropNapi006, TestSize.Level1)
{
    ARKTS_Env env = ARKTS_GetContext(engine_);
    ARKTS_Value objv = ARKTS_CreateObject(env);
    char origeStr[] = "key01";
    auto key = ARKTS_CreateUtf8(env, origeStr, strlen(origeStr));
    ARKTS_Value get = ARKTS_CreateFunc(env, 0);
    ARKTS_Value set = ARKTS_CreateFunc(env, 0);
    ARKTS_DefineAccessors(env, objv, key, {get, set, ARKTS_PropertyFlag::N_ENUMERABLE});
}

HWTEST_F(ArkInteropTest, ArkTSInteropNapi007, TestSize.Level1)
{
    ARKTS_Env env = ARKTS_GetContext(engine_);
    ARKTS_GetValueType(env, ARKTS_CreateNull());
    ARKTS_GetValueType(env, ARKTS_CreateUndefined());

    char des[] = "symbol test";
    auto symv = ARKTS_CreateSymbol(env, des, strlen(des));
    ARKTS_GetValueType(env, symv);
    ARKTS_StrictEqual(env, symv, symv);

    auto boolv = ARKTS_CreateBool(false);
    ARKTS_GetValueType(env, boolv);
    ARKTS_StrictEqual(env, boolv, boolv);
}

HWTEST_F(ArkInteropTest, ArkTSInteropNapi008, TestSize.Level1)
{
    auto jsRuntimeCI = panda::JsiRuntimeCallInfo();
    ARKTS_CallInfo callinfo = P_CAST(&jsRuntimeCI, ARKTS_CallInfo);
    ARKTS_GetArgCount(callinfo);
    ARKTS_GetArg(callinfo, 0);
    ARKTS_GetThisArg(callinfo);
}

HWTEST_F(ArkInteropTest, ArkTSInteropNapiExternal001, TestSize.Level1)
{
    ARKTS_Env env = ARKTS_GetContext(engine_);
    char dllName[] = "123";
    ARKTS_LoadModule(env, dllName);
}

HWTEST_F(ArkInteropTest, ArkTSInteropNapiHitrace001, TestSize.Level1)
{
    char name[] = "";
    int32_t taskId = 0;
    ARKTS_HiTraceStartTrace(name, taskId);
    ARKTS_HiTraceFinishTrace(name, taskId);
    ARKTS_HiTraceCountTrace(name, taskId);
}

HWTEST_F(ArkInteropTest, ArkTSInteropNapiLog001, TestSize.Level1)
{
    LOGI("test LOGI");
    LOGE("test LOGE");
}

int main(int argc, char** argv)
{
    engine_ = ARKTS_CreateEngine();
    LOGI("main in");
    testing::GTEST_FLAG(output) = "xml:./";
    testing::InitGoogleTest(&argc, argv);
    int ret = testing::UnitTest::GetInstance()->Run();
    ARKTS_DestroyEngine(engine_);
    engine_ = nullptr;
    if (!ret) {
        LOGE("run test failed. return %d", ret);
        return ret;
    }
    LOGI("main out");
    return ret;
}
