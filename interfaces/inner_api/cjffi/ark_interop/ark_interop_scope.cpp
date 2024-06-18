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

#include "ark_interop_scope.h"

namespace CJ {
std::vector<ARKTS_ScopeManager> ARKTS_ScopeManager::scopeStack_{};

ARKTS_Scope ARKTS_ScopeManager::OpenScope(panda::ecmascript::EcmaVM* vm)
{
    scopeStack_.emplace_back(vm);
    size_t scopeId = scopeStack_.size();
    return P_CAST(scopeId, ARKTS_Scope);
}

bool ARKTS_ScopeManager::CloseScope(ARKTS_Scope target)
{
    auto targetScope = P_CAST(target, int64_t);
    if (targetScope > (int64_t)scopeStack_.size()) {
        return false;
    }
    while ((int64_t)scopeStack_.size() >= targetScope) {
        scopeStack_.pop_back();
    }
    return true;
}

ARKTS_ScopeManager::ARKTS_ScopeManager(panda::ecmascript::EcmaVM* vm): scope_(vm)
{
}
}