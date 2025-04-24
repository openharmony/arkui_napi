#include "gtest/gtest.h"
#include "ffi_remote_data.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::FFI;

namespace {
class FfiDataTest: public testing::Test {
public:
    static void RunLocalTest()
    {
        TestFfiManager();
    }
private:
    static void TestFfiManager();
};

class RemoteDataSample: public OHOS::FFI::RemoteData {
    DECL_TYPE(RemoteDataSample, OHOS::FFI::RemoteData)
public:
    RemoteDataSample(int id): RemoteData(id) {
        num = id;
    }
    int num = 0;
};

class FfiContainRemoteData: public OHOS::FFI::FFIData {
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
    ffiData->remoteData = RemoteData::Create<RemoteDataSample>(100);
    mgr->StoreFFIData(ffiData);
    OHOS::sptr<FfiContainRemoteData> temp = FFIData::GetData<FfiContainRemoteData>(id);
    EXPECT_TRUE(temp);
    EXPECT_TRUE(temp->remoteData);
    if (temp->remoteData->num == 100) {
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