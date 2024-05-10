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

#ifndef NAPI_ARK_INTEROP_MACRO_H
#define NAPI_ARK_INTEROP_MACRO_H

#ifdef __cplusplus
#define DECL_START extern "C" {
#define DECL_END }
#else
#define DECL_START
#define DECL_END
#endif

#ifndef EXPORT
#ifdef __WINDOWS__
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif
#endif

#ifdef CLANG_COVERAGE
#ifdef __WINDOWS__
#define COV_EXPORT __declspec(dllexport)
#else // __WINDOWS__
#define COV_EXPORT __attribute__((visibility("default")))
#endif // __WINDOWS__
#else  // COV_EXPORT
#define COV_EXPORT
#endif // COV_EXPORT

// can not quote 'TYPE' in 'TYPE*', be careful on usage
#define BIT_CAST(VALUE, TYPE) (*reinterpret_cast<TYPE*>(&(VALUE)))
#define P_CAST(VALUE, TYPE) (reinterpret_cast<TYPE>(VALUE))

#define ARKTS_INLINE inline __attribute__((always_inline))

#endif // NAPI_ARK_INTEROP_MACRO_H
