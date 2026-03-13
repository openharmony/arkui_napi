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

static const int BASE64_UPPERCASE_OFFSET = 26;
static const int BASE64_DIGIT_OFFSET = 52;
static const int BASE64_PLUS_VALUE = 62;
static const int BASE64_SLASH_VALUE = 63;
static const int BASE64_GROUP_SIZE = 3;
static const int BASE64_OUTPUT_SIZE = 4;
static const int BASE64_SHIFT_18 = 18;
static const int BASE64_SHIFT_16 = 16;
static const int BASE64_SHIFT_12 = 12;
static const int BASE64_SHIFT_8 = 8;
static const int BASE64_SHIFT_6 = 6;
static const uint32_t BASE64_MASK_3F = 0x3F;
static const uint32_t BASE64_MASK_FF = 0xFF;
static const int HEX_CHARS_PER_BYTE = 2;
static const int HEX_SHIFT_4 = 4;
static const int HEX_MASK_0F = 0x0F;
static const int HEX_VALUE_OFFSET = 10;
static const int INDEX_OFFSET_1 = 1;
static const int INDEX_OFFSET_2 = 2;
static const int INDEX_OFFSET_3 = 3;
static const int DJB2_SHIFT_BITS = 5;
static const int MD5_WORD_SIZE = 4;
static const int MD5_BYTE_SHIFT_8 = 8;
static const int MD5_BYTE_SHIFT_16 = 16;
static const int MD5_BYTE_SHIFT_24 = 24;
static const int HEX_MASK_NIBBLE = 0xF;
static const uint32_t DJB2_INIT_HASH = 5381;
static const int DJB2_HASH_BITS = 28;
static const int DJB2_NIBBLE_BITS = 4;
static const int ALPHABET_SIZE = 26;
static const int ROT13_SHIFT = 13;
static const int CRC32_TABLE_SIZE = 256;
static const int CRC32_BITS_PER_BYTE = 8;
static const uint32_t CRC32_POLYNOMIAL = 0xEDB88320;
static const uint32_t CRC32_INIT_VALUE = 0xFFFFFFFF;
static const uint32_t RAND_MULTIPLIER = 1103515245;
static const uint32_t RAND_INCREMENT = 12345;
static const int RAND_SHIFT_16 = 16;
static const uint32_t RAND_MASK_7FFF = 0x7FFF;
static const int MD5_BLOCK_SIZE = 64;
static const int MD5_LENGTH_OFFSET = 56;
static const int MD5_WORD_COUNT = 16;
static const int MD5_ROTATE_LEFT_7 = 7;
static const int MD5_ROTATE_RIGHT_25 = 25;

static const char base64Chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char hexChars[] = "0123456789abcdef";

static int Base64CharValue(char c)
{
    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + BASE64_UPPERCASE_OFFSET;
    }
    if (c >= '0' && c <= '9') {
        return c - '0' + BASE64_DIGIT_OFFSET;
    }
    if (c == '+') {
        return BASE64_PLUS_VALUE;
    }
    if (c == '/') {
        return BASE64_SLASH_VALUE;
    }
    return -1;
}

std::string Base64Encode(const std::string& input)
{
    std::string result;
    size_t i = 0;
    uint8_t byte3[BASE64_GROUP_SIZE];
    size_t len = input.length();

    while (i < len) {
        byte3[0] = (i < len) ? static_cast<uint8_t>(input[i++]) : 0;
        byte3[1] = (i < len) ? static_cast<uint8_t>(input[i++]) : 0;
        byte3[2] = (i < len) ? static_cast<uint8_t>(input[i++]) : 0;

        uint32_t triple = (byte3[0] << BASE64_SHIFT_16) | (byte3[1] << BASE64_SHIFT_8) | byte3[2];

        result += base64Chars[(triple >> BASE64_SHIFT_18) & BASE64_MASK_3F];
        result += base64Chars[(triple >> BASE64_SHIFT_12) & BASE64_MASK_3F];
        result += base64Chars[(triple >> BASE64_SHIFT_6) & BASE64_MASK_3F];
        result += base64Chars[triple & BASE64_MASK_3F];
    }

    if (len % BASE64_GROUP_SIZE >= 1) {
        result[result.length() - INDEX_OFFSET_1] = '=';
    }
    if (len % BASE64_GROUP_SIZE == 1) {
        result[result.length() - INDEX_OFFSET_2] = '=';
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
    if (len >= INDEX_OFFSET_1 && input[len - INDEX_OFFSET_1] == '=') {
        padding++;
    }
    if (len >= INDEX_OFFSET_2 && input[len - INDEX_OFFSET_2] == '=') {
        padding++;
    }

    for (size_t i = 0; i < len; i += BASE64_OUTPUT_SIZE) {
        int v0 = Base64CharValue(input[i]);
        int v1 = (i + INDEX_OFFSET_1 < len) ? Base64CharValue(input[i + INDEX_OFFSET_1]) : 0;
        int v2 = (i + INDEX_OFFSET_2 < len && input[i + INDEX_OFFSET_2] != '=') ? Base64CharValue(input[i + INDEX_OFFSET_2]) : 0;
        int v3 = (i + INDEX_OFFSET_3 < len && input[i + INDEX_OFFSET_3] != '=') ? Base64CharValue(input[i + INDEX_OFFSET_3]) : 0;

        if (v0 < 0 || v1 < 0 || v2 < 0 || v3 < 0) {
            continue;
        }

        uint32_t triple = (static_cast<uint32_t>(v0) << BASE64_SHIFT_18) |
                          (static_cast<uint32_t>(v1) << BASE64_SHIFT_12) |
                          (static_cast<uint32_t>(v2) << BASE64_SHIFT_6) |
                          static_cast<uint32_t>(v3);

        result += static_cast<char>((triple >> BASE64_SHIFT_16) & BASE64_MASK_FF);
        result += static_cast<char>((triple >> BASE64_SHIFT_8) & BASE64_MASK_FF);
        result += static_cast<char>(triple & BASE64_MASK_FF);
    }

    if (padding > 0) {
        result = result.substr(0, result.length() - padding);
    }

    return result;
}

std::string HexEncode(const std::string& input)
{
    std::string result;
    result.reserve(input.length() * HEX_CHARS_PER_BYTE);

    for (unsigned char c : input) {
        result += hexChars[(c >> HEX_SHIFT_4) & HEX_MASK_0F];
        result += hexChars[c & HEX_MASK_0F];
    }

    return result;
}

static int HexCharValue(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + HEX_VALUE_OFFSET;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + HEX_VALUE_OFFSET;
    }
    return -1;
}

std::string HexDecode(const std::string& input)
{
    std::string result;
    if (input.length() % HEX_CHARS_PER_BYTE != 0) {
        return result;
    }

    for (size_t i = 0; i < input.length(); i += HEX_CHARS_PER_BYTE) {
        int hi = HexCharValue(input[i]);
        int lo = HexCharValue(input[i + INDEX_OFFSET_1]);
        if (hi < 0 || lo < 0) {
            continue;
        }
        result += static_cast<char>((hi << HEX_SHIFT_4) | lo);
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
    uint32_t hash = DJB2_INIT_HASH;

    for (char c : input) {
        hash = ((hash << DJB2_SHIFT_BITS) + hash) + static_cast<uint8_t>(c);
    }

    std::string result;
    for (int i = DJB2_HASH_BITS; i >= 0; i -= DJB2_NIBBLE_BITS) {
        result += hexChars[(hash >> i) & HEX_MASK_NIBBLE];
    }

    return result;
}

std::string Rot13(const std::string& input)
{
    std::string result;

    for (char c : input) {
        if (c >= 'A' && c <= 'Z') {
            result += static_cast<char>('A' + (c - 'A' + ROT13_SHIFT) % ALPHABET_SIZE);
        } else if (c >= 'a' && c <= 'z') {
            result += static_cast<char>('a' + (c - 'a' + ROT13_SHIFT) % ALPHABET_SIZE);
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

static uint32_t g_crc32Table[CRC32_TABLE_SIZE];
static bool crc32TableInit = false;

static void InitCrc32Table()
{
    if (crc32TableInit) {
        return;
    }

    for (uint32_t i = 0; i < CRC32_TABLE_SIZE; i++) {
        uint32_t crc = i;
        for (int j = 0; j < CRC32_BITS_PER_BYTE; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
            } else {
                crc >>= 1;
            }
        }
        g_crc32Table[i] = crc;
    }
    crc32TableInit = true;
}

uint32_t Crc32(const std::string& input)
{
    InitCrc32Table();

    uint32_t crc = CRC32_INIT_VALUE;

    for (char c : input) {
        uint8_t byte = static_cast<uint8_t>(c);
        crc = (crc >> CRC32_BITS_PER_BYTE) ^ g_crc32Table[(crc ^ byte) & BASE64_MASK_FF];
    }

    return crc ^ CRC32_INIT_VALUE;
}

std::string CaesarEncrypt(const std::string& input, int shift)
{
    shift = ((shift % ALPHABET_SIZE) + ALPHABET_SIZE) % ALPHABET_SIZE;
    std::string result;

    for (char c : input) {
        if (c >= 'A' && c <= 'Z') {
            result += static_cast<char>('A' + (c - 'A' + shift) % ALPHABET_SIZE);
        } else if (c >= 'a' && c <= 'z') {
            result += static_cast<char>('a' + (c - 'a' + shift) % ALPHABET_SIZE);
        } else {
            result += c;
        }
    }

    return result;
}

std::string CaesarDecrypt(const std::string& input, int shift)
{
    return CaesarEncrypt(input, ALPHABET_SIZE - shift);
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
            result += static_cast<char>('A' + (c - 'A' + shift) % ALPHABET_SIZE);
            keyIndex++;
        } else if (c >= 'a' && c <= 'z') {
            int shift = normalizedKey[keyIndex % normalizedKey.length()] - 'A';
            result += static_cast<char>('a' + (c - 'a' + shift) % ALPHABET_SIZE);
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
            result += static_cast<char>('A' + (c - 'A' - shift + ALPHABET_SIZE) % ALPHABET_SIZE);
            keyIndex++;
        } else if (c >= 'a' && c <= 'z') {
            int shift = normalizedKey[keyIndex % normalizedKey.length()] - 'A';
            result += static_cast<char>('a' + (c - 'a' - shift + ALPHABET_SIZE) % ALPHABET_SIZE);
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
    seed = seed * RAND_MULTIPLIER + RAND_INCREMENT;

    for (size_t i = 0; i < length; i++) {
        seed = seed * RAND_MULTIPLIER + RAND_INCREMENT;
        uint32_t randValue = (seed >> RAND_SHIFT_16) & RAND_MASK_7FFF;
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

static std::string Md5PadMessage(const std::string& input, size_t& origLen)
{
    std::string message = input;
    origLen = message.length();

    message += static_cast<char>(0x80);
    while ((message.length() % MD5_BLOCK_SIZE) != MD5_LENGTH_OFFSET) {
        message += static_cast<char>(0x00);
    }

    uint64_t bitLen = origLen * CRC32_BITS_PER_BYTE;
    for (int i = 0; i < CRC32_BITS_PER_BYTE; i++) {
        message += static_cast<char>((bitLen >> (i * CRC32_BITS_PER_BYTE)) & BASE64_MASK_FF);
    }

    return message;
}

static void Md5ProcessBlock(const std::string& message, size_t chunk, uint32_t& a0, uint32_t& b0,
                            uint32_t& c0, uint32_t& d0, const uint32_t* k)
{
    uint32_t M[MD5_WORD_COUNT];
    for (int i = 0; i < MD5_WORD_COUNT; i++) {
        M[i] = static_cast<uint8_t>(message[chunk + i * MD5_WORD_SIZE]) |
               (static_cast<uint8_t>(message[chunk + i * MD5_WORD_SIZE + INDEX_OFFSET_1]) << MD5_BYTE_SHIFT_8) |
               (static_cast<uint8_t>(message[chunk + i * MD5_WORD_SIZE + INDEX_OFFSET_2]) << MD5_BYTE_SHIFT_16) |
               (static_cast<uint8_t>(message[chunk + i * MD5_WORD_SIZE + INDEX_OFFSET_3]) << MD5_BYTE_SHIFT_24);
    }

    uint32_t a = a0;
    uint32_t b = b0;
    uint32_t c = c0;
    uint32_t d = d0;

    for (int i = 0; i < MD5_WORD_COUNT; i++) {
        uint32_t f = (b & c) | ((~b) & d);
        f = f + a + k[i] + M[i];
        a = d;
        d = c;
        c = b;
        b = b + ((f << MD5_ROTATE_LEFT_7) | (f >> MD5_ROTATE_RIGHT_25));
    }

    a0 += a;
    b0 += b;
    c0 += c;
    d0 += d;
}

static std::string Md5ToHex(uint32_t a0, uint32_t b0, uint32_t c0, uint32_t d0)
{
    std::string result;
    uint32_t values[MD5_WORD_SIZE] = {a0, b0, c0, d0};

    for (int j = 0; j < MD5_WORD_SIZE; j++) {
        for (int i = 0; i < HEX_CHARS_PER_BYTE; i++) {
            result += hexChars[(values[j] >> (i * CRC32_BITS_PER_BYTE + HEX_SHIFT_4)) & HEX_MASK_NIBBLE];
            result += hexChars[(values[j] >> (i * CRC32_BITS_PER_BYTE)) & HEX_MASK_NIBBLE];
        }
    }

    return result;
}

std::string SimpleMd5(const std::string& input)
{
    uint32_t a0 = 0x67452301;
    uint32_t b0 = 0xefcdab89;
    uint32_t c0 = 0x98badcfe;
    uint32_t d0 = 0x10325476;

    size_t origLen = 0;
    std::string message = Md5PadMessage(input, origLen);

    static const uint32_t k[MD5_WORD_COUNT] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821
    };

    for (size_t chunk = 0; chunk < message.length(); chunk += MD5_BLOCK_SIZE) {
        Md5ProcessBlock(message, chunk, a0, b0, c0, d0, k);
    }

    return Md5ToHex(a0, b0, c0, d0);
}

}