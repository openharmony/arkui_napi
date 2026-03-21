# File Operations 示例

## 概述

本示例演示了如何使用 NAPI 实现文件操作功能，这是实际业务开发中最常用的功能之一。文件操作模块展示了：
- 同步和异步文件读取
- 文件写入操作
- 文件信息获取
- 目录遍历
- 文件存在性检查

## 业务场景

在实际业务开发中，文件操作通常用于：

1. **配置文件管理**
   - 读取应用配置
   - 保存用户设置
   - 管理本地缓存

2. **日志处理**
   - 写入日志文件
   - 日志文件轮转
   - 日志压缩和归档

3. **数据持久化**
   - 保存应用数据
   - 读取历史记录
   - 数据导入导出

4. **资源管理**
   - 读取静态资源
   - 管理上传下载的文件
   - 处理临时文件

## 功能说明

### 1. readFileSync(filePath)

同步读取文件内容。

**参数：**
- `filePath`: 文件路径（字符串）

**返回值：**
- string: 文件内容

**异常：**
- 如果文件不存在或读取失败，抛出错误

**示例：**
```javascript
const fileOps = require('file_operations');

try {
    const content = fileOps.readFileSync('/data/config.json');
    console.log('File content:', content);
} catch (error) {
    console.error('Failed to read file:', error);
}
```

### 2. readFileAsync(filePath, callback?)

异步读取文件内容，支持 Promise 和回调两种模式。

**参数：**
- `filePath`: 文件路径（字符串）
- `callback`: 可选回调函数 `(error, content) => void`

**返回值：**
- Promise: 如果没有提供回调，返回 Promise
- undefined: 如果提供了回调

**示例：**

Promise 模式：
```javascript
async function loadConfig() {
    try {
        const content = await fileOps.readFileAsync('/data/config.json');
        const config = JSON.parse(content);
        return config;
    } catch (error) {
        console.error('Failed to load config:', error);
        return null;
    }
}
```

回调模式：
```javascript
fileOps.readFileAsync('/data/config.json', (error, content) => {
    if (error) {
        console.error('Failed to read file:', error);
        return;
    }
    console.log('File content:', content);
});
```

### 3. writeFileSync(filePath, content)

同步写入文件内容。

**参数：**
- `filePath`: 文件路径（字符串）
- `content`: 要写入的内容（字符串）

**返回值：**
- boolean: 是否成功写入

**异常：**
- 如果写入失败，抛出错误

**示例：**
```javascript
const config = {
    theme: 'dark',
    language: 'zh-CN',
    fontSize: 16
};

try {
    const success = fileOps.writeFileSync('/data/config.json', JSON.stringify(config));
    if (success) {
        console.log('Config saved successfully');
    }
} catch (error) {
    console.error('Failed to save config:', error);
}
```

### 4. getFileInfo(filePath)

获取文件信息。

**参数：**
- `filePath`: 文件路径（字符串）

**返回值：**
- Object: 文件信息对象，包含以下属性：
  - `size`: 文件大小（字节）
  - `isDirectory`: 是否为目录
  - `isFile`: 是否为文件
  - `modifiedTime`: 修改时间（时间戳）

**示例：**
```javascript
const info = fileOps.getFileInfo('/data/config.json');
console.log('File size:', info.size);
console.log('Is file:', info.isFile);
console.log('Modified time:', new Date(info.modifiedTime * 1000));
```

### 5. listDirectory(dirPath)

列出目录中的所有文件和子目录。

**参数：**
- `dirPath`: 目录路径（字符串）

**返回值：**
- Array: 文件和目录名称数组

**示例：**
```javascript
const entries = fileOps.listDirectory('/data');
console.log('Directory entries:', entries);

entries.forEach(entry => {
    console.log(' -', entry);
});
```

### 6. fileExists(filePath)

检查文件或目录是否存在。

**参数：**
- `filePath`: 文件或目录路径（字符串）

**返回值：**
- boolean: 是否存在

**示例：**
```javascript
const configPath = '/data/config.json';

if (fileOps.fileExists(configPath)) {
    console.log('Config file exists');
    const content = fileOps.readFileSync(configPath);
} else {
    console.log('Config file not found, using defaults');
}
```

## 代码结构

```
file_operations/
├── BUILD.gn            # GN 构建配置文件
├── file_operations.cpp  # 文件操作实现
└── README.md           # 本文档
```

## 编译和使用

### 编译

在项目根目录执行：
```bash

./build.sh --product-name rk3568 --target-cpu arm64 --build-target file_operations
```

### 使用

在 JavaScript 代码中引入模块：
```javascript
const fileOps = require('file_operations');
```

## 实际业务示例

### 示例 1: 配置管理器

```javascript
class ConfigManager {
    constructor(configPath) {
        this.configPath = configPath;
        this.config = null;
    }

    async load() {
        try {
            const content = await fileOps.readFileAsync(this.configPath);
            this.config = JSON.parse(content);
            return this.config;
        } catch (error) {
            console.error('Failed to load config:', error);
            this.config = this.getDefaultConfig();
            return this.config;
        }
    }

    async save() {
        try {
            const content = JSON.stringify(this.config, null, 2);
            await fileOps.writeFileAsync(this.configPath, content);
            return true;
        } catch (error) {
            console.error('Failed to save config:', error);
            return false;
        }
    }

    get(key) {
        return this.config ? this.config[key] : null;
    }

    set(key, value) {
        if (this.config) {
            this.config[key] = value;
        }
    }

    getDefaultConfig() {
        return {
            theme: 'light',
            language: 'en-US',
            fontSize: 14,
            autoSave: true
        };
    }
}

// 使用配置管理器
const configManager = new ConfigManager('/data/app_config.json');
await configManager.load();

configManager.set('theme', 'dark');
await configManager.save();
```

### 示例 2: 日志管理器

```javascript
class LogManager {
    constructor(logPath) {
        this.logPath = logPath;
        this.maxFileSize = 1024 * 1024; // 1MB
    }

    async log(message) {
        const timestamp = new Date().toISOString();
        const logEntry = `[${timestamp}] ${message}\n`;

        try {
            if (fileOps.fileExists(this.logPath)) {
                const info = fileOps.getFileInfo(this.logPath);
                if (info.size >= this.maxFileSize) {
                    await this.rotateLog();
                }
            }

            const content = await fileOps.readFileAsync(this.logPath);
            const newContent = content + logEntry;
            await fileOps.writeFileAsync(this.logPath, newContent);
        } catch (error) {
            console.error('Failed to write log:', error);
        }
    }

    async rotateLog() {
        const backupPath = this.logPath + '.bak';
        
        try {
            if (fileOps.fileExists(backupPath)) {
                await fileOps.writeFileAsync(backupPath, '');
            }
            
            const content = await fileOps.readFileAsync(this.logPath);
            await fileOps.writeFileAsync(backupPath, content);
            await fileOps.writeFileAsync(this.logPath, '');
        } catch (error) {
            console.error('Failed to rotate log:', error);
        }
    }
}

// 使用日志管理器
const logManager = new LogManager('/data/app.log');
await logManager.log('Application started');
await logManager.log('User logged in');
```

### 示例 3: 缓存管理器

```javascript
class CacheManager {
    constructor(cacheDir) {
        this.cacheDir = cacheDir;
        this.maxCacheSize = 100 * 1024 * 1024; // 100MB
    }

    async get(key) {
        const cachePath = `${this.cacheDir}/${key}.cache`;
        
        if (!fileOps.fileExists(cachePath)) {
            return null;
        }

        try {
            const content = await fileOps.readFileAsync(cachePath);
            return JSON.parse(content);
        } catch (error) {
            console.error('Failed to read cache:', error);
            return null;
        }
    }

    async set(key, value) {
        const cachePath = `${this.cacheDir}/${key}.cache`;
        
        try {
            const content = JSON.stringify(value);
            await fileOps.writeFileAsync(cachePath, content);
            
            await this.checkCacheSize();
        } catch (error) {
            console.error('Failed to write cache:', error);
        }
    }

    async checkCacheSize() {
        let totalSize = 0;
        const entries = fileOps.listDirectory(this.cacheDir);
        
        for (const entry of entries) {
            const filePath = `${this.cacheDir}/${entry}`;
            const info = fileOps.getFileInfo(filePath);
            totalSize += info.size;
        }

        if (totalSize > this.maxCacheSize) {
            await this.cleanupCache();
        }
    }

    async cleanupCache() {
        const entries = fileOps.listDirectory(this.cacheDir);
        const entriesWithTime = [];

        for (const entry of entries) {
            const filePath = `${this.cacheDir}/${entry}`;
            const info = fileOps.getFileInfo(filePath);
            entriesWithTime.push({
                name: entry,
            time: info.modifiedTime
            });
        }

        entriesWithTime.sort((a, b) => a.time - b.time);
        
        const toDelete = Math.floor(entriesWithTime.length / 2);
        for (let i = 0; i < toDelete; i++) {
            const filePath = `${this.cacheDir}/${entriesWithTime[i].name}`;
            await fileOps.writeFileAsync(filePath, '');
        }
    }
}

// 使用缓存管理器
const cacheManager = new CacheManager('/data/cache');
await cacheManager.set('user_data', { name: 'John', age: 30 });
const userData = await cacheManager.get('user_data');
```

## 性能优化建议

1. **使用异步操作**
   - 对于大文件，使用异步读取避免阻塞主线程
   - 批量文件操作时使用 Promise.all 并行处理

2. **缓存文件内容**
   - 频繁读取的文件可以缓存内容
   - 使用文件修改时间判断是否需要重新读取

3. **限制文件大小**
   - 避免读取过大的文件
   - 对于大文件，使用流式读取

4. **批量操作**
   - 批量文件操作时，考虑使用工作线程
   - 避免在循环中进行同步文件操作

## 安全性考虑

1. **路径验证**
   - 验证文件路径，防止路径遍历攻击
   - 限制访问的目录范围

2. **权限检查**
   - 检查文件读写权限
   - 处理权限不足的情况

3. **敏感数据**
   - 避免在日志中记录敏感文件内容
   - 加密存储敏感配置文件

4. **资源限制**
   - 限制文件大小，防止内存耗尽
   - 限制并发文件操作数量

## 错误处理

所有函数都可能抛出错误，建议添加适当的错误处理：

```javascript
try {
    const content = fileOps.readFileSync(filePath);
} catch (error) {
    if (error.message.includes('Failed to open file')) {
        console.error('File not found:', filePath);
    } else if (error.message.includes('File too large')) {
        console.error('File exceeds size limit');
    } else {
        console.error('Unknown error:', error);
    }
}
```

## 注意事项

1. **文件大小限制**
   - 单个文件最大支持 10MB
   - 超过限制会抛出错误

2. **路径格式**
   - 使用完整路径
   - 确保路径分隔符正确

3. **异步操作**
   - 异步操作在工作线程中执行
   - 回调在 JavaScript 线程中执行

4. **并发操作**
   - 多个异步操作可以并行执行
   - 注意文件锁和并发写入问题
