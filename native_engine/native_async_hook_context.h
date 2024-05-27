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

#include "callback_scope_manager/native_callback_scope_manager.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "native_value.h"
#include "native_engine.h"
#include "utils/log.h"

class NativeAsyncHookContext;
static panda::JSValueRef* InternalMakeCallback(NativeEngine* engine, panda::FunctionRef* funRef,
                                               panda::JSValueRef* obj, panda::JSValueRef *const argv[],
                                               size_t argc, NativeAsyncHookContext* asyncContext);

enum CallbackScopeCode {
    CALLBACK_SCOPE_OK = 0,
    CALLBACK_SCOPE_INVALID_ARG,
    CALLBACK_SCOPE_MISMATCH,
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

    inline AsyncIdInfo GetAsyncIdInfo()
    {
        return {asyncId_, triggerAsyncId_};
    }

    inline NativeCallbackScope* OpenCallbackScope()
    {
        EnsureReference();
        auto callbackScopeManager = env_->GetCallbackScopeManager();
        if (callbackScopeManager == nullptr) {
            return nullptr;
        }

        auto callbackScope = callbackScopeManager->Open(env_, resource_.ToLocal(), GetAsyncIdInfo());
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

    static CallbackScopeCode CloseCallbackScope(NativeEngine* env, NativeCallbackScope* scope)
    {
        auto callbackScopeManager = env->GetCallbackScopeManager();
        if (callbackScopeManager == nullptr) {
            return CALLBACK_SCOPE_INVALID_ARG;
        }
        if (callbackScopeManager->GetOpenCallbackScopes() > 0) {
            callbackScopeManager->DecrementOpenCallbackScopes();
            callbackScopeManager->Close(scope);
            return CALLBACK_SCOPE_OK;
        } else {
            return CALLBACK_SCOPE_MISMATCH;
        }
    }

    panda::JSValueRef* MakeCallback(panda::FunctionRef* funRef, panda::JSValueRef* obj,
                                    panda::JSValueRef *const argv[], size_t argc)
    {
        EnsureReference();
        panda::JSValueRef* rst = InternalMakeCallback(env_, funRef, obj, argv, argc, this);
        return rst;
    }
private:
    NativeEngine* env_ {nullptr};
    double asyncId_ = 0;
    double triggerAsyncId_ = 0;
    panda::Global<panda::ObjectRef> resource_;
    bool lostReference_ = false;
};

static panda::FunctionRef* AsyncHooksCallbackTrampoline()
{
    return nullptr; // not support async_hook yet, and the actions need to be improved in the future
}

static void CloseScope(NativeAsyncHookContext* nativeAsyncContext,
                       NativeCallbackScopeManager* callbackScopeMgr,
                       NativeCallbackScope* callbackScope,
                       NativeEngine* engine)
{
    if (nativeAsyncContext != nullptr) {
        nativeAsyncContext->CloseCallbackScope(engine, callbackScope);
    } else {
        if (callbackScopeMgr != nullptr) {
            callbackScopeMgr->Close(callbackScope);
        }
    }
}

static panda::JSValueRef* InternalMakeCallback(NativeEngine* engine, panda::FunctionRef* funRef,
                                               panda::JSValueRef* obj, panda::JSValueRef* const argv[],
                                               size_t argc, NativeAsyncHookContext* asyncContext)
{
    auto vm = engine->GetEcmaVm();
    if (funRef == nullptr) {
        HILOG_ERROR("funRef is nullptr!");
        return *panda::JSValueRef::Undefined(vm);
    }

    bool useAsyncHooksTrampoline = false;
    panda::FunctionRef* rstFunRef = AsyncHooksCallbackTrampoline();
    if (rstFunRef != nullptr) {
        useAsyncHooksTrampoline = true;
    }
    NativeCallbackScopeManager* callbackScopeMgr = nullptr;
    NativeCallbackScope* callbackScope;
    if (asyncContext == nullptr) {
        callbackScopeMgr = engine->GetCallbackScopeManager();
        callbackScope = callbackScopeMgr->Open(engine,
            panda::Local<panda::ObjectRef>(reinterpret_cast<uintptr_t>(obj)), {0, 0});
    } else {
        callbackScope = asyncContext->OpenCallbackScope();
    }

    panda::JSValueRef* callBackRst;
    if (useAsyncHooksTrampoline) {
        callBackRst = nullptr; // not support async_hook yet, and the actions need to be improved in the future
    } else {
        callBackRst = funRef->CallForNapi(vm, obj, argv, argc);
    }

    if (callbackScope != nullptr) {
        if (callBackRst == nullptr) {
            callbackScope->MarkAsFailed();
        }
        callbackScope->Close();
    }

    CloseScope(asyncContext, callbackScopeMgr, callbackScope, engine);
    return callBackRst;
}

panda::JSValueRef* MakeCallback(NativeEngine* engine, panda::FunctionRef* funRef,
                                panda::JSValueRef* obj, size_t argc,
                                panda::JSValueRef* const argv[], NativeAsyncHookContext* asyncContext)
{
    auto vm = engine->GetEcmaVm();
    panda::JSValueRef* rst = InternalMakeCallback(engine, funRef, obj, argv, argc, asyncContext);
    NativeCallbackScopeManager* callbackScopeMgr = engine->GetCallbackScopeManager();
    size_t depth = callbackScopeMgr->GetAsyncCallbackScopeDepth();
    if (rst == nullptr && depth == 0) {
        return *panda::JSValueRef::Undefined(vm);
    }
    return rst;
}

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_ASYNC_HOOK_CONTEXT_H */
