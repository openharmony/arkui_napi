/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_UTILS_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_UTILS_H

#include <cstring>
#include "securec.h"

#include "ecmascript/napi/include/jsnapi.h"
#include "interfaces/inner_api/napi/native_node_api.h"
#include "utils/log.h"

static constexpr uint64_t NAPI_SPECIAL_STATUS = 5;

inline napi_value JsValueFromLocalValue(panda::Local<panda::JSValueRef> local)
{
    return reinterpret_cast<napi_value>(*local);
}

inline panda::Local<panda::JSValueRef> LocalValueFromJsValue(napi_value v)
{
    panda::Local<panda::JSValueRef> local(reinterpret_cast<uintptr_t>(v));
    return local;
}

inline bool NapiStatusValidationCheck(panda::JSValueRef* value)
{
    if ((value != nullptr) && (*(panda::JSTaggedType *)(value) == NAPI_SPECIAL_STATUS)) {
        return false;
    }
    return true;
}

inline bool NapiStatusValidationCheck(panda::Local<panda::JSValueRef> value)
{
    // Since we sink napi heavy logics to ark runtime,
    // we need to check napi status using a special value.
    // Here we use JSTaggedValue::Hole(0x5) as the special value.
    return NapiStatusValidationCheck(*value);
}
#endif  // FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_UTILS_H
