/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with License.
 * You may obtain a copy of License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

static constexpr int REQUIRED_ARGS_ONE = 1;
static constexpr int REQUIRED_ARGS_TWO = 2;
static constexpr int ARG_INDEX_ZERO = 0;
static constexpr int ARG_INDEX_ONE = 1;

static napi_value Stringify(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ZERO], &type));

    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value json = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "JSON", &json));

    napi_value stringifyFunc = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, json, "stringify", &stringifyFunc));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_call_function(env, json, stringifyFunc, argc, argv, &result));
    return result;
}

static napi_value Parse(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: json string");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ZERO], &type));
    NAPI_ASSERT(env, type == napi_string, "Argument must be a string");

    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value json = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "JSON", &json));

    napi_value parseFunc = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, json, "parse", &parseFunc));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_call_function(env, json, parseFunc, argc, argv, &result));
    return result;
}

static napi_value IsJsonObject(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ZERO], &type));

    bool isObject = (type == napi_object);
    if (isObject) {
        bool isArray = false;
        NAPI_CALL(env, napi_is_array(env, argv[ARG_INDEX_ZERO], &isArray));
        isObject = !isArray;
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, isObject, &result));
    return result;
}

static napi_value IsJsonArray(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[ARG_INDEX_ZERO], &isArray));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, isArray, &result));
    return result;
}

static napi_value GetJsonValue(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: object, key");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ZERO], &type));
    NAPI_ASSERT(env, type == napi_object, "First argument must be an object");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_property(env, argv[ARG_INDEX_ZERO], argv[ARG_INDEX_ONE], &result));
    return result;
}

static napi_value SetJsonValue(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: object, key");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ZERO], &type));
    NAPI_ASSERT(env, type == napi_object, "First argument must be an object");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

static napi_value GetJsonArrayLength(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: array");

    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[ARG_INDEX_ZERO], &isArray));
    NAPI_ASSERT(env, isArray, "Argument must be an array");

    uint32_t length = 0;
    NAPI_CALL(env, napi_get_array_length(env, argv[ARG_INDEX_ZERO], &length));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_uint32(env, length, &result));
    return result;
}

static napi_value GetJsonArrayElement(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: array, index");

    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[ARG_INDEX_ZERO], &isArray));
    NAPI_ASSERT(env, isArray, "First argument must be an array");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[ARG_INDEX_ONE], &type));
    NAPI_ASSERT(env, type == napi_number, "Second argument must be a number");

    uint32_t index = 0;
    NAPI_CALL(env, napi_get_value_uint32(env, argv[ARG_INDEX_ONE], &index));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_element(env, argv[ARG_INDEX_ZERO], index, &result));
    return result;
}

static napi_value CreateJsonObject(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    return result;
}

static napi_value CreateJsonArray(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_array(env, &result));
    return result;
}

static napi_value PushJsonArrayElement(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: array, element");

    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[ARG_INDEX_ZERO], &isArray));
    NAPI_ASSERT(env, isArray, "First argument must be an array");

    uint32_t length = 0;
    NAPI_CALL(env, napi_get_array_length(env, argv[ARG_INDEX_ZERO], &length));

    NAPI_CALL(env, napi_set_element(env, argv[ARG_INDEX_ZERO], length, argv[ARG_INDEX_ONE]));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("stringify", Stringify),
        DECLARE_NAPI_FUNCTION("parse", Parse),
        DECLARE_NAPI_FUNCTION("isObject", IsJsonObject),
        DECLARE_NAPI_FUNCTION("isArray", IsJsonArray),
        DECLARE_NAPI_FUNCTION("get", GetJsonValue),
        DECLARE_NAPI_FUNCTION("set", SetJsonValue),
        DECLARE_NAPI_FUNCTION("getArrayLength", GetJsonArrayLength),
        DECLARE_NAPI_FUNCTION("getArrayElement", GetJsonArrayElement),
        DECLARE_NAPI_FUNCTION("createObject", CreateJsonObject),
        DECLARE_NAPI_FUNCTION("createArray", CreateJsonArray),
        DECLARE_NAPI_FUNCTION("pushArrayElement", PushJsonArrayElement),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
                                      sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module jsonSuiteModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "jsonSuite",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void JsonSuiteRegisterModule(void)
{
    napi_module_register(&jsonSuiteModule);
}