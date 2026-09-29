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

#include "load_abc.h"

#include <climits>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <fstream>
#include <sys/stat.h>

#include "ark_native_engine.h"
#include "check_helper.h"
#include "jsnapi.h"
#include "native_utils.h"

namespace ohos::check {
namespace {

// Same signature as NativeModuleManager::GetJSCodeCallback.
using GetAbcCodeCallback = void (*)(const char** buf, int* len);

// NAPI_<module with '.'/'/' turned into '_'>_GetABCCode, same rule as FindNativeModuleByDisk.
std::string BuildAbcCodeSymbolName(const std::string& requireName)
{
    std::string symbol = "NAPI_" + requireName + "_GetABCCode";
    for (char& c : symbol) {
        if (c == '.' || c == '/') {
            c = '_';
        }
    }
    return symbol;
}

std::string ToLowerAscii(const std::string& input)
{
    std::string out;
    out.reserve(input.size());
    for (char c : input) {
        out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return out;
}

// RTLD_NOW|RTLD_LOCAL: resolve symbols eagerly (dlsym must work) without polluting the global table.
bool ExtractAbcFromSo(const std::string& soPath, const std::string& requireName,
                      const char*& outBuf, int& outLen)
{
    void* handle = dlopen(soPath.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (handle == nullptr) {
        // dlerror() clears the error state when read: consume it once and reuse the value.
        // A second call would return nullptr, and streaming a null const char* is UB.
        const char* err = dlerror();
        LogWarn() << "dlopen failed for ABC extraction: " << soPath << ", dlerror: " <<
            (err != nullptr ? err : "unknown");
        return false;
    }
    std::string symbol = BuildAbcCodeSymbolName(requireName);
    auto getAbcCode = reinterpret_cast<GetAbcCodeCallback>(dlsym(handle, symbol.c_str()));
    if (getAbcCode == nullptr) {
        // Some engine paths lowercase the module key; try the lowercase symbol variant.
        getAbcCode = reinterpret_cast<GetAbcCodeCallback>(dlsym(handle, ToLowerAscii(symbol).c_str()));
    }
    if (getAbcCode == nullptr) {
        dlclose(handle);
        return false;
    }
    outBuf = nullptr;
    outLen = 0;
    getAbcCode(&outBuf, &outLen);
    if (outBuf == nullptr || outLen <= 0) {
        dlclose(handle);
        return false;
    }
    // Do not dlclose: buf points into the .so's static data and would dangle.
    return true;
}

// Candidate record names (engine virtual name / module name / .so stem), covering the
// shapes produced by hvigor builds and the engine's LoadNativeModule.
std::vector<std::string> BuildRecordNameCandidates(const std::string& requireName, const std::string& soPath)
{
    std::vector<std::string> candidates;
    candidates.push_back("lib" + requireName + ".z.so/" + requireName + ".js");
    candidates.push_back(requireName);
    std::string soStem = soPath.substr(soPath.find_last_of('/') + 1);
    const std::string libPrefix = "lib";
    if (soStem.compare(0, libPrefix.size(), libPrefix) == 0) {
        soStem = soStem.substr(libPrefix.size());
    }
    size_t dot = soStem.find('.');
    if (dot != std::string::npos) {
        soStem = soStem.substr(0, dot);
    }
    if (!soStem.empty()) {
        candidates.push_back(soStem);
    }
    return candidates;
}

// Tries each candidate record name; every failure must clear the pending exception.
bool ExecuteAbcAndGetExports(napi_env env, const AbcLoadParam& param,
                             const char* buf, int len, napi_value& outExports)
{
    NativeEngine* engine = reinterpret_cast<NativeEngine*>(env);
    const EcmaVM* vm = engine->GetEcmaVm();
    for (const std::string& recordName : BuildRecordNameCandidates(param.requireName, param.soPath)) {
        bool ok = panda::JSNApi::ExecuteModuleFromBuffer(
            const_cast<panda::EcmaVM*>(vm), buf, len, recordName, param.soPath);
        if (!ok) {
            ClearPendingException(env);
            LogDebug() << "ExecuteModuleFromBuffer failed, recordName: " << recordName;
            continue;
        }
        Local<panda::ObjectRef> obj =
            panda::JSNApi::GetExportObjectFromBuffer(const_cast<panda::EcmaVM*>(vm), recordName, "default");
        ClearPendingException(env);
        if (obj->IsUndefined() || obj->IsNull()) {
            LogDebug() << "GetExportObjectFromBuffer empty, recordName: " << recordName;
            continue;
        }
        outExports = JsValueFromLocalValue(obj);
        LogInfo() << "ABC executed via recordName: " << recordName << " (so: " << param.soPath << ")";
        return true;
    }
    return false;
}

void TraverseAbcExports(napi_env env, const AbcLoadParam& param,
                        napi_value exports, std::vector<ApiMeta>& exposedMeta)
{
    TraverseContext ctx;
    ctx.env = env;
    ctx.kit = param.kit;
    ctx.file = param.displayName;
    ctx.expandableNames = param.expandableNames;
    ctx.outMeta = &exposedMeta;
    TraverseExports(ctx, exports, "");
}

} // namespace

CheckErrorCode LoadAbcFileAndGetExposedMeta(
    napi_env env,
    const AbcLoadParam& param,
    std::vector<ApiMeta>& exposedMeta)
{
    char absBuf[PATH_MAX] = {0};
    if (realpath(param.abcFilePath.c_str(), absBuf) == nullptr) {
        LogWarn() << "LoadAbcFileAndGetExposedMeta: file not found: " << param.abcFilePath;
        return ERROR_FILE_OPEN_FAILED;
    }
    std::string absPath = absBuf;

    NativeEngine* engine = reinterpret_cast<NativeEngine*>(env);
    const EcmaVM* vm = engine->GetEcmaVm();

    // Entry candidates: the absolute path (unmerged single file) and the extension-less
    // base name (--merge-abc records the output file name without extension).
    std::string baseName = absPath.substr(absPath.find_last_of('/') + 1);
    size_t dotPos = baseName.find_last_of('.');
    std::string mergedEntry = (dotPos == std::string::npos) ? baseName : baseName.substr(0, dotPos);
    const std::string candidates[] = {absPath, mergedEntry};

    for (const std::string& entry : candidates) {
        if (entry.empty()) {
            continue;
        }
        bool ret = panda::JSNApi::ExecuteForAbsolutePath(vm, absPath, entry);
        if (!ret) {
            ClearPendingException(env);
            continue;
        }
        Local<panda::ObjectRef> obj =
            panda::JSNApi::GetExportsFromFile(const_cast<panda::EcmaVM*>(vm), absPath, entry);
        ClearPendingException(env);
        if (obj->IsUndefined()) {
            continue;
        }
        napi_value result = JsValueFromLocalValue(obj);
        TraverseAbcExports(env, param, result, exposedMeta);
        LogInfo() << "abc file executed: " << absPath << " (entry: " << entry << ", " <<
            exposedMeta.size() << " exports)";
        return CHECK_OK;
    }

    LogError() << "LoadAbcFileAndGetExposedMeta failed: " << absPath;
    return ERROR_JS_EXECUTE_FAILED;
}

bool SoHasEmbeddedAbc(const std::string& soPath)
{
    // Byte-level substring search only (same idea as `strings`); chunked reads keep large
    // .so files out of memory.
    std::ifstream ifs(soPath, std::ios::binary);
    if (!ifs.is_open()) {
        return false;
    }
    static const char marker[] = "GetABCCode";
    static const size_t markerLen = sizeof(marker) - 1;
    std::string tail;
    char chunk[8192];
    while (ifs.read(chunk, sizeof(chunk)) || ifs.gcount() > 0) {
        std::string data(chunk, static_cast<size_t>(ifs.gcount()));
        // carry the tail so markers spanning chunk boundaries are still found
        std::string combined = tail + data;
        if (combined.find(marker) != std::string::npos) {
            return true;
        }
        if (combined.size() > markerLen) {
            tail = combined.substr(combined.size() - markerLen + 1);
        } else {
            tail = combined;
        }
        if (!ifs) {
            break;
        }
    }
    return false;
}

std::vector<std::string> CandidateSoPaths(const std::string& soDir, const std::string& requireName)
{
    std::vector<std::string> paths;
    if (soDir.empty() || requireName.empty()) {
        return paths;
    }
    // Mirrors GetNativeModulePath: lowercase, last segment becomes the .so name, other dots become dirs.
    std::string lower = ToLowerAscii(requireName);
    size_t lastDot = lower.find_last_of('.');
    std::string stem = (lastDot == std::string::npos) ? lower : lower.substr(lastDot + 1);
    std::string dirPart = (lastDot == std::string::npos) ? "" : lower.substr(0, lastDot);
    for (char& c : dirPart) {
        if (c == '.') {
            c = '/';
        }
    }
    // platform suffix, same as the engine: .z.so on device, plain .so on a LINUX host
#if defined(OHOS_PLATFORM)
    const char* zfix = ".z";
#else
    const char* zfix = "";
#endif
    std::string prefix = dirPart.empty() ? soDir : soDir + "/" + dirPart;
    paths.push_back(prefix + "/lib" + stem + zfix + ".so");
    paths.push_back(prefix + "/lib" + stem + "_napi" + zfix + ".so");
    return paths;
}

CheckErrorCode LoadMixedSoAbcAndGetExposedMeta(
    napi_env env,
    const AbcLoadParam& param,
    std::vector<ApiMeta>& exposedMeta)
{
    const char* buf = nullptr;
    int len = 0;
    if (!ExtractAbcFromSo(param.soPath, param.requireName, buf, len)) {
        LogWarn() << "ABC extraction failed: " << param.soPath << " (module: " << param.displayName << ")";
        return ERROR_JS_EXECUTE_FAILED;
    }
    LogInfo() << "ABC extracted: " << len << " bytes from " << param.soPath;

    napi_value exports = nullptr;
    if (!ExecuteAbcAndGetExports(env, param, buf, len, exports)) {
        LogWarn() << "ABC execution failed (all record name candidates): " << param.soPath <<
            " (module: " << param.displayName << ")";
        ClearPendingException(env);
        return ERROR_JS_EXECUTE_FAILED;
    }

    TraverseAbcExports(env, param, exports, exposedMeta);
    LogInfo() << "mixed so ABC loaded: " << param.displayName << " (" << exposedMeta.size() << " exports)";
    return CHECK_OK;
}

} // namespace ohos::check
