# AGENTS.md

This file provides guidance to agents when working with code in repository.

## 构建系统

- 使用 GN (Generate Ninja) 构建系统，不是传统的 Make 或 CMake
- 主要构建文件: BUILD.gn (根目录) 和 napi.gni (配置文件)
- 构建目标: `//foundation/arkui/napi:napi_packages`
- 测试目标: `//foundation/arkui/napi:napi_packages_test`
- 编译命令：
   编译命令需要严格按照目录结构进行选择

   > 注意事项
   >
   > 若无特殊说明，如下命令工作目录均为gn根目录
   >
   > 注: target-cpu 需要询问用户，可选值为 arm 和 arm64

   - 若 `//foundation/arkui` 目录下仅存在 `//foundation/arkui/napi` 文件夹
      ```bash
      # 更新依赖
      bash ./build/prebuilts_config.sh

      # 运行编译命令
      # 注: 若未 hb 工具安装，可以路径 //build/hb 进行安装
      # 不编译测试用例
      hb build napi -i --target-cpu arm64 # 编译除测试用例外所有构建目标
      # 编译所有目标
      hb build napi -t --target-cpu arm64 # 编译所有构建目标, 含测试用例
      ```
   - 其余情况除可使用上述命令进行编译时，还可以使用下列命令进行编译
      ```bash
      # 更新依赖
      bash ./build/prebuilts_download.sh
      # 编译命令
      ./build.sh --product-name rk3568 --target-cpu arm64 --build-target ace_napi # 若需编译测试套，则将编译目标修改为 napi_packages_test
      ```

## 测试命令

- 单元测试: `//foundation/arkui/napi/test/unittest:unittest`
- 模糊测试: `//foundation/arkui/napi/test/fuzztest:fuzztest`


## 关键实现细节

- Ark JS 引擎实现位于 `native_engine/impl/ark/` 目录
- 支持多平台: OHOS、Linux、Windows、macOS、iOS、Android
- 支持多架构: ARM64、ARM32、x86_64、x86
- 数据保护功能在 ARM64 OHOS 平台上自动启用

## 开发注意事项

- 新模块需要注册到 napi.gni 的 napi_sources 列表中
- 平台特定代码使用条件编译指令 (如 #ifdef OHOS_PLATFORM)
- 错误处理使用 NativeErrorExtendedInfo 结构体
- 异步操作使用 NativeAsyncWork 和 NativeSafeAsyncWork
- 线程安全操作使用适当的同步机制
- 需要与 JSValueRef 等虚拟机对象交互的接口, 需要调用方确保线程安全
- 对于线程安全的 api 接口, 需要在返回错误码时, 调用 `napi_set_last_error` 或 `napi_clear_last_error` 更新异常信息并返回
- 公开 api 接口应该声明到 `interfaces/kits/napi/native_api.h`, 内部接口 (供系统其余模块使用) 应当声明到 `interfaces/inner_api/napi/native_node_api.h`
