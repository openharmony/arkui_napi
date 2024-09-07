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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_ARK_NATIVE_OPTIONS_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_ARK_NATIVE_OPTIONS_H

namespace NapiProperties {
enum NapiProperties : uint32_t {
    DEFAULT = -1,  // default value 000'0000'0000'0000'0000'0000 -> 0x0
};
} // namespace NapiProperties

class NapiOptions {
public:
    void SetProperties(int properties)
    {
        uint32_t prop = static_cast<uint32_t>(properties);
        if (prop != NapiProperties::DEFAULT) {
            napiProperties_ = prop;
        }
        ParseProperties();
        return;
    }

private:
    // do not remove, extern switch would be added
    void ParseProperties()
    {
        return;
    }

    uint32_t GetDefaultProperties() const
    {
        return 0;
    }

    inline int GetProperties() const
    {
        return napiProperties_;
    }

    uint32_t napiProperties_ = GetDefaultProperties();
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_ARK_NATIVE_OPTIONS_H */
