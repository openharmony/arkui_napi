# 引擎与实现知识

本文记录 NativeEngine 抽象层 OH 差异和 Ark 引擎实现。与 Node.js N-API 一致的基础概念不再展开。异步/引用/API/错误差异见 `docs/knowledge/napi-differences.md`，模块加载见 `docs/knowledge/module-manager.md`，Cangjie 互操作见 `docs/knowledge/cjffi.md`，构建见 `docs/knowledge/build-platform.md`。

## 核心模型

ArkNativeEngine 继承 NativeEngine，桥接 Ark VM 与 NAPI 抽象层。构造函数执行 20 步初始化（ark_native_engine.cpp:495-564）：基类 Init → `JSNApi::SetEnv` → 保存 context_ 为 Global → 创建 NapiOptions → 注入 requireNapi 全局函数 → `SetLoop` → 注册弱引用终结回调→PostFinalizeTasks → 注册定时器回调 → `NotifyEnvInitialized` → `ArkIdleMonitor::EnableIdleGC`。

## 与 Node.js N-API 的关键差异

| 概念 | Node.js N-API | OpenHarmony NAPI | 影响 |
|---|---|---|---|
| napi_env → 引擎 | 通过 engine 实例 | `reinterpret_cast<NativeEngine*>(env)`（native_api_internal.h:56,79） | 同一指针两视角 |
| napi_value → JS 值 | engine 特定 | `reinterpret_cast` 到 `panda::Local<panda::JSValueRef>`（native_utils.h:28-36） | 零拷贝，无 "NativeValue" 类 |
| HandleScope | 自有 | `HandleScopeWrapper` 封装 `panda::LocalScope`（native_api.cpp:64-70） | 直接映射 |
| CallbackScope | 自有 | `NativeCallbackScope` 内含 LocalScope+异常+async hook（native_callback_scope_manager.cpp:25） | 独立于 HandleScope |
| NativeScopeManager | — | 已废弃 `// To be delete`（scope_manager/native_scope_manager.h:19） | 不要使用 |
| AsyncWork complete | 在 callback scope 内 | **不在 CallbackScope**，仅 LocalScope+TryCatch（native_async_work.cpp:249,268） | 无 CallbackScope 语义 |
| CriticalScope | — | 封装 `JsiFastNativeScope`，未关闭则 HILOG_FATAL（native_async_work.cpp:282-286） | 独有概念 |
| 容器作用域 | — | 编译期 `napi_enable_container_scope` + 运行时 `persist.ace.napiContainerScope.enabled`（ark_native_engine.cpp:513） | 双重条件 |
| 异常跨作用域 | pending exception | `TryCatch` 析构存入 `lastException_`，`NAPI_PREAMBLE` 检测（native_engine.h:891-905） | 字段传播 |

## Ark 引擎关键组件

| 组件 | 职责 | 关键约束 |
|---|---|---|
| ArkIdleMonitor | 空闲 GC 监控（单例） | 默认 1000ms 间隔；前台需连续 15 周期 idleNotify≤10 且 idleRatio≥0.985 |
| ArkNativeTimer | uv_timer 定时器 | 回调在 uv_loop 线程；一次性回调后自动删除 |
| ArkFinalizersPack | 批量 finalizer | 主线程默认异步，pending>500MB 切同步；env 析构期间跳过 |
| ArkXRefNativeReference | Hybrid 引用（XRef 跨 VM） | refCount>0 强引用，=0 弱引用+finalize |
| NapiOptions | 引擎配置 | 预留位掩码，`ParseProperties()` 为空实现 |

## 约束规则

- 不要在 NativeEngine 基类写引擎特定逻辑。原因：基类是跨引擎契约。
- 不要跨线程传递 napi_env。原因：CROSS_THREAD_CHECK 会检查。
- 不要脱离作用域持有 napi_value。原因：可能被 GC 回收。
- 不要用普通 Scope 替代 CallbackScope。原因：CallbackScope 有异常处理语义。
- 不要跨 Worker 使用普通引用。原因：需 Sendable 变体。
- 必须 在回调作用域关闭后检查 pending exception。
- 不要继续使用 NativeScopeManager。原因：已废弃。

## 历史坑与反模式

- NativeScopeManager 已废弃（`// To be delete`），不要继续使用。
- 代码中不存在 "NativeValue" C++ 类——napi_value 是 JSValueRef 的 reinterpret。
- 容器作用域需编译期+运行时双重条件，仅改 GN 参数不生效。
- AsyncWork complete 不在 CallbackScope 内——不要假设有 CallbackScope 语义。

## 修改前检查

- [ ] 改动是否影响所有引擎实现，还是仅 Ark？
- [ ] napi_value 是否在作用域内使用完毕？
- [ ] 跨作用域持有的值是否已转 NativeReference？
- [ ] 回调是否在 CallbackScope 内执行？
- [ ] env 是否在正确的线程？
- [ ] 引用类型选择是否正确（普通/Sendable/Hybrid）？
- [ ] 容器作用域改动是否同时考虑编译期和运行时？

## 代码和测试

### 代码锚点

| 关键流程 | 入口路径 |
|---|---|
| NativeEngine 基类 | `native_engine/native_engine.{h,cpp}` |
| env→NativeEngine 转换 | `native_engine/native_api_internal.h` |
| napi_value↔JSValueRef | `native_engine/native_utils.h` |
| NativeValue/Property/Deferred/Event | `native_engine/native_value.h`、`native_property.h`、`native_deferred.h`、`native_event.{h,cpp}` |
| NativeContainerScope | `native_engine/native_container_scope.h` |
| env 创建 | `native_engine/native_create_env.{h,cpp}` |
| 回调作用域 | `callback_scope_manager/native_callback_scope_manager.{h,cpp}` |
| ArkNativeEngine | `native_engine/impl/ark/ark_native_engine.{h,cpp}` |
| ArkIdleMonitor | `native_engine/impl/ark/ark_idle_monitor.{h,cpp}` |
| ArkNativeTimer | `native_engine/impl/ark/ark_native_timer.{h,cpp}` |
| ArkFinalizersPack | `native_engine/impl/ark/ark_finalizers_pack.h` |
| ArkNativeOptions | `native_engine/impl/ark/ark_native_options.h` |
| Ark 引用实现 | `native_engine/impl/ark/ark_native_reference.{h,cpp}`、`ark_sendable_native_reference.{h,cpp}`、`ark_hybrid_native_reference.{h,cpp}` |

### 测试锚点

| 测试类型 | 路径/类名 |
|---|---|
| 引擎测试 | `test/unittest/engine/test_ark.cpp` |
| 空闲监控 | `test/unittest/test_ark_idle_monitor.cpp` |
| 基础测试 | `test/unittest/test_napi.cpp` |
| 上下文测试 | `test/unittest/test_napi_context.cpp` |
| pending exception | `test/unittest/test_napi_pendingexception.cpp` |
| 全局引用追踪 | `test/unittest/test_napi_global_ref_track.cpp` |
| 性能/临界 | `test/unittest/test_napi_performance.cpp`、`test_napi_critical.cpp` |
| Hybrid/Sendable | `test/unittest/test_napi_hybrid.cpp`、`test_sendable_napi.cpp` |
| Fuzz | `test/fuzztest/loadarkmodule_fuzzer/`、`executejsbin_fuzzer/`、`runactor_fuzzer/` |
