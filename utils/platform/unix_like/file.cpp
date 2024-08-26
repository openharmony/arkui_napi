/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <cerrno>
#include <cstring>
#include <securec.h>

#include "utils/file.h"
#include "utils/log.h"

bool RealPath(const char *path, char *resolvedPath, size_t bufferSize)
{
    if (path == nullptr || resolvedPath == nullptr) {
        return false;
    }
    size_t pathLen = strlen(path);
    if (pathLen == 0 || pathLen > PATH_MAX) {
        HILOG_WARN("File path is illeage");
        return false;
    }
    char buffer[PATH_MAX + 1] = { 0 };
    if (realpath(path, buffer) == NULL) {
        // Maybe file does not exist.
        HILOG_WARN("File path: %{public}s full path failure", path);
        return false;
    }
    if (strcpy_s(resolvedPath, bufferSize - 1, buffer) != 0) {
        return false;
    }
    return true;
}