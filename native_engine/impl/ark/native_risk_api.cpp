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

#include "ark_native_engine.h"

#ifdef ENABLE_HITRACE
#include <sys/prctl.h>
#endif

#include <sstream>
#include "ark_native_deferred.h"
#if !defined(is_arkui_x) && defined(OHOS_PLATFORM)
#include "unwinder.h"
#endif
#include "ark_native_reference.h"
#include "native_engine/native_property.h"
#include "native_engine/native_utils.h"
#include "native_sendable.h"
#include "securec.h"
#include "utils/log.h"
#if !defined(PREVIEW) && !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
#include "parameters.h"
#include <uv.h>
#endif
#ifdef ENABLE_CONTAINER_SCOPE
#include "core/common/container_scope.h"
#endif

using panda::JsiRuntimeCallInfo;
using panda::BooleanRef;
using panda::ObjectRef;
using panda::StringRef;
using panda::Global;
using panda::JSNApi;
using panda::FunctionRef;
using panda::PrimitiveRef;
using panda::ArrayBufferRef;
using panda::TypedArrayRef;
using panda::PromiseCapabilityRef;
using panda::PropertyAttribute;
using panda::NativePointerRef;
using panda::SymbolRef;
using panda::IntegerRef;
using panda::DateRef;
using panda::BigIntRef;

static constexpr auto PANDA_MAIN_FUNCTION = "_GLOBAL::func_main_0";

#define EXECUTE_BUFFER(functionName)                                                                 \
    if (panda::JSNApi::IsBundle(vm_)) {                                                              \
        /* FA doesn't enable securemem */                                                            \
        ret = panda::JSNApi::Execute(vm_, buffer, bufferSize, PANDA_MAIN_FUNCTION, desc);            \
    } else if (bufferSize != 0) {                                                                    \
        if (entryPoint == nullptr) {                                                                 \
            HILOG_DEBUG("Input entryPoint is nullptr, please input entryPoint for merged ESModule"); \
            /* this path for bundle and abc compiled by single module js */                          \
            ret = panda::JSNApi::functionName(vm_, buffer, bufferSize, PANDA_MAIN_FUNCTION, desc);   \
        } else {                                                                                     \
            /* this path for mergeabc with specific entryPoint */                                    \
            /* entryPoint: bundleName/moduleName/xxx/xxx */                                          \
            panda::JSNApi::SetModuleInfo(vm_, desc, std::string(entryPoint));                        \
            if (panda::JSNApi::HasPendingException(vm_)) {                                           \
                HandleUncaughtException();                                                           \
                return nullptr;                                                                      \
            }                                                                                        \
            ret = panda::JSNApi::functionName(vm_, buffer, bufferSize, entryPoint, desc);            \
        }                                                                                            \
    } else {                                                                                         \
        /* this path for worker */                                                                   \
        ret = panda::JSNApi::Execute(vm_, desc, PANDA_MAIN_FUNCTION);                                \
    }

 // The security interface needs to be modified accordingly.
napi_value ArkNativeEngine::RunScriptBuffer(const char* path, std::vector<uint8_t>& buffer, bool isBundle)
{
    if (path == nullptr) {
        HILOG_ERROR("path is nullptr");
        return nullptr;
    }
    panda::EscapeLocalScope scope(vm_);
    [[maybe_unused]] bool ret = false;
    if (isBundle) {
        ret = panda::JSNApi::Execute(vm_, buffer.data(), buffer.size(), PANDA_MAIN_FUNCTION, path);
    } else {
        ret = panda::JSNApi::ExecuteModuleBuffer(vm_, buffer.data(), buffer.size(), path);
    }

    if (panda::JSNApi::HasPendingException(vm_)) {
        HandleUncaughtException();
        return nullptr;
    }
    Local<JSValueRef> undefObj = JSValueRef::Undefined(vm_);
    return JsValueFromLocalValue(scope.Escape(undefObj));
}

bool ArkNativeEngine::RunScriptBuffer(const std::string& path, uint8_t* buffer, size_t size, bool isBundle)
{
    if (path.size() == 0 || buffer == nullptr) {
        HILOG_ERROR("path is empty or buffer is null");
        return false;
    }
    panda::JSExecutionScope executionScope(vm_);
    LocalScope scope(vm_);
    bool ret = false;
    if (isBundle) {
        ret = panda::JSNApi::ExecuteSecure(vm_, buffer, size, PANDA_MAIN_FUNCTION, path);
    } else {
        ret = panda::JSNApi::ExecuteModuleBufferSecure(vm_, buffer, size, path);
    }

    if (panda::JSNApi::HasPendingException(vm_)) {
        HandleUncaughtException();
        return false;
    }
    return ret;
}

bool ArkNativeEngine::RunScriptPath(const char* path)
{
    if (path == nullptr) {
        HILOG_ERROR("path is nullptr");
        return false;
    }
    panda::JSExecutionScope executionScope(vm_);
    LocalScope scope(vm_);
    [[maybe_unused]] bool ret = panda::JSNApi::Execute(vm_, path, PANDA_MAIN_FUNCTION);
    if (panda::JSNApi::HasPendingException(vm_)) {
        HandleUncaughtException();
        return false;
    }
    return true;
}

napi_value ArkNativeEngine::RunBufferScript(std::vector<uint8_t>& buffer)
{
    panda::EscapeLocalScope scope(vm_);
    [[maybe_unused]] bool ret = panda::JSNApi::Execute(vm_, buffer.data(), buffer.size(), PANDA_MAIN_FUNCTION);

    if (panda::JSNApi::HasPendingException(vm_)) {
        HandleUncaughtException();
        return nullptr;
    }
    Local<JSValueRef> undefObj = JSValueRef::Undefined(vm_);
    return JsValueFromLocalValue(scope.Escape(undefObj));
}

napi_value ArkNativeEngine::RunActor(uint8_t* buffer, size_t bufferSize, const char* descriptor, char* entryPoint)
{
    panda::EscapeLocalScope scope(vm_);
    std::string desc(descriptor);
    [[maybe_unused]] bool ret = false;
    // if apiVersion > API11, use secure path.
    if (IsApplicationApiVersionAPI11Plus()) {
        EXECUTE_BUFFER(ExecuteSecure);
    } else {
        EXECUTE_BUFFER(Execute);
    }
    if (panda::JSNApi::HasPendingException(vm_)) {
        HandleUncaughtException();
        return nullptr;
    }
    Local<JSValueRef> undefObj = JSValueRef::Undefined(vm_);
    return JsValueFromLocalValue(scope.Escape(undefObj));
}

bool ArkNativeEngine::ExecuteJsBin(const std::string& fileName)
{
    panda::JSExecutionScope executionScope(vm_);
    LocalScope scope(vm_);
    bool ret = JSNApi::Execute(vm_, fileName, PANDA_MAIN_FUNCTION);
    return ret;
}

panda::Local<panda::ObjectRef> ArkNativeEngine::LoadArkModule(const void* buffer,
    int32_t len, const std::string& fileName)
{
    panda::EscapeLocalScope scope(vm_);
    Local<ObjectRef> undefObj(JSValueRef::Undefined(vm_));
    if (buffer == nullptr || len <= 0 || fileName.empty()) {
        HILOG_ERROR("fileName is nullptr or source code is nullptr");
        return scope.Escape(undefObj);
    }

    bool res = JSNApi::ExecuteModuleFromBuffer(vm_, buffer, len, fileName);
    if (!res) {
        HILOG_ERROR("Execute module failed");
        return scope.Escape(undefObj);
    }

    Local<ObjectRef> exportObj = JSNApi::GetExportObjectFromBuffer(vm_, fileName, "default");
    if (exportObj->IsNull()) {
        HILOG_ERROR("Get export object failed");
        return scope.Escape(undefObj);
    }

    HILOG_DEBUG("LoadModule end");
    return scope.Escape(exportObj);
}