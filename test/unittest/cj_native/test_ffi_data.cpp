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
    }
private:
    static void TestFfiManager();
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

TEST_F(FfiDataTest, Types)
{
    RunLocalTest();
}
}