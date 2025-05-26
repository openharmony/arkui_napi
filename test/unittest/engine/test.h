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

#ifndef FOUNDATION_ACE_NAPI_TEST_UNITTEST_TEST_H
#define FOUNDATION_ACE_NAPI_TEST_UNITTEST_TEST_H

#include "ark_native_engine.h"
#include "event_handler.h"
#include "napi/native_api.h"
#include "native_engine.h"
#include "test_common.h"

class NativeEngineTest : public testing::Test {
public:
    NativeEngineTest();
    virtual ~NativeEngineTest();
    static void* Run(void* args);

protected:
    NativeEngine* engine_;
    NativeEngine* multiContextEngine_;
    NativeEngine* moduleNameEngine_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> eventHandler_ = nullptr;
};

class NativeEngineProxy {
public:
    NativeEngineProxy()
    {
        // Setup
        panda::RuntimeOption option;
        option.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
        const int64_t poolSize = 0x1000000; // 16M
        option.SetGcPoolSize(poolSize);
        option.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::ERROR);
        option.SetDebuggerLibraryPath("");
        vm_ = panda::JSNApi::CreateJSVM(option);
        if (vm_ == nullptr) {
            return;
        }

        engine_ = new ArkNativeEngine(vm_, nullptr);
        loop_ = engine_->GetUVLoop();
        napi_open_handle_scope(reinterpret_cast<napi_env>(engine_), &scope_);
    }

    NativeEngineProxy(NativeEngine* engine) : isContextEngine_(true)
    {
        napi_create_ark_context(reinterpret_cast<napi_env>(engine), reinterpret_cast<napi_env*>(&engine_));
        vm_ = const_cast<EcmaVM*>(engine->GetEcmaVm());
        napi_open_handle_scope(reinterpret_cast<napi_env>(engine_), &scope_);
        loop_ = engine->GetUVLoop();
    }

    ~NativeEngineProxy()
    {
        napi_close_handle_scope(reinterpret_cast<napi_env>(engine_), scope_);
        scope_ = nullptr;
        if (!isContextEngine_) {
            delete engine_;
            panda::JSNApi::DestroyJSVM(vm_);
        } else {
            engine_->DestroyContext();
        }
        engine_ = nullptr;
        vm_ = nullptr;
    }

    void RunLoop(uv_run_mode mode) const
    {
        uv_run(loop_, mode);
    }

    inline ArkNativeEngine* operator->() const
    {
        return engine_;
    }

    inline operator napi_env() const
    {
        return reinterpret_cast<napi_env>(engine_);
    }

private:
    EcmaVM* vm_ { nullptr };
    ArkNativeEngine* engine_ { nullptr };
    napi_handle_scope scope_ = nullptr;
    uv_loop_t* loop_ { nullptr };
    const bool isContextEngine_ { false };
};

class LoggerCollector {
public:
    struct LogInfo {
        LogType type;
        LogLevel level;
        unsigned int domain;
        std::string tag;
        std::string msg;
    };

    explicit LoggerCollector(LogLevel level = LogLevel::LOG_ERROR, unsigned int domain = 0)
        : level_(level), domain_(domain)
    {
        Restart();
    }

    ~LoggerCollector()
    {
        Pause();
    }

    const LogInfo& GetLastLogInfo()
    {
        return lastMessageInfo_;
    }

    void Pause()
    {
        if (prev_) {
            prev_->next_ = next_;
        }
        if (next_) {
            next_->prev_ = prev_;
        }
        if (last_ == this) {
            last_ = prev_;
        }
        prev_ = nullptr;
        next_ = nullptr;
        if (last_ == nullptr) {
            LOG_SetCallback(nullptr);
        }
    }

    void Restart()
    {
        if (last_ != nullptr) {
            last_->next_ = this;
        } else {
            LOG_SetCallback(OnCollect);
        }
        prev_ = last_;
        last_ = this;
    }

    int IndexOf(const char* msg, int index = 0)
    {
        std::string str = stream_.str();
        if (index >= str.size()) {
            return -1;
        }
        auto pos = str.find(msg, index);
        if (pos == std::string::npos) {
            return -1;
        }
        return pos;
    }

    int Includes(const char* msg, int index = 0)
    {
        return IndexOf(msg, index) >= 0;
    }

    void Clear()
    {
        stream_.flush();
    }

private:
    void Collect(const LogType type, const LogLevel level, const unsigned int domain, const char* tag, const char* msg)
    {
        if (domain_ != 0 && domain_ != domain) {
            return;
        }
        if (level < level_) {
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        lastMessageInfo_.type = type;
        lastMessageInfo_.level = level;
        lastMessageInfo_.domain = domain;
        lastMessageInfo_.tag = tag;
        lastMessageInfo_.msg = msg;
        stream_ << msg;
    }

    static void
    OnCollect(const LogType type, const LogLevel level, const unsigned int domain, const char* tag, const char* msg)
    {
        auto collector = last_;
        while (collector) {
            collector->Collect(type, level, domain, tag, msg);
            collector = collector->prev_;
        }
    };

    const LogLevel level_;
    const unsigned int domain_;
    // write stream and lastMessageInfo should under lock
    std::mutex mutex_;
    std::ostringstream stream_;
    LogInfo lastMessageInfo_;
    LoggerCollector* prev_ { nullptr };
    LoggerCollector* next_ { nullptr };
    static LoggerCollector* last_;
};

class UVLoopRunner {
public:
    explicit UVLoopRunner(uv_loop_t* loop) : loop_(loop)
    {
        uv_walk(
            loop,
            [](uv_handle_t* handle, void* data) {
                // Unref all handles to allow loop auto-exit.
                if (uv_has_ref(handle) != 0) {
                    uv_unref(handle);
                    reinterpret_cast<UVLoopRunner*>(data)->handles_.insert(handle);
                }
            },
            this);
    }
    explicit UVLoopRunner(NativeEngine* engine) : UVLoopRunner(GetUVLoop(engine)) {}
    explicit UVLoopRunner(napi_env* env) : UVLoopRunner(reinterpret_cast<NativeEngine*>(env)) {}

    ~UVLoopRunner()
    {
        uv_walk(
            loop_,
            [](uv_handle_t* handle, void* data) {
                // Restore ref count for all saved handles.
                if (uv_has_ref(handle) == 0) {
                    auto* that = reinterpret_cast<UVLoopRunner*>(data);
                    if (that->handles_.find(handle) != that->handles_.end()) {
                        uv_ref(handle);
                    }
                }
            },
            this);
        handles_.clear();
    }

    void Run(uv_run_mode mode = UV_RUN_DEFAULT)
    {
        uv_run(loop_, mode);
    }

    static uv_loop_t* GetUVLoop(NativeEngine* engine)
    {
        if (engine->IsMainEnvContext()) {
            return engine->GetUVLoop();
        } else {
            return engine->GetParent()->GetUVLoop();
        }
    }

private:
    uv_loop_t* loop_ { nullptr };
    std::unordered_set<uv_handle_t*> handles_ {};
};

class NapiAsyncWorkTestData {
public:
    NapiAsyncWorkTestData(napi_env env, const char* name) : env_(env)
    {
        napi_value jsName = nullptr;
        napi_create_string_utf8((env), name, NAPI_AUTO_LENGTH, &jsName);
        napi_create_async_work((env), nullptr, jsName, OnExecute, OnComplete, this, &(work_));
    }

    ~NapiAsyncWorkTestData()
    {
        if (work_ != nullptr) {
            napi_delete_async_work(env_, work_);
        }
    }

    virtual void Execute(napi_env env) {}
    virtual void Complete(napi_env env, napi_status status) {}

    napi_status Queue()
    {
        return napi_queue_async_work(env_, work_);
    }

    napi_status Queue(napi_qos_t qos)
    {
        return napi_queue_async_work_with_qos(env_, work_, qos);
    }

    napi_status Cancel()
    {
        LoggerCollector collector;
        napi_status status = napi_cancel_async_work(env_, work_);
        if (status != napi_ok) {
            return status;
        }
        if (collector.Includes("uv_cancel failed")) {
            return napi_generic_failure;
        }
        return napi_ok;
    }

protected:
    napi_env env_;

private:
    static void OnExecute(napi_env env, void* data)
    {
        auto* that = reinterpret_cast<NapiAsyncWorkTestData*>(data);
        that->Execute(env);
    }

    static void OnComplete(napi_env env, napi_status status, void* data)
    {
        auto* that = reinterpret_cast<NapiAsyncWorkTestData*>(data);
        napi_delete_async_work(env, that->work_);
        that->work_ = nullptr;
        that->Complete(env, status);
    }

    napi_async_work work_ { nullptr };
};

#endif /* FOUNDATION_ACE_NAPI_TEST_UNITTEST_TEST_H */
