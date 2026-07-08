# INSTALL — LinkerHand C++ SDK 安装指南

本文档面向"只想集成 SDK 到自有工程"的下游用户。若你在参与 SDK 本身的开发或需要从源码构建，请回到 [`README.md`](README.md)。

---

## 目录

- [一条命令安装](#一条命令安装)
- [版本锁定](#版本锁定)
- [离线 / 手动安装](#离线--手动安装)
- [下游 CMake 集成](#下游-cmake-集成)
- [卸载](#卸载)
- [常见问题](#常见问题)

---

## 一条命令安装

### Linux（x86_64 / aarch64）

```bash
# 默认：装最新 Release 到 /usr/local，自动使用 sudo
curl -fsSL https://raw.githubusercontent.com/linker-bot/linkerhand-cpp-sdk/main/scripts/install.sh | bash
```

支持的参数（通过 `-s --` 传给脚本）：

| 参数 | 说明 |
|------|------|
| `--version <TAG>` | 指定版本（`v2.1.8` 或 `2.1.8` 均可），默认 latest |
| `--prefix <PATH>` | 安装前缀，默认 `/usr/local` |
| `--no-sudo` | 不使用 sudo（在 root 容器或已具备写权限时） |
| `--keep-tmp` | 保留临时目录，排查用 |
| `-h`, `--help` | 帮助 |

```bash
# 举例：装指定版本到用户目录，无需 sudo
curl -fsSL https://raw.githubusercontent.com/linker-bot/linkerhand-cpp-sdk/main/scripts/install.sh \
  | bash -s -- --version v2.1.8 --prefix $HOME/.local --no-sudo
```

### Windows x64

以 **管理员** 打开 PowerShell：

```powershell
iwr https://raw.githubusercontent.com/linker-bot/linkerhand-cpp-sdk/main/scripts/install.ps1 -UseBasicParsing | iex
```

带参调用需要显式实例化脚本块：

```powershell
& ([scriptblock]::Create(
    (iwr https://raw.githubusercontent.com/linker-bot/linkerhand-cpp-sdk/main/scripts/install.ps1 -UseBasicParsing).Content
  )) -Version v2.1.8 -Prefix "C:\SDKs\LinkerHand" -AddToPath
```

参数：`-Version`、`-Prefix`、`-AddToPath`（把 `<Prefix>\bin` 加入用户 PATH）。

安装脚本会将 `<Prefix>` 追加到用户环境变量 `CMAKE_PREFIX_PATH`，新终端生效。

---

## 版本锁定

生产环境建议**锁定版本**，避免上游发新版后无感知升级：

```bash
# CI / Dockerfile 中固化版本
curl -fsSL https://raw.githubusercontent.com/linker-bot/linkerhand-cpp-sdk/main/scripts/install.sh \
  | bash -s -- --version v2.1.8
```

各 Release 的 SHA256 汇总由 `SHA256SUMS` 附件承载，安装脚本会**自动校验**，任何篡改或下载损坏都会终止安装。

---

## 离线 / 手动安装

无法访问 GitHub 的环境（内网、离线机、气隙网络）：

1. 到 [Releases 页面](https://github.com/linker-bot/linkerhand-cpp-sdk/releases) 手动下载对应平台的产物：
   - `linkerhand-cpp-sdk-<VER>-linux-x86_64.tar.gz`
   - `linkerhand-cpp-sdk-<VER>-linux-aarch64.tar.gz`
   - `linkerhand-cpp-sdk-<VER>-windows-x64.zip`
   - `SHA256SUMS`（校验用）
2. 校验：`sha256sum -c SHA256SUMS`（Linux）或 `Get-FileHash`（PowerShell）
3. Linux：`tar -xzf ...tar.gz && sudo rsync -a linkerhand-cpp-sdk-*/ /usr/local/ && sudo ldconfig`
4. Windows：解压 zip 后整个目录移动到目标位置，将其加入 `CMAKE_PREFIX_PATH`

---

## 下游 CMake 集成

推荐使用 CMake config 包：

```cmake
cmake_minimum_required(VERSION 3.15)
project(my_app LANGUAGES CXX)

find_package(linkerhand-cpp-sdk CONFIG REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE LinkerHand::linkerhand_cpp_sdk)
```

- 装到 `/usr/local` 或 `C:\Program Files\LinkerHand\cpp-sdk`：`find_package` 自动发现
- 装到自定义前缀：`cmake -DCMAKE_PREFIX_PATH=<前缀> ...`
- Windows 上，安装脚本已把前缀写入用户 `CMAKE_PREFIX_PATH`

---

## 卸载

### Linux

安装到 `/usr/local`（或指定前缀）后，可用以下命令清理：

```bash
sudo rm -rf /usr/local/include/linkerhand-cpp-sdk \
            /usr/local/lib/linkerhand-cpp-sdk \
            /usr/local/lib/cmake/linkerhand-cpp-sdk \
            /usr/local/lib/liblinkerhand_cpp_sdk.*
sudo ldconfig
```

如仍保留了 SDK 源码目录，也可直接用 `sudo ./build.sh -u --prefix /usr/local`。

### Windows

删除 `%ProgramFiles%\LinkerHand\cpp-sdk`（或自定义前缀）目录，并从用户 `CMAKE_PREFIX_PATH` / `Path` 中移除对应条目：

```powershell
Remove-Item -Recurse -Force "$env:ProgramFiles\LinkerHand\cpp-sdk"
```

---

## 常见问题

**Q: 一键脚本安全吗？我把陌生脚本 pipe 到 bash 心里发怵。**
A: 脚本源码位于本仓库 [`scripts/install.sh`](scripts/install.sh) 与 [`scripts/install.ps1`](scripts/install.ps1)，可在执行前先看一遍：
```bash
curl -fsSL https://raw.githubusercontent.com/linker-bot/linkerhand-cpp-sdk/main/scripts/install.sh | less
```
脚本只做：查询 Release → 下载 tarball 与 `SHA256SUMS` → 校验 → 解压到指定前缀。不下载任何仓库以外的资源。

**Q: 我的架构不在支持列表怎么办？**
A: 当前仅提供 Linux x86_64/aarch64 与 Windows x64 预编译产物。其他架构（如 ARMv7、RISC-V）请提 issue，或克隆仓库自行构建。

**Q: 安装后 `find_package` 找不到？**
A: 若装到非标准前缀，配置下游时加 `-DCMAKE_PREFIX_PATH=<前缀>`；Windows 需重开终端让环境变量生效。

**Q: 想用 apt / conan / vcpkg 装？**
A: 暂未支持。如有需求请提 issue，会评估纳入后续发布通道。
