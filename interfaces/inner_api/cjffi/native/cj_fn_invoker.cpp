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

#include "cj_fn_invoker.h"

#include <iostream>

#include "hilog/log_cpp.h"

using namespace OHOS;
using namespace OHOS::HiviewDFX;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, 0xD003901, "CJ-FFIFuncs" };
} // namespace

CJFFIFnInvoker* CJFFIFnInvoker::GetInstance()
{
    static CJFFIFnInvoker instance;
    return &instance;
}

const FFIAtCPackage& CJFFIFnInvoker::GetCJFuncs()
{
    if (!fnLoaded_) {
        HiLog::Fatal(LABEL, "CJ-FFI functions not registered yet!");
    }
    return cjFFIFn_;
}

bool CJFFIFnInvoker::FFIDataExist(int64_t id) const
{
    auto cjFunc = cjFFIFn_.atCOHOSFFIExistFFIDataExist;
    if (!cjFunc) {
        HiLog::Fatal(LABEL, "CJ-FFI atCOHOSFFIExistFFIDataExist is unregistered!");
        return false;
    }
    return cjFunc(id);
}

bool CJFFIFnInvoker::ReleaseFFIData(int64_t id) const
{
    auto cjFunc = cjFFIFn_.atCOHOSFFIReleaseFFIData;
    if (!cjFunc) {
        HiLog::Fatal(LABEL, "CJ-FFI atCOHOSFFIReleaseFFIData is unregistered!");
        return false;
    }
    return cjFunc(id);
}

bool CJFFIFnInvoker::ReleaseRemoteData(int64_t id) const
{
    auto cjFunc = cjFFIFn_.atCOHOSFFIReleaseRemoteData;
    if (!cjFunc) {
        HiLog::Fatal(LABEL, "CJ-FFI atCOHOSFFIReleaseRemoteData is unregistered!");
        return false;
    }
    return cjFunc(id);
}

void CJFFIFnInvoker::ThrowCJError(const std::string& msg) const
{
    auto cjFunc = cjFFIFn_.atCOHOSFFICJThrow;
    if (!cjFunc) {
        HiLog::Fatal(LABEL, "CJ-FFI atCOHOSFFICJThrow is unregistered!");
        return;
    }
    cjFunc(msg.c_str());
}

void CJFFIFnInvoker::InvokeLambda(int64_t lambdaId, int32_t argc, void** argv, void* result) const
{
    auto cjFunc = cjFFIFn_.atCOHOSFFICallbackInvoker;
    if (!cjFunc) {
        HiLog::Fatal(LABEL, "CJ-FFI callback invoker atCOHOSFFICallbackInvoker is unregistered!");
        return;
    }
    cjFunc(lambdaId, argc, argv, result);
}
