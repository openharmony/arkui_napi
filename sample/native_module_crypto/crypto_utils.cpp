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
#include <cstring>
#include <ctime>

namespace CryptoUtils {

static const char base64Chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char hexChars[] = "0123456789abcdef";

static int Base64CharValue(char c)
{
    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 26;
    }
    if (c >= '0' && c <= '9') {
        return c - '0' + 52;
    }
    if (c == '+') {
        return 62;
    }
    if (c == '/') {
        return 63;
    }
    return -1;
}

std::string Base64Encode(const std::string& input)
{
    std::string result;
    size_t i = 0;
    uint8_t byte3[3];
    size_t len = input.length();

    while (i < len) {
        byte3[0] = (i < len) ? static_cast<uint8_t>(input[i++]) : 0;
        byte3[1] = (i < len) ? static_cast<uint8_t>(input[i++]) : 0;
        byte3[2] = (i < len) ? static_cast<uint8_t>(input[i++]) : 0;

        uint32_t triple = (byte3[0] << 16) | (byte3[1] << 8) | byte3[2];

        result += base64Chars[(triple >> 18) & 0x3F];
        result += base64Chars[(triple >> 12) & 0x3F];
        result += base64Chars[(triple >> 6) & 0x3F];
        result += base64Chars[triple & 0x3F];
    }

    if (len % 3 >= 1) {
        result[result.length() - 1] = '=';
    }
    if (len % 3 == 1) {
        result[result.length() - 2] = '=';
    }

    return result;
}

std::string Base64Decode(const std::string& input)
{
    std::string result;
    size_t len = input.length();
    if (len == 0) {
        return result;
    }

    size_t padding = 0;
    if (len >= 1 && input[len - 1] == '=') {
        padding++;
    }
    if (len >= 2 && input[len - 2] == '=') {
        padding++;
    }

    for (size_t i = 0; i < len; i += 4) {
        int v0 = Base64CharValue(input[i]);
        int v1 = (i + 1 < len) ? Base64CharValue(input[i + 1]) : 0;
        int v2 = (i + 2 < len && input[i + 2] != '=') ? Base64CharValue(input[i + 2]) : 0;
        int v3 = (i + 3 < len && input[i + 3] != '=') ? Base64CharValue(input[i + 3]) : 0;

        if (v0 < 0 || v1 < 0 || v2 < 0 || v3 < 0) {
            continue;
        }

        uint32_t triple = (static_cast<uint32_t>(v0) << 18) |
                          (static_cast<uint32_t>(v1) << 12) |
                          (static_cast<uint32_t>(v2) << 6) |
                          static_cast<uint32_t>(v3);

        result += static_cast<char>((triple >> 16) & 0xFF);
        result += static_cast<char>((triple >> 8) & 0xFF);
        result += static_cast<char>(triple & 0xFF);
    }

    if (padding > 0) {
        result = result.substr(0, result.length() - padding);
    }

    return result;
}

std::string HexEncode(const std::string& input)
{
    std::string result;
    result.reserve(input.length() * 2);

    for (unsigned char c : input) {
        result += hexChars[(c >> 4) & 0x0F];
        result += hexChars[c & 0x0F];
    }

    return result;
}

static int HexCharValue(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

std::string HexDecode(const std::string& input)
{
    std::string result;
    if (input.length() % 2 != 0) {
        return result;
    }

    for (size_t i = 0; i < input.length(); i += 2) {
        int hi = HexCharValue(input[i]);
        int lo = HexCharValue(input[i + 1]);
        if (hi < 0 || lo < 0) {
            continue;
        }
        result += static_cast<char>((hi << 4) | lo);
    }

    return result;
}

std::string XorEncrypt(const std::string& input, const std::string& key)
{
    if (key.empty()) {
        return input;
    }

    std::string result;
    size_t keyLen = key.length();

    for (size_t i = 0; i < input.length(); i++) {
        result += static_cast<char>(input[i] ^ key[i % keyLen]);
    }

    return result;
}

std::string XorDecrypt(const std::string& input, const std::string& key)
{
    return XorEncrypt(input, key);
}

std::string SimpleHash(const std::string& input)
{
    uint32_t hash = 5381;

    for (char c : input) {
        hash = ((hash << 5) + hash) + static_cast<uint8_t>(c);
    }

    std::string result;
    for (int i = 28; i >= 0; i -= 4) {
        result += hexChars[(hash >> i) & 0xF];
    }

    return result;
}

std::string Rot13(const std::string& input)
{
    std::string result;

    for (char c : input) {
        if (c >= 'A' && c <= 'Z') {
            result += static_cast<char>('A' + (c - 'A' + 13) % 26);
        } else if (c >= 'a' && c <= 'z') {
            result += static_cast<char>('a' + (c - 'a' + 13) % 26);
        } else {
            result += c;
        }
    }

    return result;
}

std::string ReverseString(const std::string& input)
{
    return std::string(input.rbegin(), input.rend());
}

static uint32_t crc32Table[256];
static bool crc32TableInit = false;

static void InitCrc32Table()
{
    if (crc32TableInit) {
        return;
    }

    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
        crc32Table[i] = crc;
    }
    crc32TableInit = true;
}

uint32_t Crc32(const std::string& input)
{
    InitCrc32Table();

    uint32_t crc = 0xFFFFFFFF;

    for (char c : input) {
        uint8_t byte = static_cast<uint8_t>(c);
        crc = (crc >> 8) ^ crc32Table[(crc ^ byte) & 0xFF];
    }

    return crc ^ 0xFFFFFFFF;
}

std::string CaesarEncrypt(const std::string& input, int shift)
{
    shift = ((shift % 26) + 26) % 26;
    std::string result;

    for (char c : input) {
        if (c >= 'A' && c <= 'Z') {
            result += static_cast<char>('A' + (c - 'A' + shift) % 26);
        } else if (c >= 'a' && c <= 'z') {
            result += static_cast<char>('a' + (c - 'a' + shift) % 26);
        } else {
            result += c;
        }
    }

    return result;
}

std::string CaesarDecrypt(const std::string& input, int shift)
{
    return CaesarEncrypt(input, 26 - shift);
}

std::string VigenereEncrypt(const std::string& input, const std::string& key)
{
    if (key.empty()) {
        return input;
    }

    std::string normalizedKey;
    for (char c : key) {
        if (c >= 'A' && c <= 'Z') {
            normalizedKey += c;
        } else if (c >= 'a' && c <= 'z') {
            normalizedKey += static_cast<char>(c - 'a' + 'A');
        }
    }

    if (normalizedKey.empty()) {
        return input;
    }

    std::string result;
    size_t keyIndex = 0;

    for (char c : input) {
        if (c >= 'A' && c <= 'Z') {
            int shift = normalizedKey[keyIndex % normalizedKey.length()] - 'A';
            result += static_cast<char>('A' + (c - 'A' + shift) % 26);
            keyIndex++;
        } else if (c >= 'a' && c <= 'z') {
            int shift = normalizedKey[keyIndex % normalizedKey.length()] - 'A';
            result += static_cast<char>('a' + (c - 'a' + shift) % 26);
            keyIndex++;
        } else {
            result += c;
        }
    }

    return result;
}

std::string VigenereDecrypt(const std::string& input, const std::string& key)
{
    if (key.empty()) {
        return input;
    }

    std::string normalizedKey;
    for (char c : key) {
        if (c >= 'A' && c <= 'Z') {
            normalizedKey += c;
        } else if (c >= 'a' && c <= 'z') {
            normalizedKey += static_cast<char>(c - 'a' + 'A');
        }
    }

    if (normalizedKey.empty()) {
        return input;
    }

    std::string result;
    size_t keyIndex = 0;

    for (char c : input) {
        if (c >= 'A' && c <= 'Z') {
            int shift = normalizedKey[keyIndex % normalizedKey.length()] - 'A';
            result += static_cast<char>('A' + (c - 'A' - shift + 26) % 26);
            keyIndex++;
        } else if (c >= 'a' && c <= 'z') {
            int shift = normalizedKey[keyIndex % normalizedKey.length()] - 'A';
            result += static_cast<char>('a' + (c - 'a' - shift + 26) % 26);
            keyIndex++;
        } else {
            result += c;
        }
    }

    return result;
}

std::string RandomString(size_t length)
{
    static const char charset[] =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    std::string result;
    result.reserve(length);

    static uint32_t seed = static_cast<uint32_t>(std::time(nullptr));
    seed = seed * 1103515245 + 12345;

    for (size_t i = 0; i < length; i++) {
        seed = seed * 1103515245 + 12345;
        uint32_t randValue = (seed >> 16) & 0x7FFF;
        result += charset[randValue % (sizeof(charset) - 1)];
    }

    return result;
}

std::string PasswordHash(const std::string& password, const std::string& salt)
{
    std::string combined = password + salt;
    std::string hash1 = SimpleHash(combined);
    std::string hash2 = SimpleHash(hash1 + salt);
    std::string hash3 = SimpleHash(hash2 + password);
    return SimpleHash(hash3 + combined);
}

bool PasswordVerify(const std::string& password, const std::string& salt, const std::string& hash)
{
    return PasswordHash(password, salt) == hash;
}

std::string SimpleMd5(const std::string& input)
{
    uint32_t a0 = 0x67452301;
    uint32_t b0 = 0xefcdab89;
    uint32_t c0 = 0x98badcfe;
    uint32_t d0 = 0x10325476;

    std::string message = input;
    size_t origLen = message.length();

    message += static_cast<char>(0x80);
    while ((message.length() % 64) != 56) {
        message += static_cast<char>(0x00);
    }

    uint64_t bitLen = origLen * 8;
    for (int i = 0; i < 8; i++) {
        message += static_cast<char>((bitLen >> (i * 8)) & 0xFF);
    }

    static const uint32_t k[16] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821
    };

    for (size_t chunk = 0; chunk < message.length(); chunk += 64) {
        uint32_t M[16];
        for (int i = 0; i < 16; i++) {
            M[i] = static_cast<uint8_t>(message[chunk + i * 4]) |
                   (static_cast<uint8_t>(message[chunk + i * 4 + 1]) << 8) |
                   (static_cast<uint8_t>(message[chunk + i * 4 + 2]) << 16) |
                   (static_cast<uint8_t>(message[chunk + i * 4 + 3]) << 24);
        }

        uint32_t A = a0;
        uint32_t B = b0;
        uint32_t C = c0;
        uint32_t D = d0;

        for (int i = 0; i < 16; i++) {
            uint32_t F = (B & C) | ((~B) & D);
            F = F + A + k[i] + M[i];
            A = D;
            D = C;
            C = B;
            B = B + ((F << 7) | (F >> 25));
        }

        a0 += A;
        b0 += B;
        c0 += C;
        d0 += D;
    }

    std::string result;
    for (int i = 0; i < 4; i++) {
        result += hexChars[(a0 >> (i * 8 + 4)) & 0xF];
        result += hexChars[(a0 >> (i * 8)) & 0xF];
    }
    for (int i = 0; i < 4; i++) {
        result += hexChars[(b0 >> (i * 8 + 4)) & 0xF];
        result += hexChars[(b0 >> (i * 8)) & 0xF];
    }
    for (int i = 0; i < 4; i++) {
        result += hexChars[(c0 >> (i * 8 + 4)) & 0xF];
        result += hexChars[(c0 >> (i * 8)) & 0xF];
    }
    for (int i = 0; i < 4; i++) {
        result += hexChars[(d0 >> (i * 8 + 4)) & 0xF];
        result += hexChars[(d0 >> (i * 8)) & 0xF];
    }

    return result;
}

}