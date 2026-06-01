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
#include <cstring>
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

namespace {

// ============================================================================
// Named Constants — no magic numbers allowed
// ============================================================================

static constexpr int32_t TEST_INT_MARKER = 42;
static constexpr int32_t TEST_INT_LARGE = 999;
static constexpr int32_t TEST_INT_MEDIUM = 200;
static constexpr int32_t TEST_INT_SMALL = 10;
static constexpr int32_t TEST_INT_FIFTY = 50;
static constexpr int32_t TEST_INT_TWENTY = 20;
static constexpr int32_t TEST_INT_THIRTY = 30;
static constexpr int64_t EXTERNAL_MEMORY_DELTA = 1024;
static constexpr int64_t EXTERNAL_MEMORY_LARGE = 65536;
static constexpr int64_t EXTERNAL_MEMORY_NEGATIVE = -512;
static constexpr int64_t EXTERNAL_MEMORY_ZERO = 0;
static constexpr uint32_t INITIAL_REF_COUNT = 1;
static constexpr uint32_t WEAK_REF_COUNT = 0;
static constexpr uint32_t REF_COUNT_ZERO = 0;
static constexpr uint32_t REF_COUNT_ONE = 1;
static constexpr uint32_t REF_COUNT_TWO = 2;
static constexpr uint32_t REF_COUNT_THREE = 3;
static constexpr int32_t NESTED_SCOPE_DEPTH = 3;
static constexpr int32_t DEEP_NEST_DEPTH = 5;
static constexpr int32_t STRESS_SCOPE_DEPTH = 8;
static constexpr int32_t STRESS_OBJECT_COUNT = 50;
static constexpr int32_t MULTI_REF_COUNT = 5;
static constexpr int32_t REF_CYCLE_ROUNDS = 4;
static constexpr uint32_t MIN_NAPI_VERSION = 1;
static constexpr uint32_t MODULE_VERSION = 1;
static constexpr uint32_t NO_MODULE_FLAGS = 0;
static constexpr size_t SINGLE_DESCRIPTOR = 1;
static constexpr int32_t CLEANUP_MARKER_VALUE = 777;
static constexpr size_t STRING_BUFFER_SIZE = 256;

// Cleanup hook tracking
static int32_t g_cleanupHookCallCount = 0;
static int32_t g_cleanupHookArg = 0;
static int32_t g_asyncCleanupHookCallCount = 0;

// ============================================================================
// Helper Functions
// ============================================================================

static void SetNamedBool(napi_env env, napi_value obj, const char* name, bool value)
{
    napi_value napiValue = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, value, &napiValue));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, obj, name, napiValue));
}

static void SetNamedInt32(napi_env env, napi_value obj, const char* name, int32_t value)
{
    napi_value napiValue = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, value, &napiValue));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, obj, name, napiValue));
}

static void SetNamedString(napi_env env, napi_value obj, const char* name, const char* value)
{
    napi_value napiValue = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &napiValue));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, obj, name, napiValue));
}

static napi_value CreateResult(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    return result;
}

// ============================================================================
// Cleanup Hook Callbacks
// ============================================================================

static void EnvCleanupCallback(void* arg)
{
    g_cleanupHookCallCount++;
    if (arg != nullptr) {
        g_cleanupHookArg = *static_cast<int32_t*>(arg);
    }
}

static void EnvCleanupCallbackNoArg(void* /* arg */)
{
    g_cleanupHookCallCount++;
}

static void AsyncCleanupHookCallback(napi_async_cleanup_hook_handle handle, void* /* data */)
{
    g_asyncCleanupHookCallCount++;
    if (handle != nullptr) {
        napi_remove_async_cleanup_hook(handle);
    }
}

// ============================================================================
// Test 1: napi_async_init / napi_async_destroy — basic lifecycle
// ============================================================================

static napi_value TestAsyncInitDestroy(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "testAsyncResource",
        NAPI_AUTO_LENGTH, &resourceName));
    napi_async_context context = nullptr;
    NAPI_CALL(env, napi_async_init(env, nullptr, resourceName, &context));
    bool initSuccess = (context != nullptr);
    NAPI_CALL(env, napi_async_destroy(env, context));

    SetNamedBool(env, result, "initSuccess", initSuccess);
    SetNamedBool(env, result, "destroySuccess", true);
    return result;
}

// ============================================================================
// Test 2: napi_async_init with resource object
// ============================================================================

static napi_value TestAsyncInitWithResource(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value resource = nullptr;
    NAPI_CALL(env, napi_create_object(env, &resource));
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "asyncWithResource",
        NAPI_AUTO_LENGTH, &resourceName));
    napi_async_context context = nullptr;
    NAPI_CALL(env, napi_async_init(env, resource, resourceName, &context));
    bool contextValid = (context != nullptr);
    NAPI_CALL(env, napi_async_destroy(env, context));

    SetNamedBool(env, result, "contextValid", contextValid);
    SetNamedBool(env, result, "resourceUsed", true);
    return result;
}

// ============================================================================
// Test 3: napi_open_callback_scope / napi_close_callback_scope
// ============================================================================

static napi_value TestCallbackScope(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value resource = nullptr;
    NAPI_CALL(env, napi_create_object(env, &resource));
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "callbackScopeTest",
        NAPI_AUTO_LENGTH, &resourceName));
    napi_async_context context = nullptr;
    NAPI_CALL(env, napi_async_init(env, resource, resourceName, &context));
    napi_callback_scope scope = nullptr;
    NAPI_CALL(env, napi_open_callback_scope(env, resource, context, &scope));
    bool scopeValid = (scope != nullptr);
    NAPI_CALL(env, napi_close_callback_scope(env, scope));
    NAPI_CALL(env, napi_async_destroy(env, context));

    SetNamedBool(env, result, "scopeOpened", scopeValid);
    SetNamedBool(env, result, "scopeClosed", true);
    return result;
}

// ============================================================================
// Test 4: callback scope with value creation inside
// ============================================================================

static napi_value TestCallbackScopeWithValue(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value resource = nullptr;
    NAPI_CALL(env, napi_create_object(env, &resource));
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "cbScopeValue",
        NAPI_AUTO_LENGTH, &resourceName));
    napi_async_context context = nullptr;
    NAPI_CALL(env, napi_async_init(env, resource, resourceName, &context));
    napi_callback_scope scope = nullptr;
    NAPI_CALL(env, napi_open_callback_scope(env, resource, context, &scope));
    napi_value innerVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_MARKER, &innerVal));
    int32_t retrieved = 0;
    NAPI_CALL(env, napi_get_value_int32(env, innerVal, &retrieved));
    NAPI_CALL(env, napi_close_callback_scope(env, scope));
    NAPI_CALL(env, napi_async_destroy(env, context));

    SetNamedBool(env, result, "valueCreated", true);
    SetNamedBool(env, result, "valueCorrect", retrieved == TEST_INT_MARKER);
    return result;
}

// ============================================================================
// Test 5: napi_add_env_cleanup_hook / napi_remove_env_cleanup_hook
// ============================================================================

static napi_value TestEnvCleanupHook(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    static int32_t cleanupArg = CLEANUP_MARKER_VALUE;
    napi_status addStatus = napi_add_env_cleanup_hook(env, EnvCleanupCallback, &cleanupArg);
    bool addSuccess = (addStatus == napi_ok);
    napi_status removeStatus = napi_remove_env_cleanup_hook(env, EnvCleanupCallback, &cleanupArg);
    bool removeSuccess = (removeStatus == napi_ok);

    SetNamedBool(env, result, "hookAdded", addSuccess);
    SetNamedBool(env, result, "hookRemoved", removeSuccess);
    SetNamedInt32(env, result, "markerValue", CLEANUP_MARKER_VALUE);
    return result;
}

// ============================================================================
// Test 6: add and remove cleanup hook with nullptr arg
// ============================================================================

static napi_value TestEnvCleanupHookNullArg(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_status addStatus = napi_add_env_cleanup_hook(env, EnvCleanupCallbackNoArg, nullptr);
    bool addSuccess = (addStatus == napi_ok);
    napi_status removeStatus = napi_remove_env_cleanup_hook(env, EnvCleanupCallbackNoArg, nullptr);
    bool removeSuccess = (removeStatus == napi_ok);

    SetNamedBool(env, result, "addNullArgSuccess", addSuccess);
    SetNamedBool(env, result, "removeNullArgSuccess", removeSuccess);
    return result;
}

// ============================================================================
// Test 7: napi_add_async_cleanup_hook / napi_remove_async_cleanup_hook
// ============================================================================

static napi_value TestAsyncCleanupHook(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_async_cleanup_hook_handle handle = nullptr;
    napi_status addStatus = napi_add_async_cleanup_hook(
        env, AsyncCleanupHookCallback, nullptr, &handle);
    bool addSuccess = (addStatus == napi_ok);
    bool handleValid = (handle != nullptr);
    napi_status removeStatus = napi_remove_async_cleanup_hook(handle);
    bool removeSuccess = (removeStatus == napi_ok);

    SetNamedBool(env, result, "asyncHookAdded", addSuccess);
    SetNamedBool(env, result, "handleValid", handleValid);
    SetNamedBool(env, result, "asyncHookRemoved", removeSuccess);
    return result;
}

// ============================================================================
// Test 8: napi_open_handle_scope / napi_close_handle_scope — basic
// ============================================================================

static napi_value TestOpenCloseHandleScope(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_handle_scope scope = nullptr;
    NAPI_CALL(env, napi_open_handle_scope(env, &scope));
    napi_value testValue = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_MARKER, &testValue));
    int32_t retrieved = 0;
    NAPI_CALL(env, napi_get_value_int32(env, testValue, &retrieved));
    NAPI_CALL(env, napi_close_handle_scope(env, scope));

    SetNamedBool(env, result, "scopeOpenClose", true);
    SetNamedBool(env, result, "valueCorrect", retrieved == TEST_INT_MARKER);
    return result;
}

// ============================================================================
// Test 9: objects created inside a handle scope
// ============================================================================

static napi_value TestHandleScopeObjects(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

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

// ============================================================================
// Helper: recursively create value in nested scopes
// ============================================================================

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

// ============================================================================
// Test 10: nested handle scopes
// ============================================================================

static napi_value TestNestedHandleScopes(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value innerVal = CreateNestedScopeValue(env, NESTED_SCOPE_DEPTH, TEST_INT_MEDIUM);
    int32_t retrieved = 0;
    NAPI_CALL(env, napi_get_value_int32(env, innerVal, &retrieved));

    SetNamedBool(env, result, "nestedSuccess", true);
    SetNamedBool(env, result, "valuePreserved", retrieved == TEST_INT_MEDIUM);
    SetNamedInt32(env, result, "depth", NESTED_SCOPE_DEPTH);
    return result;
}

// ============================================================================
// Test 11: deeply nested handle scopes
// ============================================================================

static napi_value TestDeepNestedHandleScopes(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value deepVal = CreateNestedScopeValue(env, DEEP_NEST_DEPTH, TEST_INT_LARGE);
    int32_t retrieved = 0;
    NAPI_CALL(env, napi_get_value_int32(env, deepVal, &retrieved));

    SetNamedBool(env, result, "deepNestedSuccess", true);
    SetNamedBool(env, result, "valuePreserved", retrieved == TEST_INT_LARGE);
    SetNamedInt32(env, result, "depth", DEEP_NEST_DEPTH);
    return result;
}

// ============================================================================
// Test 12: stress test — many scopes opened and closed
// ============================================================================

static napi_value TestStressHandleScopes(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value stressVal = CreateNestedScopeValue(env, STRESS_SCOPE_DEPTH, TEST_INT_FIFTY);
    int32_t retrieved = 0;
    NAPI_CALL(env, napi_get_value_int32(env, stressVal, &retrieved));

    SetNamedBool(env, result, "stressSuccess", true);
    SetNamedBool(env, result, "valueIntact", retrieved == TEST_INT_FIFTY);
    SetNamedInt32(env, result, "stressDepth", STRESS_SCOPE_DEPTH);
    return result;
}

// ============================================================================
// Helper: allocate many objects inside a single scope
// ============================================================================

static bool AllocateManyObjectsInScope(napi_env env, int32_t count)
{
    napi_handle_scope scope = nullptr;
    if (napi_open_handle_scope(env, &scope) != napi_ok) {
        return false;
    }
    for (int32_t i = 0; i < count; i++) {
        napi_value obj = nullptr;
        if (napi_create_object(env, &obj) != napi_ok) {
            napi_close_handle_scope(env, scope);
            return false;
        }
    }
    napi_close_handle_scope(env, scope);
    return true;
}

// ============================================================================
// Test 13: handle scope with many object allocations
// ============================================================================

static napi_value TestHandleScopeManyObjects(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    bool allocSuccess = AllocateManyObjectsInScope(env, STRESS_OBJECT_COUNT);

    SetNamedBool(env, result, "manyObjectsAllocated", allocSuccess);
    SetNamedInt32(env, result, "objectCount", STRESS_OBJECT_COUNT);
    return result;
}

// ============================================================================
// Test 14: napi_open_escapable_handle_scope basic
// ============================================================================

static napi_value TestOpenCloseEscapableScope(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

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

// ============================================================================
// Test 15: napi_escape_handle with integer value
// ============================================================================

static napi_value TestEscapeHandleInt(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_escapable_handle_scope scope = nullptr;
    NAPI_CALL(env, napi_open_escapable_handle_scope(env, &scope));
    napi_value innerValue = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_MARKER, &innerValue));
    napi_value escaped = nullptr;
    NAPI_CALL(env, napi_escape_handle(env, scope, innerValue, &escaped));
    NAPI_CALL(env, napi_close_escapable_handle_scope(env, scope));
    int32_t retrieved = 0;
    NAPI_CALL(env, napi_get_value_int32(env, escaped, &retrieved));

    SetNamedBool(env, result, "escapeSuccess", true);
    SetNamedBool(env, result, "valueIntact", retrieved == TEST_INT_MARKER);
    SetNamedInt32(env, result, "escapedValue", retrieved);
    return result;
}

// ============================================================================
// Test 16: escape handle with string value
// ============================================================================

static napi_value TestEscapeHandleString(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_escapable_handle_scope scope = nullptr;
    NAPI_CALL(env, napi_open_escapable_handle_scope(env, &scope));
    napi_value innerStr = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "escaped_string",
        NAPI_AUTO_LENGTH, &innerStr));
    napi_value escaped = nullptr;
    NAPI_CALL(env, napi_escape_handle(env, scope, innerStr, &escaped));
    NAPI_CALL(env, napi_close_escapable_handle_scope(env, scope));
    napi_valuetype valType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, escaped, &valType));

    SetNamedBool(env, result, "stringEscapeSuccess", true);
    SetNamedBool(env, result, "escapedIsString", valType == napi_string);
    return result;
}

// ============================================================================
// Test 17: escapable scope with nested regular scope
// ============================================================================

static napi_value TestEscapableWithNestedScope(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_escapable_handle_scope escScope = nullptr;
    NAPI_CALL(env, napi_open_escapable_handle_scope(env, &escScope));
    napi_handle_scope innerScope = nullptr;
    NAPI_CALL(env, napi_open_handle_scope(env, &innerScope));
    napi_value innerStr = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "nested_escaped",
        NAPI_AUTO_LENGTH, &innerStr));
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

// ============================================================================
// Test 18: escape handle with object and properties
// ============================================================================

static napi_value TestEscapeHandleWithProps(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_escapable_handle_scope scope = nullptr;
    NAPI_CALL(env, napi_open_escapable_handle_scope(env, &scope));
    napi_value innerObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &innerObj));
    SetNamedInt32(env, innerObj, "marker", TEST_INT_MARKER);
    napi_value escaped = nullptr;
    NAPI_CALL(env, napi_escape_handle(env, scope, innerObj, &escaped));
    NAPI_CALL(env, napi_close_escapable_handle_scope(env, scope));
    napi_value markerOut = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, escaped, "marker", &markerOut));
    int32_t markerVal = 0;
    NAPI_CALL(env, napi_get_value_int32(env, markerOut, &markerVal));

    SetNamedBool(env, result, "escapeWithPropsSuccess", true);
    SetNamedBool(env, result, "markerMatch", markerVal == TEST_INT_MARKER);
    return result;
}

// ============================================================================
// Test 19: napi_create_reference / napi_delete_reference
// ============================================================================

static napi_value TestCreateDeleteReference(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

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

// ============================================================================
// Test 20: napi_reference_ref (increment refcount)
// ============================================================================

static napi_value TestReferenceRef(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

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

// ============================================================================
// Test 21: napi_reference_unref (decrement refcount)
// ============================================================================

static napi_value TestReferenceUnref(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

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

// ============================================================================
// Test 22: napi_get_reference_value with marker property
// ============================================================================

static napi_value TestGetReferenceValue(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value original = nullptr;
    NAPI_CALL(env, napi_create_object(env, &original));
    SetNamedInt32(env, original, "marker", TEST_INT_MARKER);
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
    SetNamedBool(env, result, "markerMatch", markerVal == TEST_INT_MARKER);
    return result;
}

// ============================================================================
// Test 23: weak reference (refcount = 0)
// ============================================================================

static napi_value TestWeakReference(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

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

// ============================================================================
// Test 24: ref/unref round-trip preserving refcount
// ============================================================================

static napi_value TestRefUnrefRoundTrip(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

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

// ============================================================================
// Helper: manage a single reference lifecycle
// ============================================================================

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

// ============================================================================
// Test 25: multiple reference lifecycle management
// ============================================================================

static napi_value TestMultipleReferenceLifecycle(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    bool allPassed = true;
    for (int32_t i = 0; i < MULTI_REF_COUNT; i++) {
        napi_value obj = nullptr;
        NAPI_CALL(env, napi_create_object(env, &obj));
        uint32_t finalCount = 0;
        if (!ManageSingleRef(env, obj, &finalCount) || finalCount != REF_COUNT_ONE) {
            allPassed = false;
        }
    }

    SetNamedBool(env, result, "multiRefSuccess", allPassed);
    SetNamedInt32(env, result, "refsTested", MULTI_REF_COUNT);
    return result;
}

// ============================================================================
// Test 26: reference counting — ref multiple times then unref
// ============================================================================

static napi_value TestRefMultipleTimes(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, obj, INITIAL_REF_COUNT, &ref));
    uint32_t count = 0;
    NAPI_CALL(env, napi_reference_ref(env, ref, &count));
    NAPI_CALL(env, napi_reference_ref(env, ref, &count));
    bool peakCorrect = (count == REF_COUNT_THREE);
    NAPI_CALL(env, napi_reference_unref(env, ref, &count));
    NAPI_CALL(env, napi_reference_unref(env, ref, &count));
    bool backToOne = (count == REF_COUNT_ONE);
    NAPI_CALL(env, napi_delete_reference(env, ref));

    SetNamedBool(env, result, "multiRefSuccess", peakCorrect);
    SetNamedBool(env, result, "unrefBackToOne", backToOne);
    SetNamedInt32(env, result, "peakCount", static_cast<int32_t>(REF_COUNT_THREE));
    return result;
}

// ============================================================================
// Helper: perform ref-unref cycles on a reference
// ============================================================================

static bool PerformRefUnrefCycles(napi_env env, napi_ref ref, int32_t rounds)
{
    uint32_t count = 0;
    for (int32_t i = 0; i < rounds; i++) {
        if (napi_reference_ref(env, ref, &count) != napi_ok) {
            return false;
        }
    }
    for (int32_t i = 0; i < rounds; i++) {
        if (napi_reference_unref(env, ref, &count) != napi_ok) {
            return false;
        }
    }
    return (count == REF_COUNT_ONE);
}

// ============================================================================
// Test 27: reference counting edge cases — cycles of ref/unref
// ============================================================================

static napi_value TestRefUnrefCycles(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, obj, INITIAL_REF_COUNT, &ref));
    bool cycleSuccess = PerformRefUnrefCycles(env, ref, REF_CYCLE_ROUNDS);
    NAPI_CALL(env, napi_delete_reference(env, ref));

    SetNamedBool(env, result, "cycleSuccess", cycleSuccess);
    SetNamedInt32(env, result, "cycleRounds", REF_CYCLE_ROUNDS);
    return result;
}

// ============================================================================
// Test 28: weak ref — get reference value with refcount=0
// ============================================================================

static napi_value TestWeakRefGetValue(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    SetNamedInt32(env, obj, "data", TEST_INT_SMALL);
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, obj, WEAK_REF_COUNT, &ref));
    napi_value refValue = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &refValue));
    bool hasValue = (refValue != nullptr);
    NAPI_CALL(env, napi_delete_reference(env, ref));

    SetNamedBool(env, result, "weakRefQueried", true);
    SetNamedBool(env, result, "valueObtained", hasValue);
    return result;
}

// ============================================================================
// Test 29: create strong ref then unref to weak
// ============================================================================

static napi_value TestStrongToWeakRef(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, obj, INITIAL_REF_COUNT, &ref));
    uint32_t count = 0;
    NAPI_CALL(env, napi_reference_unref(env, ref, &count));
    bool isWeak = (count == REF_COUNT_ZERO);
    napi_value refValue = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &refValue));
    NAPI_CALL(env, napi_delete_reference(env, ref));

    SetNamedBool(env, result, "strongToWeakSuccess", isWeak);
    SetNamedBool(env, result, "weakRefDeleted", true);
    return result;
}

// ============================================================================
// Test 30: napi_get_version
// ============================================================================

static napi_value TestGetVersion(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    uint32_t version = 0;
    NAPI_CALL(env, napi_get_version(env, &version));

    SetNamedBool(env, result, "versionRetrieved", true);
    SetNamedBool(env, result, "versionValid", version >= MIN_NAPI_VERSION);
    SetNamedInt32(env, result, "version", static_cast<int32_t>(version));
    return result;
}

// ============================================================================
// Test 31: napi_adjust_external_memory — positive delta
// ============================================================================

static napi_value TestAdjustExternalMemory(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    int64_t adjustedValue = 0;
    napi_status status = napi_adjust_external_memory(env, EXTERNAL_MEMORY_DELTA, &adjustedValue);
    bool adjustSuccess = (status == napi_ok);

    SetNamedBool(env, result, "adjustSuccess", adjustSuccess);
    SetNamedInt32(env, result, "delta", static_cast<int32_t>(EXTERNAL_MEMORY_DELTA));
    return result;
}

// ============================================================================
// Test 32: napi_adjust_external_memory — large delta
// ============================================================================

static napi_value TestAdjustExternalMemoryLarge(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    int64_t adjustedValue = 0;
    napi_status status = napi_adjust_external_memory(env, EXTERNAL_MEMORY_LARGE, &adjustedValue);
    bool adjustSuccess = (status == napi_ok);

    SetNamedBool(env, result, "largeAdjustSuccess", adjustSuccess);
    SetNamedInt32(env, result, "largeDelta", static_cast<int32_t>(EXTERNAL_MEMORY_LARGE));
    return result;
}

// ============================================================================
// Test 33: napi_adjust_external_memory — negative delta
// ============================================================================

static napi_value TestAdjustExternalMemoryNegative(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    int64_t adjustedValue = 0;
    napi_status addStatus = napi_adjust_external_memory(env, EXTERNAL_MEMORY_DELTA, &adjustedValue);
    napi_status subStatus = napi_adjust_external_memory(
        env, EXTERNAL_MEMORY_NEGATIVE, &adjustedValue);

    SetNamedBool(env, result, "addSuccess", addStatus == napi_ok);
    SetNamedBool(env, result, "subtractSuccess", subStatus == napi_ok);
    return result;
}

// ============================================================================
// Test 34: napi_adjust_external_memory — zero delta
// ============================================================================

static napi_value TestAdjustExternalMemoryZero(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    int64_t adjustedValue = 0;
    napi_status status = napi_adjust_external_memory(env, EXTERNAL_MEMORY_ZERO, &adjustedValue);

    SetNamedBool(env, result, "zeroAdjustSuccess", status == napi_ok);
    return result;
}

// ============================================================================
// Test 35: handle scope double open/close pattern
// ============================================================================

static napi_value TestHandleScopeDoublePattern(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_handle_scope scopeA = nullptr;
    NAPI_CALL(env, napi_open_handle_scope(env, &scopeA));
    napi_value valA = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_TWENTY, &valA));
    NAPI_CALL(env, napi_close_handle_scope(env, scopeA));
    napi_handle_scope scopeB = nullptr;
    NAPI_CALL(env, napi_open_handle_scope(env, &scopeB));
    napi_value valB = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_THIRTY, &valB));
    int32_t retrievedB = 0;
    NAPI_CALL(env, napi_get_value_int32(env, valB, &retrievedB));
    NAPI_CALL(env, napi_close_handle_scope(env, scopeB));

    SetNamedBool(env, result, "doubleScopeSuccess", true);
    SetNamedBool(env, result, "secondValueCorrect", retrievedB == TEST_INT_THIRTY);
    return result;
}

// ============================================================================
// Test 36: reference to array object
// ============================================================================

static napi_value TestReferenceToArray(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value arr = nullptr;
    NAPI_CALL(env, napi_create_array(env, &arr));
    napi_value elem = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_MARKER, &elem));
    NAPI_CALL(env, napi_set_element(env, arr, REF_COUNT_ZERO, elem));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, arr, INITIAL_REF_COUNT, &ref));
    napi_value refValue = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &refValue));
    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, refValue, &isArray));
    NAPI_CALL(env, napi_delete_reference(env, ref));

    SetNamedBool(env, result, "refToArraySuccess", true);
    SetNamedBool(env, result, "isStillArray", isArray);
    return result;
}

// ============================================================================
// Test 37: add multiple cleanup hooks with different args
// ============================================================================

static napi_value TestMultipleCleanupHooks(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    static int32_t hookArgA = TEST_INT_SMALL;
    static int32_t hookArgB = TEST_INT_TWENTY;
    napi_status statusA = napi_add_env_cleanup_hook(env, EnvCleanupCallback, &hookArgA);
    napi_status statusB = napi_add_env_cleanup_hook(env, EnvCleanupCallback, &hookArgB);
    bool bothAdded = (statusA == napi_ok) && (statusB == napi_ok);
    napi_remove_env_cleanup_hook(env, EnvCleanupCallback, &hookArgA);
    napi_remove_env_cleanup_hook(env, EnvCleanupCallback, &hookArgB);

    SetNamedBool(env, result, "multipleHooksAdded", bothAdded);
    SetNamedBool(env, result, "allRemoved", true);
    return result;
}

// ============================================================================
// Test 38: async init/destroy — multiple resource names
// ============================================================================

static napi_value TestAsyncInitMultipleNames(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value nameA = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "resourceAlpha",
        NAPI_AUTO_LENGTH, &nameA));
    napi_async_context contextA = nullptr;
    NAPI_CALL(env, napi_async_init(env, nullptr, nameA, &contextA));
    napi_value nameB = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "resourceBeta",
        NAPI_AUTO_LENGTH, &nameB));
    napi_async_context contextB = nullptr;
    NAPI_CALL(env, napi_async_init(env, nullptr, nameB, &contextB));
    bool bothValid = (contextA != nullptr) && (contextB != nullptr);
    NAPI_CALL(env, napi_async_destroy(env, contextA));
    NAPI_CALL(env, napi_async_destroy(env, contextB));

    SetNamedBool(env, result, "multipleAsyncSuccess", bothValid);
    SetNamedBool(env, result, "bothDestroyed", true);
    return result;
}

// ============================================================================
// Test 39: escapable scope — escape object with string property
// ============================================================================

static napi_value TestEscapeObjectWithString(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_escapable_handle_scope scope = nullptr;
    NAPI_CALL(env, napi_open_escapable_handle_scope(env, &scope));
    napi_value innerObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &innerObj));
    SetNamedString(env, innerObj, "tag", "escapedTag");
    napi_value escaped = nullptr;
    NAPI_CALL(env, napi_escape_handle(env, scope, innerObj, &escaped));
    NAPI_CALL(env, napi_close_escapable_handle_scope(env, scope));
    napi_value tagOut = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, escaped, "tag", &tagOut));
    char buffer[STRING_BUFFER_SIZE] = {0};
    size_t copied = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(
        env, tagOut, buffer, STRING_BUFFER_SIZE, &copied));
    bool tagMatch = (strcmp(buffer, "escapedTag") == 0);

    SetNamedBool(env, result, "escapeObjStringSuccess", true);
    SetNamedBool(env, result, "tagMatch", tagMatch);
    SetNamedString(env, result, "retrievedTag", buffer);
    return result;
}

// ============================================================================
// Test 40: handle scope with array creation and element access
// ============================================================================

static napi_value TestHandleScopeWithArray(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_handle_scope scope = nullptr;
    NAPI_CALL(env, napi_open_handle_scope(env, &scope));
    napi_value arr = nullptr;
    NAPI_CALL(env, napi_create_array(env, &arr));
    napi_value elem = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_INT_LARGE, &elem));
    NAPI_CALL(env, napi_set_element(env, arr, REF_COUNT_ZERO, elem));
    uint32_t length = 0;
    NAPI_CALL(env, napi_get_array_length(env, arr, &length));
    NAPI_CALL(env, napi_close_handle_scope(env, scope));

    SetNamedBool(env, result, "arrayScopeSuccess", true);
    SetNamedBool(env, result, "hasElements", length > REF_COUNT_ZERO);
    return result;
}

// ============================================================================
// Test 41: napi_get_version — consistency check
// ============================================================================

static napi_value TestGetVersionConsistency(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    uint32_t versionA = 0;
    NAPI_CALL(env, napi_get_version(env, &versionA));
    uint32_t versionB = 0;
    NAPI_CALL(env, napi_get_version(env, &versionB));
    bool consistent = (versionA == versionB);

    SetNamedBool(env, result, "versionConsistent", consistent);
    SetNamedInt32(env, result, "versionA", static_cast<int32_t>(versionA));
    SetNamedInt32(env, result, "versionB", static_cast<int32_t>(versionB));
    return result;
}

// ============================================================================
// Test 42: reference to function value
// ============================================================================

static napi_value DummyFunction(napi_env env, napi_callback_info /* info */)
{
    napi_value undef = nullptr;
    napi_get_undefined(env, &undef);
    return undef;
}

static napi_value TestReferenceToFunction(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) { return nullptr; }

    napi_value func = nullptr;
    NAPI_CALL(env, napi_create_function(env, "dummyFn", NAPI_AUTO_LENGTH,
        DummyFunction, nullptr, &func));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, func, INITIAL_REF_COUNT, &ref));
    napi_value refValue = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &refValue));
    napi_valuetype valType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, refValue, &valType));
    bool isFunction = (valType == napi_function);
    NAPI_CALL(env, napi_delete_reference(env, ref));

    SetNamedBool(env, result, "refToFuncSuccess", true);
    SetNamedBool(env, result, "isStillFunction", isFunction);
    return result;
}

// ============================================================================
// Test Descriptor Table
// ============================================================================

static const napi_property_descriptor LIFECYCLE_TESTS[] = {
    { "testAsyncInitDestroy", nullptr, TestAsyncInitDestroy,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testAsyncInitWithResource", nullptr, TestAsyncInitWithResource,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testCallbackScope", nullptr, TestCallbackScope,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testCallbackScopeWithValue", nullptr, TestCallbackScopeWithValue,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testEnvCleanupHook", nullptr, TestEnvCleanupHook,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testEnvCleanupHookNullArg", nullptr, TestEnvCleanupHookNullArg,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testAsyncCleanupHook", nullptr, TestAsyncCleanupHook,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testOpenCloseHandleScope", nullptr, TestOpenCloseHandleScope,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testHandleScopeObjects", nullptr, TestHandleScopeObjects,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testNestedHandleScopes", nullptr, TestNestedHandleScopes,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testDeepNestedHandleScopes", nullptr, TestDeepNestedHandleScopes,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testStressHandleScopes", nullptr, TestStressHandleScopes,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testHandleScopeManyObjects", nullptr, TestHandleScopeManyObjects,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testOpenCloseEscapableScope", nullptr, TestOpenCloseEscapableScope,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testEscapeHandleInt", nullptr, TestEscapeHandleInt,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testEscapeHandleString", nullptr, TestEscapeHandleString,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testEscapableWithNestedScope", nullptr, TestEscapableWithNestedScope,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testEscapeHandleWithProps", nullptr, TestEscapeHandleWithProps,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testCreateDeleteReference", nullptr, TestCreateDeleteReference,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testReferenceRef", nullptr, TestReferenceRef,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testReferenceUnref", nullptr, TestReferenceUnref,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testGetReferenceValue", nullptr, TestGetReferenceValue,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testWeakReference", nullptr, TestWeakReference,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testRefUnrefRoundTrip", nullptr, TestRefUnrefRoundTrip,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testMultipleReferenceLifecycle", nullptr, TestMultipleReferenceLifecycle,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testRefMultipleTimes", nullptr, TestRefMultipleTimes,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testRefUnrefCycles", nullptr, TestRefUnrefCycles,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testWeakRefGetValue", nullptr, TestWeakRefGetValue,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testStrongToWeakRef", nullptr, TestStrongToWeakRef,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testGetVersion", nullptr, TestGetVersion,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testAdjustExternalMemory", nullptr, TestAdjustExternalMemory,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testAdjustExternalMemoryLarge", nullptr, TestAdjustExternalMemoryLarge,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testAdjustExternalMemoryNegative", nullptr, TestAdjustExternalMemoryNegative,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testAdjustExternalMemoryZero", nullptr, TestAdjustExternalMemoryZero,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testHandleScopeDoublePattern", nullptr, TestHandleScopeDoublePattern,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testReferenceToArray", nullptr, TestReferenceToArray,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testMultipleCleanupHooks", nullptr, TestMultipleCleanupHooks,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testAsyncInitMultipleNames", nullptr, TestAsyncInitMultipleNames,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testEscapeObjectWithString", nullptr, TestEscapeObjectWithString,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testHandleScopeWithArray", nullptr, TestHandleScopeWithArray,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testGetVersionConsistency", nullptr, TestGetVersionConsistency,
      nullptr, nullptr, nullptr, napi_default, nullptr },
    { "testReferenceToFunction", nullptr, TestReferenceToFunction,
      nullptr, nullptr, nullptr, napi_default, nullptr },
};

static constexpr size_t LIFECYCLE_TEST_DESCRIPTOR_COUNT =
    sizeof(LIFECYCLE_TESTS) / sizeof(LIFECYCLE_TESTS[0]);

}  // namespace

// ============================================================================
// Module Initialization and Registration
// ============================================================================

static napi_value InitLifecycleSuite(napi_env env, napi_value exports)
{
    NAPI_CALL(env, napi_define_properties(env, exports, LIFECYCLE_TEST_DESCRIPTOR_COUNT, LIFECYCLE_TESTS));
    return exports;
}

static napi_module g_lifecycleSuiteModule = {
    .nm_version = MODULE_VERSION,
    .nm_flags = NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitLifecycleSuite,
    .nm_modname = "lifecycle_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterLifecycleSuiteModule()
{
    napi_module_register(&g_lifecycleSuiteModule);
}
