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

using EcmaVM = panda::EcmaVM;
using panda::Global;
using JSValueRef = panda::JSValueRef;
using panda::Local;
using LocalScope = panda::LocalScope;

enum class FinalizerState {
    DESTRUCTION,
    COLLECTION,
};

enum class ReferenceOwnerShip : uint8_t {
    RUNTIME,
    USER,
};

struct ArkNativeReferenceConfig {
    uint32_t initialRefcount;
    bool deleteSelf;
    NapiNativeFinalize napiCallback;
    void* data;

    explicit ArkNativeReferenceConfig(uint32_t initialRefcount,
                                      bool deleteSelf = false,
                                      NapiNativeFinalize napiCallback = nullptr,
                                      void* data = nullptr)
        : initialRefcount(initialRefcount),
          deleteSelf(deleteSelf),
          napiCallback(napiCallback),
          data(data)
    {}
};

class ArkNativeReference : public NativeReference {
public:
    ArkNativeReference(ArkNativeEngine* engine,
                       napi_value value,
                       uint32_t initialRefcount,
                       bool deleteSelf = false,
                       NapiNativeFinalize napiCallback = nullptr,
                       void* data = nullptr,
                       void* hint = nullptr,
                       bool isAsyncCall = false,
                       size_t nativeBindingSize = 0);
    ArkNativeReference(ArkNativeEngine* engine,
                       Local<JSValueRef> value,
                       uint32_t initialRefcount,
                       bool deleteSelf,
                       NapiNativeFinalize napiCallback,
                       void* data,
                       void* hint,
                       bool isAsyncCall = false,
                       size_t nativeBindingSize = 0);
    ~ArkNativeReference() override;

    uint32_t Ref() override;
    uint32_t Unref() override;
    napi_value Get() override;
    napi_value Get(NativeEngine* engine) override;
    void* GetData() override;
    operator napi_value() override;
    void SetDeleteSelf() override;
    bool GetDeleteSelf() const override;
    uint32_t GetRefCount() override;
    bool GetFinalRun() override;
    napi_value GetNapiValue() override;
    void ResetFinalizer()  override;
#ifdef PANDA_JS_ETS_HYBRID_MODE
    void MarkFromObject();
    bool IsObjectAlive();
    bool IsValidHeapObject();
#endif // PANDA_JS_ETS_HYBRID_MODE

protected:
    ArkNativeReference(ArkNativeEngine* engine,
                       const ArkNativeReferenceConfig &config)
        : engine_(engine),
          value_(),
          refCount_(config.initialRefcount),
          ownership_(config.deleteSelf ? ReferenceOwnerShip::RUNTIME : ReferenceOwnerShip::USER),
          napiCallback_(config.napiCallback),
          data_(config.data)
    {}

    virtual void ArkNativeReferenceConstructor();

    enum ReferencePropertiesMask : uint8_t {
        DELETE_SELF_MASK = 1,
        IS_ASYNC_CALL_MASK = DELETE_SELF_MASK << 1,
        HAS_DELETE_MASK = IS_ASYNC_CALL_MASK << 1,
        FINAL_RAN_MASK = HAS_DELETE_MASK << 1,
    };

    void InitProperties(bool deleteSelf = false, bool isAsyncCall = false);

    ArkNativeEngine* engine_;
    uint64_t engineId_ {0};

    Global<JSValueRef> value_;
    uint32_t refCount_ {0};

    const ReferenceOwnerShip ownership_;
    // Bit-packed flags: saves memory and speeds up object creation vs. multiple bools.
    // std::bitset will use more memory than uint8_t number.
    uint8_t properties_ {0};

    NapiNativeFinalize napiCallback_ {nullptr};
    void* data_ {nullptr};
    void* hint_ {nullptr};
    size_t nativeBindingSize_ {0};

    NativeReference* prev_ {nullptr};
    NativeReference* next_ {nullptr};

    bool IsAsyncCall() const;
    bool HasDelete() const;
    void SetHasDelete();
    void SetFinalRan();

    void FinalizeCallback(FinalizerState state);
    void DispatchFinalizeCallback();
    void EnqueueAsyncTask();
    void EnqueueDeferredTask();

    void IncreaseCounter();
    void DecreaseCounter();

    static void NativeFinalizeCallBack(void* ref);
private:
    static void FreeGlobalCallBack(void* ref);
    friend class NativeReferenceManager;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_ARK_NATIVE_REFERENCE_H */
