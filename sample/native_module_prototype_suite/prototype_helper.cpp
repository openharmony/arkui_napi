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

#include "prototype_helper.h"

namespace {

napi_value TestConstructorCallback(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    size_t argc = PrototypeConst::K_ARG_COUNT_ONE;
    napi_value argv[PrototypeConst::K_ARG_COUNT_ONE] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr));
    if (argc > 0) {
        napi_set_named_property(env, thisArg, "_value", argv[0]);
    } else {
        napi_value defaultValue = nullptr;
        napi_create_int32(env, 0, &defaultValue);
        napi_set_named_property(env, thisArg, "_value", defaultValue);
    }
    return thisArg;
}

napi_value GetValueMethod(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr));
    napi_value value = nullptr;
    napi_get_named_property(env, thisArg, "_value", &value);
    return value;
}

}  // namespace

std::string BuildIndexedName(const char* prefix, size_t caseNumber)
{
    std::string suffix = std::to_string(caseNumber);
    if (suffix.size() < static_cast<size_t>(PrototypeConst::K_CASE_NUMBER_WIDTH)) {
        suffix.insert(
            0, static_cast<std::string::size_type>(
                PrototypeConst::K_CASE_NUMBER_WIDTH - suffix.size()),
            '0');
    }
    return std::string(prefix) + suffix;
}

size_t GetCaseIndex(void* data)
{
    return static_cast<size_t>(reinterpret_cast<uintptr_t>(data));
}

int32_t GetTypeCode(napi_valuetype type)
{
    switch (type) {
        case napi_number:
            return PrototypeConst::K_TYPEOF_NUMBER;
        case napi_string:
            return PrototypeConst::K_TYPEOF_STRING;
        case napi_boolean:
            return PrototypeConst::K_TYPEOF_BOOLEAN;
        case napi_object:
            return PrototypeConst::K_TYPEOF_OBJECT;
        case napi_function:
            return PrototypeConst::K_TYPEOF_FUNCTION;
        case napi_undefined:
            return PrototypeConst::K_TYPEOF_UNDEFINED;
        case napi_null:
            return PrototypeConst::K_TYPEOF_NULL;
        default:
            return PrototypeConst::K_TYPEOF_UNKNOWN;
    }
}

PrototypeCaseSpec GetPrototypeCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + PrototypeConst::K_FIRST_CASE_NUMBER;
    const int32_t factorBase = static_cast<int32_t>(caseNumber % PrototypeConst::K_INSTANCE_VALUE_CYCLE);
    return {
        BuildIndexedName("prototypeCase", caseNumber),
        factorBase * PrototypeConst::K_INSTANCE_VALUE_STEP + PrototypeConst::K_INSTANCE_VALUE_BASE,
        static_cast<int32_t>((caseNumber % PrototypeConst::K_DEPTH_LIMIT_CYCLE) + PrototypeConst::K_DEPTH_LIMIT_OFFSET),
        static_cast<int32_t>(caseNumber % PrototypeConst::K_TEST_CLASS_COUNT),
        (caseNumber % PrototypeConst::K_CHAIN_WALKING_CYCLE) == 0,
        (caseNumber % PrototypeConst::K_INSTANCEOF_CYCLE) == PrototypeConst::K_INSTANCEOF_OFFSET,
        (caseNumber % PrototypeConst::K_TYPEOF_CYCLE) == PrototypeConst::K_TYPEOF_OFFSET,
        (caseNumber % PrototypeConst::K_STRICT_EQUALS_CYCLE) == 0,
    };
}

std::string BuildPrototypeExportName(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + PrototypeConst::K_FIRST_CASE_NUMBER;
    return BuildIndexedName("testPrototypeCase", caseNumber);
}

bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int32(env, value, &napiValue) != napi_ok) {
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

bool FetchValueType(napi_env env, napi_value value, napi_valuetype* type)
{
    return napi_typeof(env, value, type) == napi_ok;
}

const char* ValuetypeToLabel(napi_valuetype type)
{
    switch (type) {
        case napi_number:
            return "number";
        case napi_string:
            return "string";
        case napi_boolean:
            return "boolean";
        case napi_object:
            return "object";
        case napi_function:
            return "function";
        case napi_undefined:
            return "undefined";
        case napi_null:
            return "null";
        default:
            return "unknown";
    }
}

bool CheckStrictEquality(napi_env env, napi_value a, napi_value b, bool* result)
{
    return napi_strict_equals(env, a, b, result) == napi_ok;
}

bool GetPrototypeSafely(napi_env env, napi_value object, napi_value* prototype)
{
    napi_status status = napi_get_prototype(env, object, prototype);
    return status == napi_ok;
}

bool WalkPrototypeChain(napi_env env, napi_value start, PrototypeChainResult* result)
{
    result->depth = 0;
    result->chainChecksum = 0;
    result->reachedNull = false;
    result->exceededDepth = false;
    result->walkSuccessful = false;
    result->typeofCodes.clear();
    result->typeLabels.clear();

    napi_value current = start;
    for (size_t step = 0; step < PrototypeConst::K_PROTOTYPE_CHAIN_MAX_DEPTH; step++) {
        napi_valuetype currentType = napi_undefined;
        if (!FetchValueType(env, current, &currentType)) {
            return false;
        }
        result->typeofCodes.push_back(GetTypeCode(currentType));
        result->typeLabels.push_back(ValuetypeToLabel(currentType));
        result->chainChecksum += GetTypeCode(currentType) * PrototypeConst::K_CHECKSUM_MULTIPLIER;
        result->depth = static_cast<int32_t>(step + 1);

        napi_value proto = nullptr;
        if (!GetPrototypeSafely(env, current, &proto)) {
            return false;
        }
        bool isNull = false;
        napi_value nullVal = nullptr;
        napi_get_null(env, &nullVal);
        if (CheckStrictEquality(env, proto, nullVal, &isNull) && isNull) {
            result->reachedNull = true;
            break;
        }
        current = proto;
        if (step == PrototypeConst::K_PROTOTYPE_CHAIN_MAX_DEPTH - 1) {
            result->exceededDepth = true;
        }
    }
    result->walkSuccessful = true;
    return true;
}

bool CheckInstanceof(napi_env env, napi_value object, napi_value constructor, bool* result)
{
    return napi_instanceof(env, object, constructor, result) == napi_ok;
}

bool CreateTestClass(napi_env env, const char* className, napi_value* constructor)
{
    napi_property_descriptor props[] = {
        {
            "getValue",
            nullptr,
            GetValueMethod,
            nullptr,
            nullptr,
            nullptr,
            napi_default,
            nullptr,
        },
    };
    napi_status status = napi_define_class(
        env,
        className,
        NAPI_AUTO_LENGTH,
        TestConstructorCallback,
        nullptr,
        sizeof(props) / sizeof(props[0]),
        props,
        constructor);
    return status == napi_ok;
}

bool CreateTestInstance(napi_env env, napi_value constructor, int32_t value, napi_value* instance)
{
    napi_value arg = nullptr;
    napi_create_int32(env, value, &arg);
    napi_status status = napi_new_instance(env, constructor, PrototypeConst::K_ARG_COUNT_ONE, &arg, instance);
    return status == napi_ok;
}

bool GetFunctionPrototypeProperty(napi_env env, napi_value func, napi_value* prototypeProp)
{
    bool hasPrototype = false;
    if (napi_has_named_property(env, func, "prototype", &hasPrototype) != napi_ok) {
        return false;
    }
    if (!hasPrototype) {
        return false;
    }
    return napi_get_named_property(env, func, "prototype", prototypeProp) == napi_ok;
}

bool ComparePrototypeOfInstances(
    napi_env env, napi_value constructor, napi_value instanceA, napi_value instanceB, bool* equal)
{
    napi_value protoA = nullptr;
    napi_value protoB = nullptr;
    if (!GetPrototypeSafely(env, instanceA, &protoA)) {
        return false;
    }
    if (!GetPrototypeSafely(env, instanceB, &protoB)) {
        return false;
    }
    return CheckStrictEquality(env, protoA, protoB, equal);
}
