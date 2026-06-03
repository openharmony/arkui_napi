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

#include <cstdint>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

// ============================================================================
// Named Constants — no magic numbers allowed
// ============================================================================

static constexpr int32_t TEST_INT_A = 42;
static constexpr int32_t TEST_INT_B = 100;
static constexpr int32_t TEST_INT_C = 200;
static constexpr int32_t TEST_INT_D = 300;
static constexpr int32_t TEST_INT_E = 500;
static constexpr int32_t TEST_INT_F = 10;
static constexpr int32_t TEST_INT_G = 20;
static constexpr int32_t TEST_INT_H = 30;
static constexpr int32_t TEST_INT_I = 999;
static constexpr int32_t FREEZE_ORIGINAL = 77;
static constexpr int32_t FREEZE_ATTEMPT = 88;
static constexpr int32_t SEAL_ORIGINAL = 55;
static constexpr int32_t SEAL_ATTEMPT = 66;
static constexpr int32_t SEAL_MODIFY = 111;
static constexpr int32_t GETTER_RETURN_VALUE = 777;
static constexpr int32_t SETTER_STORE_VALUE = 888;
static constexpr int32_t STATIC_ATTR_VALUE = 333;
static constexpr int32_t SYMBOL_VALUE = 444;
static constexpr int32_t DELETE_VALUE = 555;
static constexpr int32_t ELEMENT_VALUE_A = 11;
static constexpr int32_t ELEMENT_VALUE_B = 22;
static constexpr int32_t ELEMENT_VALUE_C = 33;
static constexpr int32_t ATTR_TEST_VALUE = 99;
static constexpr int32_t STRESS_PROP_COUNT = 50;
static constexpr int32_t PROTO_MARKER = 12345;
static constexpr int32_t NAMED_PROP_VALUE_A = 60;
static constexpr int32_t NAMED_PROP_VALUE_B = 70;
static constexpr int32_t NAMED_PROP_VALUE_C = 80;
static constexpr uint32_t ELEMENT_INDEX_ZERO = 0;
static constexpr uint32_t ELEMENT_INDEX_ONE = 1;
static constexpr uint32_t ELEMENT_INDEX_TWO = 2;
static constexpr uint32_t ELEMENT_INDEX_THREE = 3;
static constexpr size_t SINGLE_DESCRIPTOR = 1;
static constexpr size_t TWO_DESCRIPTORS = 2;
static constexpr size_t THREE_DESCRIPTORS = 3;
static constexpr size_t EXPECTED_OWN_COUNT_ONE = 1;
static constexpr size_t EXPECTED_OWN_COUNT_TWO = 2;
static constexpr size_t EXPECTED_OWN_COUNT_THREE = 3;
static constexpr uint32_t MODULE_VERSION = 1;
static constexpr uint32_t NO_MODULE_FLAGS = 0;

// ============================================================================
// Helper Functions
// ============================================================================

static void SetNamedBool(napi_env env, napi_value obj, const char* name, bool value)
{
    napi_value napiValue = nullptr;
    NAPI_CALL_VOID(env, napi_get_boolean(env, value, &napiValue));
    NAPI_CALL_VOID(env, napi_set_named_property(env, obj, name, napiValue));
}

static void SetNamedInt32(napi_env env, napi_value obj, const char* name, int32_t value)
{
    napi_value napiValue = nullptr;
    NAPI_CALL_VOID(env, napi_create_int32(env, value, &napiValue));
    NAPI_CALL_VOID(env, napi_set_named_property(env, obj, name, napiValue));
}

static napi_value CreateResult(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    return result;
}

static uint32_t GetPropertyCount(napi_env env, napi_value names)
{
    uint32_t count = 0;
    napi_get_array_length(env, names, &count);
    return count;
}

// ============================================================================
// Test 1: napi_get_all_property_names — napi_key_own_only
// ============================================================================

static napi_value TestGetOwnPropertyNames(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "alpha", TEST_INT_A);
    SetNamedInt32(env, obj, "beta", TEST_INT_B);
    napi_value names = nullptr;
    NAPI_CALL(env, napi_get_all_property_names(env, obj,
        napi_key_own_only, napi_key_all_properties, napi_key_keep_numbers, &names));
    uint32_t count = GetPropertyCount(env, names);

    SetNamedBool(env, result, "ownOnlySuccess", true);
    SetNamedBool(env, result, "countCorrect",
        count == static_cast<uint32_t>(EXPECTED_OWN_COUNT_TWO));
    SetNamedInt32(env, result, "totalNames", static_cast<int32_t>(count));
    return result;
}

// ============================================================================
// Test 2: napi_get_all_property_names — napi_key_include_prototypes
// ============================================================================

static napi_value TestGetIncludePrototypeNames(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "alpha", TEST_INT_A);
    napi_value names = nullptr;
    NAPI_CALL(env, napi_get_all_property_names(env, obj,
        napi_key_include_prototypes, napi_key_all_properties, napi_key_keep_numbers, &names));
    uint32_t count = GetPropertyCount(env, names);

    SetNamedBool(env, result, "includePrototypesSuccess", true);
    SetNamedBool(env, result, "hasPrototypeProps", count >= static_cast<uint32_t>(EXPECTED_OWN_COUNT_ONE));
    SetNamedInt32(env, result, "totalNames", static_cast<int32_t>(count));
    return result;
}

// ============================================================================
// Test 3: napi_get_all_property_names — napi_key_writable filter
// ============================================================================

static napi_value TestGetWritableNames(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "writableProp", TEST_INT_A);
    napi_value names = nullptr;
    NAPI_CALL(env, napi_get_all_property_names(env, obj,
        napi_key_own_only, napi_key_writable, napi_key_keep_numbers, &names));
    uint32_t count = GetPropertyCount(env, names);

    SetNamedBool(env, result, "writableFilterSuccess", true);
    SetNamedBool(env, result, "foundWritable", count >= static_cast<uint32_t>(EXPECTED_OWN_COUNT_ONE));
    SetNamedInt32(env, result, "writableCount", static_cast<int32_t>(count));
    return result;
}

// ============================================================================
// Test 4: napi_get_all_property_names — napi_key_enumerable filter
// ============================================================================

static napi_value TestGetEnumerableNames(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "enumProp", TEST_INT_B);
    napi_value names = nullptr;
    NAPI_CALL(env, napi_get_all_property_names(env, obj,
        napi_key_own_only, napi_key_enumerable, napi_key_keep_numbers, &names));
    uint32_t count = GetPropertyCount(env, names);

    SetNamedBool(env, result, "enumerableFilterSuccess", true);
    SetNamedBool(env, result, "foundEnumerable", count >= static_cast<uint32_t>(EXPECTED_OWN_COUNT_ONE));
    SetNamedInt32(env, result, "enumerableCount", static_cast<int32_t>(count));
    return result;
}

// ============================================================================
// Test 5: napi_get_all_property_names — napi_key_configurable filter
// ============================================================================

static napi_value TestGetConfigurableNames(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "configProp", TEST_INT_C);
    napi_value names = nullptr;
    NAPI_CALL(env, napi_get_all_property_names(env, obj,
        napi_key_own_only, napi_key_configurable, napi_key_keep_numbers, &names));
    uint32_t count = GetPropertyCount(env, names);

    SetNamedBool(env, result, "configurableFilterSuccess", true);
    SetNamedBool(env, result, "foundConfigurable", count >= static_cast<uint32_t>(EXPECTED_OWN_COUNT_ONE));
    SetNamedInt32(env, result, "configurableCount", static_cast<int32_t>(count));
    return result;
}

// ============================================================================
// Test 6: napi_get_all_property_names — napi_key_skip_strings filter
// ============================================================================

static napi_value TestGetSkipStringsNames(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "strProp", TEST_INT_D);
    napi_value names = nullptr;
    NAPI_CALL(env, napi_get_all_property_names(env, obj,
        napi_key_own_only, napi_key_skip_strings, napi_key_keep_numbers, &names));
    uint32_t count = GetPropertyCount(env, names);

    SetNamedBool(env, result, "skipStringsSuccess", true);
    SetNamedBool(env, result, "noStringKeys", count == 0);
    SetNamedInt32(env, result, "nonStringCount", static_cast<int32_t>(count));
    return result;
}

// ============================================================================
// Test 7: napi_get_all_property_names — napi_key_skip_symbols filter
// ============================================================================

static napi_value TestGetSkipSymbolsNames(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "normalProp", TEST_INT_E);
    napi_value symKey = nullptr;
    NAPI_CALL(env, napi_create_symbol(env, nullptr, &symKey));
    napi_value symVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_F, &symVal));
    NAPI_CALL(env, napi_set_property(env, obj, symKey, symVal));
    napi_value names = nullptr;
    NAPI_CALL(env, napi_get_all_property_names(env, obj,
        napi_key_own_only, napi_key_skip_symbols, napi_key_keep_numbers, &names));
    uint32_t count = GetPropertyCount(env, names);

    SetNamedBool(env, result, "skipSymbolsSuccess", true);
    SetNamedBool(env, result, "symbolsSkipped", count == static_cast<uint32_t>(EXPECTED_OWN_COUNT_ONE));
    SetNamedInt32(env, result, "nonSymbolCount", static_cast<int32_t>(count));
    return result;
}

// ============================================================================
// Test 8: napi_get_all_property_names — napi_key_numbers_to_strings
// ============================================================================

static napi_value TestKeyNumbersToStrings(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "strKey", TEST_INT_A);
    napi_value names = nullptr;
    NAPI_CALL(env, napi_get_all_property_names(env, obj,
        napi_key_own_only, napi_key_all_properties, napi_key_numbers_to_strings, &names));
    uint32_t count = GetPropertyCount(env, names);
    bool allStrings = true;
    for (uint32_t i = 0; i < count; i++) {
        napi_value elem = nullptr;
        NAPI_CALL(env, napi_get_element(env, names, i, &elem));
        napi_valuetype valType = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, elem, &valType));
        if (valType != napi_string) {
            allStrings = false;
        }
    }

    SetNamedBool(env, result, "numbersToStringsSuccess", true);
    SetNamedBool(env, result, "allKeysAreStrings", allStrings);
    SetNamedInt32(env, result, "keyCount", static_cast<int32_t>(count));
    return result;
}

// ============================================================================
// Test 9: napi_get_all_property_names — napi_key_keep_numbers
// ============================================================================

static napi_value TestKeyKeepNumbers(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "prop", TEST_INT_B);
    napi_value names = nullptr;
    NAPI_CALL(env, napi_get_all_property_names(env, obj,
        napi_key_own_only, napi_key_all_properties, napi_key_keep_numbers, &names));
    uint32_t count = GetPropertyCount(env, names);

    SetNamedBool(env, result, "keepNumbersSuccess", true);
    SetNamedBool(env, result, "hasKeys", count > 0);
    SetNamedInt32(env, result, "keyCount", static_cast<int32_t>(count));
    return result;
}

// ============================================================================
// Test 10: napi_object_freeze — verify mutation fails silently
// ============================================================================

static napi_value TestObjectFreeze(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "frozenProp", FREEZE_ORIGINAL);
    NAPI_CALL(env, napi_object_freeze(env, obj));
    // Attempt mutation — should fail silently on frozen object
    napi_value newVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, FREEZE_ATTEMPT, &newVal));
    napi_set_named_property(env, obj, "frozenProp", newVal);
    napi_value readBack = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, "frozenProp", &readBack));
    int32_t readVal = 0;
    NAPI_CALL(env, napi_get_value_int32(env, readBack, &readVal));

    SetNamedBool(env, result, "freezeSuccess", true);
    SetNamedBool(env, result, "mutationBlocked", readVal == FREEZE_ORIGINAL);
    SetNamedInt32(env, result, "frozenValue", readVal);
    return result;
}

// ============================================================================
// Test 11: napi_object_seal — existing props can be modified, new ones cannot
// ============================================================================

static napi_value TestObjectSeal(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "sealedProp", SEAL_ORIGINAL);
    NAPI_CALL(env, napi_object_seal(env, obj));
    // Attempt to add new property — should fail silently
    napi_value newPropVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, SEAL_ATTEMPT, &newPropVal));
    napi_set_named_property(env, obj, "newProp", newPropVal);
    bool hasNew = false;
    NAPI_CALL(env, napi_has_named_property(env, obj, "newProp", &hasNew));
    // Modify existing property — should succeed
    napi_value modVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, SEAL_MODIFY, &modVal));
    napi_set_named_property(env, obj, "sealedProp", modVal);
    napi_value readBack = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, "sealedProp", &readBack));
    int32_t readVal = 0;
    NAPI_CALL(env, napi_get_value_int32(env, readBack, &readVal));

    SetNamedBool(env, result, "sealSuccess", true);
    SetNamedBool(env, result, "newPropBlocked", !hasNew);
    SetNamedBool(env, result, "existingModified", readVal == SEAL_MODIFY);
    return result;
}

// ============================================================================
// Test 12: napi_define_properties — getter-only accessor
// ============================================================================

static napi_value GetterCallback(napi_env env, napi_callback_info /* info */)
{
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, GETTER_RETURN_VALUE, &val));
    return val;
}

static napi_value TestDefineGetterOnly(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_property_descriptor desc = {
        "getterProp", nullptr, nullptr, GetterCallback, nullptr, nullptr, napi_default, nullptr
    };
    NAPI_CALL(env, napi_define_properties(env, obj, SINGLE_DESCRIPTOR, &desc));
    napi_value readBack = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, "getterProp", &readBack));
    int32_t readVal = 0;
    NAPI_CALL(env, napi_get_value_int32(env, readBack, &readVal));

    SetNamedBool(env, result, "getterDefined", true);
    SetNamedBool(env, result, "getterReturnsCorrect", readVal == GETTER_RETURN_VALUE);
    return result;
}

// ============================================================================
// Test 13: napi_define_properties — setter-only accessor
// ============================================================================

static int32_t g_setterStoredValue = 0;

static napi_value SetterCallback(napi_env env, napi_callback_info info)
{
    size_t argc = SINGLE_DESCRIPTOR;
    napi_value args[SINGLE_DESCRIPTOR] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_CALL(env, napi_get_value_int32(env, args[0], &g_setterStoredValue));
    return nullptr;
}

static napi_value TestDefineSetterOnly(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    g_setterStoredValue = 0;
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_property_descriptor desc = {
        "setterProp", nullptr, nullptr, nullptr, SetterCallback, nullptr, napi_default, nullptr
    };
    NAPI_CALL(env, napi_define_properties(env, obj, SINGLE_DESCRIPTOR, &desc));
    napi_value valToSet = nullptr;
    NAPI_CALL(env, napi_create_int32(env, SETTER_STORE_VALUE, &valToSet));
    NAPI_CALL(env, napi_set_named_property(env, obj, "setterProp", valToSet));

    SetNamedBool(env, result, "setterDefined", true);
    SetNamedBool(env, result, "setterCalled", g_setterStoredValue == SETTER_STORE_VALUE);
    return result;
}

// ============================================================================
// Test 14: napi_define_properties — getter + setter combination
// ============================================================================

static int32_t g_accessorValue = 0;

static napi_value AccessorGetter(napi_env env, napi_callback_info /* info */)
{
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, g_accessorValue, &val));
    return val;
}

static napi_value AccessorSetter(napi_env env, napi_callback_info info)
{
    size_t argc = SINGLE_DESCRIPTOR;
    napi_value args[SINGLE_DESCRIPTOR] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_CALL(env, napi_get_value_int32(env, args[0], &g_accessorValue));
    return nullptr;
}

static napi_value TestDefineGetterSetter(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    g_accessorValue = 0;
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_property_descriptor desc = {
        "accessorProp", nullptr, nullptr, AccessorGetter, AccessorSetter, nullptr, napi_default, nullptr
    };
    NAPI_CALL(env, napi_define_properties(env, obj, SINGLE_DESCRIPTOR, &desc));
    napi_value valToSet = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_I, &valToSet));
    NAPI_CALL(env, napi_set_named_property(env, obj, "accessorProp", valToSet));
    napi_value readBack = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, "accessorProp", &readBack));
    int32_t readVal = 0;
    NAPI_CALL(env, napi_get_value_int32(env, readBack, &readVal));

    SetNamedBool(env, result, "accessorDefined", true);
    SetNamedBool(env, result, "roundTripCorrect", readVal == TEST_INT_I);
    return result;
}

// ============================================================================
// Test 15: napi_define_properties — napi_static attribute on standalone object
// ============================================================================

static napi_value TestDefineStaticAttribute(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, STATIC_ATTR_VALUE, &val));
    napi_property_descriptor desc = {
        "staticProp", nullptr, nullptr, nullptr, nullptr, val, napi_static, nullptr
    };
    NAPI_CALL(env, napi_define_properties(env, obj, SINGLE_DESCRIPTOR, &desc));
    bool hasProp = false;
    NAPI_CALL(env, napi_has_named_property(env, obj, "staticProp", &hasProp));

    SetNamedBool(env, result, "staticDefined", true);
    SetNamedBool(env, result, "propertyExists", hasProp);
    return result;
}

// ============================================================================
// Test 16: napi_has_own_property vs napi_has_property for inherited
// ============================================================================

static napi_value TestHasOwnVsHasProperty(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "ownProp", TEST_INT_A);
    // Check own property
    napi_value ownKey = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "ownProp", NAPI_AUTO_LENGTH, &ownKey));
    bool hasOwn = false;
    NAPI_CALL(env, napi_has_own_property(env, obj, ownKey, &hasOwn));
    bool hasProp = false;
    NAPI_CALL(env, napi_has_property(env, obj, ownKey, &hasProp));
    // Check inherited property "toString"
    napi_value inheritedKey = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "toString", NAPI_AUTO_LENGTH, &inheritedKey));
    bool hasOwnInherited = false;
    NAPI_CALL(env, napi_has_own_property(env, obj, inheritedKey, &hasOwnInherited));
    bool hasInherited = false;
    NAPI_CALL(env, napi_has_property(env, obj, inheritedKey, &hasInherited));

    SetNamedBool(env, result, "ownPropFoundByHasOwn", hasOwn);
    SetNamedBool(env, result, "ownPropFoundByHas", hasProp);
    SetNamedBool(env, result, "inheritedNotOwn", !hasOwnInherited);
    SetNamedBool(env, result, "inheritedFoundByHas", hasInherited);
    return result;
}

// ============================================================================
// Test 17: napi_get_property_names vs napi_get_all_property_names comparison
// ============================================================================

static napi_value TestGetNamesComparison(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "propX", TEST_INT_F);
    SetNamedInt32(env, obj, "propY", TEST_INT_G);
    // napi_get_property_names — enumerable own string keys
    napi_value simpleNames = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, obj, &simpleNames));
    uint32_t simpleCount = GetPropertyCount(env, simpleNames);
    // napi_get_all_property_names — all own properties
    napi_value allNames = nullptr;
    NAPI_CALL(env, napi_get_all_property_names(env, obj,
        napi_key_own_only, napi_key_all_properties, napi_key_keep_numbers, &allNames));
    uint32_t allCount = GetPropertyCount(env, allNames);

    SetNamedBool(env, result, "simpleNamesRetrieved", simpleCount > 0);
    SetNamedBool(env, result, "allNamesRetrieved", allCount > 0);
    SetNamedBool(env, result, "countsMatch", simpleCount == allCount);
    SetNamedInt32(env, result, "simpleCount", static_cast<int32_t>(simpleCount));
    SetNamedInt32(env, result, "allCount", static_cast<int32_t>(allCount));
    return result;
}

// ============================================================================
// Test 18: napi_set_property / napi_get_property with symbol keys
// ============================================================================

static napi_value TestSymbolKeySetGet(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value symDesc = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "mySymbol", NAPI_AUTO_LENGTH, &symDesc));
    napi_value symKey = nullptr;
    NAPI_CALL(env, napi_create_symbol(env, symDesc, &symKey));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, SYMBOL_VALUE, &val));
    NAPI_CALL(env, napi_set_property(env, obj, symKey, val));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_property(env, obj, symKey, &retrieved));
    int32_t readVal = 0;
    NAPI_CALL(env, napi_get_value_int32(env, retrieved, &readVal));

    SetNamedBool(env, result, "symbolSetSuccess", true);
    SetNamedBool(env, result, "symbolGetMatch", readVal == SYMBOL_VALUE);
    SetNamedInt32(env, result, "symbolValue", readVal);
    return result;
}

// ============================================================================
// Test 19: napi_delete_property and verify deletion
// ============================================================================

static napi_value TestDeleteProperty(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value key = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "toDelete", NAPI_AUTO_LENGTH, &key));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, DELETE_VALUE, &val));
    NAPI_CALL(env, napi_set_property(env, obj, key, val));
    bool hasBefore = false;
    NAPI_CALL(env, napi_has_property(env, obj, key, &hasBefore));
    bool deleteOk = false;
    NAPI_CALL(env, napi_delete_property(env, obj, key, &deleteOk));
    bool hasAfter = false;
    NAPI_CALL(env, napi_has_property(env, obj, key, &hasAfter));

    SetNamedBool(env, result, "existedBefore", hasBefore);
    SetNamedBool(env, result, "deleteReturned", deleteOk);
    SetNamedBool(env, result, "goneAfter", !hasAfter);
    return result;
}

// ============================================================================
// Test 20: napi_set_element / napi_get_element
// ============================================================================

static napi_value TestSetGetElement(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ELEMENT_VALUE_A, &val));
    NAPI_CALL(env, napi_set_element(env, obj, ELEMENT_INDEX_ZERO, val));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_element(env, obj, ELEMENT_INDEX_ZERO, &retrieved));
    int32_t readVal = 0;
    NAPI_CALL(env, napi_get_value_int32(env, retrieved, &readVal));

    SetNamedBool(env, result, "setElementSuccess", true);
    SetNamedBool(env, result, "getElementMatch", readVal == ELEMENT_VALUE_A);
    SetNamedInt32(env, result, "elementValue", readVal);
    return result;
}

// ============================================================================
// Test 21: napi_has_element
// ============================================================================

static napi_value TestHasElement(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ELEMENT_VALUE_B, &val));
    NAPI_CALL(env, napi_set_element(env, obj, ELEMENT_INDEX_ONE, val));
    bool hasOne = false;
    NAPI_CALL(env, napi_has_element(env, obj, ELEMENT_INDEX_ONE, &hasOne));
    bool hasThree = false;
    NAPI_CALL(env, napi_has_element(env, obj, ELEMENT_INDEX_THREE, &hasThree));

    SetNamedBool(env, result, "hasSetElement", hasOne);
    SetNamedBool(env, result, "missingElement", !hasThree);
    return result;
}

// ============================================================================
// Test 22: napi_delete_element
// ============================================================================

static napi_value TestDeleteElement(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ELEMENT_VALUE_C, &val));
    NAPI_CALL(env, napi_set_element(env, obj, ELEMENT_INDEX_TWO, val));
    bool hasBefore = false;
    NAPI_CALL(env, napi_has_element(env, obj, ELEMENT_INDEX_TWO, &hasBefore));
    bool deleteOk = false;
    NAPI_CALL(env, napi_delete_element(env, obj, ELEMENT_INDEX_TWO, &deleteOk));
    bool hasAfter = false;
    NAPI_CALL(env, napi_has_element(env, obj, ELEMENT_INDEX_TWO, &hasAfter));

    SetNamedBool(env, result, "existedBefore", hasBefore);
    SetNamedBool(env, result, "deleteOk", deleteOk);
    SetNamedBool(env, result, "goneAfter", !hasAfter);
    return result;
}

// ============================================================================
// Test 23: Property with napi_writable attribute
// ============================================================================

static napi_value TestPropertyAttrWritable(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ATTR_TEST_VALUE, &val));
    napi_property_descriptor desc = {
        "writeProp", nullptr, nullptr, nullptr, nullptr, val, napi_writable, nullptr
    };
    NAPI_CALL(env, napi_define_properties(env, obj, SINGLE_DESCRIPTOR, &desc));
    napi_value readBack = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, "writeProp", &readBack));
    int32_t readVal = 0;
    NAPI_CALL(env, napi_get_value_int32(env, readBack, &readVal));

    SetNamedBool(env, result, "writableDefined", true);
    SetNamedBool(env, result, "valueCorrect", readVal == ATTR_TEST_VALUE);
    return result;
}

// ============================================================================
// Test 24: Property with napi_enumerable attribute
// ============================================================================

static napi_value TestPropertyAttrEnumerable(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ATTR_TEST_VALUE, &val));
    napi_property_descriptor desc = {
        "enumProp", nullptr, nullptr, nullptr, nullptr, val, napi_enumerable, nullptr
    };
    NAPI_CALL(env, napi_define_properties(env, obj, SINGLE_DESCRIPTOR, &desc));
    napi_value names = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, obj, &names));
    uint32_t count = GetPropertyCount(env, names);

    SetNamedBool(env, result, "enumDefined", true);
    SetNamedBool(env, result, "appearsInEnum", count > 0);
    SetNamedInt32(env, result, "enumCount", static_cast<int32_t>(count));
    return result;
}

// ============================================================================
// Test 25: Property with napi_configurable attribute
// ============================================================================

static napi_value TestPropertyAttrConfigurable(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ATTR_TEST_VALUE, &val));
    napi_property_descriptor desc = {
        "configProp", nullptr, nullptr, nullptr, nullptr, val, napi_configurable, nullptr
    };
    NAPI_CALL(env, napi_define_properties(env, obj, SINGLE_DESCRIPTOR, &desc));
    bool hasProp = false;
    NAPI_CALL(env, napi_has_named_property(env, obj, "configProp", &hasProp));

    SetNamedBool(env, result, "configurableDefined", true);
    SetNamedBool(env, result, "propertyExists", hasProp);
    return result;
}

// ============================================================================
// Test 26: Property with all attributes combined
// ============================================================================

static napi_value TestPropertyAllAttributes(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ATTR_TEST_VALUE, &val));
    napi_property_attributes allAttrs = static_cast<napi_property_attributes>(
        napi_writable | napi_enumerable | napi_configurable);
    napi_property_descriptor desc = {
        "allProp", nullptr, nullptr, nullptr, nullptr, val, allAttrs, nullptr
    };
    NAPI_CALL(env, napi_define_properties(env, obj, SINGLE_DESCRIPTOR, &desc));
    // Verify writable
    napi_value newVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_I, &newVal));
    NAPI_CALL(env, napi_set_named_property(env, obj, "allProp", newVal));
    napi_value readBack = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, "allProp", &readBack));
    int32_t readVal = 0;
    NAPI_CALL(env, napi_get_value_int32(env, readBack, &readVal));
    // Verify enumerable
    napi_value names = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, obj, &names));
    uint32_t nameCount = GetPropertyCount(env, names);

    SetNamedBool(env, result, "allAttrsDefined", true);
    SetNamedBool(env, result, "writableWorks", readVal == TEST_INT_I);
    SetNamedBool(env, result, "isEnumerable", nameCount > 0);
    return result;
}

// ============================================================================
// Test 27: Object with many properties — stress test
// ============================================================================

static napi_value TestManyProperties(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    for (int32_t i = 0; i < STRESS_PROP_COUNT; i++) {
        char keyBuf[32] = {0};
        int ret = snprintf_s(keyBuf, sizeof(keyBuf), sizeof(keyBuf) - 1, "prop_%d", i);
        if (ret < 0) { continue; }
        SetNamedInt32(env, obj, keyBuf, i);
    }
    napi_value names = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, obj, &names));
    uint32_t count = GetPropertyCount(env, names);

    SetNamedBool(env, result, "stressTestSuccess", true);
    SetNamedBool(env, result, "allPropsCreated",
        count == static_cast<uint32_t>(STRESS_PROP_COUNT));
    SetNamedInt32(env, result, "propertyCount", static_cast<int32_t>(count));
    return result;
}

// ============================================================================
// Test 28: napi_get_prototype chain inspection
// ============================================================================

static napi_value TestGetPrototype(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "marker", PROTO_MARKER);
    napi_value prototype = nullptr;
    NAPI_CALL(env, napi_get_prototype(env, obj, &prototype));
    napi_valuetype protoType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, prototype, &protoType));
    bool isObjectOrNull = (protoType == napi_object || protoType == napi_null);

    SetNamedBool(env, result, "prototypeRetrieved", true);
    SetNamedBool(env, result, "prototypeTypeValid", isObjectOrNull);
    return result;
}

// ============================================================================
// Test 29: napi_create_object_with_properties
// ============================================================================

static napi_value TestCreateObjectWithProperties(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value valA = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_A, &valA));
    napi_value valB = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_B, &valB));
    napi_property_descriptor descs[TWO_DESCRIPTORS] = {
        {"propA", nullptr, nullptr, nullptr, nullptr, valA,
         napi_default, nullptr},
        {"propB", nullptr, nullptr, nullptr, nullptr, valB,
         napi_default, nullptr},
    };
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object_with_properties(env, &obj, TWO_DESCRIPTORS, descs));
    bool hasA = false;
    NAPI_CALL(env, napi_has_named_property(env, obj, "propA", &hasA));
    bool hasB = false;
    NAPI_CALL(env, napi_has_named_property(env, obj, "propB", &hasB));
    napi_value readA = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, "propA", &readA));
    int32_t readValA = 0;
    NAPI_CALL(env, napi_get_value_int32(env, readA, &readValA));

    SetNamedBool(env, result, "createWithPropsSuccess", hasA && hasB);
    SetNamedBool(env, result, "propACorrect", readValA == TEST_INT_A);
    return result;
}

// ============================================================================
// Test 30: napi_create_object_with_named_properties
// ============================================================================

static napi_value TestCreateObjectWithNamedProperties(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value valA = nullptr;
    NAPI_CALL(env, napi_create_int32(env, NAMED_PROP_VALUE_A, &valA));
    napi_value valB = nullptr;
    NAPI_CALL(env, napi_create_int32(env, NAMED_PROP_VALUE_B, &valB));
    napi_value valC = nullptr;
    NAPI_CALL(env, napi_create_int32(env, NAMED_PROP_VALUE_C, &valC));
    const char* keys[THREE_DESCRIPTORS] = {"keyA", "keyB", "keyC"};
    const napi_value values[THREE_DESCRIPTORS] = {valA, valB, valC};
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object_with_named_properties(
        env, &obj, THREE_DESCRIPTORS, keys, values));
    bool hasA = false;
    NAPI_CALL(env, napi_has_named_property(env, obj, "keyA", &hasA));
    bool hasC = false;
    NAPI_CALL(env, napi_has_named_property(env, obj, "keyC", &hasC));
    napi_value readB = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, "keyB", &readB));
    int32_t readValB = 0;
    NAPI_CALL(env, napi_get_value_int32(env, readB, &readValB));

    SetNamedBool(env, result, "createWithNamedSuccess", hasA && hasC);
    SetNamedBool(env, result, "keyBCorrect", readValB == NAMED_PROP_VALUE_B);
    SetNamedInt32(env, result, "keyBValue", readValB);
    return result;
}

// ============================================================================
// Test 31: freeze then verify no new properties can be added
// ============================================================================

static napi_value TestFreezeNoNewProps(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "existing", TEST_INT_A);
    NAPI_CALL(env, napi_object_freeze(env, obj));
    napi_value newVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_B, &newVal));
    napi_set_named_property(env, obj, "brandNew", newVal);
    bool hasNew = false;
    NAPI_CALL(env, napi_has_named_property(env, obj, "brandNew", &hasNew));

    SetNamedBool(env, result, "freezeApplied", true);
    SetNamedBool(env, result, "newPropBlocked", !hasNew);
    return result;
}

// ============================================================================
// Test 32: seal then verify configurable is locked
// ============================================================================

static napi_value TestSealConfigurableLocked(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "sealedProp", TEST_INT_C);
    NAPI_CALL(env, napi_object_seal(env, obj));
    // Attempt to delete sealed property — should fail
    napi_value key = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "sealedProp", NAPI_AUTO_LENGTH, &key));
    bool deleteOk = false;
    napi_delete_property(env, obj, key, &deleteOk);
    bool stillExists = false;
    NAPI_CALL(env, napi_has_named_property(env, obj, "sealedProp", &stillExists));

    SetNamedBool(env, result, "sealApplied", true);
    SetNamedBool(env, result, "deleteBlocked", stillExists);
    return result;
}

// ============================================================================
// Test 33: napi_define_properties with multiple descriptors at once
// ============================================================================

static napi_value TestDefineMultipleDescriptors(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value valA = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_F, &valA));
    napi_value valB = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_G, &valB));
    napi_value valC = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_H, &valC));
    napi_property_descriptor descs[THREE_DESCRIPTORS] = {
        {"multiA", nullptr, nullptr, nullptr, nullptr, valA, napi_default, nullptr},
        {"multiB", nullptr, nullptr, nullptr, nullptr, valB, napi_default, nullptr},
        {"multiC", nullptr, nullptr, nullptr, nullptr, valC, napi_default, nullptr},
    };
    NAPI_CALL(env, napi_define_properties(env, obj, THREE_DESCRIPTORS, descs));
    bool hasA = false;
    NAPI_CALL(env, napi_has_named_property(env, obj, "multiA", &hasA));
    bool hasB = false;
    NAPI_CALL(env, napi_has_named_property(env, obj, "multiB", &hasB));
    bool hasC = false;
    NAPI_CALL(env, napi_has_named_property(env, obj, "multiC", &hasC));

    SetNamedBool(env, result, "allDefined", hasA && hasB && hasC);
    SetNamedInt32(env, result, "definedCount",
        static_cast<int32_t>(THREE_DESCRIPTORS));
    return result;
}

// ============================================================================
// Test 34: symbol key property deletion
// ============================================================================

static napi_value TestDeleteSymbolProperty(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value symKey = nullptr;
    NAPI_CALL(env, napi_create_symbol(env, nullptr, &symKey));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_D, &val));
    NAPI_CALL(env, napi_set_property(env, obj, symKey, val));
    bool hasBefore = false;
    NAPI_CALL(env, napi_has_property(env, obj, symKey, &hasBefore));
    bool deleteOk = false;
    NAPI_CALL(env, napi_delete_property(env, obj, symKey, &deleteOk));
    bool hasAfter = false;
    NAPI_CALL(env, napi_has_property(env, obj, symKey, &hasAfter));

    SetNamedBool(env, result, "symbolExisted", hasBefore);
    SetNamedBool(env, result, "symbolDeleted", deleteOk);
    SetNamedBool(env, result, "symbolGone", !hasAfter);
    return result;
}

// ============================================================================
// Test 35: napi_get_all_property_names with combined key filters
// ============================================================================

static napi_value TestCombinedKeyFilters(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "combo1", TEST_INT_A);
    SetNamedInt32(env, obj, "combo2", TEST_INT_B);
    SetNamedInt32(env, obj, "combo3", TEST_INT_C);
    // Filter: writable + enumerable combined
    napi_key_filter combined = static_cast<napi_key_filter>(
        napi_key_writable | napi_key_enumerable);
    napi_value names = nullptr;
    NAPI_CALL(env, napi_get_all_property_names(env, obj,
        napi_key_own_only, combined, napi_key_numbers_to_strings, &names));
    uint32_t count = GetPropertyCount(env, names);

    SetNamedBool(env, result, "combinedFilterSuccess", true);
    SetNamedBool(env, result, "foundProps",
        count == static_cast<uint32_t>(EXPECTED_OWN_COUNT_THREE));
    SetNamedInt32(env, result, "filteredCount", static_cast<int32_t>(count));
    return result;
}

// ============================================================================
// OBJECT_OPS_TESTS[] descriptor table
// ============================================================================

static const napi_property_descriptor OBJECT_OPS_TESTS[] = {
    { "testGetOwnPropertyNames", nullptr, TestGetOwnPropertyNames,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testGetIncludePrototypeNames", nullptr, TestGetIncludePrototypeNames,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testGetWritableNames", nullptr, TestGetWritableNames,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testGetEnumerableNames", nullptr, TestGetEnumerableNames,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testGetConfigurableNames", nullptr, TestGetConfigurableNames,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testGetSkipStringsNames", nullptr, TestGetSkipStringsNames,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testGetSkipSymbolsNames", nullptr, TestGetSkipSymbolsNames,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testKeyNumbersToStrings", nullptr, TestKeyNumbersToStrings,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testKeyKeepNumbers", nullptr, TestKeyKeepNumbers,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testObjectFreeze", nullptr, TestObjectFreeze,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testObjectSeal", nullptr, TestObjectSeal,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testDefineGetterOnly", nullptr, TestDefineGetterOnly,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testDefineSetterOnly", nullptr, TestDefineSetterOnly,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testDefineGetterSetter", nullptr, TestDefineGetterSetter,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testDefineStaticAttribute", nullptr, TestDefineStaticAttribute,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testHasOwnVsHasProperty", nullptr, TestHasOwnVsHasProperty,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testGetNamesComparison", nullptr, TestGetNamesComparison,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testSymbolKeySetGet", nullptr, TestSymbolKeySetGet,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testDeleteProperty", nullptr, TestDeleteProperty,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testSetGetElement", nullptr, TestSetGetElement,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testHasElement", nullptr, TestHasElement,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testDeleteElement", nullptr, TestDeleteElement,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testPropertyAttrWritable", nullptr, TestPropertyAttrWritable,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testPropertyAttrEnumerable", nullptr, TestPropertyAttrEnumerable,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testPropertyAttrConfigurable", nullptr, TestPropertyAttrConfigurable,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testPropertyAllAttributes", nullptr, TestPropertyAllAttributes,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testManyProperties", nullptr, TestManyProperties,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testGetPrototype", nullptr, TestGetPrototype,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testCreateObjectWithProperties", nullptr, TestCreateObjectWithProperties,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testCreateObjectWithNamedProperties", nullptr, TestCreateObjectWithNamedProperties,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testFreezeNoNewProps", nullptr, TestFreezeNoNewProps,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testSealConfigurableLocked", nullptr, TestSealConfigurableLocked,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testDefineMultipleDescriptors", nullptr, TestDefineMultipleDescriptors,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testDeleteSymbolProperty", nullptr, TestDeleteSymbolProperty,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testCombinedKeyFilters", nullptr, TestCombinedKeyFilters,
      nullptr, nullptr, nullptr, napi_default, nullptr },
};

static constexpr size_t OBJECT_OPS_TEST_COUNT =
    sizeof(OBJECT_OPS_TESTS) / sizeof(OBJECT_OPS_TESTS[0]);

}  // namespace

// ============================================================================
// Module Initialization and Registration
// ============================================================================

static napi_value InitObjectOpsSuite(napi_env env, napi_value exports)
{
    NAPI_CALL(env, napi_define_properties(env, exports, OBJECT_OPS_TEST_COUNT, OBJECT_OPS_TESTS));
    return exports;
}

static napi_module g_objectOpsSuiteModule = {
    .nm_version = MODULE_VERSION,
    .nm_flags = NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitObjectOpsSuite,
    .nm_modname = "object_ops_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterObjectOpsSuiteModule(void)
{
    napi_module_register(&g_objectOpsSuiteModule);
}
