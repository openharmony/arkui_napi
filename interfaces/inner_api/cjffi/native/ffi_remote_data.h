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

#ifndef OHOS_FFI_FFI_REMOTE_DATA_H
#define OHOS_FFI_FFI_REMOTE_DATA_H

#include "nocopyable.h"
#include "refbase.h"
#include "unordered_map"

#include <mutex>

#include "cj_fn_invoker.h"
#include "runtimetype.h"

namespace OHOS::FFI {

constexpr int64_t MAX_INT64 = 0x7fffffffffffffff;

class RemoteData;
class FFIData;

class FFI_EXPORT FFIDataManager {
public:
    static FFIDataManager* GetInstance();

    DISALLOW_COPY_AND_MOVE(FFIDataManager);
    FFIDataManager() = default;

    sptr<FFIData> GetFFIData(int64_t id)
    {
        std::lock_guard<std::mutex> lock(mtx);
        return ffiDataStore_[id];
    }
        
    sptr<RemoteData> GetRemoteData(int64_t id)
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto itor = remoteDataStore_.find(id);
        if (itor == remoteDataStore_.end()) {
            return nullptr;
        }
        return itor->second.promote();
    }
    void StoreFFIData(const sptr<FFIData>& data);
    void StoreRemoteData(const sptr<RemoteData>& data);
    void RemoveFFIData(int64_t id)
    {
        std::lock_guard<std::mutex> lock(mtx);
        ffiDataStore_.erase(id);
    }
    void RemoveRemoteData(int64_t id)
    {
        std::lock_guard<std::mutex> lock(mtx);
        remoteDataStore_.erase(id);
    }

    int64_t NewFFIDataId();

private:
    friend class FFIData;
    uint64_t curFFIDataId_ = 0;
    const int64_t maxId = MAX_INT64;
    // maxCapacity can be set to a larger number if needed, make sure maxCapacity is not larger than maxId
    const int64_t maxCapacity = MAX_INT64;
    std::mutex mtx;

    std::unordered_map<int64_t, sptr<FFIData>> ffiDataStore_;
    std::unordered_map<int64_t, wptr<RemoteData>> remoteDataStore_;

    bool FFIDataExist(int64_t id) const
    {
        return ffiDataStore_.find(id) != ffiDataStore_.end();
    }

    int FFIDataIdSafeIncrease();
};

#define CJ_REMOTE_CLASS(className)                               \
    DECL_TYPE(className, RemoteData)                           \
public:                                                        \
    friend class RefBase;                                      \
    friend class RemoteData;                                   \
    DISALLOW_COPY_AND_MOVE(className);                         \
    className() = delete;                                      \
                                                               \
protected:                                                     \
    explicit className(int64_t id) : OHOS::FFI::RemoteData(id) \
    {}

/**
 * a reference of cj FFIData object.
 *  standard create procedure:
 *      1. cj: create FFIData
 *      2. cj: register to FFIDataManager
 *      3. cj: pass id to native
 *      4. native: RemoteData::Create
 *  standard destroy procedure:
 *      1. native: ~RemoteData
 *      2. cj: remove from FFIDataManager
 *      3. cj: FFIData::onDestroyed
 */
class FFI_EXPORT RemoteData : public TypeBase, public virtual RefBase {
    DECL_TYPE(RemoteData, TypeBase)
public:
    DISALLOW_COPY_AND_MOVE(RemoteData);

    template<class T>
    static sptr<T> Create(int64_t id)
    {
        auto manager = FFIDataManager::GetInstance();
        auto existed = manager->GetRemoteData(id);
        if (existed != nullptr) {
            return sptr<T>(existed.GetRefPtr()->template DynamicCast<T>());
        }
        auto ref = sptr<T>(new (std::nothrow) T(id));
        if (ref) {
            manager->StoreRemoteData(ref);
        }
        return ref;
    }

    ~RemoteData() override;

protected:
    explicit RemoteData(int64_t id);

private:
    friend class RefBase;

public:
    int64_t GetID() const;

private:
    int64_t id_;
    bool isValid_;
};

class FFI_EXPORT CJLambdaRemoteData : public RemoteData {
    DECL_TYPE(CJLambdaRemoteData, RemoteData)
public:
    explicit CJLambdaRemoteData(int64_t id): RemoteData(id) {}

    ~CJLambdaRemoteData() override;
};

/**
 * object to referenced by cj.
 *  standard create procedure:
 *      0. native: Define class derived from FFIData with macro DECL_TYPE to support safe dynamic cast
 *      1. native: FFIData::Create
 *      2. native: register to FFIDataManager
 *      3. native: pass to cj side
 *      4. cj: create RemoteData
 *      5. cj: register to RemoteDataManager
 *  standard destroy procedure:
 *      1. cj: RemoteData.release
 *      2. native: remove from FFIDataManager
 *      3. native: ~FFIData
 *      4. cj: RemoteData.onDestroyed
 */
class FFI_EXPORT FFIData : public TypeBase, public RefBase {
    DECL_TYPE(FFIData, TypeBase)
public:
    DISALLOW_COPY_AND_MOVE(FFIData);
    FFIData() : id_(FFIDataManager::GetInstance()->NewFFIDataId()) {}
    ~FFIData() override;

    template<class T, class... Args>
    static sptr<T> Create(Args... args)
    {
        auto ref = sptr<T>(new (std::nothrow) T(std::forward<Args>(args)...));
        if (ref) {
            FFIDataManager::GetInstance()->StoreFFIData(ref);
        }
        return ref;
    }

    template<class T>
    static sptr<T> GetData(int64_t id)
    {
        auto data = FFIDataManager::GetInstance()->GetFFIData(id);
        if (data != nullptr) {
            return sptr<T>(data.GetRefPtr()->template DynamicCast<T>());
        }
        return nullptr;
    }

    static void Release(int64_t id)
    {
        FFIDataManager::GetInstance()->RemoveFFIData(id);
    }
    static bool Exist(int64_t id)
    {
        auto& store_ = FFIDataManager::GetInstance()->ffiDataStore_;
        return store_.find(id) != store_.end();
    }

    int64_t GetID() const
    {
        return id_;
    }

private:
    int64_t id_;
};

} // namespace OHOS::FFI

#endif // OHOS_FFI_FFI_REMOTE_DATA_H
