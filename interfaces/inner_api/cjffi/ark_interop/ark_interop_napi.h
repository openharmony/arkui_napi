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

#ifndef NAPI_ARK_INTEROP_NAPI_H
#define NAPI_ARK_INTEROP_NAPI_H

#include <cstdint>
#include <cstddef>
#include <iostream>

#include "ark_interop_macro.h"

DECL_START

/**
 * es6 types (corresponds to typeof)
 */
typedef enum {
    N_UNDEFINED,
    N_NULL,
    N_NUMBER,
    N_BOOL,
    N_BIGINT,
    N_STRING,
    N_SYMBOL,
    N_OBJECT,
    N_FUNCTION,
    N_EXTERNAL,
} ARKTS_ValueType;

/**
 * js object property descriptor attribute
 */
typedef enum {
    N_WRITABLE = 0x1,
    N_ENUMERABLE = 0x2,
    N_CONFIGURABLE = 0x4,
} ARKTS_PropertyFlag;

typedef struct ARKTS_Value_* ARKTS_Value;
typedef struct ARKTS_Env_* ARKTS_Env;
typedef struct ARKTS_CallInfo_* ARKTS_CallInfo;
typedef struct ARKTS_Result_* ARKTS_Result;
/**
 * be caution: ARKTS_Promise is not ARKTS_Value, you may not cast ARKTS_Promise to ARKTS_Value.
 */
typedef struct ARKTS_Promise_* ARKTS_Promise;
typedef struct ARKTS_Scope_* ARKTS_Scope;
typedef struct ARKTS_Global_* ARKTS_Global;
typedef struct ARKTS_Engine_* ARKTS_Engine;

typedef ARKTS_Result (*ARKTS_Callback)(ARKTS_CallInfo);
typedef ARKTS_Value (*ARKTS_ModuleExporter)(ARKTS_Env env, ARKTS_Value);

EXPORT ARKTS_Value ARKTS_GetGlobalConstant(ARKTS_Env env);
EXPORT uint64_t ARKTS_GetPosixThreadId();

EXPORT ARKTS_ValueType ARKTS_GetValueType(ARKTS_Env env, ARKTS_Value src);
EXPORT bool ARKTS_StrictEqual(ARKTS_Env env, ARKTS_Value a, ARKTS_Value b);

EXPORT double ARKTS_GetValueNumber(ARKTS_Value value);
EXPORT bool ARKTS_IsNumber(ARKTS_Value value);
EXPORT ARKTS_Value ARKTS_CreateF64(double value);
EXPORT ARKTS_Value ARKTS_CreateI32(int32_t value);

EXPORT int32_t ARKTS_GetValueUtf8(ARKTS_Env env, ARKTS_Value value, int32_t capacity, char* buffer);
EXPORT int32_t ARKTS_GetValueUtf8Size(ARKTS_Env env, ARKTS_Value value);
EXPORT ARKTS_Value ARKTS_CreateUtf8(ARKTS_Env env, const char* value, int32_t size);
/**
 * the result requires manually free, use ARKTS_FreeCString
 */
EXPORT const char* ARKTS_GetValueCString(ARKTS_Env env, ARKTS_Value value);
EXPORT void ARKTS_FreeCString(const char* src);
EXPORT bool ARKTS_IsString(ARKTS_Env env, ARKTS_Value value);

EXPORT ARKTS_Value ARKTS_CreateFunc(ARKTS_Env env, int64_t lambdaId);
/**
 * logically equals to typeof(value) == "function"
 */
EXPORT bool ARKTS_IsCallable(ARKTS_Env env, ARKTS_Value value);

EXPORT bool ARKTS_IsClass(ARKTS_Env env, ARKTS_Value value);
EXPORT ARKTS_Value ARKTS_CreateClass(ARKTS_Env env, int64_t lambdaId, ARKTS_Value super);
EXPORT ARKTS_Value ARKTS_GetPrototype(ARKTS_Env env, ARKTS_Value value);
EXPORT bool ARKTS_InstanceOf(ARKTS_Env env, ARKTS_Value object, ARKTS_Value clazz);

EXPORT ARKTS_Env ARKTS_GetCallEnv(ARKTS_CallInfo info);
EXPORT uint32_t ARKTS_GetArgCount(ARKTS_CallInfo info);
EXPORT ARKTS_Value ARKTS_GetArg(ARKTS_CallInfo info, uint32_t index);
EXPORT ARKTS_Value ARKTS_GetThisArg(ARKTS_CallInfo info);

EXPORT ARKTS_Value ARKTS_Call(ARKTS_Env env, ARKTS_Value func, ARKTS_Value thisArg,
    int32_t numArgs, ARKTS_Value args[]);
EXPORT ARKTS_Value ARKTS_New(ARKTS_Env env, ARKTS_Value clazz, int32_t numArgs, ARKTS_Value args[]);

EXPORT ARKTS_Value ARKTS_CreateNull();
EXPORT bool ARKTS_IsNull(ARKTS_Value value);
EXPORT ARKTS_Value ARKTS_CreateUndefined();
EXPORT bool ARKTS_IsUndefined(ARKTS_Value value);
EXPORT ARKTS_Value ARKTS_CreateBool(bool value);
EXPORT bool ARKTS_IsBool(ARKTS_Value value);
EXPORT bool ARKTS_GetValueBool(ARKTS_Value value);

EXPORT ARKTS_Value ARKTS_CreateObject(ARKTS_Env env);
/**
 * js types excludes [number, boolean, null, undefined] are JSHeapObject
 */
EXPORT bool ARKTS_IsHeapObject(ARKTS_Value value);

// only available for JSObject, any other heap object(like JSArray, JSProxy...) supposed to call it's type-specific api
EXPORT bool ARKTS_IsObject(ARKTS_Env env, ARKTS_Value value);
EXPORT bool ARKTS_HasOwnProperty(ARKTS_Env env, ARKTS_Value jobj, ARKTS_Value jkey);
EXPORT ARKTS_Value ARKTS_EnumOwnProperties(ARKTS_Env env, ARKTS_Value jobj);
EXPORT void ARKTS_DefineOwnProperty(ARKTS_Env env, ARKTS_Value jobj, ARKTS_Value jkey,
    ARKTS_Value jvalue, ARKTS_PropertyFlag attrs);

typedef struct ARKTS_Accessor_ {
    ARKTS_Value getter;
    ARKTS_Value setter;
    ARKTS_PropertyFlag attrs;
} ARKTS_Accessor;

EXPORT void ARKTS_DefineAccessors(ARKTS_Env env, ARKTS_Value jobj, ARKTS_Value jkey, ARKTS_Accessor accessor);
EXPORT void ARKTS_SetProperty(ARKTS_Env env, ARKTS_Value jobj, ARKTS_Value jkey, ARKTS_Value jvalue);
EXPORT ARKTS_Value ARKTS_GetProperty(ARKTS_Env env, ARKTS_Value jobj, ARKTS_Value jkey);

EXPORT ARKTS_Value ARKTS_CreateArray(ARKTS_Env env, uint32_t size);
EXPORT ARKTS_Value ARKTS_CreateArrayWithInit(ARKTS_Env env, uint32_t size, ARKTS_Value data[]);
EXPORT uint32_t ARKTS_GetArrayLength(ARKTS_Env env, ARKTS_Value array);
EXPORT void ARKTS_SetElement(ARKTS_Env env, ARKTS_Value array, uint32_t index, ARKTS_Value value);
EXPORT ARKTS_Value ARKTS_GetElement(ARKTS_Env env, ARKTS_Value array, uint32_t index);
EXPORT bool ARKTS_IsArray(ARKTS_Env env, ARKTS_Value value);

EXPORT ARKTS_Global ARKTS_CreateGlobal(ARKTS_Env env, ARKTS_Value value);
EXPORT ARKTS_Value ARKTS_GetGlobalValue(ARKTS_Global global);
EXPORT void ARKTS_DisposeGlobal(ARKTS_Env env, ARKTS_Global global);

EXPORT ARKTS_Value ARKTS_CreateExternal(ARKTS_Env env, void* data);
EXPORT bool ARKTS_IsExternal(ARKTS_Env env, ARKTS_Value value);
EXPORT void* ARKTS_GetExternalData(ARKTS_Env env, ARKTS_Value value);

EXPORT ARKTS_Value ARKTS_CreateSymbol(ARKTS_Env env, const char* description, int32_t length);
EXPORT bool ARKTS_IsSymbol(ARKTS_Env env, ARKTS_Value value);
EXPORT const char* ARKTS_GetSymbolDesc(ARKTS_Env env, ARKTS_Value value);

EXPORT ARKTS_Engine ARKTS_CreateEngine();
EXPORT void* ARKTS_GetNAPIEnv(ARKTS_Engine engine);
EXPORT void ARKTS_DestroyEngine(ARKTS_Engine engine);
EXPORT ARKTS_Env ARKTS_GetContext(ARKTS_Engine engine);
EXPORT ARKTS_Engine ARKTS_CreateEngineWithNewThread();
EXPORT uint64_t ARKTS_GetThreadIdOfEngine(ARKTS_Engine engine);

EXPORT bool ARKTS_LoadEntryFromAbc(ARKTS_Engine engine, const char* filePath, const char* entryPoint,
    bool forceReload);
EXPORT ARKTS_Value ARKTS_ImportFromEntry(ARKTS_Engine engine, const char* entryPoint, const char* importName);

EXPORT ARKTS_Value ARKTS_Require(ARKTS_Env env, const char* target, bool isNativeModule, bool isAppModule,
    const char* relativePath);

/**
 * promise in js and native are in different forms,
 * ARKTS_Promise the native form, ARKTS_Value is the js form, never cast one to another
 * the interface returns both forms, native ARKTS_Promise returns by params 'out',
 * ARKTS_Promise won't be collected until ARKTS_PromiseCapabilityResolve or ARKTS_PromiseCapabilityReject
 */
EXPORT ARKTS_Promise ARKTS_CreatePromiseCapability(ARKTS_Env env);
EXPORT ARKTS_Value ARKTS_GetPromiseFromCapability(ARKTS_Env env, ARKTS_Promise prom);
EXPORT void ARKTS_PromiseCapabilityResolve(ARKTS_Env env, ARKTS_Promise prom, ARKTS_Value result);
EXPORT void ARKTS_PromiseCapabilityReject(ARKTS_Env env, ARKTS_Promise prom, ARKTS_Value result);
EXPORT bool ARKTS_IsPromise(ARKTS_Env env, ARKTS_Value value);
EXPORT ARKTS_Value ARKTS_PromiseThen(ARKTS_Env env, ARKTS_Value prom, ARKTS_Value onFulfilled,
    ARKTS_Value onRejected);
EXPORT void ARKTS_PromiseCatch(ARKTS_Env env, ARKTS_Value prom, ARKTS_Value callback);

EXPORT void ARKTS_CreateAsyncTask(ARKTS_Env env, int64_t callbackId);

/**
 * if native func call js function, GC would be triggered and objects created by native would be collected.
 *  objects created in scope will keep alive
 * scope will close by ARKTS_Return
 */
EXPORT ARKTS_Scope ARKTS_OpenScope(ARKTS_Env env);
EXPORT void ARKTS_CloseScope(ARKTS_Env env, ARKTS_Scope scope);
EXPORT ARKTS_Result ARKTS_Return(ARKTS_Env env, ARKTS_Scope scope, ARKTS_Value value);

/**
 * if error supposed to be catch by js, returns asap,
 * some native api may lead to native exception handling procedural
 */
EXPORT void ARKTS_Throw(ARKTS_Env env, ARKTS_Value error);

EXPORT ARKTS_Value ARKTS_CreateArrayBuffer(ARKTS_Env env, int32_t);
EXPORT ARKTS_Value ARKTS_CreateArrayBufferWithData(ARKTS_Env env, void* buffer, int32_t length,
    int64_t finalizerHint);
EXPORT bool ARKTS_IsArrayBuffer(ARKTS_Env env, ARKTS_Value value);
EXPORT int32_t ARKTS_GetArrayBufferLength(ARKTS_Env env, ARKTS_Value value);
EXPORT void* ARKTS_GetArrayBufferRawPtr(ARKTS_Env env, ARKTS_Value value);
EXPORT int32_t ARKTS_ArrayBufferReadBytes(ARKTS_Env env, ARKTS_Value buffer, void* dest, int32_t count);

EXPORT ARKTS_Value ARKTS_CreateBigInt(ARKTS_Env env, int64_t value);
EXPORT ARKTS_Value ARKTS_CreateBigIntWithBytes(ARKTS_Env env, bool isNegative, int64_t size, const uint8_t bytes[]);
EXPORT bool ARKTS_IsBigInt(ARKTS_Env env, ARKTS_Value value);
EXPORT int64_t ARKTS_BigIntGetByteSize(ARKTS_Env env, ARKTS_Value value);
EXPORT void ARKTS_BigIntReadBytes(ARKTS_Env env, ARKTS_Value value, bool* isNegative,
                                  int64_t byteCount, uint8_t bytes[]);
// @deprecated
EXPORT void ARKTS_InitEventHandle(ARKTS_Env env);

EXPORT void* ARKTS_GetGlobalNapiEnv(ARKTS_Env env);

DECL_END

#endif //NAPI_ARK_INTEROP_NAPI_H
