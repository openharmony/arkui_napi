/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#include "cj_support.h"

#if defined(OHOS_STANDARD_PLATFORM)
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ark_native_engine.h"
#include "cj_envsetup.h"
#include "elf.h"
#include "utils/log.h"

#ifdef NAPI_TARGET_ARM64
#define LIBS_NAME "arm64"
#elif defined(NAPI_TARGET_ARM32)
#define LIBS_NAME "arm"
#elif defined(NAPI_TARGET_AMD64)
#define LIBS_NAME "x86_64"
#else
#error current platform not supported
#endif

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <elf.h>
#include <fcntl.h>
#include <unistd.h>

static bool LoadExtendedInfo(int fd, Elf64_Off shoff, Elf64_Half* shnum, Elf64_Half* shstrndx)
{
    Elf64_Shdr firstShdr;
    if (pread(fd, &firstShdr, sizeof(Elf64_Shdr), shoff) != sizeof(Elf64_Shdr)) {
        return false;
    }

    // Handle extended section header count
    if (*shnum == 0) {
        *shnum = firstShdr.sh_size;
    }

    // Handle extended string table index
    if (*shstrndx == SHN_XINDEX) {
        *shstrndx = firstShdr.sh_link;
    }
    return true;
}

// Check if has ".cjmetadata" section
static bool HasCJMetadata(int fd)
{
    // Read ELF header
    Elf64_Ehdr ehdr;
    if (pread(fd, &ehdr, sizeof(ehdr), 0) != sizeof(ehdr)) {
        return false;
    }

    Elf64_Half shnum = ehdr.e_shnum;
    Elf64_Half shstrndx = ehdr.e_shstrndx;
    // Load extended info
    if ((shnum == 0 || shstrndx == SHN_XINDEX) && !LoadExtendedInfo(fd, ehdr.e_shoff, &shnum, &shstrndx)) {
        return false;
    }

    // Load section header string table
    char* shstrtab = nullptr;
    Elf64_Word shstrtabSize = 0;

    if (shstrndx != SHN_UNDEF && shstrndx < shnum) {
        Elf64_Shdr shstrtabHdr;
        const Elf64_Off shdrOffset = ehdr.e_shoff + shstrndx * ehdr.e_shentsize;
        if (pread(fd, &shstrtabHdr, sizeof(shstrtabHdr), shdrOffset) != sizeof(shstrtabHdr)) {
            return false;
        }

        shstrtabSize = shstrtabHdr.sh_size;
        shstrtab = static_cast<char*>(malloc(shstrtabSize));
        if (!shstrtab) {
            return false;
        }

        if (pread(fd, shstrtab, shstrtabSize, shstrtabHdr.sh_offset) != shstrtabSize) {
            free(shstrtab);
            return false;
        }
    }

    // Iterate through section headers
    bool found = false;
    for (Elf64_Half i = 0; i < shnum; ++i) {
        const Elf64_Off shdrOffset = ehdr.e_shoff + i * ehdr.e_shentsize;
        Elf64_Shdr shdr;
        if (pread(fd, &shdr, sizeof(shdr), shdrOffset) != sizeof(shdr)) {
            break;
        }

        // Validate section name offset
        if (shdr.sh_name < shstrtabSize) {
            const char* name = shstrtab + shdr.sh_name;

            // Safe string comparison with boundary check
            const size_t maxLen = shstrtabSize - shdr.sh_name;
            if (strncmp(name, ".cjmetadata", maxLen) == 0) {
                found = true;
                break;
            }
        }
    }

    free(shstrtab);
    return found;
}

bool IsCJModule(const char* moduleName)
{
    HILOG_INFO("Checking whether is cj module, module name: %{public}s", moduleName);
    std::string absolutePath("/data/storage/el1/bundle/libs/" LIBS_NAME);
    std::string libName = "lib" + std::string(moduleName) + ".so";

    absolutePath = absolutePath + "/" + libName;
    struct stat st;
    if (stat(absolutePath.c_str(), &st) == 1) {
        return false;
    }
    const char* soPath = absolutePath.c_str();
    int fd = open(soPath, O_RDONLY);
    if (fd == -1) {
        HILOG_ERROR("Failed to open file %{public}s", soPath);
        return false;
    }

    // Check if its a valid elf file
    unsigned char eIdent[EI_NIDENT];
    if (read(fd, eIdent, EI_NIDENT) != EI_NIDENT || eIdent[EI_MAG0] != ELFMAG0 || eIdent[EI_MAG1] != ELFMAG1 ||
        eIdent[EI_MAG2] != ELFMAG2 || eIdent[EI_MAG3] != ELFMAG3) {
        HILOG_ERROR("Invalid ELF file %{public}s", soPath);
        close(fd);
        return false;
    }
    int is64bit = (eIdent[EI_CLASS] == ELFCLASS64);
    if (!is64bit) {
        close(fd);
        return false;
    }
    // Check if has .cjmetadat section
    int result = HasCJMetadata(fd);
    close(fd);

    if (result == 1) {
        HILOG_INFO("Found 'cjmetadata' section %{public}s", soPath);
        return true;
    } else {
        HILOG_INFO("Not Found 'cjmetadata' section %{public}s", soPath);
        return false;
    }
}

static bool LoadArkCJModule(napi_env env, const char* moduleName, napi_value* result)
{
    const char* targetName;
#ifdef OHOS_PLATFORM
    targetName = "libark_interop.z.so";
#elif defined(WINDOWS_PLATFORM)
    targetName = "libark_interop.dll";
#elif defined(LINUX_PLATFORM)
    targetName = "libark_interop.so";
#endif
    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto runtime = OHOS::CJEnv::LoadInstance();
    auto handle = runtime->loadLibrary(0, targetName);
    if (!handle) {
        HILOG_ERROR("open '%{public}s' failed", targetName);
        return false;
    }
    std::string libName = "lib" + std::string(moduleName) + ".so";

    if (auto symbol = runtime->getSymbol(handle, "ARKTS_LoadModuleByNapiEnv")) {
        auto loader = reinterpret_cast<napi_value (*)(napi_env, const char*)>(symbol);
        *result = loader(env, libName.c_str());
    } else if (auto symbol = runtime->getSymbol(handle, "ARKTS_LoadModule")) {
        auto loader = reinterpret_cast<napi_value (*)(EcmaVM*, const char*)>(symbol);
        auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());
        *result = loader(vm, libName.c_str());
    } else {
        return false;
    }
    return true;
}

napi_value LoadCJModule(napi_env env, const char* nameBuf)
{
    napi_value result;
    if (napi_get_undefined(env, &result) != napi_ok) {
        return result;
    }
    auto runtime = OHOS::CJEnv::LoadInstance();
    runtime->initCJAppNS("/data/storage/el1/bundle/libs/" LIBS_NAME);
    if (!runtime->startRuntime()) {
        HILOG_ERROR("start cjruntime failed");
        return result;
    }
    if (!runtime->startUIScheduler()) {
        HILOG_ERROR("start cj ui context failed");
        return result;
    }

    LoadArkCJModule(env, nameBuf, &result);
    return result;
}
#else
bool IsCJModule(const char* moduleName)
{
    return false;
}

napi_value LoadCJModule(napi_env env, const char* nameBuf)
{
    return nullptr;
}
#endif