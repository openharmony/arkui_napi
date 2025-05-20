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

#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <unistd.h>

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
constexpr static int32_t MODULE_PATH_SECONDARY_INDEX = 2;
constexpr static int32_t NATIVE_PATH_NUMBER = 3;
constexpr static int32_t IS_APP_MODULE_FLAGS = 100;
thread_local bool g_isLoadingModule = false;
enum ModuleLoadFailedReason : uint32_t {
    MODULE_LOAD_SUCCESS = 0,
    MODULE_NOT_EXIST    = 1,
};
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
        NativeModule* nativeModule = headNativeModule_;
        while (nativeModule != nullptr) {
            nativeModule = nativeModule->next;
            if (headNativeModule_->name) {
                delete[] headNativeModule_->name;
            }
            if (headNativeModule_->moduleName) {
                delete[] headNativeModule_->moduleName;
            }
            if (headNativeModule_->jsABCCode) {
                delete[] headNativeModule_->jsABCCode;
            }
            if (headNativeModule_->systemFilePath && headNativeModule_->systemFilePath[0] != '\0') {
                free(const_cast<char *>(headNativeModule_->systemFilePath));
            }
            delete headNativeModule_;
            headNativeModule_ = nativeModule;
        }
        headNativeModule_ = nullptr;
        tailNativeModule_ = nullptr;
    }

#if !defined(WINDOWS_PLATFORM) && !defined(MAC_PLATFORM) && !defined(__BIONIC__) && !defined(IOS_PLATFORM) && \
    !defined(LINUX_PLATFORM)
    if (sharedLibsSonames_) {
        delete[] sharedLibsSonames_;
    }
#endif
    appLibPathMapMutex_.lock();
    for (const auto& item : appLibPathMap_) {
        delete[] item.second;
    }
    std::map<std::string, char*>().swap(appLibPathMap_);
    appLibPathMapMutex_.unlock();

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
    if (nativeEngine != nullptr) {
        nativeEngine->SetModuleName(moduleKey);
    }
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

LIBHANDLE NativeModuleManager::GetNativeModuleHandle(const std::string& moduleKey) const
{
    HILOG_DEBUG("moduleKey is '%{public}s'", moduleKey.c_str());
    std::lock_guard<std::mutex> lock(moduleLibMutex_);
    auto it = moduleLibMap_.find(moduleKey);
    if (it == moduleLibMap_.end()) {
        return nullptr;
    }
    return it->second;
}

void NativeModuleManager::EmplaceModuleBuffer(const std::string moduleKey, const uint8_t* lib)
{
    HILOG_DEBUG("modulekey is '%{public}s'", moduleKey.c_str());
    std::lock_guard<std::mutex> lock(moduleBufMutex_);
    if (lib != nullptr) {
        moduleBufMap_.emplace(moduleKey, lib);
    }
}

bool NativeModuleManager::RemoveModuleBuffer(const std::string moduleKey)
{
    HILOG_DEBUG("moduleKey is '%{public}s'", moduleKey.c_str());
    bool deleted = false;
    std::lock_guard<std::mutex> lock(moduleBufMutex_);
    auto it = moduleBufMap_.find(moduleKey);
    if (it != moduleBufMap_.end()) {
        moduleBufMap_.erase(it);
        HILOG_DEBUG("module '%{public}s' erased", moduleKey.c_str());
        deleted = true;
    }
    return deleted;
}

const uint8_t* NativeModuleManager::GetBufferHandle(const std::string& moduleKey) const
{
    HILOG_DEBUG("moduleKey is '%{public}s'", moduleKey.c_str());
    std::lock_guard<std::mutex> lock(moduleBufMutex_);
    auto it = moduleBufMap_.find(moduleKey);
    if (it == moduleBufMap_.end()) {
        return nullptr;
    }
    return it->second;
}

bool NativeModuleManager::RemoveNativeModule(const std::string& moduleKey)
{
    bool handleAbcRemoved = RemoveModuleBuffer(moduleKey);
    bool handleRemoved = RemoveModuleLib(moduleKey);
    bool moduleRemoved = RemoveNativeModuleByCache(moduleKey);

    HILOG_DEBUG("handleAbcRemoved is %{public}d, handleRemoved is %{public}d, moduleRemoved is %{public}d",
        handleAbcRemoved, handleRemoved, moduleRemoved);
    return ((handleRemoved || handleAbcRemoved) && moduleRemoved);
}

bool NativeModuleManager::UnloadNativeModule(const std::string& moduleKey)
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

std::string NativeModuleManager::GetModuleFileName(const char* moduleName, bool isAppModule)
{
    if (moduleName == nullptr) {
        HILOG_ERROR("invalid param. moduleName is nullptr");
        return "";
    }
    HILOG_INFO("moduleName is '%{public}s', isAppModule is %{public}d", moduleName, isAppModule);

    std::string loadPath;
    std::string name = isAppModule ? (prefix_ + "/" + moduleName) : moduleName;
    char nativeModulePath[NATIVE_PATH_NUMBER][NAPI_PATH_MAX];
    const char* pathKey = "default";
    if (!GetNativeModulePath(moduleName, pathKey, "", isAppModule, nativeModulePath, NAPI_PATH_MAX)) {
        HILOG_ERROR("get native module path failed");
        return loadPath;
    }
    NativeModule* cacheNativeModule = nullptr;
    NativeModuleHeadTailStruct cacheHeadTailNativeModule;
    NativeModule* module =
        FindNativeModuleByCache(name.c_str(), nativeModulePath, cacheNativeModule, cacheHeadTailNativeModule);
    if (module == nullptr) {
        HILOG_ERROR("get module file name failed");
        return loadPath;
    }
    loadPath = nativeModulePath[0];
    if (isAppModule && IsExistedPath(pathKey)) {
        std::lock_guard<std::mutex> guard(appLibPathMapMutex_);
        loadPath = std::string(appLibPathMap_[pathKey]) + "/" + nativeModulePath[0];
    }
    HILOG_ERROR("get module file name failed, moduleName is %{public}s", moduleName);
    return loadPath;
}

void NativeModuleManager::Register(NativeModule* nativeModule)
{
    if (nativeModule == nullptr) {
        HILOG_ERROR("nativeModule value is null");
        return;
    }

    HILOG_DEBUG("native module name is '%{public}s'", nativeModule->name);
    std::lock_guard<std::mutex> lock(nativeModuleListMutex_);
    const char *nativeModuleName = nativeModule->name == nullptr ? "" : nativeModule->name;
    std::string appName = prefix_ + "/" + nativeModuleName;
    std::string tmpName = isAppModule_ ? appName : nativeModuleName;
    if (nativeModule->flags == IS_APP_MODULE_FLAGS) {
        std::string prefix = "default/";
        tmpName = prefix + nativeModuleName;
    }
    char *moduleName = strdup(tmpName.c_str());
    if (moduleName == nullptr) {
        HILOG_ERROR("strdup failed. tmpName is %{public}s", tmpName.c_str());
        return;
    }

    if (g_isLoadingModule || !strcmp(loadingModuleName_.c_str(), moduleName)) {
        if (!CreateTailNativeModule()) {
            HILOG_ERROR("create tail nativeModule failed");
            free(moduleName);
            return;
        }
        tailNativeModule_->version = nativeModule->version;
        tailNativeModule_->fileName = nativeModule->fileName;
        tailNativeModule_->isAppModule = isAppModule_;
        tailNativeModule_->name = moduleName;
        tailNativeModule_->moduleName = nullptr;  /* we update moduleName latter */
        tailNativeModule_->refCount = nativeModule->refCount;
        tailNativeModule_->registerCallback = nativeModule->registerCallback;
        tailNativeModule_->getJSCode = nativeModule->getJSCode;
        tailNativeModule_->getABCCode = nativeModule->getABCCode;
        tailNativeModule_->next = nullptr;
        tailNativeModule_->moduleLoaded = true;
        tailNativeModule_->systemFilePath = "";
        if (isAppModule_) {
            HILOG_INFO("At tail register module name is '%{public}s'", tailNativeModule_->name);
        }
        HILOG_DEBUG("At tail register module name is '%{public}s', isAppModule is %{public}d", tailNativeModule_->name,
            isAppModule_);
    } else {
        if (!CreateHeadNativeModule()) {
            HILOG_ERROR("create head nativeModule failed");
            return;
        }
        headNativeModule_->version = nativeModule->version;
        headNativeModule_->fileName = nativeModule->fileName;
        headNativeModule_->isAppModule = isAppModule_;
        headNativeModule_->name = moduleName;
        headNativeModule_->refCount = nativeModule->refCount;
        headNativeModule_->registerCallback = nativeModule->registerCallback;
        headNativeModule_->getJSCode = nativeModule->getJSCode;
        headNativeModule_->getABCCode = nativeModule->getABCCode;
        headNativeModule_->moduleLoaded = true;
        headNativeModule_->systemFilePath = "";
        HILOG_INFO("At head register module name is '%{public}s', isAppModule is %{public}d",
            headNativeModule_->name, isAppModule_);
    }
}

bool NativeModuleManager::CreateHeadNativeModule()
{
    if (headNativeModule_ == tailNativeModule_ && tailNativeModule_ == nullptr) {
        headNativeModule_ = new NativeModule();
        if (headNativeModule_ == nullptr) {
            HILOG_ERROR("first NativeModule create failed");
            return false;
        }
        tailNativeModule_ = headNativeModule_;
    } else {
        auto head = new NativeModule();
        if (head == nullptr) {
            HILOG_ERROR("head NativeModule create failed");
            return false;
        }
        if (headNativeModule_) {
            head->next = headNativeModule_;
            headNativeModule_ = head;
        }
    }
    return true;
}

bool NativeModuleManager::CreateTailNativeModule()
{
    if (headNativeModule_ == tailNativeModule_ && tailNativeModule_ == nullptr) {
        headNativeModule_ = new NativeModule();
        if (headNativeModule_ == nullptr) {
            HILOG_ERROR("first NativeModule create failed");
            return false;
        }
        tailNativeModule_ = headNativeModule_;
    } else {
        auto tail = new NativeModule();
        if (tail == nullptr) {
            HILOG_ERROR("tail NativeModule create failed");
            return false;
        }
        if (tailNativeModule_) {
            tailNativeModule_->next = tail;
            tailNativeModule_ = tailNativeModule_->next;
        }
    }
    return true;
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
        "libclang_rt.tsan.so",
        "libclang_rt.ubsan_standalone.so",
        "libclang_rt.ubsan_minimal.so",
        "libclang_rt.hwasan.so",
        // z library
        "libace_napi.z.so",
        "libace_ndk.z.so",
        "libcj_environment.z.so",
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
        "libnative_avscreen_capture.so",
        "libavplayer.so",
        "libavrecorder.so",
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
        // Performs a namespace check on the full path passed directly or the full path converted after setting rpath.
        dlns_set_namespace_separated(nsName.c_str(), true);
        // Allows access to subdirectories of this directory for shared objects (so).
        dlns_set_namespace_permitted_paths(nsName.c_str(), lib_ld_path);
        // System app can visit all ndk and default ns libs.
        if (strlen(ndk_ns.name) > 0) {
            dlns_inherit(&ns, &ndk_ns, ALLOW_ALL_SHARED_LIBS);
            dlns_inherit(&ndk_ns, &current_ns, ALLOW_ALL_SHARED_LIBS);
            dlns_inherit(&current_ns, &ndk_ns, ALLOW_ALL_SHARED_LIBS);
            dlns_inherit(&ns, &current_ns, ALLOW_ALL_SHARED_LIBS);
        }
    } else {
        dlns_create2(&ns, lib_ld_path, 0);
        // Performs a namespace check on the full path passed directly or the full path converted after setting rpath.
        dlns_set_namespace_separated(nsName.c_str(), true);
        // Allows access to subdirectories of this directory for shared objects (so).
        dlns_set_namespace_permitted_paths(nsName.c_str(), lib_ld_path);
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

    HILOG_DEBUG("end. moduleName: %{public}s, path: %{public}s", moduleName.c_str(), lib_ld_path);
#endif
}

void NativeModuleManager::SetAppLibPath(const std::string& moduleName, const std::vector<std::string>& appLibPath,
                                        const bool& isSystemApp)
{
    HILOG_DEBUG("moduleName is %{public}s, isisSystemApp is %{public}d", moduleName.c_str(), isSystemApp);

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

    char *tmp = strdup(tmpPath.c_str());
    if (tmp == nullptr) {
        HILOG_ERROR("strdup failed. tmpPath is %{public}s", tmpPath.c_str());
        return;
    }

    std::lock_guard<std::mutex> guard(appLibPathMapMutex_);
    if (appLibPathMap_[moduleName] != nullptr) {
        delete[] appLibPathMap_[moduleName];
    }
    appLibPathMap_[moduleName] = tmp;
    CreateLdNamespace(moduleName, tmp, isSystemApp);
    HILOG_DEBUG("path: %{public}s", appLibPathMap_[moduleName]);
}

void NativeModuleManager::MoveApiAllowListCheckerPtr(
    std::unique_ptr<ApiAllowListChecker>& apiAllowListChecker, NativeModule* nativeModule)
{
    if (apiAllowListChecker != nullptr) {
        nativeModule->apiAllowListChecker.reset(apiAllowListChecker.release());
    }
}

NativeModule* NativeModuleManager::LoadNativeModule(const char* moduleName, const char* path, bool isAppModule,
    std::string& errInfo, bool internal, const char* relativePath)
{
    if (moduleName == nullptr) {
        errInfo = "load native module failed. moduleName is nullptr";
        HILOG_ERROR("%{public}s", errInfo.c_str());
        return nullptr;
    }

    if (relativePath == nullptr) {
        errInfo = "load native module failed. relativePath is nullptr";
        HILOG_ERROR("%{public}s", errInfo.c_str());
        return nullptr;
    }

    std::string relativePathStr(relativePath);
    if (relativePathStr.find("..") != std::string::npos) {
        HILOG_ERROR("get relativePath error,relativePath contains .., moduleName:%{public}s", moduleName);
        return nullptr;
    }

    HILOG_DEBUG("moduleName is %{public}s, path is %{public}s, relativePath is %{public}s",
        moduleName, path, relativePath);

    std::unique_ptr<ApiAllowListChecker> apiAllowListChecker = nullptr;
    if (moduleLoadChecker_ && !moduleLoadChecker_->DiskCheckOnly() &&
        !moduleLoadChecker_->CheckModuleLoadable(moduleName, apiAllowListChecker, isAppModule)) {
        errInfo = "module " + std::string(moduleName) + " is in blocklist, loading prohibited";
        HILOG_ERROR("%{public}s", errInfo.c_str());
        return nullptr;
    }
    std::string prefixTmp;
#if defined(ANDROID_PLATFORM)
    std::string strModule(moduleName);
    std::string strCutName = strModule;
    if (path != nullptr) {
        if (IsExistedPath(path)) {
            strModule = path;
        }
        prefixTmp = "default";
        strModule = prefixTmp + '/' + moduleName;
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

    char nativeModulePath[NATIVE_PATH_NUMBER][NAPI_PATH_MAX];
    nativeModulePath[0][0] = 0;
    nativeModulePath[1][0] = 0;
    nativeModulePath[2][0] = 0; // 2 : Element index value
    NativeModule* cacheNativeModule = nullptr;
    NativeModuleHeadTailStruct cacheHeadTailNativeModule;
#if defined(ANDROID_PLATFORM)
    if (!GetNativeModulePath(strCutName.c_str(), path, relativePath, isAppModule, nativeModulePath, NAPI_PATH_MAX)) {
        errInfo = "failed to get native file path of module " + std::string(moduleName);
        HILOG_WARN("%{public}s", errInfo.c_str());
        return nullptr;
    }
    NativeModule* nativeModule =
        FindNativeModuleByCache(strModule.c_str(), nativeModulePath, cacheNativeModule, cacheHeadTailNativeModule);
#else
    std::string key(moduleName);
    if (isAppModule) {
        prefixTmp = "default";
        if (path && IsExistedPath(path)) {
            prefixTmp = path;
        }
        key = prefixTmp + '/' + moduleName;
        HILOG_INFO("key is %{public}s", key.c_str());
    }
    if (!GetNativeModulePath(moduleName, prefixTmp.c_str(), relativePath, isAppModule, nativeModulePath,
                             NAPI_PATH_MAX)) {
        errInfo = "failed to get native file path of module " + std::string(moduleName);
        HILOG_WARN("%{public}s", errInfo.c_str());
        return nullptr;
    }
#ifdef ENABLE_HITRACE
    StartTrace(HITRACE_TAG_ACE, moduleName);
#endif
    NativeModule* nativeModule =
        FindNativeModuleByCache(key.c_str(), nativeModulePath, cacheNativeModule, cacheHeadTailNativeModule);
#endif
    if (nativeModule == nullptr) {
        (void)pthread_mutex_lock(&mutex_);
#ifndef IOS_PLATFORM
        if (CheckNativeListChanged(cacheHeadTailNativeModule.headNativeModule,
                                   cacheHeadTailNativeModule.tailNativeModule)) {
#ifdef ANDROID_PLATFORM
            nativeModule = FindNativeModuleByCache(strModule.c_str(), nativeModulePath, cacheNativeModule,
                                                   cacheHeadTailNativeModule);
#else
            nativeModule =
                FindNativeModuleByCache(key.c_str(), nativeModulePath, cacheNativeModule, cacheHeadTailNativeModule);
#endif
        }
#else
#endif
        if (nativeModule == nullptr) {
            prefix_ = prefixTmp;
            isAppModule_ = isAppModule;
            g_isLoadingModule = true;
#ifdef ANDROID_PLATFORM
            HILOG_DEBUG("module '%{public}s' does not in cache", strCutName.c_str());
            nativeModule = FindNativeModuleByDisk(strCutName.c_str(), path, relativePath, internal, isAppModule,
                                                  errInfo, nativeModulePath, cacheNativeModule);
#elif defined(IOS_PLATFORM)
            nativeModule =
                FindNativeModuleByCache(moduleName, nativeModulePath, cacheNativeModule, cacheHeadTailNativeModule);
            if (nativeModule == nullptr) {
                HILOG_DEBUG("module '%{public}s' does not in cache", moduleName);
                nativeModule = FindNativeModuleByDisk(moduleName, path, relativePath, internal, isAppModule, errInfo,
                                                      nativeModulePath, cacheNativeModule);
            }
#else
            HILOG_DEBUG("module '%{public}s' does not in cache", moduleName);
            nativeModule = FindNativeModuleByDisk(moduleName, prefix_.c_str(), relativePath, internal, isAppModule,
                                                  errInfo, nativeModulePath, cacheNativeModule);
#endif
            g_isLoadingModule = false;
        }
        (void)pthread_mutex_unlock(&mutex_);
    }
    if (nativeModule != nullptr && nativeModule->apiAllowListChecker == nullptr) {
        MoveApiAllowListCheckerPtr(apiAllowListChecker, nativeModule);
    }
#ifdef ENABLE_HITRACE
    FinishTrace(HITRACE_TAG_ACE);
#endif
    HILOG_DEBUG("load native module %{public}s", (nativeModule == nullptr) ? "failed" : "success");
    return nativeModule;
}

bool NativeModuleManager::CheckNativeListChanged(
    const NativeModule* cacheHeadNativeModule, const NativeModule* cacheTailNativeModule)
{
    std::lock_guard<std::mutex> lock(nativeModuleListMutex_);
    if (!cacheHeadNativeModule || !cacheTailNativeModule || !headNativeModule_ || !tailNativeModule_) {
        return true;
    }
    if (strcmp(cacheHeadNativeModule->name, headNativeModule_->name) != 0 ||
        strcmp(cacheTailNativeModule->name, tailNativeModule_->name) != 0) {
        return true;
    }

    return false;
}
bool NativeModuleManager::GetNativeModulePath(const char* moduleName, const char* path,
    const char* relativePath, bool isAppModule, char nativeModulePath[][NAPI_PATH_MAX], int32_t pathLength)
{
#ifdef WINDOWS_PLATFORM
    const char* soPostfix = ".dll";
    const char* zfix = "";
    std::string sysPrefix("./module");
    const char* abcfix = ".abc";
    std::string sysAbcPrefix("./module");
#elif defined(MAC_PLATFORM)
    const char* soPostfix = ".dylib";
    const char* zfix = "";
    std::string sysPrefix("./module");
    const char* abcfix = ".abc";
    std::string sysAbcPrefix("./module");
#elif defined(_ARM64_) || defined(SIMULATOR)
    const char* soPostfix = ".so";
    const char* zfix = ".z";
    std::string sysPrefix("/system/lib64/module");
    const char* abcfix = ".abc";
    std::string sysAbcPrefix("/system/etc/abc");
#elif defined(LINUX_PLATFORM)
    const char* soPostfix = ".so";
    const char* zfix = "";
    std::string sysPrefix("./module");
    const char* abcfix = ".abc";
    std::string sysAbcPrefix("./module");
#else
    const char* soPostfix = ".so";
    const char* zfix = ".z";
    std::string sysPrefix("/system/lib/module");
    const char* abcfix = ".abc";
    std::string sysAbcPrefix("/system/etc/abc");
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
        appLibPathMapMutex_.lock();
        prefix = appLibPathMap_[path];
        appLibPathMapMutex_.unlock();
#ifdef ANDROID_PLATFORM
        for (int32_t i = 0; i < lengthOfModuleName; i++) {
            dupModuleName[i] = tolower(dupModuleName[i]);
        }
        if (prefix == nullptr) {
            return false;
        }
        /*
         * The requirement is to ensure the abc format file is placed in the path
         * /data/user/0/com.example.myapplication/files/arkui-x/systemres/abc.
         * Steps to follow:
         * 1.	First, check if the /files/arkui-x directory exists.
         * 2.	Verify that the original path consists of two paths separated by a colon (:).
         * 3.	Identify whether /files/arkui-x is located in the portion before or after the colon.
         * 4.	Extract and concatenate the appropriate part of the path to construct
         *      the full abc file path for subsequent logic to access the file
         */
        std::string prefixStr(prefix);
        const std::string arkuiKey = "/files/arkui-x";
        std::size_t arkuiPos = prefixStr.find(arkuiKey);
        if (arkuiPos != std::string::npos) {
            std::size_t endPos = arkuiPos + arkuiKey.length();
            std::size_t delimiterPos = prefixStr.find(':');
            if (delimiterPos != std::string::npos) {
                sysAbcPrefix = (arkuiPos < delimiterPos)
                                   ? prefixStr.substr(0, endPos) + "/systemres/abc"
                                   : prefixStr.substr(delimiterPos + 1, endPos - (delimiterPos + 1)) +
                                        "/systemres/abc";
            } else {
                sysAbcPrefix = prefixStr.substr(0, endPos) + "/systemres/abc";
            }
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
#ifdef IOS_PLATFORM
        if (appLibPathMap_.find("default") != appLibPathMap_.end()) {
            std::string sandboxPath(appLibPathMap_["default"]);
            sysAbcPrefix = sandboxPath + "/systemres/abc";
        }
#endif
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
            
            if (sprintf_s(nativeModulePath[MODULE_PATH_SECONDARY_INDEX], pathLength, "%s/%s%s",
                sysAbcPrefix.c_str(), dupModuleName, abcfix) == -1) {
                return false;
            }
        } else {
#if !defined(WINDOWS_PLATFORM) && !defined(MAC_PLATFORM) && !defined(__BIONIC__) && !defined(IOS_PLATFORM) && \
    !defined(LINUX_PLATFORM)
            if (sprintf_s(nativeModulePath[0], pathLength, "lib%s%s", dupModuleName, soPostfix) == -1) {
                return false;
            }
#elif defined(ANDROID_PLATFORM)
            std::string libPath;
            int sprintfResult = 0;
            std::string prefixStr = std::string(prefix);
            std::size_t pos = prefixStr.find(':');
            if (pos != std::string::npos) {
                sprintfResult = sprintf_s(nativeModulePath[1], pathLength, "%s/lib%s%s",
                    prefixStr.substr(0, pos).c_str(), dupModuleName, soPostfix);
                libPath = prefixStr.substr(pos + 1);
            } else {
                sprintfResult = sprintf_s(nativeModulePath[1], pathLength, "lib%s%s", dupModuleName, soPostfix);
                libPath = prefixStr;
            }
            if (sprintfResult == -1) {
                return false;
            }
#else
            if (sprintf_s(nativeModulePath[0], pathLength, "%s/lib%s%s", prefix, dupModuleName, soPostfix) == -1) {
                return false;
            }
#endif
#ifdef ANDROID_PLATFORM
            if (sprintf_s(nativeModulePath[0], pathLength, "%s/lib%s%s", libPath.c_str(),
                dupModuleName, soPostfix) == -1) {
                return false;
            }
            char* lastUnderScore = strrchr(dupModuleName, '_');
            const char* moduleNamePart = (lastUnderScore != nullptr) ? (lastUnderScore + 1) : dupModuleName;
            if (sprintf_s(nativeModulePath[MODULE_PATH_SECONDARY_INDEX], pathLength, "%s/%s%s",
                sysAbcPrefix.c_str(), moduleNamePart, abcfix) == -1) {
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
#ifdef IOS_PLATFORM
            if (sprintf_s(nativeModulePath[MODULE_PATH_SECONDARY_INDEX], pathLength, "%s/%s%s",
                sysAbcPrefix.c_str(), afterDot, abcfix) == -1) {
                return false;
            }
#else
            if (sprintf_s(nativeModulePath[2], pathLength, "%s/%s/%s%s", // 2 : Element index value
                sysAbcPrefix.c_str(), dupModuleName, afterDot, abcfix) == -1) {
                return false;
            }
#endif
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

LIBHANDLE NativeModuleManager::LoadModuleLibrary(std::string& moduleKey, const char* path,
    const char* pathKey, const bool isAppModule, std::string& errInfo, uint32_t& errReason)
{
    if (strlen(path) == 0) {
        errInfo += "load module " + moduleKey  + " failed. module path is empty";
        HILOG_ERROR("%{public}s", errInfo.c_str());
        return nullptr;
    }

    LIBHANDLE lib = nullptr;

    HILOG_DEBUG("path: %{public}s, pathKey: %{public}s, isAppModule: %{public}d", path, pathKey, isAppModule);
#ifdef ENABLE_HITRACE
    StartTrace(HITRACE_TAG_ACE, path);
#endif
#if defined(WINDOWS_PLATFORM)
    if (CheckModuleExist(path) == false) {
        errReason = MODULE_NOT_EXIST;
        return nullptr;
    }
    lib = LoadLibrary(path);
    if (lib == nullptr) {
        errInfo += "load module failed. " + std::to_string(GetLastError());
        HILOG_WARN("%{public}s", errInfo.c_str());
    }
#elif defined(MAC_PLATFORM) || defined(__BIONIC__) || defined(LINUX_PLATFORM)
#ifndef ANDROID_PLATFORM
    if (CheckModuleExist(path) == false) {
        errReason = MODULE_NOT_EXIST;
        return nullptr;
    }
#endif
    lib = dlopen(path, RTLD_LAZY);
    if (lib == nullptr) {
        char* dlerr = dlerror();
        auto dlerrMsg = dlerr != nullptr ? dlerr : "dlerror msg is empty";
        errInfo += "load module failed. " +  std::string(dlerrMsg);
    }

#elif defined(IOS_PLATFORM)
    lib = nullptr;
#else
    if (isAppModule && IsExistedPath(pathKey)) {
        Dl_namespace ns = nsMap_[pathKey];
        lib = dlopen_ns(&ns, path, RTLD_LAZY);
    } else if (access(path, F_OK) == 0) {
        lib = dlopen(path, RTLD_LAZY);
    }
    if (lib == nullptr) {
        char* dlerr = dlerror();
        auto dlerrMsg = dlerr != nullptr ? dlerr :
            "Error loading path " + std::string(path) + ":No such file or directory";
        errInfo += "load app module failed. " +  std::string(dlerrMsg);
    }
#endif
#ifdef ENABLE_HITRACE
    FinishTrace(HITRACE_TAG_ACE);
#endif
    EmplaceModuleLib(moduleKey, lib);

    return lib;
}

const uint8_t* NativeModuleManager::GetFileBuffer(const std::string& filePath,
    const std::string& moduleKey, size_t &len)
{
    const uint8_t* lib = nullptr;
    std::ifstream inFile(filePath, std::ios::ate | std::ios::binary);
    if (!inFile.is_open()) {
        HILOG_ERROR("%{public}s is not existed.", filePath.c_str());
        return lib;
    }
    len = static_cast<size_t>(inFile.tellg());
    std::string abcModuleKey = moduleKey;
    lib = GetBufferHandle(abcModuleKey);
    if (lib != nullptr) {
        HILOG_DEBUG("get native abc handle success. moduleKey is %{public}s", moduleKey.c_str());
        inFile.close();
        return lib;
    }

    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(len);
    inFile.seekg(0);
    inFile.read(reinterpret_cast<char*>(buffer.get()), len);
    inFile.close();
    lib = buffer.release();
    EmplaceModuleBuffer(abcModuleKey, lib);
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
    char* dlerr = dlerror();
    auto dlerrMsg = dlerr != nullptr ? dlerr : "dlerror msg is empty";
    HILOG_WARN("dlclose failed: %{public}s", dlerrMsg);
#endif
    return false;
}

bool NativeModuleManager::CheckModuleExist(const char* modulePath)
{
    if (modulePath) {
        std::ifstream inFile(modulePath, std::ios::ate | std::ios::binary);
        if (inFile.is_open()) {
            inFile.close();
            return true;
        }
    }
    return false;
}

void NativeModuleManager::Napi_onLoadCallback(LIBHANDLE lib, const char* moduleName)
{
    auto onLoadFunc = reinterpret_cast<NapiOnLoadCallback>(LIBSYM(lib, "napi_onLoad"));
    if (onLoadFunc != nullptr) {
        onLoadFunc();
        HILOG_INFO("napi_onLoad call, moduleName:%{public}s", moduleName);
    }
}

NativeModule* NativeModuleManager::FindNativeModuleByDisk(const char* moduleName, const char* path,
    const char* relativePath, bool internal, const bool isAppModule, std::string& errInfo,
    char nativeModulePath[][NAPI_PATH_MAX], NativeModule* cacheNativeModule)
{
    std::unique_ptr<ApiAllowListChecker> apiAllowListChecker = nullptr;
    if (moduleLoadChecker_ && !moduleLoadChecker_->CheckModuleLoadable(moduleName, apiAllowListChecker, isAppModule)) {
        errInfo = "module " + std::string(moduleName) + " is in blocklist, loading prohibited";
        HILOG_WARN("%{public}s", errInfo.c_str());
        return nullptr;
    }

    std::string moduleKey(moduleName);
    if (isAppModule) {
        moduleKey = path;
        moduleKey = moduleKey + '/' + moduleName;
    }
    loadingModuleName_ = moduleKey;

    // load primary module path first
    char* loadPath = nativeModulePath[0];
    HILOG_DEBUG("moduleName is %{public}s. get primary module path is %{public}s", moduleName, loadPath);
    uint32_t errReason0 = MODULE_LOAD_SUCCESS;
    errInfo = "First attempt: ";
    LIBHANDLE lib = LoadModuleLibrary(moduleKey, loadPath, path, isAppModule, errInfo, errReason0);
    if (lib == nullptr) {
        errInfo += "\nSecond attempt: ";
        loadPath = nativeModulePath[1];
        HILOG_DEBUG("try to load secondary module path: %{public}s", loadPath);
        uint32_t errReason1 = MODULE_LOAD_SUCCESS;
        lib = LoadModuleLibrary(moduleKey, loadPath, path, isAppModule, errInfo, errReason1);
        if (lib == nullptr && errReason0 == MODULE_NOT_EXIST && errReason1 == MODULE_NOT_EXIST) {
            HILOG_ERROR("%{public}s does not exist, errMsg %{public}s", nativeModulePath[0], errInfo.c_str());
        }
    }

    //Maintain compatibility
    if (lib == nullptr && cacheNativeModule != nullptr) {
        HILOG_DEBUG("Maintain compatibility.");
        return cacheNativeModule;
    }

    const uint8_t* abcBuffer = nullptr;
    size_t len = 0;
    if (lib == nullptr) {
        loadPath = nativeModulePath[2]; // 2 : Element index value
        HILOG_DEBUG("try to load abc module path: %{public}s", loadPath);
        abcBuffer = GetFileBuffer(loadPath, moduleKey, len);
        if (!abcBuffer) {
            errInfo += "\ntry to load abc file from " + std::string(loadPath) + " failed";
            HILOG_ERROR("%{public}s", errInfo.c_str());
            return nullptr;
        }
    }

    if (lib != nullptr) {
        Napi_onLoadCallback(lib, moduleName);
    }
    
    std::lock_guard<std::mutex> lock(nativeModuleListMutex_);
    if (tailNativeModule_ && !abcBuffer) {
        const char* moduleName = strdup(moduleKey.c_str());
        if (moduleName == nullptr) {
            HILOG_ERROR("strdup failed. moduleKey is %{public}s", moduleKey.c_str());
            return nullptr;
        }

        tailNativeModule_->moduleName = moduleName;
        tailNativeModule_->systemFilePath = strdup(loadPath);
        if (strcmp(tailNativeModule_->moduleName, tailNativeModule_->name)) {
            HILOG_WARN("mismatch: moduleName is %{public}s, name is %{public}s",
                tailNativeModule_->moduleName, tailNativeModule_->name);
            HILOG_WARN("suggestion: keep .nm_modname the same as moduleName imported or required");
        }
    }

    if (!internal) {
        char symbol[NAPI_PATH_MAX] = { 0 };
        if (sprintf_s(symbol, sizeof(symbol), "NAPI_%s_GetABCCode", moduleKey.c_str()) == -1) {
            if (lib != nullptr) {
                LIBFREE(lib);
            }
            errInfo = "sprintf symbol NAPI_" + moduleKey + "_GetABCCode failed";
            HILOG_ERROR("%{public}s", errInfo.c_str());
            return nullptr;
        }

        // replace '.' and '/' with '_'
        for (char* p = strchr(symbol, '.'); p != nullptr; p = strchr(p + 1, '.')) {
            *p = '_';
        }
        for (char* p = strchr(symbol, '/'); p != nullptr; p = strchr(p + 1, '/')) {
            *p = '_';
        }

        if (lib != nullptr) {
            auto getJSCode = reinterpret_cast<GetJSCodeCallback>(LIBSYM(lib, symbol));
            if (getJSCode == nullptr) {
                HILOG_DEBUG("ignore: no %{public}s in %{public}s", symbol, loadPath);
                MoveApiAllowListCheckerPtr(apiAllowListChecker, tailNativeModule_);
                return tailNativeModule_;
            }
            const char* buf = nullptr;
            int bufLen = 0;
            getJSCode(&buf, &bufLen);
            if (tailNativeModule_) {
                HILOG_DEBUG("get js code from module: bufLen: %{public}d", bufLen);
                tailNativeModule_->jsCode = buf;
                tailNativeModule_->jsCodeLen = bufLen;
            }
        } else {
            RegisterByBuffer(moduleKey, abcBuffer, len);
            tailNativeModule_->systemFilePath = strdup(loadPath);
        }
    }
    if (tailNativeModule_) {
        tailNativeModule_->moduleLoaded = true;
        if (tailNativeModule_->name && tailNativeModule_->moduleName) {
            HILOG_DEBUG("last native info: name is %{public}s, moduleName is %{public}s",
                tailNativeModule_->name, tailNativeModule_->moduleName);
        }
        MoveApiAllowListCheckerPtr(apiAllowListChecker, tailNativeModule_);
    }
    return tailNativeModule_;
}

void NativeModuleManager::RegisterByBuffer(const std::string& moduleKey, const uint8_t* abcBuffer, size_t len)
{
    HILOG_DEBUG("native module name is '%{public}s'", moduleKey.c_str());
    if (!CreateTailNativeModule()) {
        HILOG_ERROR("create tail nativeModule failed");
        return;
    }

    char *moduleName = strdup(moduleKey.c_str());
    if (moduleName == nullptr) {
        HILOG_ERROR("strdup failed. moduleKey is %{public}s", moduleKey.c_str());
        return;
    }
    tailNativeModule_->moduleName = moduleName;
    tailNativeModule_->name = strdup(moduleName);
    if (tailNativeModule_->name == nullptr) {
        HILOG_ERROR("strdup failed. moduleKey is %{public}s", moduleName);
        free(moduleName);
        tailNativeModule_->moduleName = nullptr;
        return;
    }
    tailNativeModule_->jsABCCode = abcBuffer;
    tailNativeModule_->jsCodeLen = static_cast<int32_t>(len);
    tailNativeModule_->next = nullptr;

    HILOG_INFO("Register by buffer success. module name is '%{public}s'", tailNativeModule_->moduleName);
}

bool NativeModuleManager::RemoveNativeModuleByCache(const std::string& moduleKey)
{
    std::lock_guard<std::mutex> lock(nativeModuleListMutex_);
    if (headNativeModule_ == nullptr) {
        HILOG_WARN("NativeModule list is empty");
        return false;
    }
    NativeModule* nativeModule = headNativeModule_;
    if (!strcasecmp(nativeModule->moduleName, moduleKey.c_str())) {
        if (headNativeModule_ == tailNativeModule_) {
            tailNativeModule_ = nullptr;
        }
        headNativeModule_ = headNativeModule_->next;
        delete[] nativeModule->name;
        if (nativeModule->moduleName) {
            delete[] nativeModule->moduleName;
        }
        if (nativeModule->jsABCCode) {
            delete[] nativeModule->jsABCCode;
        }
        if (nativeModule->systemFilePath && nativeModule->systemFilePath[0] != '\0') {
            free(const_cast<char *>(nativeModule->systemFilePath));
        }
        delete nativeModule;
        HILOG_DEBUG("module %{public}s deleted from cache", moduleKey.c_str());
        return true;
    }
    bool moduleDeleted = false;
    NativeModule* prev = headNativeModule_;
    NativeModule* curr = prev->next;
    while (curr != nullptr) {
        if (!strcasecmp(curr->moduleName, moduleKey.c_str())) {
            if (curr == tailNativeModule_) {
                tailNativeModule_ = prev;
            }
            prev->next = curr->next;
            delete[] curr->name;
            if (curr->moduleName) {
                delete[] curr->moduleName;
            }
            if (curr->jsABCCode) {
                delete[] curr->jsABCCode;
            }
            if (curr->systemFilePath && curr->systemFilePath[0] != '\0') {
                free(const_cast<char *>(curr->systemFilePath));
            }
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

NativeModule* NativeModuleManager::FindNativeModuleByCache(const char* moduleName,
                                                           char nativeModulePath[][NAPI_PATH_MAX],
                                                           NativeModule*& cacheNativeModule,
                                                           NativeModuleHeadTailStruct& cacheHeadTailStruct)
{
    NativeModule* result = nullptr;

    std::lock_guard<std::mutex> lock(nativeModuleListMutex_);
    cacheNativeModule = nullptr;
    for (NativeModule* temp = headNativeModule_; temp != nullptr; temp = temp->next) {
        if ((temp->moduleName && !strcmp(temp->moduleName, moduleName))
            || !strcasecmp(temp->name, moduleName)) {
            int label = 0;
#if !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
            while (label < NATIVE_PATH_NUMBER && strcmp(temp->systemFilePath, nativeModulePath[label])) {
                label++;
            }
#endif
            if (label < NATIVE_PATH_NUMBER || !strcmp(temp->systemFilePath, "")) {
                result = temp;
                break;
            } else {
                HILOG_WARN("moduleName '%{public}s' is in different path", moduleName);
                cacheNativeModule = temp;
            }
        }
    }
    cacheHeadTailStruct.headNativeModule = headNativeModule_;
    cacheHeadTailStruct.tailNativeModule = tailNativeModule_;

    return result;
}

bool NativeModuleManager::IsExistedPath(const char* pathKey) const
{
    HILOG_DEBUG("pathKey is '%{public}s'", pathKey);
    std::lock_guard<std::mutex> guard(appLibPathMapMutex_);
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
