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

#include "native_engine/impl/ark/ark_sendable_native_reference.h"
#include "native_engine/native_utils.h"
#include "utils/log.h"

ArkSendableNativeReference::ArkSendableNativeReference(ArkNativeEngine* engine, Local<JSValueRef> value)
    : value_(engine->GetEcmaVm(), value)
{}

void ArkSendableNativeReference::DeleteSendableRef(ArkNativeEngine* engine)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (value_.IsEmpty()) {
        HILOG_FATAL("delete napi_sendable_ref value failed, value is empty.");
        return;
    }
    value_.FreeSendableGlobalHandleAddr(engine->GetEcmaVm());
}

napi_value ArkSendableNativeReference::Get(ArkNativeEngine* engine)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (value_.IsEmpty()) {
        HILOG_FATAL("get napi_sendable_ref value failed, value is empty.");
        return nullptr;
    }

    Local<JSValueRef> value = value_.ToLocal(engine->GetEcmaVm());
    return JsValueFromLocalValue(value);
}

