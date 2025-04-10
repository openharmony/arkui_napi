/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FOUNDATION_ACE_NAPI_UTILS_DATA_PROTECTOR_H
#define FOUNDATION_ACE_NAPI_UTILS_DATA_PROTECTOR_H

#include <cstdint>
#include "utils/macros.h"

class DataProtector {
public:
    DataProtector() : encryptedAddrOrData(0) {}
    explicit DataProtector(const uintptr_t pointer)
    {
        Update(pointer);
    }

    ~DataProtector() = default;

    NAPI_EXPORT uintptr_t AutDecrypt(const uintptr_t pointer, [[maybe_unused]]const uintptr_t address) const;
    NAPI_EXPORT uintptr_t PacEncrypt(const uintptr_t pointer, [[maybe_unused]]const uintptr_t address);

    void Update(const uintptr_t pointer)
    {
        if (pointer == 0) {
            encryptedAddrOrData = 0;
            return;
        }
        encryptedAddrOrData = PacEncrypt(pointer, reinterpret_cast<uintptr_t>(&encryptedAddrOrData));
    }

    uintptr_t GetData() const
    {
        if (encryptedAddrOrData == 0) {
            return encryptedAddrOrData;
        }
        return AutDecrypt(encryptedAddrOrData, reinterpret_cast<uintptr_t>(&encryptedAddrOrData));
    }
    
private:
    uintptr_t encryptedAddrOrData;
};
#endif
