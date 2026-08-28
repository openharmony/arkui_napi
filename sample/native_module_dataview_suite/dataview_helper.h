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

#ifndef FOUNDATION_ARKUI_NAPI_SAMPLE_NATIVE_MODULE_DATAVIEW_HELPER_H
#define FOUNDATION_ARKUI_NAPI_SAMPLE_NATIVE_MODULE_DATAVIEW_HELPER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include <cstdint>
#include <string>

namespace DataViewConst {
    constexpr size_t K_SMALL_BUFFER_BYTES = 16;
    constexpr size_t K_MEDIUM_BUFFER_BYTES = 64;
    constexpr size_t K_LARGE_BUFFER_BYTES = 256;
    constexpr size_t K_EXTRA_LARGE_BUFFER_BYTES = 1024;
    constexpr size_t K_ZERO_LENGTH = 0;
    constexpr size_t K_SINGLE_BYTE = 1;
    constexpr size_t K_EIGHT_BYTES = 8;
    constexpr size_t K_SIXTEEN_BYTES = 16;
    constexpr size_t K_THIRTY_TWO_BYTES = 32;
    constexpr size_t K_OFFSET_ZERO = 0;
    constexpr size_t K_OFFSET_FOUR = 4;
    constexpr size_t K_OFFSET_EIGHT = 8;
    constexpr size_t K_OFFSET_SIXTEEN = 16;
    constexpr size_t K_DATAVIEW_LENGTH_8 = 8;
    constexpr size_t K_DATAVIEW_LENGTH_16 = 16;
    constexpr size_t K_DATAVIEW_LENGTH_32 = 32;
    constexpr size_t K_DATAVIEW_LENGTH_64 = 64;
    constexpr uint8_t K_FILL_BYTE_VALUE = 0xAB;
    constexpr uint8_t K_SECOND_FILL_BYTE = 0xCD;
    constexpr uint8_t K_THIRD_FILL_BYTE = 0x42;
    constexpr uint8_t K_FOURTH_FILL_BYTE = 0x55;
    constexpr int32_t K_TEST_BYTE_VALUE = 127;
    constexpr int32_t K_BYTE_ZERO_INDEX = 0;
    constexpr int32_t K_BYTE_ONE_INDEX = 1;
    constexpr int32_t K_BYTE_FOUR_INDEX = 4;
    constexpr uint32_t K_MODULE_VERSION = 1;
    constexpr uint32_t K_NO_MODULE_FLAGS = 0;
    constexpr size_t K_MAX_TEST_CASES = 50;
    constexpr size_t K_MIN_BUFFER_SIZE = 1;
    constexpr size_t K_MAX_BUFFER_SIZE = 4096;
    constexpr size_t K_PATTERN_BYTE_0 = 0x00;
    constexpr size_t K_PATTERN_BYTE_FF = 0xFF;
    constexpr size_t K_PATTERN_BYTE_AA = 0xAA;
    constexpr size_t K_PATTERN_BYTE_55 = 0x55;
    constexpr uint8_t K_CHECKSUM_BYTE_MASK = 0xFF;
    constexpr int32_t K_TEST_INDEX_FIRST = 0;
    constexpr int32_t K_TEST_INDEX_SECOND = 1;
    constexpr int32_t K_TEST_INDEX_THIRD = 2;
    constexpr int32_t K_TEST_INDEX_FOURTH = 3;
    constexpr int32_t K_TEST_INDEX_FIFTH = 4;
    constexpr int32_t K_TEST_INDEX_SIXTH = 5;
    constexpr int32_t K_TEST_INDEX_SEVENTH = 6;
    constexpr int32_t K_TEST_INDEX_EIGHTH = 7;
    constexpr size_t K_DATAVIEW_MIN_OFFSET = 0;
    constexpr size_t K_DATAVIEW_MAX_OFFSET_FACTOR = 2;
    constexpr double K_TEST_FLOAT_VALUE = 3.14159;
    constexpr int64_t K_TEST_INT64_VALUE = 12345678901234;
    constexpr uint32_t K_TEST_UINT32_VALUE = 0xDEADBEEF;
    constexpr int32_t K_TEST_INT32_NEGATIVE = -999999;
    constexpr size_t K_BOUNDARY_SMALL = 8;
    constexpr size_t K_BOUNDARY_MEDIUM = 32;
    constexpr size_t K_BOUNDARY_LARGE = 128;
    constexpr uint8_t K_BYTE_PATTERN_ALT_1 = 0x11;
    constexpr uint8_t K_BYTE_PATTERN_ALT_2 = 0x22;
    constexpr uint8_t K_BYTE_PATTERN_ALT_3 = 0x33;
    constexpr uint8_t K_BYTE_PATTERN_ALT_4 = 0x44;
}

struct DataViewTestSpec {
    std::string testName;
    size_t bufferSize;
    size_t dataViewOffset;
    size_t dataViewLength;
    bool expectSuccess;
};

struct DataViewInfoResult {
    bool dataNotNull;
    bool lengthMatch;
    bool offsetMatch;
    bool bufferNotNull;
    size_t byteLength;
    size_t byteOffset;
};

struct DataViewWriteResult {
    bool writeSuccess;
    bool readBackMatch;
    uint8_t writtenValue;
    uint8_t readValue;
};

struct ArrayBufferSpec {
    std::string label;
    size_t byteLength;
    bool fillWithPattern;
};

struct DataViewBoundarySpec {
    size_t bufferSize;
    size_t dataViewOffset;
    size_t dataViewLength;
    std::string description;
};

struct BufferTestData {
    uint8_t fillPattern;
    size_t fillLength;
    size_t startOffset;
    bool verifyAfterFill;
};

struct DataViewTestCase {
    std::string name;
    size_t bufferBytes;
    size_t viewOffset;
    size_t viewLength;
    uint8_t writePattern;
    bool checkIdentity;
};

struct MultiViewSpec {
    size_t bufferBytes;
    size_t viewCount;
    size_t viewSize;
    bool sharedBuffer;
};

struct ByteAccessPattern {
    size_t accessIndex;
    uint8_t writeValue;
    uint8_t expectedReadValue;
    bool sequential;
};

struct BufferRangeSpec {
    size_t startOffset;
    size_t endOffset;
    uint8_t pattern;
    bool expectMatch;
};

bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value);
bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value);
bool SetNamedUint32(napi_env env, napi_value object, const char* name, uint32_t value);
bool SetNamedDouble(napi_env env, napi_value object, const char* name, double value);
bool SetNamedString(napi_env env, napi_value object, const char* name, const std::string& value);
bool SetNamedInt64(napi_env env, napi_value object, const char* name, int64_t value);

napi_value CreateResultObject(napi_env env);
napi_value CreateArrayBufferWithSize(napi_env env, size_t byteLength, void** data);
napi_value CreateDataViewWithParams(napi_env env, size_t byteLength, napi_value arrayBuffer,
    size_t byteOffset, napi_value* dataView);

void FillBufferWithPattern(void* data, size_t length, uint8_t value);
bool VerifyBufferPattern(void* data, size_t length, uint8_t expectedValue);
void FillBufferPartial(void* data, size_t length, size_t start, size_t count, uint8_t value);
bool VerifyBufferPartial(void* data, size_t length, size_t start, size_t count, uint8_t expectedValue);

napi_value BuildArrayBufferInfoResult(napi_env env, void* expectedData, size_t expectedLength,
    void* actualData, size_t actualLength);

napi_value BuildDataViewInfoResult(napi_env env, const DataViewInfoResult& result);
napi_value BuildDataViewWriteResult(napi_env env, const DataViewWriteResult& result);

napi_value BuildBooleanResult(napi_env env, bool value, const char* keyName);
napi_value BuildMultiBooleanResult(napi_env env, bool* values, const char** keys, size_t count);

size_t GetSafeArgCount(napi_env env, napi_callback_info info, size_t expected);
bool GetArgValueAsSize(napi_env env, napi_value arg, size_t* outValue);
bool GetArgValueAsInt32(napi_env env, napi_value arg, int32_t* outValue);
bool GetArgValueAsDouble(napi_env env, napi_value arg, double* outValue);

napi_value CreateArrayBufferAndFill(napi_env env, size_t byteLength, uint8_t fillValue, void** data);
napi_value CreateDataViewFromSpec(napi_env env, const DataViewTestSpec& spec, napi_value* dataView);

bool WriteByteToView(void* viewData, size_t index, uint8_t value);
bool ReadByteFromView(void* viewData, size_t index, uint8_t* value);
bool WriteMultipleBytes(void* viewData, const uint8_t* values, size_t count);
bool ReadMultipleBytes(void* viewData, uint8_t* values, size_t count);

napi_value BuildTestResultWithMessage(napi_env env, bool success, const char* message);
napi_value BuildCompareResult(napi_env env, bool match, size_t expected, size_t actual);

bool IsPowerOfTwo(size_t value);
size_t AlignToBoundary(size_t value, size_t boundary);
uint8_t ComputeChecksum(uint8_t* data, size_t length);
bool ValidateChecksum(uint8_t* data, size_t length, uint8_t expectedChecksum);

#endif  // FOUNDATION_ARKUI_NAPI_SAMPLE_NATIVE_MODULE_DATAVIEW_HELPER_H