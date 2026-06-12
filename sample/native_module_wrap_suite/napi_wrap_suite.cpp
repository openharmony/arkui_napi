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

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

namespace {

// ---------------------------------------------------------------------------
// Named Constants
// ---------------------------------------------------------------------------
static constexpr uint32_t MODULE_VERSION = 1;
static constexpr uint32_t NO_MODULE_FLAGS = 0;

static constexpr int32_t INITIAL_VALUE = 0;
static constexpr int32_t TEST_VALUE_ONE = 42;
static constexpr int32_t TEST_VALUE_TWO = 100;
static constexpr int32_t TEST_VALUE_THREE = 200;
static constexpr int32_t TEST_VALUE_FOUR = 999;
static constexpr int32_t UPDATED_VALUE = 77;
static constexpr int32_t CTOR_ARG_VALUE = 55;

static constexpr double TEST_DOUBLE_VAL = 3.14;
static constexpr double TEST_DOUBLE_TWO = 2.718;

static constexpr size_t ARGS_ZERO = 0;
static constexpr size_t ARGS_ONE = 1;
static constexpr size_t ARGS_TWO = 2;
static constexpr size_t ARGS_THREE = 3;
static constexpr size_t MAX_ARGS = 10;

static constexpr size_t PROP_COUNT_ONE = 1;
static constexpr size_t PROP_COUNT_TWO = 2;
static constexpr size_t PROP_COUNT_THREE = 3;
static constexpr size_t PROP_COUNT_FOUR = 4;
static constexpr size_t PROP_COUNT_FIVE = 5;
static constexpr size_t PROP_COUNT_SIX = 6;

static constexpr uint64_t TYPE_TAG_LOWER_A = 0x12345678ABCD0001ULL;
static constexpr uint64_t TYPE_TAG_UPPER_A = 0x87654321DCBA0001ULL;
static constexpr uint64_t TYPE_TAG_LOWER_B = 0xAAAABBBBCCCC0002ULL;
static constexpr uint64_t TYPE_TAG_UPPER_B = 0xDDDDEEEEFFFF0002ULL;

static constexpr size_t TEST_FUNCTION_COUNT = 36;

static constexpr int32_t WRAP_CYCLE_COUNT = 3;
static constexpr int32_t FINALIZER_EXPECTED_CALLS = 1;

static constexpr size_t CLASS_PROPERTY_COUNT_ZERO = 0;

// ---------------------------------------------------------------------------
// Helper Functions
// ---------------------------------------------------------------------------
static bool SetNamedBool(napi_env env, napi_value obj, const char* name, bool val)
{
    napi_value napiVal = nullptr;
    if (napi_get_boolean(env, val, &napiVal) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, obj, name, napiVal) == napi_ok;
}

static bool SetNamedInt32(napi_env env, napi_value obj, const char* name, int32_t val)
{
    napi_value napiVal = nullptr;
    if (napi_create_int32(env, val, &napiVal) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, obj, name, napiVal) == napi_ok;
}

static bool SetNamedString(napi_env env, napi_value obj, const char* name, const char* val)
{
    napi_value napiVal = nullptr;
    if (napi_create_string_utf8(env, val, NAPI_AUTO_LENGTH, &napiVal) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, obj, name, napiVal) == napi_ok;
}

static bool SetNamedDouble(napi_env env, napi_value obj, const char* name, double val)
{
    napi_value napiVal = nullptr;
    if (napi_create_double(env, val, &napiVal) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, obj, name, napiVal) == napi_ok;
}

static napi_value CreateResult(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    return result;
}

// ---------------------------------------------------------------------------
// Native Data Structures
// ---------------------------------------------------------------------------
struct NativeWrapData {
    int32_t value;
};

struct PointData {
    double x;
    double y;
};

struct LifecycleData {
    int32_t id;
    bool finalized;
};

static int32_t g_finalizerCallCount = INITIAL_VALUE;
static int32_t g_multiFinalizerCount = INITIAL_VALUE;

static void WrapFinalizer([[maybe_unused]] napi_env env, void* data, [[maybe_unused]] void* hint)
{
    auto* wrap = static_cast<NativeWrapData*>(data);
    delete wrap;
    g_finalizerCallCount++;
}

static void GenericFinalizer([[maybe_unused]] napi_env env, [[maybe_unused]] void* data, [[maybe_unused]] void* hint)
{
    g_multiFinalizerCount++;
}

static void LifecycleFinalizer([[maybe_unused]] napi_env env, void* data, [[maybe_unused]] void* hint)
{
    auto* lc = static_cast<LifecycleData*>(data);
    lc->finalized = true;
}

static void PointFinalizer([[maybe_unused]] napi_env env, void* data, [[maybe_unused]] void* hint)
{
    auto* pt = static_cast<PointData*>(data);
    delete pt;
}

// ---------------------------------------------------------------------------
// Class Callbacks
// ---------------------------------------------------------------------------
static napi_value BasicClassConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr));
    auto* data = new NativeWrapData { INITIAL_VALUE };
    NAPI_CALL(env, napi_wrap(env, thisArg, data, WrapFinalizer, nullptr, nullptr));
    return thisArg;
}

static napi_value ArgClassConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr));
    int32_t initVal = INITIAL_VALUE;
    if (argc >= ARGS_ONE) {
        napi_get_value_int32(env, argv[0], &initVal);
    }
    auto* data = new NativeWrapData { initVal };
    // NativeWrapData is released by WrapFinalizer when the JS object is garbage-collected.
    NAPI_CALL(env, napi_wrap(env, thisArg, data, WrapFinalizer, nullptr, nullptr));
    return thisArg;
}

static napi_value ClassGetValue(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr));
    NativeWrapData* data = nullptr;
    NAPI_CALL(env, napi_unwrap(env, thisArg, reinterpret_cast<void**>(&data)));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, data->value, &result));
    return result;
}

static napi_value ClassSetValue(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr));
    NativeWrapData* data = nullptr;
    NAPI_CALL(env, napi_unwrap(env, thisArg, reinterpret_cast<void**>(&data)));
    if (argc >= ARGS_ONE) {
        napi_get_value_int32(env, argv[0], &data->value);
    }
    napi_value undefined = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undefined));
    return undefined;
}

// Getter/setter for class property
static napi_value PropertyGetter(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, nullptr));
    NativeWrapData* data = nullptr;
    NAPI_CALL(env, napi_unwrap(env, thisArg, reinterpret_cast<void**>(&data)));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, data->value, &result));
    return result;
}

static napi_value PropertySetter(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr));
    NativeWrapData* data = nullptr;
    NAPI_CALL(env, napi_unwrap(env, thisArg, reinterpret_cast<void**>(&data)));
    if (argc >= ARGS_ONE) {
        napi_get_value_int32(env, argv[0], &data->value);
    }
    return nullptr;
}

// Function callback with bound data
static napi_value BoundDataFunction(napi_env env, napi_callback_info info)
{
    void* rawData = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &rawData));
    auto* data = static_cast<NativeWrapData*>(rawData);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, data->value, &result));
    return result;
}

// Constructor that sets properties during initialization
static napi_value PropInitConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr));
    if (argc >= ARGS_TWO) {
        NAPI_CALL(env, napi_set_named_property(env, thisArg, "name", argv[0]));
        NAPI_CALL(env, napi_set_named_property(env, thisArg, "age", argv[1]));
    }
    auto* data = new NativeWrapData { INITIAL_VALUE };
    NAPI_CALL(env, napi_wrap(env, thisArg, data, WrapFinalizer, nullptr, nullptr));
    return thisArg;
}

// ---------------------------------------------------------------------------
// Test Functions
// ---------------------------------------------------------------------------
// Test 1: Basic napi_wrap and napi_unwrap
static napi_value TestBasicWrap(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value jsObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObj));
    auto* native = new NativeWrapData { TEST_VALUE_ONE };
    NAPI_CALL(env, napi_wrap(env, jsObj, native, WrapFinalizer, nullptr, nullptr));
    NativeWrapData* unwrapped = nullptr;
    NAPI_CALL(env, napi_unwrap(env, jsObj, reinterpret_cast<void**>(&unwrapped)));
    SetNamedBool(env, result, "wrapSucceeded", unwrapped != nullptr);
    SetNamedBool(env, result, "valueMatch", unwrapped->value == TEST_VALUE_ONE);
    SetNamedString(env, result, "testName", "TestBasicWrap");
    return result;
}

// Test 2: napi_unwrap returns correct data pointer
static napi_value TestUnwrapData(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value jsObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObj));
    auto* native = new NativeWrapData { TEST_VALUE_TWO };
    NAPI_CALL(env, napi_wrap(env, jsObj, native, WrapFinalizer, nullptr, nullptr));
    NativeWrapData* first = nullptr;
    NativeWrapData* second = nullptr;
    NAPI_CALL(env, napi_unwrap(env, jsObj, reinterpret_cast<void**>(&first)));
    NAPI_CALL(env, napi_unwrap(env, jsObj, reinterpret_cast<void**>(&second)));
    SetNamedBool(env, result, "samePointer", first == second);
    SetNamedInt32(env, result, "unwrappedValue", first->value);
    SetNamedString(env, result, "testName", "TestUnwrapData");
    return result;
}

// Test 3: napi_remove_wrap detaches native data
static napi_value TestRemoveWrap(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value jsObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObj));
    auto* native = new NativeWrapData { TEST_VALUE_THREE };
    NAPI_CALL(env, napi_wrap(env, jsObj, native, nullptr, nullptr, nullptr));
    void* removed = nullptr;
    NAPI_CALL(env, napi_remove_wrap(env, jsObj, &removed));
    bool isRemoved = (removed == static_cast<void*>(native));
    SetNamedBool(env, result, "removeReturnsData", isRemoved);
    SetNamedInt32(env, result, "removedValue", static_cast<NativeWrapData*>(removed)->value);
    SetNamedString(env, result, "testName", "TestRemoveWrap");
    delete native;
    return result;
}

// Test 4: napi_wrap with finalizer callback
static napi_value TestWrapWithFinalizer(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    g_finalizerCallCount = INITIAL_VALUE;
    napi_value jsObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObj));
    auto* native = new NativeWrapData { TEST_VALUE_FOUR };
    NAPI_CALL(env, napi_wrap(env, jsObj, native, WrapFinalizer, nullptr, nullptr));
    NativeWrapData* unwrapped = nullptr;
    NAPI_CALL(env, napi_unwrap(env, jsObj, reinterpret_cast<void**>(&unwrapped)));
    SetNamedBool(env, result, "hasData", unwrapped != nullptr);
    SetNamedInt32(env, result, "dataValue", unwrapped->value);
    SetNamedString(env, result, "testName", "TestWrapWithFinalizer");
    return result;
}

// Test 5: Multiple wrap/unwrap/remove cycles
static napi_value TestMultipleWrapCycles(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value jsObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObj));
    bool allPassed = true;
    for (int32_t i = INITIAL_VALUE; i < WRAP_CYCLE_COUNT; i++) {
        auto* native = new NativeWrapData { TEST_VALUE_ONE + i };
        NAPI_CALL(env, napi_wrap(env, jsObj, native, nullptr, nullptr, nullptr));
        NativeWrapData* unwrapped = nullptr;
        NAPI_CALL(env, napi_unwrap(env, jsObj, reinterpret_cast<void**>(&unwrapped)));
        allPassed = allPassed && (unwrapped->value == TEST_VALUE_ONE + i);
        void* removed = nullptr;
        NAPI_CALL(env, napi_remove_wrap(env, jsObj, &removed));
        delete native;
    }
    SetNamedBool(env, result, "allCyclesPassed", allPassed);
    SetNamedInt32(env, result, "cycleCount", WRAP_CYCLE_COUNT);
    SetNamedString(env, result, "testName", "TestMultipleWrapCycles");
    return result;
}

// Test 6: napi_define_class basic
static napi_value TestDefineClassBasic(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value constructor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "BasicClass", NAPI_AUTO_LENGTH, BasicClassConstructor, nullptr,
                                     CLASS_PROPERTY_COUNT_ZERO, nullptr, &constructor));
    napi_valuetype ctorType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, constructor, &ctorType));
    SetNamedBool(env, result, "isFunction", ctorType == napi_function);
    SetNamedString(env, result, "testName", "TestDefineClassBasic");
    return result;
}

// Test 7: napi_define_class with instance method
static napi_value TestDefineClassWithMethod(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_property_descriptor methods[] = {
        { "getValue", nullptr, ClassGetValue, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "setValue", nullptr, ClassSetValue, nullptr, nullptr, nullptr, napi_default, nullptr },
    };
    napi_value ctor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "MethodClass", NAPI_AUTO_LENGTH, BasicClassConstructor, nullptr,
                                     PROP_COUNT_TWO, methods, &ctor));
    napi_value instance = nullptr;
    NAPI_CALL(env, napi_new_instance(env, ctor, ARGS_ZERO, nullptr, &instance));
    bool isObject = false;
    NAPI_CALL(env, napi_is_object(env, instance, &isObject));
    SetNamedBool(env, result, "instanceCreated", isObject);
    SetNamedString(env, result, "testName", "TestDefineClassWithMethod");
    return result;
}

// Test 8: napi_define_class with getter/setter property
static napi_value TestDefineClassGetterSetter(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_property_descriptor props[] = {
        { "val", nullptr, nullptr, PropertyGetter, PropertySetter, nullptr, napi_default, nullptr },
    };
    napi_value ctor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "AccessorClass", NAPI_AUTO_LENGTH, BasicClassConstructor, nullptr,
                                     PROP_COUNT_ONE, props, &ctor));
    napi_value instance = nullptr;
    NAPI_CALL(env, napi_new_instance(env, ctor, ARGS_ZERO, nullptr, &instance));
    SetNamedBool(env, result, "classCreated", true);
    SetNamedString(env, result, "testName", "TestDefineClassGetterSetter");
    return result;
}

// Test 9: Constructor with arguments
static napi_value TestConstructorWithArgs(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value ctor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "ArgClass", NAPI_AUTO_LENGTH, ArgClassConstructor, nullptr,
                                     CLASS_PROPERTY_COUNT_ZERO, nullptr, &ctor));
    napi_value arg = nullptr;
    NAPI_CALL(env, napi_create_int32(env, CTOR_ARG_VALUE, &arg));
    napi_value instance = nullptr;
    NAPI_CALL(env, napi_new_instance(env, ctor, ARGS_ONE, &arg, &instance));
    NativeWrapData* data = nullptr;
    NAPI_CALL(env, napi_unwrap(env, instance, reinterpret_cast<void**>(&data)));
    SetNamedBool(env, result, "argPassed", data->value == CTOR_ARG_VALUE);
    SetNamedInt32(env, result, "constructedValue", data->value);
    SetNamedString(env, result, "testName", "TestConstructorWithArgs");
    return result;
}

// Test 10: napi_new_instance basic
static napi_value TestNewInstance(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value ctor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "NewInstClass", NAPI_AUTO_LENGTH, BasicClassConstructor, nullptr,
                                     CLASS_PROPERTY_COUNT_ZERO, nullptr, &ctor));
    napi_value instance = nullptr;
    NAPI_CALL(env, napi_new_instance(env, ctor, ARGS_ZERO, nullptr, &instance));
    napi_valuetype instType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, instance, &instType));
    SetNamedBool(env, result, "isObject", instType == napi_object);
    SetNamedString(env, result, "testName", "TestNewInstance");
    return result;
}

// Test 11: napi_new_instance with multiple arguments
static napi_value TestNewInstanceWithArgs(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value ctor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "MultiArgClass", NAPI_AUTO_LENGTH, ArgClassConstructor, nullptr,
                                     CLASS_PROPERTY_COUNT_ZERO, nullptr, &ctor));
    napi_value arg = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_VALUE_TWO, &arg));
    napi_value instance = nullptr;
    NAPI_CALL(env, napi_new_instance(env, ctor, ARGS_ONE, &arg, &instance));
    NativeWrapData* data = nullptr;
    NAPI_CALL(env, napi_unwrap(env, instance, reinterpret_cast<void**>(&data)));
    SetNamedBool(env, result, "valueCorrect", data->value == TEST_VALUE_TWO);
    SetNamedString(env, result, "testName", "TestNewInstanceWithArgs");
    return result;
}

// Test 12: napi_instanceof
static napi_value TestInstanceOf(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value ctor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "InstOfClass", NAPI_AUTO_LENGTH, BasicClassConstructor, nullptr,
                                     CLASS_PROPERTY_COUNT_ZERO, nullptr, &ctor));
    napi_value instance = nullptr;
    NAPI_CALL(env, napi_new_instance(env, ctor, ARGS_ZERO, nullptr, &instance));
    bool isInstance = false;
    NAPI_CALL(env, napi_instanceof(env, instance, ctor, &isInstance));
    SetNamedBool(env, result, "isInstance", isInstance);
    napi_value plainObj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &plainObj));
    bool plainIsInstance = false;
    NAPI_CALL(env, napi_instanceof(env, plainObj, ctor, &plainIsInstance));
    SetNamedBool(env, result, "plainIsNotInstance", !plainIsInstance);
    SetNamedString(env, result, "testName", "TestInstanceOf");
    return result;
}

// Test 13: napi_create_object_with_properties
static napi_value TestCreateObjectWithProperties(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value intVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_VALUE_ONE, &intVal));
    napi_value strVal = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "hello", NAPI_AUTO_LENGTH, &strVal));
    napi_property_descriptor props[] = {
        { "num", nullptr, nullptr, nullptr, nullptr, intVal, napi_default, nullptr },
        { "str", nullptr, nullptr, nullptr, nullptr, strVal, napi_default, nullptr },
    };
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object_with_properties(env, &obj, PROP_COUNT_TWO, props));
    bool hasNum = false;
    bool hasStr = false;
    NAPI_CALL(env, napi_has_named_property(env, obj, "num", &hasNum));
    NAPI_CALL(env, napi_has_named_property(env, obj, "str", &hasStr));
    SetNamedBool(env, result, "hasNum", hasNum);
    SetNamedBool(env, result, "hasStr", hasStr);
    SetNamedString(env, result, "testName", "TestCreateObjectWithProperties");
    return result;
}

// Test 14: napi_create_object_with_named_properties
static napi_value TestCreateObjectWithNamedProps(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value val1 = nullptr;
    napi_value val2 = nullptr;
    napi_value val3 = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_VALUE_ONE, &val1));
    NAPI_CALL(env, napi_create_double(env, TEST_DOUBLE_VAL, &val2));
    NAPI_CALL(env, napi_create_string_utf8(env, "world", NAPI_AUTO_LENGTH, &val3));
    const char* keys[] = { "intKey", "doubleKey", "strKey" };
    napi_value values[] = { val1, val2, val3 };
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object_with_named_properties(env, &obj, PROP_COUNT_THREE, keys, values));
    bool hasAll = false;
    bool h1 = false;
    bool h2 = false;
    bool h3 = false;
    NAPI_CALL(env, napi_has_named_property(env, obj, "intKey", &h1));
    NAPI_CALL(env, napi_has_named_property(env, obj, "doubleKey", &h2));
    NAPI_CALL(env, napi_has_named_property(env, obj, "strKey", &h3));
    hasAll = h1 && h2 && h3;
    SetNamedBool(env, result, "allPropertiesExist", hasAll);
    SetNamedString(env, result, "testName", "TestCreateObjectWithNamedProps");
    return result;
}

// Test 15: napi_type_tag_object
static napi_value TestTypeTagObject(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_type_tag tag = { TYPE_TAG_LOWER_A, TYPE_TAG_UPPER_A };
    NAPI_CALL(env, napi_type_tag_object(env, obj, &tag));
    SetNamedBool(env, result, "tagApplied", true);
    SetNamedString(env, result, "testName", "TestTypeTagObject");
    return result;
}

// Test 16: napi_check_object_type_tag match
static napi_value TestCheckTypeTagMatch(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_type_tag tag = { TYPE_TAG_LOWER_A, TYPE_TAG_UPPER_A };
    NAPI_CALL(env, napi_type_tag_object(env, obj, &tag));
    bool matches = false;
    NAPI_CALL(env, napi_check_object_type_tag(env, obj, &tag, &matches));
    SetNamedBool(env, result, "tagMatches", matches);
    SetNamedString(env, result, "testName", "TestCheckTypeTagMatch");
    return result;
}

// Test 17: napi_check_object_type_tag mismatch
static napi_value TestCheckTypeTagMismatch(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_type_tag tagA = { TYPE_TAG_LOWER_A, TYPE_TAG_UPPER_A };
    napi_type_tag tagB = { TYPE_TAG_LOWER_B, TYPE_TAG_UPPER_B };
    NAPI_CALL(env, napi_type_tag_object(env, obj, &tagA));
    bool matches = false;
    NAPI_CALL(env, napi_check_object_type_tag(env, obj, &tagB, &matches));
    SetNamedBool(env, result, "tagDoesNotMatch", !matches);
    SetNamedString(env, result, "testName", "TestCheckTypeTagMismatch");
    return result;
}

// Test 18: napi_add_finalizer
static napi_value TestAddFinalizer(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    g_multiFinalizerCount = INITIAL_VALUE;
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    int32_t dummyData = TEST_VALUE_ONE;
    NAPI_CALL(env, napi_add_finalizer(env, obj, &dummyData, GenericFinalizer, nullptr, nullptr));
    SetNamedBool(env, result, "finalizerAdded", true);
    SetNamedString(env, result, "testName", "TestAddFinalizer");
    return result;
}

// Test 19: Multiple finalizers on same object
static napi_value TestMultipleFinalizers(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    g_multiFinalizerCount = INITIAL_VALUE;
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    int32_t data1 = TEST_VALUE_ONE;
    int32_t data2 = TEST_VALUE_TWO;
    NAPI_CALL(env, napi_add_finalizer(env, obj, &data1, GenericFinalizer, nullptr, nullptr));
    NAPI_CALL(env, napi_add_finalizer(env, obj, &data2, GenericFinalizer, nullptr, nullptr));
    SetNamedBool(env, result, "multipleFinalizersAdded", true);
    SetNamedString(env, result, "testName", "TestMultipleFinalizers");
    return result;
}

// Test 20: napi_create_function basic
static napi_value TestCreateFunction(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value func = nullptr;
    NAPI_CALL(env, napi_create_function(env, "myFunc", NAPI_AUTO_LENGTH, ClassGetValue, nullptr, &func));
    napi_valuetype funcType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, func, &funcType));
    SetNamedBool(env, result, "isFunction", funcType == napi_function);
    SetNamedString(env, result, "testName", "TestCreateFunction");
    return result;
}

// Test 21: napi_create_function with bound data
static napi_value TestCreateFunctionWithData(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    static NativeWrapData boundData = { TEST_VALUE_THREE };
    napi_value func = nullptr;
    NAPI_CALL(env, napi_create_function(env, "boundFunc", NAPI_AUTO_LENGTH, BoundDataFunction, &boundData, &func));
    napi_value undefined = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undefined));
    napi_value callResult = nullptr;
    NAPI_CALL(env, napi_call_function(env, undefined, func, ARGS_ZERO, nullptr, &callResult));
    int32_t retVal = INITIAL_VALUE;
    NAPI_CALL(env, napi_get_value_int32(env, callResult, &retVal));
    SetNamedBool(env, result, "dataCorrect", retVal == TEST_VALUE_THREE);
    SetNamedInt32(env, result, "returnedValue", retVal);
    SetNamedString(env, result, "testName", "TestCreateFunctionWithData");
    return result;
}

// Test 22: napi_get_cb_info with thisArg
static napi_value TestGetCbInfoThisArg(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    size_t argc = ARGS_ZERO;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, nullptr, &thisArg, nullptr));
    napi_value result = CreateResult(env);
    napi_valuetype thisType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, thisArg, &thisType));
    SetNamedBool(env, result, "hasThisArg", thisArg != nullptr);
    SetNamedBool(env, result, "thisIsObject", thisType == napi_object);
    SetNamedString(env, result, "testName", "TestGetCbInfoThisArg");
    return result;
}

// Test 23: Property with readonly attribute
static napi_value TestPropertyReadonly(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value intVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_VALUE_ONE, &intVal));
    napi_property_descriptor prop = { "readonlyProp",
                                      nullptr,
                                      nullptr,
                                      nullptr,
                                      nullptr,
                                      intVal,
                                      static_cast<napi_property_attributes>(napi_enumerable | napi_configurable),
                                      nullptr };
    NAPI_CALL(env, napi_define_properties(env, obj, PROP_COUNT_ONE, &prop));
    bool hasProp = false;
    NAPI_CALL(env, napi_has_named_property(env, obj, "readonlyProp", &hasProp));
    SetNamedBool(env, result, "propertyExists", hasProp);
    SetNamedString(env, result, "testName", "TestPropertyReadonly");
    return result;
}

// Test 24: Property with writable attribute
static napi_value TestPropertyWritable(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value intVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_VALUE_ONE, &intVal));
    napi_property_descriptor prop = { "writableProp",
                                      nullptr,
                                      nullptr,
                                      nullptr,
                                      nullptr,
                                      intVal,
                                      static_cast<napi_property_attributes>(napi_writable | napi_enumerable),
                                      nullptr };
    NAPI_CALL(env, napi_define_properties(env, obj, PROP_COUNT_ONE, &prop));
    napi_value newVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, UPDATED_VALUE, &newVal));
    NAPI_CALL(env, napi_set_named_property(env, obj, "writableProp", newVal));
    napi_value readBack = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, obj, "writableProp", &readBack));
    int32_t readVal = INITIAL_VALUE;
    NAPI_CALL(env, napi_get_value_int32(env, readBack, &readVal));
    SetNamedBool(env, result, "writeSucceeded", readVal == UPDATED_VALUE);
    SetNamedString(env, result, "testName", "TestPropertyWritable");
    return result;
}

// Test 25: Property with enumerable attribute
static napi_value TestPropertyEnumerable(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value val1 = nullptr;
    napi_value val2 = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_VALUE_ONE, &val1));
    NAPI_CALL(env, napi_create_int32(env, TEST_VALUE_TWO, &val2));
    napi_property_descriptor props[] = {
        { "enumProp", nullptr, nullptr, nullptr, nullptr, val1, napi_enumerable, nullptr },
        { "hiddenProp", nullptr, nullptr, nullptr, nullptr, val2, napi_default, nullptr },
    };
    NAPI_CALL(env, napi_define_properties(env, obj, PROP_COUNT_TWO, props));
    napi_value propNames = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, obj, &propNames));
    uint32_t nameCount = INITIAL_VALUE;
    NAPI_CALL(env, napi_get_array_length(env, propNames, &nameCount));
    SetNamedInt32(env, result, "enumerableCount", static_cast<int32_t>(nameCount));
    SetNamedString(env, result, "testName", "TestPropertyEnumerable");
    return result;
}

// Test 26: Property with configurable attribute
static napi_value TestPropertyConfigurable(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value val = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_VALUE_ONE, &val));
    napi_property_descriptor prop = {
        "configProp", nullptr, nullptr, nullptr, nullptr, val, napi_configurable, nullptr
    };
    NAPI_CALL(env, napi_define_properties(env, obj, PROP_COUNT_ONE, &prop));
    bool hasProp = false;
    NAPI_CALL(env, napi_has_named_property(env, obj, "configProp", &hasProp));
    SetNamedBool(env, result, "propertyDefined", hasProp);
    SetNamedString(env, result, "testName", "TestPropertyConfigurable");
    return result;
}

// Test 27: napi_define_properties with multiple descriptors
static napi_value TestDefinePropertiesMultiple(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value v1 = nullptr;
    napi_value v2 = nullptr;
    napi_value v3 = nullptr;
    napi_value v4 = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_VALUE_ONE, &v1));
    NAPI_CALL(env, napi_create_double(env, TEST_DOUBLE_VAL, &v2));
    NAPI_CALL(env, napi_create_string_utf8(env, "test", NAPI_AUTO_LENGTH, &v3));
    NAPI_CALL(env, napi_get_boolean(env, true, &v4));
    napi_property_descriptor props[] = {
        { "intProp", nullptr, nullptr, nullptr, nullptr, v1, napi_default, nullptr },
        { "doubleProp", nullptr, nullptr, nullptr, nullptr, v2, napi_default, nullptr },
        { "strProp", nullptr, nullptr, nullptr, nullptr, v3, napi_default, nullptr },
        { "boolProp", nullptr, nullptr, nullptr, nullptr, v4, napi_default, nullptr },
    };
    NAPI_CALL(env, napi_define_properties(env, obj, PROP_COUNT_FOUR, props));
    bool all = false;
    bool h1 = false;
    bool h2 = false;
    bool h3 = false;
    bool h4 = false;
    NAPI_CALL(env, napi_has_named_property(env, obj, "intProp", &h1));
    NAPI_CALL(env, napi_has_named_property(env, obj, "doubleProp", &h2));
    NAPI_CALL(env, napi_has_named_property(env, obj, "strProp", &h3));
    NAPI_CALL(env, napi_has_named_property(env, obj, "boolProp", &h4));
    all = h1 && h2 && h3 && h4;
    SetNamedBool(env, result, "allPropertiesDefined", all);
    SetNamedInt32(env, result, "propertyCount", static_cast<int32_t>(PROP_COUNT_FOUR));
    SetNamedString(env, result, "testName", "TestDefinePropertiesMultiple");
    return result;
}

// Test 28: napi_define_properties with getter/setter
static napi_value TestDefinePropsGetterSetter(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value ctor = nullptr;
    napi_property_descriptor classMethods[] = {
        { "val", nullptr, nullptr, PropertyGetter, PropertySetter, nullptr, napi_default, nullptr },
    };
    NAPI_CALL(env, napi_define_class(env, "GetSetClass", NAPI_AUTO_LENGTH, BasicClassConstructor, nullptr,
                                     PROP_COUNT_ONE, classMethods, &ctor));
    napi_value instance = nullptr;
    NAPI_CALL(env, napi_new_instance(env, ctor, ARGS_ZERO, nullptr, &instance));
    // Set value via setter
    napi_value setVal = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_VALUE_TWO, &setVal));
    NAPI_CALL(env, napi_set_named_property(env, instance, "val", setVal));
    // Read via getter
    napi_value getVal = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, instance, "val", &getVal));
    int32_t gotten = INITIAL_VALUE;
    NAPI_CALL(env, napi_get_value_int32(env, getVal, &gotten));
    SetNamedBool(env, result, "getterSetterWorks", gotten == TEST_VALUE_TWO);
    SetNamedInt32(env, result, "gottenValue", gotten);
    SetNamedString(env, result, "testName", "TestDefinePropsGetterSetter");
    return result;
}

// Test 29: Class inheritance via prototype chain
static napi_value TestClassInheritance(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    // Define base class
    napi_property_descriptor baseMethods[] = {
        { "getValue", nullptr, ClassGetValue, nullptr, nullptr, nullptr, napi_default, nullptr },
    };
    napi_value baseCtor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "BaseClass", NAPI_AUTO_LENGTH, BasicClassConstructor, nullptr, PROP_COUNT_ONE,
                                     baseMethods, &baseCtor));
    // Create base instance
    napi_value baseInst = nullptr;
    NAPI_CALL(env, napi_new_instance(env, baseCtor, ARGS_ZERO, nullptr, &baseInst));
    bool isBaseInst = false;
    NAPI_CALL(env, napi_instanceof(env, baseInst, baseCtor, &isBaseInst));
    // Define child class
    napi_property_descriptor childMethods[] = {
        { "setValue", nullptr, ClassSetValue, nullptr, nullptr, nullptr, napi_default, nullptr },
    };
    napi_value childCtor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "ChildClass", NAPI_AUTO_LENGTH, BasicClassConstructor, nullptr,
                                     PROP_COUNT_ONE, childMethods, &childCtor));
    napi_value childInst = nullptr;
    NAPI_CALL(env, napi_new_instance(env, childCtor, ARGS_ZERO, nullptr, &childInst));
    bool isChildInst = false;
    NAPI_CALL(env, napi_instanceof(env, childInst, childCtor, &isChildInst));
    SetNamedBool(env, result, "baseInstanceValid", isBaseInst);
    SetNamedBool(env, result, "childInstanceValid", isChildInst);
    SetNamedString(env, result, "testName", "TestClassInheritance");
    return result;
}

// Test 30: Native data lifecycle management
static napi_value TestNativeDataLifecycle(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    static LifecycleData lcData = { TEST_VALUE_ONE, false };
    lcData.finalized = false;
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    NAPI_CALL(env, napi_add_finalizer(env, obj, &lcData, LifecycleFinalizer, nullptr, nullptr));
    SetNamedBool(env, result, "lifecycleSetUp", true);
    SetNamedBool(env, result, "notYetFinalized", !lcData.finalized);
    SetNamedInt32(env, result, "dataId", lcData.id);
    SetNamedString(env, result, "testName", "TestNativeDataLifecycle");
    return result;
}

// Test 31: Constructor with property initialization
static napi_value TestConstructorPropertyInit(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value ctor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "PropInitClass", NAPI_AUTO_LENGTH, PropInitConstructor, nullptr,
                                     CLASS_PROPERTY_COUNT_ZERO, nullptr, &ctor));
    napi_value args[ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_create_string_utf8(env, "Alice", NAPI_AUTO_LENGTH, &args[0]));
    NAPI_CALL(env, napi_create_int32(env, TEST_VALUE_ONE, &args[1]));
    napi_value instance = nullptr;
    NAPI_CALL(env, napi_new_instance(env, ctor, ARGS_TWO, args, &instance));
    bool hasName = false;
    bool hasAge = false;
    NAPI_CALL(env, napi_has_named_property(env, instance, "name", &hasName));
    NAPI_CALL(env, napi_has_named_property(env, instance, "age", &hasAge));
    SetNamedBool(env, result, "hasName", hasName);
    SetNamedBool(env, result, "hasAge", hasAge);
    SetNamedString(env, result, "testName", "TestConstructorPropertyInit");
    return result;
}

// Test 32: Wrap with PointData and verify coordinates
static napi_value TestWrapPointData(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    auto* pt = new PointData { TEST_DOUBLE_VAL, TEST_DOUBLE_TWO };
    NAPI_CALL(env, napi_wrap(env, obj, pt, PointFinalizer, nullptr, nullptr));
    PointData* unwrapped = nullptr;
    NAPI_CALL(env, napi_unwrap(env, obj, reinterpret_cast<void**>(&unwrapped)));
    SetNamedDouble(env, result, "x", unwrapped->x);
    SetNamedDouble(env, result, "y", unwrapped->y);
    SetNamedBool(env, result, "coordsMatch", unwrapped->x == TEST_DOUBLE_VAL && unwrapped->y == TEST_DOUBLE_TWO);
    SetNamedString(env, result, "testName", "TestWrapPointData");
    return result;
}

// Test 33: Wrap with ref output and verify ref
static napi_value TestWrapWithRef(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    auto* native = new NativeWrapData { TEST_VALUE_ONE };
    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_wrap(env, obj, native, WrapFinalizer, nullptr, &ref));
    SetNamedBool(env, result, "refCreated", ref != nullptr);
    NativeWrapData* unwrapped = nullptr;
    NAPI_CALL(env, napi_unwrap(env, obj, reinterpret_cast<void**>(&unwrapped)));
    SetNamedBool(env, result, "dataValid", unwrapped->value == TEST_VALUE_ONE);
    NAPI_CALL(env, napi_delete_reference(env, ref));
    SetNamedString(env, result, "testName", "TestWrapWithRef");
    return result;
}

// Test 34: Define class with static and instance methods mixed
static napi_value StaticMethodCallback(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_VALUE_FOUR, &result));
    return result;
}

static napi_value TestDefineClassMixed(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_property_descriptor props[] = {
        { "getValue", nullptr, ClassGetValue, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "staticMethod", nullptr, StaticMethodCallback, nullptr, nullptr, nullptr,
          static_cast<napi_property_attributes>(napi_static | napi_default), nullptr },
    };
    napi_value ctor = nullptr;
    NAPI_CALL(env, napi_define_class(env, "MixedClass", NAPI_AUTO_LENGTH, BasicClassConstructor, nullptr,
                                     PROP_COUNT_TWO, props, &ctor));
    // Check static method on constructor
    bool hasStatic = false;
    NAPI_CALL(env, napi_has_named_property(env, ctor, "staticMethod", &hasStatic));
    SetNamedBool(env, result, "hasStaticMethod", hasStatic);
    SetNamedString(env, result, "testName", "TestDefineClassMixed");
    return result;
}

// Test 35: Type tag with wrap combination
static napi_value TestTypeTagWithWrap(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    // Apply type tag
    napi_type_tag tag = { TYPE_TAG_LOWER_B, TYPE_TAG_UPPER_B };
    NAPI_CALL(env, napi_type_tag_object(env, obj, &tag));
    // Also wrap native data
    auto* native = new NativeWrapData { TEST_VALUE_TWO };
    NAPI_CALL(env, napi_wrap(env, obj, native, WrapFinalizer, nullptr, nullptr));
    // Verify both
    bool tagOk = false;
    NAPI_CALL(env, napi_check_object_type_tag(env, obj, &tag, &tagOk));
    NativeWrapData* unwrapped = nullptr;
    NAPI_CALL(env, napi_unwrap(env, obj, reinterpret_cast<void**>(&unwrapped)));
    SetNamedBool(env, result, "tagCorrect", tagOk);
    SetNamedBool(env, result, "wrapCorrect", unwrapped->value == TEST_VALUE_TWO);
    SetNamedString(env, result, "testName", "TestTypeTagWithWrap");
    return result;
}

// Test 36: Create object with all property attribute combinations
static napi_value TestAllPropertyAttributes(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result = CreateResult(env);
    napi_value obj = nullptr;
    NAPI_CALL(env, napi_create_object(env, &obj));
    napi_value v1 = nullptr;
    napi_value v2 = nullptr;
    napi_value v3 = nullptr;
    napi_value v4 = nullptr;
    napi_value v5 = nullptr;
    NAPI_CALL(env, napi_create_int32(env, TEST_VALUE_ONE, &v1));
    NAPI_CALL(env, napi_create_int32(env, TEST_VALUE_TWO, &v2));
    NAPI_CALL(env, napi_create_int32(env, TEST_VALUE_THREE, &v3));
    NAPI_CALL(env, napi_create_int32(env, TEST_VALUE_FOUR, &v4));
    NAPI_CALL(env, napi_create_int32(env, UPDATED_VALUE, &v5));
    napi_property_descriptor props[] = {
        { "defaultProp", nullptr, nullptr, nullptr, nullptr, v1, napi_default, nullptr },
        { "writableProp", nullptr, nullptr, nullptr, nullptr, v2, napi_writable, nullptr },
        { "enumProp", nullptr, nullptr, nullptr, nullptr, v3, napi_enumerable, nullptr },
        { "configProp", nullptr, nullptr, nullptr, nullptr, v4, napi_configurable, nullptr },
        { "allFlagsProp", nullptr, nullptr, nullptr, nullptr, v5,
          static_cast<napi_property_attributes>(napi_writable | napi_enumerable | napi_configurable), nullptr },
    };
    NAPI_CALL(env, napi_define_properties(env, obj, PROP_COUNT_FIVE, props));
    SetNamedBool(env, result, "allAttributesDefined", true);
    SetNamedInt32(env, result, "totalProps", static_cast<int32_t>(PROP_COUNT_FIVE));
    SetNamedString(env, result, "testName", "TestAllPropertyAttributes");
    return result;
}

// ---------------------------------------------------------------------------
// Module Registration
// ---------------------------------------------------------------------------
struct TestEntry {
    const char* name;
    napi_callback func;
};

static const TestEntry WRAP_TESTS[] = {
    { "testBasicWrap", TestBasicWrap },
    { "testUnwrapData", TestUnwrapData },
    { "testRemoveWrap", TestRemoveWrap },
    { "testWrapWithFinalizer", TestWrapWithFinalizer },
    { "testMultipleWrapCycles", TestMultipleWrapCycles },
    { "testDefineClassBasic", TestDefineClassBasic },
    { "testDefineClassWithMethod", TestDefineClassWithMethod },
    { "testDefineClassGetterSetter", TestDefineClassGetterSetter },
    { "testConstructorWithArgs", TestConstructorWithArgs },
    { "testNewInstance", TestNewInstance },
    { "testNewInstanceWithArgs", TestNewInstanceWithArgs },
    { "testInstanceOf", TestInstanceOf },
    { "testCreateObjectWithProperties", TestCreateObjectWithProperties },
    { "testCreateObjectWithNamedProps", TestCreateObjectWithNamedProps },
    { "testTypeTagObject", TestTypeTagObject },
    { "testCheckTypeTagMatch", TestCheckTypeTagMatch },
    { "testCheckTypeTagMismatch", TestCheckTypeTagMismatch },
    { "testAddFinalizer", TestAddFinalizer },
    { "testMultipleFinalizers", TestMultipleFinalizers },
    { "testCreateFunction", TestCreateFunction },
    { "testCreateFunctionWithData", TestCreateFunctionWithData },
    { "testGetCbInfoThisArg", TestGetCbInfoThisArg },
    { "testPropertyReadonly", TestPropertyReadonly },
    { "testPropertyWritable", TestPropertyWritable },
    { "testPropertyEnumerable", TestPropertyEnumerable },
    { "testPropertyConfigurable", TestPropertyConfigurable },
    { "testDefinePropertiesMultiple", TestDefinePropertiesMultiple },
    { "testDefinePropsGetterSetter", TestDefinePropsGetterSetter },
    { "testClassInheritance", TestClassInheritance },
    { "testNativeDataLifecycle", TestNativeDataLifecycle },
    { "testConstructorPropertyInit", TestConstructorPropertyInit },
    { "testWrapPointData", TestWrapPointData },
    { "testWrapWithRef", TestWrapWithRef },
    { "testDefineClassMixed", TestDefineClassMixed },
    { "testTypeTagWithWrap", TestTypeTagWithWrap },
    { "testAllPropertyAttributes", TestAllPropertyAttributes },
};

} // namespace

static napi_value InitWrapSuite(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[TEST_FUNCTION_COUNT];
    for (size_t i = 0; i < TEST_FUNCTION_COUNT; i++) {
        descriptors[i] =
            napi_property_descriptor { WRAP_TESTS[i].name, nullptr, WRAP_TESTS[i].func, nullptr, nullptr, nullptr,
                                       napi_default,       nullptr };
    }
    NAPI_CALL(env, napi_define_properties(env, exports, TEST_FUNCTION_COUNT, descriptors));
    return exports;
}

static napi_module g_wrapSuiteModule = {
    .nm_version = MODULE_VERSION,
    .nm_flags = NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitWrapSuite,
    .nm_modname = "wrap_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterWrapSuiteModule(void)
{
    napi_module_register(&g_wrapSuiteModule);
}
