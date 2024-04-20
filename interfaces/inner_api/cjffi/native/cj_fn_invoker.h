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

#ifndef OHOS_FFI_CJ_FN_INVOKER_H
#define OHOS_FFI_CJ_FN_INVOKER_H

#include <cstdint>
#include <string>

#include "nocopyable.h"

#ifndef FFI_EXPORT
#ifndef WINDOWS_PLATFORM
#define FFI_EXPORT __attribute__((visibility("default")))
#else
#define FFI_EXPORT __declspec(dllexport)
#endif
#endif

extern "C" {
struct FFIAtCPackage {
    void (*atCOHOSFFICJThrow)(const char* msg) = nullptr;
    bool (*atCOHOSFFIExistFFIDataExist)(int64_t id) = nullptr;
    bool (*atCOHOSFFIReleaseFFIData)(int64_t id) = nullptr;
    bool (*atCOHOSFFIReleaseRemoteData)(int64_t id) = nullptr;
    void (*atCOHOSFFICallbackInvoker)(int64_t lambdaId, int32_t argc, void** argv, void* res) = nullptr;
};
}

class FFI_EXPORT CJFFIFnInvoker {
public:
    DISALLOW_COPY_AND_MOVE(CJFFIFnInvoker);
    static CJFFIFnInvoker* GetInstance();

    void RegisterCJFuncs(FFIAtCPackage funcs)
    {
        cjFFIFn_ = funcs;
        fnLoaded_ = true;
    }

    const FFIAtCPackage& GetCJFuncs();

    bool CJFuncsLoaded() const
    {
        return fnLoaded_;
    }

    bool FFIDataExist(int64_t id) const;
    bool ReleaseFFIData(int64_t id) const;
    bool ReleaseRemoteData(int64_t id) const;
    void ThrowCJError(const std::string& msg) const;
    void InvokeLambda(int64_t lambdaId, int32_t argc, void** argv, void* result) const;

private:
    CJFFIFnInvoker() = default;
    bool fnLoaded_ = false;
    FFIAtCPackage cjFFIFn_;
};

#endif // OHOS_FFI_CJ_FN_INVOKER_H
