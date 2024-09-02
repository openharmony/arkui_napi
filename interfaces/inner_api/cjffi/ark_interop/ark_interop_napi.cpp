/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "ark_interop_napi.h"

#include <thread>
#include <ecmascript/js_tagged_value.h>

#include "ark_interop_external.h"
#include "ark_interop_internal.h"
#include "ark_interop_log.h"
#include "ark_interop_scope.h"
#include "securec.h"

#ifdef ERROR
#undef ERROR
#endif

using namespace panda;
using namespace panda::ecmascript;

bool ARKTSInner_ReportJSErrors(ARKTS_Env env, bool abortIfUnhandled)
{
    auto vm = P_CAST(env, EcmaVM*);
    if (!JSNApi::HasPendingException(vm)) {
        return false;
    }

    auto exception = JSNApi::GetAndClearUncaughtException(vm);
    auto jError = ARKTS_FromHandle(exception);

    // throw CJ exception
    ARKTSInner_ThrowJSErrorToCJ(env, jError);

    return true;
}

void ARKTSInner_ReportNativeError(const char* format, ...)
{
    constexpr size_t MAX_MESSAGE_SIZE = 0x100;
    char buffer[MAX_MESSAGE_SIZE];

    va_list args;
    va_start(args, format);
#ifdef __clang__
    /* code specific to clang compiler */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#elif __GNUC__
    /* code for GNU C compiler */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
    int ret = vsnprintf_s(buffer, sizeof(buffer) / sizeof(char), MAX_MESSAGE_SIZE - 1, format, args);
#ifdef __clang__
#pragma clang diagnostic pop
#elif __GNUC__
#pragma GCC diagnostic pop
#endif
    va_end(args);
    const char* formatMsg;

    if (ret == -1) {
        formatMsg = format;
    } else {
        buffer[ret] = '\0';
        formatMsg = buffer;
    }
    ARKTSInner_ThrowNativeErrorToCJ(formatMsg);
}

std::string ARKTSInner_FormatJSError(ARKTS_Env env, ARKTS_Value jsError)
{
    auto vm = P_CAST(env, EcmaVM*);
    auto exception = BIT_CAST(jsError, Local<JSValueRef>);
    if (exception->IsString(vm)) {
        auto errorInsJs = *P_CAST(jsError, StringRef*);
        return errorInsJs.ToString(vm);
    } else if (exception->IsError(vm)) {
        auto errorObJ = *P_CAST(jsError, ObjectRef*);
        auto nameValue = errorObJ.Get(vm, StringRef::NewFromUtf8(vm, "name"));
        auto messageValue = errorObJ.Get(vm, StringRef::NewFromUtf8(vm, "message"));
        auto stackValue = errorObJ.Get(vm, StringRef::NewFromUtf8(vm, "stack"));
        std::string name = nameValue->IsString(vm) ? nameValue->ToString(vm)->ToString(vm) : "";
        auto message = messageValue->IsString(vm) ? messageValue->ToString(vm)->ToString(vm) : "";
        auto stack = stackValue->IsString(vm) ? stackValue->ToString(vm)->ToString(vm) : "";
        return name + "\n" + message + "\n" + stack;
    } else {
        return "[unknown js Error type]";
    }
}

ARKTS_Value ARKTS_GetGlobalConstant(ARKTS_Env env)
{
    ARKTS_ASSERT_P(env, "env is null");
    auto vm = P_CAST(env, EcmaVM*);
    auto result = JSNApi::GetGlobalObject(vm);
    return ARKTS_FromHandle(result);
}

uint64_t ARKTS_GetPosixThreadId()
{
    auto id = std::this_thread::get_id();
    return *reinterpret_cast<uint64_t*>(&id);
}

ARKTS_ValueType ARKTS_GetValueType(ARKTS_Env env, ARKTS_Value src)
{
    ARKTS_ASSERT(src, "src is null", N_UNDEFINED);
    auto value = BIT_CAST(src, JSValueRef);
    if (value.IsHole()) {
        return N_UNDEFINED;
    } else if (value.IsNull()) {
        return N_NULL;
    } else if (value.IsUndefined()) {
        return N_UNDEFINED;
    } else if (value.IsBoolean()) {
        return N_BOOL;
    } else if (value.IsNumber()) {
        return N_NUMBER;
    }

    value = *P_CAST(src, JSValueRef*);
    auto vm = P_CAST(env, EcmaVM*);
    if (value.IsNull()) {
        return N_NULL;
    } else if (value.IsString(vm)) {
        return N_STRING;
    } else if (value.IsSymbol(vm)) {
        return N_SYMBOL;
    } else if (value.IsFunction(vm)) {
        return N_FUNCTION;
    } else if (value.IsBigInt(vm)) {
        return N_BIGINT;
    } else if (value.IsNativePointer(vm)) {
        return N_EXTERNAL;
    } else {
        return N_OBJECT;
    }
}

bool ARKTS_StrictEqual(ARKTS_Env env, ARKTS_Value a, ARKTS_Value b)
{
    ARKTS_ASSERT_F(env, "env is NULL");
    auto aType = ARKTS_GetValueType(env, a);
    auto bType = ARKTS_GetValueType(env, b);
    if (aType != bType) {
        return false;
    }
    if (aType == N_UNDEFINED || aType == N_NULL) {
        return true;
    }
    switch (aType) {
        case N_BOOL: {
            auto aRef = BIT_CAST(a, BooleanRef);
            auto bRef = BIT_CAST(b, BooleanRef);

            return aRef.Value() == bRef.Value();
        }
        case N_NUMBER: {
            auto aRef = BIT_CAST(a, JSValueRef);
            auto bRef = BIT_CAST(b, JSValueRef);

            double aValue = aRef.IsInt() ? BIT_CAST(a, IntegerRef).Value() : BIT_CAST(a, NumberRef).Value();
            double bValue = bRef.IsInt() ? BIT_CAST(b, IntegerRef).Value() : BIT_CAST(b, NumberRef).Value();

            return aValue == bValue;
        }
        default:
            auto aTag = *P_CAST(a, JSValueRef*);
            auto bTag = BIT_CAST(b, Local<JSValueRef>);
            auto vm = P_CAST(env, EcmaVM*);
            return aTag.IsStrictEquals(vm, bTag);
    }
}

ARKTS_Value ARKTS_CreateNull()
{
    return P_CAST(JSTaggedValue::VALUE_NULL, ARKTS_Value);
}

bool ARKTS_IsNull(ARKTS_Value value)
{
    return P_CAST(value, JSTaggedType) == JSTaggedValue::VALUE_NULL;
}

ARKTS_Value ARKTS_CreateUndefined()
{
    return P_CAST(JSTaggedValue::VALUE_UNDEFINED, ARKTS_Value);
}

bool ARKTS_IsUndefined(ARKTS_Value value)
{
    return P_CAST(value, JSTaggedType) == JSTaggedValue::VALUE_UNDEFINED;
}

ARKTS_Value ARKTS_CreateBool(bool value)
{
    return P_CAST(value ? JSTaggedValue::VALUE_TRUE : JSTaggedValue::VALUE_FALSE, ARKTS_Value);
}

bool ARKTS_IsBool(ARKTS_Value value)
{
    auto v = BIT_CAST(value, JSValueRef);
    return v.IsBoolean();
}

bool ARKTS_GetValueBool(ARKTS_Value value)
{
    auto tag = BIT_CAST(value, JSValueRef);
    ARKTS_ASSERT_F(tag.IsBoolean(), "not a boolean");
    auto boolRef = BIT_CAST(value, BooleanRef);
    return boolRef.Value();
}

ARKTS_Value ARKTS_CreateF64(double value)
{
    JSTaggedValue result(value);
    return BIT_CAST(result, ARKTS_Value);
}

double ARKTS_GetValueNumber(ARKTS_Value value)
{
    auto ref = BIT_CAST(value, JSValueRef);
    ARKTS_ASSERT_I(ref.IsNumber(), "not a number");
    if (ref.IsInt()) {
        return BIT_CAST(value, IntegerRef).Value();
    } else {
        return BIT_CAST(value, NumberRef).Value();
    }
}

bool ARKTS_IsNumber(ARKTS_Value value)
{
    auto jValue = BIT_CAST(value, JSValueRef);
    return jValue.IsNumber();
}

struct LambdaData {
    ARKTS_Env env;
    int64_t lambdaId;
};

static Local<JSValueRef> CJLambdaInvoker(JsiRuntimeCallInfo* callInfo)
{
    auto data = reinterpret_cast<LambdaData*>(callInfo->GetData());
    if (!data) {
        return JSValueRef::Undefined(callInfo->GetVM());
    }
    auto result = ARKTSInner_CJLambdaInvoker(P_CAST(callInfo, ARKTS_CallInfo), data->lambdaId);
    return BIT_CAST(result, Local<JSValueRef>);
}

static void CJLambdaDeleter(void* /* env: napi_env */, void* /*invoker*/, void* raw)
{
    auto data = reinterpret_cast<LambdaData*>(raw);
    if (!data) {
        return;
    }
    ARKTSInner_CJLambdaDeleter(data->env, data->lambdaId);
    delete data;
}

ARKTS_Value ARKTS_CreateFunc(ARKTS_Env env, int64_t lambdaId)
{
    ARKTS_ASSERT_P(env, "env is null");

    auto vm = P_CAST(env, EcmaVM*);
    auto result = FunctionRef::New(vm, CJLambdaInvoker, CJLambdaDeleter, new LambdaData {env, lambdaId}, true);

    return ARKTS_FromHandle(result);
}

bool ARKTS_IsClass(ARKTS_Env env, ARKTS_Value value)
{
    auto tag = BIT_CAST(value, JSValueRef);
    if (!tag.IsHeapObject()) {
        return false;
    }
    auto vm = P_CAST(env, EcmaVM*);
    tag = *P_CAST(value, JSValueRef*);
    return tag.IsConstructor(vm);
}

ARKTS_Value ARKTS_CreateClass(ARKTS_Env env, int64_t lambdaId, ARKTS_Value base)
{
    ARKTS_ASSERT_P(env, "env is null");

    auto vm = P_CAST(env, EcmaVM*);

    auto result = FunctionRef::NewClassFunction(vm, CJLambdaInvoker, CJLambdaDeleter,
                                                new LambdaData {env, lambdaId}, true);

    if (ARKTS_IsClass(env, base)) {
        auto baseClass = BIT_CAST(base, Local<FunctionRef>);
        result->Inherit(vm, baseClass);
    }

    return ARKTS_FromHandle(result);
}

ARKTS_Value ARKTS_GetPrototype(ARKTS_Env env, ARKTS_Value value)
{
    ARKTS_ASSERT_P(env, "env is null");
    ARKTS_ASSERT_P(ARKTS_IsClass(env, value), "value is not constructor");
    auto vm = P_CAST(env, EcmaVM*);
    auto clazz = BIT_CAST(value, Local<FunctionRef>);
    auto result = clazz->GetFunctionPrototype(vm);
    return ARKTS_FromHandle(result);
}

bool ARKTS_InstanceOf(ARKTS_Env env, ARKTS_Value object, ARKTS_Value clazz)
{
    ARKTS_ASSERT_F(env, "env is null");
    ARKTS_ASSERT_F(ARKTS_IsHeapObject(object), "object is not heap object");
    ARKTS_ASSERT_F(ARKTS_IsClass(env, clazz), "clazz is not a class");

    auto vm = P_CAST(env, EcmaVM*);
    auto targetObject = BIT_CAST(object, Local<JSValueRef>);
    auto targetClass = BIT_CAST(clazz, Local<FunctionRef>);

    return targetObject->InstanceOf(vm, targetClass);
}

bool ARKTS_IsCallable(ARKTS_Env env, ARKTS_Value value)
{
    return ARKTS_GetValueType(env, value) == N_FUNCTION;
}

static constexpr auto MAX_CALL_ARGS = 255;

ARKTS_INLINE void FormatArguments(int32_t numArgs, ARKTS_Value args[], Local<JSValueRef>* result)
{
    for (int32_t i = 0;i < numArgs; ++i) {
        result[i] = ARKTS_ToHandle<JSValueRef>(args[i]);
    }
}

ARKTS_Value ARKTS_Call(ARKTS_Env env, ARKTS_Value func, ARKTS_Value thisArg, int32_t numArgs, ARKTS_Value args[])
{
    ARKTS_ASSERT_P(env, "env is null");
    ARKTS_ASSERT_P(ARKTS_IsCallable(env, func), "func is not callable");
    ARKTS_ASSERT_P(numArgs <= MAX_CALL_ARGS, "too many arguments, 255 most");

    auto vm = P_CAST(env, EcmaVM*);

    auto funcHandle = *P_CAST(func, FunctionRef*);
    auto thisHandle = ARKTS_ToHandle<JSValueRef>(thisArg);

    Local<JSValueRef> formattedArgs[MAX_CALL_ARGS];
    FormatArguments(numArgs, args, formattedArgs);

    auto result = funcHandle.Call(vm, thisHandle, formattedArgs, numArgs);

    ARKTSInner_ReportJSErrors(env, true);

    return ARKTS_FromHandle(result);
}

ARKTS_Value ARKTS_New(ARKTS_Env env, ARKTS_Value clazz, int32_t numArgs, ARKTS_Value args[])
{
    ARKTS_ASSERT_P(env, "env is null");
    ARKTS_ASSERT_P(ARKTS_IsClass(env, clazz), "clazz is not class");
    ARKTS_ASSERT_P(numArgs <= MAX_CALL_ARGS, "too many arguments, 255 most");

    auto vm = P_CAST(env, EcmaVM*);
    auto funcHandle = *P_CAST(clazz, FunctionRef*);

    Local<JSValueRef> formattedArgs[MAX_CALL_ARGS];
    FormatArguments(numArgs, args, formattedArgs);

    auto result = funcHandle.Constructor(vm, formattedArgs, numArgs);

    ARKTSInner_ReportJSErrors(env, true);

    return ARKTS_FromHandle(result);
}

// JSArray elements stored in vector mode and dict mode.
// default in vector mode,
// if capacity grows gap bigger than 1024, will transform to dict mode.
ARKTS_Value ARKTS_CreateArray(ARKTS_Env env, uint32_t size)
{
    ARKTS_ASSERT_P(env, "env is null");

    auto vm = P_CAST(env, EcmaVM*);
    auto result = ArrayRef::New(vm, size);
    return BIT_CAST(result, ARKTS_Value);
}

ARKTS_Value ARKTS_CreateArrayWithInit(ARKTS_Env env, uint32_t size, ARKTS_Value* data)
{
    ARKTS_ASSERT_P(env, "env is null");
    auto vm = P_CAST(env, EcmaVM*);

    auto result = ArrayRef::New(vm, size);
    for (uint32_t i = 0;i < size; ++i) {
        ArrayRef::SetValueAt(vm, result, i, ARKTS_ToHandle<JSValueRef>(data[i]));
    }

    return BIT_CAST(result, ARKTS_Value);
}

uint32_t ARKTS_GetArrayLength(ARKTS_Env env, ARKTS_Value array)
{
    ARKTS_ASSERT_I(env, "env is null");
    ARKTS_ASSERT_I(ARKTS_IsArray(env, array), "array is not array");

    auto vm = P_CAST(env, EcmaVM*);
    auto jArr = *P_CAST(array, ArrayRef*);

    return jArr.Length(vm);
}

void ARKTS_SetElement(ARKTS_Env env, ARKTS_Value array, uint32_t index, ARKTS_Value value)
{
    ARKTS_ASSERT_V(env, "env is null");
    ARKTS_ASSERT_V(ARKTS_IsArray(env, array), "array is not array");

    auto vm = P_CAST(env, EcmaVM*);
    auto jArr = BIT_CAST(array, Local<ArrayRef>);

    ArrayRef::SetValueAt(vm, jArr, index, ARKTS_ToHandle<JSValueRef>(value));
}

ARKTS_Value ARKTS_GetElement(ARKTS_Env env, ARKTS_Value array, uint32_t index)
{
    ARKTS_ASSERT_P(env, "env is null");
    ARKTS_ASSERT_P(ARKTS_IsArray(env, array), "array is not array");
    auto vm = P_CAST(env, EcmaVM*);
    auto arr = BIT_CAST(array, Local<ArrayRef>);
    ARKTS_ASSERT_P(arr->Length(vm) > index, "out of index");

    auto result = ArrayRef::GetValueAt(vm, arr, index);

    return ARKTS_FromHandle(result);
}

bool ARKTS_IsArray(ARKTS_Env env, ARKTS_Value value)
{
    ARKTS_ASSERT_F(env, "env is NULL");
    auto v = BIT_CAST(value, JSValueRef);
    if (!v.IsHeapObject()) {
        return false;
    }
    v = *P_CAST(value, JSValueRef*);
    auto vm = P_CAST(env, EcmaVM*);
    return v.IsArray(vm);
}

ARKTS_Value ARKTS_CreateArrayBuffer(ARKTS_Env env, int32_t length)
{
    ARKTS_ASSERT_P(env, "env is null");
    auto vm = P_CAST(env, EcmaVM*);
    auto result = ArrayBufferRef::New(vm, length);
    return ARKTS_FromHandle(result);
}

ARKTS_Value ARKTS_CreateArrayBufferWithData(ARKTS_Env env, void* buffer, int32_t length,
    int64_t finalizerHint)
{
    ARKTS_ASSERT_P(env, "env is null");

    auto vm = P_CAST(env, EcmaVM*);
    auto result = ArrayBufferRef::New(vm, buffer, length, ARKTSInner_CJArrayBufferDeleter,
                                      reinterpret_cast<void*>(finalizerHint));

    return ARKTS_FromHandle(result);
}

bool ARKTS_IsArrayBuffer(ARKTS_Env env, ARKTS_Value value)
{
    auto tag = BIT_CAST(value, JSValueRef);
    if (!tag.IsHeapObject()) {
        return false;
    }
    auto vm = P_CAST(env, EcmaVM*);
    tag = *P_CAST(value, JSValueRef*);
    return tag.IsArrayBuffer(vm) || tag.IsTypedArray(vm) || tag.IsDataView(vm);
}

int32_t ARKTS_GetArrayBufferLength(ARKTS_Env env, ARKTS_Value value)
{
    ARKTS_ASSERT_I(env, "env is null");
    ARKTS_ASSERT_I(ARKTS_IsArrayBuffer(env, value), "value is not arrayBuffer");

    auto vm = P_CAST(env, EcmaVM*);
    auto tag = *P_CAST(value, JSValueRef*);
    if (tag.IsArrayBuffer(vm)) {
        return P_CAST(value, ArrayBufferRef*)->ByteLength(vm);
    } else if (tag.IsTypedArray(vm)) {
        auto arr = P_CAST(value, TypedArrayRef*);
        return arr->ByteLength(vm) - arr->ByteOffset(vm);
    } else {
        auto arr = P_CAST(value, DataViewRef*);
        return arr->ByteLength() - arr->ByteOffset();
    }
}

void* ARKTS_GetArrayBufferRawPtr(ARKTS_Env env, ARKTS_Value value)
{
    ARKTS_ASSERT_P(env, "env is null");
    ARKTS_ASSERT_P(ARKTS_IsArrayBuffer(env, value), "value is not arrayBuffer");

    auto vm = P_CAST(env, EcmaVM*);
    auto tag = *BIT_CAST(value, JSValueRef*);
    if (tag.IsArrayBuffer(vm)) {
        return P_CAST(value, ArrayBufferRef*)->GetBuffer(vm);
    } else if (tag.IsTypedArray(vm)) {
        auto arr = P_CAST(value, TypedArrayRef*);
        auto rawStart = arr->GetArrayBuffer(vm)->GetBuffer(vm);
        auto rawOffset = arr->ByteOffset(vm);
        return P_CAST(rawStart, uint8_t*) + rawOffset;
    } else {
        auto arr = P_CAST(value, DataViewRef*);
        auto rawStart = arr->GetArrayBuffer(vm)->GetBuffer(vm);
        auto rawOffset = arr->ByteOffset();
        return P_CAST(rawStart, uint8_t*) + rawOffset;
    }
}

int32_t ARKTS_ArrayBufferReadBytes(ARKTS_Env env, ARKTS_Value buffer, void* dest, int32_t count)
{
    auto src = ARKTS_GetArrayBufferRawPtr(env, buffer);
    auto srcSize = ARKTS_GetArrayBufferLength(env, buffer);
    auto targetSize = std::min(srcSize, count);
    if (memcpy_s(dest, count, src, targetSize) != targetSize) {
        LOGE("ARKTS_ArrayBufferReadBytes error, memcpy_s ret size incorrect");
    }
    return targetSize;
}

ARKTS_Value ARKTS_CreateExternal(ARKTS_Env env, void* data)
{
    ARKTS_ASSERT_P(env, "env is null");

    auto vm = P_CAST(env, EcmaVM*);
    auto result = NativePointerRef::New(vm, data, ARKTSInner_CJExternalDeleter, env);
    return BIT_CAST(result, ARKTS_Value);
}

bool ARKTS_IsExternal(ARKTS_Env env, ARKTS_Value value)
{
    auto prime = BIT_CAST(value, JSValueRef);
    if (!prime.IsHeapObject()) {
        return false;
    }
    auto handle = BIT_CAST(value, JSValueRef*);
    auto vm = P_CAST(env, EcmaVM*);
    return handle->IsNativePointer(vm);
}

void* ARKTS_GetExternalData(ARKTS_Env env, ARKTS_Value value)
{
    ARKTS_ASSERT_P(ARKTS_IsExternal(env, value), "value is not external data");
    auto external = *P_CAST(value, NativePointerRef*);
    return external.Value();
}

ARKTS_Promise ARKTS_CreatePromiseCapability(ARKTS_Env env)
{
    ARKTS_ASSERT_P(env, "env is null");

    auto vm = P_CAST(env, EcmaVM*);

    auto promise = PromiseCapabilityRef::New(vm);
    auto result = new Global<PromiseCapabilityRef>(vm, promise);

    return BIT_CAST(result, ARKTS_Promise);
}

ARKTS_Value ARKTS_GetPromiseFromCapability(ARKTS_Env env, ARKTS_Promise prom)
{
    ARKTS_ASSERT_P(env, "env is null");
    ARKTS_ASSERT_P(prom, "prom is null");

    auto vm = P_CAST(env, EcmaVM*);
    auto promise = *P_CAST(prom, Global<PromiseCapabilityRef>*);
    auto result = (*promise)->GetPromise(vm);

    return BIT_CAST(result, ARKTS_Value);
}

void ARKTS_PromiseCapabilityResolve(ARKTS_Env env, ARKTS_Promise prom, ARKTS_Value result)
{
    ARKTS_ASSERT_V(env, "env is null");
    ARKTS_ASSERT_V(prom, "prom is null");

    auto vm = P_CAST(env, EcmaVM*);
    auto promise = P_CAST(prom, Global<PromiseCapabilityRef>*);
    (*promise)->Resolve(vm, ARKTS_ToHandle<JSValueRef>(result));
    delete promise;
}

void ARKTS_PromiseCapabilityReject(ARKTS_Env env, ARKTS_Promise prom, ARKTS_Value result)
{
    ARKTS_ASSERT_V(env, "env is null");
    ARKTS_ASSERT_V(prom, "prom is null");

    auto vm = P_CAST(env, EcmaVM*);
    auto promise = P_CAST(prom, Global<PromiseCapabilityRef>*);
    (*promise)->Reject(vm, ARKTS_ToHandle<JSValueRef>(result));
    delete promise;
}

bool ARKTS_IsPromise(ARKTS_Env env, ARKTS_Value value)
{
    auto v = BIT_CAST(value, JSValueRef);
    if (!v.IsHeapObject()) {
        return false;
    }
    auto vm = P_CAST(env, EcmaVM*);
    v= *P_CAST(value, JSValueRef*);
    return v.IsPromise(vm);
}

ARKTS_Value ARKTS_PromiseThen(ARKTS_Env env, ARKTS_Value prom, ARKTS_Value onFulfilled, ARKTS_Value onRejected)
{
    ARKTS_ASSERT_P(env, "env is null");
    ARKTS_ASSERT_P(ARKTS_IsPromise(env, prom), "arg is not a JSPromise");
    ARKTS_ASSERT_P(ARKTS_IsCallable(env, onFulfilled), "onFulfilled is not callable");

    auto vm = P_CAST(env, EcmaVM*);
    auto promise = *BIT_CAST(prom, PromiseRef*);
    auto onFulfilledFunc = BIT_CAST(onFulfilled, Local<FunctionRef>);
    auto onRejectTag = BIT_CAST(onRejected, Local<JSValueRef>);
    Local<PromiseRef> result;
    if (onRejectTag->IsFunction(vm)) {
        auto onRejectedFunc = BIT_CAST(onRejected, Local<FunctionRef>);
        result = promise.Then(vm, onFulfilledFunc, onRejectedFunc);
    } else {
        result = promise.Then(vm, onFulfilledFunc);
    }
    return ARKTS_FromHandle(result);
}

void ARKTS_PromiseCatch(ARKTS_Env env, ARKTS_Value prom, ARKTS_Value callback)
{
    ARKTS_ASSERT_V(env, "env is null");
    ARKTS_ASSERT_V(ARKTS_IsPromise(env, prom), "arg is not a JSPromise");
    ARKTS_ASSERT_V(ARKTS_IsCallable(env, callback), "callback is not callable");

    auto vm = P_CAST(env, EcmaVM*);
    auto promise = BIT_CAST(prom, PromiseRef*);
    auto callbackFunc = BIT_CAST(callback, Local<FunctionRef>);
    promise->Catch(vm, callbackFunc);
}

ARKTS_Scope ARKTS_OpenScope(ARKTS_Env env)
{
    ARKTS_ASSERT_P(env, "env is null");
    auto vm = P_CAST(env, EcmaVM*);
    return CJ::ARKTS_ScopeManager::OpenScope(vm);
}

ARKTS_Result ARKTS_Return(ARKTS_Env env, ARKTS_Scope scope, ARKTS_Value value)
{
    ARKTS_ASSERT_P(env, "env is null");
    ARKTS_ASSERT_P(value, "value is invalid");

    auto vm = P_CAST(env, EcmaVM*);
    if (!CJ::ARKTS_ScopeManager::CloseScope(scope)) {
        ARKTSInner_ReportNativeError("exit scope of unknown");
    }

    return ARKTS_ToResult(vm, value);
}

void ARKTS_CloseScope(ARKTS_Env env, ARKTS_Scope scope)
{
    ARKTS_ASSERT_V(env, "env is null");
    ARKTS_ASSERT_V(scope, "scope is invalid");

    if (!CJ::ARKTS_ScopeManager::CloseScope(scope)) {
        ARKTSInner_ReportNativeError("exit scope of unknown");
    }
}

panda::JSValueRef* ARKTSInner_Escape(ARKTS_Env env, ARKTS_Scope scope, ARKTS_Value ret)
{
    if (!CJ::ARKTS_ScopeManager::CloseScope(scope)) {
        ARKTSInner_ReportNativeError("exit scope of unknown");
    }

    auto vm = P_CAST(env, EcmaVM*);
    auto value = ARKTS_ToValue(ret);

    auto result = JSNApi::CreateLocal(vm, value);

    return BIT_CAST(result, JSValueRef*);
}

void ARKTS_Throw(ARKTS_Env env, ARKTS_Value error)
{
    ARKTS_ASSERT_V(env, "env is null");
    auto vm = P_CAST(env, EcmaVM*);

    if (JSNApi::HasPendingException(vm)) {
        LOGE("ARKTS_Throw failed, vm has uncaught exception");
        return;
    }

    auto exception = ARKTS_ToHandle<JSValueRef>(error);
    if (exception->IsString(vm)) {
        JSNApi::ThrowException(vm, Exception::Error(vm, BIT_CAST(exception, Local<StringRef>)));
    } else {
        JSNApi::ThrowException(vm, exception);
    }
}

ARKTS_Env ARKTS_GetCallEnv(ARKTS_CallInfo info)
{
    ARKTS_ASSERT_I(info, "info is null");
    auto runInfo = P_CAST(info, JsiRuntimeCallInfo*);
    auto vm = runInfo->GetVM();
    return P_CAST(vm, ARKTS_Env);
}

uint32_t ARKTS_GetArgCount(ARKTS_CallInfo info)
{
    ARKTS_ASSERT_I(info, "info is null");
    auto runInfo = P_CAST(info, JsiRuntimeCallInfo*);
    return runInfo->GetArgsNumber();
}

ARKTS_Value ARKTS_GetArg(ARKTS_CallInfo info, uint32_t index)
{
    ARKTS_ASSERT_P(info, "info is null");
    auto runInfo = P_CAST(info, JsiRuntimeCallInfo*);
    ARKTS_ASSERT_P(runInfo->GetArgsNumber() > index, "out of index");
    auto result = runInfo->GetCallArgRef(index);
    return ARKTS_FromHandle(result);
}

ARKTS_Value ARKTS_GetThisArg(ARKTS_CallInfo info)
{
    ARKTS_ASSERT_P(info, "info is null");
    auto runInfo = P_CAST(info, JsiRuntimeCallInfo*);
    auto result = runInfo->GetThisRef();
    return ARKTS_FromHandle(result);
}

void* ARKTS_GetGlobalNapiEnv(ARKTS_Env env)
{
    ARKTS_ASSERT_P(env, "env is null");
    auto vm = P_CAST(env, EcmaVM*);
    return JSNApi::GetEnv(vm);
}
