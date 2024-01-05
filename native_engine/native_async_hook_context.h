/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_ASYNC_HOOK_CONTEXT_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_ASYNC_HOOK_CONTEXT_H

#include "native_value.h"

class NativeAsyncWrap {
public:
    static void EmitAsyncInit(NativeEngine* env,
                              panda::Local<panda::ObjectRef> object,
                              panda::Local<panda::StringRef> type,
                              double async_id,
                              double trigger_async_id) {}

    static void EmitDestroy(NativeEngine* env, double async_id) {}
};

class NativeAsyncHookContext {
public:
    NativeAsyncHookContext(NativeEngine* env,
                           panda::Local<panda::ObjectRef> resourceObject,
                           const panda::Local<panda::StringRef> resourceName,
                           bool isExternalResource) : env_(env), resource_(env->GetEcmaVm(), resourceObject)
    {
        asyncId_ = env->NewAsyncId();
        triggerAsyncId_ = env->GetDefaultTriggerAsyncId();
        lostReference_ = false;
        if (isExternalResource) {
            resource_.SetWeak();
            resource_.SetWeakCallback(reinterpret_cast<void*>(this), FreeGlobalCallBack, NativeFinalizeCallBack);
        }

        NativeAsyncWrap::EmitAsyncInit(env,
                                       resourceObject,
                                       resourceName,
                                       asyncId_,
                                       triggerAsyncId_);
    }

    ~NativeAsyncHookContext()
    {
        resource_.FreeGlobalHandleAddr();
        lostReference_ = true;
        NativeAsyncWrap::EmitDestroy(env_, asyncId_);
    }

    static void FreeGlobalCallBack(void* ref)
    {
        auto that = reinterpret_cast<NativeAsyncHookContext*>(ref);
        that->resource_.FreeGlobalHandleAddr();
        that->lostReference_ = true;
    }

    static void NativeFinalizeCallBack(void* ref) {}

    inline NativeCallbackScope* OpenCallbackScope()
    {
        EnsureReference();
        auto callbackScopeManager = env_->GetCallbackScopeManager();
        if (callbackScopeManager == nullptr) {
            return nullptr;
        }

        auto callbackScope = callbackScopeManager->Open(env_);
        callbackScopeManager->IncrementOpenCallbackScopes();

        return callbackScope;
    }

    inline void EnsureReference()
    {
        if (lostReference_) {
            auto ecmaVm = env_->GetEcmaVm();
            panda::Global<panda::ObjectRef> resource(ecmaVm, panda::ObjectRef::New(ecmaVm));
            resource_ = resource;
            lostReference_ = false;
        }
    }

    static void CloseCallbackScope(NativeEngine* env, NativeCallbackScope* scope)
    {
        auto callbackScopeManager = env->GetCallbackScopeManager();
        if (callbackScopeManager == nullptr) {
            return;
        }
        if (callbackScopeManager->GetOpenCallbackScopes() > 0) {
            callbackScopeManager->DecrementOpenCallbackScopes();
            callbackScopeManager->Close(scope);
        }
    }

private:
    NativeEngine* env_ {nullptr};
    double asyncId_ = 0;
    double triggerAsyncId_ = 0;
    panda::Global<panda::ObjectRef> resource_;
    bool lostReference_ = false;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_ASYNC_HOOK_CONTEXT_H */
