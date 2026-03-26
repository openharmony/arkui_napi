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

#include <string>

#include "dlsym_mock_guard.h"
#include "mock_native_module_manager.h"
#include "module_load_checker.h"

using namespace testing::ext;

namespace {
    constexpr static int32_t NATIVE_PATH_NUMBER = 3;
    constexpr static int32_t IS_APP_MODULE_FLAGS = 100;
    constexpr char GREYLIST_CONFIG_PATH_LT[] =
        "/data/service/el0/public/musl_namespace_config/greylist.json";
};

class ModuleManagerTest : public testing::Test {
public:
    ModuleManagerTest() {}

    virtual ~ModuleManagerTest() {}

    static void SetUpTestCase();

    static void TearDownTestCase();

    void InitNativeModule(NativeModule* nativeModule, std::string moduleName);
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

void ModuleManagerTest::InitNativeModule(NativeModule* nativeModule, std::string moduleName)
{
    nativeModule->flags = IS_APP_MODULE_FLAGS;
    nativeModule->name = strdup(moduleName.c_str());
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
    NativeEngine* engine = nullptr;
    NativeModuleManager moduleManager;
    moduleManager.SetNativeEngine(moduleKey, engine);

    std::string result = moduleManager.GetModuleFileName(moduleKey.c_str(), true);
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
    NativeModuleManager moduleManager;

    NativeModule module;
    std::string moduleName = "testModuleName";
    InitNativeModule(&module, moduleName);
    moduleManager.prefix_ = "default";
    moduleManager.Register(&module);
    std::string result = moduleManager.GetModuleFileName(moduleName.c_str(), true);
    EXPECT_FALSE(result.empty());
    if (module.name) {
        free(const_cast<char *>(module.name));
    }
    GTEST_LOG_(INFO) << "ModuleManagerTest, LoadNativeModuleTest_007 end";
}

/*
 * @tc.name: GetLoadingNativeModuleKey_WhenSetLoadingNativeModuleKeyToPointValue
 * @tc.desc: test NativeModule's loadingModuleKey_ set and get
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, GetLoadingNativeModuleKey_WhenSetLoadingNativeModuleKeyToPointValue, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLoadingNativeModuleKey_WhenSetLoadingNativeModuleKeyToPointValue starts";

    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();

    moduleManager->SetLoadingNativeModuleKey("");
    EXPECT_EQ(moduleManager->GetLoadingNativeModuleKey(), "");
    
    moduleManager->SetLoadingNativeModuleKey("1234");
    EXPECT_EQ(moduleManager->GetLoadingNativeModuleKey(), "1234");

    GTEST_LOG_(INFO) << "GetLoadingNativeModuleKey_WhenSetLoadingNativeModuleKeyToPointValue end";
}

/*
 * @tc.name: FindNativeModuleByCache_WhenNativeModuleListIsEmpty
 * @tc.desc: test NativeModule's FindNativeModuleByCache func
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, FindNativeModuleByCache_WhenNativeModuleListIsEmpty, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindNativeModuleByCache_WhenNativeModuleListIsEmpty starts";

    NativeModuleManager moduleManager;
    std::string moduleKey = "default/";
    char nativeModulePath[NATIVE_PATH_NUMBER][NAPI_PATH_MAX];
    NativeModule* cacheNativeModule = nullptr;
    NativeModuleHeadTailStruct cacheHeadTailStruct = {nullptr, nullptr, nullptr};
    NativeModule *nativeModule = moduleManager.FindNativeModuleByCache(moduleKey.c_str(), nativeModulePath,
        cacheNativeModule, cacheHeadTailStruct, false);
    EXPECT_EQ(nativeModule, nullptr);
    nativeModule = moduleManager.FindNativeModuleByCache(moduleKey.c_str(), nativeModulePath,
        cacheNativeModule, cacheHeadTailStruct);
    EXPECT_EQ(nativeModule, nullptr);
    GTEST_LOG_(INFO) << "FindNativeModuleByCache_WhenNativeModuleListIsEmpty end";
}

/*
 * @tc.name: FindNativeModuleByCache_WhenModuleNameExistInListAndNotInLoading
 * @tc.desc: test NativeModule's FindNativeModuleByCache func
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, FindNativeModuleByCache_WhenModuleNameExistInListAndNotInLoading, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindNativeModuleByCache_WhenModuleNameExistInListAndNotInLoading starts";

    NativeModuleManager moduleManager;
    char nativeModulePath[NATIVE_PATH_NUMBER][NAPI_PATH_MAX];
    NativeModule* cacheNativeModule = nullptr;
    NativeModuleHeadTailStruct cacheHeadTailStruct = {nullptr, nullptr, nullptr};

    NativeModule module;
    std::string moduleName = "testModuleName";
    InitNativeModule(&module, moduleName);
    std::string moduleKey = "default/" + moduleName;
    moduleManager.loadingModuleName_ = moduleKey;
    moduleManager.Register(&module);
    EXPECT_EQ(moduleManager.GetLoadingNativeModuleKey(), moduleKey);
    moduleManager.SetLoadingNativeModuleKey(moduleName.c_str());
    NativeModule *nativeModule = moduleManager.FindNativeModuleByCache(moduleKey.c_str(), nativeModulePath,
        cacheNativeModule, cacheHeadTailStruct, false);
    EXPECT_NE(nativeModule, nullptr);

    nativeModule = moduleManager.FindNativeModuleByCache(moduleKey.c_str(), nativeModulePath,
        cacheNativeModule, cacheHeadTailStruct);
    EXPECT_NE(nativeModule, nullptr);
    if (module.name) {
        free(const_cast<char *>(module.name));
    }
    GTEST_LOG_(INFO) << "FindNativeModuleByCache_WhenModuleNameExistInListAndNotInLoading end";
}

/*
 * @tc.name: FindNativeModuleByCache_WhenModuleNameExistInListAndInLoading
 * @tc.desc: test NativeModule's FindNativeModuleByCache func
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, FindNativeModuleByCache_WhenModuleNameExistInListAndInLoading, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindNativeModuleByCache_WhenModuleNameExistInListAndInLoading starts";

    NativeModuleManager moduleManager;
    char nativeModulePath[NATIVE_PATH_NUMBER][NAPI_PATH_MAX];
    NativeModule* cacheNativeModule = nullptr;
    NativeModuleHeadTailStruct cacheHeadTailStruct = {nullptr, nullptr, nullptr};

    NativeModule module;
    std::string moduleName = "testModuleName";
    InitNativeModule(&module, moduleName);
    std::string moduleKey = "default/" + moduleName;
    moduleManager.loadingModuleName_ = moduleKey;
    moduleManager.Register(&module);
    moduleManager.SetLoadingNativeModuleKey(moduleKey.c_str());

    NativeModule *nativeModule = moduleManager.FindNativeModuleByCache(moduleKey.c_str(), nativeModulePath,
        cacheNativeModule, cacheHeadTailStruct);
    EXPECT_NE(nativeModule, nullptr);
    EXPECT_EQ(cacheHeadTailStruct.matchLoadingNativeModule, nullptr);

    nativeModule = moduleManager.FindNativeModuleByCache(moduleKey.c_str(), nativeModulePath,
        cacheNativeModule, cacheHeadTailStruct, true);
    EXPECT_EQ(nativeModule, nullptr);
    EXPECT_NE(cacheHeadTailStruct.matchLoadingNativeModule, nullptr);
    if (module.name) {
        free(const_cast<char *>(module.name));
    }
    GTEST_LOG_(INFO) << "FindNativeModuleByCache_WhenModuleNameExistInListAndInLoading end";
}

/*
 * @tc.name: FindNativeModuleByCache_ShouldReturnModuleWhenFindTwiceModuleNameExistInListAndInLoading
 * @tc.desc: test NativeModule's FindNativeModuleByCache func
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, FindNativeModuleByCache_ShouldReturnModuleWhenFindTwiceModuleNameExistInListAndInLoading,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindNativeModuleByCache_ShouldReturnModuleWhenFindTwiceModuleNameExistInListAndInLoading"
                        " starts";

    NativeModuleManager moduleManager;
    char nativeModulePath[NATIVE_PATH_NUMBER][NAPI_PATH_MAX];
    NativeModule* cacheNativeModule = nullptr;
    NativeModuleHeadTailStruct cacheHeadTailStruct = {nullptr, nullptr, nullptr};

    NativeModule module;
    std::string moduleName = "testModuleName";
    InitNativeModule(&module, moduleName);
    std::string moduleKey = "default/" + moduleName;
    moduleManager.loadingModuleName_ = moduleKey;
    moduleManager.Register(&module);
    moduleManager.SetLoadingNativeModuleKey(moduleKey.c_str());

    NativeModule *nativeModule = moduleManager.FindNativeModuleByCache(moduleKey.c_str(), nativeModulePath,
        cacheNativeModule, cacheHeadTailStruct, true);
    EXPECT_EQ(nativeModule, nullptr);
    EXPECT_NE(cacheHeadTailStruct.matchLoadingNativeModule, nullptr);

    nativeModule = moduleManager.FindNativeModuleByCache(moduleKey.c_str(), nativeModulePath,
        cacheNativeModule, cacheHeadTailStruct);
    EXPECT_NE(cacheHeadTailStruct.matchLoadingNativeModule, nullptr);
    EXPECT_EQ(nativeModule, cacheHeadTailStruct.matchLoadingNativeModule);
    if (module.name) {
        free(const_cast<char *>(module.name));
    }
    GTEST_LOG_(INFO) << "FindNativeModuleByCache_ShouldReturnModuleWhenFindTwiceModuleNameExistInListAndInLoading end";
}

/*
 * @tc.name: CheckNativeListChanged_ShouldReturnTrueWhenCacheHeadModuleIsNullptr
 * @tc.desc: test NativeModule's CheckNativeListChanged func
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, CheckNativeListChanged_ShouldReturnTrueWhenCacheHeadModuleIsNullptr, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNativeListChanged_ShouldReturnTrueWhenCacheHeadModuleIsNullptr starts";

    NativeModuleManager moduleManager;
    NativeModule *cacheHeadNativeModule = nullptr;
    NativeModule cacheTailNativeModule;
    NativeModule *matchLoadingNativeModule = nullptr;
    moduleManager.headNativeModule_ = new NativeModule();
    EXPECT_NE(moduleManager.headNativeModule_, nullptr);
    moduleManager.tailNativeModule_ = new NativeModule();
    EXPECT_NE(moduleManager.tailNativeModule_, nullptr);

    bool result = moduleManager.CheckNativeListChanged(cacheHeadNativeModule, &cacheTailNativeModule,
        matchLoadingNativeModule);
    EXPECT_TRUE(result);
    delete moduleManager.headNativeModule_;
    moduleManager.headNativeModule_ = nullptr;
    delete moduleManager.tailNativeModule_;
    moduleManager.tailNativeModule_ = nullptr;
    GTEST_LOG_(INFO) << "CheckNativeListChanged_ShouldReturnTrueWhenCacheHeadModuleIsNullptr end";
}

/*
 * @tc.name: CheckNativeListChanged_ShouldReturnTrueWhenCacheTailModuleIsNullptr
 * @tc.desc: test NativeModule's CheckNativeListChanged func
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, CheckNativeListChanged_ShouldReturnTrueWhenCacheTailModuleIsNullptr, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNativeListChanged_ShouldReturnTrueWhenCacheTailModuleIsNullptr starts";

    NativeModuleManager moduleManager;
    NativeModule cacheHeadNativeModule;
    NativeModule *cacheTailNativeModule = nullptr;
    NativeModule *matchLoadingNativeModule = nullptr;
    moduleManager.headNativeModule_ = new NativeModule();
    EXPECT_NE(moduleManager.headNativeModule_, nullptr);
    moduleManager.tailNativeModule_ = new NativeModule();
    EXPECT_NE(moduleManager.tailNativeModule_, nullptr);

    bool result = moduleManager.CheckNativeListChanged(&cacheHeadNativeModule, cacheTailNativeModule,
        matchLoadingNativeModule);
    EXPECT_TRUE(result);
    delete moduleManager.headNativeModule_;
    moduleManager.headNativeModule_ = nullptr;
    delete moduleManager.tailNativeModule_;
    moduleManager.tailNativeModule_ = nullptr;
    GTEST_LOG_(INFO) << "CheckNativeListChanged_ShouldReturnTrueWhenCacheTailModuleIsNullptr end";
}

/*
 * @tc.name: CheckNativeListChanged_ShouldReturnTrueWhenHeadModuleIsNullptr
 * @tc.desc: test NativeModule's CheckNativeListChanged func
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, CheckNativeListChanged_ShouldReturnTrueWhenHeadModuleIsNullptr, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNativeListChanged_ShouldReturnTrueWhenHeadModuleIsNullptr starts";

    NativeModuleManager moduleManager;
    NativeModule cacheHeadNativeModule;
    NativeModule cacheTailNativeModule;
    NativeModule *matchLoadingNativeModule = nullptr;
    moduleManager.tailNativeModule_ = new NativeModule();
    EXPECT_NE(moduleManager.tailNativeModule_, nullptr);

    bool result = moduleManager.CheckNativeListChanged(&cacheHeadNativeModule, &cacheTailNativeModule,
        matchLoadingNativeModule);
    EXPECT_TRUE(result);
    delete moduleManager.tailNativeModule_;
    moduleManager.tailNativeModule_ = nullptr;
    GTEST_LOG_(INFO) << "CheckNativeListChanged_ShouldReturnTrueWhenHeadModuleIsNullptr end";
}

/*
 * @tc.name: CheckNativeListChanged_ShouldReturnTrueWhenTailModuleIsNullptr
 * @tc.desc: test NativeModule's CheckNativeListChanged func
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, CheckNativeListChanged_ShouldReturnTrueWhenTailModuleIsNullptr, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNativeListChanged_ShouldReturnTrueWhenTailModuleIsNullptr starts";

    NativeModuleManager moduleManager;
    NativeModule cacheHeadNativeModule;
    NativeModule cacheTailNativeModule;
    NativeModule *matchLoadingNativeModule = nullptr;
    moduleManager.headNativeModule_ = new NativeModule();
    EXPECT_NE(moduleManager.headNativeModule_, nullptr);

    bool result = moduleManager.CheckNativeListChanged(&cacheHeadNativeModule, &cacheTailNativeModule,
        matchLoadingNativeModule);
    EXPECT_TRUE(result);
    delete moduleManager.headNativeModule_;
    moduleManager.headNativeModule_ = nullptr;
    GTEST_LOG_(INFO) << "CheckNativeListChanged_ShouldReturnTrueWhenTailModuleIsNullptr end";
}

/*
 * @tc.name: CheckNativeListChanged_ShouldReturnTrueWhenMatchLoadingNativeModuleIsNotNull
 * @tc.desc: test NativeModule's CheckNativeListChanged func
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, CheckNativeListChanged_ShouldReturnTrueWhenMatchLoadingNativeModuleIsNotNull,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNativeListChanged_ShouldReturnTrueWhenMatchLoadingNativeModuleIsNotNull starts";

    NativeModuleManager moduleManager;
    NativeModule cacheHeadNativeModule;
    NativeModule cacheTailNativeModule;
    NativeModule matchLoadingNativeModule;
    moduleManager.headNativeModule_ = new NativeModule();
    EXPECT_NE(moduleManager.headNativeModule_, nullptr);
    moduleManager.tailNativeModule_ = new NativeModule();
    EXPECT_NE(moduleManager.tailNativeModule_, nullptr);

    bool result = moduleManager.CheckNativeListChanged(&cacheHeadNativeModule, &cacheTailNativeModule,
        &matchLoadingNativeModule);
    EXPECT_TRUE(result);
    delete moduleManager.headNativeModule_;
    moduleManager.headNativeModule_ = nullptr;
    delete moduleManager.tailNativeModule_;
    moduleManager.tailNativeModule_ = nullptr;
    GTEST_LOG_(INFO) << "CheckNativeListChanged_ShouldReturnTrueWhenMatchLoadingNativeModuleIsNotNull end";
}

/*
 * @tc.name: CheckNativeListChanged_ShouldReturnTrueWhenHeadNameIsNotMatch
 * @tc.desc: test NativeModule's CheckNativeListChanged func
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, CheckNativeListChanged_ShouldReturnTrueWhenHeadNameIsNotMatch,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNativeListChanged_ShouldReturnTrueWhenHeadNameIsNotMatch starts";

    NativeModuleManager moduleManager;
    std::string moduleName = "testModuleName";
    std::string diffModuleName = "diffModuleName";
    NativeModule cacheHeadNativeModule;
    cacheHeadNativeModule.name = strdup(moduleName.c_str());
    EXPECT_NE(cacheHeadNativeModule.name, nullptr);
    NativeModule cacheTailNativeModule;
    cacheTailNativeModule.name = strdup(moduleName.c_str());
    EXPECT_NE(cacheTailNativeModule.name, nullptr);
    NativeModule matchLoadingNativeModule;
    moduleManager.headNativeModule_ = new NativeModule();
    EXPECT_NE(moduleManager.headNativeModule_, nullptr);
    moduleManager.headNativeModule_->name = strdup(diffModuleName.c_str());
    EXPECT_NE(moduleManager.headNativeModule_->name, nullptr);
    moduleManager.tailNativeModule_ = new NativeModule();
    EXPECT_NE(moduleManager.tailNativeModule_, nullptr);
    moduleManager.tailNativeModule_->name = strdup(moduleName.c_str());
    EXPECT_NE(moduleManager.tailNativeModule_->name, nullptr);

    bool result = moduleManager.CheckNativeListChanged(&cacheHeadNativeModule, &cacheTailNativeModule,
        &matchLoadingNativeModule);
    EXPECT_TRUE(result);
    free(const_cast<char *>(cacheHeadNativeModule.name));
    cacheHeadNativeModule.name = nullptr;
    free(const_cast<char *>(cacheTailNativeModule.name));
    cacheTailNativeModule.name = nullptr;
    free(const_cast<char *>(moduleManager.tailNativeModule_->name));
    moduleManager.tailNativeModule_->name = nullptr;
    free(const_cast<char *>(moduleManager.headNativeModule_->name));
    moduleManager.headNativeModule_->name = nullptr;
    delete moduleManager.headNativeModule_;
    moduleManager.headNativeModule_ = nullptr;
    delete moduleManager.tailNativeModule_;
    moduleManager.tailNativeModule_ = nullptr;
    GTEST_LOG_(INFO) << "CheckNativeListChanged_ShouldReturnTrueWhenHeadNameIsNotMatch end";
}

/*
 * @tc.name: CheckNativeListChanged_ShouldReturnTrueWhenTailNameIsNotMatch
 * @tc.desc: test NativeModule's CheckNativeListChanged func
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, CheckNativeListChanged_ShouldReturnTrueWhenTailNameIsNotMatch,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNativeListChanged_ShouldReturnTrueWhenTailNameIsNotMatch starts";

    NativeModuleManager moduleManager;
    std::string moduleName = "testModuleName";
    std::string diffModuleName = "diffModuleName";
    NativeModule cacheHeadNativeModule;
    cacheHeadNativeModule.name = strdup(moduleName.c_str());
    EXPECT_NE(cacheHeadNativeModule.name, nullptr);
    NativeModule cacheTailNativeModule;
    cacheTailNativeModule.name = strdup(moduleName.c_str());
    EXPECT_NE(cacheTailNativeModule.name, nullptr);
    NativeModule matchLoadingNativeModule;
    moduleManager.headNativeModule_ = new NativeModule();
    EXPECT_NE(moduleManager.headNativeModule_, nullptr);
    moduleManager.headNativeModule_->name = strdup(moduleName.c_str());
    EXPECT_NE(moduleManager.headNativeModule_->name, nullptr);
    moduleManager.tailNativeModule_ = new NativeModule();
    EXPECT_NE(moduleManager.tailNativeModule_, nullptr);
    moduleManager.tailNativeModule_->name = strdup(diffModuleName.c_str());
    EXPECT_NE(moduleManager.tailNativeModule_->name, nullptr);

    bool result = moduleManager.CheckNativeListChanged(&cacheHeadNativeModule, &cacheTailNativeModule,
        &matchLoadingNativeModule);
    EXPECT_TRUE(result);
    free(const_cast<char *>(cacheHeadNativeModule.name));
    cacheHeadNativeModule.name = nullptr;
    free(const_cast<char *>(cacheTailNativeModule.name));
    cacheTailNativeModule.name = nullptr;
    free(const_cast<char *>(moduleManager.tailNativeModule_->name));
    moduleManager.tailNativeModule_->name = nullptr;
    free(const_cast<char *>(moduleManager.headNativeModule_->name));
    moduleManager.headNativeModule_->name = nullptr;
    delete moduleManager.headNativeModule_;
    moduleManager.headNativeModule_ = nullptr;
    delete moduleManager.tailNativeModule_;
    moduleManager.tailNativeModule_ = nullptr;
    GTEST_LOG_(INFO) << "CheckNativeListChanged_ShouldReturnTrueWhenTailNameIsNotMatch end";
}

/*
 * @tc.name: CheckNativeListChanged_ShouldReturnFalseWhenAllNameMatch
 * @tc.desc: test NativeModule's CheckNativeListChanged func
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, CheckNativeListChanged_ShouldReturnFalseWhenAllNameMatch,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNativeListChanged_ShouldReturnFalseWhenAllNameMatch starts";

    NativeModuleManager moduleManager;
    std::string moduleName = "testModuleName";
    std::string diffModuleName = "diffModuleName";
    NativeModule cacheHeadNativeModule;
    cacheHeadNativeModule.name = strdup(moduleName.c_str());
    EXPECT_NE(cacheHeadNativeModule.name, nullptr);
    NativeModule cacheTailNativeModule;
    cacheTailNativeModule.name = strdup(moduleName.c_str());
    EXPECT_NE(cacheTailNativeModule.name, nullptr);
    NativeModule *matchLoadingNativeModule = nullptr;
    moduleManager.headNativeModule_ = new NativeModule();
    EXPECT_NE(moduleManager.headNativeModule_, nullptr);
    moduleManager.headNativeModule_->name = strdup(moduleName.c_str());
    EXPECT_NE(moduleManager.headNativeModule_->name, nullptr);
    moduleManager.tailNativeModule_ = new NativeModule();
    EXPECT_NE(moduleManager.tailNativeModule_, nullptr);
    moduleManager.tailNativeModule_->name = strdup(moduleName.c_str());
    EXPECT_NE(moduleManager.tailNativeModule_->name, nullptr);

    bool result = moduleManager.CheckNativeListChanged(&cacheHeadNativeModule, &cacheTailNativeModule,
        matchLoadingNativeModule);
    EXPECT_FALSE(result);
    free(const_cast<char *>(cacheHeadNativeModule.name));
    cacheHeadNativeModule.name = nullptr;
    free(const_cast<char *>(cacheTailNativeModule.name));
    cacheTailNativeModule.name = nullptr;
    free(const_cast<char *>(moduleManager.tailNativeModule_->name));
    moduleManager.tailNativeModule_->name = nullptr;
    free(const_cast<char *>(moduleManager.headNativeModule_->name));
    moduleManager.headNativeModule_->name = nullptr;
    delete moduleManager.headNativeModule_;
    moduleManager.headNativeModule_ = nullptr;
    delete moduleManager.tailNativeModule_;
    moduleManager.tailNativeModule_ = nullptr;
    GTEST_LOG_(INFO) << "CheckNativeListChanged_ShouldReturnFalseWhenAllNameMatch end";
}

/*
 * Link-time Mock based test cases for Greylist configuration
 * These tests use the --wrap linker option to mock file operations
 * without modifying real files on the filesystem
 */

/*
 * @tc.name: LoadGreylistConfig_WithMock_EmptyFile
 * @tc.desc: test LoadGreylistConfig when greylist config file is empty using link-time mock
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, LoadGreylistConfig_WithMock_EmptyFile, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_EmptyFile starts";

    DlsymMockGuard mockGuard;
    mockGuard.SetFileContent(GREYLIST_CONFIG_PATH_LT, "");

    auto moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    moduleManager->CreateSharedLibsSonames();
    EXPECT_NE(moduleManager->sharedLibsSonames_, nullptr);

    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_EmptyFile end";
}

/*
 * @tc.name: LoadGreylistConfig_WithMock_InvalidFormat
 * @tc.desc: test LoadGreylistConfig when greylist config has invalid JSON format using link-time mock
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, LoadGreylistConfig_WithMock_InvalidFormat, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_InvalidFormat starts";

    DlsymMockGuard mockGuard;
    mockGuard.SetFileContent(GREYLIST_CONFIG_PATH_LT, "invalid json content");

    auto moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    moduleManager->CreateSharedLibsSonames();
    EXPECT_NE(moduleManager->sharedLibsSonames_, nullptr);

    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_InvalidFormat end";
}

/*
 * @tc.name: LoadGreylistConfig_WithMock_MergeLibraries
 * @tc.desc: test CreateSharedLibsSonames correctly merges greylist libraries using link-time mock
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, LoadGreylistConfig_WithMock_MergeLibraries, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_MergeLibraries starts";

    DlsymMockGuard mockGuard;
    mockGuard.SetFileContent(GREYLIST_CONFIG_PATH_LT,
        "[\"libcustom1.z.so\", \"libcustom2.z.so\"]");

    auto moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    moduleManager->CreateSharedLibsSonames();
    EXPECT_NE(moduleManager->sharedLibsSonames_, nullptr);

    std::string sonames(moduleManager->sharedLibsSonames_);
    EXPECT_NE(sonames.find("libc.so"), std::string::npos);
    EXPECT_NE(sonames.find("libace_napi.z.so"), std::string::npos);
    EXPECT_NE(sonames.find("libcustom1.z.so"), std::string::npos);
    EXPECT_NE(sonames.find("libcustom2.z.so"), std::string::npos);

    size_t pos = 0;
    int libCount = 0;
    while ((pos = sonames.find(':', pos)) != std::string::npos) {
        libCount++;
        pos++;
    }
    EXPECT_GT(libCount, 0);

    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_MergeLibraries end";
}

/*
 * @tc.name: LoadGreylistConfig_WithMock_InvalidLibNames
 * @tc.desc: test LoadGreylistConfig rejects library names not ending with .so using link-time mock
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, LoadGreylistConfig_WithMock_InvalidLibNames, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_InvalidLibNames starts";

    DlsymMockGuard mockGuard;
    mockGuard.SetFileContent(GREYLIST_CONFIG_PATH_LT,
        "[\"libvalid1.z.so\", \"libinvalid\", \"libvalid2.so\", \"libinvalid.dll\"]");

    auto moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    moduleManager->CreateSharedLibsSonames();
    EXPECT_NE(moduleManager->sharedLibsSonames_, nullptr);

    std::string sonames(moduleManager->sharedLibsSonames_);
    EXPECT_NE(sonames.find("libvalid1.z.so"), std::string::npos);
    EXPECT_NE(sonames.find("libvalid2.so"), std::string::npos);
    EXPECT_EQ(sonames.find("\"libinvalid\","), std::string::npos);
    EXPECT_EQ(sonames.find("libinvalid.dll"), std::string::npos);

    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_InvalidLibNames end";
}

/*
 * @tc.name: LoadGreylistConfig_WithMock_SpecialCharacters
 * @tc.desc: test LoadGreylistConfig accepts library names with special characters using link-time mock
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, LoadGreylistConfig_WithMock_SpecialCharacters, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_SpecialCharacters starts";

    DlsymMockGuard mockGuard;
    mockGuard.SetFileContent(GREYLIST_CONFIG_PATH_LT,
        "[\"lib-test.so\", \"lib_test.so\", \"lib.test.so\", \"lib测试.so\", \"libvalid.so\"]");

    auto moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    moduleManager->CreateSharedLibsSonames();
    EXPECT_NE(moduleManager->sharedLibsSonames_, nullptr);

    std::string sonames(moduleManager->sharedLibsSonames_);
    EXPECT_NE(sonames.find("lib-test.so"), std::string::npos);
    EXPECT_NE(sonames.find("lib_test.so"), std::string::npos);
    EXPECT_NE(sonames.find("lib.test.so"), std::string::npos);
    EXPECT_NE(sonames.find("lib测试.so"), std::string::npos);
    EXPECT_NE(sonames.find("libvalid.so"), std::string::npos);

    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_SpecialCharacters end";
}

/*
 * @tc.name: LoadGreylistConfig_WithMock_EmptyArray
 * @tc.desc: test LoadGreylistConfig when greylist config is empty array using link-time mock
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, LoadGreylistConfig_WithMock_EmptyArray, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_EmptyArray starts";

    DlsymMockGuard mockGuard;
    mockGuard.SetFileContent(GREYLIST_CONFIG_PATH_LT, "[]");

    auto moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    moduleManager->CreateSharedLibsSonames();
    EXPECT_NE(moduleManager->sharedLibsSonames_, nullptr);
    EXPECT_NE(moduleManager->sharedLibsSonames_[0], '\0');

    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_EmptyArray end";
}

/*
 * @tc.name: LoadGreylistConfig_WithMock_OnlyOpenBracket
 * @tc.desc: test LoadGreylistConfig when config has only '[' without ']' using link-time mock
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, LoadGreylistConfig_WithMock_OnlyOpenBracket, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_OnlyOpenBracket starts";

    DlsymMockGuard mockGuard;
    mockGuard.SetFileContent(GREYLIST_CONFIG_PATH_LT, "[\"libtest.so\"");

    auto moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    moduleManager->CreateSharedLibsSonames();
    EXPECT_NE(moduleManager->sharedLibsSonames_, nullptr);

    std::string sonames(moduleManager->sharedLibsSonames_);
    EXPECT_NE(sonames.find("libc.so"), std::string::npos);
    EXPECT_EQ(sonames.find("libtest.so"), std::string::npos);

    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_OnlyOpenBracket end";
}

/*
 * @tc.name: LoadGreylistConfig_WithMock_OnlyCloseBracket
 * @tc.desc: test LoadGreylistConfig when config has only ']' without '[' using link-time mock
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, LoadGreylistConfig_WithMock_OnlyCloseBracket, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_OnlyCloseBracket starts";

    DlsymMockGuard mockGuard;
    mockGuard.SetFileContent(GREYLIST_CONFIG_PATH_LT, "\"libtest.so\"]");

    auto moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    moduleManager->CreateSharedLibsSonames();
    EXPECT_NE(moduleManager->sharedLibsSonames_, nullptr);

    std::string sonames(moduleManager->sharedLibsSonames_);
    EXPECT_NE(sonames.find("libc.so"), std::string::npos);
    EXPECT_EQ(sonames.find("libtest.so"), std::string::npos);

    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_OnlyCloseBracket end";
}

/*
 * @tc.name: LoadGreylistConfig_WithMock_ReversedBrackets
 * @tc.desc: test LoadGreylistConfig when config has ']' before '[' using link-time mock
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, LoadGreylistConfig_WithMock_ReversedBrackets, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_ReversedBrackets starts";

    DlsymMockGuard mockGuard;
    mockGuard.SetFileContent(GREYLIST_CONFIG_PATH_LT, "][\"libtest.so\"]");

    auto moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    moduleManager->CreateSharedLibsSonames();
    EXPECT_NE(moduleManager->sharedLibsSonames_, nullptr);

    std::string sonames(moduleManager->sharedLibsSonames_);
    EXPECT_NE(sonames.find("libc.so"), std::string::npos);
    EXPECT_EQ(sonames.find("libtest.so"), std::string::npos);

    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_ReversedBrackets end";
}

/*
 * @tc.name: LoadGreylistConfig_WithMock_FileNotExist
 * @tc.desc: test LoadGreylistConfig when greylist config file does not exist using link-time mock
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, LoadGreylistConfig_WithMock_FileNotExist, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_FileNotExist starts";

    DlsymMockGuard mockGuard;
    mockGuard.SetFileNotExists(GREYLIST_CONFIG_PATH_LT);

    auto moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    moduleManager->CreateSharedLibsSonames();
    EXPECT_NE(moduleManager->sharedLibsSonames_, nullptr);
    EXPECT_NE(moduleManager->sharedLibsSonames_[0], '\0');

    std::string sonames(moduleManager->sharedLibsSonames_);
    EXPECT_NE(sonames.find("libc.so"), std::string::npos);
    EXPECT_NE(sonames.find("libace_napi.z.so"), std::string::npos);

    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_FileNotExist end";
}

/*
 * @tc.name: LoadGreylistConfig_WithMock_MultipleOpenBrackets
 * @tc.desc: test LoadGreylistConfig when config has multiple '[' brackets using link-time mock
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, LoadGreylistConfig_WithMock_MultipleOpenBrackets, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_MultipleOpenBrackets starts";

    DlsymMockGuard mockGuard;
    mockGuard.SetFileContent(GREYLIST_CONFIG_PATH_LT, "[[\"libtest.so\"]");

    auto moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    moduleManager->CreateSharedLibsSonames();
    EXPECT_NE(moduleManager->sharedLibsSonames_, nullptr);

    std::string sonames(moduleManager->sharedLibsSonames_);
    EXPECT_NE(sonames.find("libc.so"), std::string::npos);
    EXPECT_EQ(sonames.find("libtest.so"), std::string::npos);

    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_MultipleOpenBrackets end";
}

/*
 * @tc.name: LoadGreylistConfig_WithMock_MultipleCloseBrackets
 * @tc.desc: test LoadGreylistConfig when config has multiple ']' brackets using link-time mock
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, LoadGreylistConfig_WithMock_MultipleCloseBrackets, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_MultipleCloseBrackets starts";

    DlsymMockGuard mockGuard;
    mockGuard.SetFileContent(GREYLIST_CONFIG_PATH_LT, "[\"libtest.so\"]]");

    auto moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    moduleManager->CreateSharedLibsSonames();
    EXPECT_NE(moduleManager->sharedLibsSonames_, nullptr);

    std::string sonames(moduleManager->sharedLibsSonames_);
    EXPECT_NE(sonames.find("libc.so"), std::string::npos);
    EXPECT_EQ(sonames.find("libtest.so"), std::string::npos);

    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_MultipleCloseBrackets end";
}

/*
 * @tc.name: LoadGreylistConfig_WithMock_MultipleBracketPairs
 * @tc.desc: test LoadGreylistConfig when config has multiple '[]' pairs using link-time mock
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, LoadGreylistConfig_WithMock_MultipleBracketPairs, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_MultipleBracketPairs starts";

    DlsymMockGuard mockGuard;
    mockGuard.SetFileContent(GREYLIST_CONFIG_PATH_LT, "[\"libtest.so\"][\"libtest2.so\"]");

    auto moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    moduleManager->CreateSharedLibsSonames();
    EXPECT_NE(moduleManager->sharedLibsSonames_, nullptr);

    std::string sonames(moduleManager->sharedLibsSonames_);
    EXPECT_NE(sonames.find("libc.so"), std::string::npos);
    EXPECT_EQ(sonames.find("libtest.so"), std::string::npos);
    EXPECT_EQ(sonames.find("libtest2.so"), std::string::npos);

    GTEST_LOG_(INFO) << "LoadGreylistConfig_WithMock_MultipleBracketPairs end";
}

/*
 * @tc.name: UpdateNamespaceLibPath_001
 * @tc.desc: test NativeModule's UpdateNamespaceLibPath function when namespace not exist
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, UpdateNamespaceLibPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, UpdateNamespaceLibPath_001 starts";

    std::string moduleName = "moduleName_UpdateNamespaceLibPath_001";
    std::shared_ptr<NativeModuleManager> moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    std::string nsName;
    bool res = moduleManager->GetLdNamespaceName(moduleName, nsName);
    EXPECT_EQ(res, false);

    std::vector<std::string> appLibPath = { "/new/path1", "/new/path2" };
    moduleManager->UpdateNamespaceLibPath(moduleName, appLibPath);

    GTEST_LOG_(INFO) << "ModuleManagerTest, UpdateNamespaceLibPath_001 end";
}

/*
 * @tc.name: UpdateNamespaceLibPath_002
 * @tc.desc: test NativeModule's UpdateNamespaceLibPath function when namespace exist
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, UpdateNamespaceLibPath_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, UpdateNamespaceLibPath_002 starts";

    std::string moduleName = "moduleName_UpdateNamespaceLibPath_002";
    std::shared_ptr<NativeModuleManager> moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    moduleManager->CreateLdNamespace(moduleName, "/initial/path", false);

    std::string nsName;
    bool res = moduleManager->GetLdNamespaceName(moduleName, nsName);
    EXPECT_EQ(res, true);

    std::vector<std::string> appLibPath = { "/new/path1", "/new/path2" };
    moduleManager->UpdateNamespaceLibPath(moduleName, appLibPath);

    GTEST_LOG_(INFO) << "ModuleManagerTest, UpdateNamespaceLibPath_002 end";
}

/*
 * @tc.name: UpdateNamespaceLibPath_003
 * @tc.desc: test NativeModule's UpdateNamespaceLibPath function with empty path
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, UpdateNamespaceLibPath_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, UpdateNamespaceLibPath_003 starts";

    std::string moduleName = "moduleName_UpdateNamespaceLibPath_003";
    std::shared_ptr<NativeModuleManager> moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    moduleManager->CreateLdNamespace(moduleName, "/initial/path", false);

    std::vector<std::string> appLibPath;
    moduleManager->UpdateNamespaceLibPath(moduleName, appLibPath);

    GTEST_LOG_(INFO) << "ModuleManagerTest, UpdateNamespaceLibPath_003 end";
}

/*
 * @tc.name: UpdateNamespaceLibPath_004
 * @tc.desc: test NativeModule's UpdateNamespaceLibPath function with SetAppLibPath first
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, UpdateNamespaceLibPath_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, UpdateNamespaceLibPath_004 starts";

    std::string moduleName = "moduleName_UpdateNamespaceLibPath_004";
    std::shared_ptr<NativeModuleManager> moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    std::vector<std::string> initialLibPath = { "/initial/path1", "/initial/path2" };
    moduleManager->SetAppLibPath(moduleName, initialLibPath, false);

    std::string nsName;
    bool res = moduleManager->GetLdNamespaceName(moduleName, nsName);
    EXPECT_EQ(res, true);

    std::vector<std::string> newLibPath = { "/new/path1", "/new/path2" };
    moduleManager->UpdateNamespaceLibPath(moduleName, newLibPath);

    GTEST_LOG_(INFO) << "ModuleManagerTest, UpdateNamespaceLibPath_004 end";
}

/*
 * @tc.name: UpdateNamespaceLibPath_005
 * @tc.desc: test NativeModule's UpdateNamespaceLibPath function with empty element in path
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, UpdateNamespaceLibPath_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModuleManagerTest, UpdateNamespaceLibPath_005 starts";

    std::string moduleName = "moduleName_UpdateNamespaceLibPath_005";
    std::shared_ptr<NativeModuleManager> moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    moduleManager->CreateLdNamespace(moduleName, "/initial/path", false);

    std::vector<std::string> appLibPath = { "", "/new/path1", "", "/new/path2", "" };
    moduleManager->UpdateNamespaceLibPath(moduleName, appLibPath);

    GTEST_LOG_(INFO) << "ModuleManagerTest, UpdateNamespaceLibPath_005 end";
}