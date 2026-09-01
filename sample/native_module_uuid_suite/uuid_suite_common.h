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

#ifndef SAMPLE_NATIVE_MODULE_UUID_SUITE_COMMON_H
#define SAMPLE_NATIVE_MODULE_UUID_SUITE_COMMON_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "napi/native_api.h"

struct RandomState {
    uint32_t value;
};

constexpr int REQUIRED_ARGS_ONE = 1;
constexpr int REQUIRED_ARGS_TWO = 2;
constexpr int REQUIRED_ARGS_THREE = 3;
constexpr int ARG_INDEX_ZERO = 0;
constexpr int ARG_INDEX_ONE = 1;
constexpr int ARG_INDEX_TWO = 2;

constexpr int XORSHIFT_SHIFT_ONE = 13;
constexpr int XORSHIFT_SHIFT_TWO = 17;
constexpr int XORSHIFT_SHIFT_THREE = 5;
constexpr uint32_t XORSHIFT_INITIAL_STATE = 2463534242u;
constexpr double UNIFORM_DIVISOR = 4294967296.0;

constexpr int UUID_BYTE_LENGTH = 16;
constexpr size_t UUID_STRING_LENGTH = 36;
constexpr int UUID_HYPHEN_COUNT = 4;
constexpr size_t UUID_HYPHEN_POS_FIRST = 8;
constexpr size_t UUID_HYPHEN_POS_SECOND = 13;
constexpr size_t UUID_HYPHEN_POS_THIRD = 18;
constexpr size_t UUID_HYPHEN_POS_FOURTH = 23;
constexpr size_t UUID_VERSION_CHAR_INDEX = 14;
constexpr size_t UUID_VARIANT_CHAR_INDEX = 19;
constexpr uint8_t UUID_V4_CLOCK_SEQ_HI_BITS = 0x40u;
constexpr uint8_t UUID_V4_CLOCK_SEQ_HI_MASK = 0x3Fu;
constexpr uint8_t UUID_V4_CLOCK_SEQ_LOW_MASK = 0x80u;
constexpr uint8_t UUID_VERSION_NIBBLE_MASK = 0x0Fu;
constexpr int UUID_VERSION_BYTE_INDEX = 6;
constexpr int UUID_VARIANT_BYTE_INDEX = 8;
constexpr int UUID_VARIANT_SHIFT = 6;
constexpr int UUID_MIN_VERSION = 1;
constexpr int UUID_MAX_VERSION = 5;

constexpr int HEX_DIGITS_PER_BYTE = 2;
constexpr uint32_t HEX_BASE = 16u;
constexpr uint8_t HEX_NIBBLE_MASK = 0x0Fu;
constexpr int HEX_HIGH_SHIFT = 4;
constexpr uint8_t BYTE_VALUE_MASK = 0xFFu;
constexpr int BYTE_BITS = 8;
constexpr char CHAR_HYPHEN = '-';
const char* const HEX_DIGITS = "0123456789abcdef";
const char* const HEX_DIGITS_UPPER = "0123456789ABCDEF";
const char* const UUID_NIL = "00000000-0000-0000-0000-000000000000";
const char* const UUID_MAX = "ffffffff-ffff-ffff-ffff-ffffffffffff";
const char* const ALPHABET_LETTERS = "abcdefghijklmnopqrstuvwxyz";
const char* const ALPHABET_UPPER = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char* const ALPHABET_DIGITS = "0123456789";
const char* const ALPHABET_BASE32 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
const char* const ALPHABET_BASE58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
const char* const NANOID_EXTRA_CHARS = "-_";

constexpr int SHORT_ID_MIN_LENGTH = 1;
constexpr int SHORT_ID_MAX_LENGTH = 64;
constexpr int OBJECT_ID_LENGTH = 12;
constexpr int OBJECTID_TIMESTAMP_BYTES = 4;
constexpr int OBJECTID_RANDOM_BYTES = 5;
constexpr int OBJECTID_COUNTER_BYTES = 3;
constexpr uint32_t OBJECTID_COUNTER_MASK = 0xFFFFFFu;
constexpr int TIME_ORDERED_TIMESTAMP_BYTES = 8;
constexpr int TIME_ORDERED_RANDOM_BYTES = 8;

constexpr double GAUSSIAN_BOX_MULLER_TWO = 2.0;
constexpr double GAUSSIAN_TWO_PI = 6.283185307179586;
constexpr int DICE_MIN_SIDES = 2;
constexpr int COIN_SIDES = 2;
constexpr int CHAR_INVALID_VALUE = -1;
constexpr int PARITY_MODULUS = 2;
constexpr int PARITY_EVEN_REMAINDER = 0;
constexpr int PARITY_ODD_REMAINDER = 1;
constexpr double RANDOM_WALK_THRESHOLD_LOWER = 1.0 / 3.0;
constexpr double RANDOM_WALK_THRESHOLD_UPPER = 2.0 / 3.0;
constexpr int RANDOM_WALK_MIN_STEPS = 1;
constexpr int RANDOM_WALK_MAX_STEPS = 1024;

uint32_t XorshiftNext(RandomState& state);
double NextUnitUniform(RandomState& state);
RandomState MakeRandomState(uint32_t seed);
void AppendHexByte(std::string& out, uint8_t value);
int CharToValue(char ch, bool& valid);
void IsHexString(const std::string& text, bool& valid);
napi_status ExtractUint32Arg(napi_env env, napi_value value, uint32_t& result);
napi_status ExtractInt32Arg(napi_env env, napi_value value, int32_t& result);
napi_status ExtractInt64Arg(napi_env env, napi_value value, int64_t& result);
napi_status ExtractStringArg(napi_env env, napi_value value, std::string& result);
napi_status CreateStringValue(napi_env env, const std::string& text, napi_value& result);

napi_value UuidV4(napi_env env, napi_callback_info info);
napi_value UuidV4Upper(napi_env env, napi_callback_info info);
napi_value UuidNil(napi_env env, napi_callback_info info);
napi_value UuidMax(napi_env env, napi_callback_info info);
napi_value IsNil(napi_env env, napi_callback_info info);
napi_value IsUuid(napi_env env, napi_callback_info info);
napi_value UuidVersion(napi_env env, napi_callback_info info);
napi_value UuidVariant(napi_env env, napi_callback_info info);
napi_value UuidCompare(napi_env env, napi_callback_info info);
napi_value ShortId(napi_env env, napi_callback_info info);
napi_value RandomHex(napi_env env, napi_callback_info info);
napi_value RandomDigits(napi_env env, napi_callback_info info);
napi_value RandomAlpha(napi_env env, napi_callback_info info);
napi_value RandomAlphaNumeric(napi_env env, napi_callback_info info);
napi_value RandomBase32(napi_env env, napi_callback_info info);
napi_value Nanoid(napi_env env, napi_callback_info info);
napi_value ObjectIdLike(napi_env env, napi_callback_info info);
napi_value TimeOrderedId(napi_env env, napi_callback_info info);
napi_value RandomInt(napi_env env, napi_callback_info info);
napi_value RandomRange(napi_env env, napi_callback_info info);
napi_value DiceRoll(napi_env env, napi_callback_info info);
napi_value CoinFlip(napi_env env, napi_callback_info info);
napi_value RandomPick(napi_env env, napi_callback_info info);
napi_value ShuffleArray(napi_env env, napi_callback_info info);
napi_value RandomByte(napi_env env, napi_callback_info info);
napi_value WeightedIndex(napi_env env, napi_callback_info info);
napi_value RandomBase58(napi_env env, napi_callback_info info);
napi_value RandomGauss(napi_env env, napi_callback_info info);
napi_value RandomWalk(napi_env env, napi_callback_info info);
napi_value RandomStringFrom(napi_env env, napi_callback_info info);
napi_value RandomBytesHex(napi_env env, napi_callback_info info);
napi_value RandomEven(napi_env env, napi_callback_info info);
napi_value RandomOdd(napi_env env, napi_callback_info info);
napi_value RandomLetter(napi_env env, napi_callback_info info);

napi_status RegisterUuidOpsFunctions(napi_env env, napi_value exports);

#endif
