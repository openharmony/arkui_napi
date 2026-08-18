/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"
#include "napi/native_api.h"
#include "native_utils.h"
#define private public
#define protected public
#include "test.h"
#undef private
#include "test_common.h"
#include "utils/log.h"

/**
 * @tc.name: ArkHookListTest001
 * @tc.desc: GetHookModule returns the original module name when no hook list is registered.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, ArkHookListTest001, testing::ext::TestSize.Level0)
{
    NativeEngineProxy engine;
    ASSERT_NE(*engine, nullptr);
    EXPECT_EQ(engine->GetHookModule("moduleA"), "moduleA");
    EXPECT_EQ(engine->GetHookModule("moduleB"), "moduleB");
    EXPECT_EQ(engine->GetHookModule(""), "");
}

/**
 * @tc.name: ArkHookListTest002
 * @tc.desc: SetHookList with an empty string does not register any mapping.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, ArkHookListTest002, testing::ext::TestSize.Level0)
{
    NativeEngineProxy engine;
    ASSERT_NE(*engine, nullptr);
    engine->SetHookList("");
    EXPECT_EQ(engine->GetHookModule("moduleA"), "moduleA");
    EXPECT_TRUE(engine->hookList_.empty());
}

/**
 * @tc.name: ArkHookListTest003
 * @tc.desc: SetHookList parses a single key:value pair correctly.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, ArkHookListTest003, testing::ext::TestSize.Level0)
{
    NativeEngineProxy engine;
    ASSERT_NE(*engine, nullptr);
    engine->SetHookList("moduleA:hookA");
    EXPECT_EQ(engine->GetHookModule("moduleA"), "hookA");
    EXPECT_EQ(engine->GetHookModule("moduleB"), "moduleB");
    EXPECT_EQ(engine->hookList_.size(), 1u);
}

/**
 * @tc.name: ArkHookListTest004
 * @tc.desc: SetHookList parses multiple pairs separated by ';'.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, ArkHookListTest004, testing::ext::TestSize.Level0)
{
    NativeEngineProxy engine;
    ASSERT_NE(*engine, nullptr);
    engine->SetHookList("moduleA:hookA;moduleB:hookB;moduleC:hookC");
    EXPECT_EQ(engine->GetHookModule("moduleA"), "hookA");
    EXPECT_EQ(engine->GetHookModule("moduleB"), "hookB");
    EXPECT_EQ(engine->GetHookModule("moduleC"), "hookC");
    EXPECT_EQ(engine->hookList_.size(), 3u);
}

/**
 * @tc.name: ArkHookListTest005
 * @tc.desc: SetHookList skips tokens that do not contain a colon.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, ArkHookListTest005, testing::ext::TestSize.Level0)
{
    NativeEngineProxy engine;
    ASSERT_NE(*engine, nullptr);
    engine->SetHookList("noColonToken;moduleA:hookA;anotherToken");
    EXPECT_EQ(engine->GetHookModule("noColonToken"), "noColonToken");
    EXPECT_EQ(engine->GetHookModule("anotherToken"), "anotherToken");
    EXPECT_EQ(engine->GetHookModule("moduleA"), "hookA");
    EXPECT_EQ(engine->hookList_.size(), 1u);
}

/**
 * @tc.name: ArkHookListTest006
 * @tc.desc: SetHookList tolerates empty tokens produced by leading/trailing/consecutive delimiters.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, ArkHookListTest006, testing::ext::TestSize.Level0)
{
    {
        NativeEngineProxy engine;
        ASSERT_NE(*engine, nullptr);
        engine->SetHookList("moduleA:hookA;");
        EXPECT_EQ(engine->GetHookModule("moduleA"), "hookA");
        EXPECT_EQ(engine->hookList_.size(), 1u);
    }
    {
        NativeEngineProxy engine;
        ASSERT_NE(*engine, nullptr);
        engine->SetHookList(";moduleA:hookA");
        EXPECT_EQ(engine->GetHookModule("moduleA"), "hookA");
        EXPECT_EQ(engine->hookList_.size(), 1u);
    }
    {
        NativeEngineProxy engine;
        ASSERT_NE(*engine, nullptr);
        engine->SetHookList("moduleA:hookA;;moduleB:hookB");
        EXPECT_EQ(engine->GetHookModule("moduleA"), "hookA");
        EXPECT_EQ(engine->GetHookModule("moduleB"), "hookB");
        EXPECT_EQ(engine->hookList_.size(), 2u);
    }
    {
        NativeEngineProxy engine;
        ASSERT_NE(*engine, nullptr);
        engine->SetHookList(";");
        EXPECT_TRUE(engine->hookList_.empty());
        EXPECT_EQ(engine->GetHookModule("moduleA"), "moduleA");
    }
}

/**
 * @tc.name: ArkHookListTest007
 * @tc.desc: SetHookList overwrites the value when a key appears more than once (last wins).
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, ArkHookListTest007, testing::ext::TestSize.Level0)
{
    NativeEngineProxy engine;
    ASSERT_NE(*engine, nullptr);
    engine->SetHookList("moduleA:hookA;moduleA:hookB");
    EXPECT_EQ(engine->GetHookModule("moduleA"), "hookB");
    EXPECT_EQ(engine->hookList_.size(), 1u);
}

/**
 * @tc.name: ArkHookListTest008
 * @tc.desc: SetHookList splits only on the first colon, allowing the value to contain colons.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, ArkHookListTest008, testing::ext::TestSize.Level0)
{
    NativeEngineProxy engine;
    ASSERT_NE(*engine, nullptr);
    engine->SetHookList("moduleA:hookA:extra:part");
    EXPECT_EQ(engine->GetHookModule("moduleA"), "hookA:extra:part");
    EXPECT_EQ(engine->hookList_["moduleA"], "hookA:extra:part");
}

/**
 * @tc.name: ArkHookListTest009
 * @tc.desc: SetHookList handles empty key and empty value edge cases.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, ArkHookListTest009, testing::ext::TestSize.Level0)
{
    NativeEngineProxy engine;
    ASSERT_NE(*engine, nullptr);
    engine->SetHookList("moduleA:;:hookA");
    ASSERT_EQ(engine->hookList_.size(), 2u);
    auto iterEmptyValue = engine->hookList_.find("moduleA");
    ASSERT_NE(iterEmptyValue, engine->hookList_.end());
    EXPECT_EQ(iterEmptyValue->second, "");
    EXPECT_EQ(engine->GetHookModule("moduleA"), "");
    auto iterEmptyKey = engine->hookList_.find("");
    ASSERT_NE(iterEmptyKey, engine->hookList_.end());
    EXPECT_EQ(iterEmptyKey->second, "hookA");
    EXPECT_EQ(engine->GetHookModule(""), "hookA");
}

/**
 * @tc.name: ArkHookListTest010
 * @tc.desc: Multiple SetHookList calls accumulate mappings on the same engine.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, ArkHookListTest010, testing::ext::TestSize.Level0)
{
    NativeEngineProxy engine;
    ASSERT_NE(*engine, nullptr);
    engine->SetHookList("moduleA:hookA");
    engine->SetHookList("moduleB:hookB");
    EXPECT_EQ(engine->GetHookModule("moduleA"), "hookA");
    EXPECT_EQ(engine->GetHookModule("moduleB"), "hookB");
    EXPECT_EQ(engine->hookList_.size(), 2u);
    engine->SetHookList("moduleA:hookUpdated");
    EXPECT_EQ(engine->GetHookModule("moduleA"), "hookUpdated");
    EXPECT_EQ(engine->hookList_.size(), 2u);
}
