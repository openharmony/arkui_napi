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

#include <unordered_map>

#include "ark_headers.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "ecmascript/napi/include/dfx_jsnapi.h"
#include "native_engine/native_engine.h"

namespace panda::ecmascript {
struct JsFrameInfo {
    std::string functionName;
    std::string fileName;
    std::string pos;
    uintptr_t* nativePointer = nullptr;
};
}
using ArkJsFrameInfo = panda::ecmascript::JsFrameInfo;

using panda::ecmascript::EcmaVM;
using panda::Local;
using panda::LocalScope;
using panda::JSValueRef;
using panda::JSNApi;
using panda::DFXJSNApi;

enum class ForceExpandState : int32_t {
    FINISH_COLD_START = 0,
    START_HIGH_SENSITIVE,
    FINISH_HIGH_SENSITIVE,
};

class ArkNativeObject;

NAPI_EXPORT Local<JSValueRef> NapiValueToLocalValue(napi_value v);

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
    ArkNativeEngine(EcmaVM* vm, void* jsEngine);
    // ArkNativeEngine destructor
    ~ArkNativeEngine() override;

    const EcmaVM* GetEcmaVm() const;

    void Loop(LoopMode mode, bool needSync = false) override;

    // Get global native object value
    NativeValue* GetGlobal() override;
    // Create native null value
    NativeValue* CreateNull() override;
    // Create native undefined value
    NativeValue* CreateUndefined() override;
    // Create native boolean value
    NativeValue* CreateBoolean(bool value) override;
    // Create number value by int32_t
    NativeValue* CreateNumber(int32_t value) override;
    // Create number value by uint32_t
    NativeValue* CreateNumber(uint32_t value) override;
    // Create native number value by int64_t
    NativeValue* CreateNumber(int64_t value) override;
    // Create native number value by double
    NativeValue* CreateNumber(double value) override;
    // Create native bigint value by int64_t
    NativeValue* CreateBigInt(int64_t value) override;
    // Create native bigint value by uint64_t
    NativeValue* CreateBigInt(uint64_t value) override;
    // Create native string value by const char pointer
    NativeValue* CreateString(const char* value, size_t length) override;
    // Create native string value by const char16_t pointer
    NativeValue* CreateString16(const char16_t* value, size_t length) override;
    // Create native symbol value
    NativeValue* CreateSymbol(NativeValue* value) override;
    // Create native value of external pointer
    NativeValue* CreateExternal(void* value, NativeFinalize callback, void* hint,
        size_t nativeBindingSize = 0) override;
    // Create native object value
    NativeValue* CreateObject() override;
    // Create native function value
    NativeValue* CreateFunction(const char* name, size_t length, NativeCallback cb, void* value) override;
    // Create native array value
    NativeValue* CreateArray(size_t length) override;
    // Create native array buffer value
    NativeValue* CreateArrayBuffer(void** value, size_t length) override;
    // Create native array buffer value of external
    NativeValue* CreateArrayBufferExternal(void* value, size_t length, NativeFinalize cb, void* hint) override;
    NativeValue* CreateBuffer(void** value, size_t length) override;
    NativeValue* CreateBufferCopy(void** value, size_t length, const void* data) override;
    NativeValue* CreateBufferExternal(void* value, size_t length, NativeFinalize cb, void* hint) override;
    // Create native typed array value
    NativeValue* CreateTypedArray(NativeTypedArrayType type,
                                  NativeValue* value,
                                  size_t length,
                                  size_t offset) override;
    // Create native data view value
    NativeValue* CreateDataView(NativeValue* value, size_t length, size_t offset) override;
    // Create native promise value
    NativeValue* CreatePromise(NativeDeferred** deferred) override;
    void SetPromiseRejectCallback(NativeReference* rejectCallbackRef, NativeReference* checkCallbackRef) override;
    // Create native error value
    NativeValue* CreateError(NativeValue* code, NativeValue* message) override;
    // For concurrent
    bool InitTaskPoolThread(NativeEngine* engine, NapiConcurrentCallback callback) override;
    bool InitTaskPoolThread(napi_env env, NapiConcurrentCallback callback) override;
    bool InitTaskPoolFunc(NativeEngine* engine, NativeValue* func, void* taskInfo) override;
    bool InitTaskPoolFunc(napi_env env, napi_value func, void* taskInfo) override;
    bool HasPendingJob() override;
    bool IsProfiling() override;
    void* GetCurrentTaskInfo() const override;
    // Call function
    NativeValue* CallFunction(NativeValue* thisVar,
                              NativeValue* function,
                              NativeValue* const* argv,
                              size_t argc) override;
    // Run script
    NativeValue* RunScript(NativeValue* script) override;
    NativeValue* RunScriptPath(const char* path) override;

    NativeValue* RunScriptBuffer(const char* path, std::vector<uint8_t>& buffer, bool isBundle) override;
    bool RunScriptBuffer(const std::string& path, uint8_t* buffer, size_t size, bool isBundle) override;

    // Run buffer script
    NativeValue* RunBufferScript(std::vector<uint8_t>& buffer) override;
    NativeValue* RunActor(std::vector<uint8_t>& buffer, const char* descriptor) override;
    // Set lib path
    void SetPackagePath(const std::string appLinPathKey, const std::vector<std::string>& packagePath);
    // Define native class
    NativeValue* DefineClass(const char* name,
                             NativeCallback callback,
                             void* data,
                             const NativePropertyDescriptor* properties,
                             size_t length) override;
    // Create instance by defined class
    NativeValue* CreateInstance(NativeValue* constructor, NativeValue* const* argv, size_t argc) override;

    // Create native reference
    NativeReference* CreateReference(NativeValue* value, uint32_t initialRefcount,
        NativeFinalize callback = nullptr, void* data = nullptr, void* hint = nullptr) override;
    bool IsExceptionPending() const override;
    NativeValue* GetAndClearLastException() override;
    // Throw exception
    bool Throw(NativeValue* error) override;
    // Throw exception
    bool Throw(NativeErrorType type, const char* code, const char* message) override;

    void* CreateRuntime() override;
    NativeValue* Serialize(NativeEngine* context, NativeValue* value, NativeValue* transfer) override;
    NativeValue* Deserialize(NativeEngine* context, NativeValue* recorder) override;
    void DeleteSerializationData(NativeValue* value) const override;
    NativeValue* LoadModule(NativeValue* str, const std::string& fileName) override;
    NativeValue* LoadArkModule(const char* str, int32_t len, const std::string& fileName);

    static NativeValue* ArkValueToNativeValue(ArkNativeEngine* engine, Local<JSValueRef> value);

    napi_value ValueToNapiValue(JSValueWrapper& value) override;
    NativeValue* ValueToNativeValue(JSValueWrapper& value) override;
    static napi_value ArkValueToNapiValue(napi_env env, Local<JSValueRef> value);

    std::string GetSourceCodeInfo(napi_value value, ErrorPos pos) override;

    bool ExecuteJsBin(const std::string& fileName);
    panda::Local<panda::ObjectRef> LoadModuleByName(const std::string& moduleName, bool isAppModule,
        const std::string& param, const std::string& instanceName, void* instance, const std::string& path = "");

    bool TriggerFatalException(NativeValue* error) override;
    NativeValue* CreateDate(double value) override;
    NativeValue* CreateBigWords(int sign_bit, size_t word_count, const uint64_t* words) override;
    bool AdjustExternalMemory(int64_t ChangeInBytes, int64_t* AdjustedValue) override;

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
        DumpFormat dumpFormat = DumpFormat::JSON) override;
    // Dump the file into faultlog for heap leak.
    void DumpHeapSnapshot(bool isVmMode = true, DumpFormat dumpFormat = DumpFormat::JSON,
        bool isPrivate = false) override;
    bool BuildNativeAndJsStackTrace(std::string& stackTraceStr) override;
    bool BuildJsStackTrace(std::string& stackTraceStr) override;
    bool BuildJsStackInfoList(uint32_t tid, std::vector<JsFrameInfo>& jsFrames) override;

    bool DeleteWorker(NativeEngine* workerEngine) override;
    bool StartHeapTracking(double timeInterval, bool isVmMode = true) override;
    bool StopHeapTracking(const std::string& filePath) override;

    void PrintStatisticResult() override;
    void StartRuntimeStat() override;
    void StopRuntimeStat() override;
    size_t GetArrayBufferSize() override;
    size_t GetHeapTotalSize() override;
    size_t GetHeapUsedSize() override;
    void NotifyApplicationState(bool inBackground) override;
    void NotifyIdleStatusControl(std::function<void(bool)> callback) override;
    void NotifyIdleTime(int idleMicroSec) override;
    void NotifyMemoryPressure(bool inHighMemoryPressure = false) override;
    void NotifyForceExpandState(int32_t value) override;

    void AllowCrossThreadExecution() const override;
    static void PromiseRejectCallback(void* values);

    void SetMockModuleList(const std::map<std::string, std::string> &list) override;

    // debugger
    bool IsMixedDebugEnabled();
    void NotifyNativeCalling(const void *nativeAddress);

    void RegisterUncaughtExceptionHandler(UncaughtExceptionCallback callback) override;
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

    NativeChunk& GetNativeChunk();
    NativeReference* GetPromiseRejectCallBackRef()
    {
        return promiseRejectCallbackRef_;
    }
    NapiConcurrentCallback GetConcurrentCallbackFunc()
    {
        return concurrentCallbackFunc_;
    }

    NativeReference* GetCheckCallbackRef()
    {
        return checkCallbackRef_;
    }

    static bool napiProfilerEnabled;

private:
    static NativeEngine* CreateRuntimeFunc(NativeEngine* engine, void* jsEngine);

    EcmaVM* vm_ = nullptr;
    panda::LocalScope topScope_;
    NapiConcurrentCallback concurrentCallbackFunc_ { nullptr };
    NativeReference* promiseRejectCallbackRef_ { nullptr };
    NativeReference* checkCallbackRef_ { nullptr };
    std::unordered_map<NativeModule*, panda::Global<panda::JSValueRef>> loadedModules_;
    static PermissionCheckCallback permissionCheckCallback_;
    UncaughtExceptionCallback uncaughtExceptionCallback_ { nullptr };
    NapiUncaughtExceptionCallback napiUncaughtExceptionCallback_ { nullptr };
    SourceMapCallback SourceMapCallback_ { nullptr };
    inline void SetModuleName(ArkNativeObject *nativeObj, std::string moduleName);
    static bool napiProfilerParamReaded;
    static std::string tempModuleName_;
};
#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_ARK_NATIVE_ENGINE_H */
