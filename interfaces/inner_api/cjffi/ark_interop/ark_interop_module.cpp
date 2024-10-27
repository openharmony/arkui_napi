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

#include <unordered_map>

#include "ark_interop_internal.h"
#include "ark_interop_external.h"
#include "ark_interop_log.h"
#include "cj_envsetup.h"

struct ARKTS_ModuleCallbacks {
    ARKTS_Value (*exportModule)(ARKTS_Env env, const char* dllName, ARKTS_Value exports) = nullptr;
    bool (*hasModuleHandle)(const char* dllName) = nullptr;
    void (*throwJSError)(ARKTS_Env env, ARKTS_Value) = nullptr;
    void (*throwNativeError)(const char*) = nullptr;
    void (*deleteArrayBufferRawData)(void* buffer, int64_t lambdaId) = nullptr;
    void (*deleteExternal)(int64_t id, ARKTS_Env env) = nullptr;
    ARKTS_Value (*invokerLambda)(ARKTS_CallInfo, int64_t lambdaId) = nullptr;
    void (*deleteLambda)(ARKTS_Env env, int64_t lambdaId) = nullptr;
    void (*invokeAsyncLambda)(ARKTS_Env env, int64_t lambdaId) = nullptr;
    void (*deleteJSContext)(ARKTS_Env env) = nullptr;
};

namespace {
// each native register only be available during module loading
ARKTS_ModuleCallbacks* g_cjModuleCallbacks = nullptr;
}

bool ARKTSInner_ThrowJSErrorToCJ(ARKTS_Env env, ARKTS_Value error)
{
    if (!g_cjModuleCallbacks) {
        LOGE("napi module depends on another napi module is forbidden");
        return false;
    }
    g_cjModuleCallbacks->throwJSError(env, error);
    return true;
}

bool ARKTSInner_ThrowNativeErrorToCJ(const char* error)
{
    if (!g_cjModuleCallbacks) {
        LOGE("napi module depends on another napi module is forbidden");
        return false;
    }
    g_cjModuleCallbacks->throwNativeError(error);
    return true;
}

void ARKTS_SetCJModuleCallback(ARKTS_ModuleCallbacks* callback)
{
    if (!callback) {
        LOGE("register empty module callback is senseless");
        return;
    }
    if (g_cjModuleCallbacks) {
        LOGE("should never happen");
        return;
    }
    g_cjModuleCallbacks = new ARKTS_ModuleCallbacks(*callback);
}

// export but only for internal
panda::JSValueRef* ARKTS_LoadModule(ARKTS_Env env, const char* dllName)
{
    LOGE("ARKTS_LoadCJModule start: %{public}s", dllName);
    // HandleScope
    auto scope = ARKTS_OpenScope(env);

    auto undefined = ARKTS_CreateUndefined();
    auto runtime = OHOS::CJEnv::LoadInstance();
    if (!runtime) {
        return ARKTSInner_Escape(env, scope, undefined);
    }

    if (!runtime->startRuntime()) {
        LOGE("start cj runtime failed");
        return ARKTSInner_Escape(env, scope, undefined);
    }
    if (!g_cjModuleCallbacks || !g_cjModuleCallbacks->hasModuleHandle(dllName)) {
        if (!runtime->loadCJModule(dllName)) {
            LOGE("load cj library failed, try load as native");
            return ARKTSInner_Escape(env, scope, undefined);
        }
    }

    if (!g_cjModuleCallbacks) {
        LOGE("cj module must dependent on ohos_ark_interop");
        return ARKTSInner_Escape(env, scope, undefined);
    }

    auto exports = ARKTS_CreateObject(env);

    exports = g_cjModuleCallbacks->exportModule(env, dllName, exports);

    return ARKTSInner_Escape(env, scope, exports);
}

EXPORT panda::JSValueRef* ARKTS_LoadModuleByNapiEnv(void* env, const char* dllName)
{
    if (!env) {
        LOGE("env is null");
        return nullptr;
    }
    if (!dllName) {
        LOGE("dllName is null");
        return nullptr;
    }
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());
    if (!vm) {
        LOGE("vm is null");
        return nullptr;
    }
    auto loop = engine->GetUVLoop();
    if (!loop) {
        LOGE("uvloop is null");
        return nullptr;
    }
    if (!ARKTSInner_InitLoop(reinterpret_cast<ARKTS_Env>(vm), loop)) {
        LOGE("init loop failed");
        return nullptr;
    }
    return ARKTS_LoadModule(reinterpret_cast<ARKTS_Env>(vm), dllName);
}

void ARKTSInner_CJArrayBufferDeleter(void*, void* buffer, void* lambdaId)
{
    if (!g_cjModuleCallbacks) {
        LOGE("native ark-interop library has no registered module");
        return;
    }
    g_cjModuleCallbacks->deleteArrayBufferRawData(buffer,
        reinterpret_cast<int64_t>(lambdaId));
}

void ARKTSInner_CJExternalDeleter(void*, void* data, void* env)
{
    if (!g_cjModuleCallbacks) {
        LOGE("native ark-interop library has no registered module");
        return;
    }
    g_cjModuleCallbacks->deleteExternal(
        reinterpret_cast<int64_t>(data),
        reinterpret_cast<ARKTS_Env>(env)
    );
}

ARKTS_Result ARKTSInner_CJLambdaInvoker(ARKTS_CallInfo callInfo, int64_t lambdaId)
{
    auto env = ARKTS_GetCallEnv(callInfo);
    auto scope = ARKTS_OpenScope(env);

    if (!g_cjModuleCallbacks) {
        LOGE("native ark-interop library has no registered module");
        return ARKTS_Return(env, scope, ARKTS_CreateUndefined());
    }

    auto result = g_cjModuleCallbacks->invokerLambda(callInfo, lambdaId);
    return ARKTS_Return(env, scope, result);
}

void ARKTSInner_CJLambdaDeleter(ARKTS_Env env, int64_t lambdaId)
{
    if (!g_cjModuleCallbacks) {
        LOGE("native ark-interop library has no registered module");
        return;
    }

    g_cjModuleCallbacks->deleteLambda(env, lambdaId);
}

void ARKTSInner_CJAsyncCallback(ARKTS_Env env, void* data)
{
    if (!g_cjModuleCallbacks) {
        LOGE("native ark-interop library has no registered module");
        return;
    }

    g_cjModuleCallbacks->invokeAsyncLambda(env, reinterpret_cast<int64_t>(data));
}

void ARKTS_DisposeJSContext(ARKTS_Env env)
{
    if (!g_cjModuleCallbacks) {
        LOGE("native ark-interop library has no registered module");
        return;
    }
    g_cjModuleCallbacks->deleteJSContext(env);
}
