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

/*
 * dlsym Mock Implementation
 *
 * Principle:
 * 1. Use dlsym(RTLD_NEXT, "func_name") to get the real function pointer
 * 2. Define same-name functions to override original functions
 * 3. Decide at runtime whether to call mock logic or real function
 *
 * Notes:
 * - std::ifstream uses open() system call internally, not fopen()
 * - Must mock open/read/close/fstat to intercept ifstream operations
 */

#ifndef DLSYM_MOCK_H
#define DLSYM_MOCK_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ============================================================================
 * Function Pointer Type Definitions
 * ============================================================================
 */

typedef int (*DlsymAccessFuncPtr)(const char*, int);
typedef int (*DlsymOpenFuncPtr)(const char*, int, ...);
typedef ssize_t (*DlsymReadFuncPtr)(int, void*, size_t);
typedef ssize_t (*DlsymWriteFuncPtr)(int, const void*, size_t);
typedef int (*DlsymCloseFuncPtr)(int);
typedef off_t (*DlsymLseekFuncPtr)(int, off_t, int);
typedef int (*DlsymStatFuncPtr)(const char*, struct stat*);
typedef int (*DlsymFstatFuncPtr)(int, struct stat*);
typedef FILE* (*DlsymFopenFuncPtr)(const char*, const char*);

/*
 * ============================================================================
 * Mock Control Interface
 * ============================================================================
 */

void DlsymMockEnable(void);
void DlsymMockDisable(void);
void DlsymMockSetFileContent(const char* path, const char* content);
void DlsymMockSetFileExists(const char* path, int exists);
void DlsymMockClearAll(void);
int DlsymMockIsEnabled(void);

/*
 * ============================================================================
 * Debug Interface
 * ============================================================================
 */

void DlsymMockSetDebugMode(int enable);

#ifdef __cplusplus
}
#endif

#endif /* DLSYM_MOCK_H */
