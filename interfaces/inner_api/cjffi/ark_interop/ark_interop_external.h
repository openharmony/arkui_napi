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

#ifndef NAPI_ARK_INTEROP_EXTERNAL_H
#define NAPI_ARK_INTEROP_EXTERNAL_H

#include "ecmascript/napi/include/jsnapi.h"
#include "ark_interop_napi.h"

// below interface will be export, but not available for user

DECL_START

EXPORT void ARKTS_SetCJModuleCallback(struct ARKTS_ModuleCallbacks* callback);
EXPORT panda::JSValueRef* ARKTS_LoadModule(ARKTS_Env env, const char* dllName);

DECL_END

#endif // NAPI_ARK_INTEROP_EXTERNAL_H
