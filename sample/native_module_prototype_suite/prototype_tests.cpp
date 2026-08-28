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
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "prototype_helper.h"

namespace {

static napi_value TestPlainObjectPrototype(napi_env env, napi_callback_info info)
{
    napi_value plainObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &plainObject));

    napi_value prototype = nullptr;
    bool gotPrototype = GetPrototypeSafely(env, plainObject, &prototype);

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedBool(env, result, "gotPrototype", gotPrototype);
    SetNamedString(env, result, "name", "plainObjectPrototype");

    if (gotPrototype && prototype != nullptr) {
        napi_valuetype protoType = napi_undefined;
        FetchValueType(env, prototype, &protoType);
        SetNamedString(env, result, "prototypeType", ValuetypeToLabel(protoType));

        PrototypeChainResult chainResult;
        if (WalkPrototypeChain(env, plainObject, &chainResult)) {
            SetNamedInt32(env, result, "chainDepth", chainResult.depth);
            SetNamedInt32(env, result, "chainChecksum", chainResult.chainChecksum);
            SetNamedBool(env, result, "reachedNull", chainResult.reachedNull);
            SetNamedBool(env, result, "exceededDepth", chainResult.exceededDepth);
        }
    }
    return result;
}

static napi_value TestArrayPrototype(napi_env env, napi_callback_info info)
{
    napi_value array = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, PrototypeConst::K_ARRAY_SIZE, &array));

    for (uint32_t i = 0; i < static_cast<uint32_t>(PrototypeConst::K_ARRAY_SIZE); i++) {
        napi_value elem = nullptr;
        napi_create_int32(env, static_cast<int32_t>(i * PrototypeConst::K_ARRAY_ELEMENT_STEP), &elem);
        napi_set_element(env, array, i, elem);
    }

    napi_value prototype = nullptr;
    bool gotPrototype = GetPrototypeSafely(env, array, &prototype);

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedBool(env, result, "gotPrototype", gotPrototype);
    SetNamedString(env, result, "name", "arrayPrototype");

    if (gotPrototype && prototype != nullptr) {
        napi_valuetype protoType = napi_undefined;
        FetchValueType(env, prototype, &protoType);
        SetNamedString(env, result, "prototypeType", ValuetypeToLabel(protoType));

        bool isArray = false;
        napi_is_array(env, prototype, &isArray);
        SetNamedBool(env, result, "prototypeIsArray", isArray);
    }
    return result;
}

static napi_value TestFunctionPrototype(napi_env env, napi_callback_info info)
{
    auto callback = [](napi_env cbEnv, napi_callback_info cbInfo) -> napi_value {
        size_t argc = PrototypeConst::K_ARG_COUNT_ONE;
        napi_value arg = nullptr;
        napi_get_cb_info(cbEnv, cbInfo, &argc, &arg, nullptr, nullptr);
        return arg;
    };

    napi_value func = nullptr;
    napi_status status = napi_create_function(
        env, "testFunc", NAPI_AUTO_LENGTH, callback, nullptr, &func);
    bool createdFunc = (status == napi_ok);

    napi_value prototype = nullptr;
    bool gotPrototype = false;
    if (createdFunc) {
        gotPrototype = GetPrototypeSafely(env, func, &prototype);
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedBool(env, result, "createdFunc", createdFunc);
    SetNamedBool(env, result, "gotPrototype", gotPrototype);
    SetNamedString(env, result, "name", "functionPrototype");

    if (gotPrototype && prototype != nullptr) {
        napi_valuetype protoType = napi_undefined;
        FetchValueType(env, prototype, &protoType);
        SetNamedString(env, result, "prototypeType", ValuetypeToLabel(protoType));
    }

    napi_value funcPrototypeProp = nullptr;
    bool hasPrototypeProp = GetFunctionPrototypeProperty(env, func, &funcPrototypeProp);
    SetNamedBool(env, result, "hasPrototypeProperty", hasPrototypeProp);
    return result;
}

static napi_value TestInstancePrototype(napi_env env, napi_callback_info info)
{
    const char* className = "TestClass";
    napi_value constructor = nullptr;
    bool createdClass = CreateTestClass(env, className, &constructor);

    napi_value instance = nullptr;
    bool createdInstance = false;
    const int32_t instanceValue = PrototypeConst::K_DEFAULT_INSTANCE_VALUE;
    if (createdClass) {
        createdInstance = CreateTestInstance(env, constructor, instanceValue, &instance);
    }

    napi_value prototype = nullptr;
    bool gotPrototype = false;
    if (createdInstance) {
        gotPrototype = GetPrototypeSafely(env, instance, &prototype);
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedBool(env, result, "createdClass", createdClass);
    SetNamedBool(env, result, "createdInstance", createdInstance);
    SetNamedBool(env, result, "gotPrototype", gotPrototype);
    SetNamedString(env, result, "name", "instancePrototype");
    SetNamedInt32(env, result, "instanceValue", instanceValue);

    if (gotPrototype && prototype != nullptr) {
        napi_valuetype protoType = napi_undefined;
        FetchValueType(env, prototype, &protoType);
        SetNamedString(env, result, "prototypeType", ValuetypeToLabel(protoType));
    }
    return result;
}

static napi_value TestPrototypeChainWalking(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = PrototypeConst::K_ARG_COUNT_ONE;
    napi_value arg = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &arg, nullptr, &data));

    const size_t caseIndex = GetCaseIndex(data);
    const auto spec = GetPrototypeCaseSpec(caseIndex);

    napi_value startObject = arg;
    if (argc < PrototypeConst::K_ARG_COUNT_ONE || startObject == nullptr) {
        NAPI_CALL(env, napi_create_object(env, &startObject));
    }

    PrototypeChainResult chainResult;
    bool walked = WalkPrototypeChain(env, startObject, &chainResult);

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", spec.name);
    SetNamedBool(env, result, "walked", walked);
    SetNamedInt32(env, result, "depth", chainResult.depth);
    SetNamedInt32(env, result, "chainChecksum", chainResult.chainChecksum);
    SetNamedBool(env, result, "reachedNull", chainResult.reachedNull);
    SetNamedBool(env, result, "exceededDepth", chainResult.exceededDepth);
    return result;
}

static napi_value TestInstanceOfPositive(napi_env env, napi_callback_info info)
{
    const char* className = "PositiveTestClass";
    napi_value constructor = nullptr;
    bool createdClass = CreateTestClass(env, className, &constructor);

    napi_value instance = nullptr;
    bool createdInstance = false;
    if (createdClass) {
        createdInstance = CreateTestInstance(env, constructor, PrototypeConst::K_INSTANCE_VALUE_BASE, &instance);
    }

    bool instanceofResult = false;
    if (createdInstance) {
        CheckInstanceof(env, instance, constructor, &instanceofResult);
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedBool(env, result, "createdClass", createdClass);
    SetNamedBool(env, result, "createdInstance", createdInstance);
    SetNamedBool(env, result, "instanceof", instanceofResult);
    SetNamedString(env, result, "name", "instanceofPositive");
    return result;
}

static napi_value TestInstanceOfNegativeUnrelated(napi_env env, napi_callback_info info)
{
    napi_value constructorA = nullptr;
    napi_value constructorB = nullptr;
    bool createdA = CreateTestClass(env, "ClassA", &constructorA);
    bool createdB = CreateTestClass(env, "ClassB", &constructorB);

    napi_value instanceA = nullptr;
    bool createdInstance = false;
    if (createdA) {
        createdInstance = CreateTestInstance(env, constructorA, PrototypeConst::K_ALT_INSTANCE_VALUE_TWO, &instanceA);
    }

    bool instanceofResult = false;
    bool instanceofOfB = false;
    if (createdInstance && createdB) {
        CheckInstanceof(env, instanceA, constructorA, &instanceofResult);
        CheckInstanceof(env, instanceA, constructorB, &instanceofOfB);
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedBool(env, result, "createdClassA", createdA);
    SetNamedBool(env, result, "createdClassB", createdB);
    SetNamedBool(env, result, "instanceofOwnClass", instanceofResult);
    SetNamedBool(env, result, "instanceofUnrelatedClass", instanceofOfB);
    SetNamedString(env, result, "name", "instanceofNegativeUnrelated");
    return result;
}

static napi_value TestInstanceOfNegativePlain(napi_env env, napi_callback_info info)
{
    napi_value constructor = nullptr;
    bool createdClass = CreateTestClass(env, "PlainTestClass", &constructor);

    napi_value plainObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &plainObject));
    napi_value value = nullptr;
    napi_create_int32(env, PrototypeConst::K_PLAIN_OBJECT_VALUE, &value);
    napi_set_named_property(env, plainObject, "value", value);

    bool instanceofResult = false;
    if (createdClass) {
        CheckInstanceof(env, plainObject, constructor, &instanceofResult);
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedBool(env, result, "createdClass", createdClass);
    SetNamedBool(env, result, "instanceof", instanceofResult);
    SetNamedString(env, result, "name", "instanceofNegativePlain");
    return result;
}

static napi_value TestSameConstructorPrototypes(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    const auto spec = GetPrototypeCaseSpec(caseIndex);

    napi_value constructor = nullptr;
    bool createdClass = CreateTestClass(env, "CompareTestClass", &constructor);

    napi_value instanceA = nullptr;
    napi_value instanceB = nullptr;
    bool createdA = false;
    bool createdB = false;
    if (createdClass) {
        createdA = CreateTestInstance(env, constructor, spec.valueFactor, &instanceA);
        createdB = CreateTestInstance(env, constructor,
            spec.valueFactor + PrototypeConst::K_INSTANCE_VALUE_OFFSET, &instanceB);
    }

    bool prototypesEqual = false;
    if (createdA && createdB) {
        ComparePrototypeOfInstances(env, constructor, instanceA, instanceB, &prototypesEqual);
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", spec.name);
    SetNamedBool(env, result, "createdClass", createdClass);
    SetNamedBool(env, result, "createdInstanceA", createdA);
    SetNamedBool(env, result, "createdInstanceB", createdB);
    SetNamedBool(env, result, "prototypesEqual", prototypesEqual);
    SetNamedInt32(env, result, "valueA", spec.valueFactor);
    SetNamedInt32(env, result, "valueB", spec.valueFactor + PrototypeConst::K_INSTANCE_VALUE_OFFSET);
    return result;
}

static napi_value TestFunctionPrototypeProperty(napi_env env, napi_callback_info info)
{
    napi_value constructor = nullptr;
    bool createdClass = CreateTestClass(env, "PrototypePropertyClass", &constructor);

    napi_value instance = nullptr;
    bool createdInstance = false;
    if (createdClass) {
        createdInstance = CreateTestInstance(env, constructor, PrototypeConst::K_ALT_INSTANCE_VALUE, &instance);
    }

    napi_value funcPrototypeProp = nullptr;
    bool hasPrototypeProp = false;
    if (createdClass) {
        hasPrototypeProp = GetFunctionPrototypeProperty(env, constructor, &funcPrototypeProp);
    }

    napi_value instancePrototype = nullptr;
    bool gotInstanceProto = false;
    if (createdInstance) {
        gotInstanceProto = GetPrototypeSafely(env, instance, &instancePrototype);
    }

    bool prototypeEquals = false;
    if (hasPrototypeProp && gotInstanceProto) {
        CheckStrictEquality(env, funcPrototypeProp, instancePrototype, &prototypeEquals);
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedBool(env, result, "createdClass", createdClass);
    SetNamedBool(env, result, "createdInstance", createdInstance);
    SetNamedBool(env, result, "hasPrototypeProperty", hasPrototypeProp);
    SetNamedBool(env, result, "gotInstancePrototype", gotInstanceProto);
    SetNamedBool(env, result, "prototypeEquals", prototypeEquals);
    SetNamedString(env, result, "name", "functionPrototypeProperty");
    return result;
}

static napi_value TestTypeofAlongChain(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    const auto spec = GetPrototypeCaseSpec(caseIndex);

    napi_value constructor = nullptr;
    bool createdClass = CreateTestClass(env, "TypeofTestClass", &constructor);

    napi_value instance = nullptr;
    bool createdInstance = false;
    if (createdClass) {
        createdInstance = CreateTestInstance(env, constructor, spec.valueFactor, &instance);
    }

    PrototypeChainResult chainResult = {};
    bool walkSuccessful = false;
    if (createdInstance) {
        walkSuccessful = WalkPrototypeChain(env, instance, &chainResult);
    }

    napi_value typeArray = nullptr;
    NAPI_CALL(env, napi_create_array(env, &typeArray));
    const std::vector<std::string>& typeLabels = chainResult.typeLabels;
    for (size_t i = 0; i < typeLabels.size(); i++) {
        napi_value label = nullptr;
        napi_create_string_utf8(env, typeLabels[i].c_str(), typeLabels[i].size(), &label);
        napi_set_element(env, typeArray, static_cast<uint32_t>(i), label);
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", spec.name);
    SetNamedBool(env, result, "createdClass", createdClass);
    SetNamedBool(env, result, "createdInstance", createdInstance);
    SetNamedBool(env, result, "walkSuccessful", walkSuccessful);
    napi_set_named_property(env, result, "typeChain", typeArray);
    SetNamedInt32(env, result, "chainLength", static_cast<int32_t>(typeLabels.size()));
    return result;
}

static napi_value TestChainReachedNull(napi_env env, napi_callback_info info)
{
    napi_value plainObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &plainObject));

    PrototypeChainResult chainResult = {};
    const bool walkSuccessful = WalkPrototypeChain(env, plainObject, &chainResult);

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", "chainReachedNull");
    SetNamedBool(env, result, "walkSuccessful", walkSuccessful);
    SetNamedBool(env, result, "reachedNull", chainResult.reachedNull);
    SetNamedBool(env, result, "exceededDepth", chainResult.exceededDepth);
    SetNamedInt32(env, result, "depth", chainResult.depth);
    return result;
}

static napi_value TestChainDepthWithinBound(napi_env env, napi_callback_info info)
{
    napi_value constructor = nullptr;
    bool createdClass = CreateTestClass(env, "DepthBoundClass", &constructor);

    napi_value instance = nullptr;
    bool createdInstance = false;
    if (createdClass) {
        createdInstance = CreateTestInstance(env, constructor, PrototypeConst::K_INSTANCE_VALUE_BASE, &instance);
    }

    PrototypeChainResult chainResult = {};
    bool walkSuccessful = false;
    if (createdInstance) {
        walkSuccessful = WalkPrototypeChain(env, instance, &chainResult);
    }
    const bool depthWithinBound =
        chainResult.depth > 0 && chainResult.depth <= PrototypeConst::K_PROTOTYPE_CHAIN_MAX_DEPTH;

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", "chainDepthWithinBound");
    SetNamedBool(env, result, "createdInstance", createdInstance);
    SetNamedBool(env, result, "walkSuccessful", walkSuccessful);
    SetNamedBool(env, result, "depthWithinBound", depthWithinBound);
    SetNamedInt32(env, result, "depth", chainResult.depth);
    return result;
}

static napi_value TestChainChecksumConsistency(napi_env env, napi_callback_info info)
{
    napi_value constructor = nullptr;
    bool createdClass = CreateTestClass(env, "ChecksumClass", &constructor);

    PrototypeChainResult firstResult = {};
    PrototypeChainResult secondResult = {};
    bool createdFirst = false;
    bool createdSecond = false;
    if (createdClass) {
        napi_value firstInstance = nullptr;
        napi_value secondInstance = nullptr;
        createdFirst = CreateTestInstance(
            env, constructor, PrototypeConst::K_INSTANCE_VALUE_BASE, &firstInstance);
        createdSecond = CreateTestInstance(
            env, constructor,
            PrototypeConst::K_INSTANCE_VALUE_BASE + PrototypeConst::K_INSTANCE_VALUE_BASE,
            &secondInstance);
        if (createdFirst) {
            WalkPrototypeChain(env, firstInstance, &firstResult);
        }
        if (createdSecond) {
            WalkPrototypeChain(env, secondInstance, &secondResult);
        }
    }
    const bool checksumsConsistent =
        createdFirst && createdSecond && firstResult.chainChecksum == secondResult.chainChecksum;

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", "chainChecksumConsistency");
    SetNamedBool(env, result, "createdClass", createdClass);
    SetNamedBool(env, result, "createdFirst", createdFirst);
    SetNamedBool(env, result, "createdSecond", createdSecond);
    SetNamedBool(env, result, "checksumsConsistent", checksumsConsistent);
    SetNamedInt32(env, result, "firstDepth", firstResult.depth);
    SetNamedInt32(env, result, "secondDepth", secondResult.depth);
    return result;
}

static napi_value TestPrototypeStrictEquals(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data));
    const size_t caseIndex = GetCaseIndex(data);
    const auto spec = GetPrototypeCaseSpec(caseIndex);

    napi_value objA = nullptr;
    napi_value objB = nullptr;
    NAPI_CALL(env, napi_create_object(env, &objA));
    NAPI_CALL(env, napi_create_object(env, &objB));
    napi_set_named_property(env, objA, "id",
        ([](napi_env e, int32_t v) -> napi_value {
            napi_value val = nullptr;
            napi_create_int32(e, v, &val);
            return val;
        })(env, spec.valueFactor));
    napi_set_named_property(env, objB, "id",
        ([](napi_env e, int32_t v) -> napi_value {
            napi_value val = nullptr;
            napi_create_int32(e, v + 1, &val);
            return val;
        })(env, spec.valueFactor));

    napi_value protoA = nullptr;
    napi_value protoB = nullptr;
    bool gotA = GetPrototypeSafely(env, objA, &protoA);
    bool gotB = GetPrototypeSafely(env, objB, &protoB);

    bool prototypesEqual = false;
    if (gotA && gotB) {
        CheckStrictEquality(env, protoA, protoB, &prototypesEqual);
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", spec.name);
    SetNamedBool(env, result, "gotPrototypeA", gotA);
    SetNamedBool(env, result, "gotPrototypeB", gotB);
    SetNamedBool(env, result, "prototypesEqual", prototypesEqual);
    return result;
}

static napi_value TestUnrelatedConstructors(napi_env env, napi_callback_info info)
{
    napi_value constructorA = nullptr;
    napi_value constructorB = nullptr;
    bool createdClassA = CreateTestClass(env, "UnrelatedClassA", &constructorA);
    bool createdClassB = CreateTestClass(env, "UnrelatedClassB", &constructorB);

    napi_value instanceA = nullptr;
    napi_value instanceB = nullptr;
    bool createdInstanceA = false;
    bool createdInstanceB = false;
    if (createdClassA) {
        createdInstanceA = CreateTestInstance(env, constructorA,
            PrototypeConst::K_DEFAULT_INSTANCE_VALUE, &instanceA);
    }
    if (createdClassB) {
        createdInstanceB = CreateTestInstance(env, constructorB,
            PrototypeConst::K_ALT_INSTANCE_VALUE, &instanceB);
    }

    bool instanceOfOwn = false;
    bool instanceOfOther = false;
    if (createdInstanceA && createdInstanceB) {
        CheckInstanceof(env, instanceA, constructorA, &instanceOfOwn);
        CheckInstanceof(env, instanceA, constructorB, &instanceOfOther);
    }

    bool prototypesCompared = false;
    bool prototypesDistinct = false;
    if (createdInstanceA && createdInstanceB) {
        napi_value protoA = nullptr;
        napi_value protoB = nullptr;
        bool equal = false;
        if (GetPrototypeSafely(env, instanceA, &protoA) && GetPrototypeSafely(env, instanceB, &protoB) &&
            CheckStrictEquality(env, protoA, protoB, &equal)) {
            prototypesCompared = true;
            prototypesDistinct = !equal;
        }
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", "unrelatedConstructors");
    SetNamedBool(env, result, "createdClasses", createdClassA && createdClassB);
    SetNamedBool(env, result, "instanceOfOwn", instanceOfOwn);
    SetNamedBool(env, result, "notInstanceOfOther", !instanceOfOther);
    SetNamedBool(env, result, "prototypesCompared", prototypesCompared);
    SetNamedBool(env, result, "prototypesDistinct", prototypesDistinct);
    return result;
}

static napi_value TestChainTypeSequence(napi_env env, napi_callback_info info)
{
    napi_value constructor = nullptr;
    bool createdClass = CreateTestClass(env, "TypeSequenceClass", &constructor);

    napi_value instance = nullptr;
    bool createdInstance = false;
    if (createdClass) {
        createdInstance = CreateTestInstance(
            env, constructor, PrototypeConst::K_INSTANCE_VALUE_BASE, &instance);
    }

    PrototypeChainResult chainResult = {};
    bool walkSuccessful = false;
    if (createdInstance) {
        walkSuccessful = WalkPrototypeChain(env, instance, &chainResult);
    }

    bool firstLabelIsObject = false;
    bool allLabelsObject = false;
    if (walkSuccessful && !chainResult.typeLabels.empty()) {
        const std::string objectLabel = ValuetypeToLabel(napi_object);
        firstLabelIsObject = chainResult.typeLabels.front() == objectLabel;
        allLabelsObject = firstLabelIsObject;
        for (size_t i = 0; i < chainResult.typeLabels.size(); i++) {
            if (chainResult.typeLabels[i] != objectLabel) {
                allLabelsObject = false;
            }
        }
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", "chainTypeSequence");
    SetNamedBool(env, result, "createdInstance", createdInstance);
    SetNamedBool(env, result, "walkSuccessful", walkSuccessful);
    SetNamedBool(env, result, "firstLabelIsObject", firstLabelIsObject);
    SetNamedBool(env, result, "allLabelsObject", allLabelsObject);
    SetNamedBool(env, result, "reachedNull", chainResult.reachedNull);
    SetNamedInt32(env, result, "depth", chainResult.depth);
    return result;
}

static napi_value TestConstructorPrototypeStability(napi_env env, napi_callback_info info)
{
    napi_value constructor = nullptr;
    bool createdClass = CreateTestClass(env, "StableProtoClass", &constructor);

    napi_value firstProp = nullptr;
    napi_value secondProp = nullptr;
    bool gotFirst = false;
    bool gotSecond = false;
    if (createdClass) {
        gotFirst = GetFunctionPrototypeProperty(env, constructor, &firstProp);
        gotSecond = GetFunctionPrototypeProperty(env, constructor, &secondProp);
    }

    bool sameReference = false;
    if (gotFirst && gotSecond) {
        CheckStrictEquality(env, firstProp, secondProp, &sameReference);
    }

    napi_value instance = nullptr;
    bool createdInstance = false;
    if (createdClass) {
        createdInstance = CreateTestInstance(
            env, constructor, PrototypeConst::K_INSTANCE_VALUE_BASE, &instance);
    }

    bool instanceMatches = false;
    if (createdInstance && gotFirst) {
        napi_value instanceProto = nullptr;
        if (GetPrototypeSafely(env, instance, &instanceProto)) {
            CheckStrictEquality(env, firstProp, instanceProto, &instanceMatches);
        }
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", "constructorPrototypeStability");
    SetNamedBool(env, result, "createdClass", createdClass);
    SetNamedBool(env, result, "gotPrototypeProperty", gotFirst && gotSecond);
    SetNamedBool(env, result, "sameReference", sameReference);
    SetNamedBool(env, result, "instanceMatches", instanceMatches);
    return result;
}

static napi_value RunPrototypeCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = PrototypeConst::K_ARG_COUNT_TWO;
    napi_value args[PrototypeConst::K_ARG_COUNT_TWO] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));

    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= PrototypeConst::K_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid prototype case");
        return nullptr;
    }

    const auto spec = GetPrototypeCaseSpec(caseIndex);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", spec.name);
    SetNamedInt32(env, result, "valueFactor", spec.valueFactor);
    SetNamedInt32(env, result, "depthLimit", spec.depthLimit);
    SetNamedBool(env, result, "testChainWalking", spec.testChainWalking);
    SetNamedBool(env, result, "testInstanceof", spec.testInstanceof);
    SetNamedBool(env, result, "testTypeof", spec.testTypeof);

    if (argc >= PrototypeConst::K_ARG_COUNT_ONE && args[0] != nullptr) {
        napi_valuetype argType = napi_undefined;
        FetchValueType(env, args[0], &argType);
        SetNamedString(env, result, "arg0Type", ValuetypeToLabel(argType));
    }
    if (argc >= PrototypeConst::K_ARG_COUNT_TWO && args[PrototypeConst::K_SECOND_ARG_INDEX] != nullptr) {
        napi_valuetype argType = napi_undefined;
        FetchValueType(env, args[PrototypeConst::K_SECOND_ARG_INDEX], &argType);
        SetNamedString(env, result, "arg1Type", ValuetypeToLabel(argType));
    }
    return result;
}

}  // namespace

static napi_value InitPrototypeSuite(napi_env env, napi_value exports)
{
    napi_property_descriptor namedDesc[] = {
        DECLARE_NAPI_FUNCTION("testPlainObjectPrototype", TestPlainObjectPrototype),
        DECLARE_NAPI_FUNCTION("testArrayPrototype", TestArrayPrototype),
        DECLARE_NAPI_FUNCTION("testFunctionPrototype", TestFunctionPrototype),
        DECLARE_NAPI_FUNCTION("testInstancePrototype", TestInstancePrototype),
        DECLARE_NAPI_FUNCTION("testPrototypeChainWalking", TestPrototypeChainWalking),
        DECLARE_NAPI_FUNCTION("testInstanceOfPositive", TestInstanceOfPositive),
        DECLARE_NAPI_FUNCTION("testInstanceOfNegativeUnrelated", TestInstanceOfNegativeUnrelated),
        DECLARE_NAPI_FUNCTION("testInstanceOfNegativePlain", TestInstanceOfNegativePlain),
        DECLARE_NAPI_FUNCTION("testSameConstructorPrototypes", TestSameConstructorPrototypes),
        DECLARE_NAPI_FUNCTION("testFunctionPrototypeProperty", TestFunctionPrototypeProperty),
        DECLARE_NAPI_FUNCTION("testTypeofAlongChain", TestTypeofAlongChain),
        DECLARE_NAPI_FUNCTION("testChainReachedNull", TestChainReachedNull),
        DECLARE_NAPI_FUNCTION("testChainDepthWithinBound", TestChainDepthWithinBound),
        DECLARE_NAPI_FUNCTION("testChainChecksumConsistency", TestChainChecksumConsistency),
        DECLARE_NAPI_FUNCTION("testPrototypeStrictEquals", TestPrototypeStrictEquals),
        DECLARE_NAPI_FUNCTION("testUnrelatedConstructors", TestUnrelatedConstructors),
        DECLARE_NAPI_FUNCTION("testChainTypeSequence", TestChainTypeSequence),
        DECLARE_NAPI_FUNCTION("testConstructorPrototypeStability", TestConstructorPrototypeStability),
    };

    std::vector<napi_property_descriptor> descriptors(
        namedDesc, namedDesc + sizeof(namedDesc) / sizeof(namedDesc[0]));
    std::vector<std::string> exportNames;
    for (size_t caseIndex = 0; caseIndex < PrototypeConst::K_CASE_COUNT; caseIndex++) {
        exportNames.emplace_back(BuildPrototypeExportName(caseIndex));
        descriptors.push_back(napi_property_descriptor{
            exportNames.back().c_str(),
            nullptr,
            RunPrototypeCase,
            nullptr,
            nullptr,
            nullptr,
            napi_default,
            reinterpret_cast<void*>(static_cast<uintptr_t>(caseIndex)),
        });
    }

    NAPI_CALL(env, napi_define_properties(env, exports, descriptors.size(), descriptors.data()));
    return exports;
}

static napi_module g_prototypeSuiteModule = {
    .nm_version = PrototypeConst::K_MODULE_VERSION,
    .nm_flags = PrototypeConst::K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitPrototypeSuite,
    .nm_modname = "prototype_suite",
    .nm_priv = nullptr,
    .reserved = { nullptr },
};

extern "C" __attribute__((constructor)) void RegisterPrototypeSuiteModule(void)
{
    napi_module_register(&g_prototypeSuiteModule);
}
