/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_NAPI_TEST_UNITTEST_TEST_COMMON_H
#define FOUNDATION_ACE_NAPI_TEST_UNITTEST_TEST_COMMON_H
#include "gtest/gtest.h"
#include "utils/log.h"
#define ASSERT_CHECK_CALL(call)   \
    {                             \
        ASSERT_EQ(call, napi_ok); \
    }

#define ASSERT_CHECK_VALUE_TYPE(env, value, type)                   \
    {                                                               \
        do {                                                        \
            napi_valuetype valueType = napi_undefined;              \
            ASSERT_TRUE((value) != nullptr);                        \
            ASSERT_CHECK_CALL(napi_typeof(env, value, &valueType)); \
            ASSERT_EQ(valueType, type);                             \
        } while (0);                                                \
    }

#define GET_UV_EVENT_LOOP(loop, env)                           \
    uv_loop_t*(loop) = nullptr;                                \
    {                                                          \
        auto* engine = reinterpret_cast<NativeEngine*>((env)); \
        if (engine->IsMainEnvContext()) {                      \
            (loop) = engine->GetUVLoop();                      \
        } else {                                               \
            (loop) = engine->GetParent()->GetUVLoop();         \
        }                                                      \
        ASSERT_NE((loop), nullptr);                            \
    }

#define RUN_EVENT_LOOP_WITH_MODE(env, mode) \
    do {                                    \
        GET_UV_EVENT_LOOP(loop, env);       \
        uv_run(loop, (mode));               \
    } while (0)

#define RUN_EVENT_LOOP(env) RUN_EVENT_LOOP_WITH_MODE((env), UV_RUN_DEFAULT)

#define STOP_EVENT_LOOP(env)          \
    do {                              \
        GET_UV_EVENT_LOOP(loop, env); \
        uv_stop(loop);                \
    } while (0)

#define NAPI_THROW_ERROR(env, code, message)                            \
    {                                                                   \
        /* Update exception to engine after throw */                    \
        TryCatch tryCatch((env));                                       \
        EXPECT_EQ(napi_throw_error((env), (code), (message)), napi_ok); \
    }

inline const char* GetTestCaseName()
{
    return testing::UnitTest::GetInstance()->current_test_info()->name();
}

inline napi_value GetNapiTCName(napi_env env)
{
    napi_value value = nullptr;
    napi_create_string_utf8(env, GetTestCaseName(), NAPI_AUTO_LENGTH, &value);
    return value;
}

#endif /* FOUNDATION_ACE_NAPI_TEST_UNITTEST_TEST_COMMON_H */
