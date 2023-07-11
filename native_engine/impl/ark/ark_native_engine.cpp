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

#include "ark_native_engine.h"

#include "ark_native_deferred.h"
#include "ark_native_reference.h"
#include "scope_manager/native_scope_manager.h"

#ifdef ENABLE_CONTAINER_SCOPE
#include "core/common/container_scope.h"
#endif

#include "native_engine/native_property.h"

#include "native_value/ark_native_array.h"
#include "native_value/ark_native_array_buffer.h"
#include "native_value/ark_native_buffer.h"
#include "native_value/ark_native_big_int.h"
#include "native_value/ark_native_boolean.h"
#include "native_value/ark_native_data_view.h"
#include "native_value/ark_native_external.h"
#include "native_value/ark_native_function.h"
#include "native_value/ark_native_number.h"
#include "native_value/ark_native_object.h"
#include "native_value/ark_native_string.h"
#include "native_value/ark_native_typed_array.h"
#include "native_value/ark_native_date.h"

#ifdef ENABLE_HITRACE
#include "hitrace_meter.h"
#endif
#if !defined(PREVIEW) && !defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
#include "parameters.h"
#endif
#include "securec.h"
#include "utils/log.h"
#ifdef ENABLE_HITRACE
#include "parameter.h"
#endif

using panda::JsiRuntimeCallInfo;
using panda::BooleanRef;
using panda::ObjectRef;
using panda::StringRef;
using panda::FunctionRef;
using panda::PrimitiveRef;
using panda::JSValueRef;
using panda::ArrayBufferRef;
using panda::TypedArrayRef;
using panda::PromiseCapabilityRef;
using panda::NativePointerRef;
using panda::SymbolRef;
using panda::IntegerRef;
using panda::DateRef;
using panda::BigIntRef;
static constexpr auto PANDA_MAIN_FUNCTION = "_GLOBAL::func_main_0";
#ifdef ENABLE_HITRACE
constexpr auto NAPI_PROFILER_PARAM_SIZE = 10;
#endif

std::string ArkNativeEngine::tempModuleName_ {""};
bool ArkNativeEngine::napiProfilerEnabled {false};
bool ArkNativeEngine::napiProfilerParamReaded {false};
PermissionCheckCallback ArkNativeEngine::permissionCheckCallback_ {nullptr};

struct MoudleNameLocker {
    explicit MoudleNameLocker(std::string moduleName)
    {
        ArkNativeEngine::tempModuleName_ = moduleName;
    }
    ~MoudleNameLocker()
    {
        ArkNativeEngine::tempModuleName_ = "";
    }
};

ArkNativeEngine::ArkNativeEngine(EcmaVM* vm, void* jsEngine) : NativeEngine(jsEngine), vm_(vm), topScope_(vm)
{
    HILOG_DEBUG("ArkNativeEngine::ArkNativeEngine");
#ifdef ENABLE_HITRACE
    if (!ArkNativeEngine::napiProfilerParamReaded) {
        char napiProfilerParam[NAPI_PROFILER_PARAM_SIZE] = {0};
        int ret = GetParameter("persist.hiviewdfx.napiprofiler.enabled", "false",
            napiProfilerParam, sizeof(napiProfilerParam));
        if (ret > 0 && strcmp(napiProfilerParam, "true") == 0) {
            ArkNativeEngine::napiProfilerEnabled = true;
        }
        ArkNativeEngine::napiProfilerParamReaded = true;
    }
#endif
    LocalScope scope(vm_);
    Local<StringRef> requireInternalName = StringRef::NewFromUtf8(vm, "requireInternal");
    void* requireData = static_cast<void*>(this);

    Local<FunctionRef> requireNapi =
        FunctionRef::New(
            vm,
            [](JsiRuntimeCallInfo *info) -> Local<JSValueRef> {
                EcmaVM *ecmaVm = info->GetVM();
                panda::EscapeLocalScope scope(ecmaVm);
                NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
                ArkNativeEngine* arkNativeEngine = static_cast<ArkNativeEngine*>(info->GetData());
                Local<StringRef> moduleName(info->GetCallArgRef(0));
                NativeModule* module = nullptr;
                bool isAppModule = false;
#ifdef IOS_PLATFORM
                module =
                    moduleManager->LoadNativeModule(moduleName->ToString().c_str(), nullptr, false, false);
#else
                const uint32_t lengthMax = 2;
                if (info->GetArgsNumber() >= lengthMax) {
                    Local<BooleanRef> ret(info->GetCallArgRef(1));
                    isAppModule = ret->Value();
                }
                if (info->GetArgsNumber() == 3) { // 3:Determine if the number of parameters is equal to 3
                    Local<StringRef> path(info->GetCallArgRef(2)); // 2:Take the second parameter
                    module =
                        moduleManager->LoadNativeModule(moduleName->ToString().c_str(), path->ToString().c_str(),
                            isAppModule, false);
                } else if (info->GetArgsNumber() == 4) { // 4:Determine if the number of parameters is equal to 4
                    Local<StringRef> path(info->GetCallArgRef(2)); // 2:Take the second parameter
                    Local<StringRef> relativePath(info->GetCallArgRef(3)); // 3:Take the second parameter
                    module =
                        moduleManager->LoadNativeModule(moduleName->ToString().c_str(), nullptr, isAppModule, false,
                            relativePath->ToString().c_str());
                } else {
                    module =
                        moduleManager->LoadNativeModule(moduleName->ToString().c_str(), nullptr, isAppModule, false);
                }
#endif
                Local<JSValueRef> exports(JSValueRef::Undefined(ecmaVm));
                if (module != nullptr) {
                    auto it = arkNativeEngine->loadedModules_.find(module);
                    if (it != arkNativeEngine->loadedModules_.end()) {
                        return scope.Escape(it->second.ToLocal(ecmaVm));
                    }
                    std::string strModuleName = moduleName->ToString();
                    moduleManager->SetNativeEngine(strModuleName, arkNativeEngine);
                    MoudleNameLocker nameLocker(strModuleName);

                    if (module->jsCode == nullptr && module->getABCCode != nullptr) {
                        module->getABCCode(&module->jsCode, &module->jsCodeLen);
                    }
                    if (module->jsCode != nullptr) {
                        char fileName[NAPI_PATH_MAX] = { 0 };
                        const char* name = module->name;
                        if (sprintf_s(fileName, sizeof(fileName), "lib%s.z.so/%s.js", name, name) == -1) {
                            HILOG_ERROR("sprintf_s file name failed");
                            return scope.Escape(exports);
                        }
                        HILOG_DEBUG("load js code from %{public}s", fileName);
                        NativeValue* exportObject = arkNativeEngine->LoadArkModule(module->jsCode,
                            module->jsCodeLen, fileName);
                        if (exportObject == nullptr) {
                            HILOG_ERROR("load module failed");
                            return scope.Escape(exports);
                        } else {
                            Global<JSValueRef> globalExports = *exportObject;
                            exports = globalExports.ToLocal(ecmaVm);
                            arkNativeEngine->loadedModules_[module] = Global<JSValueRef>(ecmaVm, exports);
                        }
                    } else if (module->registerCallback != nullptr) {
                        NativeScopeManager* scopeManager = arkNativeEngine->GetScopeManager();
                        if (scopeManager == nullptr) {
                            HILOG_ERROR("scope manager is null");
                            return scope.Escape(exports);
                        }
                        NativeScope* nativeScope = scopeManager->Open();
                        NativeValue* exportObject = arkNativeEngine->CreateObject();
                        if (!arkNativeEngine) {
                            HILOG_ERROR("init module failed");
                            scopeManager->Close(nativeScope);
                            return scope.Escape(exports);
                        }
#ifdef ENABLE_HITRACE
                        StartTrace(HITRACE_TAG_ACE, "NAPI module init, name = " + std::string(module->name));
#endif
                        ArkNativeObject* exportObj = reinterpret_cast<ArkNativeObject*>(exportObject);
                        arkNativeEngine->SetModuleName(exportObj, module->name);
                        module->registerCallback(arkNativeEngine, exportObject);
#ifdef ENABLE_HITRACE
                        FinishTrace(HITRACE_TAG_ACE);
#endif
                        Global<JSValueRef> globalExports = *exportObject;
                        exports = globalExports.ToLocal(ecmaVm);
                        arkNativeEngine->loadedModules_[module] = Global<JSValueRef>(ecmaVm, exports);
                        scopeManager->Close(nativeScope);
                    } else {
                        HILOG_ERROR("init module failed");
                        return scope.Escape(exports);
                    }
                }
                return scope.Escape(exports);
            },
            nullptr,
            requireData);

    Local<FunctionRef> requireInternal =
        FunctionRef::New(
            vm,
            [](JsiRuntimeCallInfo *info) -> Local<JSValueRef> {
                EcmaVM *ecmaVm = info->GetVM();
                panda::EscapeLocalScope scope(ecmaVm);
                NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
                ArkNativeEngine* arkNativeEngine = static_cast<ArkNativeEngine*>(info->GetData());
                Local<StringRef> moduleName(info->GetCallArgRef(0));
                NativeModule* module = moduleManager->LoadNativeModule(moduleName->ToString().c_str(), nullptr, false);
                Local<JSValueRef> exports(JSValueRef::Undefined(ecmaVm));
                MoudleNameLocker nameLocker(moduleName->ToString());
                if (module != nullptr && arkNativeEngine) {
                    auto it = arkNativeEngine->loadedModules_.find(module);
                    if (it != arkNativeEngine->loadedModules_.end()) {
                        return scope.Escape(it->second.ToLocal(ecmaVm));
                    }
                    std::string strModuleName = moduleName->ToString();
                    moduleManager->SetNativeEngine(strModuleName, arkNativeEngine);
                    NativeScopeManager* scopeManager = arkNativeEngine->GetScopeManager();
                    if (scopeManager == nullptr) {
                        HILOG_ERROR("scope manager is null");
                        return scope.Escape(exports);
                    }
                    NativeScope* nativeScope = scopeManager->Open();
                    NativeValue* exportObject = arkNativeEngine->CreateObject();
                    if (exportObject != nullptr) {
                        if (!arkNativeEngine) {
                            HILOG_ERROR("exportObject is nullptr");
                            scopeManager->Close(nativeScope);
                            return scope.Escape(exports);
                        }
                        ArkNativeObject* exportObj = reinterpret_cast<ArkNativeObject*>(exportObject);
                        arkNativeEngine->SetModuleName(exportObj, module->name);
                        module->registerCallback(arkNativeEngine, exportObject);
                        Global<JSValueRef> globalExports = *exportObject;
                        exports = globalExports.ToLocal(ecmaVm);
                        arkNativeEngine->loadedModules_[module] = Global<JSValueRef>(ecmaVm, exports);
                        scopeManager->Close(nativeScope);
                    } else {
                        HILOG_ERROR("exportObject is nullptr");
                        scopeManager->Close(nativeScope);
                        return scope.Escape(exports);
                    }
                }
                return scope.Escape(exports);
            },
            nullptr,
            requireData);

    Local<ObjectRef> global = panda::JSNApi::GetGlobalObject(vm);
#if !defined(PREVIEW)
    Local<StringRef> requireName = StringRef::NewFromUtf8(vm, "requireNapi");
    global->Set(vm, requireName, requireNapi);
#else
    Local<StringRef> requireNapiPreview = StringRef::NewFromUtf8(vm, "requireNapiPreview");
    global->Set(vm, requireNapiPreview, requireNapi);
#endif
    global->Set(vm, requireInternalName, requireInternal);
    JSNApi::SetNativePtrGetter(vm, reinterpret_cast<void*>(ArkNativeFunction::GetNativePtrCallBack));
    // need to call init of base class.
    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
    JSNApi::SetUnloadNativeModuleCallback(vm, std::bind(&NativeModuleManager::UnloadNativeModule,
        moduleManager, std::placeholders::_1));
    Init();
    panda::JSNApi::SetLoop(vm, loop_);
}

ArkNativeEngine::~ArkNativeEngine()
{
    HILOG_INFO("ArkNativeEngine::~ArkNativeEngine");
    Deinit();
    // Free cached objects
    for (auto&& [module, exportObj] : loadedModules_) {
        exportObj.FreeGlobalHandleAddr();
    }
    // Free callbackRef
    if (promiseRejectCallbackRef_ != nullptr) {
        delete promiseRejectCallbackRef_;
    }
    if (checkCallbackRef_ != nullptr) {
        delete checkCallbackRef_;
    }
}

const EcmaVM* ArkNativeEngine::GetEcmaVm() const
{
    return vm_;
}

panda::Local<panda::ObjectRef> ArkNativeEngine::GetModuleFromName(
    const std::string& moduleName, bool isAppModule, const std::string& id, const std::string& param,
    const std::string& instanceName, void** instance)
{
    panda::EscapeLocalScope scope(vm_);
    Local<ObjectRef> exports(JSValueRef::Undefined(vm_));
    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
    NativeModule* module = moduleManager->LoadNativeModule(moduleName.c_str(), nullptr, isAppModule);
    if (module != nullptr) {
        NativeChunk& chunk = GetNativeChunk();
        NativeValue* idValue = chunk.New<ArkNativeString>(this, id.c_str(), id.size());
        NativeValue* paramValue = chunk.New<ArkNativeString>(this, param.c_str(), param.size());
        NativeValue* exportObject = chunk.New<ArkNativeObject>(this);

        NativePropertyDescriptor idProperty, paramProperty;
        idProperty.utf8name = "id";
        idProperty.value = idValue;
        paramProperty.utf8name = "param";
        paramProperty.value = paramValue;

        ArkNativeObject* exportObj = reinterpret_cast<ArkNativeObject*>(exportObject);
        SetModuleName(exportObj, module->name);
        exportObj->DefineProperty(idProperty);
        exportObj->DefineProperty(paramProperty);
        MoudleNameLocker nameLocker(module->name);
        module->registerCallback(this, exportObject);

        napi_value nExport = reinterpret_cast<napi_value>(exportObject);
        napi_value exportInstance = nullptr;
        napi_status status = napi_get_named_property(
            reinterpret_cast<napi_env>(this), nExport, instanceName.c_str(), &exportInstance);
        if (status != napi_ok) {
            HILOG_ERROR("GetModuleFromName napi_get_named_property status != napi_ok");
        }

        status = napi_unwrap(reinterpret_cast<napi_env>(this), exportInstance, reinterpret_cast<void**>(instance));
        if (status != napi_ok) {
            HILOG_ERROR("GetModuleFromName napi_unwrap status != napi_ok");
        }

        Global<ObjectRef> globalExports = *exportObject;
        exports = globalExports.ToLocal(vm_);
    }
    return scope.Escape(exports);
}

panda::Local<panda::ObjectRef> ArkNativeEngine::LoadModuleByName(const std::string& moduleName, bool isAppModule,
    const std::string& param, const std::string& instanceName, void* instance, const std::string& path)
{
    panda::EscapeLocalScope scope(vm_);
    Local<ObjectRef> exports(JSValueRef::Undefined(vm_));
    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
    NativeModule* module =
        moduleManager->LoadNativeModule(moduleName.c_str(), path.empty() ? nullptr : path.c_str(), isAppModule);
    if (module != nullptr) {
        NativeValue* exportObject = new ArkNativeObject(this);
        ArkNativeObject* exportObj = reinterpret_cast<ArkNativeObject*>(exportObject);

        NativePropertyDescriptor paramProperty, instanceProperty;

        NativeValue* paramValue =
            new ArkNativeString(this, param.c_str(), param.size());
        paramProperty.utf8name = "param";
        paramProperty.value = paramValue;

        auto instanceValue = new ArkNativeObject(this);
        instanceValue->SetNativePointer(instance, nullptr, nullptr);
        instanceProperty.utf8name = instanceName.c_str();
        instanceProperty.value = instanceValue;

        SetModuleName(exportObj, module->name);
        exportObj->DefineProperty(paramProperty);
        exportObj->DefineProperty(instanceProperty);

        MoudleNameLocker nameLocker(module->name);
        module->registerCallback(this, exportObject);
        Global<ObjectRef> globalExports = *exportObject;
        exports = globalExports.ToLocal(vm_);
    }
    return scope.Escape(exports);
}

void ArkNativeEngine::Loop(LoopMode mode, bool needSync)
{
    LocalScope scope(vm_);
    NativeEngine::Loop(mode, needSync);
    panda::JSNApi::ExecutePendingJob(vm_);
}

inline void ArkNativeEngine::SetModuleName(ArkNativeObject *nativeObj, std::string moduleName)
{
#ifdef ENABLE_HITRACE
    if (ArkNativeEngine::napiProfilerEnabled) {
        nativeObj->SetModuleName(moduleName);
    }
#endif
}

NativeValue* ArkNativeEngine::GetGlobal()
{
    LocalScope scope(vm_);
    Local<ObjectRef> value = panda::JSNApi::GetGlobalObject(vm_);
    return ArkValueToNativeValue(this, value);
}

NativeValue* ArkNativeEngine::CreateNull()
{
    LocalScope scope(vm_);
    Local<PrimitiveRef> value = JSValueRef::Null(vm_);
    return GetNativeChunk().New<ArkNativeValue>(this, value);
}

NativeValue* ArkNativeEngine::CreateUndefined()
{
    LocalScope scope(vm_);
    Local<PrimitiveRef> value = JSValueRef::Undefined(vm_);
    return GetNativeChunk().New<ArkNativeValue>(this, value);
}

NativeValue* ArkNativeEngine::CreateBoolean(bool value)
{
    return GetNativeChunk().New<ArkNativeBoolean>(this, value);
}

NativeValue* ArkNativeEngine::CreateNumber(int32_t value)
{
    return GetNativeChunk().New<ArkNativeNumber>(this, value);
}

NativeValue* ArkNativeEngine::CreateNumber(uint32_t value)
{
    return GetNativeChunk().New<ArkNativeNumber>(this, value);
}

NativeValue* ArkNativeEngine::CreateNumber(int64_t value)
{
    return GetNativeChunk().New<ArkNativeNumber>(this, value);
}

NativeValue* ArkNativeEngine::CreateNumber(double value)
{
    return GetNativeChunk().New<ArkNativeNumber>(this, value);
}

NativeValue* ArkNativeEngine::CreateBigInt(int64_t value)
{
    return GetNativeChunk().New<ArkNativeBigInt>(this, value);
}

NativeValue* ArkNativeEngine::CreateBigInt(uint64_t value)
{
    return GetNativeChunk().New<ArkNativeBigInt>(this, value, true);
}

NativeValue* ArkNativeEngine::CreateString(const char* value, size_t length)
{
    return GetNativeChunk().New<ArkNativeString>(this, value, length);
}

NativeValue* ArkNativeEngine::CreateString16(const char16_t* value, size_t length)
{
    return GetNativeChunk().New<ArkNativeString>(this, value, length);
}

NativeValue* ArkNativeEngine::CreateSymbol(NativeValue* value)
{
    LocalScope scope(vm_);
    Global<StringRef> str = *value;
    Local<SymbolRef> symbol = SymbolRef::New(vm_, str.ToLocal(vm_));
    return GetNativeChunk().New<ArkNativeValue>(this, symbol);
}

NativeValue* ArkNativeEngine::CreateExternal(void* value, NativeFinalize callback, void* hint,
    size_t nativeBindingSize)
{
    return GetNativeChunk().New<ArkNativeExternal>(this, value, callback, hint, nativeBindingSize);
}

NativeValue* ArkNativeEngine::CreateObject()
{
    return GetNativeChunk().New<ArkNativeObject>(this);
}

NativeValue* ArkNativeEngine::CreateFunction(const char* name, size_t length, NativeCallback cb, void* value)
{
    return GetNativeChunk().New<ArkNativeFunction>(this, name, length, cb, value);
}

NativeValue* ArkNativeEngine::CreateArray(size_t length)
{
    return GetNativeChunk().New<ArkNativeArray>(this, length);
}

NativeValue* ArkNativeEngine::CreateArrayBuffer(void** value, size_t length)
{
    return GetNativeChunk().New<ArkNativeArrayBuffer>(this, (uint8_t**)value, length);
}

NativeValue* ArkNativeEngine::CreateArrayBufferExternal(void* value, size_t length, NativeFinalize cb, void* hint)
{
    return GetNativeChunk().New<ArkNativeArrayBuffer>(this, (uint8_t*)value, length, cb, hint);
}

NativeValue* ArkNativeEngine::CreateTypedArray(
    NativeTypedArrayType type, NativeValue* value, size_t length, size_t offset)
{
    LocalScope scope(vm_);
    Global<ArrayBufferRef> globalBuffer = *value;
    Local<ArrayBufferRef> buffer = globalBuffer.ToLocal(vm_);
    Local<TypedArrayRef> typedArray(JSValueRef::Undefined(vm_));

    switch (type) {
        case NATIVE_INT8_ARRAY:
            typedArray = panda::Int8ArrayRef::New(vm_, buffer, offset, length);
            break;
        case NATIVE_UINT8_ARRAY:
            typedArray = panda::Uint8ArrayRef::New(vm_, buffer, offset, length);
            break;
        case NATIVE_UINT8_CLAMPED_ARRAY:
            typedArray = panda::Uint8ClampedArrayRef::New(vm_, buffer, offset, length);
            break;
        case NATIVE_INT16_ARRAY:
            typedArray = panda::Int16ArrayRef::New(vm_, buffer, offset, length);
            break;
        case NATIVE_UINT16_ARRAY:
            typedArray = panda::Uint16ArrayRef::New(vm_, buffer, offset, length);
            break;
        case NATIVE_INT32_ARRAY:
            typedArray = panda::Int32ArrayRef::New(vm_, buffer, offset, length);
            break;
        case NATIVE_UINT32_ARRAY:
            typedArray = panda::Uint32ArrayRef::New(vm_, buffer, offset, length);
            break;
        case NATIVE_FLOAT32_ARRAY:
            typedArray = panda::Float32ArrayRef::New(vm_, buffer, offset, length);
            break;
        case NATIVE_FLOAT64_ARRAY:
            typedArray = panda::Float64ArrayRef::New(vm_, buffer, offset, length);
            break;
        case NATIVE_BIGINT64_ARRAY:
            typedArray = panda::BigInt64ArrayRef::New(vm_, buffer, offset, length);
            break;
        case NATIVE_BIGUINT64_ARRAY:
            typedArray = panda::BigUint64ArrayRef::New(vm_, buffer, offset, length);
            break;
        default:
            return nullptr;
    }
    return GetNativeChunk().New<ArkNativeTypedArray>(this, typedArray);
}

NativeValue* ArkNativeEngine::CreateDataView(NativeValue* value, size_t length, size_t offset)
{
    return GetNativeChunk().New<ArkNativeDataView>(this, value, length, offset);
}

NativeValue* ArkNativeEngine::CreatePromise(NativeDeferred** deferred)
{
    LocalScope scope(vm_);
    Local<PromiseCapabilityRef> capability = PromiseCapabilityRef::New(vm_);
    *deferred = new ArkNativeDeferred(this, capability);
    return GetNativeChunk().New<ArkNativeObject>(this, capability->GetPromise(vm_));
}

NativeValue* ArkNativeEngine::CreateError(NativeValue* code, NativeValue* message)
{
    LocalScope scope(vm_);
    Local<JSValueRef> errorVal = panda::Exception::Error(vm_, *message);
    if (code != nullptr) {
        Local<StringRef> codeKey = StringRef::NewFromUtf8(vm_, "code");
        Local<ObjectRef> errorObj(errorVal);
        errorObj->Set(vm_, codeKey, *code);
    }
    return ArkValueToNativeValue(this, errorVal);
}

static void ConcurrentCallbackFunc(Local<JSValueRef> result, bool success, void *taskInfo, void *data)
{
    if (data == nullptr) {
        return;
    }
    auto engine = static_cast<ArkNativeEngine *>(data);
    auto concurrentCallbackFunc = engine->GetConcurrentCallbackFunc();
    if (concurrentCallbackFunc == nullptr) {
        return;
    }
    concurrentCallbackFunc(engine, ArkNativeEngine::ArkValueToNativeValue(engine, result), success, taskInfo);
}

bool ArkNativeEngine::InitTaskPoolThread(NativeEngine* engine, NapiConcurrentCallback callback)
{
    concurrentCallbackFunc_ = callback;
    return JSNApi::InitForConcurrentThread(vm_, ConcurrentCallbackFunc, static_cast<void *>(this));
}

bool ArkNativeEngine::InitTaskPoolFunc(NativeEngine* engine, NativeValue* func, void* taskInfo)
{
    LocalScope scope(vm_);
    Global<JSValueRef> globalObj = *func;
    Local<JSValueRef> function = globalObj.ToLocal(vm_);
    return JSNApi::InitForConcurrentFunction(vm_, function, taskInfo);
}

bool ArkNativeEngine::HasPendingJob()
{
    return JSNApi::HasPendingJob(vm_);
}

bool ArkNativeEngine::IsProfiling()
{
    return JSNApi::IsProfiling(vm_);
}

void* ArkNativeEngine::GetCurrentTaskInfo() const
{
    return JSNApi::GetCurrentTaskInfo(vm_);
}

NativeValue* ArkNativeEngine::CallFunction(
    NativeValue* thisVar, NativeValue* function, NativeValue* const* argv, size_t argc)
{
    if (function == nullptr) {
        return nullptr;
    }
    LocalScope scope(vm_);
    NativeScopeManager* scopeManager = GetScopeManager();
    if (scopeManager == nullptr) {
        HILOG_ERROR("scope manager is null");
        return nullptr;
    }
    NativeScope* nativeScope = scopeManager->Open();
    Local<JSValueRef> thisObj = JSValueRef::Undefined(vm_);
    if (thisVar != nullptr) {
        Global<JSValueRef> globalObj = *thisVar;
        thisObj = globalObj.ToLocal(vm_);
    }
    Global<FunctionRef> funcObj = *function;
#ifdef ENABLE_CONTAINER_SCOPE
    auto nativeFunction = static_cast<NativeFunction*>(function->GetInterface(NativeFunction::INTERFACE_ID));
    if (nativeFunction == nullptr) {
        HILOG_ERROR("nativeFunction is null");
        return nullptr;
    }
    auto arkNativeFunc = static_cast<ArkNativeFunction*>(nativeFunction);
    OHOS::Ace::ContainerScope containerScope(arkNativeFunc->GetScopeId());
#endif
    std::vector<Local<JSValueRef>> args;
    args.reserve(argc);
    for (size_t i = 0; i < argc; i++) {
        if (argv[i] != nullptr) {
            Global<JSValueRef> arg = *argv[i];
            args.emplace_back(arg.ToLocal(vm_));
        } else {
            args.emplace_back(JSValueRef::Undefined(vm_));
        }
    }

    Local<JSValueRef> value = funcObj->Call(vm_, thisObj, args.data(), argc);
    scopeManager->Close(nativeScope);
    if (panda::JSNApi::HasPendingException(vm_)) {
        HILOG_ERROR("pending exception when js function called");
        HILOG_ERROR("print exception info: ");
        panda::JSNApi::PrintExceptionInfo(vm_);
        return nullptr;
    }

    return ArkValueToNativeValue(this, value);
}

NativeValue* ArkNativeEngine::RunScript(NativeValue* script)
{
    return nullptr;
}

NativeValue* ArkNativeEngine::RunScriptPath(const char* path)
{
    panda::JSExecutionScope executionScope(vm_);
    LocalScope scope(vm_);
    [[maybe_unused]] bool ret = panda::JSNApi::Execute(vm_, path, PANDA_MAIN_FUNCTION);
    if (panda::JSNApi::HasPendingException(vm_)) {
        HandleUncaughtException();
        return nullptr;
    }
    return CreateUndefined();
}

NativeEngine* ArkNativeEngine::GetWorkerEngine(uint32_t tid)
{
#if !defined(PREVIEW) && !defined(IOS_PLATFORM)
    return reinterpret_cast<ArkNativeEngine*>(panda::DFXJSNApi::GetWorkerVm(vm_, tid));
#else
    HILOG_WARN("ARK does not support dfx on windows");
    return nullptr;
#endif    
}

// The security interface needs to be modified accordingly.
NativeValue* ArkNativeEngine::RunScriptBuffer(const char* path, std::vector<uint8_t>& buffer, bool isBundle)
{
    panda::JSExecutionScope executionScope(vm_);
    LocalScope scope(vm_);
    [[maybe_unused]] bool ret = false;
    if (isBundle) {
        ret = panda::JSNApi::Execute(vm_, buffer.data(), buffer.size(), PANDA_MAIN_FUNCTION, path);
    } else {
        ret = panda::JSNApi::ExecuteModuleBuffer(vm_, buffer.data(), buffer.size(), path);
    }

    if (panda::JSNApi::HasPendingException(vm_)) {
        HandleUncaughtException();
        return nullptr;
    }
    return CreateUndefined();
}

bool ArkNativeEngine::RunScriptBuffer(const std::string& path, uint8_t* buffer, size_t size, bool isBundle)
{
    panda::JSExecutionScope executionScope(vm_);
    LocalScope scope(vm_);
    bool ret = false;
    if (isBundle) {
        ret = panda::JSNApi::ExecuteSecure(vm_, buffer, size, PANDA_MAIN_FUNCTION, path);
    } else {
        ret = panda::JSNApi::ExecuteModuleBufferSecure(vm_, buffer, size, path);
    }
    
    if (panda::JSNApi::HasPendingException(vm_)) {
        HandleUncaughtException();
        return false;
    }
    return ret;
}

void ArkNativeEngine::SetPackagePath(const std::string appLibPathKey, const std::vector<std::string>& packagePath)
{
    auto moduleManager = NativeModuleManager::GetInstance();
    if (moduleManager && !packagePath.empty()) {
        moduleManager->SetAppLibPath(appLibPathKey, packagePath);
    }
}

NativeValue* ArkNativeEngine::DefineClass(const char* name, NativeCallback callback,
    void* data, const NativePropertyDescriptor* properties, size_t length)
{
    LocalScope scope(vm_);
    std::string className(name);
    if (ArkNativeEngine::napiProfilerEnabled) {
        className = ArkNativeEngine::tempModuleName_ + "." + name;
    }
    NativeChunk& chunk = GetNativeChunk();
    auto classConstructor = chunk.New<ArkNativeFunction>(this, className.c_str(), callback, data);
    SetModuleName(classConstructor, className);
    auto classPrototype = classConstructor->GetFunctionPrototype();

    for (size_t i = 0; i < length; i++) {
        if (properties[i].attributes & NATIVE_STATIC) {
            classConstructor->DefineProperty(properties[i]);
        } else {
            if (classPrototype == nullptr) {
                HILOG_ERROR("ArkNativeEngineImpl::Class's prototype is null");
                continue;
            }
            ArkNativeObject* arkNativeobj = static_cast<ArkNativeObject*>(classPrototype);
            SetModuleName(arkNativeobj, className);
            arkNativeobj->DefineProperty(properties[i]);
        }
    }

    return classConstructor;
}

NativeValue* ArkNativeEngine::CreateInstance(NativeValue* constructor, NativeValue* const *argv, size_t argc)
{
    if (constructor == nullptr) {
        return nullptr;
    }
    LocalScope scope(vm_);
    Global<FunctionRef> value = *constructor;
    std::vector<Local<JSValueRef>> args;
    args.reserve(argc);
    for (size_t i = 0; i < argc; i++) {
        if (argv[i] != nullptr) {
            Global<JSValueRef> arg = *argv[i];
            args.emplace_back(arg.ToLocal(vm_));
        } else {
            args.emplace_back(JSValueRef::Undefined(vm_));
        }
    }
    Local<JSValueRef> instance = value->Constructor(vm_, args.data(), argc);
    Local<ObjectRef> excep = panda::JSNApi::GetUncaughtException(vm_);
    if (!excep.IsNull()) {
        HILOG_ERROR("ArkNativeEngineImpl::CreateInstance occur Exception");
        return nullptr;
    }
    return ArkValueToNativeValue(this, instance);
}

NativeReference* ArkNativeEngine::CreateReference(NativeValue* value, uint32_t initialRefcount,
    NativeFinalize callback, void* data, void* hint)
{
    return new ArkNativeReference(this, value, initialRefcount, false);
}

bool ArkNativeEngine::IsExceptionPending() const
{
    return panda::JSNApi::HasPendingException(vm_);
}

NativeValue* ArkNativeEngine::GetAndClearLastException()
{
    LocalScope scope(vm_);
    Local<ObjectRef> exception = panda::JSNApi::GetAndClearUncaughtException(vm_);
    if (exception.IsNull()) {
        return nullptr;
    }

    return ArkValueToNativeValue(this, exception);
}

bool ArkNativeEngine::Throw(NativeValue* error)
{
    LocalScope scope(vm_);
    Global<JSValueRef> errorVal = *error;
    panda::JSNApi::ThrowException(vm_, errorVal.ToLocal(vm_));
    return true;
}

bool ArkNativeEngine::Throw(NativeErrorType type, const char* code, const char* message)
{
    LocalScope scope(vm_);
    Local<JSValueRef> error(JSValueRef::Undefined(vm_));
    switch (type) {
        case NATIVE_COMMON_ERROR:
            error = panda::Exception::Error(vm_, StringRef::NewFromUtf8(vm_, message));
            break;
        case NATIVE_TYPE_ERROR:
            error = panda::Exception::TypeError(vm_, StringRef::NewFromUtf8(vm_, message));
            break;
        case NATIVE_RANGE_ERROR:
            error = panda::Exception::RangeError(vm_, StringRef::NewFromUtf8(vm_, message));
            break;
        default:
            return false;
    }
    if (code != nullptr) {
        Local<JSValueRef> codeKey = StringRef::NewFromUtf8(vm_, "code");
        Local<JSValueRef> codeValue = StringRef::NewFromUtf8(vm_, code);
        Local<ObjectRef> errorObj(error);
        errorObj->Set(vm_, codeKey, codeValue);
    }

    panda::JSNApi::ThrowException(vm_, error);
    return true;
}

NativeEngine* ArkNativeEngine::CreateRuntimeFunc(NativeEngine* engine, void* jsEngine)
{
    panda::RuntimeOption option;
#if defined(OHOS_PLATFORM) && !defined(IOS_PLATFORM)
    int arkProperties = OHOS::system::GetIntParameter<int>("persist.ark.properties", -1);
    std::string bundleName = OHOS::system::GetParameter("persist.ark.arkbundlename", "");
    size_t gcThreadNum = OHOS::system::GetUintParameter<size_t>("persist.ark.gcthreads", 7);
    size_t longPauseTime = OHOS::system::GetUintParameter<size_t>("persist.ark.longpausetime", 40);
    bool asmInterpreterEnabled = OHOS::system::GetBoolParameter("persist.ark.asminterpreter", true);
    std::string asmOpcodeDisableRange = OHOS::system::GetParameter("persist.ark.asmopcodedisablerange", "");
    bool builtinsLazyEnabled = OHOS::system::GetBoolParameter("persist.ark.enablebuiltinslazy", true);
    option.SetEnableBuiltinsLazy(builtinsLazyEnabled);
    option.SetArkProperties(arkProperties);
    option.SetArkBundleName(bundleName);
    option.SetGcThreadNum(gcThreadNum);
    option.SetLongPauseTime(longPauseTime);
    option.SetEnableAsmInterpreter(asmInterpreterEnabled);
    option.SetAsmOpcodeDisableRange(asmOpcodeDisableRange);
    option.SetIsWorker();
    HILOG_INFO("ArkNativeEngineImpl::CreateRuntimeFunc ark properties = %{public}d, bundlename = %{public}s",
        arkProperties, bundleName.c_str());
#endif
    option.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
    const int64_t poolSize = 0x1000000;
    option.SetGcPoolSize(poolSize);
#if !defined(PREVIEW) && !defined(IOS_PLATFORM)
    option.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::INFO);
#endif
    option.SetDebuggerLibraryPath("");
    EcmaVM* vm = panda::JSNApi::CreateJSVM(option);
    if (vm == nullptr) {
        return nullptr;
    }
    // worker adaptation mergeabc
    const panda::ecmascript::EcmaVM* hostVM = reinterpret_cast<ArkNativeEngine*>(engine)->GetEcmaVm();
    panda::JSNApi::SynchronizVMInfo(vm, hostVM);
    ArkNativeEngine* arkEngine = new ArkNativeEngine(vm, jsEngine);
    // init callback
    arkEngine->RegisterWorkerFunction(engine);
    arkEngine->SetHostEngine(engine);

    auto cleanEnv = [vm]() {
        if (vm != nullptr) {
            HILOG_INFO("cleanEnv is called");
            panda::JSNApi::DestroyJSVM(vm);
        }
    };
    arkEngine->SetCleanEnv(cleanEnv);

    if (hostVM != nullptr) {
        panda::JSNApi::addWorker(const_cast<EcmaVM*>(hostVM), vm);
    }
    return arkEngine;
}

void* ArkNativeEngine::CreateRuntime()
{
    return ArkNativeEngine::CreateRuntimeFunc(this, jsEngine_);
}

NativeValue* ArkNativeEngine::Serialize(NativeEngine* context, NativeValue* value, NativeValue* transfer)
{
    const panda::ecmascript::EcmaVM* vm = reinterpret_cast<ArkNativeEngine*>(context)->GetEcmaVm();
    LocalScope scope(vm);
    Global<JSValueRef> arkValue = *value;
    Global<JSValueRef> arkTransfer = *transfer;
    void* result = panda::JSNApi::SerializeValue(vm, arkValue.ToLocal(vm), arkTransfer.ToLocal(vm));
    return reinterpret_cast<NativeValue*>(result);
}

NativeValue* ArkNativeEngine::Deserialize(NativeEngine* context, NativeValue* recorder)
{
    const panda::ecmascript::EcmaVM* vm = reinterpret_cast<ArkNativeEngine*>(context)->GetEcmaVm();
    LocalScope scope(vm);
    Local<JSValueRef> result = panda::JSNApi::DeserializeValue(vm, recorder, reinterpret_cast<void*>(context));
    return ArkValueToNativeValue(this, result);
}

void ArkNativeEngine::DeleteSerializationData(NativeValue* value) const
{
    void* data = reinterpret_cast<void*>(value);
    panda::JSNApi::DeleteSerializationData(data);
}

void ArkNativeEngine::StartCpuProfiler(const std::string& fileName)
{
    JSNApi::SetNativePtrGetter(vm_, reinterpret_cast<void*>(ArkNativeFunction::GetNativePtrCallBack));
    DFXJSNApi::StartCpuProfilerForFile(vm_, fileName);
}

void ArkNativeEngine::StopCpuProfiler()
{
    DFXJSNApi::StopCpuProfilerForFile(vm_);
    JSNApi::SetNativePtrGetter(vm_, nullptr);
}

void ArkNativeEngine::ResumeVM()
{
    DFXJSNApi::ResumeVM(vm_);
}

bool ArkNativeEngine::SuspendVM()
{
    return DFXJSNApi::SuspendVM(vm_);
}

bool ArkNativeEngine::IsSuspended()
{
    return DFXJSNApi::IsSuspended(vm_);
}

bool ArkNativeEngine::CheckSafepoint()
{
    return DFXJSNApi::CheckSafepoint(vm_);
}

NativeValue* ArkNativeEngine::RunBufferScript(std::vector<uint8_t>& buffer)
{
    panda::JSExecutionScope executionScope(vm_);
    LocalScope scope(vm_);
    [[maybe_unused]] bool ret = panda::JSNApi::Execute(vm_, buffer.data(), buffer.size(), PANDA_MAIN_FUNCTION);

    if (panda::JSNApi::HasPendingException(vm_)) {
        HandleUncaughtException();
        return nullptr;
    }
    return CreateUndefined();
}

NativeValue* ArkNativeEngine::RunActor(std::vector<uint8_t>& buffer, const char* descriptor)
{
    panda::JSExecutionScope executionScope(vm_);
    LocalScope scope(vm_);
    std::string desc(descriptor);
    [[maybe_unused]] bool ret = false;
    if (panda::JSNApi::IsBundle(vm_) || !buffer.empty()) {
        ret = panda::JSNApi::Execute(vm_, buffer.data(), buffer.size(), PANDA_MAIN_FUNCTION, desc);
    } else {
        ret = panda::JSNApi::Execute(vm_, desc, PANDA_MAIN_FUNCTION);
    }

    if (panda::JSNApi::HasPendingException(vm_)) {
        HandleUncaughtException();
        return nullptr;
    }
    return CreateUndefined();
}

NativeValue* ArkNativeEngine::LoadArkModule(const char* str, int32_t len, const std::string& fileName)
{
    HILOG_DEBUG("ArkNativeEngineImpl::LoadModule start, buffer = %{public}s", str);
    if (str == nullptr || len <= 0 || fileName.empty()) {
        HILOG_ERROR("fileName is nullptr or source code is nullptr");
        return nullptr;
    }

    bool res = JSNApi::ExecuteModuleFromBuffer(vm_, str, len, fileName);
    if (!res) {
        HILOG_ERROR("Execute module failed");
        return nullptr;
    }

    LocalScope scope(vm_);
    Local<ObjectRef> exportObj = JSNApi::GetExportObjectFromBuffer(vm_, fileName, "default");
    if (exportObj->IsNull()) {
        HILOG_ERROR("Get export object failed");
        return nullptr;
    }

    HILOG_DEBUG("ArkNativeEngineImpl::LoadModule end");
    return ArkValueToNativeValue(this, exportObj);
}

NativeValue* ArkNativeEngine::LoadModule(NativeValue* str, const std::string& fileName)
{
    return nullptr;
}

NativeChunk& ArkNativeEngine::GetNativeChunk()
{
    return GetScopeManager()->GetNativeChunk();
}

NativeValue* ArkNativeEngine::ArkValueToNativeValue(ArkNativeEngine* engine, Local<JSValueRef> value)
{
    NativeValue* result = nullptr;
    NativeChunk& chunk = engine->GetNativeChunk();
    if (value->IsNull() || value->IsUndefined() || value->IsSymbol()) {
        result = chunk.New<ArkNativeValue>(engine, value);
    } else if (value->IsNumber()) {
        result = chunk.New<ArkNativeNumber>(engine, value);
    } else if (value->IsString()) {
        result = chunk.New<ArkNativeString>(engine, value);
    } else if (value->IsArray(engine->GetEcmaVm())) {
        result = chunk.New<ArkNativeArray>(engine, value);
    } else if (value->IsFunction()) {
        result = chunk.New<ArkNativeFunction>(engine, value);
    } else if (value->IsArrayBuffer()) {
        result = chunk.New<ArkNativeArrayBuffer>(engine, value);
    } else if (value->IsBuffer()) {
        result = chunk.New<ArkNativeBuffer>(engine, value);
    } else if (value->IsDataView()) {
        result = chunk.New<ArkNativeDataView>(engine, value);
    } else if (value->IsTypedArray()) {
        result = chunk.New<ArkNativeTypedArray>(engine, value);
    } else if (value->IsNativePointer()) {
        result = chunk.New<ArkNativeExternal>(engine, value);
    } else if (value->IsDate()) {
        result = chunk.New<ArkNativeDate>(engine, value);
    } else if (value->IsBigInt()) {
        result = chunk.New<ArkNativeBigInt>(engine, value);
    } else if (value->IsObject() || value->IsPromise()) {
        result = chunk.New<ArkNativeObject>(engine, value);
    } else if (value->IsBoolean()) {
        result = chunk.New<ArkNativeBoolean>(engine, value);
    } else {
        result = chunk.New<ArkNativeValue>(engine, value);
    }
    return result;
}

NativeValue* ArkNativeEngine::ValueToNativeValue(JSValueWrapper& value)
{
    LocalScope scope(vm_);
    Global<JSValueRef> arkValue = value;
    return ArkValueToNativeValue(this, arkValue.ToLocal(vm_));
}

bool ArkNativeEngine::ExecuteJsBin(const std::string& fileName)
{
    panda::JSExecutionScope executionScope(vm_);
    LocalScope scope(vm_);
    bool ret = JSNApi::Execute(vm_, fileName, PANDA_MAIN_FUNCTION);
    return ret;
}

NativeValue* ArkNativeEngine::CreateBuffer(void** value, size_t length)
{
    return GetNativeChunk().New<ArkNativeBuffer>(this, (uint8_t**)value, length);
}

NativeValue* ArkNativeEngine::CreateBufferCopy(void** value, size_t length, const void* data)
{
    return GetNativeChunk().New<ArkNativeBuffer>(this, (uint8_t**)value, length, (uint8_t*)data);
}

NativeValue* ArkNativeEngine::CreateBufferExternal(void* value, size_t length, NativeFinalize cb, void* hint)
{
    return GetNativeChunk().New<ArkNativeBuffer>(this, (uint8_t*)value, length, cb, hint);
}

NativeValue* ArkNativeEngine::CreateDate(double value)
{
    LocalScope scope(vm_);
    return ArkValueToNativeValue(this, DateRef::New(vm_, value));
}

NativeValue* ArkNativeEngine::CreateBigWords(int sign_bit, size_t word_count, const uint64_t* words)
{
    constexpr int bigintMod = 2; // 2 : used for even number judgment
    bool sign = false;
    if ((sign_bit % bigintMod) == 1) {
        sign = true;
    }
    uint32_t size = (uint32_t)word_count;

    LocalScope scope(vm_);
    Local<JSValueRef> value = BigIntRef::CreateBigWords(vm_, sign, size, words);
    return GetNativeChunk().New<ArkNativeBigInt>(this, value);
}

bool ArkNativeEngine::TriggerFatalException(NativeValue* error)
{
    return true;
}

bool ArkNativeEngine::AdjustExternalMemory(int64_t ChangeInBytes, int64_t* AdjustedValue)
{
    return true;
}

void ArkNativeEngine::SetPromiseRejectCallback(NativeReference* rejectCallbackRef, NativeReference* checkCallbackRef)
{
    if (rejectCallbackRef == nullptr || checkCallbackRef == nullptr) {
        HILOG_ERROR("rejectCallbackRef or checkCallbackRef is nullptr");
        return;
    }
    promiseRejectCallbackRef_ = rejectCallbackRef;
    checkCallbackRef_ = checkCallbackRef;
    JSNApi::SetHostPromiseRejectionTracker(vm_, reinterpret_cast<void*>(PromiseRejectCallback),
                                           reinterpret_cast<void*>(this));
}

void ArkNativeEngine::PromiseRejectCallback(void* info)
{
    panda::PromiseRejectInfo* promiseRejectInfo = reinterpret_cast<panda::PromiseRejectInfo*>(info);
    ArkNativeEngine* env = reinterpret_cast<ArkNativeEngine*>(promiseRejectInfo->GetData());

    if (env == nullptr) {
        HILOG_ERROR("engine is nullptr");
        return;
    }

    if (env->promiseRejectCallbackRef_ == nullptr || env->checkCallbackRef_ == nullptr) {
        HILOG_ERROR("promiseRejectCallbackRef or checkCallbackRef is nullptr");
        return;
    }
    panda::ecmascript::EcmaVM* vm = const_cast<EcmaVM*>(env->GetEcmaVm());
    LocalScope scope(vm);
    Local<JSValueRef> promise = promiseRejectInfo->GetPromise();
    Local<JSValueRef> reason = promiseRejectInfo->GetReason();
    panda::PromiseRejectInfo::PROMISE_REJECTION_EVENT operation = promiseRejectInfo->GetOperation();

    Local<JSValueRef> type(IntegerRef::New(vm, static_cast<int32_t>(operation)));

    Local<JSValueRef> args[] = {type, promise, reason};
    Global<FunctionRef> promiseRejectCallback = *(env->promiseRejectCallbackRef_->Get());
    if (!promiseRejectCallback.IsEmpty()) {
        promiseRejectCallback->Call(vm, JSValueRef::Undefined(vm), args, 3); // 3 args size
    }

    if (operation == panda::PromiseRejectInfo::PROMISE_REJECTION_EVENT::REJECT) {
        Global<JSValueRef> checkCallback = *(env->checkCallbackRef_->Get());
        if (!checkCallback.IsEmpty()) {
            JSNApi::SetHostEnqueueJob(vm, checkCallback.ToLocal(vm));
        }
    }
}

void ArkNativeEngine::DumpHeapSnapshot(const std::string& path, bool isVmMode, DumpFormat dumpFormat)
{
    if (dumpFormat == DumpFormat::JSON) {
        DFXJSNApi::DumpHeapSnapshot(vm_, 0, path, isVmMode);
    }
    if (dumpFormat == DumpFormat::BINARY) {
        DFXJSNApi::DumpHeapSnapshot(vm_, 1, path, isVmMode);
    }
    if (dumpFormat == DumpFormat::OTHER) {
        DFXJSNApi::DumpHeapSnapshot(vm_, 2, path, isVmMode); // 2:enum is 2
    }
}

void ArkNativeEngine::DumpHeapSnapshot(bool isVmMode, DumpFormat dumpFormat, bool isPrivate)
{
    if (dumpFormat == DumpFormat::JSON) {
        DFXJSNApi::DumpHeapSnapshot(vm_, 0, isVmMode, isPrivate);
    }
    if (dumpFormat == DumpFormat::BINARY) {
        DFXJSNApi::DumpHeapSnapshot(vm_, 1, isVmMode, isPrivate);
    }
    if (dumpFormat == DumpFormat::OTHER) {
        DFXJSNApi::DumpHeapSnapshot(vm_, 2, isVmMode, isPrivate); // 2:enum is 2
    }
}

bool ArkNativeEngine::BuildNativeAndJsStackTrace(std::string& stackTraceStr)
{
#if !defined(PREVIEW) && !defined(IOS_PLATFORM)
    return DFXJSNApi::BuildNativeAndJsStackTrace(vm_, stackTraceStr);
#else
    HILOG_WARN("ARK does not support dfx on windows");
    return false;
#endif
}

bool ArkNativeEngine::BuildJsStackTrace(std::string& stackTraceStr)
{
#if !defined(PREVIEW) && !defined(IOS_PLATFORM)
    return DFXJSNApi::BuildJsStackTrace(vm_, stackTraceStr);
#else
    HILOG_WARN("ARK does not support dfx on windows");
    return false;
#endif
}

bool ArkNativeEngine::BuildJsStackInfoList(uint32_t tid, std::vector<JsFrameInfo>& jsFrames)
{
#if !defined(PREVIEW) && !defined(IOS_PLATFORM)
    std::vector<ArkJsFrameInfo> arkJsFrames;
    bool sign = DFXJSNApi::BuildJsStackInfoList(vm_, tid, arkJsFrames);
    for (auto jf : arkJsFrames) {
        struct JsFrameInfo jsframe;
        jsframe.fileName = jf.fileName;
        jsframe.functionName = jf.functionName;
        jsframe.pos = jf.pos;
        jsframe.nativePointer = jf.nativePointer;
        jsFrames.emplace_back(jsframe);
    }
    return sign;
#else
    HILOG_WARN("ARK does not support dfx on windows");
    return false;
#endif
}

bool ArkNativeEngine::DeleteWorker(NativeEngine* workerEngine)
{
    if (workerEngine != nullptr) {
#if !defined(PREVIEW)
        const panda::ecmascript::EcmaVM* workerVM = reinterpret_cast<ArkNativeEngine*>(workerEngine)->GetEcmaVm();
        if (workerVM != nullptr) {
            return panda::JSNApi::DeleteWorker(vm_, const_cast<EcmaVM*>(workerVM));
        }
#else
        HILOG_WARN("ARK does not support dfx on windows");
#endif
        return false;
        }
    return false;
}

bool ArkNativeEngine::StartHeapTracking(double timeInterval, bool isVmMode)
{
    return DFXJSNApi::StartHeapTracking(vm_, timeInterval, isVmMode);
}

bool ArkNativeEngine::StopHeapTracking(const std::string &filePath)
{
    return DFXJSNApi::StopHeapTracking(vm_, filePath);
}

#if !defined(PREVIEW)
void ArkNativeEngine::PrintStatisticResult()
{
    DFXJSNApi::PrintStatisticResult(vm_);
}

void ArkNativeEngine::StartRuntimeStat()
{
    DFXJSNApi::StartRuntimeStat(vm_);
}

void ArkNativeEngine::StopRuntimeStat()
{
    DFXJSNApi::StopRuntimeStat(vm_);
}

size_t ArkNativeEngine::GetArrayBufferSize()
{
    return DFXJSNApi::GetArrayBufferSize(vm_);
}

size_t ArkNativeEngine::GetHeapTotalSize()
{
    return DFXJSNApi::GetHeapTotalSize(vm_);
}

size_t ArkNativeEngine::GetHeapUsedSize()
{
    return DFXJSNApi::GetHeapUsedSize(vm_);
}

void ArkNativeEngine::NotifyApplicationState(bool inBackground)
{
    DFXJSNApi::NotifyApplicationState(vm_, inBackground);
}

void ArkNativeEngine::NotifyIdleStatusControl(std::function<void(bool)> callback)
{
    DFXJSNApi::NotifyIdleStatusControl(vm_, callback);
}

void ArkNativeEngine::NotifyIdleTime(int idleMicroSec)
{
    DFXJSNApi::NotifyIdleTime(vm_, idleMicroSec);
}

void ArkNativeEngine::NotifyMemoryPressure(bool inHighMemoryPressure)
{
    DFXJSNApi::NotifyMemoryPressure(vm_, inHighMemoryPressure);
}
#else
void ArkNativeEngine::PrintStatisticResult()
{
    HILOG_WARN("ARK does not support dfx on windows");
}

void ArkNativeEngine::StartRuntimeStat()
{
    HILOG_WARN("ARK does not support dfx on windows");
}

void ArkNativeEngine::StopRuntimeStat()
{
    HILOG_WARN("ARK does not support dfx on windows");
}

size_t ArkNativeEngine::GetArrayBufferSize()
{
    HILOG_WARN("ARK does not support dfx on windows");
    return 0;
}

size_t ArkNativeEngine::GetHeapTotalSize()
{
    HILOG_WARN("ARK does not support dfx on windows");
    return 0;
}

size_t ArkNativeEngine::GetHeapUsedSize()
{
    HILOG_WARN("ARK does not support dfx on windows");
    return 0;
}

void ArkNativeEngine::NotifyApplicationState([[maybe_unused]] bool inBackground)
{
    HILOG_WARN("ARK does not support dfx on windows");
}

void ArkNativeEngine::NotifyIdleStatusControl([[maybe_unused]] std::function<void(bool)> callback)
{
    HILOG_WARN("ARK does not support dfx on windows");
}

void ArkNativeEngine::NotifyIdleTime([[maybe_unused]] int idleMicroSec)
{
    HILOG_WARN("ARK does not support dfx on windows");
}

void ArkNativeEngine::NotifyMemoryPressure([[maybe_unused]] bool inHighMemoryPressure)
{
    HILOG_WARN("ARK does not support dfx on windows");
}
#endif

void ArkNativeEngine::RegisterUncaughtExceptionHandler(UncaughtExceptionCallback callback)
{
    JSNApi::EnableUserUncaughtErrorHandler(vm_);
    uncaughtExceptionCallback_ = callback;
}

void ArkNativeEngine::HandleUncaughtException()
{
    if (uncaughtExceptionCallback_ == nullptr) {
        return;
    }
    LocalScope scope(vm_);
    Local<ObjectRef> exception = JSNApi::GetAndClearUncaughtException(vm_);
    if (!exception.IsEmpty() && !exception->IsHole()) {
        uncaughtExceptionCallback_(ArkValueToNativeValue(this, exception));
    }
}

bool ArkNativeEngine::HasPendingException()
{
    return panda::JSNApi::HasPendingException(vm_);
}

void ArkNativeEngine::RegisterPermissionCheck(PermissionCheckCallback callback)
{
    if (permissionCheckCallback_ == nullptr) {
        permissionCheckCallback_ = callback;
    }
}

bool ArkNativeEngine::ExecutePermissionCheck()
{
    if (permissionCheckCallback_ != nullptr) {
        return permissionCheckCallback_();
    } else {
        HILOG_INFO("permissionCheckCallback_ is still nullptr when executing permission check!");
        return true;
    }
}

void ArkNativeEngine::RegisterTranslateBySourceMap(SourceMapCallback callback)
{
    if (SourceMapCallback_ == nullptr) {
        SourceMapCallback_ = callback;
    }
}

std::string ArkNativeEngine::ExecuteTranslateBySourceMap(const std::string& rawStack)
{
    if (SourceMapCallback_ != nullptr) {
        return SourceMapCallback_(rawStack);
    } else {
        HILOG_WARN("SourceMapCallback_ is nullptr.");
        return "";
    }
}

bool ArkNativeEngine::IsMixedDebugEnabled()
{
    return JSNApi::IsMixedDebugEnabled(vm_);
}

void ArkNativeEngine::NotifyNativeCalling(const void *nativeAddress)
{
    JSNApi::NotifyNativeCalling(vm_, nativeAddress);
}

void ArkNativeEngine::AllowCrossThreadExecution() const
{
    JSNApi::AllowCrossThreadExecution(vm_);
}
