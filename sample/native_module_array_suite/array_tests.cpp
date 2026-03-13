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
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

struct ArrayCaseSpec {
    const char* name;
    int32_t weight;
    int32_t offset;
    int32_t threshold;
};

static const ArrayCaseSpec g_arrayCaseSpecs[] = {
    {
        "arrayCase01",
        2,
        -9,
        8,
    },
    {
        "arrayCase02",
        3,
        -8,
        16,
    },
    {
        "arrayCase03",
        4,
        -7,
        24,
    },
    {
        "arrayCase04",
        5,
        -6,
        32,
    },
    {
        "arrayCase05",
        1,
        -5,
        40,
    },
    {
        "arrayCase06",
        2,
        -4,
        48,
    },
    {
        "arrayCase07",
        3,
        -3,
        56,
    },
    {
        "arrayCase08",
        4,
        -2,
        64,
    },
    {
        "arrayCase09",
        5,
        -1,
        72,
    },
    {
        "arrayCase10",
        1,
        0,
        80,
    },
    {
        "arrayCase11",
        2,
        1,
        88,
    },
    {
        "arrayCase12",
        3,
        2,
        96,
    },
    {
        "arrayCase13",
        4,
        3,
        104,
    },
    {
        "arrayCase14",
        5,
        4,
        112,
    },
    {
        "arrayCase15",
        1,
        5,
        120,
    },
    {
        "arrayCase16",
        2,
        6,
        128,
    },
    {
        "arrayCase17",
        3,
        7,
        136,
    },
    {
        "arrayCase18",
        4,
        8,
        144,
    },
    {
        "arrayCase19",
        5,
        9,
        152,
    },
    {
        "arrayCase20",
        1,
        10,
        160,
    },
};

bool ReadInt32(napi_env env, napi_value value, int32_t* result)
{
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, value, &type) != napi_ok) {
        return false;
    }
    if (type != napi_number) {
        return false;
    }
    return napi_get_value_int32(env, value, result) == napi_ok;
}

bool ReadInt32Array(napi_env env, napi_value value, std::vector<int32_t>* result)
{
    bool isArray = false;
    if (napi_is_array(env, value, &isArray) != napi_ok || !isArray) {
        napi_throw_type_error(env, nullptr, "value must be an array");
        return false;
    }

    uint32_t length = 0;
    if (napi_get_array_length(env, value, &length) != napi_ok) {
        return false;
    }

    result->clear();
    result->reserve(length);
    for (uint32_t index = 0; index < length; index++) {
        napi_value element = nullptr;
        int32_t current = 0;
        if (napi_get_element(env, value, index, &element) != napi_ok || !ReadInt32(env, element, &current)) {
            napi_throw_type_error(env, nullptr, "array elements must be numbers");
            return false;
        }
        result->push_back(current);
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

bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value)
{
    napi_value napiValue = nullptr;
    if (napi_get_boolean(env, value, &napiValue) != napi_ok) {
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

napi_value CreateArraySummary(napi_env env, const char* name, int32_t length, int32_t weightedSum, int32_t expected,
    int32_t evenCount, bool thresholdPassed)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", name);
    SetNamedInt32(env, result, "length", length);
    SetNamedInt32(env, result, "weightedSum", weightedSum);
    SetNamedInt32(env, result, "expected", expected);
    SetNamedInt32(env, result, "evenCount", evenCount);
    SetNamedBool(env, result, "thresholdPassed", thresholdPassed);
    SetNamedBool(env, result, "passed", weightedSum == expected);
    return result;
}

static napi_value TestArrayCase01(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::vector<int32_t> values;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto& spec = g_arrayCaseSpecs[0];
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & 1) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

static napi_value TestArrayCase02(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::vector<int32_t> values;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto& spec = g_arrayCaseSpecs[1];
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & 1) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

static napi_value TestArrayCase03(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::vector<int32_t> values;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto& spec = g_arrayCaseSpecs[2];
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & 1) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

static napi_value TestArrayCase04(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::vector<int32_t> values;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto& spec = g_arrayCaseSpecs[3];
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & 1) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

static napi_value TestArrayCase05(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::vector<int32_t> values;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto& spec = g_arrayCaseSpecs[4];
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & 1) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

static napi_value TestArrayCase06(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::vector<int32_t> values;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto& spec = g_arrayCaseSpecs[5];
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & 1) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

static napi_value TestArrayCase07(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::vector<int32_t> values;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto& spec = g_arrayCaseSpecs[6];
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & 1) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

static napi_value TestArrayCase08(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::vector<int32_t> values;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto& spec = g_arrayCaseSpecs[7];
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & 1) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

static napi_value TestArrayCase09(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::vector<int32_t> values;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto& spec = g_arrayCaseSpecs[8];
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & 1) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

static napi_value TestArrayCase10(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::vector<int32_t> values;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto& spec = g_arrayCaseSpecs[9];
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & 1) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

static napi_value TestArrayCase11(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::vector<int32_t> values;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto& spec = g_arrayCaseSpecs[10];
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & 1) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

static napi_value TestArrayCase12(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::vector<int32_t> values;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto& spec = g_arrayCaseSpecs[11];
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & 1) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

static napi_value TestArrayCase13(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::vector<int32_t> values;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto& spec = g_arrayCaseSpecs[12];
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & 1) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

static napi_value TestArrayCase14(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::vector<int32_t> values;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto& spec = g_arrayCaseSpecs[13];
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & 1) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

static napi_value TestArrayCase15(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::vector<int32_t> values;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto& spec = g_arrayCaseSpecs[14];
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & 1) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

static napi_value TestArrayCase16(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::vector<int32_t> values;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto& spec = g_arrayCaseSpecs[15];
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & 1) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

static napi_value TestArrayCase17(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::vector<int32_t> values;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto& spec = g_arrayCaseSpecs[16];
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & 1) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

static napi_value TestArrayCase18(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::vector<int32_t> values;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto& spec = g_arrayCaseSpecs[17];
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & 1) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

static napi_value TestArrayCase19(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::vector<int32_t> values;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto& spec = g_arrayCaseSpecs[18];
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & 1) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

static napi_value TestArrayCase20(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    std::vector<int32_t> values;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "array is required");
        return nullptr;
    }
    if (!ReadInt32Array(env, args[0], &values)) {
        return nullptr;
    }

    const auto& spec = g_arrayCaseSpecs[19];
    int32_t weightedSum = 0;
    int32_t evenCount = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        weightedSum += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
        if ((values[itemIndex] & 1) == 0) {
            evenCount++;
        }
    }

    int32_t expected = 0;
    for (size_t itemIndex = 0; itemIndex < values.size(); itemIndex++) {
        expected += (values[itemIndex] + spec.offset) * static_cast<int32_t>(itemIndex + spec.weight);
    }

    return CreateArraySummary(env, spec.name, static_cast<int32_t>(values.size()), weightedSum, expected, evenCount,
        weightedSum >= spec.threshold);
}

}  // namespace

static napi_value InitBranch03Array(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[] = {
        DECLARE_NAPI_FUNCTION("testArrayCase01", TestArrayCase01),
        DECLARE_NAPI_FUNCTION("testArrayCase02", TestArrayCase02),
        DECLARE_NAPI_FUNCTION("testArrayCase03", TestArrayCase03),
        DECLARE_NAPI_FUNCTION("testArrayCase04", TestArrayCase04),
        DECLARE_NAPI_FUNCTION("testArrayCase05", TestArrayCase05),
        DECLARE_NAPI_FUNCTION("testArrayCase06", TestArrayCase06),
        DECLARE_NAPI_FUNCTION("testArrayCase07", TestArrayCase07),
        DECLARE_NAPI_FUNCTION("testArrayCase08", TestArrayCase08),
        DECLARE_NAPI_FUNCTION("testArrayCase09", TestArrayCase09),
        DECLARE_NAPI_FUNCTION("testArrayCase10", TestArrayCase10),
        DECLARE_NAPI_FUNCTION("testArrayCase11", TestArrayCase11),
        DECLARE_NAPI_FUNCTION("testArrayCase12", TestArrayCase12),
        DECLARE_NAPI_FUNCTION("testArrayCase13", TestArrayCase13),
        DECLARE_NAPI_FUNCTION("testArrayCase14", TestArrayCase14),
        DECLARE_NAPI_FUNCTION("testArrayCase15", TestArrayCase15),
        DECLARE_NAPI_FUNCTION("testArrayCase16", TestArrayCase16),
        DECLARE_NAPI_FUNCTION("testArrayCase17", TestArrayCase17),
        DECLARE_NAPI_FUNCTION("testArrayCase18", TestArrayCase18),
        DECLARE_NAPI_FUNCTION("testArrayCase19", TestArrayCase19),
        DECLARE_NAPI_FUNCTION("testArrayCase20", TestArrayCase20),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(descriptors) / sizeof(descriptors[0]), descriptors));
    return exports;
}

static napi_module g_branch03ArrayModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitBranch03Array,
    .nm_modname = "array_suite",
    .nm_priv = nullptr,
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterBranch03ArrayModule(void)
{
    napi_module_register(&g_branch03ArrayModule);
}
