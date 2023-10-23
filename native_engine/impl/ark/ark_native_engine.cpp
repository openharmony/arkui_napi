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

#include "native_reference.h"
#include "scope_manager/native_scope_manager.h"

#ifdef ENABLE_CONTAINER_SCOPE
#include "core/common/container_scope.h"
#endif

#include "native_engine/native_property.h"

#include "native_value/ark_native_function.h"
#include "native_value/ark_native_object.h"

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
using panda::PropertyAttribute;
static constexpr auto PANDA_MAIN_FUNCTION = "_GLOBAL::func_main_0";
static constexpr auto PANDA_MODULE_NAME = "_GLOBAL_MODULE_NAME";
static const auto PANDA_MODULE_NAME_LEN = 32;
#if !defined(PREVIEW) && !defined(IOS_PLATFORM)
static constexpr uint32_t DEC_TO_INT = 100;
static size_t g_threshold = OHOS::system::GetUintParameter<size_t>("persist.ark.leak.threshold", 85);
static constexpr int TIME_OUT = 20;
static size_t g_checkInterval = OHOS::system::GetUintParameter<size_t>("persist.ark.leak.checkinterval", 60);
static constexpr int DEFAULT_SLEEP_TIME = 100000; //poll every 0.1s
static bool g_enableProperty = OHOS::system::GetBoolParameter("persist.ark.leak.enableproperty", true);
static uint64_t g_lastHeapDumpTime = 0;
static bool g_debugLeak = OHOS::system::GetBoolParameter("debug.arkengine.tags.enableleak", false);
static constexpr uint64_t HEAP_DUMP_REPORT_INTERVAL = 24 * 3600 * 1000;
static constexpr uint64_t SEC_TO_MILSEC = 1000;
#endif
#ifdef ENABLE_HITRACE
constexpr auto NAPI_PROFILER_PARAM_SIZE = 10;
#endif

std::string ArkNativeEngine::tempModuleName_ {""};
bool ArkNativeEngine::napiProfilerEnabled {false};
bool ArkNativeEngine::napiProfilerParamReaded {false};
PermissionCheckCallback ArkNativeEngine::permissionCheckCallback_ {nullptr};

Local<JSValueRef> NapiValueToLocalValue(napi_value v)
{
    auto nativeValue = reinterpret_cast<NativeValue*>(v);
    auto engine = reinterpret_cast<ArkNativeEngine*>(nativeValue->GetEngine());
    Global<JSValueRef> result = *nativeValue;
    return result.ToLocal(engine->GetEcmaVm());
}

inline napi_value JsValueFromLocalValue(Local<panda::JSValueRef> local)
{
    return reinterpret_cast<napi_value>(*local);
}

inline Local<panda::JSValueRef> LocalValueFromJsValue(napi_value v)
{
    Local<panda::JSValueRef> local;
    memcpy(static_cast<void*>(&local), &v, sizeof(v));
    return local;
}

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

void* ArkNativeEngine::GetNativePtrCallBack(void* data)
{
    auto info = reinterpret_cast<NapiNativeFunctionInfo*>(data);
    auto cb = reinterpret_cast<void*>(info->callback);
    return cb;
}

ArkNativeEngine::ArkNativeEngine(EcmaVM* vm, void* jsEngine) : NativeEngine(jsEngine), vm_(vm), topScope_(vm)
{
    HILOG_DEBUG("ArkNativeEngine::ArkNativeEngine");
#if !defined(PREVIEW) && !defined(IOS_PLATFORM)
    if (g_enableProperty) {
        std::call_once(flag_, [this] {
            if (threadJsHeap_ == nullptr) {
                threadJsHeap_ = std::make_unique<std::thread>(&ArkNativeEngine::JsHeapStart, this);
                HILOG_ERROR("JsHeapStart is OK");
            }
        });
    }
#endif
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
                        Local<ObjectRef> exportObj = ObjectRef::New(ecmaVm);
                        if (!arkNativeEngine) {
                            HILOG_ERROR("init module failed");
                            scopeManager->Close(nativeScope);
                            return scope.Escape(exports);
                        }
#ifdef ENABLE_HITRACE
                        StartTrace(HITRACE_TAG_ACE, "NAPI module init, name = " + std::string(module->name));
#endif
                        arkNativeEngine->SetModuleName(exportObj, module->name);
                        module->registerCallback(reinterpret_cast<napi_env>(arkNativeEngine),
                                                JsValueFromLocalValue(exportObj));
#ifdef ENABLE_HITRACE
                        FinishTrace(HITRACE_TAG_ACE);
#endif
                        exports = exportObj;
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
                    Local<ObjectRef> exportObj = ObjectRef::New(ecmaVm);
                    if (!exportObj->IsNull()) {
                        if (!arkNativeEngine) {
                            HILOG_ERROR("exportObject is nullptr");
                            scopeManager->Close(nativeScope);
                            return scope.Escape(exports);
                        }
                        arkNativeEngine->SetModuleName(exportObj, module->name);
                        module->registerCallback(reinterpret_cast<napi_env>(arkNativeEngine),
                                                JsValueFromLocalValue(exportObj));
                        exports = exportObj;
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
    JSNApi::SetNativePtrGetter(vm, reinterpret_cast<void*>(ArkNativeEngine::GetNativePtrCallBack));
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
#if !defined(PREVIEW) && !defined(IOS_PLATFORM)
    // Free threadJsHeap_
    needStop_ = true;
    condition_.notify_all();
    if (threadJsHeap_->joinable()) {
        threadJsHeap_->join();
    }
#endif
}

const EcmaVM* ArkNativeEngine::GetEcmaVm() const
{
    return vm_;
}

static inline void StartNapiProfilerTrace(panda::JsiRuntimeCallInfo *runtimeInfo)
{
#ifdef ENABLE_HITRACE
        if (ArkNativeEngine::napiProfilerEnabled) {
            EcmaVM *vm = runtimeInfo->GetVM();
            LocalScope scope(vm);
            Local<panda::FunctionRef> fn = runtimeInfo->GetFunctionRef();
            Local<panda::StringRef> nameRef = fn->GetName(vm);
            char threadName[128];
            prctl(PR_GET_NAME, threadName);
            StartTraceArgs(HITRACE_TAG_ACE, "Napi called:%s, tname:%s", nameRef->ToString().c_str(), threadName);
        }
#endif
}

static inline void FinishNapiProfilerTrace()
{
#ifdef ENABLE_HITRACE
    if (ArkNativeEngine::napiProfilerEnabled) {
        FinishTrace(HITRACE_TAG_ACE);
    }
#endif
}

Local<panda::JSValueRef> ArkNativeFunctionCallBack(JsiRuntimeCallInfo *runtimeInfo)
{
    EcmaVM *vm = runtimeInfo->GetVM();
    panda::EscapeLocalScope scope(vm);
    auto info = reinterpret_cast<NapiNativeFunctionInfo*>(runtimeInfo->GetData());
    auto engine = reinterpret_cast<NativeEngine*>(info->env);
    auto cb = info->callback;
    if (engine == nullptr) {
        HILOG_ERROR("native engine is null");
        return JSValueRef::Undefined(vm);
    }

    uint32_t MAX_CHUNK_ARRAY_SIZE = 10;
    NapiNativeCallbackInfo cbInfo = { 0 };
    StartNapiProfilerTrace(runtimeInfo);
    cbInfo.thisVar = JsValueFromLocalValue(runtimeInfo->GetThisRef());
    cbInfo.function = JsValueFromLocalValue(runtimeInfo->GetNewTargetRef());
    cbInfo.argc = static_cast<size_t>(runtimeInfo->GetArgsNumber());
    cbInfo.argv = nullptr;
    cbInfo.functionInfo = info;
    if (cbInfo.argc > 0) {
        if (cbInfo.argc > MAX_CHUNK_ARRAY_SIZE) {
            cbInfo.argv = new napi_value [cbInfo.argc];
        }
        for (size_t i = 0; i < cbInfo.argc; i++) {
            cbInfo.argv[i] = JsValueFromLocalValue(runtimeInfo->GetCallArgRef(i));
        }
    }

    if (JSNApi::IsMixedDebugEnabled(vm)) {
        JSNApi::NotifyNativeCalling(vm, reinterpret_cast<void *>(cb));
    }

    napi_value result = nullptr;
    if (cb != nullptr) {
        result = cb(info->env, &cbInfo);
    }

    if (cbInfo.argv != nullptr) {
        if (cbInfo.argc > MAX_CHUNK_ARRAY_SIZE) {
            delete[] cbInfo.argv;
        }
        cbInfo.argv = nullptr;
    }

    Local<panda::JSValueRef> localRet = panda::JSValueRef::Undefined(vm);
    if (result != nullptr) {
        localRet = LocalValueFromJsValue(result);
    }

    FinishNapiProfilerTrace();
    if (localRet.IsEmpty()) {
        return JSValueRef::Undefined(vm);
    }
    return localRet;
}

Local<panda::JSValueRef> NapiNativeCreateFunction(napi_env env, const char* name, NapiNativeCallback cb, void* value)
{
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());
    NapiNativeFunctionInfo* funcInfo = NapiNativeFunctionInfo::CreateNewInstance();
    if (funcInfo == nullptr) {
        HILOG_ERROR("funcInfo is nullptr");
        return JSValueRef::Undefined(vm);
    }
    funcInfo->env = env;
    funcInfo->callback = cb;
    funcInfo->data = value;

    Local<panda::FunctionRef> fn = panda::FunctionRef::New(vm, ArkNativeFunctionCallBack,
                                             [](void* externalPointer, void* data) {
                                                auto info = reinterpret_cast<NapiNativeFunctionInfo*>(data);
                                                if (info != nullptr) {
                                                    delete info;
                                                }
                                             },
                                             reinterpret_cast<void*>(funcInfo), true);
    Local<panda::StringRef> fnName = panda::StringRef::NewFromUtf8(vm, name);
    fn->SetName(vm, fnName);
    return fn;
}

Local<JSValueRef> GetProperty(EcmaVM* vm, Local<panda::ObjectRef> &obj, const char* name)
{

    Local<StringRef> key = StringRef::NewFromUtf8(vm, name);
    Local<JSValueRef> val = obj->Get(vm, key);
    return val;
}

void GetCString(EcmaVM* vm, Local<StringRef> str, char* buffer, size_t size, size_t* length)
{
    if (length == nullptr) {
        return;
    }
    if (buffer == nullptr) {
        *length = str->Utf8Length(vm) - 1;
    } else if (size != 0) {
        int copied = str->WriteUtf8(buffer, size - 1, true) - 1;
        buffer[copied] = '\0';
        *length = copied;
    } else {
        *length = 0;
    }
}

std::string NapiGetModuleName(napi_env env, Local<panda::ObjectRef> &obj)
{
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());
    std::string moduleName("");
    auto nativeModuleName = GetProperty(vm, obj, PANDA_MODULE_NAME);
    if (nativeModuleName->IsString()) {
        char arrayName[PANDA_MODULE_NAME_LEN] = {0};
        size_t len = 0;
        GetCString(vm, nativeModuleName, arrayName, PANDA_MODULE_NAME_LEN, &len);
        moduleName += arrayName;
        moduleName += ".";
    }
    return moduleName;
}

bool NapiNativeDefineProperty(napi_env env, Local<panda::ObjectRef> &obj, Napi_NativePropertyDescriptor propertyDescriptor)
{
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto vm = engine->GetEcmaVm();
    bool result = false;
    Local<panda::StringRef> propertyName = panda::StringRef::NewFromUtf8(vm, propertyDescriptor.utf8name);

    bool writable = (propertyDescriptor.attributes & NATIVE_WRITABLE) != 0;
    bool enumable = (propertyDescriptor.attributes & NATIVE_ENUMERABLE) != 0;
    bool configable = (propertyDescriptor.attributes & NATIVE_CONFIGURABLE) != 0;

    std::string fullName("");
#ifdef ENABLE_HITRACE
    fullName += NapiGetModuleName(env, obj);
#endif
    if (propertyDescriptor.getter != nullptr || propertyDescriptor.setter != nullptr) {
        Local<panda::JSValueRef> localGetter = panda::JSValueRef::Undefined(vm);
        Local<panda::JSValueRef> localSetter = panda::JSValueRef::Undefined(vm);

        if (propertyDescriptor.getter != nullptr) {
            fullName += "getter";
            localGetter = NapiNativeCreateFunction(env, fullName.c_str(),
                                             propertyDescriptor.getter, propertyDescriptor.data);
        }
        if (propertyDescriptor.setter != nullptr) {
            fullName += "setter";
            localSetter = NapiNativeCreateFunction(env, fullName.c_str(),
                                             propertyDescriptor.setter, propertyDescriptor.data);
        }

        PropertyAttribute attr(panda::JSValueRef::Undefined(vm), false, enumable, configable);
        result = obj->SetAccessorProperty(vm, propertyName, localGetter, localSetter, attr);
    } else if (propertyDescriptor.method != nullptr) {
        fullName += propertyDescriptor.utf8name;
        Local<panda::JSValueRef> cbObj = NapiNativeCreateFunction(env, fullName.c_str(),
                                             propertyDescriptor.method, propertyDescriptor.data);
        PropertyAttribute attr(cbObj, writable, enumable, configable);
        result = obj->DefineProperty(vm, propertyName, attr);
    } else {
        Local<panda::JSValueRef> val = LocalValueFromJsValue(propertyDescriptor.value);

        PropertyAttribute attr(val, writable, enumable, configable);
        result = obj->DefineProperty(vm, propertyName, attr);
    }
    Local<panda::ObjectRef> excep = panda::JSNApi::GetUncaughtException(vm);
    if (!excep.IsNull()) {
        HILOG_ERROR("ArkNativeObject::DefineProperty occur Exception");
        panda::JSNApi::GetAndClearUncaughtException(vm);
    }
    return result;
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
        Local<StringRef> idStr = StringRef::NewFromUtf8(vm_, id.c_str(), id.size());
        napi_value idValue = JsValueFromLocalValue(idStr);
        Local<StringRef> paramStr = StringRef::NewFromUtf8(vm_, param.c_str(), param.size());
        napi_value paramValue = JsValueFromLocalValue(paramStr);
        Local<ObjectRef> exportObj = ObjectRef::New(vm_);
        Napi_NativePropertyDescriptor idProperty, paramProperty;
        idProperty.utf8name = "id";
        idProperty.value = idValue;
        paramProperty.utf8name = "param";
        paramProperty.value = paramValue;
        SetModuleName(exportObj, module->name);
        NapiNativeDefineProperty(reinterpret_cast<napi_env>(this), exportObj, idProperty);
        NapiNativeDefineProperty(reinterpret_cast<napi_env>(this), exportObj, paramProperty);
        MoudleNameLocker nameLocker(module->name);
        module->registerCallback(reinterpret_cast<napi_env>(this), JsValueFromLocalValue(exportObj));
        napi_value nExport = JsValueFromLocalValue(exportObj);
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
        exports = exportObj;
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
        Local<ObjectRef> exportObj = ObjectRef::New(vm_);
        Napi_NativePropertyDescriptor paramProperty, instanceProperty;
        Local<StringRef> paramStr = StringRef::NewFromUtf8(vm_, param.c_str(), param.size());
        napi_value paramValue = JsValueFromLocalValue(paramStr);
        paramProperty.utf8name = "param";
        paramProperty.value = paramValue;
        Local<ObjectRef> instanceValue = ObjectRef::New(vm_);
        Global<ObjectRef> value(vm_, instanceValue);
        Local<StringRef> key = StringRef::GetNapiWrapperString(vm_);
        if (instance == nullptr && value->Has(vm_, key)) {
            Local<ObjectRef> wrapper = value->Get(vm_, key);
            auto ref = reinterpret_cast<NativeReference*>(wrapper->GetNativePointerField(0));
            wrapper->SetNativePointerField(0, nullptr, nullptr, nullptr, 0);
            value->Delete(vm_, key);
            delete ref;
        } else {
            Local<ObjectRef> object = ObjectRef::New(vm_);
            NativeReference* ref = nullptr;
            ref = new NativeReference(this, value.ToLocal(vm_), 0, true, nullptr, instance, nullptr);

            object->SetNativePointerFieldCount(1);
            object->SetNativePointerField(0, ref, nullptr, nullptr, 0);
            PropertyAttribute attr(object, true, false, true);
            value->DefineProperty(vm_, key, attr);
        }
        instanceProperty.utf8name = instanceName.c_str();
        instanceProperty.value = JsValueFromLocalValue(instanceValue);
        SetModuleName(exportObj, module->name);
        NapiNativeDefineProperty(reinterpret_cast<napi_env>(this), exportObj, paramProperty);
        NapiNativeDefineProperty(reinterpret_cast<napi_env>(this), exportObj, instanceProperty);

        MoudleNameLocker nameLocker(module->name);
        module->registerCallback(reinterpret_cast<napi_env>(this), JsValueFromLocalValue(exportObj));
        exports = exportObj;
    }
    return scope.Escape(exports);
}

void ArkNativeEngine::Loop(LoopMode mode, bool needSync)
{
    LocalScope scope(vm_);
    NativeEngine::Loop(mode, needSync);
    panda::JSNApi::ExecutePendingJob(vm_);
}

inline void ArkNativeEngine::SetModuleName(Local<ObjectRef> &nativeObj, std::string moduleName)
{
#ifdef ENABLE_HITRACE
    if (ArkNativeEngine::napiProfilerEnabled) {
        Local<StringRef> moduleNameStr = StringRef::NewFromUtf8(vm_, moduleName.c_str(), moduleName.size());
        Local<StringRef> key = StringRef::NewFromUtf8(vm_, NativeObject::PANDA_MODULE_NAME);
        nativeObj->Set(vm_, key, moduleNameStr);
    }
#endif
}

// NativeValue* ArkNativeEngine::GetGlobal()
// {
//     LocalScope scope(vm_);
//     Local<ObjectRef> value = panda::JSNApi::GetGlobalObject(vm_);
//     return ArkValueToNativeValue(this, value);
// }

// NativeValue* ArkNativeEngine::CreateNull()
// {
//     LocalScope scope(vm_);
//     Local<PrimitiveRef> value = JSValueRef::Null(vm_);
//     return GetNativeChunk().New<ArkNativeValue>(this, value);
// }

// NativeValue* ArkNativeEngine::CreateUndefined()
// {
//     LocalScope scope(vm_);
//     Local<PrimitiveRef> value = JSValueRef::Undefined(vm_);
//     return GetNativeChunk().New<ArkNativeValue>(this, value);
// }

// NativeValue* ArkNativeEngine::CreateSymbol(NativeValue* value)
// {
//     LocalScope scope(vm_);
//     Global<StringRef> str = *value;
//     Local<SymbolRef> symbol = SymbolRef::New(vm_, str.ToLocal(vm_));
//     return GetNativeChunk().New<ArkNativeValue>(this, symbol);
// }

// NativeValue* ArkNativeEngine::CreateObject()
// {
//     return GetNativeChunk().New<ArkNativeObject>(this);
// }

// NativeValue* ArkNativeEngine::CreateFunction(const char* name, size_t length, NativeCallback cb, void* value)
// {
//     return GetNativeChunk().New<ArkNativeFunction>(this, name, length, cb, value);
// }

// NativeValue* ArkNativeEngine::CreateError(NativeValue* code, NativeValue* message)
// {
//     LocalScope scope(vm_);
//     Local<JSValueRef> errorVal = panda::Exception::Error(vm_, *message);
//     if (code != nullptr) {
//         Local<StringRef> codeKey = StringRef::NewFromUtf8(vm_, "code");
//         Local<ObjectRef> errorObj(errorVal);
//         errorObj->Set(vm_, codeKey, *code);
//     }
//     return ArkValueToNativeValue(this, errorVal);
// }

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
    napi_env env = reinterpret_cast<napi_env>(engine);
    concurrentCallbackFunc(env, ArkNativeEngine::ArkValueToNapiValue(env, result), success, taskInfo);
}

bool ArkNativeEngine::InitTaskPoolThread(NativeEngine* engine, NapiConcurrentCallback callback)
{
    concurrentCallbackFunc_ = callback;
    return JSNApi::InitForConcurrentThread(vm_, ConcurrentCallbackFunc, static_cast<void *>(this));
}

bool ArkNativeEngine::InitTaskPoolThread(napi_env env, NapiConcurrentCallback callback)
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

bool ArkNativeEngine::InitTaskPoolFunc(napi_env env, napi_value func, void* taskInfo)
{
    LocalScope scope(vm_);
    NativeValue* funcVal = reinterpret_cast<NativeValue*>(func);
    Global<JSValueRef> globalObj = *funcVal;
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
    napi_value thisVar, NativeValue* function, NativeValue* const* argv, size_t argc)
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
//        Global<JSValueRef> globalObj = *thisVar;
        thisObj = LocalValueFromJsValue(thisVar);
    }
    Global<FunctionRef> funcObj = *function;
#ifdef ENABLE_CONTAINER_SCOPE
    // auto nativeFunction = static_cast<NativeFunction*>(function->GetInterface(NativeFunction::INTERFACE_ID));
    // if (nativeFunction == nullptr) {
    //     HILOG_ERROR("nativeFunction is null");
    //     return nullptr;
    // }
    // auto arkNativeFunc = static_cast<ArkNativeFunction*>(nativeFunction);
    OHOS::Ace::ContainerScope containerScope( OHOS::Ace::ContainerScope::CurrentId());
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

// NativeValue* ArkNativeEngine::RunScript(NativeValue* script)
// {
//     return nullptr;
// }

void* ArkNativeEngine::RunScriptPath(const char* path)
{
    panda::JSExecutionScope executionScope(vm_);
    LocalScope scope(vm_);
    [[maybe_unused]] bool ret = panda::JSNApi::Execute(vm_, path, PANDA_MAIN_FUNCTION);
    if (panda::JSNApi::HasPendingException(vm_)) {
        HandleUncaughtException();
        return nullptr;
    }
    return nullptr;
}

bool ArkNativeEngine::SuspendVMById(uint32_t tid)
{
#if !defined(PREVIEW) && !defined(IOS_PLATFORM)
    return DFXJSNApi::SuspendVMById(vm_, tid);
#else
    HILOG_WARN("ARK does not support dfx on windows");
    return false;
#endif
}

void ArkNativeEngine::ResumeVMById(uint32_t tid)
{
#if !defined(PREVIEW) && !defined(IOS_PLATFORM)
    DFXJSNApi::ResumeVMById(vm_, tid);
#else
    HILOG_WARN("ARK does not support dfx on windows");
    return;
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
    Local<JSValueRef> undefObj = JSValueRef::Undefined(vm_);
    return ArkValueToNativeValue(this, undefObj);
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
    Global<JSValueRef> arkValue = *value;
    return new NativeReference(this, arkValue.ToLocal(vm_), initialRefcount, false, callback, data, hint);
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

// NativeValue* ArkNativeEngine::Serialize(NativeEngine* context, NativeValue* value, NativeValue* transfer)
// {
//     const panda::ecmascript::EcmaVM* vm = reinterpret_cast<ArkNativeEngine*>(context)->GetEcmaVm();
//     LocalScope scope(vm);
//     Global<JSValueRef> arkValue = *value;
//     Global<JSValueRef> arkTransfer = *transfer;
//     void* result = panda::JSNApi::SerializeValue(vm, arkValue.ToLocal(vm), arkTransfer.ToLocal(vm));
//     return reinterpret_cast<NativeValue*>(result);
// }

// NativeValue* ArkNativeEngine::Deserialize(NativeEngine* context, NativeValue* recorder)
// {
//     const panda::ecmascript::EcmaVM* vm = reinterpret_cast<ArkNativeEngine*>(context)->GetEcmaVm();
//     LocalScope scope(vm);
//     Local<JSValueRef> result = panda::JSNApi::DeserializeValue(vm, recorder, reinterpret_cast<void*>(context));
//     return ArkValueToNativeValue(this, result);
// }

// void ArkNativeEngine::DeleteSerializationData(NativeValue* value) const
// {
//     void* data = reinterpret_cast<void*>(value);
//     panda::JSNApi::DeleteSerializationData(data);
// }

void ArkNativeEngine::StartCpuProfiler(const std::string& fileName)
{
    JSNApi::SetNativePtrGetter(vm_, reinterpret_cast<void*>(ArkNativeEngine::GetNativePtrCallBack));
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
    Local<JSValueRef> undefObj = JSValueRef::Undefined(vm_);
    return ArkValueToNativeValue(this, undefObj);
}

napi_value ArkNativeEngine::RunActor(std::vector<uint8_t>& buffer, const char* descriptor)
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
    Local<JSValueRef> undefObj = JSValueRef::Undefined(vm_);
    return JsValueFromLocalValue(undefObj);
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

// NativeValue* ArkNativeEngine::LoadModule(NativeValue* str, const std::string& fileName)
// {
//     return nullptr;
// }

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
//        result = chunk.New<ArkNativeNumber>(engine, value);
    } else if (value->IsString()) {
//        result = chunk.New<ArkNativeString>(engine, value);
    } else if (value->IsArray(engine->GetEcmaVm())) {
//        result = chunk.New<ArkNativeArray>(engine, value);
    } else if (value->IsFunction()) {
//        result = chunk.New<ArkNativeFunction>(engine, value);
    } else if (value->IsArrayBuffer()) {
//        result = chunk.New<ArkNativeArrayBuffer>(engine, value);
    } else if (value->IsBuffer()) {
//        result = chunk.New<ArkNativeBuffer>(engine, value);
    } else if (value->IsDataView()) {
//        result = chunk.New<ArkNativeDataView>(engine, value);
    } else if (value->IsTypedArray()) {
//        result = chunk.New<ArkNativeTypedArray>(engine, value);
    } else if (value->IsNativePointer()) {
//        result = chunk.New<ArkNativeExternal>(engine, value);
    } else if (value->IsDate()) {
//        result = chunk.New<ArkNativeDate>(engine, value);
    } else if (value->IsBigInt()) {
//       result = chunk.New<ArkNativeBigInt>(engine, value);
    } else if (value->IsObject() || value->IsPromise()) {
//        result = chunk.New<ArkNativeObject>(engine, value);
    } else if (value->IsBoolean()) {
//        result = chunk.New<ArkNativeBoolean>(engine, value);
    } else {
        result = chunk.New<ArkNativeValue>(engine, value);
    }
    return result;
}

napi_value ArkNativeEngine::ValueToNapiValue(JSValueWrapper& value)
{
    return reinterpret_cast<napi_value>(ValueToNativeValue(value));
}

NativeValue* ArkNativeEngine::ValueToNativeValue(JSValueWrapper& value)
{
    LocalScope scope(vm_);
    Global<JSValueRef> arkValue = value;
    return ArkValueToNativeValue(this, arkValue.ToLocal(vm_));
}

napi_value ArkNativeEngine::ArkValueToNapiValue(napi_env env, Local<JSValueRef> value)
{
    return reinterpret_cast<napi_value>(ArkValueToNativeValue(reinterpret_cast<ArkNativeEngine*>(env), value));
}

std::string ArkNativeEngine::GetSourceCodeInfo(napi_value value, ErrorPos pos)
{
    if (value == nullptr || pos.first == 0) {
        return "";
    }

    LocalScope scope(vm_);
    Local<panda::FunctionRef> func = NapiValueToLocalValue(value);
    uint32_t line = pos.first;
    uint32_t column = pos.second;
    Local<panda::StringRef> sourceCode = func->GetSourceCode(vm_, line);
    std::string sourceCodeStr = sourceCode->ToString();
    if (sourceCodeStr.empty()) {
        return "";
    }
    std::string sourceCodeInfo = "SourceCode:\n";
    sourceCodeInfo.append(sourceCodeStr).append("\n");
    for (uint32_t k = 0; k < column - 1; k++) {
        sourceCodeInfo.push_back(' ');
    }
    sourceCodeInfo.append("^\n");
    return sourceCodeInfo;
}

bool ArkNativeEngine::ExecuteJsBin(const std::string& fileName)
{
    panda::JSExecutionScope executionScope(vm_);
    LocalScope scope(vm_);
    bool ret = JSNApi::Execute(vm_, fileName, PANDA_MAIN_FUNCTION);
    return ret;
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

void ArkNativeEngine::DumpHeapSnapshot(bool isVmMode, DumpFormat dumpFormat, bool isPrivate, bool isFullGC)
{
    if (dumpFormat == DumpFormat::JSON) {
        DFXJSNApi::DumpHeapSnapshot(vm_, 0, isVmMode, isPrivate, false, isFullGC);
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

size_t ArkNativeEngine::GetHeapLimitSize()
{
    return DFXJSNApi::GetHeapLimitSize(vm_);
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

void ArkNativeEngine::NotifyForceExpandState(int32_t value)
{
    switch (ForceExpandState(value)) {
        case ForceExpandState::FINISH_COLD_START:
            DFXJSNApi::NotifyFinishColdStart(vm_, true);
            break;
        case ForceExpandState::START_HIGH_SENSITIVE:
            DFXJSNApi::NotifyHighSensitive(vm_, true);
            break;
        case ForceExpandState::FINISH_HIGH_SENSITIVE:
            DFXJSNApi::NotifyHighSensitive(vm_, false);
            break;
        default:
            HILOG_ERROR("Invalid Force Expand State: %{public}d.", value);
            break;
    }
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

size_t ArkNativeEngine::GetHeapLimitSize()
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

void ArkNativeEngine::NotifyForceExpandState([[maybe_unused]] int32_t value)
{
    HILOG_WARN("ARK does not support dfx on windows");
}
#endif

void ArkNativeEngine::SetMockModuleList(const std::map<std::string, std::string> &list)
{
    JSNApi::SetMockModuleList(vm_, list);
}

void ArkNativeEngine::RegisterUncaughtExceptionHandler(UncaughtExceptionCallback callback)
{
    JSNApi::EnableUserUncaughtErrorHandler(vm_);
    uncaughtExceptionCallback_ = callback;
}

void ArkNativeEngine::RegisterNapiUncaughtExceptionHandler(NapiUncaughtExceptionCallback callback)
{
    JSNApi::EnableUserUncaughtErrorHandler(vm_);
    napiUncaughtExceptionCallback_ = callback;
}

void ArkNativeEngine::HandleUncaughtException()
{
    if (uncaughtExceptionCallback_ == nullptr && napiUncaughtExceptionCallback_ == nullptr) {
        return;
    }
    LocalScope scope(vm_);
    Local<ObjectRef> exception = JSNApi::GetAndClearUncaughtException(vm_);
    if (!exception.IsEmpty() && !exception->IsHole()) {
        if (uncaughtExceptionCallback_ != nullptr) {
            uncaughtExceptionCallback_(ArkValueToNativeValue(this, exception));
        } 
        if (napiUncaughtExceptionCallback_ != nullptr) {
            napiUncaughtExceptionCallback_(ArkValueToNapiValue(reinterpret_cast<napi_env>(this), exception));
        }
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

void ArkNativeEngine::RegisterSourceMapTranslateCallback(SourceMapTranslateCallback callback)
{
    panda::JSNApi::SetSourceMapTranslateCallback(vm_, callback);
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

#if !defined(PREVIEW) && !defined(IOS_PLATFORM)
uint64_t ArkNativeEngine::GetCurrentTickMillseconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

void ArkNativeEngine::JudgmentDump(size_t limitSize)
{
    if (!limitSize) {
        return;
    }
    size_t nowPrecent = GetHeapUsedSize() * DEC_TO_INT / limitSize;
    if (g_debugLeak || (nowPrecent >= g_threshold && (g_lastHeapDumpTime == 0 ||
        GetCurrentTickMillseconds() - g_lastHeapDumpTime > HEAP_DUMP_REPORT_INTERVAL))) {
        HILOG_INFO("dumpheapSnapshot ready");
        int pid = -1;
        DFXJSNApi::GetHeapPrepare(vm_);
        time_t startTime = time(nullptr);
        g_lastHeapDumpTime = GetCurrentTickMillseconds();
        if((pid = fork()) < 0) {
            HILOG_ERROR("ready dumpheapSnapshot Fork error, err:%{public}d", errno);
            sleep(g_checkInterval);
            return;
        }
        if (pid == 0) {
            AllowCrossThreadExecution();
            DumpHeapSnapshot(true, DumpFormat::JSON, false, false);
            HILOG_INFO("dumpheapSnapshot successful, now you can check some file");
            _exit(0);
        }
        while (true) {
            int status = 0;
            pid_t p = waitpid(pid, &status, 0);
            if (p < 0) {
                HILOG_ERROR("dumpheapSnapshot Waitpid return p=%{public}d, err:%{public}d", p, errno);
                break;
            }
            if (p == pid) {
                HILOG_ERROR("dumpheapSnapshot dump process exited status is %{public}d", status);
                break;
            }
            if (time(nullptr) > startTime + TIME_OUT) {
                HILOG_ERROR("time out to wait child process, killing forkpid %{public}d", pid);
                kill(pid, SIGKILL);
                break;
            }
            usleep(DEFAULT_SLEEP_TIME);
        }
    }
}

void ArkNativeEngine::JsHeapStart()
{
    if (pthread_setname_np(pthread_self(), "JsHeapThread") != 0) {
        HILOG_ERROR("Failed to set threadName for JsHeap, errno:%d", errno);
    }
    while (!needStop_) {
        std::unique_lock<std::mutex> lock(lock_);
        condition_.wait_for(lock, std::chrono::milliseconds(g_checkInterval * SEC_TO_MILSEC));
        if (needStop_) {
            return;
        }
        size_t limitSize = GetHeapLimitSize();
        JudgmentDump(limitSize);
    }
}
#endif
