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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_ENGINE_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_ENGINE_H

#include <functional>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#ifdef LINUX_PLATFORM
#include<atomic>
#endif

#include "callback_scope_manager/native_callback_scope_manager.h"
#include "module_manager/native_module_manager.h"
#include "native_engine/native_async_work.h"
#include "native_engine/native_deferred.h"
#include "native_engine/native_reference.h"
#include "native_engine/native_safe_async_work.h"
#include "native_engine/native_value.h"
#include "native_property.h"
#include "reference_manager/native_reference_manager.h"
#include "scope_manager/native_scope_manager.h"
#include "utils/macros.h"

typedef int32_t (*GetContainerScopeIdCallback)(void);
typedef void (*ContainerScopeCallback)(int32_t);

typedef struct uv_loop_s uv_loop_t;

struct NativeErrorExtendedInfo {
    const char* message = nullptr;
    void* reserved = nullptr;
    uint32_t engineErrorCode = 0;
    int errorCode = 0;
};

struct ExceptionInfo {
    const char* message_ = nullptr;
    int32_t lineno_ = 0;
    int32_t colno_ = 0;

    ~ExceptionInfo()
    {
        if (message_ != nullptr) {
            delete[] message_;
        }
    }
};

enum LoopMode {
    LOOP_DEFAULT, LOOP_ONCE, LOOP_NOWAIT
};

enum class DumpFormat {
    JSON, BINARY, OTHER
};

enum class WorkerVersion {
    NONE, OLD, NEW
};

class CleanupHookCallback {
public:
    using Callback = void (*)(void*);

    CleanupHookCallback(Callback fn, void* arg, uint64_t insertion_order_counter)
        : fn_(fn), arg_(arg), insertion_order_counter_(insertion_order_counter)
    {}

    struct Hash {
        inline size_t operator()(const CleanupHookCallback& cb) const
        {
            return std::hash<void*>()(cb.arg_);
        }
    };
    struct Equal {
        inline bool operator()(const CleanupHookCallback& a, const CleanupHookCallback& b) const
        {
            return a.fn_ == b.fn_ && a.arg_ == b.arg_;
        };
    };

private:
    friend class NativeEngine;
    Callback fn_;
    void* arg_;
    uint64_t insertion_order_counter_;
};

using PostTask = std::function<void(bool needSync)>;
using CleanEnv = std::function<void()>;
using InitWorkerFunc = std::function<void(NativeEngine* engine)>;
using GetAssetFunc = std::function<void(const std::string& uri, std::vector<uint8_t>& content, std::string& ami)>;
using OffWorkerFunc = std::function<void(NativeEngine* engine)>;
using DebuggerPostTask = std::function<void(std::function<void()>&&)>;
using UncaughtExceptionCallback = std::function<void(NativeValue* value)>;
using PermissionCheckCallback = std::function<bool()>;
using NapiConcurrentCallback = void (*)(NativeEngine* engine, NativeValue* result, bool success, void* data);
using SourceMapCallback = std::function<std::string(const std::string& rawStack)>;
using SourceMapTranslateCallback = std::function<bool(std::string& url, int& line, int& column)>;

class NAPI_EXPORT NativeEngine {
public:
    explicit NativeEngine(void* jsEngine);
    virtual ~NativeEngine();

    virtual NativeScopeManager* GetScopeManager();
    virtual NativeModuleManager* GetModuleManager();
    virtual NativeReferenceManager* GetReferenceManager();
    virtual NativeCallbackScopeManager* GetCallbackScopeManager();
    virtual uv_loop_t* GetUVLoop() const;
    virtual pthread_t GetTid() const;

    virtual bool ReinitUVLoop();

    virtual void Loop(LoopMode mode, bool needSync = false);
    virtual void SetPostTask(PostTask postTask);
    virtual void TriggerPostTask();
#if !defined(PREVIEW)
    virtual void CheckUVLoop();
    virtual void CancelCheckUVLoop();
#endif
    virtual void* GetJsEngine();

    virtual NativeValue* GetGlobal() = 0;

    virtual NativeValue* CreateNull() = 0;
    virtual NativeValue* CreateUndefined() = 0;
    virtual NativeValue* CreateBoolean(bool value) = 0;
    virtual NativeValue* CreateNumber(int32_t value) = 0;
    virtual NativeValue* CreateNumber(uint32_t value) = 0;
    virtual NativeValue* CreateNumber(int64_t value) = 0;
    virtual NativeValue* CreateNumber(double value) = 0;
    virtual NativeValue* CreateBigInt(int64_t value) = 0;
    virtual NativeValue* CreateBigInt(uint64_t value) = 0;
    virtual NativeValue* CreateString(const char* value, size_t length) = 0;
    virtual NativeValue* CreateString16(const char16_t* value, size_t length) = 0;

    virtual NativeValue* CreateSymbol(NativeValue* value) = 0;
    virtual NativeValue* CreateExternal(void* value, NativeFinalize callback, void* hint,
        size_t nativeBindingSize = 0) = 0;

    virtual NativeValue* CreateObject() = 0;
    virtual NativeValue* CreateFunction(const char* name, size_t length, NativeCallback cb, void* value) = 0;
    virtual NativeValue* CreateArray(size_t length) = 0;
    virtual NativeValue* CreateBuffer(void** value, size_t length) = 0;
    virtual NativeValue* CreateBufferCopy(void** value, size_t length, const void* data) = 0;
    virtual NativeValue* CreateBufferExternal(void* value, size_t length, NativeFinalize cb, void* hint) = 0;
    virtual NativeValue* CreateArrayBuffer(void** value, size_t length) = 0;
    virtual NativeValue* CreateArrayBufferExternal(void* value, size_t length, NativeFinalize cb, void* hint) = 0;

    virtual NativeValue* CreateTypedArray(NativeTypedArrayType type,
                                          NativeValue* value,
                                          size_t length,
                                          size_t offset) = 0;
    virtual NativeValue* CreateDataView(NativeValue* value, size_t length, size_t offset) = 0;
    virtual NativeValue* CreatePromise(NativeDeferred** deferred) = 0;
    virtual void SetPromiseRejectCallback(NativeReference* rejectCallbackRef, NativeReference* checkCallbackRef) = 0;
    virtual NativeValue* CreateError(NativeValue* code, NativeValue* message) = 0;

    virtual bool InitTaskPoolThread(NativeEngine* engine, NapiConcurrentCallback callback) = 0;
    virtual bool InitTaskPoolFunc(NativeEngine* engine, NativeValue* func, void* taskInfo) = 0;
    virtual bool HasPendingJob() = 0;
    virtual bool IsProfiling() = 0;
    virtual void* GetCurrentTaskInfo() const = 0;

    virtual NativeValue* CallFunction(NativeValue* thisVar,
                                      NativeValue* function,
                                      NativeValue* const *argv,
                                      size_t argc) = 0;
    virtual NativeValue* RunScript(NativeValue* script) = 0;
    virtual NativeValue* RunScriptPath(const char* path) = 0;
    virtual NativeValue* RunScriptBuffer(const char* path, std::vector<uint8_t>& buffer, bool isBundle) = 0;
    virtual bool RunScriptBuffer(const std::string &path, uint8_t* buffer, size_t size, bool isBundle) = 0;
    virtual NativeValue* RunBufferScript(std::vector<uint8_t>& buffer) = 0;
    virtual NativeValue* RunActor(std::vector<uint8_t>& buffer, const char* descriptor) = 0;
    virtual NativeValue* DefineClass(const char* name,
                                     NativeCallback callback,
                                     void* data,
                                     const NativePropertyDescriptor* properties,
                                     size_t length) = 0;

    virtual NativeValue* CreateInstance(NativeValue* constructor, NativeValue* const *argv, size_t argc) = 0;

    virtual NativeReference* CreateReference(NativeValue* value, uint32_t initialRefcount,
        NativeFinalize callback = nullptr, void* data = nullptr, void* hint = nullptr) = 0;

    virtual NativeAsyncWork* CreateAsyncWork(NativeValue* asyncResource,
                                             NativeValue* asyncResourceName,
                                             NativeAsyncExecuteCallback execute,
                                             NativeAsyncCompleteCallback complete,
                                             void* data);

    virtual NativeAsyncWork* CreateAsyncWork(const std::string &asyncResourceName,
                                             NativeAsyncExecuteCallback execute,
                                             NativeAsyncCompleteCallback complete,
                                             void* data);
    virtual NativeSafeAsyncWork* CreateSafeAsyncWork(NativeValue* func, NativeValue* asyncResource,
        NativeValue* asyncResourceName, size_t maxQueueSize, size_t threadCount, void* finalizeData,
        NativeFinalize finalizeCallback, void* context, NativeThreadSafeFunctionCallJs callJsCallback);

    virtual bool Throw(NativeValue* error) = 0;
    virtual bool Throw(NativeErrorType type, const char* code, const char* message) = 0;

    virtual void* CreateRuntime() = 0;
    virtual NativeValue* Serialize(NativeEngine* context, NativeValue* value, NativeValue* transfer) = 0;
    virtual NativeValue* Deserialize(NativeEngine* context, NativeValue* recorder) = 0;
    virtual void DeleteSerializationData(NativeValue* value) const = 0;
    virtual NativeValue* LoadModule(NativeValue* str, const std::string& fileName) = 0;

    virtual void StartCpuProfiler(const std::string& fileName = "") = 0;
    virtual void StopCpuProfiler() = 0;

    virtual void ResumeVM() = 0;
    virtual bool SuspendVM() = 0;
    virtual bool IsSuspended() = 0;
    virtual bool CheckSafepoint() = 0;
    virtual bool SuspendVMById(uint32_t tid) = 0;
    virtual void ResumeVMById(uint32_t tid) = 0;

    virtual void DumpHeapSnapshot(const std::string &path, bool isVmMode = true,
        DumpFormat dumpFormat = DumpFormat::JSON) = 0;
    virtual void DumpHeapSnapshot(bool isVmMode = true, DumpFormat dumpFormat = DumpFormat::JSON,
        bool isPrivate = false) = 0;
    virtual bool BuildNativeAndJsStackTrace(std::string &stackTraceStr) = 0;
    virtual bool BuildJsStackTrace(std::string &stackTraceStr) = 0;
    virtual bool BuildJsStackInfoList(uint32_t tid, std::vector<JsFrameInfo>& jsFrames) = 0;
    virtual bool DeleteWorker(NativeEngine* workerEngine) = 0;
    virtual bool StartHeapTracking(double timeInterval, bool isVmMode = true) = 0;
    virtual bool StopHeapTracking(const std::string &filePath) = 0;

    virtual void AllowCrossThreadExecution() const = 0;

    NativeErrorExtendedInfo* GetLastError();
    void SetLastError(int errorCode, uint32_t engineErrorCode = 0, void* engineReserved = nullptr);
    void ClearLastError();
    virtual bool IsExceptionPending() const = 0;
    virtual NativeValue* GetAndClearLastException() = 0;
    void EncodeToUtf8(NativeValue* nativeValue, char* buffer, int32_t* written, size_t bufferSize, int32_t* nchars);
    void EncodeToChinese(NativeValue* nativeValue, std::string& buffer, const std::string& encoding);
    NativeEngine(NativeEngine&) = delete;
    virtual NativeEngine& operator=(NativeEngine&) = delete;

    virtual NativeValue* ValueToNativeValue(JSValueWrapper& value) = 0;
    virtual bool TriggerFatalException(NativeValue* error) = 0;
    virtual bool AdjustExternalMemory(int64_t ChangeInBytes, int64_t* AdjustedValue) = 0;

    void MarkWorkerThread()
    {
        jsThreadType_ = JSThreadType::WORKER_THREAD;
    }
    void MarkTaskPoolThread()
    {
        jsThreadType_ = JSThreadType::TASKPOOL_THREAD;
    }
    bool IsWorkerThread() const
    {
        return jsThreadType_ == JSThreadType::WORKER_THREAD;
    }
    bool IsTaskPoolThread() const
    {
        return jsThreadType_ == JSThreadType::TASKPOOL_THREAD;
    }
    bool IsMainThread() const
    {
        return jsThreadType_ == JSThreadType::MAIN_THREAD;
    }

    bool CheckAndSetWorkerVersion(WorkerVersion expected, WorkerVersion desired)
    {
        return workerVersion_.compare_exchange_strong(expected, desired);
    }
    bool IsTargetWorkerVersion(WorkerVersion target) const
    {
        return workerVersion_.load() == target;
    }

    void IncreaseSubEnvCounter()
    {
        subEnvCounter_++;
    }
    void DecreaseSubEnvCounter()
    {
        subEnvCounter_--;
    }
    bool HasSubEnv()
    {
        return subEnvCounter_.load() != 0;
    }

    void SetCleanEnv(CleanEnv cleanEnv)
    {
        cleanEnv_ = cleanEnv;
    }

    // register init worker func
    virtual void SetInitWorkerFunc(InitWorkerFunc func);
    InitWorkerFunc GetInitWorkerFunc() const;
    virtual void SetGetAssetFunc(GetAssetFunc func);
    GetAssetFunc GetGetAssetFunc() const;
    virtual void SetOffWorkerFunc(OffWorkerFunc func);
    OffWorkerFunc GetOffWorkerFunc() const;

    // call init worker func
    virtual bool CallInitWorkerFunc(NativeEngine* engine);
    virtual bool CallGetAssetFunc(const std::string& uri, std::vector<uint8_t>& content, std::string& ami);
    virtual bool CallOffWorkerFunc(NativeEngine* engine);

    // adapt worker to ace container
    virtual void SetGetContainerScopeIdFunc(GetContainerScopeIdCallback func);
    virtual void SetInitContainerScopeFunc(ContainerScopeCallback func);
    virtual void SetFinishContainerScopeFunc(ContainerScopeCallback func);
    virtual int32_t GetContainerScopeIdFunc();
    virtual bool InitContainerScopeFunc(int32_t id);
    virtual bool FinishContainerScopeFunc(int32_t id);

#if !defined(PREVIEW)
    virtual void SetDebuggerPostTaskFunc(DebuggerPostTask func);
    virtual void CallDebuggerPostTaskFunc(std::function<void()>&& task);
#endif

    virtual void SetHostEngine(NativeEngine* engine);
    virtual NativeEngine* GetHostEngine() const;

    virtual NativeValue* CreateDate(double value) = 0;
    virtual NativeValue* CreateBigWords(int sign_bit, size_t word_count, const uint64_t* words) = 0;
    using CleanupCallback = CleanupHookCallback::Callback;
    virtual void AddCleanupHook(CleanupCallback fun, void* arg);
    virtual void RemoveCleanupHook(CleanupCallback fun, void* arg);

    void CleanupHandles();
    void IncreaseWaitingRequestCounter();
    void DecreaseWaitingRequestCounter();
    bool HasWaitingRequest();

    virtual void RunCleanup();

    bool IsStopping() const
    {
        return isStopping_.load();
    }

    void SetStopping(bool value)
    {
        isStopping_.store(value);
    }

    virtual void PrintStatisticResult() = 0;
    virtual void StartRuntimeStat() = 0;
    virtual void StopRuntimeStat() = 0;
    virtual size_t GetArrayBufferSize() = 0;
    virtual size_t GetHeapTotalSize() = 0;
    virtual size_t GetHeapUsedSize() = 0;
    virtual void NotifyApplicationState(bool inBackground) = 0;
    virtual void NotifyIdleStatusControl(std::function<void(bool)> callback) = 0;
    virtual void NotifyIdleTime(int idleMicroSec) = 0;
    virtual void NotifyMemoryPressure(bool inHighMemoryPressure = false) = 0;

    void RegisterWorkerFunction(const NativeEngine* engine);

    virtual void RegisterUncaughtExceptionHandler(UncaughtExceptionCallback callback) = 0;
    virtual void HandleUncaughtException() = 0;
    virtual bool HasPendingException()
    {
        return false;
    }
    virtual void RegisterPermissionCheck(PermissionCheckCallback callback) = 0;
    virtual bool ExecutePermissionCheck() = 0;
    virtual void RegisterTranslateBySourceMap(SourceMapCallback callback) = 0;
    virtual std::string ExecuteTranslateBySourceMap(const std::string& rawStack) = 0;
    virtual void RegisterSourceMapTranslateCallback(SourceMapTranslateCallback callback) = 0;
    // run script by path
    NativeValue* RunScript(const char* path);

    const char* GetModuleFileName();

    void SetModuleFileName(std::string &moduleName);

    void SetInstanceData(void* data, NativeFinalize finalize_cb, void* hint);
    void GetInstanceData(void** data);

    /**
     * @brief Set the Extension Infos
     * 
     * @param extensionInfos extension infos to set 
     */
    void SetExtensionInfos(std::unordered_map<std::string, int32_t>&& extensionInfos);

    /**
     * @brief Get the Extension Infos
     * 
     * @return extension infos
     */
    const std::unordered_map<std::string, int32_t>& GetExtensionInfos();

    /**
     * @brief Set the Module Blocklist
     * 
     * @param blocklist the blocklist set to native engine
     */
    void SetModuleBlocklist(std::unordered_map<int32_t, std::unordered_set<std::string>>&& blocklist);

    /**
     * @brief Set the Module Load Checker
     *
     * @param moduleCheckerDelegate the module checker delegate will intercept the module loading
     */
    void SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate>& moduleCheckerDelegate);

private:
    void StartCleanupTimer();
protected:
    void *jsEngine_;
    void* jsEngineInterface_;

    void Init();
    void Deinit();

    NativeModuleManager* moduleManager_ = nullptr;
    NativeScopeManager* scopeManager_ = nullptr;
    NativeReferenceManager* referenceManager_ = nullptr;
    NativeCallbackScopeManager* callbackScopeManager_ = nullptr;

    uv_loop_t* loop_ = nullptr;

    NativeErrorExtendedInfo lastError_;

    // register for worker
    InitWorkerFunc initWorkerFunc_ {nullptr};
    GetAssetFunc getAssetFunc_ {nullptr};
    OffWorkerFunc offWorkerFunc_ {nullptr};
#if !defined(PREVIEW)
    DebuggerPostTask debuggerPostTaskFunc_ {nullptr};
#endif
    NativeEngine* hostEngine_ {nullptr};

private:
    std::string moduleName_;
    std::mutex instanceDataLock_;
    NativeObjectInfo instanceDataInfo_;
    void FinalizerInstanceData(void);
    pthread_t tid_ = 0;
    std::unordered_map<std::string, int32_t> extensionInfos_;
    uv_sem_t uvSem_;

    // the old worker api use before api9, the new worker api start with api9
    enum JSThreadType { MAIN_THREAD, WORKER_THREAD, TASKPOOL_THREAD };
    JSThreadType jsThreadType_ = JSThreadType::MAIN_THREAD;
    // current is hostengine, can create old worker, new worker, or no workers on hostengine
    std::atomic<WorkerVersion> workerVersion_ { WorkerVersion::NONE };

#if !defined(PREVIEW)
    static void UVThreadRunner(void* nativeEngine);
    void PostLoopTask();

    bool checkUVLoop_ = false;
    uv_thread_t uvThread_;
#endif

    PostTask postTask_ = nullptr;
    CleanEnv cleanEnv_ = nullptr;
    uv_async_t uvAsync_;
    std::unordered_set<CleanupHookCallback, CleanupHookCallback::Hash, CleanupHookCallback::Equal> cleanup_hooks_;
    uint64_t cleanup_hook_counter_ = 0;
    std::atomic_int request_waiting_ { 0 };
    std::atomic_int subEnvCounter_ { 0 };
    std::atomic_bool isStopping_ { false };
    bool cleanupTimeout_ = false;
    uv_timer_t timer_;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_ENGINE_H */
