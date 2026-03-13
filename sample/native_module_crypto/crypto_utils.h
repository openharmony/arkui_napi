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

#ifndef FOUNDATION_ACE_NAPI_SAMPLE_NATIVE_MODULE_CRYPTO_CRYPTO_UTILS_H
#define FOUNDATION_ACE_NAPI_SAMPLE_NATIVE_MODULE_CRYPTO_CRYPTO_UTILS_H

#include <string>
#include <cstdint>

namespace CryptoUtils {

std::string Base64Encode(const std::string& input);
std::string Base64Decode(const std::string& input);
std::string HexEncode(const std::string& input);
std::string HexDecode(const std::string& input);
std::string XorEncrypt(const std::string& input, const std::string& key);
std::string XorDecrypt(const std::string& input, const std::string& key);
std::string SimpleHash(const std::string& input);
std::string Rot13(const std::string& input);
std::string ReverseString(const std::string& input);
uint32_t Crc32(const std::string& input);
std::string CaesarEncrypt(const std::string& input, int shift);
std::string CaesarDecrypt(const std::string& input, int shift);
std::string VigenereEncrypt(const std::string& input, const std::string& key);
std::string VigenereDecrypt(const std::string& input, const std::string& key);
std::string RandomString(size_t length);
std::string PasswordHash(const std::string& password, const std::string& salt);
bool PasswordVerify(const std::string& password, const std::string& salt, const std::string& hash);
std::string SimpleMd5(const std::string& input);

}

#endif