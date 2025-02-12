/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "module_checker_delegate.h"
#include "ark_native_engine.h"
#include "native_engine/native_utils.h"
#include "securec.h"
#include "test.h"
#include "test_common.h"
#include "utils/log.h"

using panda::ArrayRef;
using panda::JSValueRef;
using panda::Local;
using panda::ObjectRef;

class ArkApiAllowlistTest : public NativeEngineTest {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "ArkApiAllowlistTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "ArkApiAllowlistTest TearDownTestCase";
    }

    void SetUp() override {}
    void TearDown() override {}
};


napi_value TestFunction(napi_env env, napi_callback_info info)
{
    HILOG_INFO("this is TestFunction");
    return nullptr;
}

void CheckPropertyNames(Local<ObjectRef> &obj,
                        const EcmaVM* vm,
                        uint32_t &filter,
                        std::unordered_map<std::string, bool> &keyToCond)
{
    Local<ArrayRef> propertyNamesArrayVal = obj->GetAllPropertyNames(vm, filter);
    for (uint32_t i = 0; i < propertyNamesArrayVal->Length(vm); ++i) {
        Local<JSValueRef> nameValue = ArrayRef::GetValueAt(vm, propertyNamesArrayVal, i);
        std::string keyname = nameValue->ToString(vm)->ToString(vm);
        HILOG_INFO("exportCopy->system->function:%{public}s", keyname.c_str());
        if (keyToCond.find(keyname) != keyToCond.end()) {
            keyToCond[keyname] = true;
        }
    }
}

bool Test001(const EcmaVM* vm, Local<ObjectRef> &exportCopy, uint32_t &filter)
{
    bool condition1 = false;
    bool condition2 = false;
    bool condition3 = true;
    Local<ArrayRef> propertyNamesArrayVal = exportCopy->GetAllPropertyNames(vm, filter);
    for (uint32_t i = 0; i < propertyNamesArrayVal->Length(vm); ++i) {
        Local<JSValueRef> nameValue = ArrayRef::GetValueAt(vm, propertyNamesArrayVal, i);
        std::string keyname = nameValue->ToString(vm)->ToString(vm);
        HILOG_INFO("exportCopy->function:%{public}s", keyname.c_str());
        if (keyname == "System") {
            Local<ObjectRef> obj = exportCopy->Get(vm, nameValue);
            std::unordered_map<std::string, bool> keyToCond;
            keyToCond["getSystemLanguage"] = condition1;
            keyToCond["is24HourClock"] = condition2;
            CheckPropertyNames(obj, vm, filter, keyToCond);
            condition1 = keyToCond["getSystemLanguage"];
            condition2 = keyToCond["is24HourClock"];
        } else {
            condition3 = false;
        }
    }
    return condition1 && condition2 && condition3;
}

bool Test002(const EcmaVM* vm, Local<ObjectRef> &exportCopy, uint32_t &filter)
{
    bool condition1 = false;
    bool condition2 = false;
    bool condition3 = true;
    bool condition4 = false;
    bool condition5 = false;
    bool condition6 = false;
    bool condition7 = false;
    Local<ArrayRef> propertyNamesArrayVal = exportCopy->GetAllPropertyNames(vm, filter);
    for (uint32_t i = 0; i < propertyNamesArrayVal->Length(vm); ++i) {
        Local<JSValueRef> nameValue = ArrayRef::GetValueAt(vm, propertyNamesArrayVal, i);
        std::string keyname = nameValue->ToString(vm)->ToString(vm);
        HILOG_INFO("exportCopy->function:%{public}s", keyname.c_str());
        if (keyname == "Locale") {
            condition1 = true;
            Local<ObjectRef> obj = exportCopy->Get(vm, nameValue);
            std::unordered_map<std::string, bool> keyToCond;
            keyToCond["function001"] = condition4;
            keyToCond["systemOtherFunction"] = condition5;
            CheckPropertyNames(obj, vm, filter, keyToCond);
            condition4 = keyToCond["function001"];
            condition5 = keyToCond["systemOtherFunction"];
        } else if (keyname == "DateTimeFormat") {
            condition2 = true;
            Local<ObjectRef> obj = exportCopy->Get(vm, nameValue);
            std::unordered_map<std::string, bool> keyToCond;
            keyToCond["function001"] = condition6;
            keyToCond["systemOtherFunction"] = condition7;
            CheckPropertyNames(obj, vm, filter, keyToCond);
            condition6 = keyToCond["function001"];
            condition7 = keyToCond["systemOtherFunction"];
        } else if (nameValue->Typeof(vm)->ToString(vm) == "object") {
            condition3 = false;
        }
    }
    return condition1 && condition2 && condition3 && condition4 && condition5 && condition6 && condition7;
}
