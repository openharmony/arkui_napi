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
#include "form_module_checker.h"
#include "ark_native_engine.h"
#include "napi/native_engine/native_utils.h"
#include "securec.h"
#include "test.h"
#include "test_common.h"
#include "utils/log.h"


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


napi_value TestFunction01(napi_env env, napi_callback_info info)
{
    HILOG_INFO("this is TestFunction01");
    return nullptr;
}


/**
 * @tc.name: CopyPropertyApiFilterTest002
 * @tc.desc: Test CopyPropertyApiFilter Functional Logic.
 * @tc.type: FUNC
 */
HWTEST_F(ArkApiAllowlistTest, CopyPropertyApiFilterTest001, testing::ext::TestSize.Level1)
{
    ArkNativeEngine* arkNativeEngine = static_cast<ArkNativeEngine*>(engine_);
    const EcmaVM* vm = arkNativeEngine->GetEcmaVm();
    napi_env env = reinterpret_cast<napi_env>(arkNativeEngine);
    std::unique_ptr<ApiAllowListChecker> apiAllowListFilter = nullptr;
    std::shared_ptr<FormModuleChecker> formChecker = std::make_shared<FormModuleChecker>();
    formChecker->CheckModuleLoadable("i18n", apiAllowListFilter);

    napi_property_descriptor systemProperties[] = {
        DECLARE_NAPI_FUNCTION("getSystemLanguage", TestFunction01),
        DECLARE_NAPI_FUNCTION("is24HourClock", TestFunction01),
        DECLARE_NAPI_FUNCTION("systemOtherFunction", TestFunction01),
    };
    napi_value system;
    napi_create_object(env, &system);
    napi_define_properties(env, system, sizeof(systemProperties) / sizeof(napi_property_descriptor), systemProperties);

    napi_property_descriptor i18nProperties[] = {
        DECLARE_NAPI_FUNCTION("getSystemLanguage", TestFunction01),
        DECLARE_NAPI_FUNCTION("i18nOtherFunction", TestFunction01),
        DECLARE_NAPI_PROPERTY("System", system),
    };
    napi_value i18n;
    napi_create_object(env, &i18n);
    napi_define_properties(env, i18n, sizeof(i18nProperties) / sizeof(napi_property_descriptor), i18nProperties);
    panda::Local<panda::ObjectRef> i18nObj = LocalValueFromJsValue(i18n);

    panda::Local<panda::ObjectRef> exportCopy = panda::ObjectRef::New(vm);

    if (apiAllowListFilter != nullptr) {
        std::string apiPath = "i18n";
        if ((*apiAllowListFilter)(apiPath)) {
            ArkNativeEngine::CopyPropertyApiFilter(apiAllowListFilter, vm, i18nObj, exportCopy, apiPath);
        }
    }

    uint32_t filter = NATIVE_DEFAULT;
    panda::Local<panda::ArrayRef> propertyNamesArrayVal01 = i18nObj->GetAllPropertyNames(vm, filter);
    for (int i = propertyNamesArrayVal01->Length(vm) - 1; i >= 0; i--) {
        HILOG_INFO("exportObj->function:%{public}s",
            panda::ArrayRef::GetValueAt(vm, propertyNamesArrayVal01, i)->ToString(vm)->ToString().c_str());
    }
    panda::Local<panda::ArrayRef> propertyNamesArrayVal02 = exportCopy->GetAllPropertyNames(vm, filter);
    bool condition1 = false;
    bool condition2 = false;
    bool condition3 = true;
    for (int i = propertyNamesArrayVal02->Length(vm) - 1; i >= 0; i--) {
        panda::Local<panda::JSValueRef> nameValue = panda::ArrayRef::GetValueAt(vm, propertyNamesArrayVal02, i);
        std::string keyname = nameValue->ToString(vm)->ToString();
        HILOG_INFO("exportCopy->function:%{public}s", keyname.c_str());
        if (keyname == "System") {
            panda::Local<panda::ObjectRef> obj = exportCopy->Get(vm, nameValue);
            panda::Local<panda::ArrayRef> propertyNamesArrayVal03 = obj->GetAllPropertyNames(vm, filter);
            for (int j = propertyNamesArrayVal03->Length(vm) - 1; j >= 0; j--) {
                nameValue = panda::ArrayRef::GetValueAt(vm, propertyNamesArrayVal03, j);
                keyname = nameValue->ToString(vm)->ToString();
                HILOG_INFO("exportCopy->system->function:%{public}s", keyname.c_str());
                if (keyname == "getSystemLanguage") {
                    condition1 = true;
                }
                if (keyname == "is24HourClock") {
                    condition2 = true;
                }
            }
        } else {
            condition3 = false;
        }
    }
    ASSERT_EQ(condition1 && condition2 && condition3, true);
}


/**
 * @tc.name: CopyPropertyApiFilterTest001
 * @tc.desc: Test CopyPropertyApiFilter Functional Logic.
 * @tc.type: FUNC
 */
HWTEST_F(ArkApiAllowlistTest, CopyPropertyApiFilterTest002, testing::ext::TestSize.Level1)
{
    ArkNativeEngine* arkNativeEngine = static_cast<ArkNativeEngine*>(engine_);
    const EcmaVM* vm = arkNativeEngine->GetEcmaVm();
    napi_env env = reinterpret_cast<napi_env>(arkNativeEngine);
    std::unique_ptr<ApiAllowListChecker> apiAllowListFilter = nullptr;
    std::shared_ptr<FormModuleChecker> formChecker = std::make_shared<FormModuleChecker>();
    formChecker->CheckModuleLoadable("intl", apiAllowListFilter);

    napi_property_descriptor l2Properties[] = {
        DECLARE_NAPI_FUNCTION("function001", TestFunction01),
        DECLARE_NAPI_FUNCTION("function002", TestFunction01),
        DECLARE_NAPI_FUNCTION("systemOtherFunction", TestFunction01),
    };
    napi_value l2obj;
    napi_create_object(env, &l2obj);
    napi_define_properties(env, l2obj, sizeof(l2Properties) / sizeof(napi_property_descriptor), l2Properties);

    napi_property_descriptor intlProperties[] = {
        DECLARE_NAPI_FUNCTION("getSystemLanguage", TestFunction01),
        DECLARE_NAPI_FUNCTION("i18nOtherFunction", TestFunction01),
        DECLARE_NAPI_PROPERTY("Locale2", l2obj),
        DECLARE_NAPI_PROPERTY("Locale", l2obj),
        DECLARE_NAPI_PROPERTY("DateTimeFormat", l2obj),
    };
    napi_value intl;
    napi_create_object(env, &intl);
    napi_define_properties(env, intl, sizeof(intlProperties) / sizeof(napi_property_descriptor), intlProperties);
    panda::Local<panda::ObjectRef> intlObj = LocalValueFromJsValue(intl);

    panda::Local<panda::ObjectRef> exportCopy = panda::ObjectRef::New(vm);

    if (apiAllowListFilter != nullptr) {
        std::string apiPath = "intl";
        if ((*apiAllowListFilter)(apiPath)) {
            ArkNativeEngine::CopyPropertyApiFilter(apiAllowListFilter, vm, intlObj, exportCopy, apiPath);
        }
    }

    uint32_t filter = NATIVE_DEFAULT;
    panda::Local<panda::ArrayRef> propertyNamesArrayVal01 = intlObj->GetAllPropertyNames(vm, filter);
    for (int i = propertyNamesArrayVal01->Length(vm) - 1; i >= 0; i--) {
        HILOG_INFO("exportObj->function:%{public}s",
            panda::ArrayRef::GetValueAt(vm, propertyNamesArrayVal01, i)->ToString(vm)->ToString().c_str());
    }
    panda::Local<panda::ArrayRef> propertyNamesArrayVal02 = exportCopy->GetAllPropertyNames(vm, filter);
    bool condition1 = false;
    bool condition2 = false;
    bool condition3 = true;
    bool condition4 = false;
    bool condition5 = false;
    bool condition6 = false;
    bool condition7 = false;
    for (int i = propertyNamesArrayVal02->Length(vm) - 1; i >= 0; i--) {
        panda::Local<panda::JSValueRef> nameValue = panda::ArrayRef::GetValueAt(vm, propertyNamesArrayVal02, i);
        std::string keyname = nameValue->ToString(vm)->ToString();
        HILOG_INFO("exportCopy->function:%{public}s", keyname.c_str());
        if (keyname == "Locale") {
            condition1 = true;
            panda::Local<panda::ObjectRef> obj = exportCopy->Get(vm, nameValue);
            panda::Local<panda::ArrayRef> propertyNamesArrayVal03 = obj->GetAllPropertyNames(vm, filter);
            for (int j = propertyNamesArrayVal03->Length(vm) - 1; j >= 0; j--) {
                nameValue = panda::ArrayRef::GetValueAt(vm, propertyNamesArrayVal03, j);
                keyname = nameValue->ToString(vm)->ToString();
                HILOG_INFO("exportCopy->system->function:%{public}s", keyname.c_str());
                if (keyname == "function001") {
                    condition4 = true;
                }
                if (keyname == "systemOtherFunction") {
                    condition5 = true;
                }
            }
        } else if (keyname == "DateTimeFormat") {
            condition2 = true;
            panda::Local<panda::ObjectRef> obj = exportCopy->Get(vm, nameValue);
            panda::Local<panda::ArrayRef> propertyNamesArrayVal03 = obj->GetAllPropertyNames(vm, filter);
            for (int j = propertyNamesArrayVal03->Length(vm) - 1; j >= 0; j--) {
                nameValue = panda::ArrayRef::GetValueAt(vm, propertyNamesArrayVal03, j);
                keyname = nameValue->ToString(vm)->ToString();
                HILOG_INFO("exportCopy->system->function:%{public}s", keyname.c_str());
                if (keyname == "function001") {
                    condition6 = true;
                }
                if (keyname == "systemOtherFunction") {
                    condition7 = true;
                }
            }
        } else if (nameValue->Typeof(vm)->ToString() == "object") {
            condition3 = false;
        }
    }
    ASSERT_EQ(condition1 && condition2 && condition3 && condition4 && condition5 && condition6 && condition7, true);
}
