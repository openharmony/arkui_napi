/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_NAPI_INTERFACES_KITS_NAPI_NATIVE_API_H
#define FOUNDATION_ACE_NAPI_INTERFACES_KITS_NAPI_NATIVE_API_H

#ifndef NAPI_VERSION
#define NAPI_VERSION 8
#endif

#ifndef NAPI_EXPERIMENTAL
#define NAPI_EXPERIMENTAL
#endif

#include "common.h"
#include "node_api.h"

#ifdef NAPI_TEST
#ifdef _WIN32
#define NAPI_INNER_EXTERN __declspec(dllexport)
#else
#define NAPI_INNER_EXTERN __attribute__((visibility("default")))
#endif
#else
#ifdef _WIN32
#define NAPI_INNER_EXTERN __declspec(deprecated)
#else
#define NAPI_INNER_EXTERN __attribute__((__deprecated__))
#endif
#endif

NAPI_EXTERN napi_status napi_fatal_exception(napi_env env, napi_value err);


NAPI_EXTERN napi_status napi_create_string_utf16(napi_env env,
                                                 const char16_t* str,
                                                 size_t length,
                                                 napi_value* result);

NAPI_EXTERN napi_status napi_get_value_string_utf16(napi_env env,
                                                    napi_value value,
                                                    char16_t* buf,
                                                    size_t bufsize,
                                                    size_t* result);

NAPI_EXTERN napi_status napi_type_tag_object(napi_env env,
                                             napi_value value,
                                             const napi_type_tag* type_tag);

NAPI_EXTERN napi_status napi_check_object_type_tag(napi_env env,
                                                   napi_value value,
                                                   const napi_type_tag* type_tag,
                                                   bool* result);


NAPI_INNER_EXTERN napi_status napi_adjust_external_memory(napi_env env,
                                                          int64_t change_in_bytes,
                                                          int64_t* adjusted_value);

#ifdef __cplusplus
extern "C" {
#endif

typedef void* (*napi_native_binding_detach_callback)(napi_env env, void* native_object, void* hint);
typedef napi_value (*napi_native_binding_attach_callback)(napi_env env, void* native_object, void* hint);
typedef void (*napi_detach_finalize_callback)(void* detachedObject, void* finalizeHint);

NAPI_EXTERN napi_status node_api_get_module_file_name(napi_env env, const char** result);
NAPI_EXTERN napi_status napi_run_script_path(napi_env env, const char* path, napi_value* result);
NAPI_EXTERN napi_status napi_queue_async_work_with_qos(napi_env env, napi_async_work work, napi_qos_t qos);
NAPI_EXTERN napi_status napi_load_module(napi_env env, const char* path, napi_value* result);
NAPI_EXTERN napi_status napi_load_module_with_info(napi_env env,
                                                   const char* path,
                                                   const char* module_info,
                                                   napi_value* result);
NAPI_EXTERN napi_status napi_create_object_with_properties(napi_env env,
                                                           napi_value* result,
                                                           size_t property_count,
                                                           const napi_property_descriptor* properties);
NAPI_EXTERN napi_status napi_create_object_with_named_properties(napi_env env,
                                                                 napi_value* result,
                                                                 size_t property_count,
                                                                 const char** keys,
                                                                 const napi_value* values);
NAPI_EXTERN napi_status napi_coerce_to_native_binding_object(napi_env env,
                                                             napi_value js_object,
                                                             napi_native_binding_detach_callback detach_cb,
                                                             napi_native_binding_attach_callback attach_cb,
                                                             void* native_object,
                                                             void* hint);
NAPI_EXTERN napi_status napi_add_detached_finalizer(napi_env env,
                                                    napi_value native_binding_object,
                                                    napi_detach_finalize_callback detach_finalize_cb,
                                                    void* finalize_hint);
NAPI_EXTERN napi_status napi_run_event_loop(napi_env env, napi_event_mode mode);
NAPI_EXTERN napi_status napi_stop_event_loop(napi_env env);
NAPI_EXTERN napi_status napi_create_ark_runtime(napi_env *env);
NAPI_EXTERN napi_status napi_destroy_ark_runtime(napi_env *env);

NAPI_EXTERN napi_status napi_serialize(napi_env env,
                                       napi_value object,
                                       napi_value transfer_list,
                                       napi_value clone_list,
                                       void** result);
NAPI_EXTERN napi_status napi_deserialize(napi_env env, void* buffer, napi_value* object);
NAPI_EXTERN napi_status napi_delete_serialization_data(napi_env env, void* buffer);
NAPI_EXTERN napi_status napi_is_concurrent_function(napi_env env, napi_value value, bool* result);


NAPI_EXTERN napi_status napi_call_threadsafe_function_with_priority(napi_threadsafe_function func,
                                                                    void *data,
                                                                    napi_task_priority priority,
                                                                    bool isTail);
NAPI_EXTERN napi_status napi_create_map(napi_env env, napi_value* result);
NAPI_EXTERN napi_status napi_map_set_property(napi_env env, napi_value map, napi_value key, napi_value value);
NAPI_EXTERN napi_status napi_map_set_named_property(napi_env env,
                                                    napi_value map,
                                                    const char* utf8name,
                                                    napi_value value);
NAPI_EXTERN napi_status napi_map_get_property(napi_env env, napi_value map, napi_value key, napi_value* result);
NAPI_EXTERN napi_status napi_map_get_named_property(napi_env env,
                                                    napi_value map,
                                                    const char* utf8name,
                                                    napi_value* result);
NAPI_EXTERN napi_status napi_map_has_property(napi_env env, napi_value map, napi_value key, bool* result);
NAPI_EXTERN napi_status napi_map_has_named_property(napi_env env, napi_value map, const char* utf8name, bool* result);
NAPI_EXTERN napi_status napi_map_delete_property(napi_env env, napi_value map, napi_value key);
NAPI_EXTERN napi_status napi_map_clear(napi_env env, napi_value map);
NAPI_EXTERN napi_status napi_map_get_size(napi_env env, napi_value map, uint32_t* result);
NAPI_EXTERN napi_status napi_map_get_entries(napi_env env, napi_value map, napi_value* result);
NAPI_EXTERN napi_status napi_map_get_keys(napi_env env, napi_value map, napi_value* result);
NAPI_EXTERN napi_status napi_map_get_values(napi_env env, napi_value map, napi_value* result);
NAPI_EXTERN napi_status napi_map_iterator_get_next(napi_env env, napi_value iterator, napi_value* result);
NAPI_EXTERN napi_status napi_wrap_enhance(napi_env env,
                                          napi_value js_object,
                                          void* native_object,
                                          napi_finalize finalize_cb,
                                          bool async_finalizer,
                                          void* finalize_hint,
                                          size_t native_binding_size,
                                          napi_ref* result);
NAPI_EXTERN void napi_onLoad();
// ======================================= sendable api begin ======================================= //
NAPI_EXTERN napi_status napi_define_sendable_class(napi_env env,
                                                   const char* utf8name,
                                                   size_t length,
                                                   napi_callback constructor,
                                                   void* data,
                                                   size_t property_count,
                                                   const napi_property_descriptor* properties,
                                                   napi_value parent,
                                                   napi_value* result);
NAPI_EXTERN napi_status napi_create_sendable_object_with_properties(napi_env env,
                                                                    size_t property_count,
                                                                    const napi_property_descriptor* properties,
                                                                    napi_value* result);
NAPI_EXTERN napi_status napi_create_sendable_array(napi_env env, napi_value* result);
NAPI_EXTERN napi_status napi_create_sendable_array_with_length(napi_env env, size_t length, napi_value* result);
NAPI_EXTERN napi_status napi_create_sendable_arraybuffer(napi_env env, size_t byte_length,
                                                         void** data, napi_value* result);
NAPI_EXTERN napi_status napi_create_sendable_typedarray(napi_env env,
                                                        napi_typedarray_type type,
                                                        size_t length,
                                                        napi_value arraybuffer,
                                                        size_t byte_offset,
                                                        napi_value* result);
NAPI_EXTERN napi_status napi_create_sendable_map(napi_env env, napi_value* result);
/*
 * Determine whether it can be held by a sendable object, including the sendable object and primitive type.
 */
NAPI_EXTERN napi_status napi_is_sendable(napi_env env, napi_value value, bool* result);
NAPI_EXTERN napi_status napi_wrap_sendable(napi_env env,
                                           napi_value js_object,
                                           void* native_object,
                                           napi_finalize finalize_cb,
                                           void* finalize_hint);
NAPI_EXTERN napi_status napi_wrap_sendable_with_size(napi_env env,
                                                     napi_value js_object,
                                                     void* native_object,
                                                     napi_finalize finalize_cb,
                                                     void* finalize_hint,
                                                     size_t native_binding_size);
NAPI_EXTERN napi_status napi_unwrap_sendable(napi_env env, napi_value js_object, void** result);
NAPI_EXTERN napi_status napi_remove_wrap_sendable(napi_env env, napi_value js_object, void** result);
// ======================================== sendable api end ======================================== //

// ======================================== multiple-context ======================================== //
NAPI_EXTERN napi_status napi_create_ark_context(napi_env env, napi_env* newEnv);

NAPI_EXTERN napi_status napi_switch_ark_context(napi_env env);

NAPI_EXTERN napi_status napi_destroy_ark_context(napi_env env);
#ifdef __cplusplus
}
#endif
#endif /* FOUNDATION_ACE_NAPI_INTERFACES_KITS_NAPI_NATIVE_API_H */
