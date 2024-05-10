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

#include "ark_interop_internal.h"
#include "ark_interop_napi.h"
#include "ark_interop_log.h"

#include <unordered_map>
#include <mutex>

using namespace panda::ecmascript;

#ifdef __OHOS__
using namespace OHOS::AppExecFwk;

namespace {
std::unordered_map<ARKTS_Env, ARKTS_Loop> g_eventHandlers_;
std::mutex g_eventHandlerMutex_;
}
#endif

ARKTS_Loop ARKTS_GetOrCreateEventHandler(ARKTS_Env env)
{
    if (!env) {
        return nullptr;
    }
#ifdef __OHOS__
    std::lock_guard lock(g_eventHandlerMutex_);
    auto searchRet = g_eventHandlers_.find(env);
    if (searchRet != g_eventHandlers_.end()) {
        return searchRet->second;
    }
    auto handler = std::make_shared<EventHandler>(EventRunner::Current());
    g_eventHandlers_[env] = handler;
    return handler;
#else
    return nullptr;
#endif
}

#ifdef __OHOS__
static ARKTS_Loop ARKTS_GetEventHandler(ARKTS_Env env)
{
    if (!env) {
        return nullptr;
    }
    std::lock_guard lock(g_eventHandlerMutex_);
    auto searchRet = g_eventHandlers_.find(env);
    if (searchRet != g_eventHandlers_.end()) {
        return searchRet->second;
    }
    return nullptr;
}
#endif

void ARKTS_InitEventHandle(ARKTS_Env env)
{
    ARKTS_ASSERT_V(env, "env is null");
    void (ARKTS_GetOrCreateEventHandler(env));
}

void ARKTS_DisposeEventHandler(ARKTS_Env env)
{
    if (!env) {
        return;
    }
#ifdef __OHOS__
    std::lock_guard lock(g_eventHandlerMutex_);
    g_eventHandlers_.erase(env);
#endif
}

void ARKTSInner_CreateAsyncTask(ARKTS_Env env, ARKTS_AsyncCallback callback, void* data)
{
#ifdef __OHOS__
    auto handler = ARKTS_GetEventHandler(env);
    if (!handler) {
        LOGE("event handler not initialized");
        return;
    }
    handler->PostTask([env, callback, data] {
        callback(env, data);
    });
#endif
}

void ARKTS_CreateAsyncTask(ARKTS_Env env, int64_t callbackId)
{
    ARKTSInner_CreateAsyncTask(env, ARKTSInner_CJAsyncCallback, reinterpret_cast<void*>(callbackId));
}
