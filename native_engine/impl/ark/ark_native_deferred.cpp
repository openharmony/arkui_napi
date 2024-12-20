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


#include "ark_native_engine.h"
#ifdef ENABLE_CONTAINER_SCOPE
#include "core/common/container_scope.h"
#endif

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
    deferred_->Resolve(vm, reinterpret_cast<uintptr_t>(data));
}

void ArkNativeDeferred::Reject(napi_value reason)
{
#ifdef ENABLE_CONTAINER_SCOPE
    OHOS::Ace::ContainerScope containerScope(scopeId_);
#endif
    auto vm = engine_->GetEcmaVm();
    deferred_->Reject(vm, reinterpret_cast<uintptr_t>(reason));
}
