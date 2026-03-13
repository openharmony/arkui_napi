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

#include "js_encoding_helper.h"

#include "securec.h"

using namespace EncodingBufferSize;

/***********************************************
 * Base64 Implementation
 ***********************************************/
const char Base64Helper::encodeTable[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const int Base64Helper::decodeTable[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1
};

std::string Base64Helper::Encode(const std::string& input)
{
    return EncodeBytes(std::vector<uint8_t>(input.begin(), input.end()));
}

std::string Base64Helper::EncodeBytes(const std::vector<uint8_t>& input)
{
    std::string result;
    size_t len = input.size();
    result.reserve((len + Base64Const::SECOND_BYTE_OFFSET) / Base64Const::GROUP_SIZE *
        Base64Const::ENCODED_GROUP_SIZE);
    for (size_t i = 0; i < len; i += Base64Const::GROUP_SIZE) {
        uint32_t triple = static_cast<uint32_t>(input[i]) << Base64Const::SHIFT_HIGH;
        if (i + 1 < len) {
            triple |= static_cast<uint32_t>(input[i + 1]) << Base64Const::SHIFT_MID;
        }
        if (i + Base64Const::SECOND_BYTE_OFFSET < len) {
            triple |= static_cast<uint32_t>(input[i + Base64Const::SECOND_BYTE_OFFSET]);
        }
        result += encodeTable[(triple >> Base64Const::SHIFT_DECODE_A) & 0x3F];
        result += encodeTable[(triple >> Base64Const::SHIFT_DECODE_B) & 0x3F];
        result += (i + 1 < len) ? encodeTable[(triple >> Base64Const::SHIFT_DECODE_C) & 0x3F] : padding;
        result += (i + Base64Const::SECOND_BYTE_OFFSET < len) ? encodeTable[triple & 0x3F] : padding;
    }
    return result;
}

std::string Base64Helper::Decode(const std::string& input)
{
    std::vector<uint8_t> bytes = DecodeToBytes(input);
    return std::string(bytes.begin(), bytes.end());
}

std::vector<uint8_t> Base64Helper::DecodeToBytes(const std::string& input)
{
    std::vector<uint8_t> result;
    if (input.length() % Base64Const::ENCODED_GROUP_SIZE != 0) {
        return result;
    }
    size_t len = input.length();
    size_t outLen = len / Base64Const::ENCODED_GROUP_SIZE * Base64Const::GROUP_SIZE;
    if (len > 0 && input[len - 1] == padding) {
        outLen--;
    }
    if (len > 1 && input[len - Base64Const::SECOND_BYTE_OFFSET] == padding) {
        outLen--;
    }
    result.reserve(outLen);
    for (size_t i = 0; i < len; i += Base64Const::ENCODED_GROUP_SIZE) {
        int a = (input[i] < Base64Const::TABLE_SIZE) ?
            decodeTable[static_cast<unsigned char>(input[i])] : -1;
        int b = (input[i + 1] < Base64Const::TABLE_SIZE) ?
            decodeTable[static_cast<unsigned char>(input[i + 1])] : -1;
        int c = (input[i + Base64Const::SECOND_BYTE_OFFSET] < Base64Const::TABLE_SIZE) ?
            decodeTable[static_cast<unsigned char>(input[i + Base64Const::SECOND_BYTE_OFFSET])] : -1;
        int d = (input[i + Base64Const::THIRD_BYTE_OFFSET] < Base64Const::TABLE_SIZE) ?
            decodeTable[static_cast<unsigned char>(input[i + Base64Const::THIRD_BYTE_OFFSET])] : -1;
        if (a < 0 || b < 0) {
            break;
        }
        uint32_t triple = (static_cast<uint32_t>(a) << Base64Const::SHIFT_DECODE_A) |
            (static_cast<uint32_t>(b) << Base64Const::SHIFT_DECODE_B);
        if (c >= 0) {
            triple |= static_cast<uint32_t>(c) << Base64Const::SHIFT_DECODE_C;
        }
        if (d >= 0) {
            triple |= static_cast<uint32_t>(d);
        }
        result.push_back(static_cast<uint8_t>((triple >> Base64Const::SHIFT_HIGH) & 0xFF));
        if (input[i + Base64Const::SECOND_BYTE_OFFSET] != padding) {
            result.push_back(static_cast<uint8_t>((triple >> Base64Const::SHIFT_MID) & 0xFF));
        }
        if (input[i + Base64Const::THIRD_BYTE_OFFSET] != padding) {
            result.push_back(static_cast<uint8_t>(triple & 0xFF));
        }
    }
    return result;
}

/***********************************************
 * Hex Implementation
 ***********************************************/
const char HexHelper::hexChars[] = "0123456789abcdef";

int HexHelper::CharToNibble(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + HexConst::ALPHA_OFFSET;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + HexConst::ALPHA_OFFSET;
    }
    return -1;
}

std::string HexHelper::Encode(const std::string& input)
{
    return EncodeBytes(std::vector<uint8_t>(input.begin(), input.end()));
}

std::string HexHelper::EncodeBytes(const std::vector<uint8_t>& input)
{
    std::string result;
    result.reserve(input.size() * HexConst::HEX_PAIR);
    for (uint8_t byte : input) {
        result += hexChars[(byte >> HexConst::NIBBLE_SHIFT) & 0x0F];
        result += hexChars[byte & 0x0F];
    }
    return result;
}

std::string HexHelper::Decode(const std::string& input)
{
    std::vector<uint8_t> bytes = DecodeToBytes(input);
    return std::string(bytes.begin(), bytes.end());
}

std::vector<uint8_t> HexHelper::DecodeToBytes(const std::string& input)
{
    std::vector<uint8_t> result;
    if (input.length() % HexConst::HEX_PAIR != 0) {
        return result;
    }
    result.reserve(input.length() / HexConst::HEX_PAIR);
    for (size_t i = 0; i < input.length(); i += HexConst::HEX_PAIR) {
        int high = CharToNibble(input[i]);
        int low = CharToNibble(input[i + 1]);
        if (high < 0 || low < 0) {
            break;
        }
        result.push_back(static_cast<uint8_t>((high << HexConst::NIBBLE_SHIFT) | low));
    }
    return result;
}

bool HexHelper::IsValidHex(const std::string& input)
{
    if (input.length() % HexConst::HEX_PAIR != 0) {
        return false;
    }
    for (char c : input) {
        if (CharToNibble(c) < 0) {
            return false;
        }
    }
    return true;
}

/***********************************************
 * Hash Implementation
 ***********************************************/
uint32_t HashHelper::SimpleHash(const std::string& input)
{
    uint32_t hash = 0;
    for (char c : input) {
        hash = hash * HashConst::SIMPLE_MULTIPLIER + static_cast<uint8_t>(c);
    }
    return hash;
}

uint32_t HashHelper::DJB2Hash(const std::string& input)
{
    uint32_t hash = HashConst::DJB2_INIT;
    for (char c : input) {
        hash = ((hash << HashConst::DJB2_SHIFT) + hash) + static_cast<uint8_t>(c);
    }
    return hash;
}

uint32_t HashHelper::FNV1aHash(const std::string& input)
{
    uint32_t hash = HashConst::FNV1A_INIT;
    for (char c : input) {
        hash ^= static_cast<uint8_t>(c);
        hash *= HashConst::FNV1A_PRIME;
    }
    return hash;
}

uint32_t HashHelper::CRC32(const std::string& input)
{
    uint32_t crc = HashConst::CRC_INIT;
    for (char c : input) {
        crc ^= static_cast<uint8_t>(c);
        for (int j = 0; j < HashConst::CRC_BITS; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ HashConst::CRC_POLY;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc ^ HashConst::CRC_INIT;
}

std::string HashHelper::HashToHex(uint32_t hash)
{
    char buf[HashConst::HASH_HEX_BUF_SIZE] = { 0 };
    if (snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "%08x", hash) < 0) {
        return "00000000";
    }
    return std::string(buf);
}

/***********************************************
 * Checksum Implementation
 ***********************************************/
uint8_t ChecksumHelper::XORChecksum(const std::string& input)
{
    uint8_t checksum = 0;
    for (char c : input) {
        checksum ^= static_cast<uint8_t>(c);
    }
    return checksum;
}

uint16_t ChecksumHelper::Fletcher16(const std::string& input)
{
    uint16_t sum1 = 0;
    uint16_t sum2 = 0;
    for (char c : input) {
        sum1 = (sum1 + static_cast<uint8_t>(c)) % ChecksumConst::FLETCHER_MOD;
        sum2 = (sum2 + sum1) % ChecksumConst::FLETCHER_MOD;
    }
    return (sum2 << ChecksumConst::FLETCHER_SHIFT) | sum1;
}

uint32_t ChecksumHelper::Adler32(const std::string& input)
{
    uint32_t a = 1;
    uint32_t b = 0;
    for (char c : input) {
        a = (a + static_cast<uint8_t>(c)) % ChecksumConst::ADLER_MOD;
        b = (b + a) % ChecksumConst::ADLER_MOD;
    }
    return (b << ChecksumConst::ADLER_SHIFT) | a;
}

/***********************************************
 * ROT13 Implementation
 ***********************************************/
std::string ROT13Helper::Transform(const std::string& input)
{
    std::string result = input;
    for (char& c : result) {
        if (c >= 'a' && c <= 'z') {
            c = 'a' + (c - 'a' + CipherConst::ROT13_SHIFT) % CipherConst::ALPHABET_SIZE;
        } else if (c >= 'A' && c <= 'Z') {
            c = 'A' + (c - 'A' + CipherConst::ROT13_SHIFT) % CipherConst::ALPHABET_SIZE;
        }
    }
    return result;
}

/***********************************************
 * URL Encoding Implementation
 ***********************************************/
bool URLEncodingHelper::ShouldEncode(char c)
{
    if (c >= 'A' && c <= 'Z') {
        return false;
    }
    if (c >= 'a' && c <= 'z') {
        return false;
    }
    if (c >= '0' && c <= '9') {
        return false;
    }
    if (c == '-' || c == '_' || c == '.' || c == '~') {
        return false;
    }
    return true;
}

std::string URLEncodingHelper::Encode(const std::string& input)
{
    std::string result;
    result.reserve(input.size() * URLConst::RESERVE_MULTIPLIER);
    for (char c : input) {
        if (ShouldEncode(c)) {
            char buf[URLConst::URL_BUF_SIZE] = { 0 };
            if (snprintf_s(buf, sizeof(buf), sizeof(buf) - 1, "%%%02X",
                static_cast<unsigned char>(c)) >= 0) {
                result += buf;
            }
        } else {
            result += c;
        }
    }
    return result;
}

std::string URLEncodingHelper::Decode(const std::string& input)
{
    std::string result;
    result.reserve(input.size());
    size_t i = 0;
    while (i < input.size()) {
        if (input[i] == '%' && i + URLConst::PERCENT_SKIP < input.size()) {
            int high = HexHelper::IsValidHex(input.substr(i + 1, URLConst::PERCENT_SKIP)) ?
                ((input[i + 1] >= '0' && input[i + 1] <= '9') ? input[i + 1] - '0' :
                 (input[i + 1] >= 'a' && input[i + 1] <= 'f') ?
                     input[i + 1] - 'a' + HexConst::ALPHA_OFFSET :
                 (input[i + 1] >= 'A' && input[i + 1] <= 'F') ?
                     input[i + 1] - 'A' + HexConst::ALPHA_OFFSET : -1)
                : -1;
            int low = (high >= 0) ?
                ((input[i + 2] >= '0' && input[i + 2] <= '9') ? input[i + 2] - '0' :
                 (input[i + 2] >= 'a' && input[i + 2] <= 'f') ?
                     input[i + 2] - 'a' + HexConst::ALPHA_OFFSET :
                 (input[i + 2] >= 'A' && input[i + 2] <= 'F') ?
                     input[i + 2] - 'A' + HexConst::ALPHA_OFFSET : -1)
                : -1;
            if (high >= 0 && low >= 0) {
                result += static_cast<char>((high << HexConst::NIBBLE_SHIFT) | low);
                i += URLConst::PERCENT_SKIP + 1;
                continue;
            } else {
                result += input[i];
            }
        } else if (input[i] == '+') {
            result += ' ';
        } else {
            result += input[i];
        }
        i++;
    }
    return result;
}

/***********************************************
 * NAPI Functions
 ***********************************************/
static napi_value JSBase64Encode(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);
    std::string result = Base64Helper::Encode(std::string(input, inputLen));
    napi_value output = nullptr;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

static napi_value JSBase64Decode(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);
    std::string result = Base64Helper::Decode(std::string(input, inputLen));
    napi_value output = nullptr;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

static napi_value JSHexEncode(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);
    std::string result = HexHelper::Encode(std::string(input, inputLen));
    napi_value output = nullptr;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

static napi_value JSHexDecode(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);
    std::string result = HexHelper::Decode(std::string(input, inputLen));
    napi_value output = nullptr;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

static napi_value JSIsValidHex(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);
    napi_value result = nullptr;
    napi_get_boolean(env, HexHelper::IsValidHex(std::string(input, inputLen)), &result);
    return result;
}

static napi_value JSSimpleHash(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);
    std::string hex = HashHelper::HashToHex(HashHelper::SimpleHash(std::string(input, inputLen)));
    napi_value output = nullptr;
    napi_create_string_utf8(env, hex.c_str(), hex.length(), &output);
    return output;
}

static napi_value JSDJB2Hash(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);
    std::string hex = HashHelper::HashToHex(HashHelper::DJB2Hash(std::string(input, inputLen)));
    napi_value output = nullptr;
    napi_create_string_utf8(env, hex.c_str(), hex.length(), &output);
    return output;
}

static napi_value JSFNV1aHash(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);
    std::string hex = HashHelper::HashToHex(HashHelper::FNV1aHash(std::string(input, inputLen)));
    napi_value output = nullptr;
    napi_create_string_utf8(env, hex.c_str(), hex.length(), &output);
    return output;
}

static napi_value JSCRC32(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);
    std::string hex = HashHelper::HashToHex(HashHelper::CRC32(std::string(input, inputLen)));
    napi_value output = nullptr;
    napi_create_string_utf8(env, hex.c_str(), hex.length(), &output);
    return output;
}

static napi_value JSAdler32(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);
    std::string hex = HashHelper::HashToHex(ChecksumHelper::Adler32(std::string(input, inputLen)));
    napi_value output = nullptr;
    napi_create_string_utf8(env, hex.c_str(), hex.length(), &output);
    return output;
}

static napi_value JSROT13(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);
    std::string result = ROT13Helper::Transform(std::string(input, inputLen));
    napi_value output = nullptr;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

static napi_value JSURLEncode(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);
    std::string result = URLEncodingHelper::Encode(std::string(input, inputLen));
    napi_value output = nullptr;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

static napi_value JSFletcher16(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);
    napi_value output = nullptr;
    napi_create_int32(env, ChecksumHelper::Fletcher16(std::string(input, inputLen)), &output);
    return output;
}

static napi_value JSXORChecksum(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);
    napi_value output = nullptr;
    napi_create_int32(env, ChecksumHelper::XORChecksum(std::string(input, inputLen)), &output);
    return output;
}

// caesarCipher(input: string, shift: number): string
static napi_value JSCaesarCipher(napi_env env, napi_callback_info info)
{
    size_t argc = CryptoArgCount::TWO;
    napi_value argv[CryptoArgCount::TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= CryptoArgCount::TWO, "requires 2 parameters");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype0));
    NAPI_ASSERT(env, valuetype0 == napi_string, "first argument must be string");

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype1));
    NAPI_ASSERT(env, valuetype1 == napi_number, "second argument must be number");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    int32_t shift = 0;
    napi_get_value_int32(env, argv[1], &shift);
    shift = ((shift % CipherConst::ALPHABET_SIZE) + CipherConst::ALPHABET_SIZE) %
        CipherConst::ALPHABET_SIZE;

    std::string result(input, inputLen);
    for (char& c : result) {
        if (c >= 'a' && c <= 'z') {
            c = 'a' + (c - 'a' + shift) % CipherConst::ALPHABET_SIZE;
        } else if (c >= 'A' && c <= 'Z') {
            c = 'A' + (c - 'A' + shift) % CipherConst::ALPHABET_SIZE;
        }
    }

    napi_value output = nullptr;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

// reverseBytes(input: string): string
static napi_value JSReverseBytes(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    std::string hex = HexHelper::Encode(std::string(input, inputLen));
    std::string reversed;
    reversed.reserve(hex.size());
    for (int i = static_cast<int>(hex.size()) - HexConst::HEX_PAIR; i >= 0; i -= HexConst::HEX_PAIR) {
        reversed += hex[i];
        reversed += hex[i + 1];
    }

    napi_value output = nullptr;
    napi_create_string_utf8(env, reversed.c_str(), reversed.length(), &output);
    return output;
}

// byteLength(input: string): number
static napi_value JSByteLength(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    napi_value output = nullptr;
    napi_create_int32(env, static_cast<int32_t>(inputLen), &output);
    return output;
}

// toCharCodes(input: string): number[]
static napi_value JSToCharCodes(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");

    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);

    napi_value result = nullptr;
    napi_create_array_with_length(env, inputLen, &result);
    for (size_t i = 0; i < inputLen; i++) {
        napi_value code = nullptr;
        napi_create_int32(env, static_cast<int32_t>(static_cast<uint8_t>(input[i])), &code);
        napi_set_element(env, result, i, code);
    }
    return result;
}

static napi_value JSURLDecode(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "requires 1 parameter");
    char input[INPUT_BUFFER_SIZE] = { 0 };
    size_t inputLen = 0;
    napi_get_value_string_utf8(env, argv[0], input, INPUT_BUFFER_SIZE, &inputLen);
    std::string result = URLEncodingHelper::Decode(std::string(input, inputLen));
    napi_value output = nullptr;
    napi_create_string_utf8(env, result.c_str(), result.length(), &output);
    return output;
}

/***********************************************
 * Module export and register
 ***********************************************/
static napi_value EncodingExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("base64Encode", JSBase64Encode),
        DECLARE_NAPI_FUNCTION("base64Decode", JSBase64Decode),
        DECLARE_NAPI_FUNCTION("hexEncode", JSHexEncode),
        DECLARE_NAPI_FUNCTION("hexDecode", JSHexDecode),
        DECLARE_NAPI_FUNCTION("isValidHex", JSIsValidHex),
        DECLARE_NAPI_FUNCTION("simpleHash", JSSimpleHash),
        DECLARE_NAPI_FUNCTION("djb2Hash", JSDJB2Hash),
        DECLARE_NAPI_FUNCTION("fnv1aHash", JSFNV1aHash),
        DECLARE_NAPI_FUNCTION("crc32", JSCRC32),
        DECLARE_NAPI_FUNCTION("adler32", JSAdler32),
        DECLARE_NAPI_FUNCTION("rot13", JSROT13),
        DECLARE_NAPI_FUNCTION("urlEncode", JSURLEncode),
        DECLARE_NAPI_FUNCTION("urlDecode", JSURLDecode),
        DECLARE_NAPI_FUNCTION("fletcher16", JSFletcher16),
        DECLARE_NAPI_FUNCTION("xorChecksum", JSXORChecksum),
        DECLARE_NAPI_FUNCTION("caesarCipher", JSCaesarCipher),
        DECLARE_NAPI_FUNCTION("reverseBytes", JSReverseBytes),
        DECLARE_NAPI_FUNCTION("byteLength", JSByteLength),
        DECLARE_NAPI_FUNCTION("toCharCodes", JSToCharCodes),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module g_encodingModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = EncodingExport,
    .nm_modname = "encoding",
    .nm_priv = nullptr,
    .reserved = { nullptr },
};

extern "C" __attribute__((constructor)) void EncodingRegister()
{
    napi_module_register(&g_encodingModule);
}
