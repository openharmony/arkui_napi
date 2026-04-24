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

#include "cj_backtrace.h"

#include <unistd.h>
#include <vector>

#include "dfx_frame_formatter.h"
#include "fp_backtrace.h"

static std::vector<void*> g_ptrs;
static bool g_ignoreNextBt = false;
static OHOS::HiviewDFX::FpBacktrace* backtrace;

bool CJDFX_HasBacktrace(void)
{
    if (gettid() != getpid()) {
        return false;
    }
    if (!backtrace) {
        return false;
    }
    return !g_ptrs.empty();
}

bool CJDFX_Backtrace(uint32_t removeTopFrames)
{
    if (gettid() != getpid()) {
        return false;
    }
    if (g_ignoreNextBt) {
        g_ignoreNextBt = false;
        return true;
    }
    if (!backtrace) {
        backtrace = OHOS::HiviewDFX::FpBacktrace::CreateInstance();
        if (!backtrace) {
            return false;
        }
    }
    constexpr size_t maxFrames = 256;
    void* frames[maxFrames];

    const auto size = backtrace->BacktraceFromFp(__builtin_frame_address(0), frames, maxFrames);
    if (size <= removeTopFrames) {
        g_ptrs.clear();
        return true;
    }
    g_ptrs.resize(size - removeTopFrames);
    const auto begin = frames + removeTopFrames;
    const auto end = frames + size;
    g_ptrs.assign(begin, end);
    return true;
}

void CJDFX_IgnoreNextBacktrace()
{
    if (gettid() != getpid()) {
        return;
    }
    g_ignoreNextBt = true;
}

bool CJDFX_GetHybridStack(std::string* trace, std::function<bool(std::string&, int&, int&, std::string&)>* translator)
{
    if (gettid() != getpid()) {
        return false;
    }
    if (!trace) {
        return false;
    }
    if (!backtrace) {
        return false;
    }
    std::vector<OHOS::HiviewDFX::DfxFrame> frames;
    int index = 0;
    for (const auto pc : g_ptrs) {
        const auto frame = backtrace->SymbolicAddress(pc);
        if (!frame) {
            continue;
        }
        frame->index = index++;
        if (translator) {
            (*translator)(frame->mapName, frame->line, frame->column, frame->packageName);
        }
        frames.push_back(*frame);
    }
    *trace = OHOS::HiviewDFX::DfxFrameFormatter::GetFramesStr(frames);
    g_ptrs.clear();
    return true;
}
