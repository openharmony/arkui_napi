# 模块管理知识

本文只记录 `module_manager/` 目录下的模块注册、加载和校验逻辑。公共/内部 API 声明见 `docs/knowledge/napi-differences.md`，构建配置见 `docs/knowledge/build-platform.md`。

## 核心模型

ModuleManager 负责原生模块的注册、加载、缓存和校验。模块通过 `napi_module_register` 注册，运行时按 `nm_modname` 加载对应 .so 并执行注册回调。

```
napi_module_register(&module)        ← 模块注册入口（constructor 属性）
  │
  ├─ NativeModuleManager             ← 模块信息存储与查找
  │   ├─ 模块缓存（避免重复加载）
  │   └─ .so 动态加载
  │
  ├─ ModuleLoadChecker               ← 加载前校验
  │   └─ ModuleCheckerDelegate       ← 校验委托（权限/来源/白名单）
  │
  └─ 注册回调 nm_register_func        ← 执行模块导出逻辑
```

加载时序分三阶段：(A) .so 加载时 constructor 属性触发 `napi_module_register` → `NativeModuleManager::Register()` 将模块插入链表（registerCallback 记录但未执行）；(B) 运行时 `LoadNativeModule` 先查缓存 `FindNativeModuleByCache`（key=moduleName 或 prefix+moduleName），未命中加锁调用 `FindNativeModuleByDisk` → 先 `ModuleLoadChecker::CheckModuleLoadable` 校验 → `LoadModuleLibrary` → dlopen → constructor 自动 Register → 查找 `napi_onLoad` 符号执行；(C) ArkNativeEngine 执行 `module->registerCallback(env, exportObj)` 即 nm_register_func（ark_native_engine.cpp:878）。系统模块 .so 路径：ARM64=`/system/lib64/module/lib{moduleName}.z.so`，ARM32=`/system/lib/module/...`；App 模块用 `dlopen_ns` namespace 机制。校验失败返回 nullptr 阻止加载，errInfo="module xxx is in blocklist"（native_module_manager.cpp:866-872）。

## 边界与身份

| 概念 | 是什么 | 不是什么 | 常见误用 |
|---|---|---|---|
| napi_module | 模块描述结构体 | 模块实例 | 用同一结构体多次注册不更新版本 |
| NativeModuleManager | 模块加载和缓存管理器 | 模块本身 | 把业务逻辑写进 manager |
| ModuleLoadChecker | 加载前校验器 | 权限系统 | 误以为是权限授予入口 |
| ModuleCheckerDelegate | 校验委托接口 | 具体校验实现 | 在 delegate 中写硬编码校验逻辑 |
| nm_register_func | 模块导出回调 | 模块构造函数 | 在注册回调中做初始化副作用 |

## 约束规则

- 不要在 nm_register_func 中执行重初始化或阻塞操作。原因：注册回调在模块加载路径上，阻塞影响启动时序。
- 必须通过 napi_module_register 注册模块，不要手动加载 .so。原因：绕过 ModuleManager 会破坏缓存和校验。
- 不要修改已注册模块的 nm_modname。原因：模块名是加载查找 key，变更导致缓存失效和重复加载。
- 只适合在 ModuleCheckerDelegate 中实现校验策略委托，具体策略由外部注入。原因：校验策略因平台/场景而异，不应硬编码。
- 避免 在 ModuleManager 中持有模块的业务对象。原因：Manager 只管加载和缓存，业务对象归模块自身。

.so 路径规则：无点号模块拼 `lib{moduleName}.z.so` + `lib{moduleName}_napi.z.so`；有点号模块（@ohos.xxx）点号转 `/`；含 .so 后缀直接用。模块版本兼容性：`nm_version` 仅作元数据透传，代码中无版本校验逻辑（NAPI_MODULE_VERSION=1 固定）。卸载时序：`UnloadNativeModule` → `GetNativeModuleHandle` → `RemoveNativeModule`（清缓存链表+map）→ `UnloadModuleLibrary` → `dlclose`。

### 历史坑与反模式

- `nm_version` 仅作元数据透传，代码中无版本校验逻辑——不要假设版本号有兼容性检查。
- .so 句柄无引用计数——缓存命中不会重复 dlopen，但也不支持同一 .so 的多次加载计数。
- `NativeModule.refCount` 字段存在但从未用于引用计数逻辑，只是从 `napi_module` 原样拷贝。

## 状态与生命周期

| 对象 | 生命周期归属 | 创建时机 | 清理触发 | 注意事项 |
|---|---|---|---|---|
| napi_module | 静态存储 | constructor 执行 | 进程退出 | 不可变，注册后不改 |
| 模块缓存项 | NativeModuleManager | 初次加载 | 进程退出/手动卸载 | 支持 UnloadNativeModule→dlclose |
| .so 句柄 | NativeModuleManager | dlopen | dlclose/退出 | 无引用计数，缓存命中不重复 dlopen |
| 校验会话 | ModuleLoadChecker | 每次加载 | 加载完成 | 无跨调用状态，仅持有 shared_ptr<delegate>+shared_mutex |

## 修改前检查

- [ ] 模块注册路径是否完整（register → manager → checker → load）？
- [ ] 模块名是否唯一，是否与已有模块冲突？
- [ ] .so 加载路径是否符合平台规则？
- [ ] 校验逻辑是否通过 delegate 注入，而非硬编码？
- [ ] 缓存策略改动是否影响热加载/卸载场景？
- [ ] 新增模块是否已在 `napi.gni` 或对应 BUILD.gn 中声明？

## 代码和测试

### 代码锚点

| 关键流程 | 入口路径 |
|---|---|
| NativeModuleManager 定义 | `module_manager/native_module_manager.h` |
| NativeModuleManager 实现 | `module_manager/native_module_manager.cpp` |
| ModuleLoadChecker | `module_manager/module_load_checker.{h,cpp}` |
| ModuleCheckerDelegate | `module_manager/module_checker_delegate.{h,cpp}` |

### 测试锚点

| 测试类型 | 路径/类名 |
|---|---|
| 单元测试 | `module_manager/test/unittest/module_manager_test/` |
| Mock | `module_manager/test/mock/` |
| Fuzz: 加载 Ark 模块 | `test/fuzztest/loadarkmodule_fuzzer/` |
