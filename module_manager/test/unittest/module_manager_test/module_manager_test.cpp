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

constexpr char MODULE_NS[] = "moduleNs_";

/*
 * @tc.name: LoadNativeModuleTest_001
 * @tc.desc: test NativeModule's LoadNativeModule function
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, LoadNativeModuleTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_001 starts";

    std::string errInfo = "";
    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
    NativeModule *module = moduleManager->LoadNativeModule(nullptr, nullptr, false, errInfo, false, nullptr);
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

    std::string errInfo = "";
    module = moduleManager->LoadNativeModule(moduleName, nullptr, false, errInfo, false, "");
    EXPECT_EQ(module, &mockModule);

    module = moduleManager->LoadNativeModule(moduleName, nullptr, false, errInfo, false);
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

    std::string errInfo = "";
    module = moduleManager->LoadNativeModule(moduleName, nullptr, false, errInfo, false, "");
    EXPECT_EQ(module, nullptr);

    module = moduleManager->LoadNativeModule(moduleName, nullptr, false, errInfo, false);
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

    std::string errInfo = "";
    module = moduleManager->LoadNativeModule(moduleName, nullptr, false, errInfo, false, relativePath);
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

    std::string errInfo = "";
    NativeModule *module = moduleManager->LoadNativeModule(moduleName, nullptr,
                                                           false, errInfo, false, relativePath);
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

    std::string moduleKey = "this is moduleKey";
    const char *moduleName = "moduleName_006";
    NativeEngine* engine = nullptr;
    NativeModuleManager moduleManager;
    moduleManager.SetNativeEngine(moduleKey, engine);

    MockFindNativeModuleByCache(nullptr);
    std::string result = moduleManager.GetModuleFileName(moduleName, true);
    EXPECT_TRUE(result.empty());
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
    std::string result = moduleManager.GetModuleFileName(moduleName, true);
    EXPECT_FALSE(result.empty());
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
    std::string nsName;
    bool res = moduleManager->GetLdNamespaceName(moduleName, nsName);
    EXPECT_EQ(res, false);
    moduleManager->CreateLdNamespace(moduleName, libPath, true);
    res = moduleManager->GetLdNamespaceName(moduleName, nsName);
    EXPECT_EQ(res, true);
    EXPECT_STREQ(nsName.c_str(), (MODULE_NS + moduleName).c_str());
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

    std::string nsName;
    bool res = moduleManager->GetLdNamespaceName(moduleName, nsName);
    EXPECT_EQ(res, false);
    moduleManager->CreateLdNamespace(moduleName, libPath, false);
    res = moduleManager->GetLdNamespaceName(moduleName, nsName);
    EXPECT_EQ(res, true);
    EXPECT_STREQ(nsName.c_str(), (MODULE_NS + moduleName).c_str());
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
    char nativeModulePath[3][4096];
    nativeModulePath[0][0] = 0;
    nativeModulePath[1][0] = 0;
    nativeModulePath[2][0] = 0;

    std::string errInfo = "";
    EXPECT_EQ(moduleManager->FindNativeModuleByDisk(moduleName, nullptr, nullptr, false, false, errInfo,
        nativeModulePath, nullptr), nullptr);
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_011 end";
}

/*
 * @tc.name: LoadNativeModuleTest_012
 * @tc.desc: test NativeModule's EmplaceModuleLib function
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, LoadNativeModuleTest_012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_012 starts";
    std::shared_ptr<NativeModuleManager> moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    std::string moduleKey = "aa";
    moduleManager->EmplaceModuleLib(moduleKey, nullptr);
    bool result1 = moduleManager->RemoveModuleLib(moduleKey);
    std::string moduleKey1 = "bb";
    bool result2 = moduleManager->RemoveModuleLib(moduleKey1);
    EXPECT_EQ(result1, false);
    EXPECT_EQ(result2, false);
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_012 end";
}

/*
 * @tc.name: LoadNativeModuleTest_013
 * @tc.desc: test NativeModule's RemoveNativeModule function
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, LoadNativeModuleTest_013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_013 starts";
    std::shared_ptr<NativeModuleManager> moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    std::string moduleKey = "aa";
    moduleManager->EmplaceModuleLib(moduleKey, nullptr);
    std::string moduleKey1 = "bb";
    EXPECT_EQ(moduleManager->GetNativeModuleHandle(moduleKey1), nullptr);
    bool result2 = moduleManager->UnloadNativeModule(moduleKey1);
    EXPECT_EQ(result2, false);
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_013 end";
}

/*
 * @tc.name: LoadNativeModuleTest_014
 * @tc.desc: test NativeModule's RemoveNativeModule function
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, LoadNativeModuleTest_014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_014 starts";
    std::shared_ptr<NativeModuleManager> moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    std::string moduleKey = "aa";
    moduleManager->EmplaceModuleLib(moduleKey, nullptr);
    std::string moduleKey1 = "bb";
    bool result = moduleManager->RemoveModuleLib(moduleKey1);
    EXPECT_EQ(result, false);

    bool result2 = moduleManager->RemoveNativeModule(moduleKey1);
    EXPECT_EQ(result2, false);
    bool result3 = moduleManager->UnloadNativeModule(moduleKey1);
    EXPECT_EQ(result3, false);
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_014 end";
}

/*
 * @tc.name: LoadNativeModuleTest_015
 * @tc.desc: test NativeModule's UnloadNativeModule function
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, LoadNativeModuleTest_015, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_015 starts";
    std::shared_ptr<NativeModuleManager> moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    std::string moduleKey = "aa";
    moduleManager->EmplaceModuleLib(moduleKey, nullptr);
    bool result = moduleManager->UnloadNativeModule(moduleKey);
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_015 end";
}

/*
 * @tc.name: LoadNativeModuleTest_016
 * @tc.desc: test NativeModule's UnloadModuleLibrary function
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, LoadNativeModuleTest_016, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_016 starts";
    std::shared_ptr<NativeModuleManager> moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    bool result = moduleManager->UnloadModuleLibrary(nullptr);
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_016 end";
}

/*
 * @tc.name: LoadNativeModuleTest_017
 * @tc.desc: test NativeModule's RemoveNativeModuleByCache function
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, LoadNativeModuleTest_017, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_017 starts";
    std::shared_ptr<NativeModuleManager> moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    std::string moduleKey = "aa";
    bool result = moduleManager->RemoveNativeModuleByCache(moduleKey);
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_017 end";
}

/*
 * @tc.name: LoadNativeModuleTest_018
 * @tc.desc: test NativeModule's LoadNativeModule function
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, LoadNativeModuleTest_018, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_018 starts";

    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();

    std::string errInfo = "";
    /* isModuleRestricted is true and isAppModule is false, we will check the restriction */
    EXPECT_EQ(moduleManager->CheckModuleRestricted("dummy"), false);

    EXPECT_EQ(moduleManager->CheckModuleRestricted("worker"), true);

    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_018 end";
}

/*
 * @tc.name: LoadNativeModuleTest_019
 * @tc.desc: test NativeModule's LoadNativeModule function
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, LoadNativeModuleTest_019, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_019 starts";

    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();

    /* isModuleRestricted is true and isAppModule is false, we will check the restriction */
    EXPECT_EQ(moduleManager->CheckModuleRestricted("dummy"), false);

    EXPECT_EQ(moduleManager->CheckModuleRestricted("worker"), true);

    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_019 end";
}
