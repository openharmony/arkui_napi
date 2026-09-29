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

#ifndef LOAD_ABC_H
#define LOAD_ABC_H

#include <set>
#include <string>
#include <vector>

#include "api_meta.h"
#include "error_codes.h"
#include "napi/native_api.h"

namespace ohos::check {

// ABC loading parameters: requireName drives symbol/record-name candidates; use abcFilePath
// for a standalone file or soPath for a mixed .so, plus the optional expandable-name set.
struct AbcLoadParam {
    std::string kit;
    std::string displayName;
    std::string requireName;
    std::string abcFilePath;
    std::string soPath;
    const std::set<std::string>* expandableNames = nullptr;
};

/**
 * @brief Executes a standalone .abc file and collects the exported API metadata.
 */
CheckErrorCode LoadAbcFileAndGetExposedMeta(
    napi_env env,
    const AbcLoadParam& param,
    std::vector<ApiMeta>& exposedMeta
);

/**
 * @brief Detects embedded ArkTS ABC in a .so (GetABCCode symbol) by scanning file bytes
 * only; never dlopens or executes anything.
 */
bool SoHasEmbeddedAbc(const std::string& soPath);

/**
 * @brief Fallback for mixed .so modules when requireNapi fails: dlopen the .so, pull the
 * embedded ABC via NAPI_<module>_GetABCCode and execute it with candidate record names.
 */
CheckErrorCode LoadMixedSoAbcAndGetExposedMeta(
    napi_env env,
    const AbcLoadParam& param,
    std::vector<ApiMeta>& exposedMeta
);

/**
 * @brief Candidate .so paths mirroring the engine's GetNativeModulePath rules.
 */
std::vector<std::string> CandidateSoPaths(const std::string& soDir, const std::string& requireName);

} // namespace ohos::check

#endif // LOAD_ABC_H
