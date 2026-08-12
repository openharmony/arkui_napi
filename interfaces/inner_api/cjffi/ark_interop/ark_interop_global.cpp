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
#include <mutex>

#include "ark_interop_scope.h"

#ifdef HW_ASAN
constexpr bool USE_HWASAN = true;
#else
constexpr bool USE_HWASAN = false;
#endif

using namespace panda;
using namespace panda::ecmascript;

class GlobalManager;

constexpr size_t INVALID_ID = -1;

struct ARKTS_Global_ {
    ARKTS_Global_(EcmaVM *vm, const Local<JSValueRef>& value): ref(vm, value), isDisposed(false)
    {
#ifdef HW_ASAN
        env_ = reinterpret_cast<ARKTS_Env>(vm);
#endif
    }

    ~ARKTS_Global_();

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
        if constexpr (USE_HWASAN) {
            return ARKTS_Scope_::NormalPointer(const_cast<ARKTS_Global_*>(this));
        } else {
            return ARKTS_Scope_::NormalPointer(BIT_CAST(const_cast<Global<JSValueRef>&>(ref), void*));
        }
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
    friend class GlobalManager;
    Global<JSValueRef> ref;
    bool isDisposed;
#ifdef HW_ASAN
    ARKTS_Env env_;
    size_t recordId_ {INVALID_ID};
#endif
};

template <typename T>
struct Slab {
    size_t Append(T data)
    {
        if (lastEmpty != INVALID_ID) {
            SlabItem& last = items_[lastEmpty];
            last.data = data;
            size_t result = lastEmpty;
            lastEmpty = last.prev;
            return result;
        }
        size_t result = items_.size();
        items_.emplace_back(data);
        return result;
    }

    std::optional<T> Remove(size_t id)
    {
        if (id >= items_.size()) {
            return std::nullopt;
        }
        SlabItem& item = items_[id];
        if (!item.data.has_value()) {
            return std::nullopt;
        }
        item.prev = lastEmpty;
        lastEmpty = id;
        return std::move(item.data);
    }

    std::optional<T> Get(size_t id)
    {
        if (id >= items_.size()) {
            return std::nullopt;
        }
        return items_[id].data;
    }

    void Clear()
    {
        items_.clear();
        lastEmpty = INVALID_ID;
    }
private:
    struct SlabItem {
        std::optional<T> data;
        size_t prev;

        explicit SlabItem(T data): data(std::move(data)), prev(INVALID_ID) {}
    };
    std::vector<SlabItem> items_{};
    size_t lastEmpty {INVALID_ID};
};

class GlobalManager {
public:
    static void Dispose(ARKTS_Env env, ARKTS_Global handle);
    static void AsyncDisposer(ARKTS_Env env, int64_t data);
    static void AddManager(ARKTS_Env env);
    static void RemoveManager(ARKTS_Env env);
    static void RemoveRecord(ARKTS_Env env, size_t id);
    static double RecordGlobal(ARKTS_Env env, ARKTS_Global handle);
    static ARKTS_Global GetRecord(ARKTS_Env env, double value);

    explicit GlobalManager(ARKTS_Env env);
    ~GlobalManager();

private:

    static std::mutex managersMutex_;
    static std::unordered_map<ARKTS_Env, GlobalManager*> managers_;

private:
    ARKTS_Env vm_;
    bool onSchedule_;
    std::mutex handleMutex_ {};
    std::vector<ARKTS_Global> handlesToDispose_ {};
#ifdef HW_ASAN
    Slab<ARKTS_Global> records_;
#endif
};

std::unordered_map<ARKTS_Env, GlobalManager*> GlobalManager::managers_;
std::mutex GlobalManager::managersMutex_;

GlobalManager::GlobalManager(ARKTS_Env env): vm_(env)
{
    onSchedule_ = false;
}

void GlobalManager::AsyncDisposer(ARKTS_Env env, int64_t)
{
    GlobalManager* current = nullptr;
    {
        std::lock_guard lock(managersMutex_);
        auto exist = managers_.find(env);
        if (exist != managers_.end()) {
            current = exist->second;
        }
    }
    if (!current) {
        return;
    }
    std::vector<ARKTS_Global> toDispose;
    {
        std::lock_guard lock(current->handleMutex_);
        current->onSchedule_ = false;
        std::swap(toDispose, current->handlesToDispose_);
    }

    for (auto global : toDispose) {
        delete global;
    }
}

void GlobalManager::Dispose(ARKTS_Env env, ARKTS_Global_* global)
{
    GlobalManager* manager = nullptr;
    {
        std::lock_guard lock(managersMutex_);
        auto itor = managers_.find(env);
        if (itor != managers_.end()) {
            manager = itor->second;
        }
    }
    if (!manager) {
        // JSRuntime is disposed, still need to delete c++ object.
        // JSRuntime is disposed, can not FreeGlobal by now, set disposed to skip FreeGlobal.
        global->SetDisposed();
        delete global;
        return;
    }
    bool needSchedule = false;
    {
        std::lock_guard lock(manager->handleMutex_);
        manager->handlesToDispose_.push_back(global);
        if (!manager->onSchedule_) {
            manager->onSchedule_ = needSchedule = true;
        }
    }
    if (needSchedule) {
        ARKTSInner_CreateAsyncTask(env, AsyncDisposer, 0);
    }
}

void GlobalManager::AddManager(ARKTS_Env env)
{
    auto manager = new GlobalManager(env);
    std::lock_guard lock(managersMutex_);
    auto result = managers_.try_emplace(env, manager);
    if (!result.second) {
        delete manager;
    }
}

void GlobalManager::RemoveManager(ARKTS_Env env)
{
    GlobalManager* current = nullptr;
    {
        std::lock_guard lock(managersMutex_);
        auto itor = managers_.find(env);
        if (itor != managers_.end()) {
            current = itor->second;
            managers_.erase(itor);
        }
    }
    if (current) {
        delete current;
    }
}

GlobalManager::~GlobalManager()
{
    std::vector<ARKTS_Global> toDispose;
    {
        std::lock_guard lock(handleMutex_);
        std::swap(toDispose, handlesToDispose_);
    }
    for (ARKTS_Global global : toDispose) {
        delete global;
    }
}

// assume value is object
ARKTS_Global ARKTS_CreateGlobal(ARKTS_Env env, ARKTS_Value value)
{
    ARKTS_ASSERT_P(env, "env is null");
    auto vm = P_CAST(env, EcmaVM*);
    panda::JsiFastNativeScope fastNativeScope(vm);
    ARKTS_ASSERT_P(ARKTS_IsHeapObject(value), "value is not heap object");

    auto handle = ARKTS_Scope_::GetLocal(env, value);
    auto result = new ARKTS_Global_(vm, handle);

    return P_CAST(result, ARKTS_Global);
}

ARKTS_Value ARKTS_GetGlobalValue(ARKTS_Global global)
{
    ARKTS_ASSERT_U(global, "global is null");

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
    GlobalManager::Dispose(env, global);
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

double GlobalManager::RecordGlobal(ARKTS_Env env, ARKTS_Global global)
{
#ifndef HW_ASAN
    auto value = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(global)) & GLOBAL_MASK;
    value = value | GLOBAL_TAG;
    return value;
#else
    size_t recordId = global->recordId_;
    if (recordId == INVALID_ID) {
        GlobalManager* current = nullptr;
        {
            std::lock_guard lock(managersMutex_);
            current = managers_[env];
        }
        if (!current) {
            return NAN;
        }
        recordId = current->records_.Append(global);
        global->recordId_ = recordId;
    }

    auto value = static_cast<uint64_t>(static_cast<uintptr_t>(recordId)) & GLOBAL_MASK;
    value = value | GLOBAL_TAG;
    return value;
#endif
}

#ifdef HW_ASAN
void GlobalManager::RemoveRecord(ARKTS_Env env, size_t id)
{
    GlobalManager* current = nullptr;
    {
        std::lock_guard lock(managersMutex_);
        current = managers_[env];
    }
    if (!current) {
        return;
    }
    current->records_.Remove(id);
}
#endif

ARKTS_Global_::~ARKTS_Global_()
{
    if (!isDisposed) {
        if (ref.IsWeak()) {
            ref.ClearWeak();
        }
        ref.FreeGlobalHandleAddr();
    }
#ifdef HW_ASAN
    if (recordId_ != INVALID_ID) {
        GlobalManager::RemoveRecord(env_, recordId_);
    }
#endif
}

ARKTS_Value ARKTS_GlobalToValue(ARKTS_Env env, ARKTS_Global global)
{
    ARKTS_ASSERT_U(env, "env is null");
    ARKTS_ASSERT_U(global, "global is null");

    auto value = GlobalManager::RecordGlobal(env, global);
    return ARKTS_CreateF64(value);
}

ARKTS_Global GlobalManager::GetRecord(ARKTS_Env env, double value)
{
#ifndef HW_ASAN
    auto iValue = static_cast<uint64_t>(value) & GLOBAL_MASK;
    uintptr_t addr = iValue;
    return reinterpret_cast<ARKTS_Global>(addr);
#else
    GlobalManager* current = nullptr;
    {
        std::lock_guard lock(managersMutex_);
        current = managers_[env];
    }
    if (!current) {
        return nullptr;
    }
    auto iValue = static_cast<size_t>(value) & GLOBAL_MASK;
    std::optional<ARKTS_Global> result = current->records_.Get(iValue);
    return result.value_or(nullptr);
#endif
}

ARKTS_Global ARKTS_GlobalFromValue(ARKTS_Env env, ARKTS_Value value)
{
    ARKTS_ASSERT_P(env, "env is null");
    ARKTS_ASSERT_P(ARKTS_IsNumber(value), "value is a number");

    auto dValue = ARKTS_GetValueNumber(value);
    auto iValue = static_cast<uint64_t>(dValue);
    ARKTS_ASSERT_P((iValue & GLOBAL_TAG) == GLOBAL_TAG, "invalid tag value");
    return GlobalManager::GetRecord(env, dValue);
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