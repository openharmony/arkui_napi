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
static constexpr uint32_t MAX_ARRAY_SIZE = 1000;

static napi_value CreateNumber(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_number, "Argument must be a number");

    double value = 0;
    NAPI_CALL(env, napi_get_value_double(env, argv[0], &value));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_double(env, value, &result));

    return result;
}

static napi_value CreateString(napi_env env, napi_callback_info info)
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

static napi_value CreateBoolean(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_boolean, "Argument must be a boolean");

    bool value = false;
    NAPI_CALL(env, napi_get_value_bool(env, argv[0], &value));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, value, &result));

    return result;
}

static napi_value CreateObject(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments");

    napi_valuetype keyType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &keyType));
    NAPI_ASSERT(env, keyType == napi_string, "Key must be a string");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    NAPI_CALL(env, napi_set_named_property(env, result, "key", argv[1]));

    return result;
}

static napi_value CreateArray(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_number, "Argument must be a number");

    double doubleValue = 0;
    NAPI_CALL(env, napi_get_value_double(env, argv[0], &doubleValue));
    uint32_t length = static_cast<uint32_t>(doubleValue);
    NAPI_ASSERT(env, length <= MAX_ARRAY_SIZE, "Array length must not exceed 1000");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, length, &result));

    for (uint32_t i = 0; i < length; i++) {
        napi_value element = nullptr;
        NAPI_CALL(env, napi_create_uint32(env, i, &element));
        NAPI_CALL(env, napi_set_element(env, result, i, element));
    }

    return result;
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("createNumber", CreateNumber),
        DECLARE_NAPI_FUNCTION("createString", CreateString),
        DECLARE_NAPI_FUNCTION("createBoolean", CreateBoolean),
        DECLARE_NAPI_FUNCTION("createObject", CreateObject),
        DECLARE_NAPI_FUNCTION("createArray", CreateArray),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
                                      sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module functionModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "function",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void FunctionRegisterModule(void)
{
    napi_module_register(&functionModule);
}
