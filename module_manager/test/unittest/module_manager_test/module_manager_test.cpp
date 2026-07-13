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

#include <algorithm>
#include <cstring>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#include "dlsym_mock_guard.h"
#include "mock_native_module_manager.h"
#include "module_load_checker.h"

using namespace testing::ext;

namespace {
    constexpr static int32_t NATIVE_PATH_NUMBER = 3;
    constexpr static int32_t IS_APP_MODULE_FLAGS = 100;
    constexpr char GREYLIST_CONFIG_PATH_LT[] =
        "/data/service/el0/public/for-all-app/musl_namespace_config/greylist.json";
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

    MockCheckModuleLoadable(true);
    MockLoadModuleLibrary(nullptr);
    // Register module into cache, then LoadNativeModule finds it via FindNativeModuleByCache
    mockModule.name = strdup(moduleName);
    mockModule.moduleName = strdup(moduleName);
    moduleManager->Register(&mockModule);

    std::string errInfo = "";
    module = moduleManager->LoadNativeModule(moduleName, nullptr, false, errInfo, false, "");
    EXPECT_NE(module, nullptr);

    module = moduleManager->LoadNativeModule(moduleName, nullptr, false, errInfo, false);
    EXPECT_NE(module, nullptr);

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

    MockCheckModuleLoadable(true);
    MockLoadModuleLibrary(nullptr);
    mockModule.name = strdup(moduleName);
    mockModule.moduleName = strdup(moduleName);
    moduleManager->Register(&mockModule);

    std::string errInfo = "";
    module = moduleManager->LoadNativeModule(moduleName, nullptr, false, errInfo, false, relativePath);
    EXPECT_NE(module, nullptr);

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
    char nativeModulePath[3][4096];
    nativeModulePath[0][0] = 0;
    nativeModulePath[1][0] = 0;
    nativeModulePath[2][0] = 0;

    std::string errInfo = "";
    EXPECT_EQ(moduleManager->FindNativeModuleByDisk(moduleName, nullptr, nullptr, false, false, errInfo,
        nullptr, nativeModulePath, nullptr), nullptr);
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
 * @tc.desc: test NativeModule's UnloadNativeModule function on missing key
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
 * @tc.desc: test NativeModule's RemoveModuleLib/UnloadNativeModule on missing key
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

/*
 * @tc.name: SetAppLibPath_ShouldFreeOldPathWhenSetTwice
 * @tc.desc: test SetAppLibPath should free old path when set twice
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ModuleManagerTest, SetAppLibPath_ShouldFreeOldPathWhenSetTwice, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetAppLibPath_ShouldFreeOldPathWhenSetTwice starts";
    std::string moduleName = "moduleName_setpath_twice";
    std::shared_ptr<NativeModuleManager> moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);

    std::vector<std::string> appLibPath1 = { "/path/first" };
    std::vector<std::string> appLibPath2 = { "/path/second" };

    moduleManager->SetAppLibPath(moduleName, appLibPath1, false);
    moduleManager->SetAppLibPath(moduleName, appLibPath2, false);

    EXPECT_NE(moduleManager->appLibPathMap_[moduleName], nullptr);
    EXPECT_STREQ(moduleManager->appLibPathMap_[moduleName], "/path/second");
    GTEST_LOG_(INFO) << "SetAppLibPath_ShouldFreeOldPathWhenSetTwice end";
}

/*
 * @tc.name: RemoveNativeModuleByCache_ShouldHandleNullModuleName
 * @tc.desc: test RemoveNativeModuleByCache should handle null moduleName
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ModuleManagerTest, RemoveNativeModuleByCache_ShouldHandleNullModuleName, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveNativeModuleByCache_ShouldHandleNullModuleName starts";
    NativeModuleManager moduleManager;

    NativeModule* module = new NativeModule();
    module->name = strdup("testModule");
    module->moduleName = nullptr;
    module->next = nullptr;
    moduleManager.headNativeModule_ = module;
    moduleManager.tailNativeModule_ = module;

    bool result = moduleManager.RemoveNativeModuleByCache("testModule");
    EXPECT_FALSE(result);

    delete module;
    moduleManager.headNativeModule_ = nullptr;
    moduleManager.tailNativeModule_ = nullptr;
    GTEST_LOG_(INFO) << "RemoveNativeModuleByCache_ShouldHandleNullModuleName end";
}

/*
 * @tc.name: RemoveNativeModuleByCache_ShouldFreeMemoryWhenRemoveHead
 * @tc.desc: test RemoveNativeModuleByCache should free memory when remove head node
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ModuleManagerTest, RemoveNativeModuleByCache_ShouldFreeMemoryWhenRemoveHead, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveNativeModuleByCache_ShouldFreeMemoryWhenRemoveHead starts";
    NativeModuleManager moduleManager;

    NativeModule* module = new NativeModule();
    std::string moduleName = "headModule";
    module->name = strdup(moduleName.c_str());
    module->moduleName = strdup(moduleName.c_str());
    module->next = nullptr;
    moduleManager.headNativeModule_ = module;
    moduleManager.tailNativeModule_ = module;

    bool result = moduleManager.RemoveNativeModuleByCache(moduleName);
    EXPECT_TRUE(result);
    EXPECT_EQ(moduleManager.headNativeModule_, nullptr);
    EXPECT_EQ(moduleManager.tailNativeModule_, nullptr);
    GTEST_LOG_(INFO) << "RemoveNativeModuleByCache_ShouldFreeMemoryWhenRemoveHead end";
}

/*
 * @tc.name: RemoveNativeModuleByCache_ShouldFreeMemoryWhenRemoveTail
 * @tc.desc: test RemoveNativeModuleByCache should free memory when remove tail node
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ModuleManagerTest, RemoveNativeModuleByCache_ShouldFreeMemoryWhenRemoveTail, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveNativeModuleByCache_ShouldFreeMemoryWhenRemoveTail starts";
    NativeModuleManager moduleManager;

    NativeModule* headModule = new NativeModule();
    headModule->name = strdup("headModule");
    headModule->moduleName = strdup("headModule");
    headModule->next = nullptr;

    NativeModule* tailModule = new NativeModule();
    tailModule->name = strdup("tailModule");
    tailModule->moduleName = strdup("tailModule");
    tailModule->next = nullptr;

    headModule->next = tailModule;
    moduleManager.headNativeModule_ = headModule;
    moduleManager.tailNativeModule_ = tailModule;

    bool result = moduleManager.RemoveNativeModuleByCache("tailModule");
    EXPECT_TRUE(result);
    EXPECT_EQ(moduleManager.headNativeModule_, headModule);
    EXPECT_EQ(moduleManager.tailNativeModule_, headModule);

    free(const_cast<char*>(headModule->name));
    free(const_cast<char*>(headModule->moduleName));
    delete headModule;
    moduleManager.headNativeModule_ = nullptr;
    moduleManager.tailNativeModule_ = nullptr;
    GTEST_LOG_(INFO) << "RemoveNativeModuleByCache_ShouldFreeMemoryWhenRemoveTail end";
}

/*
 * @tc.name: Destructor_ShouldFreeAppLibPathMapMemory
 * @tc.desc: test destructor should free appLibPathMap_ memory correctly
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ModuleManagerTest, Destructor_ShouldFreeAppLibPathMapMemory, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Destructor_ShouldFreeAppLibPathMapMemory starts";
    {
        NativeModuleManager moduleManager;
        std::vector<std::string> appLibPath = { "/test/path" };
        moduleManager.SetAppLibPath("module1", appLibPath, false);
        moduleManager.SetAppLibPath("module2", appLibPath, false);
        EXPECT_EQ(moduleManager.appLibPathMap_.size(), 2);
    }
    GTEST_LOG_(INFO) << "Destructor_ShouldFreeAppLibPathMapMemory end";
}

/*
 * @tc.name: Destructor_ShouldClearNativeEngineListSafely
 * @tc.desc: 析构时应安全清空 nativeEngineList_（AC-17.1 swap + 锁外 delete 路径）
 * @tc.type: FUNC
 * @tc.require: issue #2157 问题 #17
 */
HWTEST_F(ModuleManagerTest, Destructor_ShouldClearNativeEngineListSafely, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Destructor_ShouldClearNativeEngineListSafely starts";
    {
        NativeModuleManager moduleManager;
        // 直接入库 nullptr 值，析构会 swap 出整个 map 后对 nullptr delete（no-op）
        moduleManager.nativeEngineList_.emplace("engine.k1", nullptr);
        moduleManager.nativeEngineList_.emplace("engine.k2", nullptr);
        moduleManager.nativeEngineList_.emplace("engine.k3", nullptr);
        EXPECT_EQ(moduleManager.nativeEngineList_.size(), 3u);
    }
    // 出作用域即调用析构，若走到死锁/UB 会在用例进程中暴露
    GTEST_LOG_(INFO) << "Destructor_ShouldClearNativeEngineListSafely end";
}

/*
 * @tc.name: RemoveNativeModuleByCache_ShouldFreeMemoryWhenRemoveMiddle
 * @tc.desc: test RemoveNativeModuleByCache should free memory when remove middle node
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ModuleManagerTest, RemoveNativeModuleByCache_ShouldFreeMemoryWhenRemoveMiddle, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveNativeModuleByCache_ShouldFreeMemoryWhenRemoveMiddle starts";
    NativeModuleManager moduleManager;

    NativeModule* headModule = new NativeModule();
    headModule->name = strdup("headModule");
    headModule->moduleName = strdup("headModule");
    headModule->next = nullptr;

    NativeModule* middleModule = new NativeModule();
    middleModule->name = strdup("middleModule");
    middleModule->moduleName = strdup("middleModule");
    middleModule->next = nullptr;

    NativeModule* tailModule = new NativeModule();
    tailModule->name = strdup("tailModule");
    tailModule->moduleName = strdup("tailModule");
    tailModule->next = nullptr;

    headModule->next = middleModule;
    middleModule->next = tailModule;
    moduleManager.headNativeModule_ = headModule;
    moduleManager.tailNativeModule_ = tailModule;

    bool result = moduleManager.RemoveNativeModuleByCache("middleModule");
    EXPECT_TRUE(result);
    EXPECT_EQ(moduleManager.headNativeModule_, headModule);
    EXPECT_EQ(moduleManager.tailNativeModule_, tailModule);
    EXPECT_EQ(headModule->next, tailModule);

    free(const_cast<char*>(headModule->name));
    free(const_cast<char*>(headModule->moduleName));
    delete headModule;
    free(const_cast<char*>(tailModule->name));
    free(const_cast<char*>(tailModule->moduleName));
    delete tailModule;
    moduleManager.headNativeModule_ = nullptr;
    moduleManager.tailNativeModule_ = nullptr;
    GTEST_LOG_(INFO) << "RemoveNativeModuleByCache_ShouldFreeMemoryWhenRemoveMiddle end";
}

/*
 * @tc.name: LoadNativeModule_ErrInfo_ModuleNameNull
 * @tc.desc: test LoadNativeModule errInfo when moduleName is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, LoadNativeModule_ErrInfo_ModuleNameNull, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadNativeModule_ErrInfo_ModuleNameNull starts";

    std::string errInfo = "";
    std::string loadErrInfo = "";
    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
    NativeModule* module = moduleManager->LoadNativeModule(nullptr, nullptr, false, errInfo, false, nullptr,
        &loadErrInfo);
    EXPECT_EQ(module, nullptr);
    EXPECT_EQ(errInfo, "nullptr");
    EXPECT_EQ(loadErrInfo, "moduleName is nullptr");

    GTEST_LOG_(INFO) << "LoadNativeModule_ErrInfo_ModuleNameNull end";
}

/*
 * @tc.name: LoadNativeModule_ErrInfo_RelativePathNull
 * @tc.desc: test LoadNativeModule errInfo when relativePath is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, LoadNativeModule_ErrInfo_RelativePathNull, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadNativeModule_ErrInfo_RelativePathNull starts";

    const char* moduleName = "testModule";
    std::string errInfo = "";
    std::string loadErrInfo = "";
    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
    NativeModule* module = moduleManager->LoadNativeModule(moduleName, nullptr, false, errInfo, false, nullptr,
        &loadErrInfo);
    EXPECT_EQ(module, nullptr);
    EXPECT_EQ(errInfo, "nullptr");
    EXPECT_EQ(loadErrInfo, "relativePath is nullptr");

    GTEST_LOG_(INFO) << "LoadNativeModule_ErrInfo_RelativePathNull end";
}

/*
 * @tc.name: LoadNativeModule_ErrInfo_InvalidRelativePath
 * @tc.desc: test LoadNativeModule errInfo when relativePath contains ".."
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, LoadNativeModule_ErrInfo_InvalidRelativePath, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadNativeModule_ErrInfo_InvalidRelativePath starts";

    const char* moduleName = "testModule";
    std::string errInfo = "";
    std::string loadErrInfo = "";
    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
    NativeModule* module = moduleManager->LoadNativeModule(moduleName, nullptr, false, errInfo, false, "../test",
        &loadErrInfo);
    EXPECT_EQ(module, nullptr);
    EXPECT_EQ(loadErrInfo, "invalid relativePath");

    GTEST_LOG_(INFO) << "LoadNativeModule_ErrInfo_InvalidRelativePath end";
}

/*
 * @tc.name: LoadNativeModule_ErrInfo_RelativePathNotContainDotDot
 * @tc.desc: test LoadNativeModule errInfo when relativePath does not contain ".."
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, LoadNativeModule_ErrInfo_RelativePathNotContainDotDot, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadNativeModule_ErrInfo_RelativePathNotContainDotDot starts";

    const char* moduleName = "testModule";
    std::string errInfo = "";
    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
    MockCheckModuleLoadable(true);
    MockLoadModuleLibrary(nullptr);

    NativeModule* module = moduleManager->LoadNativeModule(moduleName, nullptr, false, errInfo, false, "validPath");
    EXPECT_EQ(module, nullptr);
    // relativePath does not contain "..", should NOT set "invalid relativePath" in loadErrInfo
    std::string loadErrInfo;
    module = moduleManager->LoadNativeModule(moduleName, nullptr, false, errInfo, false, "validPath", &loadErrInfo);
    EXPECT_EQ(module, nullptr);
    EXPECT_NE(loadErrInfo, "invalid relativePath");

    GTEST_LOG_(INFO) << "LoadNativeModule_ErrInfo_RelativePathNotContainDotDot end";
}

/*
 * @tc.name: LoadNativeModule_ErrInfo_Blocklisted
 * @tc.desc: test LoadNativeModule loadErrInfo when module is in blocklist
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, LoadNativeModule_ErrInfo_Blocklisted, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadNativeModule_ErrInfo_Blocklisted starts";

    const char* moduleName = "blockedModule";
    std::string errInfo = "";
    std::string loadErrInfo = "";
    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();

    // Set up moduleLoadChecker_ so blocklist check is triggered
    if (!moduleManager->moduleLoadChecker_) {
        moduleManager->moduleLoadChecker_ = std::make_unique<ModuleLoadChecker>();
    }
    MockCheckModuleLoadable(false);
    MockDiskCheckOnly(false);
    MockLoadModuleLibrary(nullptr);

    NativeModule* module = moduleManager->LoadNativeModule(moduleName, nullptr, false, errInfo, false, "path",
        &loadErrInfo);
    EXPECT_EQ(module, nullptr);
    EXPECT_EQ(loadErrInfo, std::string("module ") + moduleName + " is in blocklist");

    GTEST_LOG_(INFO) << "LoadNativeModule_ErrInfo_Blocklisted end";
}

/*
 * @tc.name: LoadNativeModule_ErrInfo_LoadErrInfoNull
 * @tc.desc: test LoadNativeModule does not crash when loadErrInfo is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, LoadNativeModule_ErrInfo_LoadErrInfoNull, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadNativeModule_ErrInfo_LoadErrInfoNull starts";

    std::string errInfo = "";
    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
    // moduleName == nullptr, loadErrInfo defaults to nullptr
    NativeModule* module = moduleManager->LoadNativeModule(nullptr, nullptr, false, errInfo, false, nullptr);
    EXPECT_EQ(module, nullptr);
    EXPECT_EQ(errInfo, "nullptr");

    // relativePath contains "..", loadErrInfo is nullptr
    const char* moduleName = "testModule";
    module = moduleManager->LoadNativeModule(moduleName, nullptr, false, errInfo, false, "../test");
    EXPECT_EQ(module, nullptr);

    GTEST_LOG_(INFO) << "LoadNativeModule_ErrInfo_LoadErrInfoNull end";
}

/*
 * @tc.name: FindNativeModuleByDisk_ErrInfo_ModuleNotFound
 * @tc.desc: test FindNativeModuleByDisk loadErrInfo when system module not found (no dlopen called)
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, FindNativeModuleByDisk_ErrInfo_ModuleNotFound, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindNativeModuleByDisk_ErrInfo_ModuleNotFound starts";

    auto moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);
    MockCheckModuleLoadable(true);

    char nativeModulePath[NATIVE_PATH_NUMBER][NAPI_PATH_MAX];
    nativeModulePath[0][0] = 0;
    nativeModulePath[1][0] = 0;
    nativeModulePath[2][0] = 0;

    std::string errInfo = "";
    std::string loadErrInfo = "";
    // isAppModule=false, all paths empty, no dlopen -> "module not found"
    NativeModule* result = moduleManager->FindNativeModuleByDisk("notExistModule", nullptr, "", false, false,
        errInfo, &loadErrInfo, nativeModulePath, nullptr);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(loadErrInfo, "module not found");

    GTEST_LOG_(INFO) << "FindNativeModuleByDisk_ErrInfo_ModuleNotFound end";
}

/*
 * @tc.name: FindNativeModuleByDisk_ErrInfo_AppLibPathNotRegistered
 * @tc.desc: test FindNativeModuleByDisk loadErrInfo when app lib path not registered in namespace
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, FindNativeModuleByDisk_ErrInfo_AppLibPathNotRegistered, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindNativeModuleByDisk_ErrInfo_AppLibPathNotRegistered starts";

    auto moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);
    MockCheckModuleLoadable(true);

    char nativeModulePath[NATIVE_PATH_NUMBER][NAPI_PATH_MAX];
    nativeModulePath[0][0] = 0;
    nativeModulePath[1][0] = 0;
    nativeModulePath[2][0] = 0;

    std::string errInfo = "";
    std::string loadErrInfo = "";
    // isAppModule=true, path="default" not registered -> "app lib path not registered in namespace 'default'"
    NativeModule* result = moduleManager->FindNativeModuleByDisk("notExistModule", "default", "", false, true,
        errInfo, &loadErrInfo, nativeModulePath, nullptr);
    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(loadErrInfo, "app lib path not registered in namespace 'default'");

    GTEST_LOG_(INFO) << "FindNativeModuleByDisk_ErrInfo_AppLibPathNotRegistered end";
}

/*
 * @tc.name: FindNativeModuleByDisk_ErrInfo_LoadErrInfoNull
 * @tc.desc: test FindNativeModuleByDisk does not crash when loadErrInfo is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, FindNativeModuleByDisk_ErrInfo_LoadErrInfoNull, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindNativeModuleByDisk_ErrInfo_LoadErrInfoNull starts";

    auto moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);
    MockCheckModuleLoadable(true);

    char nativeModulePath[NATIVE_PATH_NUMBER][NAPI_PATH_MAX];
    nativeModulePath[0][0] = 0;
    nativeModulePath[1][0] = 0;
    nativeModulePath[2][0] = 0;

    std::string errInfo = "";
    // loadErrInfo is nullptr, should not crash
    NativeModule* result = moduleManager->FindNativeModuleByDisk("notExistModule", nullptr, "", false, false,
        errInfo, nullptr, nativeModulePath, nullptr);
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "FindNativeModuleByDisk_ErrInfo_LoadErrInfoNull end";
}

/*
 * @tc.name: FindNativeModuleByDisk_ErrInfo_DlopenFailed
 * @tc.desc: test FindNativeModuleByDisk loadErrInfo when dlopen fails on a real invalid file
 * @tc.type: FUNC
 */
HWTEST_F(ModuleManagerTest, FindNativeModuleByDisk_ErrInfo_DlopenFailed, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindNativeModuleByDisk_ErrInfo_DlopenFailed starts";

    auto moduleManager = std::make_shared<NativeModuleManager>();
    ASSERT_NE(nullptr, moduleManager);
    MockCheckModuleLoadable(true);

    // Create a temp file that is not a valid SO, so dlopen will fail
    const char* tempDir = "/tmp";
    std::string tempLibPath = std::string(tempDir) + "/libtestDlopenFail.z.so";
    std::ofstream ofs(tempLibPath);
    ofs << "not a valid so file";
    ofs.close();

    // Set up app lib path so IsExistedPath("default") returns true
    std::vector<std::string> libPaths = { tempDir };
    moduleManager->SetAppLibPath("default", libPaths, false);

    // Construct nativeModulePath[0] pointing to the temp file
    char nativeModulePath[NATIVE_PATH_NUMBER][NAPI_PATH_MAX];
    for (int i = 0; i < NATIVE_PATH_NUMBER; ++i) {
        nativeModulePath[i][0] = '\0';
    }
    size_t copyLen = std::min(tempLibPath.size(), static_cast<size_t>(NAPI_PATH_MAX - 1));
    tempLibPath.copy(nativeModulePath[0], copyLen);
    nativeModulePath[0][copyLen] = '\0';
    EXPECT_EQ(copyLen, tempLibPath.size());

    std::string errInfo = "";
    std::string loadErrInfo = "";
    // isAppModule=true, path="default" registered, file exists but dlopen fails
    NativeModule* result = moduleManager->FindNativeModuleByDisk("testDlopenFail", "default", "", false, true,
        errInfo, &loadErrInfo, nativeModulePath, nullptr);
    EXPECT_EQ(result, nullptr);
    // dlopenFailed should be true, loadErrInfo should contain "dlopen failed"
    EXPECT_NE(loadErrInfo.find("dlopen failed"), std::string::npos);

    // Clean up temp file
    remove(tempLibPath.c_str());

    GTEST_LOG_(INFO) << "FindNativeModuleByDisk_ErrInfo_DlopenFailed end";
}

/*
 * @tc.name: FindNativeModuleByCache_WhenNodeNameIsNullptrShouldNotCrash
 * @tc.desc: test FindNativeModuleByCache does not dereference nullptr when temp->name is nullptr
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, FindNativeModuleByCache_WhenNodeNameIsNullptrShouldNotCrash, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindNativeModuleByCache_WhenNodeNameIsNullptrShouldNotCrash starts";

    NativeModuleManager moduleManager;
    char nativeModulePath[NATIVE_PATH_NUMBER][NAPI_PATH_MAX];
    NativeModule* cacheNativeModule = nullptr;
    NativeModuleHeadTailStruct cacheHeadTailStruct = {nullptr, nullptr, nullptr};

    // Node A: name=nullptr (default), moduleName=nullptr (default), systemFilePath=""
    // Verifies: when temp->name is nullptr, strcasecmp must not be called (avoid null-pointer deref)
    NativeModule* nodeA = new NativeModule();
    ASSERT_NE(nodeA, nullptr);
    nodeA->systemFilePath = "";

    // Node B: name=nullptr (default), moduleName=matched target, systemFilePath=""
    // Verifies: even with a name==nullptr node preceding, moduleName-field match still works
    NativeModule* nodeB = new NativeModule();
    ASSERT_NE(nodeB, nullptr);
    nodeB->moduleName = "test.module.nullptr.target";
    nodeB->systemFilePath = "";
    nodeB->next = nullptr;

    // Chain: A -> B
    nodeA->next = nodeB;
    moduleManager.headNativeModule_ = nodeA;
    moduleManager.tailNativeModule_ = nodeB;

    std::string target = "test.module.nullptr.target";
    // AC-2: list contains name==nullptr node A; moduleName match on B should succeed without crash
    NativeModule* result = moduleManager.FindNativeModuleByCache(target.c_str(), nativeModulePath,
        cacheNativeModule, cacheHeadTailStruct, false);
    EXPECT_EQ(result, nodeB);

    // AC-1: list contains only name==nullptr node A (B unlinked); should not crash, no false match -> nullptr
    nodeA->next = nullptr;
    moduleManager.tailNativeModule_ = nodeA;
    result = moduleManager.FindNativeModuleByCache(target.c_str(), nativeModulePath,
        cacheNativeModule, cacheHeadTailStruct, false);
    EXPECT_EQ(result, nullptr);

    // Cleanup
    delete nodeA;
    nodeA = nullptr;
    delete nodeB;
    nodeB = nullptr;
    moduleManager.headNativeModule_ = nullptr;
    moduleManager.tailNativeModule_ = nullptr;

    GTEST_LOG_(INFO) << "FindNativeModuleByCache_WhenNodeNameIsNullptrShouldNotCrash end";
}

/*
 * @tc.name: FindNativeModuleByCache_WhenNodeSystemFilePathIsNullptrShouldNotCrash
 * @tc.desc: test FindNativeModuleByCache does not dereference nullptr when temp->systemFilePath is nullptr
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, FindNativeModuleByCache_WhenNodeSystemFilePathIsNullptrShouldNotCrash, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindNativeModuleByCache_WhenNodeSystemFilePathIsNullptrShouldNotCrash starts";

    NativeModuleManager moduleManager;
    char nativeModulePath[NATIVE_PATH_NUMBER][NAPI_PATH_MAX];
    NativeModule* cacheNativeModule = nullptr;
    NativeModuleHeadTailStruct cacheHeadTailStruct = {nullptr, nullptr, nullptr};

    // Node X: moduleName matches the query, name=nullptr (keeps outer-if driven by moduleName only),
    // systemFilePath=nullptr (DEFAULT) -> triggers L1637/L1641 nullptr risk once outer-if holds.
    NativeModule* nodeX = new NativeModule();
    ASSERT_NE(nodeX, nullptr);
    nodeX->moduleName = "test.module.syspath.nullptr.target";
    // nodeX->name left as nullptr (default)
    // nodeX->systemFilePath left as nullptr (default) -- this is the bug-trigger condition

    moduleManager.headNativeModule_ = nodeX;
    moduleManager.tailNativeModule_ = nodeX;

    std::string target = "test.module.syspath.nullptr.target";
    // AC-2.1: outer-if holds (moduleName match), systemFilePath==nullptr -> must not crash.
    // Per minimum-fix semantics: nullptr short-circuits while (label stays 0);
    // L1641 if holds via (label < NATIVE_PATH_NUMBER) -> node is treated as path-0 match and returned.
    NativeModule* result = moduleManager.FindNativeModuleByCache(target.c_str(), nativeModulePath,
        cacheNativeModule, cacheHeadTailStruct, false);
    EXPECT_EQ(result, nodeX);

    // AC-2.2: prepend node Y (systemFilePath="" normal match) before X; both have same moduleName.
    // Demonstrates that nullptr-systemFilePath node's presence does not break normal matching flow.
    NativeModule* nodeY = new NativeModule();
    ASSERT_NE(nodeY, nullptr);
    nodeY->moduleName = "test.module.syspath.nullptr.target";
    nodeY->systemFilePath = "";
    nodeY->next = nodeX;
    moduleManager.headNativeModule_ = nodeY;

    result = moduleManager.FindNativeModuleByCache(target.c_str(), nativeModulePath,
        cacheNativeModule, cacheHeadTailStruct, false);
    EXPECT_EQ(result, nodeY);

    // Cleanup
    delete nodeY;
    nodeY = nullptr;
    delete nodeX;
    nodeX = nullptr;
    moduleManager.headNativeModule_ = nullptr;
    moduleManager.tailNativeModule_ = nullptr;

    GTEST_LOG_(INFO) << "FindNativeModuleByCache_WhenNodeSystemFilePathIsNullptrShouldNotCrash end";
}

/*
 * @tc.name: EmplaceModuleBuffer_ShouldReleaseOldBufferOnDuplicateKey
 * @tc.desc: test EmplaceModuleBuffer releases old buffer and overwrites on duplicate moduleKey
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, EmplaceModuleBuffer_ShouldReleaseOldBufferOnDuplicateKey, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EmplaceModuleBuffer_ShouldReleaseOldBufferOnDuplicateKey starts";

    NativeModuleManager moduleManager;
    std::string key = "test.module.emplace.dupkey";

    // Allocate two distinct uint8_t[] buffers as the values to be cached.
    // Use new uint8_t[] so the fix's delete[] semantics match the allocation.
    uint8_t* lib1 = new uint8_t[16];
    ASSERT_NE(lib1, nullptr);
    uint8_t* lib2 = new uint8_t[16];
    ASSERT_NE(lib2, nullptr);

    // AC-3.3: first call -- emplace path. Map size becomes 1, map[key] == lib1.
    moduleManager.EmplaceModuleBuffer(key, lib1);
    EXPECT_EQ(moduleManager.moduleBufMap_.size(), 1u);
    EXPECT_EQ(moduleManager.moduleBufMap_[key], lib1);

    // AC-3.1 + AC-3.2: second call with same key, different buffer.
    // Fix semantics: delete[] old (lib1), replace with new (lib2). Map size stays 1, map[key] == lib2.
    moduleManager.EmplaceModuleBuffer(key, lib2);
    EXPECT_EQ(moduleManager.moduleBufMap_.size(), 1u);
    EXPECT_EQ(moduleManager.moduleBufMap_[key], lib2);

    // Cleanup: lib1 already delete[]-ed by the fix; lib2 is still in the map.
    // Map destructor does NOT free raw pointers, so we must delete[] lib2 manually.
    // To avoid touching the map after asserting, just delete[] lib2 (the current owner).
    delete[] lib2;
    lib2 = nullptr;
    lib1 = nullptr;
    moduleManager.moduleBufMap_.clear();

    GTEST_LOG_(INFO) << "EmplaceModuleBuffer_ShouldReleaseOldBufferOnDuplicateKey end";
}

/*
 * @tc.name: GetNativeModulePath_WhenAppLibPathValueIsNullptrShouldNotCrash
 * @tc.desc: test GetNativeModulePath does not pollute map nor dereference nullptr when appLibPathMap_ value is null
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, GetNativeModulePath_WhenAppLibPathValueIsNullptrShouldNotCrash, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNativeModulePath_WhenAppLibPathValueIsNullptrShouldNotCrash starts";

    NativeModuleManager moduleManager;
    char nativeModulePath[NATIVE_PATH_NUMBER][NAPI_PATH_MAX];
    const char* moduleName = "somemodule";

    // AC-4.1: appLibPathMap_ has key with nullptr value.
    // Original buggy code: prefix = appLibPathMap_[path] returns nullptr -> enters if-branch ->
    // sprintf_s("%s/...", nullptr, ...) on non-Android -> crash.
    // Fixed code: prefix != nullptr short-circuits to fallback branch, no crash, no map pollution.
    const std::string keyNull = "test.app.path.nullptr";
    moduleManager.appLibPathMap_[keyNull] = nullptr;
    size_t sizeBefore = moduleManager.appLibPathMap_.size();
    bool ret = moduleManager.GetNativeModulePath(moduleName, keyNull.c_str(), "", true,
        nativeModulePath, NAPI_PATH_MAX);
    EXPECT_TRUE(ret);
    EXPECT_EQ(moduleManager.appLibPathMap_.size(), sizeBefore); // map not polluted
    // nativeModulePath[0] populated via fallback branch (prefix = sysPrefix).
    EXPECT_GT(strlen(nativeModulePath[0]), 0u);

    // AC-4.2: key does not exist in map. Original operator[] would insert "key -> nullptr" pollution.
    // Fixed find-based code does not insert.
    moduleManager.appLibPathMap_.clear();
    EXPECT_EQ(moduleManager.appLibPathMap_.size(), 0u);
    const std::string keyMissing = "test.app.path.nonexistent";
    ret = moduleManager.GetNativeModulePath(moduleName, keyMissing.c_str(), "", true,
        nativeModulePath, NAPI_PATH_MAX);
    EXPECT_TRUE(ret);
    EXPECT_EQ(moduleManager.appLibPathMap_.size(), 0u); // not polluted by operator[]

    // AC-4.3: key exists with non-empty value. Behavior unchanged: prefix taken from map,
    // enters if-branch (prefix != nullptr), nativeModulePath[0] starts with injected prefix.
    const std::string keyValid = "test.app.path.valid";
    const std::string injectedPrefix = "/some/valid/prefix";
    moduleManager.appLibPathMap_[keyValid] = const_cast<char*>(injectedPrefix.c_str());
    ret = moduleManager.GetNativeModulePath(moduleName, keyValid.c_str(), "", true,
        nativeModulePath, NAPI_PATH_MAX);
    EXPECT_TRUE(ret);
    EXPECT_NE(strstr(nativeModulePath[0], injectedPrefix.c_str()), nullptr);

    // Cleanup
    moduleManager.appLibPathMap_.clear();

    GTEST_LOG_(INFO) << "GetNativeModulePath_WhenAppLibPathValueIsNullptrShouldNotCrash end";
}

/*
 * @tc.name: SetAppLibPath_WhenAppLibPathIsEmptyShouldNotCrash
 * @tc.desc: test SetAppLibPath does not dereference empty string back() when appLibPath is empty or all-empty
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, SetAppLibPath_WhenAppLibPathIsEmptyShouldNotCrash, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetAppLibPath_WhenAppLibPathIsEmptyShouldNotCrash starts";

    NativeModuleManager moduleManager;

    // AC-5.1: empty vector. Original buggy code: tmpPath stays "" -> tmpPath.back() is UB (likely crash).
    // Fixed code: short-circuit via !tmpPath.empty(), no back() call, no crash.
    std::vector<std::string> emptyVec;
    moduleManager.SetAppLibPath("test.module.empty_vector", emptyVec, false);
    // Reaching this assertion means no crash occurred.
    SUCCEED();

    // AC-5.2: all elements are empty strings. Loop skips all -> tmpPath stays "" -> same UB on buggy code.
    std::vector<std::string> allEmptyStringsVec = {"", "", ""};
    moduleManager.SetAppLibPath("test.module.all_empty_strings", allEmptyStringsVec, false);
    SUCCEED();

    // AC-5.3: at least one non-empty element. Behavior unchanged: tmpPath = "/path1:/path2:" -> pop_back -> "/path1:/path2".
    std::vector<std::string> normalVec = {"/path1", "/path2"};
    moduleManager.SetAppLibPath("test.module.normal", normalVec, false);
    auto it = moduleManager.appLibPathMap_.find("test.module.normal");
    ASSERT_NE(it, moduleManager.appLibPathMap_.end());
    ASSERT_NE(it->second, nullptr);
    EXPECT_EQ(std::string(it->second), "/path1:/path2");

    // Cleanup: each char* was strdup-allocated on heap; free before clear to avoid test leak.
    for (auto& entry : moduleManager.appLibPathMap_) {
        if (entry.second != nullptr) {
            free(entry.second);
            entry.second = nullptr;
        }
    }
    moduleManager.appLibPathMap_.clear();

    GTEST_LOG_(INFO) << "SetAppLibPath_WhenAppLibPathIsEmptyShouldNotCrash end";
}

/*
 * @tc.name: IsSafeRelativePath_PathValidationTest
 * @tc.desc: test NativeModuleManager::IsSafeRelativePath validates relativePath via whitelist (no leading slash,
 *           non-empty/non-"."/non-".." segments, alnum/_/./ chars only)
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, IsSafeRelativePath_PathValidationTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsSafeRelativePath_PathValidationTest starts";

    // AC-6.1: ".." as substring but not as full segment -> accept (eliminates original false positive)
    EXPECT_TRUE(NativeModuleManager::IsSafeRelativePath("my..pkg"));
    EXPECT_TRUE(NativeModuleManager::IsSafeRelativePath("a..b/c"));
    EXPECT_TRUE(NativeModuleManager::IsSafeRelativePath("com.example..foo"));

    // AC-6.2: absolute path -> reject
    EXPECT_FALSE(NativeModuleManager::IsSafeRelativePath("/etc/passwd"));
    EXPECT_FALSE(NativeModuleManager::IsSafeRelativePath("/"));
    EXPECT_FALSE(NativeModuleManager::IsSafeRelativePath("/module"));

    // AC-6.3: segment is . / .. / empty -> reject
    EXPECT_FALSE(NativeModuleManager::IsSafeRelativePath("a/./b"));
    EXPECT_FALSE(NativeModuleManager::IsSafeRelativePath("a/../b"));
    EXPECT_FALSE(NativeModuleManager::IsSafeRelativePath("a//b"));
    EXPECT_FALSE(NativeModuleManager::IsSafeRelativePath("."));
    EXPECT_FALSE(NativeModuleManager::IsSafeRelativePath(".."));

    // AC-6.4: non-whitelisted char -> reject
    EXPECT_FALSE(NativeModuleManager::IsSafeRelativePath("a;b"));
    EXPECT_FALSE(NativeModuleManager::IsSafeRelativePath("a$b"));
    EXPECT_FALSE(NativeModuleManager::IsSafeRelativePath("%2e"));
    EXPECT_FALSE(NativeModuleManager::IsSafeRelativePath("a\\b")); // backslash
    EXPECT_FALSE(NativeModuleManager::IsSafeRelativePath("a b"));  // space

    // AC-6.5: legitimate relative path -> accept (backward compat)
    EXPECT_TRUE(NativeModuleManager::IsSafeRelativePath(""));
    EXPECT_TRUE(NativeModuleManager::IsSafeRelativePath("module"));
    EXPECT_TRUE(NativeModuleManager::IsSafeRelativePath("module/sub"));
    EXPECT_TRUE(NativeModuleManager::IsSafeRelativePath("a.b.c"));
    EXPECT_TRUE(NativeModuleManager::IsSafeRelativePath("com.example.foo"));
    EXPECT_TRUE(NativeModuleManager::IsSafeRelativePath("module_name"));

    GTEST_LOG_(INFO) << "IsSafeRelativePath_PathValidationTest end";
}

/*
 * @tc.name: IsValidLibNameStrict_LibNameValidationTest
 * @tc.desc: test NativeModuleManager::IsValidLibNameStrict validates lib name via strict charset
 *           (alnum/_/-/. only, must end with .so, no "..." sequence, size < NAME_MAX)
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, IsValidLibNameStrict_LibNameValidationTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsValidLibNameStrict_LibNameValidationTest starts";

    // AC-7.1: contains colon -> reject (namespace injection vector via dlns_inherit ":")
    EXPECT_FALSE(NativeModuleManager::IsValidLibNameStrict("liba:b.so"));

    // AC-7.2: contains space/slash/backslash/non-whitelist char -> reject
    EXPECT_FALSE(NativeModuleManager::IsValidLibNameStrict("lib a.so"));
    EXPECT_FALSE(NativeModuleManager::IsValidLibNameStrict("lib/a.so"));
    EXPECT_FALSE(NativeModuleManager::IsValidLibNameStrict("lib\\a.so"));
    EXPECT_FALSE(NativeModuleManager::IsValidLibNameStrict("lib$a.so"));
    EXPECT_FALSE(NativeModuleManager::IsValidLibNameStrict("lib%a.so"));
    EXPECT_FALSE(NativeModuleManager::IsValidLibNameStrict("lib:a.so"));   // dup safety with AC-7.1

    // AC-7.3: contains "..." sequence -> reject (path-traversal-style tricks)
    EXPECT_FALSE(NativeModuleManager::IsValidLibNameStrict("lib...so"));
    EXPECT_FALSE(NativeModuleManager::IsValidLibNameStrict("a...b.so"));

    // AC-7.4: does not end with .so -> reject
    EXPECT_FALSE(NativeModuleManager::IsValidLibNameStrict("libfoo"));
    EXPECT_FALSE(NativeModuleManager::IsValidLibNameStrict("libfoo.txt"));
    EXPECT_FALSE(NativeModuleManager::IsValidLibNameStrict("lib.so.txt"));

    // AC-7.5: empty or oversized (>= NAME_MAX) -> reject
    EXPECT_FALSE(NativeModuleManager::IsValidLibNameStrict(""));
    std::string oversized(NAME_MAX, 'a');   // length == NAME_MAX -> reject
    EXPECT_FALSE(NativeModuleManager::IsValidLibNameStrict(oversized));

    // AC-7.6: legitimate names -> accept (backward compat)
    EXPECT_TRUE(NativeModuleManager::IsValidLibNameStrict("libfoo.so"));
    EXPECT_TRUE(NativeModuleManager::IsValidLibNameStrict("lib_foo-bar.so"));
    EXPECT_TRUE(NativeModuleManager::IsValidLibNameStrict("lib123.so"));
    EXPECT_TRUE(NativeModuleManager::IsValidLibNameStrict("a.so"));
    EXPECT_TRUE(NativeModuleManager::IsValidLibNameStrict("lib.so"));   // boundary: just ".so" suffix

    GTEST_LOG_(INFO) << "IsValidLibNameStrict_LibNameValidationTest end";
}

/**
 * @tc.name: UnloadNativeModule_WhenModuleNotInLibMapShouldReturnFalseSafely
 * @tc.desc: 验证 UnloadNativeModule 在 moduleLibMap_ 不存在 key 时安全早退（TOCTOU 加固后的 find==end() 路径）
 * @tc.type: FUNC
 * @tc.require: issue #2157 问题 #8
 */
HWTEST_F(ModuleManagerTest, UnloadNativeModule_WhenModuleNotInLibMapShouldReturnFalseSafely, TestSize.Level1)
{
    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
    // moduleLibMap_ 默认为空（SetUp 不主动填充），调用应走双锁临界区的 find == end() 早退路径，
    // 返回 false 且不触发 dlclose（不会因悬空指针崩溃）。
    bool result = moduleManager->UnloadNativeModule("test_unload_no_exist_key");
    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "UnloadNativeModule_WhenModuleNotInLibMapShouldReturnFalseSafely end";
}

/**
 * @tc.name: EmplaceModuleLib_ShouldNotEmplaceWhenLibIsNullptr
 * @tc.desc: 验证 EmplaceModuleLib 在 lib == nullptr 时早退，不触碰 map（AC-9.1）
 * @tc.type: FUNC
 * @tc.require: issue #2157 问题 #9
 */
HWTEST_F(ModuleManagerTest, EmplaceModuleLib_ShouldNotEmplaceWhenLibIsNullptr, TestSize.Level1)
{
    NativeModuleManager& mgr = NativeModuleManager::GetInstance();
    size_t before = mgr.moduleLibMap_.size();
    mgr.EmplaceModuleLib("test_emplace_nullptr_key_9", nullptr);
    EXPECT_EQ(mgr.moduleLibMap_.size(), before);
    EXPECT_EQ(mgr.moduleLibMap_.count("test_emplace_nullptr_key_9"), 0u);

    GTEST_LOG_(INFO) << "EmplaceModuleLib_ShouldNotEmplaceWhenLibIsNullptr end";
}

/**
 * @tc.name: EmplaceModuleLib_ShouldEmplaceWhenKeyNotExists
 * @tc.desc: 验证 EmplaceModuleLib 在 key 不存在时 emplace 入库（AC-9.2）
 * @tc.type: FUNC
 * @tc.require: issue #2157 问题 #9
 */
HWTEST_F(ModuleManagerTest, EmplaceModuleLib_ShouldEmplaceWhenKeyNotExists, TestSize.Level1)
{
    NativeModuleManager& mgr = NativeModuleManager::GetInstance();
    LIBHANDLE sentinel = reinterpret_cast<LIBHANDLE>(0xDEADBEEF);
    mgr.EmplaceModuleLib("test_emplace_new_key_9", sentinel);
    EXPECT_EQ(mgr.moduleLibMap_.count("test_emplace_new_key_9"), 1u);
    EXPECT_EQ(mgr.moduleLibMap_["test_emplace_new_key_9"], sentinel);
    // sentinel 不是真实 dlopen 句柄，不能 dlclose，手动 erase 清理避免污染后续测试
    mgr.moduleLibMap_.erase("test_emplace_new_key_9");

    GTEST_LOG_(INFO) << "EmplaceModuleLib_ShouldEmplaceWhenKeyNotExists end";
}

namespace {
constexpr char GET_FILE_BUFFER_TEST_DIR[] = "/data/local/tmp";
constexpr char GET_FILE_BUFFER_TEST_FILE[] = "/data/local/tmp/napi_getfilebuffer_test_11.bin";
constexpr char GET_FILE_BUFFER_TEST_KEY[] = "test.getfilebuffer.11";

// Helper：写入指定字节的测试文件。返回是否成功。
bool WriteTestFile(const std::string& path, const std::vector<uint8_t>& bytes)
{
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }
    if (!bytes.empty()) {
        out.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    }
    out.close();
    return out.good() || bytes.empty();
}

// Helper，清理测试产物与缓存（缓存中的 raw 指针需手动 delete[]）。
void CleanupModuleBuffer(NativeModuleManager& mgr, const std::string& key)
{
    auto it = mgr.moduleBufMap_.find(key);
    if (it != mgr.moduleBufMap_.end()) {
        delete[] it->second;
        mgr.moduleBufMap_.erase(it);
    }
}
} // namespace

/**
 * @tc.name: GetFileBuffer_ShouldReturnBufferAndMatchLenForNormalFile
 * @tc.desc: 正常 ABC 文件应返回非空 buffer，len 等于文件大小，内容一致（AC-11.1/11.3）
 * @tc.type: FUNC
 * @tc.require: issue #2157 问题 #11
 */
HWTEST_F(ModuleManagerTest, GetFileBuffer_ShouldReturnBufferAndMatchLenForNormalFile, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileBuffer_ShouldReturnBufferAndMatchLenForNormalFile starts";

    NativeModuleManager moduleManager;
    std::vector<uint8_t> payload = {0x41, 0x42, 0x43, 0x44, 0x45}; // "ABCDE"
    ASSERT_TRUE(WriteTestFile(GET_FILE_BUFFER_TEST_FILE, payload));

    size_t len = 0;
    const uint8_t* buf = moduleManager.GetFileBuffer(GET_FILE_BUFFER_TEST_FILE, GET_FILE_BUFFER_TEST_KEY, len);
    EXPECT_NE(buf, nullptr);
    EXPECT_EQ(len, payload.size());
    if (buf != nullptr && len == payload.size()) {
        EXPECT_EQ(memcmp(buf, payload.data(), len), 0);
    }
    CleanupModuleBuffer(moduleManager, GET_FILE_BUFFER_TEST_KEY);
    std::remove(GET_FILE_BUFFER_TEST_FILE);

    GTEST_LOG_(INFO) << "GetFileBuffer_ShouldReturnBufferAndMatchLenForNormalFile end";
}

/**
 * @tc.name: GetFileBuffer_ShouldReturnSamePointerOnSecondCallFromCache
 * @tc.desc: 同一 moduleKey 二次调用应命中缓存返回相同指针，且不再读取文件（AC-11.4）
 * @tc.type: FUNC
 * @tc.require: issue #2157 问题 #11
 */
HWTEST_F(ModuleManagerTest, GetFileBuffer_ShouldReturnSamePointerOnSecondCallFromCache, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileBuffer_ShouldReturnSamePointerOnSecondCallFromCache starts";

    NativeModuleManager moduleManager;
    std::vector<uint8_t> payload = {0x10, 0x20, 0x30};
    ASSERT_TRUE(WriteTestFile(GET_FILE_BUFFER_TEST_FILE, payload));

    size_t len1 = 0;
    const uint8_t* buf1 = moduleManager.GetFileBuffer(GET_FILE_BUFFER_TEST_FILE, GET_FILE_BUFFER_TEST_KEY, len1);
    ASSERT_NE(buf1, nullptr);

    // 删除源文件以确保二次调用不依赖磁盘读取
    std::remove(GET_FILE_BUFFER_TEST_FILE);

    size_t len2 = 0;
    const uint8_t* buf2 = moduleManager.GetFileBuffer(GET_FILE_BUFFER_TEST_FILE, GET_FILE_BUFFER_TEST_KEY, len2);
    EXPECT_EQ(buf2, buf1);
    EXPECT_EQ(len2, len1);

    CleanupModuleBuffer(moduleManager, GET_FILE_BUFFER_TEST_KEY);

    GTEST_LOG_(INFO) << "GetFileBuffer_ShouldReturnSamePointerOnSecondCallFromCache end";
}

/**
 * @tc.name: GetFileBuffer_ShouldReturnNullptrWhenFileDoesNotExist
 * @tc.desc: 文件不存在(is_open 失败)时应返回 nullptr，len 保持 0（AC-11.2）
 * @tc.type: FUNC
 * @tc.require: issue #2157 问题 #11
 */
HWTEST_F(ModuleManagerTest, GetFileBuffer_ShouldReturnNullptrWhenFileDoesNotExist, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileBuffer_ShouldReturnNullptrWhenFileDoesNotExist starts";

    NativeModuleManager moduleManager;
    std::string nonExisting = std::string(GET_FILE_BUFFER_TEST_DIR) + "/napi_not_exist_11.bin";
    std::remove(nonExisting.c_str());

    size_t len = 1; // 故意非 0，校验函数未写出
    const uint8_t* buf = moduleManager.GetFileBuffer(nonExisting, "test.getfilebuffer.notexist", len);
    EXPECT_EQ(buf, nullptr);

    GTEST_LOG_(INFO) << "GetFileBuffer_ShouldReturnNullptrWhenFileDoesNotExist end";
}

/**
 * @tc.name: GetFileBuffer_ShouldReturnNullptrForEmptyFile
 * @tc.desc: 空文件 tellg() 返回 0，按修复后逻辑直接拒绝，返回 nullptr（AC-11.5 fileSize==0 早退）
 * @tc.type: FUNC
 * @tc.require: issue #2157 问题 #11
 */
HWTEST_F(ModuleManagerTest, GetFileBuffer_ShouldReturnNullptrForEmptyFile, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileBuffer_ShouldReturnNullptrForEmptyFile starts";

    NativeModuleManager moduleManager;
    ASSERT_TRUE(WriteTestFile(GET_FILE_BUFFER_TEST_FILE, {}));

    size_t len = 1; // 故意非 0，校验函数清零
    const uint8_t* buf = moduleManager.GetFileBuffer(GET_FILE_BUFFER_TEST_FILE, GET_FILE_BUFFER_TEST_KEY, len);
    EXPECT_EQ(buf, nullptr);
    EXPECT_EQ(len, 0u);
    EXPECT_EQ(moduleManager.moduleBufMap_.count(GET_FILE_BUFFER_TEST_KEY), 0u);
    std::remove(GET_FILE_BUFFER_TEST_FILE);

    GTEST_LOG_(INFO) << "GetFileBuffer_ShouldReturnNullptrForEmptyFile end";
}

/**
 * @tc.name: CreateHeadNativeModule_ShouldInitHeadAndTailWhenListEmpty
 * @tc.desc: 空链表首次调用 CreateHeadNativeModule 应返回 true，head/tail 同指向新节点（AC-12.1）
 * @tc.type: FUNC
 * @tc.require: issue #2157 问题 #12
 */
HWTEST_F(ModuleManagerTest, CreateHeadNativeModule_ShouldInitHeadAndTailWhenListEmpty, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateHeadNativeModule_ShouldInitHeadAndTailWhenListEmpty starts";

    NativeModuleManager moduleManager;
    ASSERT_EQ(moduleManager.headNativeModule_, nullptr);
    ASSERT_EQ(moduleManager.tailNativeModule_, nullptr);

    EXPECT_TRUE(moduleManager.CreateHeadNativeModule());
    EXPECT_NE(moduleManager.headNativeModule_, nullptr);
    EXPECT_EQ(moduleManager.headNativeModule_, moduleManager.tailNativeModule_);
    EXPECT_EQ(moduleManager.headNativeModule_->next, nullptr);

    GTEST_LOG_(INFO) << "CreateHeadNativeModule_ShouldInitHeadAndTailWhenListEmpty end";
}

/**
 * @tc.name: CreateHeadNativeModule_ShouldPrependNewHeadWhenListExists
 * @tc.desc: 已有节点时再次调用应前插新 head，旧 head 串到 next（AC-12.2）
 * @tc.type: FUNC
 * @tc.require: issue #2157 问题 #12
 */
HWTEST_F(ModuleManagerTest, CreateHeadNativeModule_ShouldPrependNewHeadWhenListExists, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateHeadNativeModule_ShouldPrependNewHeadWhenListExists starts";

    NativeModuleManager moduleManager;
    ASSERT_TRUE(moduleManager.CreateHeadNativeModule());
    NativeModule* oldHead = moduleManager.headNativeModule_;
    ASSERT_NE(oldHead, nullptr);

    EXPECT_TRUE(moduleManager.CreateHeadNativeModule());
    EXPECT_NE(moduleManager.headNativeModule_, oldHead);
    EXPECT_EQ(moduleManager.headNativeModule_->next, oldHead);
    EXPECT_EQ(moduleManager.tailNativeModule_, oldHead);

    GTEST_LOG_(INFO) << "CreateHeadNativeModule_ShouldPrependNewHeadWhenListExists end";
}

/**
 * @tc.name: CreateTailNativeModule_ShouldInitHeadAndTailWhenListEmpty
 * @tc.desc: 空链表首次调用 CreateTailNativeModule 应返回 true，head/tail 同指向新节点（AC-12.3）
 * @tc.type: FUNC
 * @tc.require: issue #2157 问题 #12
 */
HWTEST_F(ModuleManagerTest, CreateTailNativeModule_ShouldInitHeadAndTailWhenListEmpty, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateTailNativeModule_ShouldInitHeadAndTailWhenListEmpty starts";

    NativeModuleManager moduleManager;
    ASSERT_EQ(moduleManager.headNativeModule_, nullptr);
    ASSERT_EQ(moduleManager.tailNativeModule_, nullptr);

    EXPECT_TRUE(moduleManager.CreateTailNativeModule());
    EXPECT_NE(moduleManager.tailNativeModule_, nullptr);
    EXPECT_EQ(moduleManager.headNativeModule_, moduleManager.tailNativeModule_);
    EXPECT_EQ(moduleManager.tailNativeModule_->next, nullptr);

    GTEST_LOG_(INFO) << "CreateTailNativeModule_ShouldInitHeadAndTailWhenListEmpty end";
}

/**
 * @tc.name: CreateTailNativeModule_ShouldAppendNewTailWhenListExists
 * @tc.desc: 已有节点时再次调用应追加新 tail，旧 tail->next 指向新 tail（AC-12.4）
 * @tc.type: FUNC
 * @tc.require: issue #2157 问题 #12
 */
HWTEST_F(ModuleManagerTest, CreateTailNativeModule_ShouldAppendNewTailWhenListExists, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateTailNativeModule_ShouldAppendNewTailWhenListExists starts";

    NativeModuleManager moduleManager;
    ASSERT_TRUE(moduleManager.CreateTailNativeModule());
    NativeModule* oldTail = moduleManager.tailNativeModule_;
    ASSERT_NE(oldTail, nullptr);

    EXPECT_TRUE(moduleManager.CreateTailNativeModule());
    EXPECT_NE(moduleManager.tailNativeModule_, oldTail);
    EXPECT_EQ(oldTail->next, moduleManager.tailNativeModule_);
    EXPECT_EQ(moduleManager.tailNativeModule_->next, nullptr);
    EXPECT_EQ(moduleManager.headNativeModule_, oldTail);

    GTEST_LOG_(INFO) << "CreateTailNativeModule_ShouldAppendNewTailWhenListExists end";
}

/**
 * @tc.name: GetInstance_ShouldReturnStablePointerAcrossCalls
 * @tc.desc: 同一线程多次调用 GetInstance 应返回相同指针（AC-16.1）
 * @tc.type: FUNC
 * @tc.require: issue #2157 问题 #16
 */
HWTEST_F(ModuleManagerTest, GetInstance_ShouldReturnStablePointerAcrossCalls, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstance_ShouldReturnStablePointerAcrossCalls starts";

    NativeModuleManager* p1 = NativeModuleManager::GetInstance();
    NativeModuleManager* p2 = NativeModuleManager::GetInstance();
    EXPECT_NE(p1, nullptr);
    EXPECT_EQ(p1, p2);

    GTEST_LOG_(INFO) << "GetInstance_ShouldReturnStablePointerAcrossCalls end";
}

/**
 * @tc.name: GetInstance_ShouldReturnSamePointerUnderConcurrency
 * @tc.desc: 多线程并发首次/重复调用 GetInstance 都应返回相同指针（AC-16.2 回归保护）
 * @tc.type: FUNC
 * @tc.require: issue #2157 问题 #16
 */
HWTEST_F(ModuleManagerTest, GetInstance_ShouldReturnSamePointerUnderConcurrency, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstance_ShouldReturnSamePointerUnderConcurrency starts";

    constexpr int threadCount = 16;
    constexpr int iterPerThread = 200;
    std::vector<NativeModuleManager*> results(threadCount, nullptr);
    std::vector<std::thread> threads;
    threads.reserve(threadCount);

    for (int i = 0; i < threadCount; ++i) {
        threads.emplace_back([&results, i]() {
            NativeModuleManager* last = nullptr;
            for (int j = 0; j < iterPerThread; ++j) {
                NativeModuleManager* cur = NativeModuleManager::GetInstance();
                ASSERT_NE(cur, nullptr);
                if (last != nullptr) {
                    ASSERT_EQ(cur, last);
                }
                last = cur;
            }
            results[i] = last;
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    // 所有线程看到的指针必须一致
    NativeModuleManager* expected = results[0];
    EXPECT_NE(expected, nullptr);
    for (int i = 1; i < threadCount; ++i) {
        EXPECT_EQ(results[i], expected) << "thread " << i << " saw different instance pointer";
    }

    GTEST_LOG_(INFO) << "GetInstance_ShouldReturnSamePointerUnderConcurrency end";
}