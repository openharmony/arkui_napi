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

#include "external_helper.h"

#include <cstdint>
#include <cstring>
#include <vector>

namespace {

// ============================================================
// Test 1: Create external with simple payload and verify pointer
// ============================================================
static napi_value TestCreateExternalBasic(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    SimplePayload* payload = new SimplePayload();
    InitSimplePayload(payload, ExternalConst::INT32_VALUE_A, ExternalConst::DOUBLE_VALUE_A, ExternalConst::TAG_ALPHA);

    napi_value external = nullptr;
    napi_status status = napi_create_external(env, payload, SimpleFinalizer, nullptr, &external);
    if (status != napi_ok) {
        delete payload;
        return CreateResultObject(env, false);
    }
    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, true);

    void* retrieved = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, external, &retrieved));
    SetNamedBool(env, result, ExternalConst::PROP_MATCH, retrieved == payload);

    return result;
}

// ============================================================
// Test 2: Create external with size and verify creation succeeds
// ============================================================
static napi_value TestCreateExternalWithSize(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    SimplePayload* payload = new SimplePayload();
    InitSimplePayload(payload, ExternalConst::INT32_VALUE_B, ExternalConst::DOUBLE_VALUE_B, ExternalConst::TAG_BETA);

    napi_value external = nullptr;
    napi_status status = napi_create_external_with_size(
        env, payload, SimpleFinalizer, nullptr, ExternalConst::MEMORY_SIZE_SMALL, &external);
    if (status != napi_ok) {
        delete payload;
        return CreateResultObject(env, false);
    }
    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, true);

    void* retrieved = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, external, &retrieved));
    SetNamedBool(env, result, ExternalConst::PROP_MATCH, retrieved == payload);

    return result;
}

// ============================================================
// Test 3: Get value external - pointer identity round-trip
// ============================================================
static napi_value TestGetValueExternalIdentity(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    SimplePayload* payload = new SimplePayload();
    InitSimplePayload(payload, ExternalConst::INT32_VALUE_C, ExternalConst::DOUBLE_VALUE_A, ExternalConst::TAG_GAMMA);

    napi_value external = nullptr;
    NAPI_CALL(env, napi_create_external(env, payload, SimpleFinalizer, nullptr, &external));

    void* ptr1 = nullptr;
    void* ptr2 = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, external, &ptr1));
    NAPI_CALL(env, napi_get_value_external(env, external, &ptr2));

    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, true);
    SetNamedBool(env, result, ExternalConst::PROP_EQUAL, ptr1 == ptr2 && ptr1 == payload);

    return result;
}

// ============================================================
// Test 4: Multiple externals with different payloads
// ============================================================
static napi_value TestMultipleExternals(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    SimplePayload* payload1 = new SimplePayload();
    SimplePayload* payload2 = new SimplePayload();
    InitSimplePayload(payload1, ExternalConst::TEST_ID_FIRST, ExternalConst::DOUBLE_VALUE_A, ExternalConst::TAG_ALPHA);
    InitSimplePayload(payload2, ExternalConst::TEST_ID_SECOND, ExternalConst::DOUBLE_VALUE_B, ExternalConst::TAG_BETA);

    napi_value external1 = nullptr;
    napi_value external2 = nullptr;
    NAPI_CALL(env, napi_create_external(env, payload1, SimpleFinalizer, nullptr, &external1));
    NAPI_CALL(env, napi_create_external(env, payload2, SimpleFinalizer, nullptr, &external2));

    void* ptr1 = nullptr;
    void* ptr2 = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, external1, &ptr1));
    NAPI_CALL(env, napi_get_value_external(env, external2, &ptr2));

    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, true);
    SetNamedBool(env, result, "differentPointers", ptr1 != ptr2);
    SetNamedBool(env, result, "firstMatch", ptr1 == payload1);
    SetNamedBool(env, result, "secondMatch", ptr2 == payload2);

    return result;
}

// ============================================================
// Test 5: External with array payload
// ============================================================
static napi_value TestExternalArrayPayload(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    ArrayPayload* payload = new ArrayPayload();
    InitArrayPayload(payload, ExternalConst::ARRAY_START_TEN, static_cast<int32_t>(ExternalConst::ARRAY_SIZE_MEDIUM));

    napi_value external = nullptr;
    NAPI_CALL(env, napi_create_external(env, payload, ArrayPayloadFinalizer, nullptr, &external));

    void* retrieved = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, external, &retrieved));
    ArrayPayload* retrievedPayload = static_cast<ArrayPayload*>(retrieved);

    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, true);
    SetNamedBool(env, result, ExternalConst::PROP_MATCH,
        ValidateArrayPayload(retrievedPayload, ExternalConst::ARRAY_START_TEN,
            static_cast<int32_t>(ExternalConst::ARRAY_SIZE_MEDIUM)));

    return result;
}

// ============================================================
// Test 6: External with string payload
// ============================================================
static napi_value TestExternalStringPayload(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    StringPayload* payload = new StringPayload();
    InitStringPayload(payload, ExternalConst::STRING_HELLO);

    napi_value external = nullptr;
    NAPI_CALL(env, napi_create_external(env, payload, StringPayloadFinalizer, nullptr, &external));

    void* retrieved = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, external, &retrieved));
    StringPayload* retrievedPayload = static_cast<StringPayload*>(retrieved);

    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, true);
    SetNamedBool(env, result, ExternalConst::PROP_MATCH,
        ValidateStringPayload(retrievedPayload, ExternalConst::STRING_HELLO));

    return result;
}

// ============================================================
// Test 7: External with nested payload
// ============================================================
static napi_value TestExternalNestedPayload(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    NestedPayload* payload = new NestedPayload();
    InitNestedPayload(payload, ExternalConst::TEST_ID_THIRD, ExternalConst::DOUBLE_VALUE_A, ExternalConst::TAG_DELTA,
        ExternalConst::INT32_VALUE_C);

    napi_value external = nullptr;
    napi_status status = napi_create_external_with_size(
        env, payload, SimpleFinalizer, nullptr, ExternalConst::MEMORY_SIZE_MEDIUM, &external);
    if (status != napi_ok) {
        delete payload;
        return CreateResultObject(env, false);
    }

    void* retrieved = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, external, &retrieved));
    NestedPayload* retrievedPayload = static_cast<NestedPayload*>(retrieved);

    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, true);
    SetNamedBool(env, result, "innerMatch", retrievedPayload->inner.id == ExternalConst::TEST_ID_THIRD);
    SetNamedBool(env, result, "extraMatch", retrievedPayload->extra == ExternalConst::INT32_VALUE_C);

    return result;
}

// ============================================================
// Test 8: External with vector payload
// ============================================================
static napi_value TestExternalVectorPayload(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    VectorPayload* payload = new VectorPayload();
    InitVectorPayload(payload, ExternalConst::DOUBLE_VALUE_A, ExternalConst::DOUBLE_VALUE_B,
        ExternalConst::DOUBLE_VALUE_A, ExternalConst::DOUBLE_VALUE_B);

    napi_value external = nullptr;
    NAPI_CALL(env, napi_create_external(env, payload, SimpleFinalizer, nullptr, &external));

    void* retrieved = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, external, &retrieved));
    VectorPayload* retrievedPayload = static_cast<VectorPayload*>(retrieved);

    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, true);
    SetNamedBool(env, result, ExternalConst::PROP_X, retrievedPayload->x == ExternalConst::DOUBLE_VALUE_A);
    SetNamedBool(env, result, ExternalConst::PROP_Y, retrievedPayload->y == ExternalConst::DOUBLE_VALUE_B);

    return result;
}

// ============================================================
// Test 9: Add finalizer with context tracking
// ============================================================
static napi_value TestAddFinalizerWithContext(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    int32_t finalizerCallCount = ExternalConst::FINALIZER_COUNT_INIT;
    SimplePayload* payload = new SimplePayload();
    InitSimplePayload(payload, ExternalConst::TEST_ID_FOURTH, ExternalConst::DOUBLE_VALUE_B, ExternalConst::TAG_ALPHA);

    FinalizerContext* ctx = new FinalizerContext();
    ctx->callCount = &finalizerCallCount;
    ctx->originalData = payload;
    ctx->expectedId = ExternalConst::TEST_ID_FOURTH;

    napi_value external = nullptr;
    NAPI_CALL(env, napi_create_external(env, payload, nullptr, nullptr, &external));
    NAPI_CALL(env, napi_add_finalizer(env, external, ctx, ContextFinalizer, nullptr));

    void* retrieved = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, external, &retrieved));
    SimplePayload* retrievedPayload = static_cast<SimplePayload*>(retrieved);

    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, true);
    SetNamedBool(env, result, ExternalConst::PROP_MATCH, retrievedPayload->id == ExternalConst::TEST_ID_FOURTH);
    SetNamedInt32(env, result, ExternalConst::PROP_COUNT, finalizerCallCount);

    return result;
}

// ============================================================
// Test 10: Add finalizer to object with data
// ============================================================
static napi_value TestAddFinalizerToObject(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    int32_t finalizerCallCount = ExternalConst::FINALIZER_COUNT_INIT;
    SimplePayload* payload = new SimplePayload();
    InitSimplePayload(payload, ExternalConst::TEST_ID_FIFTH, ExternalConst::DOUBLE_VALUE_A, ExternalConst::TAG_BETA);

    napi_value object = nullptr;
    NAPI_CALL(env, napi_create_object(env, &object));
    napi_value propValue = nullptr;
    NAPI_CALL(env, napi_create_int32(env, ExternalConst::INT32_VALUE_A, &propValue));
    NAPI_CALL(env, napi_set_named_property(env, object, ExternalConst::PROP_ID, propValue));

    FinalizerContext* ctx = new FinalizerContext();
    ctx->callCount = &finalizerCallCount;
    ctx->originalData = payload;
    ctx->expectedId = ExternalConst::TEST_ID_FIFTH;

    NAPI_CALL(env, napi_add_finalizer(env, object, ctx, ContextFinalizer, nullptr));

    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, true);
    SetNamedInt32(env, result, "finalizerAttached", ExternalConst::INT32_ONE);

    return result;
}

// ============================================================
// Test 11: Adjust external memory - increase
// ============================================================
static napi_value TestAdjustExternalMemoryIncrease(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    int64_t adjusted = ExternalConst::MEMORY_SIZE_ZERO;
    napi_status status = napi_adjust_external_memory(env, ExternalConst::MEMORY_SIZE_SMALL, &adjusted);

    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, status == napi_ok);
    SetNamedInt64(env, result, ExternalConst::PROP_ADJUSTED, adjusted);

    int64_t restore = ExternalConst::MEMORY_SIZE_ZERO;
    napi_adjust_external_memory(env, -ExternalConst::MEMORY_SIZE_SMALL, &restore);

    return result;
}

// ============================================================
// Test 12: Adjust external memory - decrease
// ============================================================
static napi_value TestAdjustExternalMemoryDecrease(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    int64_t first = ExternalConst::MEMORY_SIZE_ZERO;
    int64_t second = ExternalConst::MEMORY_SIZE_ZERO;
    NAPI_CALL(env, napi_adjust_external_memory(env, ExternalConst::MEMORY_SIZE_MEDIUM, &first));
    NAPI_CALL(env, napi_adjust_external_memory(env, -ExternalConst::MEMORY_SIZE_SMALL, &second));

    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, true);
    SetNamedInt64(env, result, "firstAdjust", first);
    SetNamedInt64(env, result, "secondAdjust", second);

    int64_t restore = ExternalConst::MEMORY_SIZE_ZERO;
    napi_adjust_external_memory(env, -(ExternalConst::MEMORY_SIZE_MEDIUM - ExternalConst::MEMORY_SIZE_SMALL), &restore);

    return result;
}

// ============================================================
// Test 13: Create external string UTF-16
// ============================================================
static napi_value TestCreateExternalStringUtf16(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    static char16_t utf16Data[] = u"hello";
    static size_t utf16Length = ExternalConst::STRING_SHORT_LEN;

    napi_value externalString = nullptr;
    napi_status status = napi_create_external_string_utf16(
        env, utf16Data, utf16Length, nullptr, nullptr, &externalString);

    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, status == napi_ok);

    if (status == napi_ok) {
        char buffer[ExternalConst::STRING_BUFFER_SIZE] = {0};
        size_t copied = ExternalConst::INT32_ZERO;
        napi_status copyStatus = napi_get_value_string_utf8(env, externalString, buffer,
            ExternalConst::STRING_BUFFER_SIZE, &copied);
        SetNamedBool(env, result, "copied", copyStatus == napi_ok && copied > ExternalConst::INT32_ZERO);
        SetNamedString(env, result, ExternalConst::PROP_STRING_DATA, buffer);
    }

    return result;
}

// ============================================================
// Test 14: Create external string ASCII
// ============================================================
static napi_value TestCreateExternalStringAscii(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    static char asciiData[] = "world";
    static size_t asciiLength = ExternalConst::STRING_SHORT_LEN;

    napi_value externalString = nullptr;
    napi_status status = napi_create_external_string_ascii(
        env, asciiData, asciiLength, nullptr, nullptr, &externalString);

    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, status == napi_ok);

    if (status == napi_ok) {
        char buffer[ExternalConst::STRING_BUFFER_SIZE] = {0};
        size_t copied = ExternalConst::INT32_ZERO;
        napi_status copyStatus = napi_get_value_string_utf8(env, externalString, buffer,
            ExternalConst::STRING_BUFFER_SIZE, &copied);
        SetNamedBool(env, result, "copied", copyStatus == napi_ok && copied > ExternalConst::INT32_ZERO);
        SetNamedString(env, result, ExternalConst::PROP_STRING_DATA, buffer);
    }

    return result;
}

// ============================================================
// Test 15: External string UTF-16 with longer content
// ============================================================
static napi_value TestExternalStringUtf16Longer(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    static char16_t utf16Data[] = u"external_test";
    static size_t utf16Length = ExternalConst::STRING_LONG_LEN;

    napi_value externalString = nullptr;
    napi_status status = napi_create_external_string_utf16(
        env, utf16Data, utf16Length, nullptr, nullptr, &externalString);

    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, status == napi_ok);
    SetNamedSizeT(env, result, ExternalConst::PROP_STRING_LENGTH, utf16Length);

    return result;
}

// ============================================================
// Test 16: External string ASCII with longer content
// ============================================================
static napi_value TestExternalStringAsciiLonger(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    static char asciiData[] = "longer_ascii_string_test";
    static size_t asciiLength = ExternalConst::STRING_LONG_LEN + ExternalConst::STRING_SHORT_LEN;

    napi_value externalString = nullptr;
    napi_status status = napi_create_external_string_ascii(
        env, asciiData, asciiLength, nullptr, nullptr, &externalString);

    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, status == napi_ok);
    SetNamedSizeT(env, result, ExternalConst::PROP_STRING_LENGTH, asciiLength);

    return result;
}

// ============================================================
// Test 17: Multiple finalizers on same external
// ============================================================
static napi_value TestMultipleFinalizers(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    int32_t counter1 = ExternalConst::FINALIZER_COUNT_INIT;
    int32_t counter2 = ExternalConst::FINALIZER_COUNT_INIT;

    SimplePayload* payload = new SimplePayload();
    InitSimplePayload(payload, ExternalConst::TEST_ID_FIRST, ExternalConst::DOUBLE_VALUE_A, ExternalConst::TAG_GAMMA);

    napi_value external = nullptr;
    NAPI_CALL(env, napi_create_external(env, payload, SimpleFinalizer, nullptr, &external));

    FinalizerContext* ctx1 = new FinalizerContext();
    ctx1->callCount = &counter1;
    ctx1->originalData = nullptr;
    ctx1->expectedId = ExternalConst::INT32_ZERO;

    FinalizerContext* ctx2 = new FinalizerContext();
    ctx2->callCount = &counter2;
    ctx2->originalData = nullptr;
    ctx2->expectedId = ExternalConst::INT32_ZERO;

    NAPI_CALL(env, napi_add_finalizer(env, external, ctx1, ContextFinalizer, nullptr));
    NAPI_CALL(env, napi_add_finalizer(env, external, ctx2, ContextFinalizer, nullptr));

    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, true);
    SetNamedInt32(env, result, "counter1Initial", counter1);
    SetNamedInt32(env, result, "counter2Initial", counter2);

    return result;
}

// ============================================================
// Test 18: External with large memory adjustment
// ============================================================
static napi_value TestLargeMemoryAdjustment(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    int64_t adjusted1 = ExternalConst::MEMORY_SIZE_ZERO;
    int64_t adjusted2 = ExternalConst::MEMORY_SIZE_ZERO;

    NAPI_CALL(env, napi_adjust_external_memory(env, ExternalConst::MEMORY_SIZE_LARGE, &adjusted1));
    NAPI_CALL(env, napi_adjust_external_memory(env, ExternalConst::MEMORY_SIZE_MEDIUM, &adjusted2));

    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, true);
    SetNamedInt64(env, result, "largeAdjust", adjusted1);
    SetNamedInt64(env, result, "mediumAdjust", adjusted2);

    int64_t restore = ExternalConst::MEMORY_SIZE_ZERO;
    napi_adjust_external_memory(env, -(ExternalConst::MEMORY_SIZE_LARGE + ExternalConst::MEMORY_SIZE_MEDIUM), &restore);

    return result;
}

// ============================================================
// Test 19: Create external with null finalizer
// ============================================================
static napi_value TestCreateExternalNullFinalizer(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    SimplePayload* payload = new SimplePayload();
    InitSimplePayload(payload, ExternalConst::TEST_ID_SECOND, ExternalConst::DOUBLE_VALUE_B, ExternalConst::TAG_DELTA);

    napi_value external = nullptr;
    napi_status status = napi_create_external(env, payload, nullptr, nullptr, &external);

    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, status == napi_ok);

    if (status == napi_ok) {
        void* retrieved = nullptr;
        NAPI_CALL(env, napi_get_value_external(env, external, &retrieved));
        SetNamedBool(env, result, ExternalConst::PROP_MATCH, retrieved == payload);
        delete payload;
    } else {
        delete payload;
    }

    return result;
}

// ============================================================
// Test 20: External pointer strict equality check
// ============================================================
static napi_value TestExternalPointerEquality(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    SimplePayload* payload = new SimplePayload();
    InitSimplePayload(payload, ExternalConst::TEST_ID_THIRD, ExternalConst::DOUBLE_VALUE_A, ExternalConst::TAG_ALPHA);

    napi_value external = nullptr;
    NAPI_CALL(env, napi_create_external(env, payload, SimpleFinalizer, nullptr, &external));

    void* ptrA = nullptr;
    void* ptrB = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, external, &ptrA));
    NAPI_CALL(env, napi_get_value_external(env, external, &ptrB));

    bool pointersEqual = ComparePayloadPointers(ptrA, ptrB);
    bool payloadMatch = ComparePayloadPointers(ptrA, payload);

    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, true);
    SetNamedBool(env, result, "pointersEqual", pointersEqual);
    SetNamedBool(env, result, "payloadMatch", payloadMatch);

    return result;
}

// ============================================================
// Test 21: External with matrix payload
// ============================================================
static napi_value TestExternalMatrixPayload(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    MatrixPayload* payload = new MatrixPayload();
    InitMatrixPayload(payload, ExternalConst::ARRAY_SIZE_SMALL,
        ExternalConst::ARRAY_SIZE_SMALL, ExternalConst::DOUBLE_VALUE_A);

    napi_value external = nullptr;
    napi_status status = napi_create_external_with_size(
        env, payload, SimpleFinalizer, nullptr, ExternalConst::MEMORY_SIZE_LARGE, &external);
    if (status != napi_ok) {
        delete payload;
        return CreateResultObject(env, false);
    }

    void* retrieved = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, external, &retrieved));
    MatrixPayload* retrievedPayload = static_cast<MatrixPayload*>(retrieved);

    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, true);
    SetNamedSizeT(env, result, "matrixRows", retrievedPayload->rows);
    SetNamedSizeT(env, result, "matrixCols", retrievedPayload->cols);

    return result;
}

// ============================================================
// Test 22: External with buffer payload
// ============================================================
static napi_value TestExternalBufferPayload(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    uint8_t data[] = {
        ExternalConst::UINT8_VALUE_A,
        ExternalConst::UINT8_VALUE_B,
        ExternalConst::UINT8_VALUE_C,
        ExternalConst::UINT8_VALUE_D,
        ExternalConst::UINT8_VALUE_E
    };
    BufferPayload* payload = new BufferPayload();
    InitBufferPayload(payload, data, ExternalConst::ARRAY_SIZE_SMALL);

    napi_value external = nullptr;
    NAPI_CALL(env, napi_create_external(env, payload, SimpleFinalizer, nullptr, &external));

    void* retrieved = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, external, &retrieved));
    BufferPayload* retrievedPayload = static_cast<BufferPayload*>(retrieved);

    SetNamedBool(env, result, ExternalConst::PROP_SUCCESS, true);
    SetNamedSizeT(env, result, ExternalConst::PROP_SIZE, retrievedPayload->length);
    SetNamedInt32(env, result, "checksum", retrievedPayload->checksum);

    return result;
}

// ============================================================
// Module registration
// ============================================================
constexpr size_t EXTERNAL_TEST_COUNT = 22;

struct ExternalTestEntry {
    const char* name;
    napi_callback callback;
};

const ExternalTestEntry EXTERNAL_TESTS[] = {
    {"testCreateExternalBasic", TestCreateExternalBasic},
    {"testCreateExternalWithSize", TestCreateExternalWithSize},
    {"testGetValueExternalIdentity", TestGetValueExternalIdentity},
    {"testMultipleExternals", TestMultipleExternals},
    {"testExternalArrayPayload", TestExternalArrayPayload},
    {"testExternalStringPayload", TestExternalStringPayload},
    {"testExternalNestedPayload", TestExternalNestedPayload},
    {"testExternalVectorPayload", TestExternalVectorPayload},
    {"testAddFinalizerWithContext", TestAddFinalizerWithContext},
    {"testAddFinalizerToObject", TestAddFinalizerToObject},
    {"testAdjustExternalMemoryIncrease", TestAdjustExternalMemoryIncrease},
    {"testAdjustExternalMemoryDecrease", TestAdjustExternalMemoryDecrease},
    {"testCreateExternalStringUtf16", TestCreateExternalStringUtf16},
    {"testCreateExternalStringAscii", TestCreateExternalStringAscii},
    {"testExternalStringUtf16Longer", TestExternalStringUtf16Longer},
    {"testExternalStringAsciiLonger", TestExternalStringAsciiLonger},
    {"testMultipleFinalizers", TestMultipleFinalizers},
    {"testLargeMemoryAdjustment", TestLargeMemoryAdjustment},
    {"testCreateExternalNullFinalizer", TestCreateExternalNullFinalizer},
    {"testExternalPointerEquality", TestExternalPointerEquality},
    {"testExternalMatrixPayload", TestExternalMatrixPayload},
    {"testExternalBufferPayload", TestExternalBufferPayload},
};

}  // namespace

static napi_value InitExternalSuite(napi_env env, napi_value exports)
{
    std::vector<napi_property_descriptor> descriptors(EXTERNAL_TEST_COUNT);
    for (size_t i = ExternalConst::INT32_ZERO; i < EXTERNAL_TEST_COUNT; i++) {
        descriptors[i] = napi_property_descriptor{
            EXTERNAL_TESTS[i].name,
            nullptr,
            EXTERNAL_TESTS[i].callback,
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

static napi_module g_externalSuiteModule = {
    .nm_version = ExternalConst::MODULE_VERSION,
    .nm_flags = ExternalConst::MODULE_FLAGS_NONE,
    .nm_filename = nullptr,
    .nm_register_func = InitExternalSuite,
    .nm_modname = "external_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterExternalSuiteModule(void)
{
    napi_module_register(&g_externalSuiteModule);
}