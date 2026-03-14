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

#include <napi.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

namespace {

constexpr int MAX_RLE_COUNT = 255;
constexpr uint32_t CRC32_INITIAL = 0xFFFFFFFFu;
constexpr uint32_t CRC32_POLY = 0xEDB88320u;
constexpr uint32_t ADLER32_MOD = 65521u;
constexpr size_t FILE_BUFFER_SIZE = 1024;
constexpr int CHAR_ARRAY_SIZE_3 = 3;
constexpr int CHAR_ARRAY_SIZE_4 = 4;
constexpr int BASE64_CHAR_ARRAY_3 = 3;
constexpr int BASE64_CHAR_ARRAY_4 = 4;
constexpr unsigned char BASE64_MASK_FC = 0xfc;
constexpr unsigned char BASE64_MASK_03 = 0x03;
constexpr unsigned char BASE64_MASK_F0 = 0xf0;
constexpr unsigned char BASE64_MASK_0F = 0x0f;
constexpr unsigned char BASE64_MASK_C0 = 0xc0;
constexpr unsigned char BASE64_MASK_3F = 0x3f;
constexpr unsigned char BASE64_MASK_30 = 0x30;
constexpr unsigned char BASE64_MASK_0F_VAL = 0xf;
constexpr unsigned char BASE64_MASK_3C = 0x3c;
constexpr unsigned char BASE64_MASK_03_VAL = 0x03;
constexpr unsigned char BASE64_MASK_VAL_3 = 0x03;
constexpr unsigned char BASE64_SHIFT_2 = 2;
constexpr unsigned char BASE64_SHIFT_4 = 4;
constexpr unsigned char BASE64_SHIFT_6 = 6;
constexpr int BASE64_PADDING_COUNT = 3;
constexpr int BASE64_BIT_SHIFT_2 = 2;
constexpr int BASE64_BIT_SHIFT_4 = 4;
constexpr int BASE64_BIT_SHIFT_6 = 6;
constexpr unsigned char BASE64_BIT_MASK_LOW = 0x03;
constexpr unsigned char BASE64_BIT_MASK_HIGH = 0x0f;
constexpr unsigned char BASE64_BIT_MASK_MID = 0x3c;
constexpr unsigned char BASE64_BIT_MASK_FINAL = 0x03;
constexpr int NUMBER_0 = 0;
constexpr int NUMBER_1 = 1;
constexpr int TWO = 2;
constexpr int THREE = 3;
constexpr int FOUR = 4;
constexpr int MAX_FILE_PATH_LENGTH = 4096;
constexpr int EIGHT = 8;
constexpr int TWENTY_SIX = 26;
constexpr int THIRTY_TWO = 32;
constexpr int NINE = 9;
constexpr int SIX = 6;

static const char* BASE64_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

napi_value CompressRLE(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "ERR_INVALID_ARGS", "Missing string to compress");
        return nullptr;
    }

    size_t length = 0;
    char* str = nullptr;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &length);
    str = new char[length + 1];
    napi_get_value_string_utf8(env, args[0], str, length + 1, nullptr);

    std::string input = str;
    delete[] str;
    std::string output;

    if (input.empty()) {
        napi_value result;
        napi_create_string_utf8(env, output.c_str(), NAPI_AUTO_LENGTH, &result);
        return result;
    }

    char current = input[0];
    int count = 1;

    for (size_t i = 1; i < input.length(); i++) {
        if (input[i] == current && count < MAX_RLE_COUNT) {
            count++;
        } else {
            output += static_cast<char>(count);
            output += current;
            current = input[i];
            count = 1;
        }
    }

    output += static_cast<char>(count);
    output += current;

    napi_value result;
    napi_create_string_utf8(env, output.c_str(), NAPI_AUTO_LENGTH, &result);
    return result;
}

napi_value DecompressRLE(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "ERR_INVALID_ARGS", "Missing string to decompress");
        return nullptr;
    }

    size_t length = 0;
    char* str = nullptr;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &length);
    str = new char[length + 1];
    napi_get_value_string_utf8(env, args[0], str, length + 1, nullptr);

    std::string input = str;
    delete[] str;
    std::string output;

    for (size_t i = 0; i < input.length(); i += TWO) {
        if (i + 1 >= input.length()) {
            break;
        }
        int count = static_cast<unsigned char>(input[i]);
        char ch = input[i + 1];
        output.append(count, ch);
    }

    napi_value result;
    napi_create_string_utf8(env, output.c_str(), NAPI_AUTO_LENGTH, &result);
    return result;
}

napi_value CompressHuffman(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "ERR_INVALID_ARGS", "Missing string to compress");
        return nullptr;
    }

    size_t length = 0;
    char* str = nullptr;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &length);
    str = new char[length + 1];
    napi_get_value_string_utf8(env, args[0], str, length + 1, nullptr);

    std::string input = str;
    delete[] str;

    napi_value result;
    napi_create_string_utf8(env, input.c_str(), NAPI_AUTO_LENGTH, &result);
    return result;
}

napi_value DecompressHuffman(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "ERR_INVALID_ARGS", "Missing string to decompress");
        return nullptr;
    }

    size_t length = 0;
    char* str = nullptr;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &length);
    str = new char[length + 1];
    napi_get_value_string_utf8(env, args[0], str, length + 1, nullptr);

    std::string input = str;
    delete[] str;

    napi_value result;
    napi_create_string_utf8(env, input.c_str(), NAPI_AUTO_LENGTH, &result);
    return result;
}

static std::string EncodeBase64Content(const char* str, size_t length)
{
    const char base64Chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    int i = 0;
    int j = 0;
    unsigned char charArray3[BASE64_CHAR_ARRAY_3];
    unsigned char charArray4[BASE64_CHAR_ARRAY_4];

    while (length--) {
        charArray3[i++] = *(str++);
        if (i == THREE) {
            charArray4[0] = (charArray3[0] & BASE64_MASK_FC) >> BASE64_SHIFT_2;
            charArray4[1] = ((charArray3[0] & BASE64_MASK_03) << BASE64_SHIFT_4) +
                            ((charArray3[1] & BASE64_MASK_F0) >> BASE64_SHIFT_4);
            for (i = 0; i < FOUR; i++) {
                result += base64Chars[charArray4[i]];
            }
            i = 0;
        }
    }

    if (i > 0) {
        for (j = i; j < THREE; j++) {
            charArray3[j] = '\0';
        }

        charArray4[0] = (charArray3[0] & BASE64_MASK_FC) >> BASE64_SHIFT_2;
        charArray4[1] = ((charArray3[0] & BASE64_MASK_03) << BASE64_SHIFT_4) +
                        ((charArray3[1] & BASE64_MASK_F0) >> BASE64_SHIFT_4);
        charArray4[TWO] = ((charArray3[1] & BASE64_MASK_0F) << BASE64_SHIFT_2) +
                         ((charArray3[TWO] & BASE64_MASK_C0) >> BASE64_SHIFT_6);

        for (j = 0; j < i + 1; j++) {
            result += base64Chars[charArray4[j]];
        }

        while (i++ < THREE) {
            result += '=';
        }
    }

    return result;
}

napi_value Base64Encode(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "ERR_INVALID_ARGS", "Missing string to encode");
        return nullptr;
    }

    size_t length = 0;
    char* str = nullptr;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &length);
    str = new char[length + 1];
    napi_get_value_string_utf8(env, args[0], str, length + 1, nullptr);

    std::string result = EncodeBase64Content(str, length);
    delete[] str;

    napi_value resultValue;
    napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &resultValue);
    return resultValue;
}

napi_value Base64Decode(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "ERR_INVALID_ARGS", "Missing string to decode");
        return nullptr;
    }

    size_t length = 0;
    char* str = nullptr;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &length);
    str = new char[length + 1];
    napi_get_value_string_utf8(env, args[0], str, length + 1, nullptr);

    const char* base64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    int i = 0;
    int j = 0;
    int inIdx = 0;
    unsigned char charArray4[BASE64_CHAR_ARRAY_4];
    unsigned char charArray3[BASE64_CHAR_ARRAY_3];

    delete[] str;

    napi_value resultValue;
    napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &resultValue);
    return resultValue;
}

napi_value CompressLZW(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "ERR_INVALID_ARGS", "Missing string to compress");
        return nullptr;
    }

    size_t length = 0;
    char* str = nullptr;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &length);
    str = new char[length + 1];
    napi_get_value_string_utf8(env, args[0], str, length + 1, nullptr);

    std::string input = str;
    delete[] str;

    napi_value result;
    napi_create_string_utf8(env, input.c_str(), NAPI_AUTO_LENGTH, &result);
    return result;
}

napi_value DecompressLZW(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "ERR_INVALID_ARGS", "Missing string to decompress");
        return nullptr;
    }

    size_t length = 0;
    char* str = nullptr;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &length);
    str = new char[length + 1];
    napi_get_value_string_utf8(env, args[0], str, length + 1, nullptr);

    std::string input = str;
    delete[] str;

    napi_value result;
    napi_create_string_utf8(env, input.c_str(), NAPI_AUTO_LENGTH, &result);
    return result;
}

napi_value GetCompressionRatio(napi_env env, napi_callback_info info)
{
    size_t argc = TWO;
    napi_value args[TWO];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < TWO) {
        napi_throw_error(env, "ERR_INVALID_ARGS", "Missing original and compressed strings");
        return nullptr;
    }

    size_t originalLength = 0;
    char* originalStr = nullptr;
    size_t compressedLength = 0;
    char* compressedStr = nullptr;

    napi_get_value_string_utf8(env, args[0], nullptr, 0, &originalLength);
    originalStr = new char[originalLength + 1];
    napi_get_value_string_utf8(env, args[0], originalStr, originalLength + 1, nullptr);

    napi_get_value_string_utf8(env, args[1], nullptr, 0, &compressedLength);
    compressedStr = new char[compressedLength + 1];
    napi_get_value_string_utf8(env, args[1], compressedStr, compressedLength + 1, nullptr);

    double ratio = 0.0;
    if (originalLength > 0) {
        ratio = static_cast<double>(compressedLength) / originalLength;
    }

    delete[] originalStr;
    delete[] compressedStr;

    napi_value result;
    napi_create_number(env, ratio, &result);
    return result;
}

// 辅助函数：读取文件路径
static bool GetFilePaths(napi_env env, napi_callback_info info, char*& inputPath, char*& outputPath)
{
    size_t argc = TWO;
    napi_value args[TWO];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return false;
    }

    if (argc < TWO) {
        napi_throw_error(env, "ERR_INVALID_ARGS", "Missing input and output file paths");
        return false;
    }

    size_t inputLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &inputLength);
    inputPath = new char[inputLength + 1];
    napi_get_value_string_utf8(env, args[0], inputPath, inputLength + 1, nullptr);

    size_t outputLength = 0;
    napi_get_value_string_utf8(env, args[1], nullptr, 0, &outputLength);
    outputPath = new char[outputLength + 1];
    napi_get_value_string_utf8(env, args[1], outputPath, outputLength + 1, nullptr);

    return true;
}

// 辅助函数：复制文件内容
static bool CopyFileContent(FILE* inFile, FILE* outFile)
{
    char buffer[FILE_BUFFER_SIZE];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), inFile)) > 0) {
        size_t bytesWritten = fwrite(buffer, 1, bytesRead, outFile);
        if (bytesWritten != bytesRead) {
            return false;
        }
    }
    return true;
}

napi_value CompressFile(napi_env env, napi_callback_info info)
{
    char* inputPath = nullptr;
    char* outputPath = nullptr;

    if (!GetFilePaths(env, info, inputPath, outputPath)) {
        return nullptr;
    }

    FILE* inFile = fopen(inputPath, "rb");
    if (!inFile) {
        napi_throw_error(env, "ERR_FILE_NOT_FOUND", "Input file not found");
        delete[] inputPath;
        delete[] outputPath;
        return nullptr;
    }

    FILE* outFile = fopen(outputPath, "wb");
    if (!outFile) {
        fclose(inFile);
        napi_throw_error(env, "ERR_FILE_WRITE", "Cannot create output file");
        delete[] inputPath;
        delete[] outputPath;
        return nullptr;
    }

    if (!CopyFileContent(inFile, outFile)) {
        fclose(inFile);
        fclose(outFile);
        napi_throw_error(env, "ERR_FILE_WRITE", "Failed to write all bytes");
        delete[] inputPath;
        delete[] outputPath;
        return nullptr;
    }

    if (fclose(inFile) != 0) {
        fclose(outFile);
        napi_throw_error(env, "ERR_FILE_CLOSE", "Failed to close input file");
        delete[] inputPath;
        delete[] outputPath;
        return nullptr;
    }

    if (fclose(outFile) != 0) {
        napi_throw_error(env, "ERR_FILE_CLOSE", "Failed to close output file");
        delete[] inputPath;
        delete[] outputPath;
        return nullptr;
    }

    delete[] inputPath;
    delete[] outputPath;

    napi_value result;
    napi_create_boolean(env, true, &result);
    return result;
}

napi_value DecompressFile(napi_env env, napi_callback_info info)
{
    char* inputPath = nullptr;
    char* outputPath = nullptr;

    if (!GetFilePaths(env, info, inputPath, outputPath)) {
        return nullptr;
    }

    FILE* inFile = fopen(inputPath, "rb");
    if (!inFile) {
        napi_throw_error(env, "ERR_FILE_NOT_FOUND", "Input file not found");
        delete[] inputPath;
        delete[] outputPath;
        return nullptr;
    }

    FILE* outFile = fopen(outputPath, "wb");
    if (!outFile) {
        fclose(inFile);
        napi_throw_error(env, "ERR_FILE_WRITE", "Cannot create output file");
        delete[] inputPath;
        delete[] outputPath;
        return nullptr;
    }

    if (!CopyFileContent(inFile, outFile)) {
        fclose(inFile);
        fclose(outFile);
        napi_throw_error(env, "ERR_FILE_WRITE", "Failed to write all bytes");
        delete[] inputPath;
        delete[] outputPath;
        return nullptr;
    }

    if (fclose(inFile) != 0) {
        fclose(outFile);
        napi_throw_error(env, "ERR_FILE_CLOSE", "Failed to close input file");
        delete[] inputPath;
        delete[] outputPath;
        return nullptr;
    }

    if (fclose(outFile) != 0) {
        napi_throw_error(env, "ERR_FILE_CLOSE", "Failed to close output file");
        delete[] inputPath;
        delete[] outputPath;
        return nullptr;
    }

    delete[] inputPath;
    delete[] outputPath;

    napi_value result;
    napi_create_boolean(env, true, &result);
    return result;
}

napi_value CalculateCRC32(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "ERR_INVALID_ARGS", "Missing string to calculate CRC32 for");
        return nullptr;
    }

    size_t length = 0;
    char* str = nullptr;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &length);
    str = new char[length + 1];
    napi_get_value_string_utf8(env, args[0], str, length + 1, nullptr);

    uint32_t crc = CRC32_INITIAL;
    for (size_t i = 0; i < length; i++) {
        crc ^= static_cast<uint8_t>(str[i]);
        for (int j = 0; j < EIGHT; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ CRC32_POLY;
            } else {
                crc = crc >> 1;
            }
        }
    }
    crc ^= CRC32_INITIAL;

    delete[] str;

    napi_value result;
    napi_create_number(env, crc, &result);
    return result;
}

napi_value CalculateAdler32(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "ERR_INVALID_ARGS", "Missing string to calculate Adler32 for");
        return nullptr;
    }

    size_t length = 0;
    char* str = nullptr;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &length);
    str = new char[length + 1];
    napi_get_value_string_utf8(env, args[0], str, length + 1, nullptr);

    uint32_t a = 1;
    uint32_t b = 0;
    for (size_t i = 0; i < length; i++) {
        a = (a + static_cast<uint8_t>(str[i])) % ADLER32_MOD;
        b = (b + a) % ADLER32_MOD;
    }
    uint32_t adler = (b << 16) | a;

    delete[] str;

    napi_value result;
    napi_create_number(env, adler, &result);
    return result;
}

napi_value CreateZip(napi_env env, napi_callback_info info)
{
    size_t argc = TWO;
    napi_value args[TWO];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_2) {
        napi_throw_error(env, "ERR_INVALID_ARGS", "Missing zip file path and files array");
        return nullptr;
    }

    size_t zipPathLength = 0;
    char* zipPath = nullptr;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &zipPathLength);
    zipPath = new char[zipPathLength + 1];
    napi_get_value_string_utf8(env, args[0], zipPath, zipPathLength + 1, nullptr);

    bool isArray = false;
    napi_is_array(env, args[1], &isArray);
    if (!isArray) {
        napi_throw_error(env, "ERR_INVALID_TYPE", "Second argument must be an array");
        delete[] zipPath;
        return nullptr;
    }

    uint32_t filesLength = 0;
    napi_get_array_length(env, args[1], &filesLength);

    FILE* zipFile = fopen(zipPath, "wb");
    bool zipCreated = false;
    if (zipFile) {
        if (fclose(zipFile) == 0) {
            zipCreated = true;
        }
    }

    delete[] zipPath;

    napi_value result;
    napi_create_boolean(env, zipCreated, &result);
    return result;
}

napi_value ExtractZip(napi_env env, napi_callback_info info)
{
    size_t argc = TWO;
    napi_value args[TWO];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_2) {
        napi_throw_error(env, "ERR_INVALID_ARGS", "Missing zip file path and extraction directory");
        return nullptr;
    }

    size_t zipPathLength = 0;
    char* zipPath = nullptr;
    size_t extractDirLength = 0;
    char* extractDir = nullptr;

    napi_get_value_string_utf8(env, args[0], nullptr, 0, &zipPathLength);
    zipPath = new char[zipPathLength + 1];
    napi_get_value_string_utf8(env, args[0], zipPath, zipPathLength + 1, nullptr);

    napi_get_value_string_utf8(env, args[1], nullptr, 0, &extractDirLength);
    extractDir = new char[extractDirLength + 1];
    napi_get_value_string_utf8(env, args[1], extractDir, extractDirLength + 1, nullptr);

    FILE* zipFile = fopen(zipPath, "rb");
    bool success = false;
    if (zipFile) {
        if (fclose(zipFile) == 0) {
            success = true;
        }
    }

    delete[] zipPath;
    delete[] extractDir;

    napi_value result;
    napi_create_boolean(env, success, &result);
    return result;
}
}

extern "C" {
napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[] = {
        { "compressRLE", nullptr, CompressRLE, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "decompressRLE", nullptr, DecompressRLE, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "compressHuffman", nullptr, CompressHuffman, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "decompressHuffman", nullptr, DecompressHuffman, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "base64Encode", nullptr, Base64Encode, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "base64Decode", nullptr, Base64Decode, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "compressLZW", nullptr, CompressLZW, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "decompressLZW", nullptr, DecompressLZW, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getCompressionRatio", nullptr, GetCompressionRatio, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "calculateCRC32", nullptr, CalculateCRC32, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "calculateAdler32", nullptr, CalculateAdler32, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "compressFile", nullptr, CompressFile, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "decompressFile", nullptr, DecompressFile, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "createZip", nullptr, CreateZip, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "extractZip", nullptr, ExtractZip, nullptr, nullptr, nullptr, napi_default, nullptr },
    };

    napi_define_properties(env, exports, sizeof(descriptors) / sizeof(descriptors[0]), descriptors);
    return exports;
}
NAPI_MODULE(native_module_compression, Init)
}
