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

#ifndef NAPI_ARK_INTEROP_INTERNAL_H
#define NAPI_ARK_INTEROP_INTERNAL_H

#include <string>
#include <native_engine/native_engine.h>

#include "ark_interop_macro.h"
#include "ark_interop_napi.h"
#include "ecmascript/napi/include/jsnapi.h"

using EcmaVM = panda::EcmaVM;

// below interface shares internal, never export
typedef void (*ARKTS_AsyncCallback)(ARKTS_Env env, void*);

bool ARKTSInner_ReportJSErrors(ARKTS_Env env, bool abortIfUnhandled);
void ARKTSInner_ReportNativeError(const char* format, ...) __attribute__((format(printf, 1, 2)));
COV_EXPORT std::string ARKTSInner_FormatJSError(ARKTS_Env env, ARKTS_Value jsError);
bool ARKTSInner_ThrowJSErrorToCJ(ARKTS_Env env, ARKTS_Value error);
bool ARKTSInner_ThrowNativeErrorToCJ(const char* error);
void ARKTSInner_CreateAsyncTask(ARKTS_Env env, ARKTS_AsyncCallback callback, void* data);
// cangjie callbacks
void ARKTSInner_CJArrayBufferDeleter(void*, void* buffer, void* lambdaId);
void ARKTSInner_CJExternalDeleter(void*, void* data, void* env);
ARKTS_Result ARKTSInner_CJLambdaInvoker(ARKTS_CallInfo callInfo, int64_t lambdaId);
void ARKTSInner_CJLambdaDeleter(ARKTS_Env env, int64_t lambdaId);
COV_EXPORT void ARKTSInner_CJAsyncCallback(ARKTS_Env env, void* data);

panda::JSValueRef* ARKTSInner_Escape(ARKTS_Env env, ARKTS_Scope scope, ARKTS_Value ret);

#ifdef __OHOS__
#include <memory>
#include "event_handler.h"

using ARKTS_Loop = std::shared_ptr<OHOS::AppExecFwk::EventHandler>;
#else
using ARKTS_Loop = void*;
#endif

// @deprecated
ARKTS_Loop ARKTS_GetOrCreateEventHandler(ARKTS_Env env);
bool ARKTSInner_InitLoop(ARKTS_Env env, uv_loop_t* loop);
void ARKTS_DisposeEventHandler(ARKTS_Env env);
void ARKTS_DisposeJSContext(ARKTS_Env env);

template<typename T>
ARKTS_INLINE panda::Local<T> ARKTS_ToHandle(ARKTS_Value& value)
{
    auto v = BIT_CAST(value, panda::JSValueRef);
    void* addr;
    if (v.IsHeapObject()) {
        addr = value;
    } else {
        addr = &value;
    }
    return BIT_CAST(addr, panda::Local<T>);
}

ARKTS_INLINE panda::JSValueRef ARKTS_ToValue(ARKTS_Value value)
{
    auto v = BIT_CAST(value, panda::JSValueRef);
    if (v.IsHeapObject()) {
        return *BIT_CAST(value, panda::JSValueRef*);
    } else {
        return v;
    }
}

template<typename T>
ARKTS_INLINE ARKTS_Value ARKTS_FromHandle(panda::Local<T>& value)
{
    if (value.IsNull()) {
        return ARKTS_CreateUndefined();
    } else if (value->IsHeapObject()) {
        return BIT_CAST(value, ARKTS_Value);
    } else {
        return *BIT_CAST(value, ARKTS_Value*);
    }
}

ARKTS_INLINE ARKTS_Result ARKTS_ToResult(panda::EcmaVM* vm, ARKTS_Value value)
{
    auto tag = BIT_CAST(value, panda::JSValueRef);
    if (!tag.IsHeapObject()) {
        auto local = panda::JSNApi::CreateLocal(vm, tag);
        return BIT_CAST(local, ARKTS_Result);
    } else {
        return BIT_CAST(value, ARKTS_Result);
    }
}

#define NATIVE_ERROR(msg) ARKTSInner_ReportNativeError("[%s]: %s", __FUNCTION__, msg)
#define ARKTS_ASSERT(condition, error, ret)                                            \
    if (!(condition)) {                                                   \
        NATIVE_ERROR(error);                                                        \
        return ret;                                                                 \
    }
#define ARKTS_ASSERT_P(condition, error) ARKTS_ASSERT(condition, error, nullptr)
#define ARKTS_ASSERT_F(condition, error) ARKTS_ASSERT(condition, error, false)
#define ARKTS_ASSERT_I(condition, error) ARKTS_ASSERT(condition, error, 0)
#define ARKTS_ASSERT_V(condition, error) ARKTS_ASSERT(condition, error,)

#ifdef DEBUG_JS
#define ARKTS_CHECK(condition, ret)                                                    \
    if (UNLIKELY(!(condition))) {                                                   \
        return ret;                                                                 \
    }
#else
#define ARKTS_CHECK(condition, ret) condition
#endif

#define ARKTS_CHECK_F(condition) ARKTS_CHECK(condition, false)

#endif // NAPI_ARK_INTEROP_INTERNAL_H
