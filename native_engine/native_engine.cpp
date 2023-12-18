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

#include "native_engine/native_engine.h"

#include <uv.h>
#if !defined(WINDOWS_PLATFORM) && !defined(MAC_PLATFORM) && !defined(IOS_PLATFORM) && !defined(LINUX_PLATFORM)
#include <sys/epoll.h>
#endif
#ifdef IOS_PLATFORM
#include <sys/event.h>
#endif

#include "ecmascript/napi/include/jsnapi.h"
#include "native_engine/native_utils.h"
#include "unicode/ucnv.h"
#include "utils/log.h"

constexpr size_t NAME_BUFFER_SIZE = 64;
static constexpr size_t DESTRUCTION_TIMEOUT = 3000;

using panda::JSValueRef;
using panda::Local;
using panda::LocalScope;
using panda::ObjectRef;
using panda::StringRef;

namespace {
const char* g_errorMessages[] = {
    nullptr,
    "Invalid parameter",
    "Need object",
    "Need string",
    "Need string or symbol",
    "Need function",
    "Need number",
    "Need boolean",
    "Need array",
    "Generic failure",
    "An exception is blocking",
    "Asynchronous work cancelled",
    "Escape called twice",
    "Handle scope mismatch",
    "Callback scope mismatch",
    "Asynchronous work queue is full",
    "Asynchronous work handle is closing",
    "Need bigint",
    "Need date",
    "Need arraybuffer",
    "Need detachable arraybuffer",
};
} // namespace

static GetContainerScopeIdCallback getContainerScopeIdFunc_;
static ContainerScopeCallback initContainerScopeFunc_;
static ContainerScopeCallback finishContainerScopeFunc_;

NativeEngine::NativeEngine(void* jsEngine) : jsEngine_(jsEngine) {}

NativeEngine::~NativeEngine()
{
    HILOG_INFO("NativeEngine::~NativeEngine");
    if (cleanEnv_ != nullptr) {
        cleanEnv_();
    }
    std::lock_guard<std::mutex> insLock(instanceDataLock_);
    FinalizerInstanceData();
}

void NativeEngine::Init()
{
    HILOG_INFO("NativeEngine::Init");
    moduleManager_ = NativeModuleManager::GetInstance();
    referenceManager_ = new NativeReferenceManager();
    callbackScopeManager_ = new NativeCallbackScopeManager();
    loop_ = uv_loop_new();
    if (loop_ == nullptr) {
        return;
    }
    tid_ = pthread_self();
    uv_async_init(loop_, &uvAsync_, nullptr);
    uv_sem_init(&uvSem_, 0);
}

void NativeEngine::Deinit()
{
    HILOG_INFO("NativeEngine::Deinit");
    uv_sem_destroy(&uvSem_);
    uv_close((uv_handle_t*)&uvAsync_, nullptr);
    RunCleanup();
    if (referenceManager_ != nullptr) {
        delete referenceManager_;
        referenceManager_ = nullptr;
    }

    SetStopping(true);
    uv_loop_delete(loop_);
    loop_ = nullptr;
}

NativeReferenceManager* NativeEngine::GetReferenceManager()
{
    return referenceManager_;
}

NativeModuleManager* NativeEngine::GetModuleManager()
{
    return moduleManager_;
}

NativeCallbackScopeManager* NativeEngine::GetCallbackScopeManager()
{
    return callbackScopeManager_;
}

uv_loop_t* NativeEngine::GetUVLoop() const
{
    return loop_;
}

pthread_t NativeEngine::GetTid() const
{
    return tid_;
}

bool NativeEngine::ReinitUVLoop()
{
    if (loop_ != nullptr) {
        uv_sem_destroy(&uvSem_);
        uv_close((uv_handle_t*)&uvAsync_, nullptr);
        uv_run(loop_, UV_RUN_ONCE);
        uv_loop_delete(loop_);
    }

    loop_ = uv_loop_new();
    if (loop_ == nullptr) {
        return false;
    }
    tid_ = pthread_self();
    uv_async_init(loop_, &uvAsync_, nullptr);
    uv_sem_init(&uvSem_, 0);
    return true;
}

void NativeEngine::Loop(LoopMode mode, bool needSync)
{
    bool more = true;
    switch (mode) {
        case LoopMode::LOOP_DEFAULT:
            more = uv_run(loop_, UV_RUN_DEFAULT);
            break;
        case LoopMode::LOOP_ONCE:
            more = uv_run(loop_, UV_RUN_ONCE);
            break;
        case LoopMode::LOOP_NOWAIT:
            more = uv_run(loop_, UV_RUN_NOWAIT);
            break;
        default:
            return;
    }
    if (more == false) {
        uv_loop_alive(loop_);
    }

    if (needSync) {
        uv_sem_post(&uvSem_);
    }
}

NativeAsyncWork* NativeEngine::CreateAsyncWork(napi_value asyncResource, napi_value asyncResourceName,
    NativeAsyncExecuteCallback execute, NativeAsyncCompleteCallback complete, void* data)
{
    (void)asyncResource;
    (void)asyncResourceName;
    char name[NAME_BUFFER_SIZE] = {0};
    if (asyncResourceName != nullptr) {
        auto val = LocalValueFromJsValue(asyncResourceName);
        size_t strLength = 0;
        auto vm = GetEcmaVm();
        LocalScope scope(vm);
        auto str = val->ToString(vm);
        char* buffer = name;
        if (buffer == nullptr) {
            strLength = static_cast<size_t>(str->Utf8Length(vm) - 1);
        } else if (NAME_BUFFER_SIZE != 0) {
            int copied = str->WriteUtf8(buffer, NAME_BUFFER_SIZE - 1, true) - 1;
            buffer[copied] = '\0';
            strLength = static_cast<size_t>(copied);
        } else {
            strLength = 0;
        }
    }
    return new NativeAsyncWork(this, execute, complete, name, data);
}

NativeAsyncWork* NativeEngine::CreateAsyncWork(const std::string& asyncResourceName, NativeAsyncExecuteCallback execute,
    NativeAsyncCompleteCallback complete, void* data)
{
    return new NativeAsyncWork(this, execute, complete, asyncResourceName, data);
}

NativeSafeAsyncWork* NativeEngine::CreateSafeAsyncWork(napi_value func, napi_value asyncResource,
    napi_value asyncResourceName, size_t maxQueueSize, size_t threadCount, void* finalizeData,
    NativeFinalize finalizeCallback, void* context, NativeThreadSafeFunctionCallJs callJsCallback)
{
    return new NativeSafeAsyncWork(this, func, asyncResource, asyncResourceName, maxQueueSize, threadCount,
        finalizeData, finalizeCallback, context, callJsCallback);
}

NativeErrorExtendedInfo* NativeEngine::GetLastError()
{
    return &lastError_;
}

void NativeEngine::SetLastError(int errorCode, uint32_t engineErrorCode, void* engineReserved)
{
    lastError_.errorCode = errorCode;
    lastError_.engineErrorCode = engineErrorCode;
    lastError_.message = g_errorMessages[lastError_.errorCode];
    lastError_.reserved = engineReserved;
}

void SubEncodeToUtf8(const EcmaVM* vm,
                     Local<JSValueRef>& nativeValue,
                     Local<StringRef>& nativeString,
                     char* buffer,
                     int32_t* written,
                     size_t bufferSize,
                     int32_t* nchars)
{
    int32_t length = static_cast<int32_t>(nativeString->Length());
    int32_t pos = 0;
    int32_t writableSize = static_cast<int32_t>(bufferSize);
    int32_t i = 0;
    panda::Local<ObjectRef> strObj = nativeValue->ToObject(vm);
    for (; i < length; i++) {
        panda::Local<StringRef> str = strObj->Get(vm, i)->ToString(vm);
        int32_t len = str->Utf8Length(vm) - 1;
        if (len > writableSize) {
            break;
        }
        str->WriteUtf8((buffer + pos), writableSize);
        writableSize -= len;
        pos += len;
    }
    *nchars = i;
    HILOG_DEBUG("EncodeWriteUtf8 the result of buffer: %{public}s", buffer);
    *written = pos;
}

void NativeEngine::EncodeToUtf8(napi_value value, char* buffer, int32_t* written, size_t bufferSize, int32_t* nchars)
{
    auto nativeValue = LocalValueFromJsValue(value);
    if (nativeValue->IsNull() || nchars == nullptr || written == nullptr) {
        HILOG_ERROR("NativeEngine EncodeToUtf8 args is nullptr");
        return;
    }

    auto vm = GetEcmaVm();
    LocalScope scope(vm);
    auto nativeString = nativeValue->ToString(vm);
    if (!nativeString->IsString()) {
        HILOG_ERROR("nativeValue not is string");
        return;
    }

    if (buffer == nullptr) {
        HILOG_ERROR("buffer is null");
    }

    SubEncodeToUtf8(vm, nativeValue, nativeString, buffer, written, bufferSize, nchars);
}

void SubEncodeToChinese(const EcmaVM* vm,
                        Local<JSValueRef>& nativeValue,
                        Local<StringRef>& nativeString,
                        std::string& buffer,
                        const char* encode)
{
    int32_t length = static_cast<int32_t>(nativeString->Length());
    int32_t pos = 0;
    const int32_t writableSize = 22; // 22 : encode max bytes of the ucnv_convent function;
    std::string tempBuf = "";
    tempBuf.resize(writableSize + 1);
    UErrorCode ErrorCode = U_ZERO_ERROR;
    const char* encFrom = "utf8";
    panda::Local<ObjectRef> strObj = nativeValue->ToObject(vm);
    for (int32_t i = 0; i < length; i++) {
        panda::Local<StringRef> str = strObj->Get(vm, i)->ToString(vm);
        int32_t len = str->Utf8Length(vm) - 1;
        if ((pos + len) >= writableSize) {
            char outBuf[writableSize] = {0};
            ucnv_convert(encode, encFrom, outBuf, writableSize, tempBuf.c_str(), pos, &ErrorCode);
            if (ErrorCode != U_ZERO_ERROR) {
                HILOG_ERROR("ucnv_convert is failed : ErrorCode = %{public}d", static_cast<int32_t>(ErrorCode));
                return;
            }
            buffer += outBuf;
            tempBuf.clear();
            pos = 0;
        }
        str->WriteUtf8((tempBuf.data() + pos), pos + len + 1);
        pos += len;
    }
    if (pos > 0) {
        char outBuf[writableSize] = {0};
        ucnv_convert(encode, encFrom, outBuf, writableSize, tempBuf.c_str(), pos, &ErrorCode);
        if (ErrorCode != U_ZERO_ERROR) {
            HILOG_ERROR("ucnv_convert is failed : ErrorCode = %{public}d", static_cast<int32_t>(ErrorCode));
            return;
        }
        buffer += outBuf;
    }
}

void NativeEngine::EncodeToChinese(napi_value value, std::string& buffer, const std::string& encoding)
{
    if (value == nullptr) {
        HILOG_ERROR("nativeValue GetInterface is nullptr");
        return;
    }

    auto nativeValue = LocalValueFromJsValue(value);
    auto vm = GetEcmaVm();
    LocalScope scope(vm);
    auto nativeString = nativeValue->ToString(vm);
    if (!nativeString->IsString()) {
        HILOG_ERROR("nativeValue not is string");
        return;
    }

    auto encode = encoding.c_str();
    if (encode == nullptr) {
        HILOG_ERROR("encoding is nullptr");
        return;
    }

    SubEncodeToChinese(vm, nativeValue, nativeString, buffer, encode);
}

#if !defined(PREVIEW)
void NativeEngine::CheckUVLoop()
{
    checkUVLoop_ = true;
    uv_thread_create(&uvThread_, NativeEngine::UVThreadRunner, this);
}

void NativeEngine::UVThreadRunner(void* nativeEngine)
{
    std::string name("UVLoop");
#ifdef IOS_PLATFORM
    pthread_setname_np(name.c_str());
#else
    pthread_setname_np(pthread_self(), name.c_str());
#endif
    auto engine = static_cast<NativeEngine*>(nativeEngine);
    engine->PostLoopTask();
    while (engine->checkUVLoop_) {
        int32_t fd = uv_backend_fd(engine->loop_);
        int32_t timeout = uv_backend_timeout(engine->loop_);
        int32_t result = -1;
#ifdef IOS_PLATFORM
        struct kevent events[1];
        struct timespec spec;
        static const int32_t mSec = 1000;
        static const int32_t uSec = 1000000;
        if (timeout != -1) {
            spec.tv_sec = timeout / mSec;
            spec.tv_nsec = (timeout % mSec) * uSec;
        }
        result = kevent(fd, NULL, 0, events, 1, timeout == -1 ? NULL : &spec);

#else
        struct epoll_event ev;
        result = epoll_wait(fd, &ev, 1, timeout);
#endif

        if (!engine->checkUVLoop_) {
            HILOG_INFO("break thread after epoll wait");
            break;
        }
        if (result >= 0) {
            engine->PostLoopTask();
        } else {
            HILOG_ERROR("epoll wait fail: result: %{public}d, errno: %{public}d", result, errno);
        }
        if (!engine->checkUVLoop_) {
            HILOG_INFO("break thread after post loop task");
            break;
        }
    }
}

void NativeEngine::CancelCheckUVLoop()
{
    checkUVLoop_ = false;
    uv_async_send(&uvAsync_);
    uv_sem_post(&uvSem_);
    uv_thread_join(&uvThread_);
}

void NativeEngine::PostLoopTask()
{
    postTask_(true);
    uv_sem_wait(&uvSem_);
}
#endif

void NativeEngine::SetPostTask(PostTask postTask)
{
    postTask_ = postTask;
}

void NativeEngine::TriggerPostTask()
{
    if (postTask_ == nullptr) {
        HILOG_ERROR("postTask_ is nullptr");
        return;
    }
    postTask_(false);
}

void* NativeEngine::GetJsEngine()
{
    return jsEngine_;
}

// register init worker func
void NativeEngine::SetInitWorkerFunc(InitWorkerFunc func)
{
    initWorkerFunc_ = func;
}
InitWorkerFunc NativeEngine::GetInitWorkerFunc() const
{
    return initWorkerFunc_;
}
void NativeEngine::SetGetAssetFunc(GetAssetFunc func)
{
    getAssetFunc_ = func;
}
GetAssetFunc NativeEngine::GetGetAssetFunc() const
{
    return getAssetFunc_;
}
void NativeEngine::SetOffWorkerFunc(OffWorkerFunc func)
{
    offWorkerFunc_ = func;
}
OffWorkerFunc NativeEngine::GetOffWorkerFunc() const
{
    return offWorkerFunc_;
}

// call init worker func
bool NativeEngine::CallInitWorkerFunc(NativeEngine* engine)
{
    if (initWorkerFunc_ != nullptr) {
        initWorkerFunc_(engine);
        return true;
    }
    return false;
}
bool NativeEngine::CallGetAssetFunc(const std::string& uri, std::vector<uint8_t>& content, std::string& ami)
{
    if (getAssetFunc_ != nullptr) {
        getAssetFunc_(uri, content, ami);
        return true;
    }
    return false;
}
bool NativeEngine::CallOffWorkerFunc(NativeEngine* engine)
{
    if (offWorkerFunc_ != nullptr) {
        offWorkerFunc_(engine);
        return true;
    }
    return false;
}

// adapt worker to ace container
void NativeEngine::SetGetContainerScopeIdFunc(GetContainerScopeIdCallback func)
{
    getContainerScopeIdFunc_ = func;
}
void NativeEngine::SetInitContainerScopeFunc(ContainerScopeCallback func)
{
    initContainerScopeFunc_ = func;
}
void NativeEngine::SetFinishContainerScopeFunc(ContainerScopeCallback func)
{
    finishContainerScopeFunc_ = func;
}
int32_t NativeEngine::GetContainerScopeIdFunc()
{
    int32_t scopeId = -1;
    if (getContainerScopeIdFunc_ != nullptr) {
        scopeId = getContainerScopeIdFunc_();
    }
    return scopeId;
}
bool NativeEngine::InitContainerScopeFunc(int32_t id)
{
    if (initContainerScopeFunc_ != nullptr) {
        initContainerScopeFunc_(id);
        return true;
    }
    return false;
}
bool NativeEngine::FinishContainerScopeFunc(int32_t id)
{
    if (finishContainerScopeFunc_ != nullptr) {
        finishContainerScopeFunc_(id);
        return true;
    }
    return false;
}

#if !defined(PREVIEW)
void NativeEngine::CallDebuggerPostTaskFunc(std::function<void()>&& task)
{
    if (debuggerPostTaskFunc_ != nullptr) {
        debuggerPostTaskFunc_(std::move(task));
    }
}

void NativeEngine::SetDebuggerPostTaskFunc(DebuggerPostTask func)
{
    debuggerPostTaskFunc_ = func;
}
#endif

void NativeEngine::SetHostEngine(NativeEngine* engine)
{
    hostEngine_ = engine;
}

NativeEngine* NativeEngine::GetHostEngine() const
{
    return hostEngine_;
}

void NativeEngine::AddCleanupHook(CleanupCallback fun, void* arg)
{
    HILOG_INFO("%{public}s, start.", __func__);
    auto insertion_info = cleanup_hooks_.emplace(CleanupHookCallback { fun, arg, cleanup_hook_counter_++ });
    if (insertion_info.second != true) {
        HILOG_ERROR("AddCleanupHook Failed.");
    }
    HILOG_INFO("%{public}s, end.", __func__);
}

void NativeEngine::RemoveCleanupHook(CleanupCallback fun, void* arg)
{
    HILOG_INFO("%{public}s, start.", __func__);
    CleanupHookCallback hook { fun, arg, 0 };
    cleanup_hooks_.erase(hook);
    HILOG_INFO("%{public}s, end.", __func__);
}

void NativeEngine::StartCleanupTimer()
{
    uv_timer_init(loop_, &timer_);
    timer_.data = this;
    uv_timer_start(&timer_, [](uv_timer_t* handle) {
        HILOG_DEBUG("NativeEngine:: timer is end with timeout.");
        reinterpret_cast<NativeEngine*>(handle->data)->cleanupTimeout_ = true;
    }, DESTRUCTION_TIMEOUT, 0);
    uv_unref(reinterpret_cast<uv_handle_t*>(&timer_));
}

void NativeEngine::RunCleanup()
{
    HILOG_DEBUG("%{public}s, start.", __func__);
    StartCleanupTimer();
    CleanupHandles();
    // sync clean up
    while (!cleanup_hooks_.empty()) {
        HILOG_DEBUG("NativeEngine::RunCleanup cleanup_hooks is not empty");
        // Copy into a vector, since we can't sort an unordered_set in-place.
        std::vector<CleanupHookCallback> callbacks(cleanup_hooks_.begin(), cleanup_hooks_.end());
        // We can't erase the copied elements from `cleanup_hooks_` yet, because we
        // need to be able to check whether they were un-scheduled by another hook.

        std::sort(callbacks.begin(), callbacks.end(), [](const CleanupHookCallback& a, const CleanupHookCallback& b) {
            // Sort in descending order so that the most recently inserted callbacks are run first.
            return a.insertion_order_counter_ > b.insertion_order_counter_;
        });
        HILOG_DEBUG(
            "NativeEngine::RunCleanup cleanup_hooks callbacks size:%{public}d", (int32_t)callbacks.size());
        for (const CleanupHookCallback& cb : callbacks) {
            if (cleanup_hooks_.count(cb) == 0) {
                // This hook was removed from the `cleanup_hooks_` set during another
                // hook that was run earlier. Nothing to do here.
                continue;
            }
            cb.fn_(cb.arg_);
            cleanup_hooks_.erase(cb);
        }
        CleanupHandles();
    }

    while (uv_run(loop_, UV_RUN_NOWAIT) != 0 && !cleanupTimeout_) {}
    uv_timer_stop(&timer_);
    uv_close(reinterpret_cast<uv_handle_t*>(&timer_), nullptr);
    uv_run(loop_, UV_RUN_ONCE);

    if (cleanupTimeout_) {
        HILOG_ERROR("RunCleanup timeout");
    }
    HILOG_DEBUG("%{public}s, end.", __func__);
}

void NativeEngine::CleanupHandles()
{
    while (requestWaiting_.load() > 0 && !cleanupTimeout_) {
        HILOG_INFO("%{public}s, request waiting:%{public}d.", __func__,
            requestWaiting_.load(std::memory_order_relaxed));
        uv_run(loop_, UV_RUN_ONCE);
    }
}

void NativeEngine::IncreaseWaitingRequestCounter()
{
    requestWaiting_++;
}

void NativeEngine::DecreaseWaitingRequestCounter()
{
    requestWaiting_--;
}

bool NativeEngine::HasWaitingRequest()
{
    return requestWaiting_.load() != 0;
}

void NativeEngine::IncreaseListeningCounter()
{
    listeningCounter_++;
}

void NativeEngine::DecreaseListeningCounter()
{
    listeningCounter_--;
}

bool NativeEngine::HasListeningCounter()
{
    return listeningCounter_.load() != 0;
}

void NativeEngine::RegisterWorkerFunction(const NativeEngine* engine)
{
    if (engine == nullptr) {
        return;
    }
    SetInitWorkerFunc(engine->GetInitWorkerFunc());
    SetGetAssetFunc(engine->GetGetAssetFunc());
    SetOffWorkerFunc(engine->GetOffWorkerFunc());
}

napi_value NativeEngine::RunScript(const char* path, char* entryPoint)
{
    std::vector<uint8_t> scriptContent;
    std::string pathStr(path);
    std::string ami;
    if (!CallGetAssetFunc(pathStr, scriptContent, ami)) {
        HILOG_ERROR("Get asset error");
        return nullptr;
    }
    HILOG_INFO("asset size is %{public}zu", scriptContent.size());
    return RunActor(scriptContent, ami.c_str(), entryPoint);
}

void NativeEngine::SetInstanceData(void* data, NativeFinalize finalize_cb, void* hint)
{
    HILOG_INFO("NativeEngineWraper::%{public}s, start.", __func__);
    std::lock_guard<std::mutex> insLock(instanceDataLock_);
    FinalizerInstanceData();
    instanceDataInfo_.engine = this;
    instanceDataInfo_.callback = finalize_cb;
    instanceDataInfo_.nativeObject = data;
    instanceDataInfo_.hint = hint;
}

void NativeEngine::GetInstanceData(void** data)
{
    HILOG_INFO("NativeEngineWraper::%{public}s, start.", __func__);
    std::lock_guard<std::mutex> insLock(instanceDataLock_);
    if (data) {
        *data = instanceDataInfo_.nativeObject;
    }
}

void NativeEngine::FinalizerInstanceData(void)
{
    if (instanceDataInfo_.engine != nullptr && instanceDataInfo_.callback != nullptr) {
        instanceDataInfo_.callback(instanceDataInfo_.engine, instanceDataInfo_.nativeObject, instanceDataInfo_.hint);
    }
    instanceDataInfo_.engine = nullptr;
    instanceDataInfo_.callback = nullptr;
    instanceDataInfo_.nativeObject = nullptr;
    instanceDataInfo_.hint = nullptr;
}

const char* NativeEngine::GetModuleFileName()
{
    HILOG_INFO("%{public}s, start.", __func__);
    if (moduleFileName_.empty()) {
        NativeModuleManager* moduleManager = GetModuleManager();
        HILOG_INFO("NativeEngineWraper::GetFileName GetModuleManager");
        if (moduleManager != nullptr) {
            std::string moduleFileName = moduleManager->GetModuleFileName(moduleName_.c_str(), isAppModule_);
            HILOG_INFO("NativeEngineWraper::GetFileName end filename:%{public}s", moduleFileName.c_str());
            SetModuleFileName(moduleFileName);
        }
    }
    return moduleFileName_.c_str();
}

void NativeEngine::SetModuleName(std::string& moduleName)
{
    moduleName_ = moduleName;
}

void NativeEngine::SetModuleFileName(std::string& moduleFileName)
{
    moduleFileName_ = moduleFileName;
}

void NativeEngine::SetExtensionInfos(std::unordered_map<std::string, int32_t>&& extensionInfos)
{
    extensionInfos_ = extensionInfos;
}

const std::unordered_map<std::string, int32_t>& NativeEngine::GetExtensionInfos()
{
    return extensionInfos_;
}

void NativeEngine::SetModuleLoadChecker(const std::shared_ptr<ModuleCheckerDelegate>& moduleCheckerDelegate)
{
    NativeModuleManager* moduleManager = GetModuleManager();
    if (!moduleManager) {
        HILOG_ERROR("SetModuleLoadChecker failed, moduleManager is nullptr");
        return;
    }
    moduleManager->SetModuleLoadChecker(moduleCheckerDelegate);
}