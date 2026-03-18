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
#include <cstring>
#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

static constexpr int REQUIRED_ARGS_ONE = 1;
static constexpr int ARG_INDEX_ZERO = 0;
static constexpr int MAX_INPUT_LENGTH = 1024;
static constexpr int BASE64_CHAR_COUNT = 64;
static constexpr int BASE64_CHUNK_SIZE = 4;
static constexpr int BASE64_BYTE_COUNT = 3;
static constexpr int BASE64_BYTE_BITS = 8;
static constexpr int BASE64_CHAR_BITS = 6;
static constexpr int BASE64_MULTIPLIER_3 = 3;
static constexpr int BASE64_MULTIPLIER_2 = 2;
static constexpr int CHUNK_SIZE_INCREMENT = 1;
static constexpr int SHA256_CHUNK_BYTES = 64;
static constexpr int SHA256_CHUNK_WORDS = 16;
static constexpr int SHA256_ROUNDS = 64;
static constexpr int SHA256_WORD_BYTES = 4;
static constexpr int SHA256_WORD_BITS = 32;
static constexpr int HEX_WIDTH = 8;
static constexpr int CHUNK_ALIGNMENT_BITS = 512;
static constexpr int INITIAL_PADDING = 0x80;
static constexpr int ZERO_PADDING = 0x00;
static constexpr int SHA256_CONSTANT_0 = 0x428a2f98;
static constexpr int BYTE_MASK = 0xFF;
static constexpr int BASE64_MASK = 0x3F;
static constexpr int CHAR_TABLE_SIZE = 256;
static constexpr int INVALID_CHAR_INDEX = -1;
static constexpr int BYTE_SHIFT_24 = 24;
static constexpr int BYTE_SHIFT_16 = 16;
static constexpr int BYTE_SHIFT_8 = 8;
static constexpr int ROTATION_BITS_2 = 2;
static constexpr int ROTATION_BITS_3 = 3;
static constexpr int ROTATION_BITS_6 = 6;
static constexpr int ROTATION_BITS_7 = 7;
static constexpr int ROTATION_BITS_10 = 10;
static constexpr int ROTATION_BITS_11 = 11;
static constexpr int ROTATION_BITS_13 = 13;
static constexpr int ROTATION_BITS_17 = 17;
static constexpr int ROTATION_BITS_18 = 18;
static constexpr int ROTATION_BITS_19 = 19;
static constexpr int ROTATION_BITS_22 = 22;
static constexpr int ROTATION_BITS_25 = 25;
static constexpr int WORK_ARRAY_OFFSET_2 = 2;
static constexpr int WORK_ARRAY_OFFSET_7 = 7;
static constexpr int WORK_ARRAY_OFFSET_15 = 15;
static constexpr int WORK_ARRAY_OFFSET_16 = 16;
static constexpr int BYTE_OFFSET_1 = 1;
static constexpr int BYTE_OFFSET_2 = 2;
static constexpr int BYTE_OFFSET_3 = 3;

struct HashState {
    uint32_t h0;
    uint32_t h1;
    uint32_t h2;
    uint32_t h3;
    uint32_t h4;
    uint32_t h5;
    uint32_t h6;
    uint32_t h7;
};

static std::string ExtractString(napi_env env, napi_value value)
{
    size_t bufferSize = 0;
    NAPI_CALL_BASE(env, napi_get_value_string_utf8(env, value, nullptr, 0,
                                                 &bufferSize), std::string());

    if (bufferSize == 0 || bufferSize > MAX_INPUT_LENGTH) {
        return std::string();
    }

    std::string result(bufferSize, '\0');
    size_t copiedLength = 0;
    NAPI_CALL_BASE(env, napi_get_value_string_utf8(env, value, &result[0],
                                                 bufferSize, &copiedLength), std::string());
    result.resize(copiedLength);
    return result;
}

static napi_value CreateString(napi_env env, const std::string& str)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, str.c_str(), str.length(), &result));
    return result;
}

static napi_value EncodeBase64(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: input");

    std::string input = ExtractString(env, argv[ARG_INDEX_ZERO]);
    if (input.empty()) {
        return CreateString(env, "");
    }

    std::string result;
    const char* base64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+";
    
    size_t i = 0;
    size_t len = input.length();
    
    while (i < len) {
        uint32_t chunk = 0;
        size_t chunkSize = 0;
        
        for (size_t j = 0; j < BASE64_BYTE_COUNT && i + j < len; ++j) {
            chunk = (chunk << BASE64_BYTE_BITS) + static_cast<unsigned char>(input[i + j]);
            chunkSize++;
        }

        chunk <<= (BASE64_BYTE_COUNT - chunkSize) * BASE64_BYTE_BITS;

        for (size_t j = 0; j < BASE64_CHUNK_SIZE; ++j) {
            if (j * BASE64_CHAR_BITS < (chunkSize + CHUNK_SIZE_INCREMENT) * BASE64_BYTE_BITS) {
                int shift = BASE64_BYTE_BITS * BASE64_MULTIPLIER_3 - static_cast<int>(j) * BASE64_CHAR_BITS;
                result += base64Chars[(chunk >> shift) & BASE64_MASK];
            } else {
                result += '=';
            }
        }
        
        i += chunkSize;
    }

    return CreateString(env, result);
}

static napi_value DecodeBase64(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: input");

    std::string input = ExtractString(env, argv[ARG_INDEX_ZERO]);
    if (input.empty()) {
        return CreateString(env, "");
    }

    std::string result;
    const char* base64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+";
    
    std::vector<int> charToVal(CHAR_TABLE_SIZE, INVALID_CHAR_INDEX);
    for (size_t i = 0; i < BASE64_CHAR_COUNT; ++i) {
        charToVal[static_cast<unsigned char>(base64Chars[i])] = i;
    }

    size_t i = 0;
    size_t len = input.length();
    size_t padding = 0;
    
    while (len > 0 && input[len - 1] == '=') {
        padding++;
        len--;
    }
    
    while (i < len) {
        uint32_t chunk = 0;
        size_t validChars = 0;
        
        for (size_t j = 0; j < BASE64_CHUNK_SIZE && i + j < len; ++j) {
            char c = input[i + j];
            int val = charToVal[static_cast<unsigned char>(c)];
            if (val != INVALID_CHAR_INDEX) {
                chunk = (chunk << BASE64_BYTE_BITS) + val;
                validChars++;
            }
        }

        if (validChars == BASE64_CHUNK_SIZE) {
            for (size_t j = 0; j < BASE64_BYTE_COUNT && j < (validChars - padding); ++j) {
                int shift = BASE64_BYTE_BITS * BASE64_MULTIPLIER_2 - static_cast<int>(j) * BASE64_BYTE_BITS;
                result += static_cast<char>((chunk >> shift) & BYTE_MASK);
            }
        }

        i += BASE64_CHUNK_SIZE;
    }

    return CreateString(env, result);
}

static std::string PreparePadding(const std::string& input)
{
    uint64_t bitLength = static_cast<uint64_t>(input.length()) * static_cast<uint64_t>(BASE64_BYTE_BITS);
    std::string paddedInput = input + static_cast<char>(INITIAL_PADDING);

    uint64_t totalBits = static_cast<uint64_t>(paddedInput.length()) * static_cast<uint64_t>(BASE64_BYTE_BITS) +
                         static_cast<uint64_t>(SHA256_CHUNK_BYTES);
    while (totalBits % static_cast<uint64_t>(CHUNK_ALIGNMENT_BITS) != 0) {
        paddedInput += static_cast<char>(ZERO_PADDING);
    }

    for (size_t i = 0; i < SHA256_WORD_BYTES; ++i) {
        int shift = BASE64_BYTE_BITS * SHA256_WORD_BYTES - static_cast<int>(i) * BASE64_BYTE_BITS;
        paddedInput += static_cast<char>((bitLength >> static_cast<int>(shift)) & BYTE_MASK);
    }

    return paddedInput;
}

static uint32_t Rotr(uint32_t x, uint32_t n)
{
    return (x >> n) | (x << (SHA256_WORD_BITS - n));
}

static void InitializeWorkArray(uint32_t w[], const std::string& paddedInput, size_t chunkStart)
{
    for (size_t i = 0; i < SHA256_CHUNK_WORDS; ++i) {
        size_t pos = chunkStart + i * SHA256_WORD_BYTES;
        uint32_t byte0 = static_cast<unsigned char>(paddedInput[pos]) << BYTE_SHIFT_24;
        uint32_t byte1 = static_cast<unsigned char>(paddedInput[pos + BYTE_OFFSET_1]) << BYTE_SHIFT_16;
        uint32_t byte2 = static_cast<unsigned char>(paddedInput[pos + BYTE_OFFSET_2]) << BYTE_SHIFT_8;
        uint32_t byte3 = static_cast<unsigned char>(paddedInput[pos + BYTE_OFFSET_3]);
        w[i] = byte0 | byte1 | byte2 | byte3;
    }
}

static void ExtendWorkArray(uint32_t w[])
{
    for (size_t i = SHA256_CHUNK_WORDS; i < SHA256_ROUNDS; ++i) {
        uint32_t w2 = w[i - WORK_ARRAY_OFFSET_2];
        uint32_t w15 = w[i - WORK_ARRAY_OFFSET_15];
        uint32_t sigma0 = Rotr(w2, ROTATION_BITS_17) ^ Rotr(w2, ROTATION_BITS_19) ^ (w2 >> ROTATION_BITS_10);
        uint32_t sigma1 = Rotr(w15, ROTATION_BITS_7) ^ Rotr(w15, ROTATION_BITS_18) ^ (w15 >> ROTATION_BITS_3);
        w[i] = sigma0 + w[i - WORK_ARRAY_OFFSET_7] + sigma1 + w[i - WORK_ARRAY_OFFSET_16];
    }
}

static void ProcessCompressionRound(HashState& state, const uint32_t w[], size_t roundIndex)
{
    uint32_t a = state.h0;
    uint32_t b = state.h1;
    uint32_t c = state.h2;
    uint32_t d = state.h3;
    uint32_t e = state.h4;
    uint32_t f = state.h5;
    uint32_t g = state.h6;
    uint32_t h = state.h7;

    uint32_t temp1 = h + (Rotr(e, ROTATION_BITS_6) ^ Rotr(e, ROTATION_BITS_11) ^ Rotr(e, ROTATION_BITS_25)) +
                       ((e & f) ^ (~e & g)) + SHA256_CONSTANT_0 + w[roundIndex];
    uint32_t temp2 = (Rotr(a, ROTATION_BITS_2) ^ Rotr(a, ROTATION_BITS_13) ^ Rotr(a, ROTATION_BITS_22)) +
                       ((a & b) ^ (a & c) ^ (b & c));

    h = g;
    g = f;
    f = e;
    e = d + temp1;
    d = c;
    c = b;
    b = a;
    a = temp1 + temp2;

    state.h0 += a;
    state.h1 += b;
    state.h2 += c;
    state.h3 += d;
    state.h4 += e;
    state.h5 += f;
    state.h6 += g;
    state.h7 += h;
}

static std::string FormatHashOutput(const HashState& state)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0') <<
          std::setw(HEX_WIDTH) << state.h0 << std::setw(HEX_WIDTH) << state.h1 <<
          std::setw(HEX_WIDTH) << state.h2 << std::setw(HEX_WIDTH) << state.h3 <<
          std::setw(HEX_WIDTH) << state.h4 << std::setw(HEX_WIDTH) << state.h5 <<
          std::setw(HEX_WIDTH) << state.h6 << std::setw(HEX_WIDTH) << state.h7;
    return ss.str();
}

static std::string CalculateSHA256Internal(const std::string& input)
{
    HashState state = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                       0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

    std::string paddedInput = PreparePadding(input);

    for (size_t chunkStart = 0; chunkStart < paddedInput.length(); chunkStart += SHA256_CHUNK_BYTES) {
        uint32_t w[SHA256_ROUNDS];
        InitializeWorkArray(w, paddedInput, chunkStart);
        ExtendWorkArray(w);

        for (size_t i = 0; i < SHA256_ROUNDS; ++i) {
            ProcessCompressionRound(state, w, i);
        }
    }

    return FormatHashOutput(state);
}

static napi_value CalculateSHA256(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: input");

    std::string input = ExtractString(env, argv[ARG_INDEX_ZERO]);
    if (input.empty()) {
        return CreateString(env, "");
    }

    std::string hashOutput = CalculateSHA256Internal(input);

    return CreateString(env, hashOutput);
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("encodeBase64", EncodeBase64),
        DECLARE_NAPI_FUNCTION("decodeBase64", DecodeBase64),
        DECLARE_NAPI_FUNCTION("sha256", CalculateSHA256),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
                                      sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module cryptoSuiteModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "cryptoSuite",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void CryptoSuiteRegisterModule(void)
{
    napi_module_register(&cryptoSuiteModule);
}
