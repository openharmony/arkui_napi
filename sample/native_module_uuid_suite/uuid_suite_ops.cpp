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
#include <cmath>
#include <string>
#include <vector>

#include "napi/native_node_api.h"
#include "uuid_suite_common.h"

napi_value Nanoid(napi_env env, napi_callback_info info)
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

    std::string alphabet = std::string(ALPHABET_LETTERS) + std::string(ALPHABET_DIGITS) +
                           std::string(NANOID_EXTRA_CHARS);
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

napi_value ObjectIdLike(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: seed, timestampMs");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    int64_t timestamp = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], timestamp));

    RandomState state = MakeRandomState(seed);
    std::vector<uint8_t> bytes;
    for (int i = OBJECTID_TIMESTAMP_BYTES - 1; i >= 0; --i) {
        uint8_t byte = static_cast<uint8_t>((timestamp >> (i * BYTE_BITS)) & BYTE_VALUE_MASK);
        bytes.push_back(byte);
    }
    for (int i = 0; i < OBJECTID_RANDOM_BYTES; ++i) {
        uint8_t byte = static_cast<uint8_t>(XorshiftNext(state) & BYTE_VALUE_MASK);
        bytes.push_back(byte);
    }
    uint32_t counter = XorshiftNext(state) & OBJECTID_COUNTER_MASK;
    for (int i = OBJECTID_COUNTER_BYTES - 1; i >= 0; --i) {
        uint8_t byte = static_cast<uint8_t>((counter >> (i * BYTE_BITS)) & BYTE_VALUE_MASK);
        bytes.push_back(byte);
    }
    NAPI_ASSERT(env, static_cast<int>(bytes.size()) == OBJECT_ID_LENGTH, "Object id length mismatch");

    std::string result;
    for (size_t i = 0; i < bytes.size(); ++i) {
        AppendHexByte(result, bytes[i]);
    }

    napi_value out = nullptr;
    NAPI_CALL(env, CreateStringValue(env, result, out));
    return out;
}

napi_value TimeOrderedId(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: seed, timestampMs");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    int64_t timestamp = 0;
    NAPI_CALL(env, ExtractInt64Arg(env, argv[ARG_INDEX_ONE], timestamp));

    RandomState state = MakeRandomState(seed);
    std::vector<uint8_t> bytes;
    for (int i = TIME_ORDERED_TIMESTAMP_BYTES - 1; i >= 0; --i) {
        uint8_t byte = static_cast<uint8_t>((timestamp >> (i * BYTE_BITS)) & BYTE_VALUE_MASK);
        bytes.push_back(byte);
    }
    for (int i = 0; i < TIME_ORDERED_RANDOM_BYTES; ++i) {
        uint8_t byte = static_cast<uint8_t>(XorshiftNext(state) & BYTE_VALUE_MASK);
        bytes.push_back(byte);
    }

    std::string result;
    for (size_t i = 0; i < bytes.size(); ++i) {
        AppendHexByte(result, bytes[i]);
    }

    napi_value out = nullptr;
    NAPI_CALL(env, CreateStringValue(env, result, out));
    return out;
}

napi_value RandomInt(napi_env env, napi_callback_info info)
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

    RandomState state = MakeRandomState(seed);
    double span = static_cast<double>(maxValue - minValue + 1);
    int64_t value = minValue + static_cast<int64_t>(NextUnitUniform(state) * span);
    if (value > maxValue) {
        value = maxValue;
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int64(env, value, &result));
    return result;
}

napi_value RandomRange(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: seed, min, max");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    double minValue = 0.0;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ONE], &minValue));

    double maxValue = 0.0;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_TWO], &maxValue));
    NAPI_ASSERT(env, minValue <= maxValue, "Minimum must not exceed maximum");

    RandomState state = MakeRandomState(seed);
    double value = minValue + NextUnitUniform(state) * (maxValue - minValue);

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_double(env, value, &result));
    return result;
}

napi_value DiceRoll(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: seed, sides");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    int32_t sides = 0;
    NAPI_CALL(env, ExtractInt32Arg(env, argv[ARG_INDEX_ONE], sides));
    NAPI_ASSERT(env, sides >= DICE_MIN_SIDES, "Dice must have at least 2 sides");

    RandomState state = MakeRandomState(seed);
    int32_t value = 1 + static_cast<int32_t>(NextUnitUniform(state) * static_cast<double>(sides));
    if (value > sides) {
        value = sides;
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, value, &result));
    return result;
}

napi_value CoinFlip(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: seed");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    RandomState state = MakeRandomState(seed);
    bool heads = (XorshiftNext(state) % COIN_SIDES) == 0;

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, heads, &result));
    return result;
}

napi_value RandomPick(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: seed, items");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[ARG_INDEX_ONE], &isArray));
    NAPI_ASSERT(env, isArray, "Items must be an array");

    uint32_t length = 0;
    NAPI_CALL(env, napi_get_array_length(env, argv[ARG_INDEX_ONE], &length));
    NAPI_ASSERT(env, length > 0, "Array must not be empty");

    RandomState state = MakeRandomState(seed);
    uint32_t index = static_cast<uint32_t>(NextUnitUniform(state) * static_cast<double>(length));
    if (index >= length) {
        index = length - 1;
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_element(env, argv[ARG_INDEX_ONE], index, &result));
    return result;
}

napi_value ShuffleArray(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: seed, items");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[ARG_INDEX_ONE], &isArray));
    NAPI_ASSERT(env, isArray, "Items must be an array");

    uint32_t length = 0;
    NAPI_CALL(env, napi_get_array_length(env, argv[ARG_INDEX_ONE], &length));

    std::vector<napi_value> items;
    for (uint32_t i = 0; i < length; ++i) {
        napi_value item = nullptr;
        NAPI_CALL(env, napi_get_element(env, argv[ARG_INDEX_ONE], i, &item));
        items.push_back(item);
    }

    RandomState state = MakeRandomState(seed);
    for (size_t i = items.size(); i > 1; --i) {
        size_t swapIndex = static_cast<size_t>(NextUnitUniform(state) * static_cast<double>(i));
        if (swapIndex >= i) {
            swapIndex = i - 1;
        }
        napi_value temp = items[i - 1];
        items[i - 1] = items[swapIndex];
        items[swapIndex] = temp;
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, items.size(), &result));
    for (size_t i = 0; i < items.size(); ++i) {
        NAPI_CALL(env, napi_set_element(env, result, static_cast<uint32_t>(i), items[i]));
    }
    return result;
}

napi_value RandomByte(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_ONE;
    napi_value argv[REQUIRED_ARGS_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_ONE, "Requires 1 argument: seed");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    RandomState state = MakeRandomState(seed);
    uint32_t value = XorshiftNext(state) & BYTE_VALUE_MASK;

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_uint32(env, value, &result));
    return result;
}

napi_value WeightedIndex(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_TWO;
    napi_value argv[REQUIRED_ARGS_TWO] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_TWO, "Requires 2 arguments: seed, weights");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    bool isArray = false;
    NAPI_CALL(env, napi_is_array(env, argv[ARG_INDEX_ONE], &isArray));
    NAPI_ASSERT(env, isArray, "Weights must be an array");

    uint32_t length = 0;
    NAPI_CALL(env, napi_get_array_length(env, argv[ARG_INDEX_ONE], &length));
    NAPI_ASSERT(env, length > 0, "Weights must not be empty");

    std::vector<double> weights;
    double totalWeight = 0.0;
    for (uint32_t i = 0; i < length; ++i) {
        napi_value item = nullptr;
        NAPI_CALL(env, napi_get_element(env, argv[ARG_INDEX_ONE], i, &item));
        double weight = 0.0;
        NAPI_CALL(env, napi_get_value_double(env, item, &weight));
        NAPI_ASSERT(env, weight >= 0.0, "Weights must be non-negative");
        weights.push_back(weight);
        totalWeight += weight;
    }
    NAPI_ASSERT(env, totalWeight > 0.0, "Weight sum must be positive");

    RandomState state = MakeRandomState(seed);
    double remaining = NextUnitUniform(state) * totalWeight;
    uint32_t chosenIndex = length - 1;
    for (uint32_t i = 0; i < length; ++i) {
        remaining -= weights[i];
        if (remaining < 0.0) {
            chosenIndex = i;
            break;
        }
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_uint32(env, chosenIndex, &result));
    return result;
}

napi_value RandomBase58(napi_env env, napi_callback_info info)
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

    std::string alphabet = ALPHABET_BASE58;
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

napi_value RandomGauss(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: seed, mean, stdDev");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    double mean = 0.0;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_ONE], &mean));

    double stdDev = 0.0;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_TWO], &stdDev));
    NAPI_ASSERT(env, stdDev >= 0.0, "Standard deviation must be non-negative");

    RandomState state = MakeRandomState(seed);
    double first = NextUnitUniform(state);
    double second = NextUnitUniform(state);
    double radius = std::sqrt(-GAUSSIAN_BOX_MULLER_TWO * std::log(first));
    double angle = GAUSSIAN_TWO_PI * second;
    double value = mean + stdDev * radius * std::cos(angle);

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_double(env, value, &result));
    return result;
}

napi_value RandomWalk(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: seed, steps, start");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    int32_t steps = 0;
    NAPI_CALL(env, ExtractInt32Arg(env, argv[ARG_INDEX_ONE], steps));
    NAPI_ASSERT(env, steps >= RANDOM_WALK_MIN_STEPS && steps <= RANDOM_WALK_MAX_STEPS,
                "Steps must be within walk bounds");

    double value = 0.0;
    NAPI_CALL(env, napi_get_value_double(env, argv[ARG_INDEX_TWO], &value));

    RandomState state = MakeRandomState(seed);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, static_cast<size_t>(steps), &result));
    for (int32_t i = 0; i < steps; ++i) {
        double unit = NextUnitUniform(state);
        if (unit < RANDOM_WALK_THRESHOLD_LOWER) {
            value -= 1;
        } else if (unit >= RANDOM_WALK_THRESHOLD_UPPER) {
            value += 1;
        }
        napi_value item = nullptr;
        NAPI_CALL(env, napi_create_double(env, value, &item));
        NAPI_CALL(env, napi_set_element(env, result, static_cast<uint32_t>(i), item));
    }
    return result;
}

napi_value RandomStringFrom(napi_env env, napi_callback_info info)
{
    size_t argc = REQUIRED_ARGS_THREE;
    napi_value argv[REQUIRED_ARGS_THREE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= REQUIRED_ARGS_THREE, "Requires 3 arguments: seed, charset, length");

    uint32_t seed = 0;
    NAPI_CALL(env, ExtractUint32Arg(env, argv[ARG_INDEX_ZERO], seed));

    std::string charset;
    NAPI_CALL(env, ExtractStringArg(env, argv[ARG_INDEX_ONE], charset));
    NAPI_ASSERT(env, !charset.empty(), "Charset must not be empty");

    int32_t length = 0;
    NAPI_CALL(env, ExtractInt32Arg(env, argv[ARG_INDEX_TWO], length));
    NAPI_ASSERT(env, length >= SHORT_ID_MIN_LENGTH && length <= SHORT_ID_MAX_LENGTH,
                "Length must be within short id bounds");

    RandomState state = MakeRandomState(seed);
    std::string result;
    for (int32_t i = 0; i < length; ++i) {
        uint32_t index = XorshiftNext(state) % static_cast<uint32_t>(charset.size());
        result.push_back(charset[index]);
    }

    napi_value out = nullptr;
    NAPI_CALL(env, CreateStringValue(env, result, out));
    return out;
}

napi_status RegisterUuidOpsFunctions(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("nanoid", Nanoid),
        DECLARE_NAPI_FUNCTION("objectIdLike", ObjectIdLike),
        DECLARE_NAPI_FUNCTION("timeOrderedId", TimeOrderedId),
        DECLARE_NAPI_FUNCTION("randomInt", RandomInt),
        DECLARE_NAPI_FUNCTION("randomRange", RandomRange),
        DECLARE_NAPI_FUNCTION("diceRoll", DiceRoll),
        DECLARE_NAPI_FUNCTION("coinFlip", CoinFlip),
        DECLARE_NAPI_FUNCTION("randomPick", RandomPick),
        DECLARE_NAPI_FUNCTION("shuffleArray", ShuffleArray),
        DECLARE_NAPI_FUNCTION("randomByte", RandomByte),
        DECLARE_NAPI_FUNCTION("weightedIndex", WeightedIndex),
        DECLARE_NAPI_FUNCTION("randomBase58", RandomBase58),
        DECLARE_NAPI_FUNCTION("randomGauss", RandomGauss),
        DECLARE_NAPI_FUNCTION("randomWalk", RandomWalk),
        DECLARE_NAPI_FUNCTION("randomStringFrom", RandomStringFrom),
    };
    return napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
}
