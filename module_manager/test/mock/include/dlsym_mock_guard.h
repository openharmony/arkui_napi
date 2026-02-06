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

#ifndef DLSYM_MOCK_GUARD_H
#define DLSYM_MOCK_GUARD_H

#include <string>
#include "dlsym_mock.h"

class DlsymMockGuard {
public:
    DlsymMockGuard()
    {
        DlsymMockEnable();
    }
    
    ~DlsymMockGuard()
    {
        DlsymMockDisable();
    }
    
    void SetFileContent(const std::string& path, const std::string& content)
    {
        DlsymMockSetFileContent(path.c_str(), content.c_str());
    }
    
    void SetFileNotExists(const std::string& path)
    {
        DlsymMockSetFileExists(path.c_str(), 0);
    }
    
    void SetFileExists(const std::string& path)
    {
        DlsymMockSetFileExists(path.c_str(), 1);
    }
    
    void ClearAll()
    {
        DlsymMockClearAll();
    }
    
    void SetDebugMode(bool enable)
    {
        DlsymMockSetDebugMode(enable ? 1 : 0);
    }
};

#endif // DLSYM_MOCK_GUARD_H
