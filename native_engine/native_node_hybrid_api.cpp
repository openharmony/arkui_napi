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
#ifndef NAPI_EXPERIMENTAL
#define NAPI_EXPERIMENTAL
#endif

#ifdef ENABLE_CONTAINER_SCOPE
#include "core/common/container_scope.h"
#endif

#include "ecmascript/napi/include/jsnapi.h"
#include "ecmascript/napi/include/jsnapi_expo.h"
#include "native_api_internal.h"
#include "native_engine/impl/ark/ark_native_reference.h"
#include "native_engine/native_create_env.h"
#include "native_engine/native_utils.h"
#include "native_engine/worker_manager.h"
#include "securec.h"

using panda::ObjectRef;
using panda::PropertyAttribute;
static constexpr char NAME_SPACE_TAG = '@';
NAPI_EXTERN napi_status napi_load_module_with_info_hybrid(napi_env env,
                                                          const char* path,
                                                          const char* module_info,
                                                          napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, result);
    auto engine = reinterpret_cast<NativeEngine*>(env);
    *result = engine->NapiLoadModuleWithInfoForHybridApp(path, module_info);
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_load_module_with_module_request(napi_env env, const char* request_name, napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, request_name);
    CHECK_ARG(env, result);

    if (request_name[0] == NAME_SPACE_TAG) {
        // load module with OhmUrl
        auto [path, module_info] = panda::JSNApi::ResolveOhmUrl(request_name);
        napi_load_module_with_info_hybrid(env, path.c_str(), module_info.c_str(), result);
    } else {
        napi_load_module_with_path(env, request_name, result);
    }

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_set_stackinfo(napi_env env, NapiStackInfo* napi_info)
{
    CHECK_ARG(env, napi_info);
    ASSERT(napi_info != nullptr);
    panda::StackInfo info { napi_info->stackStart, napi_info->stackSize };
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JSNApi::SetStackInfo(vm, info);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_stackinfo(napi_env env, NapiStackInfo* result)
{
    CHECK_ARG(env, result);
    ASSERT(result != nullptr);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    auto res = panda::JSNApi::GetStackInfo(vm);
    result->stackStart = res.stackStart;
    result->stackSize = res.stackSize;
    return napi_clear_last_error(env);
}

#ifdef PANDA_JS_ETS_HYBRID_MODE
NAPI_EXTERN napi_status napi_xref_wrap(napi_env env,
                                       napi_value js_object,
                                       void* native_object,
                                       napi_finalize finalize_cb,
                                       NapiXRefDirection ref_direction,
                                       napi_ref* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, native_object);
    CHECK_ARG(env, finalize_cb);

    auto nativeValue = LocalValueFromJsValue(js_object);
    auto callback = reinterpret_cast<NapiNativeFinalize>(finalize_cb);
    auto engine = reinterpret_cast<ArkNativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, nativeObject);
    size_t nativeBindingSize = 0;
    auto reference = reinterpret_cast<NativeReference**>(result);
    Local<panda::StringRef> key = panda::StringRef::GetProxyNapiWrapperString(vm);
    NativeReference* ref = nullptr;
    Local<panda::ObjectRef> object = JSValueRef::Undefined(vm);
    switch (ref_direction) {
        case NapiXRefDirection::NAPI_DIRECTION_DYNAMIC_TO_STATIC:
            object = panda::ObjectRef::NewJSXRefObject(vm);
            ref = engine->CreateXRefReference(js_object, 1, false, callback, native_object);
            break;
        case NapiXRefDirection::NAPI_DIRECTION_STATIC_TO_DYNAMIC:
            object = panda::ObjectRef::New(vm);
            ref = engine->CreateXRefReference(js_object, 1, false, callback, native_object);
            break;
        case NapiXRefDirection::NAPI_DIRECTION_HYBRID:
            // Hybrid object may only exist in cross-language inherence case.
            // To be aborted in the future according to the specification
            HILOG_ERROR("[napi_xref_wrap] napi_direction_hybrid is not supported now!");
            return napi_set_last_error(env, napi_invalid_arg);
        default:
            HILOG_ERROR("[napi_xref_wrap] invalid ref_direction!");
            return napi_set_last_error(env, napi_invalid_arg);
    }
    if (reference != nullptr) {
        *reference = ref;
    }
    object->SetNativePointerFieldCount(vm, 1);
    object->SetNativePointerField(vm, 0, ref, nullptr, nullptr, nativeBindingSize);
    panda::PropertyAttribute attr(object, true, false, true);
    nativeObject->DefineProperty(vm, key, attr);
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_xref_unwrap(napi_env env, napi_value js_object, void** result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(js_object);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, nativeObject);
    Local<panda::StringRef> key = panda::StringRef::GetProxyNapiWrapperString(vm);
    Local<panda::JSValueRef> val = nativeObject->Get(vm, key);
    *result = nullptr;
    if (val->IsObject(vm)) {
        Local<panda::ObjectRef> ext(val);
        auto ref = reinterpret_cast<NativeReference*>(ext->GetNativePointerField(vm, 0));
        *result = ref != nullptr ? ref->GetData() : nullptr;
    }

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_mark_from_object(napi_env env, napi_ref ref)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, ref);
    ArkNativeReference* reference = reinterpret_cast<ArkNativeReference*>(ref);
    reference->MarkFromObject();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_xref(napi_env env, napi_value value, uint32_t initial_refcount, napi_ref* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<ArkNativeEngine*>(env);
    auto ref = engine->CreateXRefReference(value, initial_refcount, false, nullptr, nullptr);
    *result = reinterpret_cast<napi_ref>(ref);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_ets_implements(napi_env env, napi_value value, napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<panda::JSValueRef> implementsValue = panda::JSNApi::GetImplements(vm, nativeValue);
    *result = JsValueFromLocalValue(implementsValue);
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_register_appstate_callback(napi_env env, NapiAppStateCallback callback)
{
    CHECK_ENV(env);
    CHECK_ARG(env, callback);

    auto* engine = reinterpret_cast<NativeEngine*>(env);
    engine->RegisterAppStateCallback(callback);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_vm_handshake(napi_env env,
                                          [[maybe_unused]] void* inputIface,
                                          [[maybe_unused]] void** outputIface)
{
    CHECK_ENV(env);
    CHECK_ARG(env, inputIface);
    CHECK_ARG(env, outputIface);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::HandshakeHelper::DoHandshake(const_cast<EcmaVM*>(vm), inputIface, outputIface);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_alive_object(napi_env env, napi_ref ref, bool* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, ref);
    ArkNativeReference* reference = reinterpret_cast<ArkNativeReference*>(ref);
    *result = reference->IsObjectAlive();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_contain_object(napi_env env, napi_ref ref, bool* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, ref);
    ArkNativeReference* reference = reinterpret_cast<ArkNativeReference*>(ref);
    *result = reference->IsValidHeapObject();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_xref_type(napi_env env, napi_value js_object, bool* result)
{
    *result = false;
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(js_object);
    auto engine = reinterpret_cast<ArkNativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, nativeObject);
    Local<panda::StringRef> key = panda::StringRef::GetProxyNapiWrapperString(vm);

    if (nativeObject->Has(vm, key)) {
        *result = true;
    }
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_mark_attach_with_xref(napi_env env,
                                                   napi_value js_object,
                                                   void* attach_data,
                                                   proxy_object_attach_cb attach_cb)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, attach_data);
    CHECK_ARG(env, attach_cb);

    auto nativeValue = LocalValueFromJsValue(js_object);
    auto engine = reinterpret_cast<ArkNativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, nativeObject);
    size_t nativeBindingSize = 0;
    Local<panda::StringRef> key = panda::StringRef::GetProxyNapiWrapperString(vm);
    Local<panda::ObjectRef> object = panda::ObjectRef::NewJSXRefObject(vm);
    // Create strong reference now, will update to weak reference after interop support
    panda::JSNApi::XRefBindingInfo* data = panda::JSNApi::XRefBindingInfo::CreateNewInstance();
    if (data == nullptr) {
        HILOG_ERROR("data is nullptr");
        return napi_set_last_error(env, napi_invalid_arg);
    }
    data->attachXRefFunc = reinterpret_cast<void*>(attach_cb);
    data->attachXRefData = attach_data;
    object->SetNativePointerFieldCount(vm, 1);
    object->SetNativePointerField(
        vm, 0, nullptr,
        [](void* env, void* data, void* info) {
            panda::JSNApi::XRefBindingInfo* externalInfo = reinterpret_cast<panda::JSNApi::XRefBindingInfo*>(info);
            delete externalInfo;
        },
        reinterpret_cast<void*>(data), nativeBindingSize);
    panda::PropertyAttribute attr(object, true, false, true);
    nativeObject->DefineProperty(vm, key, attr);
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status
napi_serialize_hybrid(napi_env env, napi_value object, napi_value transfer_list, napi_value clone_list, void** result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, transfer_list);
    CHECK_ARG(env, clone_list);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);
    auto transferList = LocalValueFromJsValue(transfer_list);
    RETURN_STATUS_IF_FALSE(env, transferList->IsUndefined() || transferList->IsJSArray(vm), napi_invalid_arg);
    auto cloneList = LocalValueFromJsValue(clone_list);
    RETURN_STATUS_IF_FALSE(env, cloneList->IsUndefined() || cloneList->IsJSArray(vm), napi_invalid_arg);
    *result = panda::JSNApi::InterOpSerializeValue(vm, nativeValue, transferList, cloneList, false, false);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_deserialize_hybrid(napi_env env, void* buffer, napi_value* object)
{
    CHECK_ENV(env);
    CHECK_ARG(env, buffer);
    CHECK_ARG(env, object);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<panda::JSValueRef> res = panda::JSNApi::InterOpDeserializeValue(vm, buffer, reinterpret_cast<void*>(engine));
    *object = JsValueFromLocalValue(res);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_wrap_with_xref(napi_env env,
                                            napi_value js_object,
                                            void* native_object,
                                            napi_finalize finalize_cb,
                                            napi_ref* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, native_object);
    CHECK_ARG(env, finalize_cb);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(js_object);
    auto callback = reinterpret_cast<NapiNativeFinalize>(finalize_cb);
    auto engine = reinterpret_cast<ArkNativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, nativeObject);
    size_t nativeBindingSize = 0;
    auto reference = reinterpret_cast<NativeReference**>(result);
    Local<panda::StringRef> key = panda::StringRef::GetProxyNapiWrapperString(vm);
    NativeReference* ref = nullptr;
    Local<panda::ObjectRef> object = panda::ObjectRef::NewJSXRefObject(vm);
    // Create strong reference now, will update to weak reference after interop support
    ref = engine->CreateXRefReference(js_object, 1, false, callback, native_object);
    *reference = ref;
    object->SetNativePointerFieldCount(vm, 1);
    object->SetNativePointerField(vm, 0, ref, nullptr, nullptr, nativeBindingSize);
    PropertyAttribute attr(object, true, false, true);
    nativeObject->DefineProperty(vm, key, attr);
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_setup_hybrid_environment(napi_env env)
{
    NAPI_PREAMBLE(env);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    panda::JSNApi::InitHybridVMEnv(const_cast<panda::ecmascript::EcmaVM*>(vm));
    return GET_RETURN_STATUS(env);
}
#endif // PANDA_JS_ETS_HYBRID_MODE
