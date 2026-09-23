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

// scan-dir sample: requireNapi arg is "app.ability.scandirdemo" (dots become directories),
// so the .so must sit at module/app/ability/libscandirdemo.so (the driver script prepares
// this layout).
static napi_value Ping(napi_env env, napi_callback_info /* info */)
{
    napi_value ret = nullptr;
    napi_create_string_utf8(env, "pong", NAPI_AUTO_LENGTH, &ret);
    return ret;
}

static napi_value ModuleInit(napi_env env, napi_value exports)
{
    napi_value ping = nullptr;
    napi_create_function(env, "ping", NAPI_AUTO_LENGTH, Ping, nullptr, &ping);
    napi_set_named_property(env, exports, "ping", ping);

    napi_value level = nullptr;
    napi_create_uint32(env, 1, &level);
    napi_set_named_property(env, exports, "level", level);
    return exports;
}

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = ModuleInit,
    .nm_modname = "app.ability.scandirdemo",
    .nm_priv = nullptr,
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterCheckApiScanDirDemoModule(void)
{
    napi_module_register(&demoModule);
}
