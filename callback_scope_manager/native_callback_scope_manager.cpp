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

#include "native_callback_scope_manager.h"
#include "native_engine/native_engine.h"

NativeCallbackScope::NativeCallbackScope(NativeEngine* env,
                                         panda::Local<panda::ObjectRef> object,
                                         const AsyncIdInfo& asyncIdInfo,
                                         int flags) : env_(env), asyncIdInfo_(asyncIdInfo),
    object_(object), skipHooks_(flags & SKIP_ASYNC_HOOKS), skipTaskQueues_(flags & SKIP_TASK_QUEUES)
{
    [[maybe_unused]] panda::LocalScope scope(env_->GetEcmaVm());

    // not support async_hook yet, and push_async_context actions need to be improved in the future

    pushedIds_ = true;

    if (asyncIdInfo.asyncId != 0 && !skipHooks_) {
        NativeAsyncWrap::EmitBefore(env, asyncIdInfo.asyncId);
    }
}

NativeCallbackScope::~NativeCallbackScope()
{
    Close();
}

void NativeCallbackScope::Close()
{
    if (closed_) {
        return;
    }
    closed_ = true;

    auto performStoppingCheck = [this]() {
        if (env_->IsStopping()) {
            MarkAsFailed();
            // not support async_hook yet, and clear_async_id_stack actions need to be improved in the future
        }
    };
    performStoppingCheck();

    if (env_->IsStopping()) {
        return;
    }

    if (!failed_ && asyncIdInfo_.asyncId != 0 && !skipHooks_) {
        NativeAsyncWrap::EmitAfter(env_, asyncIdInfo_.asyncId);
    }

    if (pushedIds_) {
        pushedIds_ = false;
        // not support async_hook yet, and pop_async_context actions need to be improved in the future
    }

    if (failed_) {
        return;
    }

    if (env_->GetCallbackScopeManager()->GetOpenCallbackScopes() > 1 || skipTaskQueues_) {
        return;
    }
}

NativeCallbackScopeManager::NativeCallbackScopeManager() {}

NativeCallbackScopeManager::~NativeCallbackScopeManager() {}

NativeCallbackScope* NativeCallbackScopeManager::Open(NativeEngine* env,
                                                      panda::Local<panda::ObjectRef> object,
                                                      AsyncIdInfo asyncIdInfo)
{
    NativeCallbackScope* scope = new (std::nothrow)NativeCallbackScope(env, object, asyncIdInfo);

    if (scope) {
        asyncCallbackScopeDepth_++;
        return scope;
    } else {
        return nullptr;
    }
}

void NativeCallbackScopeManager::Close(NativeCallbackScope* scope)
{
    if (scope != nullptr) {
        delete scope;
    }
    asyncCallbackScopeDepth_--;
}

size_t NativeCallbackScopeManager::IncrementOpenCallbackScopes()
{
    openCallbackScopes_++;
    return openCallbackScopes_;
}

size_t NativeCallbackScopeManager::DecrementOpenCallbackScopes()
{
    openCallbackScopes_--;
    return openCallbackScopes_;
}
