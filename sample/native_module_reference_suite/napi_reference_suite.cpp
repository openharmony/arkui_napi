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

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

namespace {

static constexpr uint32_t REFCOUNT_ZERO = 0;
static constexpr uint32_t REFCOUNT_ONE = 1;
static constexpr uint32_t REFCOUNT_TWO = 2;
static constexpr uint32_t REFCOUNT_THREE = 3;
static constexpr uint32_t REFCOUNT_FIVE = 5;
static constexpr uint32_t REFCOUNT_TEN = 10;
static constexpr size_t STRESS_REF_COUNT = 100;
static constexpr size_t ARRAY_BUFFER_BYTE_LENGTH = 64;
static constexpr uint32_t ARRAY_ELEMENT_COUNT = 5;
static constexpr size_t TYPED_ARRAY_LENGTH = 8;
static constexpr size_t DATA_VIEW_BYTE_LENGTH = 16;
static constexpr uint32_t INCREMENT_ITERATIONS = 10;
static constexpr uint32_t DECREMENT_ITERATIONS = 5;
static constexpr uint32_t CYCLE_ITERATIONS = 10;
static constexpr size_t BYTE_OFFSET_ZERO = 0;
static constexpr int32_t MODULE_VERSION = 1;
static constexpr int32_t MODULE_FLAGS_NONE = 0;

static bool g_finalizerCalled = false;

static void ExternalFinalizeCallback([[maybe_unused]] napi_env env,
                                     [[maybe_unused]] void* data,
                                     [[maybe_unused]] void* hint)
{
    g_finalizerCalled = true;
}

static napi_value DummyCallback(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value undefined = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undefined));
    return undefined;
}

static napi_value CreateTestResult(napi_env env, bool passed, const char* message)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    napi_value passedVal = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, passed, &passedVal));
    NAPI_CALL(env, napi_set_named_property(env, result, "passed", passedVal));
    napi_value msgVal = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, message, NAPI_AUTO_LENGTH, &msgVal));
    NAPI_CALL(env, napi_set_named_property(env, result, "message", msgVal));
    return result;
}

/**
 * Test 1: Create a weak reference (initial refcount 0).
 */
static napi_value TestCreateWeakReference(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value testObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &testObj));
    napi_ref weakRef = nullptr;
    napi_status status = napi_create_reference(env, testObj, REFCOUNT_ZERO, &weakRef);
    bool passed = (status == napi_ok) && (weakRef != nullptr);
    if (weakRef != nullptr) {
        NAPI_CALL(env, napi_delete_reference(env, weakRef));
    }
    return CreateTestResult(env, passed, "weak reference created with refcount 0");
}

/**
 * Test 2: Create a strong reference (initial refcount 1).
 */
static napi_value TestCreateStrongReference(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value testObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &testObj));
    napi_ref strongRef = nullptr;
    NAPI_CALL(env, napi_create_reference(env, testObj, REFCOUNT_ONE, &strongRef));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, strongRef, &retrieved));
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, retrieved, &valueType));
    bool passed = (valueType == napi_object);
    NAPI_CALL(env, napi_delete_reference(env, strongRef));
    return CreateTestResult(env, passed, "strong reference created with refcount 1");
}

/**
 * Test 3: Create a reference with a higher initial refcount.
 */
static napi_value TestCreateRefWithHighCount(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value testObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &testObj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, testObj, REFCOUNT_FIVE, &ref));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    bool passed = (retrieved != nullptr);
    uint32_t unrefResult = 0;
    for (uint32_t i = 0; i < REFCOUNT_FIVE; ++i) {
        NAPI_CALL(env, napi_reference_unref(env, ref, &unrefResult));
    }
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "reference created with refcount 5");
}

/**
 * Test 4: Delete a reference.
 */
static napi_value TestDeleteReference(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value testObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &testObj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, testObj, REFCOUNT_ONE, &ref));
    napi_status status = napi_delete_reference(env, ref);
    bool passed = (status == napi_ok);
    return CreateTestResult(env, passed, "reference deleted successfully");
}

/**
 * Test 5: Increment refcount via napi_reference_ref and verify the new count.
 */
static napi_value TestRefIncrementCount(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value testObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &testObj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, testObj, REFCOUNT_ONE, &ref));
    uint32_t newCount = 0;
    NAPI_CALL(env, napi_reference_ref(env, ref, &newCount));
    bool passed = (newCount == REFCOUNT_TWO);
    NAPI_CALL(env, napi_reference_unref(env, ref, &newCount));
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "ref increment returned count 2");
}

/**
 * Test 6: Decrement refcount via napi_reference_unref and verify the new count.
 */
static napi_value TestUnrefDecrementCount(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value testObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &testObj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, testObj, REFCOUNT_TWO, &ref));
    uint32_t newCount = 0;
    NAPI_CALL(env, napi_reference_unref(env, ref, &newCount));
    bool passed = (newCount == REFCOUNT_ONE);
    NAPI_CALL(env, napi_reference_unref(env, ref, &newCount));
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "unref decrement returned count 1");
}

/**
 * Test 7: Retrieve value from a strong reference.
 */
static napi_value TestGetValueFromStrongRef(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value testObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &testObj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, testObj, REFCOUNT_ONE, &ref));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    bool isEqual = false;
    NAPI_CALL(env, napi_strict_equals(env, testObj, retrieved, &isEqual));
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, isEqual, "strong ref returns same object");
}

/**
 * Test 8: Retrieve value from a weak reference (refcount 0).
 */
static napi_value TestGetValueFromWeakRef(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value testObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &testObj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, testObj, REFCOUNT_ZERO, &ref));
    napi_value retrieved = nullptr;
    napi_status status = napi_get_reference_value(env, ref, &retrieved);
    bool passed = (status == napi_ok);
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "weak ref get_reference_value succeeded");
}

/**
 * Test 9: Create multiple references to the same object.
 */
static napi_value TestMultipleRefsToSameObj(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value testObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &testObj));
    napi_ref refA = nullptr;
    NAPI_CALL(env, napi_create_reference(env, testObj, REFCOUNT_ONE, &refA));
    napi_ref refB = nullptr;
    NAPI_CALL(env, napi_create_reference(env, testObj, REFCOUNT_ONE, &refB));
    napi_ref refC = nullptr;
    NAPI_CALL(env, napi_create_reference(env, testObj, REFCOUNT_ONE, &refC));
    napi_value valA = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, refA, &valA));
    napi_value valB = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, refB, &valB));
    napi_value valC = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, refC, &valC));
    bool equalAB = false;
    NAPI_CALL(env, napi_strict_equals(env, valA, valB, &equalAB));
    bool equalBC = false;
    NAPI_CALL(env, napi_strict_equals(env, valB, valC, &equalBC));
    bool passed = equalAB && equalBC;
    NAPI_CALL(env, napi_delete_reference(env, refA));
    NAPI_CALL(env, napi_delete_reference(env, refB));
    NAPI_CALL(env, napi_delete_reference(env, refC));
    return CreateTestResult(env, passed, "three refs to same object all equal");
}

/**
 * Test 10: Reference to a plain object, verify type after deref.
 */
static napi_value TestRefToPlainObject(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, obj, REFCOUNT_ONE, &ref));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, retrieved, &valueType));
    bool passed = (valueType == napi_object);
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "plain object ref type is napi_object");
}

/**
 * Test 11: Reference to an array, verify it is still an array after deref.
 */
static napi_value TestRefToArray(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value arr = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, ARRAY_ELEMENT_COUNT, &arr));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, arr, REFCOUNT_ONE, &ref));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, retrieved, &isArray));
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, isArray, "array ref still reports as array");
}

/**
 * Test 12: Reference to a function, verify type is napi_function.
 */
static napi_value TestRefToFunction(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value func = nullptr;
    NAPI_CALL(env, napi_create_function(env, "dummy", NAPI_AUTO_LENGTH, DummyCallback, nullptr, &func));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, func, REFCOUNT_ONE, &ref));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, retrieved, &valueType));
    bool passed = (valueType == napi_function);
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "function ref type is napi_function");
}

/**
 * Test 13: Reference to an external value, verify type is napi_external.
 */
static napi_value TestRefToExternal(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    int32_t externalData = 0;
    napi_value ext = nullptr;
    NAPI_CALL(env, napi_create_external(env, static_cast<void*>(&externalData), nullptr, nullptr, &ext));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, ext, REFCOUNT_ONE, &ref));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, retrieved, &valueType));
    bool passed = (valueType == napi_external);
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "external ref type is napi_external");
}

/**
 * Test 14: Reference to a promise value.
 */
static napi_value TestRefToPromise(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_deferred deferred = nullptr;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, promise, REFCOUNT_ONE, &ref));

    napi_value retrieved = nullptr;
    napi_status status = napi_get_reference_value(env, ref, &retrieved);

    bool isPromise = false;
    if (status == napi_ok) {
        status = napi_is_promise(env, retrieved, &isPromise);
    }

    napi_value resolveVal = nullptr;
    if (status == napi_ok) {
        status = napi_get_undefined(env, &resolveVal);
    }
    if (status == napi_ok) {
        status = napi_resolve_deferred(env, deferred, resolveVal);
    }

    napi_delete_reference(env, ref);
    NAPI_CALL(env, status);

    return CreateTestResult(env, isPromise, "promise ref still reports as promise");
}

/**
 * Test 15: Strong reference unref'd to become a weak reference.
 * Under N-API, unreferencing a reference to a count of 0 transitions it to a
 * weak reference. We verify this by ensuring the count reaches 0.
 */
static napi_value TestStrongToWeakTransition(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value testObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &testObj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, testObj, REFCOUNT_ONE, &ref));
    uint32_t newCount = 0;
    NAPI_CALL(env, napi_reference_unref(env, ref, &newCount));
    bool passed = (newCount == REFCOUNT_ZERO);
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "strong ref unref'd to weak (count 0)");
}

/**
 * Test 16: Weak reference ref'd back to become a strong reference.
 */
static napi_value TestWeakToStrongTransition(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value testObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &testObj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, testObj, REFCOUNT_ZERO, &ref));
    uint32_t newCount = 0;
    NAPI_CALL(env, napi_reference_ref(env, ref, &newCount));
    bool passed = (newCount == REFCOUNT_ONE);
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    passed = passed && (retrieved != nullptr);
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "weak ref promoted to strong (count 1)");
}

/**
 * Test 17: Full lifecycle: strong -> weak -> strong -> delete.
 */
static napi_value TestFullRefLifecycle(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value testObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &testObj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, testObj, REFCOUNT_ONE, &ref));
    uint32_t count = 0;
    NAPI_CALL(env, napi_reference_unref(env, ref, &count));
    bool weakOk = (count == REFCOUNT_ZERO);
    NAPI_CALL(env, napi_reference_ref(env, ref, &count));
    bool strongOk = (count == REFCOUNT_ONE);
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    bool valOk = (retrieved != nullptr);
    napi_status delStatus = napi_delete_reference(env, ref);
    bool passed = weakOk && strongOk && valOk && (delStatus == napi_ok);
    return CreateTestResult(env, passed, "full lifecycle strong->weak->strong->delete");
}

/**
 * Test 18: Create reference inside a handle scope, use it outside.
 */
static napi_value TestRefCreatedInHandleScope(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_ref ref = nullptr;
    napi_handle_scope scope = nullptr;
    NAPI_CALL(env, napi_open_handle_scope(env, &scope));
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    NAPI_CALL(env, napi_create_reference(env, obj, REFCOUNT_ONE, &ref));
    NAPI_CALL(env, napi_close_handle_scope(env, scope));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    bool passed = (retrieved != nullptr);
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "ref created in handle scope usable outside");
}

/**
 * Test 18b: Create reference inside an escapable handle scope, use it outside.
 * General escapable handle scope tests are implemented in napi_scope_suite.cpp.
 */
static napi_value TestRefCreatedInEscapableHandleScope(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_ref ref = nullptr;
    napi_escapable_handle_scope scope = nullptr;
    NAPI_CALL(env, napi_open_escapable_handle_scope(env, &scope));
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value escaped = nullptr;
    NAPI_CALL(env, napi_escape_handle(env, scope, obj, &escaped));
    NAPI_CALL(env, napi_create_reference(env, escaped, REFCOUNT_ONE, &ref));
    NAPI_CALL(env, napi_close_escapable_handle_scope(env, scope));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    bool passed = (retrieved != nullptr);
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "ref created in escapable handle scope usable outside");
}

/**
 * Test 19: External with finalizer callback and a strong reference.
 * While the strong reference exists the finalizer must not fire.
 */
static napi_value TestExternalFinalizerWithRef(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    g_finalizerCalled = false;
    int32_t data = 0;
    napi_value ext = nullptr;
    NAPI_CALL(env, napi_create_external(env, static_cast<void*>(&data), ExternalFinalizeCallback, nullptr, &ext));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, ext, REFCOUNT_ONE, &ref));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    bool passed = (retrieved != nullptr) && (!g_finalizerCalled);
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "finalizer not called while strong ref held");
}

/**
 * Test 20: Create reference on a value from napi_create_external.
 */
static napi_value TestRefOnCreateExternal(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    int32_t payload = 0;
    napi_value ext = nullptr;
    NAPI_CALL(env, napi_create_external(env, static_cast<void*>(&payload), nullptr, nullptr, &ext));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, ext, REFCOUNT_ONE, &ref));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    void* resultData = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, retrieved, &resultData));
    bool passed = (resultData == static_cast<void*>(&payload));
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "external data pointer preserved through ref");
}

/**
 * Test 21: Create reference on a value from napi_create_arraybuffer.
 */
static napi_value TestRefOnArrayBuffer(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    void* bufferData = nullptr;
    napi_value arrayBuffer = nullptr;
    NAPI_CALL(env, napi_create_arraybuffer(env, ARRAY_BUFFER_BYTE_LENGTH, &bufferData, &arrayBuffer));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, arrayBuffer, REFCOUNT_ONE, &ref));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    bool isArrayBuffer = false;
    NAPI_CALL(env, napi_is_arraybuffer(env, retrieved, &isArrayBuffer));
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, isArrayBuffer, "arraybuffer ref still is arraybuffer");
}

/**
 * Test 22: Stress test – create many references and verify all return values.
 */
static napi_value TestStressCreateAndVerify(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_ref refs[STRESS_REF_COUNT] = {};
    for (size_t i = 0; i < STRESS_REF_COUNT; ++i) {
        NAPI_CALL(env, napi_create_reference(env, obj, REFCOUNT_ONE, &refs[i]));
    }
    bool allValid = true;
    for (size_t i = 0; i < STRESS_REF_COUNT; ++i) {
        napi_value val = nullptr;
        NAPI_CALL(env, napi_get_reference_value(env, refs[i], &val));
        if (val == nullptr) {
            allValid = false;
        }
    }
    for (size_t i = 0; i < STRESS_REF_COUNT; ++i) {
        NAPI_CALL(env, napi_delete_reference(env, refs[i]));
    }
    return CreateTestResult(env, allValid, "100 refs created and all verified");
}

/**
 * Test 23: Stress test – create many references then delete all.
 */
static napi_value TestStressCreateAndDelete(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_ref refs[STRESS_REF_COUNT] = {};
    for (size_t i = 0; i < STRESS_REF_COUNT; ++i) {
        NAPI_CALL(env, napi_create_reference(env, obj, REFCOUNT_ONE, &refs[i]));
    }
    bool allDeleted = true;
    for (size_t i = 0; i < STRESS_REF_COUNT; ++i) {
        napi_status status = napi_delete_reference(env, refs[i]);
        if (status != napi_ok) {
            allDeleted = false;
        }
    }
    return CreateTestResult(env, allDeleted, "100 refs created and deleted");
}

/**
 * Test 24: Boundary – increment refcount many times and verify final count.
 */
static napi_value TestRefBoundaryIncrement(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value testObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &testObj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, testObj, REFCOUNT_ONE, &ref));
    uint32_t count = 0;
    for (uint32_t i = 0; i < INCREMENT_ITERATIONS; ++i) {
        NAPI_CALL(env, napi_reference_ref(env, ref, &count));
    }
    uint32_t expectedCount = REFCOUNT_ONE + INCREMENT_ITERATIONS;
    bool passed = (count == expectedCount);
    for (uint32_t i = 0; i < INCREMENT_ITERATIONS; ++i) {
        NAPI_CALL(env, napi_reference_unref(env, ref, &count));
    }
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "ref incremented 10 times, count correct");
}

/**
 * Test 25: Boundary – decrement refcount many times from a high initial value.
 */
static napi_value TestRefBoundaryDecrement(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value testObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &testObj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, testObj, REFCOUNT_TEN, &ref));
    uint32_t count = 0;
    for (uint32_t i = 0; i < DECREMENT_ITERATIONS; ++i) {
        NAPI_CALL(env, napi_reference_unref(env, ref, &count));
    }
    uint32_t expectedCount = REFCOUNT_TEN - DECREMENT_ITERATIONS;
    bool passed = (count == expectedCount);
    for (uint32_t i = 0; i < expectedCount; ++i) {
        NAPI_CALL(env, napi_reference_unref(env, ref, &count));
    }
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "ref decremented 5 times from 10, count 5");
}

/**
 * Test 26: Delete reference and verify no crash on subsequent operations.
 */
static napi_value TestDeleteRefCleanup(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value testObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &testObj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, testObj, REFCOUNT_ONE, &ref));
    napi_value beforeDelete = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &beforeDelete));
    bool hadValue = (beforeDelete != nullptr);
    napi_status status = napi_delete_reference(env, ref);
    bool passed = hadValue && (status == napi_ok);
    return CreateTestResult(env, passed, "reference deleted cleanly after use");
}

/**
 * Test 27: Reference to a typed array (Uint8Array).
 */
static napi_value TestRefToTypedArray(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    void* bufferData = nullptr;
    napi_value arrayBuffer = nullptr;
    size_t byteLength = TYPED_ARRAY_LENGTH * sizeof(uint8_t);
    NAPI_CALL(env, napi_create_arraybuffer(env, byteLength, &bufferData, &arrayBuffer));
    napi_value typedArray = nullptr;
    NAPI_CALL(env, napi_create_typedarray(env, napi_uint8_array, TYPED_ARRAY_LENGTH, arrayBuffer, BYTE_OFFSET_ZERO,
                                          &typedArray));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, typedArray, REFCOUNT_ONE, &ref));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    bool isTypedArray = false;
    NAPI_CALL(env, napi_is_typedarray(env, retrieved, &isTypedArray));
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, isTypedArray, "typed array ref still is typedarray");
}

/**
 * Test 28: Reference to a DataView.
 */
static napi_value TestRefToDataView(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    void* bufferData = nullptr;
    napi_value arrayBuffer = nullptr;
    NAPI_CALL(env, napi_create_arraybuffer(env, DATA_VIEW_BYTE_LENGTH, &bufferData, &arrayBuffer));
    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, DATA_VIEW_BYTE_LENGTH, arrayBuffer, BYTE_OFFSET_ZERO, &dataView));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, dataView, REFCOUNT_ONE, &ref));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    bool isDataView = false;
    NAPI_CALL(env, napi_is_dataview(env, retrieved, &isDataView));
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, isDataView, "dataview ref still is dataview");
}

/**
 * Test 29: Reference to an Error object.
 */
static napi_value TestRefToErrorObject(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value code = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "TEST_ERR", NAPI_AUTO_LENGTH, &code));
    napi_value msg = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "test error", NAPI_AUTO_LENGTH, &msg));
    napi_value errorObj = nullptr;
    NAPI_CALL(env, napi_create_error(env, code, msg, &errorObj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, errorObj, REFCOUNT_ONE, &ref));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    bool isError = false;
    NAPI_CALL(env, napi_is_error(env, retrieved, &isError));
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, isError, "error object ref still is error");
}

/**
 * Test 30: Multiple ref/unref cycles on the same reference.
 */
static napi_value TestMultipleRefUnrefCycles(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value testObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &testObj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, testObj, REFCOUNT_ONE, &ref));
    uint32_t count = 0;
    bool allCorrect = true;
    for (uint32_t i = 0; i < CYCLE_ITERATIONS; ++i) {
        NAPI_CALL(env, napi_reference_ref(env, ref, &count));
        uint32_t expectedUp = REFCOUNT_ONE + i + REFCOUNT_ONE;
        if (count != expectedUp) {
            allCorrect = false;
        }
    }
    for (uint32_t i = 0; i < CYCLE_ITERATIONS; ++i) {
        NAPI_CALL(env, napi_reference_unref(env, ref, &count));
    }
    bool finalOk = (count == REFCOUNT_ONE);
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, allCorrect && finalOk, "10 ref/unref cycles all correct");
}

/**
 * Test 31: Verify that the initial refcount matches the value passed to create.
 */
static napi_value TestRefInitialCountVerify(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value testObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &testObj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, testObj, REFCOUNT_THREE, &ref));
    uint32_t count = 0;
    NAPI_CALL(env, napi_reference_ref(env, ref, &count));
    uint32_t expectedAfterRef = REFCOUNT_THREE + REFCOUNT_ONE;
    bool passed = (count == expectedAfterRef);
    NAPI_CALL(env, napi_reference_unref(env, ref, &count));
    for (uint32_t i = 0; i < REFCOUNT_THREE; ++i) {
        NAPI_CALL(env, napi_reference_unref(env, ref, &count));
    }
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "initial refcount 3 verified via ref");
}

/**
 * Test 32: Two independent references to the same object have independent counts.
 */
static napi_value TestTwoRefsIndependentCounts(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value testObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &testObj));
    napi_ref refA = nullptr;
    NAPI_CALL(env, napi_create_reference(env, testObj, REFCOUNT_ONE, &refA));
    napi_ref refB = nullptr;
    NAPI_CALL(env, napi_create_reference(env, testObj, REFCOUNT_ONE, &refB));
    uint32_t countA = 0;
    NAPI_CALL(env, napi_reference_ref(env, refA, &countA));
    uint32_t countB = 0;
    NAPI_CALL(env, napi_reference_ref(env, refB, &countB));
    bool aIsTwo = (countA == REFCOUNT_TWO);
    bool bIsTwo = (countB == REFCOUNT_TWO);
    NAPI_CALL(env, napi_reference_unref(env, refA, &countA));
    bool aBackToOne = (countA == REFCOUNT_ONE);
    NAPI_CALL(env, napi_reference_ref(env, refB, &countB));
    bool bIsThree = (countB == REFCOUNT_THREE);
    bool passed = aIsTwo && bIsTwo && aBackToOne && bIsThree;
    NAPI_CALL(env, napi_delete_reference(env, refA));
    NAPI_CALL(env, napi_delete_reference(env, refB));
    return CreateTestResult(env, passed, "two refs have independent refcounts");
}

/**
 * Test 33: Verify strict equality between original object and dereferenced value.
 */
static napi_value TestRefValueStrictEquality(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value nameKey = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "id", NAPI_AUTO_LENGTH, &nameKey));
    napi_value nameVal = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "test_obj", NAPI_AUTO_LENGTH, &nameVal));
    NAPI_CALL(env, napi_set_property(env, obj, nameKey, nameVal));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, obj, REFCOUNT_ONE, &ref));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    bool isEqual = false;
    NAPI_CALL(env, napi_strict_equals(env, obj, retrieved, &isEqual));
    napi_value prop = nullptr;
    NAPI_CALL(env, napi_get_property(env, retrieved, nameKey, &prop));
    size_t strLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, prop, nullptr, REFCOUNT_ZERO, &strLen));
    bool passed = isEqual && (strLen > REFCOUNT_ZERO);
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "deref'd value strictly equals original");
}

/**
 * Test 34: Weak reference – retrieved value still equals original when alive.
 */
static napi_value TestWeakRefValueEquality(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, obj, REFCOUNT_ZERO, &ref));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    bool passed = false;
    if (retrieved != nullptr) {
        NAPI_CALL(env, napi_strict_equals(env, obj, retrieved, &passed));
    } else {
        passed = true;
    }
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "weak ref value equals original when alive");
}

/**
 * Test 35: Create reference with initial refcount 2 and verify both unrefs.
 */
static napi_value TestRefWithInitialCountTwo(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, obj, REFCOUNT_TWO, &ref));
    uint32_t countAfterFirst = 0;
    NAPI_CALL(env, napi_reference_unref(env, ref, &countAfterFirst));
    bool firstOk = (countAfterFirst == REFCOUNT_ONE);
    uint32_t countAfterSecond = 0;
    NAPI_CALL(env, napi_reference_unref(env, ref, &countAfterSecond));
    bool secondOk = (countAfterSecond == REFCOUNT_ZERO);
    bool passed = firstOk && secondOk;
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "initial count 2 decremented to 0 correctly");
}

/**
 * Test 36: Create reference in a nested handle scope.
 */
static napi_value TestRefInNestedHandleScope(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_ref ref = nullptr;
    napi_handle_scope outerScope = nullptr;
    NAPI_CALL(env, napi_open_handle_scope(env, &outerScope));
    napi_handle_scope innerScope = nullptr;
    NAPI_CALL(env, napi_open_handle_scope(env, &innerScope));
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    NAPI_CALL(env, napi_create_reference(env, obj, REFCOUNT_ONE, &ref));
    NAPI_CALL(env, napi_close_handle_scope(env, innerScope));
    NAPI_CALL(env, napi_close_handle_scope(env, outerScope));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    bool passed = (retrieved != nullptr);
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "ref from nested scope usable outside");
}

/**
 * Test 37: Stress test with references to distinct objects.
 */
static napi_value TestStressRefsToDistinctObjs(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_ref refs[STRESS_REF_COUNT] = {};
    for (size_t i = 0; i < STRESS_REF_COUNT; ++i) {
        napi_value obj = nullptr;
        NAPI_CALL(env, napi_create_object(env, &obj));
        NAPI_CALL(env, napi_create_reference(env, obj, REFCOUNT_ONE, &refs[i]));
    }
    bool allValid = true;
    for (size_t i = 0; i < STRESS_REF_COUNT; ++i) {
        napi_value val = nullptr;
        NAPI_CALL(env, napi_get_reference_value(env, refs[i], &val));
        if (val == nullptr) {
            allValid = false;
        }
    }
    for (size_t i = 0; i < STRESS_REF_COUNT; ++i) {
        NAPI_CALL(env, napi_delete_reference(env, refs[i]));
    }
    return CreateTestResult(env, allValid, "100 refs to distinct objects all valid");
}

/**
 * Test 38: ArrayBuffer data pointer preserved through reference dereference.
 */
static napi_value TestArrayBufferDataThroughRef(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    void* originalData = nullptr;
    napi_value arrayBuf = nullptr;
    NAPI_CALL(env, napi_create_arraybuffer(env, ARRAY_BUFFER_BYTE_LENGTH, &originalData, &arrayBuf));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, arrayBuf, REFCOUNT_ONE, &ref));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    void* retrievedData = nullptr;
    size_t retrievedLen = 0;
    NAPI_CALL(env, napi_get_arraybuffer_info(env, retrieved, &retrievedData, &retrievedLen));
    bool dataMatch = (retrievedData == originalData);
    bool lenMatch = (retrievedLen == ARRAY_BUFFER_BYTE_LENGTH);
    bool passed = dataMatch && lenMatch;
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, passed, "arraybuffer data pointer preserved");
}

/**
 * Test 39: Delete one reference among multiple to same object; others stay valid.
 */
static napi_value TestDeleteOneOfMultipleRefs(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_ref refA = nullptr;
    NAPI_CALL(env, napi_create_reference(env, obj, REFCOUNT_ONE, &refA));
    napi_ref refB = nullptr;
    NAPI_CALL(env, napi_create_reference(env, obj, REFCOUNT_ONE, &refB));
    napi_ref refC = nullptr;
    NAPI_CALL(env, napi_create_reference(env, obj, REFCOUNT_ONE, &refC));
    NAPI_CALL(env, napi_delete_reference(env, refB));
    napi_value valA = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, refA, &valA));
    napi_value valC = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, refC, &valC));
    bool aValid = (valA != nullptr);
    bool cValid = (valC != nullptr);
    bool stillEqual = false;
    NAPI_CALL(env, napi_strict_equals(env, valA, valC, &stillEqual));
    bool passed = aValid && cValid && stillEqual;
    NAPI_CALL(env, napi_delete_reference(env, refA));
    NAPI_CALL(env, napi_delete_reference(env, refC));
    return CreateTestResult(env, passed, "deleting one ref leaves others valid");
}

/**
 * Test 40: Reference to a RangeError object.
 */
static napi_value TestRefToRangeError(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value code = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "RANGE_ERR", NAPI_AUTO_LENGTH, &code));
    napi_value msg = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "out of range", NAPI_AUTO_LENGTH, &msg));
    napi_value rangeErr = nullptr;
    NAPI_CALL(env, napi_create_range_error(env, code, msg, &rangeErr));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, rangeErr, REFCOUNT_ONE, &ref));
    napi_value retrieved = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &retrieved));
    bool isError = false;
    NAPI_CALL(env, napi_is_error(env, retrieved, &isError));
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, isError, "range error ref still is error");
}

/**
 * Test 41: Alternating ref and unref verifies count at each step.
 */
static napi_value TestAlternatingRefUnref(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, obj, REFCOUNT_ONE, &ref));
    bool allCorrect = true;
    uint32_t count = 0;
    NAPI_CALL(env, napi_reference_ref(env, ref, &count));
    if (count != REFCOUNT_TWO) {
        allCorrect = false;
    }
    NAPI_CALL(env, napi_reference_unref(env, ref, &count));
    if (count != REFCOUNT_ONE) {
        allCorrect = false;
    }
    NAPI_CALL(env, napi_reference_ref(env, ref, &count));
    if (count != REFCOUNT_TWO) {
        allCorrect = false;
    }
    NAPI_CALL(env, napi_reference_ref(env, ref, &count));
    if (count != REFCOUNT_THREE) {
        allCorrect = false;
    }
    NAPI_CALL(env, napi_reference_unref(env, ref, &count));
    if (count != REFCOUNT_TWO) {
        allCorrect = false;
    }
    NAPI_CALL(env, napi_reference_unref(env, ref, &count));
    if (count != REFCOUNT_ONE) {
        allCorrect = false;
    }
    NAPI_CALL(env, napi_delete_reference(env, ref));
    return CreateTestResult(env, allCorrect, "alternating ref/unref counts correct");
}

static napi_property_descriptor REFERENCE_TESTS[] = {
    DECLARE_NAPI_FUNCTION("testCreateWeakReference", TestCreateWeakReference),
    DECLARE_NAPI_FUNCTION("testCreateStrongReference", TestCreateStrongReference),
    DECLARE_NAPI_FUNCTION("testCreateRefWithHighCount", TestCreateRefWithHighCount),
    DECLARE_NAPI_FUNCTION("testDeleteReference", TestDeleteReference),
    DECLARE_NAPI_FUNCTION("testRefIncrementCount", TestRefIncrementCount),
    DECLARE_NAPI_FUNCTION("testUnrefDecrementCount", TestUnrefDecrementCount),
    DECLARE_NAPI_FUNCTION("testGetValueFromStrongRef", TestGetValueFromStrongRef),
    DECLARE_NAPI_FUNCTION("testGetValueFromWeakRef", TestGetValueFromWeakRef),
    DECLARE_NAPI_FUNCTION("testMultipleRefsToSameObj", TestMultipleRefsToSameObj),
    DECLARE_NAPI_FUNCTION("testRefToPlainObject", TestRefToPlainObject),
    DECLARE_NAPI_FUNCTION("testRefToArray", TestRefToArray),
    DECLARE_NAPI_FUNCTION("testRefToFunction", TestRefToFunction),
    DECLARE_NAPI_FUNCTION("testRefToExternal", TestRefToExternal),
    DECLARE_NAPI_FUNCTION("testRefToPromise", TestRefToPromise),
    DECLARE_NAPI_FUNCTION("testStrongToWeakTransition", TestStrongToWeakTransition),
    DECLARE_NAPI_FUNCTION("testWeakToStrongTransition", TestWeakToStrongTransition),
    DECLARE_NAPI_FUNCTION("testFullRefLifecycle", TestFullRefLifecycle),
    DECLARE_NAPI_FUNCTION("testRefCreatedInHandleScope", TestRefCreatedInHandleScope),
    DECLARE_NAPI_FUNCTION("testRefCreatedInEscapableHandleScope", TestRefCreatedInEscapableHandleScope),
    DECLARE_NAPI_FUNCTION("testExternalFinalizerWithRef", TestExternalFinalizerWithRef),
    DECLARE_NAPI_FUNCTION("testRefOnCreateExternal", TestRefOnCreateExternal),
    DECLARE_NAPI_FUNCTION("testRefOnArrayBuffer", TestRefOnArrayBuffer),
    DECLARE_NAPI_FUNCTION("testStressCreateAndVerify", TestStressCreateAndVerify),
    DECLARE_NAPI_FUNCTION("testStressCreateAndDelete", TestStressCreateAndDelete),
    DECLARE_NAPI_FUNCTION("testRefBoundaryIncrement", TestRefBoundaryIncrement),
    DECLARE_NAPI_FUNCTION("testRefBoundaryDecrement", TestRefBoundaryDecrement),
    DECLARE_NAPI_FUNCTION("testDeleteRefCleanup", TestDeleteRefCleanup),
    DECLARE_NAPI_FUNCTION("testRefToTypedArray", TestRefToTypedArray),
    DECLARE_NAPI_FUNCTION("testRefToDataView", TestRefToDataView),
    DECLARE_NAPI_FUNCTION("testRefToErrorObject", TestRefToErrorObject),
    DECLARE_NAPI_FUNCTION("testMultipleRefUnrefCycles", TestMultipleRefUnrefCycles),
    DECLARE_NAPI_FUNCTION("testRefInitialCountVerify", TestRefInitialCountVerify),
    DECLARE_NAPI_FUNCTION("testTwoRefsIndependentCounts", TestTwoRefsIndependentCounts),
    DECLARE_NAPI_FUNCTION("testRefValueStrictEquality", TestRefValueStrictEquality),
    DECLARE_NAPI_FUNCTION("testWeakRefValueEquality", TestWeakRefValueEquality),
    DECLARE_NAPI_FUNCTION("testRefWithInitialCountTwo", TestRefWithInitialCountTwo),
    DECLARE_NAPI_FUNCTION("testRefInNestedHandleScope", TestRefInNestedHandleScope),
    DECLARE_NAPI_FUNCTION("testStressRefsToDistinctObjs", TestStressRefsToDistinctObjs),
    DECLARE_NAPI_FUNCTION("testArrayBufferDataThroughRef", TestArrayBufferDataThroughRef),
    DECLARE_NAPI_FUNCTION("testDeleteOneOfMultipleRefs", TestDeleteOneOfMultipleRefs),
    DECLARE_NAPI_FUNCTION("testRefToRangeError", TestRefToRangeError),
    DECLARE_NAPI_FUNCTION("testAlternatingRefUnref", TestAlternatingRefUnref),
};

static napi_value Init(napi_env env, napi_value exports)
{
    size_t propCount = sizeof(REFERENCE_TESTS) / sizeof(REFERENCE_TESTS[0]);
    NAPI_CALL(env, napi_define_properties(env, exports, propCount, REFERENCE_TESTS));
    return exports;
}

} // namespace

static napi_module g_module = {
    .nm_version = MODULE_VERSION,
    .nm_flags = MODULE_FLAGS_NONE,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "reference_suite",
    .nm_priv = nullptr,
    .reserved = { nullptr },
};

__attribute__((constructor)) static void RegisterReferenceSuiteModule()
{
    napi_module_register(&g_module);
}
