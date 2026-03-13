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

struct AsyncCaseSpec {
    const char* name;
    int32_t multiplier;
    int32_t delta;
};

struct AsyncContext {
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback = nullptr;
    int32_t input = 0;
    int32_t output = 0;
    int32_t caseIndex = 0;
    bool useCallback = false;
};

static const AsyncCaseSpec g_asyncCaseSpecs[] = {
    {
        "asyncCase01",
        3,
        3,
    },
    {
        "asyncCase02",
        4,
        6,
    },
    {
        "asyncCase03",
        5,
        9,
    },
    {
        "asyncCase04",
        6,
        12,
    },
    {
        "asyncCase05",
        2,
        15,
    },
    {
        "asyncCase06",
        3,
        18,
    },
    {
        "asyncCase07",
        4,
        21,
    },
    {
        "asyncCase08",
        5,
        24,
    },
    {
        "asyncCase09",
        6,
        27,
    },
    {
        "asyncCase10",
        2,
        30,
    },
    {
        "asyncCase11",
        3,
        33,
    },
    {
        "asyncCase12",
        4,
        36,
    },
    {
        "asyncCase13",
        5,
        39,
    },
    {
        "asyncCase14",
        6,
        42,
    },
    {
        "asyncCase15",
        2,
        45,
    },
    {
        "asyncCase16",
        3,
        48,
    },
    {
        "asyncCase17",
        4,
        51,
    },
    {
        "asyncCase18",
        5,
        54,
    },
    {
        "asyncCase19",
        6,
        57,
    },
    {
        "asyncCase20",
        2,
        60,
    },
    {
        "asyncCase21",
        3,
        63,
    },
    {
        "asyncCase22",
        4,
        66,
    },
    {
        "asyncCase23",
        5,
        69,
    },
    {
        "asyncCase24",
        6,
        72,
    },
    {
        "asyncCase25",
        2,
        75,
    },
    {
        "asyncCase26",
        3,
        78,
    },
    {
        "asyncCase27",
        4,
        81,
    },
    {
        "asyncCase28",
        5,
        84,
    },
    {
        "asyncCase29",
        6,
        87,
    },
    {
        "asyncCase30",
        2,
        90,
    },
    {
        "asyncCase31",
        3,
        93,
    },
    {
        "asyncCase32",
        4,
        96,
    },
    {
        "asyncCase33",
        5,
        99,
    },
    {
        "asyncCase34",
        6,
        102,
    },
    {
        "asyncCase35",
        2,
        105,
    },
    {
        "asyncCase36",
        3,
        108,
    },
    {
        "asyncCase37",
        4,
        111,
    },
    {
        "asyncCase38",
        5,
        114,
    },
    {
        "asyncCase39",
        6,
        117,
    },
    {
        "asyncCase40",
        2,
        120,
    },
    {
        "asyncCase41",
        3,
        123,
    },
    {
        "asyncCase42",
        4,
        126,
    },
    {
        "asyncCase43",
        5,
        129,
    },
    {
        "asyncCase44",
        6,
        132,
    },
    {
        "asyncCase45",
        2,
        135,
    },
    {
        "asyncCase46",
        3,
        138,
    },
    {
        "asyncCase47",
        4,
        141,
    },
    {
        "asyncCase48",
        5,
        144,
    },
    {
        "asyncCase49",
        6,
        147,
    },
    {
        "asyncCase50",
        2,
        150,
    },
    {
        "asyncCase51",
        3,
        153,
    },
    {
        "asyncCase52",
        4,
        156,
    },
    {
        "asyncCase53",
        5,
        159,
    },
    {
        "asyncCase54",
        6,
        162,
    },
    {
        "asyncCase55",
        2,
        165,
    },
    {
        "asyncCase56",
        3,
        168,
    },
    {
        "asyncCase57",
        4,
        171,
    },
    {
        "asyncCase58",
        5,
        174,
    },
    {
        "asyncCase59",
        6,
        177,
    },
    {
        "asyncCase60",
        2,
        180,
    },
};

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

bool SetNamedString(napi_env env, napi_value object, const char* name, const char* value)
{
    napi_value napiValue = nullptr;
    if (napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &napiValue) != napi_ok) {
        return false;
    }
    return napi_set_named_property(env, object, name, napiValue) == napi_ok;
}

bool CreateAsyncResultObject(napi_env env, int32_t caseIndex, int32_t input, int32_t output, napi_value* result)
{
    napi_value object = nullptr;
    if (napi_create_object(env, &object) != napi_ok) {
        return false;
    }
    SetNamedString(env, object, "name", g_asyncCaseSpecs[caseIndex].name);
    SetNamedInt32(env, object, "input", input);
    SetNamedInt32(env, object, "output", output);
    SetNamedBool(env, object, "passed",
        output == input * g_asyncCaseSpecs[caseIndex].multiplier + g_asyncCaseSpecs[caseIndex].delta);
    *result = object;
    return true;
}

void ExecuteAsync(napi_env env, void* data)
{
    (void)env;
    auto* context = static_cast<AsyncContext*>(data);
    const auto& spec = g_asyncCaseSpecs[context->caseIndex];
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
            napi_call_function(env, undefined, callback, 1, &result, &callbackResult);
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
    if (napi_create_string_utf8(env, g_asyncCaseSpecs[context->caseIndex].name, NAPI_AUTO_LENGTH, &resource) !=
        napi_ok) {
        return false;
    }
    if (napi_create_async_work(env, nullptr, resource, ExecuteAsync, CompleteAsync, context, &context->work) !=
        napi_ok) {
        return false;
    }
    return napi_queue_async_work(env, context->work) == napi_ok;
}

napi_value StartAsyncPromiseCase(napi_env env, napi_callback_info info, int32_t caseIndex)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 1) {
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

napi_value StartAsyncCallbackCase(napi_env env, napi_callback_info info, int32_t caseIndex)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 2) {
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
    NAPI_CALL(env, napi_create_reference(env, args[1], 1, &context->callback));
    if (!QueueAsync(env, context)) {
        napi_delete_reference(env, context->callback);
        delete context;
        return nullptr;
    }

    napi_value undefined = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undefined));
    return undefined;
}

napi_value PreviewAsyncResult(napi_env env, napi_callback_info info, int32_t caseIndex)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    int32_t input = 0;
    if (argc < 1) {
        napi_throw_type_error(env, nullptr, "input is required");
        return nullptr;
    }
    if (!ReadInt32(env, args[0], "input must be a number", &input)) {
        return nullptr;
    }

    const auto& spec = g_asyncCaseSpecs[caseIndex];
    int32_t output = input * spec.multiplier + spec.delta;
    napi_value result = nullptr;
    CreateAsyncResultObject(env, caseIndex, input, output, &result);
    return result;
}

static napi_value RunAsyncPromiseCase01(napi_env env, napi_callback_info info)
{
    return StartAsyncPromiseCase(env, info, 0);
}

static napi_value RunAsyncPromiseCase02(napi_env env, napi_callback_info info)
{
    return StartAsyncPromiseCase(env, info, 1);
}

static napi_value RunAsyncPromiseCase03(napi_env env, napi_callback_info info)
{
    return StartAsyncPromiseCase(env, info, 2);
}

static napi_value RunAsyncPromiseCase04(napi_env env, napi_callback_info info)
{
    return StartAsyncPromiseCase(env, info, 3);
}

static napi_value RunAsyncPromiseCase05(napi_env env, napi_callback_info info)
{
    return StartAsyncPromiseCase(env, info, 4);
}

static napi_value RunAsyncPromiseCase06(napi_env env, napi_callback_info info)
{
    return StartAsyncPromiseCase(env, info, 5);
}

static napi_value RunAsyncPromiseCase07(napi_env env, napi_callback_info info)
{
    return StartAsyncPromiseCase(env, info, 6);
}

static napi_value RunAsyncPromiseCase08(napi_env env, napi_callback_info info)
{
    return StartAsyncPromiseCase(env, info, 7);
}

static napi_value RunAsyncPromiseCase09(napi_env env, napi_callback_info info)
{
    return StartAsyncPromiseCase(env, info, 8);
}

static napi_value RunAsyncPromiseCase10(napi_env env, napi_callback_info info)
{
    return StartAsyncPromiseCase(env, info, 9);
}

static napi_value RunAsyncPromiseCase11(napi_env env, napi_callback_info info)
{
    return StartAsyncPromiseCase(env, info, 10);
}

static napi_value RunAsyncPromiseCase12(napi_env env, napi_callback_info info)
{
    return StartAsyncPromiseCase(env, info, 11);
}

static napi_value RunAsyncPromiseCase13(napi_env env, napi_callback_info info)
{
    return StartAsyncPromiseCase(env, info, 12);
}

static napi_value RunAsyncPromiseCase14(napi_env env, napi_callback_info info)
{
    return StartAsyncPromiseCase(env, info, 13);
}

static napi_value RunAsyncPromiseCase15(napi_env env, napi_callback_info info)
{
    return StartAsyncPromiseCase(env, info, 14);
}

static napi_value RunAsyncPromiseCase16(napi_env env, napi_callback_info info)
{
    return StartAsyncPromiseCase(env, info, 15);
}

static napi_value RunAsyncPromiseCase17(napi_env env, napi_callback_info info)
{
    return StartAsyncPromiseCase(env, info, 16);
}

static napi_value RunAsyncPromiseCase18(napi_env env, napi_callback_info info)
{
    return StartAsyncPromiseCase(env, info, 17);
}

static napi_value RunAsyncPromiseCase19(napi_env env, napi_callback_info info)
{
    return StartAsyncPromiseCase(env, info, 18);
}

static napi_value RunAsyncPromiseCase20(napi_env env, napi_callback_info info)
{
    return StartAsyncPromiseCase(env, info, 19);
}

static napi_value RunAsyncCallbackCase21(napi_env env, napi_callback_info info)
{
    return StartAsyncCallbackCase(env, info, 20);
}

static napi_value RunAsyncCallbackCase22(napi_env env, napi_callback_info info)
{
    return StartAsyncCallbackCase(env, info, 21);
}

static napi_value RunAsyncCallbackCase23(napi_env env, napi_callback_info info)
{
    return StartAsyncCallbackCase(env, info, 22);
}

static napi_value RunAsyncCallbackCase24(napi_env env, napi_callback_info info)
{
    return StartAsyncCallbackCase(env, info, 23);
}

static napi_value RunAsyncCallbackCase25(napi_env env, napi_callback_info info)
{
    return StartAsyncCallbackCase(env, info, 24);
}

static napi_value RunAsyncCallbackCase26(napi_env env, napi_callback_info info)
{
    return StartAsyncCallbackCase(env, info, 25);
}

static napi_value RunAsyncCallbackCase27(napi_env env, napi_callback_info info)
{
    return StartAsyncCallbackCase(env, info, 26);
}

static napi_value RunAsyncCallbackCase28(napi_env env, napi_callback_info info)
{
    return StartAsyncCallbackCase(env, info, 27);
}

static napi_value RunAsyncCallbackCase29(napi_env env, napi_callback_info info)
{
    return StartAsyncCallbackCase(env, info, 28);
}

static napi_value RunAsyncCallbackCase30(napi_env env, napi_callback_info info)
{
    return StartAsyncCallbackCase(env, info, 29);
}

static napi_value RunAsyncCallbackCase31(napi_env env, napi_callback_info info)
{
    return StartAsyncCallbackCase(env, info, 30);
}

static napi_value RunAsyncCallbackCase32(napi_env env, napi_callback_info info)
{
    return StartAsyncCallbackCase(env, info, 31);
}

static napi_value RunAsyncCallbackCase33(napi_env env, napi_callback_info info)
{
    return StartAsyncCallbackCase(env, info, 32);
}

static napi_value RunAsyncCallbackCase34(napi_env env, napi_callback_info info)
{
    return StartAsyncCallbackCase(env, info, 33);
}

static napi_value RunAsyncCallbackCase35(napi_env env, napi_callback_info info)
{
    return StartAsyncCallbackCase(env, info, 34);
}

static napi_value RunAsyncCallbackCase36(napi_env env, napi_callback_info info)
{
    return StartAsyncCallbackCase(env, info, 35);
}

static napi_value RunAsyncCallbackCase37(napi_env env, napi_callback_info info)
{
    return StartAsyncCallbackCase(env, info, 36);
}

static napi_value RunAsyncCallbackCase38(napi_env env, napi_callback_info info)
{
    return StartAsyncCallbackCase(env, info, 37);
}

static napi_value RunAsyncCallbackCase39(napi_env env, napi_callback_info info)
{
    return StartAsyncCallbackCase(env, info, 38);
}

static napi_value RunAsyncCallbackCase40(napi_env env, napi_callback_info info)
{
    return StartAsyncCallbackCase(env, info, 39);
}

static napi_value PreviewAsyncCase41(napi_env env, napi_callback_info info)
{
    return PreviewAsyncResult(env, info, 40);
}

static napi_value PreviewAsyncCase42(napi_env env, napi_callback_info info)
{
    return PreviewAsyncResult(env, info, 41);
}

static napi_value PreviewAsyncCase43(napi_env env, napi_callback_info info)
{
    return PreviewAsyncResult(env, info, 42);
}

static napi_value PreviewAsyncCase44(napi_env env, napi_callback_info info)
{
    return PreviewAsyncResult(env, info, 43);
}

static napi_value PreviewAsyncCase45(napi_env env, napi_callback_info info)
{
    return PreviewAsyncResult(env, info, 44);
}

static napi_value PreviewAsyncCase46(napi_env env, napi_callback_info info)
{
    return PreviewAsyncResult(env, info, 45);
}

static napi_value PreviewAsyncCase47(napi_env env, napi_callback_info info)
{
    return PreviewAsyncResult(env, info, 46);
}

static napi_value PreviewAsyncCase48(napi_env env, napi_callback_info info)
{
    return PreviewAsyncResult(env, info, 47);
}

static napi_value PreviewAsyncCase49(napi_env env, napi_callback_info info)
{
    return PreviewAsyncResult(env, info, 48);
}

static napi_value PreviewAsyncCase50(napi_env env, napi_callback_info info)
{
    return PreviewAsyncResult(env, info, 49);
}

static napi_value PreviewAsyncCase51(napi_env env, napi_callback_info info)
{
    return PreviewAsyncResult(env, info, 50);
}

static napi_value PreviewAsyncCase52(napi_env env, napi_callback_info info)
{
    return PreviewAsyncResult(env, info, 51);
}

static napi_value PreviewAsyncCase53(napi_env env, napi_callback_info info)
{
    return PreviewAsyncResult(env, info, 52);
}

static napi_value PreviewAsyncCase54(napi_env env, napi_callback_info info)
{
    return PreviewAsyncResult(env, info, 53);
}

static napi_value PreviewAsyncCase55(napi_env env, napi_callback_info info)
{
    return PreviewAsyncResult(env, info, 54);
}

static napi_value PreviewAsyncCase56(napi_env env, napi_callback_info info)
{
    return PreviewAsyncResult(env, info, 55);
}

static napi_value PreviewAsyncCase57(napi_env env, napi_callback_info info)
{
    return PreviewAsyncResult(env, info, 56);
}

static napi_value PreviewAsyncCase58(napi_env env, napi_callback_info info)
{
    return PreviewAsyncResult(env, info, 57);
}

static napi_value PreviewAsyncCase59(napi_env env, napi_callback_info info)
{
    return PreviewAsyncResult(env, info, 58);
}

static napi_value PreviewAsyncCase60(napi_env env, napi_callback_info info)
{
    return PreviewAsyncResult(env, info, 59);
}

}  // namespace

static napi_value InitBranch09Async(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[] = {
        DECLARE_NAPI_FUNCTION("runAsyncPromiseCase01", RunAsyncPromiseCase01),
        DECLARE_NAPI_FUNCTION("runAsyncPromiseCase02", RunAsyncPromiseCase02),
        DECLARE_NAPI_FUNCTION("runAsyncPromiseCase03", RunAsyncPromiseCase03),
        DECLARE_NAPI_FUNCTION("runAsyncPromiseCase04", RunAsyncPromiseCase04),
        DECLARE_NAPI_FUNCTION("runAsyncPromiseCase05", RunAsyncPromiseCase05),
        DECLARE_NAPI_FUNCTION("runAsyncPromiseCase06", RunAsyncPromiseCase06),
        DECLARE_NAPI_FUNCTION("runAsyncPromiseCase07", RunAsyncPromiseCase07),
        DECLARE_NAPI_FUNCTION("runAsyncPromiseCase08", RunAsyncPromiseCase08),
        DECLARE_NAPI_FUNCTION("runAsyncPromiseCase09", RunAsyncPromiseCase09),
        DECLARE_NAPI_FUNCTION("runAsyncPromiseCase10", RunAsyncPromiseCase10),
        DECLARE_NAPI_FUNCTION("runAsyncPromiseCase11", RunAsyncPromiseCase11),
        DECLARE_NAPI_FUNCTION("runAsyncPromiseCase12", RunAsyncPromiseCase12),
        DECLARE_NAPI_FUNCTION("runAsyncPromiseCase13", RunAsyncPromiseCase13),
        DECLARE_NAPI_FUNCTION("runAsyncPromiseCase14", RunAsyncPromiseCase14),
        DECLARE_NAPI_FUNCTION("runAsyncPromiseCase15", RunAsyncPromiseCase15),
        DECLARE_NAPI_FUNCTION("runAsyncPromiseCase16", RunAsyncPromiseCase16),
        DECLARE_NAPI_FUNCTION("runAsyncPromiseCase17", RunAsyncPromiseCase17),
        DECLARE_NAPI_FUNCTION("runAsyncPromiseCase18", RunAsyncPromiseCase18),
        DECLARE_NAPI_FUNCTION("runAsyncPromiseCase19", RunAsyncPromiseCase19),
        DECLARE_NAPI_FUNCTION("runAsyncPromiseCase20", RunAsyncPromiseCase20),
        DECLARE_NAPI_FUNCTION("runAsyncCallbackCase21", RunAsyncCallbackCase21),
        DECLARE_NAPI_FUNCTION("runAsyncCallbackCase22", RunAsyncCallbackCase22),
        DECLARE_NAPI_FUNCTION("runAsyncCallbackCase23", RunAsyncCallbackCase23),
        DECLARE_NAPI_FUNCTION("runAsyncCallbackCase24", RunAsyncCallbackCase24),
        DECLARE_NAPI_FUNCTION("runAsyncCallbackCase25", RunAsyncCallbackCase25),
        DECLARE_NAPI_FUNCTION("runAsyncCallbackCase26", RunAsyncCallbackCase26),
        DECLARE_NAPI_FUNCTION("runAsyncCallbackCase27", RunAsyncCallbackCase27),
        DECLARE_NAPI_FUNCTION("runAsyncCallbackCase28", RunAsyncCallbackCase28),
        DECLARE_NAPI_FUNCTION("runAsyncCallbackCase29", RunAsyncCallbackCase29),
        DECLARE_NAPI_FUNCTION("runAsyncCallbackCase30", RunAsyncCallbackCase30),
        DECLARE_NAPI_FUNCTION("runAsyncCallbackCase31", RunAsyncCallbackCase31),
        DECLARE_NAPI_FUNCTION("runAsyncCallbackCase32", RunAsyncCallbackCase32),
        DECLARE_NAPI_FUNCTION("runAsyncCallbackCase33", RunAsyncCallbackCase33),
        DECLARE_NAPI_FUNCTION("runAsyncCallbackCase34", RunAsyncCallbackCase34),
        DECLARE_NAPI_FUNCTION("runAsyncCallbackCase35", RunAsyncCallbackCase35),
        DECLARE_NAPI_FUNCTION("runAsyncCallbackCase36", RunAsyncCallbackCase36),
        DECLARE_NAPI_FUNCTION("runAsyncCallbackCase37", RunAsyncCallbackCase37),
        DECLARE_NAPI_FUNCTION("runAsyncCallbackCase38", RunAsyncCallbackCase38),
        DECLARE_NAPI_FUNCTION("runAsyncCallbackCase39", RunAsyncCallbackCase39),
        DECLARE_NAPI_FUNCTION("runAsyncCallbackCase40", RunAsyncCallbackCase40),
        DECLARE_NAPI_FUNCTION("previewAsyncCase41", PreviewAsyncCase41),
        DECLARE_NAPI_FUNCTION("previewAsyncCase42", PreviewAsyncCase42),
        DECLARE_NAPI_FUNCTION("previewAsyncCase43", PreviewAsyncCase43),
        DECLARE_NAPI_FUNCTION("previewAsyncCase44", PreviewAsyncCase44),
        DECLARE_NAPI_FUNCTION("previewAsyncCase45", PreviewAsyncCase45),
        DECLARE_NAPI_FUNCTION("previewAsyncCase46", PreviewAsyncCase46),
        DECLARE_NAPI_FUNCTION("previewAsyncCase47", PreviewAsyncCase47),
        DECLARE_NAPI_FUNCTION("previewAsyncCase48", PreviewAsyncCase48),
        DECLARE_NAPI_FUNCTION("previewAsyncCase49", PreviewAsyncCase49),
        DECLARE_NAPI_FUNCTION("previewAsyncCase50", PreviewAsyncCase50),
        DECLARE_NAPI_FUNCTION("previewAsyncCase51", PreviewAsyncCase51),
        DECLARE_NAPI_FUNCTION("previewAsyncCase52", PreviewAsyncCase52),
        DECLARE_NAPI_FUNCTION("previewAsyncCase53", PreviewAsyncCase53),
        DECLARE_NAPI_FUNCTION("previewAsyncCase54", PreviewAsyncCase54),
        DECLARE_NAPI_FUNCTION("previewAsyncCase55", PreviewAsyncCase55),
        DECLARE_NAPI_FUNCTION("previewAsyncCase56", PreviewAsyncCase56),
        DECLARE_NAPI_FUNCTION("previewAsyncCase57", PreviewAsyncCase57),
        DECLARE_NAPI_FUNCTION("previewAsyncCase58", PreviewAsyncCase58),
        DECLARE_NAPI_FUNCTION("previewAsyncCase59", PreviewAsyncCase59),
        DECLARE_NAPI_FUNCTION("previewAsyncCase60", PreviewAsyncCase60),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(descriptors) / sizeof(descriptors[0]), descriptors));
    return exports;
}

static napi_module g_branch09AsyncModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitBranch09Async,
    .nm_modname = "async_suite",
    .nm_priv = nullptr,
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterBranch09AsyncModule(void)
{
    napi_module_register(&g_branch09AsyncModule);
}
