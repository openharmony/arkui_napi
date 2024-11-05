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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_ARK_FINALIZERS_PACK_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_ARK_FINALIZERS_PACK_H

#include "ark_crash_holder.h"
#include "interfaces/inner_api/napi/native_node_api.h"

class NativeEngine;

using RefFinalizer = std::pair<NapiNativeFinalize, std::tuple<NativeEngine*, void*, void*>>;
using ArkFinalizersPackFinishNotify = std::function<void(size_t totalNativeBindingSize)>;

class ArkFinalizersPack {
public:
    ArkFinalizersPack() = default;
    ~ArkFinalizersPack() = default;
    DEFAULT_MOVE_SEMANTIC(ArkFinalizersPack);
    DEFAULT_COPY_SEMANTIC(ArkFinalizersPack);

    void Clear()
    {
        finalizers_.clear();
        totalNativeBindingSize_ = 0;
        notify_ = nullptr;
    }
    bool Empty() const
    {
        return finalizers_.empty();
    }
    void RegisterFinishNotify(ArkFinalizersPackFinishNotify notify)
    {
        notify_ = notify;
    }
    size_t GetNumFinalizers() const
    {
        return finalizers_.size();
    }
    void ProcessAll() const
    {
        INIT_CRASH_HOLDER(holder);
        for (auto &iter : finalizers_) {
            NapiNativeFinalize callback = iter.first;
            auto &[p0, p1, p2] = iter.second;
            holder.UpdateCallbackPtr(reinterpret_cast<uintptr_t>(callback));
            callback(reinterpret_cast<napi_env>(p0), p1, p2);
        }
        NotifyFinish();
    }
    size_t GetTotalNativeBindingSize() const
    {
        return totalNativeBindingSize_;
    }
    void AddFinalizer(RefFinalizer &finalizer, size_t nativeBindingSize)
    {
        finalizers_.emplace_back(finalizer);
        totalNativeBindingSize_ += nativeBindingSize;
    }
private:
    void NotifyFinish() const
    {
        if (notify_ != nullptr) {
            notify_(totalNativeBindingSize_);
        }
    }
    std::vector<RefFinalizer> finalizers_ {};
    size_t totalNativeBindingSize_ {0};
    ArkFinalizersPackFinishNotify notify_ {nullptr};
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_ARK_FINALIZERS_PACK_H */
