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

#include "native_engine/native_reference.h"
#include "native_engine/native_engine.h"
#include "ecmascript/napi/include/jsnapi.h"

#ifdef ENABLE_CONTAINER_SCOPE
#include "core/common/container_scope.h"
#endif

#include "utils/log.h"

using panda::LocalScope;
using panda::Global;

NativeReference::NativeReference(NativeEngine* engine,
                                       Local<JSValueRef> value,
                                       uint32_t initialRefcount,
                                       bool deleteSelf,
                                       NativeFinalize callback,
                                       void* data,
                                       void* hint)
    : NativeReference::NativeReference(engine, value, initialRefcount, deleteSelf, callback, nullptr, data, hint)
{}

NativeReference::NativeReference(NativeEngine* engine,
                                       Local<JSValueRef> value,
                                       uint32_t initialRefcount,
                                       bool deleteSelf,
                                       NativeFinalize callback,
                                       NapiNativeFinalize napiCallback,
                                       void* data,
                                       void* hint)
    : engine_(engine),
      value_(Global<JSValueRef>(engine->GetEcmaVm(), JSValueRef::Undefined(engine->GetEcmaVm()))),
      refCount_(initialRefcount),
      deleteSelf_(deleteSelf),
      callback_(callback),
      napiCallback_(napiCallback),
      data_(data),
      hint_(hint)
{
    Global<JSValueRef> oldValue(engine->GetEcmaVm(), value);
    auto vm = engine->GetEcmaVm();
    LocalScope scope(vm);
    Global<JSValueRef> newValue(vm, oldValue.ToLocal(vm));
    value_ = newValue;
    if (initialRefcount == 0) {
        newValue.SetWeakCallback(reinterpret_cast<void*>(this), FreeGlobalCallBack, NativeFinalizeCallBack);
    }

#ifdef ENABLE_CONTAINER_SCOPE
    scopeId_ = OHOS::Ace::ContainerScope::CurrentId();
#endif

    if (deleteSelf) {
        NativeReferenceManager* referenceManager = engine->GetReferenceManager();
        if (referenceManager != nullptr) {
            referenceManager->CreateHandler(this);
        }
    }
}

NativeReference::~NativeReference()
{
    if (deleteSelf_ && engine_->GetReferenceManager()) {
        engine_->GetReferenceManager()->ReleaseHandler(this);
    }
    Global<JSValueRef> Value = value_;
    if (Value.IsEmpty()) {
        return;
    }
    hasDelete_ = true;
    Value.FreeGlobalHandleAddr();
    FinalizeCallback();
}

uint32_t NativeReference::Ref()
{
    ++refCount_;
    if (refCount_ == 1) {
        Global<JSValueRef> Value = value_;
        Value.ClearWeak();
    }
    return refCount_;
}

uint32_t NativeReference::Unref()
{
    if (refCount_ == 0) {
        return refCount_;
    }
    --refCount_;
    Global<JSValueRef> Value = value_;
    if (Value.IsEmpty()) {
        return refCount_;
    }
    if (refCount_ == 0) {
        Value.SetWeakCallback(reinterpret_cast<void*>(this), FreeGlobalCallBack, NativeFinalizeCallBack);
    }
    return refCount_;
}

NativeValue* NativeReference::Get()
{
    Global<JSValueRef> Value = value_;
    if (Value.IsEmpty()) {
        return nullptr;
    }
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
#ifdef ENABLE_CONTAINER_SCOPE
    OHOS::Ace::ContainerScope containerScope(scopeId_);
#endif
    return engine_->ValueToNativeValue(value_);
}

NativeReference::operator NativeValue*()
{
    return Get();
}

void* NativeReference::GetData()
{
    return data_;
}

void NativeReference::FinalizeCallback()
{
    if (callback_ != nullptr) {
        callback_(engine_, data_, hint_);
    }
    if (napiCallback_ != nullptr) {
        napiCallback_(reinterpret_cast<napi_env>(engine_), data_, hint_);
    }
    callback_ = nullptr;
    napiCallback_ = nullptr;
    data_ = nullptr;
    hint_ = nullptr;
    finalRun_ = true;

    if (deleteSelf_ && !hasDelete_) {
        delete this;
    }
}

void NativeReference::FreeGlobalCallBack(void* ref)
{
    auto that = reinterpret_cast<NativeReference*>(ref);
    Global<JSValueRef> Value = that->value_;
    Value.FreeGlobalHandleAddr();
}

void NativeReference::NativeFinalizeCallBack(void* ref)
{
    auto that = reinterpret_cast<NativeReference*>(ref);
    that->FinalizeCallback();
}

void NativeReference::SetDeleteSelf()
{
    deleteSelf_ = true;
}

uint32_t NativeReference::GetRefCount()
{
    return refCount_;
}

bool NativeReference::GetFinalRun()
{
    return finalRun_;
}

napi_value NativeReference::GetNapiValue()
{
    NativeValue* result = Get();
    return reinterpret_cast<napi_value>(result);
}