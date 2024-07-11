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

#include "ark_interop_napi.h"
#include "ark_interop_external.h"
#include "ark_interop_internal.h"
#include "ark_interop_macro.h"
#include "ark_interop_log.h"

ARKTS_Value ARKTS_CreateBigInt(ARKTS_Env env, int64_t value)
{
    ARKTS_ASSERT_P(env, "env is null");
    
    auto vm = P_CAST(env, panda::EcmaVM*);
    auto result = panda::BigIntRef::New(vm, value);
    return BIT_CAST(result, ARKTS_Value);
}

static bool ReverseBytes(uint8_t dst[], size_t size)
{
    auto powOf2 = (size & 1) == 0;
    if (!powOf2) {
        return false;
    }
    auto half = size >> 1;
    for (size_t i = 0;i < half; ++i) {
        auto temp = dst[i];
        dst[i] = dst[size - i - 1];
        dst[size - i - 1] = temp;
    }
    return true;
}

constexpr int WORD_BYTES = sizeof(uint64_t) / sizeof(uint8_t);
constexpr int BYTE_BITS = 8;

ARKTS_Value ARKTS_CreateBigIntWithBytes(ARKTS_Env env, bool isNegative, int64_t size, const uint8_t bytes[])
{
    ARKTS_ASSERT_P(env, "env is null");
    ARKTS_ASSERT_P(size != 0 && size <= int64_t(0xFFFF'FFFF) * WORD_BYTES, "size is invalid");
    ARKTS_ASSERT_P(bytes, "bytes is null");
    
    auto vm = P_CAST(env, panda::EcmaVM*);
    
    auto firstBytes = size % WORD_BYTES;
    auto wholeU64Cnt = size / WORD_BYTES;
    auto totalCnt = firstBytes ? wholeU64Cnt + 1 : wholeU64Cnt;
    
    std::vector<uint64_t> u64v;
    u64v.resize(totalCnt);
    
    int64_t wordStart = 0;
    int64_t wordEnd = firstBytes;
    for (int64_t index = totalCnt - 1;index >= 0; --index) {
        uint64_t value = 0;
        for (auto i = wordStart;i < wordEnd; ++i) {
            value = (value << BYTE_BITS) | bytes[i];
        }
        u64v[index] = value;
        
        wordStart = wordEnd;
        wordEnd += WORD_BYTES;
    }

    auto result = panda::BigIntRef::CreateBigWords(vm, isNegative, totalCnt, u64v.data());
    return BIT_CAST(result, ARKTS_Value);
}

bool ARKTS_IsBigInt(ARKTS_Env env, ARKTS_Value value)
{
    auto tag = BIT_CAST(value, panda::JSValueRef);
    if (!tag.IsHeapObject()) {
        return false;
    }
    tag = *P_CAST(value, panda::JSValueRef*);
    auto vm = P_CAST(env, panda::EcmaVM*);
    return tag.IsBigInt(vm);
}

int64_t ARKTS_BigIntGetByteSize(ARKTS_Env env, ARKTS_Value value)
{
    ARKTS_ASSERT_I(ARKTS_IsBigInt(env, value), "value is not bigint");
    auto vm = P_CAST(env, panda::EcmaVM*);

    auto bigint = P_CAST(value, panda::BigIntRef*);
    return bigint->GetWordsArraySize(vm) * WORD_BYTES;
}

void ARKTS_BigIntReadBytes(ARKTS_Env env, ARKTS_Value value, bool* isNegative, int64_t byteCount, uint8_t bytes[])
{
    ARKTS_ASSERT_V(bytes, "bytes is null");
    ARKTS_ASSERT_V(ARKTS_IsBigInt(env, value), "value is not bigint");
    auto vm = P_CAST(env, panda::EcmaVM*);

    auto bigint = P_CAST(value, panda::BigIntRef*);
    auto u64cnt = bigint->GetWordsArraySize(vm);
    ARKTS_ASSERT_V(byteCount >= u64cnt * WORD_BYTES, "byteCount not enough");
    bigint->GetWordsArray(vm, isNegative, u64cnt, reinterpret_cast<uint64_t*>(bytes));
    ARKTS_ASSERT_V(ReverseBytes(bytes, byteCount), "ReverseBytes failed");
}