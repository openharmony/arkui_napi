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

#include <cinttypes>

#include "ark_native_reference.h"

#include "native_engine/native_api_internal.h"
#include "native_engine/native_utils.h"

ArkNativeReference::ArkNativeReference(ArkNativeEngine* engine,
                                       napi_value value,
                                       uint32_t initialRefcount,
                                       bool deleteSelf,
                                       NapiNativeFinalize napiCallback,
                                       void* data,
                                       void* hint,
                                       bool isAsyncCall,
                                       size_t nativeBindingSize)
    : engine_(engine),
      value_(engine->GetEcmaVm(), LocalValueFromJsValue(value)),
      refCount_(initialRefcount),
      deleteSelf_(deleteSelf),
      isAsyncCall_(isAsyncCall),
      napiCallback_(napiCallback),
      data_(data),
      hint_(hint),
      nativeBindingSize_(nativeBindingSize)
{
    ArkNativeReferenceConstructor(initialRefcount, deleteSelf);
}

ArkNativeReference::ArkNativeReference(ArkNativeEngine* engine,
                                       Local<JSValueRef> value,
                                       uint32_t initialRefcount,
                                       bool deleteSelf,
                                       NapiNativeFinalize napiCallback,
                                       void* data,
                                       void* hint,
                                       bool isAsyncCall,
                                       size_t nativeBindingSize)
    : engine_(engine),
      value_(engine->GetEcmaVm(), value),
      refCount_(initialRefcount),
      deleteSelf_(deleteSelf),
      isAsyncCall_(isAsyncCall),
      napiCallback_(napiCallback),
      data_(data),
      hint_(hint),
      nativeBindingSize_(nativeBindingSize)
{
    ArkNativeReferenceConstructor(initialRefcount, deleteSelf);
}

ArkNativeReference::~ArkNativeReference()
{
    if (deleteSelf_ && engine_->GetReferenceManager()) {
        engine_->GetReferenceManager()->ReleaseHandler(this);
        prev_ = nullptr;
        next_ = nullptr;
    }
    if (value_.IsEmpty()) {
        return;
    }
    hasDelete_ = true;
    value_.FreeGlobalHandleAddr();
    FinalizeCallback(FinalizerState::DESTRUCTION);
}

uint32_t ArkNativeReference::Ref()
{
    ++refCount_;
    if (refCount_ == 1) {
        value_.ClearWeak();
    }
    return refCount_;
}

uint32_t ArkNativeReference::Unref()
{
    if (refCount_ == 0) {
        return refCount_;
    }
    --refCount_;
    if (value_.IsEmpty()) {
        return refCount_;
    }
    if (refCount_ == 0) {
        value_.SetWeakCallback(reinterpret_cast<void*>(this), FreeGlobalCallBack, NativeFinalizeCallBack);
    }
    return refCount_;
}

napi_value ArkNativeReference::Get(NativeEngine* engine)
{
    if (value_.IsEmpty()) {
        return nullptr;
    }
    if (engine != engine_) {
        LOG_IF_SPECIAL(engine, engine->IsCrossThreadCheckEnabled(), "param env is not equal to its owner");
    } else if (engineId_ != engine->GetId()) {
        LOG_IF_SPECIAL(engine, engine->IsCrossThreadCheckEnabled(),
                       "owner env has been destroyed, "
                       "ownerid: %{public}" PRIu64 ", current env id: %{public}" PRIu64,
                       engineId_, engine_->GetId());
    }
    Local<JSValueRef> value = value_.ToLocal(engine->GetEcmaVm());
    return JsValueFromLocalValue(value);
}

napi_value ArkNativeReference::Get()
{
    if (value_.IsEmpty()) {
        return nullptr;
    }
    if (engineId_ != engine_->GetId()) {
        LOG_IF_SPECIAL(engine_, engine_->IsCrossThreadCheckEnabled(),
                       "owner env has been destroyed, "
                       "ownerid: %{public}" PRIu64 ", current env id: %{public}" PRIu64,
                       engineId_, engine_->GetId());
    }
    Local<JSValueRef> value = value_.ToLocal(engine_->GetEcmaVm());
    return JsValueFromLocalValue(value);
}

ArkNativeReference::operator napi_value()
{
    return Get();
}

void* ArkNativeReference::GetData()
{
    return data_;
}

void ArkNativeReference::FinalizeCallback(FinalizerState state)
{
    if (napiCallback_ != nullptr && !engine_->IsInDestructor()) {
        if (state == FinalizerState::COLLECTION) {
            std::tuple<NativeEngine*, void*, void*> tuple = std::make_tuple(engine_, data_, hint_);
            RefFinalizer finalizer = std::make_pair(napiCallback_, tuple);
            if (isAsyncCall_) {
                engine_->GetPendingAsyncFinalizers().emplace_back(finalizer);
            } else {
                engine_->GetArkFinalizersPack().AddFinalizer(finalizer, nativeBindingSize_);
            }
        } else {
            napiCallback_(reinterpret_cast<napi_env>(engine_), data_, hint_);
        }
    }
    napiCallback_ = nullptr;
    data_ = nullptr;
    hint_ = nullptr;
    finalRun_ = true;

    if (deleteSelf_ && !hasDelete_) {
        delete this;
    }
}

void ArkNativeReference::FreeGlobalCallBack(void* ref)
{
    auto that = reinterpret_cast<ArkNativeReference*>(ref);
    that->value_.FreeGlobalHandleAddr();
}

void ArkNativeReference::NativeFinalizeCallBack(void* ref)
{
    auto that = reinterpret_cast<ArkNativeReference*>(ref);
    that->FinalizeCallback(FinalizerState::COLLECTION);
}

void ArkNativeReference::SetDeleteSelf()
{
    deleteSelf_ = true;
}

bool ArkNativeReference::GetDeleteSelf() const
{
    return deleteSelf_;
}

uint32_t ArkNativeReference::GetRefCount()
{
    return refCount_;
}

bool ArkNativeReference::GetFinalRun()
{
    return finalRun_;
}

napi_value ArkNativeReference::GetNapiValue()
{
    return Get();
}

void ArkNativeReference::ResetFinalizer()
{
    napiCallback_ = nullptr;
    data_ = nullptr;
    hint_ = nullptr;
}