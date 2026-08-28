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

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

// 匿名命名空间，用于定义文件级常量及 UPPER_CASE 常量
namespace {
constexpr int32_t INITIAL_CALL_COUNT = 0;
constexpr size_t TEST_PROPERTY_COUNT_ZERO = 0;
constexpr size_t TEST_PROPERTY_COUNT_ONE = 1;
constexpr size_t TEST_PROPERTY_COUNT_TWO = 2;
constexpr size_t TEST_ARRAY_LENGTH_ZERO = 0;
constexpr size_t TEST_ARRAY_LENGTH_TEN = 10;
constexpr size_t TEST_BUFFER_SIZE_ZERO = 0;
constexpr size_t TEST_BUFFER_SIZE_TEN = 10;
constexpr size_t TEST_BUFFER_SIZE_HUNDRED = 100;
constexpr size_t TEST_BUFFER_SIZE_HUGE = 1048576;
constexpr size_t TEST_NATIVE_BINDING_SIZE = 1024;
constexpr int32_t TEST_INT_VAL_FORTY_TWO = 42;
constexpr int32_t TEST_INT_VAL_NINETY_NINE = 99;
} // namespace

static int32_t g_finalizerCallCount = INITIAL_CALL_COUNT;

// 构造函数回调，用于 napi_define_sendable_class
static napi_value ConstructorCallback(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVal, nullptr));
    return thisVal;
}

// 释放回调，用于 napi_wrap_sendable
static void DummyFinalizer(napi_env env, void* data, void* hint)
{
    (void)env;
    (void)hint;
    if (data != nullptr) {
        auto* ptr = static_cast<int32_t*>(data);
        delete ptr;
    }
    g_finalizerCallCount++;
}

// 构建统一测试返回对象的辅助函数
static napi_value CreateTestResult(napi_env env, bool success, const char* message)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    napi_value successVal = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, success, &successVal));
    NAPI_CALL(env, napi_set_named_property(env, result, "success", successVal));
    napi_value msgVal = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, message, NAPI_AUTO_LENGTH, &msgVal));
    NAPI_CALL(env, napi_set_named_property(env, result, "message", msgVal));
    return result;
}

// 辅助检测对象是否为 sendable
static bool CheckIsSendable(napi_env env, napi_value value, bool expected)
{
    bool actual = false;
    napi_status status = napi_is_sendable(env, value, &actual);
    return (status == napi_ok) && (actual == expected);
}

// 1. TestDefineSendableClassNormal
static napi_value TestDefineSendableClassNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    napi_status status = napi_define_sendable_class(
        env,
        "SendableClassNormal",
        NAPI_AUTO_LENGTH,
        ConstructorCallback,
        nullptr,
        TEST_PROPERTY_COUNT_ZERO,
        nullptr,
        nullptr,
        &result);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "napi_define_sendable_class 正常流执行失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 2. TestDefineSendableClassWithProperties
static napi_value TestDefineSendableClassWithProperties(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value val = nullptr;
    napi_status status = napi_create_int32(env, TEST_INT_VAL_FORTY_TWO, &val);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建属性数据失败");
    }
    napi_property_descriptor properties[] = {
        { "prop", nullptr, nullptr, nullptr, nullptr, val, napi_default, nullptr }
    };
    napi_value result = nullptr;
    status = napi_define_sendable_class(
        env,
        "SendableClassWithProps",
        NAPI_AUTO_LENGTH,
        ConstructorCallback,
        nullptr,
        TEST_PROPERTY_COUNT_ONE,
        properties,
        nullptr,
        &result);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "napi_define_sendable_class 带属性执行失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 3. TestDefineSendableClassNullEnv
static napi_value TestDefineSendableClassNullEnv(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    napi_status status = napi_define_sendable_class(
        nullptr,
        "SendableClassNullEnv",
        NAPI_AUTO_LENGTH,
        ConstructorCallback,
        nullptr,
        TEST_PROPERTY_COUNT_ZERO,
        nullptr,
        nullptr,
        &result);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 4. TestDefineSendableClassNullName
static napi_value TestDefineSendableClassNullName(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    napi_status status = napi_define_sendable_class(
        env,
        nullptr,
        NAPI_AUTO_LENGTH,
        ConstructorCallback,
        nullptr,
        TEST_PROPERTY_COUNT_ZERO,
        nullptr,
        nullptr,
        &result);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 5. TestDefineSendableClassNullResult
static napi_value TestDefineSendableClassNullResult(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_status status = napi_define_sendable_class(
        env,
        "SendableClassNullResult",
        NAPI_AUTO_LENGTH,
        ConstructorCallback,
        nullptr,
        TEST_PROPERTY_COUNT_ZERO,
        nullptr,
        nullptr,
        nullptr);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 6. TestDefineSendableClassWithParent
static napi_value TestDefineSendableClassWithParent(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value parent = nullptr;
    napi_status status = napi_define_sendable_class(
        env,
        "ParentSendableClass",
        NAPI_AUTO_LENGTH,
        ConstructorCallback,
        nullptr,
        TEST_PROPERTY_COUNT_ZERO,
        nullptr,
        nullptr,
        &parent);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "定义父类失败");
    }
    napi_value child = nullptr;
    status = napi_define_sendable_class(
        env,
        "ChildSendableClass",
        NAPI_AUTO_LENGTH,
        ConstructorCallback,
        nullptr,
        TEST_PROPERTY_COUNT_ZERO,
        nullptr,
        parent,
        &child);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "定义子类失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 7. TestCreateSendableObjectWithPropertiesNormal
static napi_value TestCreateSendableObjectWithPropertiesNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value val = nullptr;
    napi_status status = napi_create_int32(env, TEST_INT_VAL_FORTY_TWO, &val);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建属性数据失败");
    }
    napi_property_descriptor properties[] = {
        { "prop", nullptr, nullptr, nullptr, nullptr, val, napi_default, nullptr }
    };
    napi_value result = nullptr;
    status = napi_create_sendable_object_with_properties(
        env,
        TEST_PROPERTY_COUNT_ONE,
        properties,
        &result);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "根据属性创建 sendable 对象失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 8. TestCreateSendableObjectWithPropertiesNullEnv
static napi_value TestCreateSendableObjectWithPropertiesNullEnv(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    napi_status status = napi_create_sendable_object_with_properties(
        nullptr,
        TEST_PROPERTY_COUNT_ZERO,
        nullptr,
        &result);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 9. TestCreateSendableObjectWithPropertiesNullResult
static napi_value TestCreateSendableObjectWithPropertiesNullResult(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_status status = napi_create_sendable_object_with_properties(
        env,
        TEST_PROPERTY_COUNT_ZERO,
        nullptr,
        nullptr);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 10. TestCreateSendableObjectWithPropertiesNullProps
static napi_value TestCreateSendableObjectWithPropertiesNullProps(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    napi_status status = napi_create_sendable_object_with_properties(
        env,
        TEST_PROPERTY_COUNT_ONE,
        nullptr,
        &result);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 11. TestCreateSendableArrayNormal
static napi_value TestCreateSendableArrayNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    napi_status status = napi_create_sendable_array(env, &result);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable 数组失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 12. TestCreateSendableArrayNullEnv
static napi_value TestCreateSendableArrayNullEnv(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    napi_status status = napi_create_sendable_array(nullptr, &result);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 13. TestCreateSendableArrayNullResult
static napi_value TestCreateSendableArrayNullResult(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_status status = napi_create_sendable_array(env, nullptr);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 14. TestCreateSendableArrayWithLengthNormal
static napi_value TestCreateSendableArrayWithLengthNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    napi_status status = napi_create_sendable_array_with_length(
        env,
        TEST_ARRAY_LENGTH_TEN,
        &result);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建指定长度的 sendable 数组失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 15. TestCreateSendableArrayWithLengthNullEnv
static napi_value TestCreateSendableArrayWithLengthNullEnv(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    napi_status status = napi_create_sendable_array_with_length(
        nullptr,
        TEST_ARRAY_LENGTH_TEN,
        &result);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 16. TestCreateSendableArrayWithLengthNullResult
static napi_value TestCreateSendableArrayWithLengthNullResult(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_status status = napi_create_sendable_array_with_length(
        env,
        TEST_ARRAY_LENGTH_TEN,
        nullptr);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 17. TestCreateSendableArrayWithLengthZero
static napi_value TestCreateSendableArrayWithLengthZero(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    napi_status status = napi_create_sendable_array_with_length(
        env,
        TEST_ARRAY_LENGTH_ZERO,
        &result);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建长度为 0 的 sendable 数组失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 18. TestCreateSendableArrayBufferNormal
static napi_value TestCreateSendableArrayBufferNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    void* data = nullptr;
    napi_value result = nullptr;
    napi_status status = napi_create_sendable_arraybuffer(
        env,
        TEST_BUFFER_SIZE_TEN,
        &data,
        &result);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable arraybuffer 失败");
    }
    if (data == nullptr) {
        return CreateTestResult(env, false, "返回的底层 data 指针为空");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 19. TestCreateSendableArrayBufferNullEnv
static napi_value TestCreateSendableArrayBufferNullEnv(napi_env env, napi_callback_info info)
{
    (void)info;
    void* data = nullptr;
    napi_value result = nullptr;
    napi_status status = napi_create_sendable_arraybuffer(
        nullptr,
        TEST_BUFFER_SIZE_TEN,
        &data,
        &result);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 20. TestCreateSendableArrayBufferNullResult
static napi_value TestCreateSendableArrayBufferNullResult(napi_env env, napi_callback_info info)
{
    (void)info;
    void* data = nullptr;
    napi_status status = napi_create_sendable_arraybuffer(
        env,
        TEST_BUFFER_SIZE_TEN,
        &data,
        nullptr);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 21. TestCreateSendableArrayBufferZeroSize
static napi_value TestCreateSendableArrayBufferZeroSize(napi_env env, napi_callback_info info)
{
    (void)info;
    void* data = nullptr;
    napi_value result = nullptr;
    napi_status status = napi_create_sendable_arraybuffer(
        env,
        TEST_BUFFER_SIZE_ZERO,
        &data,
        &result);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建大小为 0 的 sendable arraybuffer 失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 22. TestCreateSendableArrayBufferHugeSize
static napi_value TestCreateSendableArrayBufferHugeSize(napi_env env, napi_callback_info info)
{
    (void)info;
    void* data = nullptr;
    napi_value result = nullptr;
    napi_status status = napi_create_sendable_arraybuffer(
        env,
        TEST_BUFFER_SIZE_HUGE,
        &data,
        &result);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建大尺寸的 sendable arraybuffer 失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 23. TestCreateSendableTypedArrayNormal
static napi_value TestCreateSendableTypedArrayNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    void* data = nullptr;
    napi_value arraybuffer = nullptr;
    napi_status status = napi_create_sendable_arraybuffer(
        env,
        TEST_BUFFER_SIZE_HUNDRED,
        &data,
        &arraybuffer);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建底层 arraybuffer 失败");
    }
    napi_value result = nullptr;
    status = napi_create_sendable_typedarray(
        env,
        napi_int8_array,
        TEST_ARRAY_LENGTH_TEN,
        arraybuffer,
        0,
        &result);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable typedarray 失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 24. TestCreateSendableTypedArrayNullEnv
static napi_value TestCreateSendableTypedArrayNullEnv(napi_env env, napi_callback_info info)
{
    (void)info;
    void* data = nullptr;
    napi_value arraybuffer = nullptr;
    napi_status status = napi_create_sendable_arraybuffer(
        env,
        TEST_BUFFER_SIZE_HUNDRED,
        &data,
        &arraybuffer);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建底层 arraybuffer 失败");
    }
    napi_value result = nullptr;
    status = napi_create_sendable_typedarray(
        nullptr,
        napi_int8_array,
        TEST_ARRAY_LENGTH_TEN,
        arraybuffer,
        0,
        &result);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 25. TestCreateSendableTypedArrayNullResult
static napi_value TestCreateSendableTypedArrayNullResult(napi_env env, napi_callback_info info)
{
    (void)info;
    void* data = nullptr;
    napi_value arraybuffer = nullptr;
    napi_status status = napi_create_sendable_arraybuffer(
        env,
        TEST_BUFFER_SIZE_HUNDRED,
        &data,
        &arraybuffer);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建底层 arraybuffer 失败");
    }
    status = napi_create_sendable_typedarray(
        env,
        napi_int8_array,
        TEST_ARRAY_LENGTH_TEN,
        arraybuffer,
        0,
        nullptr);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 26. TestCreateSendableMapNormal
static napi_value TestCreateSendableMapNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    napi_status status = napi_create_sendable_map(env, &result);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable map 失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 27. TestCreateSendableMapNullEnv
static napi_value TestCreateSendableMapNullEnv(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    napi_status status = napi_create_sendable_map(nullptr, &result);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 28. TestCreateSendableMapNullResult
static napi_value TestCreateSendableMapNullResult(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_status status = napi_create_sendable_map(env, nullptr);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 29. TestIsSendableNormal
static napi_value TestIsSendableNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value normalObj = nullptr;
    napi_status status = napi_create_object(env, &normalObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建普通对象失败");
    }
    if (!CheckIsSendable(env, normalObj, false)) {
        return CreateTestResult(env, false, "普通对象被错误地判定为 sendable");
    }
    napi_value sendableObj = nullptr;
    status = napi_create_sendable_array(env, &sendableObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable 数组失败");
    }
    if (!CheckIsSendable(env, sendableObj, true)) {
        return CreateTestResult(env, false, "sendable 数组被错误地判定为非 sendable");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 30. TestIsSendableNullEnv
static napi_value TestIsSendableNullEnv(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value normalObj = nullptr;
    napi_status status = napi_create_object(env, &normalObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建普通对象失败");
    }
    bool isSendable = false;
    status = napi_is_sendable(nullptr, normalObj, &isSendable);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 31. TestIsSendableNullResult
static napi_value TestIsSendableNullResult(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value normalObj = nullptr;
    napi_status status = napi_create_object(env, &normalObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建普通对象失败");
    }
    status = napi_is_sendable(env, normalObj, nullptr);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 32. TestIsSendableUndefined
static napi_value TestIsSendableUndefined(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value undefinedVal = nullptr;
    napi_status status = napi_get_undefined(env, &undefinedVal);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "获取 undefined 失败");
    }
    if (!CheckIsSendable(env, undefinedVal, true)) {
        return CreateTestResult(env, false, "undefined 应当是可持有为 sendable 的");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 33. TestIsSendableNull
static napi_value TestIsSendableNull(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value nullVal = nullptr;
    napi_status status = napi_get_null(env, &nullVal);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "获取 null 失败");
    }
    if (!CheckIsSendable(env, nullVal, true)) {
        return CreateTestResult(env, false, "null 应当是可持有为 sendable 的");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 34. TestIsSendableNumber
static napi_value TestIsSendableNumber(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value numVal = nullptr;
    napi_status status = napi_create_int32(env, TEST_INT_VAL_FORTY_TWO, &numVal);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "获取数字失败");
    }
    if (!CheckIsSendable(env, numVal, true)) {
        return CreateTestResult(env, false, "基本数值类型应当是可持有为 sendable 的");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 35. TestIsSendableString
static napi_value TestIsSendableString(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value strVal = nullptr;
    napi_status status = napi_create_string_utf8(env, "hello", NAPI_AUTO_LENGTH, &strVal);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "获取字符串失败");
    }
    if (!CheckIsSendable(env, strVal, true)) {
        return CreateTestResult(env, false, "字符串类型应当是可持有为 sendable 的");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 36. TestIsSendableBoolean
static napi_value TestIsSendableBoolean(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value boolVal = nullptr;
    napi_status status = napi_get_boolean(env, true, &boolVal);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "获取布尔值失败");
    }
    if (!CheckIsSendable(env, boolVal, true)) {
        return CreateTestResult(env, false, "布尔类型应当是可持有为 sendable 的");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 37. TestIsSendableSymbol
static napi_value TestIsSendableSymbol(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value symVal = nullptr;
    napi_value strVal = nullptr;
    napi_status status = napi_create_string_utf8(env, "symbol_desc", NAPI_AUTO_LENGTH, &strVal);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "获取描述字符串失败");
    }
    status = napi_create_symbol(env, strVal, &symVal);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 symbol 失败");
    }
    if (!CheckIsSendable(env, symVal, true)) {
        return CreateTestResult(env, false, "Symbol 类型应当是可持有为 sendable 的");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 38. TestIsSendableNormalObject
static napi_value TestIsSendableNormalObject(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value normalObj = nullptr;
    napi_status status = napi_create_object(env, &normalObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建普通对象失败");
    }
    if (!CheckIsSendable(env, normalObj, false)) {
        return CreateTestResult(env, false, "普通 JS 对象不应当是 sendable 的");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 39. TestIsSendableNormalArray
static napi_value TestIsSendableNormalArray(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value normalArr = nullptr;
    napi_status status = napi_create_array(env, &normalArr);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建普通数组失败");
    }
    if (!CheckIsSendable(env, normalArr, false)) {
        return CreateTestResult(env, false, "普通 JS 数组不应当是 sendable 的");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 40. TestIsSendableNormalFunction
static napi_value TestIsSendableNormalFunction(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value normalFunc = nullptr;
    napi_status status = napi_create_function(
        env,
        "normalFunc",
        NAPI_AUTO_LENGTH,
        ConstructorCallback,
        nullptr,
        &normalFunc);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建普通函数失败");
    }
    if (!CheckIsSendable(env, normalFunc, false)) {
        return CreateTestResult(env, false, "普通函数不应当是 sendable 的");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 41. TestIsSendableNormalMap
static napi_value TestIsSendableNormalMap(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "获取 global 失败");
    }
    napi_value mapCtor = nullptr;
    status = napi_get_named_property(env, global, "Map", &mapCtor);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "获取 Map 构造函数失败");
    }
    napi_value mapObj = nullptr;
    status = napi_new_instance(env, mapCtor, 0, nullptr, &mapObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "实例化 Map 失败");
    }
    if (!CheckIsSendable(env, mapObj, false)) {
        return CreateTestResult(env, false, "普通 Map 对象不应当是 sendable 的");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 42. TestIsSendableNormalSet
static napi_value TestIsSendableNormalSet(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "获取 global 失败");
    }
    napi_value setCtor = nullptr;
    status = napi_get_named_property(env, global, "Set", &setCtor);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "获取 Set 构造函数失败");
    }
    napi_value setObj = nullptr;
    status = napi_new_instance(env, setCtor, 0, nullptr, &setObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "实例化 Set 失败");
    }
    if (!CheckIsSendable(env, setObj, false)) {
        return CreateTestResult(env, false, "普通 Set 对象不应当是 sendable 的");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 43. TestWrapSendableNormal
static napi_value TestWrapSendableNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value sendableObj = nullptr;
    napi_status status = napi_create_sendable_array(env, &sendableObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable 对象失败");
    }
    int32_t* nativeData = new int32_t(TEST_INT_VAL_FORTY_TWO);
    status = napi_wrap_sendable(
        env,
        sendableObj,
        nativeData,
        DummyFinalizer,
        nullptr);
    if (status != napi_ok) {
        delete nativeData;
        return CreateTestResult(env, false, "napi_wrap_sendable 失败");
    }
    void* unwrappedData = nullptr;
    status = napi_unwrap_sendable(env, sendableObj, &unwrappedData);
    if (status != napi_ok || unwrappedData != nativeData) {
        return CreateTestResult(env, false, "napi_unwrap_sendable 返回值或数据错误");
    }
    void* removedData = nullptr;
    status = napi_remove_wrap_sendable(env, sendableObj, &removedData);
    if (status != napi_ok || removedData != nativeData) {
        return CreateTestResult(env, false, "napi_remove_wrap_sendable 返回值或数据错误");
    }
    delete nativeData;
    return CreateTestResult(env, true, "测试成功");
}

// 44. TestWrapSendableNullEnv
static napi_value TestWrapSendableNullEnv(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value sendableObj = nullptr;
    napi_status status = napi_create_sendable_array(env, &sendableObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable 对象失败");
    }
    int32_t* nativeData = new int32_t(TEST_INT_VAL_FORTY_TWO);
    status = napi_wrap_sendable(
        nullptr,
        sendableObj,
        nativeData,
        DummyFinalizer,
        nullptr);
    if (status != napi_invalid_arg) {
        delete nativeData;
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    delete nativeData;
    return CreateTestResult(env, true, "测试成功");
}

// 45. TestWrapSendableNullObject
static napi_value TestWrapSendableNullObject(napi_env env, napi_callback_info info)
{
    (void)info;
    int32_t* nativeData = new int32_t(TEST_INT_VAL_FORTY_TWO);
    napi_status status = napi_wrap_sendable(
        env,
        nullptr,
        nativeData,
        DummyFinalizer,
        nullptr);
    if (status != napi_invalid_arg) {
        delete nativeData;
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    delete nativeData;
    return CreateTestResult(env, true, "测试成功");
}

// 46. TestWrapSendableDuplicate
static napi_value TestWrapSendableDuplicate(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value sendableObj = nullptr;
    napi_status status = napi_create_sendable_array(env, &sendableObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable 对象失败");
    }
    int32_t* nativeDataFirst = new int32_t(TEST_INT_VAL_FORTY_TWO);
    status = napi_wrap_sendable(
        env,
        sendableObj,
        nativeDataFirst,
        DummyFinalizer,
        nullptr);
    if (status != napi_ok) {
        delete nativeDataFirst;
        return CreateTestResult(env, false, "第一次 wrap 失败");
    }
    int32_t* nativeDataSecond = new int32_t(TEST_INT_VAL_NINETY_NINE);
    status = napi_wrap_sendable(
        env,
        sendableObj,
        nativeDataSecond,
        DummyFinalizer,
        nullptr);
    if (status == napi_ok) {
        delete nativeDataSecond;
        return CreateTestResult(env, false, "重复 wrap 应当失败，但它成功了");
    }
    delete nativeDataSecond;
    void* removedData = nullptr;
    status = napi_remove_wrap_sendable(env, sendableObj, &removedData);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "清理第一次 wrap 的资源失败");
    }
    delete nativeDataFirst;
    return CreateTestResult(env, true, "测试成功");
}

// 47. TestUnwrapSendableNotWrapped
static napi_value TestUnwrapSendableNotWrapped(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value sendableObj = nullptr;
    napi_status status = napi_create_sendable_array(env, &sendableObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable 数组失败");
    }
    void* unwrappedData = nullptr;
    status = napi_unwrap_sendable(env, sendableObj, &unwrappedData);
    if (status != napi_ok || unwrappedData != nullptr) {
        return CreateTestResult(env, false, "未 wrap 的 unwrap 操作应当获得 nullptr 且 status 为 ok");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 48. TestRemoveWrapSendableNotWrapped
static napi_value TestRemoveWrapSendableNotWrapped(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value sendableObj = nullptr;
    napi_status status = napi_create_sendable_array(env, &sendableObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable 数组失败");
    }
    void* removedData = nullptr;
    status = napi_remove_wrap_sendable(env, sendableObj, &removedData);
    if (status != napi_ok || removedData != nullptr) {
        return CreateTestResult(env, false, "未 wrap 的 remove_wrap 操作应当获得 nullptr 且 status 为 ok");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 49. TestStrongSendableRefNormal
static napi_value TestStrongSendableRefNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value sendableObj = nullptr;
    napi_status status = napi_create_sendable_array(env, &sendableObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable 数组失败");
    }
    napi_sendable_ref ref = nullptr;
    status = napi_create_strong_sendable_reference(env, sendableObj, &ref);
    if (status != napi_ok || ref == nullptr) {
        return CreateTestResult(env, false, "创建 strong sendable ref 失败");
    }
    napi_value resolvedVal = nullptr;
    status = napi_get_strong_sendable_reference_value(env, ref, &resolvedVal);
    if (status != napi_ok || resolvedVal == nullptr) {
        napi_delete_strong_sendable_reference(env, ref);
        return CreateTestResult(env, false, "解引用 strong sendable ref 失败");
    }
    status = napi_delete_strong_sendable_reference(env, ref);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "删除 strong sendable ref 失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 50. TestStrongSendableRefNullEnv
static napi_value TestStrongSendableRefNullEnv(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value sendableObj = nullptr;
    napi_status status = napi_create_sendable_array(env, &sendableObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable 数组失败");
    }
    napi_sendable_ref ref = nullptr;
    status = napi_create_strong_sendable_reference(nullptr, sendableObj, &ref);
    if (status != napi_invalid_arg) {
        if (status == napi_ok && ref != nullptr) {
            napi_delete_strong_sendable_reference(env, ref);
        }
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 51. TestStrongSendableRefNullResult
static napi_value TestStrongSendableRefNullResult(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value sendableObj = nullptr;
    napi_status status = napi_create_sendable_array(env, &sendableObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable 数组失败");
    }
    status = napi_create_strong_sendable_reference(env, sendableObj, nullptr);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 52. TestDeleteStrongSendableRefNullEnv
static napi_value TestDeleteStrongSendableRefNullEnv(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value sendableObj = nullptr;
    napi_status status = napi_create_sendable_array(env, &sendableObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable 数组失败");
    }
    napi_sendable_ref ref = nullptr;
    status = napi_create_strong_sendable_reference(env, sendableObj, &ref);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 strong sendable ref 失败");
    }
    status = napi_delete_strong_sendable_reference(nullptr, ref);
    if (status != napi_invalid_arg) {
        napi_delete_strong_sendable_reference(env, ref);
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    napi_delete_strong_sendable_reference(env, ref);
    return CreateTestResult(env, true, "测试成功");
}

// 53. TestGetStrongSendableRefNullEnv
static napi_value TestGetStrongSendableRefNullEnv(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value sendableObj = nullptr;
    napi_status status = napi_create_sendable_array(env, &sendableObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable 数组失败");
    }
    napi_sendable_ref ref = nullptr;
    status = napi_create_strong_sendable_reference(env, sendableObj, &ref);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 strong sendable ref 失败");
    }
    napi_value resolvedVal = nullptr;
    status = napi_get_strong_sendable_reference_value(nullptr, ref, &resolvedVal);
    if (status != napi_invalid_arg) {
        napi_delete_strong_sendable_reference(env, ref);
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    napi_delete_strong_sendable_reference(env, ref);
    return CreateTestResult(env, true, "测试成功");
}

// 54. TestGetStrongSendableRefNullResult
static napi_value TestGetStrongSendableRefNullResult(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value sendableObj = nullptr;
    napi_status status = napi_create_sendable_array(env, &sendableObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable 数组失败");
    }
    napi_sendable_ref ref = nullptr;
    status = napi_create_strong_sendable_reference(env, sendableObj, &ref);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 strong sendable ref 失败");
    }
    status = napi_get_strong_sendable_reference_value(env, ref, nullptr);
    if (status != napi_invalid_arg) {
        napi_delete_strong_sendable_reference(env, ref);
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    napi_delete_strong_sendable_reference(env, ref);
    return CreateTestResult(env, true, "测试成功");
}

// 55. TestStrongSendableRefMultipleRefs
static napi_value TestStrongSendableRefMultipleRefs(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value sendableObj = nullptr;
    napi_status status = napi_create_sendable_array(env, &sendableObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable 数组失败");
    }
    napi_sendable_ref refFirst = nullptr;
    napi_sendable_ref refSecond = nullptr;
    status = napi_create_strong_sendable_reference(env, sendableObj, &refFirst);
    if (status != napi_ok || refFirst == nullptr) {
        return CreateTestResult(env, false, "创建第一个 strong ref 失败");
    }
    status = napi_create_strong_sendable_reference(env, sendableObj, &refSecond);
    if (status != napi_ok || refSecond == nullptr) {
        napi_delete_strong_sendable_reference(env, refFirst);
        return CreateTestResult(env, false, "创建第二个 strong ref 失败");
    }
    napi_value valFirst = nullptr;
    napi_value valSecond = nullptr;
    status = napi_get_strong_sendable_reference_value(env, refFirst, &valFirst);
    if (status != napi_ok || valFirst == nullptr) {
        napi_delete_strong_sendable_reference(env, refFirst);
        napi_delete_strong_sendable_reference(env, refSecond);
        return CreateTestResult(env, false, "解引用第一个 strong ref 失败");
    }
    status = napi_get_strong_sendable_reference_value(env, refSecond, &valSecond);
    if (status != napi_ok || valSecond == nullptr) {
        napi_delete_strong_sendable_reference(env, refFirst);
        napi_delete_strong_sendable_reference(env, refSecond);
        return CreateTestResult(env, false, "解引用第二个 strong ref 失败");
    }
    napi_delete_strong_sendable_reference(env, refFirst);
    napi_delete_strong_sendable_reference(env, refSecond);
    return CreateTestResult(env, true, "测试成功");
}

// 56. TestWrapSendableNullFinalizer
static napi_value TestWrapSendableNullFinalizer(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value sendableObj = nullptr;
    napi_status status = napi_create_sendable_array(env, &sendableObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable 对象失败");
    }
    int32_t* nativeData = new int32_t(TEST_INT_VAL_FORTY_TWO);
    status = napi_wrap_sendable(
        env,
        sendableObj,
        nativeData,
        nullptr,
        nullptr);
    if (status != napi_ok) {
        delete nativeData;
        return CreateTestResult(env, false, "napi_wrap_sendable 允许为空 finalizer 失败");
    }
    void* unwrappedData = nullptr;
    status = napi_remove_wrap_sendable(env, sendableObj, &unwrappedData);
    if (status != napi_ok || unwrappedData != nativeData) {
        delete nativeData;
        return CreateTestResult(env, false, "napi_remove_wrap_sendable 失败");
    }
    delete nativeData;
    return CreateTestResult(env, true, "测试成功");
}

// 57. TestWrapSendableWithSizeNullFinalizer
static napi_value TestWrapSendableWithSizeNullFinalizer(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value sendableObj = nullptr;
    napi_status status = napi_create_sendable_array(env, &sendableObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable 对象失败");
    }
    int32_t* nativeData = new int32_t(TEST_INT_VAL_FORTY_TWO);
    status = napi_wrap_sendable_with_size(
        env,
        sendableObj,
        nativeData,
        nullptr,
        nullptr,
        TEST_NATIVE_BINDING_SIZE);
    if (status != napi_ok) {
        delete nativeData;
        return CreateTestResult(env, false, "napi_wrap_sendable_with_size 允许为空 finalizer 失败");
    }
    void* unwrappedData = nullptr;
    status = napi_remove_wrap_sendable(env, sendableObj, &unwrappedData);
    if (status != napi_ok || unwrappedData != nativeData) {
        delete nativeData;
        return CreateTestResult(env, false, "napi_remove_wrap_sendable 失败");
    }
    delete nativeData;
    return CreateTestResult(env, true, "测试成功");
}

// 58. TestUnwrapSendableNullResult
static napi_value TestUnwrapSendableNullResult(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value sendableObj = nullptr;
    napi_status status = napi_create_sendable_array(env, &sendableObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable 对象失败");
    }
    status = napi_unwrap_sendable(env, sendableObj, nullptr);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 59. TestRemoveWrapSendableNullResult
static napi_value TestRemoveWrapSendableNullResult(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value sendableObj = nullptr;
    napi_status status = napi_create_sendable_array(env, &sendableObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable 对象失败");
    }
    status = napi_remove_wrap_sendable(env, sendableObj, nullptr);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "期待 napi_invalid_arg，但获取的结果不同");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 60. TestWrapSendableWithSizeNormal
static napi_value TestWrapSendableWithSizeNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value sendableObj = nullptr;
    napi_status status = napi_create_sendable_array(env, &sendableObj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 sendable 失败");
    }
    int32_t* nativeData = new int32_t(TEST_INT_VAL_FORTY_TWO);
    status = napi_wrap_sendable_with_size(
        env,
        sendableObj,
        nativeData,
        DummyFinalizer,
        nullptr,
        TEST_NATIVE_BINDING_SIZE);
    if (status != napi_ok) {
        delete nativeData;
        return CreateTestResult(env, false, "napi_wrap_sendable_with_size 正常流执行失败");
    }
    void* resultData = nullptr;
    status = napi_unwrap_sendable(env, sendableObj, &resultData);
    if (status != napi_ok || resultData != nativeData) {
        return CreateTestResult(env, false, "unwrap_sendable 数据不匹配");
    }
    status = napi_remove_wrap_sendable(env, sendableObj, &resultData);
    if (status != napi_ok || resultData != nativeData) {
        return CreateTestResult(env, false, "remove_wrap_sendable 失败");
    }
    delete nativeData;
    return CreateTestResult(env, true, "测试成功");
}

static const napi_property_descriptor SENDABLE_TEST_DESCRIPTORS[] = {
    DECLARE_NAPI_FUNCTION("testDefineSendableClassNormal",
        TestDefineSendableClassNormal),
    DECLARE_NAPI_FUNCTION("testDefineSendableClassWithProperties",
        TestDefineSendableClassWithProperties),
    DECLARE_NAPI_FUNCTION("testDefineSendableClassNullEnv",
        TestDefineSendableClassNullEnv),
    DECLARE_NAPI_FUNCTION("testDefineSendableClassNullName",
        TestDefineSendableClassNullName),
    DECLARE_NAPI_FUNCTION("testDefineSendableClassNullResult",
        TestDefineSendableClassNullResult),
    DECLARE_NAPI_FUNCTION("testDefineSendableClassWithParent",
        TestDefineSendableClassWithParent),
    DECLARE_NAPI_FUNCTION("testCreateSendableObjectWithPropertiesNormal",
        TestCreateSendableObjectWithPropertiesNormal),
    DECLARE_NAPI_FUNCTION("testCreateSendableObjectWithPropertiesNullEnv",
        TestCreateSendableObjectWithPropertiesNullEnv),
    DECLARE_NAPI_FUNCTION("testCreateSendableObjectWithPropertiesNullResult",
        TestCreateSendableObjectWithPropertiesNullResult),
    DECLARE_NAPI_FUNCTION("testCreateSendableObjectWithPropertiesNullProps",
        TestCreateSendableObjectWithPropertiesNullProps),
    DECLARE_NAPI_FUNCTION("testCreateSendableArrayNormal",
        TestCreateSendableArrayNormal),
    DECLARE_NAPI_FUNCTION("testCreateSendableArrayNullEnv",
        TestCreateSendableArrayNullEnv),
    DECLARE_NAPI_FUNCTION("testCreateSendableArrayNullResult",
        TestCreateSendableArrayNullResult),
    DECLARE_NAPI_FUNCTION("testCreateSendableArrayWithLengthNormal",
        TestCreateSendableArrayWithLengthNormal),
    DECLARE_NAPI_FUNCTION("testCreateSendableArrayWithLengthNullEnv",
        TestCreateSendableArrayWithLengthNullEnv),
    DECLARE_NAPI_FUNCTION("testCreateSendableArrayWithLengthNullResult",
        TestCreateSendableArrayWithLengthNullResult),
    DECLARE_NAPI_FUNCTION("testCreateSendableArrayWithLengthZero",
        TestCreateSendableArrayWithLengthZero),
    DECLARE_NAPI_FUNCTION("testCreateSendableArrayBufferNormal",
        TestCreateSendableArrayBufferNormal),
    DECLARE_NAPI_FUNCTION("testCreateSendableArrayBufferNullEnv",
        TestCreateSendableArrayBufferNullEnv),
    DECLARE_NAPI_FUNCTION("testCreateSendableArrayBufferNullResult",
        TestCreateSendableArrayBufferNullResult),
    DECLARE_NAPI_FUNCTION("testCreateSendableArrayBufferZeroSize",
        TestCreateSendableArrayBufferZeroSize),
    DECLARE_NAPI_FUNCTION("testCreateSendableArrayBufferHugeSize",
        TestCreateSendableArrayBufferHugeSize),
    DECLARE_NAPI_FUNCTION("testCreateSendableTypedArrayNormal",
        TestCreateSendableTypedArrayNormal),
    DECLARE_NAPI_FUNCTION("testCreateSendableTypedArrayNullEnv",
        TestCreateSendableTypedArrayNullEnv),
    DECLARE_NAPI_FUNCTION("testCreateSendableTypedArrayNullResult",
        TestCreateSendableTypedArrayNullResult),
    DECLARE_NAPI_FUNCTION("testCreateSendableMapNormal",
        TestCreateSendableMapNormal),
    DECLARE_NAPI_FUNCTION("testCreateSendableMapNullEnv",
        TestCreateSendableMapNullEnv),
    DECLARE_NAPI_FUNCTION("testCreateSendableMapNullResult",
        TestCreateSendableMapNullResult),
    DECLARE_NAPI_FUNCTION("testIsSendableNormal",
        TestIsSendableNormal),
    DECLARE_NAPI_FUNCTION("testIsSendableNullEnv",
        TestIsSendableNullEnv),
    DECLARE_NAPI_FUNCTION("testIsSendableNullResult",
        TestIsSendableNullResult),
    DECLARE_NAPI_FUNCTION("testIsSendableUndefined",
        TestIsSendableUndefined),
    DECLARE_NAPI_FUNCTION("testIsSendableNull",
        TestIsSendableNull),
    DECLARE_NAPI_FUNCTION("testIsSendableNumber",
        TestIsSendableNumber),
    DECLARE_NAPI_FUNCTION("testIsSendableString",
        TestIsSendableString),
    DECLARE_NAPI_FUNCTION("testIsSendableBoolean",
        TestIsSendableBoolean),
    DECLARE_NAPI_FUNCTION("testIsSendableSymbol",
        TestIsSendableSymbol),
    DECLARE_NAPI_FUNCTION("testIsSendableNormalObject",
        TestIsSendableNormalObject),
    DECLARE_NAPI_FUNCTION("testIsSendableNormalArray",
        TestIsSendableNormalArray),
    DECLARE_NAPI_FUNCTION("testIsSendableNormalFunction",
        TestIsSendableNormalFunction),
    DECLARE_NAPI_FUNCTION("testIsSendableNormalMap",
        TestIsSendableNormalMap),
    DECLARE_NAPI_FUNCTION("testIsSendableNormalSet",
        TestIsSendableNormalSet),
    DECLARE_NAPI_FUNCTION("testWrapSendableNormal",
        TestWrapSendableNormal),
    DECLARE_NAPI_FUNCTION("testWrapSendableNullEnv",
        TestWrapSendableNullEnv),
    DECLARE_NAPI_FUNCTION("testWrapSendableNullObject",
        TestWrapSendableNullObject),
    DECLARE_NAPI_FUNCTION("testWrapSendableDuplicate",
        TestWrapSendableDuplicate),
    DECLARE_NAPI_FUNCTION("testUnwrapSendableNotWrapped",
        TestUnwrapSendableNotWrapped),
    DECLARE_NAPI_FUNCTION("testRemoveWrapSendableNotWrapped",
        TestRemoveWrapSendableNotWrapped),
    DECLARE_NAPI_FUNCTION("testStrongSendableRefNormal",
        TestStrongSendableRefNormal),
    DECLARE_NAPI_FUNCTION("testStrongSendableRefNullEnv",
        TestStrongSendableRefNullEnv),
    DECLARE_NAPI_FUNCTION("testStrongSendableRefNullResult",
        TestStrongSendableRefNullResult),
    DECLARE_NAPI_FUNCTION("testDeleteStrongSendableRefNullEnv",
        TestDeleteStrongSendableRefNullEnv),
    DECLARE_NAPI_FUNCTION("testGetStrongSendableRefNullEnv",
        TestGetStrongSendableRefNullEnv),
    DECLARE_NAPI_FUNCTION("testGetStrongSendableRefNullResult",
        TestGetStrongSendableRefNullResult),
    DECLARE_NAPI_FUNCTION("testStrongSendableRefMultipleRefs",
        TestStrongSendableRefMultipleRefs),
    DECLARE_NAPI_FUNCTION("testWrapSendableNullFinalizer",
        TestWrapSendableNullFinalizer),
    DECLARE_NAPI_FUNCTION("testWrapSendableWithSizeNullFinalizer",
        TestWrapSendableWithSizeNullFinalizer),
    DECLARE_NAPI_FUNCTION("testUnwrapSendableNullResult",
        TestUnwrapSendableNullResult),
    DECLARE_NAPI_FUNCTION("testRemoveWrapSendableNullResult",
        TestRemoveWrapSendableNullResult),
    DECLARE_NAPI_FUNCTION("testWrapSendableWithSizeNormal",
        TestWrapSendableWithSizeNormal),
};

// 模块初始化和导出注册
static napi_value Init(napi_env env, napi_value exports)
{
    NAPI_CALL(env, napi_define_properties(env, exports,
        sizeof(SENDABLE_TEST_DESCRIPTORS) / sizeof(SENDABLE_TEST_DESCRIPTORS[0]),
        SENDABLE_TEST_DESCRIPTORS));
    return exports;
}

static napi_module g_sendableSuiteModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "sendable_suite",
    .nm_priv = nullptr,
    .reserved = { nullptr },
};

extern "C" __attribute__((constructor)) void RegisterModule()
{
    napi_module_register(&g_sendableSuiteModule);
}
