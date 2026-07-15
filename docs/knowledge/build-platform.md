# 构建与平台知识

本文只记录 GN 构建配置、平台适配和工具层。API 分层见 `docs/knowledge/napi-differences.md`，模块注册见 `docs/knowledge/module-manager.md`。

## 核心模型

NAPI 使用 GN（Generate Ninja）构建系统。核心配置在 `BUILD.gn`（根构建文件）和 `napi.gni`（源文件列表与特性开关）。平台适配通过条件编译和 `utils/platform/` 分层实现。

```
构建配置层
  ├─ BUILD.gn                    ← 根构建目标定义
  ├─ napi.gni                    ← 源文件列表 + declare_args 特性开关
  ├─ bundle.json                 ← 部件元数据
  └─ ace_napi.versionscript      ← 符号版本管控

平台适配层
  ├─ utils/
  │   ├─ log.{h,cpp}             ← 日志（HiLog 封装）
  │   ├─ assert.h                ← 断言宏
  │   ├─ macros.h                ← 通用宏
  │   ├─ file.h                  ← 文件操作抽象
  │   ├─ data_protector.{h,cpp}  ← 数据保护（ARM64 OHOS 自动启用）
  │   └─ platform/
  │       ├─ unix_like/file.cpp  ← Unix 系平台实现
  │       └─ windows/file.cpp    ← Windows 实现
  │
  └─ 条件编译                    ← #ifdef OHOS_PLATFORM 等
```

### 特性开关（napi.gni declare_args）

| 开关 | 默认值 | 作用 |
|---|---|---|
| `napi_enable_container_scope` | false | 容器作用域特性 |
| `napi_enable_memleak_debug` | true | 内存泄漏调试 |
| `napi_feature_enable_pgo` | false | PGO 构建 |
| `napi_feature_pgo_path` | "" | PGO profdata 路径 |
| `enabled_data_protector` | false（ARM64 OHOS 自动 true） | 数据保护 |

### data_protector 自动启用条件

```
target_cpu == "arm64" && !is_arkui_x && is_ohos && !is_emulator
  → enabled_data_protector = true
```

BUILD.gn 含 17 个 target：`ace_napi_config`/`data_protector_config`/`module_manager_config`(config) → `pac_data_protector_feature`/`ace_napi_static`(source_set) → `ace_napi`(is_arkui_x 时 static_library，否则 shared_library) → `ace_napi_test` → CJ FFI targets(`cj_bind_native`/`cj_bind_ffi`/`cj_ffi_libraries`) → `napi_packages`/`napi_packages_test`(group)。依赖：ace_napi 依赖 ets_runtime、libuv、icu、c_utils、hilog、hitrace、hiview、eventhandler、ffrt、bounds_checking_function 等 17 个部件；对外暴露 6 组 inner_kits（ace_napi/cj_bind_ffi/cj_bind_native/ark_interop/napi_packages）。hb 构建流程：独立项目（`//foundation/arkui` 下仅 napi）用 `hb build napi -i`(不含测试)/`-t`(含测试)，依赖更新 `bash ./build/prebuilts_config.sh`。PGO 构建路径：`napi_feature_enable_pgo=true` + `napi_feature_pgo_path` 传入 profdata 目录，编译时 `-fprofile-use=${path}/libace_napi.profdata`，链接时 `-Wl,-mllvm,-align-all-functions=4`（BUILD.gn:220-231,273-281）。

## 边界与身份

| 概念 | 是什么 | 不是什么 | 常见误用 |
|---|---|---|---|
| napi.gni | 源文件列表和特性开关 | 构建目标定义 | 在 .gni 写 target |
| BUILD.gn | 构建目标定义 | 源文件列表 | 在 BUILD.gn 维护源文件列表 |
| napi_sources | 编译进 ace_napi 的源文件清单 | 全部源文件 | 新建 .cpp 不加入 napi_sources |
| data_protector | ARM64 OHOS 内存保护 | 通用安全层 | 在非 ARM64 依赖其行为 |
| declare_args | 可外部覆盖的特性开关 | 硬编码常量 | 当作不可变配置 |
| bundle.json | 部件元数据 | 构建脚本 | 在 bundle.json 写构建逻辑 |

## 约束规则

- 必须 在新增 .cpp 源文件时加入 `napi.gni` 的 `napi_sources` 列表。原因：napi_sources 决定编译进 ace_napi 的源文件，遗漏导致链接失败。
- 必须 在新增平台特定代码时用条件编译（#ifdef）或 `utils/platform/` 分层。原因：多平台支持，硬编码平台导致移植失败。
- 不要 在 napi.gni 中写构建 target。原因：target 定义归 BUILD.gn，.gni 只管配置。
- 不要 手动修改 `enabled_data_protector`。原因：该值由平台条件自动计算，手动覆盖破坏 ARM64 保护。
- 必须 在新增特性开关时用 `declare_args()`。原因：declare_args 允许外部构建配置覆盖，硬编码无法覆盖。
- 不要 引入未经评审的第三方依赖。原因：依赖引入影响安全和兼容性，需评审。
- 避免 在公共头文件中包含平台特定头文件。原因：破坏 API 跨平台兼容性。

- PGO 约束：需 5 个前置条件——`is_ohos` + LLVM 编译器 + `defined(enhanced_opt) && enhanced_opt` + `napi_feature_enable_pgo` + `napi_feature_pgo_path`。profdata 文件名固定 `libace_napi.profdata`。编译时加 `-Wno-error=backend-plugin`/`-Wno-profile-instr-out-of-date`/`-Wno-profile-instr-unprofiled` 容忍 profdata 不完全匹配。
- hb 与 build.sh 选择：`//foundation/arkui` 下仅有 napi→用 hb（轻量，无需 product-name/target-cpu）；有额外子目录→可用 build.sh，执行前确认 `target-cpu` 为 `arm` 或 `arm64`（示例：`--product-name rk3568 --target-cpu <target-cpu> --build-target ace_napi`）。
- versionscript 维护：仅 `is_ohos` + LLVM 编译器 + `target_cpu=="arm64"` + `defined(build_ext_path)` 时启用，global:* 导出全部 + local: 隐藏 C++ 标准库符号，伴随 `--emit-relocs`+`--no-relax`+`-mno-fix-cortex-a53-843419`。

## 状态与生命周期

| 配置项 | 作用域 | 修改频率 | 注意事项 |
|---|---|---|---|
| napi_sources | 编译 | 新增源文件时 | 必须同步更新 |
| declare_args | 构建 | 特性调整时 | 外部可覆盖 |
| enabled_data_protector | 平台自动 | 不手动改 | ARM64 OHOS 自动 true |
| 平台宏 | 编译时 | 不可变 | #ifdef OHOS_PLATFORM 等 |

## 修改前检查

- [ ] 新增 .cpp 是否加入 `napi_sources`？
- [ ] 平台特定代码是否用条件编译或 platform 分层？
- [ ] 特性开关是否用 `declare_args()`？
- [ ] 是否避免手动改 `enabled_data_protector`？
- [ ] 新增依赖是否经过评审？
- [ ] 公共头是否避免平台特定 include？
- [ ] `ace_napi.versionscript` 是否与 API 变更同步？
- [ ] `bundle.json` 部件元数据是否更新？

## 代码和测试

### 代码锚点

| 关键流程 | 入口路径 |
|---|---|
| 根构建文件 | `BUILD.gn` |
| 源文件列表与开关 | `napi.gni` |
| 部件元数据 | `bundle.json` |
| 符号版本管控 | `ace_napi.versionscript` |
| 日志 | `utils/log.{h,cpp}` |
| 断言 | `utils/assert.h` |
| 通用宏 | `utils/macros.h` |
| 文件抽象 | `utils/file.h` |
| 数据保护 | `utils/data_protector.{h,cpp}` |
| Unix 平台文件 | `utils/platform/unix_like/file.cpp` |
| Windows 平台文件 | `utils/platform/windows/file.cpp` |

### 测试锚点

| 测试类型 | 路径/类名 |
|---|---|
| 通用测试辅助 | `test/unittest/common/test_common.h` |
| 测试入口 | `test/unittest/test.h`（engine 层） |
| 构建目标 | `//foundation/arkui/napi:napi_packages`（含测试） |
| 构建目标（仅库） | `//foundation/arkui/napi:ace_napi` |
