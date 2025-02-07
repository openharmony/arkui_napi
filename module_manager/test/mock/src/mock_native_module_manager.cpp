/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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


#include "mock_native_module_manager.h"

#include <gtest/gtest.h>

namespace {
bool g_mockCheckModuleLoadable = false;
LIBHANDLE g_mockLoadModuleLibrary = nullptr;
NativeModule *g_mockFindNativeModuleByDisk = nullptr;
NativeModule *g_mockFindNativeModuleByCache = nullptr;
}

void MockFindNativeModuleByDisk(NativeModule *module)
{
    g_mockFindNativeModuleByDisk = module;
}

void MockFindNativeModuleByCache(NativeModule *module)
{
    g_mockFindNativeModuleByCache = module;
}

void MockLoadModuleLibrary(LIBHANDLE handle)
{
    g_mockLoadModuleLibrary = handle;
    GTEST_LOG_(INFO) << g_mockLoadModuleLibrary;
}

void MockCheckModuleLoadable(bool loadable)
{
    g_mockCheckModuleLoadable = loadable;
}

void MockResetModuleManagerState()
{
    g_mockFindNativeModuleByDisk = nullptr;
    g_mockFindNativeModuleByCache = nullptr;
    g_mockCheckModuleLoadable = false;
    g_mockLoadModuleLibrary = nullptr;
}

NativeModule* NativeModuleManager::FindNativeModuleByDisk(const char* moduleName, const char* path,
    const char* relativePath, bool internal, const bool isAppModule, std::string& errInfo,
    char nativeModulePath[][NAPI_PATH_MAX], NativeModule* cacheNativeModule)
{
    GTEST_LOG_(INFO) << g_mockFindNativeModuleByDisk;
    return g_mockFindNativeModuleByDisk;
}

LIBHANDLE NativeModuleManager::LoadModuleLibrary(std::string &moduleKey, const char* path,
    const char* pathKey, const bool isAppModule, std::string& errInfo, uint32_t& errReason)
{
    GTEST_LOG_(INFO) << g_mockLoadModuleLibrary;
    return g_mockLoadModuleLibrary;
}

bool ModuleLoadChecker::CheckModuleLoadable(const char* moduleName,
    std::unique_ptr<ApiAllowListChecker>& apiAllowListChecker, bool isAppModule)
{
    apiAllowListChecker = nullptr;
    GTEST_LOG_(INFO) << g_mockCheckModuleLoadable;
    return g_mockCheckModuleLoadable;
}

NativeModule* NativeModuleManager::FindNativeModuleByCache(const char* moduleName,
                                                           char nativeModulePath[][NAPI_PATH_MAX],
                                                           NativeModule*& cacheNativeModule,
                                                           NativeModuleHeadTailStruct& cacheHeadTailStruct)
{
    GTEST_LOG_(INFO) << g_mockFindNativeModuleByCache;
    return g_mockFindNativeModuleByCache;
}
