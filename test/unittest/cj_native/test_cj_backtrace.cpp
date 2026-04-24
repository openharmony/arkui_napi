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

#include <gtest/gtest.h>

#include "cj_backtrace.h"

#include <thread>

using namespace testing;
using namespace testing::ext;

class TestCJBacktrace : public testing::Test {};

TEST_F(TestCJBacktrace, Positive)
{
    EXPECT_FALSE(CJDFX_HasBacktrace());
    EXPECT_TRUE(CJDFX_Backtrace(1));
    EXPECT_TRUE(CJDFX_HasBacktrace());
    std::string trace;
    EXPECT_TRUE(CJDFX_GetHybridStack(&trace, nullptr));
    EXPECT_FALSE(trace.empty());
    EXPECT_FALSE(CJDFX_HasBacktrace());
    EXPECT_TRUE(CJDFX_Backtrace(1));
    [] {
        CJDFX_IgnoreNextBacktrace();
        EXPECT_TRUE(CJDFX_Backtrace(1));
    }();
    std::string other;
    EXPECT_TRUE(CJDFX_GetHybridStack(&other, nullptr));
    EXPECT_EQ(other, trace);
}

TEST_F(TestCJBacktrace, Negative)
{
    std::thread t([] {
        EXPECT_FALSE(CJDFX_HasBacktrace());
        EXPECT_FALSE(CJDFX_Backtrace(0));
        CJDFX_IgnoreNextBacktrace();
        std::string trace;
        EXPECT_FALSE(CJDFX_GetHybridStack(&trace, nullptr));
    });
    t.join();
    EXPECT_TRUE(CJDFX_Backtrace(0));
    EXPECT_FALSE(CJDFX_GetHybridStack(nullptr, nullptr));
}