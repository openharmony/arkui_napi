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

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

struct BufferCaseSpec {
    const char* name;
    int32_t seed;
    int32_t stride;
    int32_t mask;
};

static const BufferCaseSpec g_bufferCaseSpecs[] = {
    {
        "bufferCase01",
        5,
        2,
        17,
    },
    {
        "bufferCase02",
        10,
        3,
        18,
    },
    {
        "bufferCase03",
        15,
        4,
        19,
    },
    {
        "bufferCase04",
        20,
        1,
        20,
    },
    {
        "bufferCase05",
        25,
        2,
        21,
    },
    {
        "bufferCase06",
        30,
        3,
        22,
    },
    {
        "bufferCase07",
        35,
        4,
        23,
    },
    {
        "bufferCase08",
        40,
        1,
        24,
    },
    {
        "bufferCase09",
        45,
        2,
        25,
    },
    {
        "bufferCase10",
        50,
        3,
        26,
    },
    {
        "bufferCase11",
        55,
        4,
        27,
    },
    {
        "bufferCase12",
        60,
        1,
        28,
    },
    {
        "bufferCase13",
        65,
        2,
        29,
    },
    {
        "bufferCase14",
        70,
        3,
        30,
    },
    {
        "bufferCase15",
        75,
        4,
        31,
    },
    {
        "bufferCase16",
        80,
        1,
        32,
    },
    {
        "bufferCase17",
        85,
        2,
        33,
    },
    {
        "bufferCase18",
        90,
        3,
        34,
    },
    {
        "bufferCase19",
        95,
        4,
        35,
    },
    {
        "bufferCase20",
        100,
        1,
        36,
    },
    {
        "bufferCase21",
        105,
        2,
        37,
    },
};

bool ReadBuffer(napi_env env, napi_value value, uint8_t** data, size_t* length)
{
    bool isBuffer = false;
    if (napi_is_buffer(env, value, &isBuffer) != napi_ok || !isBuffer) {
        napi_throw_type_error(env, nullptr, "value must be a Buffer");
        return false;
    }

    void* rawData = nullptr;
    if (napi_get_buffer_info(env, value, &rawData, length) != napi_ok) {
        return false;
    }
    *data = static_cast<uint8_t*>(rawData);
    return true;
}

bool CreateModifiedCopy(napi_env env, const uint8_t* source, size_t length, int32_t mask, napi_value* result)
{
    void* rawData = nullptr;
    if (napi_create_buffer(env, length, &rawData, result) != napi_ok) {
        return false;
    }
    auto* target = static_cast<uint8_t*>(rawData);
    for (size_t index = 0; index < length; index++) {
        target[index] = static_cast<uint8_t>(source[index] ^ mask);
    }
    return true;
}

bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int32(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedString(napi_env env, napi_value object, const char* name, const char* value)
{
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

napi_value CreateBufferSummary(napi_env env, const char* name, int32_t length, int32_t checksum, napi_value copy)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", name);
    SetNamedInt32(env, result, "length", length);
    SetNamedInt32(env, result, "checksum", checksum);
    NAPI_CALL(env, napi_set_named_property(env, result, "copy", copy));
    return result;
}

static napi_value TestBufferCase01(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[0];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

static napi_value TestBufferCase02(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[1];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

static napi_value TestBufferCase03(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[2];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

static napi_value TestBufferCase04(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[3];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

static napi_value TestBufferCase05(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[4];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

static napi_value TestBufferCase06(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[5];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

static napi_value TestBufferCase07(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[6];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

static napi_value TestBufferCase08(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[7];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

static napi_value TestBufferCase09(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[8];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

static napi_value TestBufferCase10(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[9];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

static napi_value TestBufferCase11(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[10];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

static napi_value TestBufferCase12(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[11];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

static napi_value TestBufferCase13(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[12];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

static napi_value TestBufferCase14(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[13];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

static napi_value TestBufferCase15(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[14];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

static napi_value TestBufferCase16(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[15];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

static napi_value TestBufferCase17(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[16];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

static napi_value TestBufferCase18(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[17];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

static napi_value TestBufferCase19(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[18];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

static napi_value TestBufferCase20(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[19];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

static napi_value TestBufferCase21(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    uint8_t* bytes = nullptr;
    size_t length = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "buffer is required");
        return nullptr;
    }
    if (!ReadBuffer(env, args[0], &bytes, &length)) {
        return nullptr;
    }

    const auto& spec = g_bufferCaseSpecs[20];
    int32_t checksum = 0;
    for (size_t itemIndex = 0; itemIndex < length; itemIndex += static_cast<size_t>(spec.stride)) {
        checksum += static_cast<int32_t>((bytes[itemIndex] + spec.seed) ^ spec.mask);
    }

    napi_value copy = nullptr;
    if (!CreateModifiedCopy(env, bytes, length, spec.mask, &copy)) {
        return nullptr;
    }

    return CreateBufferSummary(env, spec.name, static_cast<int32_t>(length), checksum, copy);
}

}  // namespace

static napi_value InitBranch07Buffer(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[] = {
        DECLARE_NAPI_FUNCTION("testBufferCase01", TestBufferCase01),
        DECLARE_NAPI_FUNCTION("testBufferCase02", TestBufferCase02),
        DECLARE_NAPI_FUNCTION("testBufferCase03", TestBufferCase03),
        DECLARE_NAPI_FUNCTION("testBufferCase04", TestBufferCase04),
        DECLARE_NAPI_FUNCTION("testBufferCase05", TestBufferCase05),
        DECLARE_NAPI_FUNCTION("testBufferCase06", TestBufferCase06),
        DECLARE_NAPI_FUNCTION("testBufferCase07", TestBufferCase07),
        DECLARE_NAPI_FUNCTION("testBufferCase08", TestBufferCase08),
        DECLARE_NAPI_FUNCTION("testBufferCase09", TestBufferCase09),
        DECLARE_NAPI_FUNCTION("testBufferCase10", TestBufferCase10),
        DECLARE_NAPI_FUNCTION("testBufferCase11", TestBufferCase11),
        DECLARE_NAPI_FUNCTION("testBufferCase12", TestBufferCase12),
        DECLARE_NAPI_FUNCTION("testBufferCase13", TestBufferCase13),
        DECLARE_NAPI_FUNCTION("testBufferCase14", TestBufferCase14),
        DECLARE_NAPI_FUNCTION("testBufferCase15", TestBufferCase15),
        DECLARE_NAPI_FUNCTION("testBufferCase16", TestBufferCase16),
        DECLARE_NAPI_FUNCTION("testBufferCase17", TestBufferCase17),
        DECLARE_NAPI_FUNCTION("testBufferCase18", TestBufferCase18),
        DECLARE_NAPI_FUNCTION("testBufferCase19", TestBufferCase19),
        DECLARE_NAPI_FUNCTION("testBufferCase20", TestBufferCase20),
        DECLARE_NAPI_FUNCTION("testBufferCase21", TestBufferCase21),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(descriptors) / sizeof(descriptors[0]), descriptors));
    return exports;
}

static napi_module g_branch07BufferModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitBranch07Buffer,
    .nm_modname = "buffer_suite",
    .nm_priv = nullptr,
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterBranch07BufferModule(void)
{
    napi_module_register(&g_branch07BufferModule);
}
