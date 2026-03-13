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
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 2) {
        napi_throw_error(env, nullptr, "Expected 2 arguments (id, name)");
        return nullptr;
    }
    
    int32_t id;
    napi_get_value_int32(env, args[0], &id);
    
    size_t length;
    char name[256];
    napi_get_value_string_utf8(env, args[1], name, sizeof(name), &length);
    
    int index = g_dataCache.AddData(env, id, std::string(name, length));
    
    napi_value result;
    napi_create_int32(env, index, &result);
    return result;
}

static napi_value GetDataInfo(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        napi_throw_error(env, nullptr, "Expected 1 argument (index)");
        return nullptr;
    }
    
    int32_t index;
    napi_get_value_int32(env, args[0], &index);
    
    ManagedData* data = g_dataCache.GetData(index);
    if (!data) {
        napi_throw_error(env, nullptr, "Invalid data index");
        return nullptr;
    }
    
    napi_value result;
    napi_create_object(env, &result);
    
    napi_value idValue, nameValue;
    napi_create_int32(env, data->id, &idValue);
    napi_create_string_utf8(env, data->name.c_str(), data->name.length(), &nameValue);
    
    napi_set_named_property(env, result, "id", idValue);
    napi_set_named_property(env, result, "name", nameValue);
    
    return result;
}

static napi_value DeleteData(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        napi_throw_error(env, nullptr, "Expected 1 argument (index)");
        return nullptr;
    }
    
    int32_t index;
    napi_get_value_int32(env, args[0], &index);
    
    bool success = g_dataCache.RemoveData(index);
    
    napi_value result;
    napi_get_boolean(env, success, &result);
    return result;
}

static napi_value ClearCache(napi_env env, napi_callback_info info) {
    g_dataCache.Clear();
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value GetCacheSize(napi_env env, napi_callback_info info) {
    size_t size = g_dataCache.Size();
    
    napi_value result;
    napi_create_uint32(env, static_cast<uint32_t>(size), &result);
    return result;
}

static napi_value CreateArrayBuffer(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        napi_throw_error(env, nullptr, "Expected 1 argument (size)");
        return nullptr;
    }
    
    int32_t size;
    napi_get_value_int32(env, args[0], &size);
    
    if (size <= 0 || size > 1024 * 1024) {
        napi_throw_error(env, nullptr, "Invalid size (must be between 1 and 1MB)");
        return nullptr;
    }
    
    void* data;
    napi_value arrayBuffer;
    napi_create_arraybuffer(env, size, &data, &arrayBuffer);
    
    memset(data, 0, size);
    
    return arrayBuffer;
}

static napi_value CreateExternalArrayBuffer(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    if (argc < 1) {
        napi_throw_error(env, nullptr, "Expected 1 argument (size)");
        return nullptr;
    }
    
    int32_t size;
    napi_get_value_int32(env, args[0], &size);
    
    if (size <= 0 || size > 1024 * 1024) {
        napi_throw_error(env, nullptr, "Invalid size (must be between 1 and 1MB)");
        return nullptr;
    }
    
    void* data = malloc(size);
    if (!data) {
        napi_throw_error(env, nullptr, "Memory allocation failed");
        return nullptr;
    }
    
    memset(data, 0xAA, size);
    
    auto finalizeCallback = [](napi_env env, void* data, void* hint) {
        free(data);
    };
    
    napi_value arrayBuffer;
    napi_create_external_arraybuffer(env, data, size, finalizeCallback, nullptr, &arrayBuffer);
    
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
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "memory_management",
    .nm_priv = ((void*)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterMemoryManagementModule(void) {
    napi_module_register(&demoModule);
}
