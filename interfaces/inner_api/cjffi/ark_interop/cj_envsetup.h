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

#ifndef NAPI_CJ_ENVSETUP_H
#define NAPI_CJ_ENVSETUP_H

#include <string>
#include <functional>

namespace OHOS {
using UpdateStackInfoFuncType = void(*)(unsigned long long, void *, unsigned int);

struct CJErrorObject {
    const char* name;
    const char* message;
    const char* stack;
};

struct CJUncaughtExceptionInfo {
    const char* hapPath;
    std::function<void(const char* summary, const CJErrorObject errorObj)> uncaughtTask;
};

enum SanitizerKind {
    NONE,
    ASAN,
    TSAN,
    HWASAN,
};

struct CJEnvMethods {
    void (*initCJAppNS)(const std::string& path) = nullptr;
    void (*initCJSDKNS)(const std::string& path) = nullptr;
    void (*initCJSysNS)(const std::string& path) = nullptr;
    void (*initCJChipSDKNS)(const std::string& path) = nullptr;
    bool (*startRuntime)() = nullptr;
    bool (*startUIScheduler)() = nullptr;
    void* (*loadCJModule)(const char* dllName) = nullptr;
    void* (*loadLibrary)(uint32_t kind, const char* dllName) = nullptr;
    void* (*getSymbol)(void* handle, const char* symbol) = nullptr;
    void* (*loadCJLibrary)(const char* dllName) = nullptr;
    bool (*startDebugger)() = nullptr;
    void (*registerCJUncaughtExceptionHandler)(const CJUncaughtExceptionInfo& uncaughtExceptionInfo) = nullptr;
    void (*setSanitizerKindRuntimeVersion)(SanitizerKind kind) = nullptr;
    bool (*checkLoadCJLibrary)() = nullptr;
    void (*registerArkVMInRuntime)(unsigned long long arkVM) = nullptr;
    void (*registerStackInfoCallbacks)(UpdateStackInfoFuncType uFunc) = nullptr;
};

class CJEnv {
public:
    static CJEnvMethods* LoadInstance();
};

}

#endif // NAPI_CJ_ENVSETUP_H
