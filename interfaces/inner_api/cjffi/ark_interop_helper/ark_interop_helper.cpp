/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "ark_interop_helper.h"
#include "ark_native_engine.h"
#include "ark_interop_internal.h"
#include "ark_interop_log.h"
#ifndef PREVIEW
#include "core/common/container_scope.h"
#include "custom_scope.h"
#include "napi_base_context.h"
#include "ability.h"
#endif

using namespace panda::ecmascript;

extern "C" {
static napi_env g_globalEnv = nullptr;

napi_env GetGlobalNapiEnv()
{
    if (!g_globalEnv) {
        LOGE("use global env before set");
    }
    return g_globalEnv;
}

void SetGlobalNapiEnv(napi_env env)
{
    if (!g_globalEnv) {
        g_globalEnv = env;
    }
}

napi_value ArkTsValuetoNapiValue(napi_env env, ARKTS_Value arkValue)
{
    LOGI("ArkTsValuetoNapiValue start");
    if (env == nullptr) {
        LOGE("FfiOHOSArkTsValuetoNapiValue Error: env is null!");
        return nullptr;
    }
    Local<JSValueRef> js_value_ref = ARKTS_ToHandle<JSValueRef>(arkValue);
    auto ark_native_obj = ArkNativeEngine::ArkValueToNapiValue(env, js_value_ref);
    return ark_native_obj;
};

ARKTS_Value NapiValueToArkTsValue(napi_value value)
{
    auto ref = BIT_CAST(value, Local<JSValueRef>);
    return ARKTS_FromHandle(ref);
}

bool IsStageMode(napi_env env, napi_value context)
{
#ifndef PREVIEW
    LOGI("IsStageMode start");
    bool isStageMode = false;
    napi_status status = OHOS::AbilityRuntime::IsStageContext(env, context, isStageMode);
    if (status != napi_ok || !isStageMode) {
        LOGI("IsStageMode false");
        return false;
    } else {
        LOGI("IsStageMode true");
        return true;
    }
#else
    return false;
#endif
}

void* GetContextStageMode(napi_env env, napi_value context)
{
#ifndef PREVIEW
    LOGI("GetContextStageMode start");
    if (!env || !context) {
        return nullptr;
    }
    napi_valuetype type;
    if (napi_typeof(env, context, &type) != napi_ok) {
        return nullptr;
    }
    if (type != napi_object) {
        return nullptr;
    }
    void* data;
    if (napi_unwrap(env, context, &data) != napi_ok) {
        return nullptr;
    }
    if (!data) {
        return nullptr;
    }
    auto ability = OHOS::AbilityRuntime::GetStageModeContext(env, context);
    if (ability == nullptr) {
        LOGE("Failed to get native ability instance");
        return nullptr;
    }
    LOGI("GetContextStageMode success");
    return ability.get();
#else
    return nullptr;
#endif
}
#ifndef PREVIEW
void CustomScope::Enter()
{
    last_ = OHOS::Ace::ContainerScope::CurrentId();
    OHOS::Ace::ContainerScope::UpdateCurrent(id_);
}

void CustomScope::Exit() const
{
    OHOS::Ace::ContainerScope::UpdateCurrent(last_);
}

int32_t ARKTS_GetCurrentContainerId()
{
    return OHOS::Ace::ContainerScope::CurrentId();
}

ContainerScope ARKTS_CreateContainerScope(int32_t id)
{
    return new CustomScope(id);
}

void ARKTS_DestroyContainerScope(ContainerScope scope)
{
    delete scope;
}

void ARKTS_EnterContainerScope(ContainerScope scope)
{
    scope->Enter();
}

void ARKTS_ExitContainerScope(ContainerScope scope)
{
    scope->Exit();
}
#else
int32_t ARKTS_GetCurrentContainerId() { return 0; }
ContainerScope ARKTS_CreateContainerScope(int32_t id) { return nullptr; }
void ARKTS_DestroyContainerScope(ContainerScope scope) {}
void ARKTS_EnterContainerScope(ContainerScope scope) {}
void ARKTS_ExitContainerScope(ContainerScope scope) {}
#endif
}