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

#include "check_match.h"

#include <string>
#include <vector>

namespace ohos::check {
namespace {

bool MatchInCandidates(const ApiMeta& exposed, const std::vector<const ApiMeta*>& candidates,
                       const std::map<std::string, std::string>& inheritMap)
{
    for (const ApiMeta* legal : candidates) {
        if (Match(exposed, *legal, inheritMap)) {
            return true;
        }
    }
    return false;
}

} // namespace

CheckErrorCode CheckIllegalExposed(
    const LegalIndex& legalIndex,
    const std::vector<ApiMeta> &exposedList,
    const std::map<std::string, std::string> &inheritMap,
    std::vector<ApiMeta> &illegalList
)
{
    CheckErrorCode ret = CHECK_OK;
    for (const auto& exposed : exposedList) {
        // Name index: exposed names hit a small candidate set instead of scanning the whole
        // whitelist (ArkUI alone has ~20k entries across 79 modules).
        bool found = false;
        auto it = legalIndex.find(exposed.apiText);
        if (it != legalIndex.end()) {
            found = MatchInCandidates(exposed, it->second, inheritMap);
        }

        if (!found) {
            LogError() << " [CheckAPI] 非法暴露API: " <<
                exposed.apiText << " | apiType: " << exposed.apiType <<
                " | className: " << exposed.className <<
                " | file: " << exposed.file << " | kit: " << exposed.kit;
            illegalList.push_back(exposed);
            ret = ERROR_ILLEGAL_API_EXPOSED;
        }
    }

    return ret;
}

}
