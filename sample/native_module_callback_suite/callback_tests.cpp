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

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

struct CallbackCaseSpec {
    const char* name;
    int32_t left;
    int32_t right;
    int32_t calls;
};

static const CallbackCaseSpec g_callbackCaseSpecs[] = {
    {
        "callbackCase01",
        1,
        2,
        2,
    },
    {
        "callbackCase02",
        2,
        4,
        3,
    },
    {
        "callbackCase03",
        3,
        6,
        4,
    },
    {
        "callbackCase04",
        4,
        8,
        1,
    },
    {
        "callbackCase05",
        5,
        10,
        2,
    },
    {
        "callbackCase06",
        6,
        12,
        3,
    },
    {
        "callbackCase07",
        7,
        14,
        4,
    },
    {
        "callbackCase08",
        8,
        16,
        1,
    },
    {
        "callbackCase09",
        9,
        18,
        2,
    },
    {
        "callbackCase10",
        10,
        20,
        3,
    },
    {
        "callbackCase11",
        11,
        22,
        4,
    },
    {
        "callbackCase12",
        12,
        24,
        1,
    },
    {
        "callbackCase13",
        13,
        26,
        2,
    },
    {
        "callbackCase14",
        14,
        28,
        3,
    },
    {
        "callbackCase15",
        15,
        30,
        4,
    },
    {
        "callbackCase16",
        16,
        32,
        1,
    },
    {
        "callbackCase17",
        17,
        34,
        2,
    },
    {
        "callbackCase18",
        18,
        36,
        3,
    },
    {
        "callbackCase19",
        19,
        38,
        4,
    },
    {
        "callbackCase20",
        20,
        40,
        1,
    },
};

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

bool CallFunction(napi_env env, napi_value callback, size_t argc, napi_value* argv, napi_value* result)
{
    napi_value undefined = nullptr;
    if (napi_get_undefined(env, &undefined) != napi_ok) {
        return false;
    }
    return napi_call_function(env, undefined, callback, argc, argv, result) == napi_ok;
}

bool SetNamedInt32(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_value napiValue = nullptr;
    if (napi_create_int32(env, value, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool SetNamedString(napi_env env, napi_value object, const char* name, const char* value)
{
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

napi_value CreateCallbackSummary(napi_env env, const char* name, int32_t calls, napi_value outputs)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    SetNamedString(env, result, "name", name);
    SetNamedInt32(env, result, "calls", calls);
    NAPI_CALL(env, napi_set_named_property(env, result, "outputs", outputs));
    return result;
}

static napi_value TestCallbackCase01(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "callback and input are required");
        return nullptr;
    }
    if (!ReadFunction(env, args[0])) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_callbackCaseSpecs[0];
    napi_value outputs = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(spec.calls), &outputs));
    for (int32_t callIndex = 0; callIndex < spec.calls; callIndex++) {
        napi_value callbackArgs[3] = {nullptr};
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, input + spec.left + callIndex, &callbackArgs[0]));
        NAPI_CALL(env, napi_create_int32(env, spec.right + callIndex, &callbackArgs[1]));
        NAPI_CALL(env, napi_create_int32(env, callIndex, &callbackArgs[2]));
        if (!CallFunction(env, args[0], 3, callbackArgs, &callbackResult)) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, outputs, static_cast<uint32_t>(callIndex), callbackResult));
    }

    return CreateCallbackSummary(env, spec.name, spec.calls, outputs);
}

static napi_value TestCallbackCase02(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "callback and input are required");
        return nullptr;
    }
    if (!ReadFunction(env, args[0])) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_callbackCaseSpecs[1];
    napi_value outputs = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(spec.calls), &outputs));
    for (int32_t callIndex = 0; callIndex < spec.calls; callIndex++) {
        napi_value callbackArgs[3] = {nullptr};
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, input + spec.left + callIndex, &callbackArgs[0]));
        NAPI_CALL(env, napi_create_int32(env, spec.right + callIndex, &callbackArgs[1]));
        NAPI_CALL(env, napi_create_int32(env, callIndex, &callbackArgs[2]));
        if (!CallFunction(env, args[0], 3, callbackArgs, &callbackResult)) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, outputs, static_cast<uint32_t>(callIndex), callbackResult));
    }

    return CreateCallbackSummary(env, spec.name, spec.calls, outputs);
}

static napi_value TestCallbackCase03(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "callback and input are required");
        return nullptr;
    }
    if (!ReadFunction(env, args[0])) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_callbackCaseSpecs[2];
    napi_value outputs = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(spec.calls), &outputs));
    for (int32_t callIndex = 0; callIndex < spec.calls; callIndex++) {
        napi_value callbackArgs[3] = {nullptr};
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, input + spec.left + callIndex, &callbackArgs[0]));
        NAPI_CALL(env, napi_create_int32(env, spec.right + callIndex, &callbackArgs[1]));
        NAPI_CALL(env, napi_create_int32(env, callIndex, &callbackArgs[2]));
        if (!CallFunction(env, args[0], 3, callbackArgs, &callbackResult)) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, outputs, static_cast<uint32_t>(callIndex), callbackResult));
    }

    return CreateCallbackSummary(env, spec.name, spec.calls, outputs);
}

static napi_value TestCallbackCase04(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "callback and input are required");
        return nullptr;
    }
    if (!ReadFunction(env, args[0])) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_callbackCaseSpecs[3];
    napi_value outputs = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(spec.calls), &outputs));
    for (int32_t callIndex = 0; callIndex < spec.calls; callIndex++) {
        napi_value callbackArgs[3] = {nullptr};
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, input + spec.left + callIndex, &callbackArgs[0]));
        NAPI_CALL(env, napi_create_int32(env, spec.right + callIndex, &callbackArgs[1]));
        NAPI_CALL(env, napi_create_int32(env, callIndex, &callbackArgs[2]));
        if (!CallFunction(env, args[0], 3, callbackArgs, &callbackResult)) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, outputs, static_cast<uint32_t>(callIndex), callbackResult));
    }

    return CreateCallbackSummary(env, spec.name, spec.calls, outputs);
}

static napi_value TestCallbackCase05(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "callback and input are required");
        return nullptr;
    }
    if (!ReadFunction(env, args[0])) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_callbackCaseSpecs[4];
    napi_value outputs = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(spec.calls), &outputs));
    for (int32_t callIndex = 0; callIndex < spec.calls; callIndex++) {
        napi_value callbackArgs[3] = {nullptr};
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, input + spec.left + callIndex, &callbackArgs[0]));
        NAPI_CALL(env, napi_create_int32(env, spec.right + callIndex, &callbackArgs[1]));
        NAPI_CALL(env, napi_create_int32(env, callIndex, &callbackArgs[2]));
        if (!CallFunction(env, args[0], 3, callbackArgs, &callbackResult)) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, outputs, static_cast<uint32_t>(callIndex), callbackResult));
    }

    return CreateCallbackSummary(env, spec.name, spec.calls, outputs);
}

static napi_value TestCallbackCase06(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "callback and input are required");
        return nullptr;
    }
    if (!ReadFunction(env, args[0])) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_callbackCaseSpecs[5];
    napi_value outputs = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(spec.calls), &outputs));
    for (int32_t callIndex = 0; callIndex < spec.calls; callIndex++) {
        napi_value callbackArgs[3] = {nullptr};
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, input + spec.left + callIndex, &callbackArgs[0]));
        NAPI_CALL(env, napi_create_int32(env, spec.right + callIndex, &callbackArgs[1]));
        NAPI_CALL(env, napi_create_int32(env, callIndex, &callbackArgs[2]));
        if (!CallFunction(env, args[0], 3, callbackArgs, &callbackResult)) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, outputs, static_cast<uint32_t>(callIndex), callbackResult));
    }

    return CreateCallbackSummary(env, spec.name, spec.calls, outputs);
}

static napi_value TestCallbackCase07(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "callback and input are required");
        return nullptr;
    }
    if (!ReadFunction(env, args[0])) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_callbackCaseSpecs[6];
    napi_value outputs = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(spec.calls), &outputs));
    for (int32_t callIndex = 0; callIndex < spec.calls; callIndex++) {
        napi_value callbackArgs[3] = {nullptr};
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, input + spec.left + callIndex, &callbackArgs[0]));
        NAPI_CALL(env, napi_create_int32(env, spec.right + callIndex, &callbackArgs[1]));
        NAPI_CALL(env, napi_create_int32(env, callIndex, &callbackArgs[2]));
        if (!CallFunction(env, args[0], 3, callbackArgs, &callbackResult)) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, outputs, static_cast<uint32_t>(callIndex), callbackResult));
    }

    return CreateCallbackSummary(env, spec.name, spec.calls, outputs);
}

static napi_value TestCallbackCase08(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "callback and input are required");
        return nullptr;
    }
    if (!ReadFunction(env, args[0])) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_callbackCaseSpecs[7];
    napi_value outputs = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(spec.calls), &outputs));
    for (int32_t callIndex = 0; callIndex < spec.calls; callIndex++) {
        napi_value callbackArgs[3] = {nullptr};
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, input + spec.left + callIndex, &callbackArgs[0]));
        NAPI_CALL(env, napi_create_int32(env, spec.right + callIndex, &callbackArgs[1]));
        NAPI_CALL(env, napi_create_int32(env, callIndex, &callbackArgs[2]));
        if (!CallFunction(env, args[0], 3, callbackArgs, &callbackResult)) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, outputs, static_cast<uint32_t>(callIndex), callbackResult));
    }

    return CreateCallbackSummary(env, spec.name, spec.calls, outputs);
}

static napi_value TestCallbackCase09(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "callback and input are required");
        return nullptr;
    }
    if (!ReadFunction(env, args[0])) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_callbackCaseSpecs[8];
    napi_value outputs = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(spec.calls), &outputs));
    for (int32_t callIndex = 0; callIndex < spec.calls; callIndex++) {
        napi_value callbackArgs[3] = {nullptr};
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, input + spec.left + callIndex, &callbackArgs[0]));
        NAPI_CALL(env, napi_create_int32(env, spec.right + callIndex, &callbackArgs[1]));
        NAPI_CALL(env, napi_create_int32(env, callIndex, &callbackArgs[2]));
        if (!CallFunction(env, args[0], 3, callbackArgs, &callbackResult)) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, outputs, static_cast<uint32_t>(callIndex), callbackResult));
    }

    return CreateCallbackSummary(env, spec.name, spec.calls, outputs);
}

static napi_value TestCallbackCase10(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "callback and input are required");
        return nullptr;
    }
    if (!ReadFunction(env, args[0])) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_callbackCaseSpecs[9];
    napi_value outputs = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(spec.calls), &outputs));
    for (int32_t callIndex = 0; callIndex < spec.calls; callIndex++) {
        napi_value callbackArgs[3] = {nullptr};
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, input + spec.left + callIndex, &callbackArgs[0]));
        NAPI_CALL(env, napi_create_int32(env, spec.right + callIndex, &callbackArgs[1]));
        NAPI_CALL(env, napi_create_int32(env, callIndex, &callbackArgs[2]));
        if (!CallFunction(env, args[0], 3, callbackArgs, &callbackResult)) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, outputs, static_cast<uint32_t>(callIndex), callbackResult));
    }

    return CreateCallbackSummary(env, spec.name, spec.calls, outputs);
}

static napi_value TestCallbackCase11(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "callback and input are required");
        return nullptr;
    }
    if (!ReadFunction(env, args[0])) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_callbackCaseSpecs[10];
    napi_value outputs = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(spec.calls), &outputs));
    for (int32_t callIndex = 0; callIndex < spec.calls; callIndex++) {
        napi_value callbackArgs[3] = {nullptr};
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, input + spec.left + callIndex, &callbackArgs[0]));
        NAPI_CALL(env, napi_create_int32(env, spec.right + callIndex, &callbackArgs[1]));
        NAPI_CALL(env, napi_create_int32(env, callIndex, &callbackArgs[2]));
        if (!CallFunction(env, args[0], 3, callbackArgs, &callbackResult)) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, outputs, static_cast<uint32_t>(callIndex), callbackResult));
    }

    return CreateCallbackSummary(env, spec.name, spec.calls, outputs);
}

static napi_value TestCallbackCase12(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "callback and input are required");
        return nullptr;
    }
    if (!ReadFunction(env, args[0])) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_callbackCaseSpecs[11];
    napi_value outputs = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(spec.calls), &outputs));
    for (int32_t callIndex = 0; callIndex < spec.calls; callIndex++) {
        napi_value callbackArgs[3] = {nullptr};
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, input + spec.left + callIndex, &callbackArgs[0]));
        NAPI_CALL(env, napi_create_int32(env, spec.right + callIndex, &callbackArgs[1]));
        NAPI_CALL(env, napi_create_int32(env, callIndex, &callbackArgs[2]));
        if (!CallFunction(env, args[0], 3, callbackArgs, &callbackResult)) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, outputs, static_cast<uint32_t>(callIndex), callbackResult));
    }

    return CreateCallbackSummary(env, spec.name, spec.calls, outputs);
}

static napi_value TestCallbackCase13(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "callback and input are required");
        return nullptr;
    }
    if (!ReadFunction(env, args[0])) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_callbackCaseSpecs[12];
    napi_value outputs = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(spec.calls), &outputs));
    for (int32_t callIndex = 0; callIndex < spec.calls; callIndex++) {
        napi_value callbackArgs[3] = {nullptr};
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, input + spec.left + callIndex, &callbackArgs[0]));
        NAPI_CALL(env, napi_create_int32(env, spec.right + callIndex, &callbackArgs[1]));
        NAPI_CALL(env, napi_create_int32(env, callIndex, &callbackArgs[2]));
        if (!CallFunction(env, args[0], 3, callbackArgs, &callbackResult)) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, outputs, static_cast<uint32_t>(callIndex), callbackResult));
    }

    return CreateCallbackSummary(env, spec.name, spec.calls, outputs);
}

static napi_value TestCallbackCase14(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "callback and input are required");
        return nullptr;
    }
    if (!ReadFunction(env, args[0])) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_callbackCaseSpecs[13];
    napi_value outputs = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(spec.calls), &outputs));
    for (int32_t callIndex = 0; callIndex < spec.calls; callIndex++) {
        napi_value callbackArgs[3] = {nullptr};
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, input + spec.left + callIndex, &callbackArgs[0]));
        NAPI_CALL(env, napi_create_int32(env, spec.right + callIndex, &callbackArgs[1]));
        NAPI_CALL(env, napi_create_int32(env, callIndex, &callbackArgs[2]));
        if (!CallFunction(env, args[0], 3, callbackArgs, &callbackResult)) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, outputs, static_cast<uint32_t>(callIndex), callbackResult));
    }

    return CreateCallbackSummary(env, spec.name, spec.calls, outputs);
}

static napi_value TestCallbackCase15(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "callback and input are required");
        return nullptr;
    }
    if (!ReadFunction(env, args[0])) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_callbackCaseSpecs[14];
    napi_value outputs = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(spec.calls), &outputs));
    for (int32_t callIndex = 0; callIndex < spec.calls; callIndex++) {
        napi_value callbackArgs[3] = {nullptr};
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, input + spec.left + callIndex, &callbackArgs[0]));
        NAPI_CALL(env, napi_create_int32(env, spec.right + callIndex, &callbackArgs[1]));
        NAPI_CALL(env, napi_create_int32(env, callIndex, &callbackArgs[2]));
        if (!CallFunction(env, args[0], 3, callbackArgs, &callbackResult)) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, outputs, static_cast<uint32_t>(callIndex), callbackResult));
    }

    return CreateCallbackSummary(env, spec.name, spec.calls, outputs);
}

static napi_value TestCallbackCase16(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "callback and input are required");
        return nullptr;
    }
    if (!ReadFunction(env, args[0])) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_callbackCaseSpecs[15];
    napi_value outputs = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(spec.calls), &outputs));
    for (int32_t callIndex = 0; callIndex < spec.calls; callIndex++) {
        napi_value callbackArgs[3] = {nullptr};
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, input + spec.left + callIndex, &callbackArgs[0]));
        NAPI_CALL(env, napi_create_int32(env, spec.right + callIndex, &callbackArgs[1]));
        NAPI_CALL(env, napi_create_int32(env, callIndex, &callbackArgs[2]));
        if (!CallFunction(env, args[0], 3, callbackArgs, &callbackResult)) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, outputs, static_cast<uint32_t>(callIndex), callbackResult));
    }

    return CreateCallbackSummary(env, spec.name, spec.calls, outputs);
}

static napi_value TestCallbackCase17(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "callback and input are required");
        return nullptr;
    }
    if (!ReadFunction(env, args[0])) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_callbackCaseSpecs[16];
    napi_value outputs = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(spec.calls), &outputs));
    for (int32_t callIndex = 0; callIndex < spec.calls; callIndex++) {
        napi_value callbackArgs[3] = {nullptr};
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, input + spec.left + callIndex, &callbackArgs[0]));
        NAPI_CALL(env, napi_create_int32(env, spec.right + callIndex, &callbackArgs[1]));
        NAPI_CALL(env, napi_create_int32(env, callIndex, &callbackArgs[2]));
        if (!CallFunction(env, args[0], 3, callbackArgs, &callbackResult)) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, outputs, static_cast<uint32_t>(callIndex), callbackResult));
    }

    return CreateCallbackSummary(env, spec.name, spec.calls, outputs);
}

static napi_value TestCallbackCase18(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "callback and input are required");
        return nullptr;
    }
    if (!ReadFunction(env, args[0])) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_callbackCaseSpecs[17];
    napi_value outputs = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(spec.calls), &outputs));
    for (int32_t callIndex = 0; callIndex < spec.calls; callIndex++) {
        napi_value callbackArgs[3] = {nullptr};
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, input + spec.left + callIndex, &callbackArgs[0]));
        NAPI_CALL(env, napi_create_int32(env, spec.right + callIndex, &callbackArgs[1]));
        NAPI_CALL(env, napi_create_int32(env, callIndex, &callbackArgs[2]));
        if (!CallFunction(env, args[0], 3, callbackArgs, &callbackResult)) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, outputs, static_cast<uint32_t>(callIndex), callbackResult));
    }

    return CreateCallbackSummary(env, spec.name, spec.calls, outputs);
}

static napi_value TestCallbackCase19(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "callback and input are required");
        return nullptr;
    }
    if (!ReadFunction(env, args[0])) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_callbackCaseSpecs[18];
    napi_value outputs = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(spec.calls), &outputs));
    for (int32_t callIndex = 0; callIndex < spec.calls; callIndex++) {
        napi_value callbackArgs[3] = {nullptr};
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, input + spec.left + callIndex, &callbackArgs[0]));
        NAPI_CALL(env, napi_create_int32(env, spec.right + callIndex, &callbackArgs[1]));
        NAPI_CALL(env, napi_create_int32(env, callIndex, &callbackArgs[2]));
        if (!CallFunction(env, args[0], 3, callbackArgs, &callbackResult)) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, outputs, static_cast<uint32_t>(callIndex), callbackResult));
    }

    return CreateCallbackSummary(env, spec.name, spec.calls, outputs);
}

static napi_value TestCallbackCase20(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
        napi_throw_type_error(env, nullptr, "callback and input are required");
        return nullptr;
    }
    if (!ReadFunction(env, args[0])) {
        return nullptr;
    }
    if (!ReadInt32(env, args[1], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_callbackCaseSpecs[19];
    napi_value outputs = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(spec.calls), &outputs));
    for (int32_t callIndex = 0; callIndex < spec.calls; callIndex++) {
        napi_value callbackArgs[3] = {nullptr};
        napi_value callbackResult = nullptr;
        NAPI_CALL(env, napi_create_int32(env, input + spec.left + callIndex, &callbackArgs[0]));
        NAPI_CALL(env, napi_create_int32(env, spec.right + callIndex, &callbackArgs[1]));
        NAPI_CALL(env, napi_create_int32(env, callIndex, &callbackArgs[2]));
        if (!CallFunction(env, args[0], 3, callbackArgs, &callbackResult)) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_element(env, outputs, static_cast<uint32_t>(callIndex), callbackResult));
    }

    return CreateCallbackSummary(env, spec.name, spec.calls, outputs);
}

}  // namespace

static napi_value InitBranch08Callback(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[] = {
        DECLARE_NAPI_FUNCTION("testCallbackCase01", TestCallbackCase01),
        DECLARE_NAPI_FUNCTION("testCallbackCase02", TestCallbackCase02),
        DECLARE_NAPI_FUNCTION("testCallbackCase03", TestCallbackCase03),
        DECLARE_NAPI_FUNCTION("testCallbackCase04", TestCallbackCase04),
        DECLARE_NAPI_FUNCTION("testCallbackCase05", TestCallbackCase05),
        DECLARE_NAPI_FUNCTION("testCallbackCase06", TestCallbackCase06),
        DECLARE_NAPI_FUNCTION("testCallbackCase07", TestCallbackCase07),
        DECLARE_NAPI_FUNCTION("testCallbackCase08", TestCallbackCase08),
        DECLARE_NAPI_FUNCTION("testCallbackCase09", TestCallbackCase09),
        DECLARE_NAPI_FUNCTION("testCallbackCase10", TestCallbackCase10),
        DECLARE_NAPI_FUNCTION("testCallbackCase11", TestCallbackCase11),
        DECLARE_NAPI_FUNCTION("testCallbackCase12", TestCallbackCase12),
        DECLARE_NAPI_FUNCTION("testCallbackCase13", TestCallbackCase13),
        DECLARE_NAPI_FUNCTION("testCallbackCase14", TestCallbackCase14),
        DECLARE_NAPI_FUNCTION("testCallbackCase15", TestCallbackCase15),
        DECLARE_NAPI_FUNCTION("testCallbackCase16", TestCallbackCase16),
        DECLARE_NAPI_FUNCTION("testCallbackCase17", TestCallbackCase17),
        DECLARE_NAPI_FUNCTION("testCallbackCase18", TestCallbackCase18),
        DECLARE_NAPI_FUNCTION("testCallbackCase19", TestCallbackCase19),
        DECLARE_NAPI_FUNCTION("testCallbackCase20", TestCallbackCase20),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(descriptors) / sizeof(descriptors[0]), descriptors));
    return exports;
}

static napi_module g_branch08CallbackModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitBranch08Callback,
    .nm_modname = "callback_suite",
    .nm_priv = nullptr,
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterBranch08CallbackModule(void)
{
    napi_module_register(&g_branch08CallbackModule);
}
