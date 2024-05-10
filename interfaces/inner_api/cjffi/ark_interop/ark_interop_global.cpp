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

#include "ark_interop_internal.h"
#include "ark_interop_napi.h"

#include <atomic>
#include <unordered_map>
#include <vector>

using namespace panda;
using namespace panda::ecmascript;

namespace {
class __attribute__((capability("mutex"))) SpinLock final {
public:
    void Acquire() __attribute__((acquire_capability()))
    {
        bool locked = false;
        while (!isLocked_.compare_exchange_strong(locked, true)) {
            locked = false;
        }
    }

    void Release() __attribute__((release_capability()))
    {
        isLocked_ = false;
    }
private:
    std::atomic<bool> isLocked_ {false};
};

class GlobalManager {
public:
    static void Dispose(EcmaVM* vm, uintptr_t handle);
    static void AsyncDisposer(ARKTS_Env env, void* data);

    explicit GlobalManager(EcmaVM* vm);

private:
    static SpinLock managersMutex_;
    static std::unordered_map<EcmaVM*, GlobalManager> managers_ __attribute__((guarded_by(managersMutex_)));

private:
    EcmaVM* vm_;
    bool onSchedule_;
    SpinLock handleMutex_ {};
    std::vector<uintptr_t> handlesToDispose_ __attribute__((guarded_by(handleMutex_))) {};
};

std::unordered_map<EcmaVM*, GlobalManager> GlobalManager::managers_;
SpinLock GlobalManager::managersMutex_;

GlobalManager::GlobalManager(EcmaVM* vm)
{
    onSchedule_ = false;
    vm_ = vm;
}

void GlobalManager::AsyncDisposer(ARKTS_Env env, void* data)
{
    auto manager = (GlobalManager*)data;
    std::vector<uintptr_t> toDispose;
    manager->handleMutex_.Acquire();
    manager->onSchedule_ = false;
    std::swap(toDispose, manager->handlesToDispose_);
    manager->handleMutex_.Release();

    for (auto handle : toDispose) {
        auto global = P_CAST(handle, Global<JSValueRef>*);
        delete global;
    }
}

void GlobalManager::Dispose(EcmaVM* vm, uintptr_t handle)
{
    GlobalManager* manager;
    managersMutex_.Acquire();
    manager = &managers_.try_emplace(vm, vm).first->second;
    managersMutex_.Release();
    bool needSchedule = false;
    manager->handleMutex_.Acquire();
    manager->handlesToDispose_.push_back(handle);
    if (!manager->onSchedule_) {
        manager->onSchedule_ = needSchedule = true;
    }
    manager->handleMutex_.Release();
    if (needSchedule) {
        ARKTSInner_CreateAsyncTask(P_CAST(vm, ARKTS_Env), AsyncDisposer, manager);
    }
}
}

// assume value is object
ARKTS_Global ARKTS_CreateGlobal(ARKTS_Env env, ARKTS_Value value)
{
    ARKTS_ASSERT_P(env, "env is null");
    ARKTS_ASSERT_P(ARKTS_IsHeapObject(value), "value is not heap object");

    auto vm = P_CAST(env, EcmaVM*);
    auto handle = BIT_CAST(value, Local<JSValueRef>);
    auto result = new Global<JSValueRef>(vm, handle);

    return P_CAST(result, ARKTS_Global);
}

/**
 * no actual action about this convert, the data is in the same form with JSHandle,
 * it does not mean global is equal to local, the memory they are allocated are controlled by different system,
 * have no idea the consequence of NativeScope escape a global variable
 */
ARKTS_Value ARKTS_GetGlobalValue(ARKTS_Global global)
{
    ARKTS_ASSERT_P(global, "global is null");

    auto result = *P_CAST(global, Local<JSValueRef>*);

    return ARKTS_FromHandle(result);
}

/**
 * cj destructor called in isolate thread, may crash js GC.
 * store the handle to a table, do dispose at js thread on idle
 */
void ARKTS_DisposeGlobal(ARKTS_Env env, ARKTS_Global global)
{
    ARKTS_ASSERT_V(env, "env is null");
    ARKTS_ASSERT_V(global, "global is null");
    GlobalManager::Dispose(P_CAST(env, EcmaVM*), P_CAST(global, uintptr_t));
}