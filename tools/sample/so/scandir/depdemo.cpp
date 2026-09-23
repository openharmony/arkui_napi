/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "napi/native_api.h"

// Deprecated-bucket sample: oldApi is in the frozen snapshot, newApi is not — newApi must
// land in DeprecatedModules, never in IllegalApis, and must not affect the exit code.
static napi_value OldApi(napi_env env, napi_callback_info /* info */)
{
    napi_value ret = nullptr;
    napi_get_undefined(env, &ret);
    return ret;
}

// Simulates an API added after deprecation (the frozen snapshot cannot contain it).
static napi_value NewApi(napi_env env, napi_callback_info /* info */)
{
    napi_value ret = nullptr;
    napi_get_undefined(env, &ret);
    return ret;
}

static napi_value ModuleInit(napi_env env, napi_value exports)
{
    napi_value oldApi = nullptr;
    napi_create_function(env, "oldApi", NAPI_AUTO_LENGTH, OldApi, nullptr, &oldApi);
    napi_set_named_property(env, exports, "oldApi", oldApi);

    napi_value newApi = nullptr;
    napi_create_function(env, "newApi", NAPI_AUTO_LENGTH, NewApi, nullptr, &newApi);
    napi_set_named_property(env, exports, "newApi", newApi);
    return exports;
}

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = ModuleInit,
    .nm_modname = "ability.depdemo",
    .nm_priv = nullptr,
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterCheckApiDepDemoModule(void)
{
    napi_module_register(&demoModule);
}
