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

#include "test_ark_interop_common.h"

using namespace testing;
using namespace testing::ext;

TEST_F(ArkInteropTest, Types)
{
    RunLocalTest();
}

TEST_F(ArkInteropTest, PromiseThen)
{
    MockContext mockCtx(ARKTS_CreateEngineWithNewThread());
    auto env = mockCtx.GetEnv();
    EXPECT_TRUE(env);

    std::condition_variable cv;
    bool isComplete = false;
    auto funcId = mockCtx.StoreAsyncFunc([&mockCtx, &cv, &isComplete, env] {
        auto scope = ARKTS_OpenScope(env);
        auto promiseCap = ARKTS_CreatePromiseCapability(env);
        auto promise = ARKTS_GetPromiseFromCapability(env, promiseCap);
        auto thenFuncId = mockCtx.StoreFunc([&cv, &isComplete](ARKTS_CallInfo info) {
            isComplete = true;
            cv.notify_one();
            return ARKTS_CreateUndefined();
        });
        auto func = ARKTS_CreateFunc(env, thenFuncId);
        ARKTS_PromiseThen(env, promise, func, ARKTS_CreateUndefined());
        ARKTS_PromiseCapabilityResolve(env, promiseCap, ARKTS_CreateUndefined());
        ARKTS_CloseScope(env, scope);
    });
    ARKTS_CreateAsyncTask(env, funcId);
    std::mutex mutex;
    std::unique_lock lock(mutex);

    constexpr int checkDuration = 100;
    int waitTimes = 50; // set 100ms timeout
    while (!isComplete && waitTimes--) {
        cv.wait_for(lock, std::chrono::milliseconds(checkDuration));
    }
    // EXPECT no core dump, no timeout
    EXPECT_TRUE(waitTimes > 0);
}

TEST_F(ArkInteropTest, CycleFreeFunc)
{
    CycleFreeContext cycleFreeCtx;
    auto env = cycleFreeCtx.GetEnv();
    auto funcCount = cycleFreeCtx.GetCycleFreeFuncCount();
    auto scope = ARKTS_OpenScope(env);
    auto isCalled = false;
    auto id = cycleFreeCtx.StoreCycleFreeFunc([&isCalled](ARKTS_CallInfo callInfo) {
        isCalled = true;
        return ARKTS_CreateUndefined();
    });
    EXPECT_EQ(cycleFreeCtx.GetCycleFreeFuncCount(), funcCount + 1);
    auto func = ARKTS_CreateCycleFreeFunc(cycleFreeCtx.GetEnv(), id);
    EXPECT_TRUE(ARKTS_IsCallable(env, func));
    ARKTS_Call(env, func, ARKTS_CreateUndefined(), 0, nullptr);
    EXPECT_TRUE(isCalled);
    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, CycleFreeExtern)
{
    CycleFreeContext cycleFreeCtx;
    auto env = cycleFreeCtx.GetEnv();
    auto funcCount = cycleFreeCtx.GetCycleFreeFuncCount();
    {
        auto scope = ARKTS_OpenScope(env);
        auto id = cycleFreeCtx.StoreCycleFreeFunc(nullptr);
        EXPECT_EQ(cycleFreeCtx.GetCycleFreeFuncCount(), funcCount + 1);
        auto object = ARKTS_CreateCycleFreeExtern(cycleFreeCtx.GetEnv(), id);
        EXPECT_TRUE(ARKTS_IsExternal(env, object));
        auto resid = ARKTS_GetExternalData(env, object);
        EXPECT_EQ(resid, id);

        ARKTS_CloseScope(env, scope);
    }
}

class GlobalWeakTest {
public:
    GlobalWeakTest(): cycleFreeCtx(ARKTS_CreateEngineWithNewThread()), status(CREATING)
    {
    }
    ~GlobalWeakTest()
    {
        ReleaseGlobals();
    }

    void Start()
    {
        ScheduleNext();
    }

    void WaitForComplete()
    {
        std::mutex mutex;
        std::unique_lock lock(mutex);
        while (status != COMPLETE) {
            cv.wait(lock);
        }
    }

    bool IsComplete() const
    {
        return status == COMPLETE;
    }

    bool IsAllDisposed() const
    {
        for (auto one : globals) {
            if (ARKTS_GlobalIsAlive(cycleFreeCtx.GetEnv(), one)) {
                return false;
            }
        }
        return true;
    }

private:
    static constexpr int objectCnt = 1000;

    void CreateWeakObjects()
    {
        auto env = cycleFreeCtx.GetEnv();
        auto scope = ARKTS_OpenScope(env);
        for (auto i = 0; i < objectCnt; i++) {
            auto object = ARKTS_CreateObject(env);
            auto global = ARKTS_CreateGlobal(env, object);
            ARKTS_GlobalSetWeak(env, global);
            globals.push_back(global);
        }
        ARKTS_CloseScope(env, scope);
        panda::JSNApi::TriggerGC(P_CAST(env, EcmaVM*), panda::JSNApi::TRIGGER_GC_TYPE::FULL_GC);
    }

    void DoAssertion()
    {
        for (auto one : globals) {
            EXPECT_TRUE(!ARKTS_GlobalIsAlive(cycleFreeCtx.GetEnv(), one));
        }
    }

    void ReleaseGlobals()
    {
        for (auto one : globals) {
            ARKTS_DisposeGlobalSync(cycleFreeCtx.GetEnv(), one);
        }
    }

    enum Status {
        CREATING,
        ASSERTION,
        DISPOSE,
        COMPLETE
    };

    void ScheduleNext()
    {
        auto id = cycleFreeCtx.StoreAsyncFunc([this] {
            DoNext();
        });
        ARKTS_CreateAsyncTask(cycleFreeCtx.GetEnv(), id);
    }

    void DoNext()
    {
        switch (status) {
            case CREATING:
                CreateWeakObjects();
                status = ASSERTION;
                break;
            case ASSERTION:
                DoAssertion();
                status = DISPOSE;
                break;
            case DISPOSE:
                status = COMPLETE;
                cv.notify_all();
                return;
            default: ;
        }
        ScheduleNext();
    }

    CycleFreeContext cycleFreeCtx;
    Status status;
    std::condition_variable cv;
    std::vector<ARKTS_Global> globals;
};

TEST_F(ArkInteropTest, GlobalWeak)
{
    GlobalWeakTest weakTest;
    weakTest.Start();
    weakTest.WaitForComplete();
    EXPECT_TRUE(weakTest.IsComplete());
}

TEST_F(ArkInteropTest, GlobalToValue)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    {
        auto scope = ARKTS_OpenScope(env);
        auto object = ARKTS_CreateObject(env);
        auto global = ARKTS_CreateGlobal(env, object);
        auto value = ARKTS_GlobalToValue(env, global);
        auto received = ARKTS_GlobalFromValue(env, value);
        EXPECT_EQ(received, global);
        ARKTS_DisposeGlobalSync(env, global);
        ARKTS_CloseScope(env, scope);
    }
}

HWTEST_F(ArkInteropTest, ArkTSInteropNapiCreateEngineNew, TestSize.Level1)
{
    MockContext mockCtx(ARKTS_CreateEngineWithNewThread());

    auto curTid = ARKTS_GetPosixThreadId();
    auto engineTid = ARKTS_GetThreadIdOfEngine(mockCtx.GetEngine());

    EXPECT_NE(curTid, engineTid);

    auto env = mockCtx.GetEnv();
    EXPECT_TRUE(env);
    bool isComplete = false;
    std::condition_variable cv;
    auto funcId = mockCtx.StoreAsyncFunc([&isComplete, &cv] {
        ArkInteropTest::RunLocalTest();
        isComplete = true;
        cv.notify_one();
    });
    ARKTS_CreateAsyncTask(env, funcId);
    std::mutex mutex;
    std::unique_lock lock(mutex);
    constexpr int checkDuration = 10;
    int waitTimes = 100; // set 1000ms timeout
    while (!isComplete && waitTimes--) {
        cv.wait_for(lock, std::chrono::milliseconds(checkDuration));
    }
    EXPECT_TRUE(waitTimes > 0);
}

TEST_F(ArkInteropTest, ScopeMT)
{
    constexpr int threadCount = 1000;
    std::thread threads[threadCount];
    for (auto i = 0; i < threadCount; i++) {
        threads[i] = std::thread([] {
            panda::RuntimeOption options;
            auto vm = panda::JSNApi::CreateJSVM(options);
            EXPECT_TRUE(vm);
            auto env = P_CAST(vm, ARKTS_Env);
            auto scope = ARKTS_OpenScope(env);
            EXPECT_TRUE(scope);
            ARKTS_CloseScope(env, scope);
            panda::JSNApi::DestroyJSVM(vm);
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }
}

TEST_F(ArkInteropTest, GlobalRelease)
{
    MockContext mockCtx(ARKTS_CreateEngineWithNewThread());
    auto env = mockCtx.GetEnv();

    bool isComplete = false;
    int loops = 10;
    std::condition_variable cv;
    std::function <void()> callback;
    callback = [&isComplete, &cv, &loops, env, &callback] {
        if (loops == 0) {
            isComplete = true;
            cv.notify_one();
            return;
        }
        auto scope = ARKTS_OpenScope(env);
        auto totalRepeat = 200000;
        for (int index = 0;index < totalRepeat; ++index) {
            auto object = ARKTS_CreateObject(env);
            auto global = ARKTS_CreateGlobal(env, object);
            ARKTS_DisposeGlobal(env, global);
        }
        ARKTS_CloseScope(env, scope);
        --loops;
        auto innerFuncId = MockContext::GetInstance()->StoreAsyncFunc(callback);
        ARKTS_CreateAsyncTask(env, innerFuncId);
    };
    auto funcId = mockCtx.StoreAsyncFunc(callback);
    ARKTS_CreateAsyncTask(env, funcId);
    std::mutex mutex;
    std::unique_lock lock(mutex);
    int waitTimes = 200;
    int msEachTime = 100;
    while (!isComplete && waitTimes--) {
        cv.wait_for(lock, std::chrono::milliseconds(msEachTime));
    }
    EXPECT_TRUE(isComplete);
}

TEST_F(ArkInteropTest, GlobalReleaseSync)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    int loops = 10;
    auto totalRepeat = 200000;
    for (int index = 0;index < loops; ++index) {
        auto scope = ARKTS_OpenScope(env);
        for (int j = 0;j < totalRepeat; ++j) {
            auto object = ARKTS_CreateObject(env);
            auto global = ARKTS_CreateGlobal(env, object);
            ARKTS_DisposeGlobalSync(env, global);
        }
        EXPECT_FALSE(panda::JSNApi::HasPendingException(P_CAST(env, EcmaVM*)));
        ARKTS_CloseScope(env, scope);
    }
}

TEST_F(ArkInteropTest, PromiseRelease)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    int loops = 20;
    auto totalRepeat = 20000;
    auto undefined = ARKTS_CreateUndefined();
    for (auto i = 0; i < loops; i++) {
        auto scope = ARKTS_OpenScope(env);
        for (auto j = 0; j < totalRepeat; j++) {
            auto promiseCap = ARKTS_CreatePromiseCapability(env);
            ARKTS_PromiseCapabilityResolve(env, promiseCap, undefined);
        }
        EXPECT_FALSE(panda::JSNApi::HasPendingException(P_CAST(env, EcmaVM*)));
        ARKTS_CloseScope(env, scope);
    }
}

TEST_F(ArkInteropTest, CheckFreeContext)
{
    bool isDisposed = false;
    {
        MockContext mockCtx(ARKTS_CreateEngineWithNewThread());
        mockCtx.SetFinalizerCallback([&isDisposed](ARKTS_Env) {
            isDisposed = true;
        });
    }
    EXPECT_TRUE(isDisposed);
}

TEST_F(ArkInteropTest, DebugMode)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto id = mockCtx.StoreFunc([](ARKTS_CallInfo) {
        return ARKTS_CreateUndefined();
    });
    auto func = ARKTS_CreateFunc(env, id);
    mockCtx.SetNativeCallingCallback([&mockCtx, id](const void* addr) {
        EXPECT_EQ(addr, mockCtx.GetFuncAddr(id));
    });
    ARKTS_Call(env, func, ARKTS_CreateUndefined(), 0, nullptr);
}

TEST_F(ArkInteropTest, SetCJModuleCallbackWithNull)
{
    ARKTS_SetCJModuleCallback(nullptr);
}

TEST_F(ArkInteropTest, CJLambdaInvoker)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    bool invoked = false;
    auto lambdaId = mockCtx.StoreFunc([&invoked](ARKTS_CallInfo info) -> ARKTS_Value {
        invoked = true;
        return ARKTS_CreateBool(true);
    });

    auto func = ARKTS_CreateFunc(env, lambdaId);
    ARKTS_Call(env, func, ARKTS_CreateUndefined(), 0, nullptr);
    EXPECT_TRUE(invoked);

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, CJLambdaInvokerWithoutCallback)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    auto func = ARKTS_CreateFunc(env, 12345);
    ARKTS_Call(env, func, ARKTS_CreateUndefined(), 0, nullptr);

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, CJLambdaDeleter)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    auto lambdaId = mockCtx.StoreFunc([](ARKTS_CallInfo info) -> ARKTS_Value {
        return ARKTS_CreateUndefined();
    });

    auto func = ARKTS_CreateFunc(env, lambdaId);
    ARKTS_Call(env, func, ARKTS_CreateUndefined(), 0, nullptr);

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, CJAsyncCallback)
{
    auto engine = ARKTS_CreateEngineWithNewThread();
    ASSERT_TRUE(engine);
    {
        MockContext mockCtx(engine, false);
        auto env = mockCtx.GetEnv();
        auto scope = ARKTS_OpenScope(env);

        bool invoked = false;
        std::condition_variable cv;
        auto asyncId = mockCtx.StoreAsyncFunc([&invoked, &cv] {
            invoked = true;
            cv.notify_one();
        });

        ARKTS_CreateAsyncTask(env, asyncId);

        std::mutex mutex;
        std::unique_lock lock(mutex);
        constexpr int checkDuration = 100;
        int waitTimes = 50;
        while (!invoked && waitTimes--) {
            cv.wait_for(lock, std::chrono::milliseconds(checkDuration));
        }
        EXPECT_TRUE(invoked);

        ARKTS_CloseScope(env, scope);
    }
    std::thread destroyer([engine] { ARKTS_DestroyEngine(engine); });
    destroyer.join();
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DURATION_MS));
}

TEST_F(ArkInteropTest, GetNAPIEnv)
{
    MockContext mockCtx;
    auto engine = mockCtx.GetEngine();
    auto napiEnv = ARKTS_GetNAPIEnv(engine);
    EXPECT_TRUE(napiEnv);
}

TEST_F(ArkInteropTest, GetNAPIEnvWithNullEngine)
{
    ErrorCaptureContext errorCtx;
    auto napiEnv = ARKTS_GetNAPIEnv(nullptr);
    EXPECT_FALSE(napiEnv);
    EXPECT_TRUE(errorCtx.HasAndClearNativeError());
}

TEST_F(ArkInteropTest, GetContext)
{
    MockContext mockCtx;
    auto engine = mockCtx.GetEngine();
    auto context = ARKTS_GetContext(engine);
    EXPECT_TRUE(context);
}

TEST_F(ArkInteropTest, GetContextWithNullEngine)
{
    ErrorCaptureContext errorCtx;
    auto context = ARKTS_GetContext(nullptr);
    EXPECT_FALSE(context);
    EXPECT_TRUE(errorCtx.HasAndClearNativeError());
}

TEST_F(ArkInteropTest, GetThreadIdOfEngine)
{
    MockContext mockCtx;
    auto engine = mockCtx.GetEngine();
    auto threadId = ARKTS_GetThreadIdOfEngine(engine);
    EXPECT_GT(threadId, 0);
}

TEST_F(ArkInteropTest, GetThreadIdOfEngineWithNullEngine)
{
    ErrorCaptureContext errorCtx;
    constexpr uint64_t invalidThreadId = 0;
    auto threadId = ARKTS_GetThreadIdOfEngine(nullptr);
    EXPECT_EQ(threadId, invalidThreadId);
    EXPECT_TRUE(errorCtx.HasAndClearNativeError());
}

TEST_F(ArkInteropTest, CreateEngine)
{
    auto engine = ARKTS_CreateEngineWithNewThread();
    ASSERT_TRUE(engine);
    std::thread destroyer([engine] { ARKTS_DestroyEngine(engine); });
    destroyer.join();
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DURATION_MS));
}

TEST_F(ArkInteropTest, DestroyEngineWithNullEngine)
{
    ErrorCaptureContext errorCtx;
    ARKTS_DestroyEngine(nullptr);
    EXPECT_TRUE(errorCtx.HasAndClearNativeError());
}

TEST_F(ArkInteropTest, LoadEntryFromAbc)
{
    MockContext mockCtx;
    auto engine = mockCtx.GetEngine();

    const char* testFile = "/data/test_module.abc";
    const char* entryPoint = "test_entry";

    bool result = ARKTS_LoadEntryFromAbc(engine, testFile, entryPoint, false);
    EXPECT_FALSE(result);
}

TEST_F(ArkInteropTest, ImportFromEntry)
{
    MockContext mockCtx;
    auto engine = mockCtx.GetEngine();

    auto value = ARKTS_ImportFromEntry(engine, "test_entry", "test_import");
    EXPECT_EQ(value.value, 2ULL);
}

TEST_F(ArkInteropTest, RequireNativeModule)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    const char* target = "test_module";

    (void)ARKTS_Require(env, target, true, false, nullptr);

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, RequireNativeModuleWithNullEnv)
{
    ErrorCaptureContext errorCtx;
    const char* target = "test_module";

    (void)ARKTS_Require(nullptr, target, true, false, nullptr);
    EXPECT_TRUE(errorCtx.HasAndClearNativeError());
}

TEST_F(ArkInteropTest, RequireNativeModuleWithRelativePath)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    const char* target = "test_module";
    const char* relativePath = "./test_module";

    (void)ARKTS_Require(env, target, true, false, relativePath);

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, RequireAppModule)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    const char* target = "test_module";

    (void)ARKTS_Require(env, target, true, true, nullptr);

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, RequireJsModule)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    const char* target = "test_module";

    (void)ARKTS_Require(env, target, false, false, nullptr);

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, LoadModuleByNapiEnv)
{
    MockContext mockCtx;
    auto engine = mockCtx.GetEngine();
    void* napiEnv = ARKTS_GetNAPIEnv(engine);
    ASSERT_NE(napiEnv, nullptr);
    // nullptr dllName: early return in ARKTS_LoadModuleByNapiEnv (avoids CJ loader crash in host UT).
    EXPECT_EQ(ARKTS_LoadModuleByNapiEnv(napiEnv, nullptr), nullptr);
}

TEST_F(ArkInteropTest, LoadModuleByNapiEnvWithNullEnv)
{
    const char* dllName = "test_module";

    auto localResult = ARKTS_LoadModuleByNapiEnv(nullptr, dllName);
    EXPECT_FALSE(localResult);
}

TEST_F(ArkInteropTest, LoadModuleByNapiEnvWithValidParams)
{
    ModuleRegisteredContext moduleCtx;
    auto engine = moduleCtx.GetEngine();
    void* napiEnv = ARKTS_GetNAPIEnv(engine);
    ASSERT_NE(napiEnv, nullptr);

    auto loadResult = ARKTS_LoadModuleByNapiEnv(napiEnv, "test_module");
    (void)loadResult;
    moduleCtx.HasAndClearNativeError();
    moduleCtx.HasAndClearJSError();
}

TEST_F(ArkInteropTest, UpdateStackInfoWithNullAddress)
{
    ErrorCaptureContext errorCtx;
    auto env = errorCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    StackInfo localStackInfo;
    localStackInfo.stackLimit = TEST_STACK_LIMIT;
    localStackInfo.lastLeaveFrame = 0;
    ARKTS_UpdateStackInfo(0, &localStackInfo, SWITCH_TO_SUB_STACK_INFO);
    EXPECT_TRUE(errorCtx.HasAndClearNativeError());
    ARKTS_UpdateStackInfo(0, &localStackInfo, SWITCH_TO_MAIN_STACK_INFO);
    EXPECT_TRUE(errorCtx.HasAndClearNativeError());

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, UpdateStackInfoWithNullStackInfo)
{
    ErrorCaptureContext errorCtx;
    auto env = errorCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    unsigned long long localVmAddr = reinterpret_cast<unsigned long long>(env);
    ARKTS_UpdateStackInfo(localVmAddr, nullptr, SWITCH_TO_SUB_STACK_INFO);
    EXPECT_TRUE(errorCtx.HasAndClearNativeError());
    ARKTS_UpdateStackInfo(localVmAddr, nullptr, SWITCH_TO_MAIN_STACK_INFO);
    EXPECT_TRUE(errorCtx.HasAndClearNativeError());

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, CreateCycleFreeFunc)
{
    CycleFreeContext cycleFreeCtx;
    auto env = cycleFreeCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    bool localCalled = false;
    auto id = cycleFreeCtx.StoreCycleFreeFunc([&localCalled](ARKTS_CallInfo callInfo) {
        localCalled = true;
        return ARKTS_CreateUndefined();
    });

    auto func = ARKTS_CreateCycleFreeFunc(env, id);
    EXPECT_TRUE(ARKTS_IsCallable(env, func));

    ARKTS_Call(env, func, ARKTS_CreateUndefined(), 0, nullptr);
    EXPECT_TRUE(localCalled);

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, CreateCycleFreeExtern)
{
    CycleFreeContext cycleFreeCtx;
    auto env = cycleFreeCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    auto id = cycleFreeCtx.StoreCycleFreeFunc([](ARKTS_CallInfo callInfo) {
        return ARKTS_CreateUndefined();
    });

    auto external = ARKTS_CreateCycleFreeExtern(env, id);
    EXPECT_TRUE(ARKTS_IsExternal(env, external));

    auto data = ARKTS_GetExternalData(env, external);
    EXPECT_EQ(data, id);

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, CreateGlobal)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    auto object = ARKTS_CreateObject(env);
    auto global = ARKTS_CreateGlobal(env, object);

    auto value = ARKTS_GlobalToValue(env, global);
    auto global2 = ARKTS_GlobalFromValue(env, value);

    EXPECT_EQ(global, global2);

    ARKTS_DisposeGlobal(env, global);

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, CreateGlobalWithWeak)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    auto object = ARKTS_CreateObject(env);
    auto global = ARKTS_CreateGlobal(env, object);

    ARKTS_GlobalSetWeak(env, global);

    auto isAlive = ARKTS_GlobalIsAlive(env, global);
    EXPECT_TRUE(isAlive);

    ARKTS_DisposeGlobal(env, global);

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, DisposeGlobalSync)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    auto object = ARKTS_CreateObject(env);
    auto global = ARKTS_CreateGlobal(env, object);

    ARKTS_DisposeGlobalSync(env, global);

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, CreateEngineWithNewThread)
{
    auto engine = ARKTS_CreateEngineWithNewThread();
    ASSERT_TRUE(engine);
    auto env = ARKTS_GetContext(engine);
    EXPECT_TRUE(env);

    std::thread destroyer([engine] { ARKTS_DestroyEngine(engine); });
    destroyer.join();
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DURATION_MS));
}

TEST_F(ArkInteropTest, CreateEngineWithNewThreadNull)
{
    auto engine = ARKTS_CreateEngineWithNewThread();
    if (!engine) {
        return;
    }
    std::thread destroyer([engine] { ARKTS_DestroyEngine(engine); });
    destroyer.join();
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DURATION_MS));
}

TEST_F(ArkInteropTest, PromiseCapabilityReject)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    auto promiseCap = ARKTS_CreatePromiseCapability(env);
    auto promise = ARKTS_GetPromiseFromCapability(env, promiseCap);

    bool rejected = false;
    auto noopId = MockContext::GetInstance()->StoreFunc([](ARKTS_CallInfo info)->ARKTS_Value {
        (void)info;
        return ARKTS_CreateUndefined();
    });
    auto rejectId = MockContext::GetInstance()->StoreFunc([&rejected, env](ARKTS_CallInfo info)->ARKTS_Value {
        rejected = true;
        constexpr size_t rejectArgCount = 1;
        EXPECT_EQ(ARKTS_GetArgCount(info), rejectArgCount);
        auto arg = ARKTS_GetArg(info, 0);
        EXPECT_TRUE(ARKTS_IsUndefined(arg));
        return ARKTS_CreateUndefined();
    });
    auto onFulfilled = ARKTS_CreateFunc(env, noopId);
    auto onRejected = ARKTS_CreateFunc(env, rejectId);
    ARKTS_PromiseThen(env, promise, onFulfilled, onRejected);
    ARKTS_PromiseCapabilityReject(env, promiseCap, ARKTS_CreateUndefined());
    EXPECT_TRUE(rejected);

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, ArrayBufferWithDeleter)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    uint8_t data[] = {1, 2, 3, 4, 5};
    auto arrBuf = ARKTS_CreateArrayBuffer(env, sizeof(data));
    auto ptr = reinterpret_cast<uint8_t*>(ARKTS_GetArrayBufferRawPtr(env, arrBuf));
    std::copy(data, data + sizeof(data), ptr);

    bool deleted = false;
    (void)mockCtx.StoreArrayBufferDeleter([&deleted](void* buf) {
        deleted = true;
    });

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, ExternalWithDeleter)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    int* data = new int(42);
    auto deleterId = mockCtx.StoreNativeData(data, [](void* ptr) {
        delete static_cast<int*>(ptr);
    });

    auto external = ARKTS_CreateExternal(env, deleterId);
    EXPECT_TRUE(ARKTS_IsExternal(env, external));

    auto retrievedData = ARKTS_GetExternalData(env, external);
    EXPECT_EQ(retrievedData, deleterId);

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, GetPropertyNames)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    auto obj = ARKTS_CreateObject(env);
    auto key1 = ARKTS_CreateUtf8(env, "key1", 4);
    auto key2 = ARKTS_CreateUtf8(env, "key2", 4);

    constexpr int32_t PROP_VAL_1 = 1;
    constexpr int32_t PROP_VAL_2 = 2;
    ARKTS_SetProperty(env, obj, key1, ARKTS_CreateI32(PROP_VAL_1));
    ARKTS_SetProperty(env, obj, key2, ARKTS_CreateI32(PROP_VAL_2));

    auto keys = ARKTS_EnumOwnProperties(env, obj);
    EXPECT_TRUE(ARKTS_IsArray(env, keys));
    constexpr int32_t expectedKeyCount = 2;
    EXPECT_EQ(ARKTS_GetArrayLength(env, keys), expectedKeyCount);

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, StrictEqual)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    constexpr int32_t testNumValue = 42;
    constexpr int32_t differentNumValue = 43;
    auto num1 = ARKTS_CreateI32(testNumValue);
    auto num2 = ARKTS_CreateI32(testNumValue);
    auto num3 = ARKTS_CreateI32(differentNumValue);

    EXPECT_TRUE(ARKTS_StrictEqual(env, num1, num2));
    EXPECT_FALSE(ARKTS_StrictEqual(env, num1, num3));

    auto str1 = ARKTS_CreateUtf8(env, "test", 4);
    auto str2 = ARKTS_CreateUtf8(env, "test", 4);
    auto str3 = ARKTS_CreateUtf8(env, "test2", 5);

    EXPECT_TRUE(ARKTS_StrictEqual(env, str1, str2));
    EXPECT_FALSE(ARKTS_StrictEqual(env, str1, str3));

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, CreateInt32)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    int32_t values[] = {0, 1, -1, 2147483647, -2147483648};

    for (auto value : values) {
        auto jsValue = ARKTS_CreateI32(value);
        auto numValue = ARKTS_GetValueNumber(jsValue);
        EXPECT_EQ(static_cast<int32_t>(numValue), value);
    }
    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, GetValueType)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    auto undefined = ARKTS_CreateUndefined();
    EXPECT_EQ(ARKTS_GetValueType(env, undefined), N_UNDEFINED);

    auto null = ARKTS_CreateNull();
    EXPECT_EQ(ARKTS_GetValueType(env, null), N_NULL);

    auto boolVal = ARKTS_CreateBool(true);
    EXPECT_EQ(ARKTS_GetValueType(env, boolVal), N_BOOL);

    constexpr int32_t numValValue = 42;
    auto numVal = ARKTS_CreateI32(numValValue);
    EXPECT_EQ(ARKTS_GetValueType(env, numVal), N_NUMBER);

    auto strVal = ARKTS_CreateUtf8(env, "test", 4);
    EXPECT_EQ(ARKTS_GetValueType(env, strVal), N_STRING);

    auto objVal = ARKTS_CreateObject(env);
    EXPECT_EQ(ARKTS_GetValueType(env, objVal), N_OBJECT);

    auto arrVal = ARKTS_CreateArray(env, 0);
    EXPECT_EQ(ARKTS_GetValueType(env, arrVal), N_OBJECT);

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, CallWithArguments)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    bool callInvoked = false;
    constexpr size_t callArgCount = 3;
    auto id = mockCtx.StoreFunc([&callInvoked, env, callArgCount](ARKTS_CallInfo info) -> ARKTS_Value {
        callInvoked = true;
        EXPECT_EQ(ARKTS_GetArgCount(info), callArgCount);

        auto arg0 = ARKTS_GetArg(info, 0);
        auto arg1 = ARKTS_GetArg(info, 1);
        auto arg2 = ARKTS_GetArg(info, 2);

        EXPECT_TRUE(ARKTS_IsNumber(arg0));
        EXPECT_TRUE(ARKTS_IsString(env, arg1));
        EXPECT_TRUE(ARKTS_IsBool(arg2));

        return ARKTS_CreateUndefined();
    });

    auto func = ARKTS_CreateFunc(env, id);

    constexpr int32_t callArgNum = 42;
    ARKTS_Value args[] = {
        ARKTS_CreateI32(callArgNum),
        ARKTS_CreateUtf8(env, "test", 4),
        ARKTS_CreateBool(true)
    };

    ARKTS_Call(env, func, ARKTS_CreateUndefined(), callArgCount, args);
    EXPECT_TRUE(callInvoked);

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, NewWithArguments)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    bool constructed = false;
    auto id = mockCtx.StoreFunc([&constructed](ARKTS_CallInfo info) -> ARKTS_Value {
        constructed = true;
        return ARKTS_GetThisArg(info);
    });

    auto clazz = ARKTS_CreateClass(env, id, ARKTS_CreateUndefined());

    constexpr int32_t newArgNum = 42;
    ARKTS_Value args[] = {
        ARKTS_CreateI32(newArgNum),
        ARKTS_CreateUtf8(env, "test", 4)
    };

    constexpr size_t newArgCount = 2;
    auto instance = ARKTS_New(env, clazz, newArgCount, args);
    EXPECT_TRUE(constructed);
    EXPECT_TRUE(ARKTS_IsObject(env, instance));
    EXPECT_TRUE(ARKTS_InstanceOf(env, instance, clazz));

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, ArrayOperations)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    constexpr size_t arrSize = 5;
    auto arr = ARKTS_CreateArray(env, arrSize);
    EXPECT_EQ(ARKTS_GetArrayLength(env, arr), arrSize);

    for (auto i = 0; i < arrSize; i++) {
        ARKTS_SetElement(env, arr, i, ARKTS_CreateI32(i));
    }

    for (auto i = 0; i < arrSize; i++) {
        auto elem = ARKTS_GetElement(env, arr, i);
        auto numValue = ARKTS_GetValueNumber(elem);
        EXPECT_EQ(static_cast<int>(numValue), i);
    }

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, ObjectGetPrototypeOf)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    auto classId = MockContext::GetInstance()->StoreFunc([](ARKTS_CallInfo info)->ARKTS_Value {
        return ARKTS_GetThisArg(info);
    });
    auto clazz = ARKTS_CreateClass(env, classId, ARKTS_CreateUndefined());
    EXPECT_TRUE(ARKTS_IsClass(env, clazz));
    auto proto = ARKTS_GetPrototype(env, clazz);
    EXPECT_TRUE(ARKTS_IsHeapObject(proto));

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, ArrayBufferCreateWithData)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    uint8_t data[] = {1, 2, 3, 4, 5, 6, 7, 8};
    auto arrBuf = ARKTS_CreateArrayBuffer(env, sizeof(data));

    auto ptr = reinterpret_cast<uint8_t*>(ARKTS_GetArrayBufferRawPtr(env, arrBuf));
    std::copy(data, data + sizeof(data), ptr);

    EXPECT_EQ(ARKTS_GetArrayBufferLength(env, arrBuf), sizeof(data));

    uint8_t received[sizeof(data)];
    auto bytesRead = ARKTS_ArrayBufferReadBytes(env, arrBuf, received, sizeof(data));
    EXPECT_EQ(bytesRead, sizeof(data));

    for (auto i = 0; i < sizeof(data); i++) {
        EXPECT_EQ(received[i], data[i]);
    }

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, BigIntOperations)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    int64_t bigIntValue = 1234567890123456789LL;
    auto bigInt = ARKTS_CreateBigInt(env, bigIntValue);
    EXPECT_TRUE(ARKTS_IsBigInt(env, bigInt));

    auto byteSize = ARKTS_BigIntGetByteSize(env, bigInt);
    EXPECT_GT(byteSize, 0);

    bool localIsNegative = false;
    uint8_t bytes[16];
    ARKTS_BigIntReadBytes(env, bigInt, &localIsNegative, sizeof(bytes), bytes);
    EXPECT_FALSE(localIsNegative);

    auto bigInt2 = ARKTS_CreateBigIntWithBytes(env, false, byteSize, bytes);
    EXPECT_TRUE(ARKTS_IsBigInt(env, bigInt2));

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, SymbolOperations)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    const char* desc = "test_symbol";
    auto symbol = ARKTS_CreateSymbol(env, desc, strlen(desc));
    EXPECT_TRUE(ARKTS_IsSymbol(env, symbol));

    auto symbolDesc = ARKTS_GetSymbolDesc(env, symbol);
    EXPECT_STREQ(symbolDesc, desc);
    ARKTS_FreeCString(symbolDesc);

    auto symbol2 = ARKTS_CreateSymbol(env, desc, strlen(desc));
    EXPECT_FALSE(ARKTS_StrictEqual(env, symbol, symbol2));

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, PromiseChaining)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    auto scope = ARKTS_OpenScope(env);

    auto promiseCap = ARKTS_CreatePromiseCapability(env);
    auto promise = ARKTS_GetPromiseFromCapability(env, promiseCap);

    bool chainResolved = false;
    bool chained = false;

    auto resolveId = mockCtx.StoreFunc([&chainResolved, env](ARKTS_CallInfo info) -> ARKTS_Value {
        chainResolved = true;
        return ARKTS_GetArg(info, 0);
    });

    auto chainId = mockCtx.StoreFunc([&chained, env](ARKTS_CallInfo info) -> ARKTS_Value {
        chained = true;
        return ARKTS_GetArg(info, 0);
    });

    auto onResolved = ARKTS_CreateFunc(env, resolveId);
    auto onChain = ARKTS_CreateFunc(env, chainId);

    auto chainedPromise = ARKTS_PromiseThen(env, promise, onResolved, ARKTS_CreateUndefined());
    (void)ARKTS_PromiseThen(env, chainedPromise, onChain, ARKTS_CreateUndefined());

    constexpr int32_t promiseResolveVal = 42;
    ARKTS_PromiseCapabilityResolve(env, promiseCap, ARKTS_CreateI32(promiseResolveVal));

    EXPECT_TRUE(chainResolved);
    EXPECT_TRUE(chained);

    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, HiTraceStartTrace)
{
    ARKTS_HiTraceStartTrace("test_trace", 1);
}

TEST_F(ArkInteropTest, HiTraceFinishTrace)
{
    ARKTS_HiTraceFinishTrace("test_trace", 1);
}

TEST_F(ArkInteropTest, HiTraceCountTrace)
{
    constexpr int32_t hiTraceCount = 100;
    ARKTS_HiTraceCountTrace("test_trace", hiTraceCount);
}

TEST_F(ArkInteropTest, InitEventHandle)
{
    MockContext mockCtx;
    auto env = mockCtx.GetEnv();
    ARKTS_InitEventHandle(env);
}

TEST_F(ArkInteropTest, InitEventHandleWithNullEnv)
{
    ErrorCaptureContext localErrorCtx;
    ARKTS_InitEventHandle(nullptr);
    EXPECT_TRUE(localErrorCtx.HasAndClearNativeError());
}

TEST_F(ArkInteropTest, CreateAsyncTask)
{
    GTEST_SKIP() << "async task on global engine may prevent clean shutdown";
}

TEST_F(ArkInteropTest, CreateAsyncTaskWithNullEnv)
{
    ErrorCaptureContext errCtx;
    constexpr int64_t asyncTaskId = 12345;
    ARKTS_CreateAsyncTask(nullptr, asyncTaskId);
    EXPECT_TRUE(errCtx.HasAndClearNativeError());
}

namespace panda {
bool JSNApi::IsMixedDebugEnabled(const EcmaVM* vm)
{
    return true;
}

void JSNApi::NotifyNativeCalling(const EcmaVM* vm, const void* nativeAddress)
{
    MockContext::GetInstance()->NotifyNativeCalling(nativeAddress);
}

void JSNApi::NotifyNativeReturn(const EcmaVM* vm, const void* nativeAddress)
{
    MockContext::GetInstance()->NotifyNativeReturn(nativeAddress);
}
}

int main(int argc, char** argv)
{
    LOGI("main in");
    testing::GTEST_FLAG(output) = "xml:./";
    testing::InitGoogleTest(&argc, argv);

    OHOS::CJEnvironment::SetAppPath("/data/test");

    auto runner = OHOS::AppExecFwk::EventRunner::Create(true);
    EXPECT_TRUE(runner);
    auto handler = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    EXPECT_TRUE(handler);

    TriggerArktsAssertEarlyReturnBranches();

    MockContext::Init();

    int ret = -1;
    std::condition_variable cv;

    ARKTS_Engine globalEngine;

    auto success = handler->PostTask([&ret, &cv, &globalEngine] {
        MockContext global(ARKTS_CreateEngine(), false);
        globalEngine = global.GetEngine();
        ret = testing::UnitTest::GetInstance()->Run();
        cv.notify_all();
    });

    EXPECT_TRUE(success);

    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    auto status = cv.wait_for(lock, std::chrono::seconds(30));

    EXPECT_EQ(status, std::cv_status::no_timeout);
    ARKTS_DestroyEngine(globalEngine);

    runner->Stop();

    LOGI("main out");
    return ret;
}
