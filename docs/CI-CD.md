# CI/CD 文档

本文档介绍项目的持续集成和持续部署（CI/CD）配置。

## 📋 目录

- [概述](#概述)
- [工作流配置](#工作流配置)
- [触发条件](#触发条件)
- [构建矩阵](#构建矩阵)
- [作业说明](#作业说明)
- [状态徽章](#状态徽章)
- [故障排查](#故障排查)
- [本地测试](#本地测试)

## 概述

项目使用 GitHub Actions 进行持续集成，自动执行以下任务：

- ✅ 多平台构建（Linux x86_64）
- ✅ 自动运行单元测试
- ✅ 构建示例程序
- ✅ 代码质量检查
- ✅ 构建状态报告

## 工作流配置

CI/CD 工作流配置文件位于：`.github/workflows/ci.yml`

### 主要特性

- **多构建配置**: 支持 Release 和 Debug 构建
- **并行测试**: 使用多核并行运行测试以加快速度
- **缓存优化**: 缓存 CMake 构建文件以提高效率
- **详细日志**: 提供详细的构建和测试输出

## 触发条件

工作流在以下情况下自动触发：

1. **推送代码** - 推送到 `main`、`master` 或 `develop` 分支
2. **Pull Request** - 创建或更新 PR 到 `main`、`master` 或 `develop` 分支
3. **手动触发** - 通过 GitHub Actions 界面手动运行（`workflow_dispatch`）

## 构建矩阵

当前支持的构建配置：

| 操作系统 | 架构 | 构建类型 | 说明 |
|---------|------|---------|------|
| Ubuntu Latest | x86_64 | Release | 主要构建配置 |
| Ubuntu Latest | x86_64 | Debug | 调试构建配置 |
| Ubuntu Latest | aarch64 | Release | ARM64 架构构建（使用 QEMU 模拟） |

### 架构支持说明

- **x86_64**: 原生支持，使用系统默认工具链
- **aarch64**: 使用交叉编译工具链（`aarch64-linux-gnu-gcc`）和 QEMU 用户模式模拟运行测试
  - 构建速度较慢（由于 QEMU 模拟）
  - 测试并行度降低（使用 2 个并行任务）
  - 确保库文件在 `lib/aarch64/` 目录下可用


## 作业说明

### 1. build-and-test

**目的**: 构建项目并运行所有单元测试

**步骤**:
1. 检出代码
2. 设置 QEMU（仅 aarch64 架构）
3. 缓存 CMake 构建文件
4. 安装构建依赖（CMake、GCC、Git 等，aarch64 需要交叉编译工具链）
5. 验证工具链版本
6. 配置 CMake（启用测试，aarch64 使用交叉编译配置）
7. 构建项目
8. 运行测试（使用 CTest，aarch64 通过 QEMU 运行）
9. 显示测试结果和构建产物

**架构特定说明**:
- **x86_64**: 使用系统原生工具链，并行运行所有测试
- **aarch64**: 使用 `aarch64-linux-gnu-gcc` 交叉编译，通过 QEMU 用户模式运行测试（并行度降低为 2）

**输出**: 测试结果、构建的可执行文件信息

### 2. build-examples

**目的**: 验证示例程序可以成功构建

**步骤**:
1. 检出代码
2. 安装构建依赖
3. 配置 CMake（禁用测试）
4. 构建示例程序（`toolset_example`、`action_group_show_l10`）
5. 验证构建产物

**依赖**: 等待 `build-and-test` 作业完成

**输出**: 示例程序构建状态

### 3. code-quality

**目的**: 执行代码质量检查

**检查项**:
- 文件编码和换行符（确保使用 LF）
- CMake 语法验证
- 头文件包含检查

**输出**: 代码质量报告

### 4. summary

**目的**: 生成 CI/CD 摘要报告

**功能**: 汇总所有作业的状态，生成易于阅读的摘要

## 状态徽章

将以下 Markdown 代码添加到 README.md 以显示构建状态：

```markdown
![CI/CD](https://github.com/你的用户名/linkerhand-cpp-sdk/workflows/CI/CD/badge.svg)
```

或者使用 shields.io 样式：

```markdown
![CI/CD Status](https://img.shields.io/github/workflow/status/你的用户名/linkerhand-cpp-sdk/CI/CD?label=CI%2FCD)
```

## 故障排查

### 常见问题

#### 1. 测试失败

**症状**: `build-and-test` 作业失败

**可能原因**:
- 代码更改导致测试失败
- 测试超时
- 依赖库未找到

**解决方法**:
1. 查看作业日志中的详细错误信息
2. 在本地运行测试：`cd build && ctest --output-on-failure`
3. 检查 CMake 配置是否正确

#### 2. 构建失败

**症状**: CMake 配置或构建步骤失败

**可能原因**:
- CMakeLists.txt 语法错误
- 缺少依赖库
- 编译器版本不兼容

**解决方法**:
1. 检查 CMake 输出日志
2. 验证本地构建是否成功
3. 检查依赖库路径配置

#### 3. 缓存问题

**症状**: 构建使用过期的缓存

**解决方法**:
1. 在 GitHub Actions 界面清除缓存
2. 修改工作流中的缓存 key
3. 手动触发工作流

#### 4. aarch64 构建问题

**症状**: aarch64 架构构建或测试失败

**可能原因**:
- QEMU 未正确设置
- 交叉编译工具链未安装
- 库文件路径不正确
- QEMU 模拟运行失败

**解决方法**:
1. 检查 QEMU 设置步骤是否成功
2. 验证交叉编译工具链是否安装：`aarch64-linux-gnu-gcc --version`
3. 确认 `lib/aarch64/` 目录下存在库文件
4. 检查测试可执行文件架构：`file build/test_*`
5. 查看 QEMU 相关日志和错误信息
6. 如果 QEMU 模拟失败，可以尝试减少并行任务数量

### 查看日志

1. 进入 GitHub 仓库
2. 点击 "Actions" 标签
3. 选择失败的工作流运行
4. 查看具体作业的日志

## 本地测试

在提交代码前，建议在本地运行 CI/CD 流程：

### 1. 安装依赖

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  cmake \
  git \
  libpthread-stubs0-dev
```

### 2. 运行完整构建和测试

#### x86_64 架构

```bash
# 创建构建目录
mkdir -p build
cd build

# 配置 CMake（启用测试）
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON

# 构建项目
cmake --build . --config Release --parallel $(nproc)

# 运行测试
ctest --output-on-failure --parallel $(nproc)
```

#### aarch64 架构（交叉编译）

```bash
# 安装交叉编译工具链和 QEMU
sudo apt-get update
sudo apt-get install -y \
  gcc-aarch64-linux-gnu \
  g++-aarch64-linux-gnu \
  qemu-user-static \
  binfmt-support

# 创建构建目录
mkdir -p build
cd build

# 配置 CMake（交叉编译）
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=ON \
  -DCMAKE_SYSTEM_NAME=Linux \
  -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
  -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
  -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ \
  -DCMAKE_FIND_ROOT_PATH=/usr/aarch64-linux-gnu

# 构建项目
cmake --build . --config Release --parallel $(nproc)

# 运行测试（通过 QEMU）
export QEMU_LD_PREFIX=/usr/aarch64-linux-gnu
ctest --output-on-failure --parallel 2
```

### 3. 构建示例程序

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build . --target toolset_example action_group_show_l10
```

### 4. 代码质量检查

```bash
# 检查文件编码
find . -type f \( -name "*.cpp" -o -name "*.h" \) \
  -exec file {} \; | grep -q "CRLF" && echo "发现 CRLF" || echo "✓ 使用 LF"

# 验证 CMake 语法
cmake --version
mkdir -p /tmp/cmake-check
cd /tmp/cmake-check
cmake /path/to/project -Wno-dev
```

## 最佳实践

1. **提交前测试**: 在本地运行测试确保通过
2. **小步提交**: 频繁提交小改动，便于定位问题
3. **查看日志**: 失败时仔细查看详细日志
4. **更新文档**: 修改 CI/CD 配置时更新本文档
5. **监控状态**: 定期检查构建状态，及时修复问题

## 相关文档

- [测试文档](tests/README.md) - 测试框架和运行指南
- [构建文档](../README.md#构建项目) - 项目构建说明
- [故障排查](TROUBLESHOOTING.md) - 常见问题解决方案

## 贡献

如果您想改进 CI/CD 配置：

1. Fork 仓库
2. 创建特性分支
3. 修改 `.github/workflows/ci.yml`
4. 测试您的更改
5. 提交 Pull Request

---

**最后更新**: 2026-01-XX
