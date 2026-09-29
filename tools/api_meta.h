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

#ifndef API_META_H
#define API_META_H

#include <string>
#include <ostream>

namespace ohos::check {

struct ApiMeta {
    std::string apiText;
    std::string file;
    std::string apiType;  // class / method / field
    std::string className;   // declaring class/namespace; empty for module top-level
    std::string kit;

    bool operator==(const ApiMeta& other) const
    {
        return apiText == other.apiText &&
               file == other.file &&
               apiType == other.apiType &&
               className == other.className &&
               kit == other.kit;
    }
};

std::ostream& operator<<(std::ostream& os, const ApiMeta& v);
}

#endif // API_META_H