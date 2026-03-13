/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "crypto_utils.h"

#include "napi/native_api.h"
#include "napi/native_node_api.h"

static bool GetStringArg(napi_env env, napi_value arg, std::string& result)
{
    size_t strLen = 0;
    napi_status status = napi_get_value_string_utf8(env, arg, nullptr, 0, &strLen);
    if (status != napi_ok) {
        return false;
    }

    result.resize(strLen);
    status = napi_get_value_string_utf8(env, arg, &result[0], strLen + 1, &strLen);
    return status == napi_ok;
}

static napi_value CreateStringResult(napi_env env, const std::string& result)
{
    napi_value output;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

static napi_value Base64Encode(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");

    std::string input;
    NAPI_ASSERT(env, GetStringArg(env, args[0], input), "Failed to get string");

    return CreateStringResult(env, CryptoUtils::Base64Encode(input));
}

static napi_value Base64Decode(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    std::string input;
    NAPI_ASSERT(env, GetStringArg(env, args[0], input), "Failed to get string");

    return CreateStringResult(env, CryptoUtils::Base64Decode(input));
}

static napi_value HexEncode(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    std::string input;
    NAPI_ASSERT(env, GetStringArg(env, args[0], input), "Failed to get string");

    return CreateStringResult(env, CryptoUtils::HexEncode(input));
}

static napi_value HexDecode(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    std::string input;
    NAPI_ASSERT(env, GetStringArg(env, args[0], input), "Failed to get string");

    return CreateStringResult(env, CryptoUtils::HexDecode(input));
}

static napi_value XorEncrypt(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 2, "Wrong number of arguments");

    std::string input;
    std::string key;
    NAPI_ASSERT(env, GetStringArg(env, args[0], input), "Failed to get input string");
    NAPI_ASSERT(env, GetStringArg(env, args[1], key), "Failed to get key string");

    return CreateStringResult(env, CryptoUtils::XorEncrypt(input, key));
}

static napi_value XorDecrypt(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 2, "Wrong number of arguments");

    std::string input;
    std::string key;
    NAPI_ASSERT(env, GetStringArg(env, args[0], input), "Failed to get input string");
    NAPI_ASSERT(env, GetStringArg(env, args[1], key), "Failed to get key string");

    return CreateStringResult(env, CryptoUtils::XorDecrypt(input, key));
}

static napi_value SimpleHash(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    std::string input;
    NAPI_ASSERT(env, GetStringArg(env, args[0], input), "Failed to get string");

    return CreateStringResult(env, CryptoUtils::SimpleHash(input));
}

static napi_value Rot13(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    std::string input;
    NAPI_ASSERT(env, GetStringArg(env, args[0], input), "Failed to get string");

    return CreateStringResult(env, CryptoUtils::Rot13(input));
}

static napi_value ReverseString(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    std::string input;
    NAPI_ASSERT(env, GetStringArg(env, args[0], input), "Failed to get string");

    return CreateStringResult(env, CryptoUtils::ReverseString(input));
}

static napi_value Crc32(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    std::string input;
    NAPI_ASSERT(env, GetStringArg(env, args[0], input), "Failed to get string");

    napi_value output;
    NAPI_CALL(env, napi_create_uint32(env, CryptoUtils::Crc32(input), &output));
    return output;
}

static napi_value CaesarEncrypt(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 2, "Wrong number of arguments");

    std::string input;
    NAPI_ASSERT(env, GetStringArg(env, args[0], input), "Failed to get string");

    int32_t shift = 0;
    NAPI_CALL(env, napi_get_value_int32(env, args[1], &shift));

    return CreateStringResult(env, CryptoUtils::CaesarEncrypt(input, shift));
}

static napi_value CaesarDecrypt(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 2, "Wrong number of arguments");

    std::string input;
    NAPI_ASSERT(env, GetStringArg(env, args[0], input), "Failed to get string");

    int32_t shift = 0;
    NAPI_CALL(env, napi_get_value_int32(env, args[1], &shift));

    return CreateStringResult(env, CryptoUtils::CaesarDecrypt(input, shift));
}

static napi_value VigenereEncrypt(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 2, "Wrong number of arguments");

    std::string input;
    std::string key;
    NAPI_ASSERT(env, GetStringArg(env, args[0], input), "Failed to get input string");
    NAPI_ASSERT(env, GetStringArg(env, args[1], key), "Failed to get key string");

    return CreateStringResult(env, CryptoUtils::VigenereEncrypt(input, key));
}

static napi_value VigenereDecrypt(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 2, "Wrong number of arguments");

    std::string input;
    std::string key;
    NAPI_ASSERT(env, GetStringArg(env, args[0], input), "Failed to get input string");
    NAPI_ASSERT(env, GetStringArg(env, args[1], key), "Failed to get key string");

    return CreateStringResult(env, CryptoUtils::VigenereDecrypt(input, key));
}

static napi_value RandomString(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    int32_t length = 0;
    NAPI_CALL(env, napi_get_value_int32(env, args[0], &length));
    NAPI_ASSERT(env, length >= 0, "Length must be non-negative");

    return CreateStringResult(env, CryptoUtils::RandomString(static_cast<size_t>(length)));
}

static napi_value PasswordHash(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 2, "Wrong number of arguments");

    std::string password;
    std::string salt;
    NAPI_ASSERT(env, GetStringArg(env, args[0], password), "Failed to get password string");
    NAPI_ASSERT(env, GetStringArg(env, args[1], salt), "Failed to get salt string");

    return CreateStringResult(env, CryptoUtils::PasswordHash(password, salt));
}

static napi_value PasswordVerify(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 3, "Wrong number of arguments");

    std::string password;
    std::string salt;
    std::string hash;
    NAPI_ASSERT(env, GetStringArg(env, args[0], password), "Failed to get password string");
    NAPI_ASSERT(env, GetStringArg(env, args[1], salt), "Failed to get salt string");
    NAPI_ASSERT(env, GetStringArg(env, args[2], hash), "Failed to get hash string");

    napi_value output;
    NAPI_CALL(env, napi_get_boolean(env, CryptoUtils::PasswordVerify(password, salt, hash), &output));
    return output;
}

static napi_value SimpleMd5(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    std::string input;
    NAPI_ASSERT(env, GetStringArg(env, args[0], input), "Failed to get string");

    return CreateStringResult(env, CryptoUtils::SimpleMd5(input));
}

struct HashAsyncContext {
    napi_async_work work = nullptr;
    napi_ref callbackRef = nullptr;
    std::string input;
    std::string result;
};

static void HashAsyncExecute(napi_env env, void* data)
{
    HashAsyncContext* context = static_cast<HashAsyncContext*>(data);
    context->result = CryptoUtils::SimpleHash(context->input);
}

static void HashAsyncComplete(napi_env env, napi_status status, void* data)
{
    HashAsyncContext* context = static_cast<HashAsyncContext*>(data);

    napi_value callback = nullptr;
    napi_get_reference_value(env, context->callbackRef, &callback);

    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value result = nullptr;
    napi_create_string_utf8(env, context->result.c_str(), context->result.length(), &result);

    napi_value args[2] = { nullptr };
    napi_get_undefined(env, &args[0]);
    args[1] = result;

    napi_value callbackResult = nullptr;
    napi_call_function(env, undefined, callback, 2, args, &callbackResult);

    napi_delete_reference(env, context->callbackRef);
    napi_delete_async_work(env, context->work);
    delete context;
}

static napi_value HashAsync(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 2, "Wrong number of arguments");

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, args[1], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_function, "Second argument must be a function");

    HashAsyncContext* context = new HashAsyncContext();
    NAPI_ASSERT(env, GetStringArg(env, args[0], context->input), "Failed to get string");
    NAPI_CALL(env, napi_create_reference(env, args[1], 1, &context->callbackRef));

    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "HashAsync", NAPI_AUTO_LENGTH, &resourceName));

    NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName, HashAsyncExecute,
        HashAsyncComplete, context, &context->work));

    NAPI_CALL(env, napi_queue_async_work(env, context->work));

    napi_value undefined = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undefined));
    return undefined;
}

EXTERN_C_START

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("base64Encode", Base64Encode),
        DECLARE_NAPI_FUNCTION("base64Decode", Base64Decode),
        DECLARE_NAPI_FUNCTION("hexEncode", HexEncode),
        DECLARE_NAPI_FUNCTION("hexDecode", HexDecode),
        DECLARE_NAPI_FUNCTION("xorEncrypt", XorEncrypt),
        DECLARE_NAPI_FUNCTION("xorDecrypt", XorDecrypt),
        DECLARE_NAPI_FUNCTION("simpleHash", SimpleHash),
        DECLARE_NAPI_FUNCTION("rot13", Rot13),
        DECLARE_NAPI_FUNCTION("reverseString", ReverseString),
        DECLARE_NAPI_FUNCTION("crc32", Crc32),
        DECLARE_NAPI_FUNCTION("caesarEncrypt", CaesarEncrypt),
        DECLARE_NAPI_FUNCTION("caesarDecrypt", CaesarDecrypt),
        DECLARE_NAPI_FUNCTION("vigenereEncrypt", VigenereEncrypt),
        DECLARE_NAPI_FUNCTION("vigenereDecrypt", VigenereDecrypt),
        DECLARE_NAPI_FUNCTION("randomString", RandomString),
        DECLARE_NAPI_FUNCTION("passwordHash", PasswordHash),
        DECLARE_NAPI_FUNCTION("passwordVerify", PasswordVerify),
        DECLARE_NAPI_FUNCTION("simpleMd5", SimpleMd5),
        DECLARE_NAPI_FUNCTION("hashAsync", HashAsync),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    return exports;
}

EXTERN_C_END

static napi_module cryptoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "crypto",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&cryptoModule);
}