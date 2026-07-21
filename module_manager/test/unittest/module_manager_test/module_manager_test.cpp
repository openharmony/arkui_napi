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
 * @tc.desc: Destructor safely clears nativeEngineList_ (AC-17.1 swap + outside-lock delete)
 * @tc.type: FUNC
 * @tc.require: issue #2157
 */
HWTEST_F(ModuleManagerTest, Destructor_ShouldClearNativeEngineListSafely, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Destructor_ShouldClearNativeEngineListSafely starts";
    {
        NativeModuleManager moduleManager;
        moduleManager.nativeEngineList_.emplace("engine.k1", nullptr);
        moduleManager.nativeEngineList_.emplace("engine.k2", nullptr);
        moduleManager.nativeEngineList_.emplace("engine.k3", nullptr);
        EXPECT_EQ(moduleManager.nativeEngineList_.size(), 3u);
    }
    GTEST_LOG_(INFO) << "Destructor_ShouldClearNativeEngineListSafely end";
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
 * @tc.name: EmplaceModuleBuffer_ShouldKeepOriginalBufferOnDuplicateKey
 * @tc.desc: test EmplaceModuleBuffer is a no-op on duplicate moduleKey: the original buffer is
 *          preserved (map is an index, buffer ownership lives in NativeModule::jsABCCode).
 *          Replacing the map entry would free a buffer still referenced by another module.
 * @tc.type: FUNC
 * @tc.require: #I76XTV
 */
HWTEST_F(ModuleManagerTest, EmplaceModuleBuffer_ShouldKeepOriginalBufferOnDuplicateKey, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EmplaceModuleBuffer_ShouldKeepOriginalBufferOnDuplicateKey starts";

    NativeModuleManager moduleManager;
    std::string key = "test.module.emplace.dupkey";

    uint8_t* lib1 = new uint8_t[16];
    ASSERT_NE(lib1, nullptr);
    uint8_t* lib2 = new uint8_t[16];
    ASSERT_NE(lib2, nullptr);

    // First call -- emplace path. Map size becomes 1, map[key] == lib1.
    moduleManager.EmplaceModuleBuffer(key, lib1);
    EXPECT_EQ(moduleManager.moduleBufMap_.size(), 1u);
    EXPECT_EQ(moduleManager.moduleBufMap_[key], lib1);

    // Second call with same key: no-op (std::map::emplace refuses duplicate key).
    // Map keeps lib1; lib2 is the caller's responsibility (would be assigned to a
    // different NativeModule::jsABCCode by GetFileBuffer in production).
    moduleManager.EmplaceModuleBuffer(key, lib2);
    EXPECT_EQ(moduleManager.moduleBufMap_.size(), 1u);
    EXPECT_EQ(moduleManager.moduleBufMap_[key], lib1);

    // Cleanup: both buffers are owned by the test (map is a non-owning index).
    delete[] lib1;
    delete[] lib2;
    moduleManager.moduleBufMap_.clear();

    GTEST_LOG_(INFO) << "EmplaceModuleBuffer_ShouldKeepOriginalBufferOnDuplicateKey end";
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
    // sprintf_s("%s/...", nullptr, ...) on non-OH-platform -> crash.
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

    // Note: prefix from appLibPathMap_ is only consumed inside the mobile
    // cross-platform branch (sysAbcPrefix rewrite). On the Linux UT env the
    // value is read but not appended to nativeModulePath[0], so we cannot
    // assert prefix visibility here.

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

    // AC-5.3: at least one non-empty element. Behavior unchanged:
    // tmpPath = "/path1:/path2:" -> pop_back -> "/path1:/path2".
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

    // AC-7.7: names shorter than ".so" suffix must not underflow substr()/compare()
    // (greylist data is untrusted — size() < 3 used to throw std::out_of_range)
    EXPECT_FALSE(NativeModuleManager::IsValidLibNameStrict("a"));
    EXPECT_FALSE(NativeModuleManager::IsValidLibNameStrict("ab"));
    EXPECT_FALSE(NativeModuleManager::IsValidLibNameStrict(".so"));   // size == 3, no name stem
    EXPECT_FALSE(NativeModuleManager::IsValidLibNameStrict("x.s"));    // size == 3, wrong suffix

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
 * @tc.desc: UnloadNativeModule returns false safely when key not in moduleLibMap_ (TOCTOU-hardened find==end() path)
 * @tc.type: FUNC
 * @tc.require: issue #2157
 */
HWTEST_F(ModuleManagerTest, UnloadNativeModule_WhenModuleNotInLibMapShouldReturnFalseSafely, TestSize.Level1)
{
    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
    bool result = moduleManager->UnloadNativeModule("test_unload_no_exist_key");
    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "UnloadNativeModule_WhenModuleNotInLibMapShouldReturnFalseSafely end";
}

/**
 * @tc.name: EmplaceModuleLib_ShouldNotEmplaceWhenLibIsNullptr
 * @tc.desc: EmplaceModuleLib early-returns when lib is nullptr, leaving the map untouched (AC-9.1)
 * @tc.type: FUNC
 * @tc.require: issue #2157
 */
HWTEST_F(ModuleManagerTest, EmplaceModuleLib_ShouldNotEmplaceWhenLibIsNullptr, TestSize.Level1)
{
    // Use a stack instance to avoid contaminating the global singleton's moduleLibMap_.
    NativeModuleManager mgr;
    size_t before = mgr.moduleLibMap_.size();
    mgr.EmplaceModuleLib("test_emplace_nullptr_key_9", nullptr);
    EXPECT_EQ(mgr.moduleLibMap_.size(), before);
    EXPECT_EQ(mgr.moduleLibMap_.count("test_emplace_nullptr_key_9"), 0u);

    GTEST_LOG_(INFO) << "EmplaceModuleLib_ShouldNotEmplaceWhenLibIsNullptr end";
}

/**
 * @tc.name: EmplaceModuleLib_ShouldEmplaceWhenKeyNotExists
 * @tc.desc: EmplaceModuleLib emplaces a new entry when the key is absent (AC-9.2)
 * @tc.type: FUNC
 * @tc.require: issue #2157
 */
HWTEST_F(ModuleManagerTest, EmplaceModuleLib_ShouldEmplaceWhenKeyNotExists, TestSize.Level1)
{
    // Use a stack instance so the sentinel pointer never leaks into the global singleton.
    NativeModuleManager mgr;
    LIBHANDLE sentinel = reinterpret_cast<LIBHANDLE>(0xDEADBEEF);
    mgr.EmplaceModuleLib("test_emplace_new_key_9", sentinel);
    EXPECT_EQ(mgr.moduleLibMap_.count("test_emplace_new_key_9"), 1u);
    EXPECT_EQ(mgr.moduleLibMap_["test_emplace_new_key_9"], sentinel);

    GTEST_LOG_(INFO) << "EmplaceModuleLib_ShouldEmplaceWhenKeyNotExists end";
}

/**
 * @tc.name: EmplaceModuleLib_ShouldReturnCanonicalHandleOnDuplicateKey
 * @tc.desc: EmplaceModuleLib returns the canonical (existing) handle on duplicate key so
 *          LoadModuleLibrary's caller never dereferences a freed handle. The new duplicate
 *          handle is not tracked (caller's responsibility) and the map keeps the first one.
 * @tc.type: FUNC
 * @tc.require: issue #2157
 */
HWTEST_F(ModuleManagerTest, EmplaceModuleLib_ShouldReturnCanonicalHandleOnDuplicateKey, TestSize.Level1)
{
    // Use a stack instance so the sentinel pointers never leak into the global singleton.
    NativeModuleManager mgr;
    LIBHANDLE first = reinterpret_cast<LIBHANDLE>(0xCAFEBABE);
    LIBHANDLE second = reinterpret_cast<LIBHANDLE>(0xBADDCAFE);
    const std::string key = "test_emplace_dup_key_9";

    // Seed the map with the canonical handle.
    mgr.EmplaceModuleLib(key, first);
    ASSERT_EQ(mgr.moduleLibMap_.count(key), 1u);

    // Duplicate key: function must return the canonical (first) handle, not the duplicate.
    // After revert 61746b9d, production code no longer calls UnloadModuleLibrary(second)
    // (the duplicate handle is leaked to avoid dlclose refcount risk); the map keeps the
    // first handle. The stack instance's destructor does not iterate moduleLibMap_, so the
    // sentinel pointers simply leak in the test process -- no dlclose contamination.
    LIBHANDLE returned = mgr.EmplaceModuleLib(key, second);
    EXPECT_EQ(returned, first);
    EXPECT_EQ(mgr.moduleLibMap_[key], first);

    GTEST_LOG_(INFO) << "EmplaceModuleLib_ShouldReturnCanonicalHandleOnDuplicateKey end";
}

namespace {
constexpr char GET_FILE_BUFFER_TEST_DIR[] = "/data/local/tmp";
constexpr char GET_FILE_BUFFER_TEST_FILE[] = "/data/local/tmp/napi_getfilebuffer_test_11.bin";
constexpr char GET_FILE_BUFFER_TEST_KEY[] = "test.getfilebuffer.11";

// Write a test file with the given bytes. Returns true on success.
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

// Erase and delete[] a cached buffer (map owns raw pointers).
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
 * @tc.desc: GetFileBuffer returns non-null buffer with len matching file size and content (AC-11.1/11.3)
 * @tc.type: FUNC
 * @tc.require: issue #2157
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
 * @tc.desc: Second call with same moduleKey hits cache and returns the same pointer (AC-11.4)
 * @tc.type: FUNC
 * @tc.require: issue #2157
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
    // Snapshot original contents to prove cache returns old bytes even after file rewrite.
    std::vector<uint8_t> originalPayload(buf1, buf1 + len1);

    // Rewrite the source file with same length but different bytes. The
    // implementation reads file size then checks the cache; on cache hit it
    // returns the cached pointer and never re-reads, so buf2 must equal buf1
    // and reflect the ORIGINAL content, not the rewritten content.
    std::vector<uint8_t> rewrittenPayload = {0xAA, 0xBB, 0xCC};
    ASSERT_TRUE(WriteTestFile(GET_FILE_BUFFER_TEST_FILE, rewrittenPayload));

    size_t len2 = 0;
    const uint8_t* buf2 = moduleManager.GetFileBuffer(GET_FILE_BUFFER_TEST_FILE, GET_FILE_BUFFER_TEST_KEY, len2);
    EXPECT_EQ(buf2, buf1);
    EXPECT_EQ(len2, len1);
    std::vector<uint8_t> returnedPayload(buf2, buf2 + len2);
    EXPECT_EQ(returnedPayload, originalPayload);

    CleanupModuleBuffer(moduleManager, GET_FILE_BUFFER_TEST_KEY);

    GTEST_LOG_(INFO) << "GetFileBuffer_ShouldReturnSamePointerOnSecondCallFromCache end";
}

/**
 * @tc.name: GetFileBuffer_ShouldReturnNullptrWhenFileDoesNotExist
 * @tc.desc: GetFileBuffer returns nullptr and keeps len == 0 when file does not exist (AC-11.2)
 * @tc.type: FUNC
 * @tc.require: issue #2157
 */
HWTEST_F(ModuleManagerTest, GetFileBuffer_ShouldReturnNullptrWhenFileDoesNotExist, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileBuffer_ShouldReturnNullptrWhenFileDoesNotExist starts";

    NativeModuleManager moduleManager;
    std::string nonExisting = std::string(GET_FILE_BUFFER_TEST_DIR) + "/napi_not_exist_11.bin";
    std::remove(nonExisting.c_str());

    size_t len = 1; // non-zero default; function should overwrite on failure
    const uint8_t* buf = moduleManager.GetFileBuffer(nonExisting, "test.getfilebuffer.notexist", len);
    EXPECT_EQ(buf, nullptr);

    GTEST_LOG_(INFO) << "GetFileBuffer_ShouldReturnNullptrWhenFileDoesNotExist end";
}

/**
 * @tc.name: GetFileBuffer_ShouldReturnNullptrForEmptyFile
 * @tc.desc: Empty file is rejected (fileSize == 0 early-return), returns nullptr (AC-11.5)
 * @tc.type: FUNC
 * @tc.require: issue #2157
 */
HWTEST_F(ModuleManagerTest, GetFileBuffer_ShouldReturnNullptrForEmptyFile, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileBuffer_ShouldReturnNullptrForEmptyFile starts";

    NativeModuleManager moduleManager;
    ASSERT_TRUE(WriteTestFile(GET_FILE_BUFFER_TEST_FILE, {}));

    size_t len = 1; // non-zero default; function should reset to 0
    const uint8_t* buf = moduleManager.GetFileBuffer(GET_FILE_BUFFER_TEST_FILE, GET_FILE_BUFFER_TEST_KEY, len);
    EXPECT_EQ(buf, nullptr);
    EXPECT_EQ(len, 0u);
    EXPECT_EQ(moduleManager.moduleBufMap_.count(GET_FILE_BUFFER_TEST_KEY), 0u);
    std::remove(GET_FILE_BUFFER_TEST_FILE);

    GTEST_LOG_(INFO) << "GetFileBuffer_ShouldReturnNullptrForEmptyFile end";
}

/**
 * @tc.name: CreateHeadNativeModule_ShouldInitHeadAndTailWhenListEmpty
 * @tc.desc: CreateHeadNativeModule initialises head/tail to the same new node on empty list (AC-12.1)
 * @tc.type: FUNC
 * @tc.require: issue #2157
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
 * @tc.desc: CreateHeadNativeModule prepends a new head and links old head as ->next (AC-12.2)
 * @tc.type: FUNC
 * @tc.require: issue #2157
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
 * @tc.desc: CreateTailNativeModule initialises head/tail to the same new node on empty list (AC-12.3)
 * @tc.type: FUNC
 * @tc.require: issue #2157
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
 * @tc.desc: CreateTailNativeModule appends a new tail and links old tail ->next to it (AC-12.4)
 * @tc.type: FUNC
 * @tc.require: issue #2157
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
 * @tc.desc: Repeated GetInstance calls from one thread return the same pointer (AC-16.1)
 * @tc.type: FUNC
 * @tc.require: issue #2157
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