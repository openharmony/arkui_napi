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

#include "module_load_checker.h"

#include "utils/log.h"

void ModuleLoadChecker::SetModuleBlacklist(
    std::unordered_map<int32_t, std::unordered_set<std::string>>&& blacklist)
{
    moduleBlacklist_ = std::move(blacklist);
    HILOG_INFO("moduleBlacklist_ size = %{public}d", static_cast<int32_t>(moduleBlacklist_.size()));
}

void ModuleLoadChecker::SetProcessExtensionType(int32_t extensionType)
{
    processExtensionType_ = extensionType;
}

int32_t ModuleLoadChecker::GetProcessExtensionType()
{
    return processExtensionType_;
}

bool ModuleLoadChecker::CheckModuleLoadable(const char* moduleName)
{
    HILOG_INFO("check blacklist, moduleName = %{public}s, processExtensionType_ = %{public}d",
        moduleName, static_cast<int32_t>(processExtensionType_));
    const auto& blackListIter = moduleBlacklist_.find(processExtensionType_);
    if (blackListIter == moduleBlacklist_.end()) {
        return true;
    }
    auto blackList = blackListIter->second;
    if (blackList.find(moduleName) == blackList.end()) {
        return true;
    }
    return false;
}