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

#ifndef FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_EXTERNAL_EXTERNAL_HELPER_H
#define FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_EXTERNAL_EXTERNAL_HELPER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include <cstdint>

// ============================================================
// Named constants — no magic numbers
// ============================================================

namespace ExternalConst {
    // Payload sizes for external values
    constexpr size_t PAYLOAD_SIZE_SMALL = 16;
    constexpr size_t PAYLOAD_SIZE_MEDIUM = 64;
    constexpr size_t PAYLOAD_SIZE_LARGE = 128;

    // Integer test values
    constexpr int32_t INT32_VALUE_A = 42;
    constexpr int32_t INT32_VALUE_B = -99;
    constexpr int32_t INT32_VALUE_C = 100;
    constexpr int32_t INT32_ZERO = 0;
    constexpr int32_t INT32_ONE = 1;

    // Double test values
    constexpr double DOUBLE_VALUE_A = 3.14159;
    constexpr double DOUBLE_VALUE_B = -2.71828;
    constexpr double DOUBLE_ZERO = 0.0;

    // Uint8 test values for buffer payloads
    constexpr uint8_t UINT8_VALUE_A = 10;
    constexpr uint8_t UINT8_VALUE_B = 20;
    constexpr uint8_t UINT8_VALUE_C = 30;
    constexpr uint8_t UINT8_VALUE_D = 40;
    constexpr uint8_t UINT8_VALUE_E = 50;

    // String test values
    constexpr size_t STRING_BUFFER_SIZE = 256;
    constexpr size_t STRING_SHORT_LEN = 5;
    constexpr size_t STRING_MEDIUM_LEN = 10;
    constexpr size_t STRING_LONG_LEN = 20;

    // External memory adjustment values
    constexpr int64_t MEMORY_SIZE_SMALL = 1024;
    constexpr int64_t MEMORY_SIZE_MEDIUM = 4096;
    constexpr int64_t MEMORY_SIZE_LARGE = 16384;
    constexpr int64_t MEMORY_SIZE_ZERO = 0;

    // Finalizer counters
    constexpr int32_t FINALIZER_COUNT_INIT = 0;
    constexpr int32_t FINALIZER_COUNT_ONE = 1;
    constexpr int32_t FINALIZER_COUNT_TWO = 2;

    // Array sizes for batch tests
    constexpr size_t ARRAY_SIZE_SMALL = 4;
    constexpr size_t ARRAY_SIZE_MEDIUM = 8;
    constexpr size_t ARRAY_SIZE_LARGE = 16;

    // Module registration
    constexpr uint32_t MODULE_VERSION = 1;
    constexpr uint32_t MODULE_FLAGS_NONE = 0;

    // Property names for result objects
    constexpr char PROP_STATUS[] = "status";
    constexpr char PROP_VALUE[] = "value";
    constexpr char PROP_DATA[] = "data";
    constexpr char PROP_ID[] = "id";
    constexpr char PROP_COUNT[] = "count";
    constexpr char PROP_SIZE[] = "size";
    constexpr char PROP_POINTER[] = "pointer";
    constexpr char PROP_MATCH[] = "match";
    constexpr char PROP_EQUAL[] = "equal";
    constexpr char PROP_ORIGINAL[] = "original";
    constexpr char PROP_RETRIEVED[] = "retrieved";
    constexpr char PROP_TAG[] = "tag";
    constexpr char PROP_X[] = "x";
    constexpr char PROP_Y[] = "y";
    constexpr char PROP_Z[] = "z";
    constexpr char PROP_W[] = "w";
    constexpr char PROP_ADJUSTED[] = "adjusted";
    constexpr char PROP_MEMORY_CHANGE[] = "memoryChange";
    constexpr char PROP_FINALIZED[] = "finalized";
    constexpr char PROP_STRING_DATA[] = "stringData";
    constexpr char PROP_STRING_LENGTH[] = "stringLength";
    constexpr char PROP_SUCCESS[] = "success";

    // Tag strings for payloads
    constexpr char TAG_ALPHA[] = "alpha";
    constexpr char TAG_BETA[] = "beta";
    constexpr char TAG_GAMMA[] = "gamma";
    constexpr char TAG_DELTA[] = "delta";
    constexpr char TAG_EMPTY[] = "";

    // String test constants
    constexpr char STRING_HELLO[] = "hello";
    constexpr char STRING_WORLD[] = "world";
    constexpr char STRING_ASCII[] = "ASCII";
    constexpr char STRING_UTF16_PREFIX[] = "UTF16_";

    // Test case identifiers
    constexpr int32_t TEST_ID_FIRST = 1;
    constexpr int32_t TEST_ID_SECOND = 2;
    constexpr int32_t TEST_ID_THIRD = 3;
    constexpr int32_t TEST_ID_FOURTH = 4;
    constexpr int32_t TEST_ID_FIFTH = 5;

    // Array start values
    constexpr int32_t ARRAY_START_ZERO = 0;
    constexpr int32_t ARRAY_START_TEN = 10;
    constexpr int32_t ARRAY_START_HUNDRED = 100;

    // External string test lengths
    constexpr size_t EXTERNAL_STRING_MIN_LEN = 1;
    constexpr size_t EXTERNAL_STRING_MAX_LEN = 64;

    // Callback argument counts
    constexpr size_t CALLBACK_ARG_COUNT_ZERO = 0;
    constexpr size_t CALLBACK_ARG_COUNT_ONE = 1;
    constexpr size_t CALLBACK_ARG_COUNT_TWO = 2;
    constexpr size_t CALLBACK_ARG_COUNT_THREE = 3;
}

// ============================================================
// Payload structures for external values
// ============================================================

struct SimplePayload {
    int32_t id;
    double value;
    char tag[ExternalConst::PAYLOAD_SIZE_SMALL];
};

struct ArrayPayload {
    int32_t count;
    int32_t values[ExternalConst::ARRAY_SIZE_LARGE];
};

struct StringPayload {
    char utf8Data[ExternalConst::PAYLOAD_SIZE_LARGE];
    size_t length;
};

struct NestedPayload {
    SimplePayload inner;
    int32_t extra;
};

struct VectorPayload {
    double x;
    double y;
    double z;
    double w;
};

struct MatrixPayload {
    double elements[ExternalConst::ARRAY_SIZE_MEDIUM];
    size_t rows;
    size_t cols;
};

struct BufferPayload {
    uint8_t data[ExternalConst::PAYLOAD_SIZE_LARGE];
    size_t length;
    int32_t checksum;
};

struct MetadataPayload {
    int32_t version;
    int32_t flags;
    int64_t timestamp;
    char name[ExternalConst::PAYLOAD_SIZE_SMALL];
};

struct ConfigPayload {
    bool enabled;
    int32_t priority;
    double threshold;
    char label[ExternalConst::PAYLOAD_SIZE_MEDIUM];
};

struct StatePayload {
    int32_t currentState;
    int32_t previousState;
    int64_t transitionCount;
    double lastUpdate;
};

// ============================================================
// Finalizer context for tracking cleanup callbacks
// ============================================================

struct FinalizerContext {
    int32_t* callCount;
    void* originalData;
    int32_t expectedId;
};

// ============================================================
// Helper function declarations
// ============================================================

// Initialize a simple payload with given values
void InitSimplePayload(SimplePayload* payload, int32_t id, double value, const char* tag);

// Initialize an array payload with sequential values
void InitArrayPayload(ArrayPayload* payload, int32_t start, int32_t count);

// Initialize a string payload
void InitStringPayload(StringPayload* payload, const char* data);

// Initialize a nested payload
void InitNestedPayload(NestedPayload* payload, int32_t id, double value, const char* tag, int32_t extra);

// Initialize a vector payload
void InitVectorPayload(VectorPayload* payload, double x, double y, double z, double w);

// Initialize a matrix payload
void InitMatrixPayload(MatrixPayload* payload, size_t rows, size_t cols, double fillValue);

// Initialize a buffer payload
void InitBufferPayload(BufferPayload* payload, const uint8_t* data, size_t length);

// Initialize a metadata payload
void InitMetadataPayload(MetadataPayload* payload, int32_t version, int32_t flags, const char* name);

// Initialize a config payload
void InitConfigPayload(ConfigPayload* payload, bool enabled, int32_t priority, double threshold);

// Initialize a state payload
void InitStatePayload(StatePayload* payload, int32_t current, int32_t previous);

// Finalizer callback for napi_add_finalizer
void SimpleFinalizer(napi_env env, void* data, void* hint);

// Finalizer callback with context tracking
void ContextFinalizer(napi_env env, void* data, void* hint);

// Finalizer callback for array payloads
void ArrayPayloadFinalizer(napi_env env, void* data, void* hint);

// Finalizer callback for string payloads
void StringPayloadFinalizer(napi_env env, void* data, void* hint);

// Compare two payload pointers for identity
bool ComparePayloadPointers(void* ptrA, void* ptrB);

// Create result object with status and value
napi_value CreateResultObject(napi_env env, bool status);

// Create external value from simple payload
napi_value CreateExternalFromPayload(napi_env env, SimplePayload* payload);

// Create external value with size from simple payload
napi_value CreateExternalWithSizeFromPayload(napi_env env, SimplePayload* payload, size_t size);

// Validate simple payload contents
bool ValidateSimplePayload(const SimplePayload* payload, int32_t expectedId, double expectedValue);

// Validate array payload contents
bool ValidateArrayPayload(const ArrayPayload* payload, int32_t expectedStart, int32_t expectedCount);

// Validate string payload contents
bool ValidateStringPayload(const StringPayload* payload, const char* expectedData);

// Set named property helpers
bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value);
bool SetNamedDouble(napi_env env, napi_value object, const char* name, double value);
bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value);
bool SetNamedString(napi_env env, napi_value object, const char* name, const char* value);
bool SetNamedInt64(napi_env env, napi_value object, const char* name, int64_t value);
bool SetNamedUint32(napi_env env, napi_value object, const char* name, uint32_t value);
bool SetNamedSizeT(napi_env env, napi_value object, const char* name, size_t value);

#endif  // FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_EXTERNAL_EXTERNAL_HELPER_H