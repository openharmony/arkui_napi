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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_API_INTERNAL_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_API_INTERNAL_H

#include <cinttypes>

#include "napi/native_api.h"
#include "native_engine.h"
#include "utils/log.h"

static inline napi_status napi_clear_last_error(napi_env env)
{
    ((NativeEngine*)env)->ClearLastError();
    return napi_ok;
}

static inline napi_status napi_set_last_error(napi_env env,
                                              napi_status error_code,
                                              uint32_t engine_error_code = 0,
                                              void* engine_reserved = nullptr)
{
    ((NativeEngine*)env)->SetLastError(error_code, engine_error_code, engine_reserved);
    return error_code;
}

#define RETURN_STATUS_IF_FALSE(env, condition, status) \
    if (!(condition)) {                                \
        return napi_set_last_error((env), (status));   \
    }

#define CHECK_ENV(env)           \
    if ((env) == nullptr) {      \
        return napi_invalid_arg; \
    }

#define CHECK_ARG(env, arg) RETURN_STATUS_IF_FALSE((env), ((arg) != nullptr), napi_invalid_arg)

#define NAPI_PREAMBLE(env)                                                     \
    CHECK_ENV((env));                                                          \
    RETURN_STATUS_IF_FALSE(                                                    \
        (env),                                                                 \
        (reinterpret_cast<NativeEngine*>(env))->lastException_.IsEmpty(),      \
        napi_pending_exception);                                               \
        napi_clear_last_error((env));                                          \
    TryCatch tryCatch(env)

#define GET_RETURN_STATUS(env)                                                 \
    (!tryCatch.HasCaught()                                                     \
        ? napi_ok                                                              \
        : napi_set_last_error((env), napi_pending_exception))

#define CHECK_AND_CONVERT_TO_OBJECT(env, vm, nativeValue, obj)                 \
    bool isObj = (nativeValue)->IsObject((vm));                                \
    bool isFunc = (nativeValue)->IsFunction((vm));                             \
    RETURN_STATUS_IF_FALSE((env), isObj || isFunc, napi_object_expected);      \
    Local<ObjectRef> (obj);                                                    \
    if (LIKELY(isObj)) {                                                       \
        (obj) = Local<ObjectRef>((nativeValue));                               \
    } else {                                                                   \
        (obj) = (nativeValue)->ToObject((vm));                                 \
    }

#define CROSS_THREAD_CHECK(env)                                                     \
    do {                                                                            \
        NativeEngine* engine = reinterpret_cast<NativeEngine*>((env));              \
        if (UNLIKELY(engine->IsCrossThreadCheckEnabled())) {                        \
            ThreadId tid = NativeEngine::GetCurSysTid();                            \
            if (tid != engine->GetSysTid()) {                                       \
                HILOG_FATAL("current napi interface cannot run in multi-thread, "   \
                            "thread id: %{public}d, current thread id: %{public}d", \
                            engine->GetSysTid(), tid);                              \
            };                                                                      \
        };                                                                          \
    } while (0)

#define WEAK_CROSS_THREAD_CHECK(env)                                                                      \
    do {                                                                                                  \
        NativeEngine* engine = reinterpret_cast<NativeEngine*>((env));                                    \
        if (UNLIKELY(engine->IsCrossThreadCheckEnabled())) {                                              \
            ThreadId tid = NativeEngine::GetCurSysTid();                                                  \
            if (tid != engine->GetSysTid()) {                                                             \
                std::string stack;                                                                        \
                bool backtrace = DumpHybridStack(engine->GetEcmaVm(), stack, 0, 8);                      \
                HILOG_ERROR("current napi interface cannot run in multi-thread, "                         \
                            "thread id: %{public}d, current thread id: %{public}d"                        \
                            "\n%{public}s",                                                               \
                            engine->GetSysTid(), tid, backtrace ? stack.c_str() : "Faild to backtrace."); \
            };                                                                                            \
        };                                                                                                \
    } while (0)

#define LOG_IF_SPECIAL(engine, condition, fmt, ...)                                                            \
    do {                                                                                                       \
        if ((condition)) {                                                                                     \
            HILOG_FATAL(fmt, ##__VA_ARGS__);                                                                   \
        } else {                                                                                               \
            std::string stack;                                                                                 \
            bool backtrace = DumpHybridStack((engine)->GetEcmaVm(), stack, 0, 8);                             \
            HILOG_ERROR(fmt "\n%{public}s", ##__VA_ARGS__, backtrace ? stack.c_str() : "Faild to backtrace."); \
        }                                                                                                      \
    } while (0)

#ifndef LIKELY
#define LIKELY(exp) (__builtin_expect((exp) != 0, true))  // NOLINT(cppcoreguidelines-macro-usage)
#endif

#ifndef UNLIKELY
#define UNLIKELY(exp) (__builtin_expect((exp) != 0, false))  // NOLINT(cppcoreguidelines-macro-usage)
#endif

inline bool ValidEngineCheck(NativeEngine* input,
                             NativeEngine* owner,
                             uint64_t ownerId,
                             const char* func,
                             const char* file,
                             int line)
{
    if (input != owner) {
        LOG_IF_SPECIAL(input, UNLIKELY(input->IsCrossThreadCheckEnabled()),
                       "param env not equal to its owner. %{public}s at %{public}s:%{public}d",
                       func, file, line);
        return false;
    }
    if (ownerId != input->GetId()) {
        LOG_IF_SPECIAL(input, UNLIKELY(input->IsCrossThreadCheckEnabled()),
                       "owner env has been destroyed, "
                       "owner id: %{public}" PRIu64 ", current env id: %{public}" PRIu64 ". "
                       "%{public}s at %{public}s:%{public}d, ",
                       ownerId, input->GetId(), func, file, line);
        return false;
    }
    return true;
}

#define VALID_ENGINE_CHECK(input, owner, id) \
    ValidEngineCheck((input), (owner), (id), __FUNCTION__, __FILENAME__, __LINE__)

#ifdef ENABLE_CONTAINER_SCOPE
inline bool EnableContainerScope(napi_env env)
{
    NativeEngine* engine = reinterpret_cast<NativeEngine*>(env);
    return engine->IsContainerScopeEnabled();
}
#endif

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_API_INTERNAL_H */
