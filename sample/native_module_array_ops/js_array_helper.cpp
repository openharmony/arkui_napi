/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "js_array_helper.h"

/***********************************************
 * ArrayHelper Implementation
 ***********************************************/
std::vector<double> ArrayHelper::Sort(const std::vector<double>& arr, bool ascending)
{
    std::vector<double> result = arr;
    if (ascending) {
        std::sort(result.begin(), result.end());
    } else {
        std::sort(result.begin(), result.end(), std::greater<double>());
    }
    return result;
}

std::vector<double> ArrayHelper::Reverse(const std::vector<double>& arr)
{
    std::vector<double> result = arr;
    std::reverse(result.begin(), result.end());
    return result;
}

std::vector<double> ArrayHelper::Unique(const std::vector<double>& arr)
{
    std::vector<double> result;
    std::set<double> seen;
    for (double val : arr) {
        if (seen.find(val) == seen.end()) {
            seen.insert(val);
            result.push_back(val);
        }
    }
    return result;
}

std::vector<double> ArrayHelper::Flatten(const std::vector<std::vector<double>>& arr)
{
    std::vector<double> result;
    for (const auto& inner : arr) {
        for (double val : inner) {
            result.push_back(val);
        }
    }
    return result;
}

std::vector<std::vector<double>> ArrayHelper::Chunk(const std::vector<double>& arr, size_t chunkSize)
{
    std::vector<std::vector<double>> result;
    if (chunkSize == 0) {
        return result;
    }
    for (size_t i = 0; i < arr.size(); i += chunkSize) {
        size_t end = std::min(i + chunkSize, arr.size());
        result.push_back(std::vector<double>(arr.begin() + i, arr.begin() + end));
    }
    return result;
}

std::vector<double> ArrayHelper::Range(double start, double end, double step)
{
    std::vector<double> result;
    if (step == 0.0) {
        return result;
    }
    if (step > 0) {
        for (double i = start; i < end; i += step) {
            result.push_back(i);
            if (result.size() >= ARRAY_MAX_SIZE) {
                break;
            }
        }
    } else {
        for (double i = start; i > end; i += step) {
            result.push_back(i);
            if (result.size() >= ARRAY_MAX_SIZE) {
                break;
            }
        }
    }
    return result;
}

std::vector<double> ArrayHelper::Fill(size_t count, double value)
{
    if (count > ARRAY_MAX_SIZE) {
        count = ARRAY_MAX_SIZE;
    }
    return std::vector<double>(count, value);
}

std::vector<double> ArrayHelper::Concat(const std::vector<double>& a, const std::vector<double>& b)
{
    std::vector<double> result = a;
    result.insert(result.end(), b.begin(), b.end());
    return result;
}

std::vector<double> ArrayHelper::Slice(const std::vector<double>& arr, int start, int end)
{
    int size = static_cast<int>(arr.size());
    if (start < 0) {
        start = std::max(0, size + start);
    }
    if (end < 0) {
        end = std::max(0, size + end);
    }
    if (start >= size) {
        return {};
    }
    if (end > size) {
        end = size;
    }
    if (start >= end) {
        return {};
    }
    return std::vector<double>(arr.begin() + start, arr.begin() + end);
}

std::vector<double> ArrayHelper::Take(const std::vector<double>& arr, size_t count)
{
    if (count >= arr.size()) {
        return arr;
    }
    return std::vector<double>(arr.begin(), arr.begin() + count);
}

std::vector<double> ArrayHelper::TakeLast(const std::vector<double>& arr, size_t count)
{
    if (count >= arr.size()) {
        return arr;
    }
    return std::vector<double>(arr.end() - count, arr.end());
}

std::vector<double> ArrayHelper::Drop(const std::vector<double>& arr, size_t count)
{
    if (count >= arr.size()) {
        return {};
    }
    return std::vector<double>(arr.begin() + count, arr.end());
}

std::vector<double> ArrayHelper::DropLast(const std::vector<double>& arr, size_t count)
{
    if (count >= arr.size()) {
        return {};
    }
    return std::vector<double>(arr.begin(), arr.end() - count);
}

std::vector<double> ArrayHelper::Intersection(const std::vector<double>& a, const std::vector<double>& b)
{
    std::set<double> setB(b.begin(), b.end());
    std::vector<double> result;
    std::set<double> seen;
    for (double val : a) {
        if (setB.count(val) > 0 && seen.find(val) == seen.end()) {
            result.push_back(val);
            seen.insert(val);
        }
    }
    return result;
}

std::vector<double> ArrayHelper::Union(const std::vector<double>& a, const std::vector<double>& b)
{
    std::vector<double> combined = a;
    combined.insert(combined.end(), b.begin(), b.end());
    return Unique(combined);
}

std::vector<double> ArrayHelper::Difference(const std::vector<double>& a, const std::vector<double>& b)
{
    std::set<double> setB(b.begin(), b.end());
    std::vector<double> result;
    for (double val : a) {
        if (setB.count(val) == 0) {
            result.push_back(val);
        }
    }
    return result;
}

int ArrayHelper::IndexOf(const std::vector<double>& arr, double value)
{
    for (size_t i = 0; i < arr.size(); i++) {
        if (std::abs(arr[i] - value) < 1e-10) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int ArrayHelper::LastIndexOf(const std::vector<double>& arr, double value)
{
    for (int i = static_cast<int>(arr.size()) - 1; i >= 0; i--) {
        if (std::abs(arr[i] - value) < 1e-10) {
            return i;
        }
    }
    return -1;
}

bool ArrayHelper::Contains(const std::vector<double>& arr, double value)
{
    return IndexOf(arr, value) >= 0;
}

size_t ArrayHelper::Count(const std::vector<double>& arr, double value)
{
    size_t count = 0;
    for (double val : arr) {
        if (std::abs(val - value) < 1e-10) {
            count++;
        }
    }
    return count;
}

std::vector<double> ArrayHelper::Without(const std::vector<double>& arr, double value)
{
    std::vector<double> result;
    for (double val : arr) {
        if (std::abs(val - value) >= 1e-10) {
            result.push_back(val);
        }
    }
    return result;
}

std::vector<double> ArrayHelper::Compact(const std::vector<double>& arr)
{
    std::vector<double> result;
    for (double val : arr) {
        if (std::abs(val) >= 1e-10 && !std::isnan(val)) {
            result.push_back(val);
        }
    }
    return result;
}

std::vector<double> ArrayHelper::Zip(const std::vector<double>& a, const std::vector<double>& b)
{
    std::vector<double> result;
    size_t minLen = std::min(a.size(), b.size());
    for (size_t i = 0; i < minLen; i++) {
        result.push_back(a[i]);
        result.push_back(b[i]);
    }
    return result;
}

/***********************************************
 * NAPI Helper
 ***********************************************/
static bool GetDoubleArrayFromNapi(napi_env env, napi_value jsArray, std::vector<double>& result)
{
    bool isArray = false;
    napi_is_array(env, jsArray, &isArray);
    if (!isArray) {
        return false;
    }
    uint32_t length = 0;
    napi_get_array_length(env, jsArray, &length);
    if (length > ARRAY_MAX_SIZE) {
        length = ARRAY_MAX_SIZE;
    }
    result.resize(length);
    for (uint32_t i = 0; i < length; i++) {
        napi_value element = nullptr;
        napi_get_element(env, jsArray, i, &element);
        napi_get_value_double(env, element, &result[i]);
    }
    return true;
}

static napi_value CreateDoubleArrayNapi(napi_env env, const std::vector<double>& arr)
{
    napi_value result = nullptr;
    napi_create_array_with_length(env, arr.size(), &result);
    for (size_t i = 0; i < arr.size(); i++) {
        napi_value element = nullptr;
        napi_create_double(env, arr[i], &element);
        napi_set_element(env, result, i, element);
    }
    return result;
}

/***********************************************
 * NAPI Functions
 ***********************************************/
static napi_value JSArraySort(napi_env env, napi_callback_info info)
{
    size_t argc = ArrayArgCount::TWO;
    napi_value argv[ArrayArgCount::TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires at least 1 parameter");
    std::vector<double> arr;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], arr), "argument must be array");
    bool ascending = true;
    if (argc >= ArrayArgCount::TWO) {
        napi_get_value_bool(env, argv[1], &ascending);
    }
    return CreateDoubleArrayNapi(env, ArrayHelper::Sort(arr, ascending));
}

static napi_value JSArrayReverse(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    std::vector<double> arr;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], arr), "argument must be array");
    return CreateDoubleArrayNapi(env, ArrayHelper::Reverse(arr));
}

static napi_value JSArrayUnique(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    std::vector<double> arr;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], arr), "argument must be array");
    return CreateDoubleArrayNapi(env, ArrayHelper::Unique(arr));
}

static napi_value JSArrayChunk(napi_env env, napi_callback_info info)
{
    size_t argc = ArrayArgCount::TWO;
    napi_value argv[ArrayArgCount::TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ArrayArgCount::TWO, "requires 2 parameters");
    std::vector<double> arr;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], arr), "first argument must be array");
    int32_t chunkSize = 1;
    napi_get_value_int32(env, argv[1], &chunkSize);
    auto chunks = ArrayHelper::Chunk(arr, static_cast<size_t>(chunkSize));
    napi_value result = nullptr;
    napi_create_array_with_length(env, chunks.size(), &result);
    for (size_t i = 0; i < chunks.size(); i++) {
        napi_value inner = CreateDoubleArrayNapi(env, chunks[i]);
        napi_set_element(env, result, i, inner);
    }
    return result;
}

static napi_value JSArrayRange(napi_env env, napi_callback_info info)
{
    size_t argc = ArrayArgCount::THREE;
    napi_value argv[ArrayArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ArrayArgCount::TWO, "requires at least 2 parameters");
    double start;
    double end;
    napi_get_value_double(env, argv[0], &start);
    napi_get_value_double(env, argv[1], &end);
    double step = 1.0;
    if (argc >= ArrayArgCount::THREE) {
        napi_get_value_double(env, argv[ARRAY_THIRD_ARG], &step);
    }
    return CreateDoubleArrayNapi(env, ArrayHelper::Range(start, end, step));
}

static napi_value JSArrayFill(napi_env env, napi_callback_info info)
{
    size_t argc = ArrayArgCount::TWO;
    napi_value argv[ArrayArgCount::TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ArrayArgCount::TWO, "requires 2 parameters");
    int32_t count;
    napi_get_value_int32(env, argv[0], &count);
    double value;
    napi_get_value_double(env, argv[1], &value);
    return CreateDoubleArrayNapi(env, ArrayHelper::Fill(static_cast<size_t>(count), value));
}

static napi_value JSArrayConcat(napi_env env, napi_callback_info info)
{
    size_t argc = ArrayArgCount::TWO;
    napi_value argv[ArrayArgCount::TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ArrayArgCount::TWO, "requires 2 parameters");
    std::vector<double> a;
    std::vector<double> b;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], a), "first argument must be array");
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[1], b), "second argument must be array");
    return CreateDoubleArrayNapi(env, ArrayHelper::Concat(a, b));
}

static napi_value JSArraySlice(napi_env env, napi_callback_info info)
{
    size_t argc = ArrayArgCount::THREE;
    napi_value argv[ArrayArgCount::THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ArrayArgCount::THREE, "requires 3 parameters");
    std::vector<double> arr;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], arr), "first argument must be array");
    int32_t start;
    int32_t end;
    napi_get_value_int32(env, argv[1], &start);
    napi_get_value_int32(env, argv[ARRAY_THIRD_ARG], &end);
    return CreateDoubleArrayNapi(env, ArrayHelper::Slice(arr, start, end));
}

static napi_value JSArrayTake(napi_env env, napi_callback_info info)
{
    size_t argc = ArrayArgCount::TWO;
    napi_value argv[ArrayArgCount::TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ArrayArgCount::TWO, "requires 2 parameters");
    std::vector<double> arr;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], arr), "first argument must be array");
    int32_t count;
    napi_get_value_int32(env, argv[1], &count);
    return CreateDoubleArrayNapi(env, ArrayHelper::Take(arr, static_cast<size_t>(count)));
}

static napi_value JSArrayDrop(napi_env env, napi_callback_info info)
{
    size_t argc = ArrayArgCount::TWO;
    napi_value argv[ArrayArgCount::TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ArrayArgCount::TWO, "requires 2 parameters");
    std::vector<double> arr;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], arr), "first argument must be array");
    int32_t count;
    napi_get_value_int32(env, argv[1], &count);
    return CreateDoubleArrayNapi(env, ArrayHelper::Drop(arr, static_cast<size_t>(count)));
}

static napi_value JSArrayIntersection(napi_env env, napi_callback_info info)
{
    size_t argc = ArrayArgCount::TWO;
    napi_value argv[ArrayArgCount::TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ArrayArgCount::TWO, "requires 2 parameters");
    std::vector<double> a;
    std::vector<double> b;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], a), "first argument must be array");
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[1], b), "second argument must be array");
    return CreateDoubleArrayNapi(env, ArrayHelper::Intersection(a, b));
}

static napi_value JSArrayUnion(napi_env env, napi_callback_info info)
{
    size_t argc = ArrayArgCount::TWO;
    napi_value argv[ArrayArgCount::TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ArrayArgCount::TWO, "requires 2 parameters");
    std::vector<double> a;
    std::vector<double> b;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], a), "first argument must be array");
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[1], b), "second argument must be array");
    return CreateDoubleArrayNapi(env, ArrayHelper::Union(a, b));
}

static napi_value JSArrayDifference(napi_env env, napi_callback_info info)
{
    size_t argc = ArrayArgCount::TWO;
    napi_value argv[ArrayArgCount::TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ArrayArgCount::TWO, "requires 2 parameters");
    std::vector<double> a;
    std::vector<double> b;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], a), "first argument must be array");
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[1], b), "second argument must be array");
    return CreateDoubleArrayNapi(env, ArrayHelper::Difference(a, b));
}

static napi_value JSArrayIndexOf(napi_env env, napi_callback_info info)
{
    size_t argc = ArrayArgCount::TWO;
    napi_value argv[ArrayArgCount::TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ArrayArgCount::TWO, "requires 2 parameters");
    std::vector<double> arr;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], arr), "first argument must be array");
    double value;
    napi_get_value_double(env, argv[1], &value);
    napi_value result = nullptr;
    napi_create_int32(env, ArrayHelper::IndexOf(arr, value), &result);
    return result;
}

static napi_value JSArrayContains(napi_env env, napi_callback_info info)
{
    size_t argc = ArrayArgCount::TWO;
    napi_value argv[ArrayArgCount::TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ArrayArgCount::TWO, "requires 2 parameters");
    std::vector<double> arr;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], arr), "first argument must be array");
    double value;
    napi_get_value_double(env, argv[1], &value);
    napi_value result = nullptr;
    napi_get_boolean(env, ArrayHelper::Contains(arr, value), &result);
    return result;
}

static napi_value JSArrayWithout(napi_env env, napi_callback_info info)
{
    size_t argc = ArrayArgCount::TWO;
    napi_value argv[ArrayArgCount::TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ArrayArgCount::TWO, "requires 2 parameters");
    std::vector<double> arr;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], arr), "first argument must be array");
    double value;
    napi_get_value_double(env, argv[1], &value);
    return CreateDoubleArrayNapi(env, ArrayHelper::Without(arr, value));
}

static napi_value JSArrayCompact(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    std::vector<double> arr;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], arr), "argument must be array");
    return CreateDoubleArrayNapi(env, ArrayHelper::Compact(arr));
}

static napi_value JSArrayZip(napi_env env, napi_callback_info info)
{
    size_t argc = ArrayArgCount::TWO;
    napi_value argv[ArrayArgCount::TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ArrayArgCount::TWO, "requires 2 parameters");
    std::vector<double> a;
    std::vector<double> b;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], a), "first argument must be array");
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[1], b), "second argument must be array");
    return CreateDoubleArrayNapi(env, ArrayHelper::Zip(a, b));
}

static napi_value JSArrayTakeLast(napi_env env, napi_callback_info info)
{
    size_t argc = ArrayArgCount::TWO;
    napi_value argv[ArrayArgCount::TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ArrayArgCount::TWO, "requires 2 parameters");
    std::vector<double> arr;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], arr), "first argument must be array");
    int32_t count;
    napi_get_value_int32(env, argv[1], &count);
    return CreateDoubleArrayNapi(env, ArrayHelper::TakeLast(arr, static_cast<size_t>(count)));
}

static napi_value JSArrayDropLast(napi_env env, napi_callback_info info)
{
    size_t argc = ArrayArgCount::TWO;
    napi_value argv[ArrayArgCount::TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ArrayArgCount::TWO, "requires 2 parameters");
    std::vector<double> arr;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], arr), "first argument must be array");
    int32_t count;
    napi_get_value_int32(env, argv[1], &count);
    return CreateDoubleArrayNapi(env, ArrayHelper::DropLast(arr, static_cast<size_t>(count)));
}

static napi_value JSArrayLastIndexOf(napi_env env, napi_callback_info info)
{
    size_t argc = ArrayArgCount::TWO;
    napi_value argv[ArrayArgCount::TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ArrayArgCount::TWO, "requires 2 parameters");
    std::vector<double> arr;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], arr), "first argument must be array");
    double value;
    napi_get_value_double(env, argv[1], &value);
    napi_value result = nullptr;
    napi_create_int32(env, ArrayHelper::LastIndexOf(arr, value), &result);
    return result;
}

// flatten(arrays: number[][]): number[]
static napi_value JSArrayFlatten(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    bool isArray = false;
    napi_is_array(env, argv[0], &isArray);
    NAPI_ASSERT(env, isArray, "argument must be array of arrays");

    uint32_t outerLength = 0;
    napi_get_array_length(env, argv[0], &outerLength);

    std::vector<std::vector<double>> arrays;
    for (uint32_t i = 0; i < outerLength; i++) {
        napi_value inner = nullptr;
        napi_get_element(env, argv[0], i, &inner);
        std::vector<double> innerArr;
        if (GetDoubleArrayFromNapi(env, inner, innerArr)) {
            arrays.push_back(innerArr);
        }
    }

    return CreateDoubleArrayNapi(env, ArrayHelper::Flatten(arrays));
}

// size(arr: number[]): number
static napi_value JSArraySize(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    bool isArray = false;
    napi_is_array(env, argv[0], &isArray);
    NAPI_ASSERT(env, isArray, "argument must be array");

    uint32_t length = 0;
    napi_get_array_length(env, argv[0], &length);

    napi_value result = nullptr;
    napi_create_int32(env, static_cast<int32_t>(length), &result);
    return result;
}

// isEmpty(arr: number[]): boolean
static napi_value JSArrayIsEmpty(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    bool isArray = false;
    napi_is_array(env, argv[0], &isArray);
    NAPI_ASSERT(env, isArray, "argument must be array");

    uint32_t length = 0;
    napi_get_array_length(env, argv[0], &length);

    napi_value result = nullptr;
    napi_get_boolean(env, length == 0, &result);
    return result;
}

// first(arr: number[]): number
static napi_value JSArrayFirst(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    std::vector<double> arr;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], arr), "argument must be array");
    NAPI_ASSERT(env, !arr.empty(), "array must not be empty");

    napi_value result = nullptr;
    napi_create_double(env, arr.front(), &result);
    return result;
}

// last(arr: number[]): number
static napi_value JSArrayLast(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    std::vector<double> arr;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], arr), "argument must be array");
    NAPI_ASSERT(env, !arr.empty(), "array must not be empty");

    napi_value result = nullptr;
    napi_create_double(env, arr.back(), &result);
    return result;
}

// sum(arr: number[]): number
static napi_value JSArraySum(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    std::vector<double> arr;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], arr), "argument must be array");

    double sum = 0.0;
    for (double val : arr) {
        sum += val;
    }

    napi_value result = nullptr;
    napi_create_double(env, sum, &result);
    return result;
}

// product(arr: number[]): number
static napi_value JSArrayProduct(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    std::vector<double> arr;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], arr), "argument must be array");

    double product = 1.0;
    for (double val : arr) {
        product *= val;
    }

    napi_value result = nullptr;
    napi_create_double(env, product, &result);
    return result;
}

static napi_value JSArrayCount(napi_env env, napi_callback_info info)
{
    size_t argc = ArrayArgCount::TWO;
    napi_value argv[ArrayArgCount::TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= ArrayArgCount::TWO, "requires 2 parameters");
    std::vector<double> arr;
    NAPI_ASSERT(env, GetDoubleArrayFromNapi(env, argv[0], arr), "first argument must be array");
    double value;
    napi_get_value_double(env, argv[1], &value);
    napi_value result = nullptr;
    napi_create_int32(env, static_cast<int32_t>(ArrayHelper::Count(arr, value)), &result);
    return result;
}

/***********************************************
 * Module export and register
 ***********************************************/
static napi_value ArrayExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("sort", JSArraySort),
        DECLARE_NAPI_FUNCTION("reverse", JSArrayReverse),
        DECLARE_NAPI_FUNCTION("unique", JSArrayUnique),
        DECLARE_NAPI_FUNCTION("chunk", JSArrayChunk),
        DECLARE_NAPI_FUNCTION("range", JSArrayRange),
        DECLARE_NAPI_FUNCTION("fill", JSArrayFill),
        DECLARE_NAPI_FUNCTION("concat", JSArrayConcat),
        DECLARE_NAPI_FUNCTION("slice", JSArraySlice),
        DECLARE_NAPI_FUNCTION("take", JSArrayTake),
        DECLARE_NAPI_FUNCTION("drop", JSArrayDrop),
        DECLARE_NAPI_FUNCTION("intersection", JSArrayIntersection),
        DECLARE_NAPI_FUNCTION("union", JSArrayUnion),
        DECLARE_NAPI_FUNCTION("difference", JSArrayDifference),
        DECLARE_NAPI_FUNCTION("indexOf", JSArrayIndexOf),
        DECLARE_NAPI_FUNCTION("contains", JSArrayContains),
        DECLARE_NAPI_FUNCTION("without", JSArrayWithout),
        DECLARE_NAPI_FUNCTION("compact", JSArrayCompact),
        DECLARE_NAPI_FUNCTION("zip", JSArrayZip),
        DECLARE_NAPI_FUNCTION("count", JSArrayCount),
        DECLARE_NAPI_FUNCTION("takeLast", JSArrayTakeLast),
        DECLARE_NAPI_FUNCTION("dropLast", JSArrayDropLast),
        DECLARE_NAPI_FUNCTION("lastIndexOf", JSArrayLastIndexOf),
        DECLARE_NAPI_FUNCTION("flatten", JSArrayFlatten),
        DECLARE_NAPI_FUNCTION("size", JSArraySize),
        DECLARE_NAPI_FUNCTION("isEmpty", JSArrayIsEmpty),
        DECLARE_NAPI_FUNCTION("first", JSArrayFirst),
        DECLARE_NAPI_FUNCTION("last", JSArrayLast),
        DECLARE_NAPI_FUNCTION("arraySum", JSArraySum),
        DECLARE_NAPI_FUNCTION("product", JSArrayProduct),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module g_arrayModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = ArrayExport,
    .nm_modname = "array_ops",
    .nm_priv = nullptr,
    .reserved = { nullptr },
};

extern "C" __attribute__((constructor)) void ArrayRegister()
{
    napi_module_register(&g_arrayModule);
}
