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

#include "worker_manager.h"

#include <mutex>

#include "utils/log.h"
#ifdef OHOS_PLATFORM
#include "parameters.h"
#endif

static constexpr size_t MAX_JS_THREAD_COUNT = 80;
static std::mutex g_jsThreadCountMutex;
static uint32_t g_jsThreadCount = 0;

static constexpr size_t MAX_ARK_RUNTIME_COUNT = 64;
static std::mutex g_arkRuntimeCountMutex;
static uint32_t g_arkRuntimeCount = 0;

static constexpr size_t MAX_THREADWORKER_COUNT = 64;
static std::mutex g_threadWorkerCountMutex;
static uint32_t g_threadWorkerCount = 0;

static constexpr size_t MAX_LIMITWORKER_COUNT = 16;
static std::mutex g_limitWorkerCountMutex;
static uint32_t g_limitWorkerCount = 0;

static constexpr size_t MAX_OLDWORKER_COUNT = 8;
static std::mutex g_oldWorkerCountMutex;
static uint32_t g_oldWorkerCount = 0;

bool WorkerManager::IncrementArkRuntimeCount()
{
    bool success = IncrementJSThreadCount();
    if (!success) {
        HILOG_ERROR("reach max JSThread count limit");
        return false;
    }

    std::lock_guard<std::mutex> lock(g_arkRuntimeCountMutex);
    if (g_arkRuntimeCount >= MAX_ARK_RUNTIME_COUNT) {
        HILOG_ERROR("reach MAX_ARK_RUNTIME_COUNT");
        DecrementJSThreadCount();
        return false;
    } else {
        g_arkRuntimeCount++;
        return true;
    }
}

void WorkerManager::DecrementArkRuntimeCount()
{
    std::lock_guard<std::mutex> lock(g_arkRuntimeCountMutex);
    if (g_arkRuntimeCount > 0) {
        g_arkRuntimeCount--;
    } else {
        HILOG_ERROR("DecrementArkRuntimeCount, already is 0");
    }
    DecrementJSThreadCount();
}

bool WorkerManager::IncrementWorkerCount(WorkerType type)
{
    if (type == THREAD_WORKER) {
        return IncrementThreadWorkerCount();
    } else if (type == LIMITED_WORKER) {
        return IncrementLimitWorkerCount();
    } else {
        return IncrementOldWorkerCount();
    }
}

void WorkerManager::DecrementWorkerCount(WorkerType type)
{
    if (type == THREAD_WORKER) {
        DecrementThreadWorkerCount();
    } else if (type == LIMITED_WORKER) {
        DecrementLimitWorkerCount();
    } else {
        DecrementOldWorkerCount();
    }
}

bool WorkerManager::IncrementJSThreadCount()
{
    std::lock_guard<std::mutex> lock(g_jsThreadCountMutex);
    if (g_jsThreadCount >= MAX_JS_THREAD_COUNT) {
        HILOG_ERROR("reach MAX_JS_THREAD_COUNT");
        return false;
    } else {
        g_jsThreadCount++;
        return true;
    }
}

void WorkerManager::DecrementJSThreadCount()
{
    std::lock_guard<std::mutex> lock(g_jsThreadCountMutex);
    if (g_jsThreadCount > 0) {
        g_jsThreadCount--;
    } else {
        HILOG_ERROR("DecrementJSThreadCount, already is 0");
    }
}

bool WorkerManager::IncrementThreadWorkerCount()
{
    bool success = IncrementJSThreadCount();
    if (!success) {
        HILOG_ERROR("reach max JSThread count limit");
        return false;
    }

    std::lock_guard<std::mutex> lock(g_threadWorkerCountMutex);
    uint32_t maxWorkers = MAX_THREADWORKER_COUNT;
#if defined(OHOS_PLATFORM)
    maxWorkers = OHOS::system::GetIntParameter<int>("persist.commonlibrary.maxworkers", maxWorkers);
#endif
    if (g_threadWorkerCount >= maxWorkers) {
        HILOG_ERROR("reach MAX_THREADWORKER_COUNT");
        DecrementJSThreadCount();
        return false;
    } else {
        g_threadWorkerCount++;
        return true;
    }
}

void WorkerManager::DecrementThreadWorkerCount()
{
    std::lock_guard<std::mutex> lock(g_threadWorkerCountMutex);
    if (g_threadWorkerCount > 0) {
        g_threadWorkerCount--;
    } else {
        HILOG_ERROR("DecrementThreadWorkerCount, already is 0");
    }
    DecrementJSThreadCount();
}

bool WorkerManager::IncrementLimitWorkerCount()
{
    bool success = IncrementJSThreadCount();
    if (!success) {
        HILOG_ERROR("reach max JSThread count limit");
        return false;
    }

    std::lock_guard<std::mutex> lock(g_limitWorkerCountMutex);
    if (g_limitWorkerCount >= MAX_LIMITWORKER_COUNT) {
        HILOG_ERROR("reach MAX_LIMITWORKER_COUNT");
        DecrementJSThreadCount();
        return false;
    } else {
        g_limitWorkerCount++;
        return true;
    }
}

void WorkerManager::DecrementLimitWorkerCount()
{
    std::lock_guard<std::mutex> lock(g_limitWorkerCountMutex);
    if (g_limitWorkerCount > 0) {
        g_limitWorkerCount--;
    } else {
        HILOG_ERROR("DecrementLimitWorkerCount, already is 0");
    }
    DecrementJSThreadCount();
}

bool WorkerManager::IncrementOldWorkerCount()
{
    bool success = IncrementJSThreadCount();
    if (!success) {
        HILOG_ERROR("reach max JSThread count limit");
        return false;
    }

    std::lock_guard<std::mutex> lock(g_oldWorkerCountMutex);
    uint32_t maxWorkers = MAX_OLDWORKER_COUNT;
#if defined(OHOS_PLATFORM)
    maxWorkers = OHOS::system::GetIntParameter<int>("persist.commonlibrary.maxworkers", maxWorkers);
#endif
    if (g_oldWorkerCount >= maxWorkers) {
        HILOG_ERROR("reach MAX_OLDWORKER_COUNT");
        DecrementJSThreadCount();
        return false;
    } else {
        g_oldWorkerCount++;
        return true;
    }
}

void WorkerManager::DecrementOldWorkerCount()
{
    std::lock_guard<std::mutex> lock(g_oldWorkerCountMutex);
    if (g_oldWorkerCount > 0) {
        g_oldWorkerCount--;
    } else {
        HILOG_ERROR("DecrementOldWorkerCount, already is 0");
    }
    DecrementJSThreadCount();
}
