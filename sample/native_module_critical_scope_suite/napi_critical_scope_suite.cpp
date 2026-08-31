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
#include <thread>
#include <vector>
#include <atomic>
#include <string>

// 匿名命名空间，用于文件级常量及 UPPER_CASE 常量定义
namespace {
constexpr int LOOP_COUNT_100 = 100;
constexpr int LOOP_COUNT_1000 = 1000;
constexpr int LOOP_COUNT_10000 = 10000;
constexpr size_t STRING_LENGTH_5 = 5;
constexpr size_t STRING_LENGTH_10 = 10;
constexpr size_t STRING_LENGTH_20 = 20;
constexpr size_t STRING_LENGTH_100 = 100;
constexpr size_t BUFFER_SIZE_256 = 256;
constexpr size_t BUFFER_SIZE_512 = 512;
constexpr size_t THREAD_NUM_4 = 4;
constexpr size_t BYTE_LENGTH_16 = 16;
constexpr double DOUBLE_NUM_VAL = 3.14159;
constexpr int TEST_NUM_123 = 123;
constexpr int NESTED_LEVEL_5 = 5;
}

// 辅助函数：校验两个 UTF-16 字符串是否相等
static bool CheckStringEqual(const char16_t* buffer, const char16_t* expected, size_t length)
{
    if (buffer == nullptr || expected == nullptr) {
        return false;
    }
    for (size_t i = 0; i < length; ++i) {
        if (buffer[i] != expected[i]) {
            return false;
        }
    }
    return true;
}

// 辅助函数：创建测试结果的 JS 对象
static napi_value CreateTestResult(napi_env env, bool success, const char* msg)
{
    napi_value resultObj = nullptr;
    napi_create_object(env, &resultObj);
    napi_value successVal = nullptr;
    napi_get_boolean(env, success, &successVal);
    napi_set_named_property(env, resultObj, "success", successVal);
    napi_value msgVal = nullptr;
    napi_create_string_utf8(env, msg, NAPI_AUTO_LENGTH, &msgVal);
    napi_set_named_property(env, resultObj, "message", msgVal);
    return resultObj;
}

// 辅助函数：向导出对象中添加测试函数
static void AddFunction(napi_env env, napi_value exports, const char* name, napi_callback cb)
{
    napi_property_descriptor desc = DECLARE_NAPI_FUNCTION(name, cb);
    napi_define_properties(env, exports, 1, &desc);
}

// 正常打开和关闭临界作用域
static napi_value TestOpenCriticalScopeNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_critical_scope scope = nullptr;
    napi_status status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok || scope == nullptr) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    status = napi_close_critical_scope(env, scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "关闭临界作用域失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 传入空 env 开启临界作用域
static napi_value TestOpenCriticalScopeNullEnv(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_critical_scope scope = nullptr;
    napi_status status = napi_open_critical_scope(nullptr, &scope);
    if (status == napi_ok) {
        return CreateTestResult(env, false, "传入空env应返回失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 传入空 scope 指针开启临界作用域
static napi_value TestOpenCriticalScopeNullScope(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_status status = napi_open_critical_scope(env, nullptr);
    if (status == napi_ok) {
        return CreateTestResult(env, false, "传入空scope指针应返回失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 正常关闭临界作用域逻辑（与 TestOpenCriticalScopeNormal 等价，侧重关闭）
static napi_value TestCloseCriticalScopeNormal(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_critical_scope scope = nullptr;
    napi_status status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    status = napi_close_critical_scope(env, scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "关闭临界作用域失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 传入空 env 关闭临界作用域
static napi_value TestCloseCriticalScopeNullEnv(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_critical_scope scope = nullptr;
    napi_status status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    status = napi_close_critical_scope(nullptr, scope);
    if (status == napi_ok) {
        napi_close_critical_scope(env, scope);
        return CreateTestResult(env, false, "传入空env关闭应返回失败");
    }
    napi_close_critical_scope(env, scope);
    return CreateTestResult(env, true, "测试成功");
}

// 传入空 scope 关闭临界作用域
static napi_value TestCloseCriticalScopeNullScope(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_status status = napi_close_critical_scope(env, nullptr);
    if (status == napi_ok) {
        return CreateTestResult(env, false, "传入空scope关闭应返回失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 未开启临界作用域时，直接进行关闭
static napi_value TestCloseCriticalScopeWithoutOpen(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_critical_scope scope = nullptr;
    napi_status status = napi_close_critical_scope(env, scope);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "未开启时关闭应返回invalid_arg");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 开启后，对同一个临界作用域关闭两次
static napi_value TestCloseCriticalScopeDoubleClose(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_critical_scope scope = nullptr;
    napi_status status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok || scope == nullptr) {
        return CreateTestResult(env, false, "开启作用域失败");
    }
    status = napi_close_critical_scope(env, scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "第一次关闭失败");
    }
    status = napi_close_critical_scope(env, scope);
    if (status != napi_invalid_arg) {
        return CreateTestResult(env, false, "重复关闭未返回invalid_arg");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 临界作用域内获取普通的 UTF-16 字符串并校验
static napi_value TestGetBufferStringNormalUtf16(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsStr = nullptr;
    const char16_t* rawStr = u"HelloWorld";
    napi_status status = napi_create_string_utf16(env, rawStr, NAPI_AUTO_LENGTH, &jsStr);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建UTF16字符串失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, jsStr, &buffer, &length);
    if (status != napi_ok || buffer == nullptr || length == 0) {
        napi_close_critical_scope(env, scope);
        return CreateTestResult(env, false, "获取字符串buffer失败");
    }
    bool isEqual = CheckStringEqual(buffer, rawStr, length);
    napi_close_critical_scope(env, scope);
    if (!isEqual) {
        return CreateTestResult(env, false, "获取的字符串内容不匹配");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 获取字符串时传入空 env
static napi_value TestGetBufferStringUtf16NullEnv(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsStr = nullptr;
    const char16_t* rawStr = u"Test";
    napi_status status = napi_create_string_utf16(env, rawStr, NAPI_AUTO_LENGTH, &jsStr);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建字符串失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(nullptr, jsStr, &buffer, &length);
    napi_close_critical_scope(env, scope);
    if (status == napi_ok) {
        return CreateTestResult(env, false, "传入空env应返回失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 获取字符串时传入空 value
static napi_value TestGetBufferStringUtf16NullValue(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_critical_scope scope = nullptr;
    napi_status status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, nullptr, &buffer, &length);
    napi_close_critical_scope(env, scope);
    if (status == napi_ok) {
        return CreateTestResult(env, false, "传入空value应返回失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 获取字符串时传入空 buffer 指针
static napi_value TestGetBufferStringUtf16NullBuffer(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsStr = nullptr;
    const char16_t* rawStr = u"Test";
    napi_status status = napi_create_string_utf16(env, rawStr, NAPI_AUTO_LENGTH, &jsStr);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建字符串失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, jsStr, nullptr, &length);
    napi_close_critical_scope(env, scope);
    if (status == napi_ok) {
        return CreateTestResult(env, false, "传入空buffer应返回失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 获取字符串时传入空 length 指针
static napi_value TestGetBufferStringUtf16NullLength(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsStr = nullptr;
    const char16_t* rawStr = u"Test";
    napi_status status = napi_create_string_utf16(env, rawStr, NAPI_AUTO_LENGTH, &jsStr);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建字符串失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, jsStr, &buffer, nullptr);
    napi_close_critical_scope(env, scope);
    if (status == napi_ok) {
        return CreateTestResult(env, false, "传入空length应返回失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 不在临界作用域内部时获取 UTF-16 字符串
static napi_value TestGetBufferStringUtf16OutsideScope(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsStr = nullptr;
    const char16_t* rawStr = u"Test";
    napi_status status = napi_create_string_utf16(env, rawStr, NAPI_AUTO_LENGTH, &jsStr);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建字符串失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, jsStr, &buffer, &length);
    if (status != napi_generic_failure) {
        return CreateTestResult(env, false, "不在临界作用域内应返回generic_failure");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 对非字符串类型获取 UTF-16 字符串
static napi_value TestGetBufferStringUtf16NonStringValue(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsNum = nullptr;
    constexpr int testNum = TEST_NUM_123;
    napi_status status = napi_create_int32(env, testNum, &jsNum);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建数字失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, jsNum, &buffer, &length);
    napi_close_critical_scope(env, scope);
    if (status != napi_string_expected) {
        return CreateTestResult(env, false, "对非字符串获取应返回string_expected");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 获取空字符串的 UTF-16 缓冲区
static napi_value TestGetBufferStringUtf16Empty(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsStr = nullptr;
    const char16_t* rawStr = u"";
    napi_status status = napi_create_string_utf16(env, rawStr, NAPI_AUTO_LENGTH, &jsStr);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建字符串失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, jsStr, &buffer, &length);
    napi_close_critical_scope(env, scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "获取空字符串失败");
    }
    if (length != 0) {
        return CreateTestResult(env, false, "获取的字符串长度应为0");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 获取长字符串的 UTF-16 缓冲区并校验
static napi_value TestGetBufferStringUtf16Long(napi_env env, napi_callback_info info)
{
    (void)info;
    constexpr size_t longLength = LOOP_COUNT_1000;
    std::u16string longStr(longLength, u'A');
    napi_value jsStr = nullptr;
    napi_status status = napi_create_string_utf16(env, longStr.c_str(), longLength, &jsStr);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建长字符串失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, jsStr, &buffer, &length);
    if (status != napi_ok || length != longLength || buffer == nullptr) {
        napi_close_critical_scope(env, scope);
        return CreateTestResult(env, false, "获取长字符串数据失败");
    }
    bool isEqual = CheckStringEqual(buffer, longStr.c_str(), length);
    napi_close_critical_scope(env, scope);
    if (!isEqual) {
        return CreateTestResult(env, false, "长字符串内容不匹配");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 获取包含 Emoji 等特殊代理对字符的 UTF-16 缓冲区并校验
static napi_value TestGetBufferStringUtf16SpecialChars(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsStr = nullptr;
    const char16_t* rawStr = u"Emoji\U0001F600Test";
    napi_status status = napi_create_string_utf16(env, rawStr, NAPI_AUTO_LENGTH, &jsStr);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建特殊字符串失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, jsStr, &buffer, &length);
    if (status != napi_ok || buffer == nullptr || length == 0) {
        napi_close_critical_scope(env, scope);
        return CreateTestResult(env, false, "在临界作用域内获取字符串buffer失败");
    }
    bool isEqual = CheckStringEqual(buffer, rawStr, length);
    napi_close_critical_scope(env, scope);
    if (!isEqual) {
        return CreateTestResult(env, false, "获取的特殊字符串内容不匹配");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 嵌套使用两层临界作用域并以正常 LIFO 顺序关闭
static napi_value TestNestedCriticalScopeTwoLevels(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_critical_scope scopeOuter = nullptr;
    napi_status status = napi_open_critical_scope(env, &scopeOuter);
    if (status != napi_ok || scopeOuter == nullptr) {
        return CreateTestResult(env, false, "开启外层临界作用域失败");
    }
    napi_critical_scope scopeInner = nullptr;
    status = napi_open_critical_scope(env, &scopeInner);
    if (status != napi_ok || scopeInner == nullptr) {
        napi_close_critical_scope(env, scopeOuter);
        return CreateTestResult(env, false, "开启内层临界作用域失败");
    }
    status = napi_close_critical_scope(env, scopeInner);
    if (status != napi_ok) {
        napi_close_critical_scope(env, scopeOuter);
        return CreateTestResult(env, false, "关闭内层临界作用域失败");
    }
    status = napi_close_critical_scope(env, scopeOuter);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "关闭外层临界作用域失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 深层嵌套临界作用域（4层）并以正常 LIFO 顺序关闭
static napi_value TestNestedCriticalScopeDeepLevels(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_critical_scope scope1 = nullptr;
    napi_critical_scope scope2 = nullptr;
    napi_critical_scope scope3 = nullptr;
    napi_critical_scope scope4 = nullptr;
    napi_status s1 = napi_open_critical_scope(env, &scope1);
    napi_status s2 = napi_open_critical_scope(env, &scope2);
    napi_status s3 = napi_open_critical_scope(env, &scope3);
    napi_status s4 = napi_open_critical_scope(env, &scope4);
    if (s1 != napi_ok || s2 != napi_ok || s3 != napi_ok || s4 != napi_ok) {
        if (s1 == napi_ok) napi_close_critical_scope(env, scope1);
        if (s2 == napi_ok) napi_close_critical_scope(env, scope2);
        if (s3 == napi_ok) napi_close_critical_scope(env, scope3);
        return CreateTestResult(env, false, "深层嵌套开启作用域失败");
    }
    napi_status c4 = napi_close_critical_scope(env, scope4);
    napi_status c3 = napi_close_critical_scope(env, scope3);
    napi_status c2 = napi_close_critical_scope(env, scope2);
    napi_status c1 = napi_close_critical_scope(env, scope1);
    if (c4 != napi_ok || c3 != napi_ok || c2 != napi_ok || c1 != napi_ok) {
        return CreateTestResult(env, false, "深层嵌套关闭作用域失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 临界作用域内高频循环读取 UTF-16 字符串
static napi_value TestHighFrequencyReadUtf16(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsStr = nullptr;
    const char16_t* rawStr = u"HighFrequencyTest";
    napi_status status = napi_create_string_utf16(env, rawStr, NAPI_AUTO_LENGTH, &jsStr);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建字符串失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    constexpr int loopCount = LOOP_COUNT_10000;
    bool success = true;
    for (int i = 0; i < loopCount; ++i) {
        const char16_t* buffer = nullptr;
        size_t length = 0;
        status = napi_get_buffer_string_utf16_in_critical_scope(env, jsStr, &buffer, &length);
        if (status != napi_ok || buffer == nullptr || length == 0) {
            success = false;
            break;
        }
    }
    napi_close_critical_scope(env, scope);
    if (!success) {
        return CreateTestResult(env, false, "高频读取字符串失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 验证多线程环境下并发读取临界区内导出的 UTF-16 字符串缓冲区
static napi_value TestMultiThreadAccessBuffer(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsStr = nullptr;
    const char16_t* rawStr = u"MultiThreadString";
    napi_status status = napi_create_string_utf16(env, rawStr, NAPI_AUTO_LENGTH, &jsStr);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建字符串失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, jsStr, &buffer, &length);
    if (status != napi_ok || buffer == nullptr || length == 0) {
        napi_close_critical_scope(env, scope);
        return CreateTestResult(env, false, "获取字符串失败");
    }
    constexpr size_t threadNum = THREAD_NUM_4;
    std::vector<std::thread> threads;
    std::atomic<bool> allMatch{true};
    for (size_t i = 0; i < threadNum; ++i) {
        threads.emplace_back([buffer, length, rawStr, &allMatch]() {
            bool localMatch = CheckStringEqual(buffer, rawStr, length);
            if (!localMatch) {
                allMatch = false;
            }
        });
    }
    for (auto& th : threads) {
        th.join();
    }
    napi_close_critical_scope(env, scope);
    if (!allMatch) {
        return CreateTestResult(env, false, "多线程读取的数据与期望不符");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 对 ArrayBuffer 类型调用临界区字符串获取接口
static napi_value TestGetBufferStringUtf16ArrayBuffer(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value arrayBuffer = nullptr;
    void* data = nullptr;
    constexpr size_t byteLength = BYTE_LENGTH_16;
    napi_status status = napi_create_arraybuffer(env, byteLength, &data, &arrayBuffer);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建ArrayBuffer失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, arrayBuffer, &buffer, &length);
    napi_close_critical_scope(env, scope);
    if (status != napi_string_expected) {
        return CreateTestResult(env, false, "对ArrayBuffer获取应返回string_expected");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 对 Number 类型调用临界区字符串获取接口
static napi_value TestGetBufferStringUtf16Number(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsValue = nullptr;
    constexpr double numVal = DOUBLE_NUM_VAL;
    napi_status status = napi_create_double(env, numVal, &jsValue);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建Number失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, jsValue, &buffer, &length);
    napi_close_critical_scope(env, scope);
    if (status != napi_string_expected) {
        return CreateTestResult(env, false, "对Number获取应返回string_expected");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 对 Boolean 类型调用临界区字符串获取接口
static napi_value TestGetBufferStringUtf16Boolean(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsValue = nullptr;
    napi_status status = napi_get_boolean(env, true, &jsValue);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "获取Boolean失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, jsValue, &buffer, &length);
    napi_close_critical_scope(env, scope);
    if (status != napi_string_expected) {
        return CreateTestResult(env, false, "对Boolean获取应返回string_expected");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 对 Object 类型调用临界区字符串获取接口
static napi_value TestGetBufferStringUtf16Object(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsValue = nullptr;
    napi_status status = napi_create_object(env, &jsValue);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建Object失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, jsValue, &buffer, &length);
    napi_close_critical_scope(env, scope);
    if (status != napi_string_expected) {
        return CreateTestResult(env, false, "对Object获取应返回string_expected");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 对 Null 类型调用临界区字符串获取接口
static napi_value TestGetBufferStringUtf16Null(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsValue = nullptr;
    napi_status status = napi_get_null(env, &jsValue);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "获取Null失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, jsValue, &buffer, &length);
    napi_close_critical_scope(env, scope);
    if (status != napi_string_expected) {
        return CreateTestResult(env, false, "对Null获取应返回string_expected");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 对 Undefined 类型调用临界区字符串获取接口
static napi_value TestGetBufferStringUtf16Undefined(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsValue = nullptr;
    napi_status status = napi_get_undefined(env, &jsValue);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "获取Undefined失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, jsValue, &buffer, &length);
    napi_close_critical_scope(env, scope);
    if (status != napi_string_expected) {
        return CreateTestResult(env, false, "对Undefined获取应返回string_expected");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 对 Symbol 类型调用临界区字符串获取接口
static napi_value TestGetBufferStringUtf16Symbol(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value symbolDesc = nullptr;
    napi_status status = napi_create_string_utf8(env, "desc", NAPI_AUTO_LENGTH, &symbolDesc);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建字符串说明失败");
    }
    napi_value jsValue = nullptr;
    status = napi_create_symbol(env, symbolDesc, &jsValue);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建Symbol失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, jsValue, &buffer, &length);
    napi_close_critical_scope(env, scope);
    if (status != napi_string_expected) {
        return CreateTestResult(env, false, "对Symbol获取应返回string_expected");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 对 Function 类型调用临界区字符串获取接口
static napi_value TestGetBufferStringUtf16Function(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsValue = nullptr;
    napi_status status = napi_create_function(env, "func", NAPI_AUTO_LENGTH,
        TestOpenCriticalScopeNormal, nullptr, &jsValue);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建Function失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, jsValue, &buffer, &length);
    napi_close_critical_scope(env, scope);
    if (status != napi_string_expected) {
        return CreateTestResult(env, false, "对Function获取应返回string_expected");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 对 External 类型调用临界区字符串获取接口
static napi_value TestGetBufferStringUtf16External(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsValue = nullptr;
    int dataVal = TEST_NUM_123;
    napi_status status = napi_create_external(env, &dataVal, nullptr, nullptr, &jsValue);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建External失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, jsValue, &buffer, &length);
    napi_close_critical_scope(env, scope);
    if (status != napi_string_expected) {
        return CreateTestResult(env, false, "对External获取应返回string_expected");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 对 BigInt 类型调用临界区字符串获取接口
static napi_value TestGetBufferStringUtf16BigInt(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsValue = nullptr;
    constexpr int intVal = TEST_NUM_123;
    napi_status status = napi_create_bigint_int64(env, intVal, &jsValue);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建BigInt失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, jsValue, &buffer, &length);
    napi_close_critical_scope(env, scope);
    if (status != napi_string_expected) {
        return CreateTestResult(env, false, "对BigInt获取应返回string_expected");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 对 TypedArray 类型调用临界区字符串获取接口
static napi_value TestGetBufferStringUtf16TypedArray(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value arrayBuffer = nullptr;
    void* data = nullptr;
    constexpr size_t byteLength = BYTE_LENGTH_16;
    napi_status status = napi_create_arraybuffer(env, byteLength, &data, &arrayBuffer);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建ArrayBuffer失败");
    }
    napi_value typedArray = nullptr;
    status = napi_create_typedarray(env, napi_uint8_array, byteLength, arrayBuffer, 0, &typedArray);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建TypedArray失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, typedArray, &buffer, &length);
    napi_close_critical_scope(env, scope);
    if (status != napi_string_expected) {
        return CreateTestResult(env, false, "对TypedArray获取应返回string_expected");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 对 DataView 类型调用临界区字符串获取接口
static napi_value TestGetBufferStringUtf16DataView(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value arrayBuffer = nullptr;
    void* data = nullptr;
    constexpr size_t byteLength = BYTE_LENGTH_16;
    napi_status status = napi_create_arraybuffer(env, byteLength, &data, &arrayBuffer);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建ArrayBuffer失败");
    }
    napi_value dataView = nullptr;
    status = napi_create_dataview(env, byteLength, arrayBuffer, 0, &dataView);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建DataView失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, dataView, &buffer, &length);
    napi_close_critical_scope(env, scope);
    if (status != napi_string_expected) {
        return CreateTestResult(env, false, "对DataView获取应返回string_expected");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 连续多次打开不同的临界作用域，按正常 LIFO 顺序逐一关闭
static napi_value TestOpenCriticalScopeMultipleTimes(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_critical_scope scope1 = nullptr;
    napi_critical_scope scope2 = nullptr;
    napi_critical_scope scope3 = nullptr;
    napi_critical_scope scope4 = nullptr;
    napi_critical_scope scope5 = nullptr;
    napi_status s1 = napi_open_critical_scope(env, &scope1);
    napi_status s2 = napi_open_critical_scope(env, &scope2);
    napi_status s3 = napi_open_critical_scope(env, &scope3);
    napi_status s4 = napi_open_critical_scope(env, &scope4);
    napi_status s5 = napi_open_critical_scope(env, &scope5);
    if (s1 != napi_ok || s2 != napi_ok || s3 != napi_ok || s4 != napi_ok || s5 != napi_ok) {
        if (s1 == napi_ok) napi_close_critical_scope(env, scope1);
        if (s2 == napi_ok) napi_close_critical_scope(env, scope2);
        if (s3 == napi_ok) napi_close_critical_scope(env, scope3);
        if (s4 == napi_ok) napi_close_critical_scope(env, scope4);
        return CreateTestResult(env, false, "开启多个临界作用域失败");
    }
    napi_status c5 = napi_close_critical_scope(env, scope5);
    napi_status c4 = napi_close_critical_scope(env, scope4);
    napi_status c3 = napi_close_critical_scope(env, scope3);
    napi_status c2 = napi_close_critical_scope(env, scope2);
    napi_status c1 = napi_close_critical_scope(env, scope1);
    if (c5 != napi_ok || c4 != napi_ok || c3 != napi_ok || c2 != napi_ok || c1 != napi_ok) {
        return CreateTestResult(env, false, "依次关闭多个临界作用域失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 在高频循环中，每次迭代都进行临界作用域的开启、嵌套与正常关闭
static napi_value TestHighFrequencyNestedCriticalScope(napi_env env, napi_callback_info info)
{
    (void)info;
    constexpr int loopCount = LOOP_COUNT_1000;
    bool success = true;
    for (int i = 0; i < loopCount; ++i) {
        napi_critical_scope outerScope = nullptr;
        napi_critical_scope innerScope = nullptr;
        napi_status sOuter = napi_open_critical_scope(env, &outerScope);
        napi_status sInner = napi_open_critical_scope(env, &innerScope);
        if (sOuter != napi_ok || sInner != napi_ok) {
            if (sOuter == napi_ok) napi_close_critical_scope(env, outerScope);
            success = false;
            break;
        }
        napi_status cInner = napi_close_critical_scope(env, innerScope);
        napi_status cOuter = napi_close_critical_scope(env, outerScope);
        if (cInner != napi_ok || cOuter != napi_ok) {
            success = false;
            break;
        }
    }
    if (!success) {
        return CreateTestResult(env, false, "高频迭代嵌套开启与关闭作用域失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 在高频循环中，获取并校验各种字符串 UTF-16 缓冲区
static napi_value TestHighFrequencyGetBufferString(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsStr = nullptr;
    const char16_t* rawStr = u"LoopStr";
    napi_status status = napi_create_string_utf16(env, rawStr, NAPI_AUTO_LENGTH, &jsStr);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建字符串失败");
    }
    constexpr int loopCount = LOOP_COUNT_1000;
    bool success = true;
    for (int i = 0; i < loopCount; ++i) {
        napi_critical_scope scope = nullptr;
        status = napi_open_critical_scope(env, &scope);
        if (status != napi_ok) {
            success = false;
            break;
        }
        const char16_t* buffer = nullptr;
        size_t length = 0;
        status = napi_get_buffer_string_utf16_in_critical_scope(env, jsStr, &buffer, &length);
        if (status != napi_ok || buffer == nullptr || !CheckStringEqual(buffer, rawStr, length)) {
            napi_close_critical_scope(env, scope);
            success = false;
            break;
        }
        status = napi_close_critical_scope(env, scope);
        if (status != napi_ok) {
            success = false;
            break;
        }
    }
    if (!success) {
        return CreateTestResult(env, false, "高频生命周期下获取字符串数据失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 模拟异常退出分支：在未关闭临界作用域时提前返回
static napi_value TestUnclosedCriticalScopeLeak(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_critical_scope scope = nullptr;
    napi_status status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok || scope == nullptr) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    // 故意在未关闭 scope 时提前返回，用于验证引擎的异常退出机制与资源校验。
    // 该函数的调用必须被单独处理以防单元测试环境产生异常崩溃。
    return CreateTestResult(env, true, "测试成功");
}

// 校验包含零宽及控制字符等特殊 Unicode 的 UTF-16 缓冲区
static napi_value TestGetBufferStringUtf16WithSpecialUnicode(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsStr = nullptr;
    // 包含零宽空格 (u"\u200B") 和换行控制符的特殊 Unicode 字符串
    const char16_t* rawStr = u"Zero\u200BWidth\nControl";
    napi_status status = napi_create_string_utf16(env, rawStr, NAPI_AUTO_LENGTH, &jsStr);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "创建特殊Unicode字符串失败");
    }
    napi_critical_scope scope = nullptr;
    status = napi_open_critical_scope(env, &scope);
    if (status != napi_ok) {
        return CreateTestResult(env, false, "开启临界作用域失败");
    }
    const char16_t* buffer = nullptr;
    size_t length = 0;
    status = napi_get_buffer_string_utf16_in_critical_scope(env, jsStr, &buffer, &length);
    if (status != napi_ok || buffer == nullptr || length == 0) {
        napi_close_critical_scope(env, scope);
        return CreateTestResult(env, false, "在临界作用域内获取字符串数据失败");
    }
    bool isEqual = CheckStringEqual(buffer, rawStr, length);
    napi_close_critical_scope(env, scope);
    if (!isEqual) {
        return CreateTestResult(env, false, "获取的特殊Unicode字符串内容不匹配");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 在多层嵌套的临界作用域中，多次并列获取不同的 UTF-16 字符串
static napi_value TestNestedCriticalScopeMultipleGet(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value jsStr1 = nullptr;
    napi_value jsStr2 = nullptr;
    const char16_t* rawStr1 = u"NestedOne";
    const char16_t* rawStr2 = u"NestedTwo";
    napi_create_string_utf16(env, rawStr1, NAPI_AUTO_LENGTH, &jsStr1);
    napi_create_string_utf16(env, rawStr2, NAPI_AUTO_LENGTH, &jsStr2);
    napi_critical_scope scopeOuter = nullptr;
    napi_critical_scope scopeInner = nullptr;
    napi_open_critical_scope(env, &scopeOuter);
    napi_open_critical_scope(env, &scopeInner);
    const char16_t* buffer1 = nullptr;
    size_t length1 = 0;
    napi_status s1 = napi_get_buffer_string_utf16_in_critical_scope(env, jsStr1, &buffer1, &length1);
    const char16_t* buffer2 = nullptr;
    size_t length2 = 0;
    napi_status s2 = napi_get_buffer_string_utf16_in_critical_scope(env, jsStr2, &buffer2, &length2);
    bool checkOk = (s1 == napi_ok && s2 == napi_ok && buffer1 && buffer2 &&
        CheckStringEqual(buffer1, rawStr1, length1) && CheckStringEqual(buffer2, rawStr2, length2));
    napi_close_critical_scope(env, scopeInner);
    napi_close_critical_scope(env, scopeOuter);
    if (!checkOk) {
        return CreateTestResult(env, false, "嵌套临界作用域内获取多个字符串失败");
    }
    return CreateTestResult(env, true, "测试成功");
}

// 测试函数注册辅助函数：Part 1
static void RegisterSuitePart1(napi_env env, napi_value exports)
{
    AddFunction(env, exports, "testOpenCriticalScopeNormal", TestOpenCriticalScopeNormal);
    AddFunction(env, exports, "testOpenCriticalScopeNullEnv", TestOpenCriticalScopeNullEnv);
    AddFunction(env, exports, "testOpenCriticalScopeNullScope", TestOpenCriticalScopeNullScope);
    AddFunction(env, exports, "testCloseCriticalScopeNormal", TestCloseCriticalScopeNormal);
    AddFunction(env, exports, "testCloseCriticalScopeNullEnv", TestCloseCriticalScopeNullEnv);
    AddFunction(env, exports, "testCloseCriticalScopeNullScope", TestCloseCriticalScopeNullScope);
    AddFunction(env, exports, "testCloseCriticalScopeWithoutOpen", TestCloseCriticalScopeWithoutOpen);
    AddFunction(env, exports, "testCloseCriticalScopeDoubleClose", TestCloseCriticalScopeDoubleClose);
    AddFunction(env, exports, "testGetBufferStringNormalUtf16", TestGetBufferStringNormalUtf16);
    AddFunction(env, exports, "testGetBufferStringUtf16NullEnv", TestGetBufferStringUtf16NullEnv);
    AddFunction(env, exports, "testGetBufferStringUtf16NullValue", TestGetBufferStringUtf16NullValue);
    AddFunction(env, exports, "testGetBufferStringUtf16NullBuffer", TestGetBufferStringUtf16NullBuffer);
    AddFunction(env, exports, "testGetBufferStringUtf16NullLength", TestGetBufferStringUtf16NullLength);
    AddFunction(env, exports, "testGetBufferStringUtf16OutsideScope", TestGetBufferStringUtf16OutsideScope);
    AddFunction(env, exports, "testGetBufferStringUtf16NonStringValue", TestGetBufferStringUtf16NonStringValue);
    AddFunction(env, exports, "testGetBufferStringUtf16Empty", TestGetBufferStringUtf16Empty);
    AddFunction(env, exports, "testGetBufferStringUtf16Long", TestGetBufferStringUtf16Long);
    AddFunction(env, exports, "testGetBufferStringUtf16SpecialChars", TestGetBufferStringUtf16SpecialChars);
    AddFunction(env, exports, "testNestedCriticalScopeTwoLevels", TestNestedCriticalScopeTwoLevels);
    AddFunction(env, exports, "testNestedCriticalScopeDeepLevels", TestNestedCriticalScopeDeepLevels);
}

// 测试函数注册辅助函数：Part 2
static void RegisterSuitePart2(napi_env env, napi_value exports)
{
    AddFunction(env, exports, "testHighFrequencyReadUtf16", TestHighFrequencyReadUtf16);
    AddFunction(env, exports, "testMultiThreadAccessBuffer", TestMultiThreadAccessBuffer);
    AddFunction(env, exports, "testGetBufferStringUtf16ArrayBuffer", TestGetBufferStringUtf16ArrayBuffer);
    AddFunction(env, exports, "testGetBufferStringUtf16Number", TestGetBufferStringUtf16Number);
    AddFunction(env, exports, "testGetBufferStringUtf16Boolean", TestGetBufferStringUtf16Boolean);
    AddFunction(env, exports, "testGetBufferStringUtf16Object", TestGetBufferStringUtf16Object);
    AddFunction(env, exports, "testGetBufferStringUtf16Null", TestGetBufferStringUtf16Null);
    AddFunction(env, exports, "testGetBufferStringUtf16Undefined", TestGetBufferStringUtf16Undefined);
    AddFunction(env, exports, "testGetBufferStringUtf16Symbol", TestGetBufferStringUtf16Symbol);
    AddFunction(env, exports, "testGetBufferStringUtf16Function", TestGetBufferStringUtf16Function);
    AddFunction(env, exports, "testGetBufferStringUtf16External", TestGetBufferStringUtf16External);
    AddFunction(env, exports, "testGetBufferStringUtf16BigInt", TestGetBufferStringUtf16BigInt);
    AddFunction(env, exports, "testGetBufferStringUtf16TypedArray", TestGetBufferStringUtf16TypedArray);
    AddFunction(env, exports, "testGetBufferStringUtf16DataView", TestGetBufferStringUtf16DataView);
    AddFunction(env, exports, "testOpenCriticalScopeMultipleTimes", TestOpenCriticalScopeMultipleTimes);
    AddFunction(env, exports, "testHighFrequencyNestedCriticalScope", TestHighFrequencyNestedCriticalScope);
    AddFunction(env, exports, "testHighFrequencyGetBufferString", TestHighFrequencyGetBufferString);
    AddFunction(env, exports, "testUnclosedCriticalScopeLeak", TestUnclosedCriticalScopeLeak);
    AddFunction(env, exports, "testGetBufferStringUtf16WithSpecialUnicode", TestGetBufferStringUtf16WithSpecialUnicode);
    AddFunction(env, exports, "testNestedCriticalScopeMultipleGet", TestNestedCriticalScopeMultipleGet);
}

// 模块初始化注册入口
static napi_value Init(napi_env env, napi_value exports)
{
    RegisterSuitePart1(env, exports);
    RegisterSuitePart2(env, exports);
    return exports;
}

static napi_module g_criticalScopeModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "critical_scope_suite",
    .nm_priv = nullptr,
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterModule()
{
    napi_module_register(&g_criticalScopeModule);
}
