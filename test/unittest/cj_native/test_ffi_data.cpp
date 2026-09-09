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

#include "gtest/gtest.h"
#include "ffi_remote_data.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::FFI;

namespace {
class FfiDataTest : public testing::Test {
public:
    static void RunLocalTest()
    {
        TestFfiManager();
        TestIsType();
    }
private:
    static void TestFfiManager();
    static void TestIsType();
};

class RemoteDataSample : public OHOS::FFI::RemoteData {
    DECL_TYPE(RemoteDataSample, OHOS::FFI::RemoteData)
public:
    RemoteDataSample(int id): RemoteData(id) {
        num = id;
    }
    int num = 0;
};

class FfiContainRemoteData : public OHOS::FFI::FFIData {
    DECL_TYPE(FfiContainRemoteData, OHOS::FFI::FFIData)
public:
    FfiContainRemoteData() = default;
    OHOS::sptr<RemoteDataSample> remoteData{};
};

// --- IsType / DynamicCast 类型安全测试专用类层次 ---
class SampleBase : public OHOS::FFI::FFIData {
    DECL_TYPE(SampleBase, OHOS::FFI::FFIData)
public:
    SampleBase() = default;
};

class SampleDerived : public SampleBase {
    DECL_TYPE(SampleDerived, SampleBase)
public:
    SampleDerived() = default;
};

class SampleUnrelated : public OHOS::FFI::FFIData {
    DECL_TYPE(SampleUnrelated, OHOS::FFI::FFIData)
public:
    SampleUnrelated() = default;
};

void FfiDataTest::TestFfiManager()
{
    auto mgr = FFIDataManager::GetInstance();
    EXPECT_TRUE(mgr);
    auto ffiData = FFIData::Create<FfiContainRemoteData>();
    EXPECT_TRUE(ffiData);
    auto id = ffiData->GetID();
    int num = 100;
    ffiData->remoteData = RemoteData::Create<RemoteDataSample>(num);
    mgr->StoreFFIData(ffiData);
    OHOS::sptr<FfiContainRemoteData> temp = FFIData::GetData<FfiContainRemoteData>(id);
    EXPECT_TRUE(temp);
    EXPECT_TRUE(temp->remoteData);
    if (temp->remoteData->num == num) {
        EXPECT_TRUE(true);
    } else {
        EXPECT_TRUE(false);
    }
    mgr->RemoveFFIData(id);
}

void FfiDataTest::TestIsType()
{
    // 完全匹配：相同类型应是 true
    auto base = FFIData::Create<SampleBase>();
    EXPECT_TRUE(base);
    EXPECT_TRUE(base->GetRuntimeType()->IsType(base->GetRuntimeType()));

    // 子类 → 父类：派生对象应是基类类型（true）
    auto derived = FFIData::Create<SampleDerived>();
    EXPECT_TRUE(derived);
    EXPECT_TRUE(derived->GetRuntimeType()->IsType(base->GetRuntimeType()));

    // 父类 → 子类：基类对象不可能是派生类型（false）
    EXPECT_FALSE(base->GetRuntimeType()->IsType(derived->GetRuntimeType()));

    // 无关类型：应 false
    auto unrelated = FFIData::Create<SampleUnrelated>();
    EXPECT_TRUE(unrelated);
    EXPECT_FALSE(base->GetRuntimeType()->IsType(unrelated->GetRuntimeType()));
    EXPECT_FALSE(unrelated->GetRuntimeType()->IsType(base->GetRuntimeType()));

    // DynamicCast 安全性：子类可转基类，基类不可转子类，无关不可互转
    EXPECT_NE(derived->template DynamicCast<SampleBase>(), nullptr);
    EXPECT_EQ(base->template DynamicCast<SampleDerived>(), nullptr);
    EXPECT_EQ(base->template DynamicCast<SampleUnrelated>(), nullptr);
    EXPECT_EQ(unrelated->template DynamicCast<SampleBase>(), nullptr);

    // null target 应返回 false
    EXPECT_FALSE(base->GetRuntimeType()->IsType(nullptr));

    // 名字比较语义：同名即视为同一类型标识——这正是缺陷 B/C/D 要求 DECL_TYPE 名字唯一的原因
    auto dup1 = OHOS::FFI::RuntimeType::Create("DupName");
    auto dup2 = OHOS::FFI::RuntimeType::Create("DupName");
    EXPECT_TRUE(dup1.IsType(&dup2));
    EXPECT_TRUE(dup2.IsType(&dup1));

    FFIData::Release(base->GetID());
    FFIData::Release(derived->GetID());
    FFIData::Release(unrelated->GetID());
}

TEST_F(FfiDataTest, Types)
{
    RunLocalTest();
}
}