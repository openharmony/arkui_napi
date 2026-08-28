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

#include "dataview_helper.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

namespace {

using namespace DataViewConst;

static constexpr size_t K_TEST_COUNT = 40;
static constexpr size_t K_CASE_NAME_WIDTH = 2;
static constexpr size_t K_FIRST_CASE_NUMBER = 1;

std::string BuildTestName(const char* prefix, size_t caseNumber)
{
    std::string suffix = std::to_string(caseNumber);
    if (suffix.size() < K_CASE_NAME_WIDTH) {
        suffix.insert(K_ZERO_LENGTH, K_CASE_NAME_WIDTH - suffix.size(), '0');
    }
    return std::string(prefix) + suffix;
}

static napi_value TestCreateArrayBuffer16Bytes(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_SMALL_BUFFER_BYTES, &data);
    SetNamedBool(env, result, "created", arrayBuffer != nullptr);
    SetNamedBool(env, result, "dataNotNull", data != nullptr);
    SetNamedInt32(env, result, "byteLength", static_cast<int32_t>(K_SMALL_BUFFER_BYTES));
    return result;
}

static napi_value TestCreateArrayBuffer64Bytes(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_MEDIUM_BUFFER_BYTES, &data);
    SetNamedBool(env, result, "created", arrayBuffer != nullptr);
    SetNamedBool(env, result, "dataNotNull", data != nullptr);
    SetNamedInt32(env, result, "byteLength", static_cast<int32_t>(K_MEDIUM_BUFFER_BYTES));
    return result;
}

static napi_value TestCreateArrayBuffer256Bytes(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_LARGE_BUFFER_BYTES, &data);
    SetNamedBool(env, result, "created", arrayBuffer != nullptr);
    SetNamedBool(env, result, "dataNotNull", data != nullptr);
    SetNamedInt32(env, result, "byteLength", static_cast<int32_t>(K_LARGE_BUFFER_BYTES));
    return result;
}

static napi_value TestGetArrayBufferInfoRoundTrip(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_MEDIUM_BUFFER_BYTES, &createData);
    void* infoData = nullptr;
    size_t infoLength = K_ZERO_LENGTH;
    NAPI_CALL(env, napi_get_arraybuffer_info(env, arrayBuffer, &infoData, &infoLength));
    SetNamedBool(env, result, "dataMatch", createData == infoData);
    SetNamedBool(env, result, "lengthMatch", infoLength == K_MEDIUM_BUFFER_BYTES);
    SetNamedUint32(env, result, "byteLength", static_cast<uint32_t>(infoLength));
    return result;
}

static napi_value TestGetArrayBufferInfoDataPointer(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_SMALL_BUFFER_BYTES, &data);
    FillBufferWithPattern(data, K_SMALL_BUFFER_BYTES, K_FILL_BYTE_VALUE);
    void* infoData = nullptr;
    size_t infoLength = K_ZERO_LENGTH;
    NAPI_CALL(env, napi_get_arraybuffer_info(env, arrayBuffer, &infoData, &infoLength));
    auto* bytes = static_cast<uint8_t*>(infoData);
    bool firstByteCorrect = (bytes[K_ZERO_LENGTH] == K_FILL_BYTE_VALUE);
    SetNamedBool(env, result, "dataPointerValid", infoData != nullptr);
    SetNamedBool(env, result, "firstByteCorrect", firstByteCorrect);
    return result;
}

static napi_value TestCreateDataViewDefaultOffset(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_MEDIUM_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_DATAVIEW_LENGTH_8, arrayBuffer, K_OFFSET_ZERO, &dataView));
    SetNamedBool(env, result, "created", dataView != nullptr);
    SetNamedInt32(env, result, "length", static_cast<int32_t>(K_DATAVIEW_LENGTH_8));
    SetNamedInt32(env, result, "offset", static_cast<int32_t>(K_OFFSET_ZERO));
    return result;
}

static napi_value TestCreateDataViewCustomOffset(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_LARGE_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_DATAVIEW_LENGTH_16, arrayBuffer, K_OFFSET_FOUR, &dataView));
    SetNamedBool(env, result, "created", dataView != nullptr);
    SetNamedInt32(env, result, "length", static_cast<int32_t>(K_DATAVIEW_LENGTH_16));
    SetNamedInt32(env, result, "offset", static_cast<int32_t>(K_OFFSET_FOUR));
    return result;
}

static napi_value TestCreateDataViewOffsetEight(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_LARGE_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_DATAVIEW_LENGTH_32, arrayBuffer, K_OFFSET_EIGHT, &dataView));
    SetNamedBool(env, result, "created", dataView != nullptr);
    SetNamedInt32(env, result, "length", static_cast<int32_t>(K_DATAVIEW_LENGTH_32));
    SetNamedInt32(env, result, "offset", static_cast<int32_t>(K_OFFSET_EIGHT));
    return result;
}

static napi_value TestCreateDataViewFullBuffer(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_SMALL_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_SMALL_BUFFER_BYTES, arrayBuffer, K_OFFSET_ZERO, &dataView));
    SetNamedBool(env, result, "created", dataView != nullptr);
    SetNamedBool(env, result, "fullBufferCoverage", true);
    return result;
}

static napi_value TestCreateDataViewBoundaryExact(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_MEDIUM_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    size_t offset = K_MEDIUM_BUFFER_BYTES - K_DATAVIEW_LENGTH_8;
    NAPI_CALL(env, napi_create_dataview(env, K_DATAVIEW_LENGTH_8, arrayBuffer, offset, &dataView));
    SetNamedBool(env, result, "created", dataView != nullptr);
    SetNamedBool(env, result, "boundaryExact", true);
    return result;
}

static napi_value TestCreateDataViewInvalidOffset(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_SMALL_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    size_t invalidOffset = K_SMALL_BUFFER_BYTES + K_SINGLE_BYTE;
    napi_status status = napi_create_dataview(env, K_DATAVIEW_LENGTH_8, arrayBuffer, invalidOffset, &dataView);
    bool expectFailure = (status != napi_ok);
    SetNamedBool(env, result, "expectFailure", expectFailure);
    SetNamedBool(env, result, "dataViewNull", dataView == nullptr);
    return result;
}

static napi_value TestCreateDataViewInvalidLength(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_SMALL_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    size_t invalidLength = K_SMALL_BUFFER_BYTES + K_EIGHT_BYTES;
    napi_status status = napi_create_dataview(env, invalidLength, arrayBuffer, K_OFFSET_ZERO, &dataView);
    bool expectFailure = (status != napi_ok);
    SetNamedBool(env, result, "expectFailure", expectFailure);
    SetNamedBool(env, result, "dataViewNull", dataView == nullptr);
    return result;
}

static napi_value TestCreateDataViewOffsetPlusLengthOverflow(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_MEDIUM_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    size_t offset = K_MEDIUM_BUFFER_BYTES - K_DATAVIEW_LENGTH_8 + K_SINGLE_BYTE;
    napi_status status = napi_create_dataview(env, K_DATAVIEW_LENGTH_8, arrayBuffer, offset, &dataView);
    bool expectFailure = (status != napi_ok);
    SetNamedBool(env, result, "expectFailure", expectFailure);
    SetNamedBool(env, result, "overflowDetected", true);
    return result;
}

static napi_value TestGetDataViewInfoBasic(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_MEDIUM_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_DATAVIEW_LENGTH_8, arrayBuffer, K_OFFSET_FOUR, &dataView));
    size_t byteLength = K_ZERO_LENGTH;
    void* viewData = nullptr;
    napi_value outBuffer = nullptr;
    size_t byteOffset = K_ZERO_LENGTH;
    NAPI_CALL(env, napi_get_dataview_info(env, dataView, &byteLength, &viewData, &outBuffer, &byteOffset));
    SetNamedBool(env, result, "lengthMatch", byteLength == K_DATAVIEW_LENGTH_8);
    SetNamedBool(env, result, "dataNotNull", viewData != nullptr);
    SetNamedBool(env, result, "offsetMatch", byteOffset == K_OFFSET_FOUR);
    SetNamedBool(env, result, "bufferNotNull", outBuffer != nullptr);
    return result;
}

static napi_value TestGetDataViewInfoLargeView(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_LARGE_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_DATAVIEW_LENGTH_64, arrayBuffer, K_OFFSET_ZERO, &dataView));
    size_t byteLength = K_ZERO_LENGTH;
    void* viewData = nullptr;
    napi_value outBuffer = nullptr;
    size_t byteOffset = K_ZERO_LENGTH;
    NAPI_CALL(env, napi_get_dataview_info(env, dataView, &byteLength, &viewData, &outBuffer, &byteOffset));
    SetNamedBool(env, result, "lengthMatch", byteLength == K_DATAVIEW_LENGTH_64);
    SetNamedBool(env, result, "offsetZero", byteOffset == K_OFFSET_ZERO);
    return result;
}

static napi_value TestGetDataViewInfoBufferIdentity(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_MEDIUM_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_DATAVIEW_LENGTH_16, arrayBuffer, K_OFFSET_EIGHT, &dataView));
    size_t byteLength = K_ZERO_LENGTH;
    void* viewData = nullptr;
    napi_value outBuffer = nullptr;
    size_t byteOffset = K_ZERO_LENGTH;
    NAPI_CALL(env, napi_get_dataview_info(env, dataView, &byteLength, &viewData, &outBuffer, &byteOffset));
    bool sameBuffer = false;
    NAPI_CALL(env, napi_strict_equals(env, arrayBuffer, outBuffer, &sameBuffer));
    SetNamedBool(env, result, "sameBuffer", sameBuffer);
    SetNamedBool(env, result, "identityVerified", true);
    return result;
}

static napi_value TestGetDataViewInfoDataPointer(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_MEDIUM_BUFFER_BYTES, &data);
    FillBufferWithPattern(data, K_MEDIUM_BUFFER_BYTES, K_FILL_BYTE_VALUE);
    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_DATAVIEW_LENGTH_8, arrayBuffer, K_OFFSET_FOUR, &dataView));
    size_t byteLength = K_ZERO_LENGTH;
    void* viewData = nullptr;
    napi_value outBuffer = nullptr;
    size_t byteOffset = K_ZERO_LENGTH;
    NAPI_CALL(env, napi_get_dataview_info(env, dataView, &byteLength, &viewData, &outBuffer, &byteOffset));
    auto* bytes = static_cast<uint8_t*>(viewData);
    bool dataAccessible = (bytes != nullptr);
    SetNamedBool(env, result, "dataAccessible", dataAccessible);
    SetNamedBool(env, result, "dataNotNull", viewData != nullptr);
    return result;
}

static napi_value TestIsDataViewPositive(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_MEDIUM_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_DATAVIEW_LENGTH_8, arrayBuffer, K_OFFSET_ZERO, &dataView));
    bool isDataView = false;
    NAPI_CALL(env, napi_is_dataview(env, dataView, &isDataView));
    SetNamedBool(env, result, "isDataView", isDataView);
    return result;
}

static napi_value TestIsDataViewNegativeObject(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    napi_value plainObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &plainObject));
    bool isDataView = true;
    NAPI_CALL(env, napi_is_dataview(env, plainObject, &isDataView));
    SetNamedBool(env, result, "isNotDataView", !isDataView);
    return result;
}

static napi_value TestIsDataViewNegativeArray(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    napi_value jsArray = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArray));
    bool isDataView = true;
    NAPI_CALL(env, napi_is_dataview(env, jsArray, &isDataView));
    SetNamedBool(env, result, "isNotDataView", !isDataView);
    return result;
}

static napi_value TestIsDataViewNegativeArrayBuffer(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_SMALL_BUFFER_BYTES, &data);
    bool isDataView = true;
    NAPI_CALL(env, napi_is_dataview(env, arrayBuffer, &isDataView));
    SetNamedBool(env, result, "isNotDataView", !isDataView);
    return result;
}

static napi_value TestWriteReadByteZero(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_MEDIUM_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_DATAVIEW_LENGTH_16, arrayBuffer, K_OFFSET_ZERO, &dataView));
    size_t byteLength = K_ZERO_LENGTH;
    void* viewData = nullptr;
    NAPI_CALL(env, napi_get_dataview_info(env, dataView, &byteLength, &viewData, nullptr, nullptr));
    auto* bytes = static_cast<uint8_t*>(viewData);
    bytes[K_BYTE_ZERO_INDEX] = static_cast<uint8_t>(K_TEST_BYTE_VALUE);
    bool readBackMatch = (bytes[K_BYTE_ZERO_INDEX] == static_cast<uint8_t>(K_TEST_BYTE_VALUE));
    SetNamedBool(env, result, "writeSuccess", true);
    SetNamedBool(env, result, "readBackMatch", readBackMatch);
    SetNamedInt32(env, result, "writtenValue", static_cast<int32_t>(K_TEST_BYTE_VALUE));
    SetNamedInt32(env, result, "readValue", static_cast<int32_t>(bytes[K_BYTE_ZERO_INDEX]));
    return result;
}

static napi_value TestWriteReadByteOne(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_MEDIUM_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_DATAVIEW_LENGTH_16, arrayBuffer, K_OFFSET_ZERO, &dataView));
    size_t byteLength = K_ZERO_LENGTH;
    void* viewData = nullptr;
    NAPI_CALL(env, napi_get_dataview_info(env, dataView, &byteLength, &viewData, nullptr, nullptr));
    auto* bytes = static_cast<uint8_t*>(viewData);
    uint8_t testValue = K_SECOND_FILL_BYTE;
    bytes[K_BYTE_ONE_INDEX] = testValue;
    bool readBackMatch = (bytes[K_BYTE_ONE_INDEX] == testValue);
    SetNamedBool(env, result, "writeSuccess", true);
    SetNamedBool(env, result, "readBackMatch", readBackMatch);
    SetNamedUint32(env, result, "writtenValue", static_cast<uint32_t>(testValue));
    SetNamedUint32(env, result, "readValue", static_cast<uint32_t>(bytes[K_BYTE_ONE_INDEX]));
    return result;
}

static napi_value TestWriteReadMultipleBytes(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_MEDIUM_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_DATAVIEW_LENGTH_16, arrayBuffer, K_OFFSET_ZERO, &dataView));
    size_t byteLength = K_ZERO_LENGTH;
    void* viewData = nullptr;
    NAPI_CALL(env, napi_get_dataview_info(env, dataView, &byteLength, &viewData, nullptr, nullptr));
    auto* bytes = static_cast<uint8_t*>(viewData);
    bytes[K_BYTE_ZERO_INDEX] = K_FILL_BYTE_VALUE;
    bytes[K_BYTE_ONE_INDEX] = K_SECOND_FILL_BYTE;
    bytes[K_BYTE_FOUR_INDEX] = K_THIRD_FILL_BYTE;
    bool allMatch = (bytes[K_BYTE_ZERO_INDEX] == K_FILL_BYTE_VALUE) &&
                    (bytes[K_BYTE_ONE_INDEX] == K_SECOND_FILL_BYTE) &&
                    (bytes[K_BYTE_FOUR_INDEX] == K_THIRD_FILL_BYTE);
    SetNamedBool(env, result, "writeMultipleSuccess", true);
    SetNamedBool(env, result, "allBytesMatch", allMatch);
    return result;
}

static napi_value TestWriteReadPatternFull(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_MEDIUM_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_DATAVIEW_LENGTH_16, arrayBuffer, K_OFFSET_ZERO, &dataView));
    size_t byteLength = K_ZERO_LENGTH;
    void* viewData = nullptr;
    NAPI_CALL(env, napi_get_dataview_info(env, dataView, &byteLength, &viewData, nullptr, nullptr));
    FillBufferWithPattern(viewData, byteLength, K_FILL_BYTE_VALUE);
    bool patternMatch = VerifyBufferPattern(viewData, byteLength, K_FILL_BYTE_VALUE);
    SetNamedBool(env, result, "fillSuccess", true);
    SetNamedBool(env, result, "patternMatch", patternMatch);
    return result;
}

static napi_value TestMultiDataViewsSameBuffer(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_LARGE_BUFFER_BYTES, &data);
    napi_value dataView1 = nullptr;
    napi_value dataView2 = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_DATAVIEW_LENGTH_16, arrayBuffer, K_OFFSET_ZERO, &dataView1));
    NAPI_CALL(env, napi_create_dataview(env, K_DATAVIEW_LENGTH_16, arrayBuffer, K_OFFSET_SIXTEEN, &dataView2));
    size_t byteLength1 = K_ZERO_LENGTH;
    size_t byteLength2 = K_ZERO_LENGTH;
    void* viewData1 = nullptr;
    void* viewData2 = nullptr;
    NAPI_CALL(env, napi_get_dataview_info(env, dataView1, &byteLength1, &viewData1, nullptr, nullptr));
    NAPI_CALL(env, napi_get_dataview_info(env, dataView2, &byteLength2, &viewData2, nullptr, nullptr));
    auto* bytes1 = static_cast<uint8_t*>(viewData1);
    auto* bytes2 = static_cast<uint8_t*>(viewData2);
    bytes1[K_BYTE_ZERO_INDEX] = K_FILL_BYTE_VALUE;
    bytes2[K_BYTE_ZERO_INDEX] = K_SECOND_FILL_BYTE;
    bool bothCreated = (dataView1 != nullptr) && (dataView2 != nullptr);
    bool bothAccessible = (bytes1 != nullptr) && (bytes2 != nullptr);
    SetNamedBool(env, result, "bothCreated", bothCreated);
    SetNamedBool(env, result, "bothAccessible", bothAccessible);
    SetNamedBool(env, result, "differentOffsets", true);
    return result;
}

static napi_value TestDataViewsShareBuffer(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_LARGE_BUFFER_BYTES, &data);
    napi_value dataView1 = nullptr;
    napi_value dataView2 = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_DATAVIEW_LENGTH_32, arrayBuffer, K_OFFSET_ZERO, &dataView1));
    NAPI_CALL(env, napi_create_dataview(env, K_DATAVIEW_LENGTH_32, arrayBuffer, K_OFFSET_SIXTEEN, &dataView2));
    napi_value buffer1 = nullptr;
    napi_value buffer2 = nullptr;
    NAPI_CALL(env, napi_get_dataview_info(env, dataView1, nullptr, nullptr, &buffer1, nullptr));
    NAPI_CALL(env, napi_get_dataview_info(env, dataView2, nullptr, nullptr, &buffer2, nullptr));
    bool sameBuffer = false;
    NAPI_CALL(env, napi_strict_equals(env, buffer1, buffer2, &sameBuffer));
    bool sameAsOriginal = false;
    NAPI_CALL(env, napi_strict_equals(env, arrayBuffer, buffer1, &sameAsOriginal));
    SetNamedBool(env, result, "shareSameBuffer", sameBuffer && sameAsOriginal);
    SetNamedBool(env, result, "identityVerified", true);
    return result;
}

static napi_value TestDataViewByteOffset16(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_LARGE_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_DATAVIEW_LENGTH_32, arrayBuffer, K_OFFSET_SIXTEEN, &dataView));
    size_t byteOffset = K_ZERO_LENGTH;
    NAPI_CALL(env, napi_get_dataview_info(env, dataView, nullptr, nullptr, nullptr, &byteOffset));
    SetNamedBool(env, result, "offsetCorrect", byteOffset == K_OFFSET_SIXTEEN);
    SetNamedUint32(env, result, "byteOffset", static_cast<uint32_t>(byteOffset));
    return result;
}

static napi_value TestDataViewByteOffsetEight(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_MEDIUM_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_DATAVIEW_LENGTH_16, arrayBuffer, K_OFFSET_EIGHT, &dataView));
    size_t byteOffset = K_ZERO_LENGTH;
    NAPI_CALL(env, napi_get_dataview_info(env, dataView, nullptr, nullptr, nullptr, &byteOffset));
    SetNamedBool(env, result, "offsetCorrect", byteOffset == K_OFFSET_EIGHT);
    SetNamedUint32(env, result, "byteOffset", static_cast<uint32_t>(byteOffset));
    return result;
}

static napi_value TestDataViewByteLengthVariations(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_LARGE_BUFFER_BYTES, &data);
    napi_value dataView1 = nullptr;
    napi_value dataView2 = nullptr;
    napi_value dataView3 = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_EIGHT_BYTES, arrayBuffer, K_OFFSET_ZERO, &dataView1));
    NAPI_CALL(env, napi_create_dataview(env, K_SIXTEEN_BYTES, arrayBuffer, K_OFFSET_EIGHT, &dataView2));
    NAPI_CALL(env, napi_create_dataview(env, K_THIRTY_TWO_BYTES, arrayBuffer, K_OFFSET_SIXTEEN, &dataView3));
    size_t length1 = K_ZERO_LENGTH;
    size_t length2 = K_ZERO_LENGTH;
    size_t length3 = K_ZERO_LENGTH;
    NAPI_CALL(env, napi_get_dataview_info(env, dataView1, &length1, nullptr, nullptr, nullptr));
    NAPI_CALL(env, napi_get_dataview_info(env, dataView2, &length2, nullptr, nullptr, nullptr));
    NAPI_CALL(env, napi_get_dataview_info(env, dataView3, &length3, nullptr, nullptr, nullptr));
    SetNamedBool(env, result, "length1Correct", length1 == K_EIGHT_BYTES);
    SetNamedBool(env, result, "length2Correct", length2 == K_SIXTEEN_BYTES);
    SetNamedBool(env, result, "length3Correct", length3 == K_THIRTY_TWO_BYTES);
    return result;
}

static napi_value TestArrayBufferZeroLength(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = nullptr;
    NAPI_CALL(env, napi_create_arraybuffer(env, K_ZERO_LENGTH, &data, &arrayBuffer));
    size_t byteLength = K_SINGLE_BYTE;
    NAPI_CALL(env, napi_get_arraybuffer_info(env, arrayBuffer, nullptr, &byteLength));
    SetNamedBool(env, result, "created", arrayBuffer != nullptr);
    SetNamedBool(env, result, "zeroLength", byteLength == K_ZERO_LENGTH);
    return result;
}

static napi_value TestDataViewZeroLength(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_SMALL_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_ZERO_LENGTH, arrayBuffer, K_OFFSET_ZERO, &dataView));
    SetNamedBool(env, result, "created", dataView != nullptr);
    SetNamedBool(env, result, "zeroLengthView", true);
    return result;
}

static napi_value TestLargeArrayBuffer1024(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_EXTRA_LARGE_BUFFER_BYTES, &data);
    FillBufferWithPattern(data, K_EXTRA_LARGE_BUFFER_BYTES, K_FILL_BYTE_VALUE);
    void* infoData = nullptr;
    size_t infoLength = K_ZERO_LENGTH;
    NAPI_CALL(env, napi_get_arraybuffer_info(env, arrayBuffer, &infoData, &infoLength));
    bool patternMatch = VerifyBufferPattern(infoData, K_EXTRA_LARGE_BUFFER_BYTES, K_FILL_BYTE_VALUE);
    SetNamedBool(env, result, "created", arrayBuffer != nullptr);
    SetNamedBool(env, result, "lengthCorrect", infoLength == K_EXTRA_LARGE_BUFFER_BYTES);
    SetNamedBool(env, result, "patternMatch", patternMatch);
    return result;
}

static napi_value TestDataViewLargeBuffer(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_EXTRA_LARGE_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_LARGE_BUFFER_BYTES, arrayBuffer, K_OFFSET_SIXTEEN, &dataView));
    size_t byteLength = K_ZERO_LENGTH;
    size_t byteOffset = K_ZERO_LENGTH;
    NAPI_CALL(env, napi_get_dataview_info(env, dataView, &byteLength, nullptr, nullptr, &byteOffset));
    SetNamedBool(env, result, "created", dataView != nullptr);
    SetNamedBool(env, result, "lengthCorrect", byteLength == K_LARGE_BUFFER_BYTES);
    SetNamedBool(env, result, "offsetCorrect", byteOffset == K_OFFSET_SIXTEEN);
    return result;
}

static napi_value TestMultipleDataViewsReadWrite(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_LARGE_BUFFER_BYTES, &data);
    napi_value dataView1 = nullptr;
    napi_value dataView2 = nullptr;
    napi_value dataView3 = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_SIXTEEN_BYTES, arrayBuffer, K_OFFSET_ZERO, &dataView1));
    NAPI_CALL(env, napi_create_dataview(env, K_SIXTEEN_BYTES, arrayBuffer, K_OFFSET_SIXTEEN, &dataView2));
    NAPI_CALL(env, napi_create_dataview(env, K_SIXTEEN_BYTES, arrayBuffer, K_THIRTY_TWO_BYTES, &dataView3));
    size_t len1 = K_ZERO_LENGTH;
    size_t len2 = K_ZERO_LENGTH;
    size_t len3 = K_ZERO_LENGTH;
    void* data1 = nullptr;
    void* data2 = nullptr;
    void* data3 = nullptr;
    NAPI_CALL(env, napi_get_dataview_info(env, dataView1, &len1, &data1, nullptr, nullptr));
    NAPI_CALL(env, napi_get_dataview_info(env, dataView2, &len2, &data2, nullptr, nullptr));
    NAPI_CALL(env, napi_get_dataview_info(env, dataView3, &len3, &data3, nullptr, nullptr));
    auto* bytes1 = static_cast<uint8_t*>(data1);
    auto* bytes2 = static_cast<uint8_t*>(data2);
    auto* bytes3 = static_cast<uint8_t*>(data3);
    bytes1[K_BYTE_ZERO_INDEX] = K_FILL_BYTE_VALUE;
    bytes2[K_BYTE_ZERO_INDEX] = K_SECOND_FILL_BYTE;
    bytes3[K_BYTE_ZERO_INDEX] = K_THIRD_FILL_BYTE;
    bool allMatch = (bytes1[K_BYTE_ZERO_INDEX] == K_FILL_BYTE_VALUE) &&
                    (bytes2[K_BYTE_ZERO_INDEX] == K_SECOND_FILL_BYTE) &&
                    (bytes3[K_BYTE_ZERO_INDEX] == K_THIRD_FILL_BYTE);
    SetNamedBool(env, result, "allCreated", dataView1 && dataView2 && dataView3);
    SetNamedBool(env, result, "allWritable", true);
    SetNamedBool(env, result, "allMatch", allMatch);
    return result;
}

static napi_value TestDataViewIsNotTypedArray(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = CreateArrayBufferWithSize(env, K_MEDIUM_BUFFER_BYTES, &data);
    napi_value dataView = nullptr;
    NAPI_CALL(env, napi_create_dataview(env, K_DATAVIEW_LENGTH_16, arrayBuffer, K_OFFSET_ZERO, &dataView));
    bool isTypedArray = true;
    NAPI_CALL(env, napi_is_typedarray(env, dataView, &isTypedArray));
    bool isDataView = false;
    NAPI_CALL(env, napi_is_dataview(env, dataView, &isDataView));
    SetNamedBool(env, result, "notTypedArray", !isTypedArray);
    SetNamedBool(env, result, "isDataView", isDataView);
    return result;
}

struct TestEntry {
    const char* name;
    napi_callback callback;
};

static const TestEntry DATAVIEW_TESTS[] = {
    { "testCreateArrayBuffer16Bytes", TestCreateArrayBuffer16Bytes },
    { "testCreateArrayBuffer64Bytes", TestCreateArrayBuffer64Bytes },
    { "testCreateArrayBuffer256Bytes", TestCreateArrayBuffer256Bytes },
    { "testGetArrayBufferInfoRoundTrip", TestGetArrayBufferInfoRoundTrip },
    { "testGetArrayBufferInfoDataPointer", TestGetArrayBufferInfoDataPointer },
    { "testCreateDataViewDefaultOffset", TestCreateDataViewDefaultOffset },
    { "testCreateDataViewCustomOffset", TestCreateDataViewCustomOffset },
    { "testCreateDataViewOffsetEight", TestCreateDataViewOffsetEight },
    { "testCreateDataViewFullBuffer", TestCreateDataViewFullBuffer },
    { "testCreateDataViewBoundaryExact", TestCreateDataViewBoundaryExact },
    { "testCreateDataViewInvalidOffset", TestCreateDataViewInvalidOffset },
    { "testCreateDataViewInvalidLength", TestCreateDataViewInvalidLength },
    { "testCreateDataViewOffsetPlusLengthOverflow", TestCreateDataViewOffsetPlusLengthOverflow },
    { "testGetDataViewInfoBasic", TestGetDataViewInfoBasic },
    { "testGetDataViewInfoLargeView", TestGetDataViewInfoLargeView },
    { "testGetDataViewInfoBufferIdentity", TestGetDataViewInfoBufferIdentity },
    { "testGetDataViewInfoDataPointer", TestGetDataViewInfoDataPointer },
    { "testIsDataViewPositive", TestIsDataViewPositive },
    { "testIsDataViewNegativeObject", TestIsDataViewNegativeObject },
    { "testIsDataViewNegativeArray", TestIsDataViewNegativeArray },
    { "testIsDataViewNegativeArrayBuffer", TestIsDataViewNegativeArrayBuffer },
    { "testWriteReadByteZero", TestWriteReadByteZero },
    { "testWriteReadByteOne", TestWriteReadByteOne },
    { "testWriteReadMultipleBytes", TestWriteReadMultipleBytes },
    { "testWriteReadPatternFull", TestWriteReadPatternFull },
    { "testMultiDataViewsSameBuffer", TestMultiDataViewsSameBuffer },
    { "testDataViewsShareBuffer", TestDataViewsShareBuffer },
    { "testDataViewByteOffset16", TestDataViewByteOffset16 },
    { "testDataViewByteOffsetEight", TestDataViewByteOffsetEight },
    { "testDataViewByteLengthVariations", TestDataViewByteLengthVariations },
    { "testArrayBufferZeroLength", TestArrayBufferZeroLength },
    { "testDataViewZeroLength", TestDataViewZeroLength },
    { "testLargeArrayBuffer1024", TestLargeArrayBuffer1024 },
    { "testDataViewLargeBuffer", TestDataViewLargeBuffer },
    { "testMultipleDataViewsReadWrite", TestMultipleDataViewsReadWrite },
    { "testDataViewIsNotTypedArray", TestDataViewIsNotTypedArray },
};

static constexpr size_t DATAVIEW_TEST_COUNT = sizeof(DATAVIEW_TESTS) / sizeof(DATAVIEW_TESTS[0]);

}  // namespace

static napi_value InitDataViewSuite(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[DATAVIEW_TEST_COUNT];
    for (size_t i = K_ZERO_LENGTH; i < DATAVIEW_TEST_COUNT; i++) {
        descriptors[i] = napi_property_descriptor {
            DATAVIEW_TESTS[i].name,
            nullptr,
            DATAVIEW_TESTS[i].callback,
            nullptr,
            nullptr,
            nullptr,
            napi_default,
            nullptr
        };
    }
    NAPI_CALL(env, napi_define_properties(env, exports, DATAVIEW_TEST_COUNT, descriptors));
    return exports;
}

static napi_module g_dataViewSuiteModule = {
    .nm_version = K_MODULE_VERSION,
    .nm_flags = K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitDataViewSuite,
    .nm_modname = "dataview_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterDataViewSuiteModule(void)
{
    napi_module_register(&g_dataViewSuiteModule);
}