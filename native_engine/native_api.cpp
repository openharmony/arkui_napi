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

using panda::ArrayBufferRef;
using panda::ArrayRef;
using panda::BigIntRef;
using panda::BooleanRef;
using panda::BufferRef;
using panda::DataViewRef;
using panda::DateRef;
using panda::EscapeLocalScope;
using panda::FunctionRef;
using panda::Global;
using panda::IntegerRef;
using panda::JsiRuntimeCallInfo;
using panda::JSNApi;
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

static constexpr size_t MAX_BYTE_LENGTH = 2097152;
static constexpr size_t ONEMIB_BYTE_SIZE = 1048576;
static constexpr size_t SMALL_STRING_SIZE = 16;
static constexpr char NAME_SPACE_TAG = '@';

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

inline napi_handle_scope HandleScopeToNapiHandleScope(HandleScopeWrapper* s)
{
    return reinterpret_cast<napi_handle_scope>(s);
}

inline HandleScopeWrapper* NapiHandleScopeToHandleScope(napi_handle_scope s)
{
    return reinterpret_cast<HandleScopeWrapper*>(s);
}

inline napi_escapable_handle_scope EscapableHandleScopeToNapiEscapableHandleScope(EscapableHandleScopeWrapper* s)
{
    return reinterpret_cast<napi_escapable_handle_scope>(s);
}

inline EscapableHandleScopeWrapper* NapiEscapableHandleScopeToEscapableHandleScope(napi_escapable_handle_scope s)
{
    return reinterpret_cast<EscapableHandleScopeWrapper*>(s);
}

NAPI_EXTERN napi_status napi_get_last_error_info(napi_env env, const napi_extended_error_info** result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    *result = reinterpret_cast<napi_extended_error_info*>(reinterpret_cast<NativeEngine*>(env)->GetLastError());
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

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    Local<panda::PrimitiveRef> value = panda::JSValueRef::Undefined(vm);
    *result = JsValueFromLocalValue(value);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_null(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    Local<panda::PrimitiveRef> value = panda::JSValueRef::Null(vm);
    *result = JsValueFromLocalValue(value);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_global(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    const EcmaVM* vm = engine->GetEcmaVm();
    if (engine->IsMultiContextEnabled()) {
        Local<JSValueRef> context = engine->GetContext();
        Local<panda::ObjectRef> value = panda::JSNApi::GetGlobalObject(vm, context);
        *result = JsValueFromLocalValue(value);
        return napi_clear_last_error(env);
    }
    Local<panda::ObjectRef> value = panda::JSNApi::GetGlobalObject(vm);
    *result = JsValueFromLocalValue(value);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_boolean(napi_env env, bool value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    if (value) {
        *result = JsValueFromLocalValue(panda::JSValueRef::True(vm));
    } else {
        *result = JsValueFromLocalValue(panda::JSValueRef::False(vm));
    }

    return napi_clear_last_error(env);
}

// Methods to create Primitive types/Objects
NAPI_EXTERN napi_status napi_create_object(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    auto obj = panda::ObjectRef::NewObject(vm);
    *result = reinterpret_cast<napi_value>(obj);
    return napi_clear_last_error(env);
}

// Create JSObject with initial properties given by descriptors, note that property key must be String, and
// must can not convert to element_index, also all keys must not duplicate.
NAPI_EXTERN napi_status napi_create_object_with_properties(napi_env env,
                                                           napi_value* result,
                                                           size_t property_count,
                                                           const napi_property_descriptor* properties)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);
    SWITCH_CONTEXT(env);

    Local<panda::ObjectRef> object;
    if (property_count <= panda::ObjectRef::MAX_PROPERTIES_ON_STACK) {
        char attrs[sizeof(PropertyAttribute) * panda::ObjectRef::MAX_PROPERTIES_ON_STACK];
        char keys[sizeof(Local<panda::JSValueRef>) * panda::ObjectRef::MAX_PROPERTIES_ON_STACK];
        object = NapiCreateObjectWithProperties(env, property_count, properties,
                                                reinterpret_cast<Local<panda::JSValueRef> *>(keys),
                                                reinterpret_cast<PropertyAttribute *>(attrs));
    } else {
        void *attrs = malloc(sizeof(PropertyAttribute) * property_count);
        void *keys = malloc(sizeof(Local<panda::JSValueRef>) * property_count);
        if (attrs != nullptr && keys != nullptr) {
            object = NapiCreateObjectWithProperties(env, property_count, properties,
                                                    reinterpret_cast<Local<panda::JSValueRef> *>(keys),
                                                    reinterpret_cast<PropertyAttribute *>(attrs));
        } else {
            auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
            object = panda::JSValueRef::Undefined(vm);
            napi_throw_error(env, nullptr, "malloc failed in napi_create_object_with_properties");
        }
        if (attrs != nullptr) {
            free(attrs);
        }
        if (keys != nullptr) {
            free(keys);
        }
    }
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

// Create JSObject with initial properties given by keys and values, note that property key must be String, and
// must can not convert to element_index, also all keys must not duplicate.
NAPI_EXTERN napi_status napi_create_object_with_named_properties(napi_env env,
                                                                 napi_value* result,
                                                                 size_t property_count,
                                                                 const char** keys,
                                                                 const napi_value* values)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    Local<panda::ObjectRef> object = panda::ObjectRef::NewWithNamedProperties(vm, property_count, keys,
        reinterpret_cast<const Local<JSValueRef> *>(values));
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_array(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    Local<panda::ArrayRef> object = panda::ArrayRef::New(vm, 0);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_array_with_length(napi_env env, size_t length, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    Local<panda::ArrayRef> object = panda::ArrayRef::New(vm, length);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_sendable_array(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);
    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (!engine->IsMainEnvContext()) {
        HILOG_ERROR("multi-context does not support sendable feature");
        return napi_set_last_error(env, napi_invalid_arg);
    }

    Local<panda::SendableArrayRef> object = panda::SendableArrayRef::New(engine->GetEcmaVm(), 0);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_sendable_array_with_length(napi_env env, size_t length, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (!engine->IsMainEnvContext()) {
        HILOG_ERROR("multi-context does not support sendable feature");
        return napi_set_last_error(env, napi_invalid_arg);
    }

    Local<panda::SendableArrayRef> object = panda::SendableArrayRef::New(engine->GetEcmaVm(), length);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_double(napi_env env, double value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    Local<panda::NumberRef> object = panda::NumberRef::New(vm, value);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_int32(napi_env env, int32_t value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    Local<panda::NumberRef> object = panda::NumberRef::New(vm, value);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_uint32(napi_env env, uint32_t value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    Local<panda::NumberRef> object = panda::NumberRef::New(vm, value);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_int64(napi_env env, int64_t value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    Local<panda::NumberRef> object = panda::NumberRef::New(vm, value);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_string_latin1(napi_env env, const char* str, size_t length, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, str);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    if (length < SMALL_STRING_SIZE) {
        Local<panda::StringRef> object = panda::StringRef::NewFromUtf8WithoutStringTable(
            vm, str, (length == NAPI_AUTO_LENGTH) ? strlen(str) : length);
        *result = JsValueFromLocalValue(object);
    } else {
        Local<panda::StringRef> object = panda::StringRef::NewFromUtf8(
            vm, str, (length == NAPI_AUTO_LENGTH) ? strlen(str) : length);
        *result = JsValueFromLocalValue(object);
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_string_utf8(napi_env env, const char* str, size_t length, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, str);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    if (length < SMALL_STRING_SIZE) {
        Local<panda::StringRef> object = panda::StringRef::NewFromUtf8WithoutStringTable(
            vm, str, (length == NAPI_AUTO_LENGTH) ? strlen(str) : length);
        *result = JsValueFromLocalValue(object);
    } else {
        Local<panda::StringRef> object = panda::StringRef::NewFromUtf8(
            vm, str, (length == NAPI_AUTO_LENGTH) ? strlen(str) : length);
        *result = JsValueFromLocalValue(object);
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_string_utf16(
    napi_env env, const char16_t* str, size_t length, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, str);
    CHECK_ARG(env, result);
    RETURN_STATUS_IF_FALSE(env, (length == NAPI_AUTO_LENGTH) || (length <= INT_MAX && length >= 0), napi_invalid_arg);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    int char16Length = static_cast<int>(std::char_traits<char16_t>::length(str));
    if (length != NAPI_AUTO_LENGTH && length != static_cast<size_t>(char16Length)) {
        HILOG_WARN("`length` (%{public}zu) not equals to strlen(`str`) (%{public}d), result may be unexpected",
            length, char16Length);
    }
    if (length < SMALL_STRING_SIZE) {
        Local<panda::StringRef> object = panda::StringRef::NewFromUtf16WithoutStringTable(
            vm, str, (length == NAPI_AUTO_LENGTH) ? char16Length : length);
        *result = JsValueFromLocalValue(object);
    } else {
        Local<panda::StringRef> object = panda::StringRef::NewFromUtf16(
            vm, str, (length == NAPI_AUTO_LENGTH) ? char16Length : length);
        *result = JsValueFromLocalValue(object);
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_symbol(napi_env env, napi_value description, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    panda::Local<panda::JSValueRef> object = panda::JSValueRef::Undefined(vm);
    if (description == nullptr) {
        const char* str = "";
        object = panda::StringRef::NewFromUtf8(vm, str, 0);
    } else {
        object = LocalValueFromJsValue(description);
    }
    RETURN_STATUS_IF_FALSE(env, object->IsString(vm), napi_invalid_arg);
    Local<panda::SymbolRef> symbol = panda::SymbolRef::New(vm, object);
    *result = JsValueFromLocalValue(symbol);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_function(napi_env env,
                                             const char* utf8name,
                                             size_t length,
                                             napi_callback cb,
                                             void* data,
                                             napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, cb);
    CHECK_ARG(env, result);

    SWITCH_CONTEXT(env);
    auto vm = const_cast<EcmaVM*>(reinterpret_cast<NativeEngine*>(env)->GetEcmaVm());
    panda::JsiFastNativeScope fastNativeScope(vm);
    EscapeLocalScope scope(vm);
    auto callback = reinterpret_cast<NapiNativeCallback>(cb);
    const char* name = "defaultName";
    NapiFunctionInfo* funcInfo = NapiFunctionInfo::CreateNewInstance();
    if (funcInfo == nullptr) {
        HILOG_ERROR("funcInfo is nullptr");
        return napi_set_last_error(env, napi_invalid_arg);
    }
    funcInfo->callback = callback;
    funcInfo->data = data;
    funcInfo->env = env;
#ifdef ENABLE_CONTAINER_SCOPE
    if (EnableContainerScope(env)) {
        funcInfo->scopeId = OHOS::Ace::ContainerScope::CurrentId();
    }
#endif

    Local<JSValueRef> context = engine->GetContext();
    Local<panda::FunctionRef> fn = panda::FunctionRef::NewConcurrent(vm, context, ArkNativeFunctionCallBack,
        CommonDeleter, reinterpret_cast<void*>(funcInfo), true);
    Local<panda::StringRef> fnName = panda::StringRef::NewFromUtf8(vm, utf8name != nullptr ? utf8name : name);
    fn->SetName(vm, fnName);
    *result = JsValueFromLocalValue(scope.Escape(fn));
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_create_error(napi_env env, napi_value code, napi_value msg, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, msg);
    CHECK_ARG(env, result);

    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    Local<panda::JSValueRef> codeValue = panda::JSValueRef::Undefined(vm);
    if (code != nullptr) {
        codeValue = LocalValueFromJsValue(code);
        RETURN_STATUS_IF_FALSE(env, codeValue->IsString(vm) || codeValue->IsNumber(), napi_invalid_arg);
    }

    auto msgValue = LocalValueFromJsValue(msg);
    RETURN_STATUS_IF_FALSE(env, msgValue->IsString(vm), napi_invalid_arg);

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

    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    Local<panda::JSValueRef> codeValue = panda::JSValueRef::Undefined(vm);
    if (code != nullptr) {
        codeValue = LocalValueFromJsValue(code);
        RETURN_STATUS_IF_FALSE(env, codeValue->IsString(vm) || codeValue->IsNumber(), napi_invalid_arg);
    }
    auto msgValue = LocalValueFromJsValue(msg);
    RETURN_STATUS_IF_FALSE(env, msgValue->IsString(vm), napi_invalid_arg);

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

    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    Local<panda::JSValueRef> codeValue = panda::JSValueRef::Undefined(vm);

    if (code != nullptr) {
        codeValue = LocalValueFromJsValue(code);
        RETURN_STATUS_IF_FALSE(env, codeValue->IsString(vm) || codeValue->IsNumber(), napi_invalid_arg);
    }
    auto msgValue = LocalValueFromJsValue(msg);
    RETURN_STATUS_IF_FALSE(env, msgValue->IsString(vm), napi_invalid_arg);

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
    napi_valuetype resultType;
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    if (valueObj->IsNumber()) {
        resultType = napi_number;
    } else if (valueObj->IsString(vm)) {
        resultType = napi_string;
    } else if (valueObj->IsFunction(vm)) {
        resultType = napi_function;
    } else if (valueObj->IsNativePointer(vm)) {
        resultType = napi_external;
    } else if (valueObj->IsNull()) {
        resultType = napi_null;
    } else if (valueObj->IsBoolean()) {
        resultType = napi_boolean;
    } else if (valueObj->IsUndefined()) {
        resultType = napi_undefined;
    } else if (valueObj->IsSymbol(vm)) {
        resultType = napi_symbol;
    } else if (valueObj->IsBigInt(vm)) {
        resultType = napi_bigint;
    } else if (valueObj->IsObject(vm)) {
        resultType = napi_object;
    } else {
        resultType = napi_undefined;
    }
    *result = resultType;
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_double(napi_env env, napi_value value, double* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    panda::JSValueRef* nativeValue = reinterpret_cast<panda::JSValueRef*>(value);
    bool isNumber = false;
    double dValue = nativeValue->GetValueDouble(isNumber);
    RETURN_STATUS_IF_FALSE(env, isNumber, napi_number_expected);
    *result = dValue;
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_int32(napi_env env, napi_value value, int32_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    panda::JSValueRef* nativeValue = reinterpret_cast<panda::JSValueRef*>(value);
    bool isNumber = false;
    int32_t i32Value = nativeValue->GetValueInt32(isNumber);
    RETURN_STATUS_IF_FALSE(env, isNumber, napi_number_expected);
    *result = i32Value;

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_uint32(napi_env env, napi_value value, uint32_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    panda::JSValueRef* nativeValue = reinterpret_cast<panda::JSValueRef*>(value);
    bool isNumber = false;
    uint32_t u32Value = nativeValue->GetValueUint32(isNumber);
    RETURN_STATUS_IF_FALSE(env, isNumber, napi_number_expected);
    *result = u32Value;
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_int64(napi_env env, napi_value value, int64_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    panda::JSValueRef* nativeValue = reinterpret_cast<panda::JSValueRef*>(value);
    bool isNumber = false;
    int64_t i64Value = nativeValue->GetValueInt64(isNumber);
    RETURN_STATUS_IF_FALSE(env, isNumber, napi_number_expected);
    *result = i64Value;
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_bool(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    panda::JSValueRef* nativeValue = reinterpret_cast<panda::JSValueRef*>(value);
    bool isBool = false;
    bool bValue = nativeValue->GetValueBool(isBool);
    RETURN_STATUS_IF_FALSE(env, isBool, napi_boolean_expected);
    *result = bValue;
    return napi_clear_last_error(env);
}

// Copies LATIN-1 encoded bytes from a string into a buffer.
NAPI_EXTERN napi_status napi_get_value_string_latin1(napi_env env,
                                                     napi_value value,
                                                     char* buf,
                                                     size_t bufsize,
                                                     size_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    RETURN_STATUS_IF_FALSE(env, nativeValue->IsString(vm), napi_string_expected);
    Local<panda::StringRef> stringVal(nativeValue);
    if (buf == nullptr) {
        CHECK_ARG(env, result);
        *result = stringVal->Length(vm);
    } else if (bufsize != 0) {
        uint32_t copied = stringVal->WriteLatin1(vm, buf, bufsize);
        buf[copied] = '\0';
        if (result != nullptr) {
            *result = copied;
        }
    } else if (result != nullptr) {
        *result = 0;
    }

    return napi_clear_last_error(env);
}

// Copies UTF-8 encoded bytes from a string into a buffer.
NAPI_EXTERN napi_status napi_get_value_string_utf8(napi_env env,
                                                   napi_value value,
                                                   char* buf,
                                                   size_t bufsize,
                                                   size_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    RETURN_STATUS_IF_FALSE(env, nativeValue->IsString(vm), napi_string_expected);
    Local<panda::StringRef> stringVal(nativeValue);
    if (buf == nullptr) {
        CHECK_ARG(env, result);
        *result = stringVal->Utf8Length(vm, true) - 1;
    } else if (bufsize != 0) {
        uint32_t copied = stringVal->WriteUtf8(vm, buf, bufsize - 1, true) - 1;
        buf[copied] = '\0';
        if (result != nullptr) {
            *result = copied;
        }
    } else if (result != nullptr) {
        *result = 0;
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_string_utf16(napi_env env,
                                                    napi_value value,
                                                    char16_t* buf,
                                                    size_t bufsize,
                                                    size_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    RETURN_STATUS_IF_FALSE(env, nativeValue->IsString(vm), napi_string_expected);
    Local<panda::StringRef> stringVal(nativeValue);
    if (buf == nullptr) {
        CHECK_ARG(env, result);
        *result = stringVal->Length(vm);
    } else if (bufsize == 1) {
        buf[0] = '\0';
        if (result != nullptr) {
            *result = 0;
        }
    } else if (bufsize != 0) {
        uint32_t copied = stringVal->WriteUtf16(vm, buf, bufsize - 1); // bufsize - 1 : reserve the position of buf "\0"
        buf[copied] = '\0';
        if (result != nullptr) {
            *result = copied;
        }
    } else if (result != nullptr) {
        *result = 0;
    }

    return napi_clear_last_error(env);
}

// Methods to coerce values
// These APIs may execute user scripts
NAPI_EXTERN napi_status napi_coerce_to_bool(napi_env env, napi_value value, napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    Local<panda::JSValueRef> val = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    Local<panda::BooleanRef> boolVal = val->ToBoolean(vm);
    *result = JsValueFromLocalValue(boolVal);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_coerce_to_number(napi_env env, napi_value value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    *result = JsValueFromLocalValue(nativeValue->ToNumber(vm));

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_coerce_to_object(napi_env env, napi_value value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    *result = JsValueFromLocalValue(nativeValue->ToObject(vm));

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_coerce_to_string(napi_env env, napi_value value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    *result = JsValueFromLocalValue(nativeValue->ToString(vm));

    return napi_clear_last_error(env);
}

// Methods to work with Objects
NAPI_EXTERN napi_status napi_get_prototype(napi_env env, napi_value object, napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(object);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, obj);
    Local<panda::JSValueRef> val = obj->GetPrototype(vm);
    *result = JsValueFromLocalValue(val);

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_get_property_names(napi_env env, napi_value object, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(object);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, obj);
    Local<panda::ArrayRef> arrayVal = obj->GetOwnPropertyNames(vm);
    *result = JsValueFromLocalValue(arrayVal);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_set_property(napi_env env, napi_value object, napi_value key, napi_value value)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, key);
    CHECK_ARG(env, value);

    auto nativeValue = LocalValueFromJsValue(object);
    auto propKey = LocalValueFromJsValue(key);
    auto propValue = LocalValueFromJsValue(value);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, obj);
    obj->Set(vm, propKey, propValue);

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_has_property(napi_env env, napi_value object, napi_value key, bool* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, key);
    CHECK_ARG(env, result);
    
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    Local<panda::JSValueRef> hasResult = JSNApi::NapiHasProperty(vm, reinterpret_cast<uintptr_t>(object),
                                                                 reinterpret_cast<uintptr_t>(key));
    RETURN_STATUS_IF_FALSE(env, NapiStatusValidationCheck(hasResult), napi_object_expected);
    *result = hasResult->BooleaValue(vm);

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_get_property(napi_env env, napi_value object, napi_value key, napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, key);
    CHECK_ARG(env, result);

    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    Local<panda::JSValueRef> value = JSNApi::NapiGetProperty(vm, reinterpret_cast<uintptr_t>(object),
                                                             reinterpret_cast<uintptr_t>(key));
    RETURN_STATUS_IF_FALSE(env, NapiStatusValidationCheck(value), napi_object_expected);
#ifdef ENABLE_CONTAINER_SCOPE
    FunctionSetContainerId(env, value);
#endif
    *result = JsValueFromLocalValue(value);

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_delete_property(napi_env env, napi_value object, napi_value key, bool* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, key);

    auto nativeValue = LocalValueFromJsValue(object);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject(vm) || nativeValue->IsFunction(vm), napi_object_expected);
    auto deleteResult = JSNApi::NapiDeleteProperty(vm, reinterpret_cast<uintptr_t>(object),
                                                   reinterpret_cast<uintptr_t>(key));
    RETURN_STATUS_IF_FALSE(env, NapiStatusValidationCheck(deleteResult), napi_object_expected);
    if (result) {
        *result = deleteResult->BooleaValue(vm);
    }

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_has_own_property(napi_env env, napi_value object, napi_value key, bool* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, key);
    CHECK_ARG(env, result);

    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    auto hasResult = JSNApi::NapiHasOwnProperty(vm, reinterpret_cast<uintptr_t>(object),
                                                reinterpret_cast<uintptr_t>(key));
    RETURN_STATUS_IF_FALSE(env, NapiStatusValidationCheck(hasResult), napi_object_expected);
    if (result) {
        *result = hasResult->BooleaValue(vm);
    }

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_set_named_property(napi_env env, napi_value object, const char* utf8name, napi_value value)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, utf8name);
    CHECK_ARG(env, value);

    auto nativeValue = LocalValueFromJsValue(object);
    auto propVal = LocalValueFromJsValue(value);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject(vm) || nativeValue->IsFunction(vm), napi_object_expected);
    Local<panda::ObjectRef> obj(nativeValue);
    obj->Set(vm, utf8name, propVal);

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_has_named_property(napi_env env, napi_value object, const char* utf8name, bool* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, utf8name);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(object);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, obj);
    Local<panda::StringRef> key = panda::StringRef::NewFromUtf8(vm, utf8name);
    *result = obj->Has(vm, key);

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_get_named_property(napi_env env,
                                                napi_value object,
                                                const char* utf8name,
                                                napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, utf8name);
    CHECK_ARG(env, result);

    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    Local<panda::JSValueRef> value = JSNApi::NapiGetNamedProperty(vm, reinterpret_cast<uintptr_t>(object), utf8name);
    RETURN_STATUS_IF_FALSE(env, NapiStatusValidationCheck(value), napi_object_expected);
#ifdef ENABLE_CONTAINER_SCOPE
    FunctionSetContainerId(env, value);
#endif
    *result = JsValueFromLocalValue(value);

    return GET_RETURN_STATUS(env);
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

    auto nativeValue = LocalValueFromJsValue(object);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, obj);
    Local<panda::StringRef> key = panda::StringRef::NewFromUtf8(vm, utf8name);
    panda::PropertyAttribute property;
    obj->GetOwnProperty(vm, key, property);
    *result = JsValueFromLocalValue(property.GetValue(vm));
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_set_element(napi_env env, napi_value object, uint32_t index, napi_value value)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, value);

    auto nativeValue = LocalValueFromJsValue(object);
    auto elementValue = LocalValueFromJsValue(value);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, obj);
    obj->Set(vm, index, elementValue);

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_has_element(napi_env env, napi_value object, uint32_t index, bool* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(object);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, obj);
    *result = obj->Has(vm, index);

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_get_element(napi_env env, napi_value object, uint32_t index, napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(object);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, obj);
    Local<panda::JSValueRef> value = obj->Get(vm, index);
#ifdef ENABLE_CONTAINER_SCOPE
    FunctionSetContainerId(env, value);
#endif
    *result = JsValueFromLocalValue(value);

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_delete_element(napi_env env, napi_value object, uint32_t index, bool* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, object);

    auto nativeValue = LocalValueFromJsValue(object);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, obj);
    bool deleteResult = obj->Delete(vm, index);
    if (result) {
        *result = deleteResult;
    }

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_define_properties(napi_env env,
                                               napi_value object,
                                               size_t property_count,
                                               const napi_property_descriptor* properties)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, properties);

    auto nativeValue = LocalValueFromJsValue(object);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, nativeObject);

    auto nativeProperties = reinterpret_cast<const NapiPropertyDescriptor*>(properties);
    for (size_t i = 0; i < property_count; i++) {
        if (nativeProperties[i].utf8name == nullptr) {
            auto name = LocalValueFromJsValue(nativeProperties[i].name);
            RETURN_STATUS_IF_FALSE(env, !name.IsEmpty() && (name->IsString(vm) || name->IsSymbol(vm)),
                napi_name_expected);
        }
        NapiDefineProperty(env, nativeObject, nativeProperties[i]);
    }
    return GET_RETURN_STATUS(env);
}

// Methods to work with Arrays
NAPI_EXTERN napi_status napi_is_array(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    auto nativeValue = LocalValueFromJsValue(value);
    *result = nativeValue->IsJSArray(vm) || nativeValue->IsSharedArray(vm);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_array_length(napi_env env, napi_value value, uint32_t* result)
{
    // Omit to use NAPI_PREAMBLE and CHECK_ARG directly for performance.
    CHECK_ENV(env);
    RETURN_STATUS_IF_FALSE(env, (reinterpret_cast<NativeEngine*>(env))->lastException_.IsEmpty(),
        napi_pending_exception);
    napi_clear_last_error((env));

    // This interface will not throw JS exceptions actively.
    // JS exception can only occur before calling the interface.
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();

    // check value and result
    if ((value == nullptr) || (result == nullptr)) {
        if (panda::JSNApi::HasPendingException(vm)) {
            reinterpret_cast<NativeEngine *>(env)->lastException_ = panda::JSNApi::GetUncaughtException(vm);
        }
        return napi_set_last_error(env, napi_invalid_arg);
    }

    // get array length
    bool isArrayOrSharedArray = false;
    bool isPendingException = false;
    panda::Local<panda::JSValueRef> nativeValue = LocalValueFromJsValue(value);
    nativeValue->TryGetArrayLength(vm, &isPendingException, &isArrayOrSharedArray, result);

    if (!isArrayOrSharedArray) {
        if (isPendingException) {
            reinterpret_cast<NativeEngine *>(env)->lastException_ = panda::JSNApi::GetUncaughtException(vm);
        }
        return napi_set_last_error(env, napi_array_expected);
    }

    if (UNLIKELY(isPendingException)) {
        HILOG_ERROR("pending exception before napi_get_array_length called, print exception info: ");
        panda::JSNApi::PrintExceptionInfo(vm);
        reinterpret_cast<NativeEngine *>(env)->lastException_ = panda::JSNApi::GetUncaughtException(vm);
        return napi_set_last_error(env, napi_pending_exception);
    }
    return napi_ok;
}

NAPI_EXTERN napi_status napi_is_sendable(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (!engine->IsMainEnvContext()) {
        HILOG_ERROR("multi-context does not support sendable feature");
        return napi_set_last_error(env, napi_invalid_arg);
    }

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = engine->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsJSShared(vm) || nativeValue->IsString(vm) || nativeValue->IsNumber() ||
              nativeValue->IsBoolean() || nativeValue->IsUndefined() || nativeValue->IsNull() ||
              nativeValue->IsBigInt(vm);
    return napi_clear_last_error(env);
}

// Methods to compare values
NAPI_EXTERN napi_status napi_strict_equals(napi_env env, napi_value lhs, napi_value rhs, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, lhs);
    CHECK_ARG(env, rhs);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
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
    CHECK_ENV((env));
    RETURN_STATUS_IF_FALSE((env), (reinterpret_cast<NativeEngine*>(env))->lastException_.IsEmpty(),
        napi_pending_exception);
    napi_clear_last_error((env));
    CHECK_ARG(env, func);
    if (argc > 0) {
        CHECK_ARG(env, argv);
    }

    auto vm = reinterpret_cast<NativeEngine *>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    RETURN_STATUS_IF_FALSE(env, reinterpret_cast<panda::JSValueRef *>(func)->IsFunction(vm), napi_function_expected);
    panda::JSValueRef* thisObj = reinterpret_cast<panda::JSValueRef *>(recv);
    panda::FunctionRef* function = reinterpret_cast<panda::FunctionRef *>(func);
#ifdef ENABLE_CONTAINER_SCOPE
    int32_t scopeId = -1;
    bool enableContainerScope = EnableContainerScope(env);
    if (enableContainerScope) {
        scopeId = OHOS::Ace::ContainerScope::CurrentId();
        if (!function->IsConcurrentFunction(vm)) {
            auto funcInfo = reinterpret_cast<NapiFunctionInfo*>(function->GetData(vm));
            if (funcInfo != nullptr) {
                scopeId = funcInfo->scopeId;
            }
        }
    }
    OHOS::Ace::ContainerScope containerScope(scopeId, enableContainerScope);
#endif
    panda::JSValueRef* value =
        function->CallForNapi(vm, thisObj, reinterpret_cast<panda::JSValueRef *const*>(argv), argc);
    // if pending exception, value will be a pointer to JSTaggedValue::Hole.
    if (UNLIKELY(!NapiStatusValidationCheck(value))) {
        HILOG_ERROR("pending exception when js function called, print exception info: ");
        panda::JSNApi::PrintExceptionInfo(vm);
        result = nullptr;
        reinterpret_cast<NativeEngine *>(env)->lastException_ = panda::JSNApi::GetUncaughtException(vm);
        return napi_set_last_error(env, napi_pending_exception);
    }
    if (result) {
        *result = reinterpret_cast<napi_value>(value);
    }
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_new_instance(napi_env env,
                                          napi_value constructor,
                                          size_t argc,
                                          const napi_value* argv,
                                          napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, constructor);
    if (argc > 0) {
        CHECK_ARG(env, argv);
    }
    CHECK_ARG(env, result);

    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    RETURN_STATUS_IF_FALSE(env, reinterpret_cast<panda::JSValueRef*>(constructor)->IsFunction(vm),
        napi_function_expected);
    panda::FunctionRef* constructorVal = reinterpret_cast<panda::FunctionRef*>(constructor);
    panda::JSValueRef* instance = constructorVal->ConstructorOptimize(vm,
        reinterpret_cast<panda::JSValueRef**>(const_cast<napi_value*>(argv)), argc);
    if (tryCatch.HasCaught()) {
        HILOG_ERROR("CreateInstance occur Exception");
        *result = nullptr;
    } else {
        *result = reinterpret_cast<napi_value>(instance);
    }
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_instanceof(napi_env env, napi_value object, napi_value constructor, bool* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, constructor);
    CHECK_ARG(env, result);

    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    auto nativeValue = LocalValueFromJsValue(object);
    auto nativeConstructor = LocalValueFromJsValue(constructor);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject(vm), napi_object_expected);
    RETURN_STATUS_IF_FALSE(env, nativeConstructor->IsFunction(vm), napi_function_expected);
    *result = nativeValue->InstanceOf(vm, nativeConstructor);

    return GET_RETURN_STATUS(env);
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

    auto info = reinterpret_cast<panda::JsiRuntimeCallInfo*>(cbinfo);
    if ((argc != nullptr) && (argv != nullptr)) {
#ifdef ENABLE_CONTAINER_SCOPE
        auto *vm = info->GetVM();
#endif
        size_t i = 0;
        if (*argc > 0) {
            size_t j = static_cast<size_t>(info->GetArgsNumber());
            for (; i < j && i < *argc; i++) {
                panda::Local<panda::JSValueRef> value = info->GetCallArgRef(i);
#ifdef ENABLE_CONTAINER_SCOPE
                FunctionSetContainerId(env, value);
#endif
                argv[i] = JsValueFromLocalValue(value);
            }
        } else {
            i = static_cast<size_t>(info->GetArgsNumber());
        }
        if (i < *argc) {
            napi_value undefined = JsValueFromLocalValue(
                panda::JSValueRef::Undefined(reinterpret_cast<NativeEngine*>(env)->GetEcmaVm()));
            for (; i < *argc; i++) {
                argv[i] = undefined;
            }
        }
    }
    if (argc != nullptr) {
        *argc = static_cast<size_t>(info->GetArgsNumber());
    }
    if (this_arg != nullptr) {
        *this_arg = JsValueFromLocalValue(info->GetThisRef());
    }
    if (data != nullptr) {
        auto funcInfo = static_cast<NapiFunctionInfo*>(info->GetData());
        if (funcInfo != nullptr) {
            *data = funcInfo->data;
        }
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_new_target(napi_env env, napi_callback_info cbinfo, napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, cbinfo);
    CHECK_ARG(env, result);

    auto info = reinterpret_cast<panda::JsiRuntimeCallInfo*>(cbinfo);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    auto thisVarObj = info->GetThisRef();
#ifdef ENABLE_CONTAINER_SCOPE
    panda::Local<panda::JSValueRef> newValue = info->GetNewTargetRef();
    FunctionSetContainerId(env, newValue);
    auto functionVal = newValue;
#else
    auto functionVal = info->GetNewTargetRef();
#endif
    if (thisVarObj->InstanceOf(vm, functionVal)) {
        *result = JsValueFromLocalValue(functionVal);
    } else {
        *result = nullptr;
    }

    return GET_RETURN_STATUS(env);
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
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, utf8name);
    RETURN_STATUS_IF_FALSE(env, length == NAPI_AUTO_LENGTH || length <= INT_MAX, napi_object_expected);
    CHECK_ARG(env, constructor);
    if (property_count > 0) {
        CHECK_ARG(env, properties);
    }
    CHECK_ARG(env, result);

    SWITCH_CONTEXT(env);
    auto callback = reinterpret_cast<NapiNativeCallback>(constructor);
    auto nativeProperties = reinterpret_cast<const NapiPropertyDescriptor*>(properties);

    size_t nameLength = std::min(length, strlen(utf8name));
    char newName[nameLength + 1];
    if (strncpy_s(newName, nameLength + 1, utf8name, nameLength) != EOK) {
        HILOG_ERROR("napi_define_class strncpy_s failed");
        *result = nullptr;
    } else {
        auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
        panda::JsiFastNativeScope fastNativeScope(vm);
        EscapeLocalScope scope(vm);
        auto resultValue = NapiDefineClass(env, newName, callback, data, nativeProperties, property_count);
        *result = JsValueFromLocalValue(scope.Escape(resultValue));
    }

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_define_sendable_class(napi_env env,
                                                   const char* utf8name,
                                                   size_t length,
                                                   napi_callback constructor,
                                                   void* data,
                                                   size_t property_count,
                                                   const napi_property_descriptor* properties,
                                                   napi_value parent,
                                                   napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, utf8name);
    RETURN_STATUS_IF_FALSE(env, length == NAPI_AUTO_LENGTH || length <= INT_MAX,
                           napi_object_expected);
    CHECK_ARG(env, constructor);
    if (property_count > 0) {
        CHECK_ARG(env, properties);
    }
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (!engine->IsMainEnvContext()) {
        HILOG_ERROR("multi-context does not support sendable feature");
        return napi_set_last_error(env, napi_invalid_arg);
    }

    auto callback = reinterpret_cast<NapiNativeCallback>(constructor);
    auto nativeProperties = reinterpret_cast<const NapiPropertyDescriptor*>(properties);

    size_t nameLength = std::min(length, strlen(utf8name));
    char newName[nameLength + 1];
    if (strncpy_s(newName, nameLength + 1, utf8name, nameLength) != EOK) {
        HILOG_ERROR("napi_define_sendable_class strncpy_s failed");
        *result = nullptr;
    } else {
        EscapeLocalScope scope(engine->GetEcmaVm());
        auto resultValue =
            NapiDefineSendableClass(env, newName, callback, data, nativeProperties, property_count, parent);
        *result = JsValueFromLocalValue(scope.Escape(resultValue));
    }

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_create_sendable_object_with_properties(napi_env env,
                                                                    size_t property_count,
                                                                    const napi_property_descriptor* properties,
                                                                    napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (!engine->IsMainEnvContext()) {
        HILOG_ERROR("multi-context does not support sendable feature");
        return napi_set_last_error(env, napi_invalid_arg);
    }

    auto nativeProperties = reinterpret_cast<const NapiPropertyDescriptor*>(properties);
    auto object = NapiCreateSObjectWithProperties(env, property_count, nativeProperties);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_map(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    Local<panda::ArrayRef> object = panda::MapRef::New(vm);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_sendable_map(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (!engine->IsMainEnvContext()) {
        HILOG_ERROR("multi-context does not support sendable feature");
        return napi_set_last_error(env, napi_invalid_arg);
    }

    Local<panda::ArrayRef> object = panda::SendableMapRef::New(engine->GetEcmaVm());
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_map_set_property(napi_env env, napi_value map, napi_value key, napi_value value)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, map);
    CHECK_ARG(env, key);
    CHECK_ARG(env, value);

    auto nativeValue = LocalValueFromJsValue(map);
    auto propKey = LocalValueFromJsValue(key);
    auto propValue = LocalValueFromJsValue(value);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsMap(vm) || nativeValue->IsSharedMap(vm), napi_object_expected);
    if (LIKELY(nativeValue->IsMap(vm))) {
        Local<panda::MapRef> mapRef(nativeValue);
        mapRef->Set(vm, propKey, propValue);
    } else {
        Local<panda::SendableMapRef> mapRef(nativeValue);
        mapRef->Set(vm, propKey, propValue);
    }

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_map_set_named_property(napi_env env,
                                                    napi_value map,
                                                    const char* utf8name,
                                                    napi_value value)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, map);
    CHECK_ARG(env, utf8name);
    CHECK_ARG(env, value);

    auto nativeValue = LocalValueFromJsValue(map);
    auto propVal = LocalValueFromJsValue(value);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsMap(vm) || nativeValue->IsSharedMap(vm), napi_object_expected);
    Local<panda::MapRef> mapRef(nativeValue);
    if (LIKELY(nativeValue->IsMap(vm))) {
        Local<panda::MapRef> mapRef(nativeValue);
        mapRef->Set(vm, utf8name, propVal);
    } else {
        Local<panda::SendableMapRef> mapRef(nativeValue);
        mapRef->Set(vm, utf8name, propVal);
    }

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_map_get_property(napi_env env, napi_value map, napi_value key, napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, map);
    CHECK_ARG(env, key);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(map);
    auto propKey = LocalValueFromJsValue(key);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsMap(vm) || nativeValue->IsSharedMap(vm), napi_object_expected);
    Local<JSValueRef> value;
    if (LIKELY(nativeValue->IsMap(vm))) {
        Local<panda::MapRef> mapRef(nativeValue);
        value = mapRef->Get(vm, propKey);
    } else {
        Local<panda::SendableMapRef> mapRef(nativeValue);
        value = mapRef->Get(vm, propKey);
    }
    RETURN_STATUS_IF_FALSE(env, NapiStatusValidationCheck(value), napi_object_expected);
    *result = JsValueFromLocalValue(value);

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_map_get_named_property(napi_env env,
                                                    napi_value map,
                                                    const char* utf8name,
                                                    napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, map);
    CHECK_ARG(env, utf8name);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(map);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsMap(vm) || nativeValue->IsSharedMap(vm), napi_object_expected);
    Local<JSValueRef> value;
    if (LIKELY(nativeValue->IsMap(vm))) {
        Local<panda::MapRef> mapRef(nativeValue);
        value = mapRef->Get(vm, utf8name);
    } else {
        Local<panda::SendableMapRef> mapRef(nativeValue);
        value = mapRef->Get(vm, utf8name);
    }
    RETURN_STATUS_IF_FALSE(env, NapiStatusValidationCheck(value), napi_object_expected);
    *result = JsValueFromLocalValue(value);

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_map_has_property(napi_env env, napi_value map, napi_value key, bool* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, map);
    CHECK_ARG(env, key);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(map);
    auto propKey = LocalValueFromJsValue(key);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsMap(vm) || nativeValue->IsSharedMap(vm), napi_object_expected);
    bool value;
    if (LIKELY(nativeValue->IsMap(vm))) {
        Local<panda::MapRef> mapRef(nativeValue);
        value = mapRef->Has(vm, propKey);
    } else {
        Local<panda::SendableMapRef> mapRef(nativeValue);
        value = mapRef->Has(vm, propKey);
    }
    *result = value;

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_map_has_named_property(napi_env env, napi_value map, const char* utf8name, bool* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, map);
    CHECK_ARG(env, utf8name);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(map);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsMap(vm) || nativeValue->IsSharedMap(vm), napi_object_expected);
    bool value;
    if (LIKELY(nativeValue->IsMap(vm))) {
        Local<panda::MapRef> mapRef(nativeValue);
        value = mapRef->Has(vm, utf8name);
    } else {
        Local<panda::SendableMapRef> mapRef(nativeValue);
        value = mapRef->Has(vm, utf8name);
    }
    *result = value;

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_map_delete_property(napi_env env, napi_value map, napi_value key)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, map);
    CHECK_ARG(env, key);

    auto nativeValue = LocalValueFromJsValue(map);
    auto propKey = LocalValueFromJsValue(key);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsMap(vm) || nativeValue->IsSharedMap(vm), napi_object_expected);
    if (LIKELY(nativeValue->IsMap(vm))) {
        Local<panda::MapRef> mapRef(nativeValue);
        mapRef->Delete(vm, propKey);
    } else {
        Local<panda::SendableMapRef> mapRef(nativeValue);
        mapRef->Delete(vm, propKey);
    }

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_map_clear(napi_env env, napi_value map)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, map);

    auto nativeValue = LocalValueFromJsValue(map);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsMap(vm) || nativeValue->IsSharedMap(vm), napi_object_expected);
    if (LIKELY(nativeValue->IsMap(vm))) {
        Local<panda::MapRef> mapRef(nativeValue);
        mapRef->Clear(vm);
    } else {
        Local<panda::SendableMapRef> mapRef(nativeValue);
        mapRef->Clear(vm);
    }

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_map_get_size(napi_env env, napi_value map, uint32_t* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, map);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(map);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsMap(vm) || nativeValue->IsSharedMap(vm), napi_object_expected);
    uint32_t value;
    if (LIKELY(nativeValue->IsMap(vm))) {
        Local<panda::MapRef> mapRef(nativeValue);
        value = static_cast<uint32_t>(mapRef->GetSize(vm));
    } else {
        Local<panda::SendableMapRef> mapRef(nativeValue);
        value = static_cast<uint32_t>(mapRef->GetSize(vm));
    }
    *result = value;

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_map_get_entries(napi_env env, napi_value map, napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, map);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(map);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsMap(vm) || nativeValue->IsSharedMap(vm), napi_object_expected);
    if (LIKELY(nativeValue->IsMap(vm))) {
        Local<panda::MapRef> mapRef(nativeValue);
        Local<panda::ArrayRef> arrayVal = mapRef->GetEntries(vm);
        *result = JsValueFromLocalValue(arrayVal);
    } else {
        Local<panda::SendableMapRef> mapRef(nativeValue);
        Local<panda::SendableArrayRef> arrayVal = mapRef->GetEntries(vm);
        *result = JsValueFromLocalValue(arrayVal);
    }
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_map_get_keys(napi_env env, napi_value map, napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, map);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(map);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsMap(vm) || nativeValue->IsSharedMap(vm), napi_object_expected);
    if (LIKELY(nativeValue->IsMap(vm))) {
        Local<panda::MapRef> mapRef(nativeValue);
        Local<panda::ArrayRef> arrayVal = mapRef->GetKeys(vm);
        *result = JsValueFromLocalValue(arrayVal);
    } else {
        Local<panda::SendableMapRef> mapRef(nativeValue);
        Local<panda::SendableArrayRef> arrayVal = mapRef->GetKeys(vm);
        *result = JsValueFromLocalValue(arrayVal);
    }
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_map_get_values(napi_env env, napi_value map, napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, map);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(map);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsMap(vm) || nativeValue->IsSharedMap(vm), napi_object_expected);
    if (LIKELY(nativeValue->IsMap(vm))) {
        Local<panda::MapRef> mapRef(nativeValue);
        Local<panda::ArrayRef> arrayVal = mapRef->GetValues(vm);
        *result = JsValueFromLocalValue(arrayVal);
    } else {
        Local<panda::SendableMapRef> mapRef(nativeValue);
        Local<panda::SendableArrayRef> arrayVal = mapRef->GetValues(vm);
        *result = JsValueFromLocalValue(arrayVal);
    }
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_map_iterator_get_next(napi_env env, napi_value iterator, napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, iterator);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(iterator);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsMapIterator(vm) || nativeValue->IsSharedMapIterator(vm),
                           napi_object_expected);
    Local<panda::JSValueRef> value;
    if (LIKELY(nativeValue->IsMapIterator(vm))) {
        Local<panda::MapIteratorRef> mapIter(nativeValue);
        value = mapIter->Next(vm);
    } else {
        Local<panda::SendableMapIteratorRef> mapIter(nativeValue);
        value = mapIter->Next(vm);
    }
    *result = JsValueFromLocalValue(value);
    return GET_RETURN_STATUS(env);
}

// Methods to work with external data objects
NAPI_EXTERN napi_status napi_wrap(napi_env env,
                                  napi_value js_object,
                                  void* native_object,
                                  napi_finalize finalize_cb,
                                  void* finalize_hint,
                                  napi_ref* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, native_object);
    CHECK_ARG(env, finalize_cb);

    auto nativeValue = LocalValueFromJsValue(js_object);
    auto callback = reinterpret_cast<NapiNativeFinalize>(finalize_cb);
    SWITCH_CONTEXT(env);
    auto vm = engine->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, nativeObject);
    size_t nativeBindingSize = 0;
    auto reference = reinterpret_cast<NativeReference**>(result);
    Local<panda::StringRef> key = panda::StringRef::GetNapiWrapperString(vm);
    if (nativeObject->Has(vm, key)) {
        HILOG_WARN("napi_wrap: current js_object has been wrapped.");
    }
    Local<panda::ObjectRef> object = panda::ObjectRef::New(vm);
    NativeReference* ref = nullptr;
    if (reference != nullptr) {
        ref = engine->CreateReference(js_object, 1, false, callback, native_object, finalize_hint);
        *reference = ref;
    } else {
        ref = engine->CreateReference(js_object, 0, true, callback, native_object, finalize_hint);
    }
    object->SetNativePointerFieldCount(vm, 1);
    object->SetNativePointerField(vm, 0, ref, nullptr, nullptr, nativeBindingSize);
    PropertyAttribute attr(object, true, false, true);
    nativeObject->DefineProperty(vm, key, attr);
    return GET_RETURN_STATUS(env);
}

// Methods to work with external data objects, support pass native_object size and finalize_cb async execute or not
NAPI_EXTERN napi_status napi_wrap_enhance(napi_env env,
                                          napi_value js_object,
                                          void* native_object,
                                          napi_finalize finalize_cb,
                                          bool async_finalizer,
                                          void* finalize_hint,
                                          size_t native_binding_size,
                                          napi_ref* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, native_object);

    auto nativeValue = LocalValueFromJsValue(js_object);
    auto callback = reinterpret_cast<NapiNativeFinalize>(finalize_cb);
    SWITCH_CONTEXT(env);
    auto vm = engine->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, nativeObject);
    auto reference = reinterpret_cast<NativeReference**>(result);
    Local<panda::StringRef> key = panda::StringRef::GetNapiWrapperString(vm);
    Local<panda::ObjectRef> object = panda::ObjectRef::New(vm);
    NativeReference* ref = nullptr;
    if (!async_finalizer) {
        if (reference != nullptr) {
            ref = engine->CreateReference(js_object, 1, false, callback, native_object, finalize_hint,
                                          native_binding_size);
            *reference = ref;
        } else {
            ref = engine->CreateReference(js_object, 0, true, callback, native_object, finalize_hint,
                                          native_binding_size);
        }
    } else {
        if (reference != nullptr) {
            ref = engine->CreateAsyncReference(js_object, 1, false, callback, native_object, finalize_hint);
            *reference = ref;
        } else {
            ref = engine->CreateAsyncReference(js_object, 0, true, callback, native_object, finalize_hint);
        }
    }
    object->SetNativePointerFieldCount(vm, 1);
    object->SetNativePointerField(vm, 0, ref, nullptr, nullptr, native_binding_size);

    PropertyAttribute attr(object, true, false, true);
    nativeObject->DefineProperty(vm, key, attr);
    return GET_RETURN_STATUS(env);
}

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
    PropertyAttribute attr(object, true, false, true);
    nativeObject->DefineProperty(vm, key, attr);
    return GET_RETURN_STATUS(env);
}

// Ensure thread safety! Async finalizer will be called on the async thread.
NAPI_EXTERN napi_status napi_wrap_async_finalizer(napi_env env,
                                                  napi_value js_object,
                                                  void* native_object,
                                                  napi_finalize finalize_cb,
                                                  void* finalize_hint,
                                                  napi_ref* result,
                                                  size_t native_binding_size)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, native_object);

    auto nativeValue = LocalValueFromJsValue(js_object);
    auto callback = reinterpret_cast<NapiNativeFinalize>(finalize_cb);
    SWITCH_CONTEXT(env);
    auto vm = engine->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, nativeObject);
    auto reference = reinterpret_cast<NativeReference**>(result);
    Local<panda::StringRef> key = panda::StringRef::GetNapiWrapperString(vm);
    if (nativeObject->Has(vm, key)) {
        HILOG_WARN("napi_wrap_async_finalizer: current js_object has been wrapped.");
    }
    Local<panda::ObjectRef> object = panda::ObjectRef::New(vm);
    NativeReference* ref = nullptr;
    if (reference != nullptr) {
        ref = engine->CreateAsyncReference(js_object, 1, false, callback, native_object, finalize_hint);
        *reference = ref;
    } else {
        ref = engine->CreateAsyncReference(js_object, 0, true, callback, native_object, finalize_hint);
    }
    object->SetNativePointerFieldCount(vm, 1);
    object->SetNativePointerField(vm, 0, ref, nullptr, nullptr, native_binding_size);
    PropertyAttribute attr(object, true, false, true);
    nativeObject->DefineProperty(vm, key, attr);
    return GET_RETURN_STATUS(env);
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
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, native_object);

    auto nativeValue = LocalValueFromJsValue(js_object);
    auto callback = reinterpret_cast<NapiNativeFinalize>(finalize_cb);
    SWITCH_CONTEXT(env);
    auto vm = engine->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, nativeObject);
    auto reference = reinterpret_cast<NativeReference**>(result);
    Local<panda::StringRef> key = panda::StringRef::GetNapiWrapperString(vm);
    if (nativeObject->Has(vm, key)) {
        HILOG_WARN("napi_wrap_with_size: current js_object has been wrapped.");
    }
    Local<panda::ObjectRef> object = panda::ObjectRef::New(vm);
    NativeReference* ref = nullptr;
    if (reference != nullptr) {
        ref = engine->CreateReference(js_object, 1, false, callback, native_object, finalize_hint,
                                      native_binding_size);
        *reference = ref;
    } else {
        ref = engine->CreateReference(js_object, 0, true, callback, native_object, finalize_hint,
                                      native_binding_size);
    }
    object->SetNativePointerFieldCount(vm, 1);
    object->SetNativePointerField(vm, 0, ref, nullptr, nullptr, native_binding_size);
    PropertyAttribute attr(object, true, false, true);
    nativeObject->DefineProperty(vm, key, attr);

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_unwrap(napi_env env, napi_value js_object, void** result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(js_object);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, nativeObject);
    Local<panda::StringRef> key = panda::StringRef::GetNapiWrapperString(vm);
    Local<panda::JSValueRef> val = nativeObject->Get(vm, key);
    *result = nullptr;
    if (val->IsObject(vm)) {
        Local<panda::ObjectRef> ext(val);
        auto ref = reinterpret_cast<NativeReference*>(ext->GetNativePointerField(vm, 0));
        *result = ref != nullptr ? ref->GetData() : nullptr;
    }

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

NAPI_EXTERN napi_status napi_remove_wrap(napi_env env, napi_value js_object, void** result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(js_object);
    SWITCH_CONTEXT(env);
    auto vm = engine->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, nativeObject);
    Local<panda::StringRef> key = panda::StringRef::GetNapiWrapperString(vm);
    Local<panda::JSValueRef> val = nativeObject->Get(vm, key);
    *result = nullptr;
    if (val->IsObject(vm)) {
        Local<panda::ObjectRef> ext(val);
        auto ref = reinterpret_cast<NativeReference*>(ext->GetNativePointerField(vm, 0));
        *result = ref != nullptr ? ref->GetData() : nullptr;
    }

    size_t nativeBindingSize = 0;
    if (nativeObject->Has(vm, key)) {
        Local<panda::ObjectRef> wrapper = val;
        auto ref = reinterpret_cast<NativeReference*>(wrapper->GetNativePointerField(vm, 0));
        nativeObject->Delete(vm, key);
        delete ref;
    } else {
        Local<panda::ObjectRef> object = panda::ObjectRef::New(vm);
        NativeReference* ref = nullptr;
        ref = engine->CreateReference(js_object, 0, true, nullptr, nullptr, nullptr);
        object->SetNativePointerFieldCount(vm, 1);
        object->SetNativePointerField(vm, 0, ref, nullptr, nullptr, nativeBindingSize);
        PropertyAttribute attr(object, true, false, true);
        nativeObject->DefineProperty(vm, key, attr);
    }

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_wrap_sendable(napi_env env,
                                           napi_value js_object,
                                           void* native_object,
                                           napi_finalize finalize_cb,
                                           void* finalize_hint)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, native_object);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (!engine->IsMainEnvContext()) {
        HILOG_ERROR("multi-context does not support sendable feature");
        return napi_set_last_error(env, napi_invalid_arg);
    }

    auto nativeValue = LocalValueFromJsValue(js_object);
    auto callback = reinterpret_cast<panda::NativePointerCallback>(finalize_cb);
    auto vm = engine->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsSendableObject(vm), napi_object_expected);
    Local<ObjectRef> nativeObject(nativeValue);
    nativeObject->SetNativePointerFieldCount(vm, 1);
    nativeObject->SetNativePointerField(vm, 0, native_object, callback, finalize_hint);
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_wrap_sendable_with_size(napi_env env,
                                                     napi_value js_object,
                                                     void* native_object,
                                                     napi_finalize finalize_cb,
                                                     void* finalize_hint,
                                                     size_t native_binding_size)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, native_object);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (!engine->IsMainEnvContext()) {
        HILOG_ERROR("multi-context does not support sendable feature");
        return napi_set_last_error(env, napi_invalid_arg);
    }

    auto nativeValue = LocalValueFromJsValue(js_object);
    auto callback = reinterpret_cast<panda::NativePointerCallback>(finalize_cb);
    auto vm = engine->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsSendableObject(vm), napi_object_expected);
    Local<ObjectRef> nativeObject(nativeValue);
    nativeObject->SetNativePointerFieldCount(vm, 1);
    nativeObject->SetNativePointerField(vm, 0, native_object, callback, finalize_hint, native_binding_size);
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_unwrap_sendable(napi_env env, napi_value js_object, void** result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (!engine->IsMainEnvContext()) {
        HILOG_ERROR("multi-context does not support sendable feature");
        return napi_set_last_error(env, napi_invalid_arg);
    }

    auto nativeValue = LocalValueFromJsValue(js_object);
    auto vm = engine->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsSendableObject(vm), napi_object_expected);
    Local<ObjectRef> nativeObject(nativeValue);
    void* val = nativeObject->GetNativePointerField(vm, 0);
    *result = val;
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_remove_wrap_sendable(napi_env env, napi_value js_object, void** result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (!engine->IsMainEnvContext()) {
        HILOG_ERROR("multi-context does not support sendable feature");
        return napi_set_last_error(env, napi_invalid_arg);
    }

    auto nativeValue = LocalValueFromJsValue(js_object);
    auto vm = engine->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsSendableObject(vm), napi_object_expected);
    Local<ObjectRef> nativeObject(nativeValue);
    void* val = nativeObject->GetNativePointerField(vm, 0);
    *result = val;
    nativeObject->SetNativePointerField(vm, 0, nullptr, nullptr, nullptr);

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_create_external(
    napi_env env, void* data, napi_finalize finalize_cb, void* finalize_hint, napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    if (engine->IsMainEnvContext()) {
        auto callback = reinterpret_cast<panda::NativePointerCallback>(finalize_cb);
        Local<panda::NativePointerRef> object = panda::NativePointerRef::New(vm, data, callback, finalize_hint, 0);
        *result = JsValueFromLocalValue(object);
    } else {
        Local<panda::NativePointerRef> object = panda::NativePointerRef::New(vm, data, nullptr, nullptr, 0);
        *result = JsValueFromLocalValue(object);
        engine->CreateReference(*result, 0, true, finalize_cb, data, finalize_hint);
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_external_with_size(napi_env env,
                                                       void* data,
                                                       napi_finalize finalize_cb,
                                                       void* finalize_hint,
                                                       napi_value* result,
                                                       size_t native_binding_size)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<panda::NativePointerRef> object;
    if (engine->IsMainEnvContext()) {
        auto callback = reinterpret_cast<panda::NativePointerCallback>(finalize_cb);
        object = panda::NativePointerRef::New(vm, data, callback, finalize_hint, native_binding_size);
        *result = JsValueFromLocalValue(object);
    } else {
        object = panda::NativePointerRef::New(vm, data, nullptr, nullptr, native_binding_size);
        *result = JsValueFromLocalValue(object);
        engine->CreateReference(*result, 0, true, finalize_cb, data, finalize_hint, native_binding_size);
    }
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_external(napi_env env, napi_value value, void** result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    bool isNativePointer = false;
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    void* ret = nativeValue->GetNativePointerValue(vm, isNativePointer);
    RETURN_STATUS_IF_FALSE(env, isNativePointer, napi_object_expected);
    *result = ret;
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
    auto engine = reinterpret_cast<ArkNativeEngine*>(env);
    auto ref = new ArkNativeReference(engine, value, initial_refcount);

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
    NativeEngine* engine = reinterpret_cast<NativeEngine*>(env);

    *result = reference->Get(engine);
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
    delete NapiHandleScopeToHandleScope(scope);
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

    auto nativeValue = LocalValueFromJsValue(error);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    RETURN_STATUS_IF_FALSE(env, nativeValue->IsError(vm), napi_invalid_arg);
    panda::JSNApi::ThrowException(vm, nativeValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_throw_error(napi_env env, const char* code, const char* msg)
{
    CHECK_ENV(env);
    CHECK_ARG(env, msg);

    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
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

    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    Local<panda::JSValueRef> error(panda::JSValueRef::Undefined(vm));
    error = panda::Exception::TypeError(vm, StringRef::NewFromUtf8(vm, msg));
    if (code != nullptr) {
        Local<panda::JSValueRef> codeKey = panda::StringRef::NewFromUtf8(vm, "code");
        Local<panda::JSValueRef> codeValue = panda::StringRef::NewFromUtf8(vm, code);
        Local<panda::ObjectRef> errorObj(error);
        errorObj->Set(vm, codeKey, codeValue);
    }
    panda::JSNApi::ThrowException(vm, error);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_throw_range_error(napi_env env, const char* code, const char* msg)
{
    CHECK_ENV(env);
    CHECK_ARG(env, msg);

    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    Local<panda::JSValueRef> error(panda::JSValueRef::Undefined(vm));
    error = panda::Exception::RangeError(vm, StringRef::NewFromUtf8(vm, msg));
    if (code != nullptr) {
        Local<panda::JSValueRef> codeKey = panda::StringRef::NewFromUtf8(vm, "code");
        Local<panda::JSValueRef> codeValue = panda::StringRef::NewFromUtf8(vm, code);
        Local<panda::ObjectRef> errorObj(error);
        errorObj->Set(vm, codeKey, codeValue);
    }
    panda::JSNApi::ThrowException(vm, error);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_error(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    *result = nativeValue->IsError(vm);

    return napi_clear_last_error(env);
}

// Methods to support catching exceptions
NAPI_EXTERN napi_status napi_is_exception_pending(napi_env env, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    *result = panda::JSNApi::HasPendingException(vm);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_and_clear_last_exception(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    engine->lastException_.Empty();
    Local<panda::ObjectRef> exception = panda::JSNApi::GetAndClearUncaughtException(vm);
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
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsArrayBuffer(vm) || nativeValue->IsSendableArrayBuffer(vm);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_arraybuffer(napi_env env, size_t byte_length, void** data, napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, data);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    uint8_t** values = (uint8_t**)(data);
    Local<panda::ArrayBufferRef> res = panda::ArrayBufferRef::New(vm, byte_length);
    if (values != nullptr) {
        *values = reinterpret_cast<uint8_t*>(res->GetBuffer(vm));
        if (UNLIKELY(*values == nullptr && byte_length > 0)) {
            HILOG_WARN("Allocate ArrayBuffer failed, maybe size is too large, request size: %{public}zu", byte_length);
        }
    }
    *result = JsValueFromLocalValue(res);

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_create_sendable_arraybuffer(napi_env env, size_t byte_length,
                                                         void** data, napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, data);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (!engine->IsMainEnvContext()) {
        HILOG_ERROR("multi-context does not support sendable feature");
        return napi_set_last_error(env, napi_invalid_arg);
    }
    auto vm = engine->GetEcmaVm();
    uint8_t** values = (uint8_t**)(data);
    Local<panda::SendableArrayBufferRef> res = panda::SendableArrayBufferRef::New(vm, byte_length);
    if (values != nullptr) {
        *values = reinterpret_cast<uint8_t*>(res->GetBuffer(vm));
    }
    *result = JsValueFromLocalValue(res);

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_create_external_arraybuffer(napi_env env,
                                                         void* external_data,
                                                         size_t byte_length,
                                                         napi_finalize finalize_cb,
                                                         void* finalize_hint,
                                                         napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, external_data);
    CHECK_ARG(env, finalize_cb);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<panda::ArrayBufferRef> object;
    if (engine->IsMainEnvContext()) {
        auto callback = reinterpret_cast<panda::NativePointerCallback>(finalize_cb);
        object = panda::ArrayBufferRef::New(vm, external_data, byte_length, callback, finalize_hint);
        *result = JsValueFromLocalValue(object);
    } else {
        object = panda::ArrayBufferRef::New(vm, external_data, byte_length, nullptr, nullptr);
        *result = JsValueFromLocalValue(object);
        engine->CreateReference(*result, 0, true, finalize_cb, external_data, finalize_hint, 0);
    }
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_get_arraybuffer_info(napi_env env,
                                                  napi_value arraybuffer,
                                                  void** data,
                                                  size_t* byte_length)
{
    CHECK_ENV(env);
    CHECK_ARG(env, arraybuffer);
    CHECK_ARG(env, byte_length);

    auto nativeValue = LocalValueFromJsValue(arraybuffer);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    if (LIKELY(nativeValue->IsArrayBuffer(vm))) {
        Local<panda::ArrayBufferRef> res(nativeValue);
        if (data != nullptr) {
            *data = res->GetBuffer(vm);
        }
        *byte_length = res->ByteLength(vm);
    } else if (nativeValue->IsSendableArrayBuffer(vm)) {
        Local<panda::SendableArrayBufferRef> res(nativeValue);
        if (data != nullptr) {
            *data = res->GetBuffer(vm);
        }
        *byte_length = res->ByteLength(vm);
    } else {
        return napi_set_last_error(env, napi_arraybuffer_expected);
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_typedarray(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsTypedArray(vm) || nativeValue->IsSharedTypedArray(vm);

    return napi_clear_last_error(env);
}

EXTERN_C_START
NAPI_EXTERN napi_status napi_is_buffer(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsBuffer(vm);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_buffer(napi_env env, size_t size, void** data, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, data);
    CHECK_ARG(env, result);
    RETURN_STATUS_IF_FALSE(env, size > 0, napi_invalid_arg);

    uint8_t** value = reinterpret_cast<uint8_t**>(data);
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
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    Local<JSValueRef> context = engine->GetContext();
    Local<panda::BufferRef> obj = BufferRef::New(vm, context, size);
    *value = reinterpret_cast<uint8_t*>(obj->GetBuffer(vm));

    CHECK_ARG(env, *data);
    void* ptr = obj->GetBuffer(vm);
    CHECK_ARG(env, ptr);

    *result = JsValueFromLocalValue(obj);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_buffer_copy(napi_env env,
                                                size_t length,
                                                const void* data,
                                                void** result_data,
                                                napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, data);
    CHECK_ARG(env, result_data);
    CHECK_ARG(env, result);
    RETURN_STATUS_IF_FALSE(env, length > 0, napi_invalid_arg);

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
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    Local<JSValueRef> context = engine->GetContext();
    Local<panda::BufferRef> obj = BufferRef::New(vm, context, length);
    if (obj->IsUndefined()) {
        HILOG_INFO("engine create buffer_copy failed!");
    }
    *value = reinterpret_cast<uint8_t*>(obj->GetBuffer(vm));
    if (memcpy_s(*value, length, recvdata, length) != EOK) {
        HILOG_ERROR("memcpy_s failed");
    }

    void* ptr = obj->GetBuffer(vm);
    CHECK_ARG(env, ptr);

    *result = JsValueFromLocalValue(obj);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_external_buffer(napi_env env,
                                                    size_t length,
                                                    void* data,
                                                    napi_finalize finalize_cb,
                                                    void* finalize_hint,
                                                    napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, result);
    CHECK_ARG(env, data);
    RETURN_STATUS_IF_FALSE(env, length > 0, napi_invalid_arg);

    auto callback = reinterpret_cast<panda::NativePointerCallback>(finalize_cb);
    if (!data) {
        HILOG_ERROR("data is empty");
        return napi_set_last_error(env, napi_invalid_arg);
    }
    if (length > MAX_BYTE_LENGTH) {
        HILOG_ERROR("Creat failed, current size: %{public}2f MiB, limit size: %{public}2f MiB",
                    static_cast<float>(length) / static_cast<float>(ONEMIB_BYTE_SIZE),
                    static_cast<float>(MAX_BYTE_LENGTH) / static_cast<float>(ONEMIB_BYTE_SIZE));
        data = nullptr;
        return napi_set_last_error(env, napi_invalid_arg);
    }

    SWITCH_CONTEXT(env);
    auto vm = engine->GetEcmaVm();
    Local<panda::BufferRef> object;
    Local<JSValueRef> context = engine->GetContext();
    if (engine->IsMainEnvContext()) {
        object = panda::BufferRef::New(vm, context, data, length, callback, finalize_hint);
        void* ptr = object->GetBuffer(vm);
        CHECK_ARG(env, ptr);
        *result = JsValueFromLocalValue(object);
    } else {
        object = panda::BufferRef::New(vm, context, data, length, nullptr, nullptr);
        void* ptr = object->GetBuffer(vm);
        CHECK_ARG(env, ptr);
        *result = JsValueFromLocalValue(object);
        engine->CreateReference(*result, 0, true, finalize_cb, data, finalize_hint);
    }
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_get_buffer_info(napi_env env, napi_value value, void** data, size_t* length)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsBuffer(vm), napi_status::napi_arraybuffer_expected);
    Local<panda::BufferRef> res(nativeValue);
    *data = res->GetBuffer(vm);
    *length = res->ByteLength(vm);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_object_freeze(napi_env env, napi_value object)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, object);

    auto nativeValue = LocalValueFromJsValue(object);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject(vm), napi_object_expected);
    Local<panda::ObjectRef> obj = nativeValue->ToEcmaObject(vm);
    obj->Freeze(vm);

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_object_seal(napi_env env, napi_value object)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, object);

    auto nativeValue = LocalValueFromJsValue(object);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject(vm), napi_object_expected);
    Local<panda::ObjectRef> obj = nativeValue->ToEcmaObject(vm);
    obj->Seal(vm);

    return GET_RETURN_STATUS(env);
}

EXTERN_C_END

NAPI_EXTERN napi_status napi_create_typedarray(napi_env env,
                                               napi_typedarray_type type,
                                               size_t length,
                                               napi_value arraybuffer,
                                               size_t byte_offset,
                                               napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, arraybuffer);
    CHECK_ARG(env, result);

    auto value = LocalValueFromJsValue(arraybuffer);
    auto typedArrayType = (NativeTypedArrayType)type;
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, value->IsArrayBuffer(vm), napi_status::napi_arraybuffer_expected);
    Local<panda::ArrayBufferRef> arrayBuf(value);

    if (!reinterpret_cast<NativeEngine*>(env)->NapiNewTypedArray(typedArrayType, arrayBuf,
                                                                 byte_offset, length, result)) {
        HILOG_ERROR("%{public}s invalid arg", __func__);
        return napi_set_last_error(env, napi_invalid_arg);
    }
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_create_sendable_typedarray(napi_env env,
                                                        napi_typedarray_type type,
                                                        size_t length,
                                                        napi_value arraybuffer,
                                                        size_t byte_offset,
                                                        napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, arraybuffer);
    CHECK_ARG(env, result);

    auto value = LocalValueFromJsValue(arraybuffer);
    auto typedArrayType = (NativeTypedArrayType)type;
    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (!engine->IsMainEnvContext()) {
        HILOG_ERROR("multi-context does not support sendable feature");
        return napi_set_last_error(env, napi_invalid_arg);
    }

    auto vm = engine->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, value->IsSendableArrayBuffer(vm), napi_status::napi_arraybuffer_expected);
    Local<panda::SendableArrayBufferRef> arrayBuf(value);

    if (!reinterpret_cast<NativeEngine*>(env)->NapiNewSendableTypedArray(typedArrayType,
                                                                         arrayBuf, byte_offset,
                                                                         length, result)) {
        HILOG_ERROR("%{public}s invalid arg", __func__);
        return napi_set_last_error(env, napi_invalid_arg);
    }
    return GET_RETURN_STATUS(env);
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
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    if (LIKELY(value->IsTypedArray(vm))) {
        Local<panda::TypedArrayRef> typedArray = Local<panda::TypedArrayRef>(value);
        Local<ArrayBufferRef> localArrayBuffer = typedArray->GetArrayBuffer(vm);
        size_t byteOffset = typedArray->ByteOffset(vm);
        if (type != nullptr) {
            *type = static_cast<napi_typedarray_type>(engine->GetTypedArrayType(typedArray));
        }
        if (length != nullptr) {
            *length = typedArray->ByteLength(vm);
        }
        if (data != nullptr) {
            *data = static_cast<uint8_t*>(localArrayBuffer->GetBuffer(vm)) + byteOffset;
        }
        if (arraybuffer != nullptr) {
            *arraybuffer = JsValueFromLocalValue(localArrayBuffer);
        }
        if (byte_offset != nullptr) {
            *byte_offset = byteOffset;
        }
    } else if (value->IsSharedTypedArray(vm)) {
        Local<panda::SendableTypedArrayRef> typedArray = Local<panda::SendableTypedArrayRef>(value);
        Local<panda::SendableArrayBufferRef> localArrayBuffer = typedArray->GetArrayBuffer(vm);
        size_t byteOffset = typedArray->ByteOffset(vm);
        if (type != nullptr) {
            *type = static_cast<napi_typedarray_type>(engine->GetSendableTypedArrayType(typedArray));
        }
        if (length != nullptr) {
            *length = typedArray->ByteLength(vm);
        }
        if (data != nullptr) {
            *data = static_cast<uint8_t*>(localArrayBuffer->GetBuffer(vm)) + byteOffset;
        }
        if (arraybuffer != nullptr) {
            *arraybuffer = JsValueFromLocalValue(localArrayBuffer);
        }
        if (byte_offset != nullptr) {
            *byte_offset = byteOffset;
        }
    } else {
        HILOG_ERROR("%{public}s invalid arg", __func__);
        return napi_set_last_error(env, napi_invalid_arg);
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_dataview(napi_env env,
                                             size_t length,
                                             napi_value arraybuffer,
                                             size_t byte_offset,
                                             napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, arraybuffer);
    CHECK_ARG(env, result);

    auto arrayBufferValue = LocalValueFromJsValue(arraybuffer);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, arrayBufferValue->IsArrayBuffer(vm), napi_status::napi_arraybuffer_expected);
    Local<panda::ArrayBufferRef> res(arrayBufferValue);
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
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_is_dataview(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    *result = nativeValue->IsDataView(vm);

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

    auto nativeValue = LocalValueFromJsValue(dataview);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    bool isDataView = false;
    nativeValue->GetDataViewInfo(vm, isDataView, bytelength, data,
        reinterpret_cast<panda::JSValueRef**>(arraybuffer), byte_offset);
    RETURN_STATUS_IF_FALSE(env, isDataView, napi_status::napi_invalid_arg);

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
    NAPI_PREAMBLE(env);
    SWITCH_CONTEXT(env);
    if (panda::JSNApi::HasPendingException(engine->GetEcmaVm())) {
        return napi_pending_exception;
    }
    CHECK_ARG(env, deferred);
    CHECK_ARG(env, promise);

    NativeDeferred* nativeDeferred = nullptr;
    auto resultValue = engine->CreatePromise(&nativeDeferred);
    if (LocalValueFromJsValue(resultValue)->IsUndefined()) {
        return napi_set_last_error(env, napi_generic_failure);
    }
    *deferred = reinterpret_cast<napi_deferred>(nativeDeferred);
    *promise = resultValue;

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_resolve_deferred(napi_env env, napi_deferred deferred, napi_value resolution)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, deferred);
    CHECK_ARG(env, resolution);

    auto nativeDeferred = reinterpret_cast<NativeDeferred*>(deferred);
    nativeDeferred->Resolve(resolution);
    delete nativeDeferred;
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_reject_deferred(napi_env env, napi_deferred deferred, napi_value rejection)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, deferred);
    CHECK_ARG(env, rejection);

    auto nativeDeferred = reinterpret_cast<NativeDeferred*>(deferred);
    nativeDeferred->Reject(rejection);
    delete nativeDeferred;
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_is_promise(napi_env env, napi_value value, bool* is_promise)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, is_promise);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    *is_promise = nativeValue->IsPromise(vm);

    return napi_clear_last_error(env);
}

// promise reject events
NAPI_EXTERN napi_status napi_set_promise_rejection_callback(napi_env env, napi_ref ref, napi_ref checkRef)
{
    CHECK_ENV(env);
    CHECK_ARG(env, ref);
    CHECK_ARG(env, checkRef);

    auto rejectCallbackRef = reinterpret_cast<NativeReference*>(ref);
    auto checkCallbackRef = reinterpret_cast<NativeReference*>(checkRef);
    if (rejectCallbackRef == nullptr || checkCallbackRef == nullptr) {
        HILOG_ERROR("rejectCallbackRef or checkCallbackRef is nullptr");
    } else {
        auto engine = reinterpret_cast<NativeEngine*>(env);
        if (!engine->IsMainEnvContext()) {
            HILOG_FATAL("multi-context does not support this interface");
        }
        auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());
        engine->SetPromiseRejectCallBackRef(rejectCallbackRef);
        engine->SetCheckCallbackRef(checkCallbackRef);
        panda::JSNApi::SetHostPromiseRejectionTracker(const_cast<EcmaVM*>(vm), engine->GetPromiseRejectCallback(),
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

    *result = nullptr;
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_run_actor(napi_env env,
                                       const char* path,
                                       char* entryPoint,
                                       napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, result);

    std::string pathStr(path);
    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (!engine->IsMainEnvContext()) {
        HILOG_FATAL("multi-context does not support this interface");
    }
    *result = engine->GetAbcBufferAndRunActor(pathStr, entryPoint);
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_load_module(napi_env env, const char* path, napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, result);
    SWITCH_CONTEXT(env);
    *result = engine->NapiLoadModule(path);
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_load_module_with_info(napi_env env,
                                                   const char* path,
                                                   const char* module_info,
                                                   napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, result);
    SWITCH_CONTEXT(env);
    *result = engine->NapiLoadModuleWithInfo(path, module_info);
    if (*result != nullptr) {
        RETURN_STATUS_IF_FALSE(env, !LocalValueFromJsValue(*result)->IsUndefined(), napi_generic_failure);
    }
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_load_module_with_info_hybrid(napi_env env,
                                                          const char* path,
                                                          const char* module_info,
                                                          napi_value* result,
                                                          bool isHybrid)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, result);
    auto engine = reinterpret_cast<NativeEngine*>(env);
    *result = engine->NapiLoadModuleWithInfo(path, module_info, isHybrid);
    return GET_RETURN_STATUS(env);
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
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsFunction(vm);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_arguments_object(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsArgumentsObject(vm);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_async_function(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsAsyncFunction(vm);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_boolean_object(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsJSPrimitiveBoolean(vm);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_generator_function(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsGeneratorFunction(vm);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_map_iterator(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsMapIterator(vm);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_set_iterator(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsSetIterator(vm);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_generator_object(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsGeneratorObject(vm);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_module_namespace_object(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsModuleNamespaceObject(vm);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_proxy(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsProxy(vm);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_reg_exp(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsRegExp(vm);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_number_object(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsJSPrimitiveNumber(vm);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_map(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsMap(vm) || nativeValue->IsSharedMap(vm);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_set(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsSet(vm) || nativeValue->IsSharedSet(vm);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_string_object(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsJSPrimitiveString(vm);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_symbol_object(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsJSPrimitiveSymbol(vm);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_weak_map(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsWeakMap(vm);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_weak_set(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsWeakSet(vm);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_runtime(napi_env env, napi_env* result_env)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result_env);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (!engine->IsMainEnvContext()) {
        HILOG_FATAL("multi-context does not support this interface");
    }
    auto result = engine->CreateRuntime();
    if (result == nullptr) {
        return napi_generic_failure;
    }
    *result_env = reinterpret_cast<napi_env>(result);

    return napi_ok;
}

NAPI_EXTERN napi_status napi_serialize(napi_env env,
                                       napi_value object,
                                       napi_value transfer_list,
                                       napi_value clone_list,
                                       void** result)
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
    *result = panda::JSNApi::SerializeValue(vm, nativeValue, transferList, cloneList, false, false);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_serialize_inner(napi_env env, napi_value object, napi_value transfer_list,
                                             napi_value clone_list, bool defaultTransfer, bool defaultCloneSendable,
                                             void** result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, transfer_list);
    CHECK_ARG(env, result);

    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);
    auto transferList = LocalValueFromJsValue(transfer_list);
    auto cloneList = LocalValueFromJsValue(clone_list);
    *result =
        panda::JSNApi::SerializeValue(vm, nativeValue, transferList, cloneList, defaultTransfer, defaultCloneSendable);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_serialize_inner_with_error(napi_env env, napi_value object, napi_value transfer_list,
                                                        napi_value clone_list, bool defaultTransfer,
                                                        bool defaultCloneSendable, void** result, std::string& error)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, transfer_list);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(object);
    auto transferList = LocalValueFromJsValue(transfer_list);
    auto cloneList = LocalValueFromJsValue(clone_list);
    *result = panda::JSNApi::SerializeValueWithError(vm, nativeValue, transferList, cloneList, error, defaultTransfer,
                                                     defaultCloneSendable);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_deserialize(napi_env env, void* buffer, napi_value* object)
{
    CHECK_ENV(env);
    CHECK_ARG(env, buffer);
    CHECK_ARG(env, object);

    SWITCH_CONTEXT(env);
    auto vm = engine->GetEcmaVm();
    Local<panda::JSValueRef> res = panda::JSNApi::DeserializeValue(vm, buffer, reinterpret_cast<void*>(engine));
    *object = JsValueFromLocalValue(res);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_delete_serialization_data(napi_env env, void* buffer)
{
    CHECK_ENV(env);
    CHECK_ARG(env, buffer);

    panda::JSNApi::DeleteSerializationData(buffer);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_bigint_int64(napi_env env, int64_t value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    Local<panda::BigIntRef> object = panda::BigIntRef::New(vm, value);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_bigint_uint64(napi_env env, uint64_t value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
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

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsBigInt(vm), napi_bigint_expected);
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

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsBigInt(vm), napi_bigint_expected);
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
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsDate(vm);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_detached_arraybuffer(napi_env env, napi_value arraybuffer, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, arraybuffer);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(arraybuffer);
    bool isArrayBuffer = false;
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    bool isDetach = nativeValue->IsDetachedArraybuffer(vm, isArrayBuffer);
    if (isArrayBuffer) {
        *result = isDetach;
        return napi_clear_last_error(env);
    } else {
        return napi_set_last_error(env, napi_invalid_arg);
    }
}

NAPI_EXTERN napi_status napi_get_all_property_names(
    napi_env env, napi_value object, napi_key_collection_mode key_mode,
    napi_key_filter key_filter, napi_key_conversion key_conversion, napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, result);
    auto nativeValue = LocalValueFromJsValue(object);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, obj);
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
            HILOG_ERROR("%{public}s invalid arg", __func__);
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
            HILOG_ERROR("%{public}s invalid arg", __func__);
            return napi_set_last_error(env, napi_invalid_arg);
    }
    Local<panda::ArrayRef> arrayVal = obj->GetAllPropertyNames(vm, filter);
    *result = JsValueFromLocalValue(arrayVal);
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_detach_arraybuffer(napi_env env, napi_value arraybuffer)
{
    CHECK_ENV(env);
    CHECK_ARG(env, arraybuffer);

    auto nativeValue = LocalValueFromJsValue(arraybuffer);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject(vm), napi_object_expected);
    bool isArrayBuffer = false;
    nativeValue->DetachedArraybuffer(vm, isArrayBuffer);
    if (!isArrayBuffer) {
        return napi_set_last_error(env, napi_invalid_arg);
    }
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_type_tag_object(napi_env env, napi_value js_object, const napi_type_tag* type_tag)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, type_tag);

    auto nativeValue = LocalValueFromJsValue(js_object);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject(vm), napi_object_expected);
    auto obj = nativeValue->ToEcmaObject(vm);
    NapiTypeTag* typeTag = (NapiTypeTag*)type_tag;
    const char name[] = "ACENAPI_TYPETAG";
    bool hasPribate = false;
    bool result = true;
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
        Local<panda::JSValueRef> value = panda::BigIntRef::CreateBigWords(vm, sign, size,
                                                                          reinterpret_cast<const uint64_t*>(typeTag));
        Local<panda::StringRef> key = panda::StringRef::NewFromUtf8(vm, name);
        result = obj->Set(vm, key, value);
    }
    if (!result) {
        HILOG_ERROR("%{public}s invalid arg", __func__);
        return napi_set_last_error(env, napi_invalid_arg);
    }

    return napi_clear_last_error(env);
}

bool BigIntGetWordsArray(const EcmaVM* vm, Local<panda::BigIntRef> &value, int* signBit,
                         size_t* wordCount, uint64_t* words)
{
    if (wordCount == nullptr) {
        return false;
    }
    size_t size = static_cast<size_t>(value->GetWordsArraySize(vm));
    if (signBit == nullptr && words == nullptr) {
        *wordCount = size;
        return true;
    } else if (signBit != nullptr && words != nullptr) {
        if (size > *wordCount) {
            size = *wordCount;
        }
        bool sign = false;
        value->GetWordsArray(vm, &sign, size, words);
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

NAPI_EXTERN napi_status napi_check_object_type_tag(napi_env env,
                                                   napi_value js_object,
                                                   const napi_type_tag* type_tag,
                                                   bool* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, type_tag);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(js_object);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject(vm), napi_object_expected);
    auto obj = nativeValue->ToEcmaObject(vm);
    NapiTypeTag* typeTag = (NapiTypeTag*)type_tag;
    *result = false;
    const char name[] = "ACENAPI_TYPETAG";

    Local<panda::StringRef> key = panda::StringRef::NewFromUtf8(vm, name);
    *result = obj->Has(vm, key);
    if (*result) {
        Local<panda::StringRef> key = panda::StringRef::NewFromUtf8(vm, name);
        Local<panda::JSValueRef> object = obj->Get(vm, key);
        if (object->IsBigInt(vm)) {
            int sign;
            size_t size = 2; // 2: Indicates that the number of elements is 2
            NapiTypeTag tag;
            Local<panda::BigIntRef> bigintObj = object->ToBigInt(vm);
            BigIntGetWordsArray(vm, bigintObj, &sign, &size, reinterpret_cast<uint64_t*>(&tag));
            if (sign == 0 && ((size == 1) || (size == 2))) { // 2: Indicates that the number of elements is 2
                *result = (tag.lower == typeTag->lower && tag.upper == typeTag->upper);
            }
        }
    }
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_date(napi_env env, double time, napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, result);

    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    *result = JsValueFromLocalValue(DateRef::New(vm, time));

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_date_value(napi_env env, napi_value value, double* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(value);
    panda::JsiFastNativeScope fastNativeScope(vm);

    auto IsDate_result = nativeValue->IsDate(vm);
    Local<panda::DateRef> dateObj(nativeValue);
    if (IsDate_result) {
        *result = dateObj->GetTime(vm);
    } else {
        HILOG_ERROR("%{public}s date expected", __func__);
        return napi_set_last_error(env, napi_date_expected);
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_add_finalizer(napi_env env,
                                           napi_value js_object,
                                           void* native_object,
                                           napi_finalize finalize_cb,
                                           void* finalize_hint,
                                           napi_ref* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, finalize_cb);

    auto nativeValue = LocalValueFromJsValue(js_object);
    auto callback = reinterpret_cast<NapiNativeFinalize>(finalize_cb);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject(vm), napi_object_expected);
    NativeReference* reference = nullptr;
    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (result != nullptr) {
        reference = engine->CreateReference(js_object, 1, false, callback, native_object, finalize_hint);
        *result = reinterpret_cast<napi_ref>(reference);
    } else {
        reference = engine->CreateReference(js_object, 0, true, callback, native_object, finalize_hint);
    }
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_bigint_words(napi_env env,
                                                 int sign_bit,
                                                 size_t word_count,
                                                 const uint64_t* words,
                                                 napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, words);
    CHECK_ARG(env, result);
    RETURN_STATUS_IF_FALSE(env, word_count <= INT_MAX, napi_invalid_arg);

    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    constexpr int bigintMod = 2; // 2 : used for even number judgment
    bool sign = false;
    if ((sign_bit % bigintMod) == 1) {
        sign = true;
    }
    uint32_t size = (uint32_t)word_count;
    Local<panda::JSValueRef> value = panda::BigIntRef::CreateBigWords(vm, sign, size, words);

    if (panda::JSNApi::HasPendingException(vm)) {
        HILOG_ERROR("%{public}s pending exception", __func__);
        return napi_set_last_error(env, napi_pending_exception);
    }
    *result = JsValueFromLocalValue(value);
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_get_value_bigint_words(napi_env env,
                                                    napi_value value,
                                                    int* sign_bit,
                                                    size_t* word_count,
                                                    uint64_t* words)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, word_count);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    RETURN_STATUS_IF_FALSE(env, nativeValue->IsBigInt(vm), napi_object_expected);
    auto BigintObj = nativeValue->ToBigInt(vm);
    if (word_count == nullptr) {
        return napi_set_last_error(env, napi_invalid_arg);
    }
    size_t size = static_cast<size_t>(BigintObj->GetWordsArraySize(vm));
    if (sign_bit == nullptr && words == nullptr) {
        *word_count = size;
        return napi_set_last_error(env, napi_ok);
    } else if (sign_bit != nullptr && words != nullptr) {
        if (size > *word_count) {
            size = *word_count;
        }
        bool sign = false;
        BigintObj->GetWordsArray(vm, &sign, size, words);
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
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, result);

    SWITCH_CONTEXT(env);
    std::string pathStr(path);
    if (engine->IsApplicationApiVersionAPI11Plus()) {
        pathStr = panda::JSNApi::NormalizePath(path);
    }
    HILOG_DEBUG("napi_run_script_path path: %{public}s", pathStr.c_str());
    if (engine->IsRestrictedWorkerThread()) {
        *result = engine->RunScriptInRestrictedThread(pathStr.c_str());
    } else {
        *result = engine->RunScript(pathStr.c_str());
    }
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_is_big_int64_array(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsBigInt64Array(vm);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_big_uint64_array(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsBigUint64Array(vm);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_shared_array_buffer(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsSharedArrayBuffer(vm);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_stack_trace(napi_env env, std::string& stack)
{
    CHECK_ENV(env);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    [[maybe_unused]] auto vm = engine->GetEcmaVm();
    std::string rawStack;
#if !defined(PREVIEW) && !defined(IOS_PLATFORM)
    DFXJSNApi::BuildJsStackTrace(vm, rawStack);
    stack = engine->ExecuteTranslateBySourceMap(rawStack);
#else
    HILOG_WARN("GetStacktrace env get stack failed");
#endif

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_hybrid_stack_trace(napi_env env, std::string& stack)
{
    CHECK_ENV(env);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    if (DumpHybridStack(vm, stack, 3)) { // 3: skiped frames
        return napi_ok;
    } else {
        HILOG_WARN("GetHybridStacktrace env get hybrid stack failed");
        return napi_generic_failure;
    }
}

NAPI_EXTERN napi_status napi_object_get_keys(napi_env env, napi_value data, napi_value* result)
{
    CHECK_ENV(env);

    auto nativeValue = LocalValueFromJsValue(data);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject(vm), napi_object_expected);
    auto obj = nativeValue->ToEcmaObject(vm);
    Local<panda::ArrayRef> arrayVal = obj->GetOwnEnumerablePropertyNames(vm);

    *result = JsValueFromLocalValue(arrayVal);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_queue_async_work_with_qos(napi_env env, napi_async_work work, napi_qos_t qos)
{
    CHECK_ENV(env);
    CHECK_ARG(env, work);

    auto asyncWork = reinterpret_cast<NativeAsyncWork*>(work);
    asyncWork->QueueWithQos(reinterpret_cast<NativeEngine*>(env), qos);
    return napi_status::napi_ok;
}

static void* DetachFuncCallback(void* engine, void* object, void* hint, void* detachData)
{
    if (detachData == nullptr || (engine == nullptr || object == nullptr)) {
        HILOG_ERROR("DetachFuncCallback params has nullptr");
        return nullptr;
    }
    DetachCallback detach = reinterpret_cast<DetachCallback>(detachData);
    void* detachVal = detach(reinterpret_cast<NativeEngine*>(engine), object, hint);
    return detachVal;
}

static Local<panda::JSValueRef> AttachFuncCallback(void* engine, void* buffer, void* hint, void* attachData)
{
    if (engine == nullptr) {
        HILOG_ERROR("AttachFuncCallback engine is nullptr");
        return Local<panda::JSValueRef>();
    }
    auto vm = reinterpret_cast<NativeEngine*>(engine)->GetEcmaVm();
    if (attachData == nullptr || buffer == nullptr) {
        HILOG_ERROR("AttachFuncCallback params has nullptr");
        return panda::JSValueRef::Undefined(vm);
    }
    EscapeLocalScope scope(vm);
    Local<panda::JSValueRef> result = panda::JSValueRef::Undefined(vm);
    NapiAttachCallback attach = reinterpret_cast<NapiAttachCallback>(attachData);
    napi_value attachVal = attach(reinterpret_cast<napi_env>(engine), buffer, hint);
    if (attachVal == nullptr) {
        HILOG_WARN("AttachFunc return nullptr");
    } else {
        result = LocalValueFromJsValue(attachVal);
    }
    return scope.Escape(result);
}

NAPI_EXTERN napi_status napi_coerce_to_native_binding_object(napi_env env,
                                                             napi_value js_object,
                                                             napi_native_binding_detach_callback detach_cb,
                                                             napi_native_binding_attach_callback attach_cb,
                                                             void* native_object,
                                                             void* hint)
{
    CHECK_ENV(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, detach_cb);
    CHECK_ARG(env, attach_cb);
    CHECK_ARG(env, native_object);

    auto jsValue = LocalValueFromJsValue(js_object);
    SWITCH_CONTEXT(env);
    auto vm = engine->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    RETURN_STATUS_IF_FALSE(env, jsValue->IsObject(vm), napi_object_expected);
    auto obj = jsValue->ToEcmaObject(vm);

    panda::JSNApi::NativeBindingInfo* data = panda::JSNApi::NativeBindingInfo::CreateNewInstance();
    if (data == nullptr) {
        HILOG_ERROR("data is nullptr");
        return napi_set_last_error(env, napi_invalid_arg);
    }
    data->env = env;
    data->nativeValue = native_object;
    data->attachFunc = reinterpret_cast<void*>(AttachFuncCallback);
    data->attachData = reinterpret_cast<void*>(attach_cb);
    data->detachFunc = reinterpret_cast<void*>(DetachFuncCallback);
    data->detachData = reinterpret_cast<void*>(detach_cb);
    data->hint = hint;

    size_t nativeBindingSize = 7 * sizeof(void *); // 7 : params num
    Local<panda::NativePointerRef> value = panda::NativePointerRef::NewConcurrent(
        vm, data,
        [](void* env, void* data, void* info) {
            auto externalInfo = reinterpret_cast<panda::JSNApi::NativeBindingInfo*>(data);
            delete externalInfo;
        },
        nullptr, nativeBindingSize);

    bool res = obj->ConvertToNativeBindingObject(vm, value);
    if (res) {
        return napi_clear_last_error(env);
    }
    delete data;
    return napi_status::napi_generic_failure;
}

NAPI_EXTERN napi_status napi_add_detached_finalizer(napi_env env,
                                                    napi_value native_binding_object,
                                                    napi_detach_finalize_callback detach_finalize_cb,
                                                    void* finalize_hint)
{
    CHECK_ENV(env);
    CHECK_ARG(env, native_binding_object);
    CHECK_ARG(env, detach_finalize_cb);

    auto jsValue = LocalValueFromJsValue(native_binding_object);
    SWITCH_CONTEXT(env);
    auto vm = engine->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    RETURN_STATUS_IF_FALSE(env, jsValue->IsNativeBindingObject(vm), napi_object_expected);
    Local<ObjectRef> nativeObject(jsValue);

    Local<panda::NativePointerRef> nativeBindingPointer = nativeObject->GetNativeBindingPointer(vm);
    if (!nativeBindingPointer->IsNativePointer(vm)) {
        return napi_generic_failure;
    }

    auto info = static_cast<panda::JSNApi::NativeBindingInfo*>(nativeBindingPointer->Value());
    if (info == nullptr) {
        return napi_generic_failure;
    }

    info->detachedFinalizer = reinterpret_cast<void *>(detach_finalize_cb);
    info->detachedHint = finalize_hint;

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_print_string(napi_env env, napi_value value, std::string& result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    if (nativeValue->IsString(vm)) {
        Local<panda::StringRef> stringVal(nativeValue);
        result = stringVal->ToString(vm);
    }
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_run_event_loop(napi_env env, napi_event_mode mode)
{
    CHECK_ENV(env);

    if (mode < napi_event_mode_default || mode > napi_event_mode_nowait) {
        HILOG_ERROR("invalid mode %{public}d", static_cast<int32_t>(mode));
        return napi_status::napi_invalid_arg;
    }

    auto nativeEngine = reinterpret_cast<NativeEngine*>(env);
    if (!nativeEngine->IsMainEnvContext()) {
        HILOG_ERROR("multi-context does not support this interface");
        return napi_set_last_error(env, napi_invalid_arg);
    }

    auto result = nativeEngine->RunEventLoop(mode);
    if (result != napi_status::napi_ok) {
        HILOG_ERROR("failed due to error %{public}d", static_cast<int32_t>(result));
        return napi_set_last_error(env, result);
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_stop_event_loop(napi_env env)
{
    CHECK_ENV(env);

    auto nativeEngine = reinterpret_cast<NativeEngine*>(env);
    if (!nativeEngine->IsMainEnvContext()) {
        HILOG_ERROR("multi-context does not support this interface");
        return napi_set_last_error(env, napi_invalid_arg);
    }
    auto result = nativeEngine->StopEventLoop();
    if (result != napi_status::napi_ok) {
        HILOG_ERROR("stop event loop failed due to error %{public}d", static_cast<int32_t>(result));
        return napi_set_last_error(env, result);
    }
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_ark_runtime(napi_env* env)
{
    if (NativeCreateEnv::g_createNapiEnvCallback == nullptr) {
        HILOG_ERROR("invalid create callback");
        return napi_status::napi_invalid_arg;
    }

    bool success = WorkerManager::IncrementArkRuntimeCount();
    if (!success) {
        HILOG_ERROR("reach max ARKRuntime count limit");
        return napi_status::napi_create_ark_runtime_too_many_envs;
    }

    napi_status result = NativeCreateEnv::g_createNapiEnvCallback(env);
    if (result == napi_ok) {
        auto vm = reinterpret_cast<NativeEngine*>(*env)->GetEcmaVm();
        panda::JSNApi::SetExecuteBufferMode(vm);
    } else {
        WorkerManager::DecrementArkRuntimeCount();
    }
    return result;
}

NAPI_EXTERN napi_status napi_destroy_ark_runtime(napi_env* env)
{
    if (NativeCreateEnv::g_destroyNapiEnvCallback == nullptr) {
        HILOG_ERROR("invalid destroy callback");
        return napi_status::napi_invalid_arg;
    }

    napi_status result = NativeCreateEnv::g_destroyNapiEnvCallback(env);
    if (result == napi_status::napi_ok) {
        WorkerManager::DecrementArkRuntimeCount();
    }
    return result;
}

NAPI_EXTERN napi_status napi_is_concurrent_function(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsConcurrentFunction(vm);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_call_threadsafe_function_with_priority(napi_threadsafe_function func,
                                                                    void *data,
                                                                    napi_task_priority priority,
                                                                    bool isTail)
{
    CHECK_ENV(func);

    if (priority < napi_priority_immediate || priority > napi_priority_idle) {
        HILOG_ERROR("invalid priority %{public}d", static_cast<int32_t>(priority));
        return napi_status::napi_invalid_arg;
    }
    auto safeAsyncWork = reinterpret_cast<NativeSafeAsyncWork*>(func);
    int32_t innerPriority = static_cast<int32_t>(priority);
    auto res = safeAsyncWork->PostTask(data, innerPriority, isTail);
    if (res != napi_ok) {
        HILOG_ERROR("post task failed due to error %{public}d", res);
    }
    return res;
}

NAPI_EXTERN napi_status napi_open_fast_native_scope(napi_env env, napi_fast_native_scope* scope)
{
    CHECK_ENV(env);
    CHECK_ARG(env, scope);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    *scope = reinterpret_cast<napi_fast_native_scope>(new panda::JsiFastNativeScope(engine->GetEcmaVm()));
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_close_fast_native_scope(napi_env env, napi_fast_native_scope scope)
{
    CHECK_ENV(env);
    CHECK_ARG(env, scope);

    delete reinterpret_cast<panda::JsiFastNativeScope*>(scope);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_shared_array_buffer_info(napi_env env,
                                                          napi_value arraybuffer,
                                                          void** data,
                                                          size_t* byte_length)
{
    CHECK_ENV(env);
    CHECK_ARG(env, arraybuffer);
    CHECK_ARG(env, byte_length);

    auto nativeValue = LocalValueFromJsValue(arraybuffer);
    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);
    if (LIKELY(nativeValue->IsSharedArrayBuffer(vm))) {
        Local<panda::ArrayBufferRef> res(nativeValue);
        if (data != nullptr) {
            *data = res->GetBuffer(vm);
        }
        *byte_length = res->ByteLength(vm);
    } else {
        return napi_set_last_error(env, napi_arraybuffer_expected);
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_encode(napi_env env, napi_value src, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, src);
    CHECK_ARG(env, result);

    SWITCH_CONTEXT(env);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScoper(vm);

    auto nativeValue = LocalValueFromJsValue(src);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsString(vm), napi_string_expected);
    Local<panda::StringRef> stringVal(nativeValue);
    Local<TypedArrayRef> typedArray = stringVal->EncodeIntoUint8Array(vm);
    *result = JsValueFromLocalValue(typedArray);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_bitvector(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    *result = nativeValue->IsBitVector(vm);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_add_cleanup_finalizer(napi_env env, void (*fun)(void* arg), void* arg)
{
    CHECK_ENV(env);
    CHECK_ARG(env, fun);
    CROSS_THREAD_CHECK(env);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    engine->AddCleanupFinalizer(fun, arg);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_set_stackinfo(napi_env env, napi_stack_info* napi_info)
{
    CHECK_ARG(env, napi_info);
    ASSERT(napi_info != nullptr);
    panda::StackInfo info { napi_info->stack_start, napi_info->stack_size };
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JSNApi::SetStackInfo(vm, info);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_stackinfo(napi_env env, napi_stack_info* result)
{
    CHECK_ARG(env, result);
    ASSERT(result != nullptr);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    auto res = panda::JSNApi::GetStackInfo(vm);
    result->stack_start = res.stackStart;
    result->stack_size = res.stackSize;
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_throw_jsvalue(napi_env env, napi_value error)
{
    CHECK_ENV(env);
    CHECK_ARG(env, error);

    auto nativeValue = LocalValueFromJsValue(error);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::JsiFastNativeScope fastNativeScope(vm);

    panda::JSNApi::ThrowException(vm, nativeValue);
    return napi_clear_last_error(env);
}

// This interface is always used for load module on host
NAPI_EXTERN napi_status napi_load_module_with_path(napi_env env, const char* path, napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, path);
    CHECK_ARG(env, result);
    auto vm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    auto ret = panda::JSNApi::GetModuleNameSpaceWithPath(vm, path);
    if (panda::JSNApi::HasPendingException(vm)) {
        *result = nullptr;
    } else {
        *result = JsValueFromLocalValue(ret);
    }
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
        napi_load_module_with_info_hybrid(env, path.c_str(), module_info.c_str(), result, true);
    } else {
        napi_load_module_with_path(env, request_name, result);
    }

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

NAPI_EXTERN napi_status napi_remove_cleanup_finalizer(napi_env env, void (*fun)(void* arg), void* arg)
{
    CHECK_ENV(env);
    CHECK_ARG(env, fun);
    CROSS_THREAD_CHECK(env);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    engine->RemoveCleanupFinalizer(fun, arg);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_ark_context(napi_env env, napi_env* newEnv)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, newEnv);

    auto nativeEngine = reinterpret_cast<NativeEngine*>(env);
    auto vm = nativeEngine->GetEcmaVm();
    // cannot to utilize a created env to generate a new env
    if (!nativeEngine->IsMainEnvContext()) {
        HILOG_ERROR("env cannot be generate by using a created env");
        return napi_set_last_error(env, napi_invalid_arg);
    }

    // worker and taskpool will support multi-context later
    if (!nativeEngine->IsMainThread()) {
        HILOG_FATAL("multi-context feature only support main thread");
        return napi_set_last_error(env, napi_invalid_arg);
    }

    auto context = panda::JSNApi::CreateContext(vm);
    if (context.IsEmpty() || !context->IsJsGlobalEnv(vm)) {
        HILOG_ERROR("Failed to create ark context");
        return napi_set_last_error(env, napi_generic_failure);
    }
    ArkNativeEngine* newEngine = ArkNativeEngine::New(nativeEngine, const_cast<EcmaVM *>(vm), context);
    if (newEngine == nullptr) {
        HILOG_ERROR("Failed to create ark context engine");
        return napi_set_last_error(env, napi_generic_failure);
    }
    *newEnv = reinterpret_cast<napi_env>(newEngine);

    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_switch_ark_context(napi_env env)
{
    NAPI_PREAMBLE(env);
    auto nativeEngine = reinterpret_cast<NativeEngine*>(env);
    // worker and taskpool will support multi-context later
    if (!nativeEngine->IsMainThread()) {
        HILOG_FATAL("multi-context feature only support main thread");
        return napi_set_last_error(env, napi_invalid_arg);
    }

    napi_status status = nativeEngine->SwitchContext();
    if (status != napi_ok) {
        return napi_set_last_error(env, status);
    } else {
        HILOG_DEBUG("switch context successfully");
    }
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_destroy_ark_context(napi_env env)
{
    CHECK_ENV(env);

    auto nativeEngine = reinterpret_cast<NativeEngine*>(env);
    const EcmaVM* vm = nativeEngine->GetEcmaVm();
    // The destructor of `TryCatch` may update `engine->lastException_`.
    // If the engine is released before the `TryCatch` object is destroyed,
    // it can lead to a use-after-free (UAF) vulnerability.
    // Use equivalent logic instead of `TryCatch` to avoid this memory safety issue.
    if (!nativeEngine->lastException_.IsEmpty() || panda::JSNApi::HasPendingException(vm)) {
        nativeEngine->lastException_ = panda::JSNApi::GetAndClearUncaughtException(vm);
        return napi_set_last_error(env, napi_pending_exception);
    }

    // worker and taskpool will support multi-context later
    if (!nativeEngine->IsMainThread()) {
        HILOG_FATAL("multi-context feature only support main thread");
        return napi_set_last_error(env, napi_invalid_arg);
    }

    // Do not use `nativeEngine` or `env` after this if status is napi_ok
    napi_status status = nativeEngine->DestroyContext();
    if (status != napi_ok) {
        return napi_set_last_error(env, status);
    }
    return napi_ok;
}

NAPI_EXTERN napi_status napi_set_module_validate_callback(napi_module_validate_callback check_callback)
{
    CHECK_ENV(check_callback);

    ArkNativeEngine::SetModuleValidateCallback(check_callback);

    return napi_ok;
}

#ifdef PANDA_JS_ETS_HYBRID_MODE
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

NAPI_EXTERN napi_status napi_mark_from_object(napi_env env, napi_ref ref)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, ref);
    ArkNativeReference* reference = reinterpret_cast<ArkNativeReference*>(ref);
    reference->MarkFromObject();
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
#endif // PANDA_JS_ETS_HYBRID_MODE
