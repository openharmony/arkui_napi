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

#ifndef FOUNDATION_ACE_NAPI_UTILS_DATA_PROTECT_H
#define FOUNDATION_ACE_NAPI_UTILS_DATA_PROTECT_H

#include <cstdint>
#if defined(NAPI_ENABLE_DATA_PROTECT)
#include <sys/auxv.h>
#include <asm/hwcap.h>
#endif

class DataProtect {
public:
    DataProtect() : encryptedAddrOrData(0) {}
    explicit DataProtect(const uintptr_t pointer)
    {
        if (pointer == 0) {
            encryptedAddrOrData = 0;
            return;
        }
        encryptedAddrOrData = PacEncrypt(pointer, reinterpret_cast<uintptr_t>(&encryptedAddrOrData));
    }

    ~DataProtect() = default;

    static inline uintptr_t AutDecrypt(const uintptr_t pointer, [[maybe_unused]]const uintptr_t address)
    {
#if defined(NAPI_ENABLE_DATA_PROTECT)
        uint64_t hwcaps = getauxval(AT_HWCAP);
        if (!(hwcaps & HWCAP_PACA) || !(hwcaps & HWCAP_PACG)) {
            return pointer;
        }
        void *t1 = reinterpret_cast<void*>(pointer);
        void *t2 = reinterpret_cast<void*>(address);
#ifdef PAC_DFI_PTR_BKEY
        __asm__ __volatile__("autdb %0, %1":"+r"(t1):"r"(t2):);
#else
        __asm__ __volatile__("autda %0, %1":"+r"(t1):"r"(t2):);
#endif
        return reinterpret_cast<uintptr_t>(t1);
#else
        return pointer;
#endif
    }

    static inline uintptr_t PacEncrypt(const uintptr_t pointer, [[maybe_unused]]const uintptr_t address)
    {
#if defined(NAPI_ENABLE_DATA_PROTECT)
        uint64_t hwcaps = getauxval(AT_HWCAP);
        if (!(hwcaps & HWCAP_PACA) || !(hwcaps & HWCAP_PACG)) {
            return pointer;
        }
        void *t1 = reinterpret_cast<void*>(pointer);
        void *t2 = reinterpret_cast<void*>(address);
#ifdef PAC_DFI_PTR_BKEY
        __asm__ __volatile__("pacdb %0, %1":"+r"(t1):"r"(t2):);
#else
        __asm__ __volatile__("pacda %0, %1":"+r"(t1):"r"(t2):);
#endif
        return reinterpret_cast<uintptr_t>(t1);
#else
        return pointer;
#endif
    }

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
