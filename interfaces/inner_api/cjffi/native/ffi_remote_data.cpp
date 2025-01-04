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

#include "ffi_remote_data.h"

#include <cinttypes>

#include "hilog/log_cpp.h"

using namespace OHOS::FFI;
using namespace OHOS;
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD003901, "CJ-FFIBindNative" };
} // namespace

FFIDataManager* FFIDataManager::GetInstance()
{
    static FFIDataManager singleton;
    return &singleton;
}

void FFIDataManager::StoreFFIData(const sptr<FFIData>& data)
{
    std::lock_guard<std::mutex> lock(mtx);
    int64_t id = data->GetID();
    // 0 represents invalid status
    if (id == 0) {
        HiLog::Fatal(LABEL, "FFIData store invalid key");
        return;
    }
    HiLog::Info(LABEL, "FFIData store_ key put in: %{public}" PRId64, id);
    ffiDataStore_[id] = data;
}

void FFIDataManager::StoreRemoteData(const sptr<RemoteData>& data)
{
    std::lock_guard<std::mutex> lock(mtx);
    remoteDataStore_[data->GetID()] = data;
}

int64_t FFIDataManager::NewFFIDataId()
{
    std::lock_guard<std::mutex> lock(mtx);
    if (static_cast<int64_t>(ffiDataStore_.size()) >= maxCapacity) {
        HiLog::Fatal(LABEL, "FFIData store_ over max capacity: %{public}" PRId64, maxCapacity);
        // 0 represents invalid status in CJ RemoteData, will be handled by CJ Exception
        return 0;
    }
    // When ffiDataId over uint64 max value, resetTime will be increased with 1.
    auto resetTimes = FFIDataIdSafeIncrease();
    auto resetTimeUpperBound = 2;
    while ((ffiDataStore_.find(curFFIDataId_) != ffiDataStore_.end()) && resetTimes < resetTimeUpperBound) {
        resetTimes += FFIDataIdSafeIncrease();
    }
    // It means there is no one available id, When resetTimes come to UpperBound 2.
    if (resetTimes >= resetTimeUpperBound) {
        HiLog::Fatal(LABEL, "FFIData id run out");
        // 0 represents invalid status in CJ RemoteData, will be handled by CJ Exception
        return 0;
    }
    HiLog::Info(LABEL, "FFIDataManager new ID : %{public}" PRId64 ", cache size: %{public}zu", curFFIDataId_,
        ffiDataStore_.size());
    return curFFIDataId_;
}

int FFIDataManager::FFIDataIdSafeIncrease()
{
    curFFIDataId_++;
    if (curFFIDataId_ >= static_cast<uint64_t>(maxId)) {
        HiLog::Warn(LABEL, "FFIData id: %{public}" PRId64 " over max %{public}" PRId64 ", reset to 0", curFFIDataId_,
            maxCapacity);
        curFFIDataId_ = 1;
        return 1;
    }
    return 0;
}

RemoteData::RemoteData(int64_t id) : id_(id), isValid_(id != 0)
{
    HiLog::Debug(LABEL, "RemoteData constructed: %{public}" PRId64 ".", id_);
}

RemoteData::~RemoteData()
{
    HiLog::Debug(LABEL, "RemoteData destructed: %{public}" PRId64 ".", id_);
    auto cjFunc = CJFFIFnInvoker::GetInstance()->GetCJFuncs().atCOHOSFFIReleaseFFIData;
    if (!cjFunc) {
        HiLog::Error(LABEL, "Failed to invoke CJ function: FFIReleaseFFIData!");
        return;
    }
    cjFunc(GetID());
    isValid_ = false;
}

CJLambdaRemoteData::~CJLambdaRemoteData()
{
    auto manager = FFIDataManager::GetInstance();
    manager->RemoveRemoteData(GetID());
}

int64_t RemoteData::GetID() const
{
    if (!isValid_) {
        HiLog::Error(LABEL, "RemoteData::GetID error, remote data invalid: %{public}" PRId64 ".", id_);
    }
    return id_;
}

FFIData::~FFIData()
{
    auto cjFunc = CJFFIFnInvoker::GetInstance()->GetCJFuncs().atCOHOSFFIReleaseRemoteData;
    if (!cjFunc) {
        HiLog::Error(LABEL, "Failed to invoke CJ function: FFIReleaseRemoteData!");
        return;
    }
    cjFunc(GetID());
}
