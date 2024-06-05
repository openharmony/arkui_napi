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

#ifndef FOUNDATION_ACE_NAPI_INTERFACES_KITS_NAPI_NATIVE_NODE_API_H
#define FOUNDATION_ACE_NAPI_INTERFACES_KITS_NAPI_NATIVE_NODE_API_H

#include <iostream>
#include <string>
#include <vector>

#include "js_native_api.h"
#include "node_api.h"
#include "native_common.h"

typedef void (*NAPIGetJSCode)(const char** buf, int* bufLen);
typedef void (*NapiNativeFinalize)(napi_env env, void* data, void* hint);
typedef void* (*NapiDetachCallback)(napi_env env, void* nativeObject, void* hint); // hint: detach params
typedef napi_value (*NapiAttachCallback)(napi_env env, void* nativeObject, void* hint); // hint: attach params

typedef struct napi_module_with_js {
    int nm_version = 0;
    unsigned int nm_flags = 0;
    const char* nm_filename = nullptr;
    napi_addon_register_func nm_register_func = nullptr;
    const char* nm_modname = nullptr;
    void* nm_priv = nullptr;
    NAPIGetJSCode nm_get_abc_code = nullptr;
    NAPIGetJSCode nm_get_js_code = nullptr;
} napi_module_with_js;

typedef enum {
    napi_eprio_vip = 0,
    napi_eprio_immediate = 1,
    napi_eprio_high = 2,
    napi_eprio_low = 3,
    napi_eprio_idle = 4,
} napi_event_priority;

NAPI_EXTERN napi_status napi_create_limit_runtime(napi_env env, napi_env* result_env);
NAPI_EXTERN void napi_module_with_js_register(napi_module_with_js* mod);
NAPI_EXTERN napi_status napi_is_callable(napi_env env, napi_value value, bool* result);
NAPI_EXTERN napi_status napi_create_runtime(napi_env env, napi_env* result_env);
NAPI_EXTERN napi_status napi_serialize_inner(napi_env env, napi_value object, napi_value transfer_list,
                                             napi_value clone_list, bool defaultTransfer, bool defaultCloneSendable,
                                             void** result);
NAPI_EXTERN napi_status napi_run_actor(napi_env env, uint8_t* buffer, size_t bufferSize,
                                       const char* descriptor, napi_value* result, char* entryPoint = nullptr);
NAPI_EXTERN napi_status napi_wrap_with_size(napi_env env,
                                            napi_value js_object,
                                            void* native_object,
                                            napi_finalize finalize_cb,
                                            void* finalize_hint,
                                            napi_ref* result,
                                            size_t native_binding_size);
NAPI_EXTERN napi_status napi_wrap_async_finalizer(napi_env env,
                                                  napi_value js_object,
                                                  void* native_object,
                                                  napi_finalize finalize_cb,
                                                  void* finalize_hint,
                                                  napi_ref* result,
                                                  size_t native_binding_size);
NAPI_EXTERN napi_status napi_create_external_with_size(napi_env env,
                                                       void* data,
                                                       napi_finalize finalize_cb,
                                                       void* finalize_hint,
                                                       napi_value* result,
                                                       size_t native_binding_size);
NAPI_EXTERN napi_status napi_set_promise_rejection_callback(napi_env env, napi_ref ref, napi_ref checkRef);

NAPI_EXTERN napi_status napi_is_arguments_object(napi_env env, napi_value value, bool* result);
NAPI_EXTERN napi_status napi_is_async_function(napi_env env, napi_value value, bool* result);
NAPI_EXTERN napi_status napi_is_boolean_object(napi_env env, napi_value value, bool* result);
NAPI_EXTERN napi_status napi_is_generator_function(napi_env env, napi_value value, bool* result);
NAPI_EXTERN napi_status napi_is_date(napi_env env, napi_value value, bool* result);

NAPI_EXTERN napi_status napi_is_map_iterator(napi_env env, napi_value value, bool* result);
NAPI_EXTERN napi_status napi_is_set_iterator(napi_env env, napi_value value, bool* result);
NAPI_EXTERN napi_status napi_is_generator_object(napi_env env, napi_value value, bool* result);
NAPI_EXTERN napi_status napi_is_module_namespace_object(napi_env env, napi_value value, bool* result);
NAPI_EXTERN napi_status napi_is_proxy(napi_env env, napi_value value, bool* result);
NAPI_EXTERN napi_status napi_is_reg_exp(napi_env env, napi_value value, bool* result);
NAPI_EXTERN napi_status napi_is_number_object(napi_env env, napi_value value, bool* result);
NAPI_EXTERN napi_status napi_is_map(napi_env env, napi_value value, bool* result);
NAPI_EXTERN napi_status napi_is_set(napi_env env, napi_value value, bool* result);

NAPI_EXTERN napi_status napi_is_string_object(napi_env env, napi_value value, bool* result);
NAPI_EXTERN napi_status napi_is_symbol_object(napi_env env, napi_value value, bool* result);
NAPI_EXTERN napi_status napi_is_weak_map(napi_env env, napi_value value, bool* result);
NAPI_EXTERN napi_status napi_is_weak_set(napi_env env, napi_value value, bool* result);

NAPI_EXTERN napi_status napi_is_big_int64_array(napi_env env, napi_value value, bool* result);
NAPI_EXTERN napi_status napi_is_big_uint64_array(napi_env env, napi_value value, bool* result);
NAPI_EXTERN napi_status napi_is_shared_array_buffer(napi_env env, napi_value value, bool* result);
NAPI_EXTERN napi_status napi_get_stack_trace(napi_env env, std::string& stack);
NAPI_EXTERN napi_status napi_get_hybrid_stack_trace(napi_env env, std::string& stack);
NAPI_EXTERN napi_status napi_get_own_property_descriptor(napi_env env,
                                                         napi_value object,
                                                         const char* utf8name,
                                                         napi_value* result);
NAPI_EXTERN napi_status napi_object_get_keys(napi_env env, napi_value data, napi_value* result);
NAPI_EXTERN napi_status napi_get_print_string(napi_env env,
                                              napi_value value,
                                              std::string& result);
NAPI_EXTERN napi_status napi_send_event(napi_env env, const std::function<void()> cb, napi_event_priority priority);
#endif /* FOUNDATION_ACE_NAPI_INTERFACES_KITS_NAPI_NATIVE_NODE_API_H */
