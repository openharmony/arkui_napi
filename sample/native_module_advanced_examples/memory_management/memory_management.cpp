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

#include "napi/native_api.h"
#include <vector>
#include <memory>
#include <cstring>

constexpr int MAX_BUFFER_SIZE = 256;
constexpr int MAX_ARRAY_BUFFER_SIZE = 1048576;
constexpr int MODULE_VERSION = 1;
constexpr int MODULE_FLAGS = 0;

struct ManagedData {
    int id;
    std::string name;
    std::vector<int> values;
    
    ManagedData(int i, const std::string& n) : id(i), name(n) {}
};

class DataCache {
public:
    DataCache() {}
    
    ~DataCache() {
        Clear();
    }
    
    int AddData(napi_env env, int id, const std::string& name) {
        auto data = std::make_unique<ManagedData>(id, name);
        int index = static_cast<int>(dataList_.size());
        dataList_.push_back(std::move(data));
        return index;
    }
    
    bool RemoveData(int index) {
        if (index < 0 || index >= static_cast<int>(dataList_.size())) {
            return false;
        }
        dataList_[index].reset();
        return true;
    }
    
    ManagedData* GetData(int index) {
        if (index < 0 || index >= static_cast<int>(dataList_.size())) {
            return nullptr;
        }
        return dataList_[index].get();
    }
    
    void Clear() {
        dataList_.clear();
    }
    
    size_t Size() const {
        return dataList_.size();
    }
    
private:
    std::vector<std::unique_ptr<ManagedData>> dataList_;
};

static DataCache g_dataCache;

static napi_value CreateData(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get callback info");
        return nullptr;
    }
    
    if (argc < 2) {
        napi_throw_error(env, nullptr, "Expected 2 arguments (id, name)");
        return nullptr;
    }
    
    int32_t id;
    status = napi_get_value_int32(env, args[0], &id);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get id");
        return nullptr;
    }
    
    size_t length;
    char name[MAX_BUFFER_SIZE];
    status = napi_get_value_string_utf8(env, args[1], name, sizeof(name), &length);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get name");
        return nullptr;
    }
    
    int index = g_dataCache.AddData(env, id, std::string(name, length));
    
    napi_value result;
    status = napi_create_int32(env, index, &result);
    if (status != napi_ok) {
        return nullptr;
    }
    return result;
}

static napi_value GetDataInfo(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get callback info");
        return nullptr;
    }
    
    if (argc < 1) {
        napi_throw_error(env, nullptr, "Expected 1 argument (index)");
        return nullptr;
    }
    
    int32_t index;
    status = napi_get_value_int32(env, args[0], &index);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get index");
        return nullptr;
    }
    
    ManagedData* data = g_dataCache.GetData(index);
    if (!data) {
        napi_throw_error(env, nullptr, "Invalid data index");
        return nullptr;
    }
    
    napi_value result;
    status = napi_create_object(env, &result);
    if (status != napi_ok) {
        return nullptr;
    }
    
    napi_value idValue;
    napi_value nameValue;
    status = napi_create_int32(env, data->id, &idValue);
    if (status != napi_ok) {
        return nullptr;
    }
    status = napi_create_string_utf8(env, data->name.c_str(), data->name.length(), &nameValue);
    if (status != napi_ok) {
        return nullptr;
    }
    
    status = napi_set_named_property(env, result, "id", idValue);
    if (status != napi_ok) {
        return nullptr;
    }
    status = napi_set_named_property(env, result, "name", nameValue);
    if (status != napi_ok) {
        return nullptr;
    }
    
    return result;
}

static napi_value DeleteData(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get callback info");
        return nullptr;
    }
    
    if (argc < 1) {
        napi_throw_error(env, nullptr, "Expected 1 argument (index)");
        return nullptr;
    }
    
    int32_t index;
    status = napi_get_value_int32(env, args[0], &index);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get index");
        return nullptr;
    }
    
    bool success = g_dataCache.RemoveData(index);
    
    napi_value result;
    status = napi_create_boolean(env, success, &result);
    if (status != napi_ok) {
        return nullptr;
    }
    return result;
}

static napi_value ClearCache(napi_env env, napi_callback_info info) {
    g_dataCache.Clear();
    
    napi_value result;
    napi_status status = napi_get_undefined(env, &result);
    if (status != napi_ok) {
        return nullptr;
    }
    return result;
}

static napi_value GetCacheSize(napi_env env, napi_callback_info info) {
    size_t size = g_dataCache.Size();
    
    napi_value result;
    napi_status status = napi_create_uint32(env, static_cast<uint32_t>(size), &result);
    if (status != napi_ok) {
        return nullptr;
    }
    return result;
}

static napi_value CreateArrayBuffer(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get callback info");
        return nullptr;
    }
    
    if (argc < 1) {
        napi_throw_error(env, nullptr, "Expected 1 argument (size)");
        return nullptr;
    }
    
    int32_t size;
    status = napi_get_value_int32(env, args[0], &size);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get size");
        return nullptr;
    }
    
    if (size <= 0 || size > MAX_ARRAY_BUFFER_SIZE) {
        napi_throw_error(env, nullptr, "Invalid size (must be between 1 and 1MB)");
        return nullptr;
    }
    
    void* data;
    napi_value arrayBuffer;
    status = napi_create_arraybuffer(env, size, &data, &arrayBuffer);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to create array buffer");
        return nullptr;
    }
    
    std::fill(static_cast<uint8_t*>(data), static_cast<uint8_t*>(data) + size, 0);
    
    return arrayBuffer;
}

static napi_value CreateExternalArrayBuffer(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get callback info");
        return nullptr;
    }
    
    if (argc < 1) {
        napi_throw_error(env, nullptr, "Expected 1 argument (size)");
        return nullptr;
    }
    
    int32_t size;
    status = napi_get_value_int32(env, args[0], &size);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to get size");
        return nullptr;
    }
    
    if (size <= 0 || size > MAX_ARRAY_BUFFER_SIZE) {
        napi_throw_error(env, nullptr, "Invalid size (must be between 1 and 1MB)");
        return nullptr;
    }
    
    void* data = malloc(size);
    if (!data) {
        napi_throw_error(env, nullptr, "Memory allocation failed");
        return nullptr;
    }
    
    std::fill(static_cast<uint8_t*>(data), static_cast<uint8_t*>(data) + size, 0xAA);
    
    auto finalizeCallback = [](napi_env env, void* data, void* hint) {
        free(data);
    };
    
    napi_value arrayBuffer;
    status = napi_create_external_arraybuffer(env, data, size, finalizeCallback, nullptr, &arrayBuffer);
    if (status != napi_ok) {
        free(data);
        napi_throw_error(env, nullptr, "Failed to create external array buffer");
        return nullptr;
    }
    
    return arrayBuffer;
}

EXTERN_C_START
napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor properties[] = {
        {"createData", nullptr, CreateData, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getDataInfo", nullptr, GetDataInfo, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"deleteData", nullptr, DeleteData, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"clearCache", nullptr, ClearCache, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getCacheSize", nullptr, GetCacheSize, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"createArrayBuffer", nullptr, CreateArrayBuffer, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"createExternalArrayBuffer", nullptr, CreateExternalArrayBuffer, nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    
    napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = MODULE_VERSION,
    .nm_flags = MODULE_FLAGS,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "memory_management",
    .nm_priv = ((void*)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterMemoryManagementModule(void) {
    napi_module_register(&demoModule);
}
