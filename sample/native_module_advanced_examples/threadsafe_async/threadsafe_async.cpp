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
#include <atomic>
#include <vector>
#include <chrono>

struct AsyncWorkData {
    napi_async_work work = nullptr;
    napi_threadsafe_function tsfn = nullptr;
    napi_ref callbackRef = nullptr;
    int32_t input;
    int32_t result;
    std::string error;
};

static void ExecuteWork(napi_env env, void* data) {
    AsyncWorkData* asyncData = static_cast<AsyncWorkData*>(data);
    
    try {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        asyncData->result = asyncData->input * 2;
    } catch (const std::exception& e) {
        asyncData->error = e.what();
    }
}

static void WorkComplete(napi_env env, napi_status status, void* data) {
    AsyncWorkData* asyncData = static_cast<AsyncWorkData*>(data);
    
    if (status == napi_ok) {
        napi_value callback;
        napi_get_reference_value(env, asyncData->callbackRef, &callback);
        
        napi_value argv[2];
        if (asyncData->error.empty()) {
            napi_get_null(env, &argv[0]);
            napi_create_int32(env, asyncData->result, &argv[1]);
        } else {
            napi_create_string_utf8(env, asyncData->error.c_str(), asyncData->error.length(), &argv[0]);
            napi_get_undefined(env, &argv[1]);
        }
        
        napi_value global;
        napi_get_global(env, &global);
        napi_value result;
        napi_call_function(env, global, callback, 2, argv, &result);
    }
    
    if (asyncData->work) {
        napi_delete_async_work(env, asyncData->work);
    }
    if (asyncData->callbackRef) {
        napi_delete_reference(env, asyncData->callbackRef);
    }
    if (asyncData->tsfn) {
        napi_release_threadsafe_function(asyncData->tsfn, napi_tsfn_release);
    }
    
    delete asyncData;
}

static void ThreadSafeCallFinalizer(napi_env env, void* data, void* hint) {
}

static void ThreadSafeCallJs(napi_env env, napi_value jsCallback, void* context, void* data) {
    AsyncWorkData* asyncData = static_cast<AsyncWorkData*>(data);
    
    napi_value argv[2];
    if (asyncData->error.empty()) {
        napi_get_null(env, &argv[0]);
        napi_create_int32(env, asyncData->result, &argv[1]);
    } else {
        napi_create_string_utf8(env, asyncData->error.c_str(), asyncData->error.length(), &argv[0]);
        napi_get_undefined(env, &argv[1]);
    }
    
    napi_value result;
    napi_call_function(env, nullptr, jsCallback, 2, argv, &result);
}

static napi_value AsyncDouble(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 2) {
        napi_throw_error(env, nullptr, "Expected 2 arguments (value, callback)");
        return nullptr;
    }
    
    int32_t value;
    napi_get_value_int32(env, args[0], &value);
    
    napi_valuetype valuetype;
    napi_typeof(env, args[1], &valuetype);
    if (valuetype != napi_function) {
        napi_throw_error(env, nullptr, "Second argument must be a function");
        return nullptr;
    }
    
    AsyncWorkData* asyncData = new AsyncWorkData();
    asyncData->input = value;
    
    napi_value resourceName;
    napi_create_string_utf8(env, "AsyncDoubleResource", NAPI_AUTO_LENGTH, &resourceName);
    
    napi_create_reference(env, args[1], 1, &asyncData->callbackRef);
    
    napi_create_async_work(env, nullptr, resourceName, ExecuteWork, WorkComplete, asyncData, &asyncData->work);
    napi_queue_async_work(env, asyncData->work);
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value ThreadSafeAsyncDouble(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 2) {
        napi_throw_error(env, nullptr, "Expected 2 arguments (value, callback)");
        return nullptr;
    }
    
    int32_t value;
    napi_get_value_int32(env, args[0], &value);
    
    napi_valuetype valuetype;
    napi_typeof(env, args[1], &valuetype);
    if (valuetype != napi_function) {
        napi_throw_error(env, nullptr, "Second argument must be a function");
        return nullptr;
    }
    
    AsyncWorkData* asyncData = new AsyncWorkData();
    asyncData->input = value;
    
    napi_value resourceName;
    napi_create_string_utf8(env, "ThreadSafeResource", NAPI_AUTO_LENGTH, &resourceName);
    
    napi_create_threadsafe_function(env, args[1], nullptr, resourceName, 0, 1, nullptr, nullptr, 
        ThreadSafeCallFinalizer, asyncData, &asyncData->tsfn);
    
    std::thread([asyncData]() {
        try {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            asyncData->result = asyncData->input * 2;
        } catch (const std::exception& e) {
            asyncData->error = e.what();
        }
        
        napi_call_threadsafe_function(asyncData->tsfn, asyncData, napi_tsfn_blocking);
    }).detach();
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

static std::atomic<int> g_counter(0);
static std::mutex g_mutex;
static std::vector<int> g_results;

static napi_value ParallelCompute(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 2) {
        napi_throw_error(env, nullptr, "Expected 2 arguments (iterations, callback)");
        return nullptr;
    }
    
    int32_t iterations;
    napi_get_value_int32(env, args[0], &iterations);
    
    napi_valuetype valuetype;
    napi_typeof(env, args[1], &valuetype);
    if (valuetype != napi_function) {
        napi_throw_error(env, nullptr, "Second argument must be a function");
        return nullptr;
    }
    
    g_counter.store(0);
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_results.clear();
    }
    
    std::vector<std::thread> threads;
    for (int i = 0; i < iterations; i++) {
        threads.emplace_back([i]() {
            int result = i * i;
            g_counter.fetch_add(1);
            
            {
                std::lock_guard<std::mutex> lock(g_mutex);
                g_results.push_back(result);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    napi_value callback;
    napi_get_reference_value(env, nullptr, &callback);
    
    napi_value argv[2];
    napi_get_null(env, &argv[0]);
    
    napi_value resultsArray;
    napi_create_array(env, &resultsArray);
    
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        for (size_t i = 0; i < g_results.size(); i++) {
            napi_value element;
            napi_create_int32(env, g_results[i], &element);
            napi_set_element(env, resultsArray, i, element);
        }
    }
    
    argv[1] = resultsArray;
    
    napi_value global;
    napi_get_global(env, &global);
    napi_value result;
    napi_call_function(env, global, args[1], 2, argv, &result);
    
    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

EXTERN_C_START
napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor properties[] = {
        {"asyncDouble", nullptr, AsyncDouble, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"threadSafeAsyncDouble", nullptr, ThreadSafeAsyncDouble, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"parallelCompute", nullptr, ParallelCompute, nullptr, nullptr, nullptr, napi_default, nullptr},
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
    .nm_modname = "threadsafe_async",
    .nm_priv = ((void*)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterThreadSafeAsyncModule(void) {
    napi_module_register(&demoModule);
}
