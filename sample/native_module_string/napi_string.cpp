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

#include <cstddef>
#include <cstring>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

static constexpr int REQUIRED_ARGS_ONE = 1;

static napi_value CreateStringUtf8(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Argument must be a string");

    size_t bufferSize = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], nullptr, 0,
                                                    &bufferSize));

    char* buffer = new char[bufferSize + 1];
    size_t copiedLength = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], buffer,
                                                    bufferSize, &copiedLength));
    buffer[copiedLength] = '\0';

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, buffer, copiedLength,
                                                    &result));

    delete[] buffer;
    return result;
}

static napi_value CreateStringUtf16(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Argument must be a string");

    size_t bufferSize = 0;
    NAPI_CALL(env, napi_get_value_string_utf16(env, argv[0], nullptr, 0,
                                                      &bufferSize));

    char16_t* buffer = new char16_t[bufferSize + 1];
    size_t copiedLength = 0;
    NAPI_CALL(env, napi_get_value_string_utf16(env, argv[0], buffer,
                                                      bufferSize, &copiedLength));
    buffer[copiedLength] = '\0';

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf16(env, buffer, copiedLength,
                                                      &result));

    delete[] buffer;
    return result;
}

static napi_value GetStringLength(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Argument must be a string");

    size_t length = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], nullptr, 0, &length));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_uint32(env, static_cast<uint32_t>(length),
                                                    &result));

    return result;
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("createStringUtf8", CreateStringUtf8),
        DECLARE_NAPI_FUNCTION("createStringUtf16", CreateStringUtf16),
        DECLARE_NAPI_FUNCTION("getStringLength", GetStringLength),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
                                      sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module stringModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "string",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void StringRegisterModule(void)
{
    napi_module_register(&stringModule);
}
