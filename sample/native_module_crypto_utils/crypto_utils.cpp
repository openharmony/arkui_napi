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
#include <limits>
#include <random>
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
constexpr int BASE64_TABLE_SIZE = 64;
constexpr int HEX_TABLE_SIZE = 16;
constexpr int MD5_BUFFER_SIZE = 64;
constexpr int SHA256_BUFFER_SIZE = 64;
constexpr int MD5_STATE_SIZE = 4;
constexpr int SHA256_STATE_SIZE = 8;
constexpr int MD5_COUNT_WORDS = 2;
constexpr int MD5_STATE_INDEX_A = 0;
constexpr int MD5_STATE_INDEX_B = 1;
constexpr int MD5_STATE_INDEX_C = 2;
constexpr int MD5_STATE_INDEX_D = 3;
constexpr int SHA256_STATE_INDEX_0 = 0;
constexpr int SHA256_STATE_INDEX_1 = 1;
constexpr int SHA256_STATE_INDEX_2 = 2;
constexpr int SHA256_STATE_INDEX_3 = 3;
constexpr int SHA256_STATE_INDEX_4 = 4;
constexpr int SHA256_STATE_INDEX_5 = 5;
constexpr int SHA256_STATE_INDEX_6 = 6;
constexpr int SHA256_STATE_INDEX_7 = 7;
constexpr int BYTES_PER_INT32 = 4;
constexpr int BITS_PER_BYTE = 8;
constexpr int MD5_ROUNDS = 64;
constexpr int SHA256_ROUNDS = 64;
constexpr int HEX_CHAR_WIDTH = 2;
constexpr int HEX_BYTE_WIDTH = 2;
constexpr int BASE64_DECODE_TABLE_SIZE = 256;
constexpr int BASE64_SHIFT_6 = 6;
constexpr int BASE64_SHIFT_8 = 8;
constexpr int BASE64_SHIFT_12 = 12;
constexpr int BASE64_SHIFT_16 = 16;
constexpr int BASE64_SHIFT_18 = 18;
constexpr int BASE64_SHIFT_24 = 24;
constexpr size_t BASE64_INDEX_0 = 0;
constexpr size_t BASE64_INDEX_1 = 1;
constexpr size_t BASE64_INDEX_2 = 2;
constexpr size_t BASE64_INDEX_3 = 3;
constexpr size_t BASE64_PADDING_OFFSET_1 = 1;
constexpr size_t BASE64_PADDING_OFFSET_2 = 2;
constexpr int MD5_ROUND_1_END = 16;
constexpr int MD5_ROUND_2_END = 32;
constexpr int MD5_ROUND_3_END = 48;
constexpr int MD5_BLOCK_WORDS = 16;
constexpr int MD5_LENGTH_BYTES = 8;
constexpr int MD5_G_MULTIPLIER_1 = 5;
constexpr int MD5_G_OFFSET_1 = 1;
constexpr int MD5_G_MULTIPLIER_2 = 3;
constexpr int MD5_G_OFFSET_2 = 5;
constexpr int MD5_G_MULTIPLIER_3 = 7;
constexpr int MD5_COUNT_SHIFT = 3;
constexpr int MD5_COUNT_CARRY_SHIFT = 29;
constexpr uint32_t MD5_INDEX_MASK = 0x3F;
constexpr int MD5_COUNT_INDEX_SHIFT = 2;
constexpr int MD5_COUNT_INDEX_MASK = 3;
constexpr int SHA256_FIRST_WORDS = 16;
constexpr int SHA256_COUNT_HIGH_SHIFT = 56;
constexpr int SHA256_WORD_OFFSET_2 = 2;
constexpr int SHA256_WORD_OFFSET_7 = 7;
constexpr int SHA256_WORD_OFFSET_15 = 15;
constexpr int SHA256_WORD_OFFSET_16 = 16;
constexpr int BYTE_SHIFT_8 = 8;
constexpr int BYTE_SHIFT_16 = 16;
constexpr int BYTE_SHIFT_24 = 24;
constexpr int BITS_PER_UINT32 = 32;
constexpr int BYTE_OFFSET_1 = 1;
constexpr int BYTE_OFFSET_2 = 2;
constexpr int BYTE_OFFSET_3 = 3;
constexpr size_t ARG_INDEX_0 = 0;
constexpr size_t ARG_INDEX_1 = 1;
constexpr size_t SINGLE_ARG_COUNT = 1;
constexpr size_t TWO_ARG_COUNT = 2;
constexpr int SHA256_PADDING_56 = 56;
constexpr int SHA256_PADDING_120 = 120;
constexpr int BASE64_CHUNK_SIZE = 3;
constexpr int BASE64_OUTPUT_SIZE = 4;

static const char BASE64_CHARS[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char HEX_CHARS[] = "0123456789abcdef";

std::string BytesToHex(const uint8_t* data, size_t length)
{
    std::ostringstream oss;
    for (size_t i = 0; i < length; i++) {
        oss << std::hex << std::setw(HEX_CHAR_WIDTH) << std::setfill('0') << static_cast<int>(data[i]);
    }
    return oss.str();
}

std::string HexToBytes(const std::string& hex)
{
    std::string bytes;
    for (size_t i = 0; i < hex.length(); i += HEX_BYTE_WIDTH) {
        std::string byteString = hex.substr(i, HEX_BYTE_WIDTH);
        uint8_t byte = static_cast<uint8_t>(strtol(byteString.c_str(), nullptr, HEX_TABLE_SIZE));
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

        uint32_t triple = (octetA << BASE64_SHIFT_16) + (octetB << BASE64_SHIFT_8) + octetC;

        output += BASE64_CHARS[(triple >> BASE64_SHIFT_18) & 0x3F];
        output += BASE64_CHARS[(triple >> BASE64_SHIFT_12) & 0x3F];
        output += BASE64_CHARS[(triple >> BASE64_SHIFT_6) & 0x3F];
        output += BASE64_CHARS[triple & 0x3F];
    }

    size_t mod = input.length() % BASE64_CHUNK_SIZE;
    if (mod > 0) {
        output.replace(output.length() - (BASE64_CHUNK_SIZE - mod), BASE64_CHUNK_SIZE - mod,
            BASE64_CHUNK_SIZE - mod, '=');
    }

    return output;
}

std::string Base64Decode(const std::string& input)
{
    std::string output;
    std::vector<int> T(BASE64_DECODE_TABLE_SIZE, -1);

    for (int i = 0; i < BASE64_TABLE_SIZE; i++) {
        T[BASE64_CHARS[i]] = i;
    }

    for (size_t i = 0; i < input.length(); i += BASE64_OUTPUT_SIZE) {
        uint32_t a = T[input[i + BASE64_INDEX_0]];
        uint32_t b = T[input[i + BASE64_INDEX_1]];
        uint32_t c = T[input[i + BASE64_INDEX_2]];
        uint32_t d = T[input[i + BASE64_INDEX_3]];

        uint32_t triple = (a << BASE64_SHIFT_18) + (b << BASE64_SHIFT_12) + (c << BASE64_SHIFT_6) + d;

        output += static_cast<char>((triple >> BASE64_SHIFT_16) & 0xFF);
        output += static_cast<char>((triple >> BASE64_SHIFT_8) & 0xFF);
        output += static_cast<char>(triple & 0xFF);
    }

    size_t padCount = 0;
    if (input.length() > 0 && input[input.length() - BASE64_PADDING_OFFSET_1] == '=') {
        padCount++;
        if (input.length() > 1 && input[input.length() - BASE64_PADDING_OFFSET_2] == '=') {
            padCount++;
        }
    }

    return output.substr(0, output.length() - padCount);
}

struct MD5Context {
    uint32_t state[MD5_STATE_SIZE];
    uint32_t count[MD5_COUNT_WORDS];
    uint8_t buffer[MD5_BUFFER_SIZE];
};

void MD5Init(MD5Context* ctx)
{
    ctx->state[MD5_STATE_INDEX_A] = 0x67452301;
    ctx->state[MD5_STATE_INDEX_B] = 0xEFCDAB89;
    ctx->state[MD5_STATE_INDEX_C] = 0x98BADCFE;
    ctx->state[MD5_STATE_INDEX_D] = 0x10325476;
    ctx->count[ARG_INDEX_0] = 0;
    ctx->count[ARG_INDEX_1] = 0;
    std::fill(ctx->buffer, ctx->buffer + MD5_BUFFER_SIZE, 0);
}

inline uint32_t Md5F(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & y) | (~x & z);
}

inline uint32_t Md5G(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & z) | (y & ~z);
}

inline uint32_t Md5H(uint32_t x, uint32_t y, uint32_t z)
{
    return x ^ y ^ z;
}

inline uint32_t Md5I(uint32_t x, uint32_t y, uint32_t z)
{
    return y ^ (x | ~z);
}

inline uint32_t Md5RotateLeft(uint32_t x, uint32_t n)
{
    return (x << n) | (x >> (BITS_PER_UINT32 - n));
}

void Md5DecodeBlock(const uint8_t block[MD5_BUFFER_SIZE], uint32_t* x)
{
    for (int i = 0; i < MD5_BLOCK_WORDS; i++) {
        x[i] = static_cast<uint32_t>(block[i * BYTES_PER_INT32]) |
               (static_cast<uint32_t>(block[i * BYTES_PER_INT32 + BYTE_OFFSET_1]) << BYTE_SHIFT_8) |
               (static_cast<uint32_t>(block[i * BYTES_PER_INT32 + BYTE_OFFSET_2]) << BYTE_SHIFT_16) |
               (static_cast<uint32_t>(block[i * BYTES_PER_INT32 + BYTE_OFFSET_3]) << BYTE_SHIFT_24);
    }
}

void Md5ProcessRounds(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d, const uint32_t* x)
{
    static const uint32_t kShiftAmounts[MD5_ROUNDS] = {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
    };

    static const uint32_t kConstants[MD5_ROUNDS] = {
        0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE,
        0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501,
        1770035416U, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE,
        0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821,
        0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA,
        0xD62F105D, 0x02441453, 3634488961U, 0xE7D3FBC8,
        0x21E1CDE6, 0xC33707D6, 4107603335U, 0x455A14ED,
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

    for (int i = 0; i < MD5_ROUNDS; i++) {
        uint32_t f = 0;
        uint32_t g = 0;

        if (i < MD5_ROUND_1_END) {
            f = Md5F(b, c, d);
            g = i;
        } else if (i < MD5_ROUND_2_END) {
            f = Md5G(b, c, d);
            g = (MD5_G_MULTIPLIER_1 * i + MD5_G_OFFSET_1) % MD5_BLOCK_WORDS;
        } else if (i < MD5_ROUND_3_END) {
            f = Md5H(b, c, d);
            g = (MD5_G_MULTIPLIER_2 * i + MD5_G_OFFSET_2) % MD5_BLOCK_WORDS;
        } else {
            f = Md5I(b, c, d);
            g = (MD5_G_MULTIPLIER_3 * i) % MD5_BLOCK_WORDS;
        }

        uint32_t temp = d;
        d = c;
        c = b;
        b = b + Md5RotateLeft((a + f + kConstants[i] + x[g]), kShiftAmounts[i]);
        a = temp;
    }
}

void MD5Transform(uint32_t state[MD5_STATE_SIZE], const uint8_t block[MD5_BUFFER_SIZE])
{
    uint32_t a = state[MD5_STATE_INDEX_A];
    uint32_t b = state[MD5_STATE_INDEX_B];
    uint32_t c = state[MD5_STATE_INDEX_C];
    uint32_t d = state[MD5_STATE_INDEX_D];

    uint32_t x[MD5_ROUNDS];
    Md5DecodeBlock(block, x);
    Md5ProcessRounds(a, b, c, d, x);

    state[MD5_STATE_INDEX_A] += a;
    state[MD5_STATE_INDEX_B] += b;
    state[MD5_STATE_INDEX_C] += c;
    state[MD5_STATE_INDEX_D] += d;
}

void MD5Update(MD5Context* ctx, const uint8_t* data, size_t length)
{
    uint32_t i = (ctx->count[0] >> MD5_COUNT_SHIFT) & MD5_INDEX_MASK;

    ctx->count[0] += static_cast<uint32_t>(length << MD5_COUNT_SHIFT);
    if (ctx->count[0] < (length << MD5_COUNT_SHIFT)) {
        ctx->count[1]++;
    }
    ctx->count[1] += static_cast<uint32_t>(length >> MD5_COUNT_CARRY_SHIFT);

    size_t partLen = MD5_BUFFER_SIZE - i;

    if (length >= partLen) {
        std::copy(data, data + partLen, ctx->buffer + i);
        MD5Transform(ctx->state, ctx->buffer);
        for (size_t j = partLen; j + MD5_BUFFER_SIZE <= length; j += MD5_BUFFER_SIZE) {
            MD5Transform(ctx->state, data + j);
        }
        i = 0;
    } else {
        partLen = 0;
    }

    std::copy(data + partLen, data + length, ctx->buffer + i);
}

void MD5Final(uint8_t digest[MD5_DIGEST_SIZE], MD5Context* ctx)
{
    uint8_t bits[MD5_LENGTH_BYTES];
    uint32_t index = (ctx->count[0] >> MD5_COUNT_SHIFT) & MD5_INDEX_MASK;
    uint32_t padLen = (index < SHA256_PADDING_56) ? (SHA256_PADDING_56 - index) : (SHA256_PADDING_120 - index);

    MD5Update(ctx, reinterpret_cast<const uint8_t*>("\x80"), 1);

    for (uint32_t i = 0; i < padLen; i++) {
        MD5Update(ctx, reinterpret_cast<const uint8_t*>("\0"), 1);
    }

    for (int i = 0; i < MD5_LENGTH_BYTES; i++) {
        bits[i] = static_cast<uint8_t>((ctx->count[i >> MD5_COUNT_INDEX_SHIFT] >>
            ((i & MD5_COUNT_INDEX_MASK) * BITS_PER_BYTE)) & 0xFF);
    }

    MD5Update(ctx, bits, MD5_LENGTH_BYTES);

    for (int i = 0; i < MD5_STATE_SIZE; i++) {
        digest[i * BYTES_PER_INT32] = static_cast<uint8_t>(ctx->state[i] & 0xFF);
        digest[i * BYTES_PER_INT32 + BYTE_OFFSET_1] = static_cast<uint8_t>((ctx->state[i] >> BYTE_SHIFT_8) & 0xFF);
        digest[i * BYTES_PER_INT32 + BYTE_OFFSET_2] = static_cast<uint8_t>((ctx->state[i] >> BYTE_SHIFT_16) & 0xFF);
        digest[i * BYTES_PER_INT32 + BYTE_OFFSET_3] = static_cast<uint8_t>((ctx->state[i] >> BYTE_SHIFT_24) & 0xFF);
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
    uint32_t state[SHA256_STATE_SIZE];
    uint64_t count;
    uint8_t buffer[SHA256_BUFFER_SIZE];
};

void SHA256Init(SHA256Context* ctx)
{
    ctx->state[SHA256_STATE_INDEX_0] = 0x6a09e667;
    ctx->state[SHA256_STATE_INDEX_1] = 0xbb67ae85;
    ctx->state[SHA256_STATE_INDEX_2] = 0x3c6ef372;
    ctx->state[SHA256_STATE_INDEX_3] = 0xa54ff53a;
    ctx->state[SHA256_STATE_INDEX_4] = 0x510e527f;
    ctx->state[SHA256_STATE_INDEX_5] = 0x9b05688c;
    ctx->state[SHA256_STATE_INDEX_6] = 0x1f83d9ab;
    ctx->state[SHA256_STATE_INDEX_7] = 0x5be0cd19;
    ctx->count = 0;
    std::fill(ctx->buffer, ctx->buffer + SHA256_BUFFER_SIZE, 0);
}

inline uint32_t Sha256RotateRight(uint32_t x, uint32_t n)
{
    return (x >> n) | (x << (BITS_PER_UINT32 - n));
}

inline uint32_t Sha256Ch(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & y) ^ (~x & z);
}

inline uint32_t Sha256Maj(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & y) ^ (x & z) ^ (y & z);
}

inline uint32_t Sha256Ep0(uint32_t x)
{
    return Sha256RotateRight(x, 2) ^ Sha256RotateRight(x, 13) ^ Sha256RotateRight(x, 22);
}

inline uint32_t Sha256Ep1(uint32_t x)
{
    return Sha256RotateRight(x, 6) ^ Sha256RotateRight(x, 11) ^ Sha256RotateRight(x, 25);
}

inline uint32_t Sha256Sig0(uint32_t x)
{
    return Sha256RotateRight(x, 7) ^ Sha256RotateRight(x, 18) ^ (x >> 3);
}

inline uint32_t Sha256Sig1(uint32_t x)
{
    return Sha256RotateRight(x, 17) ^ Sha256RotateRight(x, 19) ^ (x >> 10);
}

void SHA256Transform(SHA256Context* ctx, const uint8_t block[SHA256_BUFFER_SIZE])
{
    uint32_t w[SHA256_ROUNDS];
    uint32_t a = ctx->state[SHA256_STATE_INDEX_0];
    uint32_t b = ctx->state[SHA256_STATE_INDEX_1];
    uint32_t c = ctx->state[SHA256_STATE_INDEX_2];
    uint32_t d = ctx->state[SHA256_STATE_INDEX_3];
    uint32_t e = ctx->state[SHA256_STATE_INDEX_4];
    uint32_t f = ctx->state[SHA256_STATE_INDEX_5];
    uint32_t g = ctx->state[SHA256_STATE_INDEX_6];
    uint32_t h = ctx->state[SHA256_STATE_INDEX_7];

    for (int i = 0; i < SHA256_FIRST_WORDS; i++) {
        w[i] = (static_cast<uint32_t>(block[i * BYTES_PER_INT32]) << BYTE_SHIFT_24) |
               (static_cast<uint32_t>(block[i * BYTES_PER_INT32 + BYTE_OFFSET_1]) << BYTE_SHIFT_16) |
               (static_cast<uint32_t>(block[i * BYTES_PER_INT32 + BYTE_OFFSET_2]) << BYTE_SHIFT_8) |
               static_cast<uint32_t>(block[i * BYTES_PER_INT32 + BYTE_OFFSET_3]);
    }

    for (int i = SHA256_FIRST_WORDS; i < SHA256_ROUNDS; i++) {
        w[i] = Sha256Sig1(w[i - SHA256_WORD_OFFSET_2]) + w[i - SHA256_WORD_OFFSET_7] +
               Sha256Sig0(w[i - SHA256_WORD_OFFSET_15]) + w[i - SHA256_WORD_OFFSET_16];
    }

    static const uint32_t kSha256Constants[SHA256_ROUNDS] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        3624381080U, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 1322822218U, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    for (int i = 0; i < SHA256_ROUNDS; i++) {
        uint32_t t1 = h + Sha256Ep1(e) + Sha256Ch(e, f, g) + kSha256Constants[i] + w[i];
        uint32_t t2 = Sha256Ep0(a) + Sha256Maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[SHA256_STATE_INDEX_0] += a;
    ctx->state[SHA256_STATE_INDEX_1] += b;
    ctx->state[SHA256_STATE_INDEX_2] += c;
    ctx->state[SHA256_STATE_INDEX_3] += d;
    ctx->state[SHA256_STATE_INDEX_4] += e;
    ctx->state[SHA256_STATE_INDEX_5] += f;
    ctx->state[SHA256_STATE_INDEX_6] += g;
    ctx->state[SHA256_STATE_INDEX_7] += h;
}

void SHA256Update(SHA256Context* ctx, const uint8_t* data, size_t length)
{
    size_t index = (ctx->count & 0x3F);
    ctx->count += length;

    size_t partLen = SHA256_BUFFER_SIZE - index;

    if (length >= partLen) {
        std::copy(data, data + partLen, ctx->buffer + index);
        SHA256Transform(ctx, ctx->buffer);
        for (size_t j = partLen; j + SHA256_BUFFER_SIZE <= length; j += SHA256_BUFFER_SIZE) {
            SHA256Transform(ctx, data + j);
        }
        index = 0;
    } else {
        partLen = 0;
    }

    std::copy(data + partLen, data + length, ctx->buffer + index);
}

void SHA256Final(uint8_t digest[SHA256_DIGEST_SIZE], SHA256Context* ctx)
{
    uint8_t bits[SHA256_STATE_SIZE];
    size_t index = (ctx->count & 0x3F);
    size_t padLen = (index < SHA256_PADDING_56) ? (SHA256_PADDING_56 - index) : (SHA256_PADDING_120 - index);

    SHA256Update(ctx, reinterpret_cast<const uint8_t*>("\x80"), 1);

    for (size_t i = 0; i < padLen; i++) {
        SHA256Update(ctx, reinterpret_cast<const uint8_t*>("\0"), 1);
    }

    for (int i = 0; i < SHA256_STATE_SIZE; i++) {
        bits[i] = static_cast<uint8_t>((ctx->count >> (SHA256_COUNT_HIGH_SHIFT - i * BITS_PER_BYTE)) & 0xFF);
    }

    SHA256Update(ctx, bits, SHA256_STATE_SIZE);

    for (int i = 0; i < SHA256_STATE_SIZE; i++) {
        digest[i * BYTES_PER_INT32] = static_cast<uint8_t>((ctx->state[i] >> BYTE_SHIFT_24) & 0xFF);
        digest[i * BYTES_PER_INT32 + BYTE_OFFSET_1] = static_cast<uint8_t>((ctx->state[i] >> BYTE_SHIFT_16) & 0xFF);
        digest[i * BYTES_PER_INT32 + BYTE_OFFSET_2] = static_cast<uint8_t>((ctx->state[i] >> BYTE_SHIFT_8) & 0xFF);
        digest[i * BYTES_PER_INT32 + BYTE_OFFSET_3] = static_cast<uint8_t>(ctx->state[i] & 0xFF);
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
    static std::mt19937 engine(std::random_device{}());
    static std::uniform_int_distribution<uint32_t> distribution(0, std::numeric_limits<uint32_t>::max());
    return distribution(engine);
}

napi_value Md5Hash(napi_env env, napi_callback_info info)
{
    size_t argc = SINGLE_ARG_COUNT;
    napi_value argv[SINGLE_ARG_COUNT];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < SINGLE_ARG_COUNT) {
        napi_throw_error(env, nullptr, "Input string is required");
        return nullptr;
    }

    size_t strLength = 0;
    napi_get_value_string_utf8(env, argv[ARG_INDEX_0], nullptr, 0, &strLength);

    std::vector<char> buffer(strLength + 1);
    napi_get_value_string_utf8(env, argv[ARG_INDEX_0], buffer.data(), strLength + 1, &strLength);

    std::string hash = MD5Hash(std::string(buffer.data()));

    napi_value result;
    napi_create_string_utf8(env, hash.c_str(), hash.length(), &result);
    return result;
}

napi_value Sha256Hash(napi_env env, napi_callback_info info)
{
    size_t argc = SINGLE_ARG_COUNT;
    napi_value argv[SINGLE_ARG_COUNT];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < SINGLE_ARG_COUNT) {
        napi_throw_error(env, nullptr, "Input string is required");
        return nullptr;
    }

    size_t strLength = 0;
    napi_get_value_string_utf8(env, argv[ARG_INDEX_0], nullptr, 0, &strLength);

    std::vector<char> buffer(strLength + 1);
    napi_get_value_string_utf8(env, argv[ARG_INDEX_0], buffer.data(), strLength + 1, &strLength);

    std::string hash = SHA256Hash(std::string(buffer.data()));

    napi_value result;
    napi_create_string_utf8(env, hash.c_str(), hash.length(), &result);
    return result;
}

napi_value Base64Encode(napi_env env, napi_callback_info info)
{
    size_t argc = SINGLE_ARG_COUNT;
    napi_value argv[SINGLE_ARG_COUNT];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < SINGLE_ARG_COUNT) {
        napi_throw_error(env, nullptr, "Input string is required");
        return nullptr;
    }

    size_t strLength = 0;
    napi_get_value_string_utf8(env, argv[ARG_INDEX_0], nullptr, 0, &strLength);

    std::vector<char> buffer(strLength + 1);
    napi_get_value_string_utf8(env, argv[ARG_INDEX_0], buffer.data(), strLength + 1, &strLength);

    std::string encoded = Base64Encode(std::string(buffer.data()));

    napi_value result;
    napi_create_string_utf8(env, encoded.c_str(), encoded.length(), &result);
    return result;
}

napi_value Base64Decode(napi_env env, napi_callback_info info)
{
    size_t argc = SINGLE_ARG_COUNT;
    napi_value argv[SINGLE_ARG_COUNT];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < SINGLE_ARG_COUNT) {
        napi_throw_error(env, nullptr, "Input string is required");
        return nullptr;
    }

    size_t strLength = 0;
    napi_get_value_string_utf8(env, argv[ARG_INDEX_0], nullptr, 0, &strLength);

    std::vector<char> buffer(strLength + 1);
    napi_get_value_string_utf8(env, argv[ARG_INDEX_0], buffer.data(), strLength + 1, &strLength);

    std::string decoded = Base64Decode(std::string(buffer.data()));

    napi_value result;
    napi_create_string_utf8(env, decoded.c_str(), decoded.length(), &result);
    return result;
}

napi_value AesEncrypt(napi_env env, napi_callback_info info)
{
    size_t argc = TWO_ARG_COUNT;
    napi_value argv[TWO_ARG_COUNT];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < TWO_ARG_COUNT) {
        napi_throw_error(env, nullptr, "Plaintext and key are required");
        return nullptr;
    }

    size_t textLength = 0;
    napi_get_value_string_utf8(env, argv[ARG_INDEX_0], nullptr, 0, &textLength);
    std::vector<char> textBuffer(textLength + 1);
    napi_get_value_string_utf8(env, argv[ARG_INDEX_0], textBuffer.data(), textLength + 1, &textLength);

    size_t keyLength = 0;
    napi_get_value_string_utf8(env, argv[ARG_INDEX_1], nullptr, 0, &keyLength);
    std::vector<char> keyBuffer(keyLength + 1);
    napi_get_value_string_utf8(env, argv[ARG_INDEX_1], keyBuffer.data(), keyLength + 1, &keyLength);

    std::string encrypted = AESEncrypt(std::string(textBuffer.data()), std::string(keyBuffer.data()));

    napi_value result;
    napi_create_string_utf8(env, encrypted.c_str(), encrypted.length(), &result);
    return result;
}

napi_value AesDecrypt(napi_env env, napi_callback_info info)
{
    size_t argc = TWO_ARG_COUNT;
    napi_value argv[TWO_ARG_COUNT];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < TWO_ARG_COUNT) {
        napi_throw_error(env, nullptr, "Ciphertext and key are required");
        return nullptr;
    }

    size_t textLength = 0;
    napi_get_value_string_utf8(env, argv[ARG_INDEX_0], nullptr, 0, &textLength);
    std::vector<char> textBuffer(textLength + 1);
    napi_get_value_string_utf8(env, argv[ARG_INDEX_0], textBuffer.data(), textLength + 1, &textLength);

    size_t keyLength = 0;
    napi_get_value_string_utf8(env, argv[ARG_INDEX_1], nullptr, 0, &keyLength);
    std::vector<char> keyBuffer(keyLength + 1);
    napi_get_value_string_utf8(env, argv[ARG_INDEX_1], keyBuffer.data(), keyLength + 1, &keyLength);

    std::string decrypted = AESDecrypt(std::string(textBuffer.data()), std::string(keyBuffer.data()));

    napi_value result;
    napi_create_string_utf8(env, decrypted.c_str(), decrypted.length(), &result);
    return result;
}

napi_value GenerateRandom(napi_env env, napi_callback_info info)
{
    size_t argc = SINGLE_ARG_COUNT;
    napi_value argv[SINGLE_ARG_COUNT];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    int32_t min = 0;
    int32_t max = RAND_MAX;

    if (argc >= SINGLE_ARG_COUNT) {
        napi_valuetype type;
        napi_typeof(env, argv[ARG_INDEX_0], &type);

        if (type == napi_number) {
            napi_get_value_int32(env, argv[ARG_INDEX_0], &max);
        } else if (type == napi_object) {
            napi_value minVal;
            napi_value maxVal;
            napi_get_named_property(env, argv[ARG_INDEX_0], "min", &minVal);
            napi_get_named_property(env, argv[ARG_INDEX_0], "max", &maxVal);

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

extern "C" __attribute__((visibility("default"))) napi_value NAPI_Register(napi_env env, napi_value exports)
{
    return Init(env, exports);
}
