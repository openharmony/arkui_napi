/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ark_native_timer.h"

#include "securec.h"

#include "utils/log.h"

void NativeTimerCallbackInfo::TimerCallback(uv_timer_t* timerReq)
{
    NativeTimerCallbackInfo* info = reinterpret_cast<NativeTimerCallbackInfo*>(timerReq->data);
    bool repeat = info->repeat_;
    if (!repeat) {
        info->timeoutExecuting_ = true;
    }
    info->cb_(info->data_);
    if (!repeat) {
        info->Erase();
        delete info;
        info = nullptr;
    }
}

void *NativeTimerCallbackInfo::TimerTaskCallback(EcmaVM* vm, void* data, TimerCallbackFunc func,
                                                 uint64_t timeout, bool repeat)
{
    if (vm == nullptr || func == nullptr) {
        HILOG_ERROR("TimerTaskCallback input is nullptr");
        return nullptr;
    }
    ArkNativeEngine* engine = reinterpret_cast<ArkNativeEngine*>(JSNApi::GetEnv(vm));
    NativeTimerCallbackInfo* info = new NativeTimerCallbackInfo(engine, func, data, repeat);
    if (!info->Init(timeout)) {
        HILOG_ERROR("NativeTimerCallbackInfo init failed");
        delete info;
        info = nullptr;
        return nullptr;
    }
    info->Insert();
    if (JSNApi::IsJSMainThreadOfEcmaVM(vm)) {
        uv_loop_t* loop = reinterpret_cast<uv_loop_t*>(engine->GetUVLoop());
        // main thread need send signal to trigger timer
        uv_async_send(&loop->wq_async);
    }
    return reinterpret_cast<void*>(info);
}

void NativeTimerCallbackInfo::CancelTimerCallback(void* timerCallbackInfo)
{
    if (timerCallbackInfo == nullptr) {
        HILOG_ERROR("CancelTimerCallback info is nullptr");
        return;
    }
    NativeTimerCallbackInfo* info = reinterpret_cast<NativeTimerCallbackInfo*>(timerCallbackInfo);
    // Timeout will auto delete after executed, if timerCallback contains cancle task will be invalid.
    // Timeout can be canceled by user only before executing.
    if (info->timeoutExecuting_) {
        return;
    }
    info->Erase();
    delete info;
    info = nullptr;
}

void NativeTimerCallbackInfo::Insert()
{
    NativeTimerCallbackInfo* timerListHead = engine_->GetTimerListHead();
    if (timerListHead) {
        timerListHead->prev_ = this;
        next_ = timerListHead;
    }
    timerListHead = this;
    engine_->SetTimerListHead(timerListHead);
}

void NativeTimerCallbackInfo::Erase()
{
    if (prev_) {
        prev_->next_ = next_;
    }
    if (this == engine_->GetTimerListHead()) {
        engine_->SetTimerListHead(next_);
    }
    if (next_) {
        next_->prev_ = prev_;
    }
    next_ = nullptr;
    prev_ = nullptr;
}

void NativeTimerCallbackInfo::ReleaseTimerList(ArkNativeEngine* engine)
{
    for (NativeTimerCallbackInfo* info = engine->GetTimerListHead();
        info != nullptr; info = engine->GetTimerListHead()) {
        info->Erase();
        delete info;
        info = nullptr;
    }
}

bool NativeTimerCallbackInfo::Init(uint64_t timeout)
{
    if (!engine_) {
        HILOG_ERROR("NativeTimerCallbackInfo engine_ is nullptr");
        return false;
    }
    uv_loop_t* loop = reinterpret_cast<uv_loop_t*>(engine_->GetUVLoop());
    if (!loop) {
        HILOG_ERROR("NativeTimerCallbackInfo loop is nullptr");
        return false;
    }
    timerReq_ = new uv_timer_t();
    if (!timerReq_) {
        HILOG_ERROR("NativeTimerCallbackInfo timerReq_ is nullptr");
        return false;
    }
    timerReq_->data = this;
    if (uv_timer_init(loop, timerReq_) != EOK) {
        HILOG_ERROR("NativeTimerCallbackInfo uv_timer_init failed");
        delete timerReq_;
        timerReq_ = nullptr;
        return false;
    }
    if (uv_timer_start(timerReq_, TimerCallback, timeout, timeout) != EOK) {
        HILOG_ERROR("NativeTimerCallbackInfo uv_timer_start failed");
        uv_close(reinterpret_cast<uv_handle_t*>(timerReq_), nullptr);
        delete timerReq_;
        timerReq_ = nullptr;
        return false;
    }
    return true;
}