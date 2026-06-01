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
#include "securec.h"

namespace {

// ---------------------------------------------------------------------------
// Named Constants — no magic numbers allowed
// ---------------------------------------------------------------------------
static constexpr int32_t TEST_INT_A = 42;
static constexpr int32_t TEST_INT_B = 100;
static constexpr int32_t TEST_INT_C = 200;
static constexpr int32_t TEST_INT_D = 300;
static constexpr int32_t TEST_INT_E = 500;
static constexpr int32_t TEST_INT_F = 10;
static constexpr int32_t TEST_INT_G = 20;
static constexpr int32_t TEST_INT_H = 30;
static constexpr int32_t TEST_INT_I = 999;
static constexpr int32_t TEST_INT_J = 77;
static constexpr double TEST_DOUBLE_A = 3.14;
static constexpr double TEST_DOUBLE_B = 2.718;
static constexpr int32_t TEST_ARRAY_LENGTH = 5;
static constexpr uint32_t INITIAL_REF_COUNT = 1;
static constexpr uint32_t WEAK_REF_COUNT = 0;
static constexpr int32_t NESTED_SCOPE_DEPTH = 3;
static constexpr int32_t DEEP_NEST_DEPTH = 5;
static constexpr int32_t MULTI_PROPERTY_COUNT = 3;
static constexpr size_t SINGLE_DESCRIPTOR = 1;
static constexpr uint32_t ELEMENT_INDEX_ZERO = 0;
static constexpr uint32_t ELEMENT_INDEX_ONE = 1;
static constexpr uint32_t ELEMENT_INDEX_TWO = 2;
static constexpr uint32_t ELEMENT_INDEX_FOUR = 4;
static constexpr uint32_t ELEMENT_LARGE_INDEX = 9999;
static constexpr uint32_t MIN_NAPI_VERSION = 1;
static constexpr uint32_t REF_COUNT_ZERO = 0;
static constexpr uint32_t REF_COUNT_ONE = 1;
static constexpr uint32_t REF_COUNT_TWO = 2;
static constexpr int32_t MULTIPLE_REF_TOTAL = 3;
static constexpr int32_t FREEZE_PROP_COUNT = 2;
static constexpr int32_t SEAL_ORIGINAL_VALUE = 400;
static constexpr int32_t PROTOTYPE_MARKER = 12345;
static constexpr int32_t ALL_ATTR_FLAGS = 7;
static constexpr uint32_t MODULE_VERSION = 1;
static constexpr uint32_t NO_MODULE_FLAGS = 0;

// ---------------------------------------------------------------------------
// Helper Functions
// ---------------------------------------------------------------------------
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

static void SetNamedString(napi_env env, napi_value obj, const char* name, const char* value)
{
    napi_value napiValue = nullptr;
    NAPI_CALL_VOID(env, napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &napiValue));
    NAPI_CALL_VOID(env, napi_set_named_property(env, obj, name, napiValue));
}

static void SetNamedDouble(napi_env env, napi_value obj, const char* name, double value)
{
    napi_value napiValue = nullptr;
    NAPI_CALL_VOID(env, napi_create_double(env, value, &napiValue));
    NAPI_CALL_VOID(env, napi_set_named_property(env, obj, name, napiValue));
}

static napi_value CreateResult(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    return result;
}

static napi_value CreateNestedScopeValue(napi_env env, int32_t depth, int32_t value)
{
    if (depth <= 0) {
        napi_value numVal = nullptr;
        NAPI_CALL(env, napi_create_int32(env, value, &numVal));
        return numVal;
    }
    napi_handle_scope scope = nullptr;
    NAPI_CALL(env, napi_open_handle_scope(env, &scope));
    napi_value inner = CreateNestedScopeValue(env, depth - 1, value);
    NAPI_CALL(env, napi_close_handle_scope(env, scope));
    return inner;
}

static bool ManageSingleRef(napi_env env, napi_value obj, uint32_t* outFinalCount)
{
    napi_ref ref = nullptr;
    if (napi_create_reference(env, obj, INITIAL_REF_COUNT, &ref) != napi_ok) {
        return false;
    }
    uint32_t countAfterRef = 0;
    if (napi_reference_ref(env, ref, &countAfterRef) != napi_ok) {
        napi_delete_reference(env, ref);
        return false;
    }
    uint32_t countAfterUnref = 0;
    if (napi_reference_unref(env, ref, &countAfterUnref) != napi_ok) {
        napi_delete_reference(env, ref);
        return false;
    }
    *outFinalCount = countAfterUnref;
    napi_delete_reference(env, ref);
    return true;
}

static void FillPropertyDescriptor(napi_property_descriptor* desc, const char* name, napi_value value)
{
    desc->utf8name = name;
    desc->name = nullptr;
    desc->method = nullptr;
    desc->getter = nullptr;
    desc->setter = nullptr;
    desc->value = value;
    desc->attributes = napi_default;
    desc->data = nullptr;
}

// ---------------------------------------------------------------------------
// Test 1: napi_open_handle_scope / napi_close_handle_scope
// ---------------------------------------------------------------------------
static napi_value TestOpenCloseHandleScope(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_handle_scope scope = nullptr;
    NAPI_CALL(env, napi_open_handle_scope(env, &scope));
    napi_value testValue = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_A, &testValue));
    int32_t retrieved = 0;
    NAPI_CALL(env, napi_get_value_int32(env, testValue, &retrieved));
    NAPI_CALL(env, napi_close_handle_scope(env, scope));

    SetNamedBool(env, result, "scopeOpenClose", true);
    SetNamedBool(env, result, "valueCorrect", retrieved == TEST_INT_A);
    return result;
}

// ---------------------------------------------------------------------------
// Test 2: creating objects inside a handle scope
// ---------------------------------------------------------------------------
static napi_value TestHandleScopeObjects(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_handle_scope scope = nullptr;
    NAPI_CALL(env, napi_open_handle_scope(env, &scope));
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedString(env, obj, "name", "scopeObject");
    bool hasProp = false;
    NAPI_CALL(env, napi_has_named_property(env, obj, "name", &hasProp));
    NAPI_CALL(env, napi_close_handle_scope(env, scope));

    SetNamedBool(env, result, "objectCreated", true);
    SetNamedBool(env, result, "propertySet", hasProp);
    return result;
}

// ---------------------------------------------------------------------------
// Test 3: nested handle scopes
// ---------------------------------------------------------------------------
static napi_value TestNestedHandleScopes(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value innerVal = CreateNestedScopeValue(env, NESTED_SCOPE_DEPTH, TEST_INT_B);
    int32_t retrieved = 0;
    NAPI_CALL(env, napi_get_value_int32(env, innerVal, &retrieved));

    SetNamedBool(env, result, "nestedSuccess", true);
    SetNamedBool(env, result, "valuePreserved", retrieved == TEST_INT_B);
    SetNamedInt32(env, result, "depth", NESTED_SCOPE_DEPTH);
    return result;
}

// ---------------------------------------------------------------------------
// Test 4: deeply nested handle scopes
// ---------------------------------------------------------------------------
static napi_value TestDeepNestedHandleScopes(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value deepVal = CreateNestedScopeValue(env, DEEP_NEST_DEPTH, TEST_INT_C);
    int32_t retrieved = 0;
    NAPI_CALL(env, napi_get_value_int32(env, deepVal, &retrieved));

    SetNamedBool(env, result, "deepNestedSuccess", true);
    SetNamedBool(env, result, "valuePreserved", retrieved == TEST_INT_C);
    SetNamedInt32(env, result, "depth", DEEP_NEST_DEPTH);
    return result;
}

// ---------------------------------------------------------------------------
// Test 5: napi_open_escapable_handle_scope / napi_close_escapable_handle_scope
// ---------------------------------------------------------------------------
static napi_value TestOpenCloseEscapableScope(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_escapable_handle_scope scope = nullptr;
    NAPI_CALL(env, napi_open_escapable_handle_scope(env, &scope));
    napi_value innerObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &innerObj));
    napi_value escaped = nullptr;
    NAPI_CALL(env, napi_escape_handle(env, scope, innerObj, &escaped));
    NAPI_CALL(env, napi_close_escapable_handle_scope(env, scope));
    napi_valuetype valType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, escaped, &valType));

    SetNamedBool(env, result, "escapableScopeSuccess", true);
    SetNamedBool(env, result, "escapedIsObject", valType == napi_object);
    return result;
}

// ---------------------------------------------------------------------------
// Test 6: napi_escape_handle with integer value
// ---------------------------------------------------------------------------
static napi_value TestEscapeHandleInt(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_escapable_handle_scope scope = nullptr;
    NAPI_CALL(env, napi_open_escapable_handle_scope(env, &scope));
    napi_value innerValue = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_A, &innerValue));
    napi_value escaped = nullptr;
    NAPI_CALL(env, napi_escape_handle(env, scope, innerValue, &escaped));
    NAPI_CALL(env, napi_close_escapable_handle_scope(env, scope));
    int32_t retrieved = 0;
    NAPI_CALL(env, napi_get_value_int32(env, escaped, &retrieved));

    SetNamedBool(env, result, "escapeSuccess", true);
    SetNamedBool(env, result, "valueIntact", retrieved == TEST_INT_A);
    SetNamedInt32(env, result, "escapedValue", retrieved);
    return result;
}

// ---------------------------------------------------------------------------
// Test 7: escapable scope with nested regular scope
// ---------------------------------------------------------------------------
static napi_value TestEscapableWithNestedScope(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_escapable_handle_scope escScope = nullptr;
    NAPI_CALL(env, napi_open_escapable_handle_scope(env, &escScope));
    napi_handle_scope innerScope = nullptr;
    NAPI_CALL(env, napi_open_handle_scope(env, &innerScope));
    napi_value innerStr = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "nested_escaped", NAPI_AUTO_LENGTH, &innerStr));
    NAPI_CALL(env, napi_close_handle_scope(env, innerScope));
    napi_value escaped = nullptr;
    NAPI_CALL(env, napi_escape_handle(env, escScope, innerStr, &escaped));
    NAPI_CALL(env, napi_close_escapable_handle_scope(env, escScope));
    napi_valuetype valType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, escaped, &valType));

    SetNamedBool(env, result, "nestedEscapeSuccess", true);
    SetNamedBool(env, result, "escapedIsString", valType == napi_string);
    return result;
}

// ---------------------------------------------------------------------------
// Test 8: napi_create_reference / napi_delete_reference
// ---------------------------------------------------------------------------
static napi_value TestCreateDeleteReference(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, obj, INITIAL_REF_COUNT, &ref));
    napi_value refValue = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &refValue));
    bool isValid = (refValue != nullptr);
    NAPI_CALL(env, napi_delete_reference(env, ref));

    SetNamedBool(env, result, "createSuccess", true);
    SetNamedBool(env, result, "refValueValid", isValid);
    SetNamedBool(env, result, "deleteSuccess", true);
    return result;
}

// ---------------------------------------------------------------------------
// Test 9: napi_reference_ref (increment refcount)
// ---------------------------------------------------------------------------
static napi_value TestReferenceRef(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, obj, INITIAL_REF_COUNT, &ref));
    uint32_t refCount = 0;
    NAPI_CALL(env, napi_reference_ref(env, ref, &refCount));
    bool countCorrect = (refCount == REF_COUNT_TWO);
    NAPI_CALL(env, napi_delete_reference(env, ref));

    SetNamedBool(env, result, "refIncrementSuccess", true);
    SetNamedBool(env, result, "refCountCorrect", countCorrect);
    SetNamedInt32(env, result, "finalRefCount", static_cast<int32_t>(refCount));
    return result;
}

// ---------------------------------------------------------------------------
// Test 10: napi_reference_unref (decrement refcount)
// ---------------------------------------------------------------------------
static napi_value TestReferenceUnref(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, obj, INITIAL_REF_COUNT, &ref));
    uint32_t refCount = 0;
    NAPI_CALL(env, napi_reference_unref(env, ref, &refCount));
    bool countCorrect = (refCount == REF_COUNT_ZERO);
    NAPI_CALL(env, napi_delete_reference(env, ref));

    SetNamedBool(env, result, "unrefSuccess", true);
    SetNamedBool(env, result, "refCountZero", countCorrect);
    SetNamedInt32(env, result, "finalRefCount", static_cast<int32_t>(refCount));
    return result;
}

// ---------------------------------------------------------------------------
// Test 11: napi_get_reference_value with marker property
// ---------------------------------------------------------------------------
static napi_value TestGetReferenceValue(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value original = nullptr;
    NAPI_CALL(env, napi_create_object(env, &original));
    SetNamedInt32(env, original, "marker", TEST_INT_A);
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, original, INITIAL_REF_COUNT, &ref));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    napi_value markerOut = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, retrieved, "marker", &markerOut));
    int32_t markerVal = 0;
    NAPI_CALL(env, napi_get_value_int32(env, markerOut, &markerVal));
    NAPI_CALL(env, napi_delete_reference(env, ref));

    SetNamedBool(env, result, "getRefValueSuccess", true);
    SetNamedBool(env, result, "markerMatch", markerVal == TEST_INT_A);
    return result;
}

// ---------------------------------------------------------------------------
// Test 12: multiple reference lifecycle management
// ---------------------------------------------------------------------------
static napi_value TestMultipleReferenceLifecycle(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    bool allPassed = true;
    for (int32_t i = 0; i < MULTIPLE_REF_TOTAL; i++) {
        napi_value obj = nullptr;
        NAPI_CALL(env, napi_create_object(env, &obj));
        uint32_t finalCount = 0;
        if (!ManageSingleRef(env, obj, &finalCount) || finalCount != REF_COUNT_ONE) {
            allPassed = false;
        }
    }

    SetNamedBool(env, result, "multiRefSuccess", allPassed);
    SetNamedInt32(env, result, "refsTested", MULTIPLE_REF_TOTAL);
    return result;
}

// ---------------------------------------------------------------------------
// Test 13: weak reference (refcount = 0)
// ---------------------------------------------------------------------------
static napi_value TestWeakReference(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, obj, WEAK_REF_COUNT, &ref));
    napi_value refValue = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &refValue));
    NAPI_CALL(env, napi_delete_reference(env, ref));

    SetNamedBool(env, result, "weakRefCreated", true);
    SetNamedBool(env, result, "deleteSuccess", true);
    return result;
}

// ---------------------------------------------------------------------------
// Test 14: ref/unref round-trip preserving refcount
// ---------------------------------------------------------------------------
static napi_value TestRefUnrefRoundTrip(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, obj, INITIAL_REF_COUNT, &ref));
    uint32_t afterRef = 0;
    NAPI_CALL(env, napi_reference_ref(env, ref, &afterRef));
    uint32_t afterUnref = 0;
    NAPI_CALL(env, napi_reference_unref(env, ref, &afterUnref));
    NAPI_CALL(env, napi_delete_reference(env, ref));

    SetNamedBool(env, result, "roundTripSuccess", true);
    SetNamedBool(env, result, "refCountRestored", afterUnref == REF_COUNT_ONE);
    SetNamedInt32(env, result, "peakRefCount", static_cast<int32_t>(afterRef));
    return result;
}

// ---------------------------------------------------------------------------
// Test 15: napi_get_version
// ---------------------------------------------------------------------------
static napi_value TestGetVersion(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    uint32_t version = 0;
    NAPI_CALL(env, napi_get_version(env, &version));

    SetNamedBool(env, result, "versionRetrieved", true);
    SetNamedBool(env, result, "versionValid", version >= MIN_NAPI_VERSION);
    SetNamedInt32(env, result, "version", static_cast<int32_t>(version));
    return result;
}

// ---------------------------------------------------------------------------
// Test 16: property descriptor with napi_writable
// ---------------------------------------------------------------------------
static napi_value TestPropertyWritable(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_A, &val));
    napi_property_descriptor desc = { "writableProp", nullptr, nullptr, nullptr, nullptr, val, napi_writable, nullptr };
    NAPI_CALL(env, napi_define_properties(env, obj, SINGLE_DESCRIPTOR, &desc));
    napi_value readBack = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, "writableProp", &readBack));
    int32_t readVal = 0;
    NAPI_CALL(env, napi_get_value_int32(env, readBack, &readVal));

    SetNamedBool(env, result, "writableDefined", true);
    SetNamedBool(env, result, "valueCorrect", readVal == TEST_INT_A);
    return result;
}

// ---------------------------------------------------------------------------
// Test 17: property descriptor with napi_enumerable
// ---------------------------------------------------------------------------
static napi_value TestPropertyEnumerable(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_B, &val));
    napi_property_descriptor desc = { "enumProp", nullptr, nullptr, nullptr, nullptr, val, napi_enumerable, nullptr };
    NAPI_CALL(env, napi_define_properties(env, obj, SINGLE_DESCRIPTOR, &desc));
    napi_value propNames = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, obj, &propNames));
    uint32_t nameCount = 0;
    NAPI_CALL(env, napi_get_array_length(env, propNames, &nameCount));

    SetNamedBool(env, result, "enumDefined", true);
    SetNamedBool(env, result, "appearsInNames", nameCount > 0);
    SetNamedInt32(env, result, "nameCount", static_cast<int32_t>(nameCount));
    return result;
}

// ---------------------------------------------------------------------------
// Test 18: property descriptor with napi_configurable
// ---------------------------------------------------------------------------
static napi_value TestPropertyConfigurable(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_C, &val));
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

// ---------------------------------------------------------------------------
// Test 19: property with all attributes combined
// ---------------------------------------------------------------------------
static napi_value TestPropertyAllAttributes(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ALL_ATTR_FLAGS, &val));
    napi_property_attributes allAttrs =
        static_cast<napi_property_attributes>(napi_writable | napi_enumerable | napi_configurable);
    napi_property_descriptor desc = { "allAttrProp", nullptr, nullptr, nullptr, nullptr, val, allAttrs, nullptr };
    NAPI_CALL(env, napi_define_properties(env, obj, SINGLE_DESCRIPTOR, &desc));
    napi_value readBack = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, "allAttrProp", &readBack));
    int32_t readVal = 0;
    NAPI_CALL(env, napi_get_value_int32(env, readBack, &readVal));

    SetNamedBool(env, result, "allAttrsDefined", true);
    SetNamedBool(env, result, "valueCorrect", readVal == ALL_ATTR_FLAGS);
    return result;
}

// ---------------------------------------------------------------------------
// Test 20: napi_define_properties with multiple descriptors
// ---------------------------------------------------------------------------
static napi_value TestDefineMultipleProperties(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value vals[MULTI_PROPERTY_COUNT] = { nullptr };
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_F, &vals[ELEMENT_INDEX_ZERO]));
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_G, &vals[ELEMENT_INDEX_ONE]));
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_H, &vals[ELEMENT_INDEX_TWO]));
    napi_property_descriptor descs[MULTI_PROPERTY_COUNT] = {};
    FillPropertyDescriptor(&descs[ELEMENT_INDEX_ZERO], "propA", vals[ELEMENT_INDEX_ZERO]);
    FillPropertyDescriptor(&descs[ELEMENT_INDEX_ONE], "propB", vals[ELEMENT_INDEX_ONE]);
    FillPropertyDescriptor(&descs[ELEMENT_INDEX_TWO], "propC", vals[ELEMENT_INDEX_TWO]);
    NAPI_CALL(env, napi_define_properties(env, obj, MULTI_PROPERTY_COUNT, descs));
    bool hasA = false;
    bool hasB = false;
    bool hasC = false;
    NAPI_CALL(env, napi_has_named_property(env, obj, "propA", &hasA));
    NAPI_CALL(env, napi_has_named_property(env, obj, "propB", &hasB));
    NAPI_CALL(env, napi_has_named_property(env, obj, "propC", &hasC));

    SetNamedBool(env, result, "multiDefineSuccess", hasA && hasB && hasC);
    SetNamedInt32(env, result, "propertiesDefined", MULTI_PROPERTY_COUNT);
    return result;
}

// ---------------------------------------------------------------------------
// Test 21: napi_object_freeze
// ---------------------------------------------------------------------------
static napi_value TestObjectFreeze(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "frozenPropA", TEST_INT_A);
    SetNamedInt32(env, obj, "frozenPropB", TEST_INT_B);
    NAPI_CALL(env, napi_object_freeze(env, obj));
    napi_value propA = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, "frozenPropA", &propA));
    int32_t valA = 0;
    NAPI_CALL(env, napi_get_value_int32(env, propA, &valA));

    SetNamedBool(env, result, "freezeSuccess", true);
    SetNamedBool(env, result, "frozenValueIntact", valA == TEST_INT_A);
    SetNamedInt32(env, result, "frozenPropCount", FREEZE_PROP_COUNT);
    return result;
}

// ---------------------------------------------------------------------------
// Test 22: napi_object_seal
// ---------------------------------------------------------------------------
static napi_value TestObjectSeal(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "sealedProp", SEAL_ORIGINAL_VALUE);
    NAPI_CALL(env, napi_object_seal(env, obj));
    bool hasProp = false;
    NAPI_CALL(env, napi_has_named_property(env, obj, "sealedProp", &hasProp));
    napi_value propVal = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, "sealedProp", &propVal));
    int32_t readVal = 0;
    NAPI_CALL(env, napi_get_value_int32(env, propVal, &readVal));

    SetNamedBool(env, result, "sealSuccess", true);
    SetNamedBool(env, result, "sealedPropExists", hasProp);
    SetNamedBool(env, result, "sealedValueIntact", readVal == SEAL_ORIGINAL_VALUE);
    return result;
}

// ---------------------------------------------------------------------------
// Test 23: napi_get_all_property_names
// ---------------------------------------------------------------------------
static napi_value TestGetAllPropertyNames(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "alpha", TEST_INT_F);
    SetNamedInt32(env, obj, "beta", TEST_INT_G);
    SetNamedInt32(env, obj, "gamma", TEST_INT_H);
    napi_value allNames = nullptr;
    NAPI_CALL(env, napi_get_all_property_names(env, obj, napi_key_own_only, napi_key_all_properties,
                                               napi_key_numbers_to_strings, &allNames));
    uint32_t nameCount = 0;
    NAPI_CALL(env, napi_get_array_length(env, allNames, &nameCount));

    SetNamedBool(env, result, "getAllNamesSuccess", true);
    SetNamedBool(env, result, "countCorrect", nameCount == static_cast<uint32_t>(MULTI_PROPERTY_COUNT));
    SetNamedInt32(env, result, "totalNames", static_cast<int32_t>(nameCount));
    return result;
}

// ---------------------------------------------------------------------------
// Test 24: napi_get_property_names
// ---------------------------------------------------------------------------
static napi_value TestGetPropertyNames(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "visible", TEST_INT_A);
    SetNamedInt32(env, obj, "alsoVisible", TEST_INT_B);
    napi_value names = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, obj, &names));
    uint32_t count = 0;
    NAPI_CALL(env, napi_get_array_length(env, names, &count));

    SetNamedBool(env, result, "getNamesSuccess", true);
    SetNamedBool(env, result, "hasNames", count > 0);
    SetNamedInt32(env, result, "nameCount", static_cast<int32_t>(count));
    return result;
}

// ---------------------------------------------------------------------------
// Test 25: napi_set_property / napi_get_property
// ---------------------------------------------------------------------------
static napi_value TestSetGetProperty(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value key = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "testKey", NAPI_AUTO_LENGTH, &key));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_D, &val));
    NAPI_CALL(env, napi_set_property(env, obj, key, val));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_property(env, obj, key, &retrieved));
    int32_t readVal = 0;
    NAPI_CALL(env, napi_get_value_int32(env, retrieved, &readVal));

    SetNamedBool(env, result, "setGetSuccess", true);
    SetNamedBool(env, result, "valueMatch", readVal == TEST_INT_D);
    SetNamedInt32(env, result, "retrievedValue", readVal);
    return result;
}

// ---------------------------------------------------------------------------
// Test 26: napi_has_property
// ---------------------------------------------------------------------------
static napi_value TestHasProperty(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value key = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "existingKey", NAPI_AUTO_LENGTH, &key));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_A, &val));
    NAPI_CALL(env, napi_set_property(env, obj, key, val));
    bool hasExisting = false;
    NAPI_CALL(env, napi_has_property(env, obj, key, &hasExisting));
    napi_value missingKey = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "missingKey", NAPI_AUTO_LENGTH, &missingKey));
    bool hasMissing = false;
    NAPI_CALL(env, napi_has_property(env, obj, missingKey, &hasMissing));

    SetNamedBool(env, result, "existingFound", hasExisting);
    SetNamedBool(env, result, "missingNotFound", !hasMissing);
    return result;
}

// ---------------------------------------------------------------------------
// Test 27: napi_delete_property
// ---------------------------------------------------------------------------
static napi_value TestDeleteProperty(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value key = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "toDelete", NAPI_AUTO_LENGTH, &key));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_E, &val));
    NAPI_CALL(env, napi_set_property(env, obj, key, val));
    bool deleteResult = false;
    NAPI_CALL(env, napi_delete_property(env, obj, key, &deleteResult));
    bool hasAfterDelete = false;
    NAPI_CALL(env, napi_has_property(env, obj, key, &hasAfterDelete));

    SetNamedBool(env, result, "deleteReturned", deleteResult);
    SetNamedBool(env, result, "propertyGone", !hasAfterDelete);
    return result;
}

// ---------------------------------------------------------------------------
// Test 28: property operations with numeric key
// ---------------------------------------------------------------------------
static napi_value TestPropertyWithNumericKey(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value key = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_A, &key));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "numericKeyValue", NAPI_AUTO_LENGTH, &val));
    NAPI_CALL(env, napi_set_property(env, obj, key, val));
    bool hasProp = false;
    NAPI_CALL(env, napi_has_property(env, obj, key, &hasProp));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_property(env, obj, key, &retrieved));
    napi_valuetype valType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, retrieved, &valType));

    SetNamedBool(env, result, "numericKeySet", hasProp);
    SetNamedBool(env, result, "valueIsString", valType == napi_string);
    return result;
}

// ---------------------------------------------------------------------------
// Test 29: napi_has_own_property
// ---------------------------------------------------------------------------
static napi_value TestHasOwnProperty(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "ownProp", TEST_INT_A);
    napi_value key = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "ownProp", NAPI_AUTO_LENGTH, &key));
    bool hasOwn = false;
    NAPI_CALL(env, napi_has_own_property(env, obj, key, &hasOwn));
    napi_value inheritedKey = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "toString", NAPI_AUTO_LENGTH, &inheritedKey));
    bool hasInherited = false;
    NAPI_CALL(env, napi_has_own_property(env, obj, inheritedKey, &hasInherited));

    SetNamedBool(env, result, "ownPropFound", hasOwn);
    SetNamedBool(env, result, "inheritedNotOwn", !hasInherited);
    return result;
}

// ---------------------------------------------------------------------------
// Test 30: napi_get_prototype
// ---------------------------------------------------------------------------
static napi_value TestGetPrototype(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value prototype = nullptr;
    NAPI_CALL(env, napi_get_prototype(env, obj, &prototype));
    napi_valuetype protoType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, prototype, &protoType));
    bool isObjectOrNull = (protoType == napi_object || protoType == napi_null);

    SetNamedBool(env, result, "prototypeRetrieved", true);
    SetNamedBool(env, result, "prototypeTypeValid", isObjectOrNull);
    return result;
}

// ---------------------------------------------------------------------------
// Test 31: napi_set_element / napi_get_element
// ---------------------------------------------------------------------------
static napi_value TestSetGetElement(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value arr = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, TEST_ARRAY_LENGTH, &arr));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_J, &val));
    NAPI_CALL(env, napi_set_element(env, arr, ELEMENT_INDEX_ZERO, val));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_element(env, arr, ELEMENT_INDEX_ZERO, &retrieved));
    int32_t readVal = 0;
    NAPI_CALL(env, napi_get_value_int32(env, retrieved, &readVal));

    SetNamedBool(env, result, "setElementSuccess", true);
    SetNamedBool(env, result, "getElementMatch", readVal == TEST_INT_J);
    SetNamedInt32(env, result, "retrievedElement", readVal);
    return result;
}

// ---------------------------------------------------------------------------
// Test 32: napi_has_element / napi_delete_element
// ---------------------------------------------------------------------------
static napi_value TestHasDeleteElement(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value arr = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, TEST_ARRAY_LENGTH, &arr));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_B, &val));
    NAPI_CALL(env, napi_set_element(env, arr, ELEMENT_INDEX_ONE, val));
    bool hasOne = false;
    NAPI_CALL(env, napi_has_element(env, arr, ELEMENT_INDEX_ONE, &hasOne));
    bool hasFour = false;
    NAPI_CALL(env, napi_has_element(env, arr, ELEMENT_INDEX_FOUR, &hasFour));
    bool deleteOk = false;
    NAPI_CALL(env, napi_delete_element(env, arr, ELEMENT_INDEX_ONE, &deleteOk));
    bool hasAfterDelete = false;
    NAPI_CALL(env, napi_has_element(env, arr, ELEMENT_INDEX_ONE, &hasAfterDelete));

    SetNamedBool(env, result, "hasSetElement", hasOne);
    SetNamedBool(env, result, "missingElement", !hasFour);
    SetNamedBool(env, result, "deleteElementOk", deleteOk);
    SetNamedBool(env, result, "elementGone", !hasAfterDelete);
    return result;
}

// ---------------------------------------------------------------------------
// Test 33: element operations at boundary conditions
// ---------------------------------------------------------------------------
static napi_value TestElementBoundaryConditions(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value arr = nullptr;
    NAPI_CALL(env, napi_create_array(env, &arr));
    // Zero index
    napi_value valZero = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_C, &valZero));
    NAPI_CALL(env, napi_set_element(env, arr, ELEMENT_INDEX_ZERO, valZero));
    napi_value getZero = nullptr;
    NAPI_CALL(env, napi_get_element(env, arr, ELEMENT_INDEX_ZERO, &getZero));
    int32_t zeroVal = 0;
    NAPI_CALL(env, napi_get_value_int32(env, getZero, &zeroVal));
    // Large index
    napi_value valLarge = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_D, &valLarge));
    NAPI_CALL(env, napi_set_element(env, arr, ELEMENT_LARGE_INDEX, valLarge));
    bool hasLarge = false;
    NAPI_CALL(env, napi_has_element(env, arr, ELEMENT_LARGE_INDEX, &hasLarge));

    SetNamedBool(env, result, "zeroIndexMatch", zeroVal == TEST_INT_C);
    SetNamedBool(env, result, "largeIndexSet", hasLarge);
    SetNamedInt32(env, result, "largeIndex", static_cast<int32_t>(ELEMENT_LARGE_INDEX));
    return result;
}

// ---------------------------------------------------------------------------
// Test 34: escape handle with object containing properties
// ---------------------------------------------------------------------------
static napi_value TestEscapeHandleWithProps(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_escapable_handle_scope scope = nullptr;
    NAPI_CALL(env, napi_open_escapable_handle_scope(env, &scope));
    napi_value innerObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &innerObj));
    SetNamedInt32(env, innerObj, "data", TEST_INT_E);
    SetNamedString(env, innerObj, "label", "escaped_obj");
    napi_value escaped = nullptr;
    NAPI_CALL(env, napi_escape_handle(env, scope, innerObj, &escaped));
    NAPI_CALL(env, napi_close_escapable_handle_scope(env, scope));
    napi_value dataProp = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, escaped, "data", &dataProp));
    int32_t dataVal = 0;
    NAPI_CALL(env, napi_get_value_int32(env, dataProp, &dataVal));

    SetNamedBool(env, result, "escapeWithPropsSuccess", true);
    SetNamedBool(env, result, "propsPreserved", dataVal == TEST_INT_E);
    return result;
}

// ---------------------------------------------------------------------------
// Test 35: reference to array object
// ---------------------------------------------------------------------------
static napi_value TestReferenceToArray(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value arr = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, TEST_ARRAY_LENGTH, &arr));
    napi_value elem = nullptr;
    NAPI_CALL(env, napi_create_int32(env, PROTOTYPE_MARKER, &elem));
    NAPI_CALL(env, napi_set_element(env, arr, ELEMENT_INDEX_ZERO, elem));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, arr, INITIAL_REF_COUNT, &ref));
    napi_value refArr = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &refArr));
    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, refArr, &isArray));
    napi_value refElem = nullptr;
    NAPI_CALL(env, napi_get_element(env, refArr, ELEMENT_INDEX_ZERO, &refElem));
    int32_t elemVal = 0;
    NAPI_CALL(env, napi_get_value_int32(env, refElem, &elemVal));
    NAPI_CALL(env, napi_delete_reference(env, ref));

    SetNamedBool(env, result, "arrayRefCreated", isArray);
    SetNamedBool(env, result, "elementPreserved", elemVal == PROTOTYPE_MARKER);
    return result;
}

// ---------------------------------------------------------------------------
// Test 36: handle scope with double value
// ---------------------------------------------------------------------------
static napi_value TestHandleScopeDouble(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_handle_scope scope = nullptr;
    NAPI_CALL(env, napi_open_handle_scope(env, &scope));
    napi_value dblVal = nullptr;
    NAPI_CALL(env, napi_create_double(env, TEST_DOUBLE_A, &dblVal));
    double retrieved = 0.0;
    NAPI_CALL(env, napi_get_value_double(env, dblVal, &retrieved));
    NAPI_CALL(env, napi_close_handle_scope(env, scope));

    SetNamedBool(env, result, "doubleCreated", true);
    SetNamedBool(env, result, "doubleValueCorrect", retrieved == TEST_DOUBLE_A);
    SetNamedDouble(env, result, "doubleValue", retrieved);
    return result;
}

// ---------------------------------------------------------------------------
// Test 37: set/get property with double value
// ---------------------------------------------------------------------------
static napi_value TestSetGetPropertyDouble(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value key = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "doubleKey", NAPI_AUTO_LENGTH, &key));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_double(env, TEST_DOUBLE_B, &val));
    NAPI_CALL(env, napi_set_property(env, obj, key, val));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_property(env, obj, key, &retrieved));
    double readVal = 0.0;
    NAPI_CALL(env, napi_get_value_double(env, retrieved, &readVal));

    SetNamedBool(env, result, "doubleSetGetSuccess", true);
    SetNamedBool(env, result, "doubleValueMatch", readVal == TEST_DOUBLE_B);
    SetNamedDouble(env, result, "retrievedDouble", readVal);
    return result;
}

// ---------------------------------------------------------------------------
// Test 38: freeze then verify property names still accessible
// ---------------------------------------------------------------------------
static napi_value TestFreezePropertyNames(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "frozenA", TEST_INT_F);
    SetNamedInt32(env, obj, "frozenB", TEST_INT_G);
    NAPI_CALL(env, napi_object_freeze(env, obj));
    napi_value names = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, obj, &names));
    uint32_t nameCount = 0;
    NAPI_CALL(env, napi_get_array_length(env, names, &nameCount));

    SetNamedBool(env, result, "frozenNamesAccessible", true);
    SetNamedBool(env, result, "countCorrect", nameCount == static_cast<uint32_t>(FREEZE_PROP_COUNT));
    SetNamedInt32(env, result, "nameCount", static_cast<int32_t>(nameCount));
    return result;
}

// ---------------------------------------------------------------------------
// Registration Table
// ---------------------------------------------------------------------------
static const napi_property_descriptor SCOPE_TESTS[] = {
    { "testOpenCloseHandleScope", nullptr, TestOpenCloseHandleScope, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testHandleScopeObjects", nullptr, TestHandleScopeObjects, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testNestedHandleScopes", nullptr, TestNestedHandleScopes, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testDeepNestedHandleScopes", nullptr, TestDeepNestedHandleScopes, nullptr, nullptr, nullptr, napi_default,
      nullptr },
    { "testOpenCloseEscapableScope", nullptr, TestOpenCloseEscapableScope, nullptr, nullptr, nullptr, napi_default,
      nullptr },
    { "testEscapeHandleInt", nullptr, TestEscapeHandleInt, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testEscapableWithNestedScope", nullptr, TestEscapableWithNestedScope, nullptr, nullptr, nullptr, napi_default,
      nullptr },
    { "testCreateDeleteReference", nullptr, TestCreateDeleteReference, nullptr, nullptr, nullptr, napi_default,
      nullptr },
    { "testReferenceRef", nullptr, TestReferenceRef, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testReferenceUnref", nullptr, TestReferenceUnref, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testGetReferenceValue", nullptr, TestGetReferenceValue, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testMultipleReferenceLifecycle", nullptr, TestMultipleReferenceLifecycle, nullptr, nullptr, nullptr,
      napi_default, nullptr },
    { "testWeakReference", nullptr, TestWeakReference, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testRefUnrefRoundTrip", nullptr, TestRefUnrefRoundTrip, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testGetVersion", nullptr, TestGetVersion, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testPropertyWritable", nullptr, TestPropertyWritable, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testPropertyEnumerable", nullptr, TestPropertyEnumerable, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testPropertyConfigurable", nullptr, TestPropertyConfigurable, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testPropertyAllAttributes", nullptr, TestPropertyAllAttributes, nullptr, nullptr, nullptr, napi_default,
      nullptr },
    { "testDefineMultipleProperties", nullptr, TestDefineMultipleProperties, nullptr, nullptr, nullptr, napi_default,
      nullptr },
    { "testObjectFreeze", nullptr, TestObjectFreeze, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testObjectSeal", nullptr, TestObjectSeal, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testGetAllPropertyNames", nullptr, TestGetAllPropertyNames, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testGetPropertyNames", nullptr, TestGetPropertyNames, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testSetGetProperty", nullptr, TestSetGetProperty, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testHasProperty", nullptr, TestHasProperty, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testDeleteProperty", nullptr, TestDeleteProperty, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testPropertyWithNumericKey", nullptr, TestPropertyWithNumericKey, nullptr, nullptr, nullptr, napi_default,
      nullptr },
    { "testHasOwnProperty", nullptr, TestHasOwnProperty, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testGetPrototype", nullptr, TestGetPrototype, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testSetGetElement", nullptr, TestSetGetElement, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testHasDeleteElement", nullptr, TestHasDeleteElement, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testElementBoundaryConditions", nullptr, TestElementBoundaryConditions, nullptr, nullptr, nullptr, napi_default,
      nullptr },
    { "testEscapeHandleWithProps", nullptr, TestEscapeHandleWithProps, nullptr, nullptr, nullptr, napi_default,
      nullptr },
    { "testReferenceToArray", nullptr, TestReferenceToArray, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testHandleScopeDouble", nullptr, TestHandleScopeDouble, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testSetGetPropertyDouble", nullptr, TestSetGetPropertyDouble, nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testFreezePropertyNames", nullptr, TestFreezePropertyNames, nullptr, nullptr, nullptr, napi_default, nullptr },
};

static constexpr size_t SCOPE_TEST_DESCRIPTOR_COUNT = sizeof(SCOPE_TESTS) / sizeof(SCOPE_TESTS[0]);

} // namespace

// ---------------------------------------------------------------------------
// Module Initialization and Registration
// ---------------------------------------------------------------------------
static napi_value InitScopeSuite(napi_env env, napi_value exports)
{
    NAPI_CALL(env, napi_define_properties(env, exports, SCOPE_TEST_DESCRIPTOR_COUNT, SCOPE_TESTS));
    return exports;
}

static napi_module g_scopeSuiteModule = {
    .nm_version = MODULE_VERSION,
    .nm_flags = NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitScopeSuite,
    .nm_modname = "scope_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterScopeSuiteModule(void)
{
    napi_module_register(&g_scopeSuiteModule);
}
