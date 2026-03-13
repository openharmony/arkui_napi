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

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <cstdio>
#include <cstring>
#include "native_api.h"

namespace fs = std::filesystem;

namespace {

constexpr int DIR_PERMISSIONS = 0755;
constexpr int TEMP_FILE_SIZE = 16;
constexpr int TEMP_DIR_SIZE = 16;
constexpr char TEMP_FILE_TEMPLATE[] = "/tmp/temp_XXXXXX";
constexpr char TEMP_DIR_TEMPLATE[] = "/tmp/temp_XXXXXX";
constexpr int CURRENT_DIR_INDICATOR = 0;
constexpr int PARENT_DIR_INDICATOR = 1;
constexpr int DEFAULT_BUFFER_SIZE = 256;
constexpr int NUMBER_0 = 0;
constexpr int NUMBER_1 = 1;
constexpr int NUMBER_2 = 2;
constexpr int NUMBER_3 = 3;

std::string ReadFile(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

void WriteFile(const std::string& path, const std::string& content)
{
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file for writing");
    }

    file.write(content.c_str(), content.size());
    if (!file) {
        throw std::runtime_error("Failed to write to file");
    }
}

void AppendFile(const std::string& path, const std::string& content)
{
    std::ofstream file(path, std::ios::binary | std::ios::app);
    if (!file) {
        throw std::runtime_error("Failed to open file for appending");
    }

    file.write(content.c_str(), content.size());
    if (!file) {
        throw std::runtime_error("Failed to append to file");
    }
}

void DeleteFile(const std::string& path)
{
    if (std::remove(path.c_str()) != 0) {
        throw std::runtime_error("Failed to delete file");
    }
}

void CopyFile(const std::string& src, const std::string& dest)
{
    std::ifstream srcFile(src, std::ios::binary);
    if (!srcFile) {
        throw std::runtime_error("Failed to open source file");
    }

    std::ofstream destFile(dest, std::ios::binary);
    if (!destFile) {
        throw std::runtime_error("Failed to open destination file");
    }

    destFile << srcFile.rdbuf();
    if (!destFile) {
        throw std::runtime_error("Failed to copy file");
    }
}

void MoveFile(const std::string& src, const std::string& dest)
{
    if (std::rename(src.c_str(), dest.c_str()) != 0) {
        CopyFile(src, dest);
        DeleteFile(src);
    }
}

void RenameFile(const std::string& oldPath, const std::string& newPath)
{
    if (std::rename(oldPath.c_str(), newPath.c_str()) != 0) {
        throw std::runtime_error("Failed to rename file");
    }
}

void CreateDirectory(const std::string& path)
{
    if (mkdir(path.c_str(), DIR_PERMISSIONS) != 0) {
        throw std::runtime_error("Failed to create directory");
    }
}

void DeleteDirectory(const std::string& path)
{
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        throw std::runtime_error("Failed to open directory");
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == CURRENT_DIR_INDICATOR ||
            strcmp(entry->d_name, "..") == PARENT_DIR_INDICATOR) {
            continue;
        }

        std::string entryPath = path + "/" + entry->d_name;
        if (entry->d_type == DT_DIR) {
            DeleteDirectory(entryPath);
        } else {
            DeleteFile(entryPath);
        }
    }

    int closeResult = closedir(dir);
    if (closeResult != 0) {
    }

    if (rmdir(path.c_str()) != 0) {
        throw std::runtime_error("Failed to remove directory");
    }
}

std::vector<std::string> ListDirectory(const std::string& path)
{
    std::vector<std::string> entries;
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        throw std::runtime_error("Failed to open directory");
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") != CURRENT_DIR_INDICATOR &&
            strcmp(entry->d_name, "..") != PARENT_DIR_INDICATOR) {
            entries.push_back(entry->d_name);
        }
    }

    int closeResult = closedir(dir);
    if (closeResult != 0) {
    }

    return entries;
}

bool FileExists(const std::string& path)
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool IsDirectory(const std::string& path)
{
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0) {
        return false;
    }
    return S_ISDIR(buffer.st_mode);
}

bool IsFile(const std::string& path)
{
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0) {
        return false;
    }
    return S_ISREG(buffer.st_mode);
}

off_t GetFileSize(const std::string& path)
{
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0) {
        throw std::runtime_error("Failed to get file size");
    }
    return buffer.st_size;
}

mode_t GetFilePermissions(const std::string& path)
{
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0) {
        throw std::runtime_error("Failed to get file permissions");
    }
    return buffer.st_mode;
}

void SetFilePermissions(const std::string& path, mode_t mode)
{
    if (chmod(path.c_str(), mode) != 0) {
        throw std::runtime_error("Failed to set file permissions");
    }
}

std::string JoinPath(const std::string& path1, const std::string& path2)
{
    return fs::path(path1) / fs::path(path2);
}

std::string Basename(const std::string& path)
{
    return fs::path(path).filename().string();
}

std::string Dirname(const std::string& path)
{
    return fs::path(path).parent_path().string();
}

std::string Extname(const std::string& path)
{
    return fs::path(path).extension().string();
}

std::string NormalizePath(const std::string& path)
{
    return fs::canonical(path).string();
}

std::string AbsolutePath(const std::string& path)
{
    return fs::absolute(path).string();
}

std::string CreateTempFile()
{
    char tempName[TEMP_FILE_SIZE];
    std::string templateStr = TEMP_FILE_TEMPLATE;
    size_t copyLen = templateStr.length();
    if (copyLen >= TEMP_FILE_SIZE) {
        copyLen = TEMP_FILE_SIZE - 1;
    }
    for (size_t i = 0; i < copyLen; i++) {
        tempName[i] = templateStr[i];
    }
    tempName[copyLen] = '\0';
    int fd = mkstemp(tempName);
    if (fd == -1) {
        throw std::runtime_error("Failed to create temporary file");
    }
    int closeResult = close(fd);
    if (closeResult != 0) {
    }
    return tempName;
}

std::string GetStringFromNapiValue(napi_env env, napi_value value
{
    size_t length = 0;
    napi_status status = napi_get_value_string_utf8(env, value, nullptr, 0, &length);
    if (status != napi_ok) {
        throw std::runtime_error("Failed to get string length");
    }
    char* buffer = new char[length + 1];
    status = napi_get_value_string_utf8(env, value, buffer, length + 1, nullptr);
    if (status != napi_ok) {
        delete[] buffer;
        throw std::runtime_error("Failed to get string value");
    }
    std::string result(buffer);
    delete[] buffer;
    return result;
}

napi_value CreateArrayFromVector(napi_env env, const std::vector<std::string>& entries)
{
    napi_value result;
    napi_status status = napi_create_array(env, &result);
    if (status != napi_ok) {
        throw std::runtime_error("Failed to create array");
    }

    for (size_t i = 0; i < entries.size(); i++) {
        napi_value entry;
        status = napi_create_string_utf8(env, entries[i].c_str(), NAPI_AUTO_LENGTH, &entry);
        if (status != napi_ok) {
            throw std::runtime_error("Failed to create string");
        }
        status = napi_set_element(env, result, i, entry);
        if (status != napi_ok) {
            throw std::runtime_error("Failed to set element");
        }
    }

    return result;
}

std::string CreateTempDirectory()
{
    char tempName[TEMP_DIR_SIZE];
    std::string templateStr = TEMP_DIR_TEMPLATE;
    size_t copyLen = templateStr.length();
    if (copyLen &gt;= TEMP_DIR_SIZE) {
        copyLen = TEMP_DIR_SIZE - 1;
    }
    for (size_t i = 0; i < copyLen; i++) {
        tempName[i] = templateStr[i];
    }
    tempName[copyLen] = '\0';
    char* dir = mkdtemp(tempName);
    if (dir == nullptr) {
        throw std::runtime_error("Failed to create temporary directory");
    }
    return dir;
}

off_t GetFreeSpace(const std::string& path)
{
    struct statvfs buf;
    if (statvfs(path.c_str(), &buf) != 0) {
        throw std::runtime_error("Failed to get free space");
    }
    return buf.f_bavail * buf.f_frsize;
}

off_t GetTotalSpace(const std::string& path)
{
    struct statvfs buf;
    if (statvfs(path.c_str(), &buf) != 0) {
        throw std::runtime_error("Failed to get total space");
    }
    return buf.f_blocks * buf.f_frsize;
}

void TruncateFile(const std::string& path, off_t length)
{
    if (truncate(path.c_str(), length) != 0) {
        throw std::runtime_error("Failed to truncate file");
    }
}

void ChmodFile(const std::string& path, mode_t mode)
{
    if (chmod(path.c_str(), mode) != 0) {
        throw std::runtime_error("Failed to change file permissions");
    }
}

void ChownFile(const std::string& path, uid_t owner, gid_t group)
{
    if (chown(path.c_str(), owner, group) != 0) {
        throw std::runtime_error("Failed to change file owner");
    }
}

}

static napi_value ReadFile(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected file path");
        return nullptr;
    }

    size_t pathLength = 0;
    status = napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get string length");
        return nullptr;
    }
    char* pathBuffer = new char[pathLength + 1];
    status = napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    if (status != napi_ok) {
        delete[] pathBuffer;
        napi_throw_error(env, "NAPI_ERROR", "Failed to get string value");
        return nullptr;
    }
    std::string path(pathBuffer);
    delete[] pathBuffer;

    try {
        std::string content = ReadFile(path);
        napi_value result;
        status = napi_create_string_utf8(env, content.c_str(), NAPI_AUTO_LENGTH, &result);
        if (status != napi_ok) {
            napi_throw_error(env, "NAPI_ERROR", "Failed to create string");
            return nullptr;
        }
        return result;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_status GetStringArg(napi_env env, napi_value arg, std::string& result)
{
    size_t length = 0;
    napi_status status = napi_get_value_string_utf8(env, arg, nullptr, 0, &length);
    if (status != napi_ok) {
        return status;
    }
    char* buffer = new char[length + 1];
    status = napi_get_value_string_utf8(env, arg, buffer, length + 1, nullptr);
    if (status != napi_ok) {
        delete[] buffer;
        return status;
    }
    result = std::string(buffer);
    delete[] buffer;
    return napi_ok;
}

static napi_value WriteFile(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_2) {
        napi_throw_error(env, "Invalid arguments", "Expected file path and content");
        return nullptr;
    }

    std::string path;
    status = GetStringArg(env, args[0], path);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get path");
        return nullptr;
    }

    std::string content;
    status = GetStringArg(env, args[1], content);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get content");
        return nullptr;
    }

    try {
        WriteFile(path, content);
        napi_value result;
        status = napi_get_undefined(env, &result);
        if (status != napi_ok) {
            napi_throw_error(env, "NAPI_ERROR", "Failed to get undefined");
            return nullptr;
        }
        return result;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value AppendFile(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_2) {
        napi_throw_error(env, "Invalid arguments", "Expected file path and content");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    size_t contentLength = 0;
    napi_get_value_string_utf8(env, args[1], nullptr, 0, &contentLength);
    char* contentBuffer = new char[contentLength + 1];
    napi_get_value_string_utf8(env, args[1], contentBuffer, contentLength + 1, nullptr);
    std::string content(contentBuffer);
    delete[] contentBuffer;

    try {
        AppendFile(path, content);
        napi_value result;
        napi_get_undefined(env, &result);
        return result;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value DeleteFile(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected file path");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    try {
        DeleteFile(path);
        napi_value result;
        napi_get_undefined(env, &result);
        return result;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value CopyFile(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_2) {
        napi_throw_error(env, "Invalid arguments", "Expected source and destination paths");
        return nullptr;
    }

    size_t srcLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &srcLength);
    char* srcBuffer = new char[srcLength + 1];
    napi_get_value_string_utf8(env, args[0], srcBuffer, srcLength + 1, nullptr);
    std::string src(srcBuffer);
    delete[] srcBuffer;

    size_t destLength = 0;
    napi_get_value_string_utf8(env, args[1], nullptr, 0, &destLength);
    char* destBuffer = new char[destLength + 1];
    napi_get_value_string_utf8(env, args[1], destBuffer, destLength + 1, nullptr);
    std::string dest(destBuffer);
    delete[] destBuffer;

    try {
        CopyFile(src, dest);
        napi_value result;
        napi_get_undefined(env, &result);
        return result;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value MoveFile(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_2) {
        napi_throw_error(env, "Invalid arguments", "Expected source and destination paths");
        return nullptr;
    }

    size_t srcLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &srcLength);
    char* srcBuffer = new char[srcLength + 1];
    napi_get_value_string_utf8(env, args[0], srcBuffer, srcLength + 1, nullptr);
    std::string src(srcBuffer);
    delete[] srcBuffer;

    size_t destLength = 0;
    napi_get_value_string_utf8(env, args[1], nullptr, 0, &destLength);
    char* destBuffer = new char[destLength + 1];
    napi_get_value_string_utf8(env, args[1], destBuffer, destLength + 1, nullptr);
    std::string dest(destBuffer);
    delete[] destBuffer;

    try {
        MoveFile(src, dest);
        napi_value result;
        napi_get_undefined(env, &result);
        return result;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value RenameFile(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_2) {
        napi_throw_error(env, "Invalid arguments", "Expected old and new paths");
        return nullptr;
    }

    size_t oldPathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &oldPathLength);
    char* oldPathBuffer = new char[oldPathLength + 1];
    napi_get_value_string_utf8(env, args[0], oldPathBuffer, oldPathLength + 1, nullptr);
    std::string oldPath(oldPathBuffer);
    delete[] oldPathBuffer;

    size_t newPathLength = 0;
    napi_get_value_string_utf8(env, args[1], nullptr, 0, &newPathLength);
    char* newPathBuffer = new char[newPathLength + 1];
    napi_get_value_string_utf8(env, args[1], newPathBuffer, newPathLength + 1, nullptr);
    std::string newPath(newPathBuffer);
    delete[] newPathBuffer;

    try {
        RenameFile(oldPath, newPath);
        napi_value result;
        napi_get_undefined(env, &result);
        return result;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value CreateDirectory(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected directory path");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    try {
        CreateDirectory(path);
        napi_value result;
        napi_get_undefined(env, &result);
        return result;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value DeleteDirectory(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected directory path");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    try {
        DeleteDirectory(path);
        napi_value result;
        napi_get_undefined(env, &result);
        return result;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value ListDirectory(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get callback info");
        return nullptr;
    }

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected directory path");
        return nullptr;
    }

    try {
        std::string path = GetStringFromNapiValue(env, args[0]);
        std::vector<std::string> entries = ListDirectory(path);
        return CreateArrayFromVector(env, entries);
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value FileExists(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected file path");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    bool exists = FileExists(path);
    napi_value result;
    napi_create_boolean(env, exists, &result);
    return result;
}

static napi_value IsDirectory(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected path");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    bool isDir = IsDirectory(path);
    napi_value result;
    napi_create_boolean(env, isDir, &result);
    return result;
}

static napi_value IsFile(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected path");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    bool isFile = IsFile(path);
    napi_value result;
    napi_create_boolean(env, isFile, &result);
    return result;
}

static napi_value GetFileSize(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected file path");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    try {
        off_t size = GetFileSize(path);
        napi_value result;
        napi_create_double(env, static_cast<double>(size), &result);
        return result;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value GetFileStats(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    size_t pathLength = 0;
    status = napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to get string length");
        return nullptr;
    }
    char* pathBuffer = new char[pathLength + 1];
    status = napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    if (status != napi_ok) {
        delete[] pathBuffer;
        napi_throw_error(env, "NAPI_ERROR", "Failed to get string value");
        return nullptr;
    }
    std::string path(pathBuffer);
    delete[] pathBuffer;

    try {
        struct stat buffer;
        if (stat(path.c_str(), &buffer) != 0) {
            throw std::runtime_error("Failed to get file stats");
        }

        napi_value result;
        status = napi_create_object(env, &result);
        if (status != napi_ok) {
            napi_throw_error(env, "NAPI_ERROR", "Failed to create object");
            return nullptr;
        }

        napi_value size;
        status = napi_create_double(env, static_cast<double>(buffer.st_size), &size);
        if (status != napi_ok) {
            napi_throw_error(env, "NAPI_ERROR", "Failed to create double");
            return nullptr;
        }
        status = napi_set_named_property(env, result, "size", size);
        if (status != napi_ok) {
            napi_throw_error(env, "NAPI_ERROR", "Failed to set property");
            return nullptr;
        }
        return result;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value GetFilePermissions(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected file path");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    try {
        mode_t permissions = GetFilePermissions(path);
        napi_value result;
        napi_create_double(env, static_cast<double>(permissions), &result);
        return result;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value SetFilePermissions(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_2) {
        napi_throw_error(env, "Invalid arguments", "Expected file path and permissions");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    double permissions = 0.0;
    napi_get_value_double(env, args[1], &permissions);

    try {
        SetFilePermissions(path, static_cast<mode_t>(permissions));
        napi_value result;
        napi_get_undefined(env, &result);
        return result;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value JoinPath(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_2) {
        napi_throw_error(env, "Invalid arguments", "Expected two paths");
        return nullptr;
    }

    size_t path1Length = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &path1Length);
    char* path1Buffer = new char[path1Length + 1];
    napi_get_value_string_utf8(env, args[0], path1Buffer, path1Length + 1, nullptr);
    std::string path1(path1Buffer);
    delete[] path1Buffer;

    size_t path2Length = 0;
    napi_get_value_string_utf8(env, args[1], nullptr, 0, &path2Length);
    char* path2Buffer = new char[path2Length + 1];
    napi_get_value_string_utf8(env, args[1], path2Buffer, path2Length + 1, nullptr);
    std::string path2(path2Buffer);
    delete[] path2Buffer;

    std::string result = JoinPath(path1, path2);
    napi_value resultValue;
    napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &resultValue);
    return resultValue;
}

static napi_value Basename(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected path");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    std::string result = Basename(path);
    napi_value resultValue;
    napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &resultValue);
    return resultValue;
}

static napi_value Dirname(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected path");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    std::string result = Dirname(path);
    napi_value resultValue;
    napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &resultValue);
    return resultValue;
}

static napi_value Extname(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected path");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    std::string result = Extname(path);
    napi_value resultValue;
    napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &resultValue);
    return resultValue;
}

static napi_value NormalizePath(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected path");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    try {
        std::string result = NormalizePath(path);
        napi_value resultValue;
        napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &resultValue);
        return resultValue;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value AbsolutePath(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected path");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    try {
        std::string result = AbsolutePath(path);
        napi_value resultValue;
        napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &resultValue);
        return resultValue;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value CreateTempFile(napi_env env, napi_callback_info info)
{
    try {
        std::string result = CreateTempFile();
        napi_value resultValue;
        napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &resultValue);
        return resultValue;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value CreateTempDirectory(napi_env env, napi_callback_info info)
{
    try {
        std::string result = CreateTempDirectory();
        napi_value resultValue;
        napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &resultValue);
        return resultValue;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value GetFreeSpace(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected path");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    try {
        off_t space = GetFreeSpace(path);
        napi_value result;
        napi_create_double(env, static_cast<double>(space), &result);
        return result;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value GetTotalSpace(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected path");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    try {
        off_t space = GetTotalSpace(path);
        napi_value result;
        napi_create_double(env, static_cast<double>(space), &result);
        return result;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value WatchFile(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected file path");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value UnwatchFile(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected file path");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value ReadFileSync(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_1;
    napi_value args[NUMBER_1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_1) {
        napi_throw_error(env, "Invalid arguments", "Expected file path");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    try {
        std::string content = ReadFile(path);
        napi_value result;
        napi_create_string_utf8(env, content.c_str(), NAPI_AUTO_LENGTH, &result);
        return result;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value WriteFileSync(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_2) {
        napi_throw_error(env, "Invalid arguments", "Expected file path and content");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    size_t contentLength = 0;
    napi_get_value_string_utf8(env, args[1], nullptr, 0, &contentLength);
    char* contentBuffer = new char[contentLength + 1];
    napi_get_value_string_utf8(env, args[1], contentBuffer, contentLength + 1, nullptr);
    std::string content(contentBuffer);
    delete[] contentBuffer;

    try {
        WriteFile(path, content);
        napi_value result;
        napi_get_undefined(env, &result);
        return result;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value TruncateFile(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_2) {
        napi_throw_error(env, "Invalid arguments", "Expected file path and length");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    double length = 0.0;
    napi_get_value_double(env, args[1], &length);

    try {
        TruncateFile(path, static_cast<off_t>(length));
        napi_value result;
        napi_get_undefined(env, &result);
        return result;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value Chmod(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_2;
    napi_value args[NUMBER_2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_2) {
        napi_throw_error(env, "Invalid arguments", "Expected file path and mode");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    double mode = 0.0;
    napi_get_value_double(env, args[1], &mode);

    try {
        ChmodFile(path, static_cast<mode_t>(mode));
        napi_value result;
        napi_get_undefined(env, &result);
        return result;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value Chown(napi_env env, napi_callback_info info)
{
    size_t argc = NUMBER_3;
    napi_value args[NUMBER_3];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    if (argc < NUMBER_3) {
        napi_throw_error(env, "Invalid arguments", "Expected file path, owner and group");
        return nullptr;
    }

    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &pathLength);
    char* pathBuffer = new char[pathLength + 1];
    napi_get_value_string_utf8(env, args[0], pathBuffer, pathLength + 1, nullptr);
    std::string path(pathBuffer);
    delete[] pathBuffer;

    double owner = 0.0;
    double group = 0.0;
    napi_get_value_double(env, args[1], &owner);
    napi_get_value_double(env, args[1], &group);

    try {
        ChownFile(path, static_cast<uid_t>(owner), static_cast<gid_t>(group));
        napi_value result;
        napi_get_undefined(env, &result);
        return result;
    } catch (const std::exception& e) {
        napi_throw_error(env, "File error", e.what());
        return nullptr;
    }
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor descriptors[] = {
        {"readFile", nullptr, ReadFile, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"writeFile", nullptr, WriteFile, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"appendFile", nullptr, AppendFile, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"deleteFile", nullptr, DeleteFile, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"copyFile", nullptr, CopyFile, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"moveFile", nullptr, MoveFile, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"renameFile", nullptr, RenameFile, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"createDirectory", nullptr, CreateDirectory, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"deleteDirectory", nullptr, DeleteDirectory, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"listDirectory", nullptr, ListDirectory, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"fileExists", nullptr, FileExists, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"isDirectory", nullptr, IsDirectory, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"isFile", nullptr, IsFile, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getFileSize", nullptr, GetFileSize, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getFileStats", nullptr, GetFileStats, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getFilePermissions", nullptr, GetFilePermissions, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setFilePermissions", nullptr, SetFilePermissions, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"joinPath", nullptr, JoinPath, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"basename", nullptr, Basename, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"dirname", nullptr, Dirname, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"extname", nullptr, Extname, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"normalizePath", nullptr, NormalizePath, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"absolutePath", nullptr, AbsolutePath, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"createTempFile", nullptr, CreateTempFile, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"createTempDirectory", nullptr, CreateTempDirectory, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getFreeSpace", nullptr, GetFreeSpace, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getTotalSpace", nullptr, GetTotalSpace, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"watchFile", nullptr, WatchFile, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"unwatchFile", nullptr, UnwatchFile, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"readFileSync", nullptr, ReadFileSync, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"writeFileSync", nullptr, WriteFileSync, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"truncateFile", nullptr, TruncateFile, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"chmod", nullptr, Chmod, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"chown", nullptr, Chown, nullptr, nullptr, nullptr, napi_default, nullptr}
    };

    size_t descCount = sizeof(descriptors) / sizeof(descriptors[0]);
    napi_status status = napi_define_properties(env, exports, descCount, descriptors);
    if (status != napi_ok) {
        napi_throw_error(env, "NAPI_ERROR", "Failed to define properties");
        return nullptr;
    }
    return exports;
}

NAPI_MODULE(napi_file, Init)
