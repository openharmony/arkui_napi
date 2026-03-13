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

#include <chrono>
#include <map>
#include <mutex>

#include <uv.h>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace timer_utils {

enum class TimerType {
    TIMEOUT,
    INTERVAL,
};

struct TimerInfo {
    uint32_t id;
    TimerType type;
    napi_ref callbackRef;
    napi_env env;
    uv_timer_t timerHandle;
    bool cancelled;
};

struct EnvData {
    uint32_t timerIdCounter;
    std::map<uint32_t, TimerInfo*> timers;
    std::mutex timersMutex;
};

static std::map<napi_env, EnvData*> g_envDataMap;
static std::mutex g_envDataMapMutex;

static EnvData* GetEnvData(napi_env env)
{
    std::lock_guard<std::mutex> lock(g_envDataMapMutex);
    auto it = g_envDataMap.find(env);
    if (it != g_envDataMap.end()) {
        return it->second;
    }
    EnvData* envData = new EnvData();
    envData->timerIdCounter = 0;
    g_envDataMap[env] = envData;
    return envData;
}

static void RemoveEnvData(napi_env env)
{
    std::lock_guard<std::mutex> lock(g_envDataMapMutex);
    auto it = g_envDataMap.find(env);
    if (it != g_envDataMap.end()) {
        delete it->second;
        g_envDataMap.erase(it);
    }
}

static void OnTimerCallback(uv_timer_t* handle)
{
    TimerInfo* info = static_cast<TimerInfo*>(handle->data);
    if (info == nullptr || info->cancelled || info->callbackRef == nullptr) {
        return;
    }

    napi_env env = info->env;
    EnvData* envData = GetEnvData(env);
    if (envData == nullptr) {
        return;
    }

    napi_value callback {};
    napi_status status = napi_get_reference_value(env, info->callbackRef, &callback);
    if (status != napi_ok || callback == nullptr) {
        return;
    }

    napi_value global {};
    status = napi_get_global(env, &global);
    if (status != napi_ok) {
        return;
    }

    napi_value result {};
    napi_call_function(env, global, callback, 0, nullptr, &result);

    if (info->type == TimerType::TIMEOUT) {
        std::lock_guard<std::mutex> lock(envData->timersMutex);
        envData->timers.erase(info->id);
        napi_delete_reference(env, info->callbackRef);
        delete info;
    }
}

static uint32_t CreateTimer(napi_env env, napi_value callback, uint64_t delay, TimerType type)
{
    EnvData* envData = GetEnvData(env);
    if (envData == nullptr) {
        return 0;
    }

    uv_loop_t* loop = nullptr;
    if (napi_get_uv_event_loop(env, &loop) != napi_ok || loop == nullptr) {
        return 0;
    }

    uint32_t timerId = ++envData->timerIdCounter;

    TimerInfo* info = new TimerInfo();
    info->id = timerId;
    info->type = type;
    info->env = env;
    info->cancelled = false;

    napi_create_reference(env, callback, 1, &info->callbackRef);

    info->timerHandle.data = info;
    if (uv_timer_init(loop, &info->timerHandle) != 0) {
        napi_delete_reference(env, info->callbackRef);
        delete info;
        return 0;
    }

    {
        std::lock_guard<std::mutex> lock(envData->timersMutex);
        envData->timers[timerId] = info;
    }

    uint64_t repeat = (type == TimerType::INTERVAL) ? delay : 0;
    if (uv_timer_start(&info->timerHandle, OnTimerCallback, delay, repeat) != 0) {
        uv_close(reinterpret_cast<uv_handle_t*>(&info->timerHandle), nullptr);
        {
            std::lock_guard<std::mutex> lock(envData->timersMutex);
            envData->timers.erase(timerId);
        }
        napi_delete_reference(env, info->callbackRef);
        delete info;
        return 0;
    }

    return timerId;
}

static bool CancelTimer(napi_env env, uint32_t timerId)
{
    EnvData* envData = GetEnvData(env);
    if (envData == nullptr) {
        return false;
    }

    std::lock_guard<std::mutex> lock(envData->timersMutex);

    auto it = envData->timers.find(timerId);
    if (it == envData->timers.end()) {
        return false;
    }

    TimerInfo* info = it->second;
    info->cancelled = true;

    uv_timer_stop(&info->timerHandle);
    uv_close(reinterpret_cast<uv_handle_t*>(&info->timerHandle), [](uv_handle_t* handle) {
        TimerInfo* timerInfo = static_cast<TimerInfo*>(handle->data);
        if (timerInfo != nullptr) {
            napi_delete_reference(timerInfo->env, timerInfo->callbackRef);
            delete timerInfo;
        }
    });

    envData->timers.erase(it);
    return true;
}

static napi_value SetTimeout(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] {};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 2) {
        napi_throw_error(env, nullptr, "setTimeout requires 2 arguments: callback and delay");
        return nullptr;
    }

    napi_valuetype callbackType;
    napi_typeof(env, args[0], &callbackType);
    if (callbackType != napi_function) {
        napi_throw_type_error(env, nullptr, "First argument must be a function");
        return nullptr;
    }

    int64_t delay;
    napi_get_value_int64(env, args[1], &delay);
    if (delay < 0) {
        delay = 0;
    }

    uint32_t timerId = CreateTimer(env, args[0], static_cast<uint64_t>(delay), TimerType::TIMEOUT);

    napi_value result {};
    napi_create_uint32(env, timerId, &result);
    return result;
}

static napi_value ClearTimeout(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] {};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1) {
        napi_throw_error(env, nullptr, "clearTimeout requires 1 argument: timerId");
        return nullptr;
    }

    uint32_t timerId;
    napi_get_value_uint32(env, args[0], &timerId);

    CancelTimer(env, timerId);

    return nullptr;
}

static napi_value SetInterval(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] {};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 2) {
        napi_throw_error(env, nullptr, "setInterval requires 2 arguments: callback and interval");
        return nullptr;
    }

    napi_valuetype callbackType;
    napi_typeof(env, args[0], &callbackType);
    if (callbackType != napi_function) {
        napi_throw_type_error(env, nullptr, "First argument must be a function");
        return nullptr;
    }

    int64_t interval;
    napi_get_value_int64(env, args[1], &interval);
    if (interval < 0) {
        interval = 0;
    }

    uint32_t timerId = CreateTimer(env, args[0], static_cast<uint64_t>(interval), TimerType::INTERVAL);

    napi_value result {};
    napi_create_uint32(env, timerId, &result);
    return result;
}

static napi_value ClearInterval(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] {};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < 1) {
        napi_throw_error(env, nullptr, "clearInterval requires 1 argument: timerId");
        return nullptr;
    }

    uint32_t timerId;
    napi_get_value_uint32(env, args[0], &timerId);

    CancelTimer(env, timerId);

    return nullptr;
}

static napi_value Now(napi_env env, napi_callback_info info)
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    napi_value result {};
    napi_create_bigint_int64(env, ms, &result);
    return result;
}

static void FinalizeEnvData(napi_env env, void* data, void* hint)
{
    RemoveEnvData(env);
}

static napi_value TimerUtilsInit(napi_env env, napi_value exports)
{
    GetEnvData(env);
    napi_set_instance_data(env, nullptr, FinalizeEnvData, nullptr);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("setTimeout", SetTimeout),
        DECLARE_NAPI_FUNCTION("clearTimeout", ClearTimeout),
        DECLARE_NAPI_FUNCTION("setInterval", SetInterval),
        DECLARE_NAPI_FUNCTION("clearInterval", ClearInterval),
        DECLARE_NAPI_FUNCTION("now", Now),
    };

    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

} // namespace timer_utils

EXTERN_C_START
static napi_module timerUtilsModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = timer_utils::TimerUtilsInit,
    .nm_modname = "timer_utils",
    .nm_priv = nullptr,
    .reserved = { 0 },
};
EXTERN_C_END

extern "C" __attribute__((constructor)) void RegisterTimerUtilsModule(void)
{
    napi_module_register(&timerUtilsModule);
}
