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
#include <string>
#include <vector>

#include "napi/native_node_api.h"
#include "uuid_suite_common.h"

static void AppendHexByteWithDigits(std::string& out, uint8_t value, const char* digits)
{
    for (int i = HEX_DIGITS_PER_BYTE - 1; i >= 0; --i) {
        uint8_t nibble = static_cast<uint8_t>((value >> (i * HEX_HIGH_SHIFT)) & HEX_NIBBLE_MASK);
        out.push_back(digits[nibble]);
    }
}

void AppendHexByte(std::string& out, uint8_t value)
{
    AppendHexByteWithDigits(out, value, HEX_DIGITS);
}

uint32_t XorshiftNext(RandomState& state)
{
    uint32_t value = state.value;
    value ^= value << XORSHIFT_SHIFT_ONE;
    value ^= value >> XORSHIFT_SHIFT_TWO;
    value ^= value << XORSHIFT_SHIFT_THREE;
    state.value = value;
    return value;
}

double NextUnitUniform(RandomState& state)
{
    return static_cast<double>(XorshiftNext(state)) / UNIFORM_DIVISOR;
}

RandomState MakeRandomState(uint32_t seed)
{
    uint32_t initial = (seed != 0) ? seed : XORSHIFT_INITIAL_STATE;
    RandomState state = { initial };
    return state;
}

int CharToValue(char ch, bool& valid)
{
    for (uint32_t i = 0; i < HEX_BASE; ++i) {
        if (HEX_DIGITS[i] == ch) {
            valid = true;
            return static_cast<int>(i);
        }
    }
    for (uint32_t i = 0; i < HEX_BASE; ++i) {
        if (HEX_DIGITS_UPPER[i] == ch) {
            valid = true;
            return static_cast<int>(i);
        }
    }
    valid = false;
    return CHAR_INVALID_VALUE;
}

void IsHexString(const std::string& text, bool& valid)
{
    valid = !text.empty();
    for (size_t i = 0; valid && i < text.size(); ++i) {
        bool digitValid = false;
        CharToValue(text[i], digitValid);
        valid = digitValid;
    }
}

napi_status ExtractUint32Arg(napi_env env, napi_value value, uint32_t& result)
{
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, value, &type);
    if (status != napi_ok) {
        return status;
    }
    if (type != napi_number) {
        return napi_number_expected;
    }
    status = napi_get_value_uint32(env, value, &result);
    if (status != napi_ok) {
        return status;
    }
    return napi_ok;
}

napi_status ExtractInt32Arg(napi_env env, napi_value value, int32_t& result)
{
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, value, &type);
    if (status != napi_ok) {
        return status;
    }
    if (type != napi_number) {
        return napi_number_expected;
    }
    status = napi_get_value_int32(env, value, &result);
    if (status != napi_ok) {
        return status;
    }
    return napi_ok;
}

napi_status ExtractInt64Arg(napi_env env, napi_value value, int64_t& result)
{
    napi_valuetype type = napi_undefined;
    napi_status status = napi_typeof(env, value, &type);
    if (status != napi_ok) {
        return status;
    }
    if (type != napi_number) {
        return napi_number_expected;
    }
    status = napi_get_value_int64(env, value, &result);
    if (status != napi_ok) {
        return status;
    }
    return napi_ok;
}

napi_status ExtractStringArg(napi_env env, napi_value value, std::string& result)
{
    size_t bufferSize = 0;
    napi_status status = napi_get_value_string_utf8(env, value, nullptr, 0, &bufferSize);
    if (status != napi_ok) {
        return status;
    }
    std::string buffer(bufferSize + 1, '\0');
    size_t copiedLength = 0;
    status = napi_get_value_string_utf8(env, value, &buffer[0], bufferSize, &copiedLength);
    if (status != napi_ok) {
        return status;
    }
    result.assign(buffer, 0, copiedLength);
    return napi_ok;
}

napi_status CreateStringValue(napi_env env, const std::string& text, napi_value& result)
{
    napi_status status = napi_create_string_utf8(env, text.c_str(), text.size(), &result);
    return status;
}

static bool IsHyphenPosition(size_t index)
{
    return index == UUID_HYPHEN_POS_FIRST || index == UUID_HYPHEN_POS_SECOND ||
           index == UUID_HYPHEN_POS_THIRD || index == UUID_HYPHEN_POS_FOURTH;
}

static std::string BuildUuidString(const std::vector<uint8_t>& bytes, bool uppercase)
{
    const char* digits = uppercase ? HEX_DIGITS_UPPER : HEX_DIGITS;
    std::string result;
    for (size_t i = 0; i < bytes.size(); ++i) {
        AppendHexByteWithDigits(result, bytes[i], digits);
        if (IsHyphenPosition(result.size())) {
            result.push_back(CHAR_HYPHEN);
        }
    }
    return result;
}

static void DrawUuidBytes(RandomState& state, std::vector<uint8_t>& bytes)
{
    bytes.clear();
    for (int i = 0; i < UUID_BYTE_LENGTH; ++i) {
        uint8_t byte = static_cast<uint8_t>(XorshiftNext(state) & BYTE_VALUE_MASK);
        bytes.push_back(byte);
    }
    uint8_t versionByte = bytes[UUID_VERSION_BYTE_INDEX];
    uint8_t versionBits = static_cast<uint8_t>(versionByte & UUID_VERSION_NIBBLE_MASK);
    bytes[UUID_VERSION_BYTE_INDEX] = static_cast<uint8_t>(versionBits | UUID_V4_CLOCK_SEQ_HI_BITS);
    uint8_t variantByte = bytes[UUID_VARIANT_BYTE_INDEX];
    uint8_t variantBits = static_cast<uint8_t>(variantByte & UUID_V4_CLOCK_SEQ_HI_MASK);
    bytes[UUID_VARIANT_BYTE_INDEX] = static_cast<uint8_t>(variantBits | UUID_V4_CLOCK_SEQ_LOW_MASK);
}

napi_value UuidV4(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: seed");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    RandomState state = MakeRandomState(seed);
    std::vector<uint8_t> bytes;
    DrawUuidBytes(state, bytes);
    std::string text = BuildUuidString(bytes, false);

    napi_value result = nullptr;
    NAPI_CALL(env, CreateStringValue(env, text, result));
    return result;
}

napi_value UuidV4Upper(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: seed");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    RandomState state = MakeRandomState(seed);
    std::vector<uint8_t> bytes;
    DrawUuidBytes(state, bytes);
    std::string text = BuildUuidString(bytes, true);

    napi_value result = nullptr;
    NAPI_CALL(env, CreateStringValue(env, text, result));
    return result;
}

napi_value UuidNil(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, CreateStringValue(env, UUID_NIL, result));
    return result;
}

napi_value UuidMax(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value result = nullptr;
    NAPI_CALL(env, CreateStringValue(env, UUID_MAX, result));
    return result;
}

napi_value IsNil(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: uuid");

    std::string value;
    NAPI_CALL(env, ExtractStringArg(env, argv[ARG_INDEX_ZERO], value));

    bool isNil = (value == UUID_NIL);

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, isNil, &result));
    return result;
}

napi_value IsUuid(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: uuid");

    std::string value;
    NAPI_CALL(env, ExtractStringArg(env, argv[ARG_INDEX_ZERO], value));

    bool valid = (value.size() == UUID_STRING_LENGTH);
    int hyphenCount = 0;
    for (size_t i = 0; valid && i < value.size(); ++i) {
        if (IsHyphenPosition(i)) {
            valid = (value[i] == CHAR_HYPHEN);
            if (valid) {
                ++hyphenCount;
            }
        } else {
            bool hexValid = false;
            CharToValue(value[i], hexValid);
            valid = hexValid;
        }
    }
    valid = valid && (hyphenCount == UUID_HYPHEN_COUNT);

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, valid, &result));
    return result;
}

napi_value UuidVersion(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: uuid");

    std::string value;
    NAPI_CALL(env, ExtractStringArg(env, argv[ARG_INDEX_ZERO], value));

    int version = CHAR_INVALID_VALUE;
    if (value.size() == UUID_STRING_LENGTH) {
        bool hexValid = false;
        int nibble = CharToValue(value[UUID_VERSION_CHAR_INDEX], hexValid);
        if (hexValid && nibble >= UUID_MIN_VERSION && nibble <= UUID_MAX_VERSION) {
            version = nibble;
        }
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, version, &result));
    return result;
}

napi_value UuidVariant(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: uuid");

    std::string value;
    NAPI_CALL(env, ExtractStringArg(env, argv[ARG_INDEX_ZERO], value));

    int variant = CHAR_INVALID_VALUE;
    if (value.size() == UUID_STRING_LENGTH) {
        bool hexValid = false;
        int nibble = CharToValue(value[UUID_VARIANT_CHAR_INDEX], hexValid);
        if (hexValid) {
            variant = nibble >> UUID_VARIANT_SHIFT;
        }
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, variant, &result));
    return result;
}

napi_value UuidCompare(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: uuidA, uuidB");

    std::string valueA;
    NAPI_CALL(env, ExtractStringArg(env, argv[ARG_INDEX_ZERO], valueA));

    std::string valueB;
    NAPI_CALL(env, ExtractStringArg(env, argv[ARG_INDEX_ONE], valueB));

    int32_t order = 0;
    if (valueA < valueB) {
        order = -1;
    } else if (valueB < valueA) {
        order = 1;
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, order, &result));
    return result;
}

napi_value ShortId(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: seed, length");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    int32_t length = 0;
    NAPI_CALL(env, ExtractInt32Arg(env, argv[ARG_INDEX_ONE], length));
    NAPI_ASSERT(env, length >= SHORT_ID_MIN_LENGTH && length <= SHORT_ID_MAX_LENGTH,
                "Length must be within short id bounds");

    std::string alphabet = std::string(ALPHABET_LETTERS) + std::string(ALPHABET_DIGITS);
    RandomState state = MakeRandomState(seed);
    std::string result;
    for (int32_t i = 0; i < length; ++i) {
        uint32_t index = XorshiftNext(state) % static_cast<uint32_t>(alphabet.size());
        result.push_back(alphabet[index]);
    }

    napi_value out = nullptr;
    NAPI_CALL(env, CreateStringValue(env, result, out));
    return out;
}

napi_value RandomHex(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: seed, length");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    int32_t length = 0;
    NAPI_CALL(env, ExtractInt32Arg(env, argv[ARG_INDEX_ONE], length));
    NAPI_ASSERT(env, length >= SHORT_ID_MIN_LENGTH && length <= SHORT_ID_MAX_LENGTH,
                "Length must be within short id bounds");

    RandomState state = MakeRandomState(seed);
    std::string result;
    for (int32_t i = 0; i < length; ++i) {
        uint32_t index = XorshiftNext(state) % HEX_BASE;
        result.push_back(HEX_DIGITS[index]);
    }

    napi_value out = nullptr;
    NAPI_CALL(env, CreateStringValue(env, result, out));
    return out;
}

napi_value RandomDigits(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: seed, length");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    int32_t length = 0;
    NAPI_CALL(env, ExtractInt32Arg(env, argv[ARG_INDEX_ONE], length));
    NAPI_ASSERT(env, length >= SHORT_ID_MIN_LENGTH && length <= SHORT_ID_MAX_LENGTH,
                "Length must be within short id bounds");

    std::string alphabet = ALPHABET_DIGITS;
    RandomState state = MakeRandomState(seed);
    std::string result;
    for (int32_t i = 0; i < length; ++i) {
        uint32_t index = XorshiftNext(state) % static_cast<uint32_t>(alphabet.size());
        result.push_back(alphabet[index]);
    }

    napi_value out = nullptr;
    NAPI_CALL(env, CreateStringValue(env, result, out));
    return out;
}

napi_value RandomAlpha(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: seed, length");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    int32_t length = 0;
    NAPI_CALL(env, ExtractInt32Arg(env, argv[ARG_INDEX_ONE], length));
    NAPI_ASSERT(env, length >= SHORT_ID_MIN_LENGTH && length <= SHORT_ID_MAX_LENGTH,
                "Length must be within short id bounds");

    std::string alphabet = ALPHABET_LETTERS;
    RandomState state = MakeRandomState(seed);
    std::string result;
    for (int32_t i = 0; i < length; ++i) {
        uint32_t index = XorshiftNext(state) % static_cast<uint32_t>(alphabet.size());
        result.push_back(alphabet[index]);
    }

    napi_value out = nullptr;
    NAPI_CALL(env, CreateStringValue(env, result, out));
    return out;
}

napi_value RandomAlphaNumeric(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: seed, length");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    int32_t length = 0;
    NAPI_CALL(env, ExtractInt32Arg(env, argv[ARG_INDEX_ONE], length));
    NAPI_ASSERT(env, length >= SHORT_ID_MIN_LENGTH && length <= SHORT_ID_MAX_LENGTH,
                "Length must be within short id bounds");

    std::string alphabet = std::string(ALPHABET_LETTERS) + std::string(ALPHABET_DIGITS);
    RandomState state = MakeRandomState(seed);
    std::string result;
    for (int32_t i = 0; i < length; ++i) {
        uint32_t index = XorshiftNext(state) % static_cast<uint32_t>(alphabet.size());
        result.push_back(alphabet[index]);
    }

    napi_value out = nullptr;
    NAPI_CALL(env, CreateStringValue(env, result, out));
    return out;
}

napi_value RandomBase32(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: seed, length");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    int32_t length = 0;
    NAPI_CALL(env, ExtractInt32Arg(env, argv[ARG_INDEX_ONE], length));
    NAPI_ASSERT(env, length >= SHORT_ID_MIN_LENGTH && length <= SHORT_ID_MAX_LENGTH,
                "Length must be within short id bounds");

    std::string alphabet = ALPHABET_BASE32;
    RandomState state = MakeRandomState(seed);
    std::string result;
    for (int32_t i = 0; i < length; ++i) {
        uint32_t index = XorshiftNext(state) % static_cast<uint32_t>(alphabet.size());
        result.push_back(alphabet[index]);
    }

    napi_value out = nullptr;
    NAPI_CALL(env, CreateStringValue(env, result, out));
    return out;
}

napi_value RandomBytesHex(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: seed, length");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    int32_t length = 0;
    NAPI_CALL(env, ExtractInt32Arg(env, argv[ARG_INDEX_ONE], length));
    NAPI_ASSERT(env, length >= SHORT_ID_MIN_LENGTH && length <= SHORT_ID_MAX_LENGTH,
                "Length must be within short id bounds");

    RandomState state = MakeRandomState(seed);
    std::string result;
    for (int32_t i = 0; i < length; ++i) {
        uint8_t byte = static_cast<uint8_t>(XorshiftNext(state) & BYTE_VALUE_MASK);
        AppendHexByte(result, byte);
    }

    napi_value out = nullptr;
    NAPI_CALL(env, CreateStringValue(env, result, out));
    return out;
}

napi_value RandomEven(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: seed, min, max");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    int64_t minValue = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], minValue));

    int64_t maxValue = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_TWO], maxValue));
    NAPI_ASSERT(env, minValue <= maxValue, "Minimum must not exceed maximum");
    NAPI_ASSERT(env, (maxValue - minValue) >= (PARITY_MODULUS - 1), "Range must contain an even number");

    RandomState state = MakeRandomState(seed);
    double span = static_cast<double>(maxValue - minValue + PARITY_MODULUS);
    int64_t value = minValue + static_cast<int64_t>(NextUnitUniform(state) * span);
    if (value > maxValue) {
        value = maxValue;
    }
    if ((value % PARITY_MODULUS) != PARITY_EVEN_REMAINDER) {
        value -= 1;
        if (value < minValue) {
            value += PARITY_MODULUS;
        }
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int64(env, value, &result));
    return result;
}

napi_value RandomOdd(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: seed, min, max");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    int64_t minValue = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], minValue));

    int64_t maxValue = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_TWO], maxValue));
    NAPI_ASSERT(env, minValue <= maxValue, "Minimum must not exceed maximum");
    NAPI_ASSERT(env, (maxValue - minValue) >= (PARITY_MODULUS - 1), "Range must contain an odd number");

    RandomState state = MakeRandomState(seed);
    double span = static_cast<double>(maxValue - minValue + PARITY_MODULUS);
    int64_t value = minValue + static_cast<int64_t>(NextUnitUniform(state) * span);
    if (value > maxValue) {
        value = maxValue;
    }
    if ((value % PARITY_MODULUS) != PARITY_ODD_REMAINDER) {
        value -= 1;
        if (value < minValue) {
            value += PARITY_MODULUS;
        }
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int64(env, value, &result));
    return result;
}

napi_value RandomLetter(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: seed");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    std::string alphabet = std::string(ALPHABET_LETTERS) + std::string(ALPHABET_UPPER);
    RandomState state = MakeRandomState(seed);
    uint32_t index = XorshiftNext(state) % static_cast<uint32_t>(alphabet.size());

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, &alphabet[index], 1, &result));
    return result;
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("uuidV4", UuidV4),
        DECLARE_NAPI_FUNCTION("uuidV4Upper", UuidV4Upper),
        DECLARE_NAPI_FUNCTION("uuidNil", UuidNil),
        DECLARE_NAPI_FUNCTION("uuidMax", UuidMax),
        DECLARE_NAPI_FUNCTION("isNil", IsNil),
        DECLARE_NAPI_FUNCTION("isUuid", IsUuid),
        DECLARE_NAPI_FUNCTION("uuidVersion", UuidVersion),
        DECLARE_NAPI_FUNCTION("uuidVariant", UuidVariant),
        DECLARE_NAPI_FUNCTION("uuidCompare", UuidCompare),
        DECLARE_NAPI_FUNCTION("shortId", ShortId),
        DECLARE_NAPI_FUNCTION("randomHex", RandomHex),
        DECLARE_NAPI_FUNCTION("randomDigits", RandomDigits),
        DECLARE_NAPI_FUNCTION("randomAlpha", RandomAlpha),
        DECLARE_NAPI_FUNCTION("randomAlphaNumeric", RandomAlphaNumeric),
        DECLARE_NAPI_FUNCTION("randomBase32", RandomBase32),
        DECLARE_NAPI_FUNCTION("randomBytesHex", RandomBytesHex),
        DECLARE_NAPI_FUNCTION("randomEven", RandomEven),
        DECLARE_NAPI_FUNCTION("randomOdd", RandomOdd),
        DECLARE_NAPI_FUNCTION("randomLetter", RandomLetter),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    NAPI_CALL_BASE(env, RegisterUuidOpsFunctions(env, exports), nullptr);
    return exports;
}

static napi_module uuidSuiteModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "uuidSuite",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void UuidSuiteRegisterModule(void)
{
    napi_module_register(&uuidSuiteModule);
}
