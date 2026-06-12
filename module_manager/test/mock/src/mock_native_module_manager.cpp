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
bool g_mockDiskCheckOnly = true;
LIBHANDLE g_mockLoadModuleLibrary = nullptr;
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

void MockDiskCheckOnly(bool diskCheckOnly)
{
    g_mockDiskCheckOnly = diskCheckOnly;
}

void MockResetModuleManagerState()
{
    g_mockCheckModuleLoadable = false;
    g_mockDiskCheckOnly = true;
    g_mockLoadModuleLibrary = nullptr;
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

bool ModuleLoadChecker::DiskCheckOnly()
{
    return g_mockDiskCheckOnly;
}
