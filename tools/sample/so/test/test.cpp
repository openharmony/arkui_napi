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

namespace {
// Values must match the whitelist declarations in test.json (READ = 1 << 0, WRITE = 1 << 1,
// TEST_CONST as an untyped const).
constexpr uint32_t MODE_READ = 1;
constexpr uint32_t MODE_WRITE = 2;
constexpr uint32_t TEST_CONST_VALUE = 42;
}

static napi_value ClassAFoo(napi_env env, napi_callback_info /* info */)
{
    napi_value ret = nullptr;
    napi_get_undefined(env, &ret);
    return ret;
}

static napi_value ClassAConstructor(napi_env env, napi_callback_info info)
{
    napi_value jsthis = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
    return jsthis;
}

static napi_value ClassSubInherited(napi_env env, napi_callback_info /* info */)
{
    napi_value ret = nullptr;
    napi_get_undefined(env, &ret);
    return ret;
}

static napi_value ClassSubConstructor(napi_env env, napi_callback_info info)
{
    napi_value jsthis = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
    return jsthis;
}

static napi_value ClassSub2QualifiedMember(napi_env env, napi_callback_info /* info */)
{
    napi_value ret = nullptr;
    napi_get_undefined(env, &ret);
    return ret;
}

static napi_value ClassSub2Constructor(napi_env env, napi_callback_info info)
{
    napi_value jsthis = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
    return jsthis;
}

static napi_value BarFunc(napi_env env, napi_callback_info /* info */)
{
    napi_value ret = nullptr;
    napi_create_string_utf8(env, "bar", NAPI_AUTO_LENGTH, &ret);
    return ret;
}

static napi_value ExportedFooFunc(napi_env env, napi_callback_info /* info */)
{
    napi_value ret = nullptr;
    napi_get_undefined(env, &ret);
    return ret;
}

static napi_value OptionalCallbackFunc(napi_env env, napi_callback_info /* info */)
{
    napi_value ret = nullptr;
    napi_get_undefined(env, &ret);
    return ret;
}

static napi_value ModuleInit(napi_env env, napi_value exports)
{
    napi_value classA = nullptr;
    napi_property_descriptor classDesc[] = {
        {"foo", nullptr, ClassAFoo, nullptr, nullptr, nullptr, napi_default, nullptr}
    };
    napi_define_class(env, "A", NAPI_AUTO_LENGTH, ClassAConstructor, nullptr, 1, classDesc, &classA);
    napi_set_named_property(env, exports, "A", classA);

    napi_value bar = nullptr;
    napi_create_function(env, "bar", NAPI_AUTO_LENGTH, BarFunc, nullptr, &bar);
    napi_set_named_property(env, exports, "bar", bar);

    // Enum members declared with shift expressions (READ = 1 << 0) stay verbatim in the
    // whitelist; members are matched by name.
    napi_value mode = nullptr;
    napi_create_object(env, &mode);
    napi_value readVal = nullptr;
    napi_create_uint32(env, MODE_READ, &readVal);
    napi_set_named_property(env, mode, "READ", readVal);
    napi_value writeVal = nullptr;
    napi_create_uint32(env, MODE_WRITE, &writeVal);
    napi_set_named_property(env, mode, "WRITE", writeVal);
    napi_set_named_property(env, exports, "Mode", mode);

    // "inherited" is declared under the second parent of "interface Sub extends Base, Aux",
    // covering comma-multi-inheritance dependency extraction (Sub -> Aux).
    napi_value classSub = nullptr;
    napi_property_descriptor subDesc[] = {
        {"inherited", nullptr, ClassSubInherited, nullptr, nullptr, nullptr, napi_default, nullptr}
    };
    napi_define_class(env, "Sub", NAPI_AUTO_LENGTH, ClassSubConstructor, nullptr, 1, subDesc, &classSub);
    napi_set_named_property(env, exports, "Sub", classSub);

    // Whitelist shapes "export function exportedFoo(...)" and "optionalCallback?(...)"
    // cover the method normalizations (modifier prefix / optional marker).
    napi_value exportedFoo = nullptr;
    napi_create_function(env, "exportedFoo", NAPI_AUTO_LENGTH, ExportedFooFunc, nullptr, &exportedFoo);
    napi_set_named_property(env, exports, "exportedFoo", exportedFoo);

    napi_value optionalCallback = nullptr;
    napi_create_function(env, "optionalCallback", NAPI_AUTO_LENGTH, OptionalCallbackFunc, nullptr, &optionalCallback);
    napi_set_named_property(env, exports, "optionalCallback", optionalCallback);

    // "qualifiedMember" is declared under the last segment of the qualified parent
    // ns.Qualified (qualified-parent collapsing).
    napi_value classSub2 = nullptr;
    napi_property_descriptor sub2Desc[] = {
        {"qualifiedMember", nullptr, ClassSub2QualifiedMember, nullptr, nullptr, nullptr, napi_default, nullptr}
    };
    napi_define_class(env, "Sub2", NAPI_AUTO_LENGTH, ClassSub2Constructor, nullptr, 1, sub2Desc, &classSub2);
    napi_set_named_property(env, exports, "Sub2", classSub2);

    // TEST_CONST is declared as an untyped const in the whitelist.
    napi_value testConst = nullptr;
    napi_create_uint32(env, TEST_CONST_VALUE, &testConst);
    napi_set_named_property(env, exports, "TEST_CONST", testConst);

    return exports;
}

static napi_module testModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = ModuleInit,
    .nm_modname = "test",
    .nm_priv = nullptr,
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterCheckApiTestModule(void)
{
    napi_module_register(&testModule);
}
