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
#include <cstring>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

// ============================================================
// Named constants — no magic numbers
// ============================================================

// Test values for object properties
static constexpr int32_t INT32_PROP_VAL = 42;
static constexpr double DOUBLE_PROP_VAL = 3.14;
static constexpr int32_t INT32_ZERO = 0;
static constexpr double DOUBLE_ZERO = 0.0;

// String constants for keys and values
static constexpr char KEY_INT[] = "intProp";
static constexpr char KEY_STR[] = "strProp";
static constexpr char KEY_BOOL[] = "boolProp";
static constexpr char KEY_DOUBLE[] = "doubleProp";
static constexpr char STR_HELLO[] = "hello";
static constexpr char STR_NAPI[] = "napi";
static constexpr char STR_WORLD[] = "world";

static constexpr size_t STRING_BUF_SIZE = 256;
static constexpr size_t PROP_COUNT_ZERO = 0;
static constexpr size_t PROP_COUNT_ONE = 1;
static constexpr size_t PROP_COUNT_THREE = 3;

// Module registration
static constexpr uint32_t MODULE_VERSION = 1;
static constexpr uint32_t MODULE_FLAGS_NONE = 0;

// ============================================================
// Helper functions
// ============================================================

bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value)
{
    napi_value napiValue = nullptr;
    if (napi_get_boolean(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

// ============================================================
// Test 1: create object with a single int32 property via descriptors
// ============================================================
static napi_value TestObjWithPropsInt32(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value intVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_PROP_VAL, &intVal));

    napi_property_descriptor props[] = {
        {KEY_INT, nullptr, nullptr, nullptr, nullptr, intVal, napi_default, nullptr},
    };
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object_with_properties(env, &obj, PROP_COUNT_ONE, props));

    napi_value got = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, KEY_INT, &got));
    int32_t outVal = INT32_ZERO;
    NAPI_CALL(env, napi_get_value_int32(env, got, &outVal));
    if (!SetNamedBool(env, result, "int32Match", outVal == INT32_PROP_VAL)) {
        return nullptr;
    }
    return result;
}

// ============================================================
// Test 2: create object with a single string property via descriptors
// ============================================================
static napi_value TestObjWithPropsString(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value strVal = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, STR_HELLO, NAPI_AUTO_LENGTH, &strVal));

    napi_property_descriptor props[] = {
        {KEY_STR, nullptr, nullptr, nullptr, nullptr, strVal, napi_default, nullptr},
    };
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object_with_properties(env, &obj, PROP_COUNT_ONE, props));

    napi_value got = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, KEY_STR, &got));
    char buf[STRING_BUF_SIZE] = {0};
    size_t len = PROP_COUNT_ZERO;
    NAPI_CALL(env, napi_get_value_string_utf8(env, got, buf, sizeof(buf), &len));
    if (!SetNamedBool(env, result, "stringMatch", strcmp(buf, STR_HELLO) == INT32_ZERO)) {
        return nullptr;
    }
    return result;
}

// ============================================================
// Test 3: create object with a single boolean property via descriptors
// ============================================================
static napi_value TestObjWithPropsBoolean(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value boolVal = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, true, &boolVal));

    napi_property_descriptor props[] = {
        {KEY_BOOL, nullptr, nullptr, nullptr, nullptr, boolVal, napi_default, nullptr},
    };
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object_with_properties(env, &obj, PROP_COUNT_ONE, props));

    napi_value got = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, KEY_BOOL, &got));
    bool outVal = false;
    NAPI_CALL(env, napi_get_value_bool(env, got, &outVal));
    if (!SetNamedBool(env, result, "booleanMatch", outVal)) {
        return nullptr;
    }
    return result;
}

// ============================================================
// Test 4: create object with a single double property via descriptors
// ============================================================
static napi_value TestObjWithPropsDouble(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value doubleVal = nullptr;
    NAPI_CALL(env, napi_create_double(env, DOUBLE_PROP_VAL, &doubleVal));

    napi_property_descriptor props[] = {
        {KEY_DOUBLE, nullptr, nullptr, nullptr, nullptr, doubleVal, napi_default, nullptr},
    };
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object_with_properties(env, &obj, PROP_COUNT_ONE, props));

    napi_value got = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, KEY_DOUBLE, &got));
    double outVal = DOUBLE_ZERO;
    NAPI_CALL(env, napi_get_value_double(env, got, &outVal));
    if (!SetNamedBool(env, result, "doubleMatch", outVal == DOUBLE_PROP_VAL)) {
        return nullptr;
    }
    return result;
}

// ============================================================
// Test 5: create object with multiple properties (int32, string, boolean)
// ============================================================
static napi_value TestObjWithPropsMultiple(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value intVal = nullptr;
    napi_value strVal = nullptr;
    napi_value boolVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_PROP_VAL, &intVal));
    NAPI_CALL(env, napi_create_string_utf8(env, STR_HELLO, NAPI_AUTO_LENGTH, &strVal));
    NAPI_CALL(env, napi_get_boolean(env, false, &boolVal));

    napi_property_descriptor props[] = {
        {KEY_INT, nullptr, nullptr, nullptr, nullptr, intVal, napi_default, nullptr},
        {KEY_STR, nullptr, nullptr, nullptr, nullptr, strVal, napi_default, nullptr},
        {KEY_BOOL, nullptr, nullptr, nullptr, nullptr, boolVal, napi_default, nullptr},
    };
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object_with_properties(env, &obj, PROP_COUNT_THREE, props));

    napi_value gotInt = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, KEY_INT, &gotInt));
    int32_t outInt = INT32_ZERO;
    NAPI_CALL(env, napi_get_value_int32(env, gotInt, &outInt));
    if (!SetNamedBool(env, result, "int32Match", outInt == INT32_PROP_VAL)) {
        return nullptr;
    }

    napi_value gotStr = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, KEY_STR, &gotStr));
    char buf[STRING_BUF_SIZE] = {0};
    size_t len = PROP_COUNT_ZERO;
    NAPI_CALL(env, napi_get_value_string_utf8(env, gotStr, buf, sizeof(buf), &len));
    if (!SetNamedBool(env, result, "stringMatch", strcmp(buf, STR_HELLO) == INT32_ZERO)) {
        return nullptr;
    }

    napi_value gotBool = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, KEY_BOOL, &gotBool));
    bool outBool = true;
    NAPI_CALL(env, napi_get_value_bool(env, gotBool, &outBool));
    if (!SetNamedBool(env, result, "boolIsFalse", !outBool)) {
        return nullptr;
    }
    return result;
}

// ============================================================
// Test 6: create object with zero properties — result is empty object
// ============================================================
static napi_value TestObjWithPropsEmpty(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object_with_properties(env, &obj, PROP_COUNT_ZERO, nullptr));

    napi_valuetype objType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, obj, &objType));
    if (!SetNamedBool(env, result, "isObject", objType == napi_object)) {
        return nullptr;
    }

    // Verify the object has no own property named KEY_INT
    bool hasProp = true;
    NAPI_CALL(env, napi_has_named_property(env, obj, KEY_INT, &hasProp));
    if (!SetNamedBool(env, result, "noUnexpectedProp", !hasProp)) {
        return nullptr;
    }
    return result;
}

// ============================================================
// Test 7: create object via named-properties API with multiple entries
// ============================================================
static napi_value TestObjWithNamedPropsMultiple(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value intVal = nullptr;
    napi_value strVal = nullptr;
    napi_value boolVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, INT32_PROP_VAL, &intVal));
    NAPI_CALL(env, napi_create_string_utf8(env, STR_NAPI, NAPI_AUTO_LENGTH, &strVal));
    NAPI_CALL(env, napi_get_boolean(env, true, &boolVal));

    const char* keys[] = {KEY_INT, KEY_STR, KEY_BOOL};
    napi_value values[] = {intVal, strVal, boolVal};
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object_with_named_properties(
        env, &obj, PROP_COUNT_THREE, keys, values));

    // Verify each property by name
    napi_value gotInt = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, KEY_INT, &gotInt));
    int32_t outInt = INT32_ZERO;
    NAPI_CALL(env, napi_get_value_int32(env, gotInt, &outInt));
    if (!SetNamedBool(env, result, "int32Match", outInt == INT32_PROP_VAL)) {
        return nullptr;
    }

    napi_value gotStr = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, KEY_STR, &gotStr));
    char buf[STRING_BUF_SIZE] = {0};
    size_t len = PROP_COUNT_ZERO;
    NAPI_CALL(env, napi_get_value_string_utf8(env, gotStr, buf, sizeof(buf), &len));
    if (!SetNamedBool(env, result, "stringMatch", strcmp(buf, STR_NAPI) == INT32_ZERO)) {
        return nullptr;
    }

    napi_value gotBool = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, KEY_BOOL, &gotBool));
    bool outBool = false;
    NAPI_CALL(env, napi_get_value_bool(env, gotBool, &outBool));
    if (!SetNamedBool(env, result, "boolIsTrue", outBool)) {
        return nullptr;
    }
    return result;
}

// ============================================================
// Test 8: create object via named-properties API with zero entries
// ============================================================
static napi_value TestObjWithNamedPropsEmpty(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object_with_named_properties(
        env, &obj, PROP_COUNT_ZERO, nullptr, nullptr));

    napi_valuetype objType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, obj, &objType));
    if (!SetNamedBool(env, result, "isObject", objType == napi_object)) {
        return nullptr;
    }
    return result;
}

// ============================================================
// Module export descriptors table
// ============================================================
static constexpr size_t OBJ_PROPS_TEST_COUNT = 8;

struct ObjPropsTestEntry {
    const char* name;
    napi_callback callback;
};

static const ObjPropsTestEntry OBJ_PROPS_TESTS[] = {
    {"testObjWithPropsInt32", TestObjWithPropsInt32},
    {"testObjWithPropsString", TestObjWithPropsString},
    {"testObjWithPropsBoolean", TestObjWithPropsBoolean},
    {"testObjWithPropsDouble", TestObjWithPropsDouble},
    {"testObjWithPropsMultiple", TestObjWithPropsMultiple},
    {"testObjWithPropsEmpty", TestObjWithPropsEmpty},
    {"testObjWithNamedPropsMultiple", TestObjWithNamedPropsMultiple},
    {"testObjWithNamedPropsEmpty", TestObjWithNamedPropsEmpty},
};
}  // namespace

static napi_value InitObjPropsSuite(napi_env env, napi_value exports)
{
    std::vector<napi_property_descriptor> descriptors(OBJ_PROPS_TEST_COUNT);
    for (size_t i = 0; i < OBJ_PROPS_TEST_COUNT; i++) {
        descriptors[i] = napi_property_descriptor{
            OBJ_PROPS_TESTS[i].name,
            nullptr,
            OBJ_PROPS_TESTS[i].callback,
            nullptr,
            nullptr,
            nullptr,
            napi_default,
            nullptr,
        };
    }
    NAPI_CALL(env, napi_define_properties(env, exports, descriptors.size(), descriptors.data()));
    return exports;
}

static napi_module g_objPropsSuiteModule = {
    .nm_version = MODULE_VERSION,
    .nm_flags = MODULE_FLAGS_NONE,
    .nm_filename = nullptr,
    .nm_register_func = InitObjPropsSuite,
    .nm_modname = "obj_props_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterObjPropsSuiteModule(void)
{
    napi_module_register(&g_objPropsSuiteModule);
}
