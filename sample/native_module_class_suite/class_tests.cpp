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

#include "class_helper.h"

namespace {

// ---------------------------------------------------------------------------
// Constructor Callbacks
// ---------------------------------------------------------------------------

// Basic constructor with no arguments - stores instance data via properties
static napi_value BasicConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr));
    InstanceData data = { K_INT_VALUE_ZERO, K_INT_VALUE_FORTY_TWO, K_DOUBLE_PI };
    StoreInstanceData(env, thisArg, data);
    return thisArg;
}

// Constructor with single int32 argument
static napi_value Int32ArgConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    size_t argc = K_ARGS_ONE;
    napi_value argv[K_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr));
    int32_t argValue = K_INT_VALUE_ZERO;
    if (argc >= K_ARGS_ONE) {
        GetInt32Value(env, argv[0], &argValue);
    }
    InstanceData data = { argValue, K_INT_VALUE_ONE_HUNDRED, K_DOUBLE_E };
    StoreInstanceData(env, thisArg, data);
    return thisArg;
}

// Constructor with multiple arguments (int32, double, string)
static napi_value MultiArgConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    size_t argc = K_ARGS_THREE;
    napi_value argv[K_ARGS_THREE] = { nullptr, nullptr, nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr));
    int32_t idVal = K_INT_VALUE_ZERO;
    double factorVal = K_DOUBLE_ZERO;
    std::string nameVal = "";
    if (argc >= K_ARGS_ONE) {
        GetInt32Value(env, argv[0], &idVal);
    }
    if (argc >= K_ARGS_TWO) {
        GetDoubleValue(env, argv[K_ARG_INDEX_SECOND], &factorVal);
    }
    if (argc >= K_ARGS_THREE) {
        GetStringValue(env, argv[K_ARG_INDEX_THIRD], nameVal);
    }
    InstanceData data = { idVal, static_cast<int32_t>(factorVal * K_DOUBLE_ONE_HUNDRED), factorVal };
    StoreInstanceData(env, thisArg, data);
    SetNamedString(env, thisArg, "name", nameVal);
    return thisArg;
}

// Constructor using napi_get_new_target to distinguish new vs direct call
static napi_value NewTargetConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    napi_value newTarget = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr));
    NAPI_CALL(env, napi_get_new_target(env, info, &newTarget));
    bool calledWithNew = (newTarget != nullptr);
    InstanceData data = { calledWithNew ? K_INT_VALUE_ONE : K_INT_VALUE_ZERO,
                          K_INT_VALUE_TWO_HUNDRED, K_DOUBLE_TWO };
    StoreInstanceData(env, thisArg, data);
    SetNamedBool(env, thisArg, "calledWithNew", calledWithNew);
    return thisArg;
}

// Constructor for Vector3D-like class
static napi_value VectorConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    size_t argc = K_ARGS_THREE;
    napi_value argv[K_ARGS_THREE] = { nullptr, nullptr, nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr));
    double x = K_DOUBLE_ZERO;
    double y = K_DOUBLE_ZERO;
    double z = K_DOUBLE_ZERO;
    if (argc >= K_ARGS_ONE) {
        GetDoubleValue(env, argv[0], &x);
    }
    if (argc >= K_ARGS_TWO) {
        GetDoubleValue(env, argv[K_ARG_INDEX_SECOND], &y);
    }
    if (argc >= K_ARGS_THREE) {
        GetDoubleValue(env, argv[K_ARG_INDEX_THIRD], &z);
    }
    VectorData data = { x, y, z };
    StoreVectorData(env, thisArg, data);
    return thisArg;
}

// Constructor with int64 argument
static napi_value Int64ArgConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    size_t argc = K_ARGS_ONE;
    napi_value argv[K_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr));
    int64_t argValue = K_INT64_ZERO;
    if (argc >= K_ARGS_ONE) {
        GetInt64Value(env, argv[0], &argValue);
    }
    SetNamedInt64(env, thisArg, "int64Value", argValue);
    InstanceData data = { static_cast<int32_t>(argValue % K_INT_VALUE_ONE_HUNDRED),
                          K_INT_VALUE_THREE_HUNDRED, K_DOUBLE_LARGE };
    StoreInstanceData(env, thisArg, data);
    return thisArg;
}

// Constructor with uint32 argument
static napi_value Uint32ArgConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    size_t argc = K_ARGS_ONE;
    napi_value argv[K_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr));
    uint32_t argValue = K_UINT32_ZERO;
    if (argc >= K_ARGS_ONE) {
        GetUInt32Value(env, argv[0], &argValue);
    }
    SetNamedInt32(env, thisArg, "uint32Value", static_cast<int32_t>(argValue));
    InstanceData data = { static_cast<int32_t>(argValue % K_INT_VALUE_ONE_HUNDRED),
                          K_INT_VALUE_FOUR, K_DOUBLE_THREE };
    StoreInstanceData(env, thisArg, data);
    return thisArg;
}

// Constructor with boolean argument
static napi_value BoolArgConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    size_t argc = K_ARGS_ONE;
    napi_value argv[K_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr));
    bool argValue = false;
    if (argc >= K_ARGS_ONE) {
        GetBoolValue(env, argv[0], &argValue);
    }
    SetNamedBool(env, thisArg, "enabled", argValue);
    InstanceData data = { argValue ? K_INT_VALUE_ONE : K_INT_VALUE_ZERO,
                          K_INT_VALUE_FIVE, K_DOUBLE_PI };
    StoreInstanceData(env, thisArg, data);
    return thisArg;
}

// ---------------------------------------------------------------------------
// Instance Method Callbacks
// ---------------------------------------------------------------------------

// Instance method returning stored id
static napi_value GetId(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr));
    InstanceData data;
    LoadInstanceData(env, thisArg, data);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, data.id, &result));
    return result;
}

// Instance method returning stored value
static napi_value GetValue(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr));
    InstanceData data;
    LoadInstanceData(env, thisArg, data);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, data.value, &result));
    return result;
}

// Instance method returning stored factor
static napi_value GetFactor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr));
    InstanceData data;
    LoadInstanceData(env, thisArg, data);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_double(env, data.factor, &result));
    return result;
}

// Instance method that sets a new value
static napi_value SetValue(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    size_t argc = K_ARGS_ONE;
    napi_value argv[K_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr));
    InstanceData data;
    LoadInstanceData(env, thisArg, data);
    if (argc >= K_ARGS_ONE) {
        GetInt32Value(env, argv[0], &data.value);
        StoreInstanceData(env, thisArg, data);
    }
    napi_value undefined = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undefined));
    return undefined;
}

// Instance method computing sum of id and value
static napi_value ComputeSum(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr));
    InstanceData data;
    LoadInstanceData(env, thisArg, data);
    int32_t sum = data.id + data.value;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, sum, &result));
    return result;
}

// Instance method computing product
// Instance method returning vector magnitude
static napi_value GetMagnitude(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr));
    VectorData data;
    LoadVectorData(env, thisArg, data);
    double mag = data.x * data.x + data.y * data.y + data.z * data.z;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_double(env, mag, &result));
    return result;
}

// ---------------------------------------------------------------------------
// Static Method Callbacks
// ---------------------------------------------------------------------------

static napi_value StaticCreateDefault(napi_env env, napi_callback_info info)
{
    napi_value ctor = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, nullptr, nullptr));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_new_instance(env, ctor, K_ARGS_ZERO, nullptr, &result));
    return result;
}

// ---------------------------------------------------------------------------
// Test Functions
// ---------------------------------------------------------------------------

// Test 1: Basic napi_define_class
static napi_value TestDefineClassBasic(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value constructor = nullptr;
    napi_status status = napi_define_class(env, "TestClass01", NAPI_AUTO_LENGTH,
        BasicConstructor, nullptr, K_PROP_COUNT_ZERO, nullptr, &constructor);
    if (status != napi_ok) {
        SetResultFailure(env, result, "TestDefineClassBasic", "napi_define_class failed");
        return result;
    }
    napi_valuetype valType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, constructor, &valType));
    SetNamedBool(env, result, "constructorIsFunction", valType == napi_function);
    SetResultSuccess(env, result, "TestDefineClassBasic");
    return result;
}

// Test 2: napi_define_class with instance methods
static napi_value TestDefineClassWithMethods(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_property_descriptor methods[] = {
        { "getId", nullptr, GetId, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getValue", nullptr, GetValue, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getFactor", nullptr, GetFactor, nullptr, nullptr, nullptr, napi_default, nullptr },
    };
    napi_value constructor = nullptr;
    napi_status status = napi_define_class(env, "TestClass02", NAPI_AUTO_LENGTH,
        BasicConstructor, nullptr, K_PROP_COUNT_THREE, methods, &constructor);
    if (status != napi_ok) {
        SetResultFailure(env, result, "TestDefineClassWithMethods", "define_class failed");
        return result;
    }
    napi_value instance = nullptr;
    NAPI_CALL(env, napi_new_instance(env, constructor, K_ARGS_ZERO, nullptr, &instance));
    SetNamedBool(env, result, "instanceCreated", instance != nullptr);
    SetResultSuccess(env, result, "TestDefineClassWithMethods");
    return result;
}

// Test 3: napi_define_class with static methods
static napi_value TestDefineClassWithStatic(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_property_descriptor props[] = {
        { "createDefault", nullptr, StaticCreateDefault, nullptr, nullptr, nullptr,
          static_cast<napi_property_attributes>(napi_static), nullptr },
    };
    napi_value constructor = nullptr;
    napi_status status = napi_define_class(env, "TestClass03", NAPI_AUTO_LENGTH,
        BasicConstructor, nullptr, K_PROP_COUNT_ONE, props, &constructor);
    if (status != napi_ok) {
        SetResultFailure(env, result, "TestDefineClassWithStatic", "define_class failed");
        return result;
    }
    bool hasStatic = false;
    NAPI_CALL(env, napi_has_named_property(env, constructor, "createDefault", &hasStatic));
    SetNamedBool(env, result, "hasStaticMethod", hasStatic);
    SetResultSuccess(env, result, "TestDefineClassWithStatic");
    return result;
}

// Test 4: napi_define_class with properties
static napi_value TestDefineClassWithProperties(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value initVal = nullptr;
    CreateInt32Value(env, K_INT_VALUE_TEN, &initVal);
    napi_property_descriptor props[] = {
        { "initialValue", nullptr, nullptr, nullptr, nullptr, initVal, napi_default, nullptr },
    };
    napi_value constructor = nullptr;
    napi_status status = napi_define_class(env, "TestClass04", NAPI_AUTO_LENGTH,
        BasicConstructor, nullptr, K_PROP_COUNT_ONE, props, &constructor);
    if (status != napi_ok) {
        SetResultFailure(env, result, "TestDefineClassWithProperties", "define_class failed");
        return result;
    }
    napi_value instance = nullptr;
    NAPI_CALL(env, napi_new_instance(env, constructor, K_ARGS_ZERO, nullptr, &instance));
    napi_value propVal = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, instance, "initialValue", &propVal));
    int32_t val = K_INT_VALUE_ZERO;
    GetInt32Value(env, propVal, &val);
    SetNamedBool(env, result, "propertyValueCorrect", val == K_INT_VALUE_TEN);
    SetResultSuccess(env, result, "TestDefineClassWithProperties");
    return result;
}

// Test 5: napi_new_instance with no arguments
static napi_value TestNewInstanceNoArgs(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value constructor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "TestClass05", NAPI_AUTO_LENGTH,
        BasicConstructor, nullptr, K_PROP_COUNT_ZERO, nullptr, &constructor));
    napi_value instance = nullptr;
    napi_status status = napi_new_instance(env, constructor, K_ARGS_ZERO, nullptr, &instance);
    if (status != napi_ok) {
        SetResultFailure(env, result, "TestNewInstanceNoArgs", "new_instance failed");
        return result;
    }
    napi_valuetype valType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, instance, &valType));
    SetNamedBool(env, result, "instanceIsObject", valType == napi_object);
    SetResultSuccess(env, result, "TestNewInstanceNoArgs");
    return result;
}

// Test 6: napi_new_instance with int32 argument
static napi_value TestNewInstanceInt32Arg(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value constructor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "TestClass06", NAPI_AUTO_LENGTH,
        Int32ArgConstructor, nullptr, K_PROP_COUNT_ZERO, nullptr, &constructor));
    napi_value arg = nullptr;
    CreateInt32Value(env, K_INT_VALUE_FORTY_TWO, &arg);
    napi_value instance = nullptr;
    napi_status status = napi_new_instance(env, constructor, K_ARGS_ONE, &arg, &instance);
    if (status != napi_ok) {
        SetResultFailure(env, result, "TestNewInstanceInt32Arg", "new_instance failed");
        return result;
    }
    InstanceData data;
    LoadInstanceData(env, instance, data);
    SetNamedBool(env, result, "argPassedCorrectly", data.id == K_INT_VALUE_FORTY_TWO);
    SetResultSuccess(env, result, "TestNewInstanceInt32Arg");
    return result;
}

// Test 7: napi_new_instance with multiple arguments
static napi_value TestNewInstanceMultiArgs(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value constructor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "TestClass07", NAPI_AUTO_LENGTH,
        MultiArgConstructor, nullptr, K_PROP_COUNT_ZERO, nullptr, &constructor));
    napi_value args[K_ARGS_THREE] = { nullptr, nullptr, nullptr };
    CreateInt32Value(env, K_INT_VALUE_ONE_HUNDRED, &args[K_ARG_INDEX_FIRST]);
    CreateDoubleValue(env, K_DOUBLE_PI, &args[K_ARG_INDEX_SECOND]);
    CreateStringValue(env, std::string("testName"), &args[K_ARG_INDEX_THIRD]);
    napi_value instance = nullptr;
    napi_status status = napi_new_instance(env, constructor, K_ARGS_THREE, args, &instance);
    if (status != napi_ok) {
        SetResultFailure(env, result, "TestNewInstanceMultiArgs", "new_instance failed");
        return result;
    }
    InstanceData data;
    LoadInstanceData(env, instance, data);
    SetNamedBool(env, result, "idCorrect", data.id == K_INT_VALUE_ONE_HUNDRED);
    SetResultSuccess(env, result, "TestNewInstanceMultiArgs");
    return result;
}

// Test 8: napi_new_instance with int64 argument
static napi_value TestNewInstanceInt64Arg(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value constructor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "TestClass08", NAPI_AUTO_LENGTH,
        Int64ArgConstructor, nullptr, K_PROP_COUNT_ZERO, nullptr, &constructor));
    napi_value arg = nullptr;
    CreateInt64Value(env, K_INT64_LARGE, &arg);
    napi_value instance = nullptr;
    napi_status status = napi_new_instance(env, constructor, K_ARGS_ONE, &arg, &instance);
    if (status != napi_ok) {
        SetResultFailure(env, result, "TestNewInstanceInt64Arg", "new_instance failed");
        return result;
    }
    InstanceData data;
    LoadInstanceData(env, instance, data);
    SetNamedBool(env, result, "idFromInt64", data.id == static_cast<int32_t>(
        K_INT64_LARGE % K_INT_VALUE_ONE_HUNDRED));
    SetResultSuccess(env, result, "TestNewInstanceInt64Arg");
    return result;
}

// Test 9: napi_new_instance with double argument
static napi_value TestNewInstanceDoubleArg(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_property_descriptor methods[] = {
        { "getMagnitude", nullptr, GetMagnitude, nullptr, nullptr, nullptr, napi_default, nullptr },
    };
    napi_value constructor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "Vector3D", NAPI_AUTO_LENGTH,
        VectorConstructor, nullptr, K_PROP_COUNT_ONE, methods, &constructor));
    napi_value args[K_ARGS_THREE] = { nullptr, nullptr, nullptr };
    CreateDoubleValue(env, K_DOUBLE_THREE, &args[K_ARG_INDEX_FIRST]);
    CreateDoubleValue(env, K_DOUBLE_FOUR, &args[K_ARG_INDEX_SECOND]);
    CreateDoubleValue(env, K_DOUBLE_ZERO, &args[K_ARG_INDEX_THIRD]);
    napi_value instance = nullptr;
    napi_status status = napi_new_instance(env, constructor, K_ARGS_THREE, args, &instance);
    if (status != napi_ok) {
        SetResultFailure(env, result, "TestNewInstanceDoubleArg", "new_instance failed");
        return result;
    }
    VectorData data;
    LoadVectorData(env, instance, data);
    SetNamedBool(env, result, "xCorrect", data.x == K_DOUBLE_THREE);
    SetNamedBool(env, result, "yCorrect", data.y == K_DOUBLE_FOUR);
    SetResultSuccess(env, result, "TestNewInstanceDoubleArg");
    return result;
}

// Test 10: napi_new_instance with boolean argument
static napi_value TestNewInstanceBoolArg(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value constructor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "TestClass10", NAPI_AUTO_LENGTH,
        BoolArgConstructor, nullptr, K_PROP_COUNT_ZERO, nullptr, &constructor));
    napi_value arg = nullptr;
    CreateBoolValue(env, true, &arg);
    napi_value instance = nullptr;
    napi_status status = napi_new_instance(env, constructor, K_ARGS_ONE, &arg, &instance);
    if (status != napi_ok) {
        SetResultFailure(env, result, "TestNewInstanceBoolArg", "new_instance failed");
        return result;
    }
    bool enabled = false;
    GetNamedBool(env, instance, "enabled", &enabled);
    SetNamedBool(env, result, "enabledTrue", enabled);
    SetResultSuccess(env, result, "TestNewInstanceBoolArg");
    return result;
}

// Test 11: napi_instanceof positive case
static napi_value TestInstanceOfPositive(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value constructor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "TestClass11", NAPI_AUTO_LENGTH,
        BasicConstructor, nullptr, K_PROP_COUNT_ZERO, nullptr, &constructor));
    napi_value instance = nullptr;
    NAPI_CALL(env, napi_new_instance(env, constructor, K_ARGS_ZERO, nullptr, &instance));
    bool isInstance = false;
    napi_status status = napi_instanceof(env, instance, constructor, &isInstance);
    if (status != napi_ok) {
        SetResultFailure(env, result, "TestInstanceOfPositive", "instanceof failed");
        return result;
    }
    SetNamedBool(env, result, "isInstanceOf", isInstance);
    SetResultSuccess(env, result, "TestInstanceOfPositive");
    return result;
}

// Test 12: napi_instanceof negative case
static napi_value TestInstanceOfNegative(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value constructor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "TestClass12", NAPI_AUTO_LENGTH,
        BasicConstructor, nullptr, K_PROP_COUNT_ZERO, nullptr, &constructor));
    napi_value plainObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &plainObj));
    bool isInstance = true;
    napi_status status = napi_instanceof(env, plainObj, constructor, &isInstance);
    if (status != napi_ok) {
        SetResultFailure(env, result, "TestInstanceOfNegative", "instanceof failed");
        return result;
    }
    SetNamedBool(env, result, "plainNotInstanceOf", !isInstance);
    SetResultSuccess(env, result, "TestInstanceOfNegative");
    return result;
}

// Test 13: napi_instanceof with different classes
static napi_value TestInstanceOfDifferentClass(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value ctorA = nullptr;
    napi_value ctorB = nullptr;
    NAPI_CALL(env, napi_define_class(env, "ClassA", NAPI_AUTO_LENGTH,
        BasicConstructor, nullptr, K_PROP_COUNT_ZERO, nullptr, &ctorA));
    NAPI_CALL(env, napi_define_class(env, "ClassB", NAPI_AUTO_LENGTH,
        Int32ArgConstructor, nullptr, K_PROP_COUNT_ZERO, nullptr, &ctorB));
    napi_value instanceA = nullptr;
    NAPI_CALL(env, napi_new_instance(env, ctorA, K_ARGS_ZERO, nullptr, &instanceA));
    bool isInstanceOfA = false;
    bool isInstanceOfB = true;
    NAPI_CALL(env, napi_instanceof(env, instanceA, ctorA, &isInstanceOfA));
    NAPI_CALL(env, napi_instanceof(env, instanceA, ctorB, &isInstanceOfB));
    SetNamedBool(env, result, "isInstanceOfA", isInstanceOfA);
    SetNamedBool(env, result, "notInstanceOfB", !isInstanceOfB);
    SetResultSuccess(env, result, "TestInstanceOfDifferentClass");
    return result;
}

// Test 14: napi_call_function for instance method
static napi_value TestCallInstanceMethod(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_property_descriptor methods[] = {
        { "getId", nullptr, GetId, nullptr, nullptr, nullptr, napi_default, nullptr },
    };
    napi_value constructor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "TestClass14", NAPI_AUTO_LENGTH,
        BasicConstructor, nullptr, K_PROP_COUNT_ONE, methods, &constructor));
    napi_value instance = nullptr;
    NAPI_CALL(env, napi_new_instance(env, constructor, K_ARGS_ZERO, nullptr, &instance));
    napi_value method = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, instance, "getId", &method));
    napi_value methodResult = nullptr;
    NAPI_CALL(env, napi_call_function(env, instance, method, K_ARGS_ZERO, nullptr, &methodResult));
    int32_t idVal = K_INT_VALUE_NEG_FIFTY;
    GetInt32Value(env, methodResult, &idVal);
    SetNamedBool(env, result, "methodReturnedId", idVal == K_INT_VALUE_ZERO);
    SetResultSuccess(env, result, "TestCallInstanceMethod");
    return result;
}

// Test 15: napi_call_function for multiple instance methods
static napi_value TestCallMultipleMethods(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_property_descriptor methods[] = {
        { "getId", nullptr, GetId, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getValue", nullptr, GetValue, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "computeSum", nullptr, ComputeSum, nullptr, nullptr, nullptr, napi_default, nullptr },
    };
    napi_value constructor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "TestClass15", NAPI_AUTO_LENGTH,
        BasicConstructor, nullptr, K_PROP_COUNT_THREE, methods, &constructor));
    napi_value instance = nullptr;
    NAPI_CALL(env, napi_new_instance(env, constructor, K_ARGS_ZERO, nullptr, &instance));
    napi_value sumMethod = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, instance, "computeSum", &sumMethod));
    napi_value sumResult = nullptr;
    NAPI_CALL(env, napi_call_function(env, instance, sumMethod, K_ARGS_ZERO, nullptr, &sumResult));
    int32_t sum = K_INT_VALUE_NEG_FIFTY;
    GetInt32Value(env, sumResult, &sum);
    SetNamedBool(env, result, "sumCorrect", sum == K_INT_VALUE_FORTY_TWO);
    SetResultSuccess(env, result, "TestCallMultipleMethods");
    return result;
}

// Test 16: napi_get_new_target when called with new
static napi_value TestGetNewTargetWithNew(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value constructor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "TestClass16", NAPI_AUTO_LENGTH,
        NewTargetConstructor, nullptr, K_PROP_COUNT_ZERO, nullptr, &constructor));
    napi_value instance = nullptr;
    NAPI_CALL(env, napi_new_instance(env, constructor, K_ARGS_ZERO, nullptr, &instance));
    bool calledWithNew = false;
    GetNamedBool(env, instance, "calledWithNew", &calledWithNew);
    SetNamedBool(env, result, "calledWithNew", calledWithNew);
    SetResultSuccess(env, result, "TestGetNewTargetWithNew");
    return result;
}

// Test 17: napi_typeof on constructor and instance
static napi_value TestTypeOfConstructorAndInstance(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_value constructor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "TestClass17", NAPI_AUTO_LENGTH,
        BasicConstructor, nullptr, K_PROP_COUNT_ZERO, nullptr, &constructor));
    napi_value instance = nullptr;
    NAPI_CALL(env, napi_new_instance(env, constructor, K_ARGS_ZERO, nullptr, &instance));
    napi_valuetype ctorType = napi_undefined;
    napi_valuetype instType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, constructor, &ctorType));
    NAPI_CALL(env, napi_typeof(env, instance, &instType));
    SetNamedBool(env, result, "ctorIsFunction", ctorType == napi_function);
    SetNamedBool(env, result, "instanceIsObject", instType == napi_object);
    SetResultSuccess(env, result, "TestTypeOfConstructorAndInstance");
    return result;
}

// Test 18: Instance field values set in constructor
static napi_value TestInstanceFieldValues(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_property_descriptor methods[] = {
        { "getId", nullptr, GetId, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getFactor", nullptr, GetFactor, nullptr, nullptr, nullptr, napi_default, nullptr },
    };
    napi_value constructor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "TestClass18", NAPI_AUTO_LENGTH,
        BasicConstructor, nullptr, K_PROP_COUNT_TWO, methods, &constructor));
    napi_value instance = nullptr;
    NAPI_CALL(env, napi_new_instance(env, constructor, K_ARGS_ZERO, nullptr, &instance));
    napi_value idMethod = nullptr;
    napi_value factorMethod = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, instance, "getId", &idMethod));
    NAPI_CALL(env, napi_get_named_property(env, instance, "getFactor", &factorMethod));
    napi_value idResult = nullptr;
    napi_value factorResult = nullptr;
    NAPI_CALL(env, napi_call_function(env, instance, idMethod, K_ARGS_ZERO, nullptr, &idResult));
    NAPI_CALL(env, napi_call_function(env, instance, factorMethod, K_ARGS_ZERO, nullptr, &factorResult));
    int32_t idVal = K_INT_VALUE_NEG_FIFTY;
    double factorVal = K_DOUBLE_NEG_ONE;
    GetInt32Value(env, idResult, &idVal);
    GetDoubleValue(env, factorResult, &factorVal);
    SetNamedBool(env, result, "idMatchesDefault", idVal == K_INT_VALUE_ZERO);
    SetNamedBool(env, result, "factorMatchesPi", factorVal == K_DOUBLE_PI);
    SetResultSuccess(env, result, "TestInstanceFieldValues");
    return result;
}

// Test 19: Multiple instances with different data
static napi_value TestMultipleInstances(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_property_descriptor methods[] = {
        { "getValue", nullptr, GetValue, nullptr, nullptr, nullptr, napi_default, nullptr },
    };
    napi_value constructor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "TestClass19", NAPI_AUTO_LENGTH,
        Int32ArgConstructor, nullptr, K_PROP_COUNT_ONE, methods, &constructor));
    napi_value arg1 = nullptr;
    napi_value arg2 = nullptr;
    CreateInt32Value(env, K_INT_VALUE_ONE, &arg1);
    CreateInt32Value(env, K_INT_VALUE_TWO, &arg2);
    napi_value inst1 = nullptr;
    napi_value inst2 = nullptr;
    NAPI_CALL(env, napi_new_instance(env, constructor, K_ARGS_ONE, &arg1, &inst1));
    NAPI_CALL(env, napi_new_instance(env, constructor, K_ARGS_ONE, &arg2, &inst2));
    InstanceData data1;
    InstanceData data2;
    LoadInstanceData(env, inst1, data1);
    LoadInstanceData(env, inst2, data2);
    SetNamedBool(env, result, "inst1HasId", data1.id == K_INT_VALUE_ONE);
    SetNamedBool(env, result, "inst2HasId", data2.id == K_INT_VALUE_TWO);
    SetResultSuccess(env, result, "TestMultipleInstances");
    return result;
}

// Test 20: Setter method updating instance data
static napi_value TestSetterMethod(napi_env env, napi_callback_info info)
{
    napi_value result = CreateResultObject(env);
    napi_property_descriptor methods[] = {
        { "getValue", nullptr, GetValue, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "setValue", nullptr, SetValue, nullptr, nullptr, nullptr, napi_default, nullptr },
    };
    napi_value constructor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "TestClass20", NAPI_AUTO_LENGTH,
        BasicConstructor, nullptr, K_PROP_COUNT_TWO, methods, &constructor));
    napi_value instance = nullptr;
    NAPI_CALL(env, napi_new_instance(env, constructor, K_ARGS_ZERO, nullptr, &instance));
    napi_value setMethod = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, instance, "setValue", &setMethod));
    napi_value newVal = nullptr;
    CreateInt32Value(env, K_INT_VALUE_TWO_HUNDRED, &newVal);
    NAPI_CALL(env, napi_call_function(env, instance, setMethod, K_ARGS_ONE, &newVal, nullptr));
    napi_value getMethod = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, instance, "getValue", &getMethod));
    napi_value getResult = nullptr;
    NAPI_CALL(env, napi_call_function(env, instance, getMethod, K_ARGS_ZERO, nullptr, &getResult));
    int32_t val = K_INT_VALUE_ZERO;
    GetInt32Value(env, getResult, &val);
    SetNamedBool(env, result, "valueUpdated", val == K_INT_VALUE_TWO_HUNDRED);
    SetResultSuccess(env, result, "TestSetterMethod");
    return result;
}

// ---------------------------------------------------------------------------
// Module Registration
// ---------------------------------------------------------------------------
struct TestEntry {
    const char* name;
    napi_callback func;
};

static const TestEntry CLASS_TESTS[] = {
    { "testDefineClassBasic", TestDefineClassBasic },
    { "testDefineClassWithMethods", TestDefineClassWithMethods },
    { "testDefineClassWithStatic", TestDefineClassWithStatic },
    { "testDefineClassWithProperties", TestDefineClassWithProperties },
    { "testNewInstanceNoArgs", TestNewInstanceNoArgs },
    { "testNewInstanceInt32Arg", TestNewInstanceInt32Arg },
    { "testNewInstanceMultiArgs", TestNewInstanceMultiArgs },
    { "testNewInstanceInt64Arg", TestNewInstanceInt64Arg },
    { "testNewInstanceDoubleArg", TestNewInstanceDoubleArg },
    { "testNewInstanceBoolArg", TestNewInstanceBoolArg },
    { "testInstanceOfPositive", TestInstanceOfPositive },
    { "testInstanceOfNegative", TestInstanceOfNegative },
    { "testInstanceOfDifferentClass", TestInstanceOfDifferentClass },
    { "testCallInstanceMethod", TestCallInstanceMethod },
    { "testCallMultipleMethods", TestCallMultipleMethods },
    { "testGetNewTargetWithNew", TestGetNewTargetWithNew },
    { "testTypeOfConstructorAndInstance", TestTypeOfConstructorAndInstance },
    { "testInstanceFieldValues", TestInstanceFieldValues },
    { "testMultipleInstances", TestMultipleInstances },
    { "testSetterMethod", TestSetterMethod },
};

}  // namespace

static napi_value InitClassSuite(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[K_TEST_FUNCTION_COUNT];
    for (size_t i = 0; i < K_TEST_FUNCTION_COUNT; i++) {
        descriptors[i] = napi_property_descriptor {
            CLASS_TESTS[i].name,
            nullptr,
            CLASS_TESTS[i].func,
            nullptr,
            nullptr,
            nullptr,
            napi_default,
            nullptr
        };
    }
    NAPI_CALL(env, napi_define_properties(env, exports, K_TEST_FUNCTION_COUNT, descriptors));
    return exports;
}

static napi_module g_classSuiteModule = {
    .nm_version = K_MODULE_VERSION,
    .nm_flags = K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitClassSuite,
    .nm_modname = "class_suite",
    .nm_priv = nullptr,
    .reserved = { 0 }
};

extern "C" __attribute__((constructor)) void RegisterClassSuiteModule(void)
{
    napi_module_register(&g_classSuiteModule);
}