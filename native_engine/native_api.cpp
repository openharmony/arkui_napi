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
#ifndef NAPI_EXPERIMENTAL
#define NAPI_EXPERIMENTAL
#endif

#ifdef ENABLE_HITRACE
#include <sys/prctl.h>
#endif

#include "ecmascript/napi/include/jsnapi.h"
#include "native_api_internal.h"
#include "native_engine/native_property.h"
#include "native_engine/native_value.h"
#include "securec.h"
#include "utils/log.h"
#ifdef ENABLE_CONTAINER_SCOPE
#include "core/common/container_scope.h"
#endif
#ifdef ENABLE_HITRACE
#include "hitrace_meter.h"
#endif

using panda::ArrayRef;
using panda::ArrayBufferRef;
using panda::BigIntRef;
using panda::BooleanRef;
using panda::BufferRef;
using panda::DateRef;
using panda::DataViewRef;
using panda::EscapeLocalScope;
using panda::FunctionRef;
using panda::Global;
using panda::IntegerRef;
using panda::JSNApi;
using panda::JsiRuntimeCallInfo;
using panda::Local;
using panda::LocalScope;
using panda::NativePointerRef;
using panda::NumberRef;
using panda::ObjectRef;
using panda::PrimitiveRef;
using panda::PromiseCapabilityRef;
using panda::PromiseRef;
using panda::PropertyAttribute;
using panda::StringRef;
using panda::SymbolRef;
using panda::TypedArrayRef;
using panda::ecmascript::EcmaVM;

static constexpr auto PANDA_MAIN_FUNCTION = "_GLOBAL::func_main_0";

class HandleScopeWrapper {
public:
    explicit HandleScopeWrapper(NativeEngine* engine) : scope_(engine->GetEcmaVm()) {}

private:
    LocalScope scope_;
};

class EscapableHandleScopeWrapper {
public:
    explicit EscapableHandleScopeWrapper(NativeEngine* engine)
        : scope_(engine->GetEcmaVm()), escapeCalled_(false) {}

    bool IsEscapeCalled() const
    {
        return escapeCalled_;
    }

    template<typename T>
    Local<T> Escape(Local<T> value)
    {
        escapeCalled_ = true;
        return scope_.Escape(value);
    }

private:
    EscapeLocalScope scope_;
    bool escapeCalled_;
};

inline napi_handle_scope HandleScopeToNapiHandleScope(HandleScopeWrapper* s) {
    return reinterpret_cast<napi_handle_scope>(s);
}

inline HandleScopeWrapper* NapiHandleScopeToHandleScope(napi_handle_scope s) {
    return reinterpret_cast<HandleScopeWrapper*>(s);
}

inline napi_escapable_handle_scope EscapableHandleScopeToNapiEscapableHandleScope(EscapableHandleScopeWrapper* s) {
    return reinterpret_cast<napi_escapable_handle_scope>(s);
}

inline EscapableHandleScopeWrapper* NapiEscapableHandleScopeToEscapableHandleScope(napi_escapable_handle_scope s) {
    return reinterpret_cast<EscapableHandleScopeWrapper*>(s);
}

inline napi_value JsValueFromLocalValue(Local<panda::JSValueRef> local)
{
    return reinterpret_cast<napi_value>(*local);
}

inline Local<panda::JSValueRef> LocalValueFromJsValue(napi_value v)
{
    Local<panda::JSValueRef> local;
    memcpy(static_cast<void*>(&local), &v, sizeof(v));
    return local;
}

inline napi_deferred JsDeferredFromLocalValue(Local<panda::JSValueRef> local)
{
    return reinterpret_cast<napi_deferred>(*local);
}

inline Local<panda::JSValueRef> LocalValueFromJsDeferred(napi_deferred v)
{
    Local<panda::JSValueRef> local;
    memcpy(static_cast<void*>(&local), &v, sizeof(v));
    return local;
}

NAPI_EXTERN napi_status napi_get_last_error_info(napi_env env, const napi_extended_error_info** result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    *result = reinterpret_cast<napi_extended_error_info*>(engine->GetLastError());
    if ((*result)->error_code == napi_ok) {
        napi_clear_last_error(env);
    }

    return napi_ok;
}

// Getters for defined singletons
NAPI_EXTERN napi_status napi_get_undefined(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    Local<panda::PrimitiveRef> value = panda::JSValueRef::Undefined(engine->GetEcmaVm());
    *result = JsValueFromLocalValue(value);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_null(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    Local<panda::PrimitiveRef> value = panda::JSValueRef::Null(engine->GetEcmaVm());
    *result = JsValueFromLocalValue(value);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_global(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    Local<panda::ObjectRef> value = panda::JSNApi::GetGlobalObject(engine->GetEcmaVm());
    *result = JsValueFromLocalValue(value);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_boolean(napi_env env, bool value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    Local<panda::BooleanRef> object = panda::BooleanRef::New(engine->GetEcmaVm(), value);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

// Methods to create Primitive types/Objects
NAPI_EXTERN napi_status napi_create_object(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    Local<panda::ObjectRef> object = panda::ObjectRef::New(engine->GetEcmaVm());
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_array(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    Local<panda::ArrayRef> object = panda::ArrayRef::New(engine->GetEcmaVm(), 0);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_array_with_length(napi_env env, size_t length, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    Local<panda::ArrayRef> object = panda::ArrayRef::New(engine->GetEcmaVm(), length);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_double(napi_env env, double value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    Local<panda::NumberRef> object = panda::NumberRef::New(engine->GetEcmaVm(), value);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_int32(napi_env env, int32_t value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    Local<panda::NumberRef> object = panda::NumberRef::New(engine->GetEcmaVm(), value);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_uint32(napi_env env, uint32_t value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    Local<panda::NumberRef> object = panda::NumberRef::New(engine->GetEcmaVm(), value);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_int64(napi_env env, int64_t value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    Local<panda::NumberRef> object = panda::NumberRef::New(engine->GetEcmaVm(), value);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_string_latin1(napi_env env, const char* str, size_t length, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, str);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    Local<panda::StringRef> object = panda::StringRef::NewFromUtf8(
        engine->GetEcmaVm(), str, (length == NAPI_AUTO_LENGTH) ? strlen(str) : length);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_string_utf8(napi_env env, const char* str, size_t length, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, str);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    Local<panda::StringRef> object = panda::StringRef::NewFromUtf8(
        engine->GetEcmaVm(), str, (length == NAPI_AUTO_LENGTH) ? strlen(str) : length);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_string_utf16(
    napi_env env, const char16_t* str, size_t length, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, str);
    CHECK_ARG(env, result);
    RETURN_STATUS_IF_FALSE(env, (length == NAPI_AUTO_LENGTH) || (length <= INT_MAX && length >= 0), napi_invalid_arg);
    auto engine = reinterpret_cast<NativeEngine*>(env);
    int char16Length = static_cast<int>(std::char_traits<char16_t>::length(str));
    Local<panda::StringRef> object = panda::StringRef::NewFromUtf16(
        engine->GetEcmaVm(), str, (length == NAPI_AUTO_LENGTH) ? char16Length : length);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_symbol(napi_env env, napi_value description, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();

    panda::Local<panda::JSValueRef> object = panda::JSValueRef::Undefined(vm);
    if (description == nullptr) {
        const char* str = "";
        object = panda::StringRef::NewFromUtf8(vm, str, 0);
    } else {
        object = LocalValueFromJsValue(description);
    }
    RETURN_STATUS_IF_FALSE(env, object->IsString(), napi_invalid_arg);
    Local<panda::SymbolRef> symbol = panda::SymbolRef::New(vm, object);
    *result = JsValueFromLocalValue(symbol);

    return napi_clear_last_error(env);
}

static inline void StartNapiProfilerTrace(panda::JsiRuntimeCallInfo *runtimeInfo)
{
#ifdef ENABLE_HITRACE
            EcmaVM *vm = runtimeInfo->GetVM();
            LocalScope scope(vm);
            Local<panda::FunctionRef> fn = runtimeInfo->GetFunctionRef();
            Local<panda::StringRef> nameRef = fn->GetName(vm);
            char threadName[128];
            prctl(PR_GET_NAME, threadName);
            StartTraceArgs(HITRACE_TAG_ACE, "Napi called:%s, tname:%s", nameRef->ToString().c_str(), threadName);
#endif
}

static inline void FinishNapiProfilerTrace()
{
#ifdef ENABLE_HITRACE
            FinishTrace(HITRACE_TAG_ACE);
#endif
}

Local<panda::JSValueRef> NativeFunctionCallBack(JsiRuntimeCallInfo *runtimeInfo)
{
    EcmaVM *vm = runtimeInfo->GetVM();
    panda::EscapeLocalScope scope(vm);
    auto info = reinterpret_cast<NapiNativeFunctionInfo*>(runtimeInfo->GetData());
    auto engine = reinterpret_cast<NativeEngine*>(info->env);
    auto cb = info->callback;
    if (engine == nullptr) {
        HILOG_ERROR("native engine is null");
        return panda::JSValueRef::Undefined(vm);
    }

    NapiNativeCallbackInfo cbInfo = { 0 };
    StartNapiProfilerTrace(runtimeInfo);
    cbInfo.thisVar = JsValueFromLocalValue(runtimeInfo->GetThisRef());
    cbInfo.function = JsValueFromLocalValue(runtimeInfo->GetNewTargetRef());
    cbInfo.argc = static_cast<size_t>(runtimeInfo->GetArgsNumber());
    cbInfo.argv = nullptr;
    cbInfo.functionInfo = info;
    if (cbInfo.argc > 0) {
//   todo     if (cbInfo.argc > MAX_CHUNK_ARRAY_SIZE) {
            cbInfo.argv = new napi_value [cbInfo.argc];
//        }
        for (size_t i = 0; i < cbInfo.argc; i++) {
            cbInfo.argv[i] = JsValueFromLocalValue(runtimeInfo->GetCallArgRef(i));
        }
    }

    if (JSNApi::IsMixedDebugEnabled(vm)) {
        JSNApi::NotifyNativeCalling(vm, reinterpret_cast<void *>(cb));
    }

    napi_value result = nullptr;
    if (cb != nullptr) {
        result = cb(info->env, &cbInfo);
    }

    if (cbInfo.argv != nullptr) {
//    todo    if (cbInfo.argc > MAX_CHUNK_ARRAY_SIZE) {
            delete[] cbInfo.argv;
//        }
        cbInfo.argv = nullptr;
    }

    Local<panda::JSValueRef> localRet = panda::JSValueRef::Undefined(vm);
    if (result != nullptr) {
        localRet = LocalValueFromJsValue(result);
    }

    FinishNapiProfilerTrace();
    if (localRet.IsEmpty()) {
        return panda::JSValueRef::Undefined(vm);
    }
    return scope.Escape(localRet);
}

NAPI_EXTERN napi_status napi_create_function(napi_env env,
                                             const char* utf8name,
                                             size_t length,
                                             napi_callback cb,
                                             void* data,
                                             napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, cb);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto callback = reinterpret_cast<NapiNativeCallback>(cb);
    auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());

    const char* name = "defaultName";
    NapiNativeFunctionInfo* funcInfo = NapiNativeFunctionInfo::CreateNewInstance();
    if (funcInfo == nullptr) {
        HILOG_ERROR("funcInfo is nullptr");
        return napi_set_last_error(env, napi_invalid_arg);
    }
    funcInfo->env = env;
    funcInfo->callback = callback;
    funcInfo->data = data;
#ifdef ENABLE_CONTAINER_SCOPE
    funcInfo->scopeId = OHOS::Ace::ContainerScope::CurrentId();
#endif

    Local<panda::FunctionRef> fn = panda::FunctionRef::New(vm, NativeFunctionCallBack,
                                             [](void* externalPointer, void* data) {
                                                auto info = reinterpret_cast<NapiNativeFunctionInfo*>(data);
                                                if (info != nullptr) {
                                                    delete info;
                                                }
                                             },
                                             reinterpret_cast<void*>(funcInfo), true);
    Local<panda::StringRef> fnName = panda::StringRef::NewFromUtf8(vm, utf8name != nullptr ? utf8name : name);
    fn->SetName(vm, fnName);
    *result = JsValueFromLocalValue(fn);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_error(napi_env env, napi_value code, napi_value msg, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, msg);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    Local<panda::JSValueRef> codeValue = panda::JSValueRef::Undefined(vm);
    if (code != nullptr) {
        codeValue = LocalValueFromJsValue(code);
        RETURN_STATUS_IF_FALSE(env, codeValue->IsString() || codeValue->IsNumber(), napi_invalid_arg);
    }

    auto msgValue = LocalValueFromJsValue(msg);
    RETURN_STATUS_IF_FALSE(env, msgValue->IsString(), napi_invalid_arg);

    Local<panda::JSValueRef> errorVal = panda::Exception::Error(vm, msgValue);
    if (code != nullptr) {
        Local<panda::StringRef> codeKey = panda::StringRef::NewFromUtf8(vm, "code");
        Local<panda::ObjectRef> errorObj(errorVal);
        errorObj->Set(vm, codeKey, codeValue);
    }
    *result = JsValueFromLocalValue(errorVal);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_type_error(napi_env env, napi_value code, napi_value msg, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, msg);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    Local<panda::JSValueRef> codeValue = panda::JSValueRef::Undefined(vm);
    if (code != nullptr) {
        codeValue = LocalValueFromJsValue(code);
        RETURN_STATUS_IF_FALSE(env, codeValue->IsString() || codeValue->IsNumber(), napi_invalid_arg);
    }
    auto msgValue = LocalValueFromJsValue(msg);
    RETURN_STATUS_IF_FALSE(env, msgValue->IsString(), napi_invalid_arg);

    Local<panda::JSValueRef> errorVal = panda::Exception::Error(vm, msgValue);
    if (code != nullptr) {
        Local<panda::StringRef> codeKey = panda::StringRef::NewFromUtf8(vm, "code");
        Local<panda::ObjectRef> errorObj(errorVal);
        errorObj->Set(vm, codeKey, codeValue);
    }
    *result = JsValueFromLocalValue(errorVal);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_range_error(napi_env env, napi_value code, napi_value msg, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, msg);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    Local<panda::JSValueRef> codeValue = panda::JSValueRef::Undefined(vm);

    if (code != nullptr) {
        codeValue = LocalValueFromJsValue(code);
        RETURN_STATUS_IF_FALSE(env, codeValue->IsString() || codeValue->IsNumber(), napi_invalid_arg);
    }
    auto msgValue = LocalValueFromJsValue(msg);
    RETURN_STATUS_IF_FALSE(env, msgValue->IsString(), napi_invalid_arg);

    Local<panda::JSValueRef> errorVal = panda::Exception::Error(vm, msgValue);
    if (code != nullptr) {
        Local<panda::StringRef> codeKey = panda::StringRef::NewFromUtf8(vm, "code");
        Local<panda::ObjectRef> errorObj(errorVal);
        errorObj->Set(vm, codeKey, codeValue);
    }
    *result = JsValueFromLocalValue(errorVal);

    return napi_clear_last_error(env);
}

// Methods to get the native napi_value from Primitive type
NAPI_EXTERN napi_status napi_typeof(napi_env env, napi_value value, napi_valuetype* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto valueObj = LocalValueFromJsValue(value);
    NativeValueType resultType;
    if (valueObj->IsNumber()) {
        resultType = NATIVE_NUMBER;
    } else if (valueObj->IsString()) {
        resultType = NATIVE_STRING;
    } else if (valueObj->IsFunction()) {
        resultType = NATIVE_FUNCTION;
    } else if (valueObj->IsNativePointer()) {
        resultType = NATIVE_EXTERNAL;
    } else if (valueObj->IsNull()) {
        resultType = NATIVE_NULL;
    } else if (valueObj->IsBoolean()) {
        resultType = NATIVE_BOOLEAN;
    } else if (valueObj->IsUndefined()) {
        resultType = NATIVE_UNDEFINED;
    } else if (valueObj->IsSymbol()) {
        resultType = NATIVE_SYMBOL;
    } else if (valueObj->IsBigInt()) {
        resultType = NATIVE_BIGINT;
    } else if (valueObj->IsObject()) {
        resultType = NATIVE_OBJECT;
    } else {
        resultType = NATIVE_UNDEFINED;
    }
    *result = (napi_valuetype)resultType;
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_double(napi_env env, napi_value value, double* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto nativeValue = LocalValueFromJsValue(value);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsNumber(), napi_number_expected);
    Local<panda::NumberRef> NumberVal = nativeValue->ToNumber(engine->GetEcmaVm());
    *result = NumberVal->Value();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_int32(napi_env env, napi_value value, int32_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto nativeValue = LocalValueFromJsValue(value);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsNumber(), napi_number_expected);
    Local<panda::NumberRef> NumberVal = nativeValue->ToNumber(engine->GetEcmaVm());
    *result = NumberVal->Value();

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_uint32(napi_env env, napi_value value, uint32_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto nativeValue = LocalValueFromJsValue(value);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsNumber(), napi_number_expected);
    Local<panda::NumberRef> NumberVal = nativeValue->ToNumber(engine->GetEcmaVm());
    *result = NumberVal->Value();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_int64(napi_env env, napi_value value, int64_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto nativeValue = LocalValueFromJsValue(value);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsNumber(), napi_number_expected);

    Local<panda::NumberRef> NumberVal = nativeValue->ToNumber(engine->GetEcmaVm());
    *result = NumberVal->Value();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_bool(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    Local<panda::JSValueRef> val = LocalValueFromJsValue(value);
    RETURN_STATUS_IF_FALSE(env, val->IsBoolean(), napi_boolean_expected);
    Local<panda::BooleanRef> boolVal = val->ToBoolean(engine->GetEcmaVm());
    *result = boolVal->Value();
    return napi_clear_last_error(env);
}

// Copies LATIN-1 encoded bytes from a string into a buffer.
NAPI_EXTERN napi_status napi_get_value_string_latin1(
    napi_env env, napi_value value, char* buf, size_t bufsize, size_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto nativeValue = LocalValueFromJsValue(value);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsString(), napi_string_expected);
    Local<panda::StringRef> stringVal = nativeValue->ToString(engine->GetEcmaVm());
    if (result != nullptr) {
        if (buf == nullptr) {
            *result = stringVal->Length();
        } else if (bufsize != 0) {
            int copied = stringVal->WriteLatin1(buf, bufsize);
            buf[copied] = '\0';
            *result = copied;
        } else {
            *result = 0;
        }
    }

    return napi_clear_last_error(env);
}

// Copies UTF-8 encoded bytes from a string into a buffer.
NAPI_EXTERN napi_status napi_get_value_string_utf8(
    napi_env env, napi_value value, char* buf, size_t bufsize, size_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(value);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsString(), napi_string_expected);
    Local<panda::StringRef> stringVal = nativeValue->ToString(vm);

    if (result != nullptr) {
        if (buf == nullptr) {
            *result = stringVal->Utf8Length(vm) - 1;
        } else if (bufsize != 0) {
            int copied = stringVal->WriteUtf8(buf, bufsize - 1, true) - 1;
            buf[copied] = '\0';
            *result = copied;
        } else {
            *result = 0;
        }
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_string_utf16(
    napi_env env, napi_value value, char16_t* buf, size_t bufsize, size_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto nativeValue = LocalValueFromJsValue(value);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsString(), napi_string_expected);
    Local<panda::StringRef> stringVal = nativeValue->ToString(engine->GetEcmaVm());

    if (result != nullptr) {
        if (buf == nullptr) {
            *result = stringVal->Length();
        } else if (bufsize == 1) {
            buf[0] = '\0';
            *result = 0;
        } else if (bufsize != 0) {
            int copied = stringVal->WriteUtf16(buf, bufsize - 1); // bufsize - 1 : reserve the position of buf "\0"
            buf[copied] = '\0';
            *result = copied;
        } else {
            *result = 0;
        }
    }

    return napi_clear_last_error(env);
}

// Methods to coerce values
// These APIs may execute user scripts
NAPI_EXTERN napi_status napi_coerce_to_bool(napi_env env, napi_value value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    Local<panda::JSValueRef> val = LocalValueFromJsValue(value);
    RETURN_STATUS_IF_FALSE(env, val->IsBoolean(), napi_boolean_expected);
    Local<panda::BooleanRef> boolVal = val->ToBoolean(engine->GetEcmaVm());
    *result = JsValueFromLocalValue(boolVal);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_coerce_to_number(napi_env env, napi_value value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto nativeValue = LocalValueFromJsValue(value);
    *result = JsValueFromLocalValue(nativeValue->ToNumber(engine->GetEcmaVm()));

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_coerce_to_object(napi_env env, napi_value value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto nativeValue = LocalValueFromJsValue(value);
    *result = JsValueFromLocalValue(nativeValue->ToObject(engine->GetEcmaVm()));

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_coerce_to_string(napi_env env, napi_value value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto nativeValue = LocalValueFromJsValue(value);
    *result = JsValueFromLocalValue(nativeValue->ToString(engine->GetEcmaVm()));

    return napi_clear_last_error(env);
}

// Methods to work with Objects
NAPI_EXTERN napi_status napi_get_prototype(napi_env env, napi_value object, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(), napi_object_expected);
    Local<panda::ObjectRef> obj = nativeValue->ToObject(vm);
    Local<panda::JSValueRef> val = obj->GetPrototype(vm);
    *result = JsValueFromLocalValue(val);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_property_names(napi_env env, napi_value object, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto napiVal = LocalValueFromJsValue(object);
    RETURN_STATUS_IF_FALSE(env, napiVal->IsObject() || napiVal->IsFunction(), napi_object_expected);

    auto obj = napiVal->ToObject(vm);
    Local<panda::ArrayRef> arrayVal = obj->GetOwnPropertyNames(vm);
    *result = JsValueFromLocalValue(arrayVal);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_set_property(napi_env env, napi_value object, napi_value key, napi_value value)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, key);
    CHECK_ARG(env, value);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);
    auto propKey = LocalValueFromJsValue(key);
    auto propValue = LocalValueFromJsValue(value);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(), napi_object_expected);

    Local<panda::ObjectRef> obj = nativeValue->ToObject(vm);
    obj->Set(vm, propKey, propValue);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_has_property(napi_env env, napi_value object, napi_value key, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, key);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);
    auto propKey = LocalValueFromJsValue(key);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(), napi_object_expected);
    Local<panda::ObjectRef> obj = nativeValue->ToObject(vm);
    *result = obj->Has(vm, propKey);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_property(napi_env env, napi_value object, napi_value key, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, key);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);
    auto propKey = LocalValueFromJsValue(key);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(), napi_object_expected);
    Local<panda::ObjectRef> obj = nativeValue->ToObject(vm);
    Local<panda::JSValueRef> val = obj->Get(vm, propKey);
    *result = JsValueFromLocalValue(val);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_delete_property(napi_env env, napi_value object, napi_value key, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, key);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);
    auto propKey = LocalValueFromJsValue(key);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(), napi_object_expected);
    Local<panda::ObjectRef> obj = nativeValue->ToObject(vm);
    bool deleteResult = obj->Delete(vm, propKey);
    if (result) {
        *result = deleteResult;
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_has_own_property(napi_env env, napi_value object, napi_value key, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, key);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);
    auto propKey = LocalValueFromJsValue(key);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(), napi_object_expected);
    Local<panda::ObjectRef> obj = nativeValue->ToObject(vm);
    bool hasResult = obj->Has(vm, propKey);
    if (result) {
        *result = hasResult;
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_set_named_property(napi_env env, napi_value object, const char* utf8name, napi_value value)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, utf8name);
    CHECK_ARG(env, value);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);
    auto propKey = LocalValueFromJsValue(value);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(), napi_object_expected);
    Local<panda::StringRef> key = panda::StringRef::NewFromUtf8(vm, utf8name);
    Local<panda::ObjectRef> obj = nativeValue->ToObject(vm);
    obj->Set(vm, key, propKey);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_has_named_property(napi_env env, napi_value object, const char* utf8name, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, utf8name);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(), napi_object_expected);

    Local<panda::StringRef> key = panda::StringRef::NewFromUtf8(vm, utf8name);
    Local<panda::ObjectRef> obj = nativeValue->ToObject(vm);
    *result = obj->Has(vm, key);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_named_property(napi_env env,
                                                napi_value object,
                                                const char* utf8name,
                                                napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, utf8name);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(), napi_object_expected);

    Local<panda::StringRef> key = panda::StringRef::NewFromUtf8(vm, utf8name);
    Local<panda::ObjectRef> obj = nativeValue->ToObject(vm);
    *result = JsValueFromLocalValue(obj->Get(vm, key));

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_own_property_descriptor(napi_env env,
                                                         napi_value object,
                                                         const char* utf8name,
                                                         napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, utf8name);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(), napi_object_expected);

    Local<panda::ObjectRef> obj = nativeValue->ToObject(vm);
    Local<panda::StringRef> key = panda::StringRef::NewFromUtf8(vm, utf8name);
    panda::PropertyAttribute property;
    obj->GetOwnProperty(vm, key, property);
    *result = JsValueFromLocalValue(property.GetValue(vm));
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_set_element(napi_env env, napi_value object, uint32_t index, napi_value value)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, value);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);
    auto elementValue = LocalValueFromJsValue(value);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(), napi_object_expected);

    Local<panda::ObjectRef> obj = nativeValue->ToObject(vm);
    obj->Set(vm, index, elementValue);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_has_element(napi_env env, napi_value object, uint32_t index, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);

    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(), napi_object_expected);

    Local<panda::ObjectRef> obj = nativeValue->ToObject(vm);
    *result = obj->Has(vm, index);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_element(napi_env env, napi_value object, uint32_t index, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);

    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(), napi_object_expected);

    Local<panda::ObjectRef> obj = nativeValue->ToObject(vm);
    *result = JsValueFromLocalValue(obj->Get(vm, index));

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_delete_element(napi_env env, napi_value object, uint32_t index, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(), napi_object_expected);
    Local<panda::ObjectRef> obj = nativeValue->ToObject(vm);
    bool deleteResult = obj->Delete(vm, index);
    if (result) {
        *result = deleteResult;
    }

    return napi_clear_last_error(env);
}

Local<panda::JSValueRef> NapiCreateFunction(napi_env env, const char* name, NapiNativeCallback cb, void* value)
{
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());
    NapiNativeFunctionInfo* funcInfo = NapiNativeFunctionInfo::CreateNewInstance();
    if (funcInfo == nullptr) {
        HILOG_ERROR("funcInfo is nullptr");
        return panda::JSValueRef::Undefined(vm);
    }
    funcInfo->env = env;
    funcInfo->callback = cb;
    funcInfo->data = value;
#ifdef ENABLE_CONTAINER_SCOPE
    funcInfo->scopeId = OHOS::Ace::ContainerScope::CurrentId();
#endif

    Local<panda::FunctionRef> fn = panda::FunctionRef::New(vm, NativeFunctionCallBack,
                                             [](void* externalPointer, void* data) {
                                                auto info = reinterpret_cast<NapiNativeFunctionInfo*>(data);
                                                if (info != nullptr) {
                                                    delete info;
                                                }
                                             },
                                             reinterpret_cast<void*>(funcInfo), true);
    Local<panda::StringRef> fnName = panda::StringRef::NewFromUtf8(vm, name);
    fn->SetName(vm, fnName);
    return fn;
}

bool NapiDefineProperty(napi_env env, Local<panda::ObjectRef> &obj, Napi_NativePropertyDescriptor propertyDescriptor)
{
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    bool result = false;
    Local<panda::StringRef> propertyName = panda::StringRef::NewFromUtf8(vm, propertyDescriptor.utf8name);

    bool writable = (propertyDescriptor.attributes & NATIVE_WRITABLE) != 0;
    bool enumable = (propertyDescriptor.attributes & NATIVE_ENUMERABLE) != 0;
    bool configable = (propertyDescriptor.attributes & NATIVE_CONFIGURABLE) != 0;

    std::string fullName("");
    if (propertyDescriptor.getter != nullptr || propertyDescriptor.setter != nullptr) {
        Local<panda::JSValueRef> localGetter = panda::JSValueRef::Undefined(vm);
        Local<panda::JSValueRef> localSetter = panda::JSValueRef::Undefined(vm);

        if (propertyDescriptor.getter != nullptr) {
            fullName += "getter";
            localGetter = NapiCreateFunction(env, fullName.c_str(),
                                             propertyDescriptor.getter, propertyDescriptor.data);
        }
        if (propertyDescriptor.setter != nullptr) {
            fullName += "setter";
            localSetter = NapiCreateFunction(env, fullName.c_str(),
                                             propertyDescriptor.setter, propertyDescriptor.data);
        }

        PropertyAttribute attr(panda::JSValueRef::Undefined(vm), false, enumable, configable);
        result = obj->SetAccessorProperty(vm, propertyName, localGetter, localSetter, attr);
    } else if (propertyDescriptor.method != nullptr) {
        fullName += propertyDescriptor.utf8name;
        Local<panda::JSValueRef> cbObj = NapiCreateFunction(env, fullName.c_str(),
                                             propertyDescriptor.method, propertyDescriptor.data);
        PropertyAttribute attr(cbObj, writable, enumable, configable);
        result = obj->DefineProperty(vm, propertyName, attr);
    } else {
        Local<panda::JSValueRef> val = LocalValueFromJsValue(propertyDescriptor.value);

        PropertyAttribute attr(val, writable, enumable, configable);
        result = obj->DefineProperty(vm, propertyName, attr);
    }
    Local<panda::ObjectRef> excep = panda::JSNApi::GetUncaughtException(vm);
    if (!excep.IsNull()) {
        HILOG_ERROR("ArkNativeObject::DefineProperty occur Exception");
        panda::JSNApi::GetAndClearUncaughtException(vm);
    }
    return result;
}

NAPI_EXTERN napi_status napi_define_properties(napi_env env,
                                               napi_value object,
                                               size_t property_count,
                                               const napi_property_descriptor* properties)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, properties);

    auto nativeValue = LocalValueFromJsValue(object);
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(),
        napi_object_expected);
    Local<panda::ObjectRef> nativeObject = nativeValue->ToObject(vm);

    for (size_t i = 0; i < property_count; i++) {
        Napi_NativePropertyDescriptor property;
        property.utf8name = properties[i].utf8name;
        property.name = properties[i].name;
        property.method = reinterpret_cast<NapiNativeCallback>(properties[i].method);
        property.getter = reinterpret_cast<NapiNativeCallback>(properties[i].getter);
        property.setter = reinterpret_cast<NapiNativeCallback>(properties[i].setter);
        property.value = properties[i].value;
        property.attributes = (uint32_t)properties[i].attributes;
        property.data = properties[i].data;
        NapiDefineProperty(env, nativeObject, property);
    }
    return napi_clear_last_error(env);
}

// Methods to work with Arrays
NAPI_EXTERN napi_status napi_is_array(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsJSArray(vm);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_array_length(napi_env env, napi_value value, uint32_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(value);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsArray(vm), napi_array_expected);
    Local<panda::ArrayRef> arr = nativeValue->ToObject(vm);
    *result = arr->Length(vm);

    return napi_clear_last_error(env);
}

// Methods to compare values
NAPI_EXTERN napi_status napi_strict_equals(napi_env env, napi_value lhs, napi_value rhs, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, lhs);
    CHECK_ARG(env, rhs);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeLhs = LocalValueFromJsValue(lhs);
    auto nativeRhs = LocalValueFromJsValue(rhs);
    *result = nativeLhs->IsStrictEquals(vm, nativeRhs);
    return napi_clear_last_error(env);
}

// Methods to work with Functions
NAPI_EXTERN napi_status napi_call_function(napi_env env,
                                           napi_value recv,
                                           napi_value func,
                                           size_t argc,
                                           const napi_value* argv,
                                           napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, func);
    if (argc > 0) {
        CHECK_ARG(env, argv);
    }

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeFunc = LocalValueFromJsValue(func);
    RETURN_STATUS_IF_FALSE(env, nativeFunc->IsFunction(), napi_function_expected);

    Local<panda::FunctionRef> function = nativeFunc->ToObject(vm);
    Local<panda::JSValueRef> thisObj = panda::JSValueRef::Undefined(vm);
    if (recv != nullptr) {
        thisObj = LocalValueFromJsValue(recv);
    }
#ifdef ENABLE_CONTAINER_SCOPE
    int32_t scopeId = OHOS::Ace::ContainerScope::CurrentId();
    auto fucInfo = reinterpret_cast<NapiNativeFunctionInfo*>(function->GetData(vm));
    if (fucInfo != nullptr) {
        scopeId = fucInfo->scopeId;
    }
    OHOS::Ace::ContainerScope containerScope(scopeId);
#endif
    std::vector<Local<panda::JSValueRef>> args;
    args.reserve(argc);
    for (size_t i = 0; i < argc; i++) {
        if (argv[i] != nullptr) {
            args.emplace_back(LocalValueFromJsValue(argv[i]));
        } else {
            args.emplace_back(panda::JSValueRef::Undefined(vm));
        }
    }

    Local<panda::JSValueRef> value = function->Call(vm, thisObj, args.data(), argc);
    if (panda::JSNApi::HasPendingException(vm)) {
        HILOG_ERROR("pending exception when js function called");
        HILOG_ERROR("print exception info: ");
        panda::JSNApi::PrintExceptionInfo(vm);
        return napi_set_last_error(env, napi_function_expected);
    }
    if (result) {
        *result = JsValueFromLocalValue(value);
    }
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_new_instance(
    napi_env env, napi_value constructor, size_t argc, const napi_value* argv, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, constructor);
    if (argc > 0) {
        CHECK_ARG(env, argv);
    }
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeConstructor = LocalValueFromJsValue(constructor);
    RETURN_STATUS_IF_FALSE(env, nativeConstructor->IsFunction(), napi_function_expected);

    Local<panda::FunctionRef> constructorVal = nativeConstructor->ToObject(vm);
    std::vector<Local<panda::JSValueRef>> args;
    args.reserve(argc);
    for (size_t i = 0; i < argc; i++) {
        if (argv[i] != nullptr) {
            Local<panda::JSValueRef> arg = LocalValueFromJsValue(argv[i]);
            args.emplace_back(arg);
        } else {
            args.emplace_back(panda::JSValueRef::Undefined(vm));
        }
    }
    Local<panda::JSValueRef> instance = constructorVal->Constructor(vm, args.data(), argc);
    Local<panda::ObjectRef> excep = panda::JSNApi::GetUncaughtException(vm);
    if (!excep.IsNull()) {
        HILOG_ERROR("ArkNativeEngineImpl::CreateInstance occur Exception");
        *result = nullptr;
    } else {
        *result = JsValueFromLocalValue(instance);
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_instanceof(napi_env env, napi_value object, napi_value constructor, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, constructor);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);
    auto nativeConstructor = LocalValueFromJsValue(constructor);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject(), napi_object_expected);
    RETURN_STATUS_IF_FALSE(env, nativeConstructor->IsFunction(), napi_function_expected);
    *result = nativeValue->InstanceOf(vm, nativeConstructor);

    return napi_clear_last_error(env);
}

// Methods to work with napi_callbacks
// Gets all callback info in a single call. (Ugly, but faster.)
NAPI_EXTERN napi_status napi_get_cb_info(napi_env env,              // [in] NAPI environment handle
                                         napi_callback_info cbinfo, // [in] Opaque callback-info handle
                                         size_t* argc,         // [in-out] Specifies the size of the provided argv array
                                                               // and receives the actual count of args.
                                         napi_value* argv,     // [out] Array of values
                                         napi_value* this_arg, // [out] Receives the JS 'this' arg for the call
                                         void** data)          // [out] Receives the data pointer for the callback.
{
    CHECK_ENV(env);
    CHECK_ARG(env, cbinfo);

    auto info = reinterpret_cast<NapiNativeCallbackInfo*>(cbinfo);

    if ((argc != nullptr) && (argv != nullptr)) {
        size_t i = 0;
        for (i = 0; (i < *argc) && (i < info->argc); i++) {
            argv[i] = info->argv[i];
        }
        *argc = i;
    }
    if (argc != nullptr) {
        *argc = info->argc;
    }
    if (this_arg != nullptr) {
        *this_arg = info->thisVar;
    }
    if (data != nullptr && info->functionInfo != nullptr) {
        *data = info->functionInfo->data;
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_new_target(napi_env env, napi_callback_info cbinfo, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, cbinfo);
    CHECK_ARG(env, result);

    auto info = reinterpret_cast<NapiNativeCallbackInfo*>(cbinfo);
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto thisVarObj = LocalValueFromJsValue(info->thisVar);
    auto functionVal = LocalValueFromJsValue(info->function);
    if (thisVarObj->InstanceOf(vm, functionVal)) {
        *result = info->function;
    } else {
        *result = nullptr;
    }

    return napi_clear_last_error(env);
}

Local<panda::JSValueRef> NapiDefineClass(napi_env env, const char* name, NapiNativeCallback callback,
    void* data, const Napi_NativePropertyDescriptor* properties, size_t length)
{
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());
    std::string className(name);

    NapiNativeFunctionInfo* funcInfo = NapiNativeFunctionInfo::CreateNewInstance();
    if (funcInfo == nullptr) {
        HILOG_ERROR("funcInfo is nullptr");
        return panda::JSValueRef::Undefined(vm);
    }
    funcInfo->env = env;
    funcInfo->callback = callback;
    funcInfo->data = data;
#ifdef ENABLE_CONTAINER_SCOPE
    funcInfo->scopeId = OHOS::Ace::ContainerScope::CurrentId();
#endif

    Local<panda::FunctionRef> fn = panda::FunctionRef::NewClassFunction(vm, NativeFunctionCallBack,
                                                    [](void* externalPointer, void* data) {
                                                        auto info = reinterpret_cast<NapiNativeFunctionInfo*>(data);
                                                        if (info != nullptr) {
                                                            delete info;
                                                        }
                                                    },
                                                    reinterpret_cast<void*>(funcInfo), true);
    Local<panda::StringRef> fnName = panda::StringRef::NewFromUtf8(vm, className.c_str());
    fn->SetName(vm, fnName);
    Local<panda::ObjectRef> classPrototype = fn->GetFunctionPrototype(vm);
    Local<panda::ObjectRef> fnObj = fn->ToObject(vm);
    for (size_t i = 0; i < length; i++) {
        if (properties[i].attributes & NATIVE_STATIC) {
            NapiDefineProperty(env, fnObj, properties[i]);
        } else {
            if (classPrototype->IsUndefined()) {
                HILOG_ERROR("ArkNativeEngineImpl::Class's prototype is null");
                continue;
            }

            NapiDefineProperty(env, classPrototype, properties[i]);
        }
    }

    return fn;
}

NAPI_EXTERN napi_status napi_define_class(napi_env env,
                                          const char* utf8name,
                                          size_t length,
                                          napi_callback constructor,
                                          void* data,
                                          size_t property_count,
                                          const napi_property_descriptor* properties,
                                          napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, utf8name);
    RETURN_STATUS_IF_FALSE(env, length == NAPI_AUTO_LENGTH || length <= INT_MAX, napi_object_expected);
    CHECK_ARG(env, constructor);
    if (property_count > 0) {
        CHECK_ARG(env, properties);
    }
    CHECK_ARG(env, result);

    auto callback = reinterpret_cast<NapiNativeCallback>(constructor);
    auto nativeProperties = reinterpret_cast<const Napi_NativePropertyDescriptor*>(properties);

    size_t nameLength = std::min(length, strlen(utf8name));
    char newName[nameLength + 1];
    if (strncpy_s(newName, nameLength + 1, utf8name, nameLength) != EOK) {
        HILOG_ERROR("napi_define_class strncpy_s failed");
        *result = nullptr;
    } else {
        auto resultValue = NapiDefineClass(env, newName, callback, data, nativeProperties, property_count);
        *result = JsValueFromLocalValue(resultValue);
    }

    return napi_clear_last_error(env);
}

// Methods to work with external data objects
NAPI_EXTERN napi_status napi_wrap(napi_env env,
                                  napi_value js_object,
                                  void* native_object,
                                  napi_finalize finalize_cb,
                                  void* finalize_hint,
                                  napi_ref* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, native_object);
    CHECK_ARG(env, finalize_cb);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());
    auto nativeValue = LocalValueFromJsValue(js_object);
    auto callback = reinterpret_cast<NativeFinalize>(finalize_cb);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(),
        napi_object_expected);
    auto nativeObject = nativeValue->ToObject(vm);
    size_t  nativeBindingSize = 0;
    auto reference = reinterpret_cast<NativeReference**>(result);
    Local<panda::StringRef> key = panda::StringRef::GetNapiWrapperString(vm);
    if (native_object == nullptr && nativeObject->Has(vm, key)) {
        Local<panda::ObjectRef> wrapper = nativeObject->Get(vm, key);
        auto ref = reinterpret_cast<NativeReference*>(wrapper->GetNativePointerField(0));
        // Try to remove native pointer from ArrayDataList
        ASSERT(nativeBindingSize == 0);
        wrapper->SetNativePointerField(0, nullptr, nullptr, nullptr, nativeBindingSize);
        nativeObject->Delete(vm, key);
        delete ref;
    } else {
        Local<panda::ObjectRef> object = panda::ObjectRef::New(vm);
        NativeReference* ref = nullptr;
        if (reference != nullptr) {
            ref = engine->CreateReference(JsValueFromLocalValue(nativeObject), 1, false, callback, native_object, finalize_hint);
            *reference = ref;
        } else {
            ref = engine->CreateReference(JsValueFromLocalValue(nativeObject), 0, true, callback, native_object, finalize_hint);

        }
        object->SetNativePointerFieldCount(1);
        object->SetNativePointerField(0, ref, nullptr, nullptr, nativeBindingSize);
        PropertyAttribute attr(object, true, false, true);
        nativeObject->DefineProperty(vm, key, attr);
    }
    return napi_clear_last_error(env);
}

// Methods to work with external data objects
NAPI_EXTERN napi_status napi_wrap_with_size(napi_env env,
                                            napi_value js_object,
                                            void* native_object,
                                            napi_finalize finalize_cb,
                                            void* finalize_hint,
                                            napi_ref* result,
                                            size_t native_binding_size)
{
    CHECK_ENV(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, native_object);
    CHECK_ARG(env, finalize_cb);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());
    auto nativeValue = LocalValueFromJsValue(js_object);
    auto callback = reinterpret_cast<NativeFinalize>(finalize_cb);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(),
        napi_object_expected);
    auto nativeObject = nativeValue->ToObject(vm);
    auto reference = reinterpret_cast<NativeReference**>(result);
    Local<panda::StringRef> key = panda::StringRef::GetNapiWrapperString(vm);
    if (native_object == nullptr && nativeObject->Has(vm, key)) {
        Local<panda::ObjectRef> wrapper = nativeObject->Get(vm, key);
        auto ref = reinterpret_cast<NativeReference*>(wrapper->GetNativePointerField(0));
        // Try to remove native pointer from ArrayDataList
        ASSERT(native_binding_size == 0);
        wrapper->SetNativePointerField(0, nullptr, nullptr, nullptr, native_binding_size);
        nativeObject->Delete(vm, key);
        delete ref;
    } else {
        Local<panda::ObjectRef> object = panda::ObjectRef::New(vm);
        NativeReference* ref = nullptr;
        if (reference != nullptr) {
            ref = engine->CreateReference(JsValueFromLocalValue(nativeObject), 1, false, callback, native_object, finalize_hint);
            *reference = ref;
        } else {
            ref = engine->CreateReference(JsValueFromLocalValue(nativeObject), 0, true, callback, native_object, finalize_hint);
        }
        object->SetNativePointerFieldCount(1);
        object->SetNativePointerField(0, ref, nullptr, nullptr, native_binding_size);
        PropertyAttribute attr(object, true, false, true);
        nativeObject->DefineProperty(vm, key, attr);
    }
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_unwrap(napi_env env, napi_value js_object, void** result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());
    auto nativeValue = LocalValueFromJsValue(js_object);

    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(),
        napi_object_expected);

    auto nativeObject = nativeValue->ToObject(vm);

    Local<panda::StringRef> key = panda::StringRef::GetNapiWrapperString(vm);
    Local<panda::JSValueRef> val = nativeObject->Get(vm, key);

    if (val->IsObject()) {
        Local<panda::ObjectRef> ext(val);
        auto ref = reinterpret_cast<NativeReference*>(ext->GetNativePointerField(0));
        *result = ref != nullptr ? ref->GetData() : nullptr;
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_remove_wrap(napi_env env, napi_value js_object, void** result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());
    auto nativeValue = LocalValueFromJsValue(js_object);

    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(),
        napi_object_expected);

    auto nativeObject = nativeValue->ToObject(vm);

    Local<panda::StringRef> key = panda::StringRef::GetNapiWrapperString(vm);
    Local<panda::JSValueRef> val = nativeObject->Get(vm, key);

    if (val->IsObject()) {
        Local<panda::ObjectRef> ext(val);
        auto ref = reinterpret_cast<NativeReference*>(ext->GetNativePointerField(0));
        *result = ref != nullptr ? ref->GetData() : nullptr;
    }

    size_t nativeBindingSize = 0;
    if (nativeObject->Has(vm, key)) {
        Local<panda::ObjectRef> wrapper =val;
        auto ref = reinterpret_cast<NativeReference*>(wrapper->GetNativePointerField(0));
        // Try to remove native pointer from ArrayDataList
        ASSERT(nativeBindingSize == 0);
        wrapper->SetNativePointerField(0, nullptr, nullptr, nullptr, nativeBindingSize);
        nativeObject->Delete(vm, key);
        delete ref;
    } else {
        Local<panda::ObjectRef> object = panda::ObjectRef::New(vm);
        NativeReference* ref = nullptr;
        ref = engine->CreateReference(JsValueFromLocalValue(nativeObject), 0, true, nullptr, nullptr, nullptr);
        object->SetNativePointerFieldCount(1);
        object->SetNativePointerField(0, ref, nullptr, nullptr, nativeBindingSize);
        PropertyAttribute attr(object, true, false, true);
        nativeObject->DefineProperty(vm, key, attr);
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_external(
    napi_env env, void* data, napi_finalize finalize_cb, void* finalize_hint, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto callback = reinterpret_cast<NativeFinalize>(finalize_cb);
    NativeObjectInfo* info = NativeObjectInfo::CreateNewInstance();
    if (info == nullptr) {
        HILOG_ERROR("info is nullptr");
        return napi_set_last_error(env, napi_function_expected);
    }
    info->engine = engine;
    info->nativeObject = nullptr;
    info->callback = callback;
    info->hint = finalize_hint;
    size_t nativeBindingSize = 0;
    Local<panda::NativePointerRef> object = panda::NativePointerRef::New(vm, data,
        [](void* data, void* info){
            auto externalInfo = reinterpret_cast<NativeObjectInfo*>(info);
            auto engine = externalInfo->engine;
            auto callback = externalInfo->callback;
            auto hint = externalInfo->hint;
            if (callback != nullptr) {
                callback(engine, data, hint);
            }
            delete externalInfo;
        }, info, nativeBindingSize);

    *result = JsValueFromLocalValue(object);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_external_with_size(napi_env env, void* data, napi_finalize finalize_cb,
    void* finalize_hint, napi_value* result, size_t native_binding_size)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto callback = reinterpret_cast<NativeFinalize>(finalize_cb);
    NativeObjectInfo* info = NativeObjectInfo::CreateNewInstance();
    if (info == nullptr) {
        HILOG_ERROR("info is nullptr");
        return napi_set_last_error(env, napi_function_expected);
    }
    info->engine = engine;
    info->nativeObject = nullptr;
    info->callback = callback;
    info->hint = finalize_hint;
    Local<panda::NativePointerRef> object = panda::NativePointerRef::New(vm, data,
        [](void* data, void* info){
            auto externalInfo = reinterpret_cast<NativeObjectInfo*>(info);
            auto engine = externalInfo->engine;
            auto callback = externalInfo->callback;
            auto hint = externalInfo->hint;
            if (callback != nullptr) {
                callback(engine, data, hint);
            }
            delete externalInfo;
        }, info, native_binding_size);

    *result = JsValueFromLocalValue(object);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_external(napi_env env, napi_value value, void** result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsNativePointer(), napi_object_expected);
    Local<panda::NativePointerRef> object = nativeValue->ToNativePointer(vm);
    *result = object->Value();
    return napi_clear_last_error(env);
}

// Methods to control object lifespan
// Set initial_refcount to 0 for a weak reference, >0 for a strong reference.
NAPI_EXTERN napi_status napi_create_reference(napi_env env,
                                              napi_value value,
                                              uint32_t initial_refcount,
                                              napi_ref* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);
    auto engine = reinterpret_cast<NativeEngine*>(env);
    NativeReference* ref = engine->CreateReference(value, initial_refcount, false, nullptr, nullptr, nullptr);

    *result = reinterpret_cast<napi_ref>(ref);
    return napi_clear_last_error(env);
}

// Deletes a reference. The referenced value is released, and may
// be GC'd unless there are other references to it.
NAPI_EXTERN napi_status napi_delete_reference(napi_env env, napi_ref ref)
{
    CHECK_ENV(env);
    CHECK_ARG(env, ref);

    auto reference = reinterpret_cast<NativeReference*>(ref);
    uint32_t refCount = reference->GetRefCount();
    if (refCount > 0 || reference->GetFinalRun()) {
        delete reference;
        reference = nullptr;
    } else {
        reference->SetDeleteSelf();
    }

    return napi_clear_last_error(env);
}

// Increments the reference count, optionally returning the resulting count.
// After this call the  reference will be a strong reference because its
// refcount is >0, and the referenced object is effectively "pinned".
// Calling this when the refcount is 0 and the object is unavailable
// results in an error.
NAPI_EXTERN napi_status napi_reference_ref(napi_env env, napi_ref ref, uint32_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, ref);

    auto reference = reinterpret_cast<NativeReference*>(ref);
    uint32_t refCount = reference->Ref();

    if (result) {
        *result = refCount;
    }

    return napi_clear_last_error(env);
}

// Decrements the reference count, optionally returning the resulting count.
// If the result is 0 the reference is now weak and the object may be GC'd
// at any time if there are no other references. Calling this when the
// refcount is already 0 results in an error.
NAPI_EXTERN napi_status napi_reference_unref(napi_env env, napi_ref ref, uint32_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, ref);

    auto reference = reinterpret_cast<NativeReference*>(ref);
    uint32_t unrefCount = reference->Unref();

    if (result) {
        *result = unrefCount;
    }

    return napi_clear_last_error(env);
}

// Attempts to get a referenced value. If the reference is weak,
// the value might no longer be available, in that case the call
// is still successful but the result is nullptr.
NAPI_EXTERN napi_status napi_get_reference_value(napi_env env, napi_ref ref, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, ref);
    CHECK_ARG(env, result);

    auto reference = reinterpret_cast<NativeReference*>(ref);

    *result = reference->Get();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_open_handle_scope(napi_env env, napi_handle_scope* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    *result = HandleScopeToNapiHandleScope(new HandleScopeWrapper(engine));
    engine->openHandleScopes_++;
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_close_handle_scope(napi_env env, napi_handle_scope scope)
{
    CHECK_ENV(env);
    CHECK_ARG(env, scope);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (engine->openHandleScopes_ == 0) {
        return napi_handle_scope_mismatch;
    }

    engine->openHandleScopes_--;
    auto handleScope = NapiHandleScopeToHandleScope(scope);
    delete handleScope;
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_open_escapable_handle_scope(napi_env env, napi_escapable_handle_scope* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    *result = EscapableHandleScopeToNapiEscapableHandleScope(new EscapableHandleScopeWrapper(engine));
    engine->openHandleScopes_++;
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_close_escapable_handle_scope(napi_env env, napi_escapable_handle_scope scope)
{
    CHECK_ENV(env);
    CHECK_ARG(env, scope);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (engine->openHandleScopes_ == 0) {
        return napi_handle_scope_mismatch;
    }

    engine->openHandleScopes_--;
    delete NapiEscapableHandleScopeToEscapableHandleScope(scope);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_escape_handle(napi_env env,
                                           napi_escapable_handle_scope scope,
                                           napi_value escapee,
                                           napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, scope);
    CHECK_ARG(env, escapee);
    CHECK_ARG(env, result);

    auto s = NapiEscapableHandleScopeToEscapableHandleScope(scope);
    if (!s->IsEscapeCalled()) {
        *result = JsValueFromLocalValue(s->Escape(LocalValueFromJsValue(escapee)));
        return napi_clear_last_error(env);
    }
    return napi_set_last_error(env, napi_escape_called_twice);
}

// Methods to support error handling
NAPI_EXTERN napi_status napi_throw(napi_env env, napi_value error)
{
    CHECK_ENV(env);
    CHECK_ARG(env, error);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(error);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsError(), napi_invalid_arg);

    panda::JSNApi::ThrowException(vm, nativeValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_throw_error(napi_env env, const char* code, const char* msg)
{
    CHECK_ENV(env);
    CHECK_ARG(env, msg);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<panda::JSValueRef> error(panda::JSValueRef::Undefined(vm));
    error = panda::Exception::Error(vm, StringRef::NewFromUtf8(vm, msg));
    if (code != nullptr) {
        Local<panda::JSValueRef> codeKey = panda::StringRef::NewFromUtf8(vm, "code");
        Local<panda::JSValueRef> codeValue = panda::StringRef::NewFromUtf8(vm, code);
        Local<panda::ObjectRef> errorObj(error);
        errorObj->Set(vm, codeKey, codeValue);
    }
    panda::JSNApi::ThrowException(vm, error);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_throw_type_error(napi_env env, const char* code, const char* msg)
{
    CHECK_ENV(env);
    CHECK_ARG(env, msg);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<panda::JSValueRef> error(panda::JSValueRef::Undefined(vm));
    error = panda::Exception::TypeError(vm, StringRef::NewFromUtf8(vm, msg));
    if (code != nullptr) {
        Local<panda::JSValueRef> codeKey = panda::StringRef::NewFromUtf8(vm, "code");
        Local<panda::JSValueRef> codeValue = panda::StringRef::NewFromUtf8(vm, code);
        Local<panda::ObjectRef> errorObj(error);
        errorObj->Set(vm, codeKey, codeValue);
    }
    panda::JSNApi::ThrowException(vm, error);
    return napi_generic_failure;
}

NAPI_EXTERN napi_status napi_throw_range_error(napi_env env, const char* code, const char* msg)
{
    CHECK_ENV(env);
    CHECK_ARG(env, msg);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<panda::JSValueRef> error(panda::JSValueRef::Undefined(vm));
    error = panda::Exception::RangeError(vm, StringRef::NewFromUtf8(vm, msg));
    if (code != nullptr) {
        Local<panda::JSValueRef> codeKey = panda::StringRef::NewFromUtf8(vm, "code");
        Local<panda::JSValueRef> codeValue = panda::StringRef::NewFromUtf8(vm, code);
        Local<panda::ObjectRef> errorObj(error);
        errorObj->Set(vm, codeKey, codeValue);
    }
    panda::JSNApi::ThrowException(vm, error);
    return napi_generic_failure;
}

NAPI_EXTERN napi_status napi_is_error(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);
    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsError();

    return napi_clear_last_error(env);
}

// Methods to support catching exceptions
NAPI_EXTERN napi_status napi_is_exception_pending(napi_env env, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    *result = panda::JSNApi::HasPendingException(engine->GetEcmaVm());
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_and_clear_last_exception(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    Local<panda::ObjectRef> exception = panda::JSNApi::GetAndClearUncaughtException(engine->GetEcmaVm());
    if (!exception.IsNull()) {
        *result = JsValueFromLocalValue(exception);
    }

    return napi_clear_last_error(env);
}

// Methods to work with array buffers and typed arrays
NAPI_EXTERN napi_status napi_is_arraybuffer(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);
    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsArrayBuffer();

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_arraybuffer(napi_env env, size_t byte_length, void** data, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, data);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    uint8_t** values = (uint8_t**)(data);
    Local<panda::ArrayBufferRef> res = panda::ArrayBufferRef::New(engine->GetEcmaVm(), byte_length);
    if (values != nullptr) {
        *values = reinterpret_cast<uint8_t*>(res->GetBuffer());
    }
    *result = JsValueFromLocalValue(res);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_external_arraybuffer(napi_env env,
                                                         void* external_data,
                                                         size_t byte_length,
                                                         napi_finalize finalize_cb,
                                                         void* finalize_hint,
                                                         napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, external_data);
    CHECK_ARG(env, finalize_cb);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto callback = reinterpret_cast<NativeFinalize>(finalize_cb);
    uint8_t* value = (uint8_t*)external_data;
    if (!value) {
        HILOG_ERROR("value is empty");
        return napi_set_last_error(env, napi_invalid_arg);
    }
    if (byte_length > MAX_BYTE_LENGTH) {
        HILOG_ERROR("Creat failed, current size: %{public}2f MiB, limit size: %{public}2f MiB",
                    static_cast<float>(byte_length) / static_cast<float>(ONEMIB_BYTE_SIZE),
                    static_cast<float>(MAX_BYTE_LENGTH) / static_cast<float>(ONEMIB_BYTE_SIZE));
        value = nullptr;
        return napi_set_last_error(env, napi_invalid_arg);
    }

    NativeObjectInfo* cbinfo = NativeObjectInfo::CreateNewInstance();
    if (cbinfo == nullptr) {
        HILOG_ERROR("cbinfo is nullptr");
        return napi_set_last_error(env, napi_function_expected);
    }
    cbinfo->engine = engine;
    cbinfo->callback = callback;
    cbinfo->hint = finalize_hint;

    Local<panda::ArrayBufferRef> object = panda::ArrayBufferRef::New(vm, value, byte_length,
        [](void* data, void* info) {
            auto externalInfo = reinterpret_cast<NativeObjectInfo*>(info);
            auto engine = externalInfo->engine;
            auto callback = externalInfo->callback;
            auto hint = externalInfo->hint;
            if (callback != nullptr) {
                callback(engine, data, hint);
            }
            delete externalInfo;
        },
        cbinfo);

    void* ptr = object->GetBuffer();
    CHECK_ARG(env, ptr);
    *result = JsValueFromLocalValue(object);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_arraybuffer_info(napi_env env,
                                                  napi_value arraybuffer,
                                                  void** data,
                                                  size_t* byte_length)
{
    CHECK_ENV(env);
    CHECK_ARG(env, arraybuffer);
    CHECK_ARG(env, data);
    CHECK_ARG(env, byte_length);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(arraybuffer);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsArrayBuffer(), napi_status::napi_arraybuffer_expected);
    Local<panda::ArrayBufferRef> res = nativeValue->ToObject(vm);
    *data = res->GetBuffer();
    *byte_length = res->ByteLength(vm);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_typedarray(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsTypedArray();

    return napi_clear_last_error(env);
}

EXTERN_C_START
NAPI_EXTERN napi_status napi_is_buffer(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsBuffer();

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_buffer(napi_env env, size_t size, void** data, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, data);
    CHECK_ARG(env, result);

    RETURN_STATUS_IF_FALSE(env, size > 0, napi_invalid_arg);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    uint8_t** value =  reinterpret_cast<uint8_t**>(data);
    if (!value) {
        HILOG_ERROR("value is empty");
        return napi_set_last_error(env, napi_invalid_arg);
    }

    if (size > MAX_BYTE_LENGTH) {
        HILOG_ERROR("Creat failed, current size: %{public}2f MiB, limit size: %{public}2f MiB",
                    static_cast<float>(size) / static_cast<float>(ONEMIB_BYTE_SIZE),
                    static_cast<float>(MAX_BYTE_LENGTH) / static_cast<float>(ONEMIB_BYTE_SIZE));
        *value = nullptr;
        return napi_set_last_error(env, napi_invalid_arg);
    }
    Local<panda::BufferRef> obj = BufferRef::New(vm, size);
    *value = reinterpret_cast<uint8_t*>(obj->GetBuffer());

    CHECK_ARG(env, *data);
    void* ptr = obj->GetBuffer();
    CHECK_ARG(env, ptr);

    *result = JsValueFromLocalValue(obj);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_buffer_copy(
    napi_env env, size_t length, const void* data, void** result_data, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, data);
    CHECK_ARG(env, result_data);
    CHECK_ARG(env, result);
    RETURN_STATUS_IF_FALSE(env, length > 0, napi_invalid_arg);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();

    uint8_t** value = reinterpret_cast<uint8_t**>(result_data);
    const uint8_t* recvdata = (uint8_t*)data;
    if (!value) {
        HILOG_ERROR("value is empty");
        return napi_set_last_error(env, napi_invalid_arg);
    }
    if (length > MAX_BYTE_LENGTH) {
        HILOG_ERROR("Creat failed, current size: %{public}2f MiB, limit size: %{public}2f MiB",
                    static_cast<float>(length) / static_cast<float>(ONEMIB_BYTE_SIZE),
                    static_cast<float>(MAX_BYTE_LENGTH) / static_cast<float>(ONEMIB_BYTE_SIZE));
        *value = nullptr;
        return napi_set_last_error(env, napi_invalid_arg);
    }
    Local<panda::BufferRef> obj = BufferRef::New(vm, length);
    if (obj->IsUndefined()) {
        HILOG_INFO("engine create buffer_copy failed!");
    }
    *value = reinterpret_cast<uint8_t*>(obj->GetBuffer());
    if (memcpy_s(*value, length, recvdata, length) != EOK) {
        HILOG_ERROR("memcpy_s failed");
    }

    void* ptr = obj->GetBuffer();
    CHECK_ARG(env, ptr);

    *result = JsValueFromLocalValue(obj);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_external_buffer(
    napi_env env, size_t length, void* data, napi_finalize finalize_cb, void* finalize_hint, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);
    CHECK_ARG(env, data);
    RETURN_STATUS_IF_FALSE(env, length > 0, napi_invalid_arg);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto callback = reinterpret_cast<NativeFinalize>(finalize_cb);
    uint8_t* value = (uint8_t*)data;
    if (!value) {
        HILOG_ERROR("value is empty");
        return napi_set_last_error(env, napi_invalid_arg);
    }
    if (length > MAX_BYTE_LENGTH) {
        HILOG_ERROR("Creat failed, current size: %{public}2f MiB, limit size: %{public}2f MiB",
                    static_cast<float>(length) / static_cast<float>(ONEMIB_BYTE_SIZE),
                    static_cast<float>(MAX_BYTE_LENGTH) / static_cast<float>(ONEMIB_BYTE_SIZE));
        value = nullptr;
        return napi_set_last_error(env, napi_invalid_arg);
    }

    std::unique_ptr<NativeObjectInfo> cbinfo(NativeObjectInfo::CreateNewInstance());
    if (!cbinfo) {
        HILOG_ERROR("cbinfo is nullptr");
        return napi_set_last_error(env, napi_function_expected);
    }
    cbinfo->engine = engine;
    cbinfo->callback = callback;
    cbinfo->hint = finalize_hint;

    Local<panda::BufferRef> object = panda::BufferRef::New(vm, value, length,
        [](void* data, void* info) {
            auto externalInfo = reinterpret_cast<NativeObjectInfo*>(info);
            auto engine = externalInfo->engine;
            auto callback = externalInfo->callback;
            auto hint = externalInfo->hint;
            if (callback != nullptr) {
                callback(engine, data, hint);
            }
            delete externalInfo;
        },
        cbinfo.get());
    cbinfo.release();
    void* ptr = object->GetBuffer();
    CHECK_ARG(env, ptr);

    *result = JsValueFromLocalValue(object);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_buffer_info(napi_env env, napi_value value, void** data, size_t* length)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(value);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsBuffer(), napi_status::napi_arraybuffer_expected);
    Local<panda::BufferRef> res = nativeValue->ToObject(vm);
    *data = res->GetBuffer();
    *length = res->ByteLength(vm);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_object_freeze(napi_env env, napi_value object)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject(), napi_object_expected);
    Local<panda::ObjectRef> obj = nativeValue->ToObject(engine->GetEcmaVm());
    obj->Freeze(vm);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_object_seal(napi_env env, napi_value object)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject(), napi_object_expected);
    Local<panda::ObjectRef> obj = nativeValue->ToObject(engine->GetEcmaVm());
    obj->Seal(vm);

    return napi_clear_last_error(env);
}

EXTERN_C_END

NAPI_EXTERN napi_status napi_create_typedarray(napi_env env,
                                               napi_typedarray_type type,
                                               size_t length,
                                               napi_value arraybuffer,
                                               size_t byte_offset,
                                               napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, arraybuffer);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto value = LocalValueFromJsValue(arraybuffer);
    auto typedArrayType = (NativeTypedArrayType)type;

    RETURN_STATUS_IF_FALSE(env, value->IsArrayBuffer(), napi_status::napi_arraybuffer_expected);
    Local<panda::ArrayBufferRef> arrayBuf = value->ToObject(vm);
    Local<panda::TypedArrayRef> typedArray(panda::JSValueRef::Undefined(vm));

    switch (typedArrayType) {
        case NATIVE_INT8_ARRAY:
            typedArray = panda::Int8ArrayRef::New(vm, arrayBuf, byte_offset, length);
            break;
        case NATIVE_UINT8_ARRAY:
            typedArray = panda::Uint8ArrayRef::New(vm, arrayBuf, byte_offset, length);
            break;
        case NATIVE_UINT8_CLAMPED_ARRAY:
            typedArray = panda::Uint8ClampedArrayRef::New(vm, arrayBuf, byte_offset, length);
            break;
        case NATIVE_INT16_ARRAY:
            typedArray = panda::Int16ArrayRef::New(vm, arrayBuf, byte_offset, length);
            break;
        case NATIVE_UINT16_ARRAY:
            typedArray = panda::Uint16ArrayRef::New(vm, arrayBuf, byte_offset, length);
            break;
        case NATIVE_INT32_ARRAY:
            typedArray = panda::Int32ArrayRef::New(vm, arrayBuf, byte_offset, length);
            break;
        case NATIVE_UINT32_ARRAY:
            typedArray = panda::Uint32ArrayRef::New(vm, arrayBuf, byte_offset, length);
            break;
        case NATIVE_FLOAT32_ARRAY:
            typedArray = panda::Float32ArrayRef::New(vm, arrayBuf, byte_offset, length);
            break;
        case NATIVE_FLOAT64_ARRAY:
            typedArray = panda::Float64ArrayRef::New(vm, arrayBuf, byte_offset, length);
            break;
        case NATIVE_BIGINT64_ARRAY:
            typedArray = panda::BigInt64ArrayRef::New(vm, arrayBuf, byte_offset, length);
            break;
        case NATIVE_BIGUINT64_ARRAY:
            typedArray = panda::BigUint64ArrayRef::New(vm, arrayBuf, byte_offset, length);
            break;
    }

    *result = JsValueFromLocalValue(typedArray);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_typedarray_info(napi_env env,
                                                 napi_value typedarray,
                                                 napi_typedarray_type* type,
                                                 size_t* length,
                                                 void** data,
                                                 napi_value* arraybuffer,
                                                 size_t* byte_offset)
{
    CHECK_ENV(env);
    CHECK_ARG(env, typedarray);

    auto value = LocalValueFromJsValue(typedarray);
    RETURN_STATUS_IF_FALSE(env, value->IsTypedArray(), napi_status::napi_invalid_arg);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<panda::TypedArrayRef> typedArray = value->ToObject(vm);
    if (type != nullptr) {
        NativeTypedArrayType thisType = NATIVE_INT8_ARRAY;
        if (typedArray->IsInt8Array()) {
            thisType = NATIVE_INT8_ARRAY;
        } else if (typedArray->IsUint8Array()) {
            thisType = NATIVE_UINT8_ARRAY;
        } else if (typedArray->IsUint8ClampedArray()) {
            thisType = NATIVE_UINT8_CLAMPED_ARRAY;
        } else if (typedArray->IsInt16Array()) {
            thisType = NATIVE_INT16_ARRAY;
        } else if (typedArray->IsUint16Array()) {
            thisType = NATIVE_UINT16_ARRAY;
        } else if (typedArray->IsInt32Array()) {
            thisType = NATIVE_INT32_ARRAY;
        } else if (typedArray->IsUint32Array()) {
            thisType = NATIVE_UINT32_ARRAY;
        } else if (typedArray->IsFloat32Array()) {
            thisType = NATIVE_FLOAT32_ARRAY;
        } else if (typedArray->IsFloat64Array()) {
            thisType = NATIVE_FLOAT64_ARRAY;
        } else if (typedArray->IsBigInt64Array()) {
            thisType = NATIVE_BIGINT64_ARRAY;
        } else if (typedArray->IsBigUint64Array()) {
            thisType = NATIVE_BIGUINT64_ARRAY;
        }
        *type = (napi_typedarray_type)(thisType);
    }
    if (length != nullptr) {
        *length = typedArray->ByteLength(vm);
    }
    if (data != nullptr) {
        *data = static_cast<uint8_t*>(typedArray->GetArrayBuffer(vm)->GetBuffer()) + typedArray->ByteOffset(vm);
    }
    if (arraybuffer != nullptr) {
        *arraybuffer = JsValueFromLocalValue(typedArray->GetArrayBuffer(vm));
    }
    if (byte_offset != nullptr) {
        *byte_offset = typedArray->ByteOffset(vm);
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_dataview(
    napi_env env, size_t length, napi_value arraybuffer, size_t byte_offset, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, arraybuffer);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto arrayBufferValue = LocalValueFromJsValue(arraybuffer);
    RETURN_STATUS_IF_FALSE(env, arrayBufferValue->IsArrayBuffer(), napi_status::napi_arraybuffer_expected);
    Local<panda::ArrayBufferRef> res = arrayBufferValue->ToObject(vm);
    if (length + byte_offset > static_cast<size_t>(res->ByteLength(vm))) {
        napi_throw_range_error(
            env,
            "ERR_NAPI_INVALID_DATAVIEW_ARGS",
            "byte_offset + byte_length should be less than or "
            "equal to the size in bytes of the array passed in");
        return napi_set_last_error(env, napi_pending_exception);
    }

    Local<panda::DataViewRef> dataView = panda::DataViewRef::New(vm, res, byte_offset, length);

    *result = JsValueFromLocalValue(dataView);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_dataview(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsDataView();

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_dataview_info(napi_env env,
                                               napi_value dataview,
                                               size_t* bytelength,
                                               void** data,
                                               napi_value* arraybuffer,
                                               size_t* byte_offset)
{
    CHECK_ENV(env);
    CHECK_ARG(env, dataview);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(dataview);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsDataView(), napi_status::napi_invalid_arg);
    Local<panda::DataViewRef> dataViewObj = nativeValue->ToObject(vm);
    if (bytelength != nullptr) {
        *bytelength = dataViewObj->ByteLength();
    }
    if (data != nullptr) {
        *data = dataViewObj->GetArrayBuffer(vm)->GetBuffer();
    }
    if (arraybuffer != nullptr) {
        *arraybuffer = JsValueFromLocalValue(dataViewObj->GetArrayBuffer(vm));
    }
    if (byte_offset != nullptr) {
        *byte_offset = dataViewObj->ByteOffset();
    }

    return napi_clear_last_error(env);
}

// version management
NAPI_EXTERN napi_status napi_get_version(napi_env env, uint32_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);
    *result = NAPI_VERSION;
    return napi_clear_last_error(env);
}

// Promises
NAPI_EXTERN napi_status napi_create_promise(napi_env env, napi_deferred* deferred, napi_value* promise)
{
    CHECK_ENV(env);
    CHECK_ARG(env, deferred);
    CHECK_ARG(env, promise);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto resultValue = engine->CreatePromise(reinterpret_cast<NativeDeferred**>(deferred));
    *promise = resultValue;

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_resolve_deferred(napi_env env, napi_deferred deferred, napi_value resolution)
{
    CHECK_ENV(env);
    CHECK_ARG(env, deferred);
    CHECK_ARG(env, resolution);

    auto nativeDeferred = reinterpret_cast<NativeDeferred*>(deferred);
    nativeDeferred->Resolve(resolution);
    delete nativeDeferred;
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_reject_deferred(napi_env env, napi_deferred deferred, napi_value rejection)
{
    CHECK_ENV(env);
    CHECK_ARG(env, deferred);
    CHECK_ARG(env, rejection);

    auto nativeDeferred = reinterpret_cast<NativeDeferred*>(deferred);
    nativeDeferred->Reject(rejection);
    delete nativeDeferred;

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_promise(napi_env env, napi_value value, bool* is_promise)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, is_promise);

    auto nativeValue = LocalValueFromJsValue(value);
    *is_promise = nativeValue->IsPromise();

    return napi_clear_last_error(env);
}

// promise reject events
NAPI_EXTERN napi_status napi_set_promise_rejection_callback(napi_env env, napi_ref ref, napi_ref checkRef)
{
    CHECK_ENV(env);
    CHECK_ARG(env, ref);
    CHECK_ARG(env, checkRef);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto rejectCallbackRef = reinterpret_cast<NativeReference*>(ref);
    auto checkCallbackRef = reinterpret_cast<NativeReference*>(checkRef);

    if (rejectCallbackRef == nullptr || checkCallbackRef == nullptr) {
        HILOG_ERROR("rejectCallbackRef or checkCallbackRef is nullptr");
    }
    else {
        engine->SetPromiseRejectCallBackRef(rejectCallbackRef);
        engine->SetCheckCallbackRef(checkCallbackRef);
        panda::JSNApi::SetHostPromiseRejectionTracker(const_cast<EcmaVM*>(engine->GetEcmaVm()), engine->GetPromiseRejectCallback(),
                                            reinterpret_cast<void*>(engine));
    }

    return napi_clear_last_error(env);
}

// Running a script
NAPI_EXTERN napi_status napi_run_script(napi_env env, napi_value script, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, script);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());

    auto scriptValue = LocalValueFromJsValue(script);
    RETURN_STATUS_IF_FALSE(env, scriptValue->IsString(), napi_status::napi_string_expected);

    std::vector<uint8_t> scriptContent;
    std::string path = panda::JSNApi::GetAssetPath(vm);
    std::string ami;
    if (!engine->CallGetAssetFunc(path, scriptContent, ami)) {
        HILOG_ERROR("Get asset error");
        *result = nullptr;
    }
    HILOG_INFO("asset size is %{public}zu", scriptContent.size());

    return napi_run_actor(env, scriptContent, ami.c_str(), result);
}

// Runnint a buffer script, only used in ark
NAPI_EXTERN napi_status napi_run_buffer_script(napi_env env, std::vector<uint8_t>& buffer, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());
    [[maybe_unused]] bool ret = panda::JSNApi::Execute(vm, buffer.data(), buffer.size(), PANDA_MAIN_FUNCTION);
    if (panda::JSNApi::HasPendingException(vm)) {
        if (engine->GetNapiUncaughtExceptionCallback() != nullptr) {
            LocalScope scope(vm);
            Local<ObjectRef> exception = panda::JSNApi::GetAndClearUncaughtException(vm);
            auto value = JsValueFromLocalValue(exception);
            if (!exception.IsEmpty() && !exception->IsHole()) {
                engine->GetNapiUncaughtExceptionCallback()(value);
            }
        }
        *result = nullptr;
    }

    Local<PrimitiveRef> value = panda::JSValueRef::Undefined(vm);
    *result = JsValueFromLocalValue(value);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_run_actor(napi_env env, std::vector<uint8_t>& buffer,
                                       const char* descriptor, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());

    std::string desc(descriptor);
    [[maybe_unused]] bool ret = false;

    if (panda::JSNApi::IsBundle(vm) || !buffer.empty()) {
        ret = panda::JSNApi::Execute(vm, buffer.data(), buffer.size(), PANDA_MAIN_FUNCTION, desc);
    } else {
        ret = panda::JSNApi::Execute(vm, desc, PANDA_MAIN_FUNCTION);
    }
    if (panda::JSNApi::HasPendingException(vm)) {
        if (engine->GetNapiUncaughtExceptionCallback() != nullptr) {
            LocalScope scope(vm);
            Local<ObjectRef> exception = panda::JSNApi::GetAndClearUncaughtException(vm);
            auto value = JsValueFromLocalValue(exception);
            if (!exception.IsEmpty() && !exception->IsHole()) {
                engine->GetNapiUncaughtExceptionCallback()(value);
            }
        }
        *result = nullptr;
    }

    Local<PrimitiveRef> value = panda::JSValueRef::Undefined(vm);
    *result = JsValueFromLocalValue(value);
    return napi_clear_last_error(env);
}

// Memory management
NAPI_INNER_EXTERN napi_status napi_adjust_external_memory(
    napi_env env, int64_t change_in_bytes, int64_t* adjusted_value)
{
    CHECK_ENV(env);
    CHECK_ARG(env, adjusted_value);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    engine->AdjustExternalMemory(change_in_bytes, adjusted_value);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_callable(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsFunction();

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_arguments_object(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsArgumentsObject();

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_async_function(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsAsyncFunction();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_boolean_object(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
	RETURN_STATUS_IF_FALSE(env, nativeValue->IsBoolean(), napi_boolean_expected);
    *result = nativeValue->IsJSPrimitiveBoolean();

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_generator_function(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsGeneratorFunction();

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_map_iterator(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsMapIterator();

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_set_iterator(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsSetIterator();

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_generator_object(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsGeneratorObject();

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_module_namespace_object(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsModuleNamespaceObject();

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_proxy(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsProxy();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_reg_exp(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsRegExp();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_number_object(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsJSPrimitiveNumber();

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_map(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsMap();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_set(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);

    *result = nativeValue->IsSet();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_string_object(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsJSPrimitiveString();

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_symbol_object(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsJSPrimitiveSymbol();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_weak_map(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsWeakMap();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_weak_set(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsWeakSet();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_runtime(napi_env env, napi_env* result_env)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result_env);

    auto engine = reinterpret_cast<NativeEngine*>(env);

    auto result = engine->CreateRuntime();
    *result_env = reinterpret_cast<napi_env>(result);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_serialize(napi_env env, napi_value object, napi_value transfer_list, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, transfer_list);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);
    auto transferList = LocalValueFromJsValue(transfer_list);
    void* res = panda::JSNApi::SerializeValue(vm, nativeValue, transferList);
    *result = reinterpret_cast<napi_value>(res);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_deserialize(napi_env env, napi_value recorder, napi_value* object)
{
    CHECK_ENV(env);
    CHECK_ARG(env, recorder);
    CHECK_ARG(env, object);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto recorderValue = reinterpret_cast<void*>(recorder);
    Local<panda::JSValueRef> res = panda::JSNApi::DeserializeValue(vm, recorderValue, reinterpret_cast<void*>(engine));

    *object = JsValueFromLocalValue(res);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_delete_serialization_data(napi_env env, napi_value value)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);

    void* data = reinterpret_cast<void*>(value);
    panda::JSNApi::DeleteSerializationData(data);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_bigint_int64(napi_env env, int64_t value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<panda::BigIntRef> object = panda::BigIntRef::New(vm, value);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_bigint_uint64(napi_env env, uint64_t value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<panda::BigIntRef> object = panda::BigIntRef::New(vm, value);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_bigint_int64(
    napi_env env, napi_value value, int64_t* result, bool* lossless)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);
    CHECK_ARG(env, lossless);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(value);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsBigInt(), napi_bigint_expected);
    Local<panda::BigIntRef> bigIntVal = nativeValue->ToBigInt(vm);
    bigIntVal->BigIntToInt64(vm, result, lossless);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_bigint_uint64(
    napi_env env, napi_value value, uint64_t* result, bool* lossless)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);
    CHECK_ARG(env, lossless);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(value);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsBigInt(), napi_bigint_expected);
    Local<panda::BigIntRef> bigIntVal = nativeValue->ToBigInt(vm);
    bigIntVal->BigIntToUint64(vm, result, lossless);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_date(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsDate();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_detached_arraybuffer(napi_env env, napi_value arraybuffer, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, arraybuffer);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(arraybuffer);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject(), napi_object_expected);
    auto ArrayBuffer_result = nativeValue->IsArrayBuffer();
    Local<panda::ArrayBufferRef> bufObj = nativeValue->ToObject(vm);
    if (ArrayBuffer_result) {

        *result = bufObj->IsDetach();
    } else {
        return napi_set_last_error(env, napi_invalid_arg);
    }
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_all_property_names(
    napi_env env, napi_value object, napi_key_collection_mode key_mode,
    napi_key_filter key_filter, napi_key_conversion key_conversion, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(), napi_object_expected);
    Local<panda::ObjectRef> obj = nativeValue->ToObject(vm);
    uint32_t filter = NATIVE_DEFAULT;
    if (key_filter & napi_key_writable) {
        filter = static_cast<uint32_t>(filter | NATIVE_WRITABLE);
    }
    if (key_filter & napi_key_enumerable) {
        filter = static_cast<uint32_t>(filter | NATIVE_ENUMERABLE);
    }
    if (key_filter & napi_key_configurable) {
        filter = static_cast<uint32_t>(filter | NATIVE_CONFIGURABLE);
    }
    if (key_filter & napi_key_skip_strings) {
        filter = static_cast<uint32_t>(filter | NATIVE_KEY_SKIP_STRINGS);
    }
    if (key_filter & napi_key_skip_symbols) {
        filter = static_cast<uint32_t>(filter | NATIVE_KEY_SKIP_SYMBOLS);
    }

    switch (key_mode) {
        case napi_key_include_prototypes:
            filter = static_cast<uint32_t>(filter | NATIVE_KEY_INCLUDE_PROTOTYPES);
            break;
        case napi_key_own_only:
            filter = static_cast<uint32_t>(filter | NATIVE_KEY_OWN_ONLY);
            break;
        default:
            *result = nullptr;
            return napi_set_last_error(env, napi_invalid_arg);
    }

    switch (key_conversion) {
        case napi_key_keep_numbers:
            filter = static_cast<uint32_t>(filter | NATIVE_KEY_KEEP_NUMBERS);
            break;
        case napi_key_numbers_to_strings:
            filter = static_cast<uint32_t>(filter | NATIVE_KEY_NUMBERS_TO_STRINGS);
            break;
        default:
            *result = nullptr;
            return napi_set_last_error(env, napi_invalid_arg);
    }
    Local<panda::ArrayRef> arrayVal = obj->GetAllPropertyNames(vm, filter);
    *result = JsValueFromLocalValue(arrayVal);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_detach_arraybuffer(napi_env env, napi_value arraybuffer)
{
    CHECK_ENV(env);
    CHECK_ARG(env, arraybuffer);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(arraybuffer);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject(), napi_object_expected);
    auto ArrayBuffer_result = nativeValue->IsArrayBuffer();
    Local<panda::ArrayBufferRef> bufObj = nativeValue->ToObject(vm);
    if (ArrayBuffer_result) {

        if (!bufObj->IsDetach()) {
            bufObj->Detach(vm);
        }
        auto ret = true;
        if (!ret) {
            return napi_set_last_error(env, napi_detachable_arraybuffer_expected);
        }
    } else {
        return napi_set_last_error(env, napi_invalid_arg);
    }
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_type_tag_object(napi_env env, napi_value js_object, const napi_type_tag* type_tag)
{
    CHECK_ENV(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, type_tag);
    bool result = true;

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(js_object);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject(), napi_object_expected);
    auto obj = nativeValue->ToObject(vm);
    NapiTypeTag* typeTag = (NapiTypeTag*)type_tag;
    const char name[] = "ACENAPI_TYPETAG";
    bool hasPribate = false;

    Local<panda::StringRef> key = StringRef::NewFromUtf8(vm, name);
    hasPribate = obj->Has(vm, key);
    if (!hasPribate) {
        constexpr int bigintMod = 2; // 2 : used for even number judgment
        int sign_bit = 0;
        size_t word_count = 2;
        bool sign = false;
        if ((sign_bit % bigintMod) == 1) {
            sign = true;
        }
        uint32_t size = (uint32_t)word_count;
        Local<panda::JSValueRef> value = panda::BigIntRef::CreateBigWords(vm, sign, size, reinterpret_cast<const uint64_t*>(typeTag));
        Local<panda::StringRef> key = panda::StringRef::NewFromUtf8(vm, name);
        result = obj->Set(vm, key, value);
    }
    if (!result) {
        return napi_set_last_error(env, napi_invalid_arg);
    }

    return napi_clear_last_error(env);
}

bool BigIntGetWordsArray(Local<panda::BigIntRef> &value, int* signBit, size_t* wordCount, uint64_t* words)
{
    if (wordCount == nullptr) {
    return false;
    }
    size_t size = static_cast<size_t>(value->GetWordsArraySize());
    if (signBit == nullptr && words == nullptr) {
        *wordCount = size;
        return true;
    } else if (signBit != nullptr && words != nullptr) {
        if (size > *wordCount) {
            size = *wordCount;
        }
        bool sign = false;
        value->GetWordsArray(&sign, size, words);
        if (sign) {
            *signBit = 1;
        } else {
            *signBit = 0;
        }
        *wordCount = size;
        return true;
    }
    return false;
}

NAPI_EXTERN napi_status napi_check_object_type_tag(
    napi_env env, napi_value js_object, const napi_type_tag* type_tag, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, type_tag);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(js_object);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject(), napi_object_expected);
    auto obj = nativeValue->ToObject(vm);
    NapiTypeTag* typeTag = (NapiTypeTag*)type_tag;
    *result = false;
    const char name[] = "ACENAPI_TYPETAG";

    Local<panda::StringRef> key = panda::StringRef::NewFromUtf8(vm, name);
    *result = obj->Has(vm, key);
    if (*result) {
        Local<panda::StringRef> key = panda::StringRef::NewFromUtf8(vm, name);
        Local<panda::JSValueRef> object = obj->Get(vm, key);
        if (object->IsBigInt()) {
            int sign;
            size_t size = 2; // 2: Indicates that the number of elements is 2
            NapiTypeTag tag;
            Local<panda::BigIntRef> bigintObj = object->ToBigInt(vm);
            BigIntGetWordsArray(bigintObj, &sign, &size, reinterpret_cast<uint64_t*>(&tag));
            if (sign == 0 && ((size == 1) || (size == 2))) { // 2: Indicates that the number of elements is 2
                *result = (tag.lower == typeTag->lower && tag.upper == typeTag->upper);
            }
        }
    }
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_date(napi_env env, double time, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    *result = JsValueFromLocalValue(DateRef::New(vm, time));

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_date_value(napi_env env, napi_value value, double* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();

    auto nativeValue = LocalValueFromJsValue(value);
    auto IsDate_result = nativeValue->IsDate();
    Local<panda::DateRef> dateObj = nativeValue->ToObject(vm);
    if (IsDate_result) {
        *result = dateObj->GetTime();
    } else {
        return napi_set_last_error(env, napi_date_expected);
    }

    return napi_clear_last_error(env);
}

NAPI_INNER_EXTERN napi_status napi_add_finalizer(napi_env env, napi_value js_object, void* native_object,
    napi_finalize finalize_cb, void* finalize_hint, napi_ref* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, finalize_cb);

    auto nativeValue = LocalValueFromJsValue(js_object);
    auto callback = reinterpret_cast<NativeFinalize>(finalize_cb);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject(), napi_object_expected);
    NativeReference* reference = nullptr;
    if (result != nullptr) {
        auto engine = reinterpret_cast<NativeEngine*>(env);
        reference = engine->CreateReference(JsValueFromLocalValue(nativeValue), 1, false, callback, native_object, finalize_hint);
        *result = reinterpret_cast<napi_ref>(reference);
    } else {
        auto engine = reinterpret_cast<NativeEngine*>(env);
        reference = engine->CreateReference(JsValueFromLocalValue(nativeValue), 0, true, callback, native_object, finalize_hint);
    }
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_bigint_words(
    napi_env env, int sign_bit, size_t word_count, const uint64_t* words, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, words);
    CHECK_ARG(env, result);
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    RETURN_STATUS_IF_FALSE(env, word_count <= INT_MAX, napi_invalid_arg);
    constexpr int bigintMod = 2; // 2 : used for even number judgment
    bool sign = false;
    if ((sign_bit % bigintMod) == 1) {
        sign = true;
    }
    uint32_t size = (uint32_t)word_count;
    Local<panda::JSValueRef> value = panda::BigIntRef::CreateBigWords(vm, sign, size, words);

    if (panda::JSNApi::HasPendingException(vm)) {
        return napi_set_last_error(env, napi_pending_exception);
    }
    *result = JsValueFromLocalValue(value);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_bigint_words(
    napi_env env, napi_value value, int* sign_bit, size_t* word_count, uint64_t* words)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, word_count);

    auto nativeValue = LocalValueFromJsValue(value);
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsBigInt(), napi_object_expected);
    auto BigintObj = nativeValue->ToBigInt(vm);
    if (word_count == nullptr) {
        return napi_set_last_error(env, napi_invalid_arg);
    }
    size_t size = static_cast<size_t>(BigintObj->GetWordsArraySize());
    if (sign_bit == nullptr && words == nullptr) {
        *word_count = size;
        return napi_set_last_error(env, napi_ok);
    } else if (sign_bit != nullptr && words != nullptr) {
        if (size > *word_count) {
            size = *word_count;
        }
        bool sign = false;
        BigintObj->GetWordsArray(&sign, size, words);
        if (sign) {
            *sign_bit = 1;
        } else {
            *sign_bit = 0;
        }
        *word_count = size;
        return napi_set_last_error(env, napi_ok);
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_run_script_path(napi_env env, const char* path, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);
    auto engine = reinterpret_cast<NativeEngine*>(env);
    *result = engine->RunScript(path);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_big_int64_array(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsBigInt64Array();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_big_uint64_array(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsBigUint64Array();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_shared_array_buffer(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsSharedArrayBuffer();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_stack_trace(napi_env env, std::string& stack)
{
    CHECK_ENV(env);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    std::string rawStack;
    bool getStackSuccess = engine->BuildJsStackTrace(rawStack);
    if (!getStackSuccess) {
        HILOG_ERROR("GetStacktrace env get stack failed");
    }
    stack = engine->ExecuteTranslateBySourceMap(rawStack);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_object_get_keys(napi_env env, napi_value data, napi_value* result)
{
    CHECK_ENV(env);
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(data);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject(), napi_object_expected);
    auto obj = nativeValue->ToObject(vm);
    Local<panda::ArrayRef> arrayVal = obj->GetOwnEnumerablePropertyNames(vm);

    *result = JsValueFromLocalValue(arrayVal);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_queue_async_work_with_qos(napi_env env, napi_async_work work, napi_qos_t qos)
{
    CHECK_ENV(env);
    CHECK_ARG(env, work);
    auto asyncWork = reinterpret_cast<NativeAsyncWork*>(work);
    asyncWork->QueueWithQos(qos);
    return napi_status::napi_ok;
}

void* DetachFuncCallback(void* engine, void* object, void* hint, void* detachData)
{
    if (detachData == nullptr || (engine == nullptr || object ==nullptr)) {
        HILOG_ERROR("DetachFuncCallback params has nullptr");
        return nullptr;
    }
    DetachCallback detach = reinterpret_cast<DetachCallback>(detachData);
    void* detachVal = detach(reinterpret_cast<NativeEngine*>(engine), object, hint);
    return detachVal;
}

Local<panda::JSValueRef> AttachFuncCallback(void* engine, void* buffer, void* hint, void* attachData)
{
    panda::EscapeLocalScope scope(reinterpret_cast<NativeEngine*>(engine)->GetEcmaVm());
    if (attachData == nullptr || (engine == nullptr || buffer ==nullptr)) {
        HILOG_ERROR("AttachFuncCallback params has nullptr");
    }
    NapiAttachCallback attach = reinterpret_cast<NapiAttachCallback>(attachData);
    napi_value attachVal = attach(reinterpret_cast<napi_env>(engine), buffer, hint);
    Local<panda::JSValueRef> result = LocalValueFromJsValue(attachVal);
    return scope.Escape(result);
}

NAPI_EXTERN napi_status napi_coerce_to_native_binding_object(napi_env env,
                                                             napi_value native_object,
                                                             NapiDetachCallback detach,
                                                             NapiAttachCallback attach,
                                                             void* object,
                                                             void* hint)
{
    CHECK_ENV(env);
    CHECK_ARG(env, native_object);
    CHECK_ARG(env, detach);
    CHECK_ARG(env, attach);
    CHECK_ARG(env, object);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(native_object);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject(), napi_object_expected);
    auto obj = nativeValue->ToObject(vm);
    bool res = obj->Set(vm, reinterpret_cast<void*>(DetachFuncCallback), reinterpret_cast<void*>(AttachFuncCallback));
    obj->SetNativePointerFieldCount(5); // 5 : NativeEngine, NativeObject, hint, detachData, attachData
    obj->SetNativePointerField(0, reinterpret_cast<void*>(engine), nullptr, nullptr);
    obj->SetNativePointerField(1, object, nullptr, nullptr);
    obj->SetNativePointerField(2, hint, nullptr, nullptr); // 2 : hint
    obj->SetNativePointerField(3, reinterpret_cast<void*>(detach), nullptr, nullptr); // 3 : detachData
    obj->SetNativePointerField(4, reinterpret_cast<void*>(attach), nullptr, nullptr); // 4 : attachData
    if (res) {
        return napi_clear_last_error(env);
    }
    return napi_status::napi_generic_failure;
}

NAPI_EXTERN napi_status napi_get_print_string(napi_env env,
                                              napi_value value,
                                              std::string& result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(value);
    if (nativeValue->IsString()) {
        Local<panda::StringRef> stringVal = nativeValue->ToString(vm);
        result = stringVal->ToString();
    }
    return napi_clear_last_error(env);
}