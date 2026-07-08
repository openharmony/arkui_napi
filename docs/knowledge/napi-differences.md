# NAPI 差异知识

本文记录异步操作、引用管理、API 表面和错误处理的 OpenHarmony 差异点。与 Node.js N-API 一致的基础概念不再展开。引擎与 Ark 实现见 `docs/knowledge/engine.md`，模块加载见 `docs/knowledge/module-manager.md`，Cangjie 互操作见 `docs/knowledge/cjffi.md`，构建见 `docs/knowledge/build-platform.md`。

## 与 Node.js N-API 的关键差异

### 异步操作

| 概念 | Node.js N-API | OpenHarmony NAPI | 影响 |
|---|---|---|---|
| SafeAsyncWork | — | 双重检测：`IsAlive` + `engineId_` 匹配（native_safe_async_work.cpp:166-179） | env 销毁后防崩溃 |
| env 销毁处理 | — | 主 env `Deinit()`→`uv_run` 让 TSFN 自行释放；context env 若 `HasActiveTsfn()` 则 **HILOG_FATAL**（ark_native_engine.cpp:710-735） | 销毁前必须释放 TSFN |
| TSFN 队列 | 自有队列 | `std::deque<void*>`+`std::mutex`（非 lock-free）（native_safe_async_work.h:109-111） | mutex 保护 |
| 任务取消 | uv_cancel | 一致：取消后 complete 仍执行，status=napi_cancelled（native_async_work.cpp:198-208） | complete 始终执行 |
| AsyncHook | async_hooks | 框架在但全部 `Emit*` 为**空 stub**（native_callback_scope_manager.h:26-36） | 功能未实现 |
| 追踪 ID | asyncId | `NewAsyncId()` 固定返回 0（native_engine.h:572-575） | 无实际追踪 |
| Worker 限制 | — | 全局 80(硬编码)/THREAD_WORKER 64(可配)/LIMITED 16(硬编码)/OLD 8(可配)（worker_manager.cpp:25-43） | 有硬上限 |

### 引用管理

| 概念 | Node.js N-API | OpenHarmony NAPI | 影响 |
|---|---|---|---|
| 引用追踪 | — | 侵入式双向链表，仅追踪 `ownership_==RUNTIME`（ark_native_reference.cpp:87-92） | USER-owned 不追踪 |
| Finalizer 时序 | 同步 | 批量：Worker 同步；主线程默认异步，pending>500MB 切同步（ark_native_engine.cpp:2123-2191） | 非立即执行 |
| Sendable 引用 | — | `SendableGlobal<JSValueRef>`，mutex 保护，**无 refCount**（ark_sendable_native_reference.h:34） | 独有类型 |
| Hybrid 引用 | — | `ArkXRefNativeReference`：XRef 跨 VM，refCount 同普通引用（ark_hybrid_native_reference.cpp:28） | 独有类型 |
| Worker 退出清理 | — | Sendable **无自动清理**，必须手动删除（ark_sendable_native_reference.h:30） | 不清理则泄漏 |

### API 表面

| 概念 | Node.js N-API | OpenHarmony NAPI | 影响 |
|---|---|---|---|
| API 版本 | 可变 | `NAPI_VERSION=8`（固定，native_api.h:19） | 与 N-API version 8 对齐 |
| 模块版本 | NAPI_MODULE_VERSION | `=1`，仅透传，**无版本校验**（native_node_api.cpp:45） | 不检查兼容性 |
| API 废弃 | — | `NAPI_INNER_EXTERN` → `__attribute__((__deprecated__))`，仅 `napi_adjust_external_memory` 使用（native_api.h:30-42） | 调用触发警告 |
| 符号版本 | — | `ace_napi.versionscript` 仅 arm64+build_ext_path 启用（BUILD.gn:287-295） | 仅 arm64 校验 |
| Hybrid API | — | `native_node_hybrid_api.h`：XGC 跨引用、stackinfo、hybrid wrap 等 | 非跨引擎标准 |

### 错误处理

| 概念 | Node.js N-API | OpenHarmony NAPI | 影响 |
|---|---|---|---|
| 错误码范围 | 0-21 | 0-21 + 3 个 Ark 专有值(22-24)（js_native_api_types.h:73-99） | 不要假设最大 21 |
| 错误结构 | `napi_extended_error_info`(`napi_status`) | `NativeErrorExtendedInfo`(`int errorCode`)（native_engine.h:53-58） | 类型不同 |
| last_error 存储 | engine 局部 | **env 局部**（`NativeEngine::lastError_`，native_engine.h:707），非 thread_local | 容易误判 |
| throw 与 last_error | throw 后 set | throw 调用 `napi_clear_last_error`（**清零**）（native_api.cpp:2683-2774） | 与直觉相反 |
| exception 优先 | — | `NAPI_PREAMBLE` 先检查 `lastException_.IsEmpty()`，短路返回 `napi_pending_exception`（native_api_internal.h:52-64） | exception > status |

## 约束规则

- 不要在 execute 回调中调用 napi 函数。原因：子线程，VM 非线程安全。
- 必须 任务完成后 delete_async_work 或关闭 TSFN。原因：未清理泄漏。
- 必须 context env 销毁前释放所有 TSFN。原因：否则 HILOG_FATAL。
- 不要跨 Worker 用普通引用。原因：需 Sendable 变体。
- 必须 Worker 退出前手动删除 Sendable 引用。原因：无自动清理。
- 不要修改公共 API 签名/错误码。原因：对外稳定。
- 必须 修改 API 后更新 `ace_napi.versionscript`。
- 必须 线程安全 API 返回错误码时 set/clear_last_error。
- 不要假设 napi_status 最大 21。原因：Ark 专有 22-24。
- 避免 依赖 AsyncHook 和 AsyncId。原因：空 stub / 返回 0。

## 历史坑与反模式

- AsyncHook 全部空 stub——不要依赖 async hook 行为。
- `NewAsyncId()` 返回 0——不要用于追踪。
- 取消任务后 complete 仍执行——不要假设 Cancel 后不回调。
- context env 销毁时活跃 TSFN 会 HILOG_FATAL——必须先释放。
- Sendable 引用无自动清理——Worker 退出必须手动删除。
- throw 调用 `napi_clear_last_error`（清零）——throw 后 last_error 为空。
- `NativeErrorExtendedInfo` 用 `int` 而非 `napi_status`——与标准不同。
- `last_error` 是 env-local 非 thread_local——容易误判。
- `NAPI_INNER_EXTERN` 非测试构建触发废弃警告——调用 `napi_adjust_external_memory` 注意。
- versionscript 仅 arm64 启用——非 arm64 平台不校验符号可见性。
- napi_status 含 3 个 Ark 专有值(22-24)——不要假设最大 21。
- `nm_version` 无版本校验——不要假设有兼容性检查。

## 修改前检查

- [ ] execute 回调中是否完全没有 napi 调用？
- [ ] 异步任务/TSFN 是否有配对 delete/release？
- [ ] context env 销毁前是否释放了所有 TSFN？
- [ ] 跨 Worker 是否用了 Sendable 变体？
- [ ] Sendable 引用是否有手动删除路径？
- [ ] 公共 API 签名/错误码是否变更？（需评审）
- [ ] `ace_napi.versionscript` 是否同步更新？
- [ ] 错误码返回是否配套 set/clear_last_error？
- [ ] 是否避免依赖 AsyncHook/AsyncId？

## 代码和测试

### 代码锚点

| 关键流程 | 入口路径 |
|---|---|
| NativeAsyncWork | `native_engine/native_async_work.{h,cpp}` |
| NativeSafeAsyncWork | `native_engine/native_safe_async_work.{h,cpp}` |
| WorkerManager | `native_engine/worker_manager.{h,cpp}` |
| AsyncContext/HookContext | `native_engine/native_async_context.h`、`native_async_hook_context.h` |
| NativeReference 抽象 | `native_engine/native_reference.h` |
| ReferenceManager | `reference_manager/native_reference_manager.{h,cpp}` |
| NativeSendable | `native_engine/native_sendable.{h,cpp}` |
| 公共 API | `interfaces/kits/napi/native_api.h`、`native_engine/native_api.cpp` |
| 内部 API | `interfaces/inner_api/napi/native_node_api.h`、`native_engine/native_node_api.cpp` |
| Hybrid API | `interfaces/inner_api/napi/native_node_hybrid_api.h`、`native_engine/native_node_hybrid_api.cpp` |
| 转换宏/错误宏 | `native_engine/native_api_internal.h` |
| 符号版本 | `ace_napi.versionscript` |
| NativeErrorExtendedInfo | `native_engine/native_engine.h:53-58` |

### 测试锚点

| 测试类型 | 路径/类名 |
|---|---|
| 线程安全 | `test/unittest/test_napi_threadsafe.cpp` |
| Worker 管理 | `test/unittest/test_worker_manager.cpp` |
| 引用追踪 | `test/unittest/test_napi_global_ref_track.cpp` |
| Sendable | `test/unittest/test_sendable_napi.cpp` |
| 基础/扩展 API | `test/unittest/test_napi.cpp`、`test_napi_ext.cpp` |
| Hybrid/白名单 | `test/unittest/test_napi_hybrid.cpp`、`test_ark_api_allowlist.cpp` |
| 错误码/异常 | `test/unittest/test_napi_errorcode.cpp`、`test_napi_pendingexception.cpp`、`test_napi_critical.cpp` |
| Fuzz | `test/fuzztest/runscriptpath_fuzzer/`、`runscriptbuffer_fuzzer/` |
