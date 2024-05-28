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

#include "ark_interop_napi.h"
#include "ark_interop_internal.h"
#include "ark_interop_log.h"

using namespace panda;
using namespace panda::ecmascript;

ARKTS_INLINE bool ARKTSInner_IsJSKey(ARKTS_Value value)
{
    auto tag = BIT_CAST(value, JSValueRef);
    if (!tag.IsHeapObject()) {
        return false;
    }
    tag = *P_CAST(value, JSValueRef*);
    return tag.IsString() || tag.IsSymbol();
}

ARKTS_Value ARKTS_CreateObject(ARKTS_Env env)
{
    ARKTS_ASSERT_P(env, "env is null");

    auto vm = P_CAST(env, EcmaVM*);
    auto result = ObjectRef::New(vm);

    return BIT_CAST(result, ARKTS_Value);
}

bool ARKTS_IsHeapObject(ARKTS_Value value)
{
    auto v = BIT_CAST(value, JSValueRef);
    return v.IsHeapObject();
}

bool ARKTS_IsObject(ARKTS_Value value)
{
    auto v = BIT_CAST(value, JSValueRef);
    if (!v.IsHeapObject()) {
        return false;
    }
    v = *P_CAST(value, JSValueRef*);
    return v.IsObject();
}

bool ARKTS_HasOwnProperty(ARKTS_Env env, ARKTS_Value jobj, ARKTS_Value jkey)
{
    ARKTS_ASSERT_F(env, "env is null");
    ARKTS_ASSERT_F(ARKTS_IsHeapObject(jobj), "object is not heap object");
    ARKTS_ASSERT_F(ARKTSInner_IsJSKey(jkey), "key is not string or symbol");

    auto vm = P_CAST(env, EcmaVM*);
    auto object = BIT_CAST(jobj, Local<ObjectRef>);
    auto key = BIT_CAST(jkey, Local<JSValueRef>);
    return object->Has(vm, key);
}

ARKTS_Value ARKTS_EnumOwnProperties(ARKTS_Env env, ARKTS_Value jobj)
{
    ARKTS_ASSERT_P(env, "env is null");
    ARKTS_ASSERT_P(ARKTS_IsHeapObject(jobj), "object is not heap object");

    auto vm = P_CAST(env, EcmaVM*);
    auto object = BIT_CAST(jobj, Local<ObjectRef>);
    auto result = object->GetOwnEnumerablePropertyNames(vm);

    return BIT_CAST(result, ARKTS_Value);
}

void ARKTS_DefineOwnProperty(ARKTS_Env env, ARKTS_Value jobj, ARKTS_Value jkey, ARKTS_Value jvalue,
    ARKTS_PropertyFlag attrs)
{
    ARKTS_ASSERT_V(env, "env is null");
    ARKTS_ASSERT_V(ARKTS_IsHeapObject(jobj), "object is not heap object");
    ARKTS_ASSERT_V(ARKTSInner_IsJSKey(jkey), "key is not string or symbol");

    auto vm = P_CAST(env, EcmaVM*);
    auto object = BIT_CAST(jobj, Local<ObjectRef>);
    auto key = BIT_CAST(jkey, Local<JSValueRef>);
    auto value = ARKTS_ToHandle<JSValueRef>(jvalue);

    PropertyAttribute attribute(value, attrs & N_WRITABLE, attrs & N_ENUMERABLE, attrs & N_CONFIGURABLE);
    object->DefineProperty(vm, key, attribute);
}

void ARKTS_DefineAccessors(ARKTS_Env env, ARKTS_Value jobj, ARKTS_Value jkey, ARKTS_Accessor accessor)
{
    ARKTS_ASSERT_V(env, "env is null");
    ARKTS_ASSERT_V(ARKTS_IsHeapObject(jobj), "object is not heap object");
    ARKTS_ASSERT_V(ARKTSInner_IsJSKey(jkey), "key is not string or symbol");

    auto undefined = ARKTS_CreateUndefined();

    ARKTS_ASSERT_V(accessor.setter == undefined || ARKTS_IsCallable(accessor.setter), "setter not callable");
    ARKTS_ASSERT_V(accessor.getter == undefined || ARKTS_IsCallable(accessor.getter), "getter not callable");
    ARKTS_ASSERT_V(accessor.getter != undefined || accessor.setter != undefined, "getter and setter is undefined");

    auto vm = P_CAST(env, EcmaVM*);
    auto object = BIT_CAST(jobj, Local<ObjectRef>);
    auto key = BIT_CAST(jkey, Local<JSValueRef>);
    auto handledUndef = ARKTS_ToHandle<JSValueRef>(undefined);
    auto handledGetter = accessor.getter != undefined ? BIT_CAST(accessor.getter, Local<JSValueRef>) : handledUndef;
    auto handledSetter = accessor.setter != undefined ? BIT_CAST(accessor.setter, Local<JSValueRef>) : handledUndef;

    PropertyAttribute attribute(handledUndef,
        accessor.attrs & N_WRITABLE,
        accessor.attrs & N_ENUMERABLE,
        accessor.attrs & N_CONFIGURABLE);
    object->SetAccessorProperty(vm, key, handledGetter, handledSetter, attribute);
}

void ARKTS_SetProperty(ARKTS_Env env, ARKTS_Value jobj, ARKTS_Value jkey, ARKTS_Value jvalue)
{
    ARKTS_ASSERT_V(env, "env is null");
    ARKTS_ASSERT_V(ARKTS_IsHeapObject(jobj), "object is not heap object");
    ARKTS_ASSERT_V(ARKTSInner_IsJSKey(jkey), "key is not string or symbol");

    auto vm = P_CAST(env, EcmaVM*);
    auto object = BIT_CAST(jobj, Local<ObjectRef>);
    auto key = BIT_CAST(jkey, Local<JSValueRef>);
    auto value = ARKTS_ToHandle<JSValueRef>(jvalue);

    object->Set(vm, key, value);
}

ARKTS_Value ARKTS_GetProperty(ARKTS_Env env, ARKTS_Value jobj, ARKTS_Value jkey)
{
    ARKTS_ASSERT_P(env, "env is null");
    ARKTS_ASSERT_P(ARKTS_IsHeapObject(jobj), "object is not heap object");
    ARKTS_ASSERT_P(ARKTSInner_IsJSKey(jkey), "key is not string or symbol");

    auto vm = P_CAST(env, EcmaVM*);
    auto object = BIT_CAST(jobj, Local<ObjectRef>);
    auto key = BIT_CAST(jkey, Local<JSValueRef>);

    auto result = object->Get(vm, key);
    return ARKTS_FromHandle(result);
}