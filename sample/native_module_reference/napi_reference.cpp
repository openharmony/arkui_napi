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

#include <cstddef>
#include <cstdint>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

static constexpr uint32_t DEFAULT_REF_COUNT = 1;

struct RefData {
    int32_t value = 0;
    napi_ref ref = nullptr;
};

static void RefDataFinalizer(napi_env env, void* data, void* hint)
{
    RefData* refData = static_cast<RefData*>(data);
    if (refData != nullptr) {
        if (refData->ref != nullptr) {
            napi_delete_reference(env, refData->ref);
        }
        delete refData;
    }
}

static napi_value CreateReference(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= 1, "Requires at least 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_number, "Value must be a number");

    int32_t value = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &value));

    uint32_t refCount = DEFAULT_REF_COUNT;
    if (argc > 1) {
        napi_valuetype countType = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[1], &countType));
        NAPI_ASSERT(env, countType == napi_number, "Count must be a number");

        double countValue = 0;
        NAPI_CALL(env, napi_get_value_double(env, argv[1], &countValue));
        refCount = static_cast<uint32_t>(countValue);
        NAPI_ASSERT(env, refCount > 0, "Count must be positive");
    }

    napi_value jsValue = nullptr;
    NAPI_CALL(env, napi_create_int32(env, value, &jsValue));

    napi_ref ref = nullptr;
    NAPI_CALL(env, napi_create_reference(env, jsValue, refCount, &ref));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value refValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, reinterpret_cast<int64_t>(ref), &refValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "ref", refValue));

    napi_value countValue = nullptr;
    NAPI_CALL(env, napi_create_uint32(env, refCount, &countValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "count", countValue));

    return result;
}

static napi_value GetReferenceValue(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= 1, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_number, "Ref must be a number");

    int64_t refAddr = 0;
    NAPI_CALL(env, napi_get_value_int64(env, argv[0], &refAddr));
    napi_ref ref = reinterpret_cast<napi_ref>(refAddr);

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, ref, &result));

    return result;
}

static napi_value CreateWeakReference(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= 1, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_number, "Value must be a number");

    int32_t value = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &value));

    napi_value jsValue = nullptr;
    NAPI_CALL(env, napi_create_int32(env, value, &jsValue));

    napi_ref weakRef = nullptr;
    NAPI_CALL(env, napi_create_reference(env, jsValue, 0, &weakRef));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int64(env, reinterpret_cast<int64_t>(weakRef), &result));

    return result;
}

static napi_value ReferenceRefCount(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= 1, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_number, "Ref must be a number");

    int64_t refAddr = 0;
    NAPI_CALL(env, napi_get_value_int64(env, argv[0], &refAddr));
    napi_ref ref = reinterpret_cast<napi_ref>(refAddr);

    uint32_t refCount = 0;
    NAPI_CALL(env, napi_reference_ref(env, ref, &refCount));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_uint32(env, refCount, &result));

    return result;
}

static napi_value DeleteReference(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= 1, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_number, "Ref must be a number");

    int64_t refAddr = 0;
    NAPI_CALL(env, napi_get_value_int64(env, argv[0], &refAddr));
    napi_ref ref = reinterpret_cast<napi_ref>(refAddr);

    NAPI_CALL(env, napi_delete_reference(env, ref));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &result));

    return result;
}

static napi_value CreateRefWithFinalizer(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= 1, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_number, "Value must be a number");

    int32_t value = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &value));

    RefData* refData = new RefData();
    refData->value = value;

    napi_value jsValue = nullptr;
    NAPI_CALL(env, napi_create_int32(env, value, &jsValue));

    NAPI_CALL(env, napi_create_reference(env, jsValue, 1, &refData->ref));

    napi_value external = nullptr;
    NAPI_CALL(env, napi_create_external(env, refData, RefDataFinalizer,
                                       nullptr, &external));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value externalValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, reinterpret_cast<int64_t>(external),
                                      &externalValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "external", externalValue));

    napi_value refValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, reinterpret_cast<int64_t>(refData->ref),
                                      &refValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "ref", refValue));

    return result;
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("createReference", CreateReference),
        DECLARE_NAPI_FUNCTION("getReferenceValue", GetReferenceValue),
        DECLARE_NAPI_FUNCTION("createWeakReference", CreateWeakReference),
        DECLARE_NAPI_FUNCTION("referenceRefCount", ReferenceRefCount),
        DECLARE_NAPI_FUNCTION("deleteReference", DeleteReference),
        DECLARE_NAPI_FUNCTION("createRefWithFinalizer", CreateRefWithFinalizer),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
                                      sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module referenceModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "reference",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void ReferenceRegisterModule(void)
{
    napi_module_register(&referenceModule);
}
