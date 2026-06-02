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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_CONTAINER_SCOPED_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_CONTAINER_SCOPED_H

#include "native_engine.h"

class NapiContainerScope {
public:
    NapiContainerScope(NativeEngine* engine, int32_t scopeId, bool enable)
        : engine_(engine), enable_(enable), initialized_(false)
    {
        if (engine_ != nullptr && enable_) {
            restoreId_ = engine_->GetContainerScopeIdFunc();
            initialized_ = engine_->InitContainerScopeFunc(scopeId);
        }
    }
    ~NapiContainerScope()
    {
        if (engine_ != nullptr && initialized_) {
            engine_->InitContainerScopeFunc(restoreId_);
        }
    }

    bool IsInitialized()
    {
        return initialized_;
    }

private:
    NativeEngine* engine_ { nullptr };
    int32_t restoreId_ { -1 };
    bool enable_ { false };
    bool initialized_ { false };
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_CONTAINER_SCOPED_H */
