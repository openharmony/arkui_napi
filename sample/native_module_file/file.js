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

const fileModule = require('libnapi_file.so');

module.exports = {
  // 文件操作
  readFile: fileModule.readFile,
  writeFile: fileModule.writeFile,
  appendFile: fileModule.appendFile,
  deleteFile: fileModule.deleteFile,
  copyFile: fileModule.copyFile,
  moveFile: fileModule.moveFile,
  renameFile: fileModule.renameFile,
  // 目录操作
  createDirectory: fileModule.createDirectory,
  deleteDirectory: fileModule.deleteDirectory,
  listDirectory: fileModule.listDirectory,
  // 文件信息
  fileExists: fileModule.fileExists,
  isDirectory: fileModule.isDirectory,
  isFile: fileModule.isFile,
  getFileSize: fileModule.getFileSize,
  getFileStats: fileModule.getFileStats,
  getFilePermissions: fileModule.getFilePermissions,
  setFilePermissions: fileModule.setFilePermissions,
  // 路径操作
  joinPath: fileModule.joinPath,
  basename: fileModule.basename,
  dirname: fileModule.dirname,
  extname: fileModule.extname,
  normalizePath: fileModule.normalizePath,
  absolutePath: fileModule.absolutePath,
  // 临时文件
  createTempFile: fileModule.createTempFile,
  createTempDirectory: fileModule.createTempDirectory,
  // 文件系统信息
  getFreeSpace: fileModule.getFreeSpace,
  getTotalSpace: fileModule.getTotalSpace,
  // 文件监控
  watchFile: fileModule.watchFile,
  unwatchFile: fileModule.unwatchFile,
  // 文件读取选项
  readFileSync: fileModule.readFileSync,
  writeFileSync: fileModule.writeFileSync,
  // 其他文件操作
  truncateFile: fileModule.truncateFile,
  chmod: fileModule.chmod,
  chown: fileModule.chown
};
