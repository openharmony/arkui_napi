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

#include "napi/native_api.h"
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>

constexpr int CALLBACK_REF_COUNT = 1;
constexpr int ARGV_COUNT_THREE = 3;
constexpr int ARGV_COUNT_ONE = 1;
constexpr int EVENT_TYPE_INDEX = 0;
constexpr int EVENT_MESSAGE_INDEX = 1;
constexpr int EVENT_VALUE_INDEX = 2;
constexpr int MESSAGE_BUFFER_SIZE = 256;
constexpr int MODULE_VERSION = 1;
constexpr int MODULE_FLAGS = 0;

struct EventData {
    int type;
    std::string message;
    int value;
};

class EventNotifier {
public:
    EventNotifier() : running_(false) {}

    ~EventNotifier()
    {
        Stop();
    }

    void Start()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_.load()) {
            running_.store(true);
            workerThread_ = std::thread(&EventNotifier::EventLoop, this);
        }
    }

    void Stop()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            running_.store(false);
            cv_.notify_all();
        }
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
    }

    void SetCallback(napi_env env, napi_value callback)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (callbackRef_) {
            napi_delete_reference(env, callbackRef_);
        }
        napi_status status = napi_create_reference(env, callback, CALLBACK_REF_COUNT, &callbackRef_);
        if (status != napi_ok) {
            callbackRef_ = nullptr;
        }
    }

    void EmitEvent(int type, const std::string& message, int value)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        EventData event{type, message, value};
        eventQueue_.push(event);
        cv_.notify_one();
    }

private:
    bool TryDequeueEvent(EventData& event)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !eventQueue_.empty() || !running_.load(); });

        if (!running_.load()) {
            return false;
        }

        event = eventQueue_.front();
        eventQueue_.pop();
        return true;
    }

    void DispatchEvent(const EventData& event)
    {
        if (!callbackRef_) {
            return;
        }

        napi_handle_scope scope;
        napi_status status = napi_open_handle_scope(env_, &scope);
        if (status != napi_ok) {
            return;
        }

        napi_value callback;
        status = napi_get_reference_value(env_, callbackRef_, &callback);
        if (status != napi_ok) {
            napi_close_handle_scope(env_, scope);
            return;
        }

        napi_value argv[ARGV_COUNT_THREE];
        status = napi_create_int32(env_, event.type, &argv[EVENT_TYPE_INDEX]);
        if (status != napi_ok) {
            napi_close_handle_scope(env_, scope);
            return;
        }
        status = napi_create_string_utf8(env_, event.message.c_str(), event.message.length(),
            &argv[EVENT_MESSAGE_INDEX]);
        if (status != napi_ok) {
            napi_close_handle_scope(env_, scope);
            return;
        }
        status = napi_create_int32(env_, event.value, &argv[EVENT_VALUE_INDEX]);
        if (status != napi_ok) {
            napi_close_handle_scope(env_, scope);
            return;
        }

        napi_value result;
        napi_call_function(env_, nullptr, callback, ARGV_COUNT_THREE, argv, &result);

        napi_close_handle_scope(env_, scope);
    }

    void EventLoop()
    {
        while (running_.load()) {
            EventData event;
            if (!TryDequeueEvent(event)) {
                break;
            }
            DispatchEvent(event);
        }
    }

    napi_env env_;
    napi_ref callbackRef_ = nullptr;
    std::thread workerThread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<EventData> eventQueue_;
    std::atomic<bool> running_;
};

static EventNotifier g_eventNotifier;

static napi_value StartEventNotifier(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_COUNT_ONE;
    napi_value args[ARGV_COUNT_ONE];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get callback info");
        return nullptr;
    }

    if (argc < ARGV_COUNT_ONE) {
        napi_throw_error(env, nullptr, "Expected 1 argument (callback)");
        return nullptr;
    }

    napi_valuetype valuetype;
    status = napi_typeof(env, args[0], &valuetype);
    if (status != napi_ok || valuetype != napi_function) {
        napi_throw_error(env, nullptr, "First argument must be a function");
        return nullptr;
    }

    g_eventNotifier.SetCallback(env, args[0]);
    g_eventNotifier.Start();

    napi_value result;
    status = napi_get_undefined(env, &result);
    if (status != napi_ok) {
        return nullptr;
    }
    return result;
}

static napi_value StopEventNotifier(napi_env env, napi_callback_info info)
{
    g_eventNotifier.Stop();

    napi_value result;
    napi_status status = napi_get_undefined(env, &result);
    if (status != napi_ok) {
        return nullptr;
    }
    return result;
}

static napi_value EmitEvent(napi_env env, napi_callback_info info)
{
    size_t argc = ARGV_COUNT_THREE;
    napi_value args[ARGV_COUNT_THREE];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get callback info");
        return nullptr;
    }

    if (argc < ARGV_COUNT_THREE) {
        napi_throw_error(env, nullptr, "Expected 3 arguments (type, message, value)");
        return nullptr;
    }

    int32_t type;
    int32_t value;
    size_t length;
    char message[MESSAGE_BUFFER_SIZE];

    status = napi_get_value_int32(env, args[0], &type);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get type");
        return nullptr;
    }
    status = napi_get_value_string_utf8(env, args[1], message, sizeof(message), &length);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get message");
        return nullptr;
    }
    status = napi_get_value_int32(env, args[EVENT_VALUE_INDEX], &value);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get value");
        return nullptr;
    }

    g_eventNotifier.EmitEvent(type, std::string(message, length), value);

    napi_value result;
    status = napi_get_undefined(env, &result);
    if (status != napi_ok) {
        return nullptr;
    }
    return result;
}

EXTERN_C_START
napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        {"startEventNotifier", nullptr, StartEventNotifier, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"stopEventNotifier", nullptr, StopEventNotifier, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"emitEvent", nullptr, EmitEvent, nullptr, nullptr, nullptr, napi_default, nullptr},
    };

    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = MODULE_VERSION,
    .nm_flags = MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "event_notification",
    .nm_priv = ((void*)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterEventNotificationModule(void)
{
    napi_module_register(&demoModule);
}
