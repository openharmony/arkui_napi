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

#include "property_names_helper.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

constexpr size_t K_BASIC_ENUM_CASE_COUNT = 20;
constexpr size_t K_COLLECTION_MODE_CASE_COUNT = 14;
constexpr size_t K_KEY_FILTER_CASE_COUNT = 18;
constexpr size_t K_CONVERSION_CASE_COUNT = 12;
constexpr size_t K_ARRAY_ENUM_CASE_COUNT = 14;
constexpr size_t K_MIXED_KEY_CASE_COUNT = 18;
constexpr size_t K_MEMBERSHIP_CHECK_CASE_COUNT = 14;
constexpr size_t K_TOTAL_CASE_COUNT = K_BASIC_ENUM_CASE_COUNT + K_COLLECTION_MODE_CASE_COUNT +
                                       K_KEY_FILTER_CASE_COUNT + K_CONVERSION_CASE_COUNT +
                                       K_ARRAY_ENUM_CASE_COUNT + K_MIXED_KEY_CASE_COUNT +
                                       K_MEMBERSHIP_CHECK_CASE_COUNT;
constexpr size_t K_FIRST_CASE_NUMBER = 1;
constexpr int32_t K_BASE_VALUE = 100;
constexpr int32_t K_VALUE_INCREMENT = 7;
constexpr int32_t K_MAX_EXTRA_KEYS = 5;
constexpr size_t K_MIN_ARG_COUNT = 1;
constexpr size_t K_ARG_COUNT_TWO = 2;
constexpr int32_t K_EXPECTED_KEY_COUNT_BASE = 3;

constexpr std::array<napi_key_collection_mode, 2> K_COLLECTION_MODES = {
    napi_key_own_only,
    napi_key_include_prototypes,
};

constexpr std::array<napi_key_filter, 6> K_KEY_FILTERS = {
    napi_key_all_properties,
    napi_key_writable,
    napi_key_enumerable,
    napi_key_configurable,
    napi_key_skip_strings,
    napi_key_skip_symbols,
};

constexpr std::array<napi_key_conversion, 2> K_CONVERSION_MODES = {
    napi_key_keep_numbers,
    napi_key_numbers_to_strings,
};

constexpr int32_t K_ITERATION_STEP = 1;
constexpr int32_t K_VALUE_RANGE_MIN = 0;
constexpr int32_t K_VALUE_RANGE_MAX = 1000;
constexpr size_t K_MAX_ITERATIONS = 100;
constexpr int32_t K_ARRAY_SIZE_MIN = 1;
constexpr int32_t K_ARRAY_SIZE_MAX = 50;
constexpr int32_t K_STRING_KEY_MIN_COUNT = 1;
constexpr int32_t K_STRING_KEY_MAX_COUNT = 10;
constexpr int32_t K_NUMERIC_KEY_MIN_COUNT = 0;
constexpr int32_t K_NUMERIC_KEY_MAX_COUNT = 10;
constexpr size_t K_MAX_DESCRIPTOR_ARRAY_SIZE = 20;
constexpr int32_t K_DEFAULT_INT_VALUE = 0;
constexpr double K_DEFAULT_DOUBLE_VALUE = 0.0;
constexpr bool K_DEFAULT_BOOL_VALUE = false;
constexpr int32_t K_VALIDATION_THRESHOLD = 10;
constexpr size_t K_KEY_PREFIX_LENGTH = 4;
constexpr int32_t K_ENUMERATION_RETRY_LIMIT = 5;
constexpr int32_t K_MINIMUM_KEY_COUNT = 1;
constexpr int32_t K_MAXIMUM_KEY_COUNT = 20;

std::vector<int32_t> GenerateTestValues(int32_t count)
{
    std::vector<int32_t> values;
    values.reserve(count);
    for (int32_t i = 0; i < count; i++) {
        values.push_back(K_BASE_VALUE + i * K_VALUE_INCREMENT);
    }
    return values;
}

std::vector<std::string> GenerateTestKeys(int32_t count, const char* prefix)
{
    std::vector<std::string> keys;
    keys.reserve(count);
    for (int32_t i = 0; i < count; i++) {
        keys.push_back(std::string(prefix) + std::to_string(i));
    }
    return keys;
}

bool ValidateEnumerationResult(const EnumResult& result, uint32_t expectedCount)
{
    if (!result.passed) {
        return false;
    }
    if (result.keyCount != expectedCount) {
        return false;
    }
    if (result.foundKeys.size() != expectedCount) {
        return false;
    }
    return true;
}

bool CheckKeyInResults(const std::vector<std::string>& keys, const std::string& targetKey)
{
    for (const auto& key : keys) {
        if (key == targetKey) {
            return true;
        }
    }
    return false;
}

bool VerifyAllKeysPresent(const std::vector<std::string>& foundKeys, const std::vector<std::string>& expectedKeys)
{
    for (const auto& expectedKey : expectedKeys) {
        if (!CheckKeyInResults(foundKeys, expectedKey)) {
            return false;
        }
    }
    return true;
}

int32_t CalculateExpectedKeyCount(int32_t baseCount, int32_t extraCount)
{
    int32_t total = baseCount + extraCount;
    if (total < K_MINIMUM_KEY_COUNT) {
        return K_MINIMUM_KEY_COUNT;
    }
    if (total > K_MAXIMUM_KEY_COUNT) {
        return K_MAXIMUM_KEY_COUNT;
    }
    return total;
}

PropertyNamesCaseSpec BuildBasicEnumSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    const int32_t extraKeys = static_cast<int32_t>(caseIndex % K_MAX_EXTRA_KEYS);
    const int32_t keyCount = PropertyNamesConst::K_KEY_NAME_COUNT;
    std::vector<KeySpec> expectedKeys;
    expectedKeys.reserve(keyCount + extraKeys);
    for (size_t i = 0; i < keyCount; i++) {
        expectedKeys.push_back({K_KEY_NAMES[i], true, K_BASE_VALUE + static_cast<int32_t>(i)});
    }
    for (int32_t i = 0; i < extraKeys; i++) {
        expectedKeys.push_back({K_EXTRA_KEY_NAMES[i], true, K_BASE_VALUE + keyCount + i});
    }
    return {
        BuildIndexedName("basicEnum", caseNumber),
        napi_key_own_only,
        napi_key_all_properties,
        napi_key_keep_numbers,
        extraKeys,
        expectedKeys,
    };
}

PropertyNamesCaseSpec BuildCollectionModeSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    const size_t modeIndex = caseIndex % K_COLLECTION_MODES.size();
    const int32_t extraKeys = static_cast<int32_t>(caseIndex % K_MAX_EXTRA_KEYS);
    std::vector<KeySpec> expectedKeys;
    const int32_t baseKeyCount = PropertyNamesConst::K_KEY_NAME_COUNT;
    expectedKeys.reserve(baseKeyCount + extraKeys);
    for (size_t i = 0; i < baseKeyCount; i++) {
        expectedKeys.push_back({K_KEY_NAMES[i], true, K_BASE_VALUE + static_cast<int32_t>(i)});
    }
    for (int32_t i = 0; i < extraKeys; i++) {
        expectedKeys.push_back({K_EXTRA_KEY_NAMES[i], true, K_BASE_VALUE + baseKeyCount + i});
    }
    return {
        BuildIndexedName("collectionMode", caseNumber),
        K_COLLECTION_MODES[modeIndex],
        napi_key_all_properties,
        napi_key_keep_numbers,
        extraKeys,
        expectedKeys,
    };
}

PropertyNamesCaseSpec BuildKeyFilterSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    const size_t filterIndex = caseIndex % K_KEY_FILTERS.size();
    const int32_t extraKeys = static_cast<int32_t>(caseIndex % K_MAX_EXTRA_KEYS);
    std::vector<KeySpec> expectedKeys;
    const int32_t baseKeyCount = PropertyNamesConst::K_KEY_NAME_COUNT;
    expectedKeys.reserve(baseKeyCount + extraKeys);
    for (size_t i = 0; i < baseKeyCount; i++) {
        expectedKeys.push_back({K_KEY_NAMES[i], true, K_BASE_VALUE + static_cast<int32_t>(i)});
    }
    for (int32_t i = 0; i < extraKeys; i++) {
        expectedKeys.push_back({K_EXTRA_KEY_NAMES[i], true, K_BASE_VALUE + baseKeyCount + i});
    }
    return {
        BuildIndexedName("keyFilter", caseNumber),
        napi_key_own_only,
        K_KEY_FILTERS[filterIndex],
        napi_key_keep_numbers,
        extraKeys,
        expectedKeys,
    };
}

PropertyNamesCaseSpec BuildConversionSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    const size_t convIndex = caseIndex % K_CONVERSION_MODES.size();
    const int32_t extraKeys = static_cast<int32_t>(caseIndex % K_MAX_EXTRA_KEYS);
    std::vector<KeySpec> expectedKeys;
    const int32_t baseKeyCount = PropertyNamesConst::K_KEY_NAME_COUNT;
    expectedKeys.reserve(baseKeyCount + extraKeys);
    for (size_t i = 0; i < baseKeyCount; i++) {
        expectedKeys.push_back({K_KEY_NAMES[i], true, K_BASE_VALUE + static_cast<int32_t>(i)});
    }
    for (int32_t i = 0; i < extraKeys; i++) {
        expectedKeys.push_back({K_EXTRA_KEY_NAMES[i], true, K_BASE_VALUE + baseKeyCount + i});
    }
    return {
        BuildIndexedName("conversion", caseNumber),
        napi_key_own_only,
        napi_key_all_properties,
        K_CONVERSION_MODES[convIndex],
        extraKeys,
        expectedKeys,
    };
}

PropertyNamesCaseSpec BuildArrayEnumSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    const int32_t arraySize = static_cast<int32_t>(caseIndex + K_EXPECTED_KEY_COUNT_BASE);
    std::vector<KeySpec> expectedKeys;
    expectedKeys.reserve(arraySize);
    for (int32_t i = 0; i < arraySize; i++) {
        expectedKeys.push_back({std::to_string(i), true, i});
    }
    return {
        BuildIndexedName("arrayEnum", caseNumber),
        napi_key_own_only,
        napi_key_all_properties,
        napi_key_keep_numbers,
        arraySize,
        expectedKeys,
    };
}

PropertyNamesCaseSpec BuildMixedKeySpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    const int32_t stringKeyCount = static_cast<int32_t>((caseIndex % PropertyNamesConst::K_KEY_NAME_COUNT) + 1);
    const int32_t extraKeyCount = static_cast<int32_t>((caseIndex % K_MAX_EXTRA_KEYS) + 1);
    std::vector<KeySpec> expectedKeys;
    expectedKeys.reserve(stringKeyCount + extraKeyCount);
    for (int32_t i = 0; i < stringKeyCount; i++) {
        expectedKeys.push_back({K_KEY_NAMES[i], true, K_BASE_VALUE + i * K_VALUE_INCREMENT});
    }
    for (int32_t i = 0; i < extraKeyCount; i++) {
        expectedKeys.push_back({K_EXTRA_KEY_NAMES[i], true, K_BASE_VALUE + stringKeyCount + i});
    }
    return {
        BuildIndexedName("mixedKey", caseNumber),
        napi_key_own_only,
        napi_key_all_properties,
        napi_key_keep_numbers,
        extraKeyCount,
        expectedKeys,
    };
}

PropertyNamesCaseSpec BuildMembershipSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    const int32_t keyCount = static_cast<int32_t>((caseIndex % PropertyNamesConst::K_KEY_NAME_COUNT) + 1);
    std::vector<KeySpec> expectedKeys;
    expectedKeys.reserve(keyCount);
    for (int32_t i = 0; i < keyCount; i++) {
        expectedKeys.push_back({K_KEY_NAMES[i], true, K_BASE_VALUE + i});
    }
    return {
        BuildIndexedName("membership", caseNumber),
        napi_key_own_only,
        napi_key_all_properties,
        napi_key_keep_numbers,
        0,
        expectedKeys,
    };
}

napi_value RunBasicEnumCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_MIN_ARG_COUNT;
    napi_value args[PropertyNamesArgCount::ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_BASIC_ENUM_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid case index");
        return nullptr;
    }
    const auto spec = BuildBasicEnumSpec(caseIndex);
    std::vector<std::string> keys;
    std::vector<int32_t> values;
    keys.reserve(spec.expectedKeys.size());
    values.reserve(spec.expectedKeys.size());
    for (const auto& keySpec : spec.expectedKeys) {
        keys.push_back(keySpec.name);
        values.push_back(keySpec.expectedValue);
    }
    napi_value object = CreateTestObject(env, keys, values);
    if (object == nullptr) {
        napi_throw_error(env, nullptr, "failed to create test object");
        return nullptr;
    }
    napi_value names = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, object, &names));
    EnumResult enumResult = {false, 0, {}};
    if (!CollectArrayKeys(env, names, &enumResult.foundKeys)) {
        napi_throw_error(env, nullptr, "failed to collect keys");
        return nullptr;
    }
    enumResult.keyCount = static_cast<uint32_t>(enumResult.foundKeys.size());
    enumResult.passed = (enumResult.keyCount == spec.expectedKeys.size());
    return CreateResultObject(env, spec.name, enumResult.passed, enumResult.keyCount, enumResult.foundKeys);
}

napi_value RunCollectionModeCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_MIN_ARG_COUNT;
    napi_value args[PropertyNamesArgCount::ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_COLLECTION_MODE_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid case index");
        return nullptr;
    }
    const auto spec = BuildCollectionModeSpec(caseIndex);
    std::vector<std::string> keys;
    std::vector<int32_t> values;
    keys.reserve(spec.expectedKeys.size());
    values.reserve(spec.expectedKeys.size());
    for (const auto& keySpec : spec.expectedKeys) {
        keys.push_back(keySpec.name);
        values.push_back(keySpec.expectedValue);
    }
    napi_value object = CreateTestObject(env, keys, values);
    if (object == nullptr) {
        napi_throw_error(env, nullptr, "failed to create test object");
        return nullptr;
    }
    EnumResult enumResult = {false, 0, {}};
    EnumOptions options = {spec.collectionMode, spec.keyFilter, spec.keyConversion};
    if (!EnumeratePropertyNames(env, object, options, &enumResult)) {
        napi_throw_error(env, nullptr, "enumeration failed");
        return nullptr;
    }
    enumResult.passed = VerifyKeyPresence(env, object, spec.expectedKeys);
    return CreateResultObject(env, spec.name, enumResult.passed, enumResult.keyCount, enumResult.foundKeys);
}

napi_value RunKeyFilterCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_MIN_ARG_COUNT;
    napi_value args[PropertyNamesArgCount::ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_KEY_FILTER_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid case index");
        return nullptr;
    }
    const auto spec = BuildKeyFilterSpec(caseIndex);
    std::vector<std::string> keys;
    std::vector<int32_t> values;
    keys.reserve(spec.expectedKeys.size());
    values.reserve(spec.expectedKeys.size());
    for (const auto& keySpec : spec.expectedKeys) {
        keys.push_back(keySpec.name);
        values.push_back(keySpec.expectedValue);
    }
    napi_value object = CreateTestObject(env, keys, values);
    if (object == nullptr) {
        napi_throw_error(env, nullptr, "failed to create test object");
        return nullptr;
    }
    EnumResult enumResult = {false, 0, {}};
    EnumOptions options = {spec.collectionMode, spec.keyFilter, spec.keyConversion};
    if (!EnumeratePropertyNames(env, object, options, &enumResult)) {
        napi_throw_error(env, nullptr, "enumeration failed");
        return nullptr;
    }
    enumResult.passed = (enumResult.keyCount > 0);
    return CreateResultObject(env, spec.name, enumResult.passed, enumResult.keyCount, enumResult.foundKeys);
}

napi_value RunConversionCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_MIN_ARG_COUNT;
    napi_value args[PropertyNamesArgCount::ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_CONVERSION_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid case index");
        return nullptr;
    }
    const auto spec = BuildConversionSpec(caseIndex);
    std::vector<std::string> keys;
    std::vector<int32_t> values;
    keys.reserve(spec.expectedKeys.size());
    values.reserve(spec.expectedKeys.size());
    for (const auto& keySpec : spec.expectedKeys) {
        keys.push_back(keySpec.name);
        values.push_back(keySpec.expectedValue);
    }
    napi_value object = CreateTestObject(env, keys, values);
    if (object == nullptr) {
        napi_throw_error(env, nullptr, "failed to create test object");
        return nullptr;
    }
    EnumResult enumResult = {false, 0, {}};
    EnumOptions options = {spec.collectionMode, spec.keyFilter, spec.keyConversion};
    if (!EnumeratePropertyNames(env, object, options, &enumResult)) {
        napi_throw_error(env, nullptr, "enumeration failed");
        return nullptr;
    }
    enumResult.passed = (enumResult.keyCount > 0);
    return CreateResultObject(env, spec.name, enumResult.passed, enumResult.keyCount, enumResult.foundKeys);
}

napi_value RunArrayEnumCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_MIN_ARG_COUNT;
    napi_value args[PropertyNamesArgCount::ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_ARRAY_ENUM_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid case index");
        return nullptr;
    }
    const auto spec = BuildArrayEnumSpec(caseIndex);
    napi_value array = nullptr;
    const uint32_t arraySize = static_cast<uint32_t>(spec.extraKeyCount);
    NAPI_CALL(env, napi_create_array_with_length(env, arraySize, &array));
    for (uint32_t i = 0; i < arraySize; i++) {
        napi_value elem = nullptr;
        NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(i), &elem));
        NAPI_CALL(env, napi_set_element(env, array, i, elem));
    }
    EnumResult enumResult = {false, 0, {}};
    EnumOptions options = {spec.collectionMode, spec.keyFilter, spec.keyConversion};
    if (!EnumeratePropertyNames(env, array, options, &enumResult)) {
        napi_throw_error(env, nullptr, "enumeration failed");
        return nullptr;
    }
    enumResult.passed = (enumResult.keyCount >= arraySize);
    return CreateResultObject(env, spec.name, enumResult.passed, enumResult.keyCount, enumResult.foundKeys);
}

napi_value RunMixedKeyCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_MIN_ARG_COUNT;
    napi_value args[PropertyNamesArgCount::ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_MIXED_KEY_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid case index");
        return nullptr;
    }
    const auto spec = BuildMixedKeySpec(caseIndex);
    std::vector<std::string> keys;
    std::vector<int32_t> values;
    keys.reserve(spec.expectedKeys.size());
    values.reserve(spec.expectedKeys.size());
    for (const auto& keySpec : spec.expectedKeys) {
        keys.push_back(keySpec.name);
        values.push_back(keySpec.expectedValue);
    }
    napi_value object = CreateTestObject(env, keys, values);
    if (object == nullptr) {
        napi_throw_error(env, nullptr, "failed to create test object");
        return nullptr;
    }
    EnumResult enumResult = {false, 0, {}};
    EnumOptions options = {spec.collectionMode, spec.keyFilter, spec.keyConversion};
    if (!EnumeratePropertyNames(env, object, options, &enumResult)) {
        napi_throw_error(env, nullptr, "enumeration failed");
        return nullptr;
    }
    enumResult.passed = VerifyKeyPresence(env, object, spec.expectedKeys);
    return CreateResultObject(env, spec.name, enumResult.passed, enumResult.keyCount, enumResult.foundKeys);
}

napi_value RunMembershipCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_ARG_COUNT_TWO;
    napi_value args[PropertyNamesArgCount::TWO] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_MEMBERSHIP_CHECK_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid case index");
        return nullptr;
    }
    const auto spec = BuildMembershipSpec(caseIndex);
    std::vector<std::string> keys;
    std::vector<int32_t> values;
    keys.reserve(spec.expectedKeys.size());
    values.reserve(spec.expectedKeys.size());
    for (const auto& keySpec : spec.expectedKeys) {
        keys.push_back(keySpec.name);
        values.push_back(keySpec.expectedValue);
    }
    napi_value object = CreateTestObject(env, keys, values);
    if (object == nullptr) {
        napi_throw_error(env, nullptr, "failed to create test object");
        return nullptr;
    }
    EnumResult enumResult = {false, 0, {}};
    EnumOptions options = {spec.collectionMode, spec.keyFilter, spec.keyConversion};
    if (!EnumeratePropertyNames(env, object, options, &enumResult)) {
        napi_throw_error(env, nullptr, "enumeration failed");
        return nullptr;
    }
    bool allMatch = true;
    std::vector<std::string> verifiedKeys;
    for (const auto& foundKey : enumResult.foundKeys) {
        MembershipResult memberResult = {false, false};
        if (!CheckPropertyMembership(env, object, foundKey, &memberResult)) {
            allMatch = false;
            break;
        }
        if (memberResult.hasProperty) {
            verifiedKeys.push_back(foundKey);
        }
    }
    return CreateResultObject(env, spec.name, allMatch, enumResult.keyCount, verifiedKeys);
}

}  // namespace

static void AppendCaseDescriptors(std::vector<napi_property_descriptor>& descriptors,
    std::vector<std::string>& exportNames, const char* namePrefix, size_t caseCount,
    napi_callback callback)
{
    for (size_t i = 0; i < caseCount; i++) {
        exportNames.emplace_back(BuildIndexedName(namePrefix, i + K_FIRST_CASE_NUMBER));
        descriptors.push_back(napi_property_descriptor{
            exportNames.back().c_str(),
            nullptr,
            callback,
            nullptr,
            nullptr,
            nullptr,
            napi_default,
            reinterpret_cast<void*>(static_cast<uintptr_t>(i)),
        });
    }
}

static napi_value InitPropertyNamesSuite(napi_env env, napi_value exports)
{
    std::vector<std::string> exportNames;
    std::vector<napi_property_descriptor> descriptors;
    descriptors.reserve(K_TOTAL_CASE_COUNT);
    exportNames.reserve(K_TOTAL_CASE_COUNT);
    AppendCaseDescriptors(descriptors, exportNames, "testBasicEnum",
        K_BASIC_ENUM_CASE_COUNT, RunBasicEnumCase);
    AppendCaseDescriptors(descriptors, exportNames, "testCollectionMode",
        K_COLLECTION_MODE_CASE_COUNT, RunCollectionModeCase);
    AppendCaseDescriptors(descriptors, exportNames, "testKeyFilter",
        K_KEY_FILTER_CASE_COUNT, RunKeyFilterCase);
    AppendCaseDescriptors(descriptors, exportNames, "testConversion",
        K_CONVERSION_CASE_COUNT, RunConversionCase);
    AppendCaseDescriptors(descriptors, exportNames, "testArrayEnum",
        K_ARRAY_ENUM_CASE_COUNT, RunArrayEnumCase);
    AppendCaseDescriptors(descriptors, exportNames, "testMixedKey",
        K_MIXED_KEY_CASE_COUNT, RunMixedKeyCase);
    AppendCaseDescriptors(descriptors, exportNames, "testMembership",
        K_MEMBERSHIP_CHECK_CASE_COUNT, RunMembershipCase);
    NAPI_CALL(env, napi_define_properties(env, exports, descriptors.size(), descriptors.data()));
    return exports;
}

static napi_module g_propertyNamesModule = {
    .nm_version = PropertyNamesConst::K_MODULE_VERSION,
    .nm_flags = PropertyNamesConst::K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitPropertyNamesSuite,
    .nm_modname = "property_names_suite",
    .nm_priv = nullptr,
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterPropertyNamesSuiteModule(void)
{
    napi_module_register(&g_propertyNamesModule);
}