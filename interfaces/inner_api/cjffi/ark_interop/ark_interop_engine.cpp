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

#include <string>
#include <unordered_map>

#include "ark_interop_internal.h"
#include "ark_interop_log.h"
#include "ark_interop_napi.h"
#include "ark_interop_external.h"
#include "jsnapi.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#ifdef __OHOS__
#include "uv_loop_handler.h"
#endif

struct ARKTS_Engine_ {
    panda::EcmaVM* vm;
    ArkNativeEngine* engine;
    std::unordered_map<std::string, std::string> loadedAbcs;
#ifdef __OHOS__
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> eventHandler;
#endif
};

#ifdef __OHOS__
static constexpr char TIMER_TASK[] = "uv_timer_task";

void UVLoopHandler::OnReadable(int32_t)
{
    OnTriggered();
}

void UVLoopHandler::OnWritable(int32_t)
{
    OnTriggered();
}

void UVLoopHandler::OnTriggered()
{
    uv_run(uvLoop_, UV_RUN_NOWAIT);
    auto eventHandler = GetOwner();
    if (!eventHandler) {
        return;
    }

    int32_t timeout = uv_backend_timeout(uvLoop_);
    if (timeout < 0) {
        if (haveTimerTask_) {
            eventHandler->RemoveTask(TIMER_TASK);
        }
        return;
    }

    int64_t timeStamp = static_cast<int64_t>(uv_now(uvLoop_)) + timeout;
    // we don't check timestamp in emulator for computer clock is inaccurate
#ifndef RUNTIME_EMULATOR
    if (timeStamp == lastTimeStamp_) {
        return;
    }
#endif

    if (haveTimerTask_) {
        eventHandler->RemoveTask(TIMER_TASK);
    }

    auto callback = [wp = weak_from_this()] {
        auto sp = wp.lock();
        if (sp) {
            // Timer task is triggered, so there is no timer task now.
            sp->haveTimerTask_ = false;
            sp->OnTriggered();
        }
    };
    eventHandler->PostTask(callback, TIMER_TASK, timeout);
    lastTimeStamp_ = timeStamp;
    haveTimerTask_ = true;
}
#endif

ARKTS_Engine ARKTS_CreateEngine()
{
    panda::RuntimeOption options;
    options.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::INFO);
    auto vm = panda::JSNApi::CreateJSVM(options);
    if (!vm) {
        LOGE("create EcmaVM failed");
        return nullptr;
    }
    auto engine = new ArkNativeEngine(vm, nullptr);
    if (!engine) {
        LOGE("alloc ArkEngine failed");
        panda::JSNApi::DestroyJSVM(vm);
        return nullptr;
    }

    auto result = new ARKTS_Engine_;
    if (!result) {
        LOGE("alloc ARKTS_Engine_ failed");
        panda::JSNApi::DestroyJSVM(vm);
        delete engine;
        return nullptr;
    }
    result->vm = vm;
    result->engine = engine;
#ifdef __OHOS__
    result->eventHandler = ARKTS_GetOrCreateEventHandler(P_CAST(vm, ARKTS_Env));
    if (!result->eventHandler) {
        ARKTS_DestroyEngine(result);
        return nullptr;
    }
#endif
    auto loop = engine->GetUVLoop();
    if (loop == nullptr) {
        if (!engine->ReinitUVLoop()) {
            LOGE("init uv loop failed");
            ARKTS_DestroyEngine(result);
            return nullptr;
        }
        loop = engine->GetUVLoop();
    }
    panda::JSNApi::SetLoop(vm, loop);

#ifdef __OHOS__
    uv_run(loop, UV_RUN_NOWAIT);
    auto fd = uv_backend_fd(loop);
    uint32_t events = OHOS::AppExecFwk::FILE_DESCRIPTOR_INPUT_EVENT | OHOS::AppExecFwk::FILE_DESCRIPTOR_OUTPUT_EVENT;
    result->eventHandler->AddFileDescriptorListener(fd, events, std::make_shared<UVLoopHandler>(loop), "uvLoopTask");
#endif
    return result;
}

void* ARKTS_GetNAPIEnv(ARKTS_Engine engine)
{
    ARKTS_ASSERT_P(engine, "engine is null");
    return engine->engine;
}

void ARKTS_DestroyEngine(ARKTS_Engine engine)
{
    delete engine->engine;
    if (engine->vm) {
        auto env = P_CAST(engine->vm, ARKTS_Env);
        ARKTS_DisposeJSContext(env);
        ARKTS_DisposeEventHandler(env);
        panda::JSNApi::DestroyJSVM(engine->vm);
    }
    engine->engine = nullptr;
    engine->vm = nullptr;
#ifdef __OHOS__
    engine->eventHandler->RemoveAllFileDescriptorListeners();
#endif
    delete engine;
}

ARKTS_Env ARKTS_GetContext(ARKTS_Engine engine)
{
    return P_CAST(engine->vm, ARKTS_Env);
}

bool ARKTS_LoadEntryFromAbc(ARKTS_Engine engine, const char* filePath, const char* entryPoint, bool forceReload)
{
    ARKTS_ASSERT_F(engine, "engine is null");
    ARKTS_ASSERT_F(filePath, "filePath is null");
    ARKTS_ASSERT_F(entryPoint, "entryPoint is null");

    if (!forceReload) {
        auto existed = engine->loadedAbcs.find(entryPoint);
        if (existed != engine->loadedAbcs.end()) {
            if (existed->second == filePath) {
                return true;
            } else {
                LOGE("can't shadow loaded entryPoint from another .abc, entryPoint: %{public}s", entryPoint);
                return false;
            }
        }
    }
    if (access(filePath, R_OK) != 0) {
        LOGE("no such file: %{public}s", filePath);
        return false;
    }
    auto vm = engine->vm;
    panda::JSNApi::NotifyLoadModule(vm);
    auto success = panda::JSNApi::Execute(vm, filePath, entryPoint);
    if (success) {
        engine->loadedAbcs[entryPoint] = filePath;
    }
    return success;
}

ARKTS_Value ARKTS_ImportFromEntry(ARKTS_Engine engine, const char* entryPoint, const char* importName)
{
    ARKTS_ASSERT_P(engine, "engine is null");
    ARKTS_ASSERT_P(entryPoint, "entryPoint is null");
    ARKTS_ASSERT_P(importName, "importName is null");

    if (engine->loadedAbcs.find(entryPoint) == engine->loadedAbcs.end()) {
        return ARKTS_CreateUndefined();
    }

    auto vm = engine->vm;
    auto value = panda::JSNApi::GetExportObject(vm, entryPoint, importName);
    if (value->IsHole()) {
        return ARKTS_CreateUndefined();
    }

    return ARKTS_FromHandle(value);
}

ARKTS_Value ARKTS_Require(
    ARKTS_Env env, const char* target, bool isNativeModule, bool isAppModule, const char* relativePath)
{
    ARKTS_ASSERT_P(env, "env is null");

    auto global = ARKTS_GetGlobalConstant(env);
    auto targetValue = ARKTS_CreateUtf8(env, target, -1);

    ARKTS_ASSERT_P(ARKTS_IsHeapObject(global), "js global is null");
    if (!isNativeModule) {
        auto funcName = ARKTS_CreateUtf8(env, "requireInternal", -1);
        auto funcValue = ARKTS_GetProperty(env, global, funcName);
        ARKTS_ASSERT_P(ARKTS_IsCallable(env, funcName), "global func requireInternal is undefined");
        return ARKTS_Call(env, funcValue, ARKTS_CreateUndefined(), 1, &targetValue);
    }
    auto funcName = ARKTS_CreateUtf8(env, "requireNapi", -1);
    auto funcValue = ARKTS_GetProperty(env, global, funcName);
    ARKTS_ASSERT_P(ARKTS_IsCallable(env, funcValue), "global func requireNapi is undefined");

    if (relativePath) {
        ARKTS_Value args[] = { targetValue, ARKTS_CreateBool(isAppModule), ARKTS_CreateUndefined(),
            ARKTS_CreateUtf8(env, relativePath, -1) };
        return ARKTS_Call(env, funcValue, ARKTS_CreateUndefined(), sizeof(args) / sizeof(ARKTS_Value), args);
    } else {
        ARKTS_Value args[] = { targetValue, ARKTS_CreateBool(isAppModule) };
        return ARKTS_Call(env, funcValue, ARKTS_CreateUndefined(), sizeof(args) / sizeof(ARKTS_Value), args);
    }
}
