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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_VALUE_INL_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_VALUE_INL_H

#include "native_engine/native_value.h"
#include "ecmascript/napi/include/jsnapi.h"

using panda::JsiRuntimeCallInfo;

napi_value NapiNativeCallbackInfo::GetThis()
{
    return reinterpret_cast<napi_value>(&stackArgs_[THIS_INDEX]);
}
uint32_t NapiNativeCallbackInfo::GetArgsNumber() const
{
    return numArgs_ - FIRST_ARGS_INDEX;
}
napi_value NapiNativeCallbackInfo::GetArgs(uint32_t idx)
{
    return reinterpret_cast<napi_value>(&stackArgs_[FIRST_ARGS_INDEX + idx]);
}
napi_value NapiNativeCallbackInfo::GetNewTargetRef()
{
    return reinterpret_cast<napi_value>(&stackArgs_[NEW_TARGET_INDEX]);
}
NapiFunctionInfo* NapiNativeCallbackInfo::GetData()
{
    return reinterpret_cast<NapiFunctionInfo *>(reinterpret_cast<panda::JsiRuntimeCallInfo *>(this)->GetData());
}

#endif // FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_VALUE_INL_H
