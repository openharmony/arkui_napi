/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_FFI_CJ_COMMON_FFI_H
#define OHOS_FFI_CJ_COMMON_FFI_H

#include <cstdint>
#include <string>

#define FFI_EXPORT __attribute__((visibility("default")))

extern "C" {
const int32_t SUCCESS_CODE = 0;
const int32_t ERR_INVALID_INSTANCE_CODE = -1;
const int32_t ERR_CODE_PARAM_INVALID = -2;
const int64_t INVALID_DATA_ID = -100;

int64_t FFI_EXPORT FfiGeneralSizeOfPointer();

typedef int RetCode;

struct RetDataCString {
    int32_t code;
    char* data;
};

struct RetDataUI {
    int32_t code;
    size_t data;
};

struct RetDataI64 {
    int32_t code;
    int64_t data;
};

struct RetDataI32 {
    int32_t code;
    int32_t data;
};

struct RetDataI64U32 {
    uint32_t code;
    int64_t data;
};

struct RetDataUI32 {
    uint32_t code;
    uint32_t data;
};

struct RetDataBool {
    int32_t code;
    bool data;
};

struct CArrI64 {
    int64_t* head;
    int64_t size;
};

struct CArrI32 {
    int32_t* head;
    int64_t size;
};

struct RetDataCArrI32 {
    int32_t code;
    CArrI32 data;
};

struct CArrUI32 {
    unsigned int* head;
    int64_t size;
};

struct RetDataCArrI64 {
    int32_t code;
    CArrI64 data;
};

struct CArrUI8 {
    uint8_t* head;
    int64_t size;
};

struct RetDataCArrUI8 {
    int32_t code;
    CArrUI8 data;
};

struct CArrString {
    char** head;
    int64_t size;
};

struct RetDataCArrString {
    int32_t code;
    CArrString data;
};

struct CArrUnit {
    void* head;
    int64_t size;
};

struct RetDataCArrUnit {
    int32_t code;
    CArrUnit data;
};
}
#endif