# Cangjie FFI 互操作知识

本文只记录 `interfaces/inner_api/cjffi/` 目录下的 Cangjie 语言与 Ark 引擎互操作层。Ark 引擎实现见 `docs/knowledge/engine.md`，API 分层见 `docs/knowledge/napi-differences.md`，构建配置见 `docs/knowledge/build-platform.md`。

## 核心模型

CJ FFI（Cangjie Foreign Function Interface）是 Cangjie 语言与 Ark JS 引擎的互操作层，提供 Cangjie 代码调用 Ark/NAPI 能力的桥接。

```
interfaces/inner_api/cjffi/
  ├─ ark_interop/                ← Ark 互操作核心
  │   ├─ ark_interop_engine      ← 引擎互操作
  │   ├─ ark_interop_napi        ← NAPI 互操作
  │   ├─ ark_interop_object      ← 对象互操作
  │   ├─ ark_interop_scope       ← 作用域互操作
  │   ├─ ark_interop_string      ← 字符串互操作
  │   ├─ ark_interop_bigint      ← BigInt 互操作
  │   ├─ ark_interop_symbol      ← Symbol 互操作
  │   ├─ ark_interop_global      ← 全局对象互操作
  │   ├─ ark_interop_module      ← 模块互操作
  │   ├─ ark_interop_async       ← 异步互操作
  │   ├─ ark_interop_hitrace     ← HiTrace 集成
  │   ├─ cj_envsetup             ← Cangjie 环境初始化
  │   └─ uv_loop_handler         ← libuv 事件循环处理
  │
  ├─ cj_ffi/                     ← Cangjie FFI 通用层
  │   ├─ cj_common_ffi           ← 通用 FFI
  │   └─ cj_data_ffi             ← 数据 FFI
  │
  ├─ cj_backtrace/               ← 回溯支持
  │
  └─ native/                     ← 原生 FFI 底层
      ├─ cj_fn_invoker           ← 函数调用器
      ├─ cj_lambda               ← Lambda 支持
      ├─ ffi_remote_data         ← 远程数据 FFI
      └── runtimetype             ← 运行时类型
```

初始化时序：ArkNativeEngine 先初始化（已存在 VM），CJ 运行时后初始化。ArkNativeEngine 检测 CJ 模块（`.cjmetadata` ELF section）→ `LoadCJModule` → `initCJAppNS`→`startRuntime`→`startUIScheduler` → 加载 `libark_interop.z.so` → `ARKTS_LoadModuleByNapiEnv` → `FixStackOverflow` 注册 Ark VM 地址到 CJ 运行时 + 注册堆栈回调（cj_support.cpp:224-246, ark_interop_module.cpp:77-88）。调用方向：Cangjie→Ark 走 `ARKTS_Call`/`ARKTS_New`；Ark→Cangjie 走 `CJLambdaInvoker`→`g_cjModuleCallbacks->invokerLambda`。uv_loop 集成：uv_loop **嵌入** Ark EventHandler——`UVLoopHandler`(继承 `FileDescriptorListener`) 监听 uv backend fd，触发时 `uv_run(UV_RUN_NOWAIT)`，timer 通过 EventHandler PostTask 调度（ark_interop_engine.cpp:56-95,117-145）。

## 边界与身份

| 概念 | 是什么 | 不是什么 | 常见误用 |
|---|---|---|---|
| ark_interop | Cangjie↔Ark 互操作桥接 | 独立引擎 | 当作 Cangjie JS 引擎 |
| cj_ffi | Cangjie 通用 FFI 层 | ark_interop | 混淆通用 FFI 和 Ark 特定互操作 |
| cj_envsetup | Cangjie 运行环境初始化 | 引擎初始化 | 与 ArkNativeEngine 初始化混淆 |
| uv_loop_handler | libuv 事件循环适配 | Ark 事件循环 | 当作 Ark 自有循环 |
| cj_fn_invoker | Cangjie 函数调用器 | JS 函数调用 | 混淆调用方向 |
| cj_backtrace | Cangjie 回溯支持 | Ark 堆栈 | 与 Ark 堆栈混淆 |

## 分类差异

| 互操作方向 | 模块 | 调用方 | 被调方 | 线程约束 |
|---|---|---|---|---|
| Cangjie → Ark | ark_interop_napi | Cangjie 代码 | Ark/NAPI | 需在 Ark VM 线程执行（直接操作 EcmaVM） |
| Ark → Cangjie | cj_fn_invoker | Ark JS | Cangjie 函数 | 需在 Ark VM 线程执行（CJLambdaInvoker 由 VM 调度） |
| 数据传递 | cj_data_ffi | 双向 | — | 无显式线程约束，但 cj_backtrace 仅限主线程 |

## 约束规则

- 不要 混淆 ark_interop（Ark 特定）和 cj_ffi（通用 FFI）的职责。原因：前者绑定 Ark 引擎，后者是语言层通用。
- 不要 绕过 cj_envsetup 直接初始化 Cangjie 运行时。原因：envsetup 处理与 Ark 的协同初始化，绕过破坏时序。
- 必须 在互操作调用中确保线程约束。原因：Ark VM 非线程安全，Cangjie 侧线程模型可能不同。
- 必须 在修改 ark_interop 时同步检查 cj_support（`native_engine/impl/ark/cj_support`）。原因：二者是互操作的两端，接口需一致。
- 避免 在 uv_loop_handler 中写 Ark 特定调度逻辑。原因：handler 是 libuv 适配层，Ark 调度归引擎。

类型映射：10 种 ARKTS_ValueType（N_UNDEFINED/N_NULL/N_NUMBER/N_BOOL/N_BIGINT/N_STRING/N_SYMBOL/N_OBJECT/N_FUNCTION/N_EXTERNAL），内联值用 JSTaggedValue 编码，堆对象存指针。字符串创建有 STRING_TABLE_THRESHOLD=128 约束。`ARKTS_Promise` 与 `ARKTS_Value` 不可互转。异常传播：Ark→CJ 走 `g_cjModuleCallbacks->throwJSError`/`throwNativeError`（ark_interop_module.cpp:44-61）；CJ→Ark 走 `ARKTS_Throw`→`JSNApi::ThrowException`（ark_interop_napi.cpp:831-848）。backtrace 拼接：`ARKTS_UpdateStackInfo` 在互操作边界切换 Ark VM 堆栈上下文（SWITCH_TO_SUB/MAIN_STACK_INFO），`CJDFX_GetHybridStack` 收集 native 帧并通过 translator 回调与 Ark 堆栈融合（ark_interop_napi.cpp:889-895, cj_backtrace.cpp:77-103）。

## 状态与生命周期

| 对象 | 生命周期归属 | 创建时机 | 清理触发 | 注意事项 |
|---|---|---|---|---|
| Cangjie 环境 | cj_envsetup | 引擎初始化 | 引擎销毁 | 与 Ark env 协同 |
| uv_loop_handler | ark_interop | 事件循环初始化 | 事件循环退出 | uv_loop 嵌入 Ark EventHandler，非独立运行 |
| 互操作对象句柄 | ark_interop_object | 互操作创建 | 互操作释放 | Ark GC 管理对象回收；CJ native 数据通过 deleter 回调委托回 Cangjie（deleteExternal/deleteArrayBufferRawData/deleteLambda） |

## 修改前检查

- [ ] 改动是 ark_interop（Ark 特定）还是 cj_ffi（通用）？
- [ ] 互操作两端（ark_interop 与 cj_support）是否同步更新？
- [ ] 线程约束是否满足（Ark VM 线程安全）？
- [ ] Cangjie 环境初始化时序是否破坏？
- [ ] uv_loop 与 Ark 事件循环的集成是否正确？
- [ ] 类型映射和异常传播是否完整？
- [ ] 新增源文件是否在对应 `BUILD.gn` 中声明？

## 代码和测试

### 代码锚点

| 关键流程 | 入口路径 |
|---|---|
| Ark 互操作-引擎 | `interfaces/inner_api/cjffi/ark_interop/ark_interop_engine.cpp` |
| Ark 互操作-NAPI | `interfaces/inner_api/cjffi/ark_interop/ark_interop_napi.{h,cpp}` |
| Ark 互操作-对象 | `interfaces/inner_api/cjffi/ark_interop/ark_interop_object.cpp` |
| Ark 互操作-作用域 | `interfaces/inner_api/cjffi/ark_interop/ark_interop_scope.{h,cpp}` |
| Ark 互操作-字符串 | `interfaces/inner_api/cjffi/ark_interop/ark_interop_string.cpp` |
| Ark 互操作-BigInt | `interfaces/inner_api/cjffi/ark_interop/ark_interop_bigint.cpp` |
| Ark 互操作-Symbol | `interfaces/inner_api/cjffi/ark_interop/ark_interop_symbol.cpp` |
| Ark 互操作-全局 | `interfaces/inner_api/cjffi/ark_interop/ark_interop_global.cpp` |
| Ark 互操作-模块 | `interfaces/inner_api/cjffi/ark_interop/ark_interop_module.cpp` |
| Ark 互操作-异步 | `interfaces/inner_api/cjffi/ark_interop/ark_interop_async.cpp` |
| Ark 互操作-HiTrace | `interfaces/inner_api/cjffi/ark_interop/ark_interop_hitrace.{h,cpp}` |
| Cangjie 环境初始化 | `interfaces/inner_api/cjffi/ark_interop/cj_envsetup.{h,cpp}` |
| uv 循环处理 | `interfaces/inner_api/cjffi/ark_interop/uv_loop_handler.h` |
| 互操作内部头 | `interfaces/inner_api/cjffi/ark_interop/ark_interop_internal.h` |
| 互操作外部头 | `interfaces/inner_api/cjffi/ark_interop/ark_interop_external.h` |
| 互操作宏 | `interfaces/inner_api/cjffi/ark_interop/ark_interop_macro.h` |
| 互操作日志 | `interfaces/inner_api/cjffi/ark_interop/ark_interop_log.h` |
| 通用 FFI | `interfaces/inner_api/cjffi/cj_ffi/cj_common_ffi.{h,cpp}` |
| 数据 FFI | `interfaces/inner_api/cjffi/cj_ffi/cj_data_ffi.{h,cpp}` |
| 回溯 | `interfaces/inner_api/cjffi/cj_backtrace/cj_backtrace.{h,cpp}` |
| 函数调用器 | `interfaces/inner_api/cjffi/native/cj_fn_invoker.{h,cpp}` |
| Lambda 支持 | `interfaces/inner_api/cjffi/native/cj_lambda.h` |
| 远程数据 FFI | `interfaces/inner_api/cjffi/native/ffi_remote_data.{h,cpp}` |
| 运行时类型 | `interfaces/inner_api/cjffi/native/runtimetype.h` |
| Cangjie 支持（引擎侧） | `native_engine/impl/ark/cj_support.{h,cpp}` |

### 测试锚点

| 测试类型 | 路径/类名 |
|---|---|
| 互操作通用测试 | `test/unittest/cj_native/test_ark_interop.{cpp,h}` |
| 互操作通用辅助 | `test/unittest/cj_native/test_ark_interop_common.{cpp,h}` |
| 互操作覆盖测试 | `test/unittest/cj_native/test_ark_interop_coverage.cpp` |
| 回溯测试 | `test/unittest/cj_native/test_cj_backtrace.cpp` |
| FFI 数据测试 | `test/unittest/cj_native/test_ffi_data.cpp` |
