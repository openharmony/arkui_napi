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

#include "check_helper.h"

#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <ostream>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "ark_native_engine.h"
#include "jsnapi.h"
#include "native_utils.h"
#include "napi/native_node_api.h"

namespace ohos::check {
namespace {

// Defensive recursion cap: deeper nesting means a self-referential or abnormal structure.
constexpr int MAX_TRAVERSE_DEPTH = 8;

LogLevel g_logLevel = LogLevel::INFO;
std::ofstream g_logFile;
std::mutex g_logMutex;

const char* LogLevelName(LogLevel level)
{
    switch (level) {
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::DEBUG: return "DEBUG";
        default:              return "LOG";
    }
}

std::vector<std::string> SplitByComma(const std::string& value)
{
    std::vector<std::string> result;
    size_t start = 0;
    while (start < value.size()) {
        size_t pos = value.find(',', start);
        if (pos == std::string::npos) {
            result.push_back(value.substr(start));
            break;
        }
        result.push_back(value.substr(start, pos - start));
        start = pos + 1;
    }
    return result;
}

// Module name -> d.ts namespace name (last dot segment), matching OpenHarmony d.ts convention.
std::string GetNamespaceOfModule(const std::string& moduleName)
{
    size_t pos = moduleName.find_last_of('.');
    if (pos == std::string::npos || pos + 1 >= moduleName.size()) {
        return moduleName;
    }
    return moduleName.substr(pos + 1);
}

// Reads properties[index] as a string; outName keeps the napi value, outKey its UTF-8 text.
bool GetStringElement(napi_env env, napi_value properties, uint32_t index,
                      napi_value& outName, std::string& outKey)
{
    napi_value nameVal = nullptr;
    if (napi_get_element(env, properties, index, &nameVal) != napi_ok) {
        return false;
    }
    size_t len = 0;
    if (napi_get_value_string_utf8(env, nameVal, nullptr, 0, &len) != napi_ok) {
        return false;
    }
    std::vector<char> nameBuf(len + 1, '\0');
    size_t copied = 0;
    if (napi_get_value_string_utf8(env, nameVal, nameBuf.data(), nameBuf.size(), &copied) != napi_ok) {
        return false;
    }
    outName = nameVal;
    outKey.assign(nameBuf.data(), copied);
    return true;
}

void AppendApiMeta(TraverseContext& ctx, const std::string& key, const std::string& type,
                   const std::string& parentClass)
{
    ApiMeta meta;
    meta.apiText = key;
    meta.file = ctx.file;
    meta.apiType = type;
    meta.className = parentClass;
    meta.kit = ctx.kit;
    ctx.outMeta->push_back(meta);

    LogDebug() << "TraverseExports: " << key << ", apiType: " << type <<
        ", className: " << parentClass << ", file: " << ctx.file;
}

// VALUE: read ok; SKIP_ACCESSOR: accessor property recorded as field without reading it;
// SKIP_ERROR: descriptor or value read threw (e.g. proxy trap), skipped.
enum class PropReadResult {
    VALUE,
    SKIP_ACCESSOR,
    SKIP_ERROR,
};

// Fetches the built-in Object.getOwnPropertyDescriptor (reading it calls no accessors);
// false lets the caller fall back to direct value reads.
bool GetOwnPropertyDescriptorFn(napi_env env, napi_value& outFunc)
{
    outFunc = nullptr;
    napi_value global = nullptr;
    if (napi_get_global(env, &global) != napi_ok) {
        return false;
    }
    napi_value objectCtor = nullptr;
    if (napi_get_named_property(env, global, "Object", &objectCtor) != napi_ok) {
        ClearPendingException(env);
        return false;
    }
    if (napi_get_named_property(env, objectCtor, "getOwnPropertyDescriptor", &outFunc) != napi_ok) {
        ClearPendingException(env);
        return false;
    }
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, outFunc, &type) != napi_ok || type != napi_function) {
        // Must null out: a non-function handle would fail every call and silently skip
        // all properties (a false "pass" with zero exports).
        outFunc = nullptr;
        return false;
    }
    return true;
}

bool DescriptorHasAccessor(napi_env env, napi_value desc)
{
    for (const char* field : { "get", "set" }) {
        napi_value accessor = nullptr;
        if (napi_get_named_property(env, desc, field, &accessor) != napi_ok) {
            ClearPendingException(env);
            continue;
        }
        napi_valuetype type = napi_undefined;
        if (napi_typeof(env, accessor, &type) == napi_ok && type == napi_function) {
            return true;
        }
    }
    return false;
}

// Accessor values are never read: native getters on class prototypes expect an instance
// receiver and crash (SIGSEGV via napi_unwrap) when called with the prototype/exports object.
PropReadResult ReadExportProperty(napi_env env, napi_value descFunc, napi_value object,
                                  napi_value key, napi_value& outValue)
{
    outValue = nullptr;
    if (descFunc != nullptr) {
        napi_value global = nullptr;
        if (napi_get_global(env, &global) == napi_ok) {
            static const uint32_t argc = 2; // 2: getOwnPropertyDescriptor(object, key)
            napi_value args[argc] = { object, key };
            napi_value desc = nullptr;
            if (napi_call_function(env, global, descFunc, argc, args, &desc) != napi_ok) {
                // Proxy trap threw: a direct read would throw too; clear and skip.
                ClearPendingException(env);
                return PropReadResult::SKIP_ERROR;
            }
            napi_valuetype descType = napi_undefined;
            if (napi_typeof(env, desc, &descType) == napi_ok && descType == napi_object &&
                DescriptorHasAccessor(env, desc)) {
                return PropReadResult::SKIP_ACCESSOR;
            }
        }
    }
    if (napi_get_property(env, object, key, &outValue) != napi_ok) {
        ClearPendingException(env);
        return PropReadResult::SKIP_ERROR;
    }
    return PropReadResult::VALUE;
}

void RecurseIntoContainer(TraverseContext& ctx, napi_value value, const std::string& key,
                          const std::string& type, int depth);

void TraverseExportsImpl(TraverseContext& ctx, napi_value exports,
                         const std::string& parentClass, int depth)
{
    if (exports == nullptr || depth >= MAX_TRAVERSE_DEPTH) {
        return;
    }

    NativeEngine *engine = reinterpret_cast<NativeEngine *>(ctx.env);
    const EcmaVM *vm = engine->GetEcmaVm();
    auto jsVal = LocalValueFromJsValue(exports);
    if (jsVal->IsUndefined() || !jsVal->IsObject(vm)) {
        return;
    }

    auto retP = Local<panda::ObjectRef>(jsVal)->GetOwnPropertyNames(vm);
    napi_value properties = JsValueFromLocalValue(retP);

    uint32_t count = 0;
    if (napi_get_array_length(ctx.env, properties, &count) != napi_ok) {
        LogWarn() << "napi_get_array_length fail: " << ctx.file;
        ClearPendingException(ctx.env);
        return;
    }

    napi_value descFunc = nullptr;
    if (!GetOwnPropertyDescriptorFn(ctx.env, descFunc)) {
        // Built-in unavailable: fall back to direct reads (accessors will fire).
        LogWarn() << "Object.getOwnPropertyDescriptor unavailable, accessor-skip disabled: " << ctx.file;
    }

    for (uint32_t i = 0; i < count; ++i) {
        napi_value nameVal = nullptr;
        std::string key;
        if (!GetStringElement(ctx.env, properties, i, nameVal, key)) {
            ClearPendingException(ctx.env);
            continue;
        }

        if (IsIgnoredProperty(key.c_str())) {
            continue;
        }

        napi_value value = nullptr;
        PropReadResult readResult = ReadExportProperty(ctx.env, descFunc, exports, nameVal, value);
        if (readResult == PropReadResult::SKIP_ACCESSOR) {
            // Accessors are recorded as fields (readonly d.ts semantics); no read, no recursion.
            AppendApiMeta(ctx, key, "field", parentClass);
            LogDebug() << "accessor property recorded as field: " << key << ", className: " <<
                parentClass << ", file: " << ctx.file;
            continue;
        }
        if (readResult == PropReadResult::SKIP_ERROR) {
            // Unreadable: clear the exception so it cannot poison later modules, then skip.
            LogWarn() << "property unreadable (proxy trap or getter threw?): file: " << ctx.file <<
                ", key: " << key << ", className: " << parentClass;
            continue;
        }

        std::string type = GetApiType(ctx.env, value);
        AppendApiMeta(ctx, key, type, parentClass);
        RecurseIntoContainer(ctx, value, key, type, depth);
    }
}

// Expands classes (constructor + prototype) and field objects named in expandableNames.
void RecurseIntoContainer(TraverseContext& ctx, napi_value value, const std::string& key,
                          const std::string& type, int depth)
{
    if (type == "class") {
        TraverseExportsImpl(ctx, value, key, depth + 1);
        napi_value prototype = nullptr;
        if (napi_get_named_property(ctx.env, value, "prototype", &prototype) == napi_ok) {
            TraverseExportsImpl(ctx, prototype, key, depth + 1);
        }
        return;
    }
    if (type == "field" && ctx.expandableNames != nullptr && ctx.expandableNames->count(key) > 0) {
        napi_valuetype valueType;
        if (napi_typeof(ctx.env, value, &valueType) == napi_ok && valueType == napi_object) {
            TraverseExportsImpl(ctx, value, key, depth + 1);
        }
    }
}

bool MatchMethod(const ApiMeta &exposed, const ApiMeta &legal)
{
    // The parser normalizes declarations to "function NAME(" / "static NAME(" / "NAME(" forms.
    if (StartsWith(legal.apiText, "function " + exposed.apiText + "(")) {
        return true;
    }
    if (StartsWith(legal.apiText, "static " + exposed.apiText + "(")) {
        return true;
    }
    if (StartsWith(legal.apiText, exposed.apiText + "(")) {
        return true;
    }
    return false;
}

bool MatchNamespace(const ApiMeta &exposed, const ApiMeta &legal)
{
    return legal.apiText == "namespace " + exposed.apiText;
}

bool MatchClass(const ApiMeta &exposed, const ApiMeta &legal)
{
    return legal.apiText == "class " + exposed.apiText;
}

bool MatchInterface(const ApiMeta &exposed, const ApiMeta &legal)
{
    return legal.apiText == "interface " + exposed.apiText;
}

bool MatchEnum(const ApiMeta &exposed, const ApiMeta &legal)
{
    return legal.apiText == "enum " + exposed.apiText;
}

bool MatchExport(const ApiMeta &exposed, const ApiMeta &legal)
{
    return legal.apiText == "export " + exposed.apiText;
}

// d.ts type declarations: class/interface/enum/namespace/export — all describe top-level
// exported types/objects of the module.
bool IsTypeDeclApiType(const std::string& apiType)
{
    return apiType == "class" || apiType == "interface" || apiType == "enum_class" ||
           apiType == "namespace" || apiType == "export";
}

// method shape: text matching when the legal entry is method/interface/class/export.
bool MatchMethodApi(const ApiMeta &exposed, const ApiMeta &legal)
{
    if (legal.apiType == "method") {
        return MatchMethod(exposed, legal);
    }
    if (legal.apiType == "interface") {
        return MatchInterface(exposed, legal);
    }
    if (legal.apiType == "class") {
        return MatchClass(exposed, legal);
    }
    if (legal.apiType == "export") {
        return MatchExport(exposed, legal);
    }
    return false;
}

// class shape: text matching when the legal entry is class/enum_class/interface/namespace/export.
bool MatchClassApi(const ApiMeta &exposed, const ApiMeta &legal)
{
    if (legal.apiType == "class") {
        return MatchClass(exposed, legal);
    }
    // Deliberately no (class, method) match: a d.ts function implemented via napi_define_class
    // is a real behavioral break (class constructors need "new"), not type noise.
    if (legal.apiType == "enum_class") {
        return MatchEnum(exposed, legal);
    }
    if (legal.apiType == "interface") {
        return MatchInterface(exposed, legal);
    }
    if (legal.apiType == "namespace") {
        return MatchNamespace(exposed, legal);
    }
    if (legal.apiType == "export") {
        return MatchExport(exposed, legal);
    }
    return false;
}

// field shape: text matching when the legal entry is enum_class/interface/namespace/export.
bool MatchFieldApi(const ApiMeta &exposed, const ApiMeta &legal)
{
    if (legal.apiType == "enum_class") {
        return MatchEnum(exposed, legal);
    }
    if (legal.apiType == "interface") {
        return MatchInterface(exposed, legal);
    }
    if (legal.apiType == "namespace") {
        return MatchNamespace(exposed, legal);
    }
    if (legal.apiType == "export") {
        return MatchExport(exposed, legal);
    }
    return false;
}

bool MatchApiText(const ApiMeta &exposed, const ApiMeta &legal)
{
    if (legal.apiText == exposed.apiText) {
        return true;
    }
    if (exposed.apiType == "method") {
        return MatchMethodApi(exposed, legal);
    }
    if (exposed.apiType == "class") {
        return MatchClassApi(exposed, legal);
    }
    if (exposed.apiType == "field") {
        return MatchFieldApi(exposed, legal);
    }
    return false;
}

bool IsInherit(const ApiMeta &exposed, const ApiMeta &legal,
               const std::map<std::string, std::string>& inheritMap)
{
    const std::string& from = exposed.className;
    const std::string& to = legal.className;
    if (from == to) {
        return true;
    }

    // BFS up the inheritance chain; inheritMap values may be comma-separated parents.
    std::set<std::string> visited;
    std::queue<std::string> pending;
    visited.insert(from);
    pending.push(from);

    while (!pending.empty()) {
        std::string current = pending.front();
        pending.pop();
        auto it = inheritMap.find(current);
        if (it == inheritMap.end()) {
            continue;
        }
        for (const auto& parent : SplitByComma(it->second)) {
            if (parent.empty()) {
                continue;
            }
            if (parent == to) {
                return true;
            }
            if (visited.find(parent) == visited.end()) {
                visited.insert(parent);
                pending.push(parent);
            }
        }
    }
    return false;
}

// Extracts the matchable name used as index key: method names from "function NAME(" forms,
// type names from "class NAME" forms; bare apiText for fields/enum instances. Returns ""
// for shapes that are not single identifiers (never indexed, never matched).
std::string ExtractMatchName(const ApiMeta& entry)
{
    const std::string& text = entry.apiText;
    if (text.empty()) {
        return "";
    }
    if (entry.apiType == "method") {
        std::string s = text;
        for (const std::string& prefix : {"function ", "static "}) {
            if (StartsWith(s, prefix)) {
                s = s.substr(prefix.size());
            }
        }
        size_t pos = s.find('(');
        if (pos == std::string::npos) {
            // bare name without a parameter list
            return s.find(' ') == std::string::npos ? s : "";
        }
        if (pos == 0) {
            return "";
        }
        return s.substr(0, pos);
    }
    for (const std::string& kind : {"class ", "interface ", "enum ", "namespace ", "export "}) {
        if (StartsWith(text, kind)) {
            std::string name = text.substr(kind.size());
            // must be a single identifier, else not a normalized form
            if (!name.empty() && name.find(' ') == std::string::npos) {
                return name;
            }
            return "";
        }
    }
    if (text.find(' ') == std::string::npos) {
        return text;
    }
    return "";
}

// Type-combination matrix (exposed apiType vs legal apiType). (class, method) is
// deliberately absent: a d.ts function implemented via napi_define_class is a real
// behavioral break, not type noise — see MatchClassApi.
bool IsAllowedTypeCombination(const std::string& exposedType, const std::string& legalType)
{
    static const std::vector<std::pair<std::string, std::string>> ALLOWED_TYPE_COMBINATIONS = {
        {"method", "class"}, {"method", "interface"}, {"method", "export"},
        {"class", "namespace"}, {"class", "enum_class"}, {"class", "interface"},
        {"class", "export"}, {"field", "namespace"}, {"field", "enum_class"},
        {"field", "enum_instance"}, {"field", "interface"}, {"field", "export"},
    };
    if (exposedType == legalType) {
        return true;
    }
    for (const auto& combination : ALLOWED_TYPE_COMBINATIONS) {
        if (combination.first == exposedType && combination.second == legalType) {
            return true;
        }
    }
    return false;
}

// An empty legal file is a wildcard; EndsWith reconciles "api/@ohos.x" vs "@ohos.x" styles.
bool IsFileMatched(const ApiMeta& exposed, const ApiMeta& legal)
{
    if (legal.file.empty() || legal.file == exposed.file) {
        return true;
    }
    return EndsWith(legal.file, "/" + exposed.file);
}

} // namespace

void SetLogLevel(LogLevel level)
{
    g_logLevel = level;
}

LogLevel GetLogLevel()
{
    return g_logLevel;
}

bool ParseLogLevel(const std::string& name, LogLevel& out)
{
    std::string lower;
    lower.reserve(name.size());
    for (char c : name) {
        lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    if (lower == "error") {
        out = LogLevel::ERROR;
    } else if (lower == "warn" || lower == "warning") {
        out = LogLevel::WARN;
    } else if (lower == "info") {
        out = LogLevel::INFO;
    } else if (lower == "debug") {
        out = LogLevel::DEBUG;
    } else {
        return false;
    }
    return true;
}

bool SetLogFile(const std::string& path)
{
    std::lock_guard<std::mutex> lock(g_logMutex);
    if (g_logFile.is_open()) {
        g_logFile.close();
    }
    g_logFile.open(path, std::ios::out | std::ios::trunc);
    return g_logFile.is_open();
}

void CloseLogFile()
{
    std::lock_guard<std::mutex> lock(g_logMutex);
    if (g_logFile.is_open()) {
        g_logFile.close();
    }
}

void WriteLog(LogLevel level, const std::string& msg)
{
    std::lock_guard<std::mutex> lock(g_logMutex);
    const std::string line = std::string("[") + LogLevelName(level) + "] " + msg + "\n";
    if (g_logFile.is_open()) {
        g_logFile << line;
        g_logFile.flush();
    }
    // WARN/ERROR always go to stderr; other levels only without a log file.
    if (level <= LogLevel::WARN || !g_logFile.is_open()) {
        std::cerr << line;
    }
}

LogStream::LogStream(LogLevel level)
    : level_(level), enabled_(level <= GetLogLevel()) {}

LogStream::~LogStream()
{
    if (enabled_) {
        WriteLog(level_, stream_.str());
    }
}

LogStream LogError()
{
    return LogStream(LogLevel::ERROR);
}

LogStream LogWarn()
{
    return LogStream(LogLevel::WARN);
}

LogStream LogInfo()
{
    return LogStream(LogLevel::INFO);
}

LogStream LogDebug()
{
    return LogStream(LogLevel::DEBUG);
}

napi_env CreateArkEnv()
{
    panda::RuntimeOption option;
    option.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
    option.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::ERROR);
    option.SetDebuggerLibraryPath("");
    EcmaVM* vm = panda::JSNApi::CreateJSVM(option);
    if (vm == nullptr) {
        return nullptr;
    }
    ArkNativeEngine* arkEngine = new (std::nothrow) ArkNativeEngine(vm, nullptr);
    if (arkEngine == nullptr) {
        panda::JSNApi::DestroyJSVM(vm);
        return nullptr;
    }
    auto cleanEnv = [vm]() {
        if (vm != nullptr) {
            panda::JSNApi::DestroyJSVM(vm);
        }
    };
    arkEngine->SetCleanEnv(cleanEnv);
    napi_env env = reinterpret_cast<napi_env>(arkEngine);
    return env;
}

void DestroyArkEnv(napi_env env)
{
    if (env != nullptr) {
        napi_destroy_runtime(env);
    }
}

void ClearPendingException(napi_env env)
{
    bool pending = false;
    if (napi_is_exception_pending(env, &pending) == napi_ok && pending) {
        napi_value exception = nullptr;
        napi_get_and_clear_last_exception(env, &exception);
    }
}

ModuleCheckGuard::ModuleCheckGuard(napi_env env) : env_(env)
{
    // Clear the exception first: napi_open_handle_scope is also short-circuited by a pending one.
    ClearPendingException(env_);
    napi_open_handle_scope(env_, &scope_);
}

ModuleCheckGuard::~ModuleCheckGuard()
{
    if (env_ != nullptr) {
        ClearPendingException(env_);
        if (scope_ != nullptr) {
            napi_close_handle_scope(env_, scope_);
        }
    }
}

// Tradeoff: these names are skipped for EVERY module, so a module that legally exports an
// API called "name"/"length"/"constructor" etc. is not checked for it (miss, not false
// positive). Accepted because every runtime object carries these engine built-ins and the
// traversal has no per-module d.ts context to tell them apart.
bool IsIgnoredProperty(const char* name)
{
    if (!name || strlen(name) == 0) {
        return true;
    }
    // Engine-internal marker (_napiwrapper), not a declared API.
    if (strcmp(name, "_napiwrapper") == 0) {
        return true;
    }
    // Numeric reverse-mapping keys of TS-style enums have no whitelist entries.
    const char* p = name;
    if (*p == '-') {
        p++;
    }
    if (*p != '\0') {
        bool allDigits = true;
        for (; *p != '\0'; ++p) {
            if (*p < '0' || *p > '9') {
                allDigits = false;
                break;
            }
        }
        if (allDigits) {
            return true;
        }
    }
    return strcmp(name, "__proto__") == 0 ||
        strcmp(name, "constructor") == 0 ||
        strcmp(name, "length") == 0 ||
        strcmp(name, "name") == 0 ||
        strcmp(name, "prototype") == 0 ||
        strcmp(name, "toString") == 0 ||
        strcmp(name, "toLocaleString") == 0 ||
        strcmp(name, "valueOf") == 0 ||
        strcmp(name, "hasOwnProperty") == 0 ||
        strcmp(name, "isPrototypeOf") == 0 ||
        strcmp(name, "propertyIsEnumerable") == 0;
}

std::string GetApiType(napi_env env, napi_value value)
{
    napi_valuetype type;
    if (napi_typeof(env, value, &type) != napi_ok) {
        return "field";
    }

    if (type == napi_function) {
        NativeEngine *engine = reinterpret_cast<NativeEngine *>(env);
        const EcmaVM *vm = engine->GetEcmaVm();
        auto jsVal = LocalValueFromJsValue(value);
        if (jsVal->IsClassConstructor(vm)) {
            return "class";
        }
        return "method";
    }
    return "field";
}

void TraverseExports(TraverseContext& ctx, napi_value exports, const std::string& parentClass)
{
    TraverseExportsImpl(ctx, exports, parentClass, 0);
}

void BuildLegalIndex(const std::vector<ApiMeta>& legalList, LegalIndex& index)
{
    index.clear();
    for (const ApiMeta& entry : legalList) {
        std::string name = ExtractMatchName(entry);
        if (!name.empty()) {
            index[name].push_back(&entry);
        }
    }
}

void CollectExpandableNames(const std::vector<ApiMeta>& legalList, std::set<std::string>& names)
{
    names.clear();
    for (const ApiMeta& entry : legalList) {
        if (entry.apiType == "namespace" || entry.apiType == "enum_class" || entry.apiType == "class") {
            if (!entry.className.empty()) {
                names.insert(entry.className);
            }
            std::string name = ExtractMatchName(entry);
            if (!name.empty()) {
                names.insert(name);
            }
        }
    }
}

bool StartsWith(const std::string& str, const std::string& prefix)
{
    if (prefix.empty()) {
        return true;
    }
    if (str.size() < prefix.size()) {
        return false;
    }
    return str.compare(0, prefix.size(), prefix) == 0;
}

bool EndsWith(const std::string& str, const std::string& suffix)
{
    if (suffix.empty()) {
        return true;
    }
    if (str.size() < suffix.size()) {
        return false;
    }
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string NormalizeModuleName(const std::string& file)
{
    std::string moduleName = file;
    if (EndsWith(moduleName, ".d.ts")) {
        moduleName = moduleName.substr(0, moduleName.size() - 5); // 5: strlen(".d.ts")
    } else if (EndsWith(moduleName, ".d.ets")) {
        moduleName = moduleName.substr(0, moduleName.size() - 6); // 6: strlen(".d.ets")
    }
    return moduleName;
}

bool EqualsIgnoreCase(const std::string& a, const std::string& b)
{
    if (a.size() != b.size()) {
        return false;
    }
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) !=
            std::tolower(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}

std::ostream& operator<<(std::ostream& os, const ApiMeta& v)
{
    os << "apiText: " << v.apiText << std::endl;
    os << "file: " << v.file << std::endl;
    os << "apiType: " << v.apiType << std::endl;
    os << "className: " << v.className << std::endl;
    os << "kit: " << v.kit << std::endl;
    return os;
}

bool Match(const ApiMeta &exposed, const ApiMeta &legal, const std::map<std::string, std::string> &inheritMap)
{
    // Prescreen: the legal apiText must contain the exposed name.
    if (legal.apiText.find(exposed.apiText) == std::string::npos) {
        return false;
    }
    // An empty legal kit is a wildcard (ignore-list entries).
    if (!legal.kit.empty() && exposed.kit != legal.kit) {
        return false;
    }
    if (!IsAllowedTypeCombination(exposed.apiType, legal.apiType)) {
        return false;
    }
    // className is the declaration scope and is compared case-insensitively (d.ts vs runtime
    // case drift). Top-level exports (empty className) match module-namespace members or type
    // declarations; class members may match declarations inherited from parents (cross-file).
    if (!EqualsIgnoreCase(legal.className, exposed.className)) {
        if (exposed.className.empty()) {
            bool inModuleNamespace = !legal.className.empty() &&
                EqualsIgnoreCase(legal.className, GetNamespaceOfModule(exposed.file));
            if (!inModuleNamespace && !IsTypeDeclApiType(legal.apiType)) {
                return false;
            }
        } else {
            // Cross-file inheritance path: the parent may live in another declaration file,
            // so the file check does not apply here.
            return IsInherit(exposed, legal, inheritMap) && MatchApiText(exposed, legal);
        }
    }
    if (!IsFileMatched(exposed, legal)) {
        return false;
    }
    return MatchApiText(exposed, legal);
}

}
