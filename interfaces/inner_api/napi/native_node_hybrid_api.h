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

#ifndef FOUNDATION_ACE_NAPI_INTERFACES_KITS_NAPI_NATIVE_NODE_HYBRID_API_H
#define FOUNDATION_ACE_NAPI_INTERFACES_KITS_NAPI_NATIVE_NODE_HYBRID_API_H

#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "js_native_api.h"
#include "native_common.h"
#include "node_api.h"

typedef napi_value (*proxy_object_attach_cb)(napi_env env, void* data);

typedef enum {
    NAPI_APP_STATE_FOREGROUND = 0,
    NAPI_APP_STATE_BACKGROUND = 1,
    NAPI_APP_STATE_SENSITIVE_START = 2,
    NAPI_APP_STATE_SENSITIVE_END = 3,
    NAPI_APP_STATE_COLD_START_FINISHED = 4,
    NAPI_APP_STATE_WARM_START = 5,
} NapiAppState;

typedef enum {
    NAPI_DIRECTION_INVALID = 0,
    NAPI_DIRECTION_DYNAMIC_TO_STATIC = 1, // JS object references the STS object
    NAPI_DIRECTION_STATIC_TO_DYNAMIC = 2, // STS object references the JS object
    NAPI_DIRECTION_HYBRID = 3,            // STS object and the JS object references each other
} NapiXRefDirection;

typedef struct NapiStackInfo {
    size_t stackStart;
    size_t stackSize;
} NapiStackInfo;

using NapiAppStateCallback = void (*)(int state, int64_t timestamp);

NAPI_EXTERN napi_status napi_load_module_with_info_hybrid(napi_env env,
                                                          const char* path,
                                                          const char* module_info,
                                                          napi_value* result);
NAPI_EXTERN napi_status napi_load_module_with_module_request(napi_env env,
                                                             const char* request_name,
                                                             napi_value* result);
NAPI_EXTERN napi_status napi_set_stackinfo(napi_env env, NapiStackInfo* napi_info);
NAPI_EXTERN napi_status napi_get_stackinfo(napi_env env, NapiStackInfo* result);

// XGC specific internal API
#ifdef PANDA_JS_ETS_HYBRID_MODE
NAPI_EXTERN napi_status napi_xref_wrap(napi_env env,
                                       napi_value js_object,
                                       void* native_object,
                                       napi_finalize finalize_cb,
                                       NapiXRefDirection ref_direction,
                                       napi_ref* result);

NAPI_EXTERN napi_status napi_xref_unwrap(napi_env env, napi_value js_object, void** result);

NAPI_EXTERN napi_status napi_mark_from_object(napi_env env, napi_ref ref);

NAPI_EXTERN napi_status napi_create_xref(napi_env env,
                                         napi_value value,
                                         uint32_t initial_refcount,
                                         napi_ref* result);

NAPI_EXTERN napi_status napi_vm_handshake(napi_env env, void* inputIface, void** outputIface);

NAPI_EXTERN napi_status napi_mark_attach_with_xref(napi_env env,
                                                   napi_value js_object,
                                                   void* attach_data,
                                                   proxy_object_attach_cb attach_cb);

NAPI_EXTERN napi_status napi_is_alive_object(napi_env env, napi_ref ref, bool* result);
NAPI_EXTERN napi_status napi_is_contain_object(napi_env env, napi_ref ref, bool* result);
NAPI_EXTERN napi_status napi_is_xref_type(napi_env env, napi_value js_object, bool* result);
NAPI_EXTERN napi_status napi_get_ets_implements(napi_env env, napi_value value, napi_value* result);
NAPI_EXTERN napi_status napi_deserialize_hybrid(napi_env env, void* buffer, napi_value* object);
NAPI_EXTERN napi_status napi_setup_hybrid_environment(napi_env env);

NAPI_EXTERN napi_status napi_serialize_hybrid(napi_env env,
                                              napi_value object,
                                              napi_value transfer_list,
                                              napi_value clone_list, void** result);
 NAPI_EXTERN napi_status napi_wrap_with_xref(napi_env env,
                                            napi_value js_object,
                                            void* native_object,
                                            napi_finalize finalize_cb,
                                            napi_ref* result);
#endif // PANDA_JS_ETS_HYBRID_MODE
NAPI_EXTERN napi_status napi_register_appstate_callback(napi_env env, NapiAppStateCallback callback);

#endif /* FOUNDATION_ACE_NAPI_INTERFACES_KITS_NAPI_NATIVE_NODE_HYBRID_API_H */