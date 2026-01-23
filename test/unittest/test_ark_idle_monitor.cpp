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
#include "test.h"
#include "test_common.h"
#include "utils/log.h"

using ArkIdleMonitor = panda::ecmascript::ArkIdleMonitor;
using panda::RuntimeOption;
using ReportDataFunc = void (*)(uint32_t resType, int64_t value,
    const std::unordered_map<std::string, std::string>& payload);

class NapiArkIdleMonitorTest : public NativeEngineTest {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "NapiErrorCodeTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "NapiErrorCodeTest TearDownTestCase";
    }

    void SetUp() override
    {
        napi_env env = reinterpret_cast<napi_env>(engine_);
        napi_open_handle_scope(env, &scope_);
    }

    void TearDown() override
    {
        napi_env env = reinterpret_cast<napi_env>(engine_);
        napi_value exception = nullptr;
        napi_get_and_clear_last_exception(env, &exception);
        napi_close_handle_scope(env, scope_);
    }
private:
    napi_handle_scope scope_ = nullptr;
};


/**
 * @tc.name: SetMainThreadEcmaVMTest
 * @tc.desc: Test interface of ArkIdleMonitor
 * @tc.type: FUNC
 */
HWTEST_F(NapiArkIdleMonitorTest, SetMainThreadEcmaVMTest001, testing::ext::TestSize.Level0)
{
    ASSERT_NE(engine_, nullptr);
    EcmaVM *vm = const_cast<EcmaVM*>(reinterpret_cast<ArkNativeEngine*>(engine_)->GetEcmaVm());

    auto arkIdleMonitor = ArkIdleMonitor::GetInstance();
    arkIdleMonitor->SetMainThreadEcmaVM(vm);

    int64_t oldIdleDuration = arkIdleMonitor->GetTotalIdleDuration();
    for (int i = 0; i < 10; i++) {
        arkIdleMonitor->NotifyLooperIdleStart(i * 500, 20);
        ASSERT_EQ(arkIdleMonitor->IsIdleState(), true);
        arkIdleMonitor->NotifyLooperIdleEnd(i * 500 + 250);
        ASSERT_EQ(arkIdleMonitor->IsIdleState(), false);
    }

    arkIdleMonitor->SetMainThreadEcmaVM(nullptr);

    for (int i = 10; i < 20; i++) {
        arkIdleMonitor->NotifyLooperIdleStart(i * 500, 20);
        ASSERT_EQ(arkIdleMonitor->IsIdleState(), true);
        arkIdleMonitor->NotifyLooperIdleEnd(i * 500 + 250);
        ASSERT_EQ(arkIdleMonitor->IsIdleState(), false);
    }

    int64_t newIdleDuration = arkIdleMonitor->GetTotalIdleDuration();
    ASSERT_TRUE(newIdleDuration > oldIdleDuration);
    ASSERT_TRUE(arkIdleMonitor->GetIdleNotifyCount() > 0);
}

/**
 * @tc.name: NotifyChangeBackgroundStateTest
 * @tc.desc: Test interface of ArkIdleMonitor
 * @tc.type: FUNC
 */
HWTEST_F(NapiArkIdleMonitorTest, NotifyChangeBackgroundStateTest001, testing::ext::TestSize.Level0)
{
    ASSERT_NE(engine_, nullptr);
    EcmaVM *vm = const_cast<EcmaVM*>(reinterpret_cast<ArkNativeEngine*>(engine_)->GetEcmaVm());

    auto arkIdleMonitor = ArkIdleMonitor::GetInstance();
    arkIdleMonitor->SetMainThreadEcmaVM(vm);
    arkIdleMonitor->SetStartTimerCallback();
    arkIdleMonitor->NotifyChangeBackgroundState(true);
    ASSERT_TRUE(arkIdleMonitor->IsInBackground() == true);
    arkIdleMonitor->NotifyChangeBackgroundState(false);
    ASSERT_TRUE(arkIdleMonitor->IsInBackground() == false);
}

/**
 * @tc.name: RegisterWorkerEnvTest
 * @tc.desc: Test interface of ArkIdleMonitor
 * @tc.type: FUNC
 */
HWTEST_F(NapiArkIdleMonitorTest, RegisterWorkerEnvTest001, testing::ext::TestSize.Level0)
{
    ASSERT_NE(engine_, nullptr);
    EcmaVM *vm = const_cast<EcmaVM*>(reinterpret_cast<ArkNativeEngine*>(engine_)->GetEcmaVm());

    auto arkIdleMonitor = ArkIdleMonitor::GetInstance();
    arkIdleMonitor->SetMainThreadEcmaVM(vm);

    RuntimeOption option;
    option.SetGcType(RuntimeOption::GC_TYPE::GEN_GC);
    const int64_t poolSize = 0x1000000;  // 16M
    option.SetGcPoolSize(poolSize);
    option.SetLogLevel(RuntimeOption::LOG_LEVEL::ERROR);
    option.SetDebuggerLibraryPath("");
    auto workerVM1 = panda::JSNApi::CreateJSVM(option);
    auto workerVM2 = panda::JSNApi::CreateJSVM(option);
    ArkNativeEngine *workerEngine1 = new ArkNativeEngine(workerVM1, nullptr);
    ArkNativeEngine *workerEngine2 = new ArkNativeEngine(workerVM2, nullptr);
    napi_env wokerEnv1 = reinterpret_cast<napi_env>(workerEngine1);
    napi_env wokerEnv2 = reinterpret_cast<napi_env>(workerEngine2);

    arkIdleMonitor->UnregisterWorkerEnv(wokerEnv1);
    arkIdleMonitor->UnregisterWorkerEnv(wokerEnv2);

    arkIdleMonitor->RegisterWorkerEnv(wokerEnv1);
    arkIdleMonitor->RegisterWorkerEnv(wokerEnv2);

    arkIdleMonitor->UnregisterWorkerEnv(wokerEnv1);
    arkIdleMonitor->UnregisterWorkerEnv(wokerEnv2);

    delete workerEngine1;
    delete workerEngine2;
}

/**
 * @tc.name: IdleMonitorDeferfreezeTest001
 * @tc.desc: Test idle Monitor deferfreeze
 * @tc.type: FUNC
 */
HWTEST_F(NapiArkIdleMonitorTest, IdleMonitorDeferfreezeTest001, testing::ext::TestSize.Level0)
{
    ASSERT_NE(engine_, nullptr);
    EcmaVM *vm = const_cast<EcmaVM*>(reinterpret_cast<ArkNativeEngine*>(engine_)->GetEcmaVm());

    auto arkIdleMonitor = ArkIdleMonitor::GetInstance();
    arkIdleMonitor->SetMainThreadEcmaVM(vm);

    arkIdleMonitor->NotifyChangeBackgroundState(true);
    ASSERT_TRUE(arkIdleMonitor->IsInBackground());
    sleep(1);

    ASSERT_FALSE(arkIdleMonitor->IsDeferfreeze());
    ASSERT_FALSE(arkIdleMonitor->IsSwitchToBackgroundTask());

    arkIdleMonitor->NotifyChangeBackgroundState(false);
    ASSERT_FALSE(arkIdleMonitor->IsInBackground());

    ASSERT_FALSE(arkIdleMonitor->IsDeferfreeze());
    ASSERT_FALSE(arkIdleMonitor->IsSwitchToBackgroundTask());
}

/**
 * @tc.name: IdleMonitorDeferfreezeTest002
 * @tc.desc: ban persist.ark.enableidlegc
 * @tc.type: FUNC
 */
HWTEST_F(NapiArkIdleMonitorTest, IdleMonitorDeferfreezeTest002, testing::ext::TestSize.Level0)
{
    ASSERT_NE(engine_, nullptr);
    EcmaVM *vm = const_cast<EcmaVM*>(reinterpret_cast<ArkNativeEngine*>(engine_)->GetEcmaVm());

    auto arkIdleMonitor = ArkIdleMonitor::GetInstance();
    arkIdleMonitor->SetMainThreadEcmaVM(vm);

    arkIdleMonitor->SetEnableDeferFreeze(false);
    ASSERT_FALSE(arkIdleMonitor->IsEnableDeferFreeze());

    arkIdleMonitor->NotifyNeedFreeze(false);
    ASSERT_FALSE(arkIdleMonitor->IsDeferfreeze());

    arkIdleMonitor->SetSwitchToBackgroundTask(true);
    ASSERT_TRUE(arkIdleMonitor->IsSwitchToBackgroundTask());
    arkIdleMonitor->NotifyNeedFreeze(false);
    ASSERT_FALSE(arkIdleMonitor->IsDeferfreeze());

    arkIdleMonitor->SetDeferfreeze(false);
    ASSERT_FALSE(arkIdleMonitor->IsDeferfreeze());
    arkIdleMonitor->NotifyNeedFreeze(false);
    ASSERT_FALSE(arkIdleMonitor->IsDeferfreeze());
}

/**
 * @tc.name: IdleMonitorDeferfreezeTest003
 * @tc.desc: Normal function
 * @tc.type: FUNC
 */
HWTEST_F(NapiArkIdleMonitorTest, IdleMonitorDeferfreezeTest003, testing::ext::TestSize.Level0)
{
    ASSERT_NE(engine_, nullptr);
    EcmaVM *vm = const_cast<EcmaVM*>(reinterpret_cast<ArkNativeEngine*>(engine_)->GetEcmaVm());

    auto arkIdleMonitor = ArkIdleMonitor::GetInstance();
    arkIdleMonitor->SetMainThreadEcmaVM(vm);

    arkIdleMonitor->SetEnableDeferFreeze(true);
    ASSERT_TRUE(arkIdleMonitor->IsEnableDeferFreeze());

    arkIdleMonitor->SetSwitchToBackgroundTask(true);
    ASSERT_TRUE(arkIdleMonitor->IsSwitchToBackgroundTask());
    arkIdleMonitor->SetDeferfreeze(false);
    ASSERT_FALSE(arkIdleMonitor->IsDeferfreeze());

    arkIdleMonitor->NotifyNeedFreeze(false);
    ASSERT_TRUE(arkIdleMonitor->IsDeferfreeze());

    arkIdleMonitor->NotifyNeedFreeze(true);
    ASSERT_FALSE(arkIdleMonitor->IsDeferfreeze());
    ASSERT_FALSE(arkIdleMonitor->IsSwitchToBackgroundTask());
}

/**
 * @tc.name: IdleMonitorDeferfreezeTest004
 * @tc.desc: Turn it on multiple times
 * @tc.type: FUNC
 */
HWTEST_F(NapiArkIdleMonitorTest, IdleMonitorDeferfreezeTest004, testing::ext::TestSize.Level0)
{
    ASSERT_NE(engine_, nullptr);
    EcmaVM *vm = const_cast<EcmaVM*>(reinterpret_cast<ArkNativeEngine*>(engine_)->GetEcmaVm());

    auto arkIdleMonitor = ArkIdleMonitor::GetInstance();
    arkIdleMonitor->SetMainThreadEcmaVM(vm);

    arkIdleMonitor->SetEnableDeferFreeze(true);
    ASSERT_TRUE(arkIdleMonitor->IsEnableDeferFreeze());

    arkIdleMonitor->SetSwitchToBackgroundTask(true);
    ASSERT_TRUE(arkIdleMonitor->IsSwitchToBackgroundTask());
    arkIdleMonitor->SetDeferfreeze(false);
    ASSERT_FALSE(arkIdleMonitor->IsDeferfreeze());

    arkIdleMonitor->NotifyNeedFreeze(false);
    ASSERT_TRUE(arkIdleMonitor->IsDeferfreeze());

    arkIdleMonitor->NotifyNeedFreeze(false);
    ASSERT_TRUE(arkIdleMonitor->IsDeferfreeze());

    arkIdleMonitor->NotifyNeedFreeze(true);
    ASSERT_FALSE(arkIdleMonitor->IsDeferfreeze());
    ASSERT_FALSE(arkIdleMonitor->IsSwitchToBackgroundTask());
}

/**
 * @tc.name: IdleMonitorDeferfreezeTest005
 * @tc.desc: Closed multiple times
 * @tc.type: FUNC
 */
HWTEST_F(NapiArkIdleMonitorTest, IdleMonitorDeferfreezeTest005, testing::ext::TestSize.Level0)
{
    ASSERT_NE(engine_, nullptr);
    EcmaVM *vm = const_cast<EcmaVM*>(reinterpret_cast<ArkNativeEngine*>(engine_)->GetEcmaVm());

    auto arkIdleMonitor = ArkIdleMonitor::GetInstance();
    arkIdleMonitor->SetMainThreadEcmaVM(vm);

    arkIdleMonitor->SetEnableDeferFreeze(true);
    ASSERT_TRUE(arkIdleMonitor->IsEnableDeferFreeze());

    arkIdleMonitor->SetSwitchToBackgroundTask(true);
    ASSERT_TRUE(arkIdleMonitor->IsSwitchToBackgroundTask());
    arkIdleMonitor->SetDeferfreeze(false);
    ASSERT_FALSE(arkIdleMonitor->IsDeferfreeze());

    arkIdleMonitor->NotifyNeedFreeze(false);
    ASSERT_TRUE(arkIdleMonitor->IsDeferfreeze());

    arkIdleMonitor->NotifyNeedFreeze(true);
    ASSERT_FALSE(arkIdleMonitor->IsDeferfreeze());
    ASSERT_FALSE(arkIdleMonitor->IsSwitchToBackgroundTask());

    arkIdleMonitor->NotifyNeedFreeze(true);
    ASSERT_FALSE(arkIdleMonitor->IsDeferfreeze());
    ASSERT_FALSE(arkIdleMonitor->IsSwitchToBackgroundTask());
}


/**
 * @tc.name: IdleMonitorDeferfreezeTest006
 * @tc.desc: The background switch task has not been enabled
 * @tc.type: FUNC
 */
HWTEST_F(NapiArkIdleMonitorTest, IdleMonitorDeferfreezeTest006, testing::ext::TestSize.Level0)
{
    ASSERT_NE(engine_, nullptr);
    EcmaVM *vm = const_cast<EcmaVM*>(reinterpret_cast<ArkNativeEngine*>(engine_)->GetEcmaVm());

    auto arkIdleMonitor = ArkIdleMonitor::GetInstance();
    arkIdleMonitor->SetMainThreadEcmaVM(vm);

    arkIdleMonitor->SetEnableDeferFreeze(true);
    ASSERT_TRUE(arkIdleMonitor->IsEnableDeferFreeze());

    arkIdleMonitor->SetSwitchToBackgroundTask(false);
    ASSERT_FALSE(arkIdleMonitor->IsSwitchToBackgroundTask());
    arkIdleMonitor->SetDeferfreeze(false);
    ASSERT_FALSE(arkIdleMonitor->IsDeferfreeze());

    arkIdleMonitor->NotifyNeedFreeze(false);
    ASSERT_FALSE(arkIdleMonitor->IsDeferfreeze());

    arkIdleMonitor->NotifyNeedFreeze(true);
    ASSERT_FALSE(arkIdleMonitor->IsDeferfreeze());
    ASSERT_FALSE(arkIdleMonitor->IsSwitchToBackgroundTask());
}

/**
 * @tc.name: IdleMonitorNotifyLooperIdleStartTest001
 * @tc.desc: NotifyLooperIdleStart
 * @tc.type: FUNC
 */
HWTEST_F(NapiArkIdleMonitorTest, IdleMonitorNotifyLooperIdleStartTest001, testing::ext::TestSize.Level0)
{
    ASSERT_NE(engine_, nullptr);
    EcmaVM *vm = const_cast<EcmaVM*>(reinterpret_cast<ArkNativeEngine*>(engine_)->GetEcmaVm());

    auto arkIdleMonitor = ArkIdleMonitor::GetInstance();
    arkIdleMonitor->SetMainThreadEcmaVM(vm);

    int64_t oldCount = arkIdleMonitor->GetIdleNotifyCount();
    for (int i = 0; i < 10; i++) {
        arkIdleMonitor->NotifyLooperIdleStart(i * 500, 20);
        ASSERT_EQ(arkIdleMonitor->IsIdleState(), true);
        arkIdleMonitor->NotifyLooperIdleEnd(i * 500 + 250);
        ASSERT_EQ(arkIdleMonitor->IsIdleState(), false);
    }

    ASSERT_TRUE(arkIdleMonitor->GetIdleNotifyCount() == oldCount + 10);
}

/**
 * @tc.name: IdleMonitorGetNotifyTimestampTest001
 * @tc.desc: GetNotifyTimestamp
 * @tc.type: FUNC
 */
HWTEST_F(NapiArkIdleMonitorTest, IdleMonitorGetNotifyTimestampTest001, testing::ext::TestSize.Level0)
{
    ASSERT_NE(engine_, nullptr);
    EcmaVM *vm = const_cast<EcmaVM*>(reinterpret_cast<ArkNativeEngine*>(engine_)->GetEcmaVm());

    auto arkIdleMonitor = ArkIdleMonitor::GetInstance();
    arkIdleMonitor->SetMainThreadEcmaVM(vm);

    int64_t oldCount = arkIdleMonitor->GetIdleNotifyCount();
    for (int i = 0; i < 10; i++) {
        arkIdleMonitor->NotifyLooperIdleStart(i * 500, 20);
        ASSERT_EQ(arkIdleMonitor->GetNotifyTimestamp(), i * 500);
        ASSERT_EQ(arkIdleMonitor->IsIdleState(), true);
        arkIdleMonitor->NotifyLooperIdleEnd(i * 500 + 250);
        ASSERT_EQ(arkIdleMonitor->IsIdleState(), false);
    }

    ASSERT_TRUE(arkIdleMonitor->GetIdleNotifyCount() == oldCount + 10);
}

/**
 * @tc.name: IdleMonitorResetIdleNotifyCountTest001
 * @tc.desc: ResetIdleNotifyCount
 * @tc.type: FUNC
 */
HWTEST_F(NapiArkIdleMonitorTest, IdleMonitorResetIdleNotifyCountTest001, testing::ext::TestSize.Level0)
{
    ASSERT_NE(engine_, nullptr);
    EcmaVM *vm = const_cast<EcmaVM*>(reinterpret_cast<ArkNativeEngine*>(engine_)->GetEcmaVm());

    auto arkIdleMonitor = ArkIdleMonitor::GetInstance();
    arkIdleMonitor->SetMainThreadEcmaVM(vm);

    for (int i = 0; i < 10; i++) {
        arkIdleMonitor->NotifyLooperIdleStart(i * 500, 20);
        ASSERT_EQ(arkIdleMonitor->GetNotifyTimestamp(), i * 500);
        ASSERT_EQ(arkIdleMonitor->IsIdleState(), true);
        arkIdleMonitor->NotifyLooperIdleEnd(i * 500 + 250);
        ASSERT_EQ(arkIdleMonitor->IsIdleState(), false);
    }

    arkIdleMonitor->ResetIdleNotifyCount();
    ASSERT_EQ(arkIdleMonitor->GetIdleNotifyCount(), 0);
}

/**
 * @tc.name: IdleMonitorGetTotalIdleDurationTest001
 * @tc.desc: GetTotalIdleDuration
 * @tc.type: FUNC
 */
HWTEST_F(NapiArkIdleMonitorTest, IdleMonitorGetTotalIdleDurationTest001, testing::ext::TestSize.Level0)
{
    ASSERT_NE(engine_, nullptr);
    EcmaVM *vm = const_cast<EcmaVM*>(reinterpret_cast<ArkNativeEngine*>(engine_)->GetEcmaVm());

    auto arkIdleMonitor = ArkIdleMonitor::GetInstance();
    arkIdleMonitor->SetMainThreadEcmaVM(vm);

    arkIdleMonitor->ResetTotalIdleDuration();
    ASSERT_EQ(arkIdleMonitor->GetTotalIdleDuration(), 0);
    for (int i = 0; i < 10; i++) {
        arkIdleMonitor->NotifyLooperIdleStart(i * 500, 20);
        ASSERT_EQ(arkIdleMonitor->GetNotifyTimestamp(), i * 500);
        ASSERT_EQ(arkIdleMonitor->IsIdleState(), true);
        arkIdleMonitor->NotifyLooperIdleEnd(i * 500 + 250);
        ASSERT_EQ(arkIdleMonitor->IsIdleState(), false);
    }

    ASSERT_EQ(arkIdleMonitor->GetTotalIdleDuration(), 2500);
}

#if defined(ENABLE_EVENT_HANDLER)
/**
 * @tc.name: IdleMonitorEnableIdleGCTest001
 * @tc.desc: close idle gc
 * @tc.type: FUNC
 */
HWTEST_F(NapiArkIdleMonitorTest, IdleMonitorSetEnableIdleGCTest001, testing::ext::TestSize.Level0)
{
    ASSERT_NE(engine_, nullptr);

    auto arkIdleMonitor = ArkIdleMonitor::GetInstance();
    arkIdleMonitor->SetMainThreadEcmaVM(nullptr);

    ArkIdleMonitor::SetEnableIdleGC(false);
    ASSERT_FALSE(ArkIdleMonitor::IsEnableIdleGC());

    arkIdleMonitor->EnableIdleGC(engine_);
    ASSERT_FALSE(arkIdleMonitor->IsIdleMonitorStarted());
}

/**
 * @tc.name: IdleMonitorEnableIdleGCTest001
 * @tc.desc: close idle gc
 * @tc.type: FUNC
 */
HWTEST_F(NapiArkIdleMonitorTest, IdleMonitorSetEnableIdleGCTest002, testing::ext::TestSize.Level0)
{
    ASSERT_NE(engine_, nullptr);
    EcmaVM *vm = const_cast<EcmaVM*>(reinterpret_cast<ArkNativeEngine*>(engine_)->GetEcmaVm());

    auto arkIdleMonitor = ArkIdleMonitor::GetInstance();
    arkIdleMonitor->SetMainThreadEcmaVM(vm);

    ArkIdleMonitor::SetEnableIdleGC(true);
    ASSERT_TRUE(ArkIdleMonitor::IsEnableIdleGC());

    arkIdleMonitor->EnableIdleGC(engine_);
    ASSERT_TRUE(arkIdleMonitor->IsIdleMonitorStarted());
}
#endif


#if defined(LINUX_PLATFORM) || defined(OHOS_PLATFORM)
/**
 * @tc.name: IdleMonitorLoadReportDataFunc001
 * @tc.desc: load
 * @tc.type: FUNC
 */
HWTEST_F(NapiArkIdleMonitorTest, IdleMonitorLoadReportDataFunc001, testing::ext::TestSize.Level0)
{
    ASSERT_NE(engine_, nullptr);

    auto arkIdleMonitor = ArkIdleMonitor::GetInstance();
    ReportDataFunc func = arkIdleMonitor->LoadReportDataFunc();
    ASSERT_TRUE(func != nullptr);
}

#endif