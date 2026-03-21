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

#include <cstring>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "hilog/log.h"

namespace {
constexpr int32_t HILOG_MODULE_ID = 0x0001;
constexpr uint32_t MODULE_VERSION = 1;
constexpr uint32_t NO_MODULE_FLAGS = 0;
constexpr size_t MAX_FILE_SIZE = 10 * 1024 * 1024;
constexpr size_t DEFAULT_BUFFER_SIZE = 4096;
constexpr int32_t FILE_ERROR_OPEN = 1;
constexpr int32_t FILE_ERROR_TOO_LARGE = 2;
constexpr int32_t FILE_ERROR_READ = 3;
constexpr size_t CALLBACK_ARG_COUNT = 2;

void LogInfo(const std::string& message)
{
    HiLogPrint(LOG_CORE, LOG_INFO, HILOG_MODULE_ID, "FileOperations", "%{public}s", message.c_str());
}

void LogError(const std::string& message)
{
    HiLogPrint(LOG_CORE, LOG_ERROR, HILOG_MODULE_ID, "FileOperations", "%{public}s", message.c_str());
}

struct AsyncFileContext {
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;
    std::string filePath;
    std::string content;
    int32_t errorCode = 0;
    std::string errorMessage;
    bool usePromise = false;
};

void AsyncFileExecute(napi_env env, void* data)
{
    (void)env;
    auto* context = static_cast<AsyncFileContext*>(data);
    if (context == nullptr) {
        return;
    }

    std::ifstream file(context->filePath, std::ios::binary);
    if (!file.is_open()) {
        context->errorCode = FILE_ERROR_OPEN;
        context->errorMessage = "Failed to open file: " + context->filePath;
        return;
    }

    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize > MAX_FILE_SIZE) {
        context->errorCode = FILE_ERROR_TOO_LARGE;
        context->errorMessage = "File too large: " + std::to_string(fileSize) + " bytes";
        return;
    }

    context->content.resize(fileSize);
    file.read(&context->content[0], fileSize);

    if (!file) {
        context->errorCode = FILE_ERROR_READ;
        context->errorMessage = "Failed to read file content";
        return;
    }

    file.close();
}

void AsyncFileComplete(napi_env env, napi_status status, void* data)
{
    (void)status;
    auto* context = static_cast<AsyncFileContext*>(data);
    if (context == nullptr) {
        return;
    }

    napi_handle_scope scope = nullptr;
    if (napi_open_handle_scope(env, &scope) != napi_ok) {
        return;
    }

    if (context->errorCode != 0) {
        napi_value error = nullptr;
        napi_create_string_utf8(env, context->errorMessage.c_str(), NAPI_AUTO_LENGTH, &error);

        if (context->usePromise && context->deferred != nullptr) {
            napi_reject_deferred(env, context->deferred, error);
        } else if (context->callbackRef != nullptr) {
            napi_value callback = nullptr;
            napi_get_reference_value(env, context->callbackRef, &callback);

            napi_value undefined = nullptr;
            napi_get_undefined(env, &undefined);

            napi_value errorValue = nullptr;
            napi_create_string_utf8(env, context->errorMessage.c_str(), NAPI_AUTO_LENGTH, &errorValue);
            napi_call_function(env, undefined, callback, 1, &errorValue, nullptr);
        }
    } else {
        napi_value result = nullptr;
        napi_create_string_utf8(env, context->content.c_str(), context->content.size(), &result);

        if (context->usePromise && context->deferred != nullptr) {
            napi_resolve_deferred(env, context->deferred, result);
        } else if (context->callbackRef != nullptr) {
            napi_value callback = nullptr;
            napi_get_reference_value(env, context->callbackRef, &callback);

            napi_value undefined = nullptr;
            napi_get_undefined(env, &undefined);

            napi_value nullValue = nullptr;
            napi_get_null(env, &nullValue);

            napi_value argv[] = { nullValue, result };
            napi_call_function(env, undefined, callback, CALLBACK_ARG_COUNT, argv, nullptr);
        }
    }

    if (context->callbackRef != nullptr) {
        napi_delete_reference(env, context->callbackRef);
    }
    if (context->work != nullptr) {
        napi_delete_async_work(env, context->work);
    }

    napi_close_handle_scope(env, scope);
    delete context;
}

napi_value ReadFileSync(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= 1, "Requires 1 argument: filePath");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &type));
    NAPI_ASSERT(env, type == napi_string, "filePath must be a string");

    size_t strLength = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strLength));

    std::vector<char> buffer(strLength + 1);
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], buffer.data(), buffer.size(), &strLength));
    std::string filePath(buffer.data(), strLength);

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        napi_throw_error(env, nullptr, ("Failed to open file: " + filePath).c_str());
        return nullptr;
    }

    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize > MAX_FILE_SIZE) {
        napi_throw_error(env, nullptr, "File too large");
        return nullptr;
    }

    std::string content(fileSize, '\0');
    file.read(&content[0], fileSize);
    file.close();

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, content.c_str(), content.size(), &result));

    LogInfo("File read successfully: " + filePath);
    return result;
}

napi_value ReadFileAsync(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPIAPI_ASSERT(env, argc >= 1, "Requires at least 1 argument: filePath");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &type));
    NAPI_ASSERT(env, type == napi_string, "filePath must be a string");

    size_t strLength = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strLength));

    std::vector<char> buffer(strLength + 1);
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], buffer.data(), buffer.size(), &strLength));
    std::string filePath(buffer.data(), strLength);

    bool usePromise = (argc == 1);

    auto* context = new AsyncFileContext();
    context->filePath = filePath;
    context->usePromise = usePromise;

    napi_value resourceName = nullptr;
    napi_create_string_utf8("ReadFileAsync", NAPI_AUTO_LENGTH, &resourceName);

    if (usePromise) {
        napi_value promise = nullptr;
        NAPI_CALL(env, napi_create_promise(env, &context->deferred, &promise));

        NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName,
            AsyncFileExecute, AsyncFileComplete, context, &context->work));

        NAPI_CALL(env, napi_queue_async_work(env, context->work));

        LogInfo("Async file read started: " + filePath);
        return promise;
    } else {
        napi_valuetype callbackType = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[1], &callbackType));
        NAPI_ASSERT(env, callbackType == napi_function, "Second argument must be a function");

        NAPI_CALL(env, napi_create_reference(env, argv[1], 1, &context->callbackRef));

        NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName,
            AsyncFileExecute, AsyncFileComplete, context, &context->work));

        NAPI_CALL(env, napi_queue_async_work(env, context->work));

        napi_value result = nullptr;
        NAPI_CALL(env, napi_get_undefined(env, &result));

        LogInfo("Async file read started: " + filePath);
        return result;
    }
}

napi_value WriteFileSync(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value argv[2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= 2, "Requires 2 arguments: filePath and content");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &type));
    NAPI_ASSERT(env, type == napi_string, "filePath must be a string");

    NAPI_CALL(env, napi_typeof(env, argv[1], &type));
    NAPI_ASSERT(env, type == napi_string, "content must be a string");

    size_t strLength = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strLength));

    std::vector<char> pathBuffer(strLength + 1);
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], pathBuffer.data(), pathBuffer.size(), &strLength));
    std::string filePath(pathBuffer.data(), strLength);

    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[1], nullptr, 0, &strLength));

    std::vector<char> contentBuffer(strLength + 1);
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[1], contentBuffer.data(), contentBuffer.size(), &strLength));
    std::string content(contentBuffer.data(), strLength);

    if (content.size() > MAX_FILE_SIZE) {
        napi_throw_error(env, nullptr, "Content too large");
        return nullptr;
    }

    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        napi_throw_error(env, nullptr, ("Failed to open file for writing: " + filePath).c_str());
        return nullptr;
    }

    file.write(content.c_str(), content.size());
    file.close();

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, true, &result));

    LogInfo("File written successfully: " + filePath);
    return result;
}

napi_value GetFileInfo(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= 1, "Requires 1 argument: filePath");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &type));
    NAPI_ASSERT(env, type == napi_string, "filePath must be a string");

    size_t strLength = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strLength));

    std::vector<char> buffer(strLength + 1);
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], buffer.data(), buffer.size(), &strLength));
    std::string filePath(buffer.data(), strLength);

    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) {
        napi_throw_error(env, nullptr, "Failed to get file info");
        return nullptr;
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));

    napi_value sizeValue = nullptr;
    NAPI_CALL(env, napi_create_uint64(env, fileStat.st_size, &sizeValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "size", sizeValue));

    napi_value isDirectoryValue = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, S_ISDIR(fileStat.st_mode), &isDirectoryValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "isDirectory", isDirectoryValue));

    napi_value isFileValue = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, S_ISREG(fileStat.st_mode), &isFileValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "isFile", isFileValue));

    napi_value modifiedTimeValue = nullptr;
    NAPI_CALL(env, napi_create_int64(env, fileStat.st_mtime, &modifiedTimeValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "modifiedTime", modifiedTimeValue));

    return result;
}

napi_value ListDirectory(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= 1, "Requires 1 argument: dirPath");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &type));
    NAPI_ASSERT(env, type == napi_string, "dirPath must be a string");

    size_t strLength = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strLength));

    std::vector<char> buffer(strLength + 1);
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], buffer.data(), buffer.size(), &strLength));
    std::string dirPath(buffer.data(), strLength);

    DIR* dir = opendir(dirPath.c_str());
    if (dir == nullptr) {
        napi_throw_error(env, nullptr, "Failed to open directory");
        return nullptr;
    }

    std::vector<std::string> entries;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name != "." && name != "..") {
            entries.push_back(name);
        }
    }
    closedir(dir);

    napi_value resultArray = nullptr;
    NAPI_CALL(env, napi_create_array_with_length(env, entries.size(), &resultArray));

    for (size_t i = 0; i < entries.size(); i++) {
        napi_value entryValue = nullptr;
        NAPI_CALL(env, napi_create_string_utf8(env, entries[i].c_str(), entries[i].size(), &entryValue));
        NAPI_CALL(env, napi_set_element(env, resultArray, i, entryValue));
    }

    LogInfo("Directory listed: " + dirPath + " (" + std::to_string(entries.size()) + " entries)");
    return resultArray;
}

napi_value FileExists(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= 1, "Requires 1 argument: filePath");

    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &type));
    NAPI_ASSERT(env, type == napi_string, "filePath must be a string");

    size_t strLength = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strLength));

    std::vector<char> buffer(strLength + 1);
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], buffer.data(), buffer.size(), &strLength));
    std::string filePath(buffer.data(), strLength);

    struct stat fileStat;
    bool exists = (stat(filePath.c_str(), &fileStat) == 0);

    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, exists, &result));

    return result;
}

}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("readFileSync", ReadFileSync),
        DECLARE_NAPI_FUNCTION("readFileAsync", ReadFileAsync),
        DECLARE_NAPI_FUNCTION("writeFileSync", WriteFileSync),
        DECLARE_NAPI_FUNCTION("getFileInfo", GetFileInfo),
        DECLARE_NAPI_FUNCTION("listDirectory", ListDirectory),
        DECLARE_NAPI_FUNCTION("fileExists", FileExists),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module g_fileOperationsModule = {
    .nm_version = MODULE_VERSION,
    .nm_flags = NO_MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "file_operations",
    .nm_priv = nullptr,
};

extern "C" __attribute__((constructor)) void RegisterFileOperationsModule(void)
{
    napi_module_register(&g_fileOperationsModule);
}
