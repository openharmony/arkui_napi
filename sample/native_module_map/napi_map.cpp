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
#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

static constexpr int REQUIRED_ARGS_ONE = 1;
static constexpr int REQUIRED_ARGS_TWO = 2;
static constexpr int REQUIRED_ARGS_THREE = 3;
static constexpr size_t MAX_MAP_SIZE = 10000;
static constexpr size_t MAX_KEY_LENGTH = 1000;
static constexpr size_t MAX_VALUE_LENGTH = 5000;

static napi_value CreateEmptyMap(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    return result;
}

static napi_value CreateMapFromObject(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Argument must be an object");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value propertyNames = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, argv[0], &propertyNames));

    uint32_t propertyCount = 0;
    NAPI_CALL(env, napi_get_array_length(env, propertyNames, &propertyCount));

    for (uint32_t i = 0; i < propertyCount; i++) {
        napi_value key = nullptr;
        NAPI_CALL(env, napi_get_element(env, propertyNames, i, &key));

        napi_value value = nullptr;
        NAPI_CALL(env, napi_get_property(env, argv[0], key, &value));

        NAPI_CALL(env, napi_set_property(env, result, key, value));
    }

    return result;
}

static napi_value SetMapValue(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments");

    napi_valuetype mapType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &mapType));
    NAPI_ASSERT(env, mapType == napi_object, "Map must be an object");

    napi_valuetype keyType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[1], &keyType));
    NAPI_ASSERT(env, keyType == napi_string, "Key must be a string");

    size_t keyLength = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[1], nullptr, 0, &keyLength));
    NAPI_ASSERT(env, keyLength <= MAX_KEY_LENGTH, "Key length exceeds maximum");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_set_property(env, argv[0], argv[1], argv[REQUIRED_ARGS_TWO]));

    NAPI_CALL(env, napi_get_undefined(env, &result));

    return result;
}

static napi_value GetMapValue(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments");

    napi_valuetype mapType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &mapType));
    NAPI_ASSERT(env, mapType == napi_object, "Map must be an object");

    napi_valuetype keyType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[1], &keyType));
    NAPI_ASSERT(env, keyType == napi_string, "Key must be a string");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_property(env, argv[0], argv[1], &result));

    return result;
}

static napi_value DeleteMapValue(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments");

    napi_valuetype mapType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &mapType));
    NAPI_ASSERT(env, mapType == napi_object, "Map must be an object");

    napi_valuetype keyType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[1], &keyType));
    NAPI_ASSERT(env, keyType == napi_string, "Key must be a string");

    bool deleted = false;
    NAPI_CALL(env, napi_delete_property(env, argv[0], argv[1], &deleted));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, deleted, &result));

    return result;
}

static napi_value HasMapKey(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments");

    napi_valuetype mapType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &mapType));
    NAPI_ASSERT(env, mapType == napi_object, "Map must be an object");

    napi_valuetype keyType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[1], &keyType));
    NAPI_ASSERT(env, keyType == napi_string, "Key must be a string");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_has_property(env, argv[0], argv[1], &result));

    return result;
}

static napi_value GetMapSize(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype mapType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &mapType));
    NAPI_ASSERT(env, mapType == napi_object, "Map must be an object");

    napi_value propertyNames = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, argv[0], &propertyNames));

    uint32_t propertyCount = 0;
    NAPI_CALL(env, napi_get_array_length(env, propertyNames, &propertyCount));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_uint32(env, propertyCount, &result));

    return result;
}

static napi_value ClearMap(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype mapType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &mapType));
    NAPI_ASSERT(env, mapType == napi_object, "Map must be an object");

    napi_value propertyNames = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, argv[0], &propertyNames));

    uint32_t propertyCount = 0;
    NAPI_CALL(env, napi_get_array_length(env, propertyNames, &propertyCount));

    for (uint32_t i = 0; i < propertyCount; i++) {
        napi_value key = nullptr;
        NAPI_CALL(env, napi_get_element(env, propertyNames, i, &key));

        NAPI_CALL(env, napi_delete_property(env, argv[0], key, nullptr));
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &result));

    return result;
}

static napi_value GetMapKeys(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype mapType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &mapType));
    NAPI_ASSERT(env, mapType == napi_object, "Map must be an object");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, argv[0], &result));

    return result;
}

static napi_value GetMapValues(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype mapType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &mapType));
    NAPI_ASSERT(env, mapType == napi_object, "Map must be an object");

    napi_value propertyNames = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, argv[0], &propertyNames));

    uint32_t propertyCount = 0;
    NAPI_CALL(env, napi_get_array_length(env, propertyNames, &propertyCount));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, propertyCount, &result));

    for (uint32_t i = 0; i < propertyCount; i++) {
        napi_value key = nullptr;
        NAPI_CALL(env, napi_get_element(env, propertyNames, i, &key));

        napi_value value = nullptr;
        NAPI_CALL(env, napi_get_property(env, argv[0], key, &value));

        NAPI_CALL(env, napi_set_element(env, result, i, value));
    }

    return result;
}

static napi_value GetMapEntries(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype mapType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &mapType));
    NAPI_ASSERT(env, mapType == napi_object, "Map must be an object");

    napi_value propertyNames = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, argv[0], &propertyNames));

    uint32_t propertyCount = 0;
    NAPI_CALL(env, napi_get_array_length(env, propertyNames, &propertyCount));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, propertyCount, &result));

    for (uint32_t i = 0; i < propertyCount; i++) {
        napi_value key = nullptr;
        NAPI_CALL(env, napi_get_element(env, propertyNames, i, &key));

        napi_value value = nullptr;
        NAPI_CALL(env, napi_get_property(env, argv[0], key, &value));

        napi_value entry = nullptr;
        NAPI_CALL(env, napi_create_array_with_length(env, REQUIRED_ARGS_TWO, &entry));

        NAPI_CALL(env, napi_set_element(env, entry, 0, key));
        NAPI_CALL(env, napi_set_element(env, entry, 1, value));

        NAPI_CALL(env, napi_set_element(env, result, i, entry));
    }

    return result;
}

static napi_value ForEachMap(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments");

    napi_valuetype mapType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &mapType));
    NAPI_ASSERT(env, mapType == napi_object, "Map must be an object");

    napi_valuetype callbackType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[1], &callbackType));
    NAPI_ASSERT(env, callbackType == napi_function, "Callback must be a function");

    napi_value propertyNames = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, argv[0], &propertyNames));

    uint32_t propertyCount = 0;
    NAPI_CALL(env, napi_get_array_length(env, propertyNames, &propertyCount));

    for (uint32_t i = 0; i < propertyCount; i++) {
        napi_value key = nullptr;
        NAPI_CALL(env, napi_get_element(env, propertyNames, i, &key));

        napi_value value = nullptr;
        NAPI_CALL(env, napi_get_property(env, argv[0], key, &value));

        napi_value callbackArgs[REQUIRED_ARGS_TWO] = { value, key };
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_call_function(env, argv[0], argv[1], REQUIRED_ARGS_TWO,
                                           callbackArgs, &callbackResult));
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &result));

    return result;
}

static napi_value MergeMaps(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments");

    napi_valuetype map1Type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &map1Type));
    NAPI_ASSERT(env, map1Type == napi_object, "First argument must be an object");

    napi_valuetype map2Type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[1], &map2Type));
    NAPI_ASSERT(env, map2Type == napi_object, "Second argument must be an object");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value propertyNames1 = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, argv[0], &propertyNames1));

    uint32_t count1 = 0;
    NAPI_CALL(env, napi_get_array_length(env, propertyNames1, &count1));

    for (uint32_t i = 0; i < count1; i++) {
        napi_value key = nullptr;
        NAPI_CALL(env, napi_get_element(env, propertyNames1, i, &key));

        napi_value value = nullptr;
        NAPI_CALL(env, napi_get_property(env, argv[0], key, &value));

        NAPI_CALL(env, napi_set_property(env, result, key, value));
    }

    napi_value propertyNames2 = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, argv[1], &propertyNames2));

    uint32_t count2 = 0;
    NAPI_CALL(env, napi_get_array_length(env, propertyNames2, &count2));

    for (uint32_t i = 0; i < count2; i++) {
        napi_value key = nullptr;
        NAPI_CALL(env, napi_get_element(env, propertyNames2, i, &key));

        napi_value value = nullptr;
        NAPI_CALL(env, napi_get_property(env, argv[1], key, &value));

        NAPI_CALL(env, napi_set_property(env, result, key, value));
    }

    return result;
}

static napi_value CloneMap(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype mapType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &mapType));
    NAPI_ASSERT(env, mapType == napi_object, "Map must be an object");

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value propertyNames = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, argv[0], &propertyNames));

    uint32_t propertyCount = 0;
    NAPI_CALL(env, napi_get_array_length(env, propertyNames, &propertyCount));

    for (uint32_t i = 0; i < propertyCount; i++) {
        napi_value key = nullptr;
        NAPI_CALL(env, napi_get_element(env, propertyNames, i, &key));

        napi_value value = nullptr;
        NAPI_CALL(env, napi_get_property(env, argv[0], key, &value));

        NAPI_CALL(env, napi_set_property(env, result, key, value));
    }

    return result;
}

static napi_value IsMapEmpty(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype mapType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &mapType));
    NAPI_ASSERT(env, mapType == napi_object, "Map must be an object");

    napi_value propertyNames = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, argv[0], &propertyNames));

    uint32_t propertyCount = 0;
    NAPI_CALL(env, napi_get_array_length(env, propertyNames, &propertyCount));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, propertyCount == 0, &result));

    return result;
}

static napi_value GetMapKeyValues(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype mapType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &mapType));
    NAPI_ASSERT(env, mapType == napi_object, "Map must be an object");

    napi_value propertyNames = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, argv[0], &propertyNames));

    uint32_t propertyCount = 0;
    NAPI_CALL(env, napi_get_array_length(env, propertyNames, &propertyCount));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    for (uint32_t i = 0; i < propertyCount; i++) {
        napi_value key = nullptr;
        NAPI_CALL(env, napi_get_element(env, propertyNames, i, &key));

        napi_value value = nullptr;
        NAPI_CALL(env, napi_get_property(env, argv[0], key, &value));

        NAPI_CALL(env, napi_set_property(env, result, key, value));
    }

    return result;
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("createEmptyMap", CreateEmptyMap),
        DECLARE_NAPI_FUNCTION("createMapFromObject", CreateMapFromObject),
        DECLARE_NAPI_FUNCTION("setMapValue", SetMapValue),
        DECLARE_NAPI_FUNCTION("getMapValue", GetMapValue),
        DECLARE_NAPI_FUNCTION("deleteMapValue", DeleteMapValue),
        DECLARE_NAPI_FUNCTION("hasMapKey", HasMapKey),
        DECLARE_NAPI_FUNCTION("getMapSize", GetMapSize),
        DECLARE_NAPI_FUNCTION("clearMap", ClearMap),
        DECLARE_NAPI_FUNCTION("getMapKeys", GetMapKeys),
        DECLARE_NAPI_FUNCTION("getMapValues", GetMapValues),
        DECLARE_NAPI_FUNCTION("getMapEntries", GetMapEntries),
        DECLARE_NAPI_FUNCTION("forEachMap", ForEachMap),
        DECLARE_NAPI_FUNCTION("mergeMaps", MergeMaps),
        DECLARE_NAPI_FUNCTION("cloneMap", CloneMap),
        DECLARE_NAPI_FUNCTION("isMapEmpty", IsMapEmpty),
        DECLARE_NAPI_FUNCTION("getMapKeyValues", GetMapKeyValues),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
                                      sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module mapModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "map",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void MapRegisterModule(void)
{
    napi_module_register(&mapModule);
}