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

#include <array>

namespace {

using FrozenConst::K_BIT_MODULUS;
using FrozenConst::K_TWO_BIT_SHIFT;

constexpr std::array<const char*, FrozenConst::K_KEY_NAMES_COUNT> K_KEY_NAMES = {
    "alpha",
    "beta",
    "gamma",
    "delta",
    "epsilon",
};

constexpr std::array<const char*, FrozenConst::K_OPERATION_COUNT> K_OPERATION_LABELS = {
    "buildAndFreeze",
    "buildAndSeal",
    "mutateFrozen",
    "mutateSealed",
    "descriptorCheck",
};

}  // namespace

std::string BuildIndexedName(const char* prefix, size_t caseNumber)
{
    std::string suffix = std::to_string(caseNumber);
    if (suffix.size() < static_cast<size_t>(FrozenConst::K_CASE_NUMBER_WIDTH)) {
        suffix.insert(
            0, static_cast<std::string::size_type>(
                FrozenConst::K_CASE_NUMBER_WIDTH - suffix.size()),
            '0');
    }
    return std::string(prefix) + suffix;
}

size_t GetCaseIndex(void* data)
{
    return static_cast<size_t>(reinterpret_cast<uintptr_t>(data));
}

FrozenCaseSpec GetFrozenCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + FrozenConst::K_FIRST_CASE_NUMBER;
    const size_t opIndex = caseIndex / FrozenConst::K_CASES_PER_OPERATION;
    const int32_t attrPattern = static_cast<int32_t>(caseIndex % FrozenConst::K_PROPERTY_COUNT);
    const int32_t initialValue = FrozenConst::K_INITIAL_VALUE_BASE +
        static_cast<int32_t>(caseIndex % FrozenConst::K_VALUE_CYCLE) *
        FrozenConst::K_INITIAL_VALUE_STEP;
    return {
        BuildIndexedName("frozenCase", caseNumber),
        static_cast<int32_t>(opIndex),
        attrPattern,
        initialValue,
    };
}

PropertyMeta GetPropertyMeta(size_t propIndex, int32_t attrPattern)
{
    const size_t keyIndex = propIndex % K_KEY_NAMES.size();
    PropertyMeta meta;
    meta.name = K_KEY_NAMES[keyIndex];
    const int32_t pattern = attrPattern + static_cast<int32_t>(propIndex);
    const bool baseWritable = (pattern % K_BIT_MODULUS) == 0;
    const bool baseEnumerable = ((pattern / K_BIT_MODULUS) % K_BIT_MODULUS) == 0;
    const bool baseConfigurable = ((pattern / K_TWO_BIT_SHIFT) % K_BIT_MODULUS) == 0;
    meta.writable = baseWritable;
    meta.enumerable = baseEnumerable;
    meta.configurable = baseConfigurable;
    return meta;
}

uint32_t BuildPropertyAttribute(const PropertyMeta& meta)
{
    uint32_t attr = napi_default;
    if (!meta.writable) {
        attr |= napi_writable;
    }
    if (!meta.enumerable) {
        attr |= napi_enumerable;
    }
    if (!meta.configurable) {
        attr |= napi_configurable;
    }
    return attr;
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

bool SetNamedString(napi_env env, napi_value object, const char* name, const std::string& value)
{
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value.c_str(), value.size(), &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedDouble(napi_env env, napi_value object, const char* name, double value)
{
    napi_value napiValue = nullptr;
    if (napi_create_double(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedUint32(napi_env env, napi_value object, const char* name, uint32_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_uint32(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedInt64(napi_env env, napi_value object, const char* name, int64_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int64(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool GetNamedInt32(napi_env env, napi_value object, const char* name, int32_t* value)
{
    napi_value retrieved = nullptr;
    if (napi_get_named_property(env, object, name, &retrieved) != napi_ok) {
        return false;
    }
    return napi_get_value_int32(env, retrieved, value) == napi_ok;
}

bool GetNamedBool(napi_env env, napi_value object, const char* name, bool* value)
{
    napi_value retrieved = nullptr;
    if (napi_get_named_property(env, object, name, &retrieved) != napi_ok) {
        return false;
    }
    return napi_get_value_bool(env, retrieved, value) == napi_ok;
}

bool ReadInt32Arg(napi_env env, napi_value arg, int32_t* value)
{
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, arg, &type) != napi_ok) {
        return false;
    }
    if (type != napi_number) {
        napi_throw_type_error(env, nullptr, "argument must be a number");
        return false;
    }
    return napi_get_value_int32(env, arg, value) == napi_ok;
}

bool ReadStringArg(napi_env env, napi_value arg, std::string* value)
{
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, arg, &type) != napi_ok) {
        return false;
    }
    if (type != napi_string) {
        napi_throw_type_error(env, nullptr, "argument must be a string");
        return false;
    }
    size_t length = 0;
    if (napi_get_value_string_utf8(env, arg, nullptr, 0, &length) != napi_ok) {
        return false;
    }
    value->resize(length + FrozenConst::K_NULL_TERMINATOR_SIZE);
    size_t copied = 0;
    if (napi_get_value_string_utf8(env, arg, value->data(), value->size(), &copied) != napi_ok) {
        return false;
    }
    value->resize(copied);
    return true;
}

napi_value CreateTestObjectWithProperties(
    napi_env env, int32_t attrPattern, int32_t baseValue, int32_t* propValues)
{
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_property_descriptor descriptors[FrozenConst::K_PROPERTY_COUNT] = {};
    for (size_t propIndex = 0; propIndex < FrozenConst::K_PROPERTY_COUNT; propIndex++) {
        const auto meta = GetPropertyMeta(propIndex, attrPattern);
        napi_value propValue = nullptr;
        const int32_t numericValue = baseValue + static_cast<int32_t>(propIndex);
        if (propValues != nullptr) {
            propValues[propIndex] = numericValue;
        }
        NAPI_CALL(env, napi_create_int32(env, numericValue, &propValue));
        descriptors[propIndex] = napi_property_descriptor{
            meta.name.c_str(),
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            propValue,
            BuildPropertyAttribute(meta),
            nullptr,
        };
    }
    NAPI_CALL(env, napi_define_properties(env, obj, FrozenConst::K_PROPERTY_COUNT, descriptors));
    return obj;
}

napi_value GetDescriptorInfo(napi_env env, napi_value obj, const char* key, DescriptorInfo* info)
{
    napi_value keyStr = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, key, NAPI_AUTO_LENGTH, &keyStr));
    napi_value desc = nullptr;
    napi_status status = napi_get_own_property_descriptor(env, obj, keyStr, &desc);
    if (status != napi_ok) {
        info->hasDescriptor = false;
        info->writable = false;
        info->enumerable = false;
        info->configurable = false;
        info->value = FrozenConst::K_INVALID_VALUE;
        return nullptr;
    }
    info->hasDescriptor = true;
    napi_value writableVal = nullptr;
    bool hasWritable = false;
    if (napi_get_named_property(env, desc, "writable", &writableVal) == napi_ok) {
        napi_get_value_bool(env, writableVal, &info->writable);
    }
    napi_value enumerableVal = nullptr;
    if (napi_get_named_property(env, desc, "enumerable", &enumerableVal) == napi_ok) {
        napi_get_value_bool(env, enumerableVal, &info->enumerable);
    }
    napi_value configurableVal = nullptr;
    if (napi_get_named_property(env, desc, "configurable", &configurableVal) == napi_ok) {
        napi_get_value_bool(env, configurableVal, &info->configurable);
    }
    napi_value valueVal = nullptr;
    if (napi_get_named_property(env, desc, "value", &valueVal) == napi_ok) {
        napi_get_value_int32(env, valueVal, &info->value);
    } else {
        info->value = FrozenConst::K_INVALID_VALUE;
    }
    return desc;
}

napi_value CreateResultObject(
    napi_env env, const std::string& caseName, const char* operation, bool passed, int32_t value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "caseName", caseName);
    SetNamedString(env, result, "operation", operation);
    SetNamedBool(env, result, "passed", passed);
    SetNamedInt32(env, result, "value", value);
    return result;
}

napi_value CreateDetailedResultObject(
    napi_env env, const FrozenCaseSpec& spec, bool passed, const int32_t* propValues,
    const DescriptorInfo* descriptors)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "caseName", spec.name);
    const char* opLabel = K_OPERATION_LABELS[static_cast<size_t>(spec.operation)];
    SetNamedString(env, result, "operation", opLabel);
    SetNamedBool(env, result, "passed", passed);
    SetNamedInt32(env, result, "initialValue", spec.initialValue);
    SetNamedInt32(env, result, "attrPattern", spec.attrPattern);
    for (size_t propIndex = 0; propIndex < FrozenConst::K_PROPERTY_COUNT; propIndex++) {
        const auto meta = GetPropertyMeta(propIndex, spec.attrPattern);
        napi_value propResult = nullptr;
        NAPI_CALL(env, napi_create_object(env, &propResult));
        if (propValues != nullptr) {
            SetNamedInt32(env, propResult, "value", propValues[propIndex]);
        }
        if (descriptors != nullptr) {
            SetNamedBool(env, propResult, "writable", descriptors[propIndex].writable);
            SetNamedBool(env, propResult, "enumerable", descriptors[propIndex].enumerable);
            SetNamedBool(env, propResult, "configurable", descriptors[propIndex].configurable);
        } else {
            SetNamedBool(env, propResult, "expectedWritable", meta.writable);
            SetNamedBool(env, propResult, "expectedEnumerable", meta.enumerable);
            SetNamedBool(env, propResult, "expectedConfigurable", meta.configurable);
        }
        napi_set_named_property(env, result, meta.name.c_str(), propResult);
    }
    return result;
}

MutationResult TryMutateProperty(napi_env env, napi_value obj, const char* key, int32_t newValue)
{
    MutationResult result = {false, FrozenConst::K_INVALID_VALUE, false};
    napi_value keyStr = nullptr;
    if (napi_create_string_utf8(env, key, NAPI_AUTO_LENGTH, &keyStr) != napi_ok) {
        return result;
    }
    napi_value newValueNapi = nullptr;
    if (napi_create_int32(env, newValue, &newValueNapi) != napi_ok) {
        return result;
    }
    napi_status status = napi_set_property(env, obj, keyStr, newValueNapi);
    result.success = (status == napi_ok);
    bool isException = false;
    napi_is_exception_pending(env, &isException);
    result.exceptionRaised = isException;
    if (isException) {
        napi_value exception = nullptr;
        napi_get_and_clear_last_exception(env, &exception);
    }
    napi_value readBack = nullptr;
    if (napi_get_named_property(env, obj, key, &readBack) == napi_ok) {
        napi_get_value_int32(env, readBack, &result.finalValue);
    }
    return result;
}

MutationResult TryDeleteProperty(napi_env env, napi_value obj, const char* key)
{
    MutationResult result = {false, FrozenConst::K_INVALID_VALUE, false};
    napi_value keyStr = nullptr;
    if (napi_create_string_utf8(env, key, NAPI_AUTO_LENGTH, &keyStr) != napi_ok) {
        return result;
    }
    napi_value deleteResult = nullptr;
    napi_status status = napi_delete_property(env, obj, keyStr, &deleteResult);
    result.success = (status == napi_ok);
    bool isException = false;
    napi_is_exception_pending(env, &isException);
    result.exceptionRaised = isException;
    if (isException) {
        napi_value exception = nullptr;
        napi_get_and_clear_last_exception(env, &exception);
    }
    bool deleted = false;
    if (napi_get_value_bool(env, deleteResult, &deleted) == napi_ok) {
        result.finalValue = deleted ? 1 : 0;
    }
    return result;
}

bool FreezeObject(napi_env env, napi_value obj)
{
    return napi_object_freeze(env, obj) == napi_ok;
}

bool SealObject(napi_env env, napi_value obj)
{
    return napi_object_seal(env, obj) == napi_ok;
}

bool IsObjectFrozen(napi_env env, napi_value obj)
{
    bool frozen = false;
    napi_status status = napi_is_exception_pending(env, &frozen);
    (void)status;
    napi_value keyStr = nullptr;
    if (napi_create_string_utf8(env, "test_frozen_marker", NAPI_AUTO_LENGTH, &keyStr) != napi_ok) {
        return false;
    }
    napi_value testValue = nullptr;
    if (napi_create_int32(env, FrozenConst::K_MUTATION_OFFSET, &testValue) != napi_ok) {
        return false;
    }
    napi_status setStatus = napi_set_property(env, obj, keyStr, testValue);
    return (setStatus != napi_ok);
}

bool IsObjectSealed(napi_env env, napi_value obj)
{
    napi_value keyStr = nullptr;
    if (napi_create_string_utf8(env, "test_sealed_marker", NAPI_AUTO_LENGTH, &keyStr) != napi_ok) {
        return false;
    }
    napi_value deleteResult = nullptr;
    napi_status deleteStatus = napi_delete_property(env, obj, keyStr, &deleteResult);
    return (deleteStatus != napi_ok);
}