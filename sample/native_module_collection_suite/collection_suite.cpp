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

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

constexpr size_t K_COLLECTION_CASE_COUNT = 15;
constexpr size_t K_REQUIRED_ARG_COUNT = 1;
constexpr size_t K_FIRST_CASE_NUMBER = 1;
constexpr int K_CASE_NUMBER_WIDTH = 2;
constexpr uint32_t K_MODULE_VERSION = 1;
constexpr uint32_t K_NO_MODULE_FLAGS = 0;
constexpr size_t K_NULL_TERMINATOR_SIZE = 1;
constexpr int32_t K_INVALID_VALUE = -1;
constexpr int32_t K_BASE_SCORE = 10;
constexpr int32_t K_BONUS_THRESHOLD = 50;
constexpr int32_t K_BONUS_AMOUNT = 15;
constexpr int32_t K_PENALTY_RATE = 2;
constexpr int32_t K_MAX_ITEM_COUNT = 10;
constexpr size_t K_OPERATION_COUNT = 7;
constexpr int32_t K_REMOVE_ITEM_COUNT = 3;
constexpr int32_t K_GET_SIZE_EXPECTED = 3;
constexpr int32_t K_BETA_VALUE = 2;
constexpr int32_t K_GAMMA_VALUE = 3;

constexpr std::array<const char*, K_OPERATION_COUNT> K_OPERATION_LABELS = {
    "addItem",
    "removeItem",
    "containsKey",
    "getSize",
    "clearAll",
    "toArray",
    "merge",
};

enum class CollectionOp : int32_t {
    ADD_ITEM = 0,
    REMOVE_ITEM = 1,
    CONTAINS_KEY = 2,
    GET_SIZE = 3,
    CLEAR_ALL = 4,
    TO_ARRAY = 5,
    MERGE = 6,
};

struct CollectionCaseSpec {
    std::string name;
    CollectionOp operation;
    int32_t baseScore;
    int32_t weight;
};

struct OperationResult {
    bool passed;
    int32_t value;
};

std::string BuildIndexedName(const char* prefix, size_t caseNumber)
{
    std::string suffix = std::to_string(caseNumber);
    if (suffix.size() < static_cast<size_t>(K_CASE_NUMBER_WIDTH)) {
        suffix.insert(0, static_cast<std::string::size_type>(
            K_CASE_NUMBER_WIDTH - suffix.size()), '0');
    }
    return std::string(prefix) + suffix;
}

size_t GetCaseIndex(void* data)
{
    return static_cast<size_t>(reinterpret_cast<uintptr_t>(data));
}

CollectionCaseSpec GetCollectionCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    const int32_t opIndex = static_cast<int32_t>(caseIndex % K_OPERATION_COUNT);
    const int32_t baseScore = static_cast<int32_t>(caseNumber) * K_BASE_SCORE;
    const int32_t weight = static_cast<int32_t>(caseNumber % K_MAX_ITEM_COUNT) + 1;
    return {
        BuildIndexedName("collectionCase", caseNumber),
        static_cast<CollectionOp>(opIndex),
        baseScore,
        weight,
    };
}

bool ReadString(napi_env env, napi_value value, const char* message,
    std::string* result)
{
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, value, &type) != napi_ok) {
        return false;
    }
    if (type != napi_string) {
        napi_throw_type_error(env, nullptr, message);
        return false;
    }
    size_t length = 0;
    if (napi_get_value_string_utf8(env, value, nullptr, 0, &length) != napi_ok) {
        return false;
    }
    std::string buffer(length + K_NULL_TERMINATOR_SIZE, '\0');
    if (napi_get_value_string_utf8(env, value, buffer.data(),
        buffer.size(), &length) != napi_ok) {
        return false;
    }
    buffer.resize(length);
    *result = buffer;
    return true;
}

bool ReadInt32(napi_env env, napi_value value, const char* message,
    int32_t* result)
{
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, value, &type) != napi_ok) {
        return false;
    }
    if (type != napi_number) {
        napi_throw_type_error(env, nullptr, message);
        return false;
    }
    return napi_get_value_int32(env, value, result) == napi_ok;
}

bool SetNamedInt32(napi_env env, napi_value object, const char* name,
    int32_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int32(env, value, &napiValue) != napi_ok) {
        return false;
    }
    if (napi_set_named_property(env, object, name, napiValue) != napi_ok) {
        return false;
    }
    napi_value retrieved = nullptr;
    if (napi_get_named_property(env, object, name, &retrieved) != napi_ok) {
        return false;
    }
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, retrieved, &type) != napi_ok || type != napi_number) {
        return false;
    }
    int32_t readBack = K_INVALID_VALUE;
    if (napi_get_value_int32(env, retrieved, &readBack) != napi_ok ||
        readBack != value) {
        return false;
    }
    return true;
}

bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value)
{
    napi_value napiValue = nullptr;
    if (napi_get_boolean(env, value, &napiValue) != napi_ok) {
        return false;
    }
    if (napi_set_named_property(env, object, name, napiValue) != napi_ok) {
        return false;
    }
    napi_value retrieved = nullptr;
    if (napi_get_named_property(env, object, name, &retrieved) != napi_ok) {
        return false;
    }
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, retrieved, &type) != napi_ok || type != napi_boolean) {
        return false;
    }
    bool readBack = false;
    if (napi_get_value_bool(env, retrieved, &readBack) != napi_ok ||
        readBack != value) {
        return false;
    }
    return true;
}

bool SetNamedString(napi_env env, napi_value object, const char* name,
    const std::string& value)
{
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value.c_str(), value.size(),
        &napiValue) != napi_ok) {
        return false;
    }
    if (napi_set_named_property(env, object, name, napiValue) != napi_ok) {
        return false;
    }
    napi_value retrieved = nullptr;
    if (napi_get_named_property(env, object, name, &retrieved) != napi_ok) {
        return false;
    }
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, retrieved, &type) != napi_ok || type != napi_string) {
        return false;
    }
    size_t readLen = 0;
    if (napi_get_value_string_utf8(env, retrieved, nullptr, 0, &readLen) !=
        napi_ok || readLen != value.size()) {
        return false;
    }
    return true;
}

napi_value CreateEmptyArray(napi_env env)
{
    napi_value result = nullptr;
    if (napi_create_array(env, &result) != napi_ok) {
        return nullptr;
    }
    return result;
}

napi_value CreateCollectionSummary(napi_env env, const std::string& name,
    const char* operation, int32_t value, bool passed)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    if (!SetNamedString(env, result, "name", name)) {
        napi_throw_error(env, nullptr, "failed to set name property");
        return nullptr;
    }
    if (!SetNamedString(env, result, "operation", std::string(operation))) {
        napi_throw_error(env, nullptr, "failed to set operation property");
        return nullptr;
    }
    if (!SetNamedInt32(env, result, "value", value)) {
        napi_throw_error(env, nullptr, "failed to set value property");
        return nullptr;
    }
    if (!SetNamedBool(env, result, "passed", passed)) {
        napi_throw_error(env, nullptr, "failed to set passed property");
        return nullptr;
    }
    return result;
}

napi_value CreateTestCollection(napi_env env)
{
    napi_value obj = nullptr;
    if (napi_create_object(env, &obj) != napi_ok) {
        return nullptr;
    }
    return obj;
}

bool SetCollectionInt32(napi_env env, napi_value obj, const char* key,
    int32_t val)
{
    napi_value napiVal = nullptr;
    if (napi_create_int32(env, val, &napiVal) != napi_ok) {
        return false;
    }
    if (napi_set_named_property(env, obj, key, napiVal) != napi_ok) {
        return false;
    }
    napi_value retrieved = nullptr;
    if (napi_get_named_property(env, obj, key, &retrieved) != napi_ok) {
        return false;
    }
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, retrieved, &type) != napi_ok || type != napi_number) {
        return false;
    }
    int32_t readBack = K_INVALID_VALUE;
    if (napi_get_value_int32(env, retrieved, &readBack) != napi_ok ||
        readBack != val) {
        return false;
    }
    return true;
}

int32_t CalculateScore(int32_t base, int32_t weight, int32_t itemCount)
{
    int32_t score = base + weight * itemCount;
    if (score >= K_BONUS_THRESHOLD) {
        score += K_BONUS_AMOUNT;
    }
    if (itemCount > K_MAX_ITEM_COUNT / K_PENALTY_RATE) {
        score -= itemCount / K_PENALTY_RATE;
    }
    return score;
}

OperationResult ExecuteAddItem(napi_env env, int32_t baseScore, int32_t weight)
{
    napi_value collection = CreateTestCollection(env);
    if (collection == nullptr) {
        return {false, K_INVALID_VALUE};
    }
    if (!SetCollectionInt32(env, collection, "itemCount", 1)) {
        return {false, K_INVALID_VALUE};
    }
    int32_t expectedScore = CalculateScore(baseScore, weight, 1);
    if (!SetCollectionInt32(env, collection, "score", expectedScore)) {
        return {false, K_INVALID_VALUE};
    }
    int32_t actual = K_INVALID_VALUE;
    napi_value retrieved = nullptr;
    if (napi_get_named_property(env, collection, "score", &retrieved) !=
        napi_ok) {
        return {false, K_INVALID_VALUE};
    }
    if (napi_get_value_int32(env, retrieved, &actual) != napi_ok) {
        return {false, K_INVALID_VALUE};
    }
    bool passed = (actual == expectedScore);
    return {passed, actual};
}

OperationResult ExecuteRemoveItem(napi_env env, int32_t baseScore,
    int32_t weight)
{
    (void)baseScore;
    (void)weight;
    napi_value collection = CreateTestCollection(env);
    if (collection == nullptr) {
        return {false, K_INVALID_VALUE};
    }
    if (!SetCollectionInt32(env, collection, "itemCount", K_REMOVE_ITEM_COUNT)) {
        return {false, K_INVALID_VALUE};
    }
    if (!SetNamedBool(env, collection, "removed", true)) {
        return {false, K_INVALID_VALUE};
    }
    int32_t itemCount = K_INVALID_VALUE;
    napi_value retrieved = nullptr;
    if (napi_get_named_property(env, collection, "itemCount", &retrieved) !=
        napi_ok) {
        return {false, K_INVALID_VALUE};
    }
    if (napi_get_value_int32(env, retrieved, &itemCount) != napi_ok) {
        return {false, K_INVALID_VALUE};
    }
    int32_t remaining = itemCount - 1;
    bool passed = (remaining >= 0);
    return {passed, remaining};
}

OperationResult ExecuteContainsKey(napi_env env, int32_t baseScore,
    int32_t weight)
{
    (void)weight;
    napi_value collection = CreateTestCollection(env);
    if (collection == nullptr) {
        return {false, K_INVALID_VALUE};
    }
    if (!SetCollectionInt32(env, collection, "key", baseScore)) {
        return {false, K_INVALID_VALUE};
    }
    bool hasKey = false;
    if (napi_has_named_property(env, collection, "key", &hasKey) != napi_ok) {
        return {false, K_INVALID_VALUE};
    }
    int32_t value = hasKey ? baseScore : K_INVALID_VALUE;
    return {hasKey, value};
}

OperationResult ExecuteGetSize(napi_env env, int32_t baseScore, int32_t weight)
{
    (void)baseScore;
    (void)weight;
    napi_value collection = CreateTestCollection(env);
    if (collection == nullptr) {
        return {false, K_INVALID_VALUE};
    }
    if (!SetCollectionInt32(env, collection, "alpha", 1)) {
        return {false, K_INVALID_VALUE};
    }
    if (!SetCollectionInt32(env, collection, "beta", K_BETA_VALUE)) {
        return {false, K_INVALID_VALUE};
    }
    if (!SetCollectionInt32(env, collection, "gamma", K_GAMMA_VALUE)) {
        return {false, K_INVALID_VALUE};
    }
    napi_value names = nullptr;
    if (napi_get_property_names(env, collection, &names) != napi_ok) {
        return {false, K_INVALID_VALUE};
    }
    uint32_t count = 0;
    if (napi_get_array_length(env, names, &count) != napi_ok) {
        return {false, K_INVALID_VALUE};
    }
    int32_t expected = K_GET_SIZE_EXPECTED;
    bool passed = (static_cast<int32_t>(count) == expected);
    return {passed, static_cast<int32_t>(count)};
}

OperationResult ExecuteClearAll(napi_env env, int32_t baseScore, int32_t weight)
{
    (void)baseScore;
    (void)weight;
    napi_value collection = CreateTestCollection(env);
    if (collection == nullptr) {
        return {false, K_INVALID_VALUE};
    }
    if (!SetCollectionInt32(env, collection, "temp", 1)) {
        return {false, K_INVALID_VALUE};
    }
    napi_value newCollection = CreateTestCollection(env);
    if (newCollection == nullptr) {
        return {false, K_INVALID_VALUE};
    }
    napi_value names = nullptr;
    if (napi_get_property_names(env, newCollection, &names) != napi_ok) {
        return {false, K_INVALID_VALUE};
    }
    uint32_t count = 0;
    if (napi_get_array_length(env, names, &count) != napi_ok) {
        return {false, K_INVALID_VALUE};
    }
    bool isEmpty = (count == 0);
    return {isEmpty, static_cast<int32_t>(count)};
}

OperationResult ExecuteToArray(napi_env env, int32_t baseScore, int32_t weight)
{
    (void)baseScore;
    (void)weight;
    napi_value array = CreateEmptyArray(env);
    if (array == nullptr) {
        return {false, K_INVALID_VALUE};
    }
    uint32_t length = 0;
    if (napi_get_array_length(env, array, &length) != napi_ok) {
        return {false, K_INVALID_VALUE};
    }
    int32_t expected = 0;
    int32_t actual = static_cast<int32_t>(length);
    bool passed = (actual == expected);
    return {passed, actual};
}

OperationResult ExecuteMerge(napi_env env, int32_t baseScore, int32_t weight)
{
    napi_value merged = CreateTestCollection(env);
    if (merged == nullptr) {
        return {false, K_INVALID_VALUE};
    }
    if (!SetCollectionInt32(env, merged, "sourceA", baseScore)) {
        return {false, K_INVALID_VALUE};
    }
    if (!SetCollectionInt32(env, merged, "sourceB", weight)) {
        return {false, K_INVALID_VALUE};
    }
    int32_t totalScore = baseScore + weight;
    if (!SetCollectionInt32(env, merged, "total", totalScore)) {
        return {false, K_INVALID_VALUE};
    }
    int32_t actual = K_INVALID_VALUE;
    napi_value retrieved = nullptr;
    if (napi_get_named_property(env, merged, "total", &retrieved) != napi_ok) {
        return {false, K_INVALID_VALUE};
    }
    if (napi_get_value_int32(env, retrieved, &actual) != napi_ok) {
        return {false, K_INVALID_VALUE};
    }
    bool passed = (actual == totalScore);
    return {passed, actual};
}

OperationResult ExecuteOperation(napi_env env, const CollectionCaseSpec& spec)
{
    switch (spec.operation) {
        case CollectionOp::ADD_ITEM:
            return ExecuteAddItem(env, spec.baseScore, spec.weight);
        case CollectionOp::REMOVE_ITEM:
            return ExecuteRemoveItem(env, spec.baseScore, spec.weight);
        case CollectionOp::CONTAINS_KEY:
            return ExecuteContainsKey(env, spec.baseScore, spec.weight);
        case CollectionOp::GET_SIZE:
            return ExecuteGetSize(env, spec.baseScore, spec.weight);
        case CollectionOp::CLEAR_ALL:
            return ExecuteClearAll(env, spec.baseScore, spec.weight);
        case CollectionOp::TO_ARRAY:
            return ExecuteToArray(env, spec.baseScore, spec.weight);
        case CollectionOp::MERGE:
            return ExecuteMerge(env, spec.baseScore, spec.weight);
        default:
            return {false, K_INVALID_VALUE};
    }
}

static napi_value RunCollectionCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_REQUIRED_ARG_COUNT;
    napi_value args[K_REQUIRED_ARG_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));

    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_COLLECTION_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid collection case");
        return nullptr;
    }

    if (argc < K_REQUIRED_ARG_COUNT) {
        napi_throw_type_error(env, nullptr, "input object is required");
        return nullptr;
    }

    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, args[0], &type) != napi_ok) {
        return nullptr;
    }
    if (type != napi_object && type != napi_bigint) {
        napi_throw_type_error(env, nullptr, "input must be an object");
        return nullptr;
    }

    const auto spec = GetCollectionCaseSpec(caseIndex);
    const auto opResult = ExecuteOperation(env, spec);
    const char* opLabel = K_OPERATION_LABELS[static_cast<size_t>(
        spec.operation)];

    return CreateCollectionSummary(env, spec.name, opLabel, opResult.value,
        opResult.passed);
}
}  // namespace

static napi_value InitCollectionSuite(napi_env env, napi_value exports)
{
    std::vector<std::string> exportNames;
    std::vector<napi_property_descriptor> descriptors(K_COLLECTION_CASE_COUNT);
    exportNames.reserve(K_COLLECTION_CASE_COUNT);

    for (size_t caseIndex = 0; caseIndex < K_COLLECTION_CASE_COUNT;
        caseIndex++) {
        exportNames.emplace_back(BuildIndexedName("testCollectionCase",
            caseIndex + K_FIRST_CASE_NUMBER));
        descriptors[caseIndex] = napi_property_descriptor{
            exportNames.back().c_str(),
            nullptr,
            RunCollectionCase,
            nullptr,
            nullptr,
            nullptr,
            napi_default,
            reinterpret_cast<void*>(static_cast<uintptr_t>(caseIndex))
        };
    }

    NAPI_CALL(env, napi_define_properties(env, exports, descriptors.size(),
        descriptors.data()));
    return exports;
}

static napi_module g_collectionSuiteModule = {
    .nm_version = K_MODULE_VERSION,
    .nm_flags = K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitCollectionSuite,
    .nm_modname = "collection_suite",
    .nm_priv = nullptr,
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterCollectionSuiteModule(void)
{
    napi_module_register(&g_collectionSuiteModule);
}
