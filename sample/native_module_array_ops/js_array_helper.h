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

#ifndef FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_ARRAY_JS_ARRAY_HELPER_H
#define FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_ARRAY_JS_ARRAY_HELPER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <vector>

constexpr size_t ARRAY_MAX_SIZE = 4096;

enum ArrayArgIndex {
    ARRAY_FIRST_ARG = 0,
    ARRAY_SECOND_ARG,
    ARRAY_THIRD_ARG,
};

namespace ArrayArgCount {
    constexpr size_t TWO = 2;
    constexpr size_t THREE = 3;
};

class ArrayHelper {
public:
    static std::vector<double> Sort(const std::vector<double>& arr, bool ascending);
    static std::vector<double> Reverse(const std::vector<double>& arr);
    static std::vector<double> Unique(const std::vector<double>& arr);
    static std::vector<double> Flatten(const std::vector<std::vector<double>>& arr);
    static std::vector<std::vector<double>> Chunk(const std::vector<double>& arr, size_t chunkSize);
    static std::vector<double> Range(double start, double end, double step);
    static std::vector<double> Fill(size_t count, double value);
    static std::vector<double> Concat(const std::vector<double>& a, const std::vector<double>& b);
    static std::vector<double> Slice(const std::vector<double>& arr, int start, int end);
    static std::vector<double> Take(const std::vector<double>& arr, size_t count);
    static std::vector<double> TakeLast(const std::vector<double>& arr, size_t count);
    static std::vector<double> Drop(const std::vector<double>& arr, size_t count);
    static std::vector<double> DropLast(const std::vector<double>& arr, size_t count);
    static std::vector<double> Intersection(const std::vector<double>& a, const std::vector<double>& b);
    static std::vector<double> Union(const std::vector<double>& a, const std::vector<double>& b);
    static std::vector<double> Difference(const std::vector<double>& a, const std::vector<double>& b);
    static int IndexOf(const std::vector<double>& arr, double value);
    static int LastIndexOf(const std::vector<double>& arr, double value);
    static bool Contains(const std::vector<double>& arr, double value);
    static size_t Count(const std::vector<double>& arr, double value);
    static std::vector<double> Without(const std::vector<double>& arr, double value);
    static std::vector<double> Compact(const std::vector<double>& arr);
    static std::vector<double> Zip(const std::vector<double>& a, const std::vector<double>& b);
};

#endif
