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
#include <string_view>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

constexpr uint32_t OBJECT_MAGIC_NUMBER = 0xDEADBEEF;
constexpr const std::string_view OBJECT_MAGIC_PROPERTY = "val";

// Proxy set handler callback
// signature on arkts: set(target: T, p: string | symbol, value: any, receiver: any) => boolean
constexpr size_t PROXY_SET_HANDLER_ARGC = 4;

static napi_value ProxySetHandler(napi_env env, napi_callback_info info)
{
    size_t argc = PROXY_SET_HANDLER_ARGC;
    napi_value argv[PROXY_SET_HANDLER_ARGC] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    napi_valuetype valueType;
    napi_typeof(env, argv[2], &valueType); // 2: index of target

    if (valueType == napi_undefined || valueType == napi_null) {
        // Log warning: attempt to assign property to undefined or null
        napi_value result = nullptr;
        napi_get_boolean(env, false, &result);
        return result;
    }

    napi_set_property(env, argv[0], argv[1], argv[2]); // 2: args of value

    napi_value result = nullptr;
    napi_get_boolean(env, true, &result);
    return result;
}

// Create proxy handler object
static napi_value MakeProxyHandler(napi_env env)
{
    napi_value handler = nullptr;
    napi_create_object(env, &handler);

    napi_value setFn = nullptr;
    napi_create_function(env, "set", NAPI_AUTO_LENGTH, ProxySetHandler, nullptr, &setFn);

    napi_set_named_property(env, handler, "set", setFn);

    return handler;
}

// Create target object with magic property
// Properties set by napi_set_property / napi_set_named_property are, by default,
// writable, enumerable, and configurable.
static napi_value CreateTargetObject(napi_env env)
{
    napi_value target = nullptr;
    napi_create_object(env, &target);

    napi_value magic = nullptr;
    napi_create_uint32(env, OBJECT_MAGIC_NUMBER, &magic);

    napi_set_named_property(env, target, OBJECT_MAGIC_PROPERTY.data(), magic);

    return target;
}

/*
   Create proxy case
   Use a Proxy to intercept and restrict property modifications
   when accessed through this proxy (e.g., from ArkTS code).
   Note: the original target object remains mutable if accessed directly.
   
   See: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Proxy
*/
static napi_value ProxyCase(napi_env env)
{
    napi_value target = CreateTargetObject(env);
    napi_value handler = MakeProxyHandler(env);

    napi_value global = nullptr;
    napi_get_global(env, &global);

    napi_value proxyConstructor = nullptr;
    napi_get_named_property(env, global, "Proxy", &proxyConstructor);

    constexpr size_t proxyConstructorArgc = 2;
    napi_value argv[proxyConstructorArgc] = { target, handler };
    napi_value proxy = nullptr;
    napi_new_instance(env, proxyConstructor, proxyConstructorArgc, argv, &proxy);

    return proxy;
}

/*
   Create frozen case
   Object.freeze() makes the object non-extensible and prevents
   existing properties from being changed or deleted.
   Note: freezing is shallow.
  
   See: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object/freeze
*/
static napi_value FreezeCase(napi_env env)
{
    napi_value target = CreateTargetObject(env);

    napi_value global = nullptr;
    napi_get_global(env, &global);

    napi_value object = nullptr;
    napi_get_named_property(env, global, "Object", &object);

    napi_value freeze = nullptr;
    napi_get_named_property(env, object, "freeze", &freeze);

    constexpr size_t freezeArgc = 1;
    napi_value argv[freezeArgc] = { target };
    napi_call_function(env, object, freeze, FREEZE_ARGC, argv, nullptr);

    return target;
}

/*
   Create non-writable case
   Properties defined via DefineProperty (napi_define_property) use custom descriptors.
   By default (if no attributes are set), they are non-writable, non-configurable, and non-enumerable.
   Here, we enable 'enumerable' while keeping the property non-writable and non-configurable.

   See: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object/defineProperty
*/
static napi_value NonWritableCase(napi_env env)
{
    napi_value target = nullptr;
    napi_create_object(env, &target);

    napi_value magic = nullptr;
    napi_create_uint32(env, OBJECT_MAGIC_NUMBER, &magic);

    napi_property_descriptor desc[] = { { OBJECT_MAGIC_PROPERTY.data(), nullptr, nullptr, nullptr, nullptr, magic,
                                          napi_enumerable, nullptr } };
    napi_define_properties(env, target, sizeof(desc) / sizeof(desc[0]), desc);

    return target;
}

// Data for setter/getter case
struct SetterGetterData {
    uint32_t value;
};

// Getter callback
static napi_value GetterCallback(napi_env env, napi_callback_info info)
{
    SetterGetterData* data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, nullptr, (void**)&data);

    napi_value result = nullptr;
    napi_create_uint32(env, data->value, &result);
    return result;
}

// Setter callback
constexpr size_t SETTER_CALLBACK_ARGC = 1;

static napi_value SetterCallback(napi_env env, napi_callback_info info)
{
    size_t argc = SETTER_CALLBACK_ARGC;
    napi_value argv[SETTER_CALLBACK_ARGC] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);

    if (valueType == napi_undefined || valueType == napi_null) {
        // Log warning: attempt to assign property to undefined or null
        // you can throw error here
        napi_value result = nullptr;
        napi_get_undefined(env, &result);
        return result;
    }

    SetterGetterData* data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, nullptr, (void**)&data);

    uint32_t value = 0;
    napi_get_value_uint32(env, argv[0], &value);
    data->value = value;

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

// Finalizer for setter/getter data
static void SetterGetterFinalizer(napi_env env, void* data, void* hint)
{
    delete static_cast<SetterGetterData*>(data);
}

/*
   Create setter/getter case
   Define a property with custom getter/setter. The actual value is stored in
   heap-allocated data (.data field), not as a regular object property.
   A finalizer ensures the memory is freed when the target object is garbage-collected.

   Go to blow links for more information:
   - https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Functions/get
   - https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Functions/set
   - https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object/defineProperty
 */
static napi_value SetterGetterCase(napi_env env)
{
    napi_value target = nullptr;
    napi_create_object(env, &target);

    SetterGetterData* data = new SetterGetterData { OBJECT_MAGIC_NUMBER };

    /*
       For simplicity, the property's value is stored in heap-allocated data attached
       to the property descriptor (via .data). This data is passed to getter/setter
       callbacks and will be cleaned up by the finalizer when the target object is GC'd.
    */
    napi_property_descriptor desc[] = { { OBJECT_MAGIC_PROPERTY.data(), nullptr, GetterCallback, SetterCallback,
                                          nullptr, nullptr, napi_default, data } };
    napi_define_properties(env, target, sizeof(desc) / sizeof(desc[0]), desc);

    // register a finalizer
    napi_add_finalizer(env, target, data, SetterGetterFinalizer, nullptr, nullptr);

    return target;
}

// Module init function
EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_value normal = CreateTargetObject(env);
    napi_value proxy = ProxyCase(env);
    napi_value frozen = FreezeCase(env);
    napi_value nonWritable = NonWritableCase(env);
    napi_value setterGetter = SetterGetterCase(env);

    napi_property_descriptor desc[] = {
        { "normal", nullptr, nullptr, nullptr, nullptr, normal, napi_default, nullptr },
        { "proxy", nullptr, nullptr, nullptr, nullptr, proxy, napi_default, nullptr },
        { "frozen", nullptr, nullptr, nullptr, nullptr, frozen, napi_default, nullptr },
        { "nonWritable", nullptr, nullptr, nullptr, nullptr, nonWritable, napi_default, nullptr },
        { "setterGetter", nullptr, nullptr, nullptr, nullptr, setterGetter, napi_default, nullptr },
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "protect_property",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterProtectPropertyModule(void)
{
    napi_module_register(&demoModule);
}
