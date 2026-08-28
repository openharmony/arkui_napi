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
#include "securec.h"

// 匿名命名空间，用于定义文件级常量，保证无魔鬼数字
namespace {
constexpr int32_t VAL_ZERO = 0;
constexpr int32_t VAL_ONE = 1;
constexpr int32_t VAL_MINUS_ONE = -1;
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t VAL_100 = 100;
constexpr int32_t VAL_123 = 123;
constexpr int32_t VAL_456 = 456;
constexpr int32_t VAL_789 = 789;
constexpr int32_t VAL_999 = 999;
constexpr double PI_VAL = 3.14159;
constexpr double E_VAL = 2.71828;
constexpr size_t BUF_SIZE_32 = 32;
constexpr size_t BUF_SIZE_64 = 64;
constexpr size_t ITER_LIMIT = 200;
constexpr uint32_t K_MODULE_VERSION = 1;
constexpr uint32_t K_NO_MODULE_FLAGS = 0;
constexpr uint32_t EXPECTED_SIZE_TWO = 2;
constexpr uint32_t EXPECTED_SIZE_FIVE = 5;
} // namespace

// 辅助函数：将布尔值转化为 JS 布尔值返回
static napi_value SetResultBool(napi_env env, bool val)
{
    napi_value result = nullptr;
    napi_status status = napi_get_boolean(
        env, val, &result);
    if (status != napi_ok) {
        return nullptr;
    }
    return result;
}

// 辅助函数：创建 JS 32位整型
static napi_value CreateJsInt32(napi_env env, int32_t val)
{
    napi_value result = nullptr;
    napi_status status = napi_create_int32(
        env, val, &result);
    if (status != napi_ok) {
        return nullptr;
    }
    return result;
}

// 辅助函数：创建 JS 双精度浮点型
static napi_value CreateJsDouble(napi_env env, double val)
{
    napi_value result = nullptr;
    napi_status status = napi_create_double(
        env, val, &result);
    if (status != napi_ok) {
        return nullptr;
    }
    return result;
}

// 辅助函数：创建 JS 字符串
static napi_value CreateJsString(napi_env env, const char* str)
{
    napi_value result = nullptr;
    napi_status status = napi_create_string_utf8(
        env, str, NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        return nullptr;
    }
    return result;
}

// 辅助函数：创建 JS 符号类型
static napi_value CreateJsSymbol(napi_env env, const char* desc)
{
    napi_value result = nullptr;
    napi_value descString = CreateJsString(
        env, desc);
    if (descString == nullptr) {
        return nullptr;
    }
    napi_status status = napi_create_symbol(
        env, descString, &result);
    if (status != napi_ok) {
        return nullptr;
    }
    return result;
}

// 辅助函数：创建普通 JS 对象
static napi_value CreateJsObject(napi_env env)
{
    napi_value result = nullptr;
    napi_status status = napi_create_object(
        env, &result);
    if (status != napi_ok) {
        return nullptr;
    }
    return result;
}

// 用例 1：测试正常创建 Map 接口
static napi_value TestCreateMapNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_valuetype type = napi_undefined;
    status = napi_typeof(
        env, map, &type);
    if (status != napi_ok || type != napi_object) {
        return SetResultBool(env, false);
    }
    uint32_t size = VAL_100;
    status = napi_map_get_size(
        env, map, &size);
    if (status != napi_ok || size != 0) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 2：测试创建 Map 时传入非法空指针
static napi_value TestCreateMapInvalid(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_status status = napi_create_map(
        env, nullptr);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 3：测试通过 napi_map_set_property 存入属性的正常场景
static napi_value TestMapSetPropertyNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value key = CreateJsString(
        env, "key1");
    napi_value value = CreateJsInt32(
        env, VAL_123);
    status = napi_map_set_property(
        env, map, key, value);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    uint32_t size = 0;
    status = napi_map_get_size(
        env, map, &size);
    if (status != napi_ok || size != 1) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 4：测试通过 napi_map_set_property 存入属性时传入非法参数
static napi_value TestMapSetPropertyInvalid(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value key = CreateJsString(
        env, "key1");
    napi_value value = CreateJsInt32(
        env, VAL_123);
    status = napi_map_set_property(
        env, nullptr, key, value);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_set_property(
        env, map, nullptr, value);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_set_property(
        env, map, key, nullptr);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 5：测试通过 napi_map_set_named_property 存入属性的正常场景
static napi_value TestMapSetNamedPropertyNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value value = CreateJsInt32(
        env, VAL_456);
    status = napi_map_set_named_property(
        env, map, "namedKey", value);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    uint32_t size = 0;
    status = napi_map_get_size(
        env, map, &size);
    if (status != napi_ok || size != 1) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 6：测试通过 napi_map_set_named_property 存入属性时传入非法参数
static napi_value TestMapSetNamedPropertyInvalid(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value value = CreateJsInt32(
        env, VAL_456);
    status = napi_map_set_named_property(
        env, nullptr, "namedKey", value);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_set_named_property(
        env, map, nullptr, value);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_set_named_property(
        env, map, "namedKey", nullptr);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 7：测试通过 napi_map_get_property 获取属性的正常场景
static napi_value TestMapGetPropertyNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value key = CreateJsString(
        env, "testKey");
    napi_value value = CreateJsInt32(
        env, VAL_789);
    status = napi_map_set_property(
        env, map, key, value);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value result = nullptr;
    status = napi_map_get_property(
        env, map, key, &result);
    if (status != napi_ok || result == nullptr) {
        return SetResultBool(env, false);
    }
    int32_t valOut = 0;
    status = napi_get_value_int32(
        env, result, &valOut);
    if (status != napi_ok || valOut != VAL_789) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 8：测试通过 napi_map_get_property 获取属性时传入非法参数
static napi_value TestMapGetPropertyInvalid(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value key = CreateJsString(
        env, "testKey");
    napi_value result = nullptr;
    status = napi_map_get_property(
        env, nullptr, key, &result);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_get_property(
        env, map, nullptr, &result);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_get_property(
        env, map, key, nullptr);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 9：测试通过 napi_map_get_property 获取不存在属性的返回类型
static napi_value TestMapGetPropertyNonExist(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value key = CreateJsString(
        env, "nonExistKey");
    napi_value result = nullptr;
    status = napi_map_get_property(
        env, map, key, &result);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_valuetype type = napi_object;
    status = napi_typeof(
        env, result, &type);
    if (status != napi_ok || type != napi_undefined) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 10：测试通过 napi_map_get_named_property 获取命名属性的正常场景
static napi_value TestMapGetNamedPropertyNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value value = CreateJsDouble(
        env, PI_VAL);
    status = napi_map_set_named_property(
        env, map, "namedDouble", value);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value result = nullptr;
    status = napi_map_get_named_property(
        env, map, "namedDouble", &result);
    if (status != napi_ok || result == nullptr) {
        return SetResultBool(env, false);
    }
    double valOut = 0.0;
    status = napi_get_value_double(
        env, result, &valOut);
    if (status != napi_ok || valOut != PI_VAL) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 11：测试通过 napi_map_get_named_property 获取命名属性时传入非法参数
static napi_value TestMapGetNamedPropertyInvalid(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value result = nullptr;
    status = napi_map_get_named_property(
        env, nullptr, "namedDouble", &result);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_get_named_property(
        env, map, nullptr, &result);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_get_named_property(
        env, map, "namedDouble", nullptr);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 12：测试通过 napi_map_get_named_property 获取不存在的命名属性
static napi_value TestMapGetNamedPropertyNonExist(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value result = nullptr;
    status = napi_map_get_named_property(
        env, map, "nonExistNamedKey", &result);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_valuetype type = napi_object;
    status = napi_typeof(
        env, result, &type);
    if (status != napi_ok || type != napi_undefined) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 13：测试通过 napi_map_has_property 检查属性存在的正常场景
static napi_value TestMapHasPropertyNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value key = CreateJsString(
        env, "hasKey");
    napi_value value = CreateJsInt32(
        env, VAL_999);
    status = napi_map_set_property(
        env, map, key, value);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    bool hasProp = false;
    status = napi_map_has_property(
        env, map, key, &hasProp);
    if (status != napi_ok || !hasProp) {
        return SetResultBool(env, false);
    }
    napi_value nonExistKey = CreateJsString(
        env, "noHasKey");
    status = napi_map_has_property(
        env, map, nonExistKey, &hasProp);
    if (status != napi_ok || hasProp) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 14：测试通过 napi_map_has_property 检查属性时传入非法参数
static napi_value TestMapHasPropertyInvalid(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value key = CreateJsString(
        env, "hasKey");
    bool hasProp = false;
    status = napi_map_has_property(
        env, nullptr, key, &hasProp);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_has_property(
        env, map, nullptr, &hasProp);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_has_property(
        env, map, key, nullptr);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 15：测试通过 napi_map_has_named_property 检查命名属性的正常场景
static napi_value TestMapHasNamedPropertyNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value value = CreateJsInt32(
        env, VAL_999);
    status = napi_map_set_named_property(
        env, map, "namedHasKey", value);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    bool hasProp = false;
    status = napi_map_has_named_property(
        env, map, "namedHasKey", &hasProp);
    if (status != napi_ok || !hasProp) {
        return SetResultBool(env, false);
    }
    status = napi_map_has_named_property(
        env, map, "noNamedHasKey", &hasProp);
    if (status != napi_ok || hasProp) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 16：测试通过 napi_map_has_named_property 检查命名属性时传入非法参数
static napi_value TestMapHasNamedPropertyInvalid(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    bool hasProp = false;
    status = napi_map_has_named_property(
        env, nullptr, "namedHasKey", &hasProp);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_has_named_property(
        env, map, nullptr, &hasProp);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_has_named_property(
        env, map, "namedHasKey", nullptr);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 17：测试通过 napi_map_delete_property 删除属性的正常场景
static napi_value TestMapDeletePropertyNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value key = CreateJsString(
        env, "delKey");
    napi_value value = CreateJsInt32(
        env, VAL_123);
    status = napi_map_set_property(
        env, map, key, value);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_delete_property(
        env, map, key);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    bool hasProp = true;
    status = napi_map_has_property(
        env, map, key, &hasProp);
    if (status != napi_ok || hasProp) {
        return SetResultBool(env, false);
    }
    uint32_t size = 0;
    status = napi_map_get_size(
        env, map, &size);
    if (status != napi_ok || size != 0) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 18：测试通过 napi_map_delete_property 删除属性时传入非法参数
static napi_value TestMapDeletePropertyInvalid(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value key = CreateJsString(
        env, "delKey");
    status = napi_map_delete_property(
        env, nullptr, key);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_delete_property(
        env, map, nullptr);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 19：测试通过 napi_map_clear 清空属性的正常场景
static napi_value TestMapClearNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value val = CreateJsInt32(
        env, VAL_123);
    status = napi_map_set_named_property(
        env, map, "keyA", val);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_set_named_property(
        env, map, "keyB", val);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_clear(
        env, map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    uint32_t size = 0;
    status = napi_map_get_size(
        env, map, &size);
    if (status != napi_ok || size != 0) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 20：测试通过 napi_map_clear 清空属性时传入非法参数
static napi_value TestMapClearInvalid(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_status status = napi_map_clear(
        env, nullptr);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 21：测试通过 napi_map_get_size 获取大小时传入非法参数
static napi_value TestMapGetSizeInvalid(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    uint32_t size = 0;
    status = napi_map_get_size(
        env, nullptr, &size);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_get_size(
        env, map, nullptr);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 22：测试通过 napi_map_get_entries 获取键值对迭代器的正常场景
static napi_value TestMapGetEntriesNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value k = CreateJsString(
        env, "k");
    napi_value v = CreateJsInt32(
        env, VAL_123);
    status = napi_map_set_property(
        env, map, k, v);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value entries = nullptr;
    status = napi_map_get_entries(
        env, map, &entries);
    if (status != napi_ok || entries == nullptr) {
        return SetResultBool(env, false);
    }
    napi_valuetype type = napi_undefined;
    status = napi_typeof(
        env, entries, &type);
    if (status != napi_ok || type != napi_object) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 23：测试通过 napi_map_get_entries 获取键值对迭代器时传入非法参数
static napi_value TestMapGetEntriesInvalid(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value entries = nullptr;
    status = napi_map_get_entries(
        env, nullptr, &entries);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_get_entries(
        env, map, nullptr);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 24：测试通过 napi_map_get_keys 获取键迭代器的正常场景
static napi_value TestMapGetKeysNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value k = CreateJsString(
        env, "k");
    napi_value v = CreateJsInt32(
        env, VAL_123);
    status = napi_map_set_property(
        env, map, k, v);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value keys = nullptr;
    status = napi_map_get_keys(
        env, map, &keys);
    if (status != napi_ok || keys == nullptr) {
        return SetResultBool(env, false);
    }
    napi_valuetype type = napi_undefined;
    status = napi_typeof(
        env, keys, &type);
    if (status != napi_ok || type != napi_object) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 25：测试通过 napi_map_get_keys 获取键迭代器时传入非法参数
static napi_value TestMapGetKeysInvalid(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value keys = nullptr;
    status = napi_map_get_keys(
        env, nullptr, &keys);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_get_keys(
        env, map, nullptr);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 26：测试通过 napi_map_get_values 获取值迭代器的正常场景
static napi_value TestMapGetValuesNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value k = CreateJsString(
        env, "k");
    napi_value v = CreateJsInt32(
        env, VAL_123);
    status = napi_map_set_property(
        env, map, k, v);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value values = nullptr;
    status = napi_map_get_values(
        env, map, &values);
    if (status != napi_ok || values == nullptr) {
        return SetResultBool(env, false);
    }
    napi_valuetype type = napi_undefined;
    status = napi_typeof(
        env, values, &type);
    if (status != napi_ok || type != napi_object) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 27：测试通过 napi_map_get_values 获取值迭代器时传入非法参数
static napi_value TestMapGetValuesInvalid(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value values = nullptr;
    status = napi_map_get_values(
        env, nullptr, &values);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_get_values(
        env, map, nullptr);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 28：测试通过 napi_map_iterator_get_next 迭代时传入非法参数
static napi_value TestMapIteratorGetNextInvalid(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value keys = nullptr;
    status = napi_map_get_keys(
        env, map, &keys);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value result = nullptr;
    status = napi_map_iterator_get_next(
        env, nullptr, &result);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_iterator_get_next(
        env, keys, nullptr);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 29：测试在空 Map 上执行各种查询、删除等操作的行为
static napi_value TestMapEmptyOperations(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value key = CreateJsString(
        env, "emptyTestKey");
    bool hasProp = true;
    status = napi_map_has_property(
        env, map, key, &hasProp);
    if (status != napi_ok || hasProp) {
        return SetResultBool(env, false);
    }
    napi_value result = nullptr;
    status = napi_map_get_property(
        env, map, key, &result);
    napi_valuetype type = napi_object;
    status = napi_typeof(
        env, result, &type);
    if (status != napi_ok || type != napi_undefined) {
        return SetResultBool(env, false);
    }
    status = napi_map_delete_property(
        env, map, key);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 30：测试同一个 key 重复设置不同值时的覆盖行为
static napi_value TestMapOverwriteProperty(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value key = CreateJsString(
        env, "dupKey");
    napi_value val1 = CreateJsInt32(
        env, VAL_123);
    napi_value val2 = CreateJsInt32(
        env, VAL_456);
    status = napi_map_set_property(
        env, map, key, val1);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_set_property(
        env, map, key, val2);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    uint32_t size = 0;
    status = napi_map_get_size(
        env, map, &size);
    if (status != napi_ok || size != 1) {
        return SetResultBool(env, false);
    }
    napi_value res = nullptr;
    status = napi_map_get_property(
        env, map, key, &res);
    int32_t valOut = 0;
    status = napi_get_value_int32(
        env, res, &valOut);
    if (status != napi_ok || valOut != VAL_456) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 31：测试删除不存在的属性时 size 和状态的变化
static napi_value TestMapDeleteNonExist(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value key1 = CreateJsString(
        env, "k1");
    napi_value value = CreateJsInt32(
        env, VAL_123);
    status = napi_map_set_property(
        env, map, key1, value);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value key2 = CreateJsString(
        env, "k2");
    status = napi_map_delete_property(
        env, map, key2);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    uint32_t size = 0;
    status = napi_map_get_size(
        env, map, &size);
    if (status != napi_ok || size != 1) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 32：测试使用混合不同类型的 Key 存入 Map 的正常逻辑
static napi_value TestMapMixedKeys(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value strKey = CreateJsString(
        env, "str");
    napi_value intKey = CreateJsInt32(
        env, VAL_123);
    napi_value doubleKey = CreateJsDouble(
        env, PI_VAL);
    napi_value symKey = CreateJsSymbol(
        env, "sym");
    napi_value objKey = CreateJsObject(
        env);
    napi_value val = CreateJsInt32(
        env, VAL_999);
    status = napi_map_set_property(
        env, map, strKey, val);
    status = napi_map_set_property(
        env, map, intKey, val);
    status = napi_map_set_property(
        env, map, doubleKey, val);
    status = napi_map_set_property(
        env, map, symKey, val);
    status = napi_map_set_property(
        env, map, objKey, val);
    uint32_t size = 0;
    status = napi_map_get_size(
        env, map, &size);
    if (status != napi_ok || size != EXPECTED_SIZE_FIVE) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 33：测试完整的 entries 迭代器遍历和字段内容校验
static napi_value TestMapIteratorEntriesComplete(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value k1 = CreateJsString(
        env, "k1");
    napi_value v1 = CreateJsInt32(
        env, VAL_123);
    status = napi_map_set_property(
        env, map, k1, v1);
    napi_value entries = nullptr;
    status = napi_map_get_entries(
        env, map, &entries);
    napi_value item = nullptr;
    status = napi_map_iterator_get_next(
        env, entries, &item);
    if (status != napi_ok || item == nullptr) {
        return SetResultBool(env, false);
    }
    napi_value val = nullptr;
    status = napi_get_named_property(
        env, item, "value", &val);
    napi_value done = nullptr;
    status = napi_get_named_property(
        env, item, "done", &done);
    bool isDone = true;
    status = napi_get_value_bool(
        env, done, &isDone);
    if (status != napi_ok || isDone) {
        return SetResultBool(env, false);
    }
    napi_value kOut = nullptr;
    napi_value vOut = nullptr;
    napi_get_element(
        env, val, INDEX_ZERO, &kOut);
    napi_get_element(
        env, val, INDEX_ONE, &vOut);
    int32_t vVal = 0;
    napi_get_value_int32(
        env, vOut, &vVal);
    if (vVal != VAL_123) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 34：测试完整的 keys 迭代器遍历和键的内容校验
static napi_value TestMapIteratorKeysComplete(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value k1 = CreateJsString(
        env, "k1");
    napi_value v1 = CreateJsInt32(
        env, VAL_123);
    status = napi_map_set_property(
        env, map, k1, v1);
    napi_value keys = nullptr;
    status = napi_map_get_keys(
        env, map, &keys);
    napi_value item = nullptr;
    status = napi_map_iterator_get_next(
        env, keys, &item);
    if (status != napi_ok || item == nullptr) {
        return SetResultBool(env, false);
    }
    napi_value val = nullptr;
    status = napi_get_named_property(
        env, item, "value", &val);
    napi_value done = nullptr;
    status = napi_get_named_property(
        env, item, "done", &done);
    bool isDone = true;
    status = napi_get_value_bool(
        env, done, &isDone);
    if (status != napi_ok || isDone) {
        return SetResultBool(env, false);
    }
    size_t length = 0;
    char buf[BUF_SIZE_32] = {0};
    napi_get_value_string_utf8(
        env, val, buf, BUF_SIZE_32, &length);
    if (std::string(buf) != "k1") {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 35：测试完整的 values 迭代器遍历和值的内容校验
static napi_value TestMapIteratorValuesComplete(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value k1 = CreateJsString(
        env, "k1");
    napi_value v1 = CreateJsInt32(
        env, VAL_123);
    status = napi_map_set_property(
        env, map, k1, v1);
    napi_value values = nullptr;
    status = napi_map_get_values(
        env, map, &values);
    napi_value item = nullptr;
    status = napi_map_iterator_get_next(
        env, values, &item);
    if (status != napi_ok || item == nullptr) {
        return SetResultBool(env, false);
    }
    napi_value val = nullptr;
    status = napi_get_named_property(
        env, item, "value", &val);
    napi_value done = nullptr;
    status = napi_get_named_property(
        env, item, "done", &done);
    bool isDone = true;
    status = napi_get_value_bool(
        env, done, &isDone);
    if (status != napi_ok || isDone) {
        return SetResultBool(env, false);
    }
    int32_t valOut = 0;
    status = napi_get_value_int32(
        env, val, &valOut);
    if (valOut != VAL_123) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 36：测试 Map 写入大容量数据时的行为及大小验证
static napi_value TestMapLargeCapacity(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    for (int32_t i = 0; i < VAL_100; i++) {
        std::string keyStr = "key_" + std::to_string(i);
        napi_value key = CreateJsString(
            env, keyStr.c_str());
        napi_value val = CreateJsInt32(
            env, i);
        status = napi_map_set_property(
            env, map, key, val);
        if (status != napi_ok) {
            return SetResultBool(env, false);
        }
    }
    uint32_t size = 0;
    status = napi_map_get_size(
        env, map, &size);
    if (status != napi_ok || size != static_cast<uint32_t>(VAL_100)) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 37：测试 Map 嵌套存储（以 Map 作为值存入另一个 Map）的正常读写
static napi_value TestMapSetAndGetNested(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value parentMap = nullptr;
    napi_status status = napi_create_map(
        env, &parentMap);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value childMap = nullptr;
    status = napi_create_map(
        env, &childMap);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value keyStr = CreateJsString(
        env, "childKey");
    napi_value valStr = CreateJsString(
        env, "nestedVal");
    status = napi_map_set_property(
        env, childMap, keyStr, valStr);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_set_named_property(
        env, parentMap, "subMap", childMap);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value retrievedMap = nullptr;
    status = napi_map_get_named_property(
        env, parentMap, "subMap", &retrievedMap);
    if (status != napi_ok || retrievedMap == nullptr) {
        return SetResultBool(env, false);
    }
    napi_value retrievedVal = nullptr;
    status = napi_map_get_property(
        env, retrievedMap, keyStr, &retrievedVal);
    if (status != napi_ok || retrievedVal == nullptr) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 38：测试当迭代器完成遍历后，继续调用 iterator_get_next 的完成状态
static napi_value TestMapIteratorMultipleNext(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value key = CreateJsString(
        env, "k");
    napi_value val = CreateJsInt32(
        env, VAL_123);
    status = napi_map_set_property(
        env, map, key, val);
    napi_value keys = nullptr;
    status = napi_map_get_keys(
        env, map, &keys);
    napi_value item1 = nullptr;
    status = napi_map_iterator_get_next(
        env, keys, &item1);
    napi_value item2 = nullptr;
    status = napi_map_iterator_get_next(
        env, keys, &item2);
    napi_value done = nullptr;
    status = napi_get_named_property(
        env, item2, "done", &done);
    bool isDone = false;
    status = napi_get_value_bool(
        env, done, &isDone);
    if (status != napi_ok || !isDone) {
        return SetResultBool(env, false);
    }
    napi_value item3 = nullptr;
    status = napi_map_iterator_get_next(
        env, keys, &item3);
    status = napi_get_named_property(
        env, item3, "done", &done);
    status = napi_get_value_bool(
        env, done, &isDone);
    if (status != napi_ok || !isDone) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 39：测试在大循环中多次执行 Map 设置属性与清空属性的内存/泄漏验证
static napi_value TestMapMemoryOrClearLoop(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    for (int32_t loop = 0; loop < VAL_100; loop++) {
        napi_value key = CreateJsString(
            env, "tempKey");
        napi_value val = CreateJsInt32(
            env, loop);
        status = napi_map_set_property(
            env, map, key, val);
        if (status != napi_ok) {
            return SetResultBool(env, false);
        }
        status = napi_map_clear(
            env, map);
        if (status != napi_ok) {
            return SetResultBool(env, false);
        }
    }
    uint32_t size = VAL_999;
    status = napi_map_get_size(
        env, map, &size);
    if (status != napi_ok || size != 0) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 40：测试以 Symbol 作为 Key 存入 Map 的正常属性操作
static napi_value TestMapSymbolKeys(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value symKey = CreateJsSymbol(
        env, "myUniqueSymbol");
    napi_value val = CreateJsString(
        env, "symbolValue");
    status = napi_map_set_property(
        env, map, symKey, val);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    bool hasProp = false;
    status = napi_map_has_property(
        env, map, symKey, &hasProp);
    if (status != napi_ok || !hasProp) {
        return SetResultBool(env, false);
    }
    napi_value res = nullptr;
    status = napi_map_get_property(
        env, map, symKey, &res);
    if (status != napi_ok || res == nullptr) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 41：测试以 Object 作为 Key 存入 Map 的正常属性操作
static napi_value TestMapObjectKeys(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value objKey = CreateJsObject(
        env);
    napi_value val = CreateJsString(
        env, "objectValue");
    status = napi_map_set_property(
        env, map, objKey, val);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    bool hasProp = false;
    status = napi_map_has_property(
        env, map, objKey, &hasProp);
    if (status != napi_ok || !hasProp) {
        return SetResultBool(env, false);
    }
    napi_value res = nullptr;
    status = napi_map_get_property(
        env, map, objKey, &res);
    if (status != napi_ok || res == nullptr) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 42：测试向非 Map 的普通 JS 对象执行 Map 扩展操作时的类型安全保护
static napi_value TestMapTypeSafety(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value notMap = CreateJsObject(
        env);
    napi_value key = CreateJsString(
        env, "k");
    napi_value val = CreateJsInt32(
        env, VAL_123);
    napi_status status = napi_map_set_property(
        env, notMap, key, val);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    uint32_t size = 0;
    status = napi_map_get_size(
        env, notMap, &size);
    if (status == napi_ok) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 43：测试对同一属性执行重复删除操作时的返回状态与安全性
static napi_value TestMapDoubleDelete(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value key = CreateJsString(
        env, "delTwiceKey");
    napi_value val = CreateJsInt32(
        env, VAL_123);
    status = napi_map_set_property(
        env, map, key, val);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_delete_property(
        env, map, key);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_delete_property(
        env, map, key);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 44：测试对同一 key 重复写入相同值时 Map 大小及值的状态
static napi_value TestMapSetPropertySameValue(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value key = CreateJsString(
        env, "sameValKey");
    napi_value val = CreateJsInt32(
        env, VAL_123);
    status = napi_map_set_property(
        env, map, key, val);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_set_property(
        env, map, key, val);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    uint32_t size = 0;
    status = napi_map_get_size(
        env, map, &size);
    if (status != napi_ok || size != 1) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 45：测试以两个不同引用的对象作为 Key 存入 Map 时的碰撞与区分逻辑
static napi_value TestMapCreateObjectAsKeyCollision(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value obj1 = CreateJsObject(
        env);
    napi_value obj2 = CreateJsObject(
        env);
    napi_value val = CreateJsInt32(
        env, VAL_123);
    status = napi_map_set_property(
        env, map, obj1, val);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    status = napi_map_set_property(
        env, map, obj2, val);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    uint32_t size = 0;
    status = napi_map_get_size(
        env, map, &size);
    if (status != napi_ok || size != EXPECTED_SIZE_TWO) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 46：测试使用 undefined 和 null 作为 Map Key 时的不同映射与存储逻辑
static napi_value TestMapUndefinedAndNullKeys(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value nullVal = nullptr;
    status = napi_get_null(
        env, &nullVal);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value undefVal = nullptr;
    status = napi_get_undefined(
        env, &undefVal);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    napi_value dataVal = CreateJsInt32(
        env, VAL_123);
    status = napi_map_set_property(
        env, map, nullVal, dataVal);
    status = napi_map_set_property(
        env, map, undefVal, dataVal);
    uint32_t size = 0;
    status = napi_map_get_size(
        env, map, &size);
    if (status != napi_ok || size != EXPECTED_SIZE_TWO) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 用例 47：测试大容量 Map 在迭代器 entries 完整遍历中的计数一致性
static napi_value TestMapIteratorLargeCapacityLoop(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    for (int32_t i = 0; i < VAL_100; i++) {
        napi_value key = CreateJsInt32(
            env, i);
        napi_value val = CreateJsInt32(
            env, i);
        status = napi_map_set_property(
            env, map, key, val);
    }
    napi_value entries = nullptr;
    status = napi_map_get_entries(
        env, map, &entries);
    int32_t count = 0;
    for (size_t i = 0; i < ITER_LIMIT; i++) {
        napi_value item = nullptr;
        status = napi_map_iterator_get_next(
            env, entries, &item);
        napi_value done = nullptr;
        status = napi_get_named_property(
            env, item, "done", &done);
        bool isDone = false;
        napi_get_value_bool(
            env, done, &isDone);
        if (isDone) {
            break;
        }
        count++;
    }
    if (count != VAL_100) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

static bool PopulateMapWithInt32(napi_env env, napi_value map, int32_t limit)
{
    for (int32_t i = 0; i < limit; i++) {
        napi_value key = CreateJsInt32(
            env, i);
        napi_value val = CreateJsInt32(
            env, i);
        napi_status status = napi_map_set_property(
            env, map, key, val);
        if (status != napi_ok) {
            return false;
        }
    }
    return true;
}

static bool SumIteratorValues(napi_env env, napi_value keys, napi_value values, int32_t& keySum, int32_t& valSum)
{
    for (size_t i = 0; i < ITER_LIMIT; i++) {
        napi_value kItem = nullptr;
        napi_value vItem = nullptr;
        napi_map_iterator_get_next(
            env, keys, &kItem);
        napi_map_iterator_get_next(
            env, values, &vItem);
        napi_value kDone = nullptr;
        napi_value vDone = nullptr;
        napi_get_named_property(
            env, kItem, "done", &kDone);
        napi_get_named_property(
            env, vItem, "done", &vDone);
        bool kIsDone = false;
        bool vIsDone = false;
        napi_get_value_bool(
            env, kDone, &kIsDone);
        napi_get_value_bool(
            env, vDone, &vIsDone);
        if (kIsDone || vIsDone) {
            break;
        }
        napi_value kVal = nullptr;
        napi_value vVal = nullptr;
        napi_get_named_property(
            env, kItem, "value", &kVal);
        napi_get_named_property(
            env, vItem, "value", &vVal);
        int32_t kNum = 0;
        int32_t vNum = 0;
        napi_get_value_int32(
            env, kVal, &kNum);
        napi_get_value_int32(
            env, vVal, &vNum);
        keySum += kNum;
        valSum += vNum;
    }
    return true;
}

// 用例 48：测试 keys 与 values 迭代器所提取数据的数量与内容匹配性
static napi_value TestMapIteratorKeysValuesMatch(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value map = nullptr;
    napi_status status = napi_create_map(
        env, &map);
    if (status != napi_ok) {
        return SetResultBool(env, false);
    }
    if (!PopulateMapWithInt32(env, map, VAL_100)) {
        return SetResultBool(env, false);
    }
    napi_value keys = nullptr;
    napi_value values = nullptr;
    napi_map_get_keys(
        env, map, &keys);
    napi_map_get_values(
        env, map, &values);
    int32_t keySum = 0;
    int32_t valSum = 0;
    if (!SumIteratorValues(env, keys, values, keySum, valSum)) {
        return SetResultBool(env, false);
    }
    if (keySum != valSum) {
        return SetResultBool(env, false);
    }
    return SetResultBool(env, true);
}

// 匿名命名空间收尾部分：测试用例表声明（符合全局常量数组全大写规范）
namespace {
const napi_property_descriptor MAP_TEST_DESCRIPTORS[] = {
    DECLARE_NAPI_FUNCTION("testCreateMapNormal", TestCreateMapNormal),
    DECLARE_NAPI_FUNCTION("testCreateMapInvalid", TestCreateMapInvalid),
    DECLARE_NAPI_FUNCTION("testMapSetPropertyNormal", TestMapSetPropertyNormal),
    DECLARE_NAPI_FUNCTION("testMapSetPropertyInvalid", TestMapSetPropertyInvalid),
    DECLARE_NAPI_FUNCTION("testMapSetNamedPropertyNormal", TestMapSetNamedPropertyNormal),
    DECLARE_NAPI_FUNCTION("testMapSetNamedPropertyInvalid", TestMapSetNamedPropertyInvalid),
    DECLARE_NAPI_FUNCTION("testMapGetPropertyNormal", TestMapGetPropertyNormal),
    DECLARE_NAPI_FUNCTION("testMapGetPropertyInvalid", TestMapGetPropertyInvalid),
    DECLARE_NAPI_FUNCTION("testMapGetPropertyNonExist", TestMapGetPropertyNonExist),
    DECLARE_NAPI_FUNCTION("testMapGetNamedPropertyNormal", TestMapGetNamedPropertyNormal),
    DECLARE_NAPI_FUNCTION("testMapGetNamedPropertyInvalid", TestMapGetNamedPropertyInvalid),
    DECLARE_NAPI_FUNCTION("testMapGetNamedPropertyNonExist", TestMapGetNamedPropertyNonExist),
    DECLARE_NAPI_FUNCTION("testMapHasPropertyNormal", TestMapHasPropertyNormal),
    DECLARE_NAPI_FUNCTION("testMapHasPropertyInvalid", TestMapHasPropertyInvalid),
    DECLARE_NAPI_FUNCTION("testMapHasNamedPropertyNormal", TestMapHasNamedPropertyNormal),
    DECLARE_NAPI_FUNCTION("testMapHasNamedPropertyInvalid", TestMapHasNamedPropertyInvalid),
    DECLARE_NAPI_FUNCTION("testMapDeletePropertyNormal", TestMapDeletePropertyNormal),
    DECLARE_NAPI_FUNCTION("testMapDeletePropertyInvalid", TestMapDeletePropertyInvalid),
    DECLARE_NAPI_FUNCTION("testMapClearNormal", TestMapClearNormal),
    DECLARE_NAPI_FUNCTION("testMapClearInvalid", TestMapClearInvalid),
    DECLARE_NAPI_FUNCTION("testMapGetSizeInvalid", TestMapGetSizeInvalid),
    DECLARE_NAPI_FUNCTION("testMapGetEntriesNormal", TestMapGetEntriesNormal),
    DECLARE_NAPI_FUNCTION("testMapGetEntriesInvalid", TestMapGetEntriesInvalid),
    DECLARE_NAPI_FUNCTION("testMapGetKeysNormal", TestMapGetKeysNormal),
    DECLARE_NAPI_FUNCTION("testMapGetKeysInvalid", TestMapGetKeysInvalid),
    DECLARE_NAPI_FUNCTION("testMapGetValuesNormal", TestMapGetValuesNormal),
    DECLARE_NAPI_FUNCTION("testMapGetValuesInvalid", TestMapGetValuesInvalid),
    DECLARE_NAPI_FUNCTION("testMapIteratorGetNextInvalid", TestMapIteratorGetNextInvalid),
    DECLARE_NAPI_FUNCTION("testMapEmptyOperations", TestMapEmptyOperations),
    DECLARE_NAPI_FUNCTION("testMapOverwriteProperty", TestMapOverwriteProperty),
    DECLARE_NAPI_FUNCTION("testMapDeleteNonExist", TestMapDeleteNonExist),
    DECLARE_NAPI_FUNCTION("testMapMixedKeys", TestMapMixedKeys),
    DECLARE_NAPI_FUNCTION("testMapIteratorEntriesComplete", TestMapIteratorEntriesComplete),
    DECLARE_NAPI_FUNCTION("testMapIteratorKeysComplete", TestMapIteratorKeysComplete),
    DECLARE_NAPI_FUNCTION("testMapIteratorValuesComplete", TestMapIteratorValuesComplete),
    DECLARE_NAPI_FUNCTION("testMapLargeCapacity", TestMapLargeCapacity),
    DECLARE_NAPI_FUNCTION("testMapSetAndGetNested", TestMapSetAndGetNested),
    DECLARE_NAPI_FUNCTION("testMapIteratorMultipleNext", TestMapIteratorMultipleNext),
    DECLARE_NAPI_FUNCTION("testMapMemoryOrClearLoop", TestMapMemoryOrClearLoop),
    DECLARE_NAPI_FUNCTION("testMapSymbolKeys", TestMapSymbolKeys),
    DECLARE_NAPI_FUNCTION("testMapObjectKeys", TestMapObjectKeys),
    DECLARE_NAPI_FUNCTION("testMapTypeSafety", TestMapTypeSafety),
    DECLARE_NAPI_FUNCTION("testMapDoubleDelete", TestMapDoubleDelete),
    DECLARE_NAPI_FUNCTION("testMapSetPropertySameValue", TestMapSetPropertySameValue),
    DECLARE_NAPI_FUNCTION("testMapCreateObjectAsKeyCollision", TestMapCreateObjectAsKeyCollision),
    DECLARE_NAPI_FUNCTION("testMapUndefinedAndNullKeys", TestMapUndefinedAndNullKeys),
    DECLARE_NAPI_FUNCTION("testMapIteratorLargeCapacityLoop", TestMapIteratorLargeCapacityLoop),
    DECLARE_NAPI_FUNCTION("testMapIteratorKeysValuesMatch", TestMapIteratorKeysValuesMatch)
};
} // namespace

// 模块导出与注册函数
static napi_value Init(napi_env env, napi_value exports)
{
    constexpr size_t descSize = sizeof(MAP_TEST_DESCRIPTORS) / sizeof(MAP_TEST_DESCRIPTORS[0]);
    NAPI_CALL(env, napi_define_properties(
        env, exports, descSize, MAP_TEST_DESCRIPTORS));
    return exports;
}

static napi_module g_mapSuiteModule = {
    .nm_version = K_MODULE_VERSION,
    .nm_flags = K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "map_suite",
    .nm_priv = nullptr,
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterModule()
{
    napi_module_register(&g_mapSuiteModule);
}
