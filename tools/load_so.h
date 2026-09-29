/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef LOAD_SO_H
#define LOAD_SO_H

#include <set>
#include <string>
#include <vector>

#include "api_meta.h"
#include "error_codes.h"
#include "napi/native_api.h"

namespace ohos::check {

/**
 * @brief Maps a module name to its requireNapi argument, mirroring GetNativeModulePath:
 * '@ohos.arkui.node' -> 'arkui.node'; '@hms.x.y'/'@arkts.x' keep the full name (every dot
 * becomes a directory); GetOhmurl NATIVE_MODULE special cases apply (e.g. '@system.app').
 */
std::string ModuleNameForRequireNapi(const std::string& file);

// displayName feeds the file-level match; requireName is what gets passed to requireNapi.
struct SoLoadParam {
    std::string kit;
    std::string displayName;
    std::string requireName;
};

/**
 * @brief Loads a native module via requireNapi and collects its exported API metadata.
 */
CheckErrorCode LoadSoAndGetExposedMeta(
    napi_env env,
    const SoLoadParam& param,
    std::vector<ApiMeta>& exposedMeta,
    const std::set<std::string>* expandableNames = nullptr
);

}

#endif
