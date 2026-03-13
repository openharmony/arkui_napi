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

#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr size_t ARG_FIRST = 0;
constexpr size_t ARG_SECOND = 1;
constexpr size_t ARG_THIRD = 2;
}

static napi_value Parse(napi_env env, napi_callback_info info)
{
    size_t requireArgc = ARGC_ONE;
    size_t argc = ARGC_ONE;
    napi_value args[ARGC_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args[ARG_FIRST], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "Argument must be a string");

    size_t strLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[ARG_FIRST], nullptr, 0, &strLen));

    std::string jsonStr(strLen, '\0');
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[ARG_FIRST], &jsonStr[0], strLen + 1, &strLen));

    napi_value result;
    napi_status status = napi_json_parse(env, args[ARG_FIRST], &result);
    if (status != napi_ok) {
        NAPI_CALL(env, napi_throw_error(env, nullptr, "Failed to parse JSON string"));
        return nullptr;
    }

    return result;
}

static napi_value Stringify(napi_env env, napi_callback_info info)
{
    size_t requireArgc = ARGC_ONE;
    size_t argc = ARGC_ONE;
    napi_value args[ARGC_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_value result;
    napi_status status = napi_json_stringify(env, args[ARG_FIRST], &result);
    if (status != napi_ok) {
        NAPI_CALL(env, napi_throw_error(env, nullptr, "Failed to stringify JSON object"));
        return nullptr;
    }

    return result;
}

static napi_value GetProperty(napi_env env, napi_callback_info info)
{
    size_t requireArgc = ARGC_TWO;
    size_t argc = ARGC_TWO;
    napi_value args[ARGC_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype objType;
    NAPI_CALL(env, napi_typeof(env, args[ARG_FIRST], &objType));
    NAPI_ASSERT(env, objType == napi_object, "First argument must be an object");

    napi_valuetype keyType;
    NAPI_CALL(env, napi_typeof(env, args[ARG_SECOND], &keyType));
    NAPI_ASSERT(env, keyType == napi_string, "Second argument must be a string");

    size_t keyLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[ARG_SECOND], nullptr, 0, &keyLen));

    std::string key(keyLen, '\0');
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[ARG_SECOND], &key[0], keyLen + 1, &keyLen));

    napi_value result;
    NAPI_CALL(env, napi_get_named_property(env, args[ARG_FIRST], key.c_str(), &result));

    return result;
}

static napi_value SetProperty(napi_env env, napi_callback_info info)
{
    constexpr size_t argCount = ARGC_THREE;
    size_t requireArgc = argCount;
    size_t argc = argCount;
    napi_value args[argCount] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype objType;
    NAPI_CALL(env, napi_typeof(env, args[ARG_FIRST], &objType));
    NAPI_ASSERT(env, objType == napi_object, "First argument must be an object");

    napi_valuetype keyType;
    NAPI_CALL(env, napi_typeof(env, args[ARG_SECOND], &keyType));
    NAPI_ASSERT(env, keyType == napi_string, "Second argument must be a string");

    size_t keyLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[ARG_SECOND], nullptr, 0, &keyLen));

    std::string key(keyLen, '\0');
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[ARG_SECOND], &key[0], keyLen + 1, &keyLen));

    NAPI_CALL(env, napi_set_named_property(env, args[ARG_FIRST], key.c_str(), args[ARG_THIRD]));

    napi_value result;
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

static napi_value HasProperty(napi_env env, napi_callback_info info)
{
    size_t requireArgc = ARGC_TWO;
    size_t argc = ARGC_TWO;
    napi_value args[ARGC_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype objType;
    NAPI_CALL(env, napi_typeof(env, args[ARG_FIRST], &objType));
    NAPI_ASSERT(env, objType == napi_object, "First argument must be an object");

    napi_valuetype keyType;
    NAPI_CALL(env, napi_typeof(env, args[ARG_SECOND], &keyType));
    NAPI_ASSERT(env, keyType == napi_string, "Second argument must be a string");

    size_t keyLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[ARG_SECOND], nullptr, 0, &keyLen));

    std::string key(keyLen, '\0');
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[ARG_SECOND], &key[0], keyLen + 1, &keyLen));

    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, args[ARG_FIRST], key.c_str(), &hasProperty));

    napi_value result;
    NAPI_CALL(env, napi_get_boolean(env, hasProperty, &result));

    return result;
}

static napi_value DeleteProperty(napi_env env, napi_callback_info info)
{
    size_t requireArgc = ARGC_TWO;
    size_t argc = ARGC_TWO;
    napi_value args[ARGC_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype objType;
    NAPI_CALL(env, napi_typeof(env, args[ARG_FIRST], &objType));
    NAPI_ASSERT(env, objType == napi_object, "First argument must be an object");

    napi_valuetype keyType;
    NAPI_CALL(env, napi_typeof(env, args[ARG_SECOND], &keyType));
    NAPI_ASSERT(env, keyType == napi_string, "Second argument must be a string");

    size_t keyLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[ARG_SECOND], nullptr, 0, &keyLen));

    std::string key(keyLen, '\0');
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[ARG_SECOND], &key[0], keyLen + 1, &keyLen));

    bool result = false;
    NAPI_CALL(env, napi_delete_property(env, args[ARG_FIRST], args[ARG_SECOND], &result));

    napi_value ret;
    NAPI_CALL(env, napi_get_boolean(env, result, &ret));

    return ret;
}

static napi_value GetPropertyNames(napi_env env, napi_callback_info info)
{
    size_t requireArgc = ARGC_ONE;
    size_t argc = ARGC_ONE;
    napi_value args[ARGC_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype objType;
    NAPI_CALL(env, napi_typeof(env, args[ARG_FIRST], &objType));
    NAPI_ASSERT(env, objType == napi_object, "Argument must be an object");

    napi_value result;
    NAPI_CALL(env, napi_get_property_names(env, args[ARG_FIRST], &result));

    return result;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("parse", Parse),
        DECLARE_NAPI_FUNCTION("stringify", Stringify),
        DECLARE_NAPI_FUNCTION("getProperty", GetProperty),
        DECLARE_NAPI_FUNCTION("setProperty", SetProperty),
        DECLARE_NAPI_FUNCTION("hasProperty", HasProperty),
        DECLARE_NAPI_FUNCTION("deleteProperty", DeleteProperty),
        DECLARE_NAPI_FUNCTION("getPropertyNames", GetPropertyNames),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    return exports;
}
EXTERN_C_END

static napi_module jsonModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "json",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void JsonRegisterModule(void)
{
    napi_module_register(&jsonModule);
}