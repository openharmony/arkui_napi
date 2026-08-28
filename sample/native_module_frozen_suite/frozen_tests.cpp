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

#include "frozen_helper.h"

#include <cstdint>
#include <string>
#include <vector>

namespace {

using FrozenConst::K_ARG_COUNT_ONE;
using FrozenConst::K_ARG_COUNT_TWO;
using FrozenConst::K_SECOND_ELEMENT_INDEX;
using FrozenConst::K_CASES_PER_OPERATION;
using FrozenConst::K_FROZEN_CASE_COUNT;
using FrozenConst::K_FIRST_CASE_NUMBER;
using FrozenConst::K_INITIAL_VALUE_BASE;
using FrozenConst::K_INITIAL_VALUE_STEP;
using FrozenConst::K_INVALID_VALUE;
using FrozenConst::K_MUTATION_OFFSET;
using FrozenConst::K_OPERATION_COUNT;
using FrozenConst::K_PROPERTY_COUNT;
using FrozenConst::K_TEST_INT_VALUE_42;
using FrozenConst::K_TEST_INT_VALUE_100;
using FrozenConst::K_TEST_INT_VALUE_999;
using FrozenConst::K_TEST_DOUBLE_PI_APPROX;
using FrozenConst::K_EXTRA_EXPORT_COUNT;
using FrozenConst::K_TEST_INT64_VALUE_LARGE;

napi_value ExecuteBuildAndFreeze(napi_env env, const FrozenCaseSpec& spec)
{
    int32_t propValues[K_PROPERTY_COUNT] = {0};
    napi_value obj = CreateTestObjectWithProperties(env, spec.attrPattern, spec.initialValue, propValues);
    if (obj == nullptr) {
        napi_throw_error(env, nullptr, "failed to create test object");
        return nullptr;
    }
    bool freezeSuccess = FreezeObject(env, obj);
    if (!freezeSuccess) {
        napi_throw_error(env, nullptr, "failed to freeze object");
        return nullptr;
    }
    bool passed = true;
    DescriptorInfo descriptors[K_PROPERTY_COUNT] = {};
    for (size_t propIndex = 0; propIndex < K_PROPERTY_COUNT; propIndex++) {
        const auto meta = GetPropertyMeta(propIndex, spec.attrPattern);
        GetDescriptorInfo(env, obj, meta.name.c_str(), &descriptors[propIndex]);
        if (descriptors[propIndex].configurable) {
            passed = false;
        }
        if (descriptors[propIndex].writable) {
            passed = false;
        }
        if (descriptors[propIndex].value != propValues[propIndex]) {
            passed = false;
        }
    }
    return CreateDetailedResultObject(env, spec, passed, propValues, descriptors);
}

napi_value ExecuteBuildAndSeal(napi_env env, const FrozenCaseSpec& spec)
{
    int32_t propValues[K_PROPERTY_COUNT] = {0};
    napi_value obj = CreateTestObjectWithProperties(env, spec.attrPattern, spec.initialValue, propValues);
    if (obj == nullptr) {
        napi_throw_error(env, nullptr, "failed to create test object");
        return nullptr;
    }
    bool sealSuccess = SealObject(env, obj);
    if (!sealSuccess) {
        napi_throw_error(env, nullptr, "failed to seal object");
        return nullptr;
    }
    bool passed = true;
    DescriptorInfo descriptors[K_PROPERTY_COUNT] = {};
    for (size_t propIndex = 0; propIndex < K_PROPERTY_COUNT; propIndex++) {
        const auto meta = GetPropertyMeta(propIndex, spec.attrPattern);
        GetDescriptorInfo(env, obj, meta.name.c_str(), &descriptors[propIndex]);
        if (descriptors[propIndex].configurable) {
            passed = false;
        }
        if (descriptors[propIndex].value != propValues[propIndex]) {
            passed = false;
        }
    }
    return CreateDetailedResultObject(env, spec, passed, propValues, descriptors);
}

napi_value ExecuteMutateFrozen(napi_env env, const FrozenCaseSpec& spec)
{
    int32_t propValues[K_PROPERTY_COUNT] = {0};
    napi_value obj = CreateTestObjectWithProperties(env, spec.attrPattern, spec.initialValue, propValues);
    if (obj == nullptr) {
        napi_throw_error(env, nullptr, "failed to create test object");
        return nullptr;
    }
    bool freezeSuccess = FreezeObject(env, obj);
    if (!freezeSuccess) {
        napi_throw_error(env, nullptr, "failed to freeze object");
        return nullptr;
    }
    bool passed = true;
    int32_t mutatedValues[K_PROPERTY_COUNT] = {0};
    for (size_t propIndex = 0; propIndex < K_PROPERTY_COUNT; propIndex++) {
        const auto meta = GetPropertyMeta(propIndex, spec.attrPattern);
        const int32_t newValue = propValues[propIndex] + K_MUTATION_OFFSET;
        MutationResult mutResult = TryMutateProperty(env, obj, meta.name.c_str(), newValue);
        mutatedValues[propIndex] = mutResult.finalValue;
        if (mutResult.success) {
            passed = false;
        }
        if (mutResult.finalValue != propValues[propIndex]) {
            passed = false;
        }
        MutationResult delResult = TryDeleteProperty(env, obj, meta.name.c_str());
        if (delResult.success) {
            passed = false;
        }
    }
    return CreateDetailedResultObject(env, spec, passed, mutatedValues, nullptr);
}

napi_value ExecuteMutateSealed(napi_env env, const FrozenCaseSpec& spec)
{
    int32_t propValues[K_PROPERTY_COUNT] = {0};
    napi_value obj = CreateTestObjectWithProperties(env, spec.attrPattern, spec.initialValue, propValues);
    if (obj == nullptr) {
        napi_throw_error(env, nullptr, "failed to create test object");
        return nullptr;
    }
    bool sealSuccess = SealObject(env, obj);
    if (!sealSuccess) {
        napi_throw_error(env, nullptr, "failed to seal object");
        return nullptr;
    }
    bool passed = true;
    int32_t mutatedValues[K_PROPERTY_COUNT] = {0};
    DescriptorInfo descriptors[K_PROPERTY_COUNT] = {};
    for (size_t propIndex = 0; propIndex < K_PROPERTY_COUNT; propIndex++) {
        const auto meta = GetPropertyMeta(propIndex, spec.attrPattern);
        const int32_t newValue = propValues[propIndex] + K_MUTATION_OFFSET;
        MutationResult mutResult = TryMutateProperty(env, obj, meta.name.c_str(), newValue);
        mutatedValues[propIndex] = mutResult.finalValue;
        GetDescriptorInfo(env, obj, meta.name.c_str(), &descriptors[propIndex]);
        if (meta.writable) {
            if (mutResult.finalValue != newValue) {
                passed = false;
            }
        } else {
            if (mutResult.success) {
                passed = false;
            }
        }
        MutationResult delResult = TryDeleteProperty(env, obj, meta.name.c_str());
        if (delResult.success) {
            passed = false;
        }
    }
    return CreateDetailedResultObject(env, spec, passed, mutatedValues, descriptors);
}

napi_value ExecuteDescriptorCheck(napi_env env, const FrozenCaseSpec& spec)
{
    int32_t propValues[K_PROPERTY_COUNT] = {0};
    napi_value obj = CreateTestObjectWithProperties(env, spec.attrPattern, spec.initialValue, propValues);
    if (obj == nullptr) {
        napi_throw_error(env, nullptr, "failed to create test object");
        return nullptr;
    }
    bool passed = true;
    DescriptorInfo descriptors[K_PROPERTY_COUNT] = {};
    for (size_t propIndex = 0; propIndex < K_PROPERTY_COUNT; propIndex++) {
        const auto meta = GetPropertyMeta(propIndex, spec.attrPattern);
        GetDescriptorInfo(env, obj, meta.name.c_str(), &descriptors[propIndex]);
        if (!descriptors[propIndex].hasDescriptor) {
            passed = false;
            continue;
        }
        if (descriptors[propIndex].writable != meta.writable) {
            passed = false;
        }
        if (descriptors[propIndex].enumerable != meta.enumerable) {
            passed = false;
        }
        if (descriptors[propIndex].configurable != meta.configurable) {
            passed = false;
        }
        if (descriptors[propIndex].value != propValues[propIndex]) {
            passed = false;
        }
    }
    return CreateDetailedResultObject(env, spec, passed, propValues, descriptors);
}

napi_value RunFrozenCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_ARG_COUNT_TWO;
    napi_value args[K_ARG_COUNT_TWO] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_FROZEN_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid frozen case index");
        return nullptr;
    }
    const FrozenCaseSpec spec = GetFrozenCaseSpec(caseIndex);
    napi_value result = nullptr;
    switch (spec.operation) {
        case FrozenOp::BUILD_AND_FREEZE:
            result = ExecuteBuildAndFreeze(env, spec);
            break;
        case FrozenOp::BUILD_AND_SEAL:
            result = ExecuteBuildAndSeal(env, spec);
            break;
        case FrozenOp::MUTATE_FROZEN:
            result = ExecuteMutateFrozen(env, spec);
            break;
        case FrozenOp::MUTATE_SEALED:
            result = ExecuteMutateSealed(env, spec);
            break;
        case FrozenOp::DESCRIPTOR_CHECK:
            result = ExecuteDescriptorCheck(env, spec);
            break;
        default:
            napi_throw_error(env, nullptr, "unknown operation");
            return nullptr;
    }
    return result;
}

napi_value TestFreezeTwice(napi_env env, napi_callback_info info)
{
    size_t argc = K_ARG_COUNT_ONE;
    napi_value args[K_ARG_COUNT_ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc < K_ARG_COUNT_ONE) {
        napi_throw_type_error(env, nullptr, "object argument required");
        return nullptr;
    }
    napi_value obj = args[0];
    bool firstFreeze = FreezeObject(env, obj);
    bool secondFreeze = FreezeObject(env, obj);
    bool passed = firstFreeze && secondFreeze;
    return CreateResultObject(env, "freezeTwice", "freeze", passed, secondFreeze ? 1 : 0);
}

napi_value TestSealTwice(napi_env env, napi_callback_info info)
{
    size_t argc = K_ARG_COUNT_ONE;
    napi_value args[K_ARG_COUNT_ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc < K_ARG_COUNT_ONE) {
        napi_throw_type_error(env, nullptr, "object argument required");
        return nullptr;
    }
    napi_value obj = args[0];
    bool firstSeal = SealObject(env, obj);
    bool secondSeal = SealObject(env, obj);
    bool passed = firstSeal && secondSeal;
    return CreateResultObject(env, "sealTwice", "seal", passed, secondSeal ? 1 : 0);
}

napi_value TestFreezeThenSeal(napi_env env, napi_callback_info info)
{
    size_t argc = K_ARG_COUNT_ONE;
    napi_value args[K_ARG_COUNT_ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc < K_ARG_COUNT_ONE) {
        napi_throw_type_error(env, nullptr, "object argument required");
        return nullptr;
    }
    napi_value obj = args[0];
    bool freezeSuccess = FreezeObject(env, obj);
    bool sealSuccess = SealObject(env, obj);
    bool passed = freezeSuccess && sealSuccess;
    return CreateResultObject(env, "freezeThenSeal", "freezeThenSeal", passed, sealSuccess ? 1 : 0);
}

napi_value TestSealThenFreeze(napi_env env, napi_callback_info info)
{
    size_t argc = K_ARG_COUNT_ONE;
    napi_value args[K_ARG_COUNT_ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc < K_ARG_COUNT_ONE) {
        napi_throw_type_error(env, nullptr, "object argument required");
        return nullptr;
    }
    napi_value obj = args[0];
    bool sealSuccess = SealObject(env, obj);
    bool freezeSuccess = FreezeObject(env, obj);
    bool passed = sealSuccess && freezeSuccess;
    return CreateResultObject(env, "sealThenFreeze", "sealThenFreeze", passed, freezeSuccess ? 1 : 0);
}

napi_value TestReadFromFrozen(napi_env env, napi_callback_info info)
{
    size_t argc = K_ARG_COUNT_TWO;
    napi_value args[K_ARG_COUNT_TWO] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc < K_ARG_COUNT_TWO) {
        napi_throw_type_error(env, nullptr, "object and key required");
        return nullptr;
    }
    napi_value obj = args[0];
    std::string key;
    if (!ReadStringArg(env, args[K_SECOND_ELEMENT_INDEX], &key)) {
        return nullptr;
    }
    if (!FreezeObject(env, obj)) {
        napi_throw_error(env, nullptr, "failed to freeze object");
        return nullptr;
    }
    int32_t value = K_INVALID_VALUE;
    bool readSuccess = GetNamedInt32(env, obj, key.c_str(), &value);
    napi_value hasProp = nullptr;
    bool hasSuccess = napi_has_named_property(env, obj, key.c_str(), &hasProp) == napi_ok;
    bool passed = readSuccess && hasSuccess;
    return CreateResultObject(env, "readFromFrozen", "read", passed, value);
}

napi_value TestReadFromSealed(napi_env env, napi_callback_info info)
{
    size_t argc = K_ARG_COUNT_TWO;
    napi_value args[K_ARG_COUNT_TWO] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc < K_ARG_COUNT_TWO) {
        napi_throw_type_error(env, nullptr, "object and key required");
        return nullptr;
    }
    napi_value obj = args[0];
    std::string key;
    if (!ReadStringArg(env, args[K_SECOND_ELEMENT_INDEX], &key)) {
        return nullptr;
    }
    if (!SealObject(env, obj)) {
        napi_throw_error(env, nullptr, "failed to seal object");
        return nullptr;
    }
    int32_t value = K_INVALID_VALUE;
    bool readSuccess = GetNamedInt32(env, obj, key.c_str(), &value);
    napi_value hasProp = nullptr;
    bool hasSuccess = napi_has_named_property(env, obj, key.c_str(), &hasProp) == napi_ok;
    bool passed = readSuccess && hasSuccess;
    return CreateResultObject(env, "readFromSealed", "read", passed, value);
}

napi_value TestMixedTypesFrozen(napi_env env, napi_callback_info info)
{
    size_t argc = K_ARG_COUNT_ONE;
    napi_value args[K_ARG_COUNT_ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc < K_ARG_COUNT_ONE) {
        napi_throw_type_error(env, nullptr, "object required");
        return nullptr;
    }
    napi_value obj = args[0];
    SetNamedInt32(env, obj, "intVal", K_TEST_INT_VALUE_42);
    SetNamedDouble(env, obj, "doubleVal", K_TEST_DOUBLE_PI_APPROX);
    SetNamedString(env, obj, "stringVal", "hello");
    SetNamedBool(env, obj, "boolVal", true);
    SetNamedUint32(env, obj, "uintVal", K_TEST_INT_VALUE_100);
    SetNamedInt64(env, obj, "int64Val", K_TEST_INT64_VALUE_LARGE);
    if (!FreezeObject(env, obj)) {
        napi_throw_error(env, nullptr, "failed to freeze object");
        return nullptr;
    }
    bool passed = true;
    int32_t intVal = 0;
    if (!GetNamedInt32(env, obj, "intVal", &intVal) || intVal != K_TEST_INT_VALUE_42) {
        passed = false;
    }
    MutationResult mutResult = TryMutateProperty(env, obj, "intVal", K_TEST_INT_VALUE_100);
    if (mutResult.success || mutResult.finalValue != K_TEST_INT_VALUE_42) {
        passed = false;
    }
    MutationResult delResult = TryDeleteProperty(env, obj, "stringVal");
    if (delResult.success) {
        passed = false;
    }
    return CreateResultObject(env, "mixedTypesFrozen", "mixedTypes", passed, passed ? 1 : 0);
}

napi_value TestMixedTypesSealed(napi_env env, napi_callback_info info)
{
    size_t argc = K_ARG_COUNT_ONE;
    napi_value args[K_ARG_COUNT_ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc < K_ARG_COUNT_ONE) {
        napi_throw_type_error(env, nullptr, "object required");
        return nullptr;
    }
    napi_value obj = args[0];
    SetNamedInt32(env, obj, "intVal", K_TEST_INT_VALUE_42);
    SetNamedDouble(env, obj, "doubleVal", K_TEST_DOUBLE_PI_APPROX);
    SetNamedString(env, obj, "stringVal", "hello");
    SetNamedBool(env, obj, "boolVal", true);
    SetNamedUint32(env, obj, "uintVal", K_TEST_INT_VALUE_100);
    SetNamedInt64(env, obj, "int64Val", K_TEST_INT64_VALUE_LARGE);
    if (!SealObject(env, obj)) {
        napi_throw_error(env, nullptr, "failed to seal object");
        return nullptr;
    }
    bool passed = true;
    MutationResult mutResult = TryMutateProperty(env, obj, "intVal", K_TEST_INT_VALUE_100);
    if (!mutResult.success || mutResult.finalValue != K_TEST_INT_VALUE_100) {
        passed = false;
    }
    MutationResult delResult = TryDeleteProperty(env, obj, "stringVal");
    if (delResult.success) {
        passed = false;
    }
    DescriptorInfo desc = {};
    GetDescriptorInfo(env, obj, "intVal", &desc);
    if (desc.configurable) {
        passed = false;
    }
    if (!desc.writable) {
        passed = false;
    }
    return CreateResultObject(env, "mixedTypesSealed", "mixedTypes", passed, passed ? 1 : 0);
}

napi_value TestPropertyDescriptorAfterFreeze(napi_env env, napi_callback_info info)
{
    size_t argc = K_ARG_COUNT_TWO;
    napi_value args[K_ARG_COUNT_TWO] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc < K_ARG_COUNT_TWO) {
        napi_throw_type_error(env, nullptr, "object and key required");
        return nullptr;
    }
    napi_value obj = args[0];
    std::string key;
    if (!ReadStringArg(env, args[K_SECOND_ELEMENT_INDEX], &key)) {
        return nullptr;
    }
    if (!FreezeObject(env, obj)) {
        napi_throw_error(env, nullptr, "failed to freeze object");
        return nullptr;
    }
    DescriptorInfo desc = {};
    GetDescriptorInfo(env, obj, key.c_str(), &desc);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedBool(env, result, "hasDescriptor", desc.hasDescriptor);
    SetNamedBool(env, result, "writable", desc.writable);
    SetNamedBool(env, result, "enumerable", desc.enumerable);
    SetNamedBool(env, result, "configurable", desc.configurable);
    SetNamedInt32(env, result, "value", desc.value);
    bool passed = desc.hasDescriptor && !desc.writable && !desc.configurable;
    SetNamedBool(env, result, "passed", passed);
    return result;
}

napi_value TestPropertyDescriptorAfterSeal(napi_env env, napi_callback_info info)
{
    size_t argc = K_ARG_COUNT_TWO;
    napi_value args[K_ARG_COUNT_TWO] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc < K_ARG_COUNT_TWO) {
        napi_throw_type_error(env, nullptr, "object and key required");
        return nullptr;
    }
    napi_value obj = args[0];
    std::string key;
    if (!ReadStringArg(env, args[K_SECOND_ELEMENT_INDEX], &key)) {
        return nullptr;
    }
    if (!SealObject(env, obj)) {
        napi_throw_error(env, nullptr, "failed to seal object");
        return nullptr;
    }
    DescriptorInfo desc = {};
    GetDescriptorInfo(env, obj, key.c_str(), &desc);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedBool(env, result, "hasDescriptor", desc.hasDescriptor);
    SetNamedBool(env, result, "writable", desc.writable);
    SetNamedBool(env, result, "enumerable", desc.enumerable);
    SetNamedBool(env, result, "configurable", desc.configurable);
    SetNamedInt32(env, result, "value", desc.value);
    bool passed = desc.hasDescriptor && !desc.configurable;
    SetNamedBool(env, result, "passed", passed);
    return result;
}

napi_value TestFreezeAlreadySealed(napi_env env, napi_callback_info info)
{
    size_t argc = K_ARG_COUNT_ONE;
    napi_value args[K_ARG_COUNT_ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc < K_ARG_COUNT_ONE) {
        napi_throw_type_error(env, nullptr, "object required");
        return nullptr;
    }
    napi_value obj = args[0];
    bool sealSuccess = SealObject(env, obj);
    bool freezeSuccess = FreezeObject(env, obj);
    int32_t propValues[K_PROPERTY_COUNT] = {0};
    bool canWrite = true;
    for (size_t i = 0; i < K_PROPERTY_COUNT; i++) {
        const auto meta = GetPropertyMeta(i, 0);
        MutationResult mutResult = TryMutateProperty(env, obj, meta.name.c_str(), K_TEST_INT_VALUE_999);
        if (mutResult.success) {
            canWrite = false;
        }
    }
    bool passed = sealSuccess && freezeSuccess && !canWrite;
    return CreateResultObject(env, "freezeAlreadySealed", "freezeAfterSeal", passed, freezeSuccess ? 1 : 0);
}

napi_value TestSealAlreadyFrozen(napi_env env, napi_callback_info info)
{
    size_t argc = K_ARG_COUNT_ONE;
    napi_value args[K_ARG_COUNT_ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc < K_ARG_COUNT_ONE) {
        napi_throw_type_error(env, nullptr, "object required");
        return nullptr;
    }
    napi_value obj = args[0];
    bool freezeSuccess = FreezeObject(env, obj);
    bool sealSuccess = SealObject(env, obj);
    bool canDelete = true;
    for (size_t i = 0; i < K_PROPERTY_COUNT; i++) {
        const auto meta = GetPropertyMeta(i, 0);
        MutationResult delResult = TryDeleteProperty(env, obj, meta.name.c_str());
        if (delResult.success) {
            canDelete = false;
        }
    }
    bool passed = freezeSuccess && sealSuccess && !canDelete;
    return CreateResultObject(env, "sealAlreadyFrozen", "sealAfterFreeze", passed, sealSuccess ? 1 : 0);
}

napi_value TestArrayFreeze(napi_env env, napi_callback_info info)
{
    size_t argc = K_ARG_COUNT_ONE;
    napi_value args[K_ARG_COUNT_ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc < K_ARG_COUNT_ONE) {
        napi_throw_type_error(env, nullptr, "array required");
        return nullptr;
    }
    napi_value arr = args[0];
    bool isArray = false;
    napi_is_array(env, arr, &isArray);
    if (!isArray) {
        napi_throw_type_error(env, nullptr, "argument must be an array");
        return nullptr;
    }
    bool freezeSuccess = FreezeObject(env, arr);
    uint32_t length = 0;
    napi_get_array_length(env, arr, &length);
    bool passed = freezeSuccess;
    napi_value testVal = nullptr;
    napi_create_int32(env, K_TEST_INT_VALUE_999, &testVal);
    napi_status setStatus = napi_set_element(env, arr, 0, testVal);
    if (setStatus == napi_ok) {
        passed = false;
    }
    return CreateResultObject(env, "arrayFreeze", "freezeArray", passed, static_cast<int32_t>(length));
}

napi_value TestArraySeal(napi_env env, napi_callback_info info)
{
    size_t argc = K_ARG_COUNT_ONE;
    napi_value args[K_ARG_COUNT_ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc < K_ARG_COUNT_ONE) {
        napi_throw_type_error(env, nullptr, "array required");
        return nullptr;
    }
    napi_value arr = args[0];
    bool isArray = false;
    napi_is_array(env, arr, &isArray);
    if (!isArray) {
        napi_throw_type_error(env, nullptr, "argument must be an array");
        return nullptr;
    }
    bool sealSuccess = SealObject(env, arr);
    uint32_t length = 0;
    napi_get_array_length(env, arr, &length);
    bool passed = sealSuccess;
    napi_value testVal = nullptr;
    napi_create_int32(env, K_TEST_INT_VALUE_999, &testVal);
    napi_status setStatus = napi_set_element(env, arr, 0, testVal);
    if (setStatus != napi_ok) {
        passed = false;
    }
    return CreateResultObject(env, "arraySeal", "sealArray", passed, static_cast<int32_t>(length));
}

std::string BuildFrozenExportName(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    return BuildIndexedName("testFrozenCase", caseNumber);
}

void PopulateCaseDescriptors(std::vector<napi_property_descriptor>& descriptors,
    std::vector<std::string>& exportNames)
{
    for (size_t caseIndex = 0; caseIndex < K_FROZEN_CASE_COUNT; caseIndex++) {
        exportNames.emplace_back(BuildFrozenExportName(caseIndex));
        descriptors[caseIndex] = napi_property_descriptor{
            exportNames.back().c_str(),
            nullptr,
            RunFrozenCase,
            nullptr,
            nullptr,
            nullptr,
            napi_default,
            reinterpret_cast<void*>(static_cast<uintptr_t>(caseIndex)),
        };
    }
}

napi_value JsCreateTestObjectWithProperties(napi_env env, napi_callback_info info)
{
    size_t argc = K_ARG_COUNT_TWO;
    napi_value args[K_ARG_COUNT_TWO] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc < K_ARG_COUNT_TWO) {
        napi_throw_type_error(env, nullptr, "attrPattern and baseValue required");
        return nullptr;
    }
    int32_t attrPattern = 0;
    int32_t baseValue = 0;
    if (!ReadInt32Arg(env, args[0], &attrPattern) || !ReadInt32Arg(env, args[K_SECOND_ELEMENT_INDEX], &baseValue)) {
        return nullptr;
    }
    return CreateTestObjectWithProperties(env, attrPattern, baseValue, nullptr);
}

napi_value JsFreezeObject(napi_env env, napi_callback_info info)
{
    size_t argc = K_ARG_COUNT_ONE;
    napi_value args[K_ARG_COUNT_ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc < K_ARG_COUNT_ONE) {
        napi_throw_type_error(env, nullptr, "object required");
        return nullptr;
    }
    bool success = FreezeObject(env, args[0]);
    napi_value result = nullptr;
    napi_get_boolean(env, success, &result);
    return result;
}

napi_value JsSealObject(napi_env env, napi_callback_info info)
{
    size_t argc = K_ARG_COUNT_ONE;
    napi_value args[K_ARG_COUNT_ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc < K_ARG_COUNT_ONE) {
        napi_throw_type_error(env, nullptr, "object required");
        return nullptr;
    }
    bool success = SealObject(env, args[0]);
    napi_value result = nullptr;
    napi_get_boolean(env, success, &result);
    return result;
}

void PopulateExtraDescriptors(std::vector<napi_property_descriptor>& descriptors, size_t& index)
{
    descriptors[index++] = {"testFreezeTwice", nullptr, TestFreezeTwice,
        nullptr, nullptr, nullptr, napi_default, nullptr};
    descriptors[index++] = {"testSealTwice", nullptr, TestSealTwice,
        nullptr, nullptr, nullptr, napi_default, nullptr};
    descriptors[index++] = {"testFreezeThenSeal", nullptr, TestFreezeThenSeal,
        nullptr, nullptr, nullptr, napi_default, nullptr};
    descriptors[index++] = {"testSealThenFreeze", nullptr, TestSealThenFreeze,
        nullptr, nullptr, nullptr, napi_default, nullptr};
    descriptors[index++] = {"testReadFromFrozen", nullptr, TestReadFromFrozen,
        nullptr, nullptr, nullptr, napi_default, nullptr};
    descriptors[index++] = {"testReadFromSealed", nullptr, TestReadFromSealed,
        nullptr, nullptr, nullptr, napi_default, nullptr};
    descriptors[index++] = {"testMixedTypesFrozen", nullptr, TestMixedTypesFrozen,
        nullptr, nullptr, nullptr, napi_default, nullptr};
    descriptors[index++] = {"testMixedTypesSealed", nullptr, TestMixedTypesSealed,
        nullptr, nullptr, nullptr, napi_default, nullptr};
    descriptors[index++] = {"testPropertyDescriptorAfterFreeze", nullptr,
        TestPropertyDescriptorAfterFreeze, nullptr, nullptr, nullptr, napi_default, nullptr};
    descriptors[index++] = {"testPropertyDescriptorAfterSeal", nullptr,
        TestPropertyDescriptorAfterSeal, nullptr, nullptr, nullptr, napi_default, nullptr};
    descriptors[index++] = {"testFreezeAlreadySealed", nullptr,
        TestFreezeAlreadySealed, nullptr, nullptr, nullptr, napi_default, nullptr};
    descriptors[index++] = {"testSealAlreadyFrozen", nullptr,
        TestSealAlreadyFrozen, nullptr, nullptr, nullptr, napi_default, nullptr};
    descriptors[index++] = {"testArrayFreeze", nullptr, TestArrayFreeze,
        nullptr, nullptr, nullptr, napi_default, nullptr};
    descriptors[index++] = {"testArraySeal", nullptr, TestArraySeal,
        nullptr, nullptr, nullptr, napi_default, nullptr};
    descriptors[index++] = {"createTestObjectWithProperties", nullptr,
        JsCreateTestObjectWithProperties, nullptr, nullptr, nullptr, napi_default, nullptr};
    descriptors[index++] = {"freezeObject", nullptr, JsFreezeObject,
        nullptr, nullptr, nullptr, napi_default, nullptr};
    descriptors[index++] = {"sealObject", nullptr, JsSealObject,
        nullptr, nullptr, nullptr, napi_default, nullptr};
}

}  // namespace

static napi_value InitFrozenSuite(napi_env env, napi_value exports)
{
    std::vector<std::string> exportNames;
    std::vector<napi_property_descriptor> descriptors(K_FROZEN_CASE_COUNT + K_EXTRA_EXPORT_COUNT);
    exportNames.reserve(K_FROZEN_CASE_COUNT + K_EXTRA_EXPORT_COUNT);
    PopulateCaseDescriptors(descriptors, exportNames);
    size_t extraIndex = K_FROZEN_CASE_COUNT;
    PopulateExtraDescriptors(descriptors, extraIndex);
    NAPI_CALL(env, napi_define_properties(env, exports, extraIndex, descriptors.data()));
    return exports;
}

static napi_module g_frozenSuiteModule = {
    .nm_version = FrozenConst::K_MODULE_VERSION,
    .nm_flags = FrozenConst::K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitFrozenSuite,
    .nm_modname = "frozen_suite",
    .nm_priv = nullptr,
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterFrozenSuiteModule(void)
{
    napi_module_register(&g_frozenSuiteModule);
}