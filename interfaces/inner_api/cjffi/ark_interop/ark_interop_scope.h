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

#ifndef NAPI_ARK_INTEROP_SCOPE_H
#define NAPI_ARK_INTEROP_SCOPE_H

#include <list>
#include <map>

#include "ark_interop_napi.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "ark_interop_internal.h"

namespace panda::ecmascript {
class EcmaVM;
}

struct ARKTS_Scope_ {
public:
    static void DisposeEnv(ARKTS_Env env);
    static ARKTS_Scope NewScope(ARKTS_Env env);
    static bool CloseScope(ARKTS_Scope target);
    static ARKTS_Value NormalPointer(void* pointer);
    EXPORT static ARKTS_Value NewValue(ARKTS_Env env, panda::Local<panda::JSValueRef> ref);
    EXPORT static panda::Local<panda::JSValueRef> GetLocal(ARKTS_Env env,
        ARKTS_Value value);
    static panda::JSValueRef GetValueRef(ARKTS_Value value);

private:
    struct ThreadScopes {
        ARKTS_Scope top {nullptr};
        ~ThreadScopes();
    };
    static ThreadScopes& GetThreadScopes(ARKTS_Env env);
    static ThreadScopes* GetThreadOpt(ARKTS_Env env);

    ARKTS_Scope_(ARKTS_Env env, ARKTS_Scope_* parent);

    std::list<panda::Local<panda::JSValueRef>> handledValues;
    const ARKTS_Env currentEnv;
    const ARKTS_Scope_* parentScope;
    std::optional<panda::LocalScope> scope;

    static std::mutex threadMutex;
    static std::map<ARKTS_Env, ThreadScopes> threads;
};

#endif //NAPI_ARK_INTEROP_SCOPE_H
