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

#include "gtest/gtest.h"

#include "native_engine/worker_manager.h"

#define NUM_80 80
#define NUM_64 64
#define NUM_16 16
#define NUM_8 8

class WorkerManagerTest : public testing::Test {
public:
    static void SetUpTestSuite()
    {
    }
    static void TearDownTestSuite()
    {
    }
};

HWTEST_F(WorkerManagerTest, WorkerTest001, testing::ext::TestSize.Level0)
{
    bool ret = false;
    for (int k = 0; k < NUM_64; k++) {
        ret = WorkerManager::IncrementArkRuntimeCount();
        ASSERT_TRUE(ret);
    }
    ret = WorkerManager::IncrementArkRuntimeCount();
    ASSERT_FALSE(ret);

    WorkerManager::DecrementArkRuntimeCount();
    ret = WorkerManager::IncrementArkRuntimeCount();
    ASSERT_TRUE(ret);

    for (int k = 0; k < NUM_64; k++) {
        WorkerManager::DecrementArkRuntimeCount();
    }
}

HWTEST_F(WorkerManagerTest, WorkerTest002, testing::ext::TestSize.Level0)
{
    bool ret = false;
    for (int k = 0; k < NUM_8; k++) {
        ret = WorkerManager::IncrementWorkerCount(WorkerType::OLD_WORKER);
        ASSERT_TRUE(ret);
    }
    ret = WorkerManager::IncrementWorkerCount(WorkerType::OLD_WORKER);
    ASSERT_FALSE(ret);

    WorkerManager::DecrementWorkerCount(WorkerType::OLD_WORKER);
    ret = WorkerManager::IncrementWorkerCount(WorkerType::OLD_WORKER);
    ASSERT_TRUE(ret);

    for (int k = 0; k < NUM_8; k++) {
        WorkerManager::DecrementWorkerCount(WorkerType::OLD_WORKER);
    }
}

HWTEST_F(WorkerManagerTest, WorkerTest003, testing::ext::TestSize.Level0)
{
    bool ret = false;
    for (int k = 0; k < NUM_16; k++) {
        ret = WorkerManager::IncrementWorkerCount(WorkerType::LIMITED_WORKER);
        ASSERT_TRUE(ret);
    }
    ret = WorkerManager::IncrementWorkerCount(WorkerType::LIMITED_WORKER);
    ASSERT_FALSE(ret);

    WorkerManager::DecrementWorkerCount(WorkerType::LIMITED_WORKER);
    ret = WorkerManager::IncrementWorkerCount(WorkerType::LIMITED_WORKER);
    ASSERT_TRUE(ret);

    for (int k = 0; k < NUM_16; k++) {
        WorkerManager::DecrementWorkerCount(WorkerType::LIMITED_WORKER);
    }
}

HWTEST_F(WorkerManagerTest, WorkerTest004, testing::ext::TestSize.Level0)
{
    bool ret = false;
    for (int k = 0; k < NUM_64; k++) {
        ret = WorkerManager::IncrementWorkerCount(WorkerType::THREAD_WORKER);
        ASSERT_TRUE(ret);
    }
    ret = WorkerManager::IncrementWorkerCount(WorkerType::THREAD_WORKER);
    ASSERT_FALSE(ret);

    WorkerManager::DecrementWorkerCount(WorkerType::THREAD_WORKER);
    ret = WorkerManager::IncrementWorkerCount(WorkerType::THREAD_WORKER);
    ASSERT_TRUE(ret);

    for (int k = 0; k < NUM_64; k++) {
        WorkerManager::DecrementWorkerCount(WorkerType::THREAD_WORKER);
    }
}

HWTEST_F(WorkerManagerTest, WorkerTest005, testing::ext::TestSize.Level0)
{
    bool ret = false;
    for (int k = 0; k < NUM_64; k++) {
        ret = WorkerManager::IncrementWorkerCount(WorkerType::THREAD_WORKER);
        ASSERT_TRUE(ret);
    }
    ret = WorkerManager::IncrementWorkerCount(WorkerType::THREAD_WORKER);
    ASSERT_FALSE(ret);

    for (int k = 0; k < NUM_16; k++) {
        ret = WorkerManager::IncrementArkRuntimeCount();
        ASSERT_TRUE(ret);
    }

    ret = WorkerManager::IncrementArkRuntimeCount();
    ASSERT_FALSE(ret);
    ret = WorkerManager::IncrementWorkerCount(WorkerType::THREAD_WORKER);
    ASSERT_FALSE(ret);
    ret = WorkerManager::IncrementWorkerCount(WorkerType::LIMITED_WORKER);
    ASSERT_FALSE(ret);
    ret = WorkerManager::IncrementWorkerCount(WorkerType::OLD_WORKER);
    ASSERT_FALSE(ret);

    for (int k = 0; k < NUM_64; k++) {
        WorkerManager::DecrementWorkerCount(WorkerType::THREAD_WORKER);
    }
    for (int k = 0; k < NUM_16; k++) {
        WorkerManager::DecrementArkRuntimeCount();
    }
}

HWTEST_F(WorkerManagerTest, WorkerTest006, testing::ext::TestSize.Level0)
{
    bool ret = false;
    for (int k = 0; k < NUM_64; k++) {
        ret = WorkerManager::IncrementWorkerCount(WorkerType::THREAD_WORKER);
        ASSERT_TRUE(ret);
    }
    ret = WorkerManager::IncrementWorkerCount(WorkerType::THREAD_WORKER);
    ASSERT_FALSE(ret);

    for (int k = 0; k < NUM_16; k++) {
        ret = WorkerManager::IncrementWorkerCount(WorkerType::LIMITED_WORKER);
        ASSERT_TRUE(ret);
    }

    ret = WorkerManager::IncrementArkRuntimeCount();
    ASSERT_FALSE(ret);
    ret = WorkerManager::IncrementWorkerCount(WorkerType::THREAD_WORKER);
    ASSERT_FALSE(ret);
    ret = WorkerManager::IncrementWorkerCount(WorkerType::LIMITED_WORKER);
    ASSERT_FALSE(ret);
    ret = WorkerManager::IncrementWorkerCount(WorkerType::OLD_WORKER);
    ASSERT_FALSE(ret);
}