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

#ifndef SAMPLE_NATIVE_MODULE_BITWISE_SUITE_COMMON_H
#define SAMPLE_NATIVE_MODULE_BITWISE_SUITE_COMMON_H

#include <cstdint>
#include <string>

#include "napi/native_api.h"

constexpr int REQUIRED_ARGS_ONE = 1;
constexpr int REQUIRED_ARGS_TWO = 2;
constexpr int REQUIRED_ARGS_THREE = 3;
constexpr int ARG_INDEX_ZERO = 0;
constexpr int ARG_INDEX_ONE = 1;
constexpr int ARG_INDEX_TWO = 2;
constexpr int UINT32_BITS = 32;
constexpr int HALF_UINT32_BITS = 16;
constexpr int BYTE_BITS = 8;
constexpr int NIBBLE_BITS = 4;
constexpr int HEX_BASE = 16;
constexpr int MAX_SHIFT = 31;
constexpr int MAX_BIT_INDEX = 31;
constexpr int SIGN_BIT_POSITION = 31;
constexpr int ROTATE_MASK = 31;
constexpr int BINARY_STRING_LENGTH = 32;
constexpr int HEX_STRING_LENGTH = 8;
constexpr int BYTE_SHIFT_THREE = 24;
constexpr int INTERLEAVE_PAIR_STRIDE = 2;
constexpr uint32_t UINT32_MAX_VALUE = 4294967295u;
constexpr uint32_t BIT_MASK_FULL = UINT32_MAX_VALUE;
constexpr uint32_t NIBBLE_MASK = 0xFu;
constexpr uint32_t BYTE_MASK_ZERO = 0x000000FFu;
constexpr uint32_t BYTE_MASK_ONE = 0x0000FF00u;
constexpr uint32_t BYTE_MASK_TWO = 0x00FF0000u;
constexpr uint32_t BYTE_MASK_THREE = 0xFF000000u;
constexpr uint32_t LOW_TWO_BYTES_MASK = 0x0000FFFFu;
constexpr uint32_t HIGH_BYTE_OF_WORD_MASK = 0x0000FF00u;
constexpr uint32_t EVEN_BITS_MASK = 0x55555555u;
constexpr uint32_t ODD_BITS_MASK = 0xAAAAAAAAu;
constexpr char BINARY_CHAR_ZERO = '0';
constexpr char BINARY_CHAR_ONE = '1';
constexpr char HEX_DIGITS[] = "0123456789ABCDEF";

uint32_t ExtractUint32Arg(napi_env env, napi_value value);
int32_t ExtractInt32Arg(napi_env env, napi_value value);
napi_value CreateUint32Value(napi_env env, uint32_t value);
napi_value CreateInt32Value(napi_env env, int32_t value);
napi_value CreateBoolValue(napi_env env, bool value);
napi_value CreateStringValue(napi_env env, const std::string& value);

napi_value And(napi_env env, napi_callback_info info);
napi_value Or(napi_env env, napi_callback_info info);
napi_value Xor(napi_env env, napi_callback_info info);
napi_value Not(napi_env env, napi_callback_info info);
napi_value LeftShift(napi_env env, napi_callback_info info);
napi_value RightShift(napi_env env, napi_callback_info info);
napi_value UnsignedRightShift(napi_env env, napi_callback_info info);
napi_value Popcount(napi_env env, napi_callback_info info);
napi_value CountLeadingZeros(napi_env env, napi_callback_info info);
napi_value CountTrailingZeros(napi_env env, napi_callback_info info);
napi_value IsPowerOfTwo(napi_env env, napi_callback_info info);
napi_value HighestSetBit(napi_env env, napi_callback_info info);
napi_value LowestSetBit(napi_env env, napi_callback_info info);
napi_value BitIsSet(napi_env env, napi_callback_info info);
napi_value SetBit(napi_env env, napi_callback_info info);
napi_value ClearBit(napi_env env, napi_callback_info info);
napi_value ToggleBit(napi_env env, napi_callback_info info);
napi_value RotateLeft(napi_env env, napi_callback_info info);
napi_value RotateRight(napi_env env, napi_callback_info info);
napi_value ReverseBits(napi_env env, napi_callback_info info);
napi_value Parity(napi_env env, napi_callback_info info);
napi_value NextPowerOfTwo(napi_env env, napi_callback_info info);
napi_value AddWithoutCarry(napi_env env, napi_callback_info info);
napi_value MultiplyByPow2(napi_env env, napi_callback_info info);
napi_value DivideByPow2(napi_env env, napi_callback_info info);
napi_value IsolateLowestOne(napi_env env, napi_callback_info info);
napi_value ClearLowestOne(napi_env env, napi_callback_info info);
napi_value LeadingOnes(napi_env env, napi_callback_info info);
napi_value TrailingOnes(napi_env env, napi_callback_info info);
napi_value ReverseNibbles(napi_env env, napi_callback_info info);
napi_value AverageFloor(napi_env env, napi_callback_info info);

napi_value GrayEncode(napi_env env, napi_callback_info info);
napi_value GrayDecode(napi_env env, napi_callback_info info);
napi_value ByteSwap32(napi_env env, napi_callback_info info);
napi_value ReverseByteOrder16(napi_env env, napi_callback_info info);
napi_value HammingDistance(napi_env env, napi_callback_info info);
napi_value MaskLowBits(napi_env env, napi_callback_info info);
napi_value MaskHighBits(napi_env env, napi_callback_info info);
napi_value ToBinaryString(napi_env env, napi_callback_info info);
napi_value ToHexString(napi_env env, napi_callback_info info);
napi_value BitInterleave(napi_env env, napi_callback_info info);
napi_value BitDeinterleaveEven(napi_env env, napi_callback_info info);
napi_value BitDeinterleaveOdd(napi_env env, napi_callback_info info);
napi_value SignBit(napi_env env, napi_callback_info info);
napi_value CountBitsRange(napi_env env, napi_callback_info info);
napi_value IsBitmaskPalindrome(napi_env env, napi_callback_info info);
napi_value SwapAdjacentBits(napi_env env, napi_callback_info info);
napi_value ExtractBits(napi_env env, napi_callback_info info);
napi_value BitsRequired(napi_env env, napi_callback_info info);
napi_value Pow2(napi_env env, napi_callback_info info);
napi_value Int32ToUint32(napi_env env, napi_callback_info info);
napi_value Uint32ToInt32(napi_env env, napi_callback_info info);
napi_value HasSameSign(napi_env env, napi_callback_info info);

napi_status RegisterBitwiseOpsFunctions(napi_env env, napi_value exports);

#endif  // SAMPLE_NATIVE_MODULE_BITWISE_SUITE_COMMON_H
