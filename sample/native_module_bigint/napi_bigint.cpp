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

static constexpr int REQUIRED_ARGS_ONE = 1;
static constexpr int REQUIRED_ARGS_TWO = 2;
static constexpr uint32_t MAX_WORD_COUNT = 1048576;

static napi_value CreateBigIntFromInt64(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_number, "Value must be a number");

    int64_t value = 0;
    NAPI_CALL(env, napi_get_value_int64(env, argv[0], &value));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_bigint_int64(env, value, &result));

    return result;
}

static napi_value CreateBigIntFromUint64(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_number, "Value must be a number");

    double doubleValue = 0;
    NAPI_CALL(env, napi_get_value_double(env, argv[0], &doubleValue));
    uint64_t value = static_cast<uint64_t>(doubleValue);

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_bigint_uint64(env, value, &result));

    return result;
}

static napi_value CreateBigIntFromWords(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments");

    napi_valuetype signType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &signType));
    NAPI_ASSERT(env, signType == napi_boolean, "Sign must be a boolean");

    napi_valuetype wordsType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[1], &wordsType));
    NAPI_ASSERT(env, wordsType == napi_object, "Words must be an array");

    bool isUnsigned = false;
    NAPI_CALL(env, napi_get_value_bool(env, argv[0], &isUnsigned));

    uint32_t wordCount = 0;
    NAPI_CALL(env, napi_get_array_length(env, argv[1], &wordCount));
    NAPI_ASSERT(env, wordCount > 0, "Word count must be positive");
    NAPI_ASSERT(env, wordCount <= MAX_WORD_COUNT, "Word count exceeds maximum");

    uint64_t* words = new uint64_t[wordCount];
    for (uint32_t i = 0; i < wordCount; i++) {
        napi_value element = nullptr;
        NAPI_CALL(env, napi_get_element(env, argv[1], i, &element));

        napi_valuetype elemType = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, element, &elemType));
        NAPI_ASSERT(env, elemType == napi_number, "Word must be a number");

        double wordValue = 0;
        NAPI_CALL(env, napi_get_value_double(env, element, &wordValue));
        words[i] = static_cast<uint64_t>(wordValue);
    }

    napi_value result = nullptr;
    int signBit = isUnsigned ? 0 : 1;
    NAPI_CALL(env, napi_create_bigint_words(env, signBit, wordCount, words, &result));

    delete[] words;
    return result;
}

static napi_value GetBigIntInt64(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_bigint, "Argument must be a BigInt");

    int64_t value = 0;
    bool lossless = false;
    NAPI_CALL(env, napi_get_value_bigint_int64(env, argv[0], &value, &lossless));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value valueValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, value, &valueValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "value", valueValue));

    napi_value losslessValue = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, lossless, &losslessValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "lossless", losslessValue));

    return result;
}

static napi_value GetBigIntUint64(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_bigint, "Argument must be a BigInt");

    uint64_t value = 0;
    bool lossless = false;
    NAPI_CALL(env, napi_get_value_bigint_uint64(env, argv[0], &value, &lossless));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value valueValue = nullptr;
    NAPI_CALL(env, napi_create_double(env, static_cast<double>(value), &valueValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "value", valueValue));

    napi_value losslessValue = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, lossless, &losslessValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "lossless", losslessValue));

    return result;
}

static napi_value GetBigIntWords(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_bigint, "Argument must be a BigInt");

    int signBit = 0;
    size_t wordCount = 0;
    NAPI_CALL(env, napi_get_value_bigint_words(env, argv[0], &signBit, &wordCount, nullptr));

    uint64_t* words = new uint64_t[wordCount];
    NAPI_CALL(env, napi_get_value_bigint_words(env, argv[0], &signBit, &wordCount, words));

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value signValue = nullptr;
    NAPI_CALL(env, napi_create_int32(env, signBit, &signValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "sign", signValue));

    napi_value wordsArray = nullptr;
    NAPI_CALL(env, napi_create_array(env, &wordsArray));

    for (size_t i = 0; i < wordCount; i++) {
        napi_value wordValue = nullptr;
        NAPI_CALL(env, napi_create_double(env, static_cast<double>(words[i]), &wordValue));
        NAPI_CALL(env, napi_set_element(env, wordsArray, i, wordValue));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "words", wordsArray));

    delete[] words;
    return result;
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("createBigIntFromInt64", CreateBigIntFromInt64),
        DECLARE_NAPI_FUNCTION("createBigIntFromUint64", CreateBigIntFromUint64),
        DECLARE_NAPI_FUNCTION("createBigIntFromWords", CreateBigIntFromWords),
        DECLARE_NAPI_FUNCTION("getBigIntInt64", GetBigIntInt64),
        DECLARE_NAPI_FUNCTION("getBigIntUint64", GetBigIntUint64),
        DECLARE_NAPI_FUNCTION("getBigIntWords", GetBigIntWords),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
                                      sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module bigintModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "bigint",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void BigIntRegisterModule(void)
{
    napi_module_register(&bigintModule);
}
