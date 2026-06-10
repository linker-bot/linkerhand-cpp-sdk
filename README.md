# LinkerHand-CPP-SDK

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows-blue.svg)]()
[![Architecture](https://img.shields.io/badge/Arch-x86__64%20%7C%20aarch64-lightgrey.svg)]()
[![CI/CD](https://github.com/linker-bot/linkerhand-cpp-sdk/workflows/CI/CD/badge.svg)](https://github.com/linker-bot/linkerhand-cpp-sdk/actions)

> 灵心巧手系列灵巧手的官方 C++ 软件开发工具包

LinkerHand-CPP-SDK 由灵心巧手（北京）科技有限公司开发，提供完整的 C++ API，用于控制 L6、L7、L10、L20、L21、L25、O6、O20、G20等型号的灵巧手设备，支持 CAN / CAN-FD / Modbus 多种总线。

---

## 📋 目录

- [特性](#-特性)
- [系统要求](#-系统要求)
- [快速开始](#-快速开始)
- [API 文档](#-api-文档)
- [通信协议](#-通信协议)
- [支持的型号](#-支持的型号)
- [项目结构](#-项目结构)
- [示例程序列表](#-示例程序列表)
- [关节映射表](#-关节映射表)
- [故障排查](#-故障排查)
- [常见问题](#-常见问题)
- [贡献](#-贡献)
- [更新日志](#-更新日志)
- [许可证](#-许可证)
- [联系我们](#-联系我们)

---

## ✨ 特性

- 🎯 **多型号支持** — L6 / L7 / L10 / L20 / L21 / L25 / O6 / O20 / G20
- 🔌 **多总线** — CAN、CAN-FD、Modbus
- 🚀 **易用** — 简洁的 C++ API，回调注入式通信层，便于嵌入到自有应用
- 📊 **传感数据** — 实时获取压力、温度、电流等传感器读数
- 🎮 **精确控制** — 支持关节位置（原始值 / 弧度）、速度、扭矩控制
- 🔄 **实时反馈** — 关节状态、电机故障码等实时上报
- 🛠️ **跨平台** — Linux (x86_64 / aarch64)、Windows (MinGW)
- 📚 **完整文档** — 附 API 参考、FAQ 与排障文档

## 💻 系统要求

**Linux**
- 操作系统：Ubuntu 18.04 及以上
- 架构：x86_64 或 aarch64
- 编译器：GCC 7.0+ 或 Clang 6.0+（C++17）
- CMake：3.15+
- 依赖：pthread

**Windows**
- 操作系统：Windows 10 / 11 (x64)
- 编译器：MinGW-w64 GCC 13+，线程模型需为 `win32`（`g++ -v` 查看 `Thread model`），或 MSVC（VS2017 / VS2019 / VS2022 任一，已实测 VS2017 消费 CI artifact 通过）
- CMake：建议 4.0+（已实测 4.0.3）
- 依赖：`PCAN-Basic` 与 mingw 运行时 DLL 已随发布包附带

> CAN-FD 在 ARM + Ubuntu ≤ 18 上自动禁用；O20（CAN-FD）目前仅在 Linux x86_64 上支持。

## 🚀 快速开始

### 1. 克隆仓库

```bash
git clone https://github.com/linker-bot/linkerhand-cpp-sdk.git
cd linkerhand-cpp-sdk
```

### 2. 使用脚本构建

发布包内附 `build.sh`（Linux）与 `build.bat`（Windows）。

| 选项 | 说明 |
|------|------|
| `-h`, `--help` | 显示帮助信息 |
| `-b`, `--build` | 构建示例程序 |
| `-i`, `--install` | 安装 SDK 到系统（需 sudo） |
| `-u`, `--uninstall` | 从系统卸载 SDK（需 sudo） |
| `-c`, `--clean` | 清理 `build/` 目录 |
| `--prefix PATH` | 指定安装前缀（默认 `/usr/local`） |

```bash
# Linux
./build.sh -b                       # 编译
sudo ./build.sh -i                  # 编译并安装到 /usr/local
sudo ./build.sh -u                  # 卸载

# Windows（cmd / PowerShell）
build.bat                           # 编译
```

> 发布包是预编译产物，`build.sh` 仅负责装配示例与触发安装；SDK 共享库取自 `lib/` 下对应平台目录，无须从源码编译。

### 3. 启动CAN设备

打开终端执行
```bash
请手动启动CAN接口(需要sudo):
  sudo ip link set canX up type can bitrate 1000000

或使用批量启动脚本:
  for i in 0 1 2 3 4; do sudo ip link set can$i up type can bitrate 1000000; sudo ip link set can$i txqueuelen 1024; done
```

### 4. 运行示例程序

构建完成后，可执行文件位于 `build/bin/`：

```bash
# Linux
cd build/bin
./linker_hand_example          # CLI 工具
./test_o6_can_0
./test_l10_can_0
./test_o20_canfd_0             # 仅 Linux x86_64

# Windows
build\bin\linker_hand_example.exe
build\bin\test_l10_can_0.exe
```

更多示例见 [`examples/`](examples/) 目录。

### 5. 快速集成

新建工程目录：

```bash
mkdir demo && cd demo
touch main.cpp CMakeLists.txt
```

两种集成方式的目录布局：

```
# 解压即用（未安装 SDK，把发布包整个拷到工程内）
demo/
├── CMakeLists.txt
├── linkerhand-cpp-sdk/      # 即本发布包
└── main.cpp

# 已安装 SDK（系统级，find_package 即可找到）
demo/
├── CMakeLists.txt
└── main.cpp
```

`demo/main.cpp`：

```cpp
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

#include "LinkerHandApi.h"
#include "CommFactory.h"

int main() {
    constexpr LINKER_HAND model = LINKER_HAND::O6;
    constexpr HAND_TYPE  side   = HAND_TYPE::LEFT;

    auto hand = std::make_shared<LinkerHandApi>(model, side, COMM_TYPE::CAN);
    auto bus  = std::shared_ptr<Communication::ICanBus>(Communication::CommFactory::createCanBus(side));

    hand->setCanTxCallback([bus](uint32_t can_id, const uint8_t *data, uintptr_t len) -> int32_t {
        std::vector<uint8_t> buf(data, data + len);
        bus->send(buf, can_id);
        return 0;
    });

    hand->setCanRxCallback([bus](uint32_t *id_out, uint8_t *data_out, uint8_t *len_out) -> int32_t {
        auto frame = bus->recv();
        if (frame.can_id == 0 && frame.can_dlc == 0) return -1;
        *id_out  = frame.can_id;
        *len_out = frame.can_dlc;
        std::memcpy(data_out, frame.data, frame.can_dlc);
        return 0;
    });

    std::cout << "-----------------------------------" << std::endl;
    std::cout << "version: " << hand->getVersion() << '\n';
    std::cout << "-----------------------------------" << std::endl;
    return 0;
}
```

`demo/CMakeLists.txt`：

```cmake
cmake_minimum_required(VERSION 3.10)
project(my_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 解压即用：把发布包目录加进 CMake 搜索路径，无需安装。
# 等价命令行：cmake -S . -B build -DCMAKE_PREFIX_PATH=${CMAKE_SOURCE_DIR}/linkerhand-cpp-sdk
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/linkerhand-cpp-sdk")

find_package(linkerhand-cpp-sdk 2.0 CONFIG REQUIRED)

add_executable(demo main.cpp)
target_link_libraries(demo PRIVATE LinkerHand::linkerhand_cpp_sdk)

# 运行时定位 .so：把 SDK lib 目录写进 RPATH，免去 LD_LIBRARY_PATH。
# linkerhand-cpp-sdk_LIBRARY_DIR 由 cmake/linkerhand-cpp-sdk-config.cmake 设置。
set_target_properties(demo PROPERTIES
    BUILD_RPATH   "${linkerhand-cpp-sdk_LIBRARY_DIR}"
    INSTALL_RPATH "${linkerhand-cpp-sdk_LIBRARY_DIR}"
)
```

编译运行：

```bash
mkdir build && cd build
cmake ..
cmake --build . -j
./demo
```

## 📚 API 文档

完整的 API 参考见 [`docs/API-Reference.md`](docs/API-Reference.md)，内容涵盖：

- 完整的 API 函数说明
- 参数与返回值详解
- 使用示例与注意事项
- 版本兼容性说明

### 主要 API 接口

**控制接口**
- `fingerMove()` — 设置关节位置（原始值 0–255）
- `fingerMoveArc()` — 设置关节位置（弧度）
- `setSpeed()` — 设置运动速度
- `setTorque()` — 设置扭矩限制

**状态查询**
- `getState()` / `getStateArc()` — 获取关节状态（原始值 / 弧度）
- `getSpeed()` / `getTorque()` — 获取当前速度 / 扭矩设置

**传感器**
- `getForce()` — 压力数据（法向 / 切向 / 方向 / 接近感应）
- `getTemperature()` — 电机温度
- `getFaultCode()` — 电机故障码

## 🔌 通信协议

| 协议 | 枚举值 | 适用范围 |
|------|--------|---------|
| CAN | `COMM_TYPE::CAN` | 全系列 |
| CAN-FD | `COMM_TYPE::CAN`（搭配 `CanFD` 实现） | O20（仅 Linux x86_64） |
| Modbus | `COMM_TYPE::MODBUS` | L7 / L10 / O6 |


通信层通过 `Communication::CommFactory` 构造（旧名 `CanBusFactory` 保留为兼容别名），调用方通过 `LinkerHandApi::setCanTxCallback` / `setCanRxCallback`（或 Modbus 等价回调）注入收发逻辑，SDK 本身不持有任何 socket。

## 🤖 支持的型号

| 型号 | 通信方式 | 备注 |
|------|----------|------|
| LinkerHand G20 | CAN | G20 series robotic hand |
| LinkerHand L6 | CAN | L6 series robotic hand |
| LinkerHand L7 | CAN / Modbus | L7 series robotic hand |
| LinkerHand L10 | CAN / Modbus | L10 series robotic hand |
| LinkerHand L20 | CAN | L20 series robotic hand |
| LinkerHand L21 | CAN | L21 series robotic hand |
| LinkerHand L25 | CAN | L25 series robotic hand |
| LinkerHand O6 | CAN / Modbus | O6 series robotic hand |
| LinkerHand O20 | CAN-FD | O20 series robotic hand（仅 Linux x86_64） |

## 📁 项目结构

```
linkerhand-cpp-sdk/
├── include/                          # 公共头文件
│   ├── api/
│   │   └── LinkerHandApi.h           # 唯一公共入口
│   ├── core/
│   │   ├── Common.h                  # 枚举：LINKER_HAND / HAND_TYPE / COMM_TYPE
│   │   ├── ErrorCode.h
│   │   └── LinkerHandExport.h        # 跨平台符号导出宏
│   └── communication/
│       ├── CommFactory.h             # 通信层工厂（推荐）
│       ├── CanBusFactory.h           # 兼容别名（旧名，将逐步废弃）
│       ├── CommunicationCallbacks.h  # 回调签名
│       ├── CanBus.h / CanFD.h / Modbus.h / PCANBus.h
│       ├── CanFrame.h
│       └── ICanBus.h / ICanFD.h / ICommunication.h / IModbus.h
├── lib/
│   ├── linux/
│   │   ├── x86_64/                   # Linux x86_64 共享库（.so）
│   │   └── arm64/                    # Linux aarch64 共享库（.so）
│   └── win64/
│       └── mingw/                    # Windows MinGW（.dll / .dll.a）
├── examples/                         # 示例源码（CAN / CAN-FD / Modbus）
├── docs/                             # API-Reference / FAQ / TROUBLESHOOTING
├── cmake/                            # find_package 用的 *-config.cmake
├── third_party/
│   ├── libcanbus/                    # CAN / CAN-FD 驱动
│   └── PCAN_Basic/                   # Windows PCAN 驱动
├── build.sh                          # 构建脚本（Linux）
├── build.bat                         # 构建脚本（Windows）
└── CMakeLists.txt                    # 顶层 CMake：聚合 examples + 安装规则
```

> 发布包以预编译共享库为核心，`include/` 与 `lib/` 是对外契约；`examples/` 与 `cmake/` 是配套消费样例与集成胶水。

## 📜 示例程序列表

| 示例文件 | 说明 |
|----------|------|
| `linker_hand_example.cpp` | 多型号 / 多协议巡检 |
| `test_g20_can_0.cpp` | G20 CAN 基础示例 |
| `test_g20_can_1.cpp` | G20 CAN 进阶示例（直连具体类） |
| `test_l7_can_0.cpp` | L7 CAN 基础示例 |
| `test_l7_can_1.cpp` | L7 CAN 进阶示例 |
| `test_l7_modbus_0.cpp` | L7 Modbus 基础示例 |
| `test_l7_modbus_1.cpp` | L7 Modbus 进阶示例 |
| `test_l10_can_0.cpp` | L10 CAN 基础示例 |
| `test_l10_can_1.cpp` | L10 CAN 进阶示例 |
| `test_l10_modbus.cpp` | L10 Modbus 示例 |
| `test_l20_can_0.cpp` | L20 CAN 示例 |
| `test_l21_can_0.cpp` | L21 CAN 示例 |
| `test_o6_can_0.cpp` | O6 CAN 基础示例 |
| `test_o6_can_1.cpp` | O6 CAN 进阶示例 |
| `test_o6_can_3.cpp` | O6 CAN 高级示例 |
| `test_o6_modbus_0.cpp` | O6 Modbus 基础示例 |
| `test_o6_modbus_1.cpp` | O6 Modbus 进阶示例 |
| `test_o6_modbus_2.cpp` | O6 Modbus 高级示例 |
| `test_o20_canfd_0.cpp` | O20 CAN-FD 基础示例（仅 Linux x86_64） |
| `test_o20_canfd_1.cpp` | O20 CAN-FD 进阶示例（仅 Linux x86_64） |
| `L10/action_group_show.cpp` | L10 动作组演示 |

## 📖 关节映射表

- L6 / O6
```
["大拇指弯曲", "大拇指横摆", "食指弯曲", "中指弯曲", "无名指弯曲", "小拇指弯曲"]
```

- L7
```
["大拇指弯曲", "大拇指横摆", "食指弯曲", "中指弯曲", "无名指弯曲", "小拇指弯曲", "拇指旋转"]
```

- L10
```
["拇指根部", "拇指侧摆", "食指根部", "中指根部", "无名指根部", "小指根部",
 "食指侧摆", "无名指侧摆", "小指侧摆", "拇指旋转"]
```

- L20
```
["拇指根部", "食指根部", "中指根部", "无名指根部", "小指根部",
 "拇指侧摆", "食指侧摆", "中指侧摆", "无名指侧摆", "小指侧摆",
 "拇指横摆", "预留", "预留", "预留", "预留",
 "拇指尖部", "食指末端", "中指末端", "无名指末端", "小指末端"]
```

- L21 / L25
详细映射见 [`docs/API-Reference.md`](docs/API-Reference.md)。

## 🔧 故障排查

- [故障排查指南](docs/TROUBLESHOOTING.md) — 编译 / 运行时 / 通信 / API 使用 / 性能
- [常见问题解答](docs/FAQ.md) — 安装、使用、API、兼容性、性能

如文档无法解决：
1. 在 [GitHub Issues](https://github.com/linker-bot/linkerhand-cpp-sdk/issues) 搜索同类问题
2. 提交新 Issue，附错误信息与复现步骤
3. 联系技术支持：<https://linkerbot.cn/aboutUs>

## ❓ 常见问题

快速答案见 [FAQ](docs/FAQ.md)，覆盖：

- 如何安装与配置 SDK
- 如何选择通信接口
- 各型号关节数量
- 如何提升性能
- 如何获取技术支持

## 🤝 贡献

欢迎社区贡献：

1. Fork 本仓库
2. 创建特性分支：`git checkout -b feature/AmazingFeature`
3. 提交更改：`git commit -m 'feat: 新增 AmazingFeature'`
4. 推送：`git push origin feature/AmazingFeature`
5. 开启 Pull Request

更多贡献指南见 `CONTRIBUTING.md`（待创建）。

## 📝 更新日志

详细版本记录见 `CHANGELOG.md`（待创建）。

## 📄 许可证

本项目采用 [MIT 许可证](LICENSE)。

Copyright (c) 2026 灵心巧手（北京）科技有限公司

## 📞 联系我们

- 官方网站：<https://linkerbot.cn>
- 关于我们：<https://linkerbot.cn/aboutUs>
- GitHub：<https://github.com/linker-bot/linkerhand-cpp-sdk>

---

**注意**：使用前请确保设备已正确连接并配置好通信接口。
