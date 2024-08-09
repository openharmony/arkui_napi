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

#ifndef OHOS_FFI_CJ_LAMBDA_H
#define OHOS_FFI_CJ_LAMBDA_H

#include <cstdint>
#include <tuple>

#include "ffi_remote_data.h"

#ifndef FFI_EXPORT
#ifndef WINDOWS_PLATFORM
#define FFI_EXPORT __attribute__((visibility("default")))
#else
#define FFI_EXPORT __declspec(dllexport)
#endif
#endif

class FFI_EXPORT CJLambda {
    template <size_t I>
    struct TupleRuntimeHelper {
        template <typename T>
        static void* GetElementAddr(T& tup, size_t idx)
        {
            if (idx == I - 1) {
                return &(std::get<I - 1>(tup));
            } else {
                return TupleRuntimeHelper<I - 1>::GetElementAddr(tup, idx);
            }
        }
    };

    template <>
    struct TupleRuntimeHelper<0> {
        template <typename T>
        static void* GetElementAddr(T& tup, size_t idx)
        {
            // Unreachable
            return nullptr;
        }
    };

    template <typename... Types>
    static inline void* GetElementAddr(std::tuple<Types...> const& tup, size_t idx)
    {
        return TupleRuntimeHelper<sizeof...(Types)>::GetElementAddr(tup, idx);
    }

    template <typename... Types>
    static inline void* GetElementAddr(std::tuple<Types...>& tup, size_t idx)
    {
        return TupleRuntimeHelper<sizeof...(Types)>::GetElementAddr(tup, idx);
    }
public:
    template<class... I>
    static std::function<void(I...)> Create(void (*callback)(I...))
    {
        auto handle = OHOS::FFI::RemoteData::Create<OHOS::FFI::CJLambdaRemoteData>(reinterpret_cast<int64_t>(callback));
        return [handle](I...args) -> void {
            constexpr int32_t argc = std::tuple_size_v<std::tuple<I...>>;
            if (argc == 0) {
                InvokeLambda(handle->GetID(), argc, nullptr, nullptr);
                return;
            }
            auto argsTuple = std::make_tuple(args...);
            void* argv[argc];
            for (size_t i = 0; i < argc; ++i) {
                argv[i] = GetElementAddr(argsTuple, i);
            }
            InvokeLambda(handle->GetID(), argc, argv, nullptr);
        };
    }

    template<class... I, class R>
    static std::function<R(I...)> Create(R (*callback)(I...))
    {
        auto handle = OHOS::FFI::RemoteData::Create<OHOS::FFI::CJLambdaRemoteData>(reinterpret_cast<int64_t>(callback));
        return [handle](I...args) -> R {
            R res;
            constexpr int32_t argc = std::tuple_size_v<std::tuple<I...>>;
            if (argc == 0) {
                InvokeLambda(handle->GetID(), argc, nullptr, &res);
                return res;
            }
            auto argsTuple = std::make_tuple(args...);
            void* argv[argc];
            for (size_t i = 0; i < argc; ++i) {
                argv[i] = GetElementAddr(argsTuple, i);
            }
            InvokeLambda(handle->GetID(), argc, argv, &res);
            return res;
        };
    }

private:
    static inline void InvokeLambda(int64_t lambdaId, int32_t argc, void** argv, void* result)
    {
        auto invoker = CJFFIFnInvoker::GetInstance()->GetCJFuncs().atCOHOSFFICallbackInvoker;
        invoker(lambdaId, argc, argv, result);
    }
};

#endif // OHOS_FFI_CJ_LAMBDA_H
