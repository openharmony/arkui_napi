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

class DataProtector {
public:
    DataProtector() : encryptedAddrOrData(0) {}
    explicit DataProtector(const uintptr_t pointer)
    {
        Update(pointer);
    }

    ~DataProtector() = default;

    static uintptr_t AutDecrypt(const uintptr_t pointer, [[maybe_unused]]const uintptr_t address);
    static uintptr_t PacEncrypt(const uintptr_t pointer, [[maybe_unused]]const uintptr_t address);

    void Update(const uintptr_t pointer)
    {
#if defined(NAPI_ENABLE_DATA_PROTECT)
        if (pointer == 0) {
            encryptedAddrOrData = 0;
            return;
        }
        encryptedAddrOrData = PacEncrypt(pointer, reinterpret_cast<uintptr_t>(&encryptedAddrOrData));
#else
        encryptedAddrOrData = pointer;
#endif
    }

    uintptr_t GetData() const
    {
#if defined(NAPI_ENABLE_DATA_PROTECT)
        if (encryptedAddrOrData == 0) {
            return encryptedAddrOrData;
        }
        return AutDecrypt(encryptedAddrOrData, reinterpret_cast<uintptr_t>(&encryptedAddrOrData));
#else
        return encryptedAddrOrData;
#endif
    }
    
private:
    uintptr_t encryptedAddrOrData;
};
#endif
