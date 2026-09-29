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

#ifndef PARSE_JSON_H
#define PARSE_JSON_H

#include <map>
#include <set>
#include <string>
#include <vector>

#include "api_meta.h"
#include "error_codes.h"

namespace ohos::check {

/**
 * @brief Parses the legal-API whitelist (ApiMeta list + inheritance map), normalizing
 * declarations on the way in (type decls, method/field shapes, "unnamed" classNames).
 */
CheckErrorCode ParseLegalApiList(
    const std::string& resultFile,
    std::vector<ApiMeta>& legalApiList,
    std::map<std::string, std::string>& inheritMap
);

/**
 * @brief Parses the check list {"kit1": ["file1", "file2"], ...}
 */
CheckErrorCode ParseKitToCheckFiles(
    const std::string& jsonFile,
    std::map<std::string, std::vector<std::string>>& kitFileMap
);

/**
 * @brief Collects the canonical module names ('@'-prefixed) declared in a whitelist json,
 * used by scan-dir mode to build the module-to-kit reverse lookup.
 */
CheckErrorCode CollectModuleNames(
    const std::string& jsonFile,
    std::set<std::string>& moduleNames
);

/**
 * @brief Writes the result json: IllegalApis, plus DeprecatedModules and UnknownModules
 * sections when non-empty (deprecated entries never mix into IllegalApis).
 */
CheckErrorCode SaveIllegalApiToJson(
    const std::string& outputFilePath,
    const std::map<std::string, std::map<std::string, std::vector<ApiMeta>>>& illegalMap,
    const std::map<std::string, std::map<std::string, std::vector<ApiMeta>>>& deprecatedMap = {},
    const std::vector<std::string>& unknownModules = {}
);

}

#endif
