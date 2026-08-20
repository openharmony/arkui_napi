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

#include "ark_interop_scope.h"

#include "native_engine/impl/ark/ark_native_engine.h"

#ifdef HW_ASAN
constexpr bool USE_HWASAN = true;
#else
constexpr bool USE_HWASAN = false;
#endif

std::mutex ARKTS_Scope_::threadMutex;
std::map<ARKTS_Env, ARKTS_Scope_::ThreadScopes> ARKTS_Scope_::threads;

ARKTS_Scope_::ThreadScopes& ARKTS_Scope_::GetThreadScopes(ARKTS_Env env)
{
    ThreadScopes* thread;
    {
        std::lock_guard lock(threadMutex);
        thread = &threads[env];
    }
    if (!thread->top) {
        thread->top = new ARKTS_Scope_(env, nullptr);
    }
    return *thread;
}

ARKTS_Scope_::ThreadScopes* ARKTS_Scope_::GetThreadOpt(ARKTS_Env env)
{
    ThreadScopes* thread = nullptr;
    {
        std::lock_guard lock(threadMutex);
        auto exist = threads.find(env);
        if (exist != threads.end()) {
            thread = &exist->second;
        }
    }
    return thread;
}

ARKTS_Scope_::ARKTS_Scope_(ARKTS_Env env, ARKTS_Scope_* parent): currentEnv(env), parentScope(parent),
    scope(std::nullopt)
{
    scope.emplace(reinterpret_cast<panda::EcmaVM*>(env));
}

void ARKTS_Scope_::DisposeEnv(ARKTS_Env env)
{
    std::lock_guard lock(threadMutex);
    threads.erase(env);
}

ARKTS_Scope_::ThreadScopes::~ThreadScopes()
{
    for (ARKTS_Scope scope : toDispose) {
        delete scope;
    }
    toDispose.clear();
    ARKTS_Scope current = top;
    while (current != nullptr) {
        auto parent = const_cast<ARKTS_Scope>(current->parentScope);
        delete current;
        current = parent;
    }
}

ARKTS_Scope ARKTS_Scope_::NewScope(ARKTS_Env env)
{
    auto& thread = GetThreadScopes(env);
    auto newScope = new ARKTS_Scope_(env, thread.top);
    thread.top = newScope;
    return newScope;
}

void ARKTS_Scope_::AddToDisposeQueue(ARKTS_Scope scope)
{
    if constexpr (!USE_HWASAN) {
        delete scope;
        return;
    }
    scope->scope = std::nullopt;
    ThreadScopes& thread = GetThreadScopes(scope->currentEnv);
    thread.toDispose.push_back(scope);
    if (!thread.isDisposing) {
        thread.isDisposing = true;
        ARKTSInner_CreateAsyncTask(scope->currentEnv, [](ARKTS_Env env, int64_t) {
            ThreadScopes* thread = GetThreadOpt(env);
            if (!thread) {
                return;
            }
            thread->isDisposing = false;
            for (ARKTS_Scope scope : thread->toDispose) {
                delete scope;
            }
            thread->toDispose.clear();
        }, 0);
    }
}

bool ARKTS_Scope_::CloseScope(ARKTS_Scope target)
{
    ThreadScopes& thread = GetThreadScopes(target->currentEnv);
    if (thread.top != target) {
        return false;
    }
    auto parent = const_cast<ARKTS_Scope>(target->parentScope);
    if (parent->parentScope != nullptr) {
        AddToDisposeQueue(target);
        thread.top = parent;
    } else {
        parent->scope = std::nullopt;
        AddToDisposeQueue(target);
        AddToDisposeQueue(parent);
        thread.top = nullptr;
    }

    return true;
}

ARKTS_Value ARKTS_Scope_::NormalPointer(void* pointer)
{
    if constexpr (sizeof(void*) == 8) {
        constexpr uintptr_t addrMask = 0xFFFF'FFFF'FFFF;
        auto addr = reinterpret_cast<uintptr_t>(pointer) & addrMask;
        return {
            .pointer = reinterpret_cast<void*>(addr)
        };
    } else {
        // can't just set .pointer, for arm32 platform pointer is short than uint64_t. due to return addr alias,
        // direct set .pointer to existed ARKTS_Value may failed to zero high 4 bytes
        ARKTS_Value value {.value =  0};
        value.pointer = pointer;
        return value;
    }
}

ARKTS_Value ARKTS_Scope_::NewValue(ARKTS_Env env, panda::Local<panda::JSValueRef> ref)
{
    if (ref.IsNull()) {
        return ARKTS_CreateUndefined();
    }
    if (!ref->IsHeapObject()) {
        return {
            .value = *reinterpret_cast<uint64_t*>(*ref)
        };
    }

    if constexpr (USE_HWASAN) {
        ARKTS_Scope topScope = GetThreadScopes(env).top;
        Local<JSValueRef>* pointer = &topScope->handledValues.emplace_back(ref);
        return NormalPointer(pointer);
    } else {
        return NormalPointer(BIT_CAST(ref, void*));
    }
}

panda::Local<panda::JSValueRef> ARKTS_Scope_::GetLocal(ARKTS_Env env, ARKTS_Value value)
{
    if (ARKTS_IsHeapObject(value)) {
        if constexpr (USE_HWASAN) {
            return *reinterpret_cast<panda::Local<panda::JSValueRef>*>(value.pointer);
        } else {
            return BIT_CAST(value, panda::Local<panda::JSValueRef>);
        }
    } else {
        auto vm = reinterpret_cast<panda::EcmaVM*>(env);
        auto tag = BIT_CAST(value, panda::JSValueRef);
        return panda::JSNApi::CreateLocal(vm, tag);
    }
}

panda::JSValueRef ARKTS_Scope_::GetValueRef(ARKTS_Value value)
{
    if (ARKTS_IsHeapObject(value)) {
        if constexpr (USE_HWASAN) {
            return **reinterpret_cast<panda::JSValueRef**>(value.pointer);
        } else {
            return *reinterpret_cast<panda::JSValueRef*>(value.pointer);
        }
    } else {
        return BIT_CAST(value, panda::JSValueRef);
    }
}
