/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef NAPI_CUSTOM_SCOPE_H
#define NAPI_CUSTOM_SCOPE_H

#include <cstdint>

class CustomScope {
public:
    explicit CustomScope(int32_t id): id_(id) {}

    void Enter();
    void Exit() const;

private:
    int32_t id_;
    int32_t last_;
};


#endif // NAPI_CUSTOM_SCOPE_H
