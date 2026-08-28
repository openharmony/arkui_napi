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

// 匿名命名空间，用于文件级常量及 UPPER_CASE 常量定义
namespace {
constexpr int32_t VAL_ZERO = 0;
constexpr int32_t VAL_ONE = 1;
constexpr int32_t VAL_NEG_ONE = -1;
constexpr int32_t VAL_TEN = 10;
constexpr int32_t VAL_FORTY_TWO = 42;
constexpr int32_t VAL_STRESS_MAX = 50;
constexpr int32_t VAL_HUNDRED = 100;
constexpr int32_t VAL_TWO_HUNDRED = 200;
constexpr int32_t VAL_THREE_HUNDRED = 300;
constexpr int32_t VAL_FOUR_HUNDRED = 400;
constexpr double DOUBLE_VAL_PI = 3.14159;
constexpr napi_type_tag LOCAL_TYPE_TAG = { 0x1234567890abcdef, 0xfedcba0987654321 };
}

// ============================================================================
// 辅助函数定义
// ============================================================================

static void SetNamedBool(napi_env env, napi_value obj, const char* name, bool value)
{
    napi_value napiValue = nullptr;
    (void)napi_get_boolean(env, value, &napiValue);
    (void)napi_set_named_property(env, obj, name, napiValue);
}

static void SetNamedInt32(napi_env env, napi_value obj, const char* name, int32_t value)
{
    napi_value napiValue = nullptr;
    (void)napi_create_int32(env, value, &napiValue);
    (void)napi_set_named_property(env, obj, name, napiValue);
}

static void SetNamedString(napi_env env, napi_value obj, const char* name, const char* value)
{
    napi_value napiValue = nullptr;
    (void)napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &napiValue);
    (void)napi_set_named_property(env, obj, name, napiValue);
}

static napi_value CreateResult(napi_env env)
{
    napi_value result = nullptr;
    (void)napi_create_object(env, &result);
    return result;
}

static napi_status RunScript(napi_env env, const char* scriptStr)
{
    napi_value script = nullptr;
    napi_status status = napi_create_string_utf8(env, scriptStr, NAPI_AUTO_LENGTH, &script);
    if (status != napi_ok) {
        return status;
    }
    napi_value result = nullptr;
    return napi_run_script(env, script, &result);
}

static napi_status RunScriptGetInt32(napi_env env, const char* scriptStr, int32_t* value)
{
    napi_value script = nullptr;
    napi_status status = napi_create_string_utf8(env, scriptStr, NAPI_AUTO_LENGTH, &script);
    if (status != napi_ok) {
        return status;
    }
    napi_value result = nullptr;
    status = napi_run_script(env, script, &result);
    if (status != napi_ok) {
        return status;
    }
    return napi_get_value_int32(env, result, value);
}

// ============================================================================
// 测试用例 1 到 36
// ============================================================================

static napi_value TestCreateContextBasic(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool isCreateOk = (status == napi_ok) && (newEnv != nullptr);
    if (isCreateOk) {
        napi_status destroyStatus = napi_destroy_ark_context(newEnv);
        SetNamedBool(env, result, "destroyOk", destroyStatus == napi_ok);
    }
    SetNamedBool(env, result, "createOk", isCreateOk);
    return result;
}

static napi_value TestCreateContextNullptr(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status1 = napi_create_ark_context(nullptr, &newEnv);
    napi_status status2 = napi_create_ark_context(env, nullptr);
    SetNamedBool(env, result, "envNullptrOk", status1 == napi_invalid_arg);
    SetNamedBool(env, result, "outNullptrOk", status2 == napi_invalid_arg);
    return result;
}

static napi_value TestCreateMultipleContexts(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv1 = nullptr;
    napi_env newEnv2 = nullptr;
    napi_status status1 = napi_create_ark_context(env, &newEnv1);
    napi_status status2 = napi_create_ark_context(env, &newEnv2);
    bool isBothCreated = (status1 == napi_ok) && (status2 == napi_ok);
    bool areDifferent = (newEnv1 != newEnv2);
    if (status1 == napi_ok) {
        (void)napi_destroy_ark_context(newEnv1);
    }
    if (status2 == napi_ok) {
        (void)napi_destroy_ark_context(newEnv2);
    }
    SetNamedBool(env, result, "bothCreated", isBothCreated);
    SetNamedBool(env, result, "differentEnvs", areDifferent);
    return result;
}

static napi_value TestSwitchContextBasic(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool step1 = (status == napi_ok);
    bool step2 = false;
    bool step3 = false;
    if (step1) {
        step2 = (napi_switch_ark_context(newEnv) == napi_ok);
        step3 = (napi_switch_ark_context(env) == napi_ok);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "createOk", step1);
    SetNamedBool(env, result, "switchToNewOk", step2);
    SetNamedBool(env, result, "switchBackOk", step3);
    return result;
}

static napi_value TestSwitchContextNullptr(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_status status = napi_switch_ark_context(nullptr);
    SetNamedBool(env, result, "switchNullptrOk", status == napi_invalid_arg);
    return result;
}

static napi_value TestDestroyContextNullptr(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_status status = napi_destroy_ark_context(nullptr);
    SetNamedBool(env, result, "destroyNullptrOk", status == napi_invalid_arg);
    return result;
}

static napi_value TestDestroyActiveContext(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status createStatus = napi_create_ark_context(env, &newEnv);
    bool isDestroyFailed = false;
    if (createStatus == napi_ok) {
        (void)napi_switch_ark_context(newEnv);
        napi_status destroyStatus = napi_destroy_ark_context(newEnv);
        isDestroyFailed = (destroyStatus == napi_invalid_arg);
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "destroyActiveFailedOk", isDestroyFailed);
    return result;
}

static napi_value TestDestroyMainContext(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_status status = napi_destroy_ark_context(env);
    SetNamedBool(env, result, "destroyMainFailedOk", status == napi_invalid_arg);
    return result;
}

static napi_value TestContextGlobalIsolation(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    if (status != napi_ok) {
        return nullptr;
    }
    (void)RunScript(env, "globalThis.testVal = 100;");
    (void)napi_switch_ark_context(newEnv);
    (void)RunScript(newEnv, "globalThis.testVal = 200;");
    int32_t valInNew = VAL_ZERO;
    (void)RunScriptGetInt32(newEnv, "globalThis.testVal;", &valInNew);
    (void)napi_switch_ark_context(env);
    int32_t valInMain = VAL_ZERO;
    (void)RunScriptGetInt32(env, "globalThis.testVal;", &valInMain);
    (void)napi_destroy_ark_context(newEnv);
    SetNamedBool(env, result, "isolationOk", (valInMain == VAL_HUNDRED) && (valInNew == VAL_TWO_HUNDRED));
    return result;
}

static napi_value TestContextLifecycleNested(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env env1 = nullptr;
    napi_env env2 = nullptr;
    bool step1 = (napi_create_ark_context(env, &env1) == napi_ok);
    bool step2 = false;
    bool step3 = false;
    if (step1) {
        (void)napi_switch_ark_context(env1);
        step2 = (napi_create_ark_context(env1, &env2) == napi_ok);
        if (step2) {
            (void)napi_switch_ark_context(env2);
            step3 = (napi_switch_ark_context(env1) == napi_ok);
            (void)napi_destroy_ark_context(env2);
        }
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(env1);
    }
    SetNamedBool(env, result, "createEnv1Ok", step1);
    SetNamedBool(env, result, "createEnv2Ok", step2);
    SetNamedBool(env, result, "switchBackOk", step3);
    return result;
}

static napi_value TestContextSwitchStress(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool stressOk = true;
    if (status == napi_ok) {
        for (int32_t i = VAL_ZERO; i < VAL_STRESS_MAX; i++) {
            if (napi_switch_ark_context(newEnv) != napi_ok) {
                stressOk = false;
                break;
            }
            if (napi_switch_ark_context(env) != napi_ok) {
                stressOk = false;
                break;
            }
        }
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "stressSwitchOk", stressOk && (status == napi_ok));
    return result;
}

static napi_value TestContextGlobalLeak(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    if (status != napi_ok) {
        return nullptr;
    }
    (void)RunScript(env, "globalThis.leakFunc = function() { return 42; };");
    (void)napi_switch_ark_context(newEnv);
    napi_value leakScript = nullptr;
    (void)napi_create_string_utf8(newEnv, "globalThis.leakFunc();", NAPI_AUTO_LENGTH, &leakScript);
    napi_value scriptRes = nullptr;
    napi_status runStatus = napi_run_script(newEnv, leakScript, &scriptRes);
    (void)napi_switch_ark_context(env);
    (void)napi_destroy_ark_context(newEnv);
    SetNamedBool(env, result, "runFailedAsExpected", runStatus != napi_ok);
    return result;
}

static napi_value TestCreateContextUnderException(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    (void)napi_throw_error(env, nullptr, "error message");
    napi_env newEnv = nullptr;
    napi_status createStatus = napi_create_ark_context(env, &newEnv);
    napi_value exception = nullptr;
    (void)napi_get_and_clear_last_exception(env, &exception);
    SetNamedBool(env, result, "createFailedUnderException", createStatus == napi_pending_exception);
    return result;
}

static napi_value TestSwitchContextUnderException(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool switchFailed = false;
    if (status == napi_ok) {
        (void)napi_throw_error(env, nullptr, "error message");
        napi_status switchStatus = napi_switch_ark_context(newEnv);
        switchFailed = (switchStatus == napi_pending_exception);
        napi_value exception = nullptr;
        (void)napi_get_and_clear_last_exception(env, &exception);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "switchFailedUnderException", switchFailed);
    return result;
}

static napi_value TestDestroyContextUnderException(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool destroyFailed = false;
    if (status == napi_ok) {
        (void)napi_throw_error(env, nullptr, "error message");
        napi_status destroyStatus = napi_destroy_ark_context(newEnv);
        destroyFailed = (destroyStatus == napi_pending_exception);
        napi_value exception = nullptr;
        (void)napi_get_and_clear_last_exception(env, &exception);
        if (destroyFailed) {
            (void)napi_destroy_ark_context(newEnv);
        }
    }
    SetNamedBool(env, result, "destroyFailedUnderException", destroyFailed);
    return result;
}

static napi_value TestSwitchToSameContext(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_status status1 = napi_switch_ark_context(env);
    napi_env newEnv = nullptr;
    napi_status status2 = napi_create_ark_context(env, &newEnv);
    bool status3 = false;
    if (status2 == napi_ok) {
        (void)napi_switch_ark_context(newEnv);
        status3 = (napi_switch_ark_context(newEnv) == napi_ok);
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "switchToSelfMainOk", status1 == napi_ok);
    SetNamedBool(env, result, "switchToSelfNewOk", status3);
    return result;
}

static napi_value TestContextFunctionSharing(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool sharedUseFailed = false;
    if (status == napi_ok) {
        napi_value func = nullptr;
        (void)RunScript(env, "globalThis.func = function() { return 42; };");
        (void)napi_get_named_property(env, result, "func", &func);
        (void)napi_switch_ark_context(newEnv);
        napi_value callRes = nullptr;
        napi_status callStatus = napi_call_function(newEnv, result, func, VAL_ZERO, nullptr, &callRes);
        sharedUseFailed = (callStatus != napi_ok);
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "sharedUseFailedOk", sharedUseFailed);
    return result;
}

static napi_value TestContextScopeBasic(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool scopeOk = false;
    if (status == napi_ok) {
        (void)napi_switch_ark_context(newEnv);
        napi_handle_scope scope = nullptr;
        napi_status openStatus = napi_open_handle_scope(newEnv, &scope);
        napi_value temp = nullptr;
        (void)napi_create_object(newEnv, &temp);
        napi_status closeStatus = napi_close_handle_scope(newEnv, scope);
        scopeOk = (openStatus == napi_ok) && (closeStatus == napi_ok);
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "scopeOk", scopeOk);
    return result;
}

static napi_value TestContextDefineClass(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool classOk = false;
    if (status == napi_ok) {
        (void)napi_switch_ark_context(newEnv);
        (void)RunScript(newEnv, "class MyClass { constructor() { this.val = 42; } } globalThis.MyClass = MyClass;");
        int32_t val = VAL_ZERO;
        (void)RunScriptGetInt32(newEnv, "new globalThis.MyClass().val;", &val);
        classOk = (val == VAL_FORTY_TWO);
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "classIsolationOk", classOk);
    return result;
}

static napi_value TestContextPromiseBasic(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool promiseOk = false;
    if (status == napi_ok) {
        (void)napi_switch_ark_context(newEnv);
        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        napi_status createPromise = napi_create_promise(newEnv, &deferred, &promise);
        napi_value val = nullptr;
        (void)napi_create_int32(newEnv, VAL_FORTY_TWO, &val);
        napi_status resolveStatus = napi_resolve_deferred(newEnv, deferred, val);
        promiseOk = (createPromise == napi_ok) && (resolveStatus == napi_ok);
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "promiseOk", promiseOk);
    return result;
}

static napi_value TestContextCreateArrayBuffer(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool bufferOk = false;
    if (status == napi_ok) {
        (void)napi_switch_ark_context(newEnv);
        void* data = nullptr;
        napi_value arrayBuffer = nullptr;
        napi_status createStatus = napi_create_arraybuffer(newEnv, VAL_HUNDRED, &data, &arrayBuffer);
        bool isDataNotNull = (data != nullptr);
        bufferOk = (createStatus == napi_ok) && isDataNotNull;
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "arrayBufferOk", bufferOk);
    return result;
}

static napi_value TestContextBigInt(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool bigintOk = false;
    if (status == napi_ok) {
        (void)napi_switch_ark_context(newEnv);
        napi_value bigint = nullptr;
        napi_status createStatus = napi_create_bigint_int64(newEnv, VAL_TWO_HUNDRED, &bigint);
        int64_t val = VAL_ZERO;
        bool lossless = false;
        napi_status getStatus = napi_get_value_bigint_int64(newEnv, bigint, &val, &lossless);
        bigintOk = (createStatus == napi_ok) && (getStatus == napi_ok) && (val == VAL_TWO_HUNDRED);
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "bigintOk", bigintOk);
    return result;
}

static napi_value TestContextTypeTag(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool tagOk = false;
    if (status == napi_ok) {
        (void)napi_switch_ark_context(newEnv);
        napi_value obj = nullptr;
        (void)napi_create_object(newEnv, &obj);
        napi_status tagStatus = napi_type_tag_object(newEnv, obj, &LOCAL_TYPE_TAG);
        bool hasTag = false;
        napi_status checkStatus = napi_check_object_type_tag(newEnv, obj, &LOCAL_TYPE_TAG, &hasTag);
        tagOk = (tagStatus == napi_ok) && (checkStatus == napi_ok) && hasTag;
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "typeTagOk", tagOk);
    return result;
}

static napi_value TestContextExceptionInheritance(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool checkOk = false;
    if (status == napi_ok) {
        (void)napi_switch_ark_context(newEnv);
        (void)napi_throw_error(newEnv, nullptr, "exception in newEnv");
        bool hasExceptionNew = false;
        (void)napi_is_exception_pending(newEnv, &hasExceptionNew);
        (void)napi_switch_ark_context(env);
        bool hasExceptionMain = false;
        (void)napi_is_exception_pending(env, &hasExceptionMain);
        checkOk = hasExceptionNew && (!hasExceptionMain);
        (void)napi_switch_ark_context(newEnv);
        napi_value exception = nullptr;
        (void)napi_get_and_clear_last_exception(newEnv, &exception);
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "exceptionIsolatedOk", checkOk);
    return result;
}

static napi_value TestContextNestedSwitch(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env envB = nullptr;
    napi_env envC = nullptr;
    bool step1 = (napi_create_ark_context(env, &envB) == napi_ok);
    bool step2 = (napi_create_ark_context(env, &envC) == napi_ok);
    bool checkOk = false;
    if (step1 && step2) {
        bool switch1 = (napi_switch_ark_context(envB) == napi_ok);
        bool switch2 = (napi_switch_ark_context(envC) == napi_ok);
        bool switch3 = (napi_switch_ark_context(envB) == napi_ok);
        bool switch4 = (napi_switch_ark_context(env) == napi_ok);
        checkOk = switch1 && switch2 && switch3 && switch4;
    }
    if (step1) {
        (void)napi_destroy_ark_context(envB);
    }
    if (step2) {
        (void)napi_destroy_ark_context(envC);
    }
    SetNamedBool(env, result, "nestedSwitchOk", checkOk);
    return result;
}

static napi_value TestContextPropertyDescriptor(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool propOk = false;
    if (status == napi_ok) {
        (void)napi_switch_ark_context(newEnv);
        napi_value obj = nullptr;
        (void)napi_create_object(newEnv, &obj);
        napi_value val = nullptr;
        (void)napi_create_int32(newEnv, VAL_FORTY_TWO, &val);
        napi_property_descriptor desc = DECLARE_NAPI_PROPERTY("testProp", val);
        napi_status defineStatus = napi_define_properties(newEnv, obj, VAL_ONE, &desc);
        napi_value checkVal = nullptr;
        napi_value key = nullptr;
        (void)napi_create_string_utf8(newEnv, "testProp", NAPI_AUTO_LENGTH, &key);
        (void)napi_get_property(newEnv, obj, key, &checkVal);
        int32_t checkInt = VAL_ZERO;
        (void)napi_get_value_int32(newEnv, checkVal, &checkInt);
        propOk = (defineStatus == napi_ok) && (checkInt == VAL_FORTY_TWO);
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "definePropertiesOk", propOk);
    return result;
}

static napi_value TestContextSymbol(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool symbolOk = false;
    if (status == napi_ok) {
        (void)napi_switch_ark_context(newEnv);
        napi_value description = nullptr;
        (void)napi_create_string_utf8(newEnv, "desc", NAPI_AUTO_LENGTH, &description);
        napi_value symbol = nullptr;
        napi_status symStatus = napi_create_symbol(newEnv, description, &symbol);
        napi_value obj = nullptr;
        (void)napi_create_object(newEnv, &obj);
        napi_value val = nullptr;
        (void)napi_create_int32(newEnv, VAL_HUNDRED, &val);
        (void)napi_set_property(newEnv, obj, symbol, val);
        napi_value getVal = nullptr;
        (void)napi_get_property(newEnv, obj, symbol, &getVal);
        int32_t checkVal = VAL_ZERO;
        (void)napi_get_value_int32(newEnv, getVal, &checkVal);
        symbolOk = (symStatus == napi_ok) && (checkVal == VAL_HUNDRED);
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "symbolOk", symbolOk);
    return result;
}

static napi_value TestContextObjectFreeze(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool freezeOk = false;
    if (status == napi_ok) {
        (void)napi_switch_ark_context(newEnv);
        napi_value obj = nullptr;
        (void)napi_create_object(newEnv, &obj);
        (void)napi_set_named_property(newEnv, result, "frozenObj", obj);
        (void)RunScript(newEnv, "Object.freeze(globalThis.frozenObj);");
        int32_t isFrozen = VAL_ZERO;
        (void)RunScriptGetInt32(newEnv, "Object.isFrozen(globalThis.frozenObj) ? 1 : 0;", &isFrozen);
        freezeOk = (isFrozen == VAL_ONE);
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "freezeOk", freezeOk);
    return result;
}

static void FinalizeCallback(napi_env /* env */, void* finalizeData, void* /* finalizeHint */)
{
    int32_t* p = static_cast<int32_t*>(finalizeData);
    if (p != nullptr) {
        *p = VAL_FORTY_TWO;
    }
}

static napi_value TestContextExternal(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool extOk = false;
    if (status == napi_ok) {
        (void)napi_switch_ark_context(newEnv);
        int32_t data = VAL_ZERO;
        napi_value ext = nullptr;
        napi_status extStatus = napi_create_external(newEnv, &data, FinalizeCallback, nullptr, &ext);
        extOk = (extStatus == napi_ok);
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "externalOk", extOk);
    return result;
}

static napi_value TestContextInstanceof(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool instanceOk = false;
    if (status == napi_ok) {
        (void)napi_switch_ark_context(newEnv);
        (void)RunScript(newEnv, "globalThis.C = class {}; globalThis.o = new globalThis.C();");
        napi_value constructor = nullptr;
        napi_value object = nullptr;
        napi_value global = nullptr;
        (void)napi_get_global(newEnv, &global);
        (void)napi_get_named_property(newEnv, global, "C", &constructor);
        (void)napi_get_named_property(newEnv, global, "o", &object);
        bool isInstance = false;
        napi_status checkStatus = napi_instanceof(newEnv, object, constructor, &isInstance);
        instanceOk = (checkStatus == napi_ok) && isInstance;
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "instanceofOk", instanceOk);
    return result;
}

static napi_value TestContextArrayBasic(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool arrayOk = false;
    if (status == napi_ok) {
        (void)napi_switch_ark_context(newEnv);
        napi_value arr = nullptr;
        napi_status createStatus = napi_create_array_with_length(newEnv, VAL_TEN, &arr);
        bool isArray = false;
        (void)napi_is_array(newEnv, arr, &isArray);
        uint32_t len = VAL_ZERO;
        (void)napi_get_array_length(newEnv, arr, &len);
        arrayOk = (createStatus == napi_ok) && isArray && (len == static_cast<uint32_t>(VAL_TEN));
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "arrayOk", arrayOk);
    return result;
}

static napi_value TestContextDate(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool dateOk = false;
    if (status == napi_ok) {
        (void)napi_switch_ark_context(newEnv);
        napi_value date = nullptr;
        napi_status createStatus = napi_create_date(newEnv, static_cast<double>(VAL_HUNDRED), &date);
        bool isDate = false;
        (void)napi_is_date(newEnv, date, &isDate);
        double val = 0.0;
        (void)napi_get_date_value(newEnv, date, &val);
        dateOk = (createStatus == napi_ok) && isDate && (val == static_cast<double>(VAL_HUNDRED));
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "dateOk", dateOk);
    return result;
}

// 用例 33: 验证在不同的 context 中创建 Promise 并串联执行异步微任务的行为
static napi_value TestContextPromiseMicrotask(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool promiseMicrotaskOk = false;
    if (status == napi_ok) {
        (void)napi_switch_ark_context(newEnv);
        (void)RunScript(newEnv, "var promiseRes = 0; Promise.resolve().then(() => { promiseRes = 42; });");
        int32_t val = VAL_ZERO;
        (void)RunScriptGetInt32(newEnv, "promiseRes;", &val);
        promiseMicrotaskOk = (val == VAL_ZERO);
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "promiseMicrotaskOk", promiseMicrotaskOk);
    return result;
}

// 用例 34: 验证 context 隔离对于内置对象原型污染的防范作用
static napi_value TestContextPrototypePollution(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool pollutionIsolated = false;
    if (status == napi_ok) {
        (void)napi_switch_ark_context(newEnv);
        (void)RunScript(newEnv, "Object.prototype.polluted = 42;");
        int32_t valInNew = VAL_ZERO;
        (void)RunScriptGetInt32(newEnv, "({}).polluted;", &valInNew);
        (void)napi_switch_ark_context(env);
        int32_t valInMain = VAL_ZERO;
        (void)RunScriptGetInt32(env, "({}).polluted;", &valInMain);
        pollutionIsolated = (valInNew == VAL_FORTY_TWO) && (valInMain != VAL_FORTY_TWO);
        (void)napi_switch_ark_context(newEnv);
        (void)RunScript(newEnv, "delete Object.prototype.polluted;");
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "pollutionIsolated", pollutionIsolated);
    return result;
}

// 用例 35: 验证在 newEnv 中创建 TypedArray 及其底层 ArrayBuffer 的属性表现
static napi_value TestContextTypedArray(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool typedArrayOk = false;
    if (status == napi_ok) {
        (void)napi_switch_ark_context(newEnv);
        void* data = nullptr;
        napi_value arrayBuffer = nullptr;
        napi_status bufStatus = napi_create_arraybuffer(newEnv, VAL_HUNDRED, &data, &arrayBuffer);
        napi_value typedArray = nullptr;
        napi_status typedStatus = napi_create_typedarray(
            newEnv, napi_int32_array, VAL_TEN, arrayBuffer, VAL_ZERO, &typedArray);
        bool isTypedArray = false;
        (void)napi_is_typedarray(newEnv, typedArray, &isTypedArray);
        typedArrayOk = (bufStatus == napi_ok) && (typedStatus == napi_ok) && isTypedArray;
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "typedArrayOk", typedArrayOk);
    return result;
}

// 用例 36: 验证在 newEnv 中创建 DataView 及其隔离状态下的基本行为
static napi_value TestContextDataView(napi_env env, napi_callback_info /* info */)
{
    napi_value result = CreateResult(env);
    if (result == nullptr) {
        return nullptr;
    }
    napi_env newEnv = nullptr;
    napi_status status = napi_create_ark_context(env, &newEnv);
    bool dataviewOk = false;
    if (status == napi_ok) {
        (void)napi_switch_ark_context(newEnv);
        void* data = nullptr;
        napi_value arrayBuffer = nullptr;
        napi_status bufStatus = napi_create_arraybuffer(newEnv, VAL_HUNDRED, &data, &arrayBuffer);
        napi_value dataView = nullptr;
        napi_status dataviewStatus = napi_create_dataview(
            newEnv, VAL_HUNDRED, arrayBuffer, VAL_ZERO, &dataView);
        bool isDataView = false;
        (void)napi_is_dataview(newEnv, dataView, &isDataView);
        dataviewOk = (bufStatus == napi_ok) && (dataviewStatus == napi_ok) && isDataView;
        (void)napi_switch_ark_context(env);
        (void)napi_destroy_ark_context(newEnv);
    }
    SetNamedBool(env, result, "dataviewOk", dataviewOk);
    return result;
}

// ============================================================================
// 模块导出与注册，必须使用 __attribute__((constructor)) 方式，并且在 Init 函数中注册所有的用例
// ============================================================================

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("testCreateContextBasic", TestCreateContextBasic),
        DECLARE_NAPI_FUNCTION("testCreateContextNullptr", TestCreateContextNullptr),
        DECLARE_NAPI_FUNCTION("testCreateMultipleContexts", TestCreateMultipleContexts),
        DECLARE_NAPI_FUNCTION("testSwitchContextBasic", TestSwitchContextBasic),
        DECLARE_NAPI_FUNCTION("testSwitchContextNullptr", TestSwitchContextNullptr),
        DECLARE_NAPI_FUNCTION("testDestroyContextNullptr", TestDestroyContextNullptr),
        DECLARE_NAPI_FUNCTION("testDestroyActiveContext", TestDestroyActiveContext),
        DECLARE_NAPI_FUNCTION("testDestroyMainContext", TestDestroyMainContext),
        DECLARE_NAPI_FUNCTION("testContextGlobalIsolation", TestContextGlobalIsolation),
        DECLARE_NAPI_FUNCTION("testContextLifecycleNested", TestContextLifecycleNested),
        DECLARE_NAPI_FUNCTION("testContextSwitchStress", TestContextSwitchStress),
        DECLARE_NAPI_FUNCTION("testContextGlobalLeak", TestContextGlobalLeak),
        DECLARE_NAPI_FUNCTION("testCreateContextUnderException", TestCreateContextUnderException),
        DECLARE_NAPI_FUNCTION("testSwitchContextUnderException", TestSwitchContextUnderException),
        DECLARE_NAPI_FUNCTION("testDestroyContextUnderException", TestDestroyContextUnderException),
        DECLARE_NAPI_FUNCTION("testSwitchToSameContext", TestSwitchToSameContext),
        DECLARE_NAPI_FUNCTION("testContextFunctionSharing", TestContextFunctionSharing),
        DECLARE_NAPI_FUNCTION("testContextScopeBasic", TestContextScopeBasic),
        DECLARE_NAPI_FUNCTION("testContextDefineClass", TestContextDefineClass),
        DECLARE_NAPI_FUNCTION("testContextPromiseBasic", TestContextPromiseBasic),
        DECLARE_NAPI_FUNCTION("testContextCreateArrayBuffer", TestContextCreateArrayBuffer),
        DECLARE_NAPI_FUNCTION("testContextBigInt", TestContextBigInt),
        DECLARE_NAPI_FUNCTION("testContextTypeTag", TestContextTypeTag),
        DECLARE_NAPI_FUNCTION("testContextExceptionInheritance", TestContextExceptionInheritance),
        DECLARE_NAPI_FUNCTION("testContextNestedSwitch", TestContextNestedSwitch),
        DECLARE_NAPI_FUNCTION("testContextPropertyDescriptor", TestContextPropertyDescriptor),
        DECLARE_NAPI_FUNCTION("testContextSymbol", TestContextSymbol),
        DECLARE_NAPI_FUNCTION("testContextObjectFreeze", TestContextObjectFreeze),
        DECLARE_NAPI_FUNCTION("testContextExternal", TestContextExternal),
        DECLARE_NAPI_FUNCTION("testContextInstanceof", TestContextInstanceof),
        DECLARE_NAPI_FUNCTION("testContextArrayBasic", TestContextArrayBasic),
        DECLARE_NAPI_FUNCTION("testContextDate", TestContextDate),
        DECLARE_NAPI_FUNCTION("testContextPromiseMicrotask", TestContextPromiseMicrotask),
        DECLARE_NAPI_FUNCTION("testContextPrototypePollution", TestContextPrototypePollution),
        DECLARE_NAPI_FUNCTION("testContextTypedArray", TestContextTypedArray),
        DECLARE_NAPI_FUNCTION("testContextDataView", TestContextDataView),
    };
    NAPI_CALL(env, napi_define_properties(
        env, exports, sizeof(desc) / sizeof(desc[VAL_ZERO]), desc));
    return exports;
}

static napi_module g_contextModule = {
    .nm_version = VAL_ONE,
    .nm_flags = VAL_ZERO,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "context_suite",
    .nm_priv = nullptr,
    .reserved = { nullptr },
};

extern "C" __attribute__((constructor)) void RegisterModule()
{
    napi_module_register(&g_contextModule);
}
