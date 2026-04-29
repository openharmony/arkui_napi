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

#include "test_ark_interop_common.h"

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

MockContext* MockContext::instance_ = nullptr;
Slab<std::function<ARKTS_Value(ARKTS_CallInfo)>> CycleFreeContext::callbacks_;
std::mutex CycleFreeContext::callbackMutex_;

uint32_t MockContext::NewId()
{
    static uint32_t id = 0;
    return id++;
}

MockContext* MockContext::GetInstance()
{
    return instance_;
}

ARKTS_Engine MockContext::GetEngine() const
{
    return engine_;
}

ARKTS_Env MockContext::GetEnv() const
{
    return ARKTS_GetContext(engine_);
}

void MockContext::Init()
{
    InitModuleCallbacks();
    InitCycleFreeCallbacks();
    InitDebugModeCallbacks();
}

MockContext::MockContext(ARKTS_Engine engine, bool needDestroyEngine)
    : idPrefix_(NewId()),
      lastContext_(instance_),
      engine_(engine),
      needDestroyEngine_(needDestroyEngine)
{
    instance_ = this;
}

MockContext::MockContext()
    : idPrefix_(NewId()),
      lastContext_(instance_),
      engine_(instance_->GetEngine()),
      needDestroyEngine_(false)
{
    instance_ = this;
}

MockContext::~MockContext()
{
    if (needDestroyEngine_) {
        ARKTS_DestroyEngine(engine_);
    }
    engine_ = nullptr;
    instance_ = lastContext_;
}

int64_t MockContext::StoreFunc(std::function<ARKTS_Value(ARKTS_CallInfo)> call)
{
    return ToId(callbacks_.Add(std::move(call)));
}

int64_t MockContext::StoreAsyncFunc(std::function<void()> call)
{
    return ToId(asyncCallbacks_.Add(std::move(call)));
}

int64_t MockContext::StoreArrayBufferDeleter(std::function<void(void*)> call)
{
    return ToId(nativeDeleters_.Add(std::move(call)));
}

int64_t MockContext::StoreNativeData(void* data, std::function<void(void*)> deleter)
{
    return ToId(data_.Add({data, std::move(deleter)}));
}

void* MockContext::GetFuncAddr(int64_t id)
{
    auto index = ToIndex(id);
    if (data_.Has(index)) {
        return &data_.Get(index);
    }
    return nullptr;
}

void MockContext::SetNativeCallingCallback(std::function<void(const void*)> callback)
{
    nativeCallingCallback_ = std::move(callback);
}

void MockContext::SetNativeReturnCallback(std::function<void(const void*)> callback)
{
    nativeReturnCallback_ = std::move(callback);
}

void MockContext::NotifyNativeCalling(const void* addr)
{
    if (nativeCallingCallback_) {
        nativeCallingCallback_(addr);
    }
}

void MockContext::NotifyNativeReturn(const void* addr)
{
    if (nativeReturnCallback_) {
        nativeReturnCallback_(addr);
    }
}

void* MockContext::GetNativeData(int64_t id)
{
    auto index = ToIndex(id);
    if (index < 0) {
        return nullptr;
    }
    return data_.Get(index).data;
}

void MockContext::SetFinalizerCallback(std::function<void(ARKTS_Env)> callback)
{
    finalizerCallback_ = std::move(callback);
}

MockContextKind MockContext::GetContextKind() const
{
    return MockContextKind::NORMAL;
}

ARKTS_Value MockContext::ExportModule(ARKTS_Env env, const char* dllName, ARKTS_Value exports)
{
    return exports;
}

bool MockContext::HasModuleHandle(const char* dllName)
{
    return false;
}

void MockContext::ThrowJSError(ARKTS_Env env, ARKTS_Value arktsValue)
{
    (void)env;
    (void)arktsValue;
    EXPECT_TRUE(false);
}

void MockContext::ThrowNativeError(const char* msg)
{
    printf("[Native Error]: %s\n", msg);
    EXPECT_TRUE(false);
}

void MockContext::DeleteArrayBufferRawData(void* buffer, int64_t id)
{
    auto index = ToIndex(id);
    if (index < 0) {
        return;
    }
    nativeDeleters_.Get(index)(buffer);
    nativeDeleters_.Del(index);
}

void MockContext::DeleteExternal(int64_t id, ARKTS_Env env)
{
    auto index = ToIndex(id);
    if (index < 0) {
        return;
    }
    auto& data = data_.Get(index);
    data.deleter(data.data);
    data_.Del(index);
}

ARKTS_Value MockContext::InvokeLambda(ARKTS_CallInfo info, int64_t id)
{
    auto index = ToIndex(id);
    if (index < 0) {
        return ARKTS_CreateUndefined();
    }
    return callbacks_.Get(index)(info);
}

void MockContext::DeleteLambda(ARKTS_Env env, int64_t id)
{
    auto index = ToIndex(id);
    if (index < 0) {
        return;
    }
    callbacks_.Del(index);
}

void MockContext::InvokeAsyncLambda(ARKTS_Env env, int64_t id)
{
    auto index = ToIndex(id);
    if (index < 0) {
        return;
    }
    asyncCallbacks_.Get(index)();
    asyncCallbacks_.Del(index);
}

void MockContext::DeleteJSContext(ARKTS_Env env)
{
    if (finalizerCallback_) {
        finalizerCallback_(env);
    }
}

ARKTS_Value MockContext::InvokeCycleFreeFunc(ARKTS_CallInfo callInfo, uint32_t id)
{
    return ARKTS_CreateUndefined();
}

void MockContext::ReleaseCycleFreeExt(uint32_t id)
{
}

int64_t MockContext::ToId(int64_t index) const
{
    return GetPrefixMask() | index;
}

int64_t MockContext::ToIndex(int64_t id) const
{
    auto myMask = GetPrefixMask();
    if ((id & myMask) == myMask) {
        return id & 0xFFFF'FFFF;
    }
    return -1;
}

void MockContext::InitModuleCallbacks()
{
    static ARKTS_ModuleCallbacks callbacks {
        .exportModule = [](ARKTS_Env env, const char* dllName, ARKTS_Value exports) -> ARKTS_Value {
            return instance_ ? instance_->ExportModule(env, dllName, exports) : exports;
        },
        .hasModuleHandle = [](const char* dllName) -> bool {
            return instance_ ? instance_->HasModuleHandle(dllName) : false;
        },
        .throwJSError = [](ARKTS_Env env, ARKTS_Value error) -> void {
            auto* inst = MockContext::GetInstance();
            if (!inst) {
                return;
            }
            if (inst->GetContextKind() == MockContextKind::ERROR_CAPTURE) {
                static_cast<ErrorCaptureContext*>(inst)->ReportCapturedJSError(env, error);
                return;
            }
            inst->ThrowJSError(env, error);
        },
        .throwNativeError = [](const char* error) -> void {
            auto* inst = MockContext::GetInstance();
            if (!inst) {
                return;
            }
            if (inst->GetContextKind() == MockContextKind::ERROR_CAPTURE) {
                static_cast<ErrorCaptureContext*>(inst)->ReportCapturedNativeError(error);
                return;
            }
            inst->ThrowNativeError(error);
        },
        .deleteArrayBufferRawData = [](void* buffer, int64_t lambdaId) -> void {
            if (instance_) instance_->DeleteArrayBufferRawData(buffer, lambdaId);
        },
        .deleteExternal = [](int64_t id, ARKTS_Env env) -> void {
            if (instance_) instance_->DeleteExternal(id, env);
        },
        .invokerLambda = [](ARKTS_CallInfo callInfo, int64_t lambdaId) -> ARKTS_Value {
            return instance_ ? instance_->InvokeLambda(callInfo, lambdaId) : ARKTS_CreateUndefined();
        },
        .deleteLambda = [](ARKTS_Env env, int64_t id) -> void {
            if (instance_) instance_->DeleteLambda(env, id);
        },
        .invokeAsyncLambda = [](ARKTS_Env env, int64_t id) -> void {
            if (instance_) instance_->InvokeAsyncLambda(env, id);
        },
        .deleteJSContext = [](ARKTS_Env env) -> void {
            if (instance_) instance_->DeleteJSContext(env);
        }
    };
    ARKTS_SetCJModuleCallback(&callbacks);
}

void MockContext::InitCycleFreeCallbacks()
{
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

void MockContext::InitDebugModeCallbacks()
{}

int64_t MockContext::GetPrefixMask() const
{
    constexpr auto idBits = 32;
    return static_cast<int64_t>(idPrefix_) << idBits;
}

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
    double g_origins[] {
        0.1,
        -12.1,
        12456.126546
    };
    constexpr auto totalCases = std::size(g_origins);
    ARKTS_Value jsValues[totalCases];
    double received[totalCases];

    for (size_t g_i = 0;g_i < totalCases; g_i++) {
        jsValues[g_i] = ARKTS_CreateF64(g_origins[g_i]);
        EXPECT_EQ(ARKTS_GetValueType(env, jsValues[g_i]), N_NUMBER);
        EXPECT_TRUE(ARKTS_IsNumber(jsValues[g_i]));
        received[g_i] = ARKTS_GetValueNumber(jsValues[g_i]);
        EXPECT_EQ(g_origins[g_i], received[g_i]);
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
    const char* g_origins[] {
        "a plain text",
        "`~!@#$%^&*()_+[]\\",
        "中文字符",
        "😊😂🤣❤️😍😒👌😘",
    };
    ARKTS_Value jsValues[] {
        ARKTS_CreateUtf8(env, g_origins[0], strlen(g_origins[0])),
        ARKTS_CreateUtf8(env, g_origins[1], strlen(g_origins[1])),
        ARKTS_CreateUtf8(env, g_origins[2], strlen(g_origins[2])),
        ARKTS_CreateUtf8(env, g_origins[3], strlen(g_origins[3])),
    };
    EXPECT_TRUE(ARKTS_IsString(env, jsValues[0]));
    EXPECT_TRUE(ARKTS_IsHeapObject(jsValues[0]));
    EXPECT_EQ(ARKTS_GetValueType(env, jsValues[0]), N_STRING);
    for (auto i = 0; i < sizeof(jsValues)/sizeof(ARKTS_Value); i++) {
        auto size = ARKTS_GetValueUtf8Size(env, jsValues[i]);
        std::string g_result;
        g_result.resize(size - 1);
        ARKTS_GetValueUtf8(env, jsValues[i], size - 1, g_result.data());
        EXPECT_EQ(g_result, g_origins[i]);

        auto cStr = ARKTS_GetValueCString(env, jsValues[i]);
        g_result = cStr;
        EXPECT_EQ(g_result, g_origins[i]);
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
    constexpr int32_t NUMERIC_KEY = 12;
    ARKTS_Value allowedKeys[] {
        ARKTS_CreateUtf8(env, "a", 1),
        ARKTS_CreateI32(NUMERIC_KEY),
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

void ArkInteropTest::TestDefineProperty()
{
    ErrorCaptureContext g_errorCtx;
    auto env = g_errorCtx.GetEnv();
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
    constexpr int EXPECT_KEYS = 4;
    auto jsKeys = ARKTS_EnumOwnProperties(env, obj);
    EXPECT_TRUE(ARKTS_IsArray(env, jsKeys));
    EXPECT_EQ(ARKTS_GetArrayLength(env, jsKeys), EXPECT_KEYS);
    for (auto i = 0; i < EXPECT_KEYS; i++) {
        auto jsKey = ARKTS_GetElement(env, jsKeys, i);
        EXPECT_TRUE(ARKTS_IsString(env, jsKey));
        char g_buffer = 0;
        ARKTS_GetValueUtf8(env, jsKey, 1, &g_buffer);
        EXPECT_TRUE(g_buffer == 'c' || g_buffer == 'e' || g_buffer == 'g' || g_buffer == 'h');
    }

    ARKTS_CloseScope(env, scope);
}

void ArkInteropTest::TestDefinePropertyWritable()
{
    ErrorCaptureContext g_errorCtx;
    auto env = g_errorCtx.GetEnv();
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
    ErrorCaptureContext errorCtx;
    auto env = errorCtx.GetEnv();
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
    constexpr int EXPECT_LENGTH = 6;
    EXPECT_EQ(ARKTS_GetArrayLength(env, jsKeys), EXPECT_LENGTH);

    ARKTS_CloseScope(env, scope);
}

void ArkInteropTest::TestDefinePropertyConfigurableV2()
{
    ErrorCaptureContext errorCtx;
    auto env = errorCtx.GetEnv();
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
            EXPECT_TRUE(errorCtx.HasAndClearJSError());
        }
    }
    auto jsKeys = ARKTS_EnumOwnProperties(env, obj);
    constexpr int EXPECT_LENGTH_V2 = 6;
    EXPECT_EQ(ARKTS_GetArrayLength(env, jsKeys), EXPECT_LENGTH_V2);

    ARKTS_CloseScope(env, scope);
}

void ArkInteropTest::TestInstanceOf()
{
    ARKTS_Env env = MockContext::GetInstance()->GetEnv();
    auto global = ARKTS_GetGlobalConstant(env);
    char g_clError[] = "Error";
    auto jError = ARKTS_CreateUtf8(env, g_clError, sizeof(g_clError) - 1);
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

    char g_c = 'c';
    constexpr double TEST_DOUBLE_VAL = 12.02;
    ARKTS_Value origin[] {
        ARKTS_CreateBool(true),
        ARKTS_CreateF64(TEST_DOUBLE_VAL),
        ARKTS_CreateUtf8(env, &g_c, 1),
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
            BIGINT_POS_VAL,
            BIGINT_NEG_VAL,
            BIGINT_MAX_VAL,
            BIGINT_MIN_VAL,
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
    bool g_isNegative = false;
    auto value = ARKTS_CreateBigIntWithBytes(env, g_isNegative, std::size(origin), origin);
    EXPECT_TRUE(ARKTS_IsBigInt(env, value));
    EXPECT_EQ(ARKTS_GetValueType(env, value), N_BIGINT);
    EXPECT_EQ(ARKTS_BigIntGetByteSize(env, value), std::size(origin));
    uint8_t received[std::size(origin)];
    ARKTS_BigIntReadBytes(env, value, &g_isNegative, std::size(origin), received);
    EXPECT_FALSE(g_isNegative);
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
    bool g_isNegative = false;
    constexpr int EXPECT_SIZE = 32;
    auto value = ARKTS_CreateBigIntWithBytes(env, g_isNegative, std::size(origin), origin);
    EXPECT_TRUE(ARKTS_IsBigInt(env, value));
    EXPECT_EQ(ARKTS_GetValueType(env, value), N_BIGINT);
    EXPECT_EQ(ARKTS_BigIntGetByteSize(env, value), EXPECT_SIZE);
    uint8_t received[EXPECT_SIZE];
    ARKTS_BigIntReadBytes(env, value, &g_isNegative, EXPECT_SIZE, received);
    EXPECT_FALSE(g_isNegative);
    for (auto i = 0; i < std::size(origin); i++) {
        EXPECT_EQ(origin[i], received[i + EXPECT_SIZE - std::size(origin)]);
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
    bool g_isNegative32 = false;
    auto value = ARKTS_CreateBigIntWithBytes(env, g_isNegative32, std::size(origin), origin);
    EXPECT_TRUE(ARKTS_IsBigInt(env, value));
    EXPECT_EQ(ARKTS_GetValueType(env, value), N_BIGINT);
    EXPECT_EQ(ARKTS_BigIntGetByteSize(env, value), std::size(origin));
    uint8_t received[std::size(origin)];
    ARKTS_BigIntReadBytes(env, value, &g_isNegative32, std::size(origin), received);
    EXPECT_FALSE(g_isNegative32);
    for (auto i = 0; i < std::size(origin); i++) {
        EXPECT_EQ(origin[i], received[i]);
    }
}

void ArkInteropTest::TestSymbol()
{
    EXPECT_TRUE(MockContext::GetInstance());
    ARKTS_Env env = MockContext::GetInstance()->GetEnv();
    EXPECT_TRUE(env);

    const char ORIGIN_DESC[] = "TestSymbol";

    auto symbol = ARKTS_CreateSymbol(env, ORIGIN_DESC, sizeof(ORIGIN_DESC) - 1);
    EXPECT_TRUE(ARKTS_IsSymbol(env, symbol));
    EXPECT_EQ(ARKTS_GetValueType(env, symbol), N_SYMBOL);

    auto retDesc = ARKTS_GetSymbolDesc(env, symbol);
    EXPECT_EQ(std::string(retDesc), ORIGIN_DESC);
    ARKTS_FreeCString(retDesc);

    auto obj = ARKTS_CreateObject(env);
    ARKTS_SetProperty(env, obj, symbol, ARKTS_CreateBool(true));
    auto value = ARKTS_GetProperty(env, obj, symbol);
    EXPECT_TRUE(ARKTS_IsBool(value));
    EXPECT_TRUE(ARKTS_GetValueBool(value));

    auto symbol1 = ARKTS_CreateSymbol(env, ORIGIN_DESC, sizeof(ORIGIN_DESC) - 1);
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

    bool g_called = false;
    constexpr size_t ZERO_ARG_COUNT = 0;
    auto id = MockContext::GetInstance()->StoreFunc(
        [&g_called, env, ZERO_ARG_COUNT](ARKTS_CallInfo info)->ARKTS_Value {
        g_called = true;
        EXPECT_EQ(ARKTS_GetArgCount(info), ZERO_ARG_COUNT);
        EXPECT_TRUE(ARKTS_IsUndefined(ARKTS_GetThisArg(info)));
        return ARKTS_CreateUndefined();
    });
    auto func = ARKTS_CreateFunc(env, id);
    EXPECT_TRUE(ARKTS_IsCallable(env, func));
    EXPECT_EQ(ARKTS_GetValueType(env, func), N_FUNCTION);
    ARKTS_Call(env, func, ARKTS_CreateUndefined(), 0, nullptr);
    EXPECT_TRUE(g_called);
    g_called = false;
    constexpr double CALL_ARG_DOUBLE = 45.1;
    ARKTS_Value args[] {
        ARKTS_CreateNull(),
        ARKTS_CreateBool(true),
        ARKTS_CreateF64(CALL_ARG_DOUBLE),
        ARKTS_CreateObject(env)
    };
    static constexpr auto totalArgs = std::size(args);
    id = MockContext::GetInstance()->StoreFunc([env, &g_called, origin = args](ARKTS_CallInfo info)->ARKTS_Value {
        g_called = true;
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
    EXPECT_TRUE(g_called);
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

    bool g_resolved = false;
    constexpr size_t SINGLE_ARG_COUNT = 1;
    auto id = MockContext::GetInstance()->StoreFunc(
        [&g_resolved, env, SINGLE_ARG_COUNT](ARKTS_CallInfo info)->ARKTS_Value {
        g_resolved = true;
        EXPECT_EQ(ARKTS_GetArgCount(info), SINGLE_ARG_COUNT);
        auto arg = ARKTS_GetArg(info, 0);
        EXPECT_TRUE(ARKTS_IsUndefined(arg));
        return ARKTS_CreateBool(true);
    });
    auto onResolved = ARKTS_CreateFunc(env, id);
    ARKTS_PromiseThen(env, promise, onResolved, ARKTS_CreateUndefined());
    ARKTS_PromiseCapabilityResolve(env, promiseCap, ARKTS_CreateUndefined());
    EXPECT_TRUE(g_resolved);

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
    auto arrBuf = ARKTS_CreateArrayBuffer(env, arrSize);
    EXPECT_TRUE(ARKTS_IsArrayBuffer(env, arrBuf));
    EXPECT_EQ(ARKTS_GetValueType(env, arrBuf), N_OBJECT);
    EXPECT_EQ(ARKTS_GetArrayBufferLength(env, arrBuf), arrSize);
    auto dst = reinterpret_cast<uint8_t*>(ARKTS_GetArrayBufferRawPtr(env, arrBuf));
    for (auto g_idx = 0; g_idx < arrSize; g_idx++) {
        dst[g_idx] = origin[g_idx];
    }
    uint8_t received[std::size(origin)];
    auto endpoint = ARKTS_ArrayBufferReadBytes(env, arrBuf, received, arrSize);
    EXPECT_EQ(endpoint, arrSize);

    for (auto g_idx = 0; g_idx < arrSize; g_idx++) {
        EXPECT_EQ(origin[g_idx], received[g_idx]);
    }

    ARKTS_CloseScope(env, scope);
}

void ArkInteropTest::TestUpdateStackInfo()
{
    EXPECT_TRUE(MockContext::GetInstance());
    ARKTS_Env env = MockContext::GetInstance()->GetEnv();
    EXPECT_TRUE(env);
    auto scope = ARKTS_OpenScope(env);
    unsigned long long g_vmAddress = reinterpret_cast<unsigned long long>(env);
    StackInfo g_stackInfo;
    g_stackInfo.stackLimit = TEST_STACK_LIMIT;
    g_stackInfo.lastLeaveFrame = 0;
    ARKTS_UpdateStackInfo(g_vmAddress, &g_stackInfo, SWITCH_TO_SUB_STACK_INFO);
    ARKTS_UpdateStackInfo(g_vmAddress, &g_stackInfo, SWITCH_TO_MAIN_STACK_INFO);
    ARKTS_CloseScope(env, scope);
}

void TriggerBasicAssertBranches(AssertTestContext& ctx)
{
    (void)ARKTS_GetGlobalConstant(ctx.nullEnv);
    (void)ARKTS_GetGlobalNapiEnv(ctx.nullEnv);
    (void)ARKTS_GetValueType(ctx.nullEnv, ctx.zeroValue);
    (void)ARKTS_StrictEqual(ctx.nullEnv, ctx.zeroValue, ctx.zeroValue);
}

void TriggerStringAssertBranches(AssertTestContext& ctx)
{
    (void)ARKTS_GetValueUtf8(ctx.nullEnv, ctx.zeroValue, 0, ctx.smallBuf);
    (void)ARKTS_GetValueUtf8Size(ctx.nullEnv, ctx.zeroValue);
    (void)ARKTS_CreateUtf8(ctx.nullEnv, nullptr, 0);
    (void)ARKTS_GetValueCString(ctx.nullEnv, ctx.zeroValue);
    (void)ARKTS_IsString(ctx.nullEnv, ctx.zeroValue);
    (void)ARKTS_GetStringInfo(ctx.nullEnv, ctx.zeroValue);
    ARKTS_StringCopy(ctx.nullEnv, ctx.zeroValue, ctx.smallBuf, 0);
    (void)ARKTS_CreateString(ctx.nullEnv, false, 0, nullptr);
    if (ctx.normalEnv) {
        (void)ARKTS_CreateUtf8(ctx.normalEnv, nullptr, 0);
        (void)ARKTS_IsString(ctx.normalEnv, ctx.zeroValue);
        (void)ARKTS_GetValueUtf8Size(ctx.normalEnv, ctx.zeroValue);
        (void)ARKTS_GetValueUtf8(ctx.normalEnv, ctx.zeroValue, 1, ctx.smallBuf);
        (void)ARKTS_GetValueCString(ctx.normalEnv, ctx.zeroValue);
        (void)ARKTS_CreateString(ctx.normalEnv, false, 0, nullptr);
        auto oneChar = ARKTS_CreateUtf8(ctx.normalEnv, "a", 1);
        (void)ARKTS_GetValueUtf8(ctx.normalEnv, oneChar, 1, nullptr);
    }
}

void TriggerFunctionClassAssertBranches(AssertTestContext& ctx)
{
    (void)ARKTS_CreateFunc(ctx.nullEnv, 0);
    (void)ARKTS_CreateExternal(ctx.nullEnv, 0);
    (void)ARKTS_IsCallable(ctx.nullEnv, ctx.zeroValue);
    (void)ARKTS_IsClass(ctx.nullEnv, ctx.zeroValue);
    (void)ARKTS_CreateClass(ctx.nullEnv, 0, ctx.zeroValue);
    (void)ARKTS_GetPrototype(ctx.nullEnv, ctx.zeroValue);
    (void)ARKTS_InstanceOf(ctx.nullEnv, ctx.zeroValue, ctx.zeroValue);
}

void TriggerCallInfoAssertBranches(AssertTestContext& ctx)
{
    (void)ARKTS_GetCallEnv(ctx.nullInfo);
    (void)ARKTS_GetArgCount(ctx.nullInfo);
    (void)ARKTS_GetArg(ctx.nullInfo, 0);
    (void)ARKTS_GetThisArg(ctx.nullInfo);
}

void TriggerCallNewAssertBranches(AssertTestContext& ctx)
{
    (void)ARKTS_Call(ctx.nullEnv, ctx.zeroValue, ctx.zeroValue, 0, nullptr);
    (void)ARKTS_New(ctx.nullEnv, ctx.zeroValue, 0, nullptr);
}

void TriggerObjectAssertBranches(AssertTestContext& ctx)
{
    (void)ARKTS_CreateObject(ctx.nullEnv);
    (void)ARKTS_IsObject(ctx.nullEnv, ctx.zeroValue);
    (void)ARKTS_HasOwnProperty(ctx.nullEnv, ctx.zeroValue, ctx.zeroValue);
    (void)ARKTS_EnumOwnProperties(ctx.nullEnv, ctx.zeroValue);
    ARKTS_DefineOwnProperty(ctx.nullEnv, ctx.zeroValue, ctx.zeroValue, ctx.zeroValue, N_WRITABLE);
    (void)ARKTS_DefineOwnPropertyV2(ctx.nullEnv, ctx.zeroValue, ctx.zeroValue, ctx.zeroValue, N_WRITABLE);
    ARKTS_DefineAccessors(ctx.nullEnv, ctx.zeroValue, ctx.zeroValue, ctx.accessor);
    (void)ARKTS_DefineAccessorsV2(ctx.nullEnv, ctx.zeroValue, ctx.zeroValue, ctx.accessor);
    ARKTS_SetProperty(ctx.nullEnv, ctx.zeroValue, ctx.zeroValue, ctx.zeroValue);
    (void)ARKTS_GetProperty(ctx.nullEnv, ctx.zeroValue, ctx.zeroValue);
    if (ctx.normalEnv) {
        auto invalidObj = ARKTS_CreateUndefined();
        auto invalidKey = ARKTS_CreateUndefined();
        (void)ARKTS_IsObject(ctx.normalEnv, ctx.zeroValue);
        (void)ARKTS_HasOwnProperty(ctx.normalEnv, invalidObj, invalidKey);
        (void)ARKTS_EnumOwnProperties(ctx.normalEnv, invalidObj);
        ARKTS_DefineOwnProperty(ctx.normalEnv, invalidObj, invalidKey, ctx.zeroValue, N_WRITABLE);
        (void)ARKTS_DefineOwnPropertyV2(ctx.normalEnv, invalidObj, invalidKey, ctx.zeroValue, N_WRITABLE);
        ARKTS_DefineAccessors(ctx.normalEnv, invalidObj, invalidKey, ctx.accessor);
        (void)ARKTS_DefineAccessorsV2(ctx.normalEnv, invalidObj, invalidKey, ctx.accessor);
        ARKTS_SetProperty(ctx.normalEnv, invalidObj, invalidKey, ctx.zeroValue);
        (void)ARKTS_GetProperty(ctx.normalEnv, invalidObj, invalidKey);
        auto validObj = ARKTS_CreateObject(ctx.normalEnv);
        (void)ARKTS_HasOwnProperty(ctx.normalEnv, validObj, invalidKey);
        ARKTS_DefineOwnProperty(ctx.normalEnv, validObj, invalidKey, ctx.zeroValue, N_WRITABLE);
        (void)ARKTS_DefineOwnPropertyV2(ctx.normalEnv, validObj, invalidKey, ctx.zeroValue, N_WRITABLE);
        ARKTS_DefineAccessors(ctx.normalEnv, validObj, invalidKey, ctx.accessor);
        (void)ARKTS_DefineAccessorsV2(ctx.normalEnv, validObj, invalidKey, ctx.accessor);
        ARKTS_SetProperty(ctx.normalEnv, validObj, invalidKey, ctx.zeroValue);
        (void)ARKTS_GetProperty(ctx.normalEnv, validObj, invalidKey);
    }
}

void TriggerArrayAssertBranches(AssertTestContext& ctx)
{
    (void)ARKTS_CreateArray(ctx.nullEnv, 0);
    (void)ARKTS_CreateArrayWithInit(ctx.nullEnv, 0, ctx.oneArg);
    (void)ARKTS_GetArrayLength(ctx.nullEnv, ctx.zeroValue);
    ARKTS_SetElement(ctx.nullEnv, ctx.zeroValue, 0, ctx.zeroValue);
    (void)ARKTS_GetElement(ctx.nullEnv, ctx.zeroValue, 0);
    (void)ARKTS_IsArray(ctx.nullEnv, ctx.zeroValue);
}

void TriggerGlobalAssertBranches(AssertTestContext& ctx)
{
    (void)ARKTS_CreateGlobal(ctx.nullEnv, ctx.zeroValue);
    (void)ARKTS_GlobalIsAlive(ctx.nullEnv, ctx.nullGlobal);
    if (ctx.normalEnv) {
        (void)ARKTS_GlobalIsAlive(ctx.normalEnv, ctx.nullGlobal);
    }
    (void)ARKTS_GetGlobalValue(ctx.nullGlobal);
    ARKTS_DisposeGlobal(ctx.nullEnv, ctx.nullGlobal);
    if (ctx.normalEnv) {
        ARKTS_DisposeGlobal(ctx.normalEnv, ctx.nullGlobal);
    }
    ARKTS_DisposeGlobalSync(ctx.nullEnv, ctx.nullGlobal);
    if (ctx.normalEnv) {
        ARKTS_DisposeGlobalSync(ctx.normalEnv, ctx.nullGlobal);
    }
    ARKTS_GlobalSetWeak(ctx.nullEnv, ctx.nullGlobal);
    if (ctx.normalEnv) {
        ARKTS_GlobalSetWeak(ctx.normalEnv, ctx.nullGlobal);
    }
    ARKTS_GlobalClearWeak(ctx.nullEnv, ctx.nullGlobal);
    if (ctx.normalEnv) {
        ARKTS_GlobalClearWeak(ctx.normalEnv, ctx.nullGlobal);
    }
    (void)ARKTS_GlobalToValue(ctx.nullEnv, ctx.nullGlobal);
    if (ctx.normalEnv) {
        (void)ARKTS_GlobalToValue(ctx.normalEnv, ctx.nullGlobal);
    }
    (void)ARKTS_GlobalFromValue(ctx.nullEnv, ctx.zeroValue);
}

void TriggerSymbolScopeAssertBranches(AssertTestContext& ctx)
{
    (void)ARKTS_IsExternal(ctx.nullEnv, ctx.zeroValue);
    (void)ARKTS_GetExternalData(ctx.nullEnv, ctx.zeroValue);
    (void)ARKTS_CreateSymbol(ctx.nullEnv, nullptr, 0);
    (void)ARKTS_IsSymbol(ctx.nullEnv, ctx.zeroValue);
    (void)ARKTS_GetSymbolDesc(ctx.nullEnv, ctx.zeroValue);
    (void)ARKTS_OpenScope(ctx.nullEnv);
    ARKTS_CloseScope(ctx.nullEnv, ctx.nullScope);
    if (ctx.normalEnv) {
        ARKTS_CloseScope(ctx.normalEnv, ctx.nullScope);
    }
}

void TriggerExceptionEngineAssertBranches(AssertTestContext& ctx)
{
    (void)ARKTS_GetExceptionAndClear(ctx.nullEnv);
    (void)ARKTS_RequireArkModule(ctx.nullEnv, nullptr, 0, ARKTS_NormalModule);
    (void)ARKTS_GetNAPIEnv(ctx.nullEngine);
    ARKTS_DestroyEngine(ctx.nullEngine);
    (void)ARKTS_GetContext(ctx.nullEngine);
    (void)ARKTS_GetThreadIdOfEngine(ctx.nullEngine);
    (void)ARKTS_LoadEntryFromAbc(ctx.nullEngine, nullptr, nullptr, false);
    (void)ARKTS_ImportFromEntry(ctx.nullEngine, nullptr, nullptr);
    (void)ARKTS_Require(ctx.nullEnv, nullptr, false, false, nullptr);
}

void TriggerPromiseAssertBranches(AssertTestContext& ctx)
{
    (void)ARKTS_CreatePromiseCapability(ctx.nullEnv);
    (void)ARKTS_GetPromiseFromCapability(ctx.nullEnv, ctx.nullPromise);
    if (ctx.normalEnv) {
        (void)ARKTS_GetPromiseFromCapability(ctx.normalEnv, ctx.nullPromise);
    }
    ARKTS_PromiseCapabilityResolve(ctx.nullEnv, ctx.nullPromise, ctx.zeroValue);
    if (ctx.normalEnv) {
        ARKTS_PromiseCapabilityResolve(ctx.normalEnv, ctx.nullPromise, ctx.zeroValue);
    }
    ARKTS_PromiseCapabilityReject(ctx.nullEnv, ctx.nullPromise, ctx.zeroValue);
    if (ctx.normalEnv) {
        ARKTS_PromiseCapabilityReject(ctx.normalEnv, ctx.nullPromise, ctx.zeroValue);
    }
    (void)ARKTS_IsPromise(ctx.nullEnv, ctx.zeroValue);
    (void)ARKTS_PromiseThen(ctx.nullEnv, ctx.zeroValue, ctx.zeroValue, ctx.zeroValue);
    (void)ARKTS_PromiseCatch(ctx.nullEnv, ctx.zeroValue, ctx.zeroValue);
    ARKTS_CreateAsyncTask(ctx.nullEnv, 0);
    ARKTS_InitEventHandle(ctx.nullEnv);
}

void TriggerArrayBufferBigIntAssertBranches(AssertTestContext& ctx)
{
    (void)ARKTS_CreateArrayBuffer(ctx.nullEnv, 0);
    (void)ARKTS_CreateArrayBufferWithData(ctx.nullEnv, nullptr, 0, 0);
    (void)ARKTS_IsArrayBuffer(ctx.nullEnv, ctx.zeroValue);
    (void)ARKTS_GetArrayBufferLength(ctx.nullEnv, ctx.zeroValue);
    (void)ARKTS_GetArrayBufferRawPtr(ctx.nullEnv, ctx.zeroValue);
    (void)ARKTS_ArrayBufferReadBytes(ctx.nullEnv, ctx.zeroValue, ctx.smallBuf, 0);
    (void)ARKTS_CreateBigInt(ctx.nullEnv, 0);
    (void)ARKTS_CreateBigIntWithBytes(ctx.nullEnv, false, 0, ctx.smallBytes);
    (void)ARKTS_IsBigInt(ctx.nullEnv, ctx.zeroValue);
    (void)ARKTS_BigIntGetByteSize(ctx.nullEnv, ctx.zeroValue);
    ARKTS_BigIntReadBytes(ctx.nullEnv, ctx.zeroValue, &ctx.isNegative, 0, ctx.smallBytes);
}

void TriggerCycleFreeMiscAssertBranches(AssertTestContext& ctx)
{
    (void)ARKTS_CreateCycleFreeFunc(ctx.nullEnv, 0);
    (void)ARKTS_CreateCycleFreeExtern(ctx.nullEnv, 0);
    ARKTS_UpdateStackInfo(0, nullptr, 0);
}

void TriggerArktsAssertEarlyReturnBranches()
{
    AssertTestContext ctx;
    ctx.tempEngine = ARKTS_CreateEngine();
    ctx.normalEnv = ctx.tempEngine ? ARKTS_GetContext(ctx.tempEngine) : nullptr;

    TriggerBasicAssertBranches(ctx);
    TriggerStringAssertBranches(ctx);
    TriggerFunctionClassAssertBranches(ctx);
    TriggerCallInfoAssertBranches(ctx);
    TriggerCallNewAssertBranches(ctx);
    TriggerObjectAssertBranches(ctx);
    TriggerArrayAssertBranches(ctx);
    TriggerGlobalAssertBranches(ctx);
    TriggerSymbolScopeAssertBranches(ctx);
    TriggerExceptionEngineAssertBranches(ctx);
    TriggerPromiseAssertBranches(ctx);
    TriggerArrayBufferBigIntAssertBranches(ctx);
    TriggerCycleFreeMiscAssertBranches(ctx);

    if (ctx.tempEngine) {
        ARKTS_DestroyEngine(ctx.tempEngine);
    }
}