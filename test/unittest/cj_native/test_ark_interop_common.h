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

#ifndef TEST_ARK_INTEROP_COMMON_H
#define TEST_ARK_INTEROP_COMMON_H

#include "ark_interop_external.h"
#include "ark_interop_hitrace.h"
#include "ark_interop_internal.h"
#include "ark_interop_log.h"
#include "ark_interop_napi.h"
#include "gtest/gtest.h"
#include "uv_loop_handler.h"
#include "cj_environment.h"
#include "napi/native_api.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

constexpr uint64_t TEST_STACK_LIMIT = 0x5be506e000;
constexpr int64_t BIGINT_POS_VAL = 0x123'4567'8945;
constexpr int64_t BIGINT_NEG_VAL = -0x123'4567'8945;
constexpr int64_t BIGINT_MAX_VAL = 0x7FFF'FFFF'FFFF'FFFF;
constexpr int64_t BIGINT_MIN_VAL = -0x7FFF'FFFF'FFFF'FFFF;
constexpr int SLEEP_DURATION_MS = 100;

struct ARKTS_ModuleCallbacks {
    ARKTS_Value(*exportModule)(ARKTS_Env env, const char* dllName, ARKTS_Value exports) = nullptr;
    bool(*hasModuleHandle)(const char* dllName) = nullptr;
    void(*throwJSError)(ARKTS_Env env, ARKTS_Value arktsValue) = nullptr;
    void(*throwNativeError)(const char*) = nullptr;
    void(*deleteArrayBufferRawData)(void* buffer, int64_t lambdaId) = nullptr;
    void(*deleteExternal)(int64_t id, ARKTS_Env env) = nullptr;
    ARKTS_Value(*invokerLambda)(ARKTS_CallInfo, int64_t lambdaId) = nullptr;
    void(*deleteLambda)(ARKTS_Env env, int64_t lambdaId) = nullptr;
    void(*invokeAsyncLambda)(ARKTS_Env env, int64_t lambdaId) = nullptr;
    void(*deleteJSContext)(ARKTS_Env env) = nullptr;
};

enum StackInfoOpKind : uint32_t {
    SWITCH_TO_SUB_STACK_INFO = 0,
    SWITCH_TO_MAIN_STACK_INFO,
};

struct StackInfo {
    uint64_t stackLimit;
    uint64_t lastLeaveFrame;
};

enum class MockContextKind { NORMAL, ERROR_CAPTURE };

struct PropCase {
    char k;
    bool writable;
    bool enumerable;
    bool configurable;

    static PropCase cases[];
};

class ArkInteropTest : public testing::Test {
public:
    static void RunLocalTest()
    {
        TestPrime();
        TestNumber();
        TestString();
        TestUtf16String();
        TestObject();
        TestKeyable();
        TestDefineProperty();
        TestDefinePropertyWritable();
        TestDefinePropertyConfigurable();
        TestDefinePropertyConfigurableV2();
        TestArray();
        TestBigintInt64();
        TestBigint16Bytes();
        TestBigint28Bytes();
        TestBigint32Bytes();
        TestPromise();
        TestSymbol();
        TestFunction();
        TestClass();
        TestInstanceOf();
        TestArrayBuffer();
        TestUpdateStackInfo();
    }
private:
    static void TestPrime();
    static void TestNumber();
    static void TestString();
    static void TestUtf16String();
    static void TestObject();
    static void TestKeyable();
    static void TestDefineProperty();
    static void TestDefinePropertyWritable();
    static void TestDefinePropertyConfigurable();
    static void TestDefinePropertyConfigurableV2();
    static void TestArray();
    static void TestBigintInt64();
    static void TestBigint16Bytes();
    static void TestBigint28Bytes();
    static void TestBigint32Bytes();
    static void TestPromise();
    static void TestSymbol();
    static void TestFunction();
    static void TestClass();
    static void TestArrayBuffer();
    static void TestInstanceOf();
    static void TestUpdateStackInfo();
};

template <typename T>
struct Slab {
public:
    int64_t Add(T item)
    {
        int64_t result;
        if (last >= 0) {
            result = last;
            auto& lastItem = items_[result];
            last = lastItem.prev;
            lastItem.data = std::move(item);
        } else {
            result = items_.size();
            items_.push_back({std::move(item), -1});
        }
        ++length;
        return result;
    }

    void Del(int64_t id)
    {
        auto& removing = items_[id];
        if (!removing.data.has_value()) {
            return;
        }
        removing.data = std::nullopt;
        removing.prev = last;
        last = id;
        --length;
    }

    size_t Size() const
    {
        return length;
    }

    bool Has(int64_t id) const
    {
        if (id < 0 || id >= length) {
            return false;
        }
        return items_[id].data.has_value();
    }

    T& Get(int64_t id)
    {
        return items_[id].data.value();
    }

    const T& Get(int64_t id) const
    {
        return items_[id].data.value();
    }
private:
    struct Item {
        std::optional<T> data;
        int64_t prev;
    };
    std::vector<Item> items_ {};
    int64_t last = -1;
    size_t length = 0;
};

class MockContext {
    static MockContext* instance_;
    static uint32_t NewId();

public:
    static MockContext* GetInstance();
    ARKTS_Engine GetEngine() const;
    ARKTS_Env GetEnv() const;
    static void Init();

    explicit MockContext(ARKTS_Engine engine, bool needDestroyEngine = true);
    MockContext();
    ~MockContext();

    int64_t StoreFunc(std::function<ARKTS_Value(ARKTS_CallInfo)> call);
    int64_t StoreAsyncFunc(std::function<void()> call);
    int64_t StoreArrayBufferDeleter(std::function<void(void*)> call);
    int64_t StoreNativeData(void* data, std::function<void(void*)> deleter);
    void* GetFuncAddr(int64_t id);

    void SetNativeCallingCallback(std::function<void(const void*)> callback);
    void SetNativeReturnCallback(std::function<void(const void*)> callback);
    void NotifyNativeCalling(const void* addr);
    void NotifyNativeReturn(const void* addr);
    void* GetNativeData(int64_t id);
    void SetFinalizerCallback(std::function<void(ARKTS_Env)> callback);

protected:
    virtual MockContextKind GetContextKind() const;
    virtual ARKTS_Value ExportModule(ARKTS_Env env, const char* dllName, ARKTS_Value exports);
    virtual bool HasModuleHandle(const char* dllName);
    virtual void ThrowJSError(ARKTS_Env env, ARKTS_Value arktsValue);
    virtual void ThrowNativeError(const char* msg);
    virtual void DeleteArrayBufferRawData(void* buffer, int64_t id);
    virtual void DeleteExternal(int64_t id, ARKTS_Env env);
    virtual ARKTS_Value InvokeLambda(ARKTS_CallInfo info, int64_t id);
    virtual void DeleteLambda(ARKTS_Env env, int64_t id);
    virtual void InvokeAsyncLambda(ARKTS_Env env, int64_t id);
    virtual void DeleteJSContext(ARKTS_Env env);
    virtual ARKTS_Value InvokeCycleFreeFunc(ARKTS_CallInfo callInfo, uint32_t id);
    virtual void ReleaseCycleFreeExt(uint32_t id);

    int64_t ToId(int64_t index) const;
    int64_t ToIndex(int64_t id) const;

private:
    static void InitModuleCallbacks();
    static void InitCycleFreeCallbacks();
    static void InitDebugModeCallbacks();

    int64_t GetPrefixMask() const;

    uint32_t idPrefix_;
    MockContext* lastContext_;
    ARKTS_Engine engine_;
    bool needDestroyEngine_;
    Slab<std::function<ARKTS_Value(ARKTS_CallInfo)>> callbacks_;
    Slab<std::function<void()>> asyncCallbacks_;
    Slab<std::function<void(void*)>> nativeDeleters_;
    struct AnyData {
        void* data;
        std::function<void(void*)> deleter;
    };
    Slab<AnyData> data_;
    std::function<void(ARKTS_Env)> finalizerCallback_;
    std::function<void(const void*)> nativeCallingCallback_;
    std::function<void(const void*)> nativeReturnCallback_;
};

class ErrorCaptureContext : public MockContext {
public:
    ErrorCaptureContext() = default;
    explicit ErrorCaptureContext(ARKTS_Engine engine, bool needDestroyEngine = true)
        : MockContext(engine, needDestroyEngine) {}
    ~ErrorCaptureContext()
    {
        if (hasJSError_ || hasNativeError_) {
            printf("has unhandled js error or native error\n");
            std::abort();
        }
    }

    bool HasAndClearJSError()
    {
        if (hasJSError_) {
            hasJSError_ = false;
            return true;
        }
        return false;
    }

    bool HasAndClearNativeError()
    {
        if (hasNativeError_) {
            hasNativeError_ = false;
            return true;
        }
        return false;
    }

    void ReportCapturedNativeError(const char* error)
    {
        if (hasNativeError_) {
            printf("has unhandled native error: %s\n", error);
            std::abort();
        }
        hasNativeError_ = true;
    }

    void ReportCapturedJSError(ARKTS_Env /* env */, ARKTS_Value /* value */)
    {
        if (hasJSError_) {
            printf("has unhandled js error\n");
            std::abort();
        }
        hasJSError_ = true;
    }
protected:
    MockContextKind GetContextKind() const override
    {
        return MockContextKind::ERROR_CAPTURE;
    }

    void ThrowJSError(ARKTS_Env env, ARKTS_Value arktsValue) override
    {
        ReportCapturedJSError(env, arktsValue);
    }

    void ThrowNativeError(const char* msg) override
    {
        printf("[Native Error]: %s\n", msg);
        ReportCapturedNativeError(msg);
    }

private:
    bool hasJSError_ = false;
    bool hasNativeError_ = false;
};

class ModuleRegisteredContext : public ErrorCaptureContext {
protected:
    bool HasModuleHandle(const char* /* dllName */) override
    {
        return true;
    }
};

class CycleFreeContext : public ErrorCaptureContext {
public:
    CycleFreeContext() = default;
    explicit CycleFreeContext(ARKTS_Engine engine, bool needDestroyEngine = true)
        : ErrorCaptureContext(engine, needDestroyEngine) {}
    uint32_t StoreCycleFreeFunc(std::function<ARKTS_Value(ARKTS_CallInfo)> callback)
    {
        std::lock_guard lock(callbackMutex_);
        return callbacks_.Add(std::move(callback));
    }

    size_t GetCycleFreeFuncCount() const
    {
        return callbacks_.Size();
    }

protected:
    ARKTS_Value InvokeCycleFreeFunc(ARKTS_CallInfo callInfo, uint32_t id) override
    {
        std::lock_guard lock(callbackMutex_);
        auto callback = callbacks_.Get(id);
        if (!callback) {
            return ARKTS_CreateUndefined();
        }
        return callback(callInfo);
    }

    void ReleaseCycleFreeExt(uint32_t id) override
    {
        callbacks_.Del(id);
    }
private:
    static Slab<std::function<ARKTS_Value(ARKTS_CallInfo)>> callbacks_;
    static std::mutex callbackMutex_;
};

struct AssertTestContext {
    ARKTS_Env nullEnv = nullptr;
    ARKTS_CallInfo nullInfo = nullptr;
    ARKTS_Engine nullEngine = nullptr;
    ARKTS_Scope nullScope = nullptr;
    ARKTS_Global nullGlobal = nullptr;
    ARKTS_Promise nullPromise = nullptr;
    ARKTS_Value zeroValue { 0 };
    char smallBuf[8] = {};
    uint8_t smallBytes[8] = {};
    bool isNegative = false;
    ARKTS_Value oneArg[1] = { zeroValue };
    ARKTS_Accessor accessor { zeroValue, zeroValue, N_WRITABLE };
    ARKTS_Engine tempEngine = nullptr;
    ARKTS_Env normalEnv = nullptr;
};

void TriggerArktsAssertEarlyReturnBranches();

#endif // TEST_ARK_INTEROP_COMMON_H
