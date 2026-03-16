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
#include <cstring>
#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

static constexpr int REQUIRED_ARGS_ONE = 1;
static constexpr size_t DECODE_MAX_ARGS = 2; // Maximum number of arguments for Decode function

// UTF-8 encoding constants
static constexpr uint8_t UTF8_1BYTE_MAX = 0x7F;              // Maximum 1-byte sequence value
static constexpr uint8_t UTF8_CONTINUATION = 0x80;           // 10xxxxxx
static constexpr uint8_t UTF8_CONTINUATION_MASK = 0xC0;      // 11000000
static constexpr uint8_t UTF8_2BYTE_MIN = 0xC2;              // Minimum valid 2-byte start
static constexpr uint8_t UTF8_2BYTE_MAX = 0xDF;              // Maximum 2-byte start
static constexpr uint8_t UTF8_3BYTE_MIN = 0xE0;              // Minimum 3-byte start
static constexpr uint8_t UTF8_3BYTE_MAX = 0xEF;              // Maximum 3-byte start
static constexpr uint8_t UTF8_4BYTE_MIN = 0xF0;              // Minimum 4-byte start
static constexpr uint8_t UTF8_4BYTE_MAX = 0xF4;              // Maximum 4-byte start
static constexpr uint8_t UTF8_2BYTE_PREFIX_MASK = 0xC0;      // 11000000
static constexpr uint8_t UTF8_2BYTE_DATA_MASK = 0x1F;        // 00011111
static constexpr uint8_t UTF8_3BYTE_PREFIX_MASK = 0xE0;      // 11100000
static constexpr uint8_t UTF8_3BYTE_DATA_MASK = 0x0F;        // 00001111
static constexpr uint8_t UTF8_4BYTE_PREFIX_MASK = 0xF0;      // 11110000
static constexpr uint8_t UTF8_4BYTE_DATA_MASK = 0x07;        // 00000111
static constexpr uint8_t UTF8_CONTINUATION_DATA_MASK = 0x3F; // 00111111
static constexpr int UTF8_SHIFT_6 = 6;                       // Shift amount for 6 bits
static constexpr int UTF8_SHIFT_12 = 12;                     // Shift amount for 12 bits
static constexpr int UTF8_SHIFT_18 = 18;                     // Shift amount for 18 bits
static constexpr size_t UTF8_2BYTE_SEQ_SIZE = 2;             // Size of 2-byte UTF-8 sequence
static constexpr size_t UTF8_3BYTE_SEQ_SIZE = 3;             // Size of 3-byte UTF-8 sequence
static constexpr size_t UTF8_4BYTE_SEQ_SIZE = 4;             // Size of 4-byte UTF-8 sequence
static constexpr size_t UTF8_2ND_BYTE_OFFSET = 1;            // Offset for second byte in UTF-8 sequence
static constexpr size_t UTF8_3RD_BYTE_OFFSET = 2;            // Offset for third byte in UTF-8 sequence
static constexpr size_t UTF8_4TH_BYTE_OFFSET = 3;            // Offset for fourth byte in UTF-8 sequence
// UTF-8 overlong encoding check masks
static constexpr uint8_t UTF8_3BYTE_OVERLONG_MASK = 0xE0;  // 11100000
static constexpr uint8_t UTF8_3BYTE_OVERLONG_VALUE = 0x80; // 10000000
static constexpr uint8_t UTF8_4BYTE_OVERLONG_MASK = 0xF0;  // 11110000
static constexpr uint8_t UTF8_4BYTE_OVERLONG_VALUE = 0x80; // 10000000
static constexpr uint8_t UTF8_4BYTE_MAX_MASK = 0xF0;       // 11110000
static constexpr uint8_t UTF8_4BYTE_MAX_VALUE = 0x90;      // 10010000

// UTF-16 encoding constants
static constexpr uint16_t UTF16_1BYTE_MAX = 0x7F;  // 0-127
static constexpr uint16_t UTF16_2BYTE_MAX = 0x7FF; // 128-2047
static constexpr uint16_t UTF16_MAX = 0xFFFF;      // Maximum UTF-16 code unit value
static constexpr uint16_t UTF16_SURROGATE_HIGH_START = 0xD800;
static constexpr uint16_t UTF16_SURROGATE_HIGH_END = 0xDBFF;
static constexpr uint16_t UTF16_SURROGATE_LOW_START = 0xDC00;
static constexpr uint16_t UTF16_SURROGATE_LOW_END = 0xDFFF;
static constexpr uint16_t UTF16_BOM = 0xFEFF;
static constexpr uint32_t UTF16_SURROGATE_PAIR_BASE = 0x10000; // Base value for surrogate pair encoding
static constexpr int UTF16_SURROGATE_LOW_SHIFT = 10;           // Shift amount for low surrogate (10 bits)
static constexpr int UTF16_BYTE_SHIFT = 8;                     // Shift amount for byte in UTF-16 (8 bits)
static constexpr size_t UTF16_CODE_UNIT_SIZE = 2;              // Size of a UTF-16 code unit in bytes
static constexpr size_t UTF16_BYTE_OFFSET = 1;                 // Offset for second byte in UTF-16 code unit

// Replacement character for invalid sequences (U+FFFD in UTF-8)
static constexpr char REPLACEMENT_CHAR = '\xEF';
static constexpr char REPLACEMENT_CHAR_2 = '\xBD';
static constexpr char REPLACEMENT_CHAR_3 = '\xBD';

enum class EncodingType { UTF8, UTF16LE, UTF16BE };

static EncodingType ParseEncodingType(napi_env env, napi_value encodingValue)
{
    if (encodingValue == nullptr) {
        return EncodingType::UTF8;
    }

    napi_valuetype valueType;
    napi_status status = napi_typeof(env, encodingValue, &valueType);
    if (status != napi_ok || valueType != napi_string) {
        return EncodingType::UTF8;
    }

    size_t length = 0;
    status = napi_get_value_string_utf8(env, encodingValue, nullptr, 0, &length);
    if (status != napi_ok) {
        return EncodingType::UTF8;
    }

    std::vector<char> encoding(length + 1);
    status = napi_get_value_string_utf8(env, encodingValue, encoding.data(), length + 1, &length);
    if (status != napi_ok) {
        return EncodingType::UTF8;
    }

    std::string encodingStr(encoding.data(), length);

    for (auto& c : encodingStr) {
        if (c >= 'A' && c <= 'Z') {
            c = c - 'A' + 'a';
        }
    }

    if (encodingStr == "utf-8" || encodingStr == "utf8") {
        return EncodingType::UTF8;
    } else if (encodingStr == "utf-16le" || encodingStr == "utf16le") {
        return EncodingType::UTF16LE;
    } else if (encodingStr == "utf-16be" || encodingStr == "utf16be") {
        return EncodingType::UTF16BE;
    }

    return EncodingType::UTF8;
}

static bool IsUTF8ContinuationByte(uint8_t byte)
{
    return (byte & UTF8_CONTINUATION_MASK) == UTF8_CONTINUATION;
}

static std::string DecodeUTF8(const uint8_t* data, size_t length)
{
    if (data == nullptr || length == 0) {
        return "";
    }

    std::string result;
    result.reserve(length);

    size_t i = 0;
    while (i < length) {
        uint8_t byte = data[i];

        // 1-byte sequence (0x00-0x7F)
        if (byte <= UTF8_1BYTE_MAX) {
            result.push_back(static_cast<char>(byte));
            i++;
        }
        // 2-byte sequence (0xC2-0xDF)
        else if (byte >= UTF8_2BYTE_MIN && byte <= UTF8_2BYTE_MAX) {
            if (i + UTF8_2ND_BYTE_OFFSET >= length) {
                // Incomplete sequence, add replacement character
                result.push_back(REPLACEMENT_CHAR);
                result.push_back(REPLACEMENT_CHAR_2);
                result.push_back(REPLACEMENT_CHAR_3);
                break;
            }
            uint8_t byte2 = data[i + UTF8_2ND_BYTE_OFFSET];
            if (!IsUTF8ContinuationByte(byte2)) {
                // Invalid continuation byte
                result.push_back(REPLACEMENT_CHAR);
                result.push_back(REPLACEMENT_CHAR_2);
                result.push_back(REPLACEMENT_CHAR_3);
                i++;
                continue;
            }
            result.push_back(static_cast<char>(byte));
            result.push_back(static_cast<char>(byte2));
            i += UTF8_2BYTE_SEQ_SIZE;
        }
        // 3-byte sequence (0xE0-0xEF)
        else if (byte >= UTF8_3BYTE_MIN && byte <= UTF8_3BYTE_MAX) {
            if (i + UTF8_3BYTE_SEQ_SIZE - 1 >= length) {
                // Incomplete sequence
                result.push_back(REPLACEMENT_CHAR);
                result.push_back(REPLACEMENT_CHAR_2);
                result.push_back(REPLACEMENT_CHAR_3);
                break;
            }
            uint8_t byte2 = data[i + UTF8_2ND_BYTE_OFFSET];
            uint8_t byte3 = data[i + UTF8_3RD_BYTE_OFFSET];

            // Check continuation bytes
            if (!IsUTF8ContinuationByte(byte2) || !IsUTF8ContinuationByte(byte3)) {
                result.push_back(REPLACEMENT_CHAR);
                result.push_back(REPLACEMENT_CHAR_2);
                result.push_back(REPLACEMENT_CHAR_3);
                i++;
                continue;
            }

            // Check for overlong encoding for 0xE0
            if (byte == UTF8_3BYTE_MIN && (byte2 & UTF8_3BYTE_OVERLONG_MASK) == UTF8_3BYTE_OVERLONG_VALUE) {
                result.push_back(REPLACEMENT_CHAR);
                result.push_back(REPLACEMENT_CHAR_2);
                result.push_back(REPLACEMENT_CHAR_3);
                i++;
                continue;
            }

            result.push_back(static_cast<char>(byte));
            result.push_back(static_cast<char>(byte2));
            result.push_back(static_cast<char>(byte3));
            i += UTF8_3BYTE_SEQ_SIZE;
        }
        // 4-byte sequence (0xF0-0xF4)
        else if (byte >= UTF8_4BYTE_MIN && byte <= UTF8_4BYTE_MAX) {
            if (i + UTF8_4BYTE_SEQ_SIZE - 1 >= length) {
                // Incomplete sequence
                result.push_back(REPLACEMENT_CHAR);
                result.push_back(REPLACEMENT_CHAR_2);
                result.push_back(REPLACEMENT_CHAR_3);
                break;
            }
            uint8_t byte2 = data[i + UTF8_2ND_BYTE_OFFSET];
            uint8_t byte3 = data[i + UTF8_3RD_BYTE_OFFSET];
            uint8_t byte4 = data[i + UTF8_4TH_BYTE_OFFSET];

            // Check continuation bytes
            if (!IsUTF8ContinuationByte(byte2) || !IsUTF8ContinuationByte(byte3) || !IsUTF8ContinuationByte(byte4)) {
                result.push_back(REPLACEMENT_CHAR);
                result.push_back(REPLACEMENT_CHAR_2);
                result.push_back(REPLACEMENT_CHAR_3);
                i++;
                continue;
            }

            // Check for overlong encoding for 0xF0
            if (byte == UTF8_4BYTE_MIN && (byte2 & UTF8_4BYTE_OVERLONG_MASK) == UTF8_4BYTE_OVERLONG_VALUE) {
                result.push_back(REPLACEMENT_CHAR);
                result.push_back(REPLACEMENT_CHAR_2);
                result.push_back(REPLACEMENT_CHAR_3);
                i++;
                continue;
            }

            // Check for code point beyond Unicode max (0x10FFFF)
            if (byte == UTF8_4BYTE_MAX && (byte2 & UTF8_4BYTE_MAX_MASK) == UTF8_4BYTE_MAX_VALUE) {
                result.push_back(REPLACEMENT_CHAR);
                result.push_back(REPLACEMENT_CHAR_2);
                result.push_back(REPLACEMENT_CHAR_3);
                i++;
                continue;
            }

            result.push_back(static_cast<char>(byte));
            result.push_back(static_cast<char>(byte2));
            result.push_back(static_cast<char>(byte3));
            result.push_back(static_cast<char>(byte4));
            i += UTF8_4BYTE_SEQ_SIZE;
        }
        // Invalid UTF-8 start byte (0xC0, 0xC1, 0xF5-0xFF)
        else {
            result.push_back(REPLACEMENT_CHAR);
            result.push_back(REPLACEMENT_CHAR_2);
            result.push_back(REPLACEMENT_CHAR_3);
            i++;
        }
    }

    return result;
}

static void AppendCodePointToUTF8(std::string& result, uint32_t codePoint)
{
    if (codePoint <= UTF16_1BYTE_MAX) {
        result.push_back(static_cast<char>(codePoint));
    } else if (codePoint <= UTF16_2BYTE_MAX) {
        result.push_back(
            static_cast<char>(UTF8_2BYTE_PREFIX_MASK | ((codePoint >> UTF8_SHIFT_6) & UTF8_2BYTE_DATA_MASK)));
        result.push_back(static_cast<char>(UTF8_CONTINUATION | (codePoint & UTF8_CONTINUATION_DATA_MASK)));
    } else if (codePoint <= UTF16_MAX) {
        result.push_back(
            static_cast<char>(UTF8_3BYTE_PREFIX_MASK | ((codePoint >> UTF8_SHIFT_12) & UTF8_3BYTE_DATA_MASK)));
        result.push_back(
            static_cast<char>(UTF8_CONTINUATION | ((codePoint >> UTF8_SHIFT_6) & UTF8_CONTINUATION_DATA_MASK)));
        result.push_back(static_cast<char>(UTF8_CONTINUATION | (codePoint & UTF8_CONTINUATION_DATA_MASK)));
    } else {
        result.push_back(
            static_cast<char>(UTF8_4BYTE_PREFIX_MASK | ((codePoint >> UTF8_SHIFT_18) & UTF8_4BYTE_DATA_MASK)));
        result.push_back(
            static_cast<char>(UTF8_CONTINUATION | ((codePoint >> UTF8_SHIFT_12) & UTF8_CONTINUATION_DATA_MASK)));
        result.push_back(
            static_cast<char>(UTF8_CONTINUATION | ((codePoint >> UTF8_SHIFT_6) & UTF8_CONTINUATION_DATA_MASK)));
        result.push_back(static_cast<char>(UTF8_CONTINUATION | (codePoint & UTF8_CONTINUATION_DATA_MASK)));
    }
}

static std::string DecodeUTF16LE(const uint8_t* data, size_t length)
{
    if (data == nullptr || length == 0) {
        return "";
    }

    std::string result;
    result.reserve(length);

    for (size_t i = 0; i + UTF16_BYTE_OFFSET < length; i += UTF16_CODE_UNIT_SIZE) {
        uint16_t codeUnit = static_cast<uint16_t>(data[i] | (data[i + 1] << UTF16_BYTE_SHIFT));

        // Skip BOM at the beginning
        if (i == 0 && codeUnit == UTF16_BOM) {
            continue;
        }

        // Check for high surrogate
        if (codeUnit >= UTF16_SURROGATE_HIGH_START && codeUnit <= UTF16_SURROGATE_HIGH_END) {
            // Need low surrogate
            if (i + UTF16_CODE_UNIT_SIZE + UTF16_BYTE_OFFSET >= length) {
                // Incomplete surrogate pair
                result.push_back(REPLACEMENT_CHAR);
                result.push_back(REPLACEMENT_CHAR_2);
                result.push_back(REPLACEMENT_CHAR_3);
                break;
            }

            uint16_t lowSurrogate =
                static_cast<uint16_t>(data[i + UTF16_CODE_UNIT_SIZE] |
                                      (data[i + UTF16_CODE_UNIT_SIZE + UTF16_BYTE_OFFSET] << UTF16_BYTE_SHIFT));

            if (lowSurrogate >= UTF16_SURROGATE_LOW_START && lowSurrogate <= UTF16_SURROGATE_LOW_END) {
                // Valid surrogate pair
                uint32_t codePoint = UTF16_SURROGATE_PAIR_BASE +
                                     ((codeUnit - UTF16_SURROGATE_HIGH_START) << UTF16_SURROGATE_LOW_SHIFT) +
                                     (lowSurrogate - UTF16_SURROGATE_LOW_START);
                AppendCodePointToUTF8(result, codePoint);
                i += UTF16_CODE_UNIT_SIZE; // Skip the low surrogate
            } else {
                // Invalid surrogate pair
                result.push_back(REPLACEMENT_CHAR);
                result.push_back(REPLACEMENT_CHAR_2);
                result.push_back(REPLACEMENT_CHAR_3);
            }
        }
        // Check for low surrogate without high surrogate
        else if (codeUnit >= UTF16_SURROGATE_LOW_START && codeUnit <= UTF16_SURROGATE_LOW_END) {
            result.push_back(REPLACEMENT_CHAR);
            result.push_back(REPLACEMENT_CHAR_2);
            result.push_back(REPLACEMENT_CHAR_3);
        }
        // Regular code unit
        else {
            AppendCodePointToUTF8(result, codeUnit);
        }
    }

    return result;
}

static std::string DecodeUTF16BE(const uint8_t* data, size_t length)
{
    if (data == nullptr || length == 0) {
        return "";
    }

    std::string result;
    result.reserve(length);

    for (size_t i = 0; i + UTF16_BYTE_OFFSET < length; i += UTF16_CODE_UNIT_SIZE) {
        uint16_t codeUnit = static_cast<uint16_t>((data[i] << UTF16_BYTE_SHIFT) | data[i + 1]);

        // Skip BOM at the beginning
        if (i == 0 && codeUnit == UTF16_BOM) {
            continue;
        }

        // Check for high surrogate
        if (codeUnit >= UTF16_SURROGATE_HIGH_START && codeUnit <= UTF16_SURROGATE_HIGH_END) {
            // Need low surrogate
            if (i + UTF16_CODE_UNIT_SIZE + UTF16_BYTE_OFFSET >= length) {
                // Incomplete surrogate pair
                result.push_back(REPLACEMENT_CHAR);
                result.push_back(REPLACEMENT_CHAR_2);
                result.push_back(REPLACEMENT_CHAR_3);
                break;
            }

            uint16_t lowSurrogate = static_cast<uint16_t>((data[i + UTF16_CODE_UNIT_SIZE] << UTF16_BYTE_SHIFT) |
                                                          data[i + UTF16_CODE_UNIT_SIZE + UTF16_BYTE_OFFSET]);

            if (lowSurrogate >= UTF16_SURROGATE_LOW_START && lowSurrogate <= UTF16_SURROGATE_LOW_END) {
                // Valid surrogate pair
                uint32_t codePoint = UTF16_SURROGATE_PAIR_BASE +
                                     ((codeUnit - UTF16_SURROGATE_HIGH_START) << UTF16_SURROGATE_LOW_SHIFT) +
                                     (lowSurrogate - UTF16_SURROGATE_LOW_START);
                AppendCodePointToUTF8(result, codePoint);
                i += UTF16_CODE_UNIT_SIZE; // Skip the low surrogate
            } else {
                // Invalid surrogate pair
                result.push_back(REPLACEMENT_CHAR);
                result.push_back(REPLACEMENT_CHAR_2);
                result.push_back(REPLACEMENT_CHAR_3);
            }
        }
        // Check for low surrogate without high surrogate
        else if (codeUnit >= UTF16_SURROGATE_LOW_START && codeUnit <= UTF16_SURROGATE_LOW_END) {
            result.push_back(REPLACEMENT_CHAR);
            result.push_back(REPLACEMENT_CHAR_2);
            result.push_back(REPLACEMENT_CHAR_3);
        }
        // Regular code unit
        else {
            AppendCodePointToUTF8(result, codeUnit);
        }
    }

    return result;
}

static napi_value Decode(napi_env env, napi_callback_info info)
{
    size_t argc = DECODE_MAX_ARGS;
    napi_value argv[DECODE_MAX_ARGS] = { nullptr, nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get callback info");
        return nullptr;
    }

    if (argc < REQUIRED_ARGS_ONE) {
        napi_throw_error(env, nullptr, "Wrong number of arguments");
        return nullptr;
    }

    napi_valuetype type;
    status = napi_typeof(env, argv[0], &type);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get argument type");
        return nullptr;
    }

    if (type != napi_object) {
        napi_throw_error(env, nullptr, "First argument must be an object");
        return nullptr;
    }

    bool isArrayBuffer;
    status = napi_is_arraybuffer(env, argv[0], &isArrayBuffer);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to check if argument is ArrayBuffer");
        return nullptr;
    }

    if (!isArrayBuffer) {
        napi_throw_error(env, nullptr, "First argument must be an ArrayBuffer");
        return nullptr;
    }

    void* data = nullptr;
    size_t length = 0;
    status = napi_get_arraybuffer_info(env, argv[0], &data, &length);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get ArrayBuffer info");
        return nullptr;
    }

    EncodingType encoding = ParseEncodingType(env, argc > 1 ? argv[1] : nullptr);

    std::string result;
    switch (encoding) {
        case EncodingType::UTF8:
            result = DecodeUTF8(static_cast<uint8_t*>(data), length);
            break;
        case EncodingType::UTF16LE:
            result = DecodeUTF16LE(static_cast<uint8_t*>(data), length);
            break;
        case EncodingType::UTF16BE:
            result = DecodeUTF16BE(static_cast<uint8_t*>(data), length);
            break;
    }

    napi_value resultString = nullptr;
    status = napi_create_string_utf8(env, result.c_str(), result.length(), &resultString);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to create result string");
        return nullptr;
    }

    return resultString;
}

static napi_value TextDecoderInit(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = { DECLARE_NAPI_FUNCTION("decode", Decode) };

    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);

    return exports;
}

extern "C" __attribute__((visibility("default"))) void* NAPI_GetSym(void* handle)
{
    return reinterpret_cast<void*>(TextDecoderInit);
}
