/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_ARK_CRASH_HOLDER_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_ARK_CRASH_HOLDER_H

#include <string>

enum class DFXObjectType : uint8_t {
    STRING,
};

#define INIT_CRASH_HOLDER(holder) \
    CrashHolder holder(__FUNCTION__)

class CrashHolder {
public:
    explicit CrashHolder(const std::string& info)
    {
        SetCrashObj(info.c_str());
    }

    explicit CrashHolder(const char* info)
    {
        SetCrashObj(info);
    }

    ~CrashHolder();

void UpdateCallbackPtr(uintptr_t addr);

private:
    void SetCrashObj(const char* info);

    uintptr_t handle_ {0};

    char* data_ {nullptr};
    size_t size_ {0};
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_ARK_CRASH_HOLDER_H */