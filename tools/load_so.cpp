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

#include "load_so.h"

#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "check_helper.h"
#include "jsnapi.h"
#include "native_engine.h"
#include "native_utils.h"

namespace ohos::check {
namespace {

// requireNapi global names injected by ArkNativeEngine; PREVIEW builds register requireNapiPreview.
constexpr const char* REQUIRE_NAPI_NAMES[] = { "requireNapi", "requireNapiPreview" };

bool GetRequireNapiFunction(napi_env env, napi_value global, napi_value& outFunc)
{
    for (const char* name : REQUIRE_NAPI_NAMES) {
        napi_value func = nullptr;
        if (napi_get_named_property(env, global, name, &func) != napi_ok) {
            continue;
        }
        napi_valuetype type;
        if (napi_typeof(env, func, &type) == napi_ok && type == napi_function) {
            outFunc = func;
            return true;
        }
    }
    return false;
}

napi_value GetModuleExports(napi_env env, const std::string& displayName,
                            const std::string& requireName)
{
    NativeEngine *engine = reinterpret_cast<NativeEngine *>(env);
    const EcmaVM *vm = engine->GetEcmaVm();

    napi_status status;
    napi_value global = nullptr;
    status = napi_get_global(env, &global);
    if (status != napi_ok) {
        LogError() << "napi_get_global failed: " << displayName;
        ClearPendingException(env);
        return nullptr;
    }

    napi_value requireNapi = nullptr;
    if (!GetRequireNapiFunction(env, global, requireNapi)) {
        // napi_get_named_property carries NAPI_PREAMBLE: one pending exception short-circuits
        // every later lookup, so it must be cleared or all following modules fail.
        LogError() << "requireNapi lookup failed (pending exception poisoned env, or requireNapi missing): " <<
            displayName;
        ClearPendingException(env);
        return nullptr;
    }

    napi_value soName = nullptr;
    status = napi_create_string_utf8(env, requireName.c_str(), requireName.size(), &soName);
    if (status != napi_ok) {
        LogError() << "napi_create_string_utf8 failed: " << displayName;
        ClearPendingException(env);
        return nullptr;
    }

    napi_value exports = nullptr;
    static const uint32_t argc = 1;
    napi_value args[argc] = { soName };
    status = napi_call_function(env, global, requireNapi, argc, args, &exports);
    if (status != napi_ok) {
        LogError() << "napi_call_function requireNapi failed: " << displayName;
        ClearPendingException(env);
        return nullptr;
    }

    // Both failure shapes must be intercepted, else failure info gets traversed as the export
    // surface (a silent false pass): undefined and the NativeModuleFailureInfo object.
    auto exportsVal = LocalValueFromJsValue(exports);
    if (exportsVal->IsUndefined()) {
        LogWarn() << "requireNapi returned undefined (no native .so on device, pure-type d.ts module,"
            " or module init failed): " << displayName;
        LogWarn() << "hint: module search path is /system/lib[64]/module on device,"
            " ./module/lib<module>.so relative to cwd on host";
        ClearPendingException(env);
        return nullptr;
    }
    if (exportsVal->IsNativeModuleFailureInfoObject(vm)) {
        LogError() << "requireNapi returned NativeModuleFailureInfo, module load failed: " << displayName;
        ClearPendingException(env);
        return nullptr;
    }
    return exports;
}

} // namespace

std::string ModuleNameForRequireNapi(const std::string& file)
{
    if (file.empty() || file[0] != '@') {
        return file;
    }
    std::string name = file.substr(1);
    // @hms./@arkts. keep the full name: the loader turns every dot into a directory
    // (module/hms/core/ar/libarengine.z.so), so stripping the first segment would fail.
    if (StartsWith(file, "@hms.") || StartsWith(file, "@arkts.")) {
        return name;
    }
    // Keep in sync with the GetOhmurl NATIVE_MODULE special cases (ark_native_engine.cpp).
    static const std::set<std::string> nativeModules = {
        "system.app", "ohos.app", "system.router", "ohos.curves",
        "ohos.matrix4", "system.matrix4", "system.curves",
    };
    if (nativeModules.count(name) > 0) {
        return name;
    }
    size_t pos = file.find('.');
    if (pos == std::string::npos || pos + 1 >= file.size()) {
        return name;
    }
    return file.substr(pos + 1);
}

CheckErrorCode LoadSoAndGetExposedMeta(napi_env env,
                                       const SoLoadParam& param,
                                       std::vector<ApiMeta>& exposedMeta,
                                       const std::set<std::string>* expandableNames)
{
    if (param.requireName.empty()) {
        return ERROR_PARAM_INVALID;
    }

    napi_value exports = GetModuleExports(env, param.displayName, param.requireName);
    if (exports == nullptr) {
        return ERROR_JS_EXECUTE_FAILED;
    }

    TraverseContext ctx;
    ctx.env = env;
    ctx.kit = param.kit;
    ctx.file = param.displayName;
    ctx.expandableNames = expandableNames;
    ctx.outMeta = &exposedMeta;
    TraverseExports(ctx, exports, "");

    return CHECK_OK;
}

}
