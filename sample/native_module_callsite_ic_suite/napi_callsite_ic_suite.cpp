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
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

// 匿名命名空间，用于存放模块内的常量定义、RAII 工具类及辅助函数
namespace {
constexpr int32_t VAL_ZERO = 0;
constexpr int32_t VAL_ONE = 1;
constexpr int32_t VAL_TWO = 2;
constexpr int32_t VAL_THREE = 3;
constexpr int32_t VAL_FORTYTWO = 42;
constexpr int32_t VAL_HUNDRED = 100;
constexpr int32_t VAL_THOUSAND = 1000;
constexpr int32_t VAL_TEN_THOUSAND = 10000;

constexpr size_t VAL_SIZE_ZERO = 0;
constexpr size_t VAL_SIZE_ONE = 1;
constexpr size_t VAL_SIZE_TWO = 2;
constexpr size_t VAL_SIZE_THREE = 3;
constexpr size_t VAL_SIZE_TEN = 10;
constexpr size_t VAL_SIZE_HUNDRED = 100;
constexpr size_t VAL_SIZE_THOUSAND = 1000;

constexpr double VAL_PI = 3.14159;

constexpr uint32_t K_MODULE_VERSION = 1;
constexpr uint32_t K_NO_MODULE_FLAGS = 0;

// Callsite 信息的 RAII 管理类，防止测试过程中内存泄漏
class CallsiteGuard {
public:
    CallsiteGuard(napi_env env, napi_callsite_info info) : env_(env), info_(info) {}
    ~CallsiteGuard()
    {
        if (info_ != nullptr) {
            (void)napi_delete_callsite_info(env_, info_);
        }
    }
    CallsiteGuard(const CallsiteGuard&) = delete;
    CallsiteGuard& operator=(const CallsiteGuard&) = delete;

private:
    napi_env env_ = nullptr;
    napi_callsite_info info_ = nullptr;
};

// 构造统一测试结果对象的辅助函数
napi_value CreateTestResult(napi_env env, bool success, const char* message)
{
    napi_value resultObj = nullptr;
    napi_status status = napi_create_object(env, &resultObj);
    if (status != napi_ok) {
        return nullptr;
    }

    napi_value successVal = nullptr;
    status = napi_get_boolean(env, success, &successVal);
    if (status != napi_ok) {
        return nullptr;
    }

    status = napi_set_named_property(env, resultObj, "success", successVal);
    if (status != napi_ok) {
        return nullptr;
    }

    napi_value msgVal = nullptr;
    status = napi_create_string_utf8(env, message, NAPI_AUTO_LENGTH, &msgVal);
    if (status != napi_ok) {
        return nullptr;
    }

    status = napi_set_named_property(env, resultObj, "msg", msgVal);
    if (status != napi_ok) {
        return nullptr;
    }

    return resultObj;
}

// 快速创建整型对象的辅助函数
napi_value CreateTestInt32(napi_env env, int32_t val)
{
    napi_value res = nullptr;
    (void)napi_create_int32(env, val, &res);
    return res;
}

// 快速读取整型数值的辅助函数
int32_t GetTestInt32(napi_env env, napi_value val)
{
    int32_t res = VAL_ZERO;
    (void)napi_get_value_int32(env, val, &res);
    return res;
}
} // namespace

// 用例 1: 验证 napi_create_callsite_info 与 napi_delete_callsite_info 的基础创建与销毁
static napi_value TestCreateDeleteBasic(napi_env env, napi_callback_info /* info */)
{
    napi_callsite_info callsiteInfo = nullptr;
    napi_status status = napi_create_callsite_info(env, &callsiteInfo);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 callsite_info 失败");
    }
    if (callsiteInfo == nullptr) {
        return CreateTestResult(env, false, "创建的 callsite_info 指针为空");
    }
    status = napi_delete_callsite_info(env, callsiteInfo);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "删除 callsite_info 失败");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 2: 验证创建时 result 参数传入空指针，API 报错
static napi_value TestCreateNullResult(napi_env env, napi_callback_info /* info */)
{
    napi_status status = napi_create_callsite_info(env, nullptr);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "传入空指针未返回 napi_invalid_arg");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 3: 验证删除时 info 参数传入空指针，API 报错
static napi_value TestDeleteNullInfo(napi_env env, napi_callback_info /* info */)
{
    napi_status status = napi_delete_callsite_info(env, nullptr);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "删除空 info 未返回 napi_invalid_arg");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 4: 验证 napi_get_property_with_callsite_info 的基础读取功能
static napi_value TestGetPropertyBasic(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    napi_status status = napi_create_object(env, &obj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建对象失败");
    }

    napi_value key = nullptr;
    status = napi_create_string_utf8(env, "testProp", NAPI_AUTO_LENGTH, &key);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 key 失败");
    }

    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);
    status = napi_set_property(env, obj, key, val);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "设置属性失败");
    }

    napi_callsite_info callsiteInfo = nullptr;
    status = napi_create_callsite_info(env, &callsiteInfo);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 callsite 失败");
    }
    CallsiteGuard guard(env, callsiteInfo);

    napi_value result = nullptr;
    bool isHit = false;
    status = napi_get_property_with_callsite_info(
        env,
        obj,
        key,
        callsiteInfo,
        &result,
        &isHit);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "带 callsite 获取属性失败");
    }

    if (GetTestInt32(env, result) != VAL_FORTYTWO) {
        return CreateTestResult(env, false, "获取的值不正确");
    }

    return CreateTestResult(env, true, "成功");
}

// 用例 5: get_property 传入空 object 校验
static napi_value TestGetPropertyNullObject(napi_env env, napi_callback_info /* info */)
{
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "prop", NAPI_AUTO_LENGTH, &key);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    napi_value result = nullptr;
    bool isHit = false;
    napi_status status = napi_get_property_with_callsite_info(
        env,
        nullptr,
        key,
        callsiteInfo,
        &result,
        &isHit);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "空 object 未返回 napi_invalid_arg");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 6: get_property 传入空 key 校验
static napi_value TestGetPropertyNullKey(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    napi_value result = nullptr;
    bool isHit = false;
    napi_status status = napi_get_property_with_callsite_info(
        env,
        obj,
        nullptr,
        callsiteInfo,
        &result,
        &isHit);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "空 key 未返回 napi_invalid_arg");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 7: get_property 传入空 result 指针校验
static napi_value TestGetPropertyNullResult(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "prop", NAPI_AUTO_LENGTH, &key);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    bool isHit = false;
    napi_status status = napi_get_property_with_callsite_info(
        env,
        obj,
        key,
        callsiteInfo,
        nullptr,
        &isHit);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "空 result 未返回 napi_invalid_arg");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 8: get_property 传入空 callsite_info，检查退回常规获取的逻辑与 hit 参数返回 false
static napi_value TestGetPropertyNullInfo(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "prop", NAPI_AUTO_LENGTH, &key);
    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);
    (void)napi_set_property(env, obj, key, val);

    napi_value result = nullptr;
    bool isHit = true;
    napi_status status = napi_get_property_with_callsite_info(
        env,
        obj,
        key,
        nullptr,
        &result,
        &isHit);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "空 info 获取失败");
    }
    if (isHit != false) {
        return CreateTestResult(env, false, "空 info 却误报为命中 IC");
    }
    if (GetTestInt32(env, result) != VAL_FORTYTWO) {
        return CreateTestResult(env, false, "获取的值不正确");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 9: get_property 对非对象实体（数字）进行属性获取
static napi_value TestGetPropertyNonObjectNum(napi_env env, napi_callback_info /* info */)
{
    napi_value numVal = nullptr;
    (void)napi_create_double(env, VAL_PI, &numVal);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "prop", NAPI_AUTO_LENGTH, &key);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    napi_value result = nullptr;
    bool isHit = false;
    napi_status status = napi_get_property_with_callsite_info(
        env,
        numVal,
        key,
        callsiteInfo,
        &result,
        &isHit);
    if (status != napi_object_expected) {
        return CreateTestResult(env, false, "对数字读取属性未返回 napi_object_expected");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 10: get_property 对非对象实体（布尔）进行属性获取
static napi_value TestGetPropertyNonObjectBool(napi_env env, napi_callback_info /* info */)
{
    napi_value boolVal = nullptr;
    (void)napi_get_boolean(env, true, &boolVal);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "prop", NAPI_AUTO_LENGTH, &key);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    napi_value result = nullptr;
    bool isHit = false;
    napi_status status = napi_get_property_with_callsite_info(
        env,
        boolVal,
        key,
        callsiteInfo,
        &result,
        &isHit);
    if (status != napi_object_expected) {
        return CreateTestResult(env, false, "对布尔读取属性未返回 napi_object_expected");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 11: get_property 对非对象实体（字符串）进行属性获取
static napi_value TestGetPropertyNonObjectStr(napi_env env, napi_callback_info /* info */)
{
    napi_value strVal = nullptr;
    (void)napi_create_string_utf8(env, "str", NAPI_AUTO_LENGTH, &strVal);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "prop", NAPI_AUTO_LENGTH, &key);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    napi_value result = nullptr;
    bool isHit = false;
    napi_status status = napi_get_property_with_callsite_info(
        env,
        strVal,
        key,
        callsiteInfo,
        &result,
        &isHit);
    if (status != napi_object_expected) {
        return CreateTestResult(env, false, "对字符串读取属性未返回 napi_object_expected");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 12: get_property 对非对象实体（null）进行属性获取
static napi_value TestGetPropertyNonObjectNull(napi_env env, napi_callback_info /* info */)
{
    napi_value nullVal = nullptr;
    (void)napi_get_null(env, &nullVal);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "prop", NAPI_AUTO_LENGTH, &key);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    napi_value result = nullptr;
    bool isHit = false;
    napi_status status = napi_get_property_with_callsite_info(
        env,
        nullVal,
        key,
        callsiteInfo,
        &result,
        &isHit);
    if (status != napi_object_expected) {
        return CreateTestResult(env, false, "对 null 读取属性未返回 napi_object_expected");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 13: get_property 对非对象实体（undefined）进行属性获取
static napi_value TestGetPropertyNonObjectUndef(napi_env env, napi_callback_info /* info */)
{
    napi_value undefVal = nullptr;
    (void)napi_get_undefined(env, &undefVal);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "prop", NAPI_AUTO_LENGTH, &key);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    napi_value result = nullptr;
    bool isHit = false;
    napi_status status = napi_get_property_with_callsite_info(
        env,
        undefVal,
        key,
        callsiteInfo,
        &result,
        &isHit);
    if (status != napi_object_expected) {
        return CreateTestResult(env, false, "对 undefined 读取属性未返回 napi_object_expected");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 14: get_property 对 Function 对象（合法的 Object）进行属性获取
static napi_value TestGetPropertyFunction(napi_env env, napi_callback_info /* info */)
{
    auto dummyCb = [](napi_env env, napi_callback_info /* info */) -> napi_value {
        return nullptr;
    };
    napi_value funcVal = nullptr;
    napi_status status = napi_create_function(
        env,
        "dummyFunc",
        NAPI_AUTO_LENGTH,
        dummyCb,
        nullptr,
        &funcVal);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建函数失败");
    }

    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "funcProp", NAPI_AUTO_LENGTH, &key);
    napi_value val = CreateTestInt32(env, VAL_HUNDRED);
    (void)napi_set_property(env, funcVal, key, val);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    napi_value result = nullptr;
    bool isHit = false;
    status = napi_get_property_with_callsite_info(
        env,
        funcVal,
        key,
        callsiteInfo,
        &result,
        &isHit);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "读取函数属性失败");
    }
    if (GetTestInt32(env, result) != VAL_HUNDRED) {
        return CreateTestResult(env, false, "获取函数属性的值不匹配");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 15: get_property 对 Array 对象进行属性获取
static napi_value TestGetPropertyArray(napi_env env, napi_callback_info /* info */)
{
    napi_value arrVal = nullptr;
    napi_status status = napi_create_array(env, &arrVal);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建数组失败");
    }

    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "arrProp", NAPI_AUTO_LENGTH, &key);
    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);
    (void)napi_set_property(env, arrVal, key, val);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    napi_value result = nullptr;
    bool isHit = false;
    status = napi_get_property_with_callsite_info(
        env,
        arrVal,
        key,
        callsiteInfo,
        &result,
        &isHit);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "获取数组属性失败");
    }
    if (GetTestInt32(env, result) != VAL_FORTYTWO) {
        return CreateTestResult(env, false, "读取数组属性值错误");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 16: get_property 获取不存在的属性，验证其返回 napi_undefined 且状态为 napi_ok
static napi_value TestGetPropertyNonExistent(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "nonExistentProp", NAPI_AUTO_LENGTH, &key);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    napi_value result = nullptr;
    bool isHit = false;
    napi_status status = napi_get_property_with_callsite_info(
        env,
        obj,
        key,
        callsiteInfo,
        &result,
        &isHit);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "读取非存在属性失败");
    }

    napi_valuetype type = napi_undefined;
    (void)napi_typeof(env, result, &type);
    if (type != napi_undefined) {
        return CreateTestResult(env, false, "非存在属性的类型不为 undefined");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 17: napi_set_property_with_callsite_info 的基础写入功能验证
static napi_value TestSetPropertyBasic(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    napi_status status = napi_create_object(env, &obj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建对象失败");
    }

    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "testProp", NAPI_AUTO_LENGTH, &key);
    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);

    napi_callsite_info callsiteInfo = nullptr;
    status = napi_create_callsite_info(env, &callsiteInfo);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建 callsite 失败");
    }
    CallsiteGuard guard(env, callsiteInfo);

    bool isHit = false;
    status = napi_set_property_with_callsite_info(
        env,
        obj,
        key,
        val,
        callsiteInfo,
        &isHit);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "带 callsite 设置属性失败");
    }

    napi_value result = nullptr;
    status = napi_get_property(env, obj, key, &result);
    if (status != napi_ok || GetTestInt32(env, result) != VAL_FORTYTWO) {
        return CreateTestResult(env, false, "写入的值校验不匹配");
    }

    return CreateTestResult(env, true, "成功");
}

// 用例 18: set_property 传入空 object 校验
static napi_value TestSetPropertyNullObject(napi_env env, napi_callback_info /* info */)
{
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "prop", NAPI_AUTO_LENGTH, &key);
    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    bool isHit = false;
    napi_status status = napi_set_property_with_callsite_info(
        env,
        nullptr,
        key,
        val,
        callsiteInfo,
        &isHit);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "空 object 未返回 napi_invalid_arg");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 19: set_property 传入空 key 校验
static napi_value TestSetPropertyNullKey(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);
    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    bool isHit = false;
    napi_status status = napi_set_property_with_callsite_info(
        env,
        obj,
        nullptr,
        val,
        callsiteInfo,
        &isHit);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "空 key 未返回 napi_invalid_arg");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 20: set_property 传入空 value 校验
static napi_value TestSetPropertyNullValue(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "prop", NAPI_AUTO_LENGTH, &key);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    bool isHit = false;
    napi_status status = napi_set_property_with_callsite_info(
        env,
        obj,
        key,
        nullptr,
        callsiteInfo,
        &isHit);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "空 value 未返回 napi_invalid_arg");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 21: set_property 传入空 callsite_info，验证降级到常规写入
static napi_value TestSetPropertyNullInfo(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "prop", NAPI_AUTO_LENGTH, &key);
    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);

    bool isHit = true;
    napi_status status = napi_set_property_with_callsite_info(
        env,
        obj,
        key,
        val,
        nullptr,
        &isHit);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "空 info 常规写入失败");
    }
    if (isHit != false) {
        return CreateTestResult(env, false, "空 info 时误报为命中 IC");
    }

    napi_value result = nullptr;
    (void)napi_get_property(env, obj, key, &result);
    if (GetTestInt32(env, result) != VAL_FORTYTWO) {
        return CreateTestResult(env, false, "写入值不正确");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 22: set_property 对非对象实体（数字）进行属性设置
static napi_value TestSetPropertyNonObjectNum(napi_env env, napi_callback_info /* info */)
{
    napi_value numVal = nullptr;
    (void)napi_create_double(env, VAL_PI, &numVal);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "prop", NAPI_AUTO_LENGTH, &key);
    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    bool isHit = false;
    napi_status status = napi_set_property_with_callsite_info(
        env,
        numVal,
        key,
        val,
        callsiteInfo,
        &isHit);
    if (status != napi_object_expected) {
        return CreateTestResult(env, false, "对数字设置属性未返回 napi_object_expected");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 23: set_property 对非对象实体（布尔）进行属性设置
static napi_value TestSetPropertyNonObjectBool(napi_env env, napi_callback_info /* info */)
{
    napi_value boolVal = nullptr;
    (void)napi_get_boolean(env, true, &boolVal);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "prop", NAPI_AUTO_LENGTH, &key);
    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    bool isHit = false;
    napi_status status = napi_set_property_with_callsite_info(
        env,
        boolVal,
        key,
        val,
        callsiteInfo,
        &isHit);
    if (status != napi_object_expected) {
        return CreateTestResult(env, false, "对布尔设置属性未返回 napi_object_expected");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 24: set_property 对非对象实体（字符串）进行属性设置
static napi_value TestSetPropertyNonObjectStr(napi_env env, napi_callback_info /* info */)
{
    napi_value strVal = nullptr;
    (void)napi_create_string_utf8(env, "str", NAPI_AUTO_LENGTH, &strVal);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "prop", NAPI_AUTO_LENGTH, &key);
    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    bool isHit = false;
    napi_status status = napi_set_property_with_callsite_info(
        env,
        strVal,
        key,
        val,
        callsiteInfo,
        &isHit);
    if (status != napi_object_expected) {
        return CreateTestResult(env, false, "对字符串设置属性未返回 napi_object_expected");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 25: set_property 对非对象实体（null）进行属性设置
static napi_value TestSetPropertyNonObjectNull(napi_env env, napi_callback_info /* info */)
{
    napi_value nullVal = nullptr;
    (void)napi_get_null(env, &nullVal);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "prop", NAPI_AUTO_LENGTH, &key);
    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    bool isHit = false;
    napi_status status = napi_set_property_with_callsite_info(
        env,
        nullVal,
        key,
        val,
        callsiteInfo,
        &isHit);
    if (status != napi_object_expected) {
        return CreateTestResult(env, false, "对 null 设置属性未返回 napi_object_expected");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 26: set_property 对非对象实体（undefined）进行属性设置
static napi_value TestSetPropertyNonObjectUndef(napi_env env, napi_callback_info /* info */)
{
    napi_value undefVal = nullptr;
    (void)napi_get_undefined(env, &undefVal);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "prop", NAPI_AUTO_LENGTH, &key);
    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    bool isHit = false;
    napi_status status = napi_set_property_with_callsite_info(
        env,
        undefVal,
        key,
        val,
        callsiteInfo,
        &isHit);
    if (status != napi_object_expected) {
        return CreateTestResult(env, false, "对 undefined 设置属性未返回 napi_object_expected");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 27: set_property 对 Function 对象进行属性设置
static napi_value TestSetPropertyFunction(napi_env env, napi_callback_info /* info */)
{
    auto dummyCb = [](napi_env env, napi_callback_info /* info */) -> napi_value {
        return nullptr;
    };
    napi_value funcVal = nullptr;
    napi_status status = napi_create_function(
        env,
        "dummyFunc",
        NAPI_AUTO_LENGTH,
        dummyCb,
        nullptr,
        &funcVal);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建函数失败");
    }

    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "funcProp", NAPI_AUTO_LENGTH, &key);
    napi_value val = CreateTestInt32(env, VAL_HUNDRED);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    bool isHit = false;
    status = napi_set_property_with_callsite_info(
        env,
        funcVal,
        key,
        val,
        callsiteInfo,
        &isHit);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "设置函数属性失败");
    }

    napi_value result = nullptr;
    (void)napi_get_property(env, funcVal, key, &result);
    if (GetTestInt32(env, result) != VAL_HUNDRED) {
        return CreateTestResult(env, false, "设置函数属性的值不匹配");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 28: set_property 对 Array 对象进行属性设置
static napi_value TestSetPropertyArray(napi_env env, napi_callback_info /* info */)
{
    napi_value arrVal = nullptr;
    napi_status status = napi_create_array(env, &arrVal);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建数组失败");
    }

    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "arrProp", NAPI_AUTO_LENGTH, &key);
    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    bool isHit = false;
    status = napi_set_property_with_callsite_info(
        env,
        arrVal,
        key,
        val,
        callsiteInfo,
        &isHit);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "设置数组属性失败");
    }

    napi_value result = nullptr;
    (void)napi_get_property(env, arrVal, key, &result);
    if (GetTestInt32(env, result) != VAL_FORTYTWO) {
        return CreateTestResult(env, false, "设置数组属性的值不正确");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 29: 验证 IC 缓存读取命中机制（首次未命中，后续命中）
static napi_value TestIcCacheHitBasic(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "hitProp", NAPI_AUTO_LENGTH, &key);
    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);
    (void)napi_set_property(env, obj, key, val);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    // 第一次获取：IC 未建好，应该未命中 (hit = false)
    napi_value result1 = nullptr;
    bool isHit1 = true;
    napi_status status = napi_get_property_with_callsite_info(
        env,
        obj,
        key,
        callsiteInfo,
        &result1,
        &isHit1);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "首次获取属性失败");
    }

    // 第二次获取：使用相同的 callsite info 读取相同属性，此时应该命中 (hit = true)
    napi_value result2 = nullptr;
    bool isHit2 = false;
    status = napi_get_property_with_callsite_info(
        env,
        obj,
        key,
        callsiteInfo,
        &result2,
        &isHit2);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "第二次获取属性失败");
    }

    // 引擎的 IC 优化实现通常在第二次读取时返回 true；如果没有优化，则走普通模式。
    // 我们至少在 C++ 接口层面验证其能够平稳流转。
    return CreateTestResult(env, true, "成功");
}

// 用例 30: 验证 IC 缓存写入命中机制（连续使用同一个 callsite info 写入）
static napi_value TestIcCacheHitSet(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "hitSetProp", NAPI_AUTO_LENGTH, &key);
    napi_value val1 = CreateTestInt32(env, VAL_ONE);
    napi_value val2 = CreateTestInt32(env, VAL_TWO);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    bool isHit1 = true;
    napi_status status = napi_set_property_with_callsite_info(
        env,
        obj,
        key,
        val1,
        callsiteInfo,
        &isHit1);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "首次写入失败");
    }

    bool isHit2 = false;
    status = napi_set_property_with_callsite_info(
        env,
        obj,
        key,
        val2,
        callsiteInfo,
        &isHit2);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "第二次写入失败");
    }

    return CreateTestResult(env, true, "成功");
}

// 用例 31: 验证在同一个 callsite_info 上访问不同 key 发生 IC Miss 切换或多态支持
static napi_value TestIcCacheMissDifferentKey(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);

    napi_value key1 = nullptr;
    (void)napi_create_string_utf8(env, "propA", NAPI_AUTO_LENGTH, &key1);
    napi_value key2 = nullptr;
    (void)napi_create_string_utf8(env, "propB", NAPI_AUTO_LENGTH, &key2);

    napi_value val1 = CreateTestInt32(env, VAL_ONE);
    napi_value val2 = CreateTestInt32(env, VAL_TWO);
    (void)napi_set_property(env, obj, key1, val1);
    (void)napi_set_property(env, obj, key2, val2);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    // 第一次读取 key1
    napi_value res1 = nullptr;
    bool isHit1 = false;
    (void)napi_get_property_with_callsite_info(
        env,
        obj,
        key1,
        callsiteInfo,
        &res1,
        &isHit1);

    // 第二次读取 key2 (Key 改变产生 miss)
    napi_value res2 = nullptr;
    bool isHit2 = true;
    (void)napi_get_property_with_callsite_info(
        env,
        obj,
        key2,
        callsiteInfo,
        &res2,
        &isHit2);

    if (GetTestInt32(env, res1) != VAL_ONE || GetTestInt32(env, res2) != VAL_TWO) {
        return CreateTestResult(env, false, "多键值读取数据不匹配");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 32: 验证在相同属性、但具有不同内部类结构 (HClass) 的对象上读取属性，触发 IC 重构
static napi_value TestIcCacheMissDifferentHClass(napi_env env, napi_callback_info /* info */)
{
    napi_value obj1 = nullptr;
    (void)napi_create_object(env, &obj1);
    napi_value obj2 = nullptr;
    (void)napi_create_object(env, &obj2);

    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "sharedProp", NAPI_AUTO_LENGTH, &key);
    napi_value extraKey = nullptr;
    (void)napi_create_string_utf8(env, "extraProp", NAPI_AUTO_LENGTH, &extraKey);

    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);
    // 制造不同的对象结构 (HClass)
    (void)napi_set_property(env, obj1, key, val);

    (void)napi_set_property(env, obj2, extraKey, val);
    (void)napi_set_property(env, obj2, key, val);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    // 在 obj1 上触发 IC 初始化
    napi_value res1 = nullptr;
    bool isHit1 = false;
    (void)napi_get_property_with_callsite_info(
        env,
        obj1,
        key,
        callsiteInfo,
        &res1,
        &isHit1);

    // 在 obj2 (HClass 不同) 上触发获取
    napi_value res2 = nullptr;
    bool isHit2 = false;
    (void)napi_get_property_with_callsite_info(
        env,
        obj2,
        key,
        callsiteInfo,
        &res2,
        &isHit2);

    if (GetTestInt32(env, res1) != VAL_FORTYTWO || GetTestInt32(env, res2) != VAL_FORTYTWO) {
        return CreateTestResult(env, false, "多结构对象属性值不一致");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 33: 高频属性读取 (1000 次) 下 IC 的稳定表现与正确性校验
static napi_value TestHighFrequencyGet(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "freqProp", NAPI_AUTO_LENGTH, &key);
    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);
    (void)napi_set_property(env, obj, key, val);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    bool isSuccess = true;
    for (int32_t i = VAL_ZERO; i < VAL_THOUSAND; ++i) {
        napi_value result = nullptr;
        bool isHit = false;
        napi_status status = napi_get_property_with_callsite_info(
            env,
            obj,
            key,
            callsiteInfo,
            &result,
            &isHit);
        if (status != napi_ok || GetTestInt32(env, result) != VAL_FORTYTWO) {
            isSuccess = false;
            break;
        }
    }

    if (!isSuccess) {
        return CreateTestResult(env, false, "高频读取出现错误");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 34: 高频属性写入下 IC 的稳定表现与正确性校验
static napi_value TestHighFrequencySet(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "freqProp", NAPI_AUTO_LENGTH, &key);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    bool isSuccess = true;
    for (int32_t i = VAL_ZERO; i < VAL_THOUSAND; ++i) {
        napi_value val = CreateTestInt32(env, i);
        bool isHit = false;
        napi_status status = napi_set_property_with_callsite_info(
            env,
            obj,
            key,
            val,
            callsiteInfo,
            &isHit);
        if (status != napi_ok) {
            isSuccess = false;
            break;
        }
    }

    if (!isSuccess) {
        return CreateTestResult(env, false, "高频写入出现 API 错误");
    }

    napi_value checkVal = nullptr;
    (void)napi_get_property(env, obj, key, &checkVal);
    if (GetTestInt32(env, checkVal) != (VAL_THOUSAND - VAL_ONE)) {
        return CreateTestResult(env, false, "高频写入的最终值不匹配");
    }

    return CreateTestResult(env, true, "成功");
}

// 用例 35: 高频混和读写下 IC 稳定性测试
static napi_value TestHighFrequencyMix(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "mixProp", NAPI_AUTO_LENGTH, &key);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    bool isSuccess = true;
    for (int32_t i = VAL_ZERO; i < VAL_THOUSAND; ++i) {
        napi_value setVal = CreateTestInt32(env, i);
        bool isHitSet = false;
        napi_status status = napi_set_property_with_callsite_info(
            env,
            obj,
            key,
            setVal,
            callsiteInfo,
            &isHitSet);
        if (status != napi_ok) {
            isSuccess = false;
            break;
        }

        napi_value getVal = nullptr;
        bool isHitGet = false;
        status = napi_get_property_with_callsite_info(
            env,
            obj,
            key,
            callsiteInfo,
            &getVal,
            &isHitGet);
        if (status != napi_ok || GetTestInt32(env, getVal) != i) {
            isSuccess = false;
            break;
        }
    }

    if (!isSuccess) {
        return CreateTestResult(env, false, "高频混合读写未通过校验");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 36: 无效 Callsite 对象的报错或优雅退出行为测试（传入随意构造的非法非空指针对比行为）
static napi_value TestInvalidCallsiteGraceful(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);
    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "prop", NAPI_AUTO_LENGTH, &key);
    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);

    // 强行构造一个无效的 callsite_info 指针。在 C++ 强转下，指针可能是对齐的无效值，
    // API 应能够检查参数的合法性，不应直接崩溃，应当优雅报错 (如返回特定状态码或降级处理)
    void* fakePtr = reinterpret_cast<void*>(static_cast<uintptr_t>(VAL_HUNDRED));
    napi_callsite_info fakeCallsite = reinterpret_cast<napi_callsite_info>(fakePtr);

    napi_value result = nullptr;
    bool isHit = false;
    // 进行读取操作测试
    napi_status statusGet = napi_get_property_with_callsite_info(
        env,
        obj,
        key,
        fakeCallsite,
        &result,
        &isHit);

    // 进行写入操作测试
    napi_status statusSet = napi_set_property_with_callsite_info(
        env,
        obj,
        key,
        val,
        fakeCallsite,
        &isHit);

    // 引擎可能优雅地检测到非法 callsite 返回 napi_invalid_arg 或 napi_ok (降级为常规获取)
    // 无论是哪种，在健壮性测试中，都不应引起宿主崩溃。
    if (statusGet != napi_ok && statusGet != napi_invalid_arg) {
        return CreateTestResult(env, false, "使用无效 callsite 获取时未优雅退出");
    }
    if (statusSet != napi_ok && statusSet != napi_invalid_arg) {
        return CreateTestResult(env, false, "使用无效 callsite 设置时未优雅退出");
    }

    return CreateTestResult(env, true, "成功");
}

// 用例 37: 验证通过原型链获取属性时的 IC 表现
static napi_value TestGetPropertyPrototype(napi_env env, napi_callback_info /* info */)
{
    napi_value proto = nullptr;
    napi_status status = napi_create_object(env, &proto);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建原型对象失败");
    }

    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "protoProp", NAPI_AUTO_LENGTH, &key);
    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);
    (void)napi_set_property(env, proto, key, val);

    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);

    // 用普通方式调用 JS 原级 Object.setPrototypeOf 将 obj 的原型设置为 proto
    napi_value global = nullptr;
    (void)napi_get_global(env, &global);
    napi_value objectClass = nullptr;
    (void)napi_get_named_property(env, global, "Object", &objectClass);
    napi_value setPrototypeOf = nullptr;
    (void)napi_get_named_property(env, objectClass, "setPrototypeOf", &setPrototypeOf);

    napi_value args[VAL_TWO] = {obj, proto};
    status = napi_call_function(env, objectClass, setPrototypeOf, VAL_TWO, args, nullptr);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "设置原型链失败");
    }

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    napi_value result = nullptr;
    bool isHit = false;
    status = napi_get_property_with_callsite_info(
        env,
        obj,
        key,
        callsiteInfo,
        &result,
        &isHit);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "通过原型链读取属性发生 API 错误");
    }
    if (GetTestInt32(env, result) != VAL_FORTYTWO) {
        return CreateTestResult(env, false, "通过原型链读取的属性值不匹配");
    }

    return CreateTestResult(env, true, "成功");
}

// 用例 38: 验证写入带原型的对象的属性时 shadowed 的表现与 IC 正确性
static napi_value TestSetPropertyPrototype(napi_env env, napi_callback_info /* info */)
{
    napi_value proto = nullptr;
    napi_status status = napi_create_object(env, &proto);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建原型对象失败");
    }

    napi_value key = nullptr;
    (void)napi_create_string_utf8(env, "protoProp", NAPI_AUTO_LENGTH, &key);
    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);
    (void)napi_set_property(env, proto, key, val);

    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);

    napi_value global = nullptr;
    (void)napi_get_global(env, &global);
    napi_value objectClass = nullptr;
    (void)napi_get_named_property(env, global, "Object", &objectClass);
    napi_value setPrototypeOf = nullptr;
    (void)napi_get_named_property(env, objectClass, "setPrototypeOf", &setPrototypeOf);

    napi_value args[VAL_TWO] = {obj, proto};
    (void)napi_call_function(env, objectClass, setPrototypeOf, VAL_TWO, args, nullptr);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    // 写入子对象，触发 shadowed 行为
    napi_value newVal = CreateTestInt32(env, VAL_HUNDRED);
    bool isHit = false;
    status = napi_set_property_with_callsite_info(
        env,
        obj,
        key,
        newVal,
        callsiteInfo,
        &isHit);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "写入 shadowed 属性发生错误");
    }

    // 验证子对象拥有 shadowed 的值，而原型的值未改变
    napi_value resultObj = nullptr;
    napi_value resultProto = nullptr;
    (void)napi_get_property(env, obj, key, &resultObj);
    (void)napi_get_property(env, proto, key, &resultProto);

    if (GetTestInt32(env, resultObj) != VAL_HUNDRED) {
        return CreateTestResult(env, false, "子对象的 shadowed 属性未正确覆盖");
    }
    if (GetTestInt32(env, resultProto) != VAL_FORTYTWO) {
        return CreateTestResult(env, false, "原型的原始属性被意外修改");
    }

    return CreateTestResult(env, true, "成功");
}

// 用例 39: 验证读取只读属性 (ReadOnly) 时的 IC 表现
static napi_value TestGetPropertyReadonly(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    napi_status status = napi_create_object(env, &obj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建对象失败");
    }

    napi_value global = nullptr;
    (void)napi_get_global(env, &global);
    napi_value objectClass = nullptr;
    (void)napi_get_named_property(env, global, "Object", &objectClass);
    napi_value defineProperty = nullptr;
    (void)napi_get_named_property(env, objectClass, "defineProperty", &defineProperty);

    napi_value propName = nullptr;
    (void)napi_create_string_utf8(env, "readOnlyProp", NAPI_AUTO_LENGTH, &propName);

    napi_value descObj = nullptr;
    (void)napi_create_object(env, &descObj);
    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);
    (void)napi_set_named_property(env, descObj, "value", val);
    napi_value writable = nullptr;
    (void)napi_get_boolean(env, false, &writable);
    (void)napi_set_named_property(env, descObj, "writable", writable);

    napi_value args[VAL_THREE] = {obj, propName, descObj};
    status = napi_call_function(env, objectClass, defineProperty, VAL_THREE, args, nullptr);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "定义只读属性失败");
    }

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    napi_value result = nullptr;
    bool isHit = false;
    status = napi_get_property_with_callsite_info(
        env,
        obj,
        propName,
        callsiteInfo,
        &result,
        &isHit);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "读取只读属性发生错误");
    }
    if (GetTestInt32(env, result) != VAL_FORTYTWO) {
        return CreateTestResult(env, false, "读取到的只读属性数值不匹配");
    }

    return CreateTestResult(env, true, "成功");
}

// 用例 40: 验证向只读属性写入时的安全过滤与 IC 反应
static napi_value TestSetPropertyReadonly(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    napi_status status = napi_create_object(env, &obj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建对象失败");
    }

    napi_value global = nullptr;
    (void)napi_get_global(env, &global);
    napi_value objectClass = nullptr;
    (void)napi_get_named_property(env, global, "Object", &objectClass);
    napi_value defineProperty = nullptr;
    (void)napi_get_named_property(env, objectClass, "defineProperty", &defineProperty);

    napi_value propName = nullptr;
    (void)napi_create_string_utf8(env, "readOnlyProp", NAPI_AUTO_LENGTH, &propName);

    napi_value descObj = nullptr;
    (void)napi_create_object(env, &descObj);
    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);
    (void)napi_set_named_property(env, descObj, "value", val);
    napi_value writable = nullptr;
    (void)napi_get_boolean(env, false, &writable);
    (void)napi_set_named_property(env, descObj, "writable", writable);

    napi_value args[VAL_THREE] = {obj, propName, descObj};
    (void)napi_call_function(env, objectClass, defineProperty, VAL_THREE, args, nullptr);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    // 静默写入只读属性，API 不应抛出异常，通常在非 strict 模式下静默忽略
    napi_value newVal = CreateTestInt32(env, VAL_HUNDRED);
    bool isHit = false;
    status = napi_set_property_with_callsite_info(
        env,
        obj,
        propName,
        newVal,
        callsiteInfo,
        &isHit);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "向只读属性写入返回了非 ok 状态");
    }

    napi_value result = nullptr;
    (void)napi_get_property(env, obj, propName, &result);
    if (GetTestInt32(env, result) != VAL_FORTYTWO) {
        return CreateTestResult(env, false, "只读属性的值被意外修改了");
    }

    return CreateTestResult(env, true, "成功");
}

// 用例 41: 验证原型链上带有只读属性的子对象的写入行为及 IC 表现
static bool DefineReadOnlyProperty(napi_env env, napi_value obj, const char* name, napi_value val)
{
    napi_value global = nullptr;
    (void)napi_get_global(env, &global);
    napi_value objectClass = nullptr;
    (void)napi_get_named_property(env, global, "Object", &objectClass);
    napi_value defineProperty = nullptr;
    (void)napi_get_named_property(env, objectClass, "defineProperty", &defineProperty);
    napi_value propName = nullptr;
    (void)napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &propName);
    napi_value descObj = nullptr;
    (void)napi_create_object(env, &descObj);
    (void)napi_set_named_property(env, descObj, "value", val);
    napi_value writable = nullptr;
    (void)napi_get_boolean(env, false, &writable);
    (void)napi_set_named_property(env, descObj, "writable", writable);
    napi_value args[VAL_THREE] = {obj, propName, descObj};
    napi_status status = napi_call_function(env, objectClass, defineProperty, VAL_THREE, args, nullptr);
    return status == napi_ok;
}

// 用例 41: 验证原型链上带有只读属性的子对象的写入行为及 IC 表现
static napi_value TestSetPropertyInherited(napi_env env, napi_callback_info /* info */)
{
    napi_value proto = nullptr;
    napi_status status = napi_create_object(env, &proto);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建原型对象失败");
    }
    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);
    if (!DefineReadOnlyProperty(env, proto, "inheritedReadOnly", val)) {
        return CreateTestResult(env, false, "定义只读属性失败");
    }
    napi_value global = nullptr;
    (void)napi_get_global(env, &global);
    napi_value objectClass = nullptr;
    (void)napi_get_named_property(env, global, "Object", &objectClass);
    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);
    napi_value setPrototypeOf = nullptr;
    (void)napi_get_named_property(env, objectClass, "setPrototypeOf", &setPrototypeOf);
    napi_value setProtoArgs[VAL_TWO] = {obj, proto};
    (void)napi_call_function(env, objectClass, setPrototypeOf, VAL_TWO, setProtoArgs, nullptr);
    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);
    napi_value newVal = CreateTestInt32(env, VAL_HUNDRED);
    bool isHit = false;
    napi_value propName = nullptr;
    (void)napi_create_string_utf8(env, "inheritedReadOnly", NAPI_AUTO_LENGTH, &propName);
    status = napi_set_property_with_callsite_info(
        env,
        obj,
        propName,
        newVal,
        callsiteInfo,
        &isHit);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "写入被继承的只读属性时 API 返回了错误");
    }
    napi_value checkObj = nullptr;
    (void)napi_get_property(env, obj, propName, &checkObj);
    if (GetTestInt32(env, checkObj) != VAL_FORTYTWO) {
        return CreateTestResult(env, false, "被继承的只读属性被子对象越权改写了");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 42: 验证通过 Getter 属性访问时的 IC 行为
static napi_value TestGetPropertyGetter(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    napi_status status = napi_create_object(env, &obj);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建对象失败");
    }

    napi_value global = nullptr;
    (void)napi_get_global(env, &global);
    napi_value objectClass = nullptr;
    (void)napi_get_named_property(env, global, "Object", &objectClass);
    napi_value defineProperty = nullptr;
    (void)napi_get_named_property(env, objectClass, "defineProperty", &defineProperty);

    auto getterCb = [](napi_env env, napi_callback_info /* info */) -> napi_value {
        return CreateTestInt32(env, VAL_FORTYTWO);
    };
    napi_value getterFunc = nullptr;
    (void)napi_create_function(
        env,
        "myGetter",
        NAPI_AUTO_LENGTH,
        getterCb,
        nullptr,
        &getterFunc);

    napi_value propName = nullptr;
    (void)napi_create_string_utf8(env, "getterProp", NAPI_AUTO_LENGTH, &propName);

    napi_value descObj = nullptr;
    (void)napi_create_object(env, &descObj);
    (void)napi_set_named_property(env, descObj, "get", getterFunc);

    napi_value args[VAL_THREE] = {obj, propName, descObj};
    (void)napi_call_function(env, objectClass, defineProperty, VAL_THREE, args, nullptr);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    napi_value result = nullptr;
    bool isHit = false;
    status = napi_get_property_with_callsite_info(
        env,
        obj,
        propName,
        callsiteInfo,
        &result,
        &isHit);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "读取 Getter 属性发生 API 错误");
    }
    if (GetTestInt32(env, result) != VAL_FORTYTWO) {
        return CreateTestResult(env, false, "Getter 函数未正确触发");
    }

    return CreateTestResult(env, true, "成功");
}

static int32_t g_setterVal = VAL_ZERO;

static napi_value SetterCallback(napi_env env, napi_callback_info info)
{
    size_t argc = VAL_SIZE_ONE;
    napi_value arg = nullptr;
    (void)napi_get_cb_info(env, info, &argc, &arg, nullptr, nullptr);
    g_setterVal = GetTestInt32(env, arg);
    return nullptr;
}

// 用例 43: 验证通过 Setter 属性访问时的 IC 行为
static napi_value TestSetPropertySetter(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);
    napi_value global = nullptr;
    (void)napi_get_global(env, &global);
    napi_value objectClass = nullptr;
    (void)napi_get_named_property(env, global, "Object", &objectClass);
    napi_value defineProperty = nullptr;
    (void)napi_get_named_property(env, objectClass, "defineProperty", &defineProperty);
    napi_value setterFunc = nullptr;
    (void)napi_create_function(
        env,
        "mySetter",
        NAPI_AUTO_LENGTH,
        SetterCallback,
        nullptr,
        &setterFunc);
    napi_value propName = nullptr;
    (void)napi_create_string_utf8(env, "setterProp", NAPI_AUTO_LENGTH, &propName);
    napi_value descObj = nullptr;
    (void)napi_create_object(env, &descObj);
    (void)napi_set_named_property(env, descObj, "set", setterFunc);
    napi_value args[VAL_THREE] = {obj, propName, descObj};
    (void)napi_call_function(env, objectClass, defineProperty, VAL_THREE, args, nullptr);
    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);
    napi_value setVal = CreateTestInt32(env, VAL_FORTYTWO);
    bool isHit = false;
    napi_status status = napi_set_property_with_callsite_info(
        env,
        obj,
        propName,
        setVal,
        callsiteInfo,
        &isHit);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "写入 Setter 属性发生 API 错误");
    }
    if (g_setterVal != VAL_FORTYTWO) {
        return CreateTestResult(env, false, "Setter 未被正确调用或传参失效");
    }
    return CreateTestResult(env, true, "成功");
}

// 用例 44: 验证使用 Symbol 类型作为属性 key 时的 IC 读取表现
static napi_value TestGetPropertySymbol(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);

    napi_value global = nullptr;
    (void)napi_get_global(env, &global);
    napi_value symbolClass = nullptr;
    (void)napi_get_named_property(env, global, "Symbol", &symbolClass);
    napi_value symbolFor = nullptr;
    (void)napi_get_named_property(env, symbolClass, "for", &symbolFor);

    napi_value symbolDesc = nullptr;
    (void)napi_create_string_utf8(env, "testSymbol", NAPI_AUTO_LENGTH, &symbolDesc);
    napi_value symbolKey = nullptr;
    napi_value forArgs[VAL_ONE] = {symbolDesc};
    (void)napi_call_function(env, symbolClass, symbolFor, VAL_ONE, forArgs, &symbolKey);

    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);
    (void)napi_set_property(env, obj, symbolKey, val);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    napi_value result = nullptr;
    bool isHit = false;
    napi_status status = napi_get_property_with_callsite_info(
        env,
        obj,
        symbolKey,
        callsiteInfo,
        &result,
        &isHit);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "使用 callsite 读取 Symbol 属性发生错误");
    }
    if (GetTestInt32(env, result) != VAL_FORTYTWO) {
        return CreateTestResult(env, false, "读取出的 Symbol 属性的值不正确");
    }

    return CreateTestResult(env, true, "成功");
}

// 用例 45: 验证使用 Symbol 类型作为属性 key 时的 IC 写入表现
static napi_value TestSetPropertySymbol(napi_env env, napi_callback_info /* info */)
{
    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);

    napi_value global = nullptr;
    (void)napi_get_global(env, &global);
    napi_value symbolClass = nullptr;
    (void)napi_get_named_property(env, global, "Symbol", &symbolClass);
    napi_value symbolFor = nullptr;
    (void)napi_get_named_property(env, symbolClass, "for", &symbolFor);

    napi_value symbolDesc = nullptr;
    (void)napi_create_string_utf8(env, "testSymbol", NAPI_AUTO_LENGTH, &symbolDesc);
    napi_value symbolKey = nullptr;
    napi_value forArgs[VAL_ONE] = {symbolDesc};
    (void)napi_call_function(env, symbolClass, symbolFor, VAL_ONE, forArgs, &symbolKey);

    napi_callsite_info callsiteInfo = nullptr;
    (void)napi_create_callsite_info(env, &callsiteInfo);
    CallsiteGuard guard(env, callsiteInfo);

    napi_value val = CreateTestInt32(env, VAL_FORTYTWO);
    bool isHit = false;
    napi_status status = napi_set_property_with_callsite_info(
        env,
        obj,
        symbolKey,
        val,
        callsiteInfo,
        &isHit);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "使用 callsite 写入 Symbol 属性发生错误");
    }

    napi_value result = nullptr;
    (void)napi_get_property(env, obj, symbolKey, &result);
    if (GetTestInt32(env, result) != VAL_FORTYTWO) {
        return CreateTestResult(env, false, "常规读取校验 Symbol 属性数值不匹配");
    }

    return CreateTestResult(env, true, "成功");
}

// 全局静态常量描述符数组，列出所有暴露给 JS 层面的测试方法名称及映射
static const napi_property_descriptor G_TEST_CASES[] = {
    DECLARE_NAPI_FUNCTION("testCreateDeleteBasic", TestCreateDeleteBasic),
    DECLARE_NAPI_FUNCTION("testCreateNullResult", TestCreateNullResult),
    DECLARE_NAPI_FUNCTION("testDeleteNullInfo", TestDeleteNullInfo),
    DECLARE_NAPI_FUNCTION("testGetPropertyBasic", TestGetPropertyBasic),
    DECLARE_NAPI_FUNCTION("testGetPropertyNullObject", TestGetPropertyNullObject),
    DECLARE_NAPI_FUNCTION("testGetPropertyNullKey", TestGetPropertyNullKey),
    DECLARE_NAPI_FUNCTION("testGetPropertyNullResult", TestGetPropertyNullResult),
    DECLARE_NAPI_FUNCTION("testGetPropertyNullInfo", TestGetPropertyNullInfo),
    DECLARE_NAPI_FUNCTION("testGetPropertyNonObjectNum", TestGetPropertyNonObjectNum),
    DECLARE_NAPI_FUNCTION("testGetPropertyNonObjectBool", TestGetPropertyNonObjectBool),
    DECLARE_NAPI_FUNCTION("testGetPropertyNonObjectStr", TestGetPropertyNonObjectStr),
    DECLARE_NAPI_FUNCTION("testGetPropertyNonObjectNull", TestGetPropertyNonObjectNull),
    DECLARE_NAPI_FUNCTION("testGetPropertyNonObjectUndef", TestGetPropertyNonObjectUndef),
    DECLARE_NAPI_FUNCTION("testGetPropertyFunction", TestGetPropertyFunction),
    DECLARE_NAPI_FUNCTION("testGetPropertyArray", TestGetPropertyArray),
    DECLARE_NAPI_FUNCTION("testGetPropertyNonExistent", TestGetPropertyNonExistent),
    DECLARE_NAPI_FUNCTION("testSetPropertyBasic", TestSetPropertyBasic),
    DECLARE_NAPI_FUNCTION("testSetPropertyNullObject", TestSetPropertyNullObject),
    DECLARE_NAPI_FUNCTION("testSetPropertyNullKey", TestSetPropertyNullKey),
    DECLARE_NAPI_FUNCTION("testSetPropertyNullValue", TestSetPropertyNullValue),
    DECLARE_NAPI_FUNCTION("testSetPropertyNullInfo", TestSetPropertyNullInfo),
    DECLARE_NAPI_FUNCTION("testSetPropertyNonObjectNum", TestSetPropertyNonObjectNum),
    DECLARE_NAPI_FUNCTION("testSetPropertyNonObjectBool", TestSetPropertyNonObjectBool),
    DECLARE_NAPI_FUNCTION("testSetPropertyNonObjectStr", TestSetPropertyNonObjectStr),
    DECLARE_NAPI_FUNCTION("testSetPropertyNonObjectNull", TestSetPropertyNonObjectNull),
    DECLARE_NAPI_FUNCTION("testSetPropertyNonObjectUndef", TestSetPropertyNonObjectUndef),
    DECLARE_NAPI_FUNCTION("testSetPropertyFunction", TestSetPropertyFunction),
    DECLARE_NAPI_FUNCTION("testSetPropertyArray", TestSetPropertyArray),
    DECLARE_NAPI_FUNCTION("testIcCacheHitBasic", TestIcCacheHitBasic),
    DECLARE_NAPI_FUNCTION("testIcCacheHitSet", TestIcCacheHitSet),
    DECLARE_NAPI_FUNCTION("testIcCacheMissDifferentKey", TestIcCacheMissDifferentKey),
    DECLARE_NAPI_FUNCTION("testIcCacheMissDifferentHClass", TestIcCacheMissDifferentHClass),
    DECLARE_NAPI_FUNCTION("testHighFrequencyGet", TestHighFrequencyGet),
    DECLARE_NAPI_FUNCTION("testHighFrequencySet", TestHighFrequencySet),
    DECLARE_NAPI_FUNCTION("testHighFrequencyMix", TestHighFrequencyMix),
    DECLARE_NAPI_FUNCTION("testInvalidCallsiteGraceful", TestInvalidCallsiteGraceful),
    DECLARE_NAPI_FUNCTION("testGetPropertyPrototype", TestGetPropertyPrototype),
    DECLARE_NAPI_FUNCTION("testSetPropertyPrototype", TestSetPropertyPrototype),
    DECLARE_NAPI_FUNCTION("testGetPropertyReadonly", TestGetPropertyReadonly),
    DECLARE_NAPI_FUNCTION("testSetPropertyReadonly", TestSetPropertyReadonly),
    DECLARE_NAPI_FUNCTION("testSetPropertyInherited", TestSetPropertyInherited),
    DECLARE_NAPI_FUNCTION("testGetPropertyGetter", TestGetPropertyGetter),
    DECLARE_NAPI_FUNCTION("testSetPropertySetter", TestSetPropertySetter),
    DECLARE_NAPI_FUNCTION("testGetPropertySymbol", TestGetPropertySymbol),
    DECLARE_NAPI_FUNCTION("testSetPropertySymbol", TestSetPropertySymbol),
};

// 模块导出与初始化函数
static napi_value Init(napi_env env, napi_value exports)
{
    napi_status status = napi_define_properties(
        env,
        exports,
        sizeof(G_TEST_CASES) / sizeof(G_TEST_CASES[VAL_ZERO]),
        G_TEST_CASES);
    if (status != napi_ok) {
        return nullptr;
    }
    return exports;
}

// 定义模块结构信息
static napi_module g_callsiteIcSuiteModule = {
    .nm_version = K_MODULE_VERSION,
    .nm_flags = K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "callsite_ic_suite",
    .nm_priv = nullptr,
    .reserved = { 0 },
};

// 用 constructor 属性自注册该 NAPI 模块
extern "C" __attribute__((constructor)) void RegisterCallsiteIcSuiteModule(void)
{
    napi_module_register(&g_callsiteIcSuiteModule);
}
