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

#include "ark_interop_external.h"
#include "ark_interop_hitrace.h"
#include "ark_interop_internal.h"
#include "ark_interop_log.h"
#include "ark_interop_napi.h"
#include "gtest/gtest.h"
#include "uv_loop_handler.h"

#include <cmath>
#include <thread>

#include "napi/native_api.h"

using namespace testing;
using namespace testing::ext;

struct ARKTS_ModuleCallbacks {
    ARKTS_Value (*exportModule)(ARKTS_Env env, const char* dllName, ARKTS_Value exports) = nullptr;
    bool (*hasModuleHandle)(const char* dllName) = nullptr;
    void (*throwJSError)(ARKTS_Env env, ARKTS_Value) = nullptr;
    void (*throwNativeError)(const char*) = nullptr;
    void (*deleteArrayBufferRawData)(void* buffer, int64_t lambdaId) = nullptr;
    void (*deleteExternal)(int64_t id, ARKTS_Env env) = nullptr;
    ARKTS_Value (*invokerLambda)(ARKTS_CallInfo, int64_t lambdaId) = nullptr;
    void (*deleteLambda)(ARKTS_Env env, int64_t lambdaId) = nullptr;
    void (*invokeAsyncLambda)(ARKTS_Env env, int64_t lambdaId) = nullptr;
    void (*deleteJSContext)(ARKTS_Env env) = nullptr;
};

enum StackInfoOpKind : uint32_t {
    SWITCH_TO_SUB_STACK_INFO = 0,
    SWITCH_TO_MAIN_STACK_INFO,
};

struct StackInfo {
    uint64_t stackLimit;
    uint64_t lastLeaveFrame;
};

namespace {
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
        if (last_ >= 0) {
            result = last_;
            auto& last = items_[result];
            last_ = last.prev;
            last.data = std::move(item);
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
        removing.prev = last_;
        last_ = id;
        --length;
    }

    size_t Size() const
    {
        return length;
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
    int64_t last_ = -1;
    size_t length = 0;
};

class MockContext {
    static MockContext* instance_;
    static uint32_t NewId()
    {
        static uint32_t id = 0;
        return id++;
    }

public:
    static MockContext* GetInstance()
    {
        return instance_;
    }

    ARKTS_Engine GetEngine() const
    {
        return engine_;
    }

    ARKTS_Env GetEnv() const
    {
        return ARKTS_GetContext(engine_);
    }

    static void Init();

    explicit MockContext(ARKTS_Engine engine, bool needDestroyEngine = true): idPrefix_(NewId()),
        lastContext_(instance_), engine_(engine), needDestroyEngine_(needDestroyEngine)
    {
        instance_ = this;
    }

    MockContext(): idPrefix_(NewId()), lastContext_(instance_), engine_(instance_->GetEngine()),
        needDestroyEngine_(false)
    {
        instance_ = this;
    }

    ~MockContext()
    {
        if (needDestroyEngine_) {
            ARKTS_DestroyEngine(engine_);
        }
        engine_ = nullptr;
        instance_ = lastContext_;
    }

    int64_t StoreFunc(std::function<ARKTS_Value(ARKTS_CallInfo)> call)
    {
        return ToId(callbacks_.Add(std::move(call)));
    }

    int64_t StoreAsyncFunc(std::function<void ()> call)
    {
        return ToId(asyncCallbacks_.Add(std::move(call)));
    }

    int64_t StoreArrayBufferDeleter(std::function<void (void*)> call)
    {
        return ToId(nativeDeleters_.Add(std::move(call)));
    }

    int64_t StoreNativeData(void* data, std::function<void (void*)>deleter)
    {
        return ToId(data_.Add({data, std::move(deleter)}));
    }

    void* GetNativeData(int64_t id)
    {
        auto index = ToIndex(id);
        if (index < 0) {
            return nullptr;
        }
        return data_.Get(index).data;
    }

    void SetFinalizerCallback(std::function<void (ARKTS_Env)> callback)
    {
        finalizerCallback_ = std::move(callback);
    }

protected:
    virtual ARKTS_Value ExportModule(ARKTS_Env env, const char* dllName, ARKTS_Value exports)
    {
        return exports;
    }

    virtual bool HasModuleHandle(const char* dllName)
    {
        return false;
    }

    virtual void ThrowJSError(ARKTS_Env env, ARKTS_Value)
    {
        EXPECT_TRUE(false);
    }

    virtual void ThrowNativeError(const char* msg)
    {
        printf("[Native Error]: %s\n", msg);
        EXPECT_TRUE(false);
    }

    virtual void DeleteArrayBufferRawData(void* buffer, int64_t id)
    {
        auto index = ToIndex(id);
        if (index < 0) {
            return;
        }
        nativeDeleters_.Get(index)(buffer);
        nativeDeleters_.Del(index);
    }

    virtual void DeleteExternal(int64_t id, ARKTS_Env env)
    {
        auto index = ToIndex(id);
        if (index < 0) {
            return;
        }
        auto& data = data_.Get(index);
        data.deleter(data.data);
        data_.Del(index);
    }

    virtual ARKTS_Value InvokeLambda(ARKTS_CallInfo info, int64_t id)
    {
        auto index = ToIndex(id);
        if (index < 0) {
            return ARKTS_CreateUndefined();
        }
        return callbacks_.Get(index)(info);
    }

    virtual void DeleteLambda(ARKTS_Env env, int64_t id)
    {
        auto index = ToIndex(id);
        if (index < 0) {
            return;
        }
        callbacks_.Del(index);
    }

    virtual void InvokeAsyncLambda(ARKTS_Env env, int64_t id)
    {
        auto index = ToIndex(id);
        if (index < 0) {
            return;
        }
        asyncCallbacks_.Get(index)();
        asyncCallbacks_.Del(index);
    }

    virtual void DeleteJSContext(ARKTS_Env env)
    {
        if (finalizerCallback_) {
            finalizerCallback_(env);
        }
    }

    virtual ARKTS_Value InvokeCycleFreeFunc(ARKTS_CallInfo callInfo, uint32_t id)
    {
        return ARKTS_CreateUndefined();
    }

    virtual void ReleaseCycleFreeExt(uint32_t id)
    {
    }

    int64_t ToId(int64_t index) const
    {
        return GetPrefixMask() | index;
    }

    int64_t ToIndex(int64_t id) const
    {
        auto myMask = GetPrefixMask();
        if ((id & myMask) == myMask) {
            return id & 0xFFFF'FFFF;
        }
        return -1;
    }

private:
    int64_t GetPrefixMask() const
    {
        constexpr auto idBits = 32;
        return static_cast<int64_t>(idPrefix_) << idBits;
    }

    uint32_t idPrefix_;
    MockContext* lastContext_;
    ARKTS_Engine engine_;
    bool needDestroyEngine_;
    Slab<std::function<ARKTS_Value (ARKTS_CallInfo)>> callbacks_;
    Slab<std::function<void ()>> asyncCallbacks_;
    Slab<std::function<void (void*)>> nativeDeleters_;
    struct AnyData {
        void* data;
        std::function<void (void*)> deleter;
    };
    Slab<AnyData> data_;
    std::function<void (ARKTS_Env)> finalizerCallback_;
};

void MockContext::Init()
{
    static ARKTS_ModuleCallbacks callbacks {
        .exportModule = [](ARKTS_Env env, const char* dllName, ARKTS_Value exports)->ARKTS_Value {
            return instance_ ? instance_->ExportModule(env, dllName, exports) : exports;
        },
        .hasModuleHandle = [](const char* dllName)->bool {
            return instance_ ? instance_->HasModuleHandle(dllName) : false;
        },
        .throwJSError = [](ARKTS_Env env, ARKTS_Value error)->void {
            if (instance_) instance_->ThrowJSError(env, error);
        },
        .throwNativeError = [](const char* error)->void {
            if (instance_) instance_->ThrowNativeError(error);
        },
        .deleteArrayBufferRawData = [](void* buffer, int64_t lambdaId)->void {
            if (instance_) instance_->DeleteArrayBufferRawData(buffer, lambdaId);
        },
        .deleteExternal = [](int64_t id, ARKTS_Env env)->void {
            if (instance_) instance_->DeleteExternal(id, env);
        },
        .invokerLambda = [](ARKTS_CallInfo callInfo, int64_t lambdaId)->ARKTS_Value {
            return instance_ ? instance_->InvokeLambda(callInfo, lambdaId) : ARKTS_CreateUndefined();
        },
        .deleteLambda = [](ARKTS_Env env, int64_t id)->void {
            if (instance_) instance_->DeleteLambda(env, id);
        },
        .invokeAsyncLambda = [](ARKTS_Env env, int64_t id)->void {
            if (instance_) instance_->InvokeAsyncLambda(env, id);
        },
        .deleteJSContext = [](ARKTS_Env env)->void {
            if (instance_) instance_->DeleteJSContext(env);
        }
    };
    ARKTS_SetCJModuleCallback(&callbacks);
    static ARKTS_CycleFreeCallback cycleFreeCallback {
        .funcInvoker = [](ARKTS_CallInfo callInfo, int64_t id) {
            if (instance_) {
                return instance_->InvokeCycleFreeFunc(callInfo, id);
            }
            return ARKTS_CreateUndefined();
        },
        .refRelease = [](int64_t id) {
            if (instance_) {
                instance_->ReleaseCycleFreeExt(id);
            }
        }
    };
    ARKTS_RegisterCycleFreeCallback(cycleFreeCallback);
}

MockContext* MockContext::instance_ = nullptr;

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
protected:
    void ThrowJSError(ARKTS_Env env, ARKTS_Value) override
    {
        if (hasJSError_) {
            printf("has unhandled js error\n");
            std::abort();
        }
        hasJSError_ = true;
    }

    void ThrowNativeError(const char* error) override
    {
        if (hasNativeError_) {
            printf("has unhandled native error: %s\n", error);
            std::abort();
        }
        hasNativeError_ = true;
    }
private:
    bool hasJSError_ = false;
    bool hasNativeError_ = false;
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

Slab<std::function<ARKTS_Value(ARKTS_CallInfo)>> CycleFreeContext::callbacks_;
std::mutex CycleFreeContext::callbackMutex_;

void ArkInteropTest::TestPrime()
{
    auto env = MockContext::GetInstance()->GetEnv();
    auto scope = ARKTS_OpenScope(env);

    auto jsUDef = ARKTS_CreateUndefined();
    EXPECT_TRUE(ARKTS_IsUndefined(jsUDef));
    EXPECT_EQ(ARKTS_GetValueType(env, jsUDef), N_UNDEFINED);

    auto jsNull = ARKTS_CreateNull();
    EXPECT_TRUE(ARKTS_IsNull(jsNull));
    EXPECT_EQ(ARKTS_GetValueType(env, jsNull), N_NULL);

    ARKTS_Value jsBools[] {
        ARKTS_CreateBool(true),
        ARKTS_CreateBool(false)
    };
    EXPECT_TRUE(ARKTS_IsBool(jsBools[0]));
    EXPECT_EQ(ARKTS_GetValueType(env, jsBools[0]), N_BOOL);
    EXPECT_TRUE(ARKTS_GetValueBool(jsBools[0]));
    EXPECT_FALSE(ARKTS_GetValueBool(jsBools[1]));

    ARKTS_CloseScope(env, scope);
}

void ArkInteropTest::TestNumber()
{
    auto env = MockContext::GetInstance()->GetEnv();
    auto scope = ARKTS_OpenScope(env);
    double origins[] {
        0.1,
        -12.1,
        12456.126546
    };
    constexpr auto totalCases = std::size(origins);
    ARKTS_Value jsValues[totalCases];
    double received[totalCases];

    for (size_t i = 0;i < totalCases; i++) {
        jsValues[i] = ARKTS_CreateF64(origins[i]);
        EXPECT_EQ(ARKTS_GetValueType(env, jsValues[i]), N_NUMBER);
        EXPECT_TRUE(ARKTS_IsNumber(jsValues[i]));
        received[i] = ARKTS_GetValueNumber(jsValues[i]);
        EXPECT_EQ(origins[i], received[i]);
    }

    auto jsNan = ARKTS_CreateF64(NAN);
    auto nNan = ARKTS_GetValueNumber(jsNan);
    EXPECT_TRUE(std::isnan(nNan));
    ARKTS_CloseScope(env, scope);
}

void ArkInteropTest::TestString()
{
    auto env = MockContext::GetInstance()->GetEnv();
    auto scope = ARKTS_OpenScope(env);
    const char* origins[] {
        "a plain text",
        "`~!@#$%^&*()_+[]\\",
        "中文字符",
        "😊😂🤣❤️😍😒👌😘",
    };
    ARKTS_Value jsValues[] {
        ARKTS_CreateUtf8(env, origins[0], strlen(origins[0])),
        ARKTS_CreateUtf8(env, origins[1], strlen(origins[1])),
        ARKTS_CreateUtf8(env, origins[2], strlen(origins[2])),
        ARKTS_CreateUtf8(env, origins[3], strlen(origins[3])),
    };
    EXPECT_TRUE(ARKTS_IsString(env, jsValues[0]));
    EXPECT_TRUE(ARKTS_IsHeapObject(jsValues[0]));
    EXPECT_EQ(ARKTS_GetValueType(env, jsValues[0]), N_STRING);
    for (auto i = 0;i < sizeof(jsValues)/sizeof(ARKTS_Value); i++) {
        auto size = ARKTS_GetValueUtf8Size(env, jsValues[i]);
        std::string result;
        result.resize(size - 1);
        ARKTS_GetValueUtf8(env, jsValues[i], size - 1, result.data());
        EXPECT_EQ(result, origins[i]);

        auto cStr = ARKTS_GetValueCString(env, jsValues[i]);
        result = cStr;
        EXPECT_EQ(result, origins[i]);
        ARKTS_FreeCString(cStr);
    }
    ARKTS_CloseScope(env, scope);
}

void ArkInteropTest::TestUtf16String()
{
    auto env = MockContext::GetInstance()->GetEnv();
    auto scope = ARKTS_OpenScope(env);

    std::string latin1Case[] {
        "a plain text",
        "hello, world!",
        "./[]124"
    };
    std::u16string utf16Cases[] {
        u"a plain text, 和",
        u"你好，世界！",
        u"🦐😀"
    };

    for (const auto& one : latin1Case) {
        auto value = ARKTS_CreateString(env, true, one.size(), one.data());
        EXPECT_TRUE(ARKTS_IsString(env, value));
        auto header = ARKTS_GetStringInfo(env, value);
        EXPECT_EQ(header.length, one.size());
        EXPECT_TRUE(header.isCompressed);
    }

    for (const auto& one : utf16Cases) {
        auto value = ARKTS_CreateString(env, false, one.size(), one.data());
        EXPECT_TRUE(ARKTS_IsString(env, value));
        auto header = ARKTS_GetStringInfo(env, value);
        EXPECT_EQ(header.length, one.size());
        EXPECT_FALSE(header.isCompressed);
    }

    ARKTS_CloseScope(env, scope);
}

void ArkInteropTest::TestObject()
{
    auto env = MockContext::GetInstance()->GetEnv();
    auto scope = ARKTS_OpenScope(env);
    auto obj = ARKTS_CreateObject(env);
    EXPECT_TRUE(ARKTS_IsHeapObject(obj));
    EXPECT_TRUE(ARKTS_IsObject(env, obj));
    EXPECT_EQ(ARKTS_GetValueType(env, obj), N_OBJECT);
    auto keyA = ARKTS_CreateUtf8(env, "a", 1);
    EXPECT_FALSE(ARKTS_HasOwnProperty(env, obj, keyA));
    auto valueA = ARKTS_GetProperty(env, obj, keyA);
    EXPECT_TRUE(ARKTS_IsUndefined(valueA));
    valueA = ARKTS_CreateBool(true);
    ARKTS_SetProperty(env, obj, keyA, valueA);
    EXPECT_TRUE(ARKTS_HasOwnProperty(env, obj, keyA));
    auto receivedA = ARKTS_GetProperty(env, obj, keyA);
    EXPECT_TRUE(ARKTS_IsBool(receivedA));
    EXPECT_TRUE(ARKTS_GetValueBool(receivedA));
    auto keys = ARKTS_EnumOwnProperties(env, obj);
    EXPECT_TRUE(ARKTS_IsArray(env, keys));
    EXPECT_EQ(ARKTS_GetArrayLength(env, keys), 1);
    auto key = ARKTS_GetElement(env, keys, 0);
    EXPECT_TRUE(ARKTS_IsString(env, key));
    EXPECT_TRUE(ARKTS_StrictEqual(env, keyA, key));
    ARKTS_CloseScope(env, scope);
}

void ArkInteropTest::TestKeyable()
{
    auto env = MockContext::GetInstance()->GetEnv();
    auto scope = ARKTS_OpenScope(env);
    auto obj = ARKTS_CreateObject(env);
    ARKTS_Value allowedKeys[] {
        ARKTS_CreateUtf8(env, "a", 1),
        ARKTS_CreateI32(12),
        ARKTS_CreateSymbol(env, "a", 1)
    };
    auto value = ARKTS_CreateBool(true);
    for (auto key : allowedKeys) {
        ARKTS_SetProperty(env, obj, key, value);
        auto received = ARKTS_GetProperty(env, obj, key);
        EXPECT_TRUE(ARKTS_IsBool(received));
        EXPECT_TRUE(ARKTS_GetValueBool(received));
    }
    ARKTS_CloseScope(env, scope);
}

struct PropCase {
    char k;
    bool writable;
    bool enumerable;
    bool configurable;

    static PropCase cases[];
};

PropCase PropCase::cases[] {
    {'a', false, false, false},
    {'b', true, false, false},
    {'c', false, true, false},
    {'d', false, false, true},
    {'e', true, true, false},
    {'f', true, false, true},
    {'g', false, true, true},
    {'h', true, true, true}
};

void ArkInteropTest::TestDefineProperty()
{
    ErrorCaptureContext local;
    auto env = local.GetEnv();
    auto scope = ARKTS_OpenScope(env);
    auto obj = ARKTS_CreateObject(env);

    constexpr auto totalCases = std::size(PropCase::cases);
    auto valueF = ARKTS_CreateBool(false);
    ARKTS_Value keys[totalCases];
    for (auto i = 0; i < totalCases; i++) {
        keys[i] = ARKTS_CreateUtf8(env, &PropCase::cases[i].k, 1);
        ARKTS_DefineOwnProperty(env, obj, keys[i], valueF, static_cast<ARKTS_PropertyFlag>(
            (PropCase::cases[i].writable ? N_WRITABLE : 0) |
            (PropCase::cases[i].enumerable ? N_ENUMERABLE : 0) |
            (PropCase::cases[i].configurable ? N_CONFIGURABLE : 0)
        ));
    }
    constexpr int expectKeys = 4;
    auto jsKeys = ARKTS_EnumOwnProperties(env, obj);
    EXPECT_TRUE(ARKTS_IsArray(env, jsKeys));
    EXPECT_EQ(ARKTS_GetArrayLength(env, jsKeys), expectKeys);
    for (auto i = 0; i < expectKeys; i++) {
        auto jsKey = ARKTS_GetElement(env, jsKeys, i);
        EXPECT_TRUE(ARKTS_IsString(env, jsKey));
        char buffer = 0;
        ARKTS_GetValueUtf8(env, jsKey, 1, &buffer);
        EXPECT_TRUE(buffer == 'c' || buffer == 'e' || buffer == 'g' || buffer == 'h');
    }

    ARKTS_CloseScope(env, scope);
}

void ArkInteropTest::TestDefinePropertyWritable()
{
    ErrorCaptureContext local;
    auto env = local.GetEnv();
    auto scope = ARKTS_OpenScope(env);
    auto obj = ARKTS_CreateObject(env);

    constexpr auto totalCases = std::size(PropCase::cases);
    auto valueT = ARKTS_CreateBool(true);
    auto valueF = ARKTS_CreateBool(false);
    ARKTS_Value keys[totalCases];
    for (auto i = 0; i < totalCases; i++) {
        keys[i] = ARKTS_CreateUtf8(env, &PropCase::cases[i].k, 1);
        ARKTS_DefineOwnProperty(env, obj, keys[i], valueF, static_cast<ARKTS_PropertyFlag>(
            (PropCase::cases[i].writable ? N_WRITABLE : 0) |
            (PropCase::cases[i].enumerable ? N_ENUMERABLE : 0) |
            (PropCase::cases[i].configurable ? N_CONFIGURABLE : 0)
        ));
    }

    for (auto i = 0; i < totalCases; i++) { // writable
        if (PropCase::cases[i].writable && PropCase::cases[i].configurable) {
            ARKTS_SetProperty(env, obj, keys[i], valueT);
            auto receivedJS = ARKTS_GetProperty(env, obj, keys[i]);
            auto recievedN = ARKTS_GetValueBool(receivedJS);
            EXPECT_TRUE(recievedN);
        }
    }

    ARKTS_CloseScope(env, scope);
}

void ArkInteropTest::TestDefinePropertyConfigurable()
{
    ErrorCaptureContext local;
    auto env = local.GetEnv();
    auto scope = ARKTS_OpenScope(env);
    auto obj = ARKTS_CreateObject(env);

    constexpr auto totalCases = std::size(PropCase::cases);
    auto valueT = ARKTS_CreateBool(true);
    auto valueF = ARKTS_CreateBool(false);
    ARKTS_Value keys[totalCases];
    for (auto i = 0; i < totalCases; i++) {
        keys[i] = ARKTS_CreateUtf8(env, &PropCase::cases[i].k, 1);
        ARKTS_DefineOwnProperty(env, obj, keys[i], valueF, static_cast<ARKTS_PropertyFlag>(
            (PropCase::cases[i].writable ? N_WRITABLE : 0) |
            (PropCase::cases[i].enumerable ? N_ENUMERABLE : 0) |
            (PropCase::cases[i].configurable ? N_CONFIGURABLE : 0)
        ));
    }

    for (auto i = 0;i < totalCases; ++i) { // configurable
        if (PropCase::cases[i].configurable) {
            ARKTS_DefineOwnProperty(env, obj, keys[i], valueT,
                static_cast<ARKTS_PropertyFlag>(N_WRITABLE | N_ENUMERABLE | N_CONFIGURABLE));
            auto received = ARKTS_GetProperty(env, obj, keys[i]);
            EXPECT_TRUE(ARKTS_IsBool(received));
            EXPECT_TRUE(ARKTS_GetValueBool(received));
        }
    }
    auto jsKeys = ARKTS_EnumOwnProperties(env, obj);
    constexpr int expectLength = 6;
    EXPECT_EQ(ARKTS_GetArrayLength(env, jsKeys), expectLength);

    ARKTS_CloseScope(env, scope);
}

void ArkInteropTest::TestDefinePropertyConfigurableV2()
{
    ErrorCaptureContext local;
    auto env = local.GetEnv();
    auto scope = ARKTS_OpenScope(env);
    auto obj = ARKTS_CreateObject(env);

    constexpr auto totalCases = std::size(PropCase::cases);
    auto valueT = ARKTS_CreateBool(true);
    auto valueF = ARKTS_CreateBool(false);
    ARKTS_Value keys[totalCases];
    for (auto i = 0; i < totalCases; i++) {
        keys[i] = ARKTS_CreateUtf8(env, &PropCase::cases[i].k, 1);
        ARKTS_DefineOwnProperty(env, obj, keys[i], valueF, static_cast<ARKTS_PropertyFlag>(
            (PropCase::cases[i].writable ? N_WRITABLE : 0) |
            (PropCase::cases[i].enumerable ? N_ENUMERABLE : 0) |
            (PropCase::cases[i].configurable ? N_CONFIGURABLE : 0)
        ));
    }

    for (auto i = 0;i < totalCases; ++i) { // configurable
        auto result = ARKTS_DefineOwnPropertyV2(env, obj, keys[i], valueT,
            static_cast<ARKTS_PropertyFlag>(N_WRITABLE | N_ENUMERABLE | N_CONFIGURABLE));
        if (PropCase::cases[i].configurable) {
            EXPECT_TRUE(result);
        } else {
            EXPECT_TRUE(local.HasAndClearJSError());
        }
    }
    auto jsKeys = ARKTS_EnumOwnProperties(env, obj);
    constexpr int expectLength = 6;
    EXPECT_EQ(ARKTS_GetArrayLength(env, jsKeys), expectLength);

    ARKTS_CloseScope(env, scope);
}

void ArkInteropTest::TestInstanceOf()
{
    ARKTS_Env env = MockContext::GetInstance()->GetEnv();
    auto global = ARKTS_GetGlobalConstant(env);
    char clError[] = "Error";
    auto jError = ARKTS_CreateUtf8(env, clError, sizeof(clError) - 1);
    auto errorCls = ARKTS_GetProperty(env, global, jError);
    auto errorObJ = ARKTS_New(env, errorCls, 0, nullptr);
    auto isError = ARKTS_InstanceOf(env, errorObJ, errorCls);
    EXPECT_TRUE(isError);
    auto jObj = ARKTS_CreateObject(env);
    EXPECT_FALSE(ARKTS_InstanceOf(env, jObj, errorCls));
}

void ArkInteropTest::TestArray()
{
    EXPECT_TRUE(MockContext::GetInstance());
    ARKTS_Env env = MockContext::GetInstance()->GetEnv();
    EXPECT_TRUE(env);

    char c = 'c';
    ARKTS_Value origin[] {
        ARKTS_CreateBool(true),
        ARKTS_CreateF64(12.02),
        ARKTS_CreateUtf8(env, &c, 1),
        ARKTS_CreateObject(env),
    };
    constexpr auto arrSize = std::size(origin);
    auto arr = ARKTS_CreateArray(env, arrSize);
    EXPECT_TRUE(ARKTS_IsArray(env, arr));
    EXPECT_EQ(ARKTS_GetValueType(env, arr), N_OBJECT);
    EXPECT_EQ(ARKTS_GetArrayLength(env, arr), arrSize);
    for (auto i = 0; i < arrSize; i++) {
        auto initialValue = ARKTS_GetElement(env, arr, i);
        EXPECT_TRUE(ARKTS_IsUndefined(initialValue));
        ARKTS_SetElement(env, arr, i, origin[i]);
        auto receivedValue = ARKTS_GetElement(env, arr, i);
        EXPECT_TRUE(ARKTS_StrictEqual(env, origin[i], receivedValue));
    }
}

void ArkInteropTest::TestBigintInt64()
{
    EXPECT_TRUE(MockContext::GetInstance());
    ARKTS_Env env = MockContext::GetInstance()->GetEnv();
    EXPECT_TRUE(env);
    {
        int64_t origins[] {
            0x123'4567'8945,
            -0x123'4567'8945,
            0x7FFF'FFFF'FFFF'FFFF,
            -0x7FFF'FFFF'FFFF'FFFF,
        };
        constexpr auto arrSize = std::size(origins);
        ARKTS_Value values[arrSize];
        for (auto i = 0; i < arrSize; i++) {
            values[i] = ARKTS_CreateBigInt(env, origins[i]);
            EXPECT_TRUE(ARKTS_IsBigInt(env, values[i]));
            EXPECT_EQ(ARKTS_GetValueType(env, values[i]), N_BIGINT);
            ARKTS_BigIntGetByteSize(env, values[i]);
        }
    }
}

void ArkInteropTest::TestBigint16Bytes()
{
    EXPECT_TRUE(MockContext::GetInstance());
    ARKTS_Env env = MockContext::GetInstance()->GetEnv();
    EXPECT_TRUE(env);
    uint8_t origin[] {
        0, 10, 20, 30, 40, 50, 60, 70,
        80, 90, 70, 50, 20, 30, 40, 50
    };
    bool isNegative = false;
    auto value = ARKTS_CreateBigIntWithBytes(env, isNegative, std::size(origin), origin);
    EXPECT_TRUE(ARKTS_IsBigInt(env, value));
    EXPECT_EQ(ARKTS_GetValueType(env, value), N_BIGINT);
    EXPECT_EQ(ARKTS_BigIntGetByteSize(env, value), std::size(origin));
    uint8_t received[std::size(origin)];
    ARKTS_BigIntReadBytes(env, value, &isNegative, std::size(origin), received);
    EXPECT_FALSE(isNegative);
    for (auto i = 0; i < std::size(origin); i++) {
        EXPECT_EQ(origin[i], received[i]);
    }
}

void ArkInteropTest::TestBigint28Bytes()
{
    EXPECT_TRUE(MockContext::GetInstance());
    ARKTS_Env env = MockContext::GetInstance()->GetEnv();
    EXPECT_TRUE(env);
    uint8_t origin[] {
        0, 10, 20, 30, 40, 50, 60, 70,
        80, 90, 70, 50, 20, 30, 40, 50,
        1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 20, 30,
    };
    bool isNegative = false;
    constexpr int expectSize = 32;
    // bigint words are 8 bytes aligned, received 32 bytes, and lower 4 bytes would be filled with 0.
    auto value = ARKTS_CreateBigIntWithBytes(env, isNegative, std::size(origin), origin);
    EXPECT_TRUE(ARKTS_IsBigInt(env, value));
    EXPECT_EQ(ARKTS_GetValueType(env, value), N_BIGINT);
    EXPECT_EQ(ARKTS_BigIntGetByteSize(env, value), expectSize);
    uint8_t received[expectSize];
    ARKTS_BigIntReadBytes(env, value, &isNegative, expectSize, received);
    EXPECT_FALSE(isNegative);
    for (auto i = 0; i < std::size(origin); i++) {
        EXPECT_EQ(origin[i], received[i + expectSize - std::size(origin)]);
    }
}

void ArkInteropTest::TestBigint32Bytes()
{
    EXPECT_TRUE(MockContext::GetInstance());
    ARKTS_Env env = MockContext::GetInstance()->GetEnv();
    EXPECT_TRUE(env);
    uint8_t origin[] {
        0, 10, 20, 30, 40, 50, 60, 70,
        80, 90, 70, 50, 20, 30, 40, 50,
        1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 20, 30, 40, 50, 60, 70,
    };
    bool isNegative = false;
    auto value = ARKTS_CreateBigIntWithBytes(env, isNegative, std::size(origin), origin);
    EXPECT_TRUE(ARKTS_IsBigInt(env, value));
    EXPECT_EQ(ARKTS_GetValueType(env, value), N_BIGINT);
    EXPECT_EQ(ARKTS_BigIntGetByteSize(env, value), std::size(origin));
    uint8_t received[std::size(origin)];
    ARKTS_BigIntReadBytes(env, value, &isNegative, std::size(origin), received);
    EXPECT_FALSE(isNegative);
    for (auto i = 0; i < std::size(origin); i++) {
        EXPECT_EQ(origin[i], received[i]);
    }
}

void ArkInteropTest::TestSymbol()
{
    EXPECT_TRUE(MockContext::GetInstance());
    ARKTS_Env env = MockContext::GetInstance()->GetEnv();
    EXPECT_TRUE(env);

    const char originDesc[] = "TestSymbol";

    auto symbol = ARKTS_CreateSymbol(env, originDesc, sizeof(originDesc) - 1);
    EXPECT_TRUE(ARKTS_IsSymbol(env, symbol));
    EXPECT_EQ(ARKTS_GetValueType(env, symbol), N_SYMBOL);

    auto retDesc = ARKTS_GetSymbolDesc(env, symbol);
    EXPECT_EQ(std::string(retDesc), originDesc);
    ARKTS_FreeCString(retDesc);

    auto obj = ARKTS_CreateObject(env);
    ARKTS_SetProperty(env, obj, symbol, ARKTS_CreateBool(true));
    auto value = ARKTS_GetProperty(env, obj, symbol);
    EXPECT_TRUE(ARKTS_IsBool(value));
    EXPECT_TRUE(ARKTS_GetValueBool(value));

    auto symbol1 = ARKTS_CreateSymbol(env, originDesc, sizeof(originDesc) - 1);
    ARKTS_SetProperty(env, obj, symbol1, ARKTS_CreateBool(false));
    auto value1 = ARKTS_GetProperty(env, obj, symbol1);
    EXPECT_TRUE(ARKTS_IsBool(value1));
    EXPECT_FALSE(ARKTS_GetValueBool(value1));

    value = ARKTS_GetProperty(env, obj, symbol);
    EXPECT_TRUE(ARKTS_IsBool(value));
    EXPECT_TRUE(ARKTS_GetValueBool(value));
}

void ArkInteropTest::TestFunction()
{
    EXPECT_TRUE(MockContext::GetInstance());
    ARKTS_Env env = MockContext::GetInstance()->GetEnv();
    EXPECT_TRUE(env);

    bool called = false;
    auto id = MockContext::GetInstance()->StoreFunc([&called](ARKTS_CallInfo info)->ARKTS_Value {
        called = true;
        EXPECT_EQ(ARKTS_GetArgCount(info), 0);
        EXPECT_TRUE(ARKTS_IsUndefined(ARKTS_GetThisArg(info)));
        return ARKTS_CreateUndefined();
    });
    auto func = ARKTS_CreateFunc(env, id);
    EXPECT_TRUE(ARKTS_IsCallable(env, func));
    EXPECT_EQ(ARKTS_GetValueType(env, func), N_FUNCTION);
    ARKTS_Call(env, func, ARKTS_CreateUndefined(), 0, nullptr);
    EXPECT_TRUE(called);
    called = false;
    ARKTS_Value args[] {
        ARKTS_CreateNull(),
        ARKTS_CreateBool(true),
        ARKTS_CreateF64(45.1),
        ARKTS_CreateObject(env)
    };
    static constexpr auto totalArgs = std::size(args);
    id = MockContext::GetInstance()->StoreFunc([env, &called, origin = args](ARKTS_CallInfo info)->ARKTS_Value {
        called = true;
        auto self = ARKTS_GetThisArg(info);
        EXPECT_TRUE(ARKTS_IsObject(env, self));
        EXPECT_EQ(totalArgs, ARKTS_GetArgCount(info));
        for (auto i = 0; i < totalArgs; i++) {
            EXPECT_TRUE(ARKTS_StrictEqual(env, ARKTS_GetArg(info, i), origin[i]));
        }
        return ARKTS_CreateBool(true);
    });
    func = ARKTS_CreateFunc(env, id);
    ARKTS_Call(env, func, ARKTS_CreateObject(env), std::size(args), args);
    EXPECT_TRUE(called);
}

void ArkInteropTest::TestClass()
{
    EXPECT_TRUE(MockContext::GetInstance());
    ARKTS_Env env = MockContext::GetInstance()->GetEnv();
    EXPECT_TRUE(env);
    auto id = MockContext::GetInstance()->StoreFunc([](ARKTS_CallInfo info)->ARKTS_Value {
        return ARKTS_GetThisArg(info);
    });
    auto clazz = ARKTS_CreateClass(env, id, ARKTS_CreateUndefined());
    EXPECT_TRUE(ARKTS_IsClass(env, clazz));
    EXPECT_EQ(ARKTS_GetValueType(env, clazz), N_FUNCTION);
    auto obj = ARKTS_New(env, clazz, 0, nullptr);
    EXPECT_TRUE(ARKTS_IsObject(env, obj));
    EXPECT_TRUE(ARKTS_InstanceOf(env, obj, clazz));
}

void ArkInteropTest::TestPromise()
{
    EXPECT_TRUE(MockContext::GetInstance());
    ARKTS_Env env = MockContext::GetInstance()->GetEnv();
    EXPECT_TRUE(env);
    auto scope = ARKTS_OpenScope(env);

    auto promiseCap = ARKTS_CreatePromiseCapability(env);
    auto promise = ARKTS_GetPromiseFromCapability(env, promiseCap);
    EXPECT_TRUE(ARKTS_IsPromise(env, promise));
    EXPECT_EQ(ARKTS_GetValueType(env, promise), N_OBJECT);

    bool resolved = false;
    auto id = MockContext::GetInstance()->StoreFunc([&resolved](ARKTS_CallInfo info)->ARKTS_Value {
        resolved = true;
        EXPECT_EQ(ARKTS_GetArgCount(info), 1);
        auto arg = ARKTS_GetArg(info, 0);
        EXPECT_TRUE(ARKTS_IsUndefined(arg));
        return ARKTS_CreateBool(true);
    });
    auto onResolved = ARKTS_CreateFunc(env, id);
    ARKTS_PromiseThen(env, promise, onResolved, ARKTS_CreateUndefined());
    ARKTS_PromiseCapabilityResolve(env, promiseCap, ARKTS_CreateUndefined());
    EXPECT_TRUE(resolved);

    ARKTS_CloseScope(env, scope);
}

void ArkInteropTest::TestArrayBuffer()
{
    EXPECT_TRUE(MockContext::GetInstance());
    ARKTS_Env env = MockContext::GetInstance()->GetEnv();
    EXPECT_TRUE(env);
    auto scope = ARKTS_OpenScope(env);

    uint8_t origin[] {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
        'a', 'b', 'c', 'd', 'e', 'f', 'g'
    };
    auto arrSize = std::size(origin);
    auto buffer = ARKTS_CreateArrayBuffer(env, arrSize);
    EXPECT_TRUE(ARKTS_IsArrayBuffer(env, buffer));
    EXPECT_EQ(ARKTS_GetValueType(env, buffer), N_OBJECT);
    EXPECT_EQ(ARKTS_GetArrayBufferLength(env, buffer), arrSize);
    auto dst = reinterpret_cast<uint8_t*>(ARKTS_GetArrayBufferRawPtr(env, buffer));
    for (auto i = 0; i < arrSize; i++) {
        dst[i] = origin[i];
    }
    uint8_t received[std::size(origin)];
    auto endpoint = ARKTS_ArrayBufferReadBytes(env, buffer, received, arrSize);
    EXPECT_EQ(endpoint, arrSize);

    for (auto i = 0; i < arrSize; i++) {
        EXPECT_EQ(origin[i], received[i]);
    }

    ARKTS_CloseScope(env, scope);
}

void ArkInteropTest::TestUpdateStackInfo()
{
    EXPECT_TRUE(MockContext::GetInstance());
    ARKTS_Env env = MockContext::GetInstance()->GetEnv();
    EXPECT_TRUE(env);
    auto scope = ARKTS_OpenScope(env);
    unsigned long long vmAddress = reinterpret_cast<unsigned long long>(env);
    StackInfo stackInfo;
    stackInfo.stackLimit = 0x5be506e000;
    stackInfo.lastLeaveFrame = 0;
    ARKTS_UpdateStackInfo(vmAddress, &stackInfo, SWITCH_TO_SUB_STACK_INFO);
    ARKTS_UpdateStackInfo(vmAddress, &stackInfo, SWITCH_TO_MAIN_STACK_INFO);
    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, Types)
{
    RunLocalTest();
}

TEST_F(ArkInteropTest, PromiseThen)
{
    MockContext local(ARKTS_CreateEngineWithNewThread());
    auto env = local.GetEnv();
    EXPECT_TRUE(env);

    std::condition_variable cv;
    bool isComplete = false;
    auto funcId = local.StoreAsyncFunc([&local, &cv, &isComplete, env] {
        auto scope = ARKTS_OpenScope(env);
        auto promiseCap = ARKTS_CreatePromiseCapability(env);
        auto promise = ARKTS_GetPromiseFromCapability(env, promiseCap);
        auto funcId = local.StoreFunc([&cv, &isComplete](ARKTS_CallInfo info) {
            isComplete = true;
            cv.notify_one();
            return ARKTS_CreateUndefined();
        });
        auto func = ARKTS_CreateFunc(env, funcId);
        ARKTS_PromiseThen(env, promise, func, ARKTS_CreateUndefined());
        ARKTS_PromiseCapabilityResolve(env, promiseCap, ARKTS_CreateUndefined());
        ARKTS_CloseScope(env, scope);
    });
    ARKTS_CreateAsyncTask(env, funcId);
    std::mutex mutex;
    std::unique_lock lock(mutex);

    constexpr int checkDuration = 10;
    int waitTimes = 10; // set 100ms timeout
    while (!isComplete && waitTimes--) {
        cv.wait_for(lock, std::chrono::milliseconds(checkDuration));
    }
    // EXPECT no core dump, no timeout
    EXPECT_TRUE(waitTimes > 0);
}

TEST_F(ArkInteropTest, CycleFreeFunc)
{
    CycleFreeContext local;
    auto env = local.GetEnv();
    auto funcCount = local.GetCycleFreeFuncCount();
    auto scope = ARKTS_OpenScope(env);
    auto isCalled = false;
    auto id = local.StoreCycleFreeFunc([&isCalled](ARKTS_CallInfo callInfo) {
        isCalled = true;
        return ARKTS_CreateUndefined();
    });
    EXPECT_EQ(local.GetCycleFreeFuncCount(), funcCount + 1);
    auto func = ARKTS_CreateCycleFreeFunc(local.GetEnv(), id);
    EXPECT_TRUE(ARKTS_IsCallable(env, func));
    ARKTS_Call(env, func, ARKTS_CreateUndefined(), 0, nullptr);
    EXPECT_TRUE(isCalled);
    ARKTS_CloseScope(env, scope);
}

TEST_F(ArkInteropTest, CycleFreeExtern)
{
    CycleFreeContext local;
    auto env = local.GetEnv();
    auto funcCount = local.GetCycleFreeFuncCount();
    {
        auto scope = ARKTS_OpenScope(env);
        auto id = local.StoreCycleFreeFunc(nullptr);
        EXPECT_EQ(local.GetCycleFreeFuncCount(), funcCount + 1);
        auto object = ARKTS_CreateCycleFreeExtern(local.GetEnv(), id);
        EXPECT_TRUE(ARKTS_IsExternal(env, object));
        auto handle = ARKTS_GetExternalData(env, object);
        uint32_t resid = reinterpret_cast<uint64_t>(handle);
        EXPECT_EQ(resid, id);

        ARKTS_CloseScope(env, scope);
    }
}

class GlobalWeakTest {
public:
    GlobalWeakTest(): local(ARKTS_CreateEngineWithNewThread()), status(CREATING)
    {
        ScheduleNext();
    }

    void WaitForComplete()
    {
        std::mutex mutex;
        std::unique_lock lock(mutex);
        while (status != COMPLETE) {
            cv.wait(lock);
        }
    }

private:
    static constexpr int objectCnt = 1000;

    void CreateWeakObjects()
    {
        auto env = local.GetEnv();
        auto scope = ARKTS_OpenScope(env);
        for (auto i = 0; i < objectCnt; i++) {
            auto object = ARKTS_CreateObject(env);
            auto global = ARKTS_CreateGlobal(env, object);
            ARKTS_GlobalSetWeak(env, global);
            globals.push_back(global);
        }
        ARKTS_CloseScope(env, scope);
        panda::JSNApi::TriggerGC(P_CAST(env, EcmaVM*), panda::JSNApi::TRIGGER_GC_TYPE::FULL_GC);
    }

    void DoAssertion()
    {
        for (auto one : globals) {
            EXPECT_TRUE(!ARKTS_GlobalIsAlive(local.GetEnv(), one));
        }
    }

    void ReleaseGlobals()
    {
        for (auto one : globals) {
            ARKTS_DisposeGlobalSync(local.GetEnv(), one);
        }
    }

    enum Status {
        CREATING,
        ASSERTION,
        DISPOSE,
        COMPLETE
    };

    void ScheduleNext()
    {
        auto id = local.StoreAsyncFunc([this] {
            DoNext();
        });
        ARKTS_CreateAsyncTask(local.GetEnv(), id);
    }

    void DoNext()
    {
        switch (status) {
            case CREATING:
                CreateWeakObjects();
                status = ASSERTION;
                break;
            case ASSERTION:
                DoAssertion();
                status = DISPOSE;
                break;
            case DISPOSE:
                ReleaseGlobals();
                status = COMPLETE;
                cv.notify_all();
                return;
            default: ;
        }
        ScheduleNext();
    }

    CycleFreeContext local;
    Status status;
    std::condition_variable cv;
    std::vector<ARKTS_Global> globals;
};

TEST_F(ArkInteropTest, GlobalWeak)
{
    GlobalWeakTest weakTest;
    weakTest.WaitForComplete();
}

TEST_F(ArkInteropTest, GlobalToValue)
{
    MockContext local;
    auto env = local.GetEnv();
    {
        auto scope = ARKTS_OpenScope(env);
        auto object = ARKTS_CreateObject(env);
        auto global = ARKTS_CreateGlobal(env, object);
        auto value = ARKTS_GlobalToValue(env, global);
        auto received = ARKTS_GlobalFromValue(env, value);
        EXPECT_EQ(received, global);
        ARKTS_DisposeGlobalSync(env, global);
        ARKTS_CloseScope(env, scope);
    }
}

HWTEST_F(ArkInteropTest, ArkTSInteropNapiCreateEngineNew, TestSize.Level1)
{
    MockContext local(ARKTS_CreateEngineWithNewThread());

    auto curTid = ARKTS_GetPosixThreadId();
    auto engineTid = ARKTS_GetThreadIdOfEngine(local.GetEngine());

    EXPECT_NE(curTid, engineTid);

    auto env = local.GetEnv();
    EXPECT_TRUE(env);
    bool isComplete = false;
    std::condition_variable cv;
    auto funcId = local.StoreAsyncFunc([&isComplete, &cv] {
        ArkInteropTest::RunLocalTest();
        isComplete = true;
        cv.notify_one();
    });
    ARKTS_CreateAsyncTask(env, funcId);
    std::mutex mutex;
    std::unique_lock lock(mutex);
    constexpr int checkDuration = 10;
    int waitTimes = 100; // set 1000ms timeout
    while (!isComplete && waitTimes--) {
        cv.wait_for(lock, std::chrono::milliseconds(checkDuration));
    }
    EXPECT_TRUE(waitTimes > 0);
}

TEST_F(ArkInteropTest, ScopeMT)
{
    constexpr int threadCount = 1000;
    std::thread threads[threadCount];
    for (int i = 0; i < threadCount; i++) {
        threads[i] = std::thread([] {
            panda::RuntimeOption options;
            auto vm = panda::JSNApi::CreateJSVM(options);
            EXPECT_TRUE(vm);
            auto env = P_CAST(vm, ARKTS_Env);
            auto scope = ARKTS_OpenScope(env);
            EXPECT_TRUE(scope);
            ARKTS_CloseScope(env, scope);
            panda::JSNApi::DestroyJSVM(vm);
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }
}

TEST_F(ArkInteropTest, GlobalRelease)
{
    MockContext local(ARKTS_CreateEngineWithNewThread());
    auto env = local.GetEnv();

    bool isComplete = false;
    int loops = 10;
    std::condition_variable cv;
    std::function <void()> callback;
    callback = [&isComplete, &cv, &loops, env, &callback] {
        if (loops == 0) {
            isComplete = true;
            cv.notify_one();
            return;
        }
        auto scope = ARKTS_OpenScope(env);
        auto totalRepeat = 500000;
        for (int i = 0;i < totalRepeat; ++i) {
            auto object = ARKTS_CreateObject(env);
            auto global = ARKTS_CreateGlobal(env, object);
            ARKTS_DisposeGlobal(env, global);
        }
        ARKTS_CloseScope(env, scope);
        --loops;
        auto funcId = MockContext::GetInstance()->StoreAsyncFunc(callback);
        ARKTS_CreateAsyncTask(env, funcId);
    };
    auto funcId = local.StoreAsyncFunc(callback);
    ARKTS_CreateAsyncTask(env, funcId);
    std::mutex mutex;
    std::unique_lock lock(mutex);
    int waitTimes = 200;
    int msEachTime = 100;
    while (!isComplete && waitTimes--) {
        cv.wait_for(lock, std::chrono::milliseconds(msEachTime));
    }
    EXPECT_TRUE(isComplete);
}

TEST_F(ArkInteropTest, GlobalReleaseSync)
{
    MockContext local(ARKTS_CreateEngineWithNewThread());
    auto env = local.GetEnv();
    int loops = 10;
    auto totalRepeat = 500000;
    for (int i = 0;i < loops; ++i) {
        auto scope = ARKTS_OpenScope(env);
        for (int j = 0;j < totalRepeat; ++j) {
            auto object = ARKTS_CreateObject(env);
            auto global = ARKTS_CreateGlobal(env, object);
            ARKTS_DisposeGlobalSync(env, global);
        }
        EXPECT_FALSE(panda::JSNApi::HasPendingException(P_CAST(env, EcmaVM*)));
        ARKTS_CloseScope(env, scope);
    }
}

TEST_F(ArkInteropTest, PromiseRelease)
{
    MockContext local;
    auto env = local.GetEnv();
    int loops = 20;
    auto totalRepeat = 50000;
    auto undefined = ARKTS_CreateUndefined();
    for (int i = 0;i < loops; ++i) {
        auto scope = ARKTS_OpenScope(env);
        for (int j = 0;j < totalRepeat; ++j) {
            auto promiseCap = ARKTS_CreatePromiseCapability(env);
            ARKTS_PromiseCapabilityResolve(env, promiseCap, undefined);
        }
        EXPECT_FALSE(panda::JSNApi::HasPendingException(P_CAST(env, EcmaVM*)));
        ARKTS_CloseScope(env, scope);
    }
}

TEST_F(ArkInteropTest, CheckFreeContext)
{
    bool isDisposed = false;
    {
        MockContext local(ARKTS_CreateEngineWithNewThread());
        local.SetFinalizerCallback([&isDisposed](ARKTS_Env) {
            isDisposed = true;
        });
    }
    EXPECT_TRUE(isDisposed);
}
} // namespace

int main(int argc, char** argv)
{
    LOGI("main in");
    testing::GTEST_FLAG(output) = "xml:./";
    testing::InitGoogleTest(&argc, argv);

    auto runner = OHOS::AppExecFwk::EventRunner::Create(true);
    EXPECT_TRUE(runner);
    auto handler = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    EXPECT_TRUE(handler);

    MockContext::Init();

    int ret = -1;
    std::condition_variable cv;

    ARKTS_Engine globalEngine;

    auto success = handler->PostTask([&ret, &cv, &globalEngine] {
        MockContext global(ARKTS_CreateEngine(), false);
        globalEngine = global.GetEngine();
        ret = testing::UnitTest::GetInstance()->Run();
        cv.notify_all();
    });

    EXPECT_TRUE(success);

    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    auto status = cv.wait_for(lock, std::chrono::seconds(30));

    EXPECT_EQ(status, std::cv_status::no_timeout);
    ARKTS_DestroyEngine(globalEngine);

    runner->Stop();

    LOGI("main out");
    return ret;
}
