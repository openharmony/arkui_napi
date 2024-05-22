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

#include "runbufferscript_fuzzer.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "securec.h"

using namespace panda;
using namespace panda::ecmascript;
using panda::RuntimeOption;


#define MAXBYTELEN sizeof(uint32_t)

class Engine {
public:
    Engine()
    {
        RuntimeOption option;
        option.SetLogLevel(RuntimeOption::LOG_LEVEL::ERROR);
        vm_ = JSNApi::CreateJSVM(option);
        arkNativeEngine_ = new ArkNativeEngine(vm_, nullptr);
    }

    void RunBufferScript(std::vector<uint8_t>& buffer)
    {
        arkNativeEngine_->RunBufferScript(buffer);
    }

    ~Engine()
    {
        if (arkNativeEngine_ != nullptr) {
            delete arkNativeEngine_;
            arkNativeEngine_ = nullptr;
        }
        if (vm_ != nullptr) {
            JSNApi::DestroyJSVM(vm_);
            vm_ = nullptr;
        }
    }
private:
    EcmaVM* vm_ {nullptr};
    ArkNativeEngine* arkNativeEngine_ {nullptr};
};

static Engine g_nativeEngine;

namespace OHOS {
    void RunBufferScriptFuzzTest(const uint8_t* data, size_t size)
    {
        if (size <= 0) {
            return;
        }
        double input = 0;
        if (size > MAXBYTELEN) {
            size = MAXBYTELEN;
        }
        if (memcpy_s(&input, MAXBYTELEN, data, size) != 0) {
            std::cout << "memcpy_s failed!" << std::endl;
            UNREACHABLE();
        }

        std::vector<uint8_t> vec(size, *data);
        g_nativeEngine.RunBufferScript(vec);
    }
}

// Fuzzer entry point.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::RunBufferScriptFuzzTest(data, size);
    return 0;
}