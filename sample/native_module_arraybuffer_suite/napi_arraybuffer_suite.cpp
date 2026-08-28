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
#include <cstring>
#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

// 匿名命名空间，用于存放文件级常量及避免魔鬼数字
namespace {
static constexpr size_t INDEX_ZERO = 0;
static constexpr size_t INDEX_ONE = 1;
static constexpr size_t INDEX_TWO = 2;
static constexpr size_t INDEX_THREE = 3;
static constexpr size_t INDEX_FOUR = 4;
static constexpr size_t INDEX_FIVE = 5;
static constexpr size_t INDEX_SIX = 6;
static constexpr size_t INDEX_SEVEN = 7;
static constexpr size_t INDEX_EIGHT = 8;
static constexpr size_t INDEX_NINE = 9;
static constexpr size_t INDEX_TEN = 10;

static constexpr size_t TEST_SIZE_ZERO = 0;
static constexpr size_t TEST_SIZE_ONE = 1;
static constexpr size_t TEST_SIZE_TWO = 2;
static constexpr size_t TEST_SIZE_TEN = 10;
static constexpr size_t TEST_SIZE_HUNDRED = 100;
static constexpr size_t TEST_SIZE_LARGE = 1024;
static constexpr size_t TEST_SIZE_TOO_LARGE = 2147483647;
static constexpr size_t MAX_SIZE_T = static_cast<size_t>(-1);

static constexpr size_t TEST_SIZE_16 = 16;
static constexpr size_t TEST_SIZE_32 = 32;
static constexpr size_t TEST_SIZE_64 = 64;
static constexpr size_t TEST_SIZE_128 = 128;
static constexpr size_t TEST_SIZE_256 = 256;
static constexpr size_t TEST_SIZE_512 = 512;
static constexpr size_t TEST_SIZE_1024 = 1024;

static constexpr uint8_t TEST_VAL_A = 0xAA;
static constexpr uint8_t TEST_VAL_B = 0x55;
static constexpr uint8_t TEST_VAL_C = 0xFF;
static constexpr uint8_t TEST_VAL_D = 0x00;

static constexpr int32_t RET_ERR_CODE = -1;
static constexpr int32_t RET_OK_CODE = 0;

static constexpr uint32_t MODULE_VERSION = 1;
static constexpr uint32_t NO_MODULE_FLAGS = 0;

// Helper 辅助函数：向对象添加 Boolean 属性
static bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value)
{
    napi_value napiValue = nullptr;
    if (napi_get_boolean(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

// Helper 辅助函数：向对象添加 Int32 属性
static bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int32(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

// Helper 辅助函数：向对象添加 String 属性
static bool SetNamedString(napi_env env, napi_value object, const char* name, const char* value)
{
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

// Helper 辅助函数：创建通用的结果对象
static napi_value CreateResultObject(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    return result;
}
} // namespace

// ---------------------------------------------------------------------------
// 用例 1: 正常创建 ArrayBuffer 基础流程
// ---------------------------------------------------------------------------
static napi_value TestCreateArrayBufferBasic(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = nullptr;
    napi_status status = napi_create_arraybuffer(env, TEST_SIZE_HUNDRED, &data, &arrayBuffer);

    bool isOk = (status == napi_ok);
    bool isNotNull = (arrayBuffer != nullptr);
    bool dataNotNull = (data != nullptr);

    (void)SetNamedBool(env, result, "statusOk", isOk);
    (void)SetNamedBool(env, result, "arrayBufferNotNull", isNotNull);
    (void)SetNamedBool(env, result, "dataNotNull", dataNotNull);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 2: 传入长度 0 时的表现
// ---------------------------------------------------------------------------
static napi_value TestCreateArrayBufferZeroLength(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = nullptr;
    napi_status status = napi_create_arraybuffer(env, TEST_SIZE_ZERO, &data, &arrayBuffer);

    bool isOk = (status == napi_ok);
    bool isNotNull = (arrayBuffer != nullptr);

    (void)SetNamedBool(env, result, "statusOk", isOk);
    (void)SetNamedBool(env, result, "arrayBufferNotNull", isNotNull);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 3: 传入极大长度（模拟分配失败边界）
// ---------------------------------------------------------------------------
static napi_value TestCreateArrayBufferTooLarge(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = nullptr;
    napi_status status = napi_create_arraybuffer(env, TEST_SIZE_TOO_LARGE, &data, &arrayBuffer);

    bool isNotOk = (status != napi_ok);

    (void)SetNamedBool(env, result, "statusNotOk", isNotOk);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 4: 返回的 napi_value 为空指针时的校验
// ---------------------------------------------------------------------------
static napi_value TestCreateArrayBufferNullResult(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_status status = napi_create_arraybuffer(env, TEST_SIZE_TEN, &data, nullptr);

    bool isInvalidArg = (status == napi_invalid_arg);

    (void)SetNamedBool(env, result, "statusInvalidArg", isInvalidArg);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 5: 数据指针传 nullptr 时的校验
// ---------------------------------------------------------------------------
static napi_value TestCreateArrayBufferNullData(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    napi_value arrayBuffer = nullptr;
    napi_status status = napi_create_arraybuffer(env, TEST_SIZE_TEN, nullptr, &arrayBuffer);

    bool isOk = (status == napi_ok);
    bool isNotNull = (arrayBuffer != nullptr);

    (void)SetNamedBool(env, result, "statusOk", isOk);
    (void)SetNamedBool(env, result, "arrayBufferNotNull", isNotNull);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 6: 溢出长度分配测试
// ---------------------------------------------------------------------------
static napi_value TestCreateArrayBufferUnderflowOverflow(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = nullptr;
    napi_status status = napi_create_arraybuffer(env, MAX_SIZE_T, &data, &arrayBuffer);

    bool isFailed = (status != napi_ok);

    (void)SetNamedBool(env, result, "statusFailed", isFailed);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 7: 数据一致性校验
// ---------------------------------------------------------------------------
static napi_value TestArrayBufferDataIntegrity(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = nullptr;
    napi_status status = napi_create_arraybuffer(env, TEST_SIZE_TEN, &data, &arrayBuffer);

    bool isOk = (status == napi_ok);
    bool writeMatch = false;

    if (isOk && data != nullptr) {
        uint8_t* byteData = static_cast<uint8_t*>(data);
        for (size_t i = INDEX_ZERO; i < TEST_SIZE_TEN; ++i) {
            byteData[i] = TEST_VAL_A;
        }

        writeMatch = true;
        for (size_t i = INDEX_ZERO; i < TEST_SIZE_TEN; ++i) {
            if (byteData[i] != TEST_VAL_A) {
                writeMatch = false;
                break;
            }
        }
    }

    (void)SetNamedBool(env, result, "statusOk", isOk);
    (void)SetNamedBool(env, result, "writeMatch", writeMatch);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 8: 正常获取 ArrayBuffer 长度和数据指针
// ---------------------------------------------------------------------------
static napi_value TestGetArrayBufferInfoBasic(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = nullptr;
    napi_status createStatus = napi_create_arraybuffer(env, TEST_SIZE_TEN, &createData, &arrayBuffer);

    void* infoData = nullptr;
    size_t infoLength = INDEX_ZERO;
    napi_status infoStatus = napi_get_arraybuffer_info(env, arrayBuffer, &infoData, &infoLength);

    bool isOk = (createStatus == napi_ok && infoStatus == napi_ok);
    bool dataMatch = (createData == infoData);
    bool lengthMatch = (infoLength == TEST_SIZE_TEN);

    (void)SetNamedBool(env, result, "statusOk", isOk);
    (void)SetNamedBool(env, result, "dataMatch", dataMatch);
    (void)SetNamedBool(env, result, "lengthMatch", lengthMatch);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 9: 获取信息时数据指针传 nullptr
// ---------------------------------------------------------------------------
static napi_value TestGetArrayBufferInfoNullData(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = nullptr;
    napi_status createStatus = napi_create_arraybuffer(env, TEST_SIZE_TEN, &createData, &arrayBuffer);

    size_t infoLength = INDEX_ZERO;
    napi_status infoStatus = napi_get_arraybuffer_info(env, arrayBuffer, nullptr, &infoLength);

    bool isOk = (createStatus == napi_ok && infoStatus == napi_ok);
    bool lengthMatch = (infoLength == TEST_SIZE_TEN);

    (void)SetNamedBool(env, result, "statusOk", isOk);
    (void)SetNamedBool(env, result, "lengthMatch", lengthMatch);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 10: 获取信息时长度指针传 nullptr
// ---------------------------------------------------------------------------
static napi_value TestGetArrayBufferInfoNullLength(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = nullptr;
    napi_status createStatus = napi_create_arraybuffer(env, TEST_SIZE_TEN, &createData, &arrayBuffer);

    void* infoData = nullptr;
    napi_status infoStatus = napi_get_arraybuffer_info(env, arrayBuffer, &infoData, nullptr);

    bool isOk = (createStatus == napi_ok && infoStatus == napi_ok);
    bool dataMatch = (createData == infoData);

    (void)SetNamedBool(env, result, "statusOk", isOk);
    (void)SetNamedBool(env, result, "dataMatch", dataMatch);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 11: 获取信息时双指针均传 nullptr
// ---------------------------------------------------------------------------
static napi_value TestGetArrayBufferInfoAllNull(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = nullptr;
    napi_status createStatus = napi_create_arraybuffer(env, TEST_SIZE_TEN, &createData, &arrayBuffer);

    napi_status infoStatus = napi_get_arraybuffer_info(env, arrayBuffer, nullptr, nullptr);

    bool isOk = (createStatus == napi_ok && infoStatus == napi_ok);

    (void)SetNamedBool(env, result, "statusOk", isOk);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 12: 对普通对象调用 get info 时的参数校验
// ---------------------------------------------------------------------------
static napi_value TestGetArrayBufferInfoInvalidObject(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    napi_value plainObject = nullptr;
    napi_status createStatus = napi_create_object(env, &plainObject);

    void* infoData = nullptr;
    size_t infoLength = INDEX_ZERO;
    napi_status infoStatus = napi_get_arraybuffer_info(env, plainObject, &infoData, &infoLength);

    bool isInvalidArg = (infoStatus == napi_invalid_arg);

    (void)SetNamedBool(env, result, "createStatusOk", createStatus == napi_ok);
    (void)SetNamedBool(env, result, "statusInvalidArg", isInvalidArg);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 13: 正确的 ArrayBuffer 被判定
// ---------------------------------------------------------------------------
static napi_value TestIsArrayBufferPositive(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = nullptr;
    napi_status createStatus = napi_create_arraybuffer(env, TEST_SIZE_TEN, &createData, &arrayBuffer);

    bool isArrayBuffer = false;
    napi_status checkStatus = napi_is_arraybuffer(env, arrayBuffer, &isArrayBuffer);

    bool isOk = (createStatus == napi_ok && checkStatus == napi_ok);

    (void)SetNamedBool(env, result, "statusOk", isOk);
    (void)SetNamedBool(env, result, "isArrayBuffer", isArrayBuffer);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 14: 普通 Object、String、Number 等类型判断的反向测试
// ---------------------------------------------------------------------------
static napi_value TestIsArrayBufferNegative(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    napi_value plainObject = nullptr;
    (void)napi_create_object(env, &plainObject);

    napi_value stringVal = nullptr;
    (void)napi_create_string_utf8(env, "notArrayBuffer", NAPI_AUTO_LENGTH, &stringVal);

    napi_value numberVal = nullptr;
    (void)napi_create_int32(env, RET_ERR_CODE, &numberVal);

    bool isObjectBuffer = true;
    (void)napi_is_arraybuffer(env, plainObject, &isObjectBuffer);

    bool isStringBuffer = true;
    (void)napi_is_arraybuffer(env, stringVal, &isStringBuffer);

    bool isNumberBuffer = true;
    (void)napi_is_arraybuffer(env, numberVal, &isNumberBuffer);

    (void)SetNamedBool(env, result, "objectIsNotBuffer", !isObjectBuffer);
    (void)SetNamedBool(env, result, "stringIsNotBuffer", !isStringBuffer);
    (void)SetNamedBool(env, result, "numberIsNotBuffer", !isNumberBuffer);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 15: TypedArray 等本身不是 ArrayBuffer 类型的二次校验
// ---------------------------------------------------------------------------
static napi_value TestArrayBufferIsArrayBufferDoubleCheck(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = nullptr;
    napi_status createStatus = napi_create_arraybuffer(env, TEST_SIZE_TEN, &createData, &arrayBuffer);

    napi_value typedArray = nullptr;
    napi_status typedStatus = napi_create_typedarray(
        env, napi_uint8_array, TEST_SIZE_TEN, arrayBuffer, INDEX_ZERO, &typedArray);

    bool isArrayBuffer = true;
    napi_status checkStatus = napi_is_arraybuffer(env, typedArray, &isArrayBuffer);

    bool isOk = (createStatus == napi_ok && typedStatus == napi_ok && checkStatus == napi_ok);

    (void)SetNamedBool(env, result, "statusOk", isOk);
    (void)SetNamedBool(env, result, "typedArrayIsNotArrayBuffer", !isArrayBuffer);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 16: 校验 napi_is_arraybuffer 在结果指针为空时的处理
// ---------------------------------------------------------------------------
static napi_value TestIsArrayBufferNullResult(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = nullptr;
    (void)napi_create_arraybuffer(env, TEST_SIZE_TEN, &createData, &arrayBuffer);

    napi_status status = napi_is_arraybuffer(env, arrayBuffer, nullptr);

    bool isInvalidArg = (status == napi_invalid_arg);

    (void)SetNamedBool(env, result, "statusInvalidArg", isInvalidArg);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 17: 正常剥离 ArrayBuffer 并检验
// ---------------------------------------------------------------------------
static napi_value TestDetachArrayBufferBasic(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = nullptr;
    (void)napi_create_arraybuffer(env, TEST_SIZE_TEN, &createData, &arrayBuffer);

    napi_status detachStatus = napi_detach_arraybuffer(env, arrayBuffer);

    bool isOk = (detachStatus == napi_ok);

    (void)SetNamedBool(env, result, "statusOk", isOk);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 18: 多次剥离同一个 ArrayBuffer
// ---------------------------------------------------------------------------
static napi_value TestDetachArrayBufferMultiple(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = nullptr;
    (void)napi_create_arraybuffer(env, TEST_SIZE_TEN, &createData, &arrayBuffer);

    napi_status detachStatusFirst = napi_detach_arraybuffer(env, arrayBuffer);
    napi_status detachStatusSecond = napi_detach_arraybuffer(env, arrayBuffer);

    bool isFirstOk = (detachStatusFirst == napi_ok);
    bool isSecondOk = (detachStatusSecond == napi_ok);

    (void)SetNamedBool(env, result, "firstOk", isFirstOk);
    (void)SetNamedBool(env, result, "secondOk", isSecondOk);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 19: 剥离长度为 0 的 ArrayBuffer
// ---------------------------------------------------------------------------
static napi_value TestDetachArrayBufferZeroLength(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = nullptr;
    (void)napi_create_arraybuffer(env, TEST_SIZE_ZERO, &createData, &arrayBuffer);

    napi_status detachStatus = napi_detach_arraybuffer(env, arrayBuffer);

    bool isOk = (detachStatus == napi_ok);

    (void)SetNamedBool(env, result, "statusOk", isOk);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 20: 剥离非 ArrayBuffer 对象的异常校验
// ---------------------------------------------------------------------------
static napi_value TestDetachArrayBufferInvalidObject(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    napi_value plainObject = nullptr;
    (void)napi_create_object(env, &plainObject);

    napi_status status = napi_detach_arraybuffer(env, plainObject);

    bool isInvalidArg = (status == napi_invalid_arg);

    (void)SetNamedBool(env, result, "statusInvalidArg", isInvalidArg);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 21: 精确测试剥离前后状态变化
// ---------------------------------------------------------------------------
static napi_value TestIsDetachedArrayBufferBasic(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = nullptr;
    (void)napi_create_arraybuffer(env, TEST_SIZE_TEN, &createData, &arrayBuffer);

    bool isDetachedBefore = true;
    (void)napi_is_detached_arraybuffer(env, arrayBuffer, &isDetachedBefore);

    (void)napi_detach_arraybuffer(env, arrayBuffer);

    bool isDetachedAfter = false;
    (void)napi_is_detached_arraybuffer(env, arrayBuffer, &isDetachedAfter);

    (void)SetNamedBool(env, result, "notDetachedBefore", !isDetachedBefore);
    (void)SetNamedBool(env, result, "detachedAfter", isDetachedAfter);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 22: 未剥离状态判断的测试
// ---------------------------------------------------------------------------
static napi_value TestIsDetachedArrayBufferNegative(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = nullptr;
    (void)napi_create_arraybuffer(env, TEST_SIZE_TEN, &createData, &arrayBuffer);

    bool isDetached = true;
    napi_status status = napi_is_detached_arraybuffer(env, arrayBuffer, &isDetached);

    bool isOk = (status == napi_ok);

    (void)SetNamedBool(env, result, "statusOk", isOk);
    (void)SetNamedBool(env, result, "isDetached", isDetached);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 23: 对非 ArrayBuffer 对象调用 napi_is_detached_arraybuffer 的处理
// ---------------------------------------------------------------------------
static napi_value TestIsDetachedArrayBufferInvalidObject(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    napi_value plainObject = nullptr;
    (void)napi_create_object(env, &plainObject);

    bool isDetached = false;
    napi_status status = napi_is_detached_arraybuffer(env, plainObject, &isDetached);

    bool isInvalidArg = (status == napi_invalid_arg);

    (void)SetNamedBool(env, result, "statusInvalidArg", isInvalidArg);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 24: napi_is_detached_arraybuffer 结果指针为 nullptr 时的测试
// ---------------------------------------------------------------------------
static napi_value TestIsDetachedArrayBufferNullResult(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = nullptr;
    (void)napi_create_arraybuffer(env, TEST_SIZE_TEN, &createData, &arrayBuffer);

    napi_status status = napi_is_detached_arraybuffer(env, arrayBuffer, nullptr);

    bool isInvalidArg = (status == napi_invalid_arg);

    (void)SetNamedBool(env, result, "statusInvalidArg", isInvalidArg);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 25: 剥离后重新读取数据信息的安全行为
// ---------------------------------------------------------------------------
static napi_value TestGetArrayBufferInfoAfterDetach(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = nullptr;
    (void)napi_create_arraybuffer(env, TEST_SIZE_TEN, &createData, &arrayBuffer);

    (void)napi_detach_arraybuffer(env, arrayBuffer);

    void* infoData = nullptr;
    size_t infoLength = TEST_SIZE_TEN;
    napi_status infoStatus = napi_get_arraybuffer_info(env, arrayBuffer, &infoData, &infoLength);

    bool isOk = (infoStatus == napi_ok);
    bool dataIsNull = (infoData == nullptr);
    bool lengthIsZero = (infoLength == INDEX_ZERO);

    (void)SetNamedBool(env, result, "statusOk", isOk);
    (void)SetNamedBool(env, result, "dataIsNull", dataIsNull);
    (void)SetNamedBool(env, result, "lengthIsZero", lengthIsZero);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 26: 剥离后读写行为的安全边界测试
// ---------------------------------------------------------------------------
static napi_value TestArrayBufferDataReadWriteAfterDetach(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = nullptr;
    napi_status createStatus = napi_create_arraybuffer(env, TEST_SIZE_TEN, &createData, &arrayBuffer);

    bool writeMatchBefore = false;
    bool isOk = (createStatus == napi_ok);

    if (isOk && createData != nullptr) {
        uint8_t* byteData = static_cast<uint8_t*>(createData);
        byteData[INDEX_ZERO] = TEST_VAL_A;
        byteData[INDEX_ONE] = TEST_VAL_B;
        if (byteData[INDEX_ZERO] == TEST_VAL_A && byteData[INDEX_ONE] == TEST_VAL_B) {
            writeMatchBefore = true;
        }
    }

    napi_status detachStatus = napi_detach_arraybuffer(env, arrayBuffer);

    void* infoData = nullptr;
    size_t infoLength = INDEX_ZERO;
    napi_status infoStatus = napi_get_arraybuffer_info(env, arrayBuffer, &infoData, &infoLength);

    bool isDetachedOk = (detachStatus == napi_ok && infoStatus == napi_ok);
    bool isCleared = (infoData == nullptr && infoLength == INDEX_ZERO);

    (void)SetNamedBool(env, result, "writeMatchBefore", writeMatchBefore);
    (void)SetNamedBool(env, result, "isDetachedOk", isDetachedOk);
    (void)SetNamedBool(env, result, "isCleared", isCleared);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 27: 检查新创建的 ArrayBuffer 的初始内存是否全零
// ---------------------------------------------------------------------------
static napi_value TestCreateArrayBufferCheckInitialValues(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* data = nullptr;
    napi_value arrayBuffer = nullptr;
    napi_status status = napi_create_arraybuffer(env, TEST_SIZE_HUNDRED, &data, &arrayBuffer);

    bool isOk = (status == napi_ok);
    bool initialZero = true;

    if (isOk && data != nullptr) {
        uint8_t* byteData = static_cast<uint8_t*>(data);
        for (size_t i = INDEX_ZERO; i < TEST_SIZE_HUNDRED; ++i) {
            if (byteData[i] != TEST_VAL_D) {
                initialZero = false;
                break;
            }
        }
    }

    (void)SetNamedBool(env, result, "statusOk", isOk);
    (void)SetNamedBool(env, result, "initialZero", initialZero);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 28: 连续两次获取 info 保持一致性的测试
// ---------------------------------------------------------------------------
static napi_value TestArrayBufferGetInfoTwice(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = nullptr;
    (void)napi_create_arraybuffer(env, TEST_SIZE_TEN, &createData, &arrayBuffer);

    void* infoDataFirst = nullptr;
    size_t infoLengthFirst = INDEX_ZERO;
    napi_status statusFirst = napi_get_arraybuffer_info(env, arrayBuffer, &infoDataFirst, &infoLengthFirst);

    void* infoDataSecond = nullptr;
    size_t infoLengthSecond = INDEX_ZERO;
    napi_status statusSecond = napi_get_arraybuffer_info(env, arrayBuffer, &infoDataSecond, &infoLengthSecond);

    bool isOk = (statusFirst == napi_ok && statusSecond == napi_ok);
    bool dataMatch = (infoDataFirst == infoDataSecond);
    bool lengthMatch = (infoLengthFirst == infoLengthSecond);

    (void)SetNamedBool(env, result, "statusOk", isOk);
    (void)SetNamedBool(env, result, "dataMatch", dataMatch);
    (void)SetNamedBool(env, result, "lengthMatch", lengthMatch);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 29: 在同一 ArrayBuffer 上建立视图并在剥离后检测视图表现
// ---------------------------------------------------------------------------
static napi_value TestArrayBufferMultipleViewsOnSameBuffer(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = nullptr;
    (void)napi_create_arraybuffer(env, TEST_SIZE_TEN, &createData, &arrayBuffer);

    napi_value firstView = nullptr;
    (void)napi_create_typedarray(env, napi_uint8_array, TEST_SIZE_TEN, arrayBuffer, INDEX_ZERO, &firstView);

    napi_value secondView = nullptr;
    (void)napi_create_typedarray(env, napi_uint8_array, TEST_SIZE_TEN, arrayBuffer, INDEX_ZERO, &secondView);

    napi_status detachStatus = napi_detach_arraybuffer(env, arrayBuffer);

    bool isDetached = false;
    (void)napi_is_detached_arraybuffer(env, arrayBuffer, &isDetached);

    (void)SetNamedBool(env, result, "detachOk", detachStatus == napi_ok);
    (void)SetNamedBool(env, result, "isDetached", isDetached);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 30: 获取信息指针并对内存进行二次读写的安全性测试
// ---------------------------------------------------------------------------
static napi_value TestArrayBufferDataModificationAfterGetInfo(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = nullptr;
    (void)napi_create_arraybuffer(env, TEST_SIZE_TEN, &createData, &arrayBuffer);

    void* infoData = nullptr;
    size_t infoLength = INDEX_ZERO;
    (void)napi_get_arraybuffer_info(env, arrayBuffer, &infoData, &infoLength);

    bool writeSuccess = false;
    if (infoData != nullptr && infoLength == TEST_SIZE_TEN) {
        uint8_t* byteData = static_cast<uint8_t*>(infoData);
        byteData[INDEX_ZERO] = TEST_VAL_A;
        byteData[INDEX_ONE] = TEST_VAL_B;
        if (byteData[INDEX_ZERO] == TEST_VAL_A && byteData[INDEX_ONE] == TEST_VAL_B) {
            writeSuccess = true;
        }
    }

    (void)SetNamedBool(env, result, "writeSuccess", writeSuccess);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 31: 循环测试不同大小 ArrayBuffer 的创建和读取信息
// ---------------------------------------------------------------------------
static napi_value TestCreateArrayBufferMultipleSizes(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    size_t sizes[] = {
        TEST_SIZE_16, TEST_SIZE_32, TEST_SIZE_64, TEST_SIZE_128, TEST_SIZE_256, TEST_SIZE_512, TEST_SIZE_1024
    };
    constexpr size_t sizeCount = sizeof(sizes) / sizeof(sizes[INDEX_ZERO]);

    bool allSizesOk = true;
    for (size_t i = INDEX_ZERO; i < sizeCount; ++i) {
        void* data = nullptr;
        napi_value arrayBuffer = nullptr;
        napi_status createStatus = napi_create_arraybuffer(env, sizes[i], &data, &arrayBuffer);
        void* infoData = nullptr;
        size_t infoLength = INDEX_ZERO;
        napi_status infoStatus = napi_get_arraybuffer_info(env, arrayBuffer, &infoData, &infoLength);
        if (createStatus != napi_ok || infoStatus != napi_ok || infoLength != sizes[i] || data == nullptr) {
            allSizesOk = false;
            break;
        }
    }

    (void)SetNamedBool(env, result, "allSizesOk", allSizesOk);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 32: 批量剥离并双重检查其状态
// ---------------------------------------------------------------------------
static napi_value TestArrayBufferDetachAndIsDetachedDoubleCheck(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* dataOne = nullptr;
    napi_value bufferOne = nullptr;
    (void)napi_create_arraybuffer(env, TEST_SIZE_TEN, &dataOne, &bufferOne);

    void* dataTwo = nullptr;
    napi_value bufferTwo = nullptr;
    (void)napi_create_arraybuffer(env, TEST_SIZE_TEN, &dataTwo, &bufferTwo);

    (void)napi_detach_arraybuffer(env, bufferOne);

    bool isDetachedOne = false;
    (void)napi_is_detached_arraybuffer(env, bufferOne, &isDetachedOne);

    bool isDetachedTwo = true;
    (void)napi_is_detached_arraybuffer(env, bufferTwo, &isDetachedTwo);

    bool checkPass = (isDetachedOne && !isDetachedTwo);

    (void)SetNamedBool(env, result, "checkPass", checkPass);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 33: 剥离后再次剥离的深度测试
// ---------------------------------------------------------------------------
static napi_value TestArrayBufferDoubleDetachAfterGetInfo(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = nullptr;
    (void)napi_create_arraybuffer(env, TEST_SIZE_TEN, &createData, &arrayBuffer);

    void* infoData = nullptr;
    size_t infoLength = INDEX_ZERO;
    (void)napi_get_arraybuffer_info(env, arrayBuffer, &infoData, &infoLength);

    napi_status statusFirst = napi_detach_arraybuffer(env, arrayBuffer);
    napi_status statusSecond = napi_detach_arraybuffer(env, arrayBuffer);

    bool isDetached = false;
    (void)napi_is_detached_arraybuffer(env, arrayBuffer, &isDetached);

    bool isOk = (statusFirst == napi_ok && statusSecond == napi_ok && isDetached);

    (void)SetNamedBool(env, result, "isOk", isOk);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 34: 剥离后尝试修改底层内存的行为测试
// ---------------------------------------------------------------------------
static napi_value TestArrayBufferMemoryCleanliness(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    void* createData = nullptr;
    napi_value arrayBuffer = nullptr;
    (void)napi_create_arraybuffer(env, TEST_SIZE_TEN, &createData, &arrayBuffer);

    if (createData != nullptr) {
        uint8_t* byteData = static_cast<uint8_t*>(createData);
        byteData[INDEX_ZERO] = TEST_VAL_C;
    }

    napi_status detachStatus = napi_detach_arraybuffer(env, arrayBuffer);

    void* infoData = nullptr;
    size_t infoLength = INDEX_ZERO;
    napi_status infoStatus = napi_get_arraybuffer_info(env, arrayBuffer, &infoData, &infoLength);

    bool isDetached = false;
    (void)napi_is_detached_arraybuffer(env, arrayBuffer, &isDetached);

    bool isOk = (detachStatus == napi_ok && infoStatus == napi_ok && isDetached);
    bool isCleaned = (infoData == nullptr && infoLength == INDEX_ZERO);

    (void)SetNamedBool(env, result, "isOk", isOk);
    (void)SetNamedBool(env, result, "isCleaned", isCleaned);

    return result;
}

// ---------------------------------------------------------------------------
// 用例 35: 大量 ArrayBuffer 并发创建与剥离状态一致性模拟
// ---------------------------------------------------------------------------
static napi_value TestArrayBufferBulkOperations(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResultObject(env);
    constexpr size_t count = INDEX_TEN;
    napi_value buffers[count];
    void* datas[count];

    bool createSuccess = true;
    for (size_t i = INDEX_ZERO; i < count; ++i) {
        buffers[i] = nullptr;
        datas[i] = nullptr;
        napi_status status = napi_create_arraybuffer(env, TEST_SIZE_TEN, &datas[i], &buffers[i]);
        if (status != napi_ok || buffers[i] == nullptr || datas[i] == nullptr) {
            createSuccess = false;
        }
    }

    bool detachSuccess = true;
    for (size_t i = INDEX_ZERO; i < count; ++i) {
        if (buffers[i] != nullptr) {
            napi_status status = napi_detach_arraybuffer(env, buffers[i]);
            if (status != napi_ok) {
                detachSuccess = false;
            }
        }
    }

    bool isDetachedCheck = true;
    for (size_t i = INDEX_ZERO; i < count; ++i) {
        if (buffers[i] != nullptr) {
            bool isDetached = false;
            (void)napi_is_detached_arraybuffer(env, buffers[i], &isDetached);
            if (!isDetached) {
                isDetachedCheck = false;
            }
        }
    }

    (void)SetNamedBool(env, result, "createSuccess", createSuccess);
    (void)SetNamedBool(env, result, "detachSuccess", detachSuccess);
    (void)SetNamedBool(env, result, "isDetachedCheck", isDetachedCheck);

    return result;
}

// 匿名命名空间，放置测试用例表
namespace {
struct ArrayBufferTest {
    const char* name;
    napi_callback_cb callback;
};

static const ArrayBufferTest ARRAYBUFFER_TESTS[] = {
    { "testCreateArrayBufferBasic", TestCreateArrayBufferBasic },
    { "testCreateArrayBufferZeroLength", TestCreateArrayBufferZeroLength },
    { "testCreateArrayBufferTooLarge", TestCreateArrayBufferTooLarge },
    { "testCreateArrayBufferNullResult", TestCreateArrayBufferNullResult },
    { "testCreateArrayBufferNullData", TestCreateArrayBufferNullData },
    { "testCreateArrayBufferUnderflowOverflow", TestCreateArrayBufferUnderflowOverflow },
    { "testArrayBufferDataIntegrity", TestArrayBufferDataIntegrity },
    { "testGetArrayBufferInfoBasic", TestGetArrayBufferInfoBasic },
    { "testGetArrayBufferInfoNullData", TestGetArrayBufferInfoNullData },
    { "testGetArrayBufferInfoNullLength", TestGetArrayBufferInfoNullLength },
    { "testGetArrayBufferInfoAllNull", TestGetArrayBufferInfoAllNull },
    { "testGetArrayBufferInfoInvalidObject", TestGetArrayBufferInfoInvalidObject },
    { "testIsArrayBufferPositive", TestIsArrayBufferPositive },
    { "testIsArrayBufferNegative", TestIsArrayBufferNegative },
    { "testArrayBufferIsArrayBufferDoubleCheck", TestArrayBufferIsArrayBufferDoubleCheck },
    { "testIsArrayBufferNullResult", TestIsArrayBufferNullResult },
    { "testDetachArrayBufferBasic", TestDetachArrayBufferBasic },
    { "testDetachArrayBufferMultiple", TestDetachArrayBufferMultiple },
    { "testDetachArrayBufferZeroLength", TestDetachArrayBufferZeroLength },
    { "testDetachArrayBufferInvalidObject", TestDetachArrayBufferInvalidObject },
    { "testIsDetachedArrayBufferBasic", TestIsDetachedArrayBufferBasic },
    { "testIsDetachedArrayBufferNegative", TestIsDetachedArrayBufferNegative },
    { "testIsDetachedArrayBufferInvalidObject", TestIsDetachedArrayBufferInvalidObject },
    { "testIsDetachedArrayBufferNullResult", TestIsDetachedArrayBufferNullResult },
    { "testGetArrayBufferInfoAfterDetach", TestGetArrayBufferInfoAfterDetach },
    { "testArrayBufferDataReadWriteAfterDetach", TestArrayBufferDataReadWriteAfterDetach },
    { "testCreateArrayBufferCheckInitialValues", TestCreateArrayBufferCheckInitialValues },
    { "testArrayBufferGetInfoTwice", TestArrayBufferGetInfoTwice },
    { "testArrayBufferMultipleViewsOnSameBuffer", TestArrayBufferMultipleViewsOnSameBuffer },
    { "testArrayBufferDataModificationAfterGetInfo", TestArrayBufferDataModificationAfterGetInfo },
    { "testCreateArrayBufferMultipleSizes", TestCreateArrayBufferMultipleSizes },
    { "testArrayBufferDetachAndIsDetachedDoubleCheck", TestArrayBufferDetachAndIsDetachedDoubleCheck },
    { "testArrayBufferDoubleDetachAfterGetInfo", TestArrayBufferDoubleDetachAfterGetInfo },
    { "testArrayBufferMemoryCleanliness", TestArrayBufferMemoryCleanliness },
    { "testArrayBufferBulkOperations", TestArrayBufferBulkOperations }
};

static constexpr size_t ARRAYBUFFER_TEST_COUNT = sizeof(ARRAYBUFFER_TESTS) / sizeof(ARRAYBUFFER_TESTS[INDEX_ZERO]);
} // namespace

// ---------------------------------------------------------------------------
// 模块初始化入口
// ---------------------------------------------------------------------------
static napi_value InitArrayBufferSuite(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[ARRAYBUFFER_TEST_COUNT];
    for (size_t i = INDEX_ZERO; i < ARRAYBUFFER_TEST_COUNT; i++) {
        descriptors[i] = napi_property_descriptor {
            ARRAYBUFFER_TESTS[i].name,
            nullptr,
            ARRAYBUFFER_TESTS[i].callback,
            nullptr,
            nullptr,
            nullptr,
            napi_default,
            nullptr
        };
    }
    NAPI_CALL(env, napi_define_properties(env, exports, ARRAYBUFFER_TEST_COUNT, descriptors));
    return exports;
}

static napi_module g_arrayBufferSuiteModule = {
    .nm_version = MODULE_VERSION,
    .nm_flags = NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitArrayBufferSuite,
    .nm_modname = "arraybuffer_suite",
    .nm_priv = nullptr,
    .reserved = { 0 }
};

extern "C" __attribute__((constructor)) void RegisterArrayBufferSuiteModule(void)
{
    napi_module_register(&g_arrayBufferSuiteModule);
}
