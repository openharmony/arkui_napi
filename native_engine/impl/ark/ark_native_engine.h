/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_ARK_NATIVE_ENGINE_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_ARK_NATIVE_ENGINE_H

#include <memory>
#if !defined(PREVIEW) && !defined(IOS_PLATFORM)
#include <sys/wait.h>
#include <sys/types.h>
#endif
#include <iostream>
#include <map>
#include <mutex>
#include <regex>
#include <thread>
#include <unistd.h>

#include "ark_idle_monitor.h"
#include "ark_native_options.h"
#include "ecmascript/napi/include/dfx_jsnapi.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "native_engine/impl/ark/ark_finalizers_pack.h"
#include "native_engine/native_engine.h"

namespace panda::ecmascript {
struct JsHeapDumpWork;
struct JsFrameInfo {
    std::string functionName;
    std::string fileName;
    std::string packageName;
    std::string pos;
    uintptr_t* nativePointer = nullptr;
};
struct ApiCheckContext {
    NativeModuleManager* moduleManager;
    EcmaVM* ecmaVm;
    panda::Local<panda::StringRef>& moduleName;
    panda::Local<panda::ObjectRef>& exportObj;
    panda::EscapeLocalScope& scope;
};
}
using JsFrameInfo = panda::ecmascript::JsFrameInfo;
using DFXJSNApi = panda::DFXJSNApi;
using LocalScope = panda::LocalScope;
using JSNApi = panda::JSNApi;
using JSValueRef = panda::JSValueRef;
using JsiRuntimeCallInfo = panda::JsiRuntimeCallInfo;
using PropertyAttribute = panda::PropertyAttribute;
using NativePointerCallbackData = panda::NativePointerCallbackData;
using AsyncNativeCallbacksPack = panda::AsyncNativeCallbacksPack;
using TriggerGCTaskCallback = panda::TriggerGCTaskCallback;
using TriggerGCData = panda::TriggerGCData;
using ArkIdleMonitor = panda::ecmascript::ArkIdleMonitor;

// indirect used by ace_engine and(or) ability_runtime
using panda::Local;

template <bool changeState = true>
panda::JSValueRef ArkNativeFunctionCallBack(JsiRuntimeCallInfo *runtimeInfo);
void NapiDefinePropertyInner(napi_env env,
                             Local<panda::ObjectRef> &obj,
                             NapiPropertyDescriptor &propertyDescriptor,
                             Local<panda::JSValueRef> &propertyName,
                             bool &result);
bool NapiDefineProperty(napi_env env, panda::Local<panda::ObjectRef> &obj, NapiPropertyDescriptor propertyDescriptor);
NAPI_EXPORT panda::Local<panda::JSValueRef> NapiValueToLocalValue(napi_value v);
NAPI_EXPORT napi_value LocalValueToLocalNapiValue(panda::Local<panda::JSValueRef> local);
#ifdef ENABLE_CONTAINER_SCOPE
void FunctionSetContainerId(const EcmaVM *vm, panda::Local<panda::JSValueRef> &local);
#endif
panda::Local<panda::JSValueRef> NapiDefineClass(napi_env env, const char* name, NapiNativeCallback callback,
                                                void* data, const NapiPropertyDescriptor* properties, size_t length);
Local<JSValueRef> NapiDefineSendableClass(napi_env env,
                                          const char* name,
                                          NapiNativeCallback callback,
                                          void* data,
                                          const NapiPropertyDescriptor* properties,
                                          size_t propertiesLength,
                                          napi_value parent);
panda::Local<panda::ObjectRef> NapiCreateSObjectWithProperties(napi_env env,
                                                               size_t propertyCount,
                                                               const NapiPropertyDescriptor* properties);
panda::Local<panda::ObjectRef> NapiCreateObjectWithProperties(napi_env env, size_t propertyCount,
                                                              const napi_property_descriptor *properties,
                                                              Local<panda::JSValueRef> *keys,
                                                              panda::PropertyAttribute *attrs);

enum class ForceExpandState : int32_t {
    FINISH_COLD_START = 0,
    START_HIGH_SENSITIVE,
    FINISH_HIGH_SENSITIVE,
};

enum class ModuleTypes : uint8_t {
    NATIVE_MODULE = 0x01,
    MODULE_INNER_FILE,
    UNKNOWN
};

class SerializationData {
public:
    SerializationData() : data_(nullptr), size_(0) {}
    ~SerializationData() = default;

    uint8_t* GetData() const
    {
        return data_.get();
    }
    size_t GetSize() const
    {
        return size_;
    }

private:
    struct DataDeleter {
        void operator()(uint8_t* p) const
        {
            free(p);
        }
    };

    std::unique_ptr<uint8_t, DataDeleter> data_;
    size_t size_;
};

class NAPI_EXPORT ArkNativeEngine : public NativeEngine {
friend struct MoudleNameLocker;
public:
    // ArkNativeEngine constructor
    ArkNativeEngine(EcmaVM* vm, void* jsEngine, bool isLimitedWorker = false);
    // ArkNativeEngine destructor
    ~ArkNativeEngine() override;

    NAPI_EXPORT const EcmaVM* GetEcmaVm() const override
    {
        return vm_;
    }

    void Loop(LoopMode mode, bool needSync = false) override;
    void SetPromiseRejectCallback(NativeReference* rejectCallbackRef, NativeReference* checkCallbackRef) override;
    // For concurrent
    bool InitTaskPoolThread(NativeEngine* engine, NapiConcurrentCallback callback) override;
    bool InitTaskPoolThread(napi_env env, NapiConcurrentCallback callback) override;
    bool InitTaskPoolFunc(napi_env env, napi_value func, void* taskInfo) override;
    void ClearCurrentTaskInfo() override;
    bool HasPendingJob() const override;
    bool IsProfiling() const override;
    bool IsExecutingPendingJob() const override;
    void* GetCurrentTaskInfo() const override;
    void TerminateExecution() const override;
    void NotifyTaskBegin() const override;
    void NotifyTaskFinished() const override;

    // judge_typedarray
    bool NapiNewTypedArray(const EcmaVM* vm, NativeTypedArrayType typedArrayType,
                           Local<panda::ArrayBufferRef> arrayBuf,
                           size_t byte_offset, size_t length, napi_value* result) override;
    bool NapiNewSendableTypedArray(const EcmaVM* vm, NativeTypedArrayType typedArrayType,
                                   Local<panda::SendableArrayBufferRef> arrayBuf,
                                   size_t byte_offset, size_t length, napi_value* result) override;
    NativeTypedArrayType GetTypedArrayType(panda::Local<panda::TypedArrayRef> typedArray) override;
    NativeTypedArrayType GetSendableTypedArrayType(panda::Local<panda::SendableTypedArrayRef> typedArray) override;
    // Call function
    napi_value CallFunction(napi_value thisVar,
                            napi_value function,
                            napi_value const* argv,
                            size_t argc) override;
    bool RunScriptPath(const char* path, bool checkPath = false) override;

    napi_value RunScriptBuffer(const char* path, std::vector<uint8_t>& buffer, bool isBundle) override;
    bool RunScriptBuffer(const std::string& path, uint8_t* buffer, size_t size, bool isBundle) override;

    // Run buffer script
    napi_value RunBufferScript(std::vector<uint8_t>& buffer) override;
    napi_value RunActor(uint8_t* buffer, size_t size, const char* descriptor,
        char* entryPoint = nullptr, bool checkPath = false) override;
    // Set lib path
    NAPI_EXPORT void SetPackagePath(const std::string appLinPathKey, const std::vector<std::string>& packagePath);
    napi_value CreateInstance(napi_value constructor, napi_value const* argv, size_t argc) override;

    // Create native reference
    NativeReference* CreateReference(napi_value value, uint32_t initialRefcount, bool flag = false,
        NapiNativeFinalize callback = nullptr, void* data = nullptr, void* hint = nullptr,
        size_t nativeBindingSize = 0) override;
    NativeReference* CreateAsyncReference(napi_value value, uint32_t initialRefcount, bool flag = false,
        NapiNativeFinalize callback = nullptr, void* data = nullptr, void* hint = nullptr) override;
    napi_value CreatePromise(NativeDeferred** deferred) override;
    void* CreateRuntime(bool isLimitedWorker = false) override;
    panda::Local<panda::ObjectRef> LoadArkModule(const void *buffer, int32_t len, const std::string& fileName);
    napi_value ValueToNapiValue(JSValueWrapper& value) override;
    NAPI_EXPORT static napi_value ArkValueToNapiValue(napi_env env, Local<JSValueRef> value);

    std::string GetSourceCodeInfo(napi_value value, ErrorPos pos) override;

    NAPI_EXPORT bool ExecuteJsBin(const std::string& fileName, bool checkPath = false);
    static bool IsValidPandaFile(const char* path);
    static bool IsValidScriptBuffer(uint8_t* scriptBuffer, size_t bufferSize);
    NAPI_EXPORT panda::Local<panda::ObjectRef> LoadModuleByName(const std::string& moduleName, bool isAppModule,
        const std::string& param, const std::string& instanceName, void* instance, const std::string& path = "");

    void TriggerFatalException(panda::Local<panda::JSValueRef> exceptionValue) override;
    bool AdjustExternalMemory(int64_t ChangeInBytes, int64_t* AdjustedValue) override;

    // Set jsdump thresholds
    void SetJsDumpThresholds(size_t thresholds) override;

    // Set Appfreeze Filter
    void SetAppFreezeFilterCallback(AppFreezeFilterCallback callback) override;

    // Detect performance to obtain cpuprofiler file
    void StartCpuProfiler(const std::string& fileName = "") override;
    void StopCpuProfiler() override;

    void ResumeVM() override;
    bool SuspendVM() override;
    bool IsSuspended() override;
    bool CheckSafepoint() override;
    bool SuspendVMById(uint32_t tid) override;
    void ResumeVMById(uint32_t tid) override;

    // isVmMode means the internal class in vm is visible.
    // isPrivate means the number and string is not visible.
    void DumpHeapSnapshot(const std::string& path, bool isVmMode = true,
        DumpFormat dumpFormat = DumpFormat::JSON, bool isPrivate = false, bool captureNumericValue = false) override;
    void DumpCpuProfile() override;
    // Dump the file into faultlog for heap leak.
    void DumpHeapSnapshot(bool isVmMode = true, DumpFormat dumpFormat = DumpFormat::JSON,
        bool isPrivate = false, bool isFullGC = true) override;
    bool BuildNativeAndJsStackTrace(std::string& stackTraceStr) override;
    bool BuildJsStackTrace(std::string& stackTraceStr) override;
    bool BuildJsStackInfoListWithCustomDepth(std::vector<JsFrameInfo>& jsFrames)
        override;

    bool DeleteWorker(NativeEngine* workerEngine) override;
    bool StartHeapTracking(double timeInterval, bool isVmMode = true) override;
    bool StopHeapTracking(const std::string& filePath) override;

    void PrintStatisticResult() override;
    void StartRuntimeStat() override;
    void StopRuntimeStat() override;
    size_t GetArrayBufferSize() override;
    size_t GetHeapTotalSize() override;
    size_t GetHeapUsedSize() override;
    size_t GetHeapObjectSize() override;
    size_t GetHeapLimitSize() override;
    size_t GetProcessHeapLimitSize() override;
    size_t GetGCCount() override;
    size_t GetGCDuration() override;
    size_t GetAccumulatedAllocateSize() override;
    size_t GetAccumulatedFreeSize() override;
    size_t GetFullGCLongTimeCount() override;
    void NotifyApplicationState(bool inBackground) override;
    void NotifyIdleStatusControl(std::function<void(bool)> callback) override;
    void NotifyIdleTime(int idleMicroSec) override;
    void NotifyMemoryPressure(bool inHighMemoryPressure = false) override;
    void NotifyForceExpandState(int32_t value) override;

    void AllowCrossThreadExecution() const override;
    static void PromiseRejectCallback(void* values);

    void SetMockModuleList(const std::map<std::string, std::string> &list) override;
    int32_t GetObjectHash(napi_env env, napi_value src) override;

    // debugger
    bool IsMixedDebugEnabled();
    void JsHeapStart();
    uint64_t GetCurrentTickMillseconds();
    bool JudgmentDumpExecuteTask(int pid);
    void JudgmentDump(size_t limitSize);
    void NotifyNativeCalling(const void *nativeAddress);

    void PostFinalizeTasks();
    void PostAsyncTask(AsyncNativeCallbacksPack *callbacksPack);
    void PostTriggerGCTask(TriggerGCData& data);

    ArkFinalizersPack &GetArkFinalizersPack()
    {
        return arkFinalizersPack_;
    }

    std::vector<RefFinalizer> &GetPendingAsyncFinalizers()
    {
        return pendingAsyncFinalizers_;
    }

    void RegisterNapiUncaughtExceptionHandler(NapiUncaughtExceptionCallback callback) override;
    void HandleUncaughtException() override;
    bool HasPendingException() override;
    void RegisterPermissionCheck(PermissionCheckCallback callback) override;
    bool ExecutePermissionCheck() override;
    void RegisterTranslateBySourceMap(SourceMapCallback callback) override;
    std::string ExecuteTranslateBySourceMap(const std::string& rawStack) override;
        void RegisterSourceMapTranslateCallback(SourceMapTranslateCallback callback) override;
    panda::Local<panda::ObjectRef> GetModuleFromName(
        const std::string& moduleName, bool isAppModule, const std::string& id, const std::string& param,
        const std::string& instanceName, void** instance);
    napi_value NapiLoadModule(const char* path) override;
    napi_value NapiLoadModuleWithInfo(const char* path, const char* module_info) override;
    std::string GetOhmurl(std::string str);
    Local<JSValueRef> NapiLoadNativeModule(std::string path);
    NativeReference* GetPromiseRejectCallBackRef()
    {
        return promiseRejectCallbackRef_;
    }

    void RegisterAllPromiseCallback(NapiAllPromiseRejectCallback callback) override
    {
        allPromiseRejectCallback_ = callback;
    }

    void SetPromiseRejectCallBackRef(NativeReference* rejectCallbackRef) override
    {
        promiseRejectCallbackRef_ = rejectCallbackRef;
    }

    NapiConcurrentCallback GetConcurrentCallbackFunc()
    {
        return concurrentCallbackFunc_;
    }

    NativeReference* GetCheckCallbackRef()
    {
        return checkCallbackRef_;
    }

    void SetCheckCallbackRef(NativeReference* checkCallbackRef) override
    {
        checkCallbackRef_ = checkCallbackRef;
    }

    NapiUncaughtExceptionCallback GetNapiUncaughtExceptionCallback() override
    {
        return napiUncaughtExceptionCallback_;
    }

    void* GetPromiseRejectCallback() override
    {
        return reinterpret_cast<void*>(PromiseRejectCallback);
    }

    void SetModuleName(panda::Local<panda::ObjectRef> &nativeObj, std::string moduleName);
    void GetCurrentModuleInfo(std::string& moduleName, std::string& fileName, bool needRecordName) override;
    bool GetIsBundle() override;
    bool GetIsNormalizedOhmUrlPack() override;
    bool GetIsDebugModeEnabled() override;
    std::string GetBundleName() override;
    std::string GetPkgName(const std::string &moduleName) override;
    bool IsExecuteModuleInAbcFile(std::string bundleName, std::string moduleName, std::string ohmurl) override;
    int GetProcessStartRealTime() override;
    void PostLooperTriggerIdleGCTask();

    static bool napiProfilerEnabled;
    static std::string tempModuleName_;

    static void* GetNativePtrCallBack(void* data);
    static void CopyPropertyApiFilter(const std::unique_ptr<ApiAllowListChecker>& apiAllowListChecker,
        const EcmaVM* ecmaVm, const panda::Local<panda::ObjectRef> exportObj,
        panda::Local<panda::ObjectRef>& exportCopy, const std::string& apiPath);

    inline bool IsCrossThreadCheckEnabled() const override
    {
        return crossThreadCheck_;
    }
    static constexpr size_t FINALIZERS_PACK_PENDING_NATIVE_BINDING_SIZE_THRESHOLD = 500 * 1024 * 1024;  // 500 MB

private:
    inline NapiOptions *GetNapiOptions() const override
    {
        return options_;
    }

    static void RunCallbacks(ArkFinalizersPack *finalizersPack);
    static void RunAsyncCallbacks(std::vector<RefFinalizer> *finalizers);
    static void RunCallbacks(AsyncNativeCallbacksPack *callbacks);
    static void RunCallbacks(TriggerGCData *triggerGCData);
    static void SetAttribute(bool isLimitedWorker, panda::RuntimeOption &option);
    static NativeEngine* CreateRuntimeFunc(NativeEngine* engine, void* jsEngine, bool isLimitedWorker = false);
    static bool CheckArkApiAllowList(
        NativeModule* module, panda::ecmascript::ApiCheckContext context, panda::Local<panda::ObjectRef>& exportCopy);
    void IncreasePendingFinalizersPackNativeBindingSize(size_t nativeBindingSize)
    {
        pendingFinalizersPackNativeBindingSize_ += nativeBindingSize;
    }
    void DecreasePendingFinalizersPackNativeBindingSize(size_t nativeBindingSize)
    {
        pendingFinalizersPackNativeBindingSize_ -= nativeBindingSize;
    }

    EcmaVM* vm_ = nullptr;
    bool needStop_ = false;
    panda::LocalScope topScope_;
    NapiConcurrentCallback concurrentCallbackFunc_ { nullptr };
    NativeReference* promiseRejectCallbackRef_ { nullptr };
    NativeReference* checkCallbackRef_ { nullptr };
    std::map<NativeModule*, panda::Global<panda::JSValueRef>> loadedModules_ {};
    static PermissionCheckCallback permissionCheckCallback_;
    NapiUncaughtExceptionCallback napiUncaughtExceptionCallback_ { nullptr };
    NapiAllPromiseRejectCallback allPromiseRejectCallback_ {nullptr};
    SourceMapCallback SourceMapCallback_ { nullptr };
    static bool napiProfilerParamReaded;
    bool isLimitedWorker_ = false;
    size_t pendingFinalizersPackNativeBindingSize_ {0};
    ArkFinalizersPack arkFinalizersPack_ {};
    std::vector<RefFinalizer> pendingAsyncFinalizers_ {};
    // napi options and its cache
    NapiOptions* options_ { nullptr };
    bool crossThreadCheck_ { false };
};
#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_ARK_NATIVE_ENGINE_H */
