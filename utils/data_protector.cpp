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

#include "data_protector.h"

#if defined(NAPI_ENABLE_DATA_PROTECT)
#include <sys/auxv.h>
#include <asm/hwcap.h>

uintptr_t DataProtector::AutDecrypt(const uintptr_t pointer, [[maybe_unused]]const uintptr_t address) const
{
    auto hwcaps = getauxval(AT_HWCAP);
    if (!(hwcaps & HWCAP_PACA)) {
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
}

uintptr_t DataProtector::PacEncrypt(const uintptr_t pointer, [[maybe_unused]]const uintptr_t address)
{
    auto hwcaps = getauxval(AT_HWCAP);
    if (!(hwcaps & HWCAP_PACA)) {
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
}
#else
uintptr_t DataProtector::AutDecrypt(const uintptr_t pointer, [[maybe_unused]]const uintptr_t address) const
{
    return pointer;
}

uintptr_t DataProtector::PacEncrypt(const uintptr_t pointer, [[maybe_unused]]const uintptr_t address)
{
    return pointer;
}
#endif