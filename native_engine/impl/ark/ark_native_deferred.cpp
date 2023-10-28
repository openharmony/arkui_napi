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

#include "ark_native_deferred.h"

#include <cstring>

#include "ark_native_engine.h"
#ifdef ENABLE_CONTAINER_SCOPE
#include "core/common/container_scope.h"
#endif

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

ArkNativeDeferred::ArkNativeDeferred(ArkNativeEngine* engine, Local<PromiseCapabilityRef> deferred)
    : engine_(engine), deferred_(engine->GetEcmaVm(), deferred)
{
#ifdef ENABLE_CONTAINER_SCOPE
    scopeId_ = OHOS::Ace::ContainerScope::CurrentId();
#endif
}

ArkNativeDeferred::~ArkNativeDeferred()
{
    // Addr of Global stored in ArkNativeDeferred should be released.
    deferred_.FreeGlobalHandleAddr();
}

void ArkNativeDeferred::Resolve(napi_value data)
{
#ifdef ENABLE_CONTAINER_SCOPE
    OHOS::Ace::ContainerScope containerScope(scopeId_);
#endif
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Local<JSValueRef> value = LocalValueFromJsValue(data);
    deferred_->Resolve(vm, value);
}

void ArkNativeDeferred::Reject(napi_value reason)
{
#ifdef ENABLE_CONTAINER_SCOPE
    OHOS::Ace::ContainerScope containerScope(scopeId_);
#endif
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Local<JSValueRef> value = LocalValueFromJsValue(reason);
    deferred_->Reject(vm, value);
}
