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

#ifndef OHOS_CJ_BACKTRACE_H
#define OHOS_CJ_BACKTRACE_H

#include <string>
#include <functional>

extern "C" {
// only works at main thread.
bool CJDFX_HasBacktrace(void);
bool CJDFX_Backtrace(uint32_t removeTopFrames);
void CJDFX_IgnoreNextBacktrace();
bool CJDFX_GetHybridStack(std::string* trace, std::function<bool(std::string&, int&, int&, std::string&)>* translator);
}

#endif // OHOS_CJ_BACKTRACE_H
