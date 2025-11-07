/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_ARK_SENDABLE_NATIVE_REFERENCE_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_ARK_SENDABLE_NATIVE_REFERENCE_H

#include "ark_native_engine.h"
#include "native_engine/native_value.h"
#include "ecmascript/napi/include/jsnapi.h"

using panda::SendableGlobal;
using JSValueRef = panda::JSValueRef;
using panda::Local;

class ArkSendableNativeReference {
public:
    ArkSendableNativeReference(ArkNativeEngine* engine, panda::Local<JSValueRef> value);
    ~ArkSendableNativeReference() = default;
    void DeleteSendableRef(ArkNativeEngine* engine);
    napi_value Get(ArkNativeEngine* engine);
private:
    std::mutex mutex_;
    panda::SendableGlobal<JSValueRef> value_;
};

#endif // FOUNDATION_ACE_NAPI_NATIVE_ENGINE_ARK_SENDABLE_NATIVE_REFERENCE_H
