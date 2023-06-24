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
#include "module_load_checker.h"

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
    moduleManager->Register(module);

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

/*
 * @tc.name: LoadNativeModuleTest_006
 * @tc.desc: test NativeModule's SetNativeEngine function
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, LoadNativeModuleTest_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_006 starts";
    const char *moduleName = "moduleName_006";
    NativeEngine* engine = nullptr;
    NativeModuleManager moduleManager;
    moduleManager.SetNativeEngine(moduleName, engine);

    MockFindNativeModuleByCache(nullptr);
    const char* result = moduleManager.GetModuleFileName(moduleName, true);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_006 end";
}

/*
 * @tc.name: LoadNativeModuleTest_007
 * @tc.desc: test NativeModule's GetModuleFileName function
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, LoadNativeModuleTest_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_007 starts";
    const char *moduleName = "moduleName_007";
    NativeModule mockModule;
    NativeModuleManager moduleManager;

    MockFindNativeModuleByCache(&mockModule);
    const char* result = moduleManager.GetModuleFileName(moduleName, true);
    EXPECT_TRUE(result != nullptr);
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_007 end";
}

/*
 * @tc.name: LoadNativeModuleTest_008
 * @tc.desc: test NativeModule's Register function
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, LoadNativeModuleTest_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_008 starts";
    std::string moduleName = "moduleName_008";
    const char* libPath = nullptr;
    std::shared_ptr<NativeModuleManager> moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    moduleManager->Register(nullptr);
    moduleManager->CreateSharedLibsSonames();
    moduleManager->CreateLdNamespace(moduleName, libPath, true);
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_008 end";
}

/*
 * @tc.name: LoadNativeModuleTest_009
 * @tc.desc: test NativeModule's CreateLdNamespace function
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, LoadNativeModuleTest_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_009 starts";
    std::string moduleName = "moduleName_009";
    const char* libPath = nullptr;
    std::shared_ptr<NativeModuleManager> moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    moduleManager->CreateLdNamespace(moduleName, libPath, false);
    std::vector<std::string> appLibPath;
    moduleManager->SetAppLibPath(moduleName, appLibPath, false);
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_009 end";
}

/*
 * @tc.name: LoadNativeModuleTest_010
 * @tc.desc: test NativeModule's SetAppLibPath function
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, LoadNativeModuleTest_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_010 starts";
    std::string moduleName = "moduleName_010";
    std::shared_ptr<NativeModuleManager> moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    std::vector<std::string> appLibPath1 = { "0", "1", "2" };
    moduleManager->SetAppLibPath(moduleName, appLibPath1, false);
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_010 end";
}

/*
 * @tc.name: LoadNativeModuleTest_011
 * @tc.desc: test NativeModule's FindNativeModuleByDisk function
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, LoadNativeModuleTest_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_011 starts";
    const char* moduleName = "moduleName_010";
    std::shared_ptr<NativeModuleManager> moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);
    MockFindNativeModuleByDisk(nullptr);

    EXPECT_EQ(moduleManager->FindNativeModuleByDisk(moduleName, nullptr, nullptr, false, false), nullptr);
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_011 end";
}
