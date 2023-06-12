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

#include "ark_native_buffer.h"
#include "ark_native_array_buffer.h"
#include <memory>
#include <securec.h>

#include "utils/log.h"

using panda::BufferRef;
using panda::JSValueRef;
// The maximum length for NativaBuffer, default is 2MiB.
static constexpr size_t K_MAX_BYTE_LENGTH = 2097152;

ArkNativeBuffer::ArkNativeBuffer(ArkNativeEngine* engine, Local<JSValueRef> value)
    : ArkNativeObject(engine, value)
{
}

ArkNativeBuffer::ArkNativeBuffer(ArkNativeEngine* engine, uint8_t** value, size_t length)
    : ArkNativeBuffer(engine, JSValueRef::Undefined(engine->GetEcmaVm()))
{
    if (!value) {
        return;
    }

    if (length > K_MAX_BYTE_LENGTH) {
        *value = nullptr;
        return;
    }

    auto vm = engine->GetEcmaVm();
    LocalScope scope(vm);
    value_ = Global<BufferRef>(vm, BufferRef::New(vm, length));
    Global<BufferRef> obj = value_;
    *value = reinterpret_cast<uint8_t*>(obj->GetBuffer());
}

ArkNativeBuffer::ArkNativeBuffer(ArkNativeEngine* engine, uint8_t** value, size_t length, const uint8_t* data)
    : ArkNativeBuffer(engine, JSValueRef::Undefined(engine->GetEcmaVm()))
{
    if (!value) {
        return;
    }

    if (length > K_MAX_BYTE_LENGTH) {
        *value = nullptr;
        return;
    }

    auto vm = engine->GetEcmaVm();
    LocalScope scope(vm);
    value_ = Global<BufferRef>(vm, BufferRef::New(vm, length));
    Global<BufferRef> obj = value_;
    *value = reinterpret_cast<uint8_t*>(obj->GetBuffer());
    if (memcpy_s(*value, length, data, length) != EOK) {
        HILOG_ERROR("memcpy_s failed");
    }
}

ArkNativeBuffer::ArkNativeBuffer(ArkNativeEngine* engine,
                                 uint8_t* value,
                                 size_t length,
                                 NativeFinalize cb,
                                 void* hint)
    : ArkNativeBuffer(engine, JSValueRef::Undefined(engine->GetEcmaVm()))
{
    if (!value || length > K_MAX_BYTE_LENGTH) {
        return;
    }

    auto vm = engine->GetEcmaVm();
    LocalScope scope(vm);

    std::unique_ptr<NativeObjectInfo> cbinfo(NativeObjectInfo::CreateNewInstance());
    if (!cbinfo) {
        HILOG_ERROR("cbinfo is nullptr");
        return;
    }
    cbinfo->engine = engine_;
    cbinfo->callback = cb;
    cbinfo->hint = hint;

    Local<BufferRef> object = BufferRef::New(vm, value, length,
        [](void* data, void* info) {
            auto externalInfo = reinterpret_cast<NativeObjectInfo*>(info);
            auto engine = externalInfo->engine;
            auto callback = externalInfo->callback;
            auto hint = externalInfo->hint;
            if (callback != nullptr) {
                callback(engine, data, hint);
            }
            delete externalInfo;
        },
        cbinfo.get());

    value_ = Global<BufferRef>(vm, object);
    cbinfo.release();
}

ArkNativeBuffer::~ArkNativeBuffer() {}

void* ArkNativeBuffer::GetInterface(int interfaceId)
{
    return (NativeBuffer::INTERFACE_ID == interfaceId) ? (NativeBuffer*)this
                                                       : ArkNativeObject::GetInterface(interfaceId);
}

void* ArkNativeBuffer::GetBuffer()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<BufferRef> v = value_;
    return v->GetBuffer();
}

size_t ArkNativeBuffer::GetLength()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<BufferRef> v = value_;
    return v->ByteLength(vm);
}

bool ArkNativeBuffer::IsBuffer()
{
    return true;
}

bool ArkNativeBuffer::IsArrayBuffer()
{
    return false;
}