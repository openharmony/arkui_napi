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

constexpr size_t K_ASYNC_CASE_COUNT = 60;
constexpr size_t K_ASYNC_PROMISE_CASE_COUNT = 20;
constexpr size_t K_ASYNC_CALLBACK_CASE_COUNT = 20;
constexpr size_t K_ASYNC_CALLBACK_START_INDEX = K_ASYNC_PROMISE_CASE_COUNT;
constexpr size_t K_ASYNC_PREVIEW_START_INDEX = K_ASYNC_CALLBACK_START_INDEX + K_ASYNC_CALLBACK_CASE_COUNT;
constexpr size_t K_FIRST_CASE_NUMBER = 1;
constexpr int K_CASE_NUMBER_WIDTH = 2;
constexpr size_t K_PROMISE_ARG_COUNT = 1;
constexpr size_t K_CALLBACK_ARG_COUNT = 2;
constexpr size_t K_CALLBACK_RESULT_ARG_COUNT = 1;
constexpr int32_t K_DELTA_STEP = 3;
constexpr int32_t K_MULTIPLIER_OFFSET = 2;
constexpr int32_t K_MULTIPLIER_CYCLE = 5;
constexpr uint32_t K_CALLBACK_REFERENCE_COUNT = 1;
constexpr uint32_t K_MODULE_VERSION = 1;
constexpr uint32_t K_NO_MODULE_FLAGS = 0;

struct AsyncCaseSpec {
    std::string name;
    int32_t multiplier;
    int32_t delta;
};

struct AsyncContext {
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback = nullptr;
    int32_t input = 0;
    int32_t output = 0;
    size_t caseIndex = 0;
    bool useCallback = false;
};

std::string BuildIndexedName(const char* prefix, size_t caseNumber)
{
    std::string suffix = std::to_string(caseNumber);
    if (suffix.size() < static_cast<size_t>(K_CASE_NUMBER_WIDTH)) {
        suffix.insert(0, static_cast<std::string::size_type>(K_CASE_NUMBER_WIDTH - suffix.size()), '0');
    }
    return std::string(prefix) + suffix;
}

size_t GetCaseIndex(void* data) { return static_cast<size_t>(reinterpret_cast<uintptr_t>(data)); }

AsyncCaseSpec GetAsyncCaseSpec(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    return {
        BuildIndexedName("asyncCase", caseNumber),
        static_cast<int32_t>(caseNumber % K_MULTIPLIER_CYCLE) + K_MULTIPLIER_OFFSET,
        static_cast<int32_t>(caseNumber) * K_DELTA_STEP,
    };
}

std::string BuildAsyncExportName(size_t caseIndex)
{
    const size_t caseNumber = caseIndex + K_FIRST_CASE_NUMBER;
    if (caseIndex < K_ASYNC_PROMISE_CASE_COUNT) {
        return BuildIndexedName("runAsyncPromiseCase", caseNumber);
    }
    if (caseIndex < K_ASYNC_PREVIEW_START_INDEX) {
        return BuildIndexedName("runAsyncCallbackCase", caseNumber);
    }
    return BuildIndexedName("previewAsyncCase", caseNumber);
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

bool ReadFunction(napi_env env, napi_value value)
{
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, value, &type) != napi_ok) {
        return false;
    }
    if (type != napi_function) {
        napi_throw_type_error(env, nullptr, "callback must be a function");
        return false;
    }
    return true;
}

bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int32(env, value, &napiValue) != napi_ok) {
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

bool SetNamedString(napi_env env, napi_value object, const char* name, const std::string& value)
{
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value.c_str(), value.size(), &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool CreateAsyncResultObject(napi_env env, size_t caseIndex, int32_t input, int32_t output, napi_value* result)
{
    const auto spec = GetAsyncCaseSpec(caseIndex);
    napi_value object = nullptr;
    if (napi_create_object(env, &object) != napi_ok) {
        return false;
    }
    SetNamedString(env, object, "name", spec.name);
    SetNamedInt32(env, object, "input", input);
    SetNamedInt32(env, object, "output", output);
    SetNamedBool(env, object, "passed", output == input * spec.multiplier + spec.delta);
    *result = object;
    return true;
}

void ExecuteAsync(napi_env env, void* data)
{
    (void)env;
    auto* context = static_cast<AsyncContext*>(data);
    const auto spec = GetAsyncCaseSpec(context->caseIndex);
    context->output = context->input * spec.multiplier + spec.delta;
}

void CompleteAsync(napi_env env, napi_status status, void* data)
{
    (void)status;
    auto* context = static_cast<AsyncContext*>(data);
    napi_value result = nullptr;
    if (CreateAsyncResultObject(env, context->caseIndex, context->input, context->output, &result)) {
        if (context->useCallback) {
            napi_value callback = nullptr;
            napi_value undefined = nullptr;
            napi_value callbackResult = nullptr;
            napi_get_reference_value(env, context->callback, &callback);
            napi_get_undefined(env, &undefined);
            napi_call_function(env, undefined, callback, K_CALLBACK_RESULT_ARG_COUNT, &result, &callbackResult);
        } else {
            napi_resolve_deferred(env, context->deferred, result);
        }
    }

    if (context->callback != nullptr) {
        napi_delete_reference(env, context->callback);
    }
    if (context->work != nullptr) {
        napi_delete_async_work(env, context->work);
    }
    delete context;
}

bool QueueAsync(napi_env env, AsyncContext* context)
{
    napi_value resource = nullptr;
    const auto spec = GetAsyncCaseSpec(context->caseIndex);
    if (napi_create_string_utf8(env, spec.name.c_str(), NAPI_AUTO_LENGTH, &resource) != napi_ok) {
        return false;
    }
    if (napi_create_async_work(env, nullptr, resource, ExecuteAsync, CompleteAsync, context, &context->work) !=
        napi_ok) {
        return false;
    }
    return napi_queue_async_work(env, context->work) == napi_ok;
}

napi_value StartAsyncPromiseCase(napi_env env, napi_callback_info info, size_t caseIndex)
{
    size_t argc = K_PROMISE_ARG_COUNT;
    napi_value args[K_PROMISE_ARG_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < K_PROMISE_ARG_COUNT) {
        napi_throw_type_error(env, nullptr, "input is required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "input must be a number", &input)) {
        return nullptr;
    }

    auto* context = new AsyncContext();
    context->caseIndex = caseIndex;
    context->input = input;

    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &context->deferred, &promise));
    if (!QueueAsync(env, context)) {
        delete context;
        return nullptr;
    }
    return promise;
}

napi_value StartAsyncCallbackCase(napi_env env, napi_callback_info info, size_t caseIndex)
{
    size_t argc = K_CALLBACK_ARG_COUNT;
    napi_value args[K_CALLBACK_ARG_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < K_CALLBACK_ARG_COUNT) {
        napi_throw_type_error(env, nullptr, "input and callback are required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "input must be a number", &input)) {
        return nullptr;
    }
    if (!ReadFunction(env, args[1])) {
        return nullptr;
    }

    auto* context = new AsyncContext();
    context->caseIndex = caseIndex;
    context->input = input;
    context->useCallback = true;
    NAPI_CALL(env, napi_create_reference(env, args[1], K_CALLBACK_REFERENCE_COUNT, &context->callback));
    if (!QueueAsync(env, context)) {
        napi_delete_reference(env, context->callback);
        delete context;
        return nullptr;
    }

    napi_value undefined = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undefined));
    return undefined;
}

napi_value PreviewAsyncResult(napi_env env, napi_callback_info info, size_t caseIndex)
{
    size_t argc = K_PROMISE_ARG_COUNT;
    napi_value args[K_PROMISE_ARG_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < K_PROMISE_ARG_COUNT) {
        napi_throw_type_error(env, nullptr, "input is required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "input must be a number", &input)) {
        return nullptr;
    }

    const auto spec = GetAsyncCaseSpec(caseIndex);
    const int32_t output = input * spec.multiplier + spec.delta;
    napi_value result = nullptr;
    CreateAsyncResultObject(env, caseIndex, input, output, &result);
    return result;
}

static napi_value RunAsyncPromiseCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_PROMISE_ARG_COUNT;
    napi_value args[K_PROMISE_ARG_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));

    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex >= K_ASYNC_PROMISE_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid async promise case");
        return nullptr;
    }
    return StartAsyncPromiseCase(env, info, caseIndex);
}

static napi_value RunAsyncCallbackCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_CALLBACK_ARG_COUNT;
    napi_value args[K_CALLBACK_ARG_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));

    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex < K_ASYNC_CALLBACK_START_INDEX || caseIndex >= K_ASYNC_PREVIEW_START_INDEX) {
        napi_throw_error(env, nullptr, "invalid async callback case");
        return nullptr;
    }
    return StartAsyncCallbackCase(env, info, caseIndex);
}

static napi_value PreviewAsyncCase(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    size_t argc = K_PROMISE_ARG_COUNT;
    napi_value args[K_PROMISE_ARG_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, &data));

    const size_t caseIndex = GetCaseIndex(data);
    if (caseIndex < K_ASYNC_PREVIEW_START_INDEX || caseIndex >= K_ASYNC_CASE_COUNT) {
        napi_throw_error(env, nullptr, "invalid async preview case");
        return nullptr;
    }
    return PreviewAsyncResult(env, info, caseIndex);
}

}  // namespace

static napi_value InitAsyncSuite(napi_env env, napi_value exports)
{
    std::vector<std::string> exportNames;
    std::vector<napi_property_descriptor> descriptors(K_ASYNC_CASE_COUNT);
    exportNames.reserve(K_ASYNC_CASE_COUNT);
    for (size_t caseIndex = 0; caseIndex < K_ASYNC_CASE_COUNT; caseIndex++) {
        napi_callback callback = PreviewAsyncCase;
        if (caseIndex < K_ASYNC_PROMISE_CASE_COUNT) {
            callback = RunAsyncPromiseCase;
        } else if (caseIndex < K_ASYNC_PREVIEW_START_INDEX) {
            callback = RunAsyncCallbackCase;
        }
        exportNames.emplace_back(BuildAsyncExportName(caseIndex));
        descriptors[caseIndex] = napi_property_descriptor{exportNames.back().c_str(), nullptr, callback, nullptr,
            nullptr, nullptr, napi_default, reinterpret_cast<void*>(static_cast<uintptr_t>(caseIndex))};
    }
    NAPI_CALL(env, napi_define_properties(env, exports, descriptors.size(), descriptors.data()));
    return exports;
}

static napi_module g_asyncSuiteModule = {
    .nm_version = K_MODULE_VERSION,
    .nm_flags = K_NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = InitAsyncSuite,
    .nm_modname = "async_suite",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterAsyncSuiteModule(void)
{
    napi_module_register(&g_asyncSuiteModule);
}
