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

uint32_t ExtractUint32Arg(napi_env env, napi_value value)
{
    napi_valuetype type = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, value, &type), 0);

    if (type != napi_number) {
        NAPI_CALL_BASE(env, napi_throw_error(env, nullptr, "Argument must be a number"), 0);
        return 0;
    }
    uint32_t result = 0;
    NAPI_CALL_BASE(env, napi_get_value_uint32(env, value, &result), 0);
    return result;
}

int32_t ExtractInt32Arg(napi_env env, napi_value value)
{
    napi_valuetype type = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, value, &type), 0);

    if (type != napi_number) {
        NAPI_CALL_BASE(env, napi_throw_error(env, nullptr, "Argument must be a number"), 0);
        return 0;
    }
    int32_t result = 0;
    NAPI_CALL_BASE(env, napi_get_value_int32(env, value, &result), 0);
    return result;
}

napi_value CreateUint32Value(napi_env env, uint32_t value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_uint32(env, value, &result));
    return result;
}

napi_value CreateInt32Value(napi_env env, int32_t value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, value, &result));
    return result;
}

napi_value CreateBoolValue(napi_env env, bool value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, value, &result));
    return result;
}

napi_value CreateStringValue(napi_env env, const std::string& value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), value.size(), &result));
    return result;
}

napi_value And(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: value, other");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t other = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);

    uint32_t result = value & other;
    return CreateUint32Value(env, result);
}

napi_value Or(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: value, other");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t other = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);

    uint32_t result = value | other;
    return CreateUint32Value(env, result);
}

napi_value Xor(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: value, other");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t other = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);

    uint32_t result = value ^ other;
    return CreateUint32Value(env, result);
}

napi_value Not(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);

    uint32_t result = value ^ BIT_MASK_FULL;
    return CreateUint32Value(env, result);
}

napi_value LeftShift(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: value, shift");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t shift = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);
    NAPI_ASSERT(env, shift <= MAX_SHIFT, "Shift count must be between 0 and 31");

    uint32_t result = value << shift;
    return CreateUint32Value(env, result);
}

napi_value RightShift(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: value, shift");

    int32_t value = ExtractInt32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t shift = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);
    NAPI_ASSERT(env, shift <= MAX_SHIFT, "Shift count must be between 0 and 31");

    int32_t result = value >> static_cast<int32_t>(shift);
    return CreateInt32Value(env, result);
}

napi_value UnsignedRightShift(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: value, shift");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t shift = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);
    NAPI_ASSERT(env, shift <= MAX_SHIFT, "Shift count must be between 0 and 31");

    uint32_t result = value >> shift;
    return CreateUint32Value(env, result);
}

napi_value Popcount(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);

    uint32_t count = 0;
    uint32_t remaining = value;
    while (remaining != 0) {
        uint32_t lowestBit = remaining & 1u;
        count += lowestBit;
        remaining >>= 1u;
    }
    return CreateUint32Value(env, count);
}

napi_value CountLeadingZeros(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);

    uint32_t count = 0;
    uint32_t probe = SIGN_BIT_POSITION;
    while (probe != 0) {
        uint32_t bit = (value >> probe) & 1u;
        if (bit != 0) {
            break;
        }
        count += 1u;
        probe -= 1u;
    }
    if (probe == 0 && (value & 1u) == 0) {
        count += 1u;
    }
    return CreateUint32Value(env, count);
}

napi_value CountTrailingZeros(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);

    uint32_t count = 0;
    uint32_t probe = 0;
    while (probe < UINT32_BITS) {
        uint32_t bit = (value >> probe) & 1u;
        if (bit != 0) {
            break;
        }
        count += 1u;
        probe += 1u;
    }
    return CreateUint32Value(env, count);
}

napi_value IsPowerOfTwo(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    if (value == 0) {
        return CreateBoolValue(env, false);
    }
    bool result = (value & (value - 1u)) == 0;
    return CreateBoolValue(env, result);
}

napi_value HighestSetBit(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    if (value == 0) {
        return CreateInt32Value(env, -1);
    }
    int32_t result = 0;
    for (int32_t index = MAX_BIT_INDEX; index >= 0; --index) {
        uint32_t bit = (value >> index) & 1u;
        if (bit != 0) {
            result = index;
            break;
        }
    }
    return CreateInt32Value(env, result);
}

napi_value LowestSetBit(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    if (value == 0) {
        return CreateInt32Value(env, -1);
    }
    int32_t result = 0;
    for (int32_t index = 0; index <= MAX_BIT_INDEX; ++index) {
        uint32_t bit = (value >> index) & 1u;
        if (bit != 0) {
            result = index;
            break;
        }
    }
    return CreateInt32Value(env, result);
}

napi_value BitIsSet(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: value, index");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t index = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);
    NAPI_ASSERT(env, index <= MAX_BIT_INDEX, "Bit index must be between 0 and 31");

    uint32_t bit = (value >> index) & 1u;
    bool result = bit != 0;
    return CreateBoolValue(env, result);
}

napi_value SetBit(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: value, index");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t index = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);
    NAPI_ASSERT(env, index <= MAX_BIT_INDEX, "Bit index must be between 0 and 31");

    uint32_t mask = 1u << index;
    uint32_t result = value | mask;
    return CreateUint32Value(env, result);
}

napi_value ClearBit(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: value, index");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t index = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);
    NAPI_ASSERT(env, index <= MAX_BIT_INDEX, "Bit index must be between 0 and 31");

    uint32_t mask = ~(1u << index);
    uint32_t result = value & mask;
    return CreateUint32Value(env, result);
}

napi_value ToggleBit(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: value, index");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t index = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);
    NAPI_ASSERT(env, index <= MAX_BIT_INDEX, "Bit index must be between 0 and 31");

    uint32_t mask = 1u << index;
    uint32_t result = value ^ mask;
    return CreateUint32Value(env, result);
}

napi_value RotateLeft(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: value, count");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t rawCount = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);
    uint32_t count = rawCount % UINT32_BITS;
    uint32_t complement = (UINT32_BITS - count) & ROTATE_MASK;

    uint32_t leftPart = value << count;
    uint32_t rightPart = value >> complement;
    uint32_t result = leftPart | rightPart;
    return CreateUint32Value(env, result);
}

napi_value RotateRight(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: value, count");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t rawCount = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);
    uint32_t count = rawCount % UINT32_BITS;
    uint32_t complement = (UINT32_BITS - count) & ROTATE_MASK;

    uint32_t rightPart = value >> count;
    uint32_t leftPart = value << complement;
    uint32_t result = rightPart | leftPart;
    return CreateUint32Value(env, result);
}

napi_value ReverseBits(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);

    uint32_t result = 0;
    for (int32_t index = 0; index < UINT32_BITS; ++index) {
        uint32_t bit = (value >> index) & 1u;
        uint32_t target = UINT32_BITS - 1 - index;
        result |= bit << target;
    }
    return CreateUint32Value(env, result);
}

napi_value Parity(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);

    uint32_t count = 0;
    uint32_t remaining = value;
    while (remaining != 0) {
        uint32_t lowestBit = remaining & 1u;
        count += lowestBit;
        remaining >>= 1u;
    }
    uint32_t result = count & 1u;
    return CreateUint32Value(env, result);
}

napi_value NextPowerOfTwo(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t limit = static_cast<uint32_t>(INT32_MAX) + 1u;
    NAPI_ASSERT(env, value <= limit, "Value too large for next power of two");

    if (value <= 1u) {
        return CreateUint32Value(env, 1u);
    }
    uint32_t result = 1u;
    while (result < value) {
        result <<= 1u;
    }
    return CreateUint32Value(env, result);
}

napi_value AddWithoutCarry(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: value, other");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t other = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);

    uint32_t result = value ^ other;
    return CreateUint32Value(env, result);
}

napi_value MultiplyByPow2(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: value, exponent");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t exponent = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);
    NAPI_ASSERT(env, exponent <= MAX_SHIFT, "Exponent must be between 0 and 31");

    uint32_t result = value << exponent;
    return CreateUint32Value(env, result);
}

napi_value DivideByPow2(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: value, exponent");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t exponent = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);
    NAPI_ASSERT(env, exponent <= MAX_SHIFT, "Exponent must be between 0 and 31");

    uint32_t result = value >> exponent;
    return CreateUint32Value(env, result);
}

napi_value IsolateLowestOne(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);

    uint32_t negated = ~value;
    uint32_t result = value & (negated + 1u);
    return CreateUint32Value(env, result);
}

napi_value ClearLowestOne(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);

    uint32_t result = value & (value - 1u);
    return CreateUint32Value(env, result);
}

napi_value LeadingOnes(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    if (value == BIT_MASK_FULL) {
        return CreateUint32Value(env, UINT32_BITS);
    }
    uint32_t count = 0;
    uint32_t probe = SIGN_BIT_POSITION;
    while (probe != 0) {
        uint32_t bit = (value >> probe) & 1u;
        if (bit != 0) {
            count += 1u;
            probe -= 1u;
        } else {
            break;
        }
    }
    if (probe == 0 && (value & 1u) != 0) {
        count += 1u;
    }
    return CreateUint32Value(env, count);
}

napi_value TrailingOnes(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    if (value == BIT_MASK_FULL) {
        return CreateUint32Value(env, UINT32_BITS);
    }
    uint32_t count = 0;
    uint32_t probe = 0;
    while (probe < UINT32_BITS) {
        uint32_t bit = (value >> probe) & 1u;
        if (bit == 0) {
            break;
        }
        count += 1u;
        probe += 1u;
    }
    return CreateUint32Value(env, count);
}

napi_value ReverseNibbles(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: value");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);

    uint32_t result = 0;
    for (int32_t index = 0; index < UINT32_BITS; index += NIBBLE_BITS) {
        uint32_t nibble = (value >> index) & NIBBLE_MASK;
        uint32_t target = UINT32_BITS - NIBBLE_BITS - index;
        result |= nibble << target;
    }
    return CreateUint32Value(env, result);
}

napi_value AverageFloor(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: value, other");

    uint32_t value = ExtractUint32Arg(env, argv[ARG_INDEX_ZERO]);
    uint32_t other = ExtractUint32Arg(env, argv[ARG_INDEX_ONE]);

    uint32_t bothSet = value & other;
    uint32_t exactlyOneSet = value ^ other;
    uint32_t result = bothSet + (exactlyOneSet >> 1u);
    return CreateUint32Value(env, result);
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("and", And),
        DECLARE_NAPI_FUNCTION("or", Or),
        DECLARE_NAPI_FUNCTION("xor", Xor),
        DECLARE_NAPI_FUNCTION("not", Not),
        DECLARE_NAPI_FUNCTION("leftShift", LeftShift),
        DECLARE_NAPI_FUNCTION("rightShift", RightShift),
        DECLARE_NAPI_FUNCTION("unsignedRightShift", UnsignedRightShift),
        DECLARE_NAPI_FUNCTION("popcount", Popcount),
        DECLARE_NAPI_FUNCTION("countLeadingZeros", CountLeadingZeros),
        DECLARE_NAPI_FUNCTION("countTrailingZeros", CountTrailingZeros),
        DECLARE_NAPI_FUNCTION("isPowerOfTwo", IsPowerOfTwo),
        DECLARE_NAPI_FUNCTION("highestSetBit", HighestSetBit),
        DECLARE_NAPI_FUNCTION("lowestSetBit", LowestSetBit),
        DECLARE_NAPI_FUNCTION("bitIsSet", BitIsSet),
        DECLARE_NAPI_FUNCTION("setBit", SetBit),
        DECLARE_NAPI_FUNCTION("clearBit", ClearBit),
        DECLARE_NAPI_FUNCTION("toggleBit", ToggleBit),
        DECLARE_NAPI_FUNCTION("rotateLeft", RotateLeft),
        DECLARE_NAPI_FUNCTION("rotateRight", RotateRight),
        DECLARE_NAPI_FUNCTION("reverseBits", ReverseBits),
        DECLARE_NAPI_FUNCTION("parity", Parity),
        DECLARE_NAPI_FUNCTION("nextPowerOfTwo", NextPowerOfTwo),
        DECLARE_NAPI_FUNCTION("addWithoutCarry", AddWithoutCarry),
        DECLARE_NAPI_FUNCTION("multiplyByPow2", MultiplyByPow2),
        DECLARE_NAPI_FUNCTION("divideByPow2", DivideByPow2),
        DECLARE_NAPI_FUNCTION("isolateLowestOne", IsolateLowestOne),
        DECLARE_NAPI_FUNCTION("clearLowestOne", ClearLowestOne),
        DECLARE_NAPI_FUNCTION("leadingOnes", LeadingOnes),
        DECLARE_NAPI_FUNCTION("trailingOnes", TrailingOnes),
        DECLARE_NAPI_FUNCTION("reverseNibbles", ReverseNibbles),
        DECLARE_NAPI_FUNCTION("averageFloor", AverageFloor),
    };
    NAPI_CALL(env, napi_define_properties(env, exports,
                                          sizeof(desc) / sizeof(desc[0]), desc));
    NAPI_CALL_BASE(env, RegisterBitwiseOpsFunctions(env, exports), nullptr);
    return exports;
}

static napi_module bitwiseSuiteModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "bitwiseSuite",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void BitwiseSuiteRegisterModule(void)
{
    napi_module_register(&bitwiseSuiteModule);
}
