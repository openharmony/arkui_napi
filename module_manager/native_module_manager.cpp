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

#include "native_module_manager.h"

#include <dirent.h>
#include <mutex>

#ifdef ANDROID_PLATFORM
#include <fstream>
#include <iostream>
#include <sstream>
#endif

#ifdef ENABLE_HITRACE
#include "hitrace_meter.h"
#endif
#include "module_load_checker.h"
#include "native_engine/native_engine.h"
#include "securec.h"
#include "utils/log.h"

#define NDK "ndk"
#define ALLOW_ALL_SHARED_LIBS "allow_all_shared_libs"

namespace {
constexpr static int32_t NATIVE_PATH_NUMBER = 2;
} // namespace

NativeModuleManager* NativeModuleManager::instance_ = NULL;
std::mutex g_instanceMutex;

NativeModuleManager::NativeModuleManager()
{
    HILOG_DEBUG("enter");
    pthread_mutex_init(&mutex_, nullptr);
    moduleLoadChecker_ = std::make_unique<ModuleLoadChecker>();
}

NativeModuleManager::~NativeModuleManager()
{
    HILOG_INFO("enter");
    {
        std::lock_guard<std::mutex> lock(nativeModuleListMutex_);
        NativeModule* nativeModule = firstNativeModule_;
        while (nativeModule != nullptr) {
            nativeModule = nativeModule->next;
            delete[] firstNativeModule_->name;
            delete firstNativeModule_;
            firstNativeModule_ = nativeModule;
        }
        firstNativeModule_ = nullptr;
        lastNativeModule_ = nullptr;
    }

#if !defined(WINDOWS_PLATFORM) && !defined(MAC_PLATFORM) && !defined(__BIONIC__) && !defined(IOS_PLATFORM) && \
    !defined(LINUX_PLATFORM)
    if (sharedLibsSonames_) {
        delete[] sharedLibsSonames_;
    }
#endif

    for (const auto& item : appLibPathMap_) {
        delete[] item.second;
    }
    std::map<std::string, char*>().swap(appLibPathMap_);

    while (nativeEngineList_.size() > 0) {
        NativeEngine* wraper = nativeEngineList_.begin()->second;
        if (wraper != nullptr) {
            delete wraper;
            wraper = nullptr;
        }
        nativeEngineList_.erase(nativeEngineList_.begin());
    }
    pthread_mutex_destroy(&mutex_);
}

NativeModuleManager* NativeModuleManager::GetInstance()
{
    if (instance_ == NULL) {
        std::lock_guard<std::mutex> lock(g_instanceMutex);
        if (instance_ == NULL) {
            instance_ = new NativeModuleManager();
            HILOG_DEBUG("create native module manager instance");
        }
    }
    return instance_;
}

void NativeModuleManager::SetNativeEngine(std::string moduleKey, NativeEngine* nativeEngine)
{
    HILOG_DEBUG("modulekey is '%{public}s'", moduleKey.c_str());
    std::lock_guard<std::mutex> lock(nativeEngineListMutex_);
    nativeEngineList_.emplace(moduleKey, nativeEngine);
}

void NativeModuleManager::EmplaceModuleLib(std::string moduleKey, const LIBHANDLE lib)
{
    HILOG_DEBUG("modulekey is '%{public}s'", moduleKey.c_str());
    std::lock_guard<std::mutex> lock(moduleLibMutex_);
    if (lib != nullptr) {
        moduleLibMap_.emplace(moduleKey, lib);
    }
}

bool NativeModuleManager::RemoveModuleLib(const std::string moduleKey)
{
    HILOG_DEBUG("moduleKey is '%{public}s'", moduleKey.c_str());
    bool deleted = false;
    std::lock_guard<std::mutex> lock(moduleLibMutex_);
    auto it = moduleLibMap_.find(moduleKey);
    if (it != moduleLibMap_.end()) {
        moduleLibMap_.erase(it);
        HILOG_DEBUG("module '%{public}s' erased", moduleKey.c_str());
        deleted = true;
    }
    return deleted;
}

LIBHANDLE NativeModuleManager::GetNativeModuleHandle(const std::string &moduleKey) const
{
    HILOG_DEBUG("moduleKey is '%{public}s'", moduleKey.c_str());
    std::lock_guard<std::mutex> lock(moduleLibMutex_);
    auto it = moduleLibMap_.find(moduleKey);
    if (it == moduleLibMap_.end()) {
        return nullptr;
    }
    return it->second;
}

bool NativeModuleManager::RemoveNativeModule(const std::string &moduleKey)
{
    bool handleRemoved = RemoveModuleLib(moduleKey);
    bool moduleRemoved = RemoveNativeModuleByCache(moduleKey);

    HILOG_DEBUG("handleRemoved is %{public}d, moduleRemoved is %{public}d", handleRemoved, moduleRemoved);
    return (handleRemoved && moduleRemoved);
}

bool NativeModuleManager::UnloadNativeModule(const std::string &moduleKey)
{
    HILOG_DEBUG("moduleKey is '%{public}s'", moduleKey.c_str());
    LIBHANDLE handle = GetNativeModuleHandle(moduleKey);
    if (handle == nullptr) {
        HILOG_ERROR("failed to get native module handle.");
        return false;
    }

    if (RemoveNativeModule(moduleKey) == false) {
        HILOG_ERROR("remove native module failed.");
        return false;
    }

    return UnloadModuleLibrary(handle);
}

const char* NativeModuleManager::GetModuleFileName(const char* moduleName, bool isAppModule)
{
    HILOG_INFO("moduleName is '%{public}s', isAppModule is %{public}d", moduleName, isAppModule);

    NativeModule* module = FindNativeModuleByCache(moduleName);
    if (module != nullptr) {
        char nativeModulePath[NATIVE_PATH_NUMBER][NAPI_PATH_MAX];
        if (!GetNativeModulePath(moduleName, "default", "", isAppModule, nativeModulePath, NAPI_PATH_MAX)) {
            HILOG_ERROR("get native module path failed");
            return nullptr;
        }
        const char* loadPath = nativeModulePath[0];
        return loadPath;
    }
    HILOG_ERROR("get module file name failed");
    return nullptr;
}

void NativeModuleManager::Register(NativeModule* nativeModule)
{
    if (nativeModule == nullptr) {
        HILOG_ERROR("nativeModule value is null");
        return;
    }

    HILOG_DEBUG("native module name is '%{public}s'", nativeModule->name);
    std::lock_guard<std::mutex> lock(nativeModuleListMutex_);
    if (firstNativeModule_ == lastNativeModule_ && lastNativeModule_ == nullptr) {
        firstNativeModule_ = new NativeModule();
        if (firstNativeModule_ == nullptr) {
            HILOG_ERROR("first NativeModule create failed");
            return;
        }
        lastNativeModule_ = firstNativeModule_;
    } else {
        auto next = new NativeModule();
        if (next == nullptr) {
            HILOG_ERROR("next NativeModule create failed");
            return;
        }
        if (lastNativeModule_) {
            lastNativeModule_->next = next;
            lastNativeModule_ = lastNativeModule_->next;
        }
    }

    const char *nativeModuleName = nativeModule->name == nullptr ? "" : nativeModule->name;
    std::string appName = prefix_ + "/" + nativeModuleName;
    const char *tmpName = isAppModule_ ? appName.c_str() : nativeModuleName;
    char *moduleName = new char[NAPI_PATH_MAX];
    errno_t err = EOK;
    err = memset_s(moduleName, NAPI_PATH_MAX, 0, NAPI_PATH_MAX);
    if (err != EOK) {
        delete[] moduleName;
        return;
    }
    err = strcpy_s(moduleName, NAPI_PATH_MAX, tmpName);
    if (err != EOK) {
        delete[] moduleName;
        return;
    }

    lastNativeModule_->version = nativeModule->version;
    lastNativeModule_->fileName = nativeModule->fileName;
    lastNativeModule_->isAppModule = isAppModule_;
    lastNativeModule_->name = moduleName;
    lastNativeModule_->refCount = nativeModule->refCount;
    lastNativeModule_->registerCallback = nativeModule->registerCallback;
    lastNativeModule_->getJSCode = nativeModule->getJSCode;
    lastNativeModule_->getABCCode = nativeModule->getABCCode;
    lastNativeModule_->next = nullptr;
#if defined(IOS_PLATFORM) || defined(ANDROID_PLATFORM)
    // For iOS and android, force make module loaded
    lastNativeModule_->moduleLoaded = true;
#endif
    HILOG_INFO("NativeModule Register success. module name is '%{public}s'", lastNativeModule_->name);
}

#if !defined(WINDOWS_PLATFORM) && !defined(MAC_PLATFORM) && !defined(__BIONIC__) && !defined(IOS_PLATFORM) && \
    !defined(LINUX_PLATFORM)
void NativeModuleManager::CreateSharedLibsSonames()
{
    HILOG_DEBUG("enter");
    const char* allowList[] = {
        // bionic library
        "libc.so",
        "libdl.so",
        "libm.so",
        "libz.so",
        "libclang_rt.asan.so",
        // z library
        "libace_napi.z.so",
        "libace_ndk.z.so",
        "libbundle_ndk.z.so",
        "libdeviceinfo_ndk.z.so",
        "libEGL.so",
        "libGLESv3.so",
        "libhiappevent_ndk.z.so",
        "libhuks_ndk.z.so",
        "libhukssdk.z.so",
        "libnative_drawing.so",
        "libnative_window.so",
        "libnative_buffer.so",
        "libnative_vsync.so",
        "libOpenSLES.so",
        "libpixelmap_ndk.z.so",
        "libimage_ndk.z.so",
        "libimage_receiver_ndk.z.so",
        "libimage_source_ndk.z.so",
        "librawfile.z.so",
        "libuv.so",
        "libhilog.so",
        "libnative_image.so",
        "libnative_media_adec.so",
        "libnative_media_aenc.so",
        "libnative_media_codecbase.so",
        "libnative_media_core.so",
        "libnative_media_vdec.so",
        "libnative_media_venc.so",
        "libnative_media_avmuxer.so",
        "libnative_media_avdemuxer.so",
        "libnative_media_avsource.so",
        // adaptor library
        "libohosadaptor.so",
        "libusb_ndk.z.so",
        "libvulkan.so",
    };

    size_t allowListLength = sizeof(allowList) / sizeof(char*);
    int32_t sharedLibsSonamesLength = 1;
    for (size_t i = 0; i < allowListLength; i++) {
        sharedLibsSonamesLength += strlen(allowList[i]) + 1;
    }
    sharedLibsSonames_ = new char[sharedLibsSonamesLength];
    int32_t cursor = 0;
    for (size_t i = 0; i < allowListLength; i++) {
        if (sprintf_s(sharedLibsSonames_ + cursor, sharedLibsSonamesLength - cursor, "%s:", allowList[i]) == -1) {
            delete[] sharedLibsSonames_;
            sharedLibsSonames_ = nullptr;
            return;
        }
        cursor += strlen(allowList[i]) + 1;
    }
    sharedLibsSonames_[cursor] = '\0';
}
#endif

void NativeModuleManager::CreateLdNamespace(const std::string moduleName, const char* lib_ld_path,
                                            [[maybe_unused]] const bool& isSystemApp)
{
#if !defined(WINDOWS_PLATFORM) && !defined(MAC_PLATFORM) && !defined(__BIONIC__) && !defined(IOS_PLATFORM) && \
    !defined(LINUX_PLATFORM)
    Dl_namespace current_ns;
    Dl_namespace ns;

    // Create module ns.
    std::string nsName = "moduleNs_" + moduleName;
    dlns_init(&ns, nsName.c_str());
    dlns_get(nullptr, &current_ns);

    Dl_namespace ndk_ns;
    dlns_get(NDK, &ndk_ns);

    if (isSystemApp) {
        /*
         * The app's so may have the same name as the system library, LOCAL_NS_PREFERED means linker will check
         * and use the app's so first.
         */
        dlns_create2(&ns, lib_ld_path, LOCAL_NS_PREFERED);
        // System app can visit all ndk and default ns libs.
        if (strlen(ndk_ns.name) > 0) {
            dlns_inherit(&ns, &ndk_ns, ALLOW_ALL_SHARED_LIBS);
            dlns_inherit(&ndk_ns, &current_ns, ALLOW_ALL_SHARED_LIBS);
            dlns_inherit(&current_ns, &ndk_ns, ALLOW_ALL_SHARED_LIBS);
            dlns_inherit(&ns, &current_ns, ALLOW_ALL_SHARED_LIBS);
        }
    } else {
        dlns_create2(&ns, lib_ld_path, 0);
        // Non-system app can visit all ndk ns libs and default ns shared libs.
        if (!sharedLibsSonames_) {
            CreateSharedLibsSonames();
        }
        dlns_inherit(&ns, &current_ns, sharedLibsSonames_);
        if (strlen(ndk_ns.name) > 0) {
            dlns_inherit(&ns, &ndk_ns, ALLOW_ALL_SHARED_LIBS);
            dlns_inherit(&ndk_ns, &current_ns, ALLOW_ALL_SHARED_LIBS);
            dlns_inherit(&current_ns, &ndk_ns, ALLOW_ALL_SHARED_LIBS);
        }
    }

    nsMap_[moduleName] = ns;

    HILOG_INFO("CreateLdNamespace success. moduleName: %{public}s, path: %{public}s", moduleName.c_str(), lib_ld_path);
#endif
}

void NativeModuleManager::SetAppLibPath(const std::string& moduleName, const std::vector<std::string>& appLibPath,
                                        const bool& isSystemApp)
{
    HILOG_DEBUG("moduleName is %{public}s, isisSystemApp is %{public}d", moduleName.c_str(), isSystemApp);
    char* tmp = new char[NAPI_PATH_MAX];
    errno_t err = EOK;
    err = memset_s(tmp, NAPI_PATH_MAX, 0, NAPI_PATH_MAX);
    if (err != EOK) {
        delete[] tmp;
        HILOG_ERROR("memset_s failed");
        return;
    }

    std::string tmpPath = "";
    for (size_t i = 0; i < appLibPath.size(); i++) {
        if (appLibPath[i].empty()) {
            continue;
        }
        tmpPath += appLibPath[i];
        tmpPath += ":";
    }
    if (tmpPath.back() == ':') {
        tmpPath.pop_back();
    }

    err = strcpy_s(tmp, NAPI_PATH_MAX, tmpPath.c_str());
    if (err != EOK) {
        delete[] tmp;
        return;
    }

    if (appLibPathMap_[moduleName] != nullptr) {
        delete[] appLibPathMap_[moduleName];
    }

    appLibPathMap_[moduleName] = tmp;
    CreateLdNamespace(moduleName, tmp, isSystemApp);
    HILOG_INFO("create ld namespace, path: %{public}s", appLibPathMap_[moduleName]);
}

NativeModule* NativeModuleManager::LoadNativeModule(const char* moduleName,
    const char* path, bool isAppModule, bool internal, const char* relativePath)
{
    if (moduleName == nullptr || relativePath == nullptr) {
        HILOG_ERROR("moduleName or relativePath is nullptr");
        return nullptr;
    }

    HILOG_DEBUG("moduleName is %{public}s, path is %{public}s", moduleName, path);
#ifdef ANDROID_PLATFORM
    std::string strModule(moduleName);
    std::string strCutName = strModule;
    if (path != nullptr) {
        if (IsExistedPath(path)) {
            strModule = path;
        }
        prefix_ = "default";
        strModule = prefix_ + '/' + moduleName;
    } else {
        path = "default";
        if (strModule.find(".") != std::string::npos) {
            char* temp = const_cast<char*>(strCutName.c_str());
            for (char* p = strchr(temp, '.'); p != nullptr; p = strchr(p + 1, '.')) {
                *p = '_';
            }
        }
    }
#endif

    if (pthread_mutex_lock(&mutex_) != 0) {
        HILOG_ERROR("pthread_mutex_lock is failed");
        return nullptr;
    }

#ifdef ANDROID_PLATFORM
    NativeModule* nativeModule = FindNativeModuleByCache(strModule.c_str());
#else
    std::string key(moduleName);
    isAppModule_ = isAppModule;
    if (isAppModule) {
        prefix_ = "default";
        if (path && IsExistedPath(path)) {
            prefix_ = path;
        }
        key = prefix_ + '/' + moduleName;
    }
    NativeModule* nativeModule = FindNativeModuleByCache(key.c_str());
#endif

#ifndef IOS_PLATFORM
    if (nativeModule == nullptr) {
#ifdef ANDROID_PLATFORM
        HILOG_WARN("module '%{public}s' does not in cache", strCutName.c_str());
        nativeModule = FindNativeModuleByDisk(strCutName.c_str(), path, relativePath, internal, isAppModule);
#else
        HILOG_WARN("module '%{public}s' does not in cache", moduleName);
        nativeModule = FindNativeModuleByDisk(moduleName, prefix_.c_str(), relativePath, internal, isAppModule);
#endif
    }
#endif

    if (pthread_mutex_unlock(&mutex_) != 0) {
        HILOG_ERROR("pthread_mutex_unlock is failed");
        return nullptr;
    }

    HILOG_DEBUG("load native module %{public}s", (nativeModule == nullptr) ? "failed" : "success");
    return nativeModule;
}

bool NativeModuleManager::GetNativeModulePath(const char* moduleName, const char* path,
    const char* relativePath, bool isAppModule, char nativeModulePath[][NAPI_PATH_MAX], int32_t pathLength)
{
#ifdef WINDOWS_PLATFORM
    const char* soPostfix = ".dll";
    const char* zfix = "";
    std::string sysPrefix("./module");
#elif defined(MAC_PLATFORM)
    const char* soPostfix = ".dylib";
    const char* zfix = "";
    std::string sysPrefix("./module");
#elif defined(_ARM64_) || defined(SIMULATOR)
    const char* soPostfix = ".so";
    const char* zfix = ".z";
    std::string sysPrefix("/system/lib64/module");
#elif defined(LINUX_PLATFORM)
    const char* soPostfix = ".so";
    const char* zfix = "";
    std::string sysPrefix("./module");
#else
    const char* soPostfix = ".so";
    const char* zfix = ".z";
    std::string sysPrefix("/system/lib/module");
#endif

#ifdef ANDROID_PLATFORM
    isAppModule = true;
#endif
    int32_t lengthOfModuleName = strlen(moduleName);
    char dupModuleName[NAPI_PATH_MAX] = { 0 };
    if (strcpy_s(dupModuleName, NAPI_PATH_MAX, moduleName) != 0) {
        HILOG_ERROR("strcpy_s moduleName '%{public}s' failed", moduleName);
        return false;
    }

    const char* prefix = nullptr;
    if (isAppModule && IsExistedPath(path)) {
        prefix = appLibPathMap_[path];
#ifdef ANDROID_PLATFORM
        for (int32_t i = 0; i < lengthOfModuleName; i++) {
            dupModuleName[i] = tolower(dupModuleName[i]);
        }
#endif
    } else {
        if (relativePath[0]) {
            if (previewSearchPath_.empty()) {
                sysPrefix = sysPrefix + "/" + relativePath;
            } else {
                sysPrefix = previewSearchPath_ + "/module";
            }
        }
        prefix = sysPrefix.c_str();
        for (int32_t i = 0; i < lengthOfModuleName; i++) {
            dupModuleName[i] = tolower(dupModuleName[i]);
        }
    }

    int32_t lengthOfPostfix = strlen(soPostfix);
    if ((lengthOfModuleName > lengthOfPostfix) &&
        (strcmp(dupModuleName + lengthOfModuleName - lengthOfPostfix, soPostfix) == 0)) {
        if (sprintf_s(nativeModulePath[0], pathLength, "%s/%s", prefix, dupModuleName) == -1) {
            return false;
        }
        return true;
    }

    char* lastDot = strrchr(dupModuleName, '.');
    if (lastDot == nullptr) {
        if (!isAppModule || !IsExistedPath(path)) {
#ifdef ANDROID_PLATFORM
            if (sprintf_s(nativeModulePath[0], pathLength, "lib%s%s", dupModuleName, soPostfix) == -1) {
                return false;
            }
#else
            if (sprintf_s(nativeModulePath[0], pathLength, "%s/lib%s%s%s",
                prefix, dupModuleName, zfix, soPostfix) == -1) {
                return false;
            }
#endif
            if (sprintf_s(nativeModulePath[1], pathLength, "%s/lib%s_napi%s%s",
                prefix, dupModuleName, zfix, soPostfix) == -1) {
                return false;
            }
        } else {
#if !defined(WINDOWS_PLATFORM) && !defined(MAC_PLATFORM) && !defined(__BIONIC__) && !defined(IOS_PLATFORM) && \
    !defined(LINUX_PLATFORM)
            if (sprintf_s(nativeModulePath[0], pathLength, "lib%s%s", dupModuleName, soPostfix) == -1) {
                return false;
            }
#elif defined(ANDROID_PLATFORM)
            std::istringstream iss(prefix);
            std::string libPath;
            while (std::getline(iss, libPath, ':')) {
                std::ifstream dupModuleFile(libPath + "/lib" + dupModuleName + soPostfix);
                std::ifstream moduleFile(libPath + "/lib" + moduleName + soPostfix);
                if(dupModuleFile.good() || moduleFile.good()) {
                    break;
                }
            }
            if (sprintf_s(nativeModulePath[0], pathLength, "%s/lib%s%s", libPath.c_str(),
                dupModuleName, soPostfix) == -1) {
                return false;
            }
#else
            if (sprintf_s(nativeModulePath[0], pathLength, "%s/lib%s%s", prefix, dupModuleName, soPostfix) == -1) {
                return false;
            }
#endif
#ifdef ANDROID_PLATFORM
            if (sprintf_s(nativeModulePath[1], pathLength, "%s/lib%s%s", libPath.c_str(),
                moduleName, soPostfix) == -1) {
                return false;
            }
#endif
        }
    } else {
        char* afterDot = lastDot + 1;
        if (*afterDot == '\0') {
            return false;
        }
        *lastDot = '\0';
        lengthOfModuleName = strlen(dupModuleName);
        for (int32_t i = 0; i < lengthOfModuleName; i++) {
            if (*(dupModuleName + i) == '.') {
                *(dupModuleName + i) = '/';
            }
        }
        if (!isAppModule || !IsExistedPath(path)) {
            if (sprintf_s(nativeModulePath[0], pathLength, "%s/%s/lib%s%s%s",
                prefix, dupModuleName, afterDot, zfix, soPostfix) == -1) {
                return false;
            }
            if (sprintf_s(nativeModulePath[1], pathLength, "%s/%s/lib%s_napi%s%s",
                prefix, dupModuleName, afterDot, zfix, soPostfix) == -1) {
                return false;
            }
        } else {
#if !defined(WINDOWS_PLATFORM) && !defined(MAC_PLATFORM) && !defined(__BIONIC__) && !defined(IOS_PLATFORM) && \
    !defined(LINUX_PLATFORM)
            if (sprintf_s(nativeModulePath[0], pathLength, "lib%s%s", afterDot, soPostfix) == -1) {
                return false;
            }
#else
            if (sprintf_s(nativeModulePath[0], pathLength, "%s/%s/lib%s%s",
                prefix, dupModuleName, afterDot, soPostfix) == -1) {
                return false;
            }
#endif
#ifdef ANDROID_PLATFORM
            if (sprintf_s(nativeModulePath[1], pathLength, "%s/%s/lib%s%s",
                prefix, moduleName, afterDot, soPostfix) == -1) {
                return false;
            }
#endif
        }
    }
    return true;
}

LIBHANDLE NativeModuleManager::LoadModuleLibrary(std::string &moduleKey, const char* path,
                                                 const char* pathKey, const bool isAppModule)
{
    if (strlen(path) == 0) {
        HILOG_ERROR("primary module path is empty");
        return nullptr;
    }

    LIBHANDLE lib = nullptr;
    lib = GetNativeModuleHandle(moduleKey);
    if (lib != nullptr) {
        HILOG_DEBUG("get native module handle success. moduleKey is %{public}s", moduleKey.c_str());
        return lib;
    }

    HILOG_DEBUG("path: %{public}s, pathKey: %{public}s, isAppModule: %{public}d", path, pathKey, isAppModule);
#ifdef ENABLE_HITRACE
    StartTrace(HITRACE_TAG_ACE, path);
#endif
#if defined(WINDOWS_PLATFORM)
    lib = LoadLibrary(path);
    if (lib == nullptr) {
        HILOG_WARN("LoadLibrary failed, error: %{public}d", GetLastError());
    }
#elif defined(MAC_PLATFORM) || defined(__BIONIC__) || defined(LINUX_PLATFORM)
    lib = dlopen(path, RTLD_LAZY);
    if (lib == nullptr) {
        HILOG_WARN("dlopen failed: %{public}s", dlerror());
    }

#elif defined(IOS_PLATFORM)
    lib = nullptr;
#else
    if (isAppModule && IsExistedPath(pathKey)) {
        Dl_namespace ns = nsMap_[pathKey];
        lib = dlopen_ns(&ns, path, RTLD_LAZY);
    } else {
        lib = dlopen(path, RTLD_LAZY);
    }
    if (lib == nullptr) {
        HILOG_WARN("dlopen failed: %{public}s", dlerror());
    }
#endif
#ifdef ENABLE_HITRACE
    FinishTrace(HITRACE_TAG_ACE);
#endif
    EmplaceModuleLib(moduleKey, lib);

    return lib;
}

bool NativeModuleManager::UnloadModuleLibrary(LIBHANDLE handle)
{
    if (handle == nullptr) {
        HILOG_WARN("handle is nullptr");
        return false;
    }
#if !defined(WINDOWS_PLATFORM) && !defined(IOS_PLATFORM)
    if (!dlclose(handle)) {
        return true;
    }
    HILOG_WARN("dlclose failed: %{public}s", dlerror());
#endif
    return false;
}

NativeModule* NativeModuleManager::FindNativeModuleByDisk(
    const char* moduleName, const char* path, const char* relativePath, bool internal,  const bool isAppModule)
{
    char nativeModulePath[NATIVE_PATH_NUMBER][NAPI_PATH_MAX];
    nativeModulePath[0][0] = 0;
    nativeModulePath[1][0] = 0;
    if (!GetNativeModulePath(moduleName, path, relativePath, isAppModule, nativeModulePath, NAPI_PATH_MAX)) {
        HILOG_WARN("get module '%{public}s' path failed", moduleName);
        return nullptr;
    }
    if (moduleLoadChecker_ && !moduleLoadChecker_->CheckModuleLoadable(moduleName)) {
        HILOG_ERROR("module '%{public}s' is not allowed to load", moduleName);
        return nullptr;
    }

    std::string moduleKey(moduleName);
    if (isAppModule) {
        moduleKey = path;
        moduleKey = moduleKey + '/' + moduleName;
    }

    // load primary module path first
    char* loadPath = nativeModulePath[0];
    HILOG_DEBUG("moduleName: %{public}s. get primary module path: %{public}s", moduleName, loadPath);
    LIBHANDLE lib = LoadModuleLibrary(moduleKey, loadPath, path, isAppModule);
    if (lib == nullptr) {
        loadPath = nativeModulePath[1];
        HILOG_DEBUG("try to load secondary module path: %{public}s", loadPath);
        lib = LoadModuleLibrary(moduleKey, loadPath, path, isAppModule);
        if (lib == nullptr) {
            HILOG_ERROR("primary and secondary path load module '%{public}s' failed", moduleName);
            return nullptr;
        }
    }

    std::lock_guard<std::mutex> lock(nativeModuleListMutex_);
    if (lastNativeModule_ && strcmp(lastNativeModule_->name, moduleKey.c_str())) {
        HILOG_WARN("moduleName '%{public}s' seems not match plugin's name '%{public}s'",
                   moduleKey.c_str(), lastNativeModule_->name);
    }

    if (!internal) {
        char symbol[NAPI_PATH_MAX] = { 0 };
        if (sprintf_s(symbol, sizeof(symbol), "NAPI_%s_GetABCCode", moduleKey.c_str()) == -1) {
            LIBFREE(lib);
            return nullptr;
        }

        // replace '.' and '/' with '_'
        for (char* p = strchr(symbol, '.'); p != nullptr; p = strchr(p + 1, '.')) {
            *p = '_';
        }
        for (char* p = strchr(symbol, '/'); p != nullptr; p = strchr(p + 1, '/')) {
            *p = '_';
        }

        auto getJSCode = reinterpret_cast<GetJSCodeCallback>(LIBSYM(lib, symbol));
        if (getJSCode != nullptr) {
            const char* buf = nullptr;
            int bufLen = 0;
            getJSCode(&buf, &bufLen);
            if (lastNativeModule_ != nullptr) {
                HILOG_DEBUG("get js code from module: bufLen: %{public}d", bufLen);
                lastNativeModule_->jsCode = buf;
                lastNativeModule_->jsCodeLen = bufLen;
            }
        } else {
            HILOG_DEBUG("ignore: no %{public}s in %{public}s", symbol, loadPath);
        }
    }
    if (lastNativeModule_) {
        lastNativeModule_->moduleLoaded = true;
        HILOG_DEBUG("last native module name is %{public}s", lastNativeModule_->name);
    }
    return lastNativeModule_;
}

bool NativeModuleManager::RemoveNativeModuleByCache(const std::string &moduleKey)
{
    std::lock_guard<std::mutex> lock(nativeModuleListMutex_);

    if (firstNativeModule_ == nullptr) {
        HILOG_WARN("NativeModule list is empty");
        return false;
    }

    NativeModule* nativeModule = firstNativeModule_;
    if (!strcasecmp(nativeModule->name, moduleKey.c_str())) {
        firstNativeModule_ = firstNativeModule_->next;
        delete[] nativeModule->name;
        delete nativeModule;
        HILOG_DEBUG("module %{public}s deleted from cache", moduleKey.c_str());
        return true;
    }

    bool moduleDeleted = false;
    NativeModule* prev = firstNativeModule_;
    NativeModule* curr = prev->next;
    while (curr != nullptr) {
        if (!strcasecmp(curr->name, moduleKey.c_str())) {
            prev->next = curr->next;
            delete[] curr->name;
            delete curr;
            HILOG_DEBUG("module %{public}s deleted from cache", moduleKey.c_str());
            moduleDeleted = true;
            break;
        }
        prev = prev->next;
        curr = prev->next;
    }

    return moduleDeleted;
}

NativeModule* NativeModuleManager::FindNativeModuleByCache(const char* moduleName)
{
    NativeModule* result = nullptr;
    NativeModule* preNativeModule = nullptr;

    std::lock_guard<std::mutex> lock(nativeModuleListMutex_);
    for (NativeModule* temp = firstNativeModule_; temp != nullptr; temp = temp->next) {
        if (!strcasecmp(temp->name, moduleName)) {
            if (strcmp(temp->name, moduleName)) {
                HILOG_WARN("moduleName '%{public}s' seems not match plugin's name '%{public}s'",
                           moduleName, temp->name);
            }
            result = temp;
            break;
        }
        preNativeModule = temp;
    }

    if (result && !result->moduleLoaded) {
        if (result == lastNativeModule_) {
            HILOG_DEBUG("module '%{public}s' does not load", result->name);
            return nullptr;
        }
        if (preNativeModule) {
            preNativeModule->next = result->next;
        } else {
            firstNativeModule_ = firstNativeModule_->next;
        }
        result->next = nullptr;
        lastNativeModule_->next = result;
        lastNativeModule_ = result;
        HILOG_DEBUG("module '%{public}s' does not found", moduleName);
        return nullptr;
    }
    HILOG_DEBUG("module '%{public}s' found in cache", moduleName);
    return result;
}

bool NativeModuleManager::IsExistedPath(const char* pathKey) const
{
    HILOG_DEBUG("pathKey is '%{public}s'", pathKey);
    return pathKey && appLibPathMap_.find(pathKey) != appLibPathMap_.end();
}

void NativeModuleManager::SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate>& moduleCheckerDelegate)
{
    HILOG_DEBUG("enter");
    if (!moduleLoadChecker_) {
        HILOG_ERROR("SetModuleLoadChecker failed, moduleLoadChecker_ is nullptr");
        return;
    }
    moduleLoadChecker_->SetDelegate(moduleCheckerDelegate);
}

void NativeModuleManager::SetPreviewSearchPath(const std::string& previewSearchPath)
{
    HILOG_DEBUG("previewSearchPath is '%{public}s'", previewSearchPath.c_str());
    previewSearchPath_ = previewSearchPath;
}
