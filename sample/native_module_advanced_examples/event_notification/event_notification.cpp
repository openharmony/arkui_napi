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
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>

struct EventData {
    int type;
    std::string message;
    int value;
};

class EventNotifier {
public:
    EventNotifier() : running_(false) {}
    
    ~EventNotifier() {
        Stop();
    }
    
    void Start() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_) {
            running_ = true;
            workerThread_ = std::thread(&EventNotifier::EventLoop, this);
        }
    }
    
    void Stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            running_ = false;
            cv_.notify_all();
        }
        if (workerThread_.joinable()) {
            workerThread_.join();
        }
    }
    
    void SetCallback(napi_env env, napi_value callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (callbackRef_) {
            napi_delete_reference(env, callbackRef_);
        }
        napi_create_reference(env, callback, 1, &callbackRef_);
    }
    
    void EmitEvent(int type, const std::string& message, int value) {
        std::lock_guard<std::mutex> lock(mutex_);
        EventData event{type, message, value};
        eventQueue_.push(event);
        cv_.notify_one();
    }
    
private:
    void EventLoop() {
        while (true) {
            EventData event;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this] { return !eventQueue_.empty() || !running_; });
                
                if (!running_) {
                    break;
                }
                
                if (!eventQueue_.empty()) {
                    event = eventQueue_.front();
                    eventQueue_.pop();
                } else {
                    continue;
                }
            }
            
            if (callbackRef_) {
                napi_handle_scope scope;
                napi_open_handle_scope(env_, &scope);
                
                napi_value callback;
                napi_get_reference_value(env_, callbackRef_, &callback);
                
                napi_value argv[3];
                napi_create_int32(env_, event.type, &argv[0]);
                napi_create_string_utf8(env_, event.message.c_str(), event.message.length(), &argv[1]);
                napi_create_int32(env_, event.value, &argv[2]);
                
                napi_value result;
                napi_call_function(env_, nullptr, callback, 3, argv, &result);
                
                napi_close_handle_scope(env_, scope);
            }
        }
    }
    
    napi_env env_;
    napi_ref callbackRef_ = nullptr;
    std::thread workerThread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<EventData> eventQueue_;
    bool running_;
};

static EventNotifier g_eventNotifier;

static napi_value StartEventNotifier(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        napi_throw_error(env, nullptr, "Expected 1 argument (callback)");
        return nullptr;
    }
    
    napi_valuetype valuetype;
    napi_typeof(env, args[0], &valuetype);
    if (valuetype != napi_function) {
        napi_throw_error(env, nullptr, "First argument must be a function");
        return nullptr;
    }
    
    g_eventNotifier.SetCallback(env, args[0]);
    g_eventNotifier.Start();
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value StopEventNotifier(napi_env env, napi_callback_info info) {
    g_eventNotifier.Stop();
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value EmitEvent(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 3) {
        napi_throw_error(env, nullptr, "Expected 3 arguments (type, message, value)");
        return nullptr;
    }
    
    int32_t type, value;
    size_t length;
    char message[256];
    
    napi_get_value_int32(env, args[0], &type);
    napi_get_value_string_utf8(env, args[1], message, sizeof(message), &length);
    napi_get_value_int32(env, args[2], &value);
    
    g_eventNotifier.EmitEvent(type, std::string(message, length), value);
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

EXTERN_C_START
napi_value Init(napi_env env, napi_value exports) {
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
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "event_notification",
    .nm_priv = ((void*)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterEventNotificationModule(void) {
    napi_module_register(&demoModule);
}
