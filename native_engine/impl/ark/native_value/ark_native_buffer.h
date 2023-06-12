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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_NATIVE_VALUE_ARK_NATIVE_BUFFER_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_NATIVE_VALUE_ARK_NATIVE_BUFFER_H

#include "ark_native_object.h"

class ArkNativeBuffer : public ArkNativeObject, public NativeBuffer {
public:
    ArkNativeBuffer(ArkNativeEngine* engine, Local<JSValueRef> value);
    ArkNativeBuffer(ArkNativeEngine* engine, uint8_t** value, size_t length);
    ArkNativeBuffer(ArkNativeEngine* engine, uint8_t** value, size_t length, const uint8_t* data);
    ArkNativeBuffer(ArkNativeEngine* engine, uint8_t* data, size_t length, NativeFinalize cb, void* hint);
    ~ArkNativeBuffer() override;

    void* GetInterface(int interfaceId) override;
    void* GetBuffer() override;
    size_t GetLength() override;
    bool IsBuffer() override;
    bool IsArrayBuffer() override;
};
#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_NATIVE_VALUE_ARK_NATIVE_BUFFER_H */