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

#include "native_reference_manager.h"

#include "native_engine/impl/ark/ark_native_reference.h"

NativeReferenceManager::~NativeReferenceManager()
{
    for (auto handler = references_; handler != nullptr; handler = references_) {
        references_ = reinterpret_cast<ArkNativeReference*>(handler)->next_;
        delete handler;
    }
}

void NativeReferenceManager::CreateHandler(NativeReference* reference)
{
    if (references_) {
        reinterpret_cast<ArkNativeReference*>(references_)->prev_ = reference;
        reinterpret_cast<ArkNativeReference*>(reference)->next_ = references_;
    }
    references_ = reference;
}

void NativeReferenceManager::ReleaseHandler(NativeReference* reference)
{
    NativeReference* prev = reinterpret_cast<ArkNativeReference*>(reference)->prev_;
    NativeReference* next = reinterpret_cast<ArkNativeReference*>(reference)->next_;

    if (prev) {
        reinterpret_cast<ArkNativeReference*>(prev)->next_ = next;
    }
    if (reference == references_) {
        // reference is the head node.
        references_ = next;
    }
    if (next) {
        reinterpret_cast<ArkNativeReference*>(next)->prev_ = prev;
    }
}
