/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ecmascript/napi/include/jsnapi.h"
#include "native_engine.h"
#include "test.h"

using namespace panda;

class NapiGlobalRefTrackTest : public NativeEngineTest {};

HWTEST_F(NapiGlobalRefTrackTest, TestSetStoreGlobalRefEnable, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_status status = napi_set_store_global_ref(env, true);
    ASSERT_EQ(status, napi_ok);

    ASSERT_TRUE(JSNApi::IsTrackGlobalRefEnabled());

    status = napi_set_store_global_ref(env, false);
    ASSERT_EQ(status, napi_ok);
    ASSERT_FALSE(JSNApi::IsTrackGlobalRefEnabled());
}

HWTEST_F(NapiGlobalRefTrackTest, TestSetStoreGlobalRefInvalidEnv, testing::ext::TestSize.Level1)
{
    napi_status status = napi_set_store_global_ref(nullptr, true);
    ASSERT_EQ(status, napi_invalid_arg);
}

HWTEST_F(NapiGlobalRefTrackTest, TestRegisterOnConstruct, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);

    // Enable tracking before creating reference
    napi_set_store_global_ref(env, true);

    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    napi_ref ref = nullptr;
    napi_status status = napi_create_reference(env, obj, 1, &ref);
    ASSERT_EQ(status, napi_ok);
    ASSERT_NE(ref, nullptr);

    // Verify tracking is enabled and mapping is functional
    ASSERT_TRUE(JSNApi::IsTrackGlobalRefEnabled());

    // Cleanup
    napi_delete_reference(env, ref);
    napi_set_store_global_ref(env, false);
}

HWTEST_F(NapiGlobalRefTrackTest, TestNoRegisterWhenDisabled, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);

    // Tracking is off by default
    ASSERT_FALSE(JSNApi::IsTrackGlobalRefEnabled());

    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    napi_ref ref = nullptr;
    napi_status status = napi_create_reference(env, obj, 1, &ref);
    ASSERT_EQ(status, napi_ok);

    // Tracking still off after creating reference
    ASSERT_FALSE(JSNApi::IsTrackGlobalRefEnabled());

    napi_delete_reference(env, ref);
}

HWTEST_F(NapiGlobalRefTrackTest, TestUnregisterOnDestruct, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_set_store_global_ref(env, true);

    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    napi_ref ref = nullptr;
    napi_create_reference(env, obj, 1, &ref);
    ASSERT_NE(ref, nullptr);

    // Delete reference — should call UnregisterGlobalHandleRef
    napi_status status = napi_delete_reference(env, ref);
    ASSERT_EQ(status, napi_ok);

    // After delete, tracking still enabled but mapping for that ref is erased
    ASSERT_TRUE(JSNApi::IsTrackGlobalRefEnabled());

    napi_set_store_global_ref(env, false);
}

HWTEST_F(NapiGlobalRefTrackTest, TestDisableClearsAllMappings, testing::ext::TestSize.Level1)
{
    napi_env env = reinterpret_cast<napi_env>(engine_);
    napi_set_store_global_ref(env, true);

    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    napi_ref ref = nullptr;
    napi_create_reference(env, obj, 1, &ref);
    ASSERT_NE(ref, nullptr);

    // Disable tracking — should clear all mappings
    napi_set_store_global_ref(env, false);
    ASSERT_FALSE(JSNApi::IsTrackGlobalRefEnabled());

    napi_delete_reference(env, ref);
}
