/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <ctime>
#include <sys/time.h>

#include "gtest/gtest.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "native_engine.h"
#include "native_engine/impl/ark/ark_native_engine.h"

using panda::RuntimeOption;

static constexpr int NUM_COUNT = 10000;
static constexpr int TIME_UNIT = 1000000;
time_t g_timeFor = 0;
struct timeval g_beginTime;
struct timeval g_endTime;
time_t g_time1 = 0;
time_t g_time2 = 0;
time_t g_time = 0;

#define TEST_TIME(NAME)                                                                  \
    {                                                                                    \
        g_time1 = (g_beginTime.tv_sec * TIME_UNIT) + (g_beginTime.tv_usec);              \
        g_time2 = (g_endTime.tv_sec * TIME_UNIT) + (g_endTime.tv_usec);                  \
        g_time = g_time2 - g_time1;                                                      \
        GTEST_LOG_(INFO) << "name =" << #NAME << " = Time =" << int(g_time - g_timeFor); \
    }

class ArkNapiPerfomanceTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "TearDownCase";
    }

    void SetUp() override
    {
        CreateEngine(vm_, nativeEngine_);
    }

    void TearDown() override
    {
        nativeEngine_->Loop(LOOP_NOWAIT);
        DestroyEngine(vm_, nativeEngine_);
    }

    void CreateEngine(EcmaVM*& vm, NativeEngine*& engine)
    {
        // Ignite the ark engine
        RuntimeOption option;
        option.SetGcType(RuntimeOption::GC_TYPE::GEN_GC);
        const int64_t poolSize = 0x1000000;  // 16M
        option.SetGcPoolSize(poolSize);
        option.SetLogLevel(RuntimeOption::LOG_LEVEL::ERROR);
        option.SetDebuggerLibraryPath("");

        vm = panda::JSNApi::CreateJSVM(option);
        engine = new ArkNativeEngine(vm, nullptr);
    }

    void DestroyEngine(EcmaVM*& vm, NativeEngine*& engine)
    {
        if (engine != nullptr) {
            delete engine;
            engine = nullptr;
        }
        if (vm != nullptr) {
            panda::JSNApi::DestroyJSVM(vm);
            vm = nullptr;
        }
    }

    EcmaVM* vm_ {nullptr};
    NativeEngine* nativeEngine_ {nullptr};
};

HWTEST_F(ArkNapiPerfomanceTest, GetBoolean001, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;
    napi_value result = nullptr;
    napi_get_boolean(env, true, &result);

    bool resultValue = false;
    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_get_value_bool(env, result, &resultValue);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_get_value_bool);
}

HWTEST_F(ArkNapiPerfomanceTest, GetValueDouble001, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;
    napi_value result = nullptr;
    napi_create_double(env, 20.22, &result); // test number

    double resultValue = 0;
    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_get_value_double(env, result, &resultValue);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_get_value_double);
}

HWTEST_F(ArkNapiPerfomanceTest, GetValueInt32, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;
    napi_value result = nullptr;
    napi_create_int32(env, 20, &result); // test number

    int32_t resultValue = 0;
    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_get_value_int32(env, result, &resultValue);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_get_value_int32);
}

HWTEST_F(ArkNapiPerfomanceTest, GetValueUint32, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;
    napi_value result = nullptr;
    napi_create_uint32(env, 20, &result); // test number

    uint32_t resultValue = 0;
    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_get_value_uint32(env, result, &resultValue);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_get_value_uint32);
}

HWTEST_F(ArkNapiPerfomanceTest, GetValueInt64, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;
    napi_value result = nullptr;
    napi_create_int64(env, 200, &result); // test number

    int64_t resultValue = 0;
    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_get_value_int64(env, result, &resultValue);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_get_value_int64);
}

HWTEST_F(ArkNapiPerfomanceTest, GetValueStringUtf8, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;
    const char testStr[] = "ut.utf8test.napi.!@#$%^&*123";
    size_t testStrLength = strlen(testStr);
    size_t copied = 0;
    napi_value result = nullptr;
    napi_create_string_utf8(env, testStr, testStrLength, &result);

    char *buffer = new char[testStrLength + 1]{ 0 };
    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_get_value_string_utf8(env, result, buffer, testStrLength + 1, &copied);
    }
    gettimeofday(&g_endTime, nullptr);

    delete []buffer;
    buffer = nullptr;
    TEST_TIME(napi_get_value_string_utf8);
}

HWTEST_F(ArkNapiPerfomanceTest, GetNull, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;
    napi_value result = nullptr;

    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_get_null(env, &result);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_get_null);
}

HWTEST_F(ArkNapiPerfomanceTest, GetUndefined, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;
    napi_value result = nullptr;

    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_get_undefined(env, &result);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_get_null);
}

HWTEST_F(ArkNapiPerfomanceTest, CreateObject, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;
    napi_value object = nullptr;

    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_create_object(env, &object);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_create_object);
}

HWTEST_F(ArkNapiPerfomanceTest, CreateInt32, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;
    napi_value res = nullptr;

    int32_t num = 123; // test number
    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_create_int32(env, num, &res);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_create_int32);
}

HWTEST_F(ArkNapiPerfomanceTest, CreateInt64, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;
    napi_value res = nullptr;

    int64_t num = 123456; // test number
    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_create_int64(env, num, &res);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_create_int32);
}

HWTEST_F(ArkNapiPerfomanceTest, CreateUint32, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;
    napi_value res = nullptr;

    uint32_t num = 123; // test number
    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_create_uint32(env, num, &res);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_create_uint32);
}

HWTEST_F(ArkNapiPerfomanceTest, CreateStringUtf8, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;
    const char testStr[] = "ut.utf8test.napi.!@#$%^&*123";
    size_t testStrLength = strlen(testStr);
    napi_value result = nullptr;

    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_create_string_utf8(env, testStr, testStrLength, &result);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_create_string_utf8);
}

HWTEST_F(ArkNapiPerfomanceTest, CreateDouble, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;
    napi_value res = nullptr;

    double num = 123.001; // test number
    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_create_double(env, num, &res);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_create_double);
}

HWTEST_F(ArkNapiPerfomanceTest, CreateStringLatin1, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;

    napi_value messageKey = nullptr;
    const char* messageKeyStr = "message";
    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_create_string_latin1(env, messageKeyStr, strlen(messageKeyStr), &messageKey);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_create_string_latin1);
}

HWTEST_F(ArkNapiPerfomanceTest, CreateArray001, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;

    napi_value array = nullptr;
    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_create_array(env, &array);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_create_array);
}

HWTEST_F(ArkNapiPerfomanceTest, CreateArrayWithLength001, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;

    napi_value array = nullptr;
    size_t length = 8; // test number
    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_create_array_with_length(env, length, &array);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_create_array_with_length);
}

HWTEST_F(ArkNapiPerfomanceTest, CreateSymbol, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;

    const char* testStr1 = "testSymbol";
    napi_value result1 = nullptr;
    napi_create_string_latin1(env, testStr1, strlen(testStr1), &result1);
    napi_value symbolVal = nullptr;

    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_create_symbol(env, result1, &symbolVal);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_create_symbol);
}

napi_value SayHello(napi_env env, napi_callback_info info)
{
    return NULL;
}

HWTEST_F(ArkNapiPerfomanceTest, CreateFunction, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;

    napi_value fn = nullptr;
    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_create_function(env, NULL, 0, SayHello, NULL, &fn);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_create_function);
}

HWTEST_F(ArkNapiPerfomanceTest, CreateError, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;

    napi_value code = nullptr;
    napi_value message = nullptr;
    napi_create_string_latin1(env, "500", NAPI_AUTO_LENGTH, &code);
    napi_create_string_latin1(env, "common error", NAPI_AUTO_LENGTH, &message);
    napi_value error = nullptr;

    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_create_error(env, code, message, &error);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_create_error);
}

HWTEST_F(ArkNapiPerfomanceTest, CreateTypeError, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;

    napi_value code1 = nullptr;
    napi_value message1 = nullptr;
    napi_create_string_latin1(env, "500", NAPI_AUTO_LENGTH, &code1);
    napi_create_string_latin1(env, "type error", NAPI_AUTO_LENGTH, &message1);
    napi_value error1 = nullptr;

    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_create_type_error(env, code1, message1, &error1);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_create_type_error);
}

HWTEST_F(ArkNapiPerfomanceTest, CreateRangeError, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;

    napi_value code2 = nullptr;
    napi_value message2 = nullptr;
    napi_create_string_latin1(env, "500", NAPI_AUTO_LENGTH, &code2);
    napi_create_string_latin1(env, "range error", NAPI_AUTO_LENGTH, &message2);
    napi_value error2 = nullptr;

    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_create_range_error(env, code2, message2, &error2);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_create_range_error);
}

HWTEST_F(ArkNapiPerfomanceTest, CoerceToBool, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;

    bool val = false;
    napi_value res = nullptr;
    napi_value result = nullptr;
    napi_get_boolean(env, val, &res);

    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_coerce_to_bool(env, res, &result);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_coerce_to_bool);
}

HWTEST_F(ArkNapiPerfomanceTest, CoerceToNumber, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;

    double num1 = 10.3; // test number
    napi_value res1 = nullptr;
    napi_value result1 = nullptr;
    napi_create_double(env, num1, &res1);

    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_coerce_to_number(env, res1, &result1);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_coerce_to_number);
}

HWTEST_F(ArkNapiPerfomanceTest, CoerceToObject, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;

    napi_value obj = nullptr;
    napi_value res2 = nullptr;
    napi_create_object(env, &obj);

    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_coerce_to_object(env, obj, &res2);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_coerce_to_object);
}

HWTEST_F(ArkNapiPerfomanceTest, CoerceToString, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;

    const char* testStringStr = "test";
    napi_value testString = nullptr;
    napi_create_string_utf8(env, testStringStr, strlen(testStringStr), &testString);
    napi_value res3 = nullptr;

    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_coerce_to_string(env, testString, &res3);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_coerce_to_string);
}

HWTEST_F(ArkNapiPerfomanceTest, SetProperty, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;

    napi_value res4 = nullptr;
    napi_create_object(env, &res4);
    napi_value messageKey = nullptr;
    const char* messageKeyStr = "message";
    napi_create_string_latin1(env, messageKeyStr, strlen(messageKeyStr), &messageKey);
    napi_value messageValue = nullptr;
    const char* messageValueStr = "ok";
    napi_create_string_latin1(env, messageValueStr, strlen(messageValueStr), &messageValue);

    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_set_property(env, res4, messageKey, messageValue);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_set_property);
}

HWTEST_F(ArkNapiPerfomanceTest, GetPrototype, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;

    napi_value res4 = nullptr;
    napi_create_object(env, &res4);
    napi_value res5 = nullptr;

    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_get_prototype(env, res4, &res5);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_get_prototype);
}

HWTEST_F(ArkNapiPerfomanceTest, HasProperty, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;

    napi_value res4 = nullptr;
    napi_create_object(env, &res4);
    napi_value messageKey = nullptr;
    const char* messageKeyStr = "message";
    napi_create_string_latin1(env, messageKeyStr, strlen(messageKeyStr), &messageKey);
    napi_value messageValue = nullptr;
    const char* messageValueStr = "ok";
    napi_create_string_latin1(env, messageValueStr, strlen(messageValueStr), &messageValue);
    bool hasProperty = false;

    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_has_property(env, res4, messageKey, &hasProperty);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_has_property);
}

HWTEST_F(ArkNapiPerfomanceTest, GetProperty, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;

    napi_value res4 = nullptr;
    napi_create_object(env, &res4);
    napi_value messageKey = nullptr;
    const char* messageKeyStr = "message";
    napi_create_string_latin1(env, messageKeyStr, strlen(messageKeyStr), &messageKey);
    napi_value messageValue = nullptr;
    const char* messageValueStr = "ok";
    napi_create_string_latin1(env, messageValueStr, strlen(messageValueStr), &messageValue);
    napi_value res6 = nullptr;

    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_get_property(env, res4, messageKey, &res6);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_get_property);
}

HWTEST_F(ArkNapiPerfomanceTest, HasOwnProperty, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;

    napi_value res4 = nullptr;
    napi_create_object(env, &res4);
    napi_value messageKey = nullptr;
    const char* messageKeyStr = "message";
    napi_create_string_latin1(env, messageKeyStr, strlen(messageKeyStr), &messageKey);
    napi_value messageValue = nullptr;
    const char* messageValueStr = "ok";
    napi_create_string_latin1(env, messageValueStr, strlen(messageValueStr), &messageValue);
    bool res7 = false;

    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_has_own_property(env, res4, messageKey, &res7);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_has_own_property);
}

HWTEST_F(ArkNapiPerfomanceTest, DeleteProperty, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;

    napi_value res4 = nullptr;
    napi_create_object(env, &res4);
    napi_value messageKey = nullptr;
    const char* messageKeyStr = "message";
    napi_create_string_latin1(env, messageKeyStr, strlen(messageKeyStr), &messageKey);
    napi_value messageValue = nullptr;
    const char* messageValueStr = "ok";
    napi_create_string_latin1(env, messageValueStr, strlen(messageValueStr), &messageValue);
    bool res8 = false;

    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_delete_property(env, res4, messageKey, &res8);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_delete_property);
}

HWTEST_F(ArkNapiPerfomanceTest, SetNameProperty, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)nativeEngine_;

    napi_value res9 = nullptr;
    napi_create_object(env, &res9);
    const char testStr[] = "1234567";
    napi_value strAttribute = nullptr;
    napi_create_string_utf8(env, testStr, strlen(testStr), &strAttribute);

    gettimeofday(&g_beginTime, nullptr);
    for (int i = 0; i < NUM_COUNT; i++) {
        napi_set_named_property(env, res9, "strAttribute", strAttribute);
    }
    gettimeofday(&g_endTime, nullptr);
    TEST_TIME(napi_set_named_property);
}
