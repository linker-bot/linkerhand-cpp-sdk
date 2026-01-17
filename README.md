# LinkerHand-CPP-SDK

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-Linux-blue.svg)](https://www.linux.org/)
[![Architecture](https://img.shields.io/badge/Arch-x86__64%20%7C%20aarch64-lightgrey.svg)]()
[![CI/CD](https://github.com/linker-bot/linkerhand-cpp-sdk/workflows/CI/CD/badge.svg)](https://github.com/linker-bot/linkerhand-cpp-sdk/actions)

> 用于灵心巧手系列灵巧手的 C++ 软件开发工具包

LinkerHand-CPP-SDK 是由灵心巧手（北京）科技有限公司开发的官方 C++ SDK，提供完整的 API 接口用于控制 O6、L6、L7、L10、L20、L21、L25 等型号的灵巧手设备。

## 📋 目录

- [特性](#-特性)
- [系统要求](#-系统要求)
- [快速开始](#-快速开始)
- [安装](#-安装)
- [使用示例](#-使用示例)
- [API 文档](#-api-文档)
- [通信协议](#-通信协议)
- [项目结构](#-项目结构)
- [构建项目](#-构建项目)
- [故障排查](#-故障排查)
- [常见问题](#-常见问题)
- [贡献](#-贡献)
- [许可证](#-许可证)
- [联系我们](#-联系我们)

## ✨ 特性

- 🎯 **多型号支持** - 支持 O6、L6、L7、L10、L20、L21、L25 等多种型号
- 🚀 **简单易用** - 提供简洁的 C++ API 接口
- 🔌 **通信协议** - 支持 CAN 通信
- 📊 **传感器数据** - 实时获取压力、温度、电流等传感器数据
- 🎮 **精确控制** - 支持关节位置、速度、扭矩的精确控制
- 🔄 **实时反馈** - 获取关节状态、电机故障码等实时信息
- 🛠️ **跨平台** - 支持 Linux (x86_64, aarch64)
- 📚 **完整文档** - 提供详细的 API 文档和使用示例

## 💻 系统要求

- **操作系统**: Linux (Ubuntu 18.04+ 推荐)
- **架构**: x86_64 或 aarch64
- **编译器**: GCC 7.0+ 或 Clang 5.0+
- **CMake**: 3.15+
- **依赖**: pthread

## 🚀 快速开始

### 1. 克隆仓库

```bash
git clone https://github.com/linker-bot/linkerhand-cpp-sdk.git
cd linkerhand-cpp-sdk
```

### 2. 使用安装脚本（推荐）

```bash
./script.sh
```

选择选项 `[2]: Install SDK` 进行构建和安装。

### 3. 基本使用示例

创建 `main.cpp` 文件：

```cpp
#include "LinkerHandApi.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    // 初始化 L10 型号右手
    LinkerHandApi hand(LINKER_HAND::L10, HAND_TYPE::RIGHT);

    // 获取版本信息
    std::cout << "SDK Version: " << hand.getVersion() << std::endl;

    // 握拳动作
    std::vector<uint8_t> fist_pose = {101, 60, 0, 0, 0, 0, 255, 255, 255, 51};
    hand.fingerMove(fist_pose);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 张开动作
    std::vector<uint8_t> open_pose = {255, 104, 255, 255, 255, 255, 255, 255, 255, 71};
    hand.fingerMove(open_pose);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}
```

### 4. 编译和运行

创建 `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.15)
project(LinkerHand-CPP-SDK)

# 检测系统架构
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(LIB_SUBDIR "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(LIB_SUBDIR "aarch64")
else()
    message(WARNING "Unknown architecture, defaulting to x86_64")
    set(LIB_SUBDIR "x86_64")
endif()

# 查找库文件
find_library(LINKER_HAND_LIB
    NAMES linkerhand_cpp_sdk
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib/${LIB_SUBDIR}
        /usr/local/lib/linkerhand-cpp-sdk/${LIB_SUBDIR}
        /usr/lib/linkerhand-cpp-sdk/${LIB_SUBDIR}
    NO_DEFAULT_PATH
)

# 查找头文件
set(LINKER_HAND_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    /usr/local/include/linkerhand-cpp-sdk
    /usr/include/linkerhand-cpp-sdk
)

if(NOT LINKER_HAND_LIB)
    message(FATAL_ERROR "linkerhand_cpp_sdk library not found!")
endif()

# 包含头文件
include_directories(${LINKER_HAND_INCLUDE_DIR})

# 创建可执行文件
add_executable(my_project main.cpp)
target_link_libraries(my_project ${LINKER_HAND_LIB} pthread)
```

编译和运行：

```bash
mkdir build && cd build
cmake ..
make
./my_project
```

## 📦 安装

### 方法一：使用安装脚本（推荐）

```bash
./script.sh
```

脚本提供以下功能：
- `[1]`: 构建 SDK
- `[2]`: 构建并安装 SDK
- `[3]`: 卸载 SDK
- `[6]`: 运行示例程序

### 方法二：手动安装

```bash
# 构建项目
mkdir build && cd build
cmake ..
make

# 安装（需要 root 权限）
sudo make install
```

安装后，库文件将安装到：
- 头文件: `/usr/local/include/linkerhand-cpp-sdk/`
- 库文件: `/usr/local/lib/linkerhand-cpp-sdk/{arch}/`

## 💡 使用示例

### 基本控制

```cpp
#include "LinkerHandApi.h"
#include <vector>
#include <thread>
#include <chrono>

int main() {
    // 创建 API 实例
    LinkerHandApi hand(LINKER_HAND::L10, HAND_TYPE::RIGHT);

    // 设置速度
    std::vector<uint8_t> speed = {200, 200, 200, 200, 200};
    hand.setSpeed(speed);

    // 设置扭矩
    std::vector<uint8_t> torque = {255, 255, 255, 255, 255};
    hand.setTorque(torque);

    // 控制手指运动
    std::vector<uint8_t> pose = {128, 128, 128, 128, 128, 128, 128, 128, 128, 128};
    hand.fingerMove(pose);

    return 0;
}
```

### 获取传感器数据

```cpp
// 获取压力数据
auto force_data = hand.getForce();

// 获取关节状态
auto joint_state = hand.getState();

// 获取电机温度
auto temperature = hand.getTemperature();

// 获取故障码
auto fault_code = hand.getFaultCode();
```

### 使用弧度制控制（L10/L20/L21/L25）

```cpp
// 使用弧度制控制关节
std::vector<double> pose_arc = {0.5, 0.3, 0.2, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
hand.fingerMoveArc(pose_arc);

// 获取弧度制关节状态
auto state_arc = hand.getStateArc();
```

更多示例请参考 `examples/` 目录。详细说明请查看 [示例代码文档](examples/README.md)。

## 📚 API 文档

详细的 API 文档请参考：[API 参考文档](docs/API-Reference.md)

文档包含：
- 完整的 API 函数说明
- 参数和返回值详解
- 使用示例和注意事项
- 版本兼容性说明

### 命名规范

项目已进行命名规范改进，详细信息请参考：[命名规范改进文档](docs/NAMING_IMPROVEMENTS.md)

主要改进包括：
- 统一命名空间结构（所有型号实现类迁移到 `linkerhand::hand` 命名空间）
- 类名改进（不同型号使用不同类名，如 `L10Hand`, `L20Hand` 等）
- 参数命名修正（`handJoint` → `handModel`）
- 完全向后兼容（旧命名仍可使用）

## 🧪 测试

### 运行测试

```bash
# 构建并运行测试
mkdir build && cd build
cmake .. -DBUILD_TESTING=ON
make
ctest --output-on-failure
```

### 测试文档

- **测试指南**: [测试文档](docs/TESTING.md)（待创建） - 详细的测试框架安装、编写和运行指南
- **测试说明**: [测试目录 README](tests/README.md) - 测试结构和快速参考
- **CI/CD 文档**: [CI/CD 文档](docs/CI-CD.md) - 持续集成和持续部署配置说明

测试文档包含：
- 测试框架安装和配置（Google Test）
- 如何编写和运行测试
- 测试覆盖率配置
- 测试最佳实践
- CI/CD 集成指南

### 当前测试覆盖

项目已包含以下单元测试：
- ✅ `test_Common.cpp` - Common.h 枚举和类型测试
- ✅ `test_RangeToArc.cpp` - 范围到弧度转换函数测试
- ✅ `test_IHand_Utils.cpp` - IHand 工具函数测试
- ✅ `test_CanFrame.cpp` - CAN 帧结构测试

### 主要 API 接口

#### 控制接口
- `fingerMove()` - 设置关节位置（0-255）
- `fingerMoveArc()` - 设置关节位置（弧度制）
- `setSpeed()` - 设置运动速度
- `setTorque()` - 设置扭矩限制

#### 状态查询接口
- `getState()` - 获取关节状态
- `getStateArc()` - 获取关节状态（弧度制）
- `getSpeed()` - 获取当前速度设置
- `getTorque()` - 获取当前扭矩设置

#### 传感器接口
- `getForce()` - 获取压力数据（法向、切向、方向、接近感应）
- `getTemperature()` - 获取电机温度
- `getFaultCode()` - 获取电机故障码
- `getCurrent()` - 获取电机电流

## 🔌 通信协议

SDK 支持以下通信协议：

- **CAN** (`COMM_CAN_0`, `COMM_CAN_1`) - CAN 总线通信
- **ModBus** (`COMM_MODBUS`) - ModBus 通信协议
- **EtherCAT** (`COMM_ETHERCAT`) - EtherCAT 工业以太网通信

默认使用 `COMM_CAN_0`。

```cpp
// 使用 CAN0 通信（默认）
LinkerHandApi hand(LINKER_HAND::L10, HAND_TYPE::RIGHT);

// 使用 CAN1 通信
LinkerHandApi hand(LINKER_HAND::L10, HAND_TYPE::RIGHT, COMM_CAN_1);

// 使用 ModBus 通信
LinkerHandApi hand(LINKER_HAND::L10, HAND_TYPE::RIGHT, COMM_MODBUS);

// 使用 EtherCAT 通信
LinkerHandApi hand(LINKER_HAND::L10, HAND_TYPE::RIGHT, COMM_ETHERCAT);
```

## 📁 项目结构

```
linkerhand-cpp-sdk/
├── include/              # 头文件
│   ├── LinkerHandApi.h   # 主 API 接口
│   ├── Common.h          # 通用定义
│   └── ...
├── lib/                  # 预编译库文件
│   ├── x86_64/          # x86_64 架构库
│   └── aarch64/         # aarch64 架构库
├── examples/            # 示例代码
│   ├── toolset_example.cpp
│   └── action_group_show_l10.cpp
├── docs/                # 文档
│   ├── API-Reference.md    # API 参考文档
│   ├── TROUBLESHOOTING.md   # 故障排查指南
│   └── FAQ.md               # 常见问题解答
├── src/                 # 源代码
├── third_party/         # 第三方依赖
├── CMakeLists.txt       # CMake 配置
├── script.sh            # 安装脚本
└── README.md            # 本文档
```

## 🔨 构建项目

### 使用 CMake

```bash
mkdir build
cd build
cmake ..
make
```

### 运行示例

```bash
cd build
./toolset_example
./action_group_show_l10
```

## 📖 关节映射表

不同型号的关节映射关系：

### L6/O6
```
["大拇指弯曲", "大拇指横摆", "食指弯曲", "中指弯曲", "无名指弯曲", "小拇指弯曲"]
```

### L7
```
["大拇指弯曲", "大拇指横摆", "食指弯曲", "中指弯曲", "无名指弯曲", "小拇指弯曲", "拇指旋转"]
```

### L10
```
["拇指根部", "拇指侧摆", "食指根部", "中指根部", "无名指根部", "小指根部", 
 "食指侧摆", "无名指侧摆", "小指侧摆", "拇指旋转"]
```

### L20
```
["拇指根部", "食指根部", "中指根部", "无名指根部", "小指根部",
 "拇指侧摆", "食指侧摆", "中指侧摆", "无名指侧摆", "小指侧摆",
 "拇指横摆", "预留", "预留", "预留", "预留",
 "拇指尖部", "食指末端", "中指末端", "无名指末端", "小指末端"]
```

### L21/L25
详细映射请参考 [API 文档](docs/API-Reference.md)。

## 🔧 故障排查

如果遇到问题，请参考以下文档：

- **[故障排查指南](docs/TROUBLESHOOTING.md)** - 常见问题的排查和解决方案
  - 编译问题
  - 运行时问题
  - 通信问题
  - API 使用问题
  - 性能问题

- **[常见问题解答](docs/FAQ.md)** - 常见问题的快速解答
  - 安装相关问题
  - 使用相关问题
  - API 相关问题
  - 兼容性问题
  - 性能相关问题

如果文档无法解决您的问题，请：
1. 查看 [GitHub Issues](https://github.com/linker-bot/linkerhand-cpp-sdk/issues) 查看是否有类似问题
2. 提交新的 Issue，并提供详细的错误信息和复现步骤
3. 联系技术支持：[https://linkerbot.cn/aboutUs](https://linkerbot.cn/aboutUs)

## ❓ 常见问题

快速查找常见问题的答案，请参考 [常见问题解答 (FAQ)](docs/FAQ.md)。

常见问题包括：
- 如何安装和配置 SDK？
- 如何选择通信接口？
- 不同型号的关节数量是多少？
- 如何提高性能？
- 如何获取技术支持？

## 🤝 贡献

我们欢迎社区贡献！请参考以下步骤：

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

更多贡献指南请参考 [CONTRIBUTING.md](CONTRIBUTING.md)（待创建）。

## 📄 许可证

本项目采用 [MIT 许可证](LICENSE)。

Copyright (c) 2026 灵心巧手（北京）科技有限公司

## 📞 联系我们

- **官方网站**: [https://linkerbot.cn](https://linkerbot.cn)
- **关于我们**: [https://linkerbot.cn/aboutUs](https://linkerbot.cn/aboutUs)
- **GitHub**: [https://github.com/linker-bot/linkerhand-cpp-sdk](https://github.com/linker-bot/linkerhand-cpp-sdk)

## 📝 更新日志

详细的版本更新记录请参考 [CHANGELOG.md](CHANGELOG.md)（待创建）。

---

**注意**: 使用前请确保设备已正确连接并配置好通信接口。
