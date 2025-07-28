/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "native_api_internal.h"
#include "native_engine/native_async_hook_context.h"
#include "native_engine/native_utils.h"
#include "native_engine/impl/ark/ark_native_engine.h"

using panda::Local;
using panda::StringRef;

static constexpr int32_t MAX_THREAD_SAFE_COUNT = 128;

NAPI_EXTERN void napi_module_register(napi_module* mod)
{
    if (mod == nullptr) {
        HILOG_ERROR("mod is nullptr");
        return;
    }

    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
    NativeModule module;

    module.version = mod->nm_version;
    module.fileName = mod->nm_filename;
    module.name = mod->nm_modname;
    module.flags = mod->nm_flags;
    module.registerCallback = (RegisterCallback)mod->nm_register_func;

    moduleManager->Register(&module);
}

NAPI_EXTERN void napi_module_with_js_register(napi_module_with_js* mod)
{
    if (mod == nullptr) {
        HILOG_ERROR("mod is nullptr");
        return;
    }

    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
    NativeModule module;

    module.version = mod->nm_version;
    module.fileName = mod->nm_filename;
    module.name = mod->nm_modname;
    module.registerCallback = (RegisterCallback)mod->nm_register_func;
    module.getJSCode = (GetJSCodeCallback)mod->nm_get_js_code;
    module.getABCCode = (GetJSCodeCallback)mod->nm_get_abc_code;

    moduleManager->Register(&module);
}

NAPI_EXTERN NAPI_NO_RETURN void napi_fatal_error(const char* location,
                                                 size_t location_len,
                                                 const char* message,
                                                 size_t message_len)
{
    (void)location_len;
    (void)message_len;
    HILOG_FATAL("FATAL ERROR: %{public}s %{public}s\n", location, message);
}

NAPI_EXTERN napi_status napi_create_limit_runtime(napi_env env, napi_env* result_env)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result_env);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (!engine->IsMainEnvContext()) {
        HILOG_FATAL("multi-context does not support this interface");
    }

    auto result = engine->CreateRuntime(true);
    *result_env = reinterpret_cast<napi_env>(result);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_fatal_exception(napi_env env, napi_value err)
{
    NAPI_PREAMBLE(env);
    CHECK_ENV(env);
    CHECK_ARG(env, err);

    auto exceptionValue = LocalValueFromJsValue(err);
    auto ecmaVm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    RETURN_STATUS_IF_FALSE(env, exceptionValue->IsError(ecmaVm), napi_invalid_arg);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    engine->TriggerFatalException(exceptionValue);
    return napi_ok;
}

// Methods to manage simple async operations
NAPI_EXTERN napi_status napi_create_async_work(napi_env env,
                                               napi_value async_resource,
                                               napi_value async_resource_name,
                                               napi_async_execute_callback execute,
                                               napi_async_complete_callback complete,
                                               void* data,
                                               napi_async_work* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, async_resource_name);
    CHECK_ARG(env, execute);
    CHECK_ARG(env, complete);
    CHECK_ARG(env, result);

    SWITCH_CONTEXT(env);
    auto ecmaVm = engine->GetEcmaVm();
    auto asyncResource = LocalValueFromJsValue(async_resource);
    auto asyncResourceName = LocalValueFromJsValue(async_resource_name);
    auto asyncExecute = reinterpret_cast<NativeAsyncExecuteCallback>(execute);
    auto asyncComplete = reinterpret_cast<NativeAsyncCompleteCallback>(complete);
    (void)asyncResource;
    (void)asyncResourceName;
    char name[64] = {0}; // 64:NAME_BUFFER_SIZE
    if (!(asyncResourceName->IsNull() || asyncResourceName->IsUndefined())) {
        panda::Local<panda::StringRef> nativeString(asyncResourceName);
        int copied = nativeString->WriteUtf8(ecmaVm, name, 63, true) - 1;  // 63:NAME_BUFFER_SIZE
        name[copied] = '\0';
    }
    auto asyncWork  = new NativeAsyncWork(engine, asyncExecute, asyncComplete, name, data);
    *result = reinterpret_cast<napi_async_work>(asyncWork);
    return napi_status::napi_ok;
}

NAPI_EXTERN napi_status napi_delete_async_work(napi_env env, napi_async_work work)
{
    CHECK_ENV(env);
    CHECK_ARG(env, work);

    auto asyncWork = reinterpret_cast<NativeAsyncWork*>(work);
    delete asyncWork;
    asyncWork = nullptr;

    return napi_status::napi_ok;
}

NAPI_EXTERN napi_status napi_queue_async_work(napi_env env, napi_async_work work)
{
    CHECK_ENV(env);
    CHECK_ARG(env, work);

    auto asyncWork = reinterpret_cast<NativeAsyncWork*>(work);

    asyncWork->Queue(reinterpret_cast<NativeEngine*>(env));
    return napi_status::napi_ok;
}

NAPI_EXTERN napi_status napi_cancel_async_work(napi_env env, napi_async_work work)
{
    CHECK_ENV(env);
    CHECK_ARG(env, work);

    auto asyncWork = reinterpret_cast<NativeAsyncWork*>(work);

    asyncWork->Cancel(reinterpret_cast<NativeEngine*>(env));
    return napi_status::napi_ok;
}

// Version management
NAPI_EXTERN napi_status napi_get_node_version(napi_env env, const napi_node_version** version)
{
    (void)version;
    return napi_status::napi_ok;
}

// Return the current libuv event loop for a given environment
NAPI_EXTERN napi_status napi_get_uv_event_loop(napi_env env, struct uv_loop_s** loop)
{
    CHECK_ENV(env);
    CHECK_ARG(env, loop);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (!NativeEngine::IsAlive(engine)) {
        HILOG_ERROR("napi_env has been destroyed!");
        return napi_status::napi_generic_failure;
    }

    if (!engine->IsMainEnvContext()) {
        HILOG_ERROR("multi-context does not support obtain uv looper");
        return napi_set_last_error(env, napi_invalid_arg);
    }
    *loop = engine->GetUVLoop();

    return napi_status::napi_ok;
}

NAPI_EXTERN napi_status napi_add_env_cleanup_hook(napi_env env, void (*fun)(void* arg), void* arg)
{
    CHECK_ENV(env);
    CHECK_ARG(env, fun);
    WEAK_CROSS_THREAD_CHECK(env);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    engine->AddCleanupHook(fun, arg);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_remove_env_cleanup_hook(napi_env env, void (*fun)(void* arg), void* arg)
{
    CHECK_ENV(env);
    CHECK_ARG(env, fun);
    WEAK_CROSS_THREAD_CHECK(env);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    engine->RemoveCleanupHook(fun, arg);

    return napi_clear_last_error(env);
}

using CleanupHook = void (*)(void* arg);
using AsyncCleanupHook = void (*)(void* arg, void (*)(void*), void*);

struct AsyncCleanupHookInfo final {
    napi_env env_;
    AsyncCleanupHook fun_;
    void* arg_;
    bool started_ = false;
    // Use a self-reference to make sure the storage is kept alive while the
    // cleanup hook is registered but not yet finished.
    std::shared_ptr<AsyncCleanupHookInfo> self_;
};

// Opaque type that is basically an alias for `shared_ptr<AsyncCleanupHookInfo>`
// (but not publicly so for easier ABI/API changes). In particular,
// std::shared_ptr does not generally maintain a consistent ABI even on a
// specific platform.
struct ACHHandle final {
    std::shared_ptr<AsyncCleanupHookInfo> info_;
};

struct DeleteACHHandle {
    void operator()(ACHHandle* handle) const
    {
        delete handle;
    };
};
using AsyncCleanupHookHandle = std::unique_ptr<ACHHandle, DeleteACHHandle>;

static void FinishAsyncCleanupHook(void* arg)
{
    HILOG_INFO("%{public}s, start.", __func__);
    AsyncCleanupHookInfo* info = static_cast<AsyncCleanupHookInfo*>(arg);
    std::shared_ptr<AsyncCleanupHookInfo> keep_alive = info->self_;
    auto engine = reinterpret_cast<NativeEngine*>(info->env_);
    engine->DecreaseWaitingRequestCounter();
    info->self_.reset();
    HILOG_INFO("%{public}s, end.", __func__);
}

static void RunAsyncCleanupHook(void* arg)
{
    HILOG_INFO("%{public}s, start.", __func__);
    AsyncCleanupHookInfo* info = static_cast<AsyncCleanupHookInfo*>(arg);
    auto engine = reinterpret_cast<NativeEngine*>(info->env_);
    engine->IncreaseWaitingRequestCounter();
    info->started_ = true;
    info->fun_(info->arg_, FinishAsyncCleanupHook, info);
    HILOG_INFO("%{public}s, end.", __func__);
}

static AsyncCleanupHookHandle AddEnvironmentCleanupHook(napi_env env, AsyncCleanupHook fun, void* arg)
{
    HILOG_INFO("%{public}s, start.", __func__);
    auto info = std::make_shared<AsyncCleanupHookInfo>();
    info->env_ = env;
    info->fun_ = fun;
    info->arg_ = arg;
    info->self_ = info;
    auto engine = reinterpret_cast<NativeEngine*>(env);
    engine->AddCleanupHook(RunAsyncCleanupHook, info.get());
    HILOG_INFO("%{public}s, end.", __func__);
    return AsyncCleanupHookHandle(new ACHHandle { info });
}

static void RemoveEnvironmentCleanupHook(AsyncCleanupHookHandle handle)
{
    if (handle->info_->started_) {
        return;
    }
    handle->info_->self_.reset();
    auto engine = reinterpret_cast<NativeEngine*>(handle->info_->env_);
    engine->RemoveCleanupHook(RunAsyncCleanupHook, handle->info_.get());
}

struct napi_async_cleanup_hook_handle__ {
    napi_async_cleanup_hook_handle__(napi_env env, napi_async_cleanup_hook user_hook, void* user_data)
        : env_(env), user_hook_(user_hook), user_data_(user_data)
    {
        handle_ = AddEnvironmentCleanupHook(env, Hook, this);
    }

    ~napi_async_cleanup_hook_handle__()
    {
        RemoveEnvironmentCleanupHook(std::move(handle_));
        if (done_cb_ != nullptr) {
            done_cb_(done_data_);
        }
    }

    static void Hook(void* data, void (*done_cb)(void*), void* done_data)
    {
        auto handle = static_cast<napi_async_cleanup_hook_handle__*>(data);
        handle->done_cb_ = done_cb;
        handle->done_data_ = done_data;
        handle->user_hook_(handle, handle->user_data_);
    }

    AsyncCleanupHookHandle handle_;
    napi_env env_ = nullptr;
    napi_async_cleanup_hook user_hook_ = nullptr;
    void* user_data_ = nullptr;
    void (*done_cb_)(void*) = nullptr;
    void* done_data_ = nullptr;
};

NAPI_EXTERN napi_status napi_add_async_cleanup_hook(
    napi_env env, napi_async_cleanup_hook hook, void* arg, napi_async_cleanup_hook_handle* remove_handle)
{
    CHECK_ENV(env);
    CHECK_ARG(env, hook);
    CROSS_THREAD_CHECK(env);

    napi_async_cleanup_hook_handle__* handle = new napi_async_cleanup_hook_handle__(env, hook, arg);

    if (remove_handle != nullptr)
        *remove_handle = handle;

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_remove_async_cleanup_hook(napi_async_cleanup_hook_handle remove_handle)
{
    if (remove_handle == nullptr) {
        return napi_invalid_arg;
    }

    delete remove_handle;
    return napi_ok;
}

// Methods to manager threadsafe
NAPI_EXTERN napi_status napi_create_threadsafe_function(napi_env env, napi_value func, napi_value async_resource,
    napi_value async_resource_name, size_t max_queue_size, size_t initial_thread_count, void* thread_finalize_data,
    napi_finalize thread_finalize_cb, void* context, napi_threadsafe_function_call_js call_js_cb,
    napi_threadsafe_function* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, async_resource_name);
    RETURN_STATUS_IF_FALSE(
        env, initial_thread_count > 0 && initial_thread_count <= MAX_THREAD_SAFE_COUNT, napi_invalid_arg);
    CHECK_ARG(env, result);
    if (func == nullptr) {
        CHECK_ARG(env, call_js_cb);
    }

    SWITCH_CONTEXT(env);
    auto finalizeCallback = reinterpret_cast<NativeFinalize>(thread_finalize_cb);
    auto callJsCallback = reinterpret_cast<NativeThreadSafeFunctionCallJs>(call_js_cb);
    auto safeAsyncWork = engine->CreateSafeAsyncWork(func, async_resource, async_resource_name, max_queue_size,
        initial_thread_count, thread_finalize_data, finalizeCallback, context, callJsCallback);
    CHECK_ENV(safeAsyncWork);

    auto ret = safeAsyncWork->Init();
    if (ret) {
        *result = reinterpret_cast<napi_threadsafe_function>(safeAsyncWork);
    } else {
        return napi_status::napi_generic_failure;
    }

    return napi_status::napi_ok;
}

NAPI_EXTERN napi_status napi_call_threadsafe_function(
    napi_threadsafe_function func, void* data, napi_threadsafe_function_call_mode is_blocking)
{
    CHECK_ENV(func);

    auto safeAsyncWork = reinterpret_cast<NativeSafeAsyncWork*>(func);
    auto callMode = static_cast<NativeThreadSafeFunctionCallMode>(is_blocking);

    napi_status status = napi_status::napi_ok;
    auto code = safeAsyncWork->Send(data, callMode);
    switch (code) {
        case SafeAsyncCode::SAFE_ASYNC_OK:
            status = napi_status::napi_ok;
            break;
        case SafeAsyncCode::SAFE_ASYNC_QUEUE_FULL:
            status = napi_status::napi_queue_full;
            break;
        case SafeAsyncCode::SAFE_ASYNC_INVALID_ARGS:
            status = napi_status::napi_invalid_arg;
            break;
        case SafeAsyncCode::SAFE_ASYNC_CLOSED:
            status = napi_status::napi_closing;
            break;
        case SafeAsyncCode::SAFE_ASYNC_FAILED:
            status = napi_status::napi_generic_failure;
            break;
        default:
            HILOG_FATAL("this branch is unreachable, code is %{public}d", code);
            break;
    }

    return status;
}

NAPI_EXTERN napi_status napi_acquire_threadsafe_function(napi_threadsafe_function func)
{
    CHECK_ENV(func);

    auto safeAsyncWork = reinterpret_cast<NativeSafeAsyncWork*>(func);

    auto ret = safeAsyncWork->Acquire();
    if (ret != SafeAsyncCode::SAFE_ASYNC_OK) {
        return napi_status::napi_generic_failure;
    }

    return napi_status::napi_ok;
}

NAPI_EXTERN napi_status napi_release_threadsafe_function(
    napi_threadsafe_function func, napi_threadsafe_function_release_mode mode)
{
    CHECK_ENV(func);

    auto safeAsyncWork = reinterpret_cast<NativeSafeAsyncWork*>(func);
    auto releaseMode = static_cast<NativeThreadSafeFunctionReleaseMode>(mode);

    auto ret = safeAsyncWork->Release(releaseMode);
    if (ret != SafeAsyncCode::SAFE_ASYNC_OK) {
        return napi_status::napi_generic_failure;
    }

    return napi_status::napi_ok;
}

NAPI_EXTERN napi_status napi_get_threadsafe_function_context(napi_threadsafe_function func, void** result)
{
    CHECK_ENV(func);
    CHECK_ENV(result);

    auto safeAsyncWork = reinterpret_cast<NativeSafeAsyncWork*>(func);
    *result = safeAsyncWork->GetContext();

    return napi_status::napi_ok;
}

NAPI_EXTERN napi_status napi_ref_threadsafe_function(napi_env env, napi_threadsafe_function func)
{
    CHECK_ENV(env);
    CHECK_ARG(env, func);

    auto safeAsyncWork = reinterpret_cast<NativeSafeAsyncWork*>(func);
    auto ret = safeAsyncWork->Ref();
    if (!ret) {
        return napi_status::napi_generic_failure;
    }

    return napi_status::napi_ok;
}

NAPI_EXTERN napi_status napi_unref_threadsafe_function(napi_env env, napi_threadsafe_function func)
{
    CHECK_ENV(env);
    CHECK_ARG(env, func);

    auto safeAsyncWork = reinterpret_cast<NativeSafeAsyncWork*>(func);
    auto ret = safeAsyncWork->Unref();
    if (!ret) {
        return napi_status::napi_generic_failure;
    }

    return napi_status::napi_ok;
}

NAPI_EXTERN napi_status napi_async_init(
    napi_env env, napi_value async_resource, napi_value async_resource_name, napi_async_context* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, async_resource_name);
    CHECK_ARG(env, result);

    SWITCH_CONTEXT(env);
    auto ecmaVm = reinterpret_cast<NativeEngine*>(env)->GetEcmaVm();
    panda::Local<panda::ObjectRef> resource;
    bool isExternalResource;
    if (async_resource != nullptr) {
        auto nativeValue = LocalValueFromJsValue(async_resource);
        resource = nativeValue->ToObject(ecmaVm);
        isExternalResource = true;
    } else {
        resource = panda::ObjectRef::New(ecmaVm);
        isExternalResource = false;
    }

    auto nativeValue = LocalValueFromJsValue(async_resource_name);
    auto resourceName = nativeValue->ToString(ecmaVm);

    auto asyncContext = new NativeAsyncHookContext(reinterpret_cast<NativeEngine*>(env),
                                                   resource,
                                                   resourceName,
                                                   isExternalResource);

    *result = reinterpret_cast<napi_async_context>(asyncContext);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_async_destroy(napi_env env, napi_async_context async_context)
{
    CHECK_ENV(env);
    CHECK_ARG(env, async_context);

    NativeAsyncHookContext* nativeAsyncContext = reinterpret_cast<NativeAsyncHookContext*>(async_context);

    delete nativeAsyncContext;

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_open_callback_scope(
    napi_env env, napi_value, napi_async_context async_context_handle, napi_callback_scope* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    NativeAsyncHookContext* nodeAsyncContext = reinterpret_cast<NativeAsyncHookContext*>(async_context_handle);

    *result = reinterpret_cast<napi_callback_scope>(nodeAsyncContext->OpenCallbackScope());

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_close_callback_scope(napi_env env, napi_callback_scope scope)
{
    CHECK_ENV(env);
    CHECK_ARG(env, scope);

    auto ret = NativeAsyncHookContext::CloseCallbackScope(reinterpret_cast<NativeEngine*>(env),
                                                          reinterpret_cast<NativeCallbackScope*>(scope));
    if (ret == CALLBACK_SCOPE_MISMATCH) {
        return napi_callback_scope_mismatch;
    } else if (ret != CALLBACK_SCOPE_OK) {
        return napi_invalid_arg;
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_set_instance_data(
    napi_env env, void* data, napi_finalize finalize_cb, void* finalize_hint)
{
    CHECK_ENV(env);
    CROSS_THREAD_CHECK(env);
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto callback = reinterpret_cast<NativeFinalize>(finalize_cb);
    engine->SetInstanceData(data, callback, finalize_hint);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_instance_data(napi_env env, void** data)
{
    CHECK_ENV(env);
    CHECK_ARG(env, data);
    CROSS_THREAD_CHECK(env);
    auto engine = reinterpret_cast<NativeEngine*>(env);
    engine->GetInstanceData(data);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status node_api_get_module_file_name(napi_env env, const char** result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);
    auto engine = reinterpret_cast<NativeEngine*>(env);
    *result = engine->GetModuleFileName();
    HILOG_INFO("%{public}s, napi called fileName : %{public}s", __func__, *result);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_make_callback(napi_env env,
                                           napi_async_context async_context,
                                           napi_value recv,
                                           napi_value func,
                                           size_t argc,
                                           const napi_value* argv,
                                           napi_value* result)
{
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, func);
    CHECK_ARG(env, recv);
    if (argc > 0) {
        CHECK_ARG(env, argv);
    }
    SWITCH_CONTEXT(env);
    auto vm = engine->GetEcmaVm();
    RETURN_STATUS_IF_FALSE(env, reinterpret_cast<panda::JSValueRef *>(recv)->IsObject(vm), napi_object_expected);
    RETURN_STATUS_IF_FALSE(env, reinterpret_cast<panda::JSValueRef *>(func)->IsFunction(vm), napi_function_expected);
    panda::JSValueRef* Obj = reinterpret_cast<panda::JSValueRef *>(recv);
    panda::FunctionRef* funRef = reinterpret_cast<panda::FunctionRef *>(func);
    panda::JSValueRef* callBackRst;
    if (async_context == nullptr) {
        callBackRst = MakeCallback(engine, funRef, Obj,
                                   argc, reinterpret_cast<panda::JSValueRef* const*>(argv), nullptr);
    } else {
        NativeAsyncHookContext* nativeAsyncContext = reinterpret_cast<NativeAsyncHookContext*>(async_context);
        callBackRst = nativeAsyncContext->MakeCallback(funRef, Obj,
                                                       reinterpret_cast<panda::JSValueRef* const*>(argv), argc);
    }
    if (tryCatch.HasCaught()) {
        HILOG_ERROR("print exception info: ");
        panda::JSNApi::PrintExceptionInfo(vm);
        return napi_set_last_error(env, napi_pending_exception);
    }
    if (result) {
        *result = reinterpret_cast<napi_value>(callBackRst);
    }
    return GET_RETURN_STATUS(env);
}

NAPI_EXTERN napi_status napi_set_module_validate_callback(napi_module_validate_callback check_callback)
{
    CHECK_ENV(check_callback);

    if (ArkNativeEngine::SetModuleValidateCallback(check_callback)) {
        return napi_ok;
    }
    return napi_generic_failure;
}