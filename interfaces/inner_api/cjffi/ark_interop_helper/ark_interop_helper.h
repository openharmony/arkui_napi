/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef NAPI_ARK_INTEROP_HELPER_H
#define NAPI_ARK_INTEROP_HELPER_H

#include "ark_interop_napi.h"
#include "napi/native_api.h"

#include <cstdint>

using ContainerScope = class CustomScope*;

extern "C" {
EXPORT napi_value ArkTsValuetoNapiValue(napi_env env, ARKTS_Value arkValue);
EXPORT ARKTS_Value NapiValueToArkTsValue(napi_value value);
EXPORT bool IsStageMode(napi_env env, napi_value context);
EXPORT napi_env GetGlobalNapiEnv();
EXPORT void SetGlobalNapiEnv(napi_env env);
EXPORT void* GetContextStageMode(napi_env env, napi_value context);
EXPORT int32_t ARKTS_GetCurrentContainerId();
EXPORT ContainerScope ARKTS_CreateContainerScope(int32_t);
EXPORT void ARKTS_DestroyContainerScope(ContainerScope scope);
EXPORT void ARKTS_EnterContainerScope(ContainerScope scope);
EXPORT void ARKTS_ExitContainerScope(ContainerScope scope);
}

#endif // NAPI_ARK_INTEROP_HELPER_H