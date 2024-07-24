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

#include <cstdint>
#include <cassert>
#include <cstdlib>
#include <vector>

using namespace panda;
using namespace panda::ecmascript;

ARKTS_Value ARKTS_CreateUtf8(ARKTS_Env env, const char* value, int32_t size)
{
    ARKTS_ASSERT_P(env, "env is null");
    ARKTS_ASSERT_P(value, "value is null");

    auto vm = P_CAST(env, EcmaVM*);
    Local<JSValueRef> result = StringRef::NewFromUtf8(vm, value, size);

    return ARKTS_FromHandle(result);
}

bool ARKTS_IsString(ARKTS_Env env, ARKTS_Value value)
{
    auto v = BIT_CAST(value, JSValueRef);
    if (!v.IsHeapObject()) {
        return false;
    }
    v = *P_CAST(value, JSValueRef*);
    auto vm = P_CAST(env, EcmaVM*);
    return v.IsString(vm);
}

int32_t ARKTS_GetValueUtf8Size(ARKTS_Env env, ARKTS_Value value)
{
    ARKTS_ASSERT_I(env, "env is null");
    ARKTS_ASSERT_I(ARKTS_IsString(env, value), "not a string");
    auto vm = P_CAST(env, EcmaVM*);
    auto v = BIT_CAST(value, Local<StringRef>);
    return v->Utf8Length(vm, true);
}

int32_t ARKTS_GetValueUtf8(ARKTS_Env env, ARKTS_Value value, int32_t capacity, char* buffer)
{
    ARKTS_ASSERT_I(ARKTS_IsString(env, value), "not a string");
    auto vm = P_CAST(env, EcmaVM*);
    auto v = BIT_CAST(value, Local<StringRef>);
    return v->WriteUtf8(vm, buffer, capacity, true);
}

const char* ARKTS_GetValueCString(ARKTS_Env env, ARKTS_Value value)
{
    ARKTS_ASSERT_I(env, "env is null");
    ARKTS_ASSERT_I(ARKTS_IsString(env, value), "not a string");
    auto vm = P_CAST(env, EcmaVM*);
    auto v = BIT_CAST(value, Local<StringRef>);
    auto size = v->Utf8Length(vm, true);
    if (size <= 0) {
        return nullptr;
    }
    auto result = (char*) malloc(sizeof(char) * size);
    if (!result) {
        LOGE("ARKTS_GetValueCString fail, out of memory");
        return nullptr;
    }
    v->WriteUtf8(vm, result, size, true);
    return result;
}

void ARKTS_FreeCString(const char* src)
{
    free(reinterpret_cast<void*>(const_cast<char*>(src)));
}
