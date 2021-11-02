/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "native_engine.h"
#include "utils/log.h"

#include <sys/epoll.h>
#include <uv.h>

#include "utils/log.h"

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

NativeEngine::NativeEngine(void* jsEngine) : jsEngine_(jsEngine) {}

void NativeEngine::Init()
{
    moduleManager_ = NativeModuleManager::GetInstance();
    referenceManager_ = new NativeReferenceManager();
    scopeManager_ = new NativeScopeManager();
    loop_ = uv_loop_new();
    if (loop_ == nullptr) {
        return;
    }
    uv_async_init(loop_, &uvAsync_, nullptr);
    uv_sem_init(&uvSem_, 0);
    lastException_ = nullptr;
}

NativeEngine::~NativeEngine()
{
    if (cleanEnv_ != nullptr) {
        cleanEnv_();
    }
}

void NativeEngine::Deinit()
{
    if (referenceManager_ != nullptr) {
        delete referenceManager_;
        referenceManager_ = nullptr;
    }
    if (scopeManager_ != nullptr) {
        delete scopeManager_;
        scopeManager_ = nullptr;
    }

    uv_sem_destroy(&uvSem_);
    uv_close((uv_handle_t*)&uvAsync_, nullptr);
    uv_run(loop_, UV_RUN_ONCE);
    uv_loop_delete(loop_);
}

NativeScopeManager* NativeEngine::GetScopeManager()
{
    return scopeManager_;
}

NativeReferenceManager* NativeEngine::GetReferenceManager()
{
    return referenceManager_;
}

NativeModuleManager* NativeEngine::GetModuleManager()
{
    return moduleManager_;
}

uv_loop_t* NativeEngine::GetUVLoop() const
{
    return loop_;
}

void NativeEngine::Loop(LoopMode mode, bool needSync)
{
    bool more = true;
    switch (mode) {
        case LOOP_DEFAULT:
            more = uv_run(loop_, UV_RUN_DEFAULT);
            break;
        case LOOP_ONCE:
            more = uv_run(loop_, UV_RUN_ONCE);
            break;
        case LOOP_NOWAIT:
            more = uv_run(loop_, UV_RUN_NOWAIT);
            break;
        default:
            return;
    }
    if (more == false) {
        more = uv_loop_alive(loop_);
    }

    if (needSync) {
        uv_sem_post(&uvSem_);
    }
}

NativeAsyncWork* NativeEngine::CreateAsyncWork(NativeValue* asyncResource, NativeValue* asyncResourceName,
    NativeAsyncExecuteCallback execute, NativeAsyncCompleteCallback complete, void* data)
{
    (void)asyncResource;
    (void)asyncResourceName;
    return new NativeAsyncWork(this, execute, complete, data);
}

NativeAsyncWork* NativeEngine::CreateAsyncWork(NativeAsyncExecuteCallback execute,
                                               NativeAsyncCompleteCallback complete,
                                               void* data)
{
    return new NativeAsyncWork(this, execute, complete, data);
}

void NativeEngine::InitAsyncWork(NativeAsyncExecuteCallback execute,
                                 NativeAsyncCompleteCallback complete,
                                 void* data)
{
    asyncWorker_ = std::make_unique<NativeAsyncWork>(this, execute, complete, data);
    asyncWorker_->Init();
}

bool NativeEngine::SendAsyncWork(void* data)
{
    if (!asyncWorker_) {
        HILOG_ERROR("asyncWorker_ is nullptr");
        return false;
    }
    asyncWorker_->Send(data);
    return true;
}

void NativeEngine::CloseAsyncWork()
{
    if (!asyncWorker_) {
        HILOG_ERROR("asyncWorker_ is nullptr");
        return;
    }
    asyncWorker_->Close();
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

void NativeEngine::ClearLastError()
{
    lastError_.errorCode = 0;
    lastError_.engineErrorCode = 0;
    lastError_.message = nullptr;
    lastError_.reserved = nullptr;
}

bool NativeEngine::IsExceptionPending() const
{
    return !(lastException_ == nullptr);
}

NativeValue* NativeEngine::GetAndClearLastException()
{
    NativeValue* temp = lastException_;
    lastException_ = nullptr;
    return temp;
}

void NativeEngine::EncodeToUtf8(NativeValue* nativeValue,
                                char* buffer,
                                int32_t* written,
                                size_t bufferSize,
                                int32_t* nchars)
{
    if (nativeValue == nullptr || nchars == nullptr || written == nullptr) {
        HILOG_ERROR("NativeEngine EncodeToUtf8 args is nullptr");
        return;
    }

    auto nativeString = reinterpret_cast<NativeString*>(nativeValue->GetInterface(NativeString::INTERFACE_ID));

    if (nativeString == nullptr) {
        HILOG_ERROR("nativeValue GetInterface is nullptr");
        return;
    }
    *written = nativeString->EncodeWriteUtf8(buffer, bufferSize, nchars);
}

void NativeEngine::CheckUVLoop()
{
    checkUVLoop_ = true;
    uv_thread_create(&uvThread_, NativeEngine::UVThreadRunner, this);
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

void NativeEngine::UVThreadRunner(void* nativeEngine)
{
    auto engine = static_cast<NativeEngine *>(nativeEngine);
    engine->PostLoopTask();
    while (engine->checkUVLoop_) {
        int32_t fd = uv_backend_fd(engine->loop_);
        int32_t timeout = uv_backend_timeout(engine->loop_);
        struct epoll_event ev;
        int32_t result = epoll_wait(fd, &ev, 1, timeout);
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

void NativeEngine::SetPostTask(PostTask postTask)
{
    HILOG_INFO("SetPostTask in");
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
void NativeEngine::SetGetAssetFunc(GetAssetFunc func)
{
    getAssetFunc_ = func;
}
void NativeEngine::SetOffWorkerFunc(OffWorkerFunc func)
{
    offWorkerFunc_ = func;
}
void NativeEngine::SetWorkerAsyncWorkFunc(NativeAsyncExecuteCallback executeCallback,
                                          NativeAsyncCompleteCallback completeCallback)
{
    nativeAsyncExecuteCallback_ = executeCallback;
    nativeAsyncCompleteCallback_ = completeCallback;
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
bool NativeEngine::CallGetAssetFunc(const std::string& uri, std::vector<uint8_t>& content)
{
    if (getAssetFunc_ != nullptr) {
        getAssetFunc_(uri, content);
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

bool NativeEngine::CallWorkerAsyncWorkFunc(NativeEngine* engine)
{
    if (nativeAsyncExecuteCallback_ != nullptr && nativeAsyncCompleteCallback_ != nullptr) {
        engine->InitAsyncWork(nativeAsyncExecuteCallback_, nativeAsyncCompleteCallback_, nullptr);
        return true;
    }
    return false;
}