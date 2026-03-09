# 常见问题解答 (FAQ)

本文档回答 LinkerHand-CPP-SDK 的常见问题。

## 目录

- [安装相关问题](#安装相关问题)
- [使用相关问题](#使用相关问题)
- [API 相关问题](#api-相关问题)
- [兼容性问题](#兼容性问题)
- [性能相关问题](#性能相关问题)
- [其他问题](#其他问题)

---

## 安装相关问题

### Q1: 如何安装 SDK？

**A**: 有两种安装方式：

**方法一：使用安装脚本（推荐）**
```bash
./script.sh
# 选择选项 [2]: Install SDK
```

**方法二：手动安装**
```bash
mkdir build && cd build
cmake ..
make
sudo make install
```

详细说明请参考 [README.md](../README.md#安装)。

---

### Q2: 安装后找不到库文件怎么办？

**A**: 检查以下几点：

1. **确认安装路径**:
   ```bash
   ls /usr/local/lib/linkerhand-cpp-sdk/
   ```

2. **检查架构匹配**:
   ```bash
   uname -m  # 查看系统架构
   ls lib/x86_64/  # 或 lib/aarch64/
   ```

3. **设置库路径**:
   ```bash
   export LD_LIBRARY_PATH=/usr/local/lib/linkerhand-cpp-sdk/x86_64:$LD_LIBRARY_PATH
   ```

4. **更新动态链接库缓存**:
   ```bash
   sudo ldconfig
   ```

---

### Q3: 支持哪些操作系统和架构？

**A**: 
- **操作系统**: Linux (Ubuntu 18.04+ 推荐)
- **架构**: x86_64、aarch64
- **编译器**: GCC 7.0+ 或 Clang 5.0+
- **CMake**: 3.11+
---

### Q4: 如何卸载 SDK？

**A**: 使用安装脚本：
```bash
./script.sh
# 选择选项 [3]: Uninstall SDK
```

或手动删除：
```bash
sudo rm -rf /usr/local/include/linkerhand-cpp-sdk
sudo rm -rf /usr/local/lib/linkerhand-cpp-sdk
```

---

## 使用相关问题

### Q5: 如何快速开始使用 SDK？

**A**: 参考以下步骤：

1. **创建项目文件**:
   ```cpp
   // main.cpp
   #include "LinkerHandApi.h"
   #include <iostream>
   
   int main() {
       LinkerHandApi hand(LINKER_HAND::L10, HAND_TYPE::RIGHT);
       std::cout << "Version: " << hand.getVersion() << std::endl;
       return 0;
   }
   ```

2. **创建 CMakeLists.txt**:
   ```cmake
   cmake_minimum_required(VERSION 3.5)
   project(MyProject)
   
   # 查找库和头文件
   find_library(LINKER_HAND_LIB linkerhand_cpp_sdk
       PATHS /usr/local/lib/linkerhand-cpp-sdk/x86_64)
   include_directories(/usr/local/include/linkerhand-cpp-sdk)
   
   add_executable(my_project main.cpp)
   target_link_libraries(my_project ${LINKER_HAND_LIB} pthread)
   ```

3. **编译和运行**:
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ./my_project
   ```

详细说明请参考 [README.md](../README.md#快速开始)。

---

### Q6: 如何选择正确的通信接口？

**A**: 根据您的硬件配置选择：

- **CAN 总线**: 默认使用 `COMM_CAN_0`，如果有多个 CAN 接口可使用 `COMM_CAN_1`
- **ModBus**: 使用 `COMM_MODBUS` 进行 ModBus 通信
- **EtherCAT**: 使用 `COMM_ETHERCAT` 进行 EtherCAT 工业以太网通信

```cpp
// CAN0（默认）
LinkerHandApi hand(LINKER_HAND::L10, HAND_TYPE::RIGHT);

// CAN1
LinkerHandApi hand(LINKER_HAND::L10, HAND_TYPE::RIGHT, COMM_CAN_1);

// ModBus
LinkerHandApi hand(LINKER_HAND::L10, HAND_TYPE::RIGHT, COMM_MODBUS);

// EtherCAT
LinkerHandApi hand(LINKER_HAND::L10, HAND_TYPE::RIGHT, COMM_ETHERCAT);
```

---

### Q7: 如何配置通信接口？

**A**: 根据不同的通信协议进行配置：

#### CAN 总线配置

在 Linux 系统中配置 CAN 总线：

```bash
# 加载 CAN 模块
sudo modprobe can
sudo modprobe can_raw
sudo modprobe vcan  # 虚拟 CAN（用于测试）

# 配置 CAN 接口
sudo ip link set can0 type can bitrate 1000000
sudo ip link set can0 up

# 查看 CAN 接口状态
ip link show can0

# 测试 CAN 通信
cansend can0 123#DEADBEEF
candump can0
```

#### ModBus 配置

ModBus 配置取决于使用的接口类型（串口或网络）：

- **串口 ModBus**: 确保串口设备权限正确，通常需要将用户添加到 `dialout` 组
- **网络 ModBus**: 确保网络连接正常，ModBus TCP/IP 端口已开放

#### EtherCAT 配置

EtherCAT 需要配置 EtherCAT 主站：

- 确保 EtherCAT 主站软件已安装和配置
- 检查 EtherCAT 网络拓扑
- 验证 EtherCAT 从站设备已正确连接

---

### Q8: 如何控制手指运动？

**A**: 使用 `fingerMove()` 函数：

```cpp
// 1. 设置速度和扭矩
std::vector<uint8_t> speed = {200, 200, 200, 200, 200};
std::vector<uint8_t> torque = {255, 255, 255, 255, 255};
hand.setSpeed(speed);
hand.setTorque(torque);

// 2. 设置关节位置（L10 示例）
std::vector<uint8_t> pose = {128, 128, 128, 128, 128, 128, 128, 128, 128, 128};
hand.fingerMove(pose);

// 3. 等待动作完成
std::this_thread::sleep_for(std::chrono::seconds(1));
```

---

### Q9: 如何读取传感器数据？

**A**: 使用相应的获取函数：

```cpp
// 获取压力数据
auto force = hand.getForce();

// 获取温度
auto temperature = hand.getTemperature();

// 获取故障码
auto fault_code = hand.getFaultCode();

// 获取电流
auto current = hand.getCurrent();

// 获取关节状态
auto state = hand.getState();
```

---

## API 相关问题

### Q10: 不同型号的关节数量是多少？

**A**: 
- **L6/O6**: 6 个关节
- **L7**: 7 个关节
- **L10**: 10 个关节
- **L20**: 20 个关节
- **L21**: 25 个关节
- **L25**: 25 个关节

详细映射关系请参考 [README.md](../README.md#关节映射表) 或 [API 参考文档](API-Reference.md)。

---

### Q11: `fingerMove()` 和 `fingerMoveArc()` 有什么区别？

**A**: 
- **`fingerMove()`**: 使用 0-255 的数值范围，适用于快速控制
- **`fingerMoveArc()`**: 使用弧度制，适用于精确角度控制

```cpp
// 使用 0-255 范围（L10）
std::vector<uint8_t> pose = {128, 128, 128, 128, 128, 128, 128, 128, 128, 128};
hand.fingerMove(pose);

// 使用弧度制（L10）
std::vector<double> pose_arc = {0.5, 0.3, 0.2, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
hand.fingerMoveArc(pose_arc);
```

**注意**: `fingerMoveArc()` 仅支持 L10、L20、L21、L25 型号。

---

### Q12: 速度和扭矩的取值范围是多少？

**A**: 
- **范围**: 0-255
- **速度**: 值越大，运动速度越快
- **扭矩**: 值越大，允许的最大扭矩越大

```cpp
// 中等速度
std::vector<uint8_t> speed = {200, 200, 200, 200, 200};

// 最大扭矩
std::vector<uint8_t> torque = {255, 255, 255, 255, 255};
```

---

### Q13: `getForce()` 返回的数据结构是什么？

**A**: `getForce()` 返回三维向量：

```cpp
std::vector<std::vector<std::vector<uint8_t>>> force = hand.getForce();

// force[finger][sensor_type][data]
// finger: 0-4 (拇指、食指、中指、无名指、小指)
// sensor_type: 0-3 (法向压力、切向压力、切向方向、接近感应)
// data: 传感器数据矩阵
```

示例：
```cpp
auto force = hand.getForce();
for (size_t finger = 0; finger < force.size(); finger++) {
    for (size_t sensor_type = 0; sensor_type < force[finger].size(); sensor_type++) {
        // 处理数据...
    }
}
```

---

### Q14: 哪些功能仅特定型号支持？

**A**: 
- **`setCurrent()`**: 仅 L20 支持
- **`setEnable()` / `setDisable()`**: 仅 L25 支持
- **`clearFaultCode()`**: 仅 L20 支持
- **`fingerMoveArc()`**: 仅 L10、L20、L21、L25 支持

详细支持矩阵请参考 [API 参考文档](API-Reference.md#版本兼容性)。

---

## 兼容性问题

### Q15: SDK 版本和设备固件版本需要匹配吗？

**A**: 建议使用匹配的版本。如果遇到兼容性问题：

1. 检查 SDK 版本：
   ```cpp
   std::string version = hand.getVersion();
   std::cout << "SDK Version: " << version << std::endl;
   ```

2. 查看 [更新日志](../CHANGELOG.md)（如果存在）了解版本变更

3. 联系技术支持获取兼容性信息

---

### Q16: 可以在 Windows 上使用吗？

**A**: 当前版本主要支持 Linux 系统。Windows 支持情况：

- **当前状态**: 主要支持 Linux
- **Windows**: 可能需要额外的配置和编译
- **建议**: 使用 Linux 系统（Ubuntu 18.04+ 推荐）

如有 Windows 支持需求，请联系技术支持。

---

### Q17: 支持哪些编译器？

**A**: 
- **GCC**: 7.0 或更高版本
- **Clang**: 5.0 或更高版本

建议使用较新的编译器版本以获得更好的兼容性。

---

## 性能相关问题

### Q18: API 调用的响应时间是多少？

**A**: 响应时间取决于多个因素：

- **系统负载**: 系统负载高时响应会变慢
- **网络延迟**: EtherCAT 网络延迟

**建议**:
- 在动作后添加适当的延时：`std::this_thread::sleep_for(std::chrono::milliseconds(100))`
- 避免过于频繁的 API 调用
- 使用多线程处理传感器数据

---

### Q19: 如何提高性能？

**A**: 以下建议可以提高性能：

1. **降低轮询频率**:
   ```cpp
   // 从 10ms 改为 100ms
   std::this_thread::sleep_for(std::chrono::milliseconds(100));
   ```

2. **使用多线程**:
   ```cpp
   std::thread sensor_thread([&hand]() {
       while (running) {
           auto force = hand.getForce();
           // 处理数据...
       }
   });
   ```

3. **批量处理操作**:
   - 减少不必要的 API 调用
   - 缓存不需要实时更新的数据

4. **优化通信**:
   - 优化通信参数

---

### Q20: CPU 占用过高怎么办？

**A**: 可能的原因和解决方案：

1. **轮询频率过高**:
   ```cpp
   // 增加延时
   std::this_thread::sleep_for(std::chrono::milliseconds(100));
   ```

2. **阻塞操作**:
   - 使用异步操作
   - 避免在循环中进行阻塞调用

3. **资源泄漏**:
   - 检查资源是否正确释放
   - 使用 RAII 原则管理资源

---

## 其他问题

### Q21: 如何获取技术支持？

**A**: 可以通过以下方式获取支持：

1. **文档**:
   - [API 参考文档](API-Reference.md)
   - [故障排查指南](TROUBLESHOOTING.md)
   - [README](../README.md)

2. **在线资源**:
   - **官方网站**: [https://linkerbot.cn](https://linkerbot.cn)
   - **技术支持**: [https://linkerbot.cn/aboutUs](https://linkerbot.cn/aboutUs)
   - **GitHub**: [https://github.com/linker-bot/linkerhand-cpp-sdk](https://github.com/linker-bot/linkerhand-cpp-sdk)

3. **GitHub Issues**:
   - 提交问题报告
   - 查看已有问题和解决方案

---

### Q22: 如何贡献代码？

**A**: 欢迎贡献！请参考以下步骤：

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

详细说明请参考 [README.md](../README.md#贡献)。

---

### Q23: 如何报告 Bug？

**A**: 在 GitHub Issues 中报告 Bug 时，请提供：

1. **错误信息**: 完整的错误信息或日志
2. **系统信息**: 
   - 操作系统和版本
   - 系统架构（x86_64/aarch64）
   - SDK 版本
3. **设备信息**: 
   - 设备型号（L6/L7/L10/L20/L21/L25）
   - 固件版本（如果知道）
4. **复现步骤**: 详细的步骤说明
5. **相关代码**: 能够复现问题的代码片段

---

### Q24: 有示例代码吗？

**A**: 是的，项目提供了示例代码：

- **位置**: `examples/` 目录
- **示例**:
  - `toolset_example.cpp` - 工具集示例
  - `action_group_show_l10.cpp` - L10 动作组示例

运行示例：
```bash
cd build
./toolset_example
./action_group_show_l10
```

详细说明请参考 [示例代码文档](../examples/README.md)。

---

### Q25: 如何更新 SDK？

**A**: 更新步骤：

1. **拉取最新代码**:
   ```bash
   git pull origin main
   ```

2. **重新编译和安装**:
   ```bash
   ./script.sh
   # 选择选项 [2]: Install SDK
   ```

3. **或手动更新**:
   ```bash
   cd build
   cmake ..
   make
   sudo make install
   ```

如有其他问题，请参考 [故障排查指南](TROUBLESHOOTING.md) 或联系技术支持。
