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
#include <cstdint>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

static constexpr int REQUIRED_ARGS_ONE = 1;
static constexpr int REQUIRED_ARGS_TWO = 2;

static napi_value ThrowError(napi_env env, napi_callback_info info)
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

    NAPI_CALL(env, napi_throw_error(env, nullptr, buffer));

    delete[] buffer;

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &result));

    return result;
}

static napi_value ThrowBusinessError(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments");

    napi_valuetype msgType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &msgType));
    NAPI_ASSERT(env, msgType == napi_string, "Message must be a string");

    napi_valuetype codeType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[1], &codeType));
    NAPI_ASSERT(env, codeType == napi_number, "Code must be a number");

    size_t bufferSize = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], nullptr, 0,
                                                    &bufferSize));

    char* buffer = new char[bufferSize + 1];
    size_t copiedLength = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], buffer,
                                                    bufferSize, &copiedLength));
    buffer[copiedLength] = '\0';

    int32_t code = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[1], &code));

    NAPI_CALL(env, napi_throw_business_error(env, code, buffer));

    delete[] buffer;

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &result));

    return result;
}

static napi_value IsError(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    bool isError = false;
    NAPI_CALL(env, napi_is_error(env, argv[0], &isError));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, isError, &result));

    return result;
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("throwError", ThrowError),
        DECLARE_NAPI_FUNCTION("throwBusinessError", ThrowBusinessError),
        DECLARE_NAPI_FUNCTION("isError", IsError),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
                                      sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module errorModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "error",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void ErrorRegisterModule(void)
{
    napi_module_register(&errorModule);
}
