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

#ifndef OHOS_FFI_RUNTIMETYPE_H
#define OHOS_FFI_RUNTIMETYPE_H

#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

namespace OHOS::FFI {

class TypeBase;

class RuntimeType final {
public:
    static RuntimeType Create(const char* className)
    {
        return RuntimeType(className);
    }
    template<typename Derive>
    static RuntimeType Create(const char* className)
    {
        auto result = Create(className);
        result.parents_.push_back(Derive::GetClassType());
        return result;
    }
    template<typename... Lists, typename Derive>
    static RuntimeType Create(const char* className)
    {
        auto result = Create<Lists...>(className);
        result.parents_.push_back(Derive::GetClassType());
        return result;
    }

    bool IsType(RuntimeType* target)
    {
        if (target == nullptr) {
            return false;
        }
        
        if (strcmp(this->GetTypeName(), target->GetTypeName())) {
            return true;
        }
        for (auto parent : parents_) {
            if (parent->IsType(target)) {
                return true;
            }
        }
        return false;
    }

    const char* GetTypeName() const
    {
        return name_;
    }

private:
    explicit RuntimeType(const char* name) : name_(name) {}
    const char* name_;
    std::vector<RuntimeType*> parents_;
};

#define DECL_TYPE(className, inheritedClasses...)                                                                 \
private:                                                                                                          \
    friend class OHOS::FFI::RuntimeType;                                                                          \
    friend class OHOS::FFI::TypeBase;                                                                             \
    static OHOS::FFI::RuntimeType* GetClassType()                                                                 \
    {                                                                                                             \
        static OHOS::FFI::RuntimeType runtimeType = OHOS::FFI::RuntimeType::Create<inheritedClasses>(#className); \
        return &runtimeType;                                                                                      \
    }                                                                                                             \
                                                                                                                  \
public:                                                                                                           \
    OHOS::FFI::RuntimeType* GetRuntimeType() override                                                             \
    {                                                                                                             \
        return GetClassType();                                                                                    \
    }

class TypeBase {
public:
    virtual ~TypeBase() = default;
    virtual RuntimeType* GetRuntimeType()
    {
        return GetClassType();
    }

    template<class O>
    O* DynamicCast()
    {
        static_assert(std::is_base_of_v<TypeBase, O>, "O must derived from TypeBase");
        if (GetRuntimeType()->IsType(O::GetClassType())) {
            return static_cast<O*>(this);
        }
        return nullptr;
    }

private:
    friend class RuntimeType;

    static RuntimeType* GetClassType()
    {
        static RuntimeType runtimeType = RuntimeType::Create("TypeBase");
        return &runtimeType;
    }
};
} // namespace OHOS::FFI
#endif // OHOS_FFI_RUNTIMETYPE_H
