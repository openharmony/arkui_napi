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

#ifndef CHECK_HELPER_H
#define CHECK_HELPER_H

#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "api_meta.h"
#include "napi/native_api.h"

namespace ohos::check {

// Log levels ERROR < WARN < INFO < DEBUG (default INFO). WARN/ERROR always go to stderr;
// other levels go to the log file when --log-file is set, else to stderr.
enum class LogLevel : int {
    ERROR = 0,
    WARN = 1,
    INFO = 2,
    DEBUG = 3,
};

void SetLogLevel(LogLevel level);
LogLevel GetLogLevel();
bool ParseLogLevel(const std::string& name, LogLevel& out);
bool SetLogFile(const std::string& path); // returns false if the file cannot be opened
void CloseLogFile();
void WriteLog(LogLevel level, const std::string& msg);

// Stream-style log record: accumulates << operands and writes the message on destruction,
// so the level is checked once and disabled records never build the message.
class LogStream {
public:
    explicit LogStream(LogLevel level);
    ~LogStream();
    LogStream(const LogStream&) = delete;
    LogStream& operator=(const LogStream&) = delete;

    template <typename T>
    LogStream& operator<<(const T& value)
    {
        if (enabled_) {
            stream_ << value;
        }
        return *this;
    }

private:
    LogLevel level_;
    bool enabled_;
    std::ostringstream stream_;
};

LogStream LogError();
LogStream LogWarn();
LogStream LogInfo();
LogStream LogDebug();

// Creates a lightweight Ark engine environment (CreateJSVM + ArkNativeEngine).
napi_env CreateArkEnv();

// Teardown order: napi_destroy_runtime -> ~NativeEngine -> cleanEnv -> DestroyJSVM.
void DestroyArkEnv(napi_env env);

// Clears a pending JS exception on the VM so later checks are not affected.
void ClearPendingException(napi_env env);

// Per-module guard: clears pending exceptions and opens a fresh handle scope on entry (and
// the reverse on exit); without it one module's exception short-circuits all later lookups.
class ModuleCheckGuard {
public:
    explicit ModuleCheckGuard(napi_env env);
    ~ModuleCheckGuard();
    ModuleCheckGuard(const ModuleCheckGuard&) = delete;
    ModuleCheckGuard& operator=(const ModuleCheckGuard&) = delete;

private:
    napi_env env_ = nullptr;
    napi_handle_scope scope_ = nullptr;
};

// Built-in properties excluded from the exported surface.
bool IsIgnoredProperty(const char* name);

// Maps an exported value to "class" / "method" / "field".
std::string GetApiType(napi_env env, napi_value value);

struct TraverseContext {
    napi_env env = nullptr;
    std::string kit;
    std::string file;
    const std::set<std::string>* expandableNames = nullptr;
    std::vector<ApiMeta>* outMeta = nullptr;
};

// Recursively walks an exports object. expandableNames holds declared namespace/enum/class
// names whose same-named field objects get expanded; nullptr expands classes only.
void TraverseExports(TraverseContext& ctx, napi_value exports, const std::string& parentClass);

// Name index over the legal list used to prescreen Match candidates (see check_match.cpp).
using LegalIndex = std::unordered_map<std::string, std::vector<const ApiMeta*>>;
void BuildLegalIndex(const std::vector<ApiMeta>& legalList, LegalIndex& index);

void CollectExpandableNames(const std::vector<ApiMeta>& legalList, std::set<std::string>& names);

bool StartsWith(const std::string& str, const std::string& prefix);
bool EndsWith(const std::string& str, const std::string& suffix);
// Strips .d.ts/.d.ets suffixes so declaration file names compare equal to runtime module
// names; bare module names pass through unchanged.
std::string NormalizeModuleName(const std::string& file);
// Case-insensitive equality: d.ts and runtime names often differ in case (avSession/avsession).
bool EqualsIgnoreCase(const std::string& a, const std::string& b);

bool Match(const ApiMeta &exposed, const ApiMeta &legal, const std::map<std::string, std::string> &inheritMap);

}

#endif
