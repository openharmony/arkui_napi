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

#include <map>
#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARG_FIRST = 0;
constexpr size_t ARG_SECOND = 1;
}

static std::map<std::string, std::string> g_kvStore;

static napi_value Put(napi_env env, napi_callback_info info)
{
    size_t requireArgc = ARGC_TWO;
    size_t argc = ARGC_TWO;
    napi_value args[ARGC_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype keyType;
    NAPI_CALL(env, napi_typeof(env, args[ARG_FIRST], &keyType));
    NAPI_ASSERT(env, keyType == napi_string, "Key must be a string");

    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, args[ARG_SECOND], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Value must be a string");

    size_t keyLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[ARG_FIRST], nullptr, 0, &keyLen));

    std::string key(keyLen, '\0');
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[ARG_FIRST], &key[0], keyLen + 1, &keyLen));

    size_t valueLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[ARG_SECOND], nullptr, 0, &valueLen));

    std::string value(valueLen, '\0');
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[ARG_SECOND], &value[0], valueLen + 1, &valueLen));

    g_kvStore[key] = value;

    napi_value result;
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

static napi_value Get(napi_env env, napi_callback_info info)
{
    size_t requireArgc = ARGC_ONE;
    size_t argc = ARGC_ONE;
    napi_value args[ARGC_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype keyType;
    NAPI_CALL(env, napi_typeof(env, args[ARG_FIRST], &keyType));
    NAPI_ASSERT(env, keyType == napi_string, "Key must be a string");

    size_t keyLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[ARG_FIRST], nullptr, 0, &keyLen));

    std::string key(keyLen, '\0');
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[ARG_FIRST], &key[0], keyLen + 1, &keyLen));

    napi_value result;
    auto it = g_kvStore.find(key);
    if (it != g_kvStore.end()) {
        NAPI_CALL(env, napi_create_string_utf8(env, it->second.c_str(), NAPI_AUTO_LENGTH, &result));
    } else {
        NAPI_CALL(env, napi_get_undefined(env, &result));
    }

    return result;
}

static napi_value Delete(napi_env env, napi_callback_info info)
{
    size_t requireArgc = ARGC_ONE;
    size_t argc = ARGC_ONE;
    napi_value args[ARGC_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype keyType;
    NAPI_CALL(env, napi_typeof(env, args[ARG_FIRST], &keyType));
    NAPI_ASSERT(env, keyType == napi_string, "Key must be a string");

    size_t keyLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[ARG_FIRST], nullptr, 0, &keyLen));

    std::string key(keyLen, '\0');
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[ARG_FIRST], &key[0], keyLen + 1, &keyLen));

    napi_value result;
    size_t oldSize = g_kvStore.size();
    g_kvStore.erase(key);
    bool deleted = (g_kvStore.size() < oldSize);
    NAPI_CALL(env, napi_get_boolean(env, deleted, &result));

    return result;
}

static napi_value Clear(napi_env env, napi_callback_info info)
{
    g_kvStore.clear();

    napi_value result;
    NAPI_CALL(env, napi_get_undefined(env, &result));
    return result;
}

static napi_value Has(napi_env env, napi_callback_info info)
{
    size_t requireArgc = ARGC_ONE;
    size_t argc = ARGC_ONE;
    napi_value args[ARGC_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype keyType;
    NAPI_CALL(env, napi_typeof(env, args[ARG_FIRST], &keyType));
    NAPI_ASSERT(env, keyType == napi_string, "Key must be a string");

    size_t keyLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[ARG_FIRST], nullptr, 0, &keyLen));

    std::string key(keyLen, '\0');
    NAPI_CALL(env, napi_get_value_string_utf8(env, args[ARG_FIRST], &key[0], keyLen + 1, &keyLen));

    napi_value result;
    bool hasKey = g_kvStore.find(key) != g_kvStore.end();
    NAPI_CALL(env, napi_get_boolean(env, hasKey, &result));

    return result;
}

static napi_value Keys(napi_env env, napi_callback_info info)
{
    napi_value result;
    NAPI_CALL(env, napi_create_array(env, &result));

    uint32_t index = 0;
    for (const auto& pair : g_kvStore) {
        napi_value key;
        NAPI_CALL(env, napi_create_string_utf8(env, pair.first.c_str(), NAPI_AUTO_LENGTH, &key));
        NAPI_CALL(env, napi_set_element(env, result, index, key));
        index++;
    }

    return result;
}

static napi_value Size(napi_env env, napi_callback_info info)
{
    napi_value result;
    NAPI_CALL(env, napi_create_uint32(env, static_cast<uint32_t>(g_kvStore.size()), &result));
    return result;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("put", Put),
        DECLARE_NAPI_FUNCTION("get", Get),
        DECLARE_NAPI_FUNCTION("delete", Delete),
        DECLARE_NAPI_FUNCTION("clear", Clear),
        DECLARE_NAPI_FUNCTION("has", Has),
        DECLARE_NAPI_FUNCTION("keys", Keys),
        DECLARE_NAPI_FUNCTION("size", Size),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    return exports;
}
EXTERN_C_END

static napi_module kvstoreModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "kvstore",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void KvstoreRegisterModule(void)
{
    napi_module_register(&kvstoreModule);
}