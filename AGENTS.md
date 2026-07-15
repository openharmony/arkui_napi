# AGENTS.md

本文件为 AI Agent 提供在本仓库工作的路由与全局约束。完整领域知识在 `docs/knowledge/`，按需加载，不要全量读取；本文件只做路由和全局约束，不复制知识内容。

## 1. 项目定位

本仓库对应 OpenHarmony 源码树 `//foundation/arkui/napi`，是 NAPI（Node.js N-API 的 ArkTS 引擎实现）的引擎抽象、Ark 引擎实现、模块管理、Cangjie FFI 互操作与构建平台适配的集合体。

最不可破坏的边界：公共 API 稳定性（`interfaces/kits/napi/native_api.h`）与 Ark VM 线程安全不变量。

关键目录（按任务定位问题）：

- `native_engine/`：NativeEngine 抽象层与 Ark 引擎实现（`impl/ark/`）
- `module_manager/`：原生模块注册、加载、缓存、校验
- `interfaces/inner_api/cjffi/`：Cangjie 语言与 Ark 引擎互操作层
- `interfaces/kits/napi/native_api.h`：公共 API（对外，应用/系统模块用）
- `interfaces/inner_api/napi/native_node_api.h`、`native_node_hybrid_api.h`：系统部件间接口，不面向应用开发者开放
- `BUILD.gn`、`napi.gni`、`bundle.json`、`ace_napi.versionscript`：构建配置与符号版本
- `utils/`（含 `platform/`）：跨平台适配、日志、数据保护
- `callback_scope_manager/`、`reference_manager/`：作用域与引用管理辅助
- `test/unittest/`、`test/fuzztest/`：单元测试与 Fuzz 测试

### 子级规则

本仓库当前无子级 AGENTS.md / CLAUDE.md，本文件为唯一入口。如 `module_manager/`、`interfaces/inner_api/cjffi/`、`native_engine/impl/ark/` 等子目录后续需要更细规则，应新增子级 AGENTS.md 并在此声明，避免规则散落在多入口而失同步。

## 2. 构建和测试

构建命令从源码根目录（GN root）执行，不在本子目录执行。`target-cpu` 取值 `arm` 或 `arm64`，需向用户确认。

### 精简 NAPI 源码树（`//foundation/arkui` 下仅包含 `napi`）

```sh
# 更新依赖
bash ./build/prebuilts_config.sh

# 构建（不含测试）——hb 未装可从 //build/hb 安装
hb build napi -i

# 构建（含测试）
hb build napi -t
```

### 完整 ArkUI 源码树（`//foundation/arkui` 下包含 `napi` 及多个 ArkUI 子目录）

```sh
# 更新依赖
bash ./build/prebuilts_download.sh

# 构建单个目标
./build.sh --product-name rk3568 --target-cpu <arm|arm64> --build-target ace_napi

# 构建测试套件：将 --build-target 改为 napi_packages_test
```

### 运行测试

```sh
# 运行 UT 套件（路径替换为项目根物理路径；// 表示项目根）
env -C //test/testfwk/developer_test/src \
    PATH="//prebuilts/ohos-sdk/linux/23/toolchains:\
    /usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin" \
    //prebuilts/python/linux-x86/current/bin/python3 -m main run -t UT -p rk3568 -ss napi

# 运行 Fuzz：将 -t UT 改为 -t FUZZ
# ohos-sdk 路径不存在时需更新
# 检测到独立项目时可能需建立 //out/standard/test → //out/rk3568 的链接
```

涉及真实硬件、设备、显示或服务集成的行为，必须补充板侧证据，不能仅靠宿主侧单测声称完成。

## 3. 知识路由

改动前按场景读取 `docs/knowledge/` 下的对应文件。链接必须有触发条件——不要把领域知识复制到本文件，只走路由。

### 3.1 场景→文件（task-based）

| 场景关键词 | 先读 |
| --- | --- |
| GN 构建、`napi.gni`、`BUILD.gn`、`bundle.json`、特性开关 `declare_args`、PGO、hb、build.sh、versionscript、平台适配 `utils/platform/`、`data_protector`、`OHOS_PLATFORM` 条件编译 | `docs/knowledge/build-platform.md` |
| ArkNativeEngine、NativeEngine 基类、`napi_env`/`napi_value` 指针 reinterpret、HandleScope/CallbackScope/CriticalScope、ArkIdleMonitor、ArkNativeTimer、ArkFinalizersPack、NativeReference/Sendable/Hybrid、容器作用域、Ark 引擎实现 `impl/ark/` | `docs/knowledge/engine.md` |
| `napi_module_register`、NativeModuleManager、ModuleLoadChecker、ModuleCheckerDelegate、`nm_register_func`、模块 .so 加载、模块缓存、白名单/校验、`nm_version` | `docs/knowledge/module-manager.md` |
| Cangjie、CJ FFI、`ark_interop_*`、`cj_ffi`、`cj_envsetup`、`uv_loop_handler`、`cj_fn_invoker`、`cj_backtrace`、`cj_support`、`ARKTS_*` 调用、互操作类型映射/异常传播 | `docs/knowledge/cjffi.md` |
| 异步 NativeAsyncWork/SafeAsyncWork/TSFN、引用管理/追踪、`NAPI_VERSION`、公共 vs 内部 API、`NativeErrorExtendedInfo`、`napi_set/clear_last_error`、`NAPI_PREAMBLE`、AsyncHook 空 stub、Worker 上限、与 Node.js N-API 差异 | `docs/knowledge/napi-differences.md` |
| 权限 / 安全 / 线程安全变更（如跨 Worker 引用、env 跨线程、模块加载校验、白名单） | `docs/knowledge/engine.md`（VM 线程安全不变量）+ `docs/knowledge/module-manager.md`（ModuleLoadChecker / 白名单）+ `docs/knowledge/napi-differences.md`（错误码 / 异常传播） |
| DFX / 日志 / 错误码 / 回溯变更（HiLog、HiTrace、`NativeErrorExtendedInfo`、`napi_set/clear_last_error`、cj_backtrace） | `docs/knowledge/napi-differences.md`（错误处理差异）+ `docs/knowledge/build-platform.md`（`utils/log`）+ `docs/knowledge/cjffi.md`（ark_interop_hitrace / cj_backtrace） |

### 3.2 路径→文件（path-based）

| 修改路径 | 先读 |
| --- | --- |
| `native_engine/`（基类与抽象层） | `docs/knowledge/engine.md` + `docs/knowledge/napi-differences.md` |
| `native_engine/impl/ark/` | `docs/knowledge/engine.md` |
| `module_manager/` | `docs/knowledge/module-manager.md` |
| `interfaces/inner_api/cjffi/` | `docs/knowledge/cjffi.md` |
| `native_engine/impl/ark/cj_support.*` | `docs/knowledge/cjffi.md`（互操作两端须同步） |
| `interfaces/kits/napi/native_api.h` | `docs/knowledge/napi-differences.md` |
| `interfaces/inner_api/napi/native_node_api.h`、`native_node_hybrid_api.h` | `docs/knowledge/napi-differences.md` |
| `BUILD.gn`、`napi.gni`、`bundle.json`、`ace_napi.versionscript` | `docs/knowledge/build-platform.md` |
| `utils/`（含 `platform/`、`data_protector.*`、`log.*`） | `docs/knowledge/build-platform.md` |
| `callback_scope_manager/` | `docs/knowledge/engine.md` |
| `reference_manager/` | `docs/knowledge/engine.md` + `docs/knowledge/napi-differences.md` |
| `test/unittest/`、`test/fuzztest/` | 对应领域文件的「测试锚点」段 |

### 3.3 术语→文件（vocabulary-based）

当任务、issue、日志、API 或改动文件出现以下术语时，规划前先读对应文档。下表只放风险非平凡的术语；通用词不在内。

| 术语 | 风险提示 | 读 |
|---|---|---|
| `napi_env` / `napi_value` | 不是 C++ 类，是指针 reinterpret（env→NativeEngine*，value→JSValueRef）。代码中无 "NativeValue" 类 | `docs/knowledge/engine.md` |
| HandleScope / CallbackScope / CriticalScope | 三者不可互换：CallbackScope 含异常语义；AsyncWork complete **不在** CallbackScope 内 | `docs/knowledge/engine.md`、`docs/knowledge/napi-differences.md` |
| NativeScopeManager | 已废弃（源码标 `// To be delete`），不要使用 | `docs/knowledge/engine.md` |
| SafeAsyncWork / TSFN | env 销毁前未释放 TSFN 触发 HILOG_FATAL；TSFN 队列非 lock-free | `docs/knowledge/napi-differences.md` |
| Sendable / Hybrid 引用 | 跨 Worker 必须用 Sendable 变体；Sendable 无自动清理，Worker 退出须手动删除 | `docs/knowledge/napi-differences.md` |
| AsyncHook / AsyncId | 全部空 stub / 固定返回 0，不要依赖其行为 | `docs/knowledge/napi-differences.md` |
| Worker 上限 | 全局 80 / THREAD_WORKER 64 / LIMITED 16 / OLD 8（部分硬编码） | `docs/knowledge/napi-differences.md` |
| `napi_set/clear_last_error` | last_error 是 env-local 非 thread_local；throw 调用 `napi_clear_last_error`（清零，与直觉相反） | `docs/knowledge/napi-differences.md` |
| `NAPI_PREAMBLE` | exception > status：pending exception 优先短路，返回 `napi_pending_exception` | `docs/knowledge/napi-differences.md` |
| `NAPI_VERSION` / `nm_version` | NAPI_VERSION=8 固定；nm_version 仅透传，**无版本校验逻辑** | `docs/knowledge/napi-differences.md` |
| `NativeErrorExtendedInfo` | 用 `int errorCode` 而非 `napi_status`；napi_status 含 Ark 专有 22-24，不要假设最大 21 | `docs/knowledge/napi-differences.md` |
| `ace_napi.versionscript` | 仅 arm64 + build_ext_path 启用；改 API 后必须同步更新 | `docs/knowledge/build-platform.md`、`docs/knowledge/napi-differences.md` |
| `napi_sources` | 决定编译进 ace_napi 的源文件清单；新增 .cpp 必须加入，否则链接失败 | `docs/knowledge/build-platform.md` |
| `declare_args` / `enabled_data_protector` | 特性开关必须用 `declare_args()`；`enabled_data_protector` 由平台自动算，不要手改 | `docs/knowledge/build-platform.md` |
| data_protector | ARM64 OHOS 自动启用；非 ARM64 不要依赖其行为 | `docs/knowledge/build-platform.md` |
| hb / build.sh | 独立项目用 hb；非独立项目用 build.sh，需 `--target-cpu arm\|arm64` | `docs/knowledge/build-platform.md` |
| PGO | 需 5 个前置条件；profdata 文件名固定 `libace_napi.profdata` | `docs/knowledge/build-platform.md` |
| `napi_module_register` / `nm_register_func` | 注册回调在加载路径上，不要做阻塞或重初始化操作 | `docs/knowledge/module-manager.md` |
| ModuleLoadChecker / ModuleCheckerDelegate | 校验策略通过 delegate 注入，不要硬编码 | `docs/knowledge/module-manager.md` |
| ark_interop / cj_ffi | 前者绑定 Ark 引擎，后者是语言层通用 FFI——职责不可混淆 | `docs/knowledge/cjffi.md` |
| cj_envsetup | 处理与 Ark 的协同初始化，不要绕过直接初始化 Cangjie 运行时 | `docs/knowledge/cjffi.md` |
| `ARKTS_*` / `CJLambdaInvoker` | 调用方向：Cangjie→Ark 走 `ARKTS_Call`；Ark→Cangjie 走 `CJLambdaInvoker` | `docs/knowledge/cjffi.md` |
| uv_loop_handler | 嵌入 Ark EventHandler，非独立循环；不要写 Ark 特定调度逻辑 | `docs/knowledge/cjffi.md` |
| 容器作用域 | 编译期 + 运行时双重条件，仅改 GN 参数不生效 | `docs/knowledge/engine.md` |

### 3.4 Plan 中声明

计划阶段必须说明：

- 任务类别（构建 / 引擎 / 模块 / 互操作 / API / 异步引用）
- 已读哪些知识文件
- 发现了哪些约束
- 是否需要触发某个 skill 或多步流程

## 4. 项目约束

### 4.1 架构不变量

- 公共 API（`interfaces/kits/napi/native_api.h`）表达稳定能力契约，不暴露内部实现
- 公共 API 与内部 API（`interfaces/inner_api/napi/native_node_api.h`）分层：接口应按使用范围区分为对外接口和系统部件间接口（inner API）
- NativeEngine 基类是跨引擎契约，不要在基类写引擎特定逻辑（Ark 特定逻辑归 `impl/ark/`）
- `napi_env` / `napi_value` 是指针 reinterpret，不是 C++ 包装类
- Ark VM 非线程安全：跨线程传递 `napi_env`、跨 Worker 用普通引用、在 execute 回调中调用 napi 函数均禁止
- `ark_interop` 与 `cj_support` 是互操作两端，改动须同步
- 校验策略通过 `ModuleCheckerDelegate` 注入，不在 Manager 内硬编码

### 4.2 不要（Do not）

- 不要修改公共 API 签名、错误码、生命周期语义，除非任务明确要求
- 不要在 execute 回调中调用 napi 函数（子线程，VM 非线程安全）
- 不要跨线程传递 `napi_env`（`CROSS_THREAD_CHECK` 会检查）
- 不要脱离作用域持有 `napi_value`（可能被 GC 回收）
- 不要用普通 Scope 替代 CallbackScope（CallbackScope 含异常处理语义）
- 不要跨 Worker 用普通引用（需 Sendable 变体）
- 不要继续使用 NativeScopeManager（已废弃）
- 不要假设 AsyncHook / AsyncId 有真实行为（空 stub / 返回 0）
- 不要在 `nm_register_func` 中执行重初始化或阻塞操作
- 不要手动加载 .so 绕过 `napi_module_register`
- 不要修改已注册模块的 `nm_modname`（缓存 key）
- 不要在 ModuleManager 中持有模块业务对象
- 不要混淆 `ark_interop`（Ark 特定）与 `cj_ffi`（通用 FFI）职责
- 不要绕过 `cj_envsetup` 直接初始化 Cangjie 运行时
- 不要在 `uv_loop_handler` 中写 Ark 特定调度逻辑
- 不要在 `napi.gni` 中写构建 target（target 归 `BUILD.gn`）
- 不要手动修改 `enabled_data_protector`（由平台条件自动计算）
- 不要假设 `napi_status` 最大 21（Ark 专有 22-24）
- 不要假设 `nm_version` 有版本校验逻辑
- 不要为通过测试删除日志、事件、错误码或 DFX 信息
- 不要在公共头文件中包含平台特定头文件
- 不要手改由工具生成的产物（如未来引入 codegen），要改 source of truth 后重新生成

### 4.3 必须（Must）

- 新增 .cpp 源文件必须加入 `napi.gni` 的 `napi_sources` 列表
- 新增平台特定代码必须用条件编译（#ifdef）或 `utils/platform/` 分层
- 新增特性开关必须用 `declare_args()`
- 异步任务完成后必须 `delete_async_work` 或关闭 TSFN
- context env 销毁前必须释放所有 TSFN（否则 HILOG_FATAL）
- Worker 退出前必须手动删除 Sendable 引用（无自动清理）
- 公共 API 改动后必须更新 `ace_napi.versionscript`
- 线程安全 API 返回错误码时必须调用 `napi_set_last_error` / `napi_clear_last_error`
- 修改 `ark_interop` 时必须同步检查 `cj_support`
- 互操作调用中必须确保线程约束（Ark VM 线程）

### 4.4 改前问人（Ask before）

- 新增第三方依赖
- 修改公共 API 语义、签名、错误码
- 修改权限模型或信任边界
- 修改协议兼容性或持久化数据格式
- 删除兼容性 shim 或迁移逻辑
- 执行可能影响连接设备的命令

## 5. 验证闭环

### 5.1 最小验证

- 构建当前模块：见 §2 构建命令
- 运行测试：见 §2 测试命令
- 静态检查：依赖宿主工具链的 lint / 编译警告
- 符号版本检查：改 API 后检查 `ace_napi.versionscript`

### 5.2 任务级验证

| 任务类型 | 验证路径 |
|---|---|
| 公共 API 变更 | 运行 `test/unittest/test_napi*`、`test_ark_api_allowlist`；更新 `ace_napi.versionscript`；arm64 构建确认符号可见性 |
| 引擎 / Ark 实现 | 构建 ace_napi；跑 `test/unittest/test_ark*`、`test_napi*` |
| 异步 / 引用 / 作用域 | 跑 `test_napi_threadsafe`、`test_worker_manager`、`test_napi_global_ref_track`、`test_sendable_napi`、`test_napi_pendingexception` |
| DFX / 日志 / 错误码 / 回溯 | 跑 `test/unittest/test_napi_errorcode`、`test_napi_pendingexception`、`test_napi_critical`；CJ 侧跑 `test/unittest/cj_native/test_cj_backtrace` |
| 模块管理 | 跑 `module_manager/test/unittest/module_manager_test/` + Fuzz `loadarkmodule_fuzzer/` |
| Cangjie FFI 互操作 | 跑 `test/unittest/cj_native/test_ark_interop*`、`test_cj_backtrace`、`test_ffi_data` |
| 构建配置 / 平台 | 跑 `ace_napi` 构建；按改动跑 PGO/versionscript 验证 |
| 纯测试变更 | 跑改动测试 + 至少一个相邻相关测试 |

### 5.3 Done 定义

任务完成必须满足：

- 请求的行为已实现
- 相关构建 / 测试 / lint / 兼容性检查已运行，或说明无法运行的原因
- 最终回复包含：变更摘要、改动文件、验证命令与结果、兼容性/权限/DFX/跨设备影响（如适用）、剩余风险或后续项
- 没有无关的格式化、重构或顺手改动
