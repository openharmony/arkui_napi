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

#include <native_engine/native_engine.h>

#include "node_api.h"
#include "ark_interop_log.h"
#include "cj_envsetup.h"

#include <mutex>
#include <map>

#ifdef USE_LIBS_ARM64
#define LIBS_NAME "arm64"
#elif defined(USE_LIBS_ARM)
#define LIBS_NAME "arm"
#elif defined(USE_LIBS_x86_64)
#define LIBS_NAME "x86_64"
#else
#error current platform not supported
#endif

static std::mutex g_mutex;
static bool g_isInited = false;
static std::map<napi_env, std::map<std::string, napi_ref>> envMap = {};

static bool ParseLoadParams(napi_env env, napi_callback_info info, char* nameBuf, size_t& size)
{
    napi_value libNameValue;
    size_t argCount = 1;
    auto status = napi_get_cb_info(env, info, &argCount, &libNameValue, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "napi call fail", "napi_get_cb_info failed");
        return false;
    }

    if (argCount != 1) {
        napi_throw_error(env, "argument invalid", "need one argument");
        return false;
    }

    napi_valuetype valuetype;
    status = napi_typeof(env, libNameValue, &valuetype);
    if (status != napi_ok) {
        napi_throw_error(env, "napi call fail", "napi_typeof failed");
        return false;
    }
    if (valuetype != napi_string) {
        napi_throw_error(env, "argument invalid", "need a string");
        return false;
    }

    status = napi_get_value_string_utf8(env, libNameValue, nameBuf, size, &size);
    if (status != napi_ok) {
        napi_throw_error(env, "argument invalid", "napi_get_value_string_utf8 not ok");
        return false;
    }
    return true;
}

static bool LoadArkCJModule(void *handle, napi_env env, const char* libName, napi_value* result)
{
    auto engine = reinterpret_cast<NativeEngine*>(env);
    std::lock_guard<std::mutex> lock(g_mutex);
    std::string str(libName);
    if (envMap.find(env) != envMap.end() && envMap[env].find(str) != envMap[env].end()) {
        napi_value callback = nullptr;
        napi_get_reference_value(env, envMap[env][str], &callback);
        *result = callback;
        return true;
    }
    auto runtime = OHOS::CJEnv::LoadInstance();
    if (handle == nullptr) {
        LOGE("get handle failed");
        return false;
    }
    if (runtime == nullptr) {
        LOGE("load Instance failed");
        return false;
    }
    if (auto symbol = runtime->getSymbol(handle, "ARKTS_LoadModuleByNapiEnv")) {
        auto loader = reinterpret_cast<napi_value(*)(napi_env, const char*)>(symbol);
        *result = loader(env, libName);
    } else if (auto symbol = runtime->getSymbol(handle, "ARKTS_LoadModule")) {
        auto loader = reinterpret_cast<napi_value(*)(EcmaVM*, const char*)>(symbol);
        auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());
        *result = loader(vm, libName);
    } else {
        return false;
    }
    napi_ref callbackRef = nullptr;
    napi_create_reference(env, *result, 1, &callbackRef);
    envMap[env][str] = callbackRef;

    return true;
}

static void *GetArkInteropLibHandle(napi_env env)
{
    const char* targetName;
#ifdef __OHOS__
    targetName = "libark_interop.z.so";
#elif defined(__WINDOWS__)
    targetName = "libark_interop.dll";
#elif defined(__LINUX__)
    targetName = "libark_interop.so";
#endif
    auto runtime = OHOS::CJEnv::LoadInstance();
    if (!runtime) {
        LOGE("load Instance failed");
        return nullptr;
    }
    auto handle = runtime->loadLibrary(0, targetName);
    if (!handle) {
        LOGE("open '%{public}s' failed", targetName);
        return nullptr;
    }
    return handle;
}

static bool RegisterStackInfoCallbacks(void *arkInteropLibHandle)
{
    auto runtime = OHOS::CJEnv::LoadInstance();
    if (!runtime) {
        LOGE("load Instance failed");
        return false;
    }
    auto updateStackInfoFunc = runtime->getSymbol(arkInteropLibHandle, "ARKTS_UpdateStackInfo");
    if (updateStackInfoFunc == nullptr) {
        LOGE("load symbol ARKTS_UpdateStackInfo failed");
        return false;
    }
    runtime->registerStackInfoCallbacks(
        reinterpret_cast<void(*)(unsigned long long, void *, unsigned int)>(updateStackInfoFunc)
    );
    return true;
}

static napi_value LoadCJModule(napi_env env, napi_callback_info info)
{
    napi_value result;
    if (napi_get_undefined(env, &result) != napi_ok) {
        return result;
    }
    constexpr size_t BUF_SIZE = 256;
    char nameBuf[BUF_SIZE];
    size_t realSize = BUF_SIZE;
    void *arkInteropHandle = nullptr;
    if (!ParseLoadParams(env, info, nameBuf, realSize)) {
        return result;
    }
    if (!g_isInited) {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (!g_isInited) {
            auto runtime = OHOS::CJEnv::LoadInstance();
            if (!runtime) {
                LOGE("load Instance failed");
                return result;
            }
            runtime->initCJChipSDKNS("/system/lib64/chipset-pub-sdk");
            runtime->initCJAppNS("/data/storage/el1/bundle/libs/" LIBS_NAME);
            runtime->initCJSDKNS("/data/storage/el1/bundle/libs/" LIBS_NAME "/ohos:"
                                 "/data/storage/el1/bundle/libs/" LIBS_NAME "/runtime");
            runtime->initCJSysNS("/system/lib64:/system/lib64/platformsdk:/system/lib64/module:/system/lib64/ndk");
            if (!runtime->startRuntime()) {
                LOGE("start cjruntime failed");
                return result;
            }
            arkInteropHandle = GetArkInteropLibHandle(env);
            RegisterStackInfoCallbacks(arkInteropHandle);
            auto engine = reinterpret_cast<NativeEngine*>(env);
            auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());
            auto vmAddr = static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(vm));
            runtime->registerArkVMInRuntime(vmAddr);
            if (!runtime->startUIScheduler()) {
                LOGE("start cj ui context failed");
                return result;
            }
            g_isInited = true;
        }
    }
    
    LoadArkCJModule(arkInteropHandle, env, nameBuf, &result);
    return result;
}

static napi_value ExportLoadCJModule(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        {"requireCJLib", nullptr, LoadCJModule, nullptr, nullptr, nullptr, napi_default, nullptr}
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc);
    return exports;
}

extern "C" __attribute__((constructor)) void LoadCJModuleRegister()
{
    static napi_module callbackModule = {
        .nm_version = 1,
        .nm_flags = 0,
        .nm_filename = nullptr,
        .nm_register_func = ExportLoadCJModule,
        .nm_modname = "cjLibLoader",
        .nm_priv = nullptr,
        .reserved = { 0 },
    };
    napi_module_register(&callbackModule);
}
