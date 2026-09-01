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

#include <string>

#include "bitwise_suite_common.h"
#include "napi/native_node_api.h"

napi_value GrayEncode(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);

    uint32_t result = value ^ (value >> 1u);
    return CreateUint32Value(env, result);
}

napi_value GrayDecode(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t result = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t shift = HALF_UINT32_BITS;
    while (shift > 0) {
        result ^= result >> shift;
        shift >>= 1u;
    }
    return CreateUint32Value(env, result);
}

napi_value ByteSwap32(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);

    uint32_t result = ((value & BYTE_MASK_ZERO) << BYTE_SHIFT_THREE) |
                      ((value & BYTE_MASK_ONE) << BYTE_BITS) |
                      ((value & BYTE_MASK_TWO) >> BYTE_BITS) |
                      ((value & BYTE_MASK_THREE) >> BYTE_SHIFT_THREE);
    return CreateUint32Value(env, result);
}

napi_value ReverseByteOrder16(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]) & LOW_TWO_BYTES_MASK;

    uint32_t result = ((value & HIGH_BYTE_OF_WORD_MASK) >> BYTE_BITS) |
                      ((value & BYTE_MASK_ZERO) << BYTE_BITS);
    return CreateUint32Value(env, result);
}

napi_value HammingDistance(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: value, other");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t other = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);

    uint32_t count = 0;
    uint32_t difference = value ^ other;
    while (difference != 0) {
        count += difference & 1u;
        difference >>= 1u;
    }
    return CreateUint32Value(env, count);
}

napi_value MaskLowBits(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: value, count");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t count = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);
    NAPI_ASSERT(env, count <= UINT32_BITS, "Count must be between 0 and 32");

    uint32_t mask = (count >= UINT32_BITS) ? BIT_MASK_FULL : ((1u << count) - 1u);
    uint32_t result = value & mask;
    return CreateUint32Value(env, result);
}

napi_value MaskHighBits(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: value, count");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t count = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);
    NAPI_ASSERT(env, count <= UINT32_BITS, "Count must be between 0 and 32");

    uint32_t mask = 0;
    if (count >= UINT32_BITS) {
        mask = BIT_MASK_FULL;
    } else if (count > 0) {
        mask = ~((1u << (UINT32_BITS - count)) - 1u);
    }
    uint32_t result = value & mask;
    return CreateUint32Value(env, result);
}

napi_value ToBinaryString(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);

    std::string result(BINARY_STRING_LENGTH, BINARY_CHAR_ZERO);
    for (int32_t index = 0; index < UINT32_BITS; ++index) {
        uint32_t bit = (value >> (UINT32_BITS - 1 - index)) & 1u;
        if (bit != 0) {
            result[index] = BINARY_CHAR_ONE;
        }
    }
    return CreateStringValue(env, result);
}

napi_value ToHexString(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);

    std::string result(HEX_STRING_LENGTH, HEX_DIGITS[0]);
    for (int32_t index = 0; index < HEX_STRING_LENGTH; ++index) {
        uint32_t shift = (HEX_STRING_LENGTH - 1 - index) * NIBBLE_BITS;
        uint32_t digit = (value >> shift) % HEX_BASE;
        result[index] = HEX_DIGITS[digit];
    }
    return CreateStringValue(env, result);
}

napi_value BitInterleave(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: first, second");

    uint32_t first = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]) & LOW_TWO_BYTES_MASK;
    uint32_t second = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]) & LOW_TWO_BYTES_MASK;

    uint32_t result = 0;
    for (int32_t index = 0; index < HALF_UINT32_BITS; ++index) {
        uint32_t evenBit = (first >> index) & 1u;
        uint32_t oddBit = (second >> index) & 1u;
        result |= evenBit << (index * INTERLEAVE_PAIR_STRIDE);
        result |= oddBit << (index * INTERLEAVE_PAIR_STRIDE + 1);
    }
    return CreateUint32Value(env, result);
}

napi_value BitDeinterleaveEven(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);

    uint32_t result = 0;
    for (int32_t index = 0; index < HALF_UINT32_BITS; ++index) {
        uint32_t bit = (value >> (index * INTERLEAVE_PAIR_STRIDE)) & 1u;
        result |= bit << index;
    }
    return CreateUint32Value(env, result);
}

napi_value BitDeinterleaveOdd(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);

    uint32_t result = 0;
    for (int32_t index = 0; index < HALF_UINT32_BITS; ++index) {
        uint32_t bit = (value >> (index * INTERLEAVE_PAIR_STRIDE + 1)) & 1u;
        result |= bit << index;
    }
    return CreateUint32Value(env, result);
}

napi_value SignBit(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    int32_t value = ExtractInt32Arg(env, argv[ARG_INDEX_ZERO]);

    uint32_t bits = static_cast<uint32_t>(value);
    bool result = ((bits >> SIGN_BIT_POSITION) & 1u) != 0;
    return CreateBoolValue(env, result);
}

napi_value CountBitsRange(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: value, from, to");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t fromIndex = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);
    uint32_t toIndex = ExtractUint32Arg(env, argv[ARG_INDEX_TWO]);
    NAPI_ASSERT(env, fromIndex <= toIndex && toIndex <= MAX_BIT_INDEX,
                "Range must satisfy from <= to <= 31");

    uint32_t count = 0;
    for (uint32_t index = fromIndex; index <= toIndex; ++index) {
        count += (value >> index) & 1u;
    }
    return CreateUint32Value(env, count);
}

napi_value IsBitmaskPalindrome(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);

    bool result = true;
    for (int32_t index = 0; index < HALF_UINT32_BITS; ++index) {
        uint32_t lowBit = (value >> index) & 1u;
        uint32_t highBit = (value >> (UINT32_BITS - 1 - index)) & 1u;
        if (lowBit != highBit) {
            result = false;
            break;
        }
    }
    return CreateBoolValue(env, result);
}

napi_value SwapAdjacentBits(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);

    uint32_t result = ((value & EVEN_BITS_MASK) << 1u) | ((value & ODD_BITS_MASK) >> 1u);
    return CreateUint32Value(env, result);
}

napi_value ExtractBits(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: value, from, to");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t fromIndex = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);
    uint32_t toIndex = ExtractUint32Arg(env, argv[ARG_INDEX_TWO]);
    NAPI_ASSERT(env, fromIndex <= toIndex && toIndex <= MAX_BIT_INDEX,
                "Range must satisfy from <= to <= 31");

    uint32_t width = toIndex - fromIndex + 1u;
    uint32_t mask = (width >= UINT32_BITS) ? BIT_MASK_FULL : ((1u << width) - 1u);
    uint32_t result = (value >> fromIndex) & mask;
    return CreateUint32Value(env, result);
}

napi_value BitsRequired(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);

    uint32_t count = 0;
    uint32_t remaining = value;
    while (remaining != 0) {
        count += 1u;
        remaining >>= 1u;
    }
    return CreateUint32Value(env, count);
}

napi_value Pow2(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: exponent");

    uint32_t exponent = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    NAPI_ASSERT(env, exponent <= MAX_SHIFT, "Exponent must be between 0 and 31");

    uint32_t result = 1u << exponent;
    return CreateUint32Value(env, result);
}

napi_value Int32ToUint32(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    int32_t value = ExtractInt32Arg(env, argv[ARG_INDEX_ZERO]);

    uint32_t result = static_cast<uint32_t>(value);
    return CreateUint32Value(env, result);
}

napi_value Uint32ToInt32(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);

    int32_t result = static_cast<int32_t>(value);
    return CreateInt32Value(env, result);
}

napi_value HasSameSign(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: first, second");

    int32_t first = ExtractInt32Arg(env, argv[ARG_INDEX_ZERO]);
    int32_t second = ExtractInt32Arg(env, argv[ARG_INDEX_ONE]);

    bool result = ((first ^ second) >= 0);
    return CreateBoolValue(env, result);
}

napi_status RegisterBitwiseOpsFunctions(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("grayEncode", GrayEncode),
        DECLARE_NAPI_FUNCTION("grayDecode", GrayDecode),
        DECLARE_NAPI_FUNCTION("byteSwap32", ByteSwap32),
        DECLARE_NAPI_FUNCTION("reverseByteOrder16", ReverseByteOrder16),
        DECLARE_NAPI_FUNCTION("hammingDistance", HammingDistance),
        DECLARE_NAPI_FUNCTION("maskLowBits", MaskLowBits),
        DECLARE_NAPI_FUNCTION("maskHighBits", MaskHighBits),
        DECLARE_NAPI_FUNCTION("toBinaryString", ToBinaryString),
        DECLARE_NAPI_FUNCTION("toHexString", ToHexString),
        DECLARE_NAPI_FUNCTION("bitInterleave", BitInterleave),
        DECLARE_NAPI_FUNCTION("bitDeinterleaveEven", BitDeinterleaveEven),
        DECLARE_NAPI_FUNCTION("bitDeinterleaveOdd", BitDeinterleaveOdd),
        DECLARE_NAPI_FUNCTION("signBit", SignBit),
        DECLARE_NAPI_FUNCTION("countBitsRange", CountBitsRange),
        DECLARE_NAPI_FUNCTION("isBitmaskPalindrome", IsBitmaskPalindrome),
        DECLARE_NAPI_FUNCTION("swapAdjacentBits", SwapAdjacentBits),
        DECLARE_NAPI_FUNCTION("extractBits", ExtractBits),
        DECLARE_NAPI_FUNCTION("bitsRequired", BitsRequired),
        DECLARE_NAPI_FUNCTION("pow2", Pow2),
        DECLARE_NAPI_FUNCTION("int32ToUint32", Int32ToUint32),
        DECLARE_NAPI_FUNCTION("uint32ToInt32", Uint32ToInt32),
        DECLARE_NAPI_FUNCTION("hasSameSign", HasSameSign),
    };
    return napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
}
