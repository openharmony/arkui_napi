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

// Accessor-traversal regression: accessors must be recorded as fields without reading their
// values (native prototype getters crash without an instance receiver). trapThrow's getter
// must never run.
static napi_value ThrowingGetter(napi_env env, napi_callback_info /* info */)
{
    napi_throw_error(env, nullptr, "poison getter");
    return nullptr;
}

static napi_value FuncGetter(napi_env env, napi_callback_info /* info */)
{
    napi_value func = nullptr;
    napi_create_function(env, "trapFuncValue", NAPI_AUTO_LENGTH, ThrowingGetter, nullptr, &func);
    return func;
}

static napi_value ModuleInit(napi_env env, napi_value exports)
{
    napi_value safe = nullptr;
    napi_create_uint32(env, 1, &safe);
    napi_set_named_property(env, exports, "safe", safe);

    napi_value after = nullptr;
    napi_create_string_utf8(env, "after", NAPI_AUTO_LENGTH, &after);
    napi_set_named_property(env, exports, "after", after);

    napi_property_descriptor accessors[] = {
        {"trapThrow", nullptr, nullptr, ThrowingGetter, nullptr, nullptr, napi_default, nullptr},
        {"trapFunc", nullptr, nullptr, FuncGetter, nullptr, nullptr, napi_default, nullptr},
    };
    napi_define_properties(env, exports, sizeof(accessors) / sizeof(accessors[0]), accessors);
    return exports;
}

static napi_module poisonModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = ModuleInit,
    .nm_modname = "poison",
    .nm_priv = nullptr,
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterCheckApiPoisonModule(void)
{
    napi_module_register(&poisonModule);
}
