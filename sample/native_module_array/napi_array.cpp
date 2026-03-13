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
static constexpr int REQUIRED_ARGS_THREE = 3;
static constexpr uint32_t MAX_ARRAY_SIZE = 1048576;

static napi_value CreateEmptyArray(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_array(env, &result));

    return result;
}

static napi_value CreateArrayWithLength(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_number, "Length must be a number");

    double doubleValue = 0;
    NAPI_CALL(env, napi_get_value_double(env, argv[0], &doubleValue));
    uint32_t length = static_cast<uint32_t>(doubleValue);
    NAPI_ASSERT(env, length <= MAX_ARRAY_SIZE, "Length exceeds maximum");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, length, &result));

    return result;
}

static napi_value GetArrayLength(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Argument must be an array");

    uint32_t length = 0;
    NAPI_CALL(env, napi_get_array_length(env, argv[0], &length));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_uint32(env, length, &result));

    return result;
}

static napi_value GetArrayElement(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments");

    napi_valuetype arrType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &arrType));
    NAPI_ASSERT(env, arrType == napi_object, "Array must be an object");

    napi_valuetype idxType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[1], &idxType));
    NAPI_ASSERT(env, idxType == napi_number, "Index must be a number");

    double doubleIndex = 0;
    NAPI_CALL(env, napi_get_value_double(env, argv[1], &doubleIndex));
    uint32_t index = static_cast<uint32_t>(doubleIndex);

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_element(env, argv[0], index, &result));

    return result;
}

static napi_value SetArrayElement(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments");

    napi_valuetype arrType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &arrType));
    NAPI_ASSERT(env, arrType == napi_object, "Array must be an object");

    napi_valuetype idxType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[1], &idxType));
    NAPI_ASSERT(env, idxType == napi_number, "Index must be a number");

    double doubleIndex = 0;
    NAPI_CALL(env, napi_get_value_double(env, argv[1], &doubleIndex));
    uint32_t index = static_cast<uint32_t>(doubleIndex);

    NAPI_CALL(env, napi_set_element(env, argv[0], index, argv[REQUIRED_ARGS_TWO]));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &result));

    return result;
}

static napi_value DeleteArrayElement(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments");

    napi_valuetype arrType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &arrType));
    NAPI_ASSERT(env, arrType == napi_object, "Array must be an object");

    napi_valuetype idxType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[1], &idxType));
    NAPI_ASSERT(env, idxType == napi_number, "Index must be a number");

    double doubleIndex = 0;
    NAPI_CALL(env, napi_get_value_double(env, argv[1], &doubleIndex));
    uint32_t index = static_cast<uint32_t>(doubleIndex);

    bool deleted = false;
    NAPI_CALL(env, napi_delete_element(env, argv[0], index, &deleted));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, deleted, &result));

    return result;
}

static napi_value IsArray(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[0], &isArray));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, isArray, &result));

    return result;
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("createEmptyArray", CreateEmptyArray),
        DECLARE_NAPI_FUNCTION("createArrayWithLength", CreateArrayWithLength),
        DECLARE_NAPI_FUNCTION("getArrayLength", GetArrayLength),
        DECLARE_NAPI_FUNCTION("getArrayElement", GetArrayElement),
        DECLARE_NAPI_FUNCTION("setArrayElement", SetArrayElement),
        DECLARE_NAPI_FUNCTION("deleteArrayElement", DeleteArrayElement),
        DECLARE_NAPI_FUNCTION("isArray", IsArray),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
                                      sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module arrayModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "array",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void ArrayRegisterModule(void)
{
    napi_module_register(&arrayModule);
}
