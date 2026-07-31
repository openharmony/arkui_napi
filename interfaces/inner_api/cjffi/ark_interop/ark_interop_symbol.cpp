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
#include "ark_interop_scope.h"

using namespace panda;
using namespace panda::ecmascript;

ARKTS_Value ARKTS_CreateSymbol(ARKTS_Env env, const char* description, int32_t length)
{
    ARKTS_ASSERT_U(env, "env is null");
    auto vm = P_CAST(env, EcmaVM*);

    panda::Local<panda::JSValueRef> object;

    if (!description || !length) {
        const char* str = "";
        object = panda::StringRef::NewFromUtf8(vm, str, 0);
    } else {
        object = panda::StringRef::NewFromUtf8(vm, description, length);
    }
    auto symbol = panda::SymbolRef::New(vm, object);
    return ARKTS_Scope_::NewValue(env, symbol);
}

bool ARKTS_IsSymbol(ARKTS_Env env, ARKTS_Value value)
{
    ARKTS_ASSERT_F(env, "env is null");
    ARKTS_ASSERT_F(value.value, "value is null");
    auto tag = BIT_CAST(value, JSValueRef);
    if (!tag.IsHeapObject()) {
        return false;
    }
    auto vm = P_CAST(env, EcmaVM*);
    JsiFastNativeScope fastNativeScope(vm);
    auto handle = ARKTS_Scope_::GetLocal(env, value);
    return handle->IsSymbol(vm);
}

const char* ARKTS_GetSymbolDesc(ARKTS_Env env, ARKTS_Value value)
{
    ARKTS_ASSERT_P(ARKTS_IsSymbol(env, value), "value is not a symbol");

    auto vm = P_CAST(env, EcmaVM*);
    JsiFastNativeScope fastNativeScope(vm);
    Local<SymbolRef> symbol = ARKTS_Scope_::GetLocal(env, value);
    auto desc = symbol->GetDescription(vm);
    if (desc->IsString(vm)) {
        return ARKTS_GetValueCString(env, ARKTS_Scope_::NewValue(env, desc));
    }
    return nullptr;
}
