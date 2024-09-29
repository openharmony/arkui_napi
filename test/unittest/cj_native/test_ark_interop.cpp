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

#include <cmath>
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

TEST_F(ArkInteropTest, Prime)
{
    auto env = ARKTS_GetContext(engine_);
    auto scope = ARKTS_OpenScope(env);

    auto jsUDef = ARKTS_CreateUndefined();
    EXPECT_TRUE(ARKTS_IsUndefined(jsUDef));
    EXPECT_EQ(ARKTS_GetValueType(env, jsUDef), N_UNDEFINED);

    auto jsNull = ARKTS_CreateNull();
    EXPECT_TRUE(ARKTS_IsNull(jsNull));
    EXPECT_EQ(ARKTS_GetValueType(env, jsNull), N_NULL);

    ARKTS_Value jsBools[] {
        ARKTS_CreateBool(true),
        ARKTS_CreateBool(false)
    };
    EXPECT_TRUE(ARKTS_IsBool(jsBools[0]));
    EXPECT_EQ(ARKTS_GetValueType(env, jsBools[0]), N_BOOL);
    EXPECT_TRUE(ARKTS_GetValueBool(jsBools[0]));
    EXPECT_FALSE(ARKTS_GetValueBool(jsBools[1]));

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, Number)
{
    auto env = ARKTS_GetContext(engine_);
    auto scope = ARKTS_OpenScope(env);
    double origins[] {
        0.1,
        -12.1,
        12456.126546
    };
    constexpr auto totalCases = std::size(origins);
    ARKTS_Value jsValues[totalCases];
    double received[totalCases];

    for (size_t i = 0;i < totalCases; i++) {
        jsValues[i] = ARKTS_CreateF64(origins[i]);
        EXPECT_EQ(ARKTS_GetValueType(env, jsValues[i]), N_NUMBER);
        EXPECT_TRUE(ARKTS_IsNumber(jsValues[i]));
        received[i] = ARKTS_GetValueNumber(jsValues[i]);
        EXPECT_EQ(origins[i], received[i]);
    }

    auto jsNan = ARKTS_CreateF64(NAN);
    auto nNan = ARKTS_GetValueNumber(jsNan);
    EXPECT_TRUE(std::isnan(nNan));
    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, String)
{
    auto env = ARKTS_GetContext(engine_);
    auto scope = ARKTS_OpenScope(env);
    const char* origins[] {
        "a plain text",
        "`~!@#$%^&*()_+[]\\",
        "中文字符",
        "😊😂🤣❤️😍😒👌😘",
    };
    ARKTS_Value jsValues[] {
        ARKTS_CreateUtf8(env, origins[0], strlen(origins[0])),
        ARKTS_CreateUtf8(env, origins[1], strlen(origins[1])),
        ARKTS_CreateUtf8(env, origins[2], strlen(origins[2])),
        ARKTS_CreateUtf8(env, origins[3], strlen(origins[3])),
    };
    EXPECT_TRUE(ARKTS_IsString(env, jsValues[0]));
    EXPECT_TRUE(ARKTS_IsHeapObject(jsValues[0]));
    EXPECT_EQ(ARKTS_GetValueType(env, jsValues[0]), N_STRING);
    for (auto i = 0;i < sizeof(jsValues)/sizeof(ARKTS_Value); i++) {
        auto size = ARKTS_GetValueUtf8Size(env, jsValues[i]);
        std::string result;
        result.resize(size - 1);
        ARKTS_GetValueUtf8(env, jsValues[i], size - 1, result.data());
        EXPECT_EQ(result, origins[i]);

        auto cStr = ARKTS_GetValueCString(env, jsValues[i]);
        result = cStr;
        EXPECT_EQ(result, origins[i]);
        ARKTS_FreeCString(cStr);
    }
    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, Object)
{
    auto env = ARKTS_GetContext(engine_);
    auto scope = ARKTS_OpenScope(env);
    auto obj = ARKTS_CreateObject(env);
    EXPECT_TRUE(ARKTS_IsHeapObject(obj));
    EXPECT_TRUE(ARKTS_IsObject(env, obj));
    EXPECT_EQ(ARKTS_GetValueType(env, obj), N_OBJECT);
    auto keyA = ARKTS_CreateUtf8(env, "a", 1);
    EXPECT_FALSE(ARKTS_HasOwnProperty(env, obj, keyA));
    auto valueA = ARKTS_GetProperty(env, obj, keyA);
    EXPECT_TRUE(ARKTS_IsUndefined(valueA));
    valueA = ARKTS_CreateBool(true);
    ARKTS_SetProperty(env, obj, keyA, valueA);
    EXPECT_TRUE(ARKTS_HasOwnProperty(env, obj, keyA));
    auto receivedA = ARKTS_GetProperty(env, obj, keyA);
    EXPECT_TRUE(ARKTS_IsBool(receivedA));
    EXPECT_TRUE(ARKTS_GetValueBool(receivedA));
    auto keys = ARKTS_EnumOwnProperties(env, obj);
    EXPECT_TRUE(ARKTS_IsArray(env, keys));
    EXPECT_EQ(ARKTS_GetArrayLength(env, keys), 1);
    auto key = ARKTS_GetElement(env, keys, 0);
    EXPECT_TRUE(ARKTS_IsString(env, key));
    EXPECT_TRUE(ARKTS_StrictEqual(env, keyA, key));
    ARKTS_CloseScope(env, scope);
}

struct PropCase {
    char k;
    bool writable;
    bool enumerable;
    bool configurable;
};

TEST_F(ArkInteropTest, DefineProperty)
{
    auto env = ARKTS_GetContext(engine_);
    auto scope = ARKTS_OpenScope(env);
    auto obj = ARKTS_CreateObject(env);
    PropCase cases[] {
        {'a', false, false, false}, {'b', true, false, false}, {'c', false, true, false}, {'d', false, false, true},
        {'e', true, true, false}, {'f', true, false, true}, {'g', false, true, true}, {'h', true, true, true}
    };
    constexpr auto totalCases = std::size(cases);
    auto valueT = ARKTS_CreateBool(true), valueF = ARKTS_CreateBool(false);
    ARKTS_Value keys[totalCases];
    for (auto i = 0; i < totalCases; i++) {
        keys[i] = ARKTS_CreateUtf8(env, &cases[i].k, 1);
        ARKTS_DefineOwnProperty(env, obj, keys[i], valueF, static_cast<ARKTS_PropertyFlag>(
            (cases[i].writable ? N_WRITABLE : 0) |
            (cases[i].enumerable ? N_ENUMERABLE : 0) |
            (cases[i].configurable ? N_CONFIGURABLE : 0)
        ));
    }
    constexpr int expectKeys = 4;
    auto jsKeys = ARKTS_EnumOwnProperties(env, obj);
    EXPECT_TRUE(ARKTS_IsArray(env, jsKeys));
    EXPECT_EQ(ARKTS_GetArrayLength(env, jsKeys), expectKeys);
    for (auto i = 0; i < expectKeys; i++) {
        auto jsKey = ARKTS_GetElement(env, jsKeys, i);
        EXPECT_TRUE(ARKTS_IsString(env, jsKey));
        char buffer = 0;
        ARKTS_GetValueUtf8(env, jsKey, 1, &buffer);
        EXPECT_TRUE(buffer == 'c' || buffer == 'e' || buffer == 'g' || buffer == 'h');
    }
    for (auto i = 0; i < totalCases; i++) { // writable
        if (cases[i].writable && cases[i].configurable) {
            ARKTS_SetProperty(env, obj, keys[i], valueT);
            auto receivedJS = ARKTS_GetProperty(env, obj, keys[i]);
            auto recievedN = ARKTS_GetValueBool(receivedJS);
            EXPECT_TRUE(recievedN);
        }
    }
    for (auto i = 0;i < totalCases; ++i) { // configurable
        if (cases[i].configurable) {
            ARKTS_DefineOwnProperty(env, obj, keys[i], valueT,
                static_cast<ARKTS_PropertyFlag>(N_WRITABLE | N_ENUMERABLE | N_CONFIGURABLE));
        }
    }
    jsKeys = ARKTS_EnumOwnProperties(env, obj);
    constexpr int expectLength = 6;
    EXPECT_EQ(ARKTS_GetArrayLength(env, jsKeys), expectLength);
    ARKTS_CloseScope(env, scope);
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

HWTEST_F(ArkInteropTest, ArkTSInteropNapiCreateEngineNew, TestSize.Level1)
{
    auto engine = ARKTS_CreateEngineWithNewThread();

    auto curTid = ARKTS_GetPosixThreadId();
    auto engineTid = ARKTS_GetThreadIdOfEngine(engine);

    EXPECT_NE(curTid, engineTid);

    ARKTS_DestroyEngine(engine);
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
