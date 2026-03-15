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

#ifndef FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_CRYPTO_JS_ENCODING_HELPER_H
#define FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_CRYPTO_JS_ENCODING_HELPER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include <cstdint>
#include <string>
#include <vector>

namespace EncodingBufferSize {
    constexpr size_t INPUT_BUFFER_SIZE = 2048;
    constexpr size_t OUTPUT_BUFFER_SIZE = 4096;
};

namespace CryptoArgCount {
    constexpr size_t TWO = 2;
};

namespace Base64Const {
    constexpr int GROUP_SIZE = 3;
    constexpr int ENCODED_GROUP_SIZE = 4;
    constexpr int SHIFT_HIGH = 16;
    constexpr int SHIFT_MID = 8;
    constexpr int SHIFT_DECODE_A = 18;
    constexpr int SHIFT_DECODE_B = 12;
    constexpr int SHIFT_DECODE_C = 6;
    constexpr int TABLE_SIZE = 128;
    constexpr int SECOND_BYTE_OFFSET = 2;
    constexpr int THIRD_BYTE_OFFSET = 3;
};

namespace HexConst {
    constexpr int NIBBLE_SHIFT = 4;
    constexpr int ALPHA_OFFSET = 10;
    constexpr int HEX_PAIR = 2;
};

namespace HashConst {
    constexpr uint32_t SIMPLE_MULTIPLIER = 31;
    constexpr uint32_t DJB2_INIT = 5381;
    constexpr uint32_t DJB2_SHIFT = 5;
    constexpr uint32_t FNV1A_INIT = 2166136261u;
    constexpr uint32_t FNV1A_PRIME = 16777619u;
    constexpr int CRC_BITS = 8;
    constexpr uint32_t CRC_INIT = 0xFFFFFFFF;
    constexpr uint32_t CRC_POLY = 0xEDB88320;
    constexpr int HASH_HEX_BUF_SIZE = 16;
};

namespace ChecksumConst {
    constexpr uint16_t FLETCHER_MOD = 255;
    constexpr uint32_t ADLER_MOD = 65521;
    constexpr int ADLER_SHIFT = 16;
    constexpr int FLETCHER_SHIFT = 8;
};

namespace CipherConst {
    constexpr int ALPHABET_SIZE = 26;
    constexpr int ROT13_SHIFT = 13;
};

namespace URLConst {
    constexpr int PERCENT_SKIP = 2;
    constexpr int URL_BUF_SIZE = 4;
    constexpr int RESERVE_MULTIPLIER = 3;
};

class Base64Helper {
public:
    static std::string Encode(const std::string& input);
    static std::string Decode(const std::string& input);
    static std::string EncodeBytes(const std::vector<uint8_t>& input);
    static std::vector<uint8_t> DecodeToBytes(const std::string& input);

private:
    static const char encodeTable[];
    static const int decodeTable[];
    static constexpr char padding = '=';
};

class HexHelper {
public:
    static std::string Encode(const std::string& input);
    static std::string Decode(const std::string& input);
    static std::string EncodeBytes(const std::vector<uint8_t>& input);
    static std::vector<uint8_t> DecodeToBytes(const std::string& input);
    static bool IsValidHex(const std::string& input);

private:
    static const char hexChars[];
    static int CharToNibble(char c);
};

class HashHelper {
public:
    static uint32_t SimpleHash(const std::string& input);
    static uint32_t DJB2Hash(const std::string& input);
    static uint32_t FNV1aHash(const std::string& input);
    static uint32_t CRC32(const std::string& input);
    static std::string HashToHex(uint32_t hash);
};

class ChecksumHelper {
public:
    static uint8_t XORChecksum(const std::string& input);
    static uint16_t Fletcher16(const std::string& input);
    static uint32_t Adler32(const std::string& input);
};

class ROT13Helper {
public:
    static std::string Transform(const std::string& input);
};

class URLEncodingHelper {
public:
    static std::string Encode(const std::string& input);
    static std::string Decode(const std::string& input);

private:
    static bool ShouldEncode(char c);
};

#endif
