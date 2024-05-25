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

#include <vector>

#include "ark_interop_napi.h"
#include "ecmascript/napi/include/jsnapi.h"

namespace panda::ecmascript {
class EcmaVM;
}

namespace CJ {
class ARKTS_ScopeManager final {
private:
    static std::vector<ARKTS_ScopeManager> scopeStack_;

public:
    static ARKTS_Scope OpenScope(panda::ecmascript::EcmaVM* vm);
    static bool CloseScope(ARKTS_Scope target);

    explicit ARKTS_ScopeManager(panda::ecmascript::EcmaVM* vm);

private:
    panda::LocalScope scope_;
};
}

#endif //NAPI_ARK_INTEROP_SCOPE_H
