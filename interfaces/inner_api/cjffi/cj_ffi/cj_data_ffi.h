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

#ifndef OHOS_FFI_CJ_DATA_FFI_H
#define OHOS_FFI_CJ_DATA_FFI_H

#include "native/cj_fn_invoker.h"
#include "native/ffi_remote_data.h"

extern "C" {
void FFI_EXPORT FfiOHOSFFIFFIDataRelease(int64_t id);
bool FFI_EXPORT FfiOHOSFFIFFIDataExist(int64_t id);
void FFI_EXPORT FfiOHOSFFIRegisterCJFuncs(FFIAtCPackage cjFuncs);
}

#endif // OHOS_FFI_CJ_DATA_FFI_H
