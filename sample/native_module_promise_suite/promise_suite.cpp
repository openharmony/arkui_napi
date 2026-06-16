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

#include <cstdint>
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

constexpr size_t K_PROMISE_CASE_COUNT = 20;
constexpr size_t K_PROMISE_RESOLVE_COUNT = 10;
constexpr size_t K_PROMISE_PREVIEW_START_INDEX = K_PROMISE_RESOLVE_COUNT;
constexpr size_t K_PROMISE_ARG_COUNT = 1;
constexpr size_t K_FIRST_CASE_NUMBER = 1;
constexpr int K_CASE_NUMBER_WIDTH = 2;
constexpr int32_t K_MULTIPLIER_BASE = 2;
constexpr int32_t K_MULTIPLIER_CYCLE = 5;
constexpr int32_t K_DIVISOR_OFFSET = 2;
constexpr int32_t K_DIVISOR_CYCLE = 7;
constexpr uint32_t K_MODULE_VERSION = 1;
constexpr uint32_t K_NO_MODULE_FLAGS = 0;

struct PromiseCaseSpec {
    std::string name;
    int32_t multiplier;
    int32_t divisor;
};

struct PromiseContext {
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    int32_t input = 0;
    int32_t output = 0;
    size_t caseIndex = 0;
    bool shouldReject = false;
};

std::string BuildIndexedName(const char* prefix, size_t caseNumber)
{
    std::string suffix = std::to_string(caseNumber);
    if (suffix.size() < static_cast<size_t>(K_CASE_NUMBER_WIDTH)) {
        suffix.insert(
            0, static_cast<std::string::size_type>(K_CASE_NUMBER_WIDTH - suffix.size()), '0');
    }
    return std::string(prefix) + suffix;
}

size_t GetCaseIndex(void* data)
{
    return static_cast<size_t>(reinterpret_cast<uintptr_t>(data));
}

PromiseCaseSpec GetPromiseCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    return {
        BuildIndexedName("promiseCase", caseNumber),
        static_cast<int32_t>(caseNumber % K_MULTIPLIER_CYCLE) + K_MULTIPLIER_BASE,
        static_cast<int32_t>(caseNumber % K_DIVISOR_CYCLE) + K_DIVISOR_OFFSET,
    };
}

std::string BuildPromiseExportName(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    if (caseIndex < K_PROMISE_RESOLVE_COUNT) {
        return BuildIndexedName("runPromiseResolveCase", caseNumber);
    }
    return BuildIndexedName("previewPromiseCase", caseNumber);
}

bool ReadInt32(napi_env env, napi_value value, const char* message, int32_t* result)
{
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, value, &type) != napi_ok) {
        return false;
    }
    if (type != napi_number) {
        napi_throw_type_error(env, nullptr, message);
        return false;
    }
    return napi_get_value_int32(env, value, result) == napi_ok;
}

bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int32(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedString(napi_env env, napi_value object, const char* name, const std::string& value)
{
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value.c_str(), value.size(), &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedBool(napi_env env, napi_value object, const char* name, bool value)
{
    napi_value napiValue = nullptr;
    if (napi_get_boolean(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

napi_value CreatePromiseResultObject(
    napi_env env, const PromiseCaseSpec& spec, int32_t input, int32_t output)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", spec.name);
    SetNamedInt32(env, result, "input", input);
    SetNamedInt32(env, result, "output", output);
    SetNamedBool(env, result, "passed", output == input * spec.multiplier);
    return result;
}

void ExecutePromise(napi_env env, void* data)
{
    (void)env;
    auto* context = static_cast<PromiseContext*>(data);
    const auto spec = GetPromiseCaseSpec(context->caseIndex);
    const int32_t remainder = context->input % spec.divisor;
    context->shouldReject = (remainder == 0);
    if (!context->shouldReject) {
        context->output = context->input * spec.multiplier;
    }
}

void CompletePromise(napi_env env, napi_status status, void* data)
{
    (void)status;
    auto* context = static_cast<PromiseContext*>(data);
    if (context->shouldReject) {
        const auto spec = GetPromiseCaseSpec(context->caseIndex);
        std::string errorMsg = spec.name + " rejected: input divisible by divisor";
        napi_value errorCode = nullptr;
        napi_value errorMessage = nullptr;
        napi_create_string_utf8(env, spec.name.c_str(), NAPI_AUTO_LENGTH, &errorCode);
        napi_create_string_utf8(env, errorMsg.c_str(), errorMsg.size(), &errorMessage);
        napi_value error = nullptr;
        napi_create_error(env, errorCode, errorMessage, &error);
        napi_reject_deferred(env, context->deferred, error);
    } else {
        napi_value result = nullptr;
        const auto spec = GetPromiseCaseSpec(context->caseIndex);
        if (CreatePromiseResultObject(env, spec, context->input, context->output, &result)) {
            napi_resolve_deferred(env, context->deferred, result);
        } else {
            napi_value error = nullptr;
            napi_create_string_utf8(env, "failed to create result", NAPI_AUTO_LENGTH, &error);
            napi_reject_deferred(env, context->deferred, error);
        }
    }
    if (context->work != nullptr) {
        napi_delete_async_work(env, context->work);
    }
    delete context;
}

bool QueuePromiseAsync(napi_env env, PromiseContext* context)
{
    napi_value resource = nullptr;
    const auto spec = GetPromiseCaseSpec(context->caseIndex);
    napi_status createStrStatus = napi_create_string_utf8(
        env, spec.name.c_str(), NAPI_AUTO_LENGTH, &resource);
    if (createStrStatus != napi_ok) {
        return false;
    }
    napi_status createWorkStatus = napi_create_async_work(
        env, nullptr, resource, ExecutePromise, CompletePromise, context, &context->work);
    if (createWorkStatus != napi_ok) {
        return false;
    }
    return napi_queue_async_work(env, context->work) == napi_ok;
}

napi_value StartPromiseResolveCase(napi_env env, napi_callback_info info, size_t caseIndex)
{
    size_t argc = K_PROMISE_ARG_COUNT;
    napi_value args[K_PROMISE_ARG_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < K_PROMISE_ARG_COUNT) {
        napi_throw_type_error(env, nullptr, "input number is required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "input must be a number", &input)) {
        return nullptr;
    }

    auto* context = new PromiseContext();
    context->caseIndex = caseIndex;
    context->input = input;

    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &context->deferred, &promise));
    if (!QueuePromiseAsync(env, context)) {
        delete context;
        return nullptr;
    }
    return promise;
}

napi_value StartPromisePreviewCase(napi_env env, napi_callback_info info, size_t caseIndex)
{
    size_t argc = K_PROMISE_ARG_COUNT;
    napi_value args[K_PROMISE_ARG_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < K_PROMISE_ARG_COUNT) {
        napi_throw_type_error(env, nullptr, "input number is required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "input must be a number", &input)) {
        return nullptr;
    }

    const auto spec = GetPromiseCaseSpec(caseIndex);
    const int32_t output = input * spec.multiplier;
    return CreatePromiseResultObject(env, spec, input, output);
}

static napi_value RunPromiseResolveCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_PROMISE_ARG_COUNT;
    napi_value args[K_PROMISE_ARG_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));

    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_PROMISE_RESOLVE_COUNT) {
        napi_throw_error(env, nullptr, "invalid promise resolve case");
        return nullptr;
    }
    return StartPromiseResolveCase(env, info, caseIndex);
}

static napi_value PreviewPromiseCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_PROMISE_ARG_COUNT;
    napi_value args[K_PROMISE_ARG_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));

    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex < K_PROMISE_PREVIEW_START_INDEX || caseIndex >= K_PROMISE_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid promise preview case");
        return nullptr;
    }
    return StartPromisePreviewCase(env, info, caseIndex);
}

}  // namespace

static napi_value InitPromiseSuite(napi_env env, napi_value exports)
{
    std::vector<std::string> exportNames;
    std::vector<napi_property_descriptor> descriptors(K_PROMISE_CASE_COUNT);
    exportNames.reserve(K_PROMISE_CASE_COUNT);
    for (size_t caseIndex = 0; caseIndex < K_PROMISE_CASE_COUNT; caseIndex++) {
        napi_callback callback = PreviewPromiseCase;
        if (caseIndex < K_PROMISE_RESOLVE_COUNT) {
            callback = RunPromiseResolveCase;
        }
        exportNames.emplace_back(BuildPromiseExportName(caseIndex));
        descriptors[caseIndex] = napi_property_descriptor{
            exportNames.back().c_str(),
            nullptr,
            callback,
            nullptr,
            nullptr,
            nullptr,
            napi_default,
            reinterpret_cast<void*>(static_cast<uintptr_t>(caseIndex)),
        };
    }
    NAPI_CALL(env, napi_define_properties(env, exports, descriptors.size(), descriptors.data()));
    return exports;
}

static napi_module g_promiseSuiteModule = {
    .nm_version = K_MODULE_VERSION,
    .nm_flags = K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitPromiseSuite,
    .nm_modname = "promise_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterPromiseSuiteModule(void)
{
    napi_module_register(&g_promiseSuiteModule);
}
