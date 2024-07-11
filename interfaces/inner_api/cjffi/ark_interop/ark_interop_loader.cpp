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

#include "node_api.h"
#include "ark_native_engine.h"
#include "ark_interop_log.h"
#include "cj_envsetup.h"

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

static bool LoadArkCJModule(napi_env env, const char* libName, napi_value* result)
{
    auto engine = reinterpret_cast<ArkNativeEngine*>(env);
    auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());
    do {
        const char* targetName;
        const char* loaderName = "ARKTS_LoadModule";
#ifdef __OHOS__
        targetName = "libark_interop.z.so";
#elif defined(__WINDOWS__)
        targetName = "libark_interop.dll";
#elif defined(__LINUX__)
        targetName = "libark_interop.so";
#endif
        auto runtime = OHOS::CJEnv::LoadInstance();
        auto handle = runtime->loadLibrary(0, targetName);
        if (!handle) {
            LOGE("open '%{public}s' failed", targetName);
            return false;
        }
        auto symbol = runtime->getSymbol(handle, loaderName);
        if (!symbol) {
            LOGE("no symbol of '%{public}s'", loaderName);
            return false;
        }
        auto loader = reinterpret_cast<napi_value(*)(EcmaVM*, const char*)>(symbol);
        *result = loader(vm, libName);
    } while (false);
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
    if (!ParseLoadParams(env, info, nameBuf, realSize)) {
        return result;
    }
    auto runtime = OHOS::CJEnv::LoadInstance();
    runtime->initCJChipSDKNS("/system/lib64/chipset-pub-sdk");
    runtime->initCJAppNS("/data/storage/el1/bundle/libs/arm64");
    runtime->initCJSDKNS("/data/storage/el1/bundle/libs/arm64/ohos:"
                                 "/data/storage/el1/bundle/libs/arm64/runtime");
    runtime->initCJSysNS("/system/lib64:/system/lib64/platformsdk:/system/lib64/module:/system/lib64/ndk");
    if (!runtime->startRuntime()) {
        LOGE("start cjruntime failed");
        return result;
    }
    if (!runtime->startUIScheduler()) {
        LOGE("start cj ui context failed");
        return result;
    }

    LoadArkCJModule(env, nameBuf, &result);
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
        .nm_modname = "ark_interop_loader",
        .nm_priv = nullptr,
        .reserved = { 0 },
    };
    napi_module_register(&callbackModule);
}
