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

#include "native_sendable.h"

#include "ark_native_engine.h"
#include "native_engine/native_utils.h"

using panda::ObjectRef;
using panda::StringRef;
using panda::SymbolRef;
using PropertyAttribute = panda::PropertyAttribute;

FunctionRef::SendablePropertiesInfos NativeSendable::CreateSendablePropertiesInfos(
    napi_env env,
    const NapiPropertyDescriptor* properties,
    size_t propertiesLength)
{
    FunctionRef::SendablePropertiesInfos infos;

    for (size_t i = 0; i < propertiesLength; ++i) {
        if (properties[i].attributes & NATIVE_STATIC) {
            InitSendablePropertiesInfo(env, infos.staticPropertiesInfo, properties[i]);
        } else if (properties[i].attributes & NATIVE_INSTANCE) {
            InitSendablePropertiesInfo(env, infos.instancePropertiesInfo, properties[i]);
        } else if (properties[i].attributes & NATIVE_INSTANCE_OBJECT) {
            InitSendablePropertiesInfo(env, infos.instancePropertiesInfo, properties[i],
                                       FunctionRef::SendableType::OBJECT);
        } else if (properties[i].attributes & NATIVE_INSTANCE_GENERIC) {
            InitSendablePropertiesInfo(env, infos.instancePropertiesInfo, properties[i],
                                       FunctionRef::SendableType::GENERIC);
        } else {
            InitSendablePropertiesInfo(env, infos.nonStaticPropertiesInfo, properties[i]);
        }
    }

    return infos;
}

void NativeSendable::InitSendablePropertiesInfo(napi_env env,
                                                FunctionRef::SendablePropertiesInfo& info,
                                                NapiPropertyDescriptor propertyDescriptor,
                                                FunctionRef::SendableType type)
{
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();

    bool writable = (propertyDescriptor.attributes & NATIVE_WRITABLE) != 0;
    bool enumable = (propertyDescriptor.attributes & NATIVE_ENUMERABLE) != 0;
    bool configable = (propertyDescriptor.attributes & NATIVE_CONFIGURABLE) != 0;

    Local<StringRef> key;
    if (propertyDescriptor.utf8name == nullptr) {
        key = LocalValueFromJsValue(propertyDescriptor.name);
    } else {
        key = StringRef::NewFromUtf8(vm, propertyDescriptor.utf8name);
    }
    info.keys.push_back(key);

    if (propertyDescriptor.getter != nullptr || propertyDescriptor.setter != nullptr) {
        Local<JSValueRef> localGetter = JSValueRef::Undefined(vm);
        Local<JSValueRef> localSetter = JSValueRef::Undefined(vm);

        if (propertyDescriptor.getter != nullptr) {
            localGetter =
                NapiNativeCreateSendableFunction(env, "getter", propertyDescriptor.getter, propertyDescriptor.data);
        }
        if (propertyDescriptor.setter != nullptr) {
            localSetter =
                NapiNativeCreateSendableFunction(env, "setter", propertyDescriptor.setter, propertyDescriptor.data);
        }

        Local<JSValueRef> val = ObjectRef::CreateSendableAccessorData(vm, localGetter, localSetter);
        info.types.push_back(FunctionRef::SendableType::OBJECT);
        info.attributes.push_back(PropertyAttribute(val, false, enumable, configable));
    } else if (propertyDescriptor.method != nullptr) {
        std::string fullName;
        if (propertyDescriptor.utf8name != nullptr) {
            fullName += propertyDescriptor.utf8name;
        } else {
            fullName += key->IsString(vm) ? Local<StringRef>(key)->ToString(vm)
                                          : Local<SymbolRef>(key)->GetDescription(vm)->ToString(vm);
        }

        Local<JSValueRef> func =
            NapiNativeCreateSendableFunction(env, fullName.c_str(), propertyDescriptor.method, propertyDescriptor.data);
        info.types.push_back(FunctionRef::SendableType::OBJECT);
        info.attributes.push_back(PropertyAttribute(func, writable, enumable, configable));
    } else {
        Local<JSValueRef> val = LocalValueFromJsValue(propertyDescriptor.value);
        info.types.push_back(type);
        info.attributes.push_back(PropertyAttribute(val, writable, enumable, configable));
    }
}

Local<JSValueRef> NativeSendable::NapiNativeCreateSendableFunction(napi_env env,
                                                                   const char* name,
                                                                   NapiNativeCallback cb,
                                                                   void* value)
{
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());
    NapiFunctionInfo* funcInfo = NapiFunctionInfo::CreateNewInstance();
    if (funcInfo == nullptr) {
        HILOG_ERROR("funcInfo is nullptr");
        return JSValueRef::Undefined(vm);
    }
    funcInfo->callback = cb;
    funcInfo->data = value;
    funcInfo->isSendable = true;

    Local<FunctionRef> fn = FunctionRef::NewSendable(
        vm, ArkNativeFunctionCallBack,
        [](void* env, void* externalPointer, void* data) {
            auto info = reinterpret_cast<NapiFunctionInfo*>(data);
            if (info != nullptr) {
                delete info;
            }
        },
        reinterpret_cast<void*>(funcInfo), true);
    return fn;
}

void NativeSendable::NapiDefineSendabledProperty(napi_env env,
                                                 Local<ObjectRef>& obj,
                                                 NapiPropertyDescriptor& propertyDescriptor,
                                                 Local<JSValueRef>& propertyName,
                                                 bool& result)
{
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();

    bool enumable = (propertyDescriptor.attributes & NATIVE_ENUMERABLE) != 0;
    bool configable = (propertyDescriptor.attributes & NATIVE_CONFIGURABLE) != 0;

    if (propertyDescriptor.getter != nullptr || propertyDescriptor.setter != nullptr) {
        Local<JSValueRef> localGetter = JSValueRef::Undefined(vm);
        Local<JSValueRef> localSetter = JSValueRef::Undefined(vm);

        if (propertyDescriptor.getter != nullptr) {
            localGetter =
                NapiNativeCreateSendableFunction(env, "getter", propertyDescriptor.getter, propertyDescriptor.data);
        }
        if (propertyDescriptor.setter != nullptr) {
            localSetter =
                NapiNativeCreateSendableFunction(env, "setter", propertyDescriptor.setter, propertyDescriptor.data);
        }

        PropertyAttribute attr(JSValueRef::Undefined(vm), false, enumable, configable);
        // note(lzl): SetSendableAccessorProperty?
        result = obj->SetAccessorProperty(vm, propertyName, localGetter, localSetter, attr);
    } else if (propertyDescriptor.method != nullptr) {
        std::string fullName;
        if (propertyDescriptor.utf8name != nullptr) {
            fullName += propertyDescriptor.utf8name;
        } else {
            fullName += propertyName->IsString(vm) ? Local<StringRef>(propertyName)->ToString(vm)
                                                   : Local<SymbolRef>(propertyName)->GetDescription(vm)->ToString(vm);
        }

        Local<JSValueRef> func =
            NapiNativeCreateSendableFunction(env, fullName.c_str(), propertyDescriptor.method, propertyDescriptor.data);
        result = obj->Set(vm, propertyName, func);
    } else {
        Local<JSValueRef> val = LocalValueFromJsValue(propertyDescriptor.value);
        result = obj->Set(vm, propertyName, val);
    }
}
