/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_NAPI_MODULE_MANAGER_NATIVE_MODULE_MANAGER_H
#define FOUNDATION_ACE_NAPI_MODULE_MANAGER_NATIVE_MODULE_MANAGER_H

#include <cstdint>
#include <map>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <pthread.h>

#include "module_load_checker.h"
#include "utils/macros.h"
#include "interfaces/inner_api/napi/native_node_api.h"

#ifdef WINDOWS_PLATFORM
#include <winsock2.h>
#include <windows.h>
using LIBHANDLE = HMODULE;
#define LIBFREE FreeLibrary
#define LIBSYM GetProcAddress
#else
#include <dlfcn.h>
using LIBHANDLE = void*;
#define LIBFREE dlclose
#define LIBSYM dlsym
#endif

#define NAPI_PATH_MAX 4096

class NativeValue;

class NativeEngine;

typedef napi_value (*RegisterCallback)(napi_env, napi_value);

typedef void (*GetJSCodeCallback)(const char** buf, int* bufLen);

struct NativeModule {
    const char* name = nullptr;
    const char* fileName = nullptr;
    RegisterCallback registerCallback = nullptr;
    GetJSCodeCallback getABCCode = nullptr;
    GetJSCodeCallback getJSCode = nullptr;
    int32_t version = 0;
    uint32_t refCount = 0;
    NativeModule* next = nullptr;
    const char* jsCode = nullptr;
    const uint8_t* jsABCCode = nullptr;
    int32_t jsCodeLen = 0;
    bool moduleLoaded = false;
    bool isAppModule = false;
    std::unique_ptr<ApiAllowListChecker> apiAllowListChecker = nullptr;
};

class NAPI_EXPORT NativeModuleManager {
public:
    static NativeModuleManager* GetInstance();
    static uint64_t Release();

    void Register(NativeModule* nativeModule);
    void SetAppLibPath(const std::string& moduleName, const std::vector<std::string>& appLibPath,
                       const bool& isSystemApp = false);
    NativeModule* LoadNativeModule(const char* moduleName, const char* path, bool isAppModule,
        bool internal = false, const char* relativePath = "", bool moduleRestricted = false);
    void SetNativeEngine(std::string moduleName, NativeEngine* nativeEngine);
    bool UnloadNativeModule(const std::string& moduleKey);
    std::string GetModuleFileName(const char* moduleName, bool isAppModule);

    /**
     * @brief Set the path for searching napi dynamic libraries, only for the previewer.
     *
     * @param previewSearchPath the path for searching napi dynamic libraries
     */
    void SetPreviewSearchPath(const std::string& previewSearchPath);

    /**
     * @brief Set the Module Load Checker delegate
     *
     * @param moduleCheckerDelegate The Module Load Checker delegate
     */
    void SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate>& moduleCheckerDelegate);

private:
    NativeModuleManager();
    virtual ~NativeModuleManager();

    bool GetNativeModulePath(const char* moduleName, const char* path, const char* relativePath,
        bool isAppModule, char nativeModulePath[][NAPI_PATH_MAX], int32_t pathLength);
    NativeModule* FindNativeModuleByDisk(const char* moduleName, const char* path, const char* relativePath,
        bool internal, const bool isAppModule);
    NativeModule* FindNativeModuleByCache(const char* moduleName);
    LIBHANDLE LoadModuleLibrary(std::string& moduleKey, const char* path, const char* pathKey, const bool isAppModule);
    bool CheckModuleRestricted(const std::string& moduleName);
    const uint8_t* GetFileBuffer(const std::string& filePath, const std::string& moduleKey, size_t &len);
    bool UnloadModuleLibrary(LIBHANDLE handle);
    bool CloseModuleLibrary(LIBHANDLE handle);
    void CreateLdNamespace(const std::string moduleName, const char* lib_ld_path, const bool& isSystemApp);
    bool IsExistedPath(const char* pathKey) const;
    void EmplaceModuleLib(const std::string moduleKey, LIBHANDLE lib);
    bool RemoveModuleLib(const std::string moduleKey);
    void EmplaceModuleBuffer(const std::string moduleKey, const uint8_t* lib);
    bool RemoveModuleBuffer(const std::string moduleKey);
    const uint8_t* GetBufferHandle(const std::string& moduleKey) const;
    void RegisterByBuffer(const std::string& moduleKey, const uint8_t* abcBuffer, size_t len);
    bool CreateNewNativeModule();
    LIBHANDLE GetNativeModuleHandle(const std::string& moduleKey) const;
    bool RemoveNativeModuleByCache(const std::string& moduleKey);
    bool RemoveNativeModule(const std::string& moduleKey);
    void MoveApiAllowListCheckerPtr(
        std::unique_ptr<ApiAllowListChecker>& apiAllowListChecker, NativeModule* nativeModule);
#if !defined(WINDOWS_PLATFORM) && !defined(MAC_PLATFORM) && !defined(__BIONIC__) && !defined(IOS_PLATFORM) && \
    !defined(LINUX_PLATFORM)
    void CreateSharedLibsSonames();

    char* sharedLibsSonames_ = nullptr;
    std::map<std::string, Dl_namespace> nsMap_;
#endif

    std::mutex nativeModuleListMutex_;
    NativeModule* firstNativeModule_ = nullptr;
    NativeModule* lastNativeModule_ = nullptr;

    static NativeModuleManager *instance_;
    pthread_mutex_t mutex_;
    std::string prefix_;
    bool isAppModule_ = false;

    std::mutex nativeEngineListMutex_;
    std::map<std::string, NativeEngine*> nativeEngineList_;

    mutable std::mutex moduleLibMutex_;
    std::map<std::string, const LIBHANDLE> moduleLibMap_;

    mutable std::mutex moduleBufMutex_;
    std::map<std::string, const uint8_t*> moduleBufMap_;

    std::map<std::string, char*> appLibPathMap_;
    std::string previewSearchPath_;
    std::unique_ptr<ModuleLoadChecker> moduleLoadChecker_ = nullptr;
};

#endif /* FOUNDATION_ACE_NAPI_MODULE_MANAGER_NATIVE_MODULE_MANAGER_H */
