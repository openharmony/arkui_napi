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

#include "ark_native_reference.h"

#include "native_engine/native_utils.h"
#include "utils/log.h"

ArkNativeReference::ArkNativeReference(ArkNativeEngine* engine,
                                       const EcmaVM* vm,
                                       napi_value value,
                                       uint32_t initialRefcount,
                                       bool deleteSelf,
                                       NapiNativeFinalize napiCallback,
                                       void* data,
                                       void* hint)
    : engine_(engine),
      vm_(vm),
      value_(vm, LocalValueFromJsValue(value)),
      refCount_(initialRefcount),
      deleteSelf_(deleteSelf),
      napiCallback_(napiCallback),
      data_(data),
      hint_(hint)
{
    ArkNativeReferenceConstructor(initialRefcount, deleteSelf);
}

ArkNativeReference::ArkNativeReference(ArkNativeEngine* engine,
                                       const EcmaVM* vm,
                                       Local<JSValueRef> value,
                                       uint32_t initialRefcount,
                                       bool deleteSelf,
                                       NapiNativeFinalize napiCallback,
                                       void* data,
                                       void* hint)
    : engine_(engine),
      vm_(vm),
      value_(vm, value),
      refCount_(initialRefcount),
      deleteSelf_(deleteSelf),
      napiCallback_(napiCallback),
      data_(data),
      hint_(hint)
{
    ArkNativeReferenceConstructor(initialRefcount, deleteSelf);
}

ArkNativeReference::~ArkNativeReference()
{
    if (deleteSelf_ && engine_->GetReferenceManager()) {
        engine_->GetReferenceManager()->ReleaseHandler(this);
    }
    if (value_.IsEmpty()) {
        return;
    }
    hasDelete_ = true;
    value_.FreeGlobalHandleAddr();
    FinalizeCallback();
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

napi_value ArkNativeReference::Get()
{
    if (value_.IsEmpty()) {
        return nullptr;
    }
    Local<JSValueRef> value = value_.ToLocal(vm_);
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

void ArkNativeReference::FinalizeCallback()
{
    if (napiCallback_ != nullptr) {
        napiCallback_(reinterpret_cast<napi_env>(engine_), data_, hint_);
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
    that->FinalizeCallback();
}

void ArkNativeReference::SetDeleteSelf()
{
    deleteSelf_ = true;
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