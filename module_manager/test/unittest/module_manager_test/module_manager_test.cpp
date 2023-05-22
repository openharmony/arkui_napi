/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "mock_native_module_manager.h"

using namespace testing::ext;

class ModuleManagerTest : public testing::Test {
public:
    ModuleManagerTest() {}

    virtual ~ModuleManagerTest() {}

    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp();

    void TearDown();
};

void ModuleManagerTest::SetUpTestCase() {}

void ModuleManagerTest::TearDownTestCase() {}

void ModuleManagerTest::SetUp()
{
    MockResetModuleManagerState();
}

void ModuleManagerTest::TearDown()
{
    MockResetModuleManagerState();
}

/*
 * @tc.name: LoadNativeModuleTest_001
 * @tc.desc: test NativeModule's LoadNativeModule function
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, LoadNativeModuleTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_001 starts";

    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();

    NativeModule *module = moduleManager->LoadNativeModule(nullptr, nullptr, false, false, nullptr);
    EXPECT_EQ(module, nullptr);

    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_001 ends";
}

/*
 * @tc.name: LoadNativeModuleTest_002
 * @tc.desc: test NativeModule's LoadNativeModule function
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, LoadNativeModuleTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_002 starts";

    const char *moduleName = "moduleName_002";
    NativeModule mockModule;
    NativeModule *module;
    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();

    MockFindNativeModuleByDisk(&mockModule);

    module = moduleManager->LoadNativeModule(moduleName, nullptr, false, false, "");
    EXPECT_EQ(module, &mockModule);

    module = moduleManager->LoadNativeModule(moduleName, nullptr, false, false);
    EXPECT_EQ(module, &mockModule);

    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_002 end";
}

/*
 * @tc.name: LoadNativeModuleTest_003
 * @tc.desc: test NativeModule's LoadNativeModule function
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, LoadNativeModuleTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_003 starts";

    const char *moduleName = "moduleName_003";
    NativeModule *module;
    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();

    module = moduleManager->LoadNativeModule(moduleName, nullptr, false, false, "");
    EXPECT_EQ(module, nullptr);

    module = moduleManager->LoadNativeModule(moduleName, nullptr, false, false);
    EXPECT_EQ(module, nullptr);

    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_003 end";
}

/*
 * @tc.name: LoadNativeModuleTest_004
 * @tc.desc: test NativeModule's LoadNativeModule function
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, LoadNativeModuleTest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_004 starts";

    const char *moduleName = "moduleName_004";
    const char *relativePath = "relativePath_004";
    NativeModule mockModule;
    NativeModule *module;
    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();

    MockFindNativeModuleByDisk(&mockModule);

    module = moduleManager->LoadNativeModule(moduleName, nullptr, false, false, relativePath);
    EXPECT_EQ(module, &mockModule);

    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_004 end";
}

/*
 * @tc.name: LoadNativeModuleTest_005
 * @tc.desc: test NativeModule's LoadNativeModule function
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, LoadNativeModuleTest_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_005 starts";

    const char *moduleName = "moduleName_005";
    const char *relativePath = "errorPath_005";

    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();

    MockCheckModuleLoadable(true);
    MockLoadModuleLibrary(nullptr);

    NativeModule *module = moduleManager->LoadNativeModule(moduleName, nullptr,
                                                           false, false, relativePath);
    EXPECT_EQ(module, nullptr);

    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_005 end";
}
