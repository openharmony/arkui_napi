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

#ifndef ERROR_CODES_H
#define ERROR_CODES_H

namespace ohos::check {
enum CheckErrorCode {
    CHECK_OK = 0,
    ERROR_PARAM_INVALID = 1001,
    ERROR_FILE_OPEN_FAILED = 1002,
    ERROR_JSON_PARSE_FAILED = 1003,
    ERROR_JS_ENGINE_INIT = 1004,
    ERROR_JS_EXECUTE_FAILED = 1005,
    ERROR_ILLEGAL_API_EXPOSED = 2001,
    ERROR_UNKNOWN_MODULE_EXPOSED = 2002
};
} //namespace ohos::check

#endif