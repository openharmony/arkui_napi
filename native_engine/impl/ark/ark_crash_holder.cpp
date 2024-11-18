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

#include <cinttypes>
#include <securec.h>

#include "ark_crash_holder.h"
#include "utils/log.h"

#if defined(OHOS_STANDARD_PLATFORM) && (defined(NAPI_TARGET_ARM64) || defined(NAPI_TARGET_ARM))
/* dfx interface, type 0 for string */
extern "C" uintptr_t DFX_SetCrashObj(uint8_t type, uintptr_t addr);
extern "C" uintptr_t DFX_ResetCrashObj(uintptr_t addr);
#endif

constexpr size_t FORMATED_FUNCPTR_LENGTH = 24; // length of dec function pointer

CrashHolder::~CrashHolder()
{
#if defined(OHOS_STANDARD_PLATFORM) && (defined(NAPI_TARGET_ARM64) || defined(NAPI_TARGET_ARM))
    DFX_ResetCrashObj(handle_);
#endif
    if (data_ != nullptr) {
        delete [] data_;
        data_ = nullptr;
    }
}

void CrashHolder::SetCrashObj(const char* info)
{
    std::string data = "[NAPI] Crash occured on ";
    data += info;
    data += ", callback: ";

    size_ = data.length();
    const size_t bufSize = size_ + FORMATED_FUNCPTR_LENGTH;
    data_ = new char[bufSize];

    if (memcpy_s(data_, bufSize, data.c_str(), size_) != EOK) {
        HILOG_WARN("Failed to init crash holder.");
        size_ = 0;
        data_[0] = '\0';
    };

#if defined(OHOS_STANDARD_PLATFORM) && (defined(NAPI_TARGET_ARM64) || defined(NAPI_TARGET_ARM))
    handle_ = DFX_SetCrashObj(static_cast<uint8_t>(DFXObjectType::STRING),
                              reinterpret_cast<uintptr_t>(data_));
#else
    handle_ = 0;
#endif
}

void CrashHolder::UpdateCallbackPtr(uintptr_t addr)
{
    if (sprintf_s(data_ + size_, FORMATED_FUNCPTR_LENGTH, "%" PRIuPTR, addr) < 0) {
        HILOG_ERROR("Failed to update callback info: %{public}" PRIuPTR, addr);
    }
}
