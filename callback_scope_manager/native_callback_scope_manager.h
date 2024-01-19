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

#ifndef FOUNDATION_ACE_NAPI_CALLBACK_SCOPE_MANAGER_NATIVE_CALLBACK_SCOPE_MANAGER_H
#define FOUNDATION_ACE_NAPI_CALLBACK_SCOPE_MANAGER_NATIVE_CALLBACK_SCOPE_MANAGER_H

#include <cstddef>
#include "ecmascript/napi/include/jsnapi.h"

class NativeEngine;

class NativeAsyncWrap {
public:
    static void EmitAsyncInit(NativeEngine* env,
                              panda::Local<panda::ObjectRef> object,
                              panda::Local<panda::StringRef> type,
                              double asyncId,
                              double triggerAsyncId) {}

    static void EmitDestroy(NativeEngine* env, double asyncId) {}

    static void EmitBefore(NativeEngine* env, double asyncId) {}

    static void EmitAfter(NativeEngine* env, double asyncId) {}
};

struct AsyncIdInfo {
    double asyncId;
    double triggerAsyncId;
};

class NativeCallbackScope {
public:
    enum Flags {
        NO_FLAGS = 0,
        SKIP_ASYNC_HOOKS = 1,
        SKIP_TASK_QUEUES = 2
    };
    NativeCallbackScope(NativeEngine* env,
                        panda::Local<panda::ObjectRef> object,
                        const AsyncIdInfo& asyncIdInfo,
                        int flags = NO_FLAGS);
    virtual ~NativeCallbackScope();
    void Close();

    inline bool Failed() const
    {
        return failed_;
    }
    inline void MarkAsFailed()
    {
        failed_ = true;
    }

private:
    bool closed_ = false;
    bool failed_ = false;
    NativeEngine* env_ = nullptr;
    AsyncIdInfo asyncIdInfo_ = {0, 0};
    [[maybe_unused]] panda::Local<panda::ObjectRef> object_;
    bool skipHooks_ = false;
    bool skipTaskQueues_ = false;
    bool pushedIds_ = false;
};

class NativeCallbackScopeManager {
public:
    NativeCallbackScopeManager();
    virtual ~NativeCallbackScopeManager();

    NativeCallbackScope* Open(NativeEngine* env, panda::Local<panda::ObjectRef> object, AsyncIdInfo asyncContext);
    void Close(NativeCallbackScope* scope);

    size_t IncrementOpenCallbackScopes();
    size_t DecrementOpenCallbackScopes();

    size_t GetOpenCallbackScopes() const
    {
        return openCallbackScopes_;
    }

    size_t GetAsyncCallbackScopeDepth() const
    {
        return asyncCallbackScopeDepth_;
    }

private:
    size_t openCallbackScopes_ = 0;
    size_t asyncCallbackScopeDepth_ = 0;
};

#endif /* FOUNDATION_ACE_NAPI_CALLBACK_SCOPE_MANAGER_NATIVE_CALLBACK_SCOPE_MANAGER_H */
