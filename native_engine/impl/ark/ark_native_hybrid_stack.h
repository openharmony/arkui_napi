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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_ARK_NATIVE_HYBRID_STACK_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_ARK_NATIVE_HYBRID_STACK_H

#include "ecmascript/napi/include/dfx_jsnapi.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "unwinder.h"

using JsFrameInfo = panda::ecmascript::JsFrameInfo;
using DfxFrame = OHOS::HiviewDFX::DfxFrame;
using Unwinder = OHOS::HiviewDFX::Unwinder;
//only for get hybrid stack, please don't modify.
class HybridStackDumper {
public:
    HybridStackDumper() = default;
    ~HybridStackDumper() = default;
    static std::string GetMixStack(const EcmaVM *vm);

private:
    bool DumpMixFrame(const EcmaVM* vm);
    void BuildJsNativeMixStack(std::vector<JsFrameInfo>& jsFrames, std::vector<DfxFrame>& nativeFrames);
    void Write(const std::string& outStr);
    std::string DumpMixStackLocked(const EcmaVM *vm);
    std::string PrintJsFrame(const JsFrameInfo& jsFrame);
    std::string MatchJsFrameByFlag(const std::vector<JsFrameInfo>& jsFrames, uint32_t& nativeFrames);

private:
    uint8_t skipframes_ = 6;
    std::shared_ptr<Unwinder> unwinder_ {nullptr};
    std::string flag_ = "stub.an";
    std::string stack_;
};
#endif