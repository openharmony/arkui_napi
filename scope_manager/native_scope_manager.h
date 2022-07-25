/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_NAPI_SCOPE_MANAGER_NATIVE_SCOPE_MANAGER_H
#define FOUNDATION_ACE_NAPI_SCOPE_MANAGER_NATIVE_SCOPE_MANAGER_H

#ifdef ENABLE_MEMLEAK_DEBUG
#include <atomic>
#endif

#ifdef ENABLE_MEMLEAK_DEBUG
#include <string>
#include <vector>
#endif

class NativeValue;
struct NativeScope;
#ifdef ENABLE_MEMLEAK_DEBUG
struct StructVma {
    uint64_t begin = 0;
    uint64_t end = 0;
    std::string path;
};
#endif

class NativeScopeManager {
public:
    NativeScopeManager();
    virtual ~NativeScopeManager();

    virtual NativeScope* Open();
    virtual void Close(NativeScope* scope);

    virtual NativeScope* OpenEscape();
    virtual void CloseEscape(NativeScope* scope);

    virtual void CreateHandle(NativeValue* value);
    virtual NativeValue* Escape(NativeScope* scope, NativeValue* value);

    NativeScopeManager(NativeScopeManager&) = delete;
    virtual NativeScopeManager& operator=(NativeScopeManager&) = delete;

#ifdef ENABLE_MEMLEAK_DEBUG
    static const int MAPINFO_SIZE = 256;
    static const int NAME_LEN = 128;
    static const int DEBUG_MEMLEAK;
    static const int BACKTRACE_DEPTH;
#endif

private:
#ifdef ENABLE_MEMLEAK_DEBUG
    static std::atomic<std::vector<struct StructVma>*> vmas;
#endif
    NativeScope* root_;
    NativeScope* current_;
};

#endif /* FOUNDATION_ACE_NAPI_SCOPE_MANAGER_NATIVE_SCOPE_MANAGER_H */
