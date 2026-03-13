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
#include <cstring>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace {

constexpr uint32_t MODULE_VERSION = 1;
constexpr uint32_t NO_MODULE_FLAGS = 0;
constexpr int AES_BLOCK_SIZE = 16;
constexpr int SHA256_DIGEST_SIZE = 32;
constexpr int MD5_DIGEST_SIZE = 16;

static const char BASE64_CHARS[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char HEX_CHARS[] = "0123456789abcdef";

std::string BytesToHex(const uint8_t* data, size_t length)
{
    std::ostringstream oss;
    for (size_t i = 0; i < length; i++) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }
    return oss.str();
}

std::string HexToBytes(const std::string& hex)
{
    std::string bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(strtol(byteString.c_str(), nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

std::string Base64Encode(const std::string& input)
{
    std::string output;
    size_t i = 0;
    while (i < input.length()) {
        uint32_t octetA = i < input.length() ? static_cast<unsigned char>(input[i++]) : 0;
        uint32_t octetB = i < input.length() ? static_cast<unsigned char>(input[i++]) : 0;
        uint32_t octetC = i < input.length() ? static_cast<unsigned char>(input[i++]) : 0;

        uint32_t triple = (octetA << 0x10) + (octetB << 0x08) + octetC;

        output += BASE64_CHARS[(triple >> 0x12) & 0x3F];
        output += BASE64_CHARS[(triple >> 0x0C) & 0x3F];
        output += BASE64_CHARS[(triple >> 0x06) & 0x3F];
        output += BASE64_CHARS[triple & 0x3F];
    }

    size_t mod = input.length() % 3;
    if (mod > 0) {
        output.replace(output.length() - (3 - mod), 3 - mod, 3 - mod, '=');
    }

    return output;
}

std::string Base64Decode(const std::string& input)
{
    std::string output;
    std::vector<int> T(256, -1);

    for (int i = 0; i < 64; i++) {
        T[BASE64_CHARS[i]] = i;
    }

    for (size_t i = 0; i < input.length(); i += 4) {
        uint32_t a = T[input[i]];
        uint32_t b = T[input[i + 1]];
        uint32_t c = T[input[i + 2]];
        uint32_t d = T[input[i + 3]];

        uint32_t triple = (a << 0x12) + (b << 0x0C) + (c << 0x06) + d;

        output += static_cast<char>((triple >> 0x10) & 0xFF);
        output += static_cast<char>((triple >> 0x08) & 0xFF);
        output += static_cast<char>(triple & 0xFF);
    }

    size_t padCount = 0;
    if (input.length() > 0 && input[input.length() - 1] == '=') {
        padCount++;
        if (input.length() > 1 && input[input.length() - 2] == '=') {
            padCount++;
        }
    }

    return output.substr(0, output.length() - padCount);
}

struct MD5Context {
    uint32_t state[4];
    uint32_t count[2];
    uint8_t buffer[64];
};

void MD5Init(MD5Context* ctx)
{
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->count[0] = 0;
    ctx->count[1] = 0;
    memset(ctx->buffer, 0, sizeof(ctx->buffer));
}

#define MD5_F(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define MD5_G(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define MD5_I(x, y, z) ((y) ^ ((x) | ~(z)))

#define MD5_ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

void MD5Transform(uint32_t state[4], const uint8_t block[64])
{
    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];

    uint32_t x[64];
    for (int i = 0; i < 16; i++) {
        x[i] = static_cast<uint32_t>(block[i * 4]) |
                (static_cast<uint32_t>(block[i * 4 + 1]) << 8) |
                (static_cast<uint32_t>(block[i * 4 + 2]) << 16) |
                (static_cast<uint32_t>(block[i * 4 + 3]) << 24);
    }

    static const uint32_t S[] = {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
    };

    static const uint32_t K[] = {
        0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE,
        0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501,
        0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE,
        0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821,
        0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA,
        0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8,
        0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED,
        0xA9E3E905, 0xFCEFA3F8, 0x676F02D9, 0x8D2A4C8A,
        0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C,
        0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70,
        0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x04881D05,
        0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665,
        0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039,
        0x655B59C3, 0x8F0CCC92, 0xFFEFF47D, 0x85845DD1,
        0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1,
        0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391
    };

    for (int i = 0; i < 64; i++) {
        uint32_t F = 0;
        uint32_t g = 0;

        if (i < 16) {
            F = MD5_F(b, c, d);
            g = i;
        } else if (i < 32) {
            F = MD5_G(b, c, d);
            g = (5 * i + 1) % 16;
        } else if (i < 48) {
            F = MD5_H(b, c, d);
            g = (3 * i + 5) % 16;
        } else {
            F = MD5_I(b, c, d);
            g = (7 * i) % 16;
        }

        uint32_t temp = d;
        d = c;
        c = b;
        b = b + MD5_ROTATE_LEFT((a + F + K[i] + x[g]), S[i]);
        a = temp;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

void MD5Update(MD5Context* ctx, const uint8_t* data, size_t length)
{
    uint32_t i = (ctx->count[0] >> 3) & 0x3F;

    ctx->count[0] += static_cast<uint32_t>(length << 3);
    if (ctx->count[0] < (length << 3)) {
        ctx->count[1]++;
    }
    ctx->count[1] += static_cast<uint32_t>(length >> 29);

    size_t partLen = 64 - i;

    if (length >= partLen) {
        memcpy(&ctx->buffer[i], data, partLen);
        MD5Transform(ctx->state, ctx->buffer);
        for (size_t j = partLen; j + 64 <= length; j += 64) {
            MD5Transform(ctx->state, data + j);
        }
        i = 0;
    } else {
        partLen = 0;
    }

    memcpy(&ctx->buffer[i], data + partLen, length - partLen);
}

void MD5Final(uint8_t digest[16], MD5Context* ctx)
{
    uint8_t bits[8];
    uint32_t index = (ctx->count[0] >> 3) & 0x3F;
    uint32_t padLen = (index < 56) ? (56 - index) : (120 - index);

    MD5Update(ctx, reinterpret_cast<const uint8_t*>("\x80"), 1);

    for (uint32_t i = 0; i < padLen; i++) {
        MD5Update(ctx, reinterpret_cast<const uint8_t*>("\0"), 1);
    }

    for (int i = 0; i < 8; i++) {
        bits[i] = static_cast<uint8_t>((ctx->count[i >> 2] >> ((i & 3) * 8)) & 0xFF);
    }

    MD5Update(ctx, bits, 8);

    for (int i = 0; i < 4; i++) {
        digest[i * 4] = static_cast<uint8_t>(ctx->state[i] & 0xFF);
        digest[i * 4 + 1] = static_cast<uint8_t>((ctx->state[i] >> 8) & 0xFF);
        digest[i * 4 + 2] = static_cast<uint8_t>((ctx->state[i] >> 16) & 0xFF);
        digest[i * 4 + 3] = static_cast<uint8_t>((ctx->state[i] >> 24) & 0xFF);
    }
}

std::string MD5Hash(const std::string& input)
{
    MD5Context ctx;
    MD5Init(&ctx);
    MD5Update(&ctx, reinterpret_cast<const uint8_t*>(input.c_str()), input.length());

    uint8_t digest[MD5_DIGEST_SIZE];
    MD5Final(digest, &ctx);

    return BytesToHex(digest, MD5_DIGEST_SIZE);
}

struct SHA256Context {
    uint32_t state[8];
    uint64_t count;
    uint8_t buffer[64];
};

void SHA256Init(SHA256Context* ctx)
{
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
    ctx->count = 0;
    memset(ctx->buffer, 0, sizeof(ctx->buffer));
}

#define SHA256_ROTRIGHT(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define SHA256_CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define SHA256_MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SHA256_EP0(x) (SHA256_ROTRIGHT(x, 2) ^ SHA256_ROTRIGHT(x, 13) ^ SHA256_ROTRIGHT(x, 22))
#define SHA256_EP1(x) (SHA256_ROTRIGHT(x, 6) ^ SHA256_ROTRIGHT(x, 11) ^ SHA256_ROTRIGHT(x, 25))
#define SHA256_SIG0(x) (SHA256_ROTRIGHT(x, 7) ^ SHA256_ROTRIGHT(x, 18) ^ ((x) >> 3))
#define SHA256_SIG1(x) (SHA256_ROTRIGHT(x, 17) ^ SHA256_ROTRIGHT(x, 19) ^ ((x) >> 10))

void SHA256Transform(SHA256Context* ctx, const uint8_t block[64])
{
    uint32_t W[64];
    uint32_t a = ctx->state[0];
    uint32_t b = ctx->state[1];
    uint32_t c = ctx->state[2];
    uint32_t d = ctx->state[3];
    uint32_t e = ctx->state[4];
    uint32_t f = ctx->state[5];
    uint32_t g = ctx->state[6];
    uint32_t h = ctx->state[7];

    for (int i = 0; i < 16; i++) {
        W[i] = (static_cast<uint32_t>(block[i * 4]) << 24) |
               (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
               (static_cast<uint32_t>(block[i * 4 + 2]) << 8) |
               static_cast<uint32_t>(block[i * 4 + 3]);
    }

    for (int i = 16; i < 64; i++) {
        W[i] = SHA256_SIG1(W[i - 2]) + W[i - 7] + SHA256_SIG0(W[i - 15]) + W[i - 16];
    }

    static const uint32_t K[] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    for (int i = 0; i < 64; i++) {
        uint32_t T1 = h + SHA256_EP1(e) + SHA256_CH(e, f, g) + K[i] + W[i];
        uint32_t T2 = SHA256_EP0(a) + SHA256_MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

void SHA256Update(SHA256Context* ctx, const uint8_t* data, size_t length)
{
    size_t index = (ctx->count & 0x3F);
    ctx->count += length;

    size_t partLen = 64 - index;

    if (length >= partLen) {
        memcpy(&ctx->buffer[index], data, partLen);
        SHA256Transform(ctx, ctx->buffer);
        for (size_t j = partLen; j + 64 <= length; j += 64) {
            SHA256Transform(ctx, data + j);
        }
        index = 0;
    } else {
        partLen = 0;
    }

    memcpy(&ctx->buffer[index], data + partLen, length - partLen);
}

void SHA256Final(uint8_t digest[32], SHA256Context* ctx)
{
    uint8_t bits[8];
    size_t index = (ctx->count & 0x3F);
    size_t padLen = (index < 56) ? (56 - index) : (120 - index);

    SHA256Update(ctx, reinterpret_cast<const uint8_t*>("\x80"), 1);

    for (size_t i = 0; i < padLen; i++) {
        SHA256Update(ctx, reinterpret_cast<const uint8_t*>("\0"), 1);
    }

    for (int i = 0; i < 8; i++) {
        bits[i] = static_cast<uint8_t>((ctx->count >> (56 - i * 8)) & 0xFF);
    }

    SHA256Update(ctx, bits, 8);

    for (int i = 0; i < 8; i++) {
        digest[i * 4] = static_cast<uint8_t>((ctx->state[i] >> 24) & 0xFF);
        digest[i * 4 + 1] = static_cast<uint8_t>((ctx->state[i] >> 16) & 0xFF);
        digest[i * 4 + 2] = static_cast<uint8_t>((ctx->state[i] >> 8) & 0xFF);
        digest[i * 4 + 3] = static_cast<uint8_t>(ctx->state[i] & 0xFF);
    }
}

std::string SHA256Hash(const std::string& input)
{
    SHA256Context ctx;
    SHA256Init(&ctx);
    SHA256Update(&ctx, reinterpret_cast<const uint8_t*>(input.c_str()), input.length());

    uint8_t digest[SHA256_DIGEST_SIZE];
    SHA256Final(digest, &ctx);

    return BytesToHex(digest, SHA256_DIGEST_SIZE);
}

std::string AESEncrypt(const std::string& plaintext, const std::string& key)
{
    std::string result = plaintext;
    std::string paddedKey = key;
    while (paddedKey.length() < AES_BLOCK_SIZE) {
        paddedKey += '\0';
    }

    for (size_t i = 0; i < result.length(); i++) {
        result[i] ^= paddedKey[i % AES_BLOCK_SIZE];
    }

    return Base64Encode(result);
}

std::string AESDecrypt(const std::string& ciphertext, const std::string& key)
{
    std::string encrypted = Base64Decode(ciphertext);
    std::string result = encrypted;
    std::string paddedKey = key;
    while (paddedKey.length() < AES_BLOCK_SIZE) {
        paddedKey += '\0';
    }

    for (size_t i = 0; i < result.length(); i++) {
        result[i] ^= paddedKey[i % AES_BLOCK_SIZE];
    }

    return result;
}

uint32_t GenerateRandomNumber()
{
    return static_cast<uint32_t>(rand()) ^ (static_cast<uint32_t>(rand()) << 16);
}

napi_value Md5Hash(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 1) {
        napi_throw_error(env, nullptr, "Input string is required");
        return nullptr;
    }

    size_t strLength = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strLength);

    std::vector<char> buffer(strLength + 1);
    napi_get_value_string_utf8(env, argv[0], buffer.data(), strLength + 1, &strLength);

    std::string hash = MD5Hash(std::string(buffer.data()));

    napi_value result;
    napi_create_string_utf8(env, hash.c_str(), hash.length(), &result);
    return result;
}

napi_value Sha256Hash(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 1) {
        napi_throw_error(env, nullptr, "Input string is required");
        return nullptr;
    }

    size_t strLength = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strLength);

    std::vector<char> buffer(strLength + 1);
    napi_get_value_string_utf8(env, argv[0], buffer.data(), strLength + 1, &strLength);

    std::string hash = SHA256Hash(std::string(buffer.data()));

    napi_value result;
    napi_create_string_utf8(env, hash.c_str(), hash.length(), &result);
    return result;
}

napi_value Base64Encode(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 1) {
        napi_throw_error(env, nullptr, "Input string is required");
        return nullptr;
    }

    size_t strLength = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strLength);

    std::vector<char> buffer(strLength + 1);
    napi_get_value_string_utf8(env, argv[0], buffer.data(), strLength + 1, &strLength);

    std::string encoded = Base64Encode(std::string(buffer.data()));

    napi_value result;
    napi_create_string_utf8(env, encoded.c_str(), encoded.length(), &result);
    return result;
}

napi_value Base64Decode(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 1) {
        napi_throw_error(env, nullptr, "Input string is required");
        return nullptr;
    }

    size_t strLength = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strLength);

    std::vector<char> buffer(strLength + 1);
    napi_get_value_string_utf8(env, argv[0], buffer.data(), strLength + 1, &strLength);

    std::string decoded = Base64Decode(std::string(buffer.data()));

    napi_value result;
    napi_create_string_utf8(env, decoded.c_str(), decoded.length(), &result);
    return result;
}

napi_value AesEncrypt(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 2) {
        napi_throw_error(env, nullptr, "Plaintext and key are required");
        return nullptr;
    }

    size_t textLength = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &textLength);
    std::vector<char> textBuffer(textLength + 1);
    napi_get_value_string_utf8(env, argv[0], textBuffer.data(), textLength + 1, &textLength);

    size_t keyLength = 0;
    napi_get_value_string_utf8(env, argv[1], nullptr, 0, &keyLength);
    std::vector<char> keyBuffer(keyLength + 1);
    napi_get_value_string_utf8(env, argv[1], keyBuffer.data(), keyLength + 1, &keyLength);

    std::string encrypted = AESEncrypt(std::string(textBuffer.data()), std::string(keyBuffer.data()));

    napi_value result;
    napi_create_string_utf8(env, encrypted.c_str(), encrypted.length(), &result);
    return result;
}

napi_value AesDecrypt(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 2) {
        napi_throw_error(env, nullptr, "Ciphertext and key are required");
        return nullptr;
    }

    size_t textLength = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &textLength);
    std::vector<char> textBuffer(textLength + 1);
    napi_get_value_string_utf8(env, argv[0], textBuffer.data(), textLength + 1, &textLength);

    size_t keyLength = 0;
    napi_get_value_string_utf8(env, argv[1], nullptr, 0, &keyLength);
    std::vector<char> keyBuffer(keyLength + 1);
    napi_get_value_string_utf8(env, argv[1], keyBuffer.data(), keyLength + 1, &keyLength);

    std::string decrypted = AESDecrypt(std::string(textBuffer.data()), std::string(keyBuffer.data()));

    napi_value result;
    napi_create_string_utf8(env, decrypted.c_str(), decrypted.length(), &result);
    return result;
}

napi_value GenerateRandom(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    int32_t min = 0;
    int32_t max = RAND_MAX;

    if (argc >= 1) {
        napi_valuetype type;
        napi_typeof(env, argv[0], &type);

        if (type == napi_number) {
            napi_get_value_int32(env, argv[0], &max);
        } else if (type == napi_object) {
            napi_value minVal;
            napi_value maxVal;
            napi_get_named_property(env, argv[0], "min", &minVal);
            napi_get_named_property(env, argv[0], "max", &maxVal);

            napi_valuetype minType;
            napi_typeof(env, minVal, &minType);
            if (minType == napi_number) {
                napi_get_value_int32(env, minVal, &min);
            }

            napi_valuetype maxType;
            napi_typeof(env, maxVal, &maxType);
            if (maxType == napi_number) {
                napi_get_value_int32(env, maxVal, &max);
            }
        }
    }

    srand(static_cast<unsigned>(time(nullptr)));
    uint32_t random = GenerateRandomNumber();
    int32_t result = min + (random % (max - min + 1));

    napi_value resultValue;
    napi_create_int32(env, result, &resultValue);
    return resultValue;
}

napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("md5", Md5Hash),
        DECLARE_NAPI_FUNCTION("sha256", Sha256Hash),
        DECLARE_NAPI_FUNCTION("base64Encode", Base64Encode),
        DECLARE_NAPI_FUNCTION("base64Decode", Base64Decode),
        DECLARE_NAPI_FUNCTION("aesEncrypt", AesEncrypt),
        DECLARE_NAPI_FUNCTION("aesDecrypt", AesDecrypt),
        DECLARE_NAPI_FUNCTION("random", GenerateRandom),
    };

    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}

}

extern "C" __attribute__((visibility("default"))) napi_value
NAPI_Register(napi_env env, napi_value exports)
{
    return Init(env, exports);
}
