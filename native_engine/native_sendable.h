/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_SENDABLE_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_SENDABLE_H

#include "ecmascript/napi/include/jsnapi.h"
#include "native_engine/native_property.h"

using FunctionRef = panda::FunctionRef;
using panda::Local;

class NativeSendable {
public:
    static FunctionRef::SendablePropertiesInfos CreateSendablePropertiesInfos(napi_env env,
                                                                              const NapiPropertyDescriptor* properties,
                                                                              size_t propertiesLength);
    static void NapiDefineSendabledProperty(napi_env env,
                                            Local<panda::ObjectRef>& obj,
                                            NapiPropertyDescriptor& propertyDescriptor,
                                            Local<panda::JSValueRef>& propertyName,
                                            bool& result);
    static void InitSendablePropertiesInfo(napi_env env,
                                           FunctionRef::SendablePropertiesInfo& info,
                                           NapiPropertyDescriptor propertyDescriptor,
                                           FunctionRef::SendableType type = FunctionRef::SendableType::NONE);

private:
    static Local<panda::JSValueRef> NapiNativeCreateSendableFunction(napi_env env,
                                                                     const char* name,
                                                                     NapiNativeCallback cb,
                                                                     void* value);
};

#endif
