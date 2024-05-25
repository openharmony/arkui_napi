/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "ark_interop_helper.h"
#include "ark_interop_napi.h"
#include "gtest/gtest.h"

using namespace testing;
using namespace testing::ext;

namespace {
ARKTS_Engine engine_ = nullptr;
}

class ArkInteropHelperTest : public testing::Test {};

HWTEST_F(ArkInteropHelperTest, ArkValue001, TestSize.Level1)
{
    napi_env env = (napi_env)ARKTS_GetNAPIEnv(engine_);
    auto napi_v = ArkTsValuetoNapiValue(env, ARKTS_CreateNull());
    NapiValueToArkTsValue(napi_v);
    IsStageMode(env, napi_v);
    SetGlobalNapiEnv(env);
    EXPECT_EQ(GetGlobalNapiEnv(), env);
    GetContextStageMode(env, napi_v);
    int32_t id = ARKTS_GetCurrentContainerId();
    auto scope = ARKTS_CreateContainerScope(id);
    ARKTS_EnterContainerScope(scope);
    ARKTS_ExitContainerScope(scope);
    ARKTS_DestroyContainerScope(scope);
}

HWTEST_F(ArkInteropHelperTest, ArkValue002, TestSize.Level1)
{
    ArkTsValuetoNapiValue(nullptr, ARKTS_CreateNull());
}

HWTEST_F(ArkInteropHelperTest, ArkValue003, TestSize.Level1)
{
    napi_env env = (napi_env)ARKTS_GetNAPIEnv(engine_);
    napi_value object = nullptr;
    napi_create_object(env, &object);
    GetContextStageMode(env, object);
}

int main(int argc, char** argv)
{
    engine_ = ARKTS_CreateEngine();
    std::cout << "main in" << std::endl;
    testing::GTEST_FLAG(output) = "xml:./";
    testing::InitGoogleTest(&argc, argv);
    int ret = testing::UnitTest::GetInstance()->Run();
    std::cout << "main out" << std::endl;
    ARKTS_DestroyEngine(engine_);
    engine_ = nullptr;
    return ret;
}
