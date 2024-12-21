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
#include "ark_interop_hitrace.h"
#include "ark_interop_internal.h"
#include "ark_interop_log.h"
#include "ark_interop_napi.h"
#include "gtest/gtest.h"
#include "uv_loop_handler.h"

#include <thread>

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

HWTEST_F(ArkInteropTest, ArkTSInteropNapiExternal001, TestSize.Level1)
{
    ARKTS_Env env = ARKTS_GetContext(engine_);
    char dllName[] = "123";
    ARKTS_LoadModule(env, dllName);
}

HWTEST_F(ArkInteropTest, ArkTSInteropNapiInstanceOf, TestSize.Level1)
{
    ARKTS_Env env = ARKTS_GetContext(engine_);
    auto global = ARKTS_GetGlobalConstant(env);
    char clError[] = "Error";
    auto jError = ARKTS_CreateUtf8(env, clError, sizeof(clError) - 1);
    auto errorCls = ARKTS_GetProperty(env, global, jError);
    auto errorObJ = ARKTS_New(env, errorCls, 0, nullptr);
    auto isError = ARKTS_InstanceOf(env, errorObJ, errorCls);
    EXPECT_TRUE(isError);
    auto jObj = ARKTS_CreateObject(env);
    EXPECT_FALSE(ARKTS_InstanceOf(env, jObj, errorCls));
}

HWTEST_F(ArkInteropTest, ArkTSInteropNapiHitrace001, TestSize.Level1)
{
    char name[] = "";
    int32_t taskId = 0;
    ARKTS_HiTraceStartTrace(name, taskId);
    ARKTS_HiTraceFinishTrace(name, taskId);
    ARKTS_HiTraceCountTrace(name, taskId);
}

[[maybe_unused]] static bool IsArrayBufferWithGC(ARKTS_Env env, ARKTS_Value value)
{
    auto tag = BIT_CAST(value, panda::JSValueRef);
    if (!tag.IsHeapObject()) {
        return false;
    }
    auto vm = P_CAST(env, EcmaVM*);
    auto handle = BIT_CAST(value, panda::Local<panda::JSValueRef>);
    panda::JSNApi::TriggerGC(vm, panda::JSNApi::TRIGGER_GC_TYPE::FULL_GC);
    return handle->IsArrayBuffer(vm) || handle->IsTypedArray(vm) || handle->IsDataView(vm);
}

TEST_F(ArkInteropTest, ArkIsArrayBufferWithGC)
{
    ARKTS_Env env = ARKTS_GetContext(engine_);
    auto object = ARKTS_CreateObject(env);
    EXPECT_EQ(ARKTS_GetValueType(env, object), N_OBJECT);

    constexpr int REPEAT = 1000;
    int index = 0;
    for (auto i = 0; i < REPEAT; i++) {
        if (!IsArrayBufferWithGC(env, object)) {
            index ++;
        }
    }
    EXPECT_EQ(index, REPEAT);
}

HWTEST_F(ArkInteropTest, ArkTSInteropNapiLog001, TestSize.Level1)
{
    LOGI("test LOGI");
    LOGE("test LOGE");
}

HWTEST_F(ArkInteropTest, ArkTSInteropNapiCreateEngineNew, TestSize.Level1)
{
    auto engine = ARKTS_CreateEngineWithNewThread();

    auto curTid = ARKTS_GetPosixThreadId();
    auto engineTid = ARKTS_GetThreadIdOfEngine(engine);

    EXPECT_NE(curTid, engineTid);

    ARKTS_DestroyEngine(engine);
}

TEST_F(ArkInteropTest, TestBigintFromInt64)
{
    ARKTS_Env env = ARKTS_GetContext(engine_);
    EXPECT_TRUE(env);
    {
        int64_t origins[] {
            0x123'4567'8945,
            -0x123'4567'8945,
            0x7FFF'FFFF'FFFF'FFFF,
            -0x7FFF'FFFF'FFFF'FFFF,
        };
        constexpr auto arrSize = std::size(origins);
        ARKTS_Value values[arrSize];
        for (auto i = 0; i < arrSize; i++) {
            values[i] = ARKTS_CreateBigInt(env, origins[i]);
            EXPECT_TRUE(ARKTS_IsBigInt(env, values[i]));
            EXPECT_EQ(ARKTS_GetValueType(env, values[i]), N_BIGINT);
            ARKTS_BigIntGetByteSize(env, values[i]);
        }
    }
}

TEST_F(ArkInteropTest, TestBigint16Bytes)
{
    ARKTS_Env env = ARKTS_GetContext(engine_);
    EXPECT_TRUE(env);
    uint8_t origin[] {
        0, 10, 20, 30, 40, 50, 60, 70,
        80, 90, 70, 50, 20, 30, 40, 50
    };
    bool isNegative = false;
    auto value = ARKTS_CreateBigIntWithBytes(env, isNegative, std::size(origin), origin);
    EXPECT_TRUE(ARKTS_IsBigInt(env, value));
    EXPECT_EQ(ARKTS_GetValueType(env, value), N_BIGINT);
    EXPECT_EQ(ARKTS_BigIntGetByteSize(env, value), std::size(origin));
    uint8_t received[std::size(origin)];
    ARKTS_BigIntReadBytes(env, value, &isNegative, std::size(origin), received);
    EXPECT_FALSE(isNegative);
    for (auto i = 0; i < std::size(origin); i++) {
        EXPECT_EQ(origin[i], received[i]);
    }
}

TEST_F(ArkInteropTest, TestBigint28Bytes)
{
    ARKTS_Env env = ARKTS_GetContext(engine_);
    EXPECT_TRUE(env);
    uint8_t origin[] {
        0, 10, 20, 30, 40, 50, 60, 70,
        80, 90, 70, 50, 20, 30, 40, 50,
        1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 20, 30,
    };
    bool isNegative = false;
    constexpr int expectSize = 32;
    // bigint words are 8 bytes aligned, received 32 bytes, and lower 4 bytes would be filled with 0.
    auto value = ARKTS_CreateBigIntWithBytes(env, isNegative, std::size(origin), origin);
    EXPECT_TRUE(ARKTS_IsBigInt(env, value));
    EXPECT_EQ(ARKTS_GetValueType(env, value), N_BIGINT);
    EXPECT_EQ(ARKTS_BigIntGetByteSize(env, value), expectSize);
    uint8_t received[expectSize];
    ARKTS_BigIntReadBytes(env, value, &isNegative, expectSize, received);
    EXPECT_FALSE(isNegative);
    for (auto i = 0; i < std::size(origin); i++) {
        EXPECT_EQ(origin[i], received[i + expectSize - std::size(origin)]);
    }
}

TEST_F(ArkInteropTest, TestBigint32Bytes)
{
    ARKTS_Env env = ARKTS_GetContext(engine_);
    EXPECT_TRUE(env);
    uint8_t origin[] {
        0, 10, 20, 30, 40, 50, 60, 70,
        80, 90, 70, 50, 20, 30, 40, 50,
        1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 20, 30, 40, 50, 60, 70,
    };
    bool isNegative = false;
    auto value = ARKTS_CreateBigIntWithBytes(env, isNegative, std::size(origin), origin);
    EXPECT_TRUE(ARKTS_IsBigInt(env, value));
    EXPECT_EQ(ARKTS_GetValueType(env, value), N_BIGINT);
    EXPECT_EQ(ARKTS_BigIntGetByteSize(env, value), std::size(origin));
    uint8_t received[std::size(origin)];
    ARKTS_BigIntReadBytes(env, value, &isNegative, std::size(origin), received);
    EXPECT_FALSE(isNegative);
    for (auto i = 0; i < std::size(origin); i++) {
        EXPECT_EQ(origin[i], received[i]);
    }
}

} // namespace

int main(int argc, char** argv)
{
    LOGI("main in");
    testing::GTEST_FLAG(output) = "xml:./";
    testing::InitGoogleTest(&argc, argv);

    auto runner = OHOS::AppExecFwk::EventRunner::Create(true);
    EXPECT_TRUE(runner);
    auto handler = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    EXPECT_TRUE(handler);

    int ret = -1;
    std::condition_variable cv;

    auto success = handler->PostTask([&ret, &cv] {
        engine_ = ARKTS_CreateEngine();

        ret = testing::UnitTest::GetInstance()->Run();
        cv.notify_all();
    });

    EXPECT_TRUE(success);

    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    auto status = cv.wait_for(lock, std::chrono::seconds(10));

    EXPECT_EQ(status, std::cv_status::no_timeout);

    ARKTS_DestroyEngine(engine_);
    engine_ = nullptr;
    runner->Stop();

    LOGI("main out");
    return ret;
}
