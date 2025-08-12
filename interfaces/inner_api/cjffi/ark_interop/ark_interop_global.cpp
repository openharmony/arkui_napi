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
#include "ark_interop_log.h"

#include <atomic>
#include <unordered_map>
#include <vector>

using namespace panda;
using namespace panda::ecmascript;

struct ARKTS_Global_ {
    ARKTS_Global_(EcmaVM *vm, const Local<JSValueRef>& value): ref(vm, value), isDisposed(false) {}

    ~ARKTS_Global_()
    {
        if (!isDisposed) {
            ref.FreeGlobalHandleAddr();
        }
    }

    void SetWeak()
    {
        if (isDisposed) {
            return;
        }
        if (ref.IsWeak()) {
            return;
        }
        ref.SetWeakCallback(this, [](void* handle) {
            auto global = P_CAST(handle, ARKTS_Global);
            if (!global) {
                return;
            }
            global->isDisposed = true;
            global->ref.FreeGlobalHandleAddr();
        }, [](void*) {});
    }

    void ClearWeak()
    {
        if (isDisposed) {
            return;
        }
        if (ref.IsEmpty() || !ref.IsWeak()) {
            return;
        }
        ref.ClearWeak();
    }

    ARKTS_Value GetValue() const
    {
        if (isDisposed) {
            return ARKTS_CreateUndefined();
        }
        auto result = BIT_CAST(ref, const Local<JSValueRef>);
        return ARKTS_FromHandle(result);
    }

    bool IsAlive() const
    {
        return !isDisposed && !ref.IsEmpty();
    }

    void SetDisposed()
    {
        isDisposed = true;
    }

private:
    Global<JSValueRef> ref;
    bool isDisposed;
};

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
    static void AddManager(ARKTS_Env env);
    static void RemoveManager(ARKTS_Env env);

    explicit GlobalManager(EcmaVM* vm);
    ~GlobalManager();

private:
    static SpinLock managersMutex_;
    static std::unordered_map<EcmaVM*, GlobalManager*> managers_ __attribute__((guarded_by(managersMutex_)));

private:
    EcmaVM* vm_;
    bool onSchedule_;
    SpinLock handleMutex_ {};
    std::vector<uintptr_t> handlesToDispose_ __attribute__((guarded_by(handleMutex_))) {};
};

std::unordered_map<EcmaVM*, GlobalManager*> GlobalManager::managers_;
SpinLock GlobalManager::managersMutex_;

GlobalManager::GlobalManager(EcmaVM* vm)
{
    onSchedule_ = false;
    vm_ = vm;
}

void GlobalManager::AsyncDisposer(ARKTS_Env env, void* data)
{
    GlobalManager* current = nullptr;
    managersMutex_.Acquire();
    auto exist = managers_.find(reinterpret_cast<EcmaVM*>(data));
    if (exist != managers_.end()) {
        current = exist->second;
    }
    managersMutex_.Release();
    if (!current || data != current) {
        return;
    }
    auto manager = (GlobalManager*)data;
    std::vector<uintptr_t> toDispose;
    manager->handleMutex_.Acquire();
    manager->onSchedule_ = false;
    std::swap(toDispose, manager->handlesToDispose_);
    manager->handleMutex_.Release();

    for (auto handle : toDispose) {
        auto global = P_CAST(handle, ARKTS_Global);
        delete global;
    }
}

void GlobalManager::Dispose(EcmaVM* vm, uintptr_t handle)
{
    GlobalManager* manager = nullptr;
    managersMutex_.Acquire();
    auto itor = managers_.find(vm);
    if (itor != managers_.end()) {
        manager = itor->second;
    }
    managersMutex_.Release();
    if (!manager) {
        // JSRuntime is disposed, still need to delete c++ object.
        auto global = reinterpret_cast<ARKTS_Global_*>(handle);
        // JSRuntime is disposed, can not FreeGlobal by now, set disposed to skip FreeGlobal.
        global->SetDisposed();
        delete global;
        return;
    }
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

void GlobalManager::AddManager(ARKTS_Env env)
{
    auto vm = reinterpret_cast<EcmaVM*>(env);
    auto manager = new GlobalManager(vm);
    managersMutex_.Acquire();
    auto result = managers_.try_emplace(vm, manager);
    managersMutex_.Release();
    if (!result.second) {
        delete manager;
    }
}

void GlobalManager::RemoveManager(ARKTS_Env env)
{
    auto vm = reinterpret_cast<EcmaVM*>(env);
    GlobalManager* current = nullptr;
    managersMutex_.Acquire();
    auto itor = managers_.find(vm);
    if (itor != managers_.end()) {
        current = itor->second;
        managers_.erase(itor);
    }
    managersMutex_.Release();
    if (current) {
        delete current;
    }
}

GlobalManager::~GlobalManager()
{
    std::vector<uintptr_t> toDispose;
    handleMutex_.Acquire();
    std::swap(toDispose, handlesToDispose_);
    handleMutex_.Release();
    for (auto p : toDispose) {
        auto global = reinterpret_cast<ARKTS_Global_*>(p);
        global->SetDisposed();
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
    auto result = new ARKTS_Global_(vm, handle);

    return P_CAST(result, ARKTS_Global);
}

ARKTS_Value ARKTS_GetGlobalValue(ARKTS_Global global)
{
    ARKTS_ASSERT_P(global, "global is null");

    return global->GetValue();
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

void ARKTS_DisposeGlobalSync(ARKTS_Env env, ARKTS_Global global)
{
    ARKTS_ASSERT_V(env, "env is null");
    ARKTS_ASSERT_V(global, "handle is null");

    delete global;
}

void ARKTS_GlobalSetWeak(ARKTS_Env env, ARKTS_Global global)
{
    ARKTS_ASSERT_V(env, "env is null");
    ARKTS_ASSERT_V(global, "global is null");
    global->SetWeak();
}

void ARKTS_GlobalClearWeak(ARKTS_Env env, ARKTS_Global global)
{
    ARKTS_ASSERT_V(env, "env is null");
    ARKTS_ASSERT_V(global, "global is null");
    global->ClearWeak();
}

constexpr uint64_t GLOBAL_TAG = 0b11111ULL << 48;
constexpr uint64_t GLOBAL_MASK = 0x0000'FFFF'FFFF'FFFF;

ARKTS_Value ARKTS_GlobalToValue(ARKTS_Env env, ARKTS_Global global)
{
    ARKTS_ASSERT_P(env, "env is null");
    ARKTS_ASSERT_P(global, "global is null");

    auto value = reinterpret_cast<uint64_t>(global) & GLOBAL_MASK;
    value = value | GLOBAL_TAG;
    auto dValue = static_cast<double>(value);
    return ARKTS_CreateF64(dValue);
}

ARKTS_Global ARKTS_GlobalFromValue(ARKTS_Env env, ARKTS_Value value)
{
    ARKTS_ASSERT_P(env, "env is null");
    ARKTS_ASSERT_P(ARKTS_IsNumber(value), "value is a number");

    auto dValue = ARKTS_GetValueNumber(value);
    auto iValue = static_cast<uint64_t>(dValue);
    ARKTS_ASSERT_P((iValue & GLOBAL_TAG) == GLOBAL_TAG, "invalid tag value");
    iValue = iValue & GLOBAL_MASK;
    return BIT_CAST(iValue, ARKTS_Global);
}

bool ARKTS_GlobalIsAlive(ARKTS_Env env, ARKTS_Global global)
{
    ARKTS_ASSERT_F(env, "env is null");
    ARKTS_ASSERT_F(global, "global is null");

    return global->IsAlive();
}

void ARKTS_InitGlobalManager(ARKTS_Env env)
{
    GlobalManager::AddManager(env);
}

void ARKTS_RemoveGlobalManager(ARKTS_Env env)
{
    GlobalManager::RemoveManager(env);
}