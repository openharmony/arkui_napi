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

#include "ark_xref_native_reference.h"
#include <cinttypes>

#include "native_engine/native_api_internal.h"
#include "native_engine/native_utils.h"

ArkXRefNativeReference::ArkXRefNativeReference(ArkNativeEngine* engine,
                                               napi_value value,
                                               const ArkNativeReferenceConfig &config)
    : ArkNativeReference(engine, config)
{
    value_.CreateXRefGloablReference(engine->GetEcmaVm(), LocalValueFromJsValue(value));
    ArkNativeReferenceConstructor(FreeXRefGlobalCallBack);
}

ArkXRefNativeReference::~ArkXRefNativeReference()
{
    VALID_ENGINE_CHECK(engine_, engine_, engineId_);

    if (engine_->GetReferenceManager()) {
        engine_->GetReferenceManager()->ReleaseHandler(this);
        prev_ = nullptr;
        next_ = nullptr;
    }
    if (value_.IsEmpty()) {
        return;
    }
    value_.FreeXRefGlobalHandleAddr();
    FinalizeCallback(FinalizerState::DESTRUCTION);
}

void ArkXRefNativeReference::FreeXRefGlobalCallBack(void* ref)
{
    auto that = reinterpret_cast<ArkXRefNativeReference*>(ref);
    that->value_.FreeXRefGlobalHandleAddr();
}

uint32_t ArkXRefNativeReference::Unref()
{
    if (refCount_ == 0) {
        return refCount_;
    }
    --refCount_;
    if (value_.IsEmpty()) {
        return refCount_;
    }
    if (refCount_ == 0) {
        value_.SetWeakCallback(reinterpret_cast<void*>(this), FreeXRefGlobalCallBack, NativeFinalizeCallBack);
    }
    return refCount_;
}
 