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
#ifndef FOUNDATION_ACE_NAPI_MODULE_MANAGER_MODULE_CHECKER_H
#define FOUNDATION_ACE_NAPI_MODULE_MANAGER_MODULE_CHECKER_H

#include <memory>
#include <functional>

typedef std::function<bool(const std::string&)> ApiAllowListChecker;

/**
 * @brief Module checker interface. check whether module can be loaded
 *
 */
class ModuleCheckerDelegate {
public:
    virtual ~ModuleCheckerDelegate() = default;

    /**
     * @brief Check whether the module is allowed to be loaded
     *
     * @param moduleName module name
     * @return true The module can be loaded
     * @return false The module cannot be loaded
     */
    virtual bool CheckModuleLoadable(const char* moduleName,
        std::unique_ptr<ApiAllowListChecker>& apiAllowListChecker, bool isAppModule) = 0;

    /**
     * @brief Whether loadable rules only work for disk case
     *
     * @return true Disk check only
     * @return false Check both cache and disk
     */
    virtual bool DiskCheckOnly();
};

#endif /* FOUNDATION_ACE_NAPI_MODULE_MANAGER_MODULE_CHECKER_H */
