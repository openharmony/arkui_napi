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
      ownership_(deleteSelf ? ReferenceOwnerShip::RUNTIME : ReferenceOwnerShip::USER),
      value_(engine->GetEcmaVm(), LocalValueFromJsValue(value)),
      refCount_(initialRefcount),
      deleteSelf_(deleteSelf),
      isAsyncCall_(isAsyncCall),
      napiCallback_(napiCallback),
      data_(data),
      hint_(hint),
      nativeBindingSize_(nativeBindingSize)
{
    ArkNativeReferenceConstructor();
}

ArkNativeReference::ArkNativeReference(ArkNativeEngine* engine,
                                       napi_value value,
                                       ArkNativeReferenceConfig& config)
    : engine_(engine),
      ownership_(config.deleteSelf ? ReferenceOwnerShip::RUNTIME : ReferenceOwnerShip::USER),
      value_(),
      refCount_(config.initialRefcount),
      isProxyReference_(config.isProxyReference),
      deleteSelf_(config.deleteSelf),
      napiCallback_(config.napiCallback),
      data_(config.data)
{
    value_.CreateXRefGloablReference(engine->GetEcmaVm(), LocalValueFromJsValue(value));
    ArkNativeReferenceConstructor();
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
      ownership_(deleteSelf ? ReferenceOwnerShip::RUNTIME : ReferenceOwnerShip::USER),
      value_(engine->GetEcmaVm(), value),
      refCount_(initialRefcount),
      deleteSelf_(deleteSelf),
      isAsyncCall_(isAsyncCall),
      napiCallback_(napiCallback),
      data_(data),
      hint_(hint),
      nativeBindingSize_(nativeBindingSize)
{
    ArkNativeReferenceConstructor();
}

void ArkNativeReference::ArkNativeReferenceConstructor()
{
    if (napiCallback_ != nullptr) {
        // Async callback will redirect to root engine, no monitoring needed.
        if (!isAsyncCall_) {
            engine_->IncreaseCallbackbleRefCounter();
            // Non-callback runtime owned napi_ref will free when env teardown.
            if (ownership_ == ReferenceOwnerShip::RUNTIME && !engine_->IsMainEnvContext()) {
                engine_->IncreaseRuntimeOwnedRefCounter();
            }
        }
    } else {
        engine_->IncreaseNonCallbackRefCounter();
    }

    if (refCount_ == 0) {
        value_.SetWeakCallback(reinterpret_cast<void*>(this), FreeGlobalCallBack, NativeFinalizeCallBack);
    }

    if (ownership_ == ReferenceOwnerShip::RUNTIME) {
        NativeReferenceManager* referenceManager = engine_->GetReferenceManager();
        if (referenceManager != nullptr) {
            referenceManager->CreateHandler(this);
        }
    }

    engineId_ = engine_->GetId();
}

ArkNativeReference::~ArkNativeReference()
{
    VALID_ENGINE_CHECK(engine_, engine_, engineId_);

    if (!napiCallback_) {
        engine_->DecreaseNonCallbackRefCounter();
    }

    NativeReferenceManager* refMgr = engine_->GetReferenceManager();
    if (ownership_ == ReferenceOwnerShip::RUNTIME && refMgr != nullptr) {
        refMgr->ReleaseHandler(this);
        prev_ = nullptr;
        next_ = nullptr;
    }
    if (value_.IsEmpty()) {
        return;
    }
    hasDelete_ = true;
    if (isProxyReference_) {
        value_.FreeXRefGlobalHandleAddr();
    } else {
        value_.FreeGlobalHandleAddr();
    }
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
    VALID_ENGINE_CHECK(engine, engine_, engineId_);
    Local<JSValueRef> value = value_.ToLocal(engine->GetEcmaVm());
    return JsValueFromLocalValue(value);
}

napi_value ArkNativeReference::Get()
{
    return Get(engine_);
}

ArkNativeReference::operator napi_value()
{
    return Get(engine_);
}

void* ArkNativeReference::GetData()
{
    return data_;
}

struct NapiSecondCallback {
    NapiNativeFinalize callback_;
    void* data_;
    ReferenceOwnerShip ownership_;
    void Invoke(napi_env env, void* hint)
    {
        callback_(env, data_, hint);
    }
    // proxy of user callback
    static void SecondFinalizeCallback(napi_env env, void* data, void* hint)
    {
        NapiSecondCallback* callbackWrap = reinterpret_cast<NapiSecondCallback*>(data);
        callbackWrap->Invoke(env, hint);
        ArkNativeEngine* engine = reinterpret_cast<ArkNativeEngine*>(env);
        engine->DecreaseCallbackbleRefCounter();

        if (callbackWrap->ownership_ == ReferenceOwnerShip::RUNTIME) {
            engine->DecreaseRuntimeOwnedRefCounter();
        }

        delete callbackWrap;
        if (engine->IsReadyToDelete()) {
            engine->Delete();
        }
    }
};

void ArkNativeReference::EnqueueAsyncTask()
{
    std::pair<void*, void*> pair = std::make_pair(data_, hint_);
    RefAsyncFinalizer asyncFinalizer = std::make_pair(napiCallback_, pair);
    // Async callback doesn't require the current engine. Use the root engine if a context is passed.
    if (engine_->IsMainEnvContext()) {
        engine_->GetPendingAsyncFinalizers().emplace_back(asyncFinalizer);
    } else {
        const_cast<ArkNativeEngine*>(engine_->GetParent())
            ->GetPendingAsyncFinalizers()
            .emplace_back(asyncFinalizer);
    }
}

void ArkNativeReference::EnqueueDeferredTask()
{
    if (engine_->IsMainEnvContext()) {
        std::tuple<NativeEngine*, void*, void*> tuple = std::make_tuple(engine_, data_, hint_);
        RefFinalizer finalizer = std::make_pair(napiCallback_, tuple);
        engine_->GetArkFinalizersPack().AddFinalizer(finalizer, nativeBindingSize_);
    } else {
        std::tuple<NativeEngine*, void*, void*> tuple =
            std::make_tuple(engine_, new NapiSecondCallback { napiCallback_, data_, ownership_ }, hint_);
        RefFinalizer finalizer = std::make_pair(NapiSecondCallback::SecondFinalizeCallback, tuple);
        // callbackble ref counter will decrease until callback is invoked
        const_cast<ArkNativeEngine*>(engine_->GetParent())
            ->GetArkFinalizersPack()
            .AddFinalizer(finalizer, nativeBindingSize_);
    }
}

void ArkNativeReference::DispatchFinalizeCallback()
{
    if (isAsyncCall_) {
        EnqueueAsyncTask();
    } else {
        EnqueueDeferredTask();
    }
}

void ArkNativeReference::FinalizeCallback(FinalizerState state)
{
    // Invoke the callback only if it is callbackble and has not already been invoked.
    if (!finalRun_ && napiCallback_ && !engine_->IsInDestructor()) {
        if (state == FinalizerState::COLLECTION) {
            DispatchFinalizeCallback();
        } else {
            if (!isAsyncCall_) {
                engine_->DecreaseCallbackbleRefCounter();
            }
            if (ownership_ == ReferenceOwnerShip::RUNTIME) {
                engine_->DecreaseRuntimeOwnedRefCounter();
            }
            napiCallback_(reinterpret_cast<napi_env>(engine_), data_, hint_);
        }
    }
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
    if (that->isProxyReference_) {
        that->value_.FreeXRefGlobalHandleAddr();
    } else {
        that->value_.FreeGlobalHandleAddr();
    }
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

#ifdef PANDA_JS_ETS_HYBRID_MODE
void ArkNativeReference::MarkFromObject()
{
    value_.MarkFromObject();
}

bool ArkNativeReference::IsObjectAlive()
{
    return value_.IsObjectAlive();
}

bool ArkNativeReference::IsValidHeapObject()
{
    return value_.IsValidHeapObject();
}
#endif // PANDA_JS_ETS_HYBRID_MODE
