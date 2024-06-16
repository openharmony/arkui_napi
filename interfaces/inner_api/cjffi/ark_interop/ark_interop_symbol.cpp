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

using namespace panda;
using namespace panda::ecmascript;

ARKTS_Value ARKTS_CreateSymbol(ARKTS_Env env, const char* description, int32_t length)
{
    ARKTS_ASSERT_P(env, "env is null");
    auto vm = P_CAST(env, EcmaVM*);

    panda::Local<panda::JSValueRef> object;

    if (!description || !length) {
        const char* str = "";
        object = panda::StringRef::NewFromUtf8(vm, str, 0);
    } else {
        object = panda::StringRef::NewFromUtf8(vm, description, length);
    }
    auto symbol = panda::SymbolRef::New(vm, object);
    return BIT_CAST(symbol, ARKTS_Value);
}

bool ARKTS_IsSymbol(ARKTS_Env env, ARKTS_Value value)
{
    auto tag = BIT_CAST(value, JSValueRef);
    if (!tag.IsHeapObject()) {
        return false;
    }
    tag = *BIT_CAST(value, JSValueRef*);
    auto vm = P_CAST(env, EcmaVM*);
    return tag.IsSymbol(vm);
}

const char* ARKTS_GetSymbolDesc(ARKTS_Env env, ARKTS_Value value)
{
    ARKTS_ASSERT_P(env, "env is null");
    ARKTS_ASSERT_P(ARKTS_IsSymbol(env, value), "value is not a symbol");

    auto vm = P_CAST(env, EcmaVM*);
    auto symbol = *P_CAST(value, SymbolRef*);
    auto desc = symbol.GetDescription(vm);
    auto desc1 = BIT_CAST(desc, ARKTS_Value);
    if (ARKTS_IsString(env, desc1)) {
        return ARKTS_GetValueCString(env, BIT_CAST(desc, ARKTS_Value));
    }
    return nullptr;
}
