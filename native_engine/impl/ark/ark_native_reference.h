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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_ARK_NATIVE_REFERENCE_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_ARK_NATIVE_REFERENCE_H

#include "ark_native_engine.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "native_engine/native_reference.h"
#include "native_engine/native_value.h"

class ArkNativeEngine;

using panda::EcmaVM;
using panda::Global;
using panda::JSValueRef;
using panda::Local;
using panda::LocalScope;

class ArkNativeReference : public NativeReference {
public:
    ArkNativeReference(ArkNativeEngine* engine,
                       const EcmaVM* vm,
                       napi_value value,
                       uint32_t initialRefcount,
                       bool deleteSelf = false,
                       NapiNativeFinalize napiCallback = nullptr,
                       void* data = nullptr,
                       void* hint = nullptr);
    ArkNativeReference(ArkNativeEngine* engine,
                       const EcmaVM* vm,
                       Local<JSValueRef> value,
                       uint32_t initialRefcount,
                       bool deleteSelf,
                       NapiNativeFinalize napiCallback,
                       void* data,
                       void* hint);
    ~ArkNativeReference() override;

    uint32_t Ref() override;
    uint32_t Unref() override;
    napi_value Get() override;
    void* GetData() override;
    operator napi_value() override;
    void SetDeleteSelf() override;
    uint32_t GetRefCount() override;
    bool GetFinalRun() override;
    napi_value GetNapiValue() override;

private:
    inline void ArkNativeReferenceConstructor(uint32_t initialRefCount, bool deleteSelf)
    {
        if (initialRefCount == 0) {
            value_.SetWeakCallback(reinterpret_cast<void*>(this), FreeGlobalCallBack, NativeFinalizeCallBack);
        }

        if (deleteSelf) {
            NativeReferenceManager* referenceManager = engine_->GetReferenceManager();
            if (referenceManager != nullptr) {
                referenceManager->CreateHandler(this);
            }
        }
    }

    ArkNativeEngine* engine_;
    const EcmaVM* vm_;
    Global<JSValueRef> value_;
    uint32_t refCount_ {0};
    bool deleteSelf_ {false};
    NapiNativeFinalize napiCallback_ = nullptr;
    void* data_ = nullptr;
    void* hint_ = nullptr;

    bool hasDelete_ {false};
    bool finalRun_ {false};

    void FinalizeCallback();

    static void FreeGlobalCallBack(void* ref);
    static void NativeFinalizeCallBack(void* ref);
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_ARK_NATIVE_REFERENCE_H */
