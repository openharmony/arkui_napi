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

#include "ecmascript/napi/include/jsnapi.h"
#include "native_api_internal.h"
#include "native_engine/native_property.h"
#include "native_engine/native_value.h"
#include "securec.h"
#include "utils/log.h"

using panda::ecmascript::EcmaVM;
using panda::Local;
using panda::LocalScope;
using panda::JSValueRef;
using panda::BooleanRef;
using panda::PrimitiveRef;
using panda::StringRef;
using panda::SymbolRef;
using panda::ObjectRef;
using panda::NumberRef;
using panda::ArrayRef;
using panda::ArrayBufferRef;
using panda::BufferRef;
using panda::BigIntRef;
using panda::DataViewRef;
using panda::DateRef;
using panda::PromiseCapabilityRef;
using panda::PromiseRef;
using panda::FunctionRef;
using panda::TypedArrayRef;
using panda::NativePointerRef;

inline napi_value JsValueFromLocalValue(Local<JSValueRef> local)
{
    return reinterpret_cast<napi_value>(*local);
}

inline Local<JSValueRef> LocalValueFromJsValue(napi_value v)
{
    Local<JSValueRef> local;
    memcpy(static_cast<void*>(&local), &v, sizeof(v));
    return local;
}

inline napi_deferred JsDeferredFromLocalValue(Local<JSValueRef> local)
{
    return reinterpret_cast<napi_deferred>(*local);
}

inline Local<JSValueRef> LocalValueFromJsDeferred(napi_deferred v)
{
    Local<JSValueRef> local;
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
    auto vm = engine->GetEcmaVm();
    Local<PrimitiveRef> value = JSValueRef::Undefined(vm);
    *result = JsValueFromLocalValue(value);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_null(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<PrimitiveRef> value = JSValueRef::Null(vm);
    *result = JsValueFromLocalValue(value);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_global(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<ObjectRef> value = panda::JSNApi::GetGlobalObject(vm);
    *result = JsValueFromLocalValue(value);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_boolean(napi_env env, bool value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<panda::BooleanRef> object = panda::BooleanRef::New(vm, value);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

// Methods to create Primitive types/Objects
NAPI_EXTERN napi_status napi_create_object(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<panda::ObjectRef> object = panda::ObjectRef::New(vm);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_array(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<panda::ArrayRef> object = panda::ArrayRef::New(vm, 0);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_array_with_length(napi_env env, size_t length, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<panda::ArrayRef> object = panda::ArrayRef::New(vm, 0);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_double(napi_env env, double value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<panda::NumberRef> object = panda::NumberRef::New(vm, value);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_int32(napi_env env, int32_t value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<panda::NumberRef> object = panda::NumberRef::New(vm, value);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_uint32(napi_env env, uint32_t value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<panda::NumberRef> object = panda::NumberRef::New(vm, value);
    *result = JsValueFromLocalValue(object);
    
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_int64(napi_env env, int64_t value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<panda::NumberRef> object = panda::NumberRef::New(vm, value);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_string_latin1(napi_env env, const char* str, size_t length, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, str);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<StringRef> object = StringRef::NewFromUtf8(vm, str, (length == NAPI_AUTO_LENGTH) ? strlen(str) : length);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_string_utf8(napi_env env, const char* str, size_t length, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, str);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<StringRef> object = StringRef::NewFromUtf8(vm, str, (length == NAPI_AUTO_LENGTH) ? strlen(str) : length);
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
    auto vm = engine->GetEcmaVm();
    Local<StringRef> object = StringRef::NewFromUtf16(vm, str, (length == NAPI_AUTO_LENGTH) ? char16Length : length);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_symbol(napi_env env, napi_value description, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<JSValueRef> object = LocalValueFromJsValue(description);
    if (description == nullptr) {
        const char* str = "";
        object = StringRef::NewFromUtf8(vm, str, 0);
    }
    RETURN_STATUS_IF_FALSE(env, object->IsString(), napi_invalid_arg);
    Local<SymbolRef> symbol = SymbolRef::New(vm, object);
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
    CHECK_ENV(env);
    CHECK_ARG(env, cb);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto callback = reinterpret_cast<NativeCallback>(cb);
    NativeValue* resultValue = nullptr;
    if (utf8name != nullptr) {
        resultValue = engine->CreateFunction(utf8name,
            (length == NAPI_AUTO_LENGTH) ? strlen(utf8name) : length, callback, data);
    } else {
        const char* name = "defaultName";
        resultValue = engine->CreateFunction(name,
            (length == NAPI_AUTO_LENGTH) ? strlen(name) : length, callback, data);
    }

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_error(napi_env env, napi_value code, napi_value msg, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, msg);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto codeValue = LocalValueFromJsValue(code);
    auto msgValue = LocalValueFromJsValue(msg);
    if (!codeValue->IsNull()) {
        RETURN_STATUS_IF_FALSE(env, codeValue->IsString(), napi_invalid_arg);
    }
    RETURN_STATUS_IF_FALSE(env, msgValue->IsString(), napi_invalid_arg);

    Local<JSValueRef> errorVal = panda::Exception::Error(vm, msgValue);
    if (!codeValue->IsNull()) {
        Local<StringRef> codeKey = StringRef::NewFromUtf8(vm, "code");
        Local<ObjectRef> errorObj(errorVal);
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

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto codeValue = LocalValueFromJsValue(code);
    auto msgValue = LocalValueFromJsValue(msg);
    if (!codeValue->IsNull()) {
        RETURN_STATUS_IF_FALSE(env, codeValue->IsString(), napi_invalid_arg);
    }
    RETURN_STATUS_IF_FALSE(env, msgValue->IsString(), napi_invalid_arg);

    Local<JSValueRef> errorVal = panda::Exception::Error(vm, msgValue);
    if (!codeValue->IsNull()) {
        Local<StringRef> codeKey = StringRef::NewFromUtf8(vm, "code");
        Local<ObjectRef> errorObj(errorVal);
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

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto codeValue = LocalValueFromJsValue(code);
    auto msgValue = LocalValueFromJsValue(msg);
    if (!codeValue->IsNull()) {
        RETURN_STATUS_IF_FALSE(env, codeValue->IsString(), napi_invalid_arg);
    }
    RETURN_STATUS_IF_FALSE(env, msgValue->IsString(), napi_invalid_arg);

    Local<JSValueRef> errorVal = panda::Exception::Error(vm, msgValue);
    if (!codeValue->IsNull()) {
        Local<StringRef> codeKey = StringRef::NewFromUtf8(vm, "code");
        Local<ObjectRef> errorObj(errorVal);
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

    Local<JSValueRef> valueObj = LocalValueFromJsValue(value);
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
    Local<NumberRef> NumberVal = nativeValue->ToNumber(engine->GetEcmaVm());
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
    Local<NumberRef> NumberVal = nativeValue->ToNumber(engine->GetEcmaVm());
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
    Local<NumberRef> NumberVal = nativeValue->ToNumber(engine->GetEcmaVm());
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

    Local<NumberRef> NumberVal = nativeValue->ToNumber(engine->GetEcmaVm());
    *result = NumberVal->Value();
    
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_bool(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    Local<JSValueRef> val = LocalValueFromJsValue(value);
    RETURN_STATUS_IF_FALSE(env, val->IsBoolean(), napi_boolean_expected);
    Local<BooleanRef> boolVal = val->ToBoolean(engine->GetEcmaVm());
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
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(value);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsString(), napi_string_expected);
    Local<StringRef> stringVal = nativeValue->ToString(vm);
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
    Local<StringRef> stringVal = nativeValue->ToString(vm);

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
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(value);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsString(), napi_string_expected);
    Local<StringRef> stringVal = nativeValue->ToString(vm);

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
    Local<JSValueRef> val = LocalValueFromJsValue(value);
    RETURN_STATUS_IF_FALSE(env, val->IsBoolean(), napi_boolean_expected);
    Local<BooleanRef> boolVal = val->ToBoolean(engine->GetEcmaVm());
    *result = JsValueFromLocalValue(boolVal);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_coerce_to_number(napi_env env, napi_value value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(value);
    *result = JsValueFromLocalValue(nativeValue->ToNumber(vm));

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_coerce_to_object(napi_env env, napi_value value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(value);
    *result = JsValueFromLocalValue(nativeValue->ToObject(vm));

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_coerce_to_string(napi_env env, napi_value value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    auto nativeValue = LocalValueFromJsValue(value);
    *result = JsValueFromLocalValue(nativeValue->ToString(vm));

    return napi_clear_last_error(env);
}

// Methods to work with Objects
NAPI_EXTERN napi_status napi_get_prototype(napi_env env, napi_value object, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto nativeValue = LocalValueFromJsValue(object);
    RETURN_STATUS_IF_FALSE(env, nativeValue->IsObject() || nativeValue->IsFunction(), napi_object_expected);
    Local<ObjectRef> obj = nativeValue->ToObject(engine->GetEcmaVm());
    Local<JSValueRef> val = obj->GetPrototype(engine->GetEcmaVm());
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
    Local<ArrayRef> arrayVal = obj->GetOwnPropertyNames(vm);
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

    Local<ObjectRef> obj = nativeValue->ToObject(vm);
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
    Local<ObjectRef> obj = nativeValue->ToObject(vm);
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
    Local<ObjectRef> obj = nativeValue->ToObject(vm);
    Local<JSValueRef> val = obj->Get(vm, propKey);
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
    Local<ObjectRef> obj = nativeValue->ToObject(vm);
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
    Local<ObjectRef> obj = nativeValue->ToObject(vm);
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
    Local<StringRef> key = StringRef::NewFromUtf8(vm, utf8name);
    Local<ObjectRef> obj = nativeValue->ToObject(vm);
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

    Local<StringRef> key = StringRef::NewFromUtf8(vm, utf8name);
    Local<ObjectRef> obj = nativeValue->ToObject(vm);
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

    Local<StringRef> key = StringRef::NewFromUtf8(vm, utf8name);
    Local<ObjectRef> obj = nativeValue->ToObject(vm);
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

    Local<ObjectRef> obj = nativeValue->ToObject(vm);
    Local<StringRef> key = StringRef::NewFromUtf8(vm, utf8name);
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

    Local<ObjectRef> obj = nativeValue->ToObject(vm);
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

    Local<ObjectRef> obj = nativeValue->ToObject(vm);
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

    Local<ObjectRef> obj = nativeValue->ToObject(vm);
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
    Local<ObjectRef> obj = nativeValue->ToObject(vm);
    bool deleteResult = obj->Delete(vm, index);
    if (result) {
        *result = deleteResult;
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_define_properties(napi_env env,
                                               napi_value object,
                                               size_t property_count,
                                               const napi_property_descriptor* properties)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, properties);

    auto nativeValue = reinterpret_cast<NativeValue*>(object);
    NativeValueType type = nativeValue->TypeOf();
    RETURN_STATUS_IF_FALSE(env, type == NATIVE_OBJECT || type == NATIVE_FUNCTION,
        napi_object_expected);
    NativeObject* nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));

    for (size_t i = 0; i < property_count; i++) {
        NativePropertyDescriptor property;
        property.utf8name = properties[i].utf8name;
        property.name = reinterpret_cast<NativeValue*>(properties[i].name);
        property.method = reinterpret_cast<NativeCallback>(properties[i].method);
        property.getter = reinterpret_cast<NativeCallback>(properties[i].getter);
        property.setter = reinterpret_cast<NativeCallback>(properties[i].setter);
        property.value = reinterpret_cast<NativeValue*>(properties[i].value);
        property.attributes = (uint32_t)properties[i].attributes;
        property.data = properties[i].data;
        nativeObject->DefineProperty(property);
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
    Local<ArrayRef> arr = nativeValue->ToObject(vm);
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
    auto nativeRecv = reinterpret_cast<NativeValue*>(recv);
    auto nativeFunc = reinterpret_cast<NativeValue*>(func);
    auto nativeArgv = reinterpret_cast<NativeValue* const*>(argv);

    RETURN_STATUS_IF_FALSE(env, nativeFunc->TypeOf() == NATIVE_FUNCTION, napi_function_expected);

    auto resultValue = engine->CallFunction(nativeRecv, nativeFunc, nativeArgv, argc);

    RETURN_STATUS_IF_FALSE(env, resultValue != nullptr, napi_pending_exception);
    if (result) {
        *result = reinterpret_cast<napi_value>(resultValue);
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

    Local<FunctionRef> constructorVal = nativeConstructor->ToObject(vm);

    std::vector<Local<JSValueRef>> args;
    args.reserve(argc);
    for (size_t i = 0; i < argc; i++) {
        if (argv[i] != nullptr) {
            Local<JSValueRef> arg = LocalValueFromJsValue(argv[i]);
            args.emplace_back(arg);
        } else {
            args.emplace_back(JSValueRef::Undefined(vm));
        }
    }
    Local<JSValueRef> instance = constructorVal->Constructor(vm, args.data(), argc);
    Local<ObjectRef> excep = panda::JSNApi::GetUncaughtException(vm);
    if (!excep.IsNull()) {
        HILOG_ERROR("ArkNativeEngineImpl::CreateInstance occur Exception");
        *result = nullptr;
    }
    *result = JsValueFromLocalValue(instance);

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
    Local<JSValueRef> thisVarObj = LocalValueFromJsValue(info->thisVar);
    Local<JSValueRef> functionVal = LocalValueFromJsValue(info->function);
    if (thisVarObj->InstanceOf(vm, functionVal)) {
        *result = info->function;
    } else {
        *result = nullptr;
    }

    return napi_clear_last_error(env);
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

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto callback = reinterpret_cast<NativeCallback>(constructor);
    auto nativeProperties = reinterpret_cast<const NativePropertyDescriptor*>(properties);

    size_t nameLength = std::min(length, strlen(utf8name));
    char newName[nameLength + 1];
    if (strncpy_s(newName, nameLength + 1, utf8name, nameLength) != EOK) {
        HILOG_ERROR("napi_define_class strncpy_s failed");
        *result = nullptr;
    } else {
        auto resultValue = engine->DefineClass(newName, callback, data, nativeProperties, property_count);
        *result = reinterpret_cast<napi_value>(resultValue);
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

    auto nativeValue = reinterpret_cast<NativeValue*>(js_object);
    auto callback = reinterpret_cast<NativeFinalize>(finalize_cb);
    NativeValueType type = nativeValue->TypeOf();
    RETURN_STATUS_IF_FALSE(env, type == NATIVE_OBJECT || type == NATIVE_FUNCTION,
        napi_object_expected);
    auto nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));
    if (result != nullptr) {
        nativeObject->SetNativePointer(
            native_object, callback, finalize_hint, reinterpret_cast<NativeReference**>(result));
    } else {
        nativeObject->SetNativePointer(native_object, callback, finalize_hint);
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

    auto nativeValue = reinterpret_cast<NativeValue*>(js_object);
    auto callback = reinterpret_cast<NativeFinalize>(finalize_cb);
    NativeValueType type = nativeValue->TypeOf();
    RETURN_STATUS_IF_FALSE(env, type == NATIVE_OBJECT || type == NATIVE_FUNCTION,
        napi_object_expected);
    auto nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));
    if (result != nullptr) {
        nativeObject->SetNativePointer(
            native_object, callback, finalize_hint, reinterpret_cast<NativeReference**>(result), native_binding_size);
    } else {
        nativeObject->SetNativePointer(native_object, callback, finalize_hint, nullptr, native_binding_size);
    }
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_unwrap(napi_env env, napi_value js_object, void** result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(js_object);

    NativeValueType type = nativeValue->TypeOf();
    RETURN_STATUS_IF_FALSE(env, type == NATIVE_OBJECT || type == NATIVE_FUNCTION,
        napi_object_expected);

    auto nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));

    *result = nativeObject->GetNativePointer();

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_remove_wrap(napi_env env, napi_value js_object, void** result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(js_object);

    NativeValueType type = nativeValue->TypeOf();
    RETURN_STATUS_IF_FALSE(env, type == NATIVE_OBJECT || type == NATIVE_FUNCTION,
        napi_object_expected);

    auto nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));

    *result = nativeObject->GetNativePointer();
    nativeObject->SetNativePointer(nullptr, nullptr, nullptr);

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
    Local<NativePointerRef> object = NativePointerRef::New(vm, data,
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
    Local<NativePointerRef> object = NativePointerRef::New(vm, data,
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
    Local<NativePointerRef> object = nativeValue->ToNativePointer(vm);
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
    auto nativeValue = reinterpret_cast<NativeValue*>(value);
    auto reference = engine->CreateReference(nativeValue, initial_refcount);

    *result = reinterpret_cast<napi_ref>(reference);
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

    auto resultValue = reference->Get();

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_open_handle_scope(napi_env env, napi_handle_scope* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);

    auto scopeManager = engine->GetScopeManager();
    if (scopeManager == nullptr) {
        return napi_set_last_error(env, napi_generic_failure);
    }
    *result = reinterpret_cast<napi_handle_scope>(scopeManager->Open());
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_close_handle_scope(napi_env env, napi_handle_scope scope)
{
    CHECK_ENV(env);
    CHECK_ARG(env, scope);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto nativeScope = reinterpret_cast<NativeScope*>(scope);

    auto scopeManager = engine->GetScopeManager();
    if (scopeManager == nullptr) {
        return napi_set_last_error(env, napi_generic_failure);
    }
    scopeManager->Close(nativeScope);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_open_escapable_handle_scope(napi_env env, napi_escapable_handle_scope* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto scopeManager = engine->GetScopeManager();

    if (scopeManager == nullptr) {
        return napi_set_last_error(env, napi_generic_failure);
    }

    auto NativeScope = scopeManager->OpenEscape();

    *result = reinterpret_cast<napi_escapable_handle_scope>(NativeScope);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_close_escapable_handle_scope(napi_env env, napi_escapable_handle_scope scope)
{
    CHECK_ENV(env);
    CHECK_ARG(env, scope);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto nativeScope = reinterpret_cast<NativeScope*>(scope);

    auto scopeManager = engine->GetScopeManager();
    if (scopeManager == nullptr) {
        return napi_set_last_error(env, napi_generic_failure);
    }
    scopeManager->CloseEscape(nativeScope);
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

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto nativeScope = reinterpret_cast<NativeScope*>(scope);
    auto escapeeValue = reinterpret_cast<NativeValue*>(escapee);

    auto scopeManager = engine->GetScopeManager();
    if (scopeManager == nullptr) {
        return napi_set_last_error(env, napi_generic_failure);
    }

    if (!nativeScope->escapeCalled) {
        auto resultValue = scopeManager->Escape(nativeScope, escapeeValue);
        *result = reinterpret_cast<napi_value>(resultValue);
        nativeScope->escapeCalled = true;
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
    Local<JSValueRef> error(JSValueRef::Undefined(vm));
    error = panda::Exception::Error(vm, StringRef::NewFromUtf8(vm, msg));
    if (code != nullptr) {
        Local<JSValueRef> codeKey = StringRef::NewFromUtf8(vm, "code");
        Local<JSValueRef> codeValue = StringRef::NewFromUtf8(vm, code);
        Local<ObjectRef> errorObj(error);
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
    Local<JSValueRef> error(JSValueRef::Undefined(vm));
    error = panda::Exception::TypeError(vm, StringRef::NewFromUtf8(vm, msg));
    if (code != nullptr) {
        Local<JSValueRef> codeKey = StringRef::NewFromUtf8(vm, "code");
        Local<JSValueRef> codeValue = StringRef::NewFromUtf8(vm, code);
        Local<ObjectRef> errorObj(error);
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
    Local<JSValueRef> error(JSValueRef::Undefined(vm));
    error = panda::Exception::RangeError(vm, StringRef::NewFromUtf8(vm, msg));
    if (code != nullptr) {
        Local<JSValueRef> codeKey = StringRef::NewFromUtf8(vm, "code");
        Local<JSValueRef> codeValue = StringRef::NewFromUtf8(vm, code);
        Local<ObjectRef> errorObj(error);
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
    auto vm = engine->GetEcmaVm();
    *result = panda::JSNApi::HasPendingException(vm);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_and_clear_last_exception(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<ObjectRef> exception = panda::JSNApi::GetAndClearUncaughtException(vm);
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
    auto vm = engine->GetEcmaVm();
    uint8_t** values = (uint8_t**)(data);
    Local<ArrayBufferRef> res = ArrayBufferRef::New(vm, byte_length);
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

    Local<ArrayBufferRef> object = ArrayBufferRef::New(vm, value, byte_length,
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
    Local<ArrayBufferRef> res = nativeValue->ToObject(vm);
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
    Local<BufferRef> obj = BufferRef::New(vm, size);
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
    Local<BufferRef> obj = BufferRef::New(vm, length);
    if (obj->IsNull()) {
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

    Local<BufferRef> object = BufferRef::New(vm, value, length,
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
    Local<BufferRef> res = nativeValue->ToObject(vm);
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
    Local<ObjectRef> obj = nativeValue->ToObject(engine->GetEcmaVm());
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
    Local<ObjectRef> obj = nativeValue->ToObject(engine->GetEcmaVm());
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
    Local<ArrayBufferRef> arrayBuf = value->ToObject(vm);
    Local<TypedArrayRef> typedArray(JSValueRef::Undefined(vm));

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
    Local<TypedArrayRef> typedArray = value->ToObject(vm);
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
    Local<ArrayBufferRef> res = arrayBufferValue->ToObject(vm);
    if (length + byte_offset > static_cast<size_t>(res->ByteLength(vm))) {
        napi_throw_range_error(
            env,
            "ERR_NAPI_INVALID_DATAVIEW_ARGS",
            "byte_offset + byte_length should be less than or "
            "equal to the size in bytes of the array passed in");
        return napi_set_last_error(env, napi_pending_exception);
    }

    Local<DataViewRef> dataView = DataViewRef::New(vm, res, byte_offset, length);

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
    Local<DataViewRef> dataViewObj = nativeValue->ToObject(vm);
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
    auto vm = engine->GetEcmaVm();
    Local<PromiseCapabilityRef> capability = PromiseCapabilityRef::New(vm);
    *deferred = JsDeferredFromLocalValue(capability);
    *promise = JsValueFromLocalValue(capability->GetPromise(vm));
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_resolve_deferred(napi_env env, napi_deferred deferred, napi_value resolution)
{
    CHECK_ENV(env);
    CHECK_ARG(env, deferred);
    CHECK_ARG(env, resolution);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<PromiseCapabilityRef> deferredObj = LocalValueFromJsDeferred(deferred);
    auto resolutionObj = LocalValueFromJsValue(resolution);
    deferredObj->Resolve(vm, resolutionObj);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_reject_deferred(napi_env env, napi_deferred deferred, napi_value rejection)
{
    CHECK_ENV(env);
    CHECK_ARG(env, deferred);
    CHECK_ARG(env, rejection);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<PromiseCapabilityRef> deferredObj = LocalValueFromJsDeferred(deferred);
    auto resolutionObj = LocalValueFromJsValue(rejection);
    deferredObj->Reject(vm, resolutionObj);

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

    engine->SetPromiseRejectCallback(rejectCallbackRef, checkCallbackRef);
    return napi_clear_last_error(env);
}

// Running a script
NAPI_EXTERN napi_status napi_run_script(napi_env env, napi_value script, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, script);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto scriptValue = reinterpret_cast<NativeValue*>(script);
    RETURN_STATUS_IF_FALSE(env, scriptValue->TypeOf() == NATIVE_STRING, napi_status::napi_string_expected);
    auto resultValue = engine->RunScript(scriptValue);
    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

// Runnint a buffer script, only used in ark
NAPI_EXTERN napi_status napi_run_buffer_script(napi_env env, std::vector<uint8_t>& buffer, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    NativeValue* resultValue = engine->RunBufferScript(buffer);
    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_run_actor(napi_env env, std::vector<uint8_t>& buffer,
                                       const char* descriptor, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto resultValue = engine->RunActor(buffer, descriptor);
    *result = reinterpret_cast<napi_value>(resultValue);
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
    Local<JSValueRef> res = panda::JSNApi::DeserializeValue(vm, recorderValue, reinterpret_cast<void*>(engine));

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
    Local<BigIntRef> object = BigIntRef::New(vm, value);
    *result = JsValueFromLocalValue(object);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_bigint_uint64(napi_env env, uint64_t value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    Local<BigIntRef> object = BigIntRef::New(vm, value);
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
    Local<BigIntRef> bigIntVal = nativeValue->ToBigInt(vm);
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
    Local<BigIntRef> bigIntVal = nativeValue->ToBigInt(vm);
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
    Local<ArrayBufferRef> bufObj = nativeValue->ToObject(vm);
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
    Local<ObjectRef> obj = nativeValue->ToObject(vm);
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
    Local<ArrayRef> arrayVal = obj->GetAllPropertyNames(vm, filter);
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
    Local<ArrayBufferRef> bufObj = nativeValue->ToObject(vm);
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

    Local<StringRef> key = StringRef::NewFromUtf8(vm, name);
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
        Local<JSValueRef> value = BigIntRef::CreateBigWords(vm, sign, size, reinterpret_cast<const uint64_t*>(typeTag));
        Local<StringRef> key = StringRef::NewFromUtf8(vm, name);
        result = obj->Set(vm, key, value);
    }
    if (!result) {
        return napi_set_last_error(env, napi_invalid_arg);
    }

    return napi_clear_last_error(env);
}

bool BigIntGetWordsArray(Local<BigIntRef> &value, int* signBit, size_t* wordCount, uint64_t* words)
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

    Local<StringRef> key = StringRef::NewFromUtf8(vm, name);
    *result = obj->Has(vm, key);
    if (*result) {
        Local<StringRef> key = StringRef::NewFromUtf8(vm, name);
        Local<JSValueRef> object = obj->Get(vm, key);
        if (object->IsBigInt()) {
            int sign;
            size_t size = 2; // 2: Indicates that the number of elements is 2
            NapiTypeTag tag;
            Local<BigIntRef> bigintObj = object->ToBigInt(vm);
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
    Local<DateRef> dateObj = nativeValue->ToObject(vm);
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

    auto nativeValue = reinterpret_cast<NativeValue*>(js_object);
    auto callback = reinterpret_cast<NativeFinalize>(finalize_cb);
    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_OBJECT, napi_object_expected);
    auto nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));
    nativeObject->AddFinalizer(native_object, callback, finalize_hint);
    if (result != nullptr) {
        auto engine = reinterpret_cast<NativeEngine*>(env);
        auto reference = engine->CreateReference(nativeValue, 1, callback, native_object, finalize_hint);
        *result = reinterpret_cast<napi_ref>(reference);
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
    Local<JSValueRef> value = BigIntRef::CreateBigWords(vm, sign, size, words);

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
    NativeValue* resultValue = nullptr;
    resultValue = engine->RunScript(path);
    *result = reinterpret_cast<napi_value>(resultValue);
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
    Local<ArrayRef> arrayVal = obj->GetOwnEnumerablePropertyNames(vm);

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
    auto nativeValue = reinterpret_cast<NativeValue*>(native_object);
    NativeValueType type = nativeValue->TypeOf();
    RETURN_STATUS_IF_FALSE(env, type == NATIVE_OBJECT || type == NATIVE_FUNCTION, napi_object_expected);
    auto nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));
    bool success = nativeObject->ConvertToNativeBindingObject(engine, detach, attach, object, hint);
    if (success) {
        return napi_clear_last_error(env);
    }
    return napi_status::napi_generic_failure;
}

NAPI_EXTERN napi_status napi_set_native_pointer(napi_env env,
                                                napi_value native_object,
                                                void* pointer,
                                                NapiNativeFinalize cb,
                                                void* hint,
                                                napi_ref* reference,
                                                size_t nativeBindingSize)
{
    CHECK_ENV(env);
    CHECK_ARG(env, native_object);

    auto nativeValue = reinterpret_cast<NativeValue*>(native_object);
    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_OBJECT, napi_object_expected);

    auto nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));
    auto nativeReference = reinterpret_cast<NativeReference**>(reference);
    nativeObject->SetNativePointer(pointer, cb, hint, nativeReference, nativeBindingSize);
    return napi_clear_last_error(env);
}