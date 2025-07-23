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

struct ARKTS_Loop_ {
    enum Status {
        IDLE,
        REQUESTING,
        STOPPING,
        STOPPED,
    };
    uv_async_t asyncReq;
    Status status;
    std::vector<std::function<void ()>> callbacks;
    std::mutex mutex;
    std::condition_variable cv;
    bool hasCleanupHook;

    explicit ARKTS_Loop_(napi_env env, uv_loop_t* loop);
    ~ARKTS_Loop_() = default;

    void PostTask(std::function<void ()> task);
    // only to be call by uv_queue_task
    void DrainTasks();
};

std::unordered_map<ARKTS_Env, ARKTS_Loop_*> g_uvLoops_;
std::mutex g_uvLoopMutex_;
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

static ARKTS_Loop_* ARKTSInner_GetUvLoop(ARKTS_Env env)
{
    std::lock_guard lock(g_uvLoopMutex_);
    auto searchRet = g_uvLoops_.find(env);
    if (searchRet != g_uvLoops_.end()) {
        return searchRet->second;
    } else {
        return nullptr;
    }
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

// @deprecated
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
    {
        std::lock_guard lock(g_eventHandlerMutex_);
        g_eventHandlers_.erase(env);
    }
#endif
    {
        std::lock_guard lock(g_uvLoopMutex_);
        g_uvLoops_.erase(env);
    }
}

void ARKTSInner_CreateAsyncTask(ARKTS_Env env, ARKTS_AsyncCallback callback, void* data)
{
    // uv_loop first, secondary is for compatible with early version.
    if (auto loop = ARKTSInner_GetUvLoop(env)) {
        loop->PostTask([env, callback, data] {
            callback(env, data);
        });
        return;
    }
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

ARKTS_Loop_::ARKTS_Loop_(napi_env env, uv_loop_t* loop) : asyncReq(), status(IDLE)
{
    auto error = uv_async_init(loop, &asyncReq, [](uv_async_t* work) {
        auto loop = static_cast<ARKTS_Loop_*>(work->data);
        if (loop) {
            loop->DrainTasks();
        }
    });
    if (error != 0) {
        LOGE("uv_async_init failed, status: %{public}d", error);
        hasCleanupHook = false;
        return;
    }
    asyncReq.data = this;
    hasCleanupHook = napi_add_cleanup_finalizer(env, [](void* data) {
        if (!data) {
            return;
        }
        auto loop = reinterpret_cast<ARKTS_Loop_*>(data);
        delete loop;
    }, this) == napi_ok;
}

void ARKTS_Loop_::PostTask(std::function<void()> task)
{
    std::lock_guard lock(mutex);
    if (status >= STOPPING) {
        return;
    }
    callbacks.push_back(std::move(task));
    if (status == IDLE) {
        int tryTimes = 3;
        while (tryTimes--) {
            auto ret = uv_async_send(&asyncReq);
            if (ret == 0) {
                status = REQUESTING;
                break;
            }
        }
    }
}

void ARKTS_Loop_::DrainTasks()
{
    std::vector<std::function<void ()>> pendingTasks;
    {
        std::lock_guard lock(mutex);
        // drop all callbacks
        if (status >= STOPPING) {
            if (status == STOPPING) {
                status = STOPPED;
                cv.notify_all();
            }
            return;
        }
        // status should be REQUESTING, skipping check.
        std::swap(callbacks, pendingTasks);
        status = IDLE;
    }
    for (const auto& task : pendingTasks) {
        task();
    }
}

// napi_add_env_cleanup_hook(napi_env, callback, hint) holds a (non-milt) map of all 'callbacks' by keyof 'hint'
// if others pass env as 'hint' may cause failure, so new CleanupTask as 'hint' to prevent collision.
struct CleanupTask {
    ARKTS_Env env;
};

bool ARKTSInner_InitLoop(napi_env env, ARKTS_Env vm, uv_loop_t* loop)
{
    std::lock_guard lock(g_uvLoopMutex_);
    if (g_uvLoops_.find(vm) != g_uvLoops_.end()) {
        return true;
    }
    auto arktsLoop = new (std::nothrow) ARKTS_Loop_(env, loop);
    if (!arktsLoop) {
        LOGE("new ARKTS_Loop_ failed.");
        return false;
    }
    if (!arktsLoop->hasCleanupHook) {
        LOGE("ARKTSLoop has no cleanup hook.");
        delete arktsLoop;
        return false;
    }

    auto cleanupTask = new (std::nothrow) CleanupTask {vm};
    if (!cleanupTask) {
        LOGE("new CleanupTask failed.");
        delete arktsLoop;
        return false;
    }
    auto status = napi_add_env_cleanup_hook(env, [](void* data) {
        if (!data) {
            return;
        }
        auto task = reinterpret_cast<CleanupTask*>(data);
        ARKTS_DisposeJSContext(task->env);
        ARKTS_DisposeEventHandler(task->env);
        ARKTS_RemoveGlobals(task->env);
        delete task;
    }, cleanupTask);
    if (status != napi_ok) {
        LOGE("add env cleanup hook failed: %{public}d", status);
        delete arktsLoop;
        delete cleanupTask;
        return false;
    }

    g_uvLoops_[vm] = arktsLoop;
    return true;
}