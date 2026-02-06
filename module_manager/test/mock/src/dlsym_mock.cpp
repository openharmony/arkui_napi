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

#include "dlsym_mock.h"

#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <fcntl.h>
#include <map>
#include "securec.h"
#include <set>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <gtest/gtest.h>

namespace {

constexpr size_t DEBUG_LOG_BUF_SIZE = 1024;
constexpr int MOCK_FD_START_VALUE = 20000;
constexpr mode_t STAT_FILE_MODE = 0644;
constexpr size_t FILE_BLOCK_SIZE = 4096;
constexpr size_t FILE_BLOCK_MASK = 511;

std::map<std::string, std::string> g_mockFileContent;
std::set<std::string> g_mockFileExists;
bool g_dlsymMockEnabled = false;
bool g_debugMode = true;

struct MockFileDescriptor {
    std::string path;
    std::string content;
    size_t readPos;
};
std::map<int, MockFileDescriptor> g_mockFdTable;
int g_nextMockFd = MOCK_FD_START_VALUE;
void DebugLog(const char* func, const char* fmt, ...)
{
    if (!g_debugMode) {
        return;
    }
    va_list args;
    va_start(args, fmt);
    char buf[DEBUG_LOG_BUF_SIZE] = {0};
    if (vsnprintf_s(buf, sizeof(buf), sizeof(buf) - 1, fmt, args) < 0) {
        va_end(args);
        GTEST_LOG_(INFO) << "[DLSYM_MOCK][" << func << "] vsnprintf failed";
        return;
    }
    va_end(args);
    GTEST_LOG_(INFO) << "[DLSYM_MOCK][" << func << "] " << buf;
}
int AllocateMockFd(const std::string& path, const std::string& content)
{
    int fd = g_nextMockFd++;
    MockFileDescriptor mockFd;
    mockFd.path = path;
    mockFd.content = content;
    mockFd.readPos = 0;
    g_mockFdTable[fd] = mockFd;
    return fd;
}
MockFileDescriptor* GetMockFd(int fd)
{
    auto it = g_mockFdTable.find(fd);
    if (it != g_mockFdTable.end()) {
        return &(it->second);
    }
    return nullptr;
}
void FreeMockFd(int fd)
{
    g_mockFdTable.erase(fd);
}
}
extern "C" {
void DlsymMockEnable(void)
{
    g_dlsymMockEnabled = true;
    DebugLog("DlsymMockEnable", "Mock enabled");
}
void DlsymMockDisable(void)
{
    g_dlsymMockEnabled = false;
    DebugLog("DlsymMockDisable", "Mock disabled");
}
void DlsymMockSetFileContent(const char* path, const char* content)
{
    if (path == nullptr) {
        return;
    }
    std::string key(path);
    g_mockFileContent[key] = content ? std::string(content) : std::string();
    g_mockFileExists.insert(key);
    DebugLog("DlsymMockSetFileContent", "Set content for %s, size=%zu", path, g_mockFileContent[key].size());
}
void DlsymMockSetFileExists(const char* path, int exists)
{
    if (path == nullptr) {
        return;
    }
    std::string key(path);
    if (exists) {
        g_mockFileExists.insert(key);
        DebugLog("DlsymMockSetFileExists", "Mark file exists: %s", path);
    } else {
        g_mockFileExists.erase(key);
        g_mockFileContent.erase(key);
        DebugLog("DlsymMockSetFileExists", "Mark file not exists: %s", path);
    }
}
void DlsymMockClearAll(void)
{
    g_mockFileContent.clear();
    g_mockFileExists.clear();
    g_mockFdTable.clear();
    DebugLog("DlsymMockClearAll", "All mock data cleared");
}
int DlsymMockIsEnabled(void)
{
    return g_dlsymMockEnabled ? 1 : 0;
}
void DlsymMockSetDebugMode(int enable)
{
    g_debugMode = enable ? true : false;
}
int access(const char* pathname, int mode)
{
    DebugLog("access", ">>> ENTER");
    DlsymAccessFuncPtr realAccess = reinterpret_cast<DlsymAccessFuncPtr>(dlsym(RTLD_NEXT, "access"));
    if (!g_dlsymMockEnabled) {
        if (realAccess == nullptr) {
            errno = ENOSYS;
            return -1;
        }
        DebugLog("access", ">>> PASSING THROUGH (mock disabled) path=%s", pathname ? pathname : "NULL");
        return realAccess(pathname, mode);
    }
    if (pathname == nullptr) {
        errno = EFAULT;
        return -1;
    }
    std::string key(pathname);
    bool hasContent = g_mockFileContent.find(key) != g_mockFileContent.end();
    bool markedExists = g_mockFileExists.find(key) != g_mockFileExists.end();
    if (hasContent || markedExists) {
        DebugLog("access", ">>> MOCKED: file exists: %s", pathname);
        return 0;
    }
    if (realAccess == nullptr) {
        DebugLog("access", ">>> MOCKED: file not in mock list: %s (returning ENOENT)", pathname);
        errno = ENOENT;
        return -1;
    }
    DebugLog("access", ">>> PASSING THROUGH (not in mock list) path=%s", pathname);
    return realAccess(pathname, mode);
}
int open(const char* pathname, int flags, ...)
{
    DebugLog("open", ">>> ENTER");
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }
    DlsymOpenFuncPtr realOpen = reinterpret_cast<DlsymOpenFuncPtr>(dlsym(RTLD_NEXT, "open"));
    if (!g_dlsymMockEnabled) {
        if (realOpen == nullptr) {
            errno = ENOSYS;
            return -1;
        }
        DebugLog("open", ">>> PASSING THROUGH (mock disabled) path=%s", pathname ? pathname : "NULL");
        return realOpen(pathname, flags, mode);
    }
    if (pathname == nullptr) {
        errno = EFAULT;
        return -1;
    }
    std::string key(pathname);
    auto contentIt = g_mockFileContent.find(key);
    if (contentIt != g_mockFileContent.end()) {
        int fd = AllocateMockFd(key, contentIt->second);
        DebugLog("open", ">>> MOCKED: opened mock file: %s, fd=%d", pathname, fd);
        return fd;
    }
    if (g_mockFileExists.find(key) != g_mockFileExists.end()) {
        int fd = AllocateMockFd(key, "");
        DebugLog("open", ">>> MOCKED: opened empty mock file: %s, fd=%d", pathname, fd);
        return fd;
    }
    if (realOpen == nullptr) {
        DebugLog("open", ">>> MOCKED: file not in mock list: %s (returning ENOENT)", pathname);
        errno = ENOENT;
        return -1;
    }
    DebugLog("open", ">>> PASSING THROUGH (not in mock list) path=%s", pathname);
    return realOpen(pathname, flags, mode);
}
ssize_t read(int fd, void* buf, size_t count)
{
    DebugLog("read", ">>> ENTER");
    DlsymReadFuncPtr realRead = reinterpret_cast<DlsymReadFuncPtr>(dlsym(RTLD_NEXT, "read"));
    if (!g_dlsymMockEnabled) {
        if (realRead == nullptr) {
            errno = ENOSYS;
            return -1;
        }
        DebugLog("read", ">>> PASSING THROUGH (mock disabled) fd=%d", fd);
        return realRead(fd, buf, count);
    }
    MockFileDescriptor* mockFd = GetMockFd(fd);
    if (mockFd == nullptr) {
        if (realRead == nullptr) {
            errno = ENOSYS;
            return -1;
        }
        DebugLog("read", ">>> PASSING THROUGH (not a mock fd) fd=%d", fd);
        return realRead(fd, buf, count);
    }
    if (buf == nullptr) {
        DebugLog("read", ">>> ERROR: buf is NULL");
        errno = EFAULT;
        return -1;
    }
    size_t remaining = mockFd->content.size() - mockFd->readPos;
    size_t toRead = (count < remaining) ? count : remaining;
    if (toRead > 0) {
        errno_t ret = memcpy_s(buf, count, mockFd->content.data() + mockFd->readPos, toRead);
        if (ret != EOK) {
            DebugLog("read", ">>> ERROR: memcpy_s failed with %d", ret);
            errno = EIO;
            return -1;
        }
        mockFd->readPos += toRead;
    }
    DebugLog("read", ">>> MOCKED: read %zu bytes from mock fd=%d", toRead, fd);
    return static_cast<ssize_t>(toRead);
}
ssize_t write(int fd, const void* buf, size_t count)
{
    DebugLog("write", ">>> ENTER");
    DlsymWriteFuncPtr realWrite = reinterpret_cast<DlsymWriteFuncPtr>(dlsym(RTLD_NEXT, "write"));
    if (!g_dlsymMockEnabled) {
        if (realWrite == nullptr) {
            errno = ENOSYS;
            return -1;
        }
        DebugLog("write", ">>> PASSING THROUGH (mock disabled) fd=%d", fd);
        return realWrite(fd, buf, count);
    }
    if (GetMockFd(fd) != nullptr) {
        DebugLog("write", ">>> MOCKED: write to mock fd not allowed, fd=%d", fd);
        errno = EROFS;
        return -1;
    }
    if (realWrite == nullptr) {
        errno = ENOSYS;
        return -1;
    }
    DebugLog("write", ">>> PASSING THROUGH to real write, fd=%d", fd);
    return realWrite(fd, buf, count);
}
int close(int fd)
{
    DebugLog("close", ">>> ENTER");
    DlsymCloseFuncPtr realClose = reinterpret_cast<DlsymCloseFuncPtr>(dlsym(RTLD_NEXT, "close"));
    if (!g_dlsymMockEnabled) {
        if (realClose == nullptr) {
            errno = ENOSYS;
            return -1;
        }
        DebugLog("close", ">>> PASSING THROUGH (mock disabled) fd=%d", fd);
        return realClose(fd);
    }
    if (GetMockFd(fd) != nullptr) {
        FreeMockFd(fd);
        DebugLog("close", ">>> MOCKED: closed mock fd=%d", fd);
        return 0;
    }
    if (realClose == nullptr) {
        errno = ENOSYS;
        return -1;
    }
    DebugLog("close", ">>> PASSING THROUGH to real close, fd=%d", fd);
    return realClose(fd);
}
off_t lseek(int fd, off_t offset, int whence)
{
    DebugLog("lseek", ">>> ENTER");
    DlsymLseekFuncPtr realLseek = reinterpret_cast<DlsymLseekFuncPtr>(dlsym(RTLD_NEXT, "lseek"));
    if (!g_dlsymMockEnabled) {
        if (realLseek == nullptr) {
            errno = ENOSYS;
            return -1;
        }
        DebugLog("lseek", ">>> PASSING THROUGH (mock disabled) fd=%d", fd);
        return realLseek(fd, offset, whence);
    }
    MockFileDescriptor* mockFd = GetMockFd(fd);
    if (mockFd == nullptr) {
        if (realLseek == nullptr) {
            errno = ENOSYS;
            return -1;
        }
        DebugLog("lseek", ">>> PASSING THROUGH (not a mock fd) fd=%d", fd);
        return realLseek(fd, offset, whence);
    }
    off_t newPos = 0;
    switch (whence) {
        case SEEK_SET:
            newPos = offset;
            break;
        case SEEK_CUR:
            newPos = static_cast<off_t>(mockFd->readPos) + offset;
            break;
        case SEEK_END:
            newPos = static_cast<off_t>(mockFd->content.size()) + offset;
            break;
        default:
            errno = EINVAL;
            return -1;
    }
    if (newPos < 0) {
        errno = EINVAL;
        return -1;
    }
    if (static_cast<size_t>(newPos) > mockFd->content.size()) {
        mockFd->readPos = mockFd->content.size();
    } else {
        mockFd->readPos = static_cast<size_t>(newPos);
    }
    DebugLog("lseek", ">>> MOCKED: new pos=%ld, fd=%d", static_cast<long>(newPos), fd);
    return newPos;
}
int stat(const char* pathname, struct stat* statbuf)
{
    DebugLog("stat", ">>> ENTER");
    DlsymStatFuncPtr realStat = reinterpret_cast<DlsymStatFuncPtr>(dlsym(RTLD_NEXT, "stat"));
    if (!g_dlsymMockEnabled) {
        if (realStat == nullptr) {
            errno = ENOSYS;
            return -1;
        }
        DebugLog("stat", ">>> PASSING THROUGH (mock disabled) path=%s", pathname ? pathname : "NULL");
        return realStat(pathname, statbuf);
    }
    if (pathname == nullptr || statbuf == nullptr) {
        DebugLog("stat", ">>> ERROR: pathname or statbuf is NULL");
        errno = EFAULT;
        return -1;
    }
    std::string key(pathname);
    auto contentIt = g_mockFileContent.find(key);
    if (contentIt != g_mockFileContent.end()) {
        errno_t ret = memset_s(statbuf, sizeof(struct stat), 0, sizeof(struct stat));
        if (ret != EOK) {
            DebugLog("stat", ">>> ERROR: memset_s failed with %d", ret);
        }
        statbuf->st_mode = S_IFREG | STAT_FILE_MODE;
        statbuf->st_size = static_cast<off_t>(contentIt->second.size());
        statbuf->st_blksize = FILE_BLOCK_SIZE;
        statbuf->st_blocks = (contentIt->second.size() + FILE_BLOCK_MASK) / (FILE_BLOCK_MASK + 1);
        DebugLog("stat", ">>> MOCKED: returning stat for %s, size=%zu", pathname,
                 static_cast<size_t>(statbuf->st_size));
        return 0;
    }
    if (g_mockFileExists.find(key) != g_mockFileExists.end()) {
        errno_t ret = memset_s(statbuf, sizeof(struct stat), 0, sizeof(struct stat));
        if (ret != EOK) {
            DebugLog("stat", ">>> ERROR: memset_s failed with %d", ret);
        }
        statbuf->st_mode = S_IFREG | STAT_FILE_MODE;
        statbuf->st_size = 0;
        statbuf->st_blksize = FILE_BLOCK_SIZE;
        DebugLog("stat", ">>> MOCKED: returning empty stat (file exists only) %s", pathname);
        return 0;
    }
    if (realStat == nullptr) {
        DebugLog("stat", ">>> MOCKED: file not in mock list: %s (returning ENOENT)", pathname);
        errno = ENOENT;
        return -1;
    }
    DebugLog("stat", ">>> PASSING THROUGH (not in mock list) path=%s", pathname);
    return realStat(pathname, statbuf);
}
int fstat(int fd, struct stat* statbuf)
{
    DebugLog("fstat", ">>> ENTER");
    DlsymFstatFuncPtr realFstat = reinterpret_cast<DlsymFstatFuncPtr>(dlsym(RTLD_NEXT, "fstat"));
    if (!g_dlsymMockEnabled) {
        if (realFstat == nullptr) {
            errno = ENOSYS;
            return -1;
        }
        DebugLog("fstat", ">>> PASSING THROUGH (mock disabled) fd=%d", fd);
        return realFstat(fd, statbuf);
    }
    if (statbuf == nullptr) {
        DebugLog("fstat", ">>> ERROR: statbuf is NULL");
        errno = EFAULT;
        return -1;
    }
    MockFileDescriptor* mockFd = GetMockFd(fd);
    if (mockFd != nullptr) {
        errno_t ret = memset_s(statbuf, sizeof(struct stat), 0, sizeof(struct stat));
        if (ret != EOK) {
            DebugLog("fstat", ">>> ERROR: memset_s failed with %d", ret);
        }
        statbuf->st_mode = S_IFREG | STAT_FILE_MODE;
        statbuf->st_size = static_cast<off_t>(mockFd->content.size());
        statbuf->st_blksize = FILE_BLOCK_SIZE;
        statbuf->st_blocks = (mockFd->content.size() + FILE_BLOCK_MASK) / (FILE_BLOCK_MASK + 1);
        DebugLog("fstat", ">>> MOCKED: fd=%d is mock fd, size=%zu", fd, mockFd->content.size());
        return 0;
    }
    if (realFstat == nullptr) {
        errno = ENOSYS;
        return -1;
    }
    DebugLog("fstat", ">>> PASSING THROUGH (not a mock fd) fd=%d", fd);
    return realFstat(fd, statbuf);
}
FILE* fopen(const char* pathname, const char* mode)
{
    DebugLog("fopen", ">>> ENTER");
    DlsymFopenFuncPtr realFopen = reinterpret_cast<DlsymFopenFuncPtr>(dlsym(RTLD_NEXT, "fopen"));
    if (!g_dlsymMockEnabled) {
        if (realFopen == nullptr) {
            errno = ENOSYS;
            return nullptr;
        }
        DebugLog("fopen", ">>> PASSING THROUGH (mock disabled) path=%s", pathname ? pathname : "NULL");
        return realFopen(pathname, mode);
    }
    if (pathname == nullptr) {
        DebugLog("fopen", ">>> ERROR: pathname is NULL");
        errno = EFAULT;
        return nullptr;
    }
    std::string key(pathname);
    auto contentIt = g_mockFileContent.find(key);
    if (contentIt != g_mockFileContent.end()) {
        FILE* memfile = fmemopen(const_cast<char*>(contentIt->second.c_str()),
            contentIt->second.size(), mode);
        DebugLog("fopen", ">>> MOCKED: opened mock file via fmemopen: %s, size=%zu",
                 pathname, contentIt->second.size());
        return memfile;
    }
    if (g_mockFileExists.find(key) != g_mockFileExists.end()) {
        static char emptyBuf[1] = {0};
        FILE* memfile = fmemopen(emptyBuf, 0, mode);
        DebugLog("fopen", ">>> MOCKED: opened empty mock file via fmemopen: %s", pathname);
        return memfile;
    }
    if (realFopen == nullptr) {
        DebugLog("fopen", ">>> MOCKED: file not in mock list: %s (returning ENOENT)", pathname);
        errno = ENOENT;
        return nullptr;
    }
    DebugLog("fopen", ">>> PASSING THROUGH (not in mock list) path=%s", pathname);
    return realFopen(pathname, mode);
}
}
