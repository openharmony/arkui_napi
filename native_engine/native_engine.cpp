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

#if !defined(WINDOWS_PLATFORM) && !defined(MAC_PLATFORM) && !defined(IOS_PLATFORM) && !defined(LINUX_PLATFORM)
#include <sys/epoll.h>
#endif
#ifdef IOS_PLATFORM
#include <sys/event.h>
#endif

#include "native_engine/native_utils.h"
#include "unicode/ucnv.h"

constexpr size_t NAME_BUFFER_SIZE = 64;
static constexpr auto PANDA_MAIN_FUNCTION = "_GLOBAL::func_main_0";
static constexpr int32_t API11 = 11;
static constexpr int32_t API_VERSION_MASK = 1000;

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

thread_local static GetContainerScopeIdCallback g_getContainerScopeIdFunc;
thread_local static ContainerScopeCallback g_initContainerScopeFunc;
thread_local static ContainerScopeCallback g_finishContainerScopeFunc;

std::mutex NativeEngine::g_alivedEngineMutex_;
std::unordered_set<NativeEngine*> NativeEngine::g_alivedEngine_;
uint64_t NativeEngine::g_lastEngineId_ = 1;
std::mutex NativeEngine::g_mainThreadEngineMutex_;
NativeEngine* NativeEngine::g_mainThreadEngine_;
NapiErrorManager* NapiErrorManager::instance_ = NULL;
static std::mutex g_errorManagerInstanceMutex;

NativeEngine::NativeEngine(void* jsEngine) : jsEngine_(jsEngine)
{
    SetMainThreadEngine(this);
    SetAlived();
    InitUvField();
    workerThreadState_ = new WorkerThreadState();
}

void NativeEngine::InitUvField()
{
    if (memset_s(&uvAsync_, sizeof(uvAsync_), 0, sizeof(uvAsync_)) != EOK) {
        HILOG_ERROR("failed to init uvAsync_");
        return;
    }
    if (memset_s(&uvSem_, sizeof(uvSem_), 0, sizeof(uvSem_)) != EOK) {
        HILOG_ERROR("failed to init uvSem_");
        return;
    }
#if !defined(PREVIEW)
    if (memset_s(&uvThread_, sizeof(uvThread_), 0, sizeof(uvThread_)) != EOK) {
        HILOG_ERROR("failed to init uvThread_");
        return;
    }
#endif
}

NativeEngine::~NativeEngine()
{
    HILOG_INFO("NativeEngine::~NativeEngine");
    isInDestructor_ = true;
    if (cleanEnv_ != nullptr) {
        cleanEnv_();
    }
    if (workerThreadState_ != nullptr) {
        delete workerThreadState_;
    }
    std::lock_guard<std::mutex> insLock(instanceDataLock_);
    FinalizerInstanceData();
}

void NativeEngine::Init()
{
    HILOG_DEBUG("NativeEngine::Init");
    moduleManager_ = NativeModuleManager::GetInstance();
    referenceManager_ = new NativeReferenceManager();
    callbackScopeManager_ = new NativeCallbackScopeManager();
    tid_ = pthread_self();
    sysTid_ = GetCurSysTid();

    loop_ = new (std::nothrow)uv_loop_t;
    if (loop_ == nullptr) {
        HILOG_ERROR("failed to create uv_loop, async task interface would not work");
        return;
    }
    if (uv_loop_init(loop_) != EOK) {
        HILOG_ERROR("failed to init uv_loop, async task interface would not work");
        delete loop_;
        loop_ = nullptr;
        return;
    }
    uv_async_init(loop_, &uvAsync_, nullptr);
    uv_sem_init(&uvSem_, 0);
    NativeEvent::CreateDefaultFunction(this, defaultFunc_, eventMutex_);
}

void NativeEngine::Deinit()
{
    HILOG_INFO("NativeEngine::Deinit");
    if (loop_ != nullptr) {
        NativeEvent::DestoryDefaultFunction(true, defaultFunc_, eventMutex_);
        uv_sem_destroy(&uvSem_);
        uv_close((uv_handle_t*)&uvAsync_, nullptr);
    }

    RunCleanup();
    if (referenceManager_ != nullptr) {
        delete referenceManager_;
        referenceManager_ = nullptr;
    }

    SetUnalived();
    SetStopping(true);
    if (loop_ == nullptr) {
        return;
    }
    if (uv_loop_close(loop_) != EOK) {
        HILOG_WARN("faild to close uv_loop, rerun it.");
        // execute works posted from finalize_cb
        uv_run(loop_, UV_RUN_DEFAULT);
        if (uv_loop_close(loop_) != EOK) {
            // it maybe up to fatal level later
            HILOG_ERROR("faild to close uv_loop, after reran.");
        }
    };
    delete loop_;
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

ThreadId NativeEngine::GetCurSysTid()
{
    return reinterpret_cast<ThreadId>(panda::JSNApi::GetCurrentThreadId());
}

// should only be called if process is forked, other case would cause fd_leak
bool NativeEngine::ReinitUVLoop()
{
    if (defaultFunc_ != nullptr) {
        NativeEvent::DestoryDefaultFunction(false, defaultFunc_, eventMutex_);
    }

    if (loop_ != nullptr) {
        delete loop_;  // only free mem due to uv_loop is invalid
        loop_ = nullptr;
    }

    tid_ = pthread_self();
    sysTid_ = GetCurSysTid();

    loop_ = new (std::nothrow)uv_loop_t;
    if (loop_ == nullptr) {
        HILOG_ERROR("failed to create uv_loop, async task interface would not work");
        return false;
    }
    if (uv_loop_init(loop_) != EOK) {
        HILOG_ERROR("failed to init uv_loop, async task interface would not work");
        delete loop_;
        loop_ = nullptr;
        return false;
    }

    uv_async_init(loop_, &uvAsync_, nullptr);
    uv_sem_init(&uvSem_, 0);
    NativeEvent::CreateDefaultFunction(this, defaultFunc_, eventMutex_);

    return true;
}

void NativeEngine::Loop(LoopMode mode, bool needSync)
{
    if (loop_ == nullptr) {
        HILOG_ERROR("uv loop is nullptr");
        return;
    }
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

// should only call once in life cycle of ArkNativeEngine(NativeEngine)
void NativeEngine::SetAlived()
{
    if (id_ != 0) {
        HILOG_FATAL("id of native engine cannot set twice");
    }
    std::lock_guard<std::mutex> alivedEngLock(g_alivedEngineMutex_);
    g_alivedEngine_.emplace(this);
    // must be protected by g_alivedEngineMutex_
    id_ = g_lastEngineId_++;
    return;
}

NativeAsyncWork* NativeEngine::CreateAsyncWork(napi_value asyncResource, napi_value asyncResourceName,
    NativeAsyncExecuteCallback execute, NativeAsyncCompleteCallback complete, void* data)
{
    (void)asyncResource;
    (void)asyncResourceName;
    char name[NAME_BUFFER_SIZE] = {0};
    if (asyncResourceName != nullptr) {
        auto val = LocalValueFromJsValue(asyncResourceName);
        [[maybe_unused]] size_t strLength = 0;
        auto vm = GetEcmaVm();
        LocalScope scope(vm);
        auto str = val->ToString(vm);
        char* buffer = name;
        if (buffer == nullptr) {
            strLength = static_cast<size_t>(str->Utf8Length(vm, true) - 1);
        } else if (NAME_BUFFER_SIZE != 0) {
            uint32_t copied = str->WriteUtf8(vm, buffer, NAME_BUFFER_SIZE - 1, true) - 1;
            buffer[copied] = '\0';
            strLength = copied;
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

static void SubEncodeToUtf8(const EcmaVM* vm,
                            Local<JSValueRef>& nativeValue,
                            Local<StringRef>& nativeString,
                            char* buffer,
                            uint32_t* written,
                            size_t bufferSize,
                            int32_t* nchars)
{
    int32_t length = static_cast<int32_t>(nativeString->Length(vm));
    uint32_t pos = 0;
    uint32_t writableSize = bufferSize;
    int32_t i = 0;
    panda::Local<ObjectRef> strObj = nativeValue->ToObject(vm);
    for (; i < length; i++) {
        panda::Local<StringRef> str = strObj->Get(vm, i)->ToString(vm);
        // return value of Uft8Length >= 1
        uint32_t len = str->Utf8Length(vm) - 1;
        if (len > writableSize) {
            break;
        }
        str->WriteUtf8(vm, (buffer + pos), writableSize);
        writableSize -= len;
        pos += len;
    }
    *nchars = i;
    HILOG_DEBUG("EncodeWriteUtf8 the result of buffer: %{public}s", buffer);
    *written = pos;
}

void NativeEngine::EncodeToUtf8(napi_value value, char* buffer, uint32_t* written, size_t bufferSize, int32_t* nchars)
{
    auto nativeValue = LocalValueFromJsValue(value);
    if (nativeValue->IsNull() || nchars == nullptr || written == nullptr) {
        HILOG_ERROR("NativeEngine EncodeToUtf8 args is nullptr");
        return;
    }

    auto vm = GetEcmaVm();
    LocalScope scope(vm);
    auto nativeString = nativeValue->ToString(vm);
    if (!nativeString->IsString(vm)) {
        HILOG_ERROR("nativeValue not is string");
        return;
    }

    if (buffer == nullptr) {
        HILOG_ERROR("buffer is null");
        return;
    }

    SubEncodeToUtf8(vm, nativeValue, nativeString, buffer, written, bufferSize, nchars);
}

static void SubEncodeToChinese(const EcmaVM* vm,
                               Local<JSValueRef>& nativeValue,
                               Local<StringRef>& nativeString,
                               std::string& buffer,
                               const char* encode)
{
    uint32_t length = nativeString->Length(vm);
    uint32_t pos = 0;
    const int32_t writableSize = 22; // 22 : encode max bytes of the ucnv_convent function;
    std::string tempBuf = "";
    tempBuf.resize(writableSize + 1);
    UErrorCode errorCode = U_ZERO_ERROR;
    const char* encFrom = "utf8";
    panda::Local<ObjectRef> strObj = nativeValue->ToObject(vm);
    for (uint32_t i = 0; i < length; i++) {
        panda::Local<StringRef> str = strObj->Get(vm, i)->ToString(vm);
        // return value of Utf8Length >= 1
        uint32_t len = str->Utf8Length(vm) - 1;
        if ((pos + len) >= writableSize) {
            char outBuf[writableSize] = {0};
            ucnv_convert(encode, encFrom, outBuf, writableSize, tempBuf.c_str(), pos, &errorCode);
            if (errorCode != U_ZERO_ERROR) {
                HILOG_ERROR("ucnv_convert is failed : ErrorCode = %{public}d", static_cast<int32_t>(errorCode));
                return;
            }
            buffer += outBuf;
            tempBuf.clear();
            pos = 0;
        }
        str->WriteUtf8(vm, (tempBuf.data() + pos), pos + len + 1);
        pos += len;
    }
    if (pos > 0) {
        char outBuf[writableSize] = {0};
        ucnv_convert(encode, encFrom, outBuf, writableSize, tempBuf.c_str(), pos, &errorCode);
        if (errorCode != U_ZERO_ERROR) {
            HILOG_ERROR("ucnv_convert is failed : ErrorCode = %{public}d", static_cast<int32_t>(errorCode));
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
    if (!nativeString->IsString(vm)) {
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

bool NativeEngine::CallGetAssetFunc(const std::string& uri, uint8_t** buffer, size_t* bufferSize,
    std::vector<uint8_t>& content, std::string& ami, bool &useSecureMem, bool isRestrictedWorker)
{
    if (getAssetFunc_ != nullptr) {
        getAssetFunc_(uri, buffer, bufferSize, content, ami, useSecureMem, isRestrictedWorker);
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
    g_getContainerScopeIdFunc = func;
}
void NativeEngine::SetInitContainerScopeFunc(ContainerScopeCallback func)
{
    g_initContainerScopeFunc = func;
}
void NativeEngine::SetFinishContainerScopeFunc(ContainerScopeCallback func)
{
    g_finishContainerScopeFunc = func;
}
int32_t NativeEngine::GetContainerScopeIdFunc()
{
    int32_t scopeId = -1;
    if (g_getContainerScopeIdFunc != nullptr) {
        scopeId = g_getContainerScopeIdFunc();
    }
    return scopeId;
}
bool NativeEngine::InitContainerScopeFunc(int32_t id)
{
    if (g_initContainerScopeFunc != nullptr) {
        g_initContainerScopeFunc(id);
        return true;
    }
    return false;
}
bool NativeEngine::FinishContainerScopeFunc(int32_t id)
{
    if (g_finishContainerScopeFunc != nullptr) {
        g_finishContainerScopeFunc(id);
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

void NativeEngine::SetApiVersion(int32_t apiVersion)
{
    apiVersion_ = apiVersion;
    realApiVersion_ = apiVersion % API_VERSION_MASK;
    EcmaVM* vm = const_cast<EcmaVM*>(GetEcmaVm());
    panda::JSNApi::SetVMAPIVersion(vm, apiVersion);
}

int32_t NativeEngine::GetApiVersion()
{
    return apiVersion_;
}

int32_t NativeEngine::GetRealApiVersion()
{
    return realApiVersion_;
}

bool NativeEngine::IsApplicationApiVersionAPI11Plus()
{
    return apiVersion_ > API11;
}

napi_status NativeEngine::AddCleanupHook(CleanupCallback fun, void* arg)
{
    auto insertion_info = cleanupHooks_.emplace(arg,
        std::pair<CleanupCallback, uint64_t>(fun, cleanupHookCounter_++));
    if (insertion_info.second) {
        return napi_ok;
    }

    std::string stack;
    if (IsCrossThreadCheckEnabled()) {
        if (DumpHybridStack(GetEcmaVm(), stack, 1, 8)) { // 1: skipd frames, 8: backtrace deepth
            HILOG_ERROR("AddCleanupHook Failed, data cannot register multiple times."
                "\n%{public}s", stack.c_str());
        } else {
            HILOG_ERROR("AddCleanupHook Failed, data cannot register multiple times, "
                "backtrace failed or unsupported platform.");
        }
    } else {
        HILOG_ERROR("AddCleanupHook Failed, data cannot register multiple times, "
            "enable cross-thread check for more information.");
    }

    return napi_generic_failure;
}

napi_status NativeEngine::RemoveCleanupHook(CleanupCallback fun, void* arg)
{
    auto cleanupHook = cleanupHooks_.find(arg);
    if (cleanupHook != cleanupHooks_.end() && cleanupHook->second.first == fun) {
        cleanupHooks_.erase(arg);
        return napi_ok;
    }

    const char *failedReason = cleanupHook == cleanupHooks_.end() ? "data is not registered or already unregistered"
                                                             : "callback not equals to last registered";
    std::string stack;
    if (IsCrossThreadCheckEnabled()) {
        if (DumpHybridStack(GetEcmaVm(), stack, 1, 8)) { // 1: skiped frames, 8: backtrace deepth
            HILOG_ERROR("RemoveCleanupHook Failed, %{public}s"
                ".\n%{public}s", failedReason, stack.c_str());
        } else {
            HILOG_ERROR("RemoveCleanupHook Failed %{public}s, "
                "backtrace failed or unsupported platform.", failedReason);
        }
    } else {
        HILOG_ERROR("RemoveCleanupHook Failed, %{public}s, "
            "enable cross thread check for more information.", failedReason);
    }

    return napi_generic_failure;
}

void NativeEngine::RunCleanup()
{
    HILOG_DEBUG("%{public}s, start.", __func__);
    CleanupHandles();
    // sync clean up
    while (!cleanupHooks_.empty()) {
        HILOG_DEBUG("NativeEngine::RunCleanup cleanupHooks_ is not empty");
        using CleanupCallbackTuple = std::pair<void*, std::pair<CleanupCallback, uint64_t>>;
        // Copy into a vector, since we can't sort an unordered_set in-place.
        std::vector<CleanupCallbackTuple> callbacks(cleanupHooks_.begin(), cleanupHooks_.end());
        // We can't erase the copied elements from `cleanupHooks_` yet, because we
        // need to be able to check whether they were un-scheduled by another hook.

        std::sort(callbacks.begin(), callbacks.end(), [](const CleanupCallbackTuple& a, const CleanupCallbackTuple& b) {
            // Sort in descending order so that the most recently inserted callbacks are run first.
            return a.second.second > b.second.second;
        });
        HILOG_DEBUG(
            "NativeEngine::RunCleanup cleanup_hooks callbacks size:%{public}d", (int32_t)callbacks.size());
        for (const CleanupCallbackTuple& cb : callbacks) {
            void* data = cb.first;
            if (cleanupHooks_.find(data) == cleanupHooks_.end()) {
                // This hook was removed from the `cleanupHooks_` set during another
                // hook that was run earlier. Nothing to do here.
                continue;
            }
            CleanupCallback fun = cb.second.first;
            if (fun != nullptr) {
                fun(data);
            }
            cleanupHooks_.erase(data);
        }
        CleanupHandles();
    }

    // make sure tsfn relese by itself
    uv_run(loop_, UV_RUN_NOWAIT);

    // Close all unclosed uv handles
    auto const ensureClosing = [](uv_handle_t *handle, void *arg) {
        if (!uv_is_closing(handle)) {
            uv_close(handle, nullptr);
        }
    };
    uv_walk(loop_, ensureClosing, nullptr);

    while (uv_run(loop_, UV_RUN_DEFAULT) != 0) {};

    HILOG_DEBUG("%{public}s, end.", __func__);
}

void NativeEngine::CleanupHandles()
{
    while (requestWaiting_.load() > 0) {
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

// this interface for restrictedWorker with entryPoint to execute mergeabc
napi_value NativeEngine::RunScriptForAbc(const char* path, char* entryPoint)
{
    EcmaVM* vm = const_cast<EcmaVM*>(GetEcmaVm());
    panda::EscapeLocalScope scope(vm);
    std::string normalizedPath = panda::JSNApi::NormalizePath(path);
    uint8_t* scriptContent = nullptr;
    size_t scriptContentSize = 0;
    std::vector<uint8_t> content;
    std::string ami;
    if (!GetAbcBuffer(normalizedPath.c_str(), &scriptContent, &scriptContentSize,
        content, ami, true)) {
        HILOG_ERROR("RunScript: GetAbcBuffer failed");
        return nullptr;
    }
    HILOG_DEBUG("RunScriptForAbc: GetAmi: %{private}s", ami.c_str());
    // if buffer is empty, return directly.
    if (scriptContentSize == 0) {
        HILOG_ERROR("asset size is %{public}zu", scriptContentSize);
        ThrowException("RunScriptForAbc: abc file is empty.");
        return nullptr;
    }
    panda::JSNApi::Execute(vm, ami, entryPoint, false, panda::ecmascript::ExecuteTypes::NAPI);
    if (panda::JSNApi::HasPendingException(vm)) {
        HandleUncaughtException();
        return nullptr;
    }
    Local<JSValueRef> undefObj = JSValueRef::Undefined(vm);
    return JsValueFromLocalValue(scope.Escape(undefObj));
}

napi_value NativeEngine::RunScript(const char* path, char* entryPoint)
{
    uint8_t* scriptContent = nullptr;
    size_t scriptContentSize = 0;
    std::vector<uint8_t> content;
    std::string ami;
    if (!GetAbcBuffer(path, &scriptContent, &scriptContentSize, content, ami)) {
        HILOG_ERROR("RunScript: GetAbcBuffer failed");
        return nullptr;
    }
    HILOG_DEBUG("RunScript: GetAmi: %{private}s", ami.c_str());
    // if buffer is empty, return directly.
    if (scriptContentSize == 0) {
        HILOG_ERROR("RunScript: buffer size is empty, please check abc path");
        return nullptr;
    }
    return RunActor(scriptContent, scriptContentSize, ami.c_str(), entryPoint, false);
}

napi_value NativeEngine::RunScriptInRestrictedThread(const char* path)
{
    auto vm = GetEcmaVm();
    panda::EscapeLocalScope scope(vm);
    uint8_t* scriptContent = nullptr;
    size_t scriptContentSize = 0;
    std::vector<uint8_t> content;
    std::string ami;
    if (!GetAbcBuffer(path, &scriptContent, &scriptContentSize, content, ami, true)) {
        HILOG_ERROR("RunScriptInRestrictedThread: GetAbcBuffer failed");
        return nullptr;
    }
    HILOG_DEBUG("RunScriptInRestrictedThread: GetAmi: %{private}s", ami.c_str());
    panda::JSNApi::Execute(vm, ami, PANDA_MAIN_FUNCTION, false, panda::ecmascript::ExecuteTypes::NAPI);
    if (panda::JSNApi::HasPendingException(vm)) {
        HandleUncaughtException();
        return nullptr;
    }
    Local<JSValueRef> undefObj = JSValueRef::Undefined(vm);
    return JsValueFromLocalValue(scope.Escape(undefObj));
}

/* buffer, bufferSize is for secureMem; content is for normalMem.
 * If output is not secureMem, fullfill buffer, bufferSize with content data and size.
 */
bool NativeEngine::GetAbcBuffer(const char* path, uint8_t **buffer, size_t* bufferSize,
    std::vector<uint8_t>& content, std::string& ami, bool isRestrictedWorker)
{
    std::string pathStr(path);
    bool useSecureMem = false;
    if (!CallGetAssetFunc(pathStr, buffer, bufferSize, content, ami, useSecureMem, isRestrictedWorker)) {
        HILOG_ERROR("Get asset error");
        return false;
    }
    if (!useSecureMem) {
        *buffer = content.data();
        *bufferSize = content.size();
    }
    return true;
}

void NativeEngine::SetInstanceData(void* data, NativeFinalize finalize_cb, void* hint)
{
    HILOG_DEBUG("NativeEngineWraper::%{public}s, start.", __func__);
    std::lock_guard<std::mutex> insLock(instanceDataLock_);
    FinalizerInstanceData();
    instanceDataInfo_.engine = this;
    instanceDataInfo_.callback = finalize_cb;
    instanceDataInfo_.nativeObject = data;
    instanceDataInfo_.hint = hint;
}

void NativeEngine::GetInstanceData(void** data)
{
    HILOG_DEBUG("NativeEngineWraper::%{public}s, start.", __func__);
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
    HILOG_DEBUG("%{public}s, start.", __func__);
    if (moduleFileName_.empty()) {
        NativeModuleManager* moduleManager = GetModuleManager();
        HILOG_DEBUG("NativeEngineWraper::GetFileName GetModuleManager");
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

napi_status NativeEngine::RunEventLoop(napi_event_mode mode)
{
    HILOG_DEBUG("%{public}s, start.", __func__);
    if (loop_ == nullptr) {
        HILOG_ERROR("nullptr loop in native engine");
        return napi_status::napi_invalid_arg;
    }

    if (!IsNativeThread()) {
        HILOG_ERROR("current thread is not native thread");
        return napi_status::napi_generic_failure;
    }

    std::unique_lock<std::mutex> lock(loopRunningMutex_);
    if (isLoopRunning_) {
        HILOG_DEBUG("loop is already undered running state");
        return napi_status::napi_ok;
    }
    isLoopRunning_ = true;
    lock.unlock();
    HILOG_DEBUG("uv loop is running with mode %{public}d", mode);
    if (mode == napi_event_mode_default) {
        uv_run(loop_, UV_RUN_DEFAULT);
    }
    if (mode == napi_event_mode_nowait) {
        uv_run(loop_, UV_RUN_NOWAIT);
    }
    HILOG_DEBUG("uv loop is stopped");
    lock.lock();
    isLoopRunning_ = false;
    return napi_status::napi_ok;
}

napi_status NativeEngine::StopEventLoop()
{
    HILOG_DEBUG("%{public}s, start.", __func__);
    if (loop_ == nullptr) {
        HILOG_ERROR("nullptr loop in native engine");
        return napi_status::napi_invalid_arg;
    }

    if (!IsNativeThread()) {
        HILOG_ERROR("current thread is not native thread");
        return napi_status::napi_generic_failure;
    }
    std::unique_lock<std::mutex> lock(loopRunningMutex_);
    if (!isLoopRunning_) {
        HILOG_DEBUG("loop is already undered stop state");
        return napi_status::napi_ok;
    }
    HILOG_DEBUG("uv loop is running");
    uv_stop(loop_);
    HILOG_DEBUG("uv loop is stopped");
    return napi_status::napi_ok;
}

void NativeEngine::ThrowException(const char* msg)
{
    auto vm = GetEcmaVm();
    Local<panda::JSValueRef> error = panda::Exception::Error(vm, StringRef::NewFromUtf8(vm, msg));
    panda::JSNApi::ThrowException(vm, error);
}

NapiErrorManager* NapiErrorManager::GetInstance()
{
    if (instance_ == NULL) {
        std::lock_guard<std::mutex> lock(g_errorManagerInstanceMutex);
        if (instance_ == NULL) {
            instance_ = new NapiErrorManager();
            HILOG_DEBUG("create error manager instance");
        }
    }
    return instance_;
}

void NapiErrorManager::NotifyUncaughtException(napi_env env, napi_value exception, std::string name, uint32_t type)
{
    auto hasOnErrorCallback = NapiErrorManager::GetInstance()->GetHasErrorCallback();
    if (!hasOnErrorCallback) {
        return;
    }
    if (!hasOnErrorCallback()) {
        return;
    }
    auto callback = NapiErrorManager::GetInstance()->GetOnWorkerErrorCallback();
    if (callback) {
        callback(env, exception, name, type);
    }
}

bool NapiErrorManager::NotifyUncaughtException(napi_env env, const std::string &summary, const std::string &name,
    const std::string &message, const std::string &stack)
{
    auto hasOnErrorCallback = NapiErrorManager::GetInstance()->GetHasErrorCallback();
    if (!hasOnErrorCallback) {
        return false;
    }
    if (!hasOnErrorCallback()) {
        return false;
    }
    panda::JSNApi::GetAndClearUncaughtException(reinterpret_cast<NativeEngine*>(env)->GetEcmaVm());
    auto callback = NapiErrorManager::GetInstance()->GetOnMainThreadErrorCallback();
    if (!callback) {
        return false;
    }
    return callback(env, summary, name, message, stack);
}

void NapiErrorManager::NotifyUnhandledRejection(napi_env env, napi_value* args, std::string name, uint32_t type)
{
    auto hasAllUnhandledRejectionCallback = NapiErrorManager::GetInstance()->GetHasAllUnhandledRejectionCallback();
    if (!hasAllUnhandledRejectionCallback) {
        return;
    }
    if (!hasAllUnhandledRejectionCallback()) {
        return;
    }
    auto callback = NapiErrorManager::GetInstance()->GetAllUnhandledRejectionCallback();
    if (callback) {
        callback(env, args, name, type);
    }
}
