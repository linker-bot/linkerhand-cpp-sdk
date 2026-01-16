# 故障排查指南

本文档提供 LinkerHand-CPP-SDK 常见问题的排查和解决方案。

## 目录

- [编译问题](#编译问题)
- [运行时问题](#运行时问题)
- [通信问题](#通信问题)
- [API 使用问题](#api-使用问题)
- [性能问题](#性能问题)
- [获取帮助](#获取帮助)

---

## 编译问题

### 问题：找不到库文件

**错误信息**:
```
linkerhand_cpp_sdk library not found!
```

**可能原因**:
1. 库文件未正确安装
2. 架构不匹配（x86_64 vs aarch64）
3. CMake 查找路径不正确

**解决方案**:
1. 检查库文件是否存在：
   ```bash
   ls -la lib/x86_64/  # 或 lib/aarch64/
   ```

2. 确认系统架构：
   ```bash
   uname -m
   ```

3. 使用安装脚本重新安装：
   ```bash
   ./script.sh
   # 选择选项 [2]: Install SDK
   ```

4. 手动指定库路径（在 CMakeLists.txt 中）：
   ```cmake
   set(LINKER_HAND_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/x86_64/liblinkerhand_cpp_sdk.so)
   ```

---

### 问题：找不到头文件

**错误信息**:
```
fatal error: LinkerHandApi.h: No such file or directory
```

**可能原因**:
1. 头文件路径未正确配置
2. 头文件未安装

**解决方案**:
1. 检查头文件是否存在：
   ```bash
   ls -la include/LinkerHandApi.h
   ```

2. 在 CMakeLists.txt 中添加包含目录：
   ```cmake
   include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
   ```

3. 如果已安装，检查安装路径：
   ```bash
   ls /usr/local/include/linkerhand-cpp-sdk/
   ```

---

### 问题：链接错误

**错误信息**:
```
undefined reference to `LinkerHandApi::...`
```

**可能原因**:
1. 未链接库文件
2. 库文件版本不匹配
3. 缺少依赖库（如 pthread）

**解决方案**:
1. 确保在 CMakeLists.txt 中链接库：
   ```cmake
   target_link_libraries(your_target ${LINKER_HAND_LIB} pthread)
   ```

2. 检查库文件完整性：
   ```bash
   ldd lib/x86_64/liblinkerhand_cpp_sdk.so
   ```

3. 重新编译和安装 SDK

---

### 问题：枚举命名冲突编译错误

**错误信息**:
```
error: 'JOINT_POSITION' conflicts with a previous declaration
error: 'TORQUE_LIMIT' conflicts with a previous declaration
error: 'TOUCH_SENSOR_TYPE' conflicts with a previous declaration
```

**可能原因**:
在旧版本中，多个枚举类型（`L6FrameProperty`、`L7FrameProperty`、`L10FrameProperty` 等）使用 `typedef enum`，导致枚举值泄漏到外层命名空间，当多个头文件同时被包含时会发生命名冲突。

**解决方案**:
此问题已在最新版本中修复。所有枚举类型已改为使用 `enum class`（C++11 强类型枚举），每个枚举值都在自己的作用域内，不会发生命名冲突。

**如果使用旧版本**:
1. 升级到最新版本的 SDK
2. 如果必须使用旧版本，可以：
   - 避免同时包含多个型号的头文件
   - 使用前向声明和单独编译单元
   - 使用命名空间别名隔离不同的枚举类型

**新版本使用方式**:
```cpp
// 使用作用域限定符访问枚举值
linkerhand::hand::L6FrameProperty prop = linkerhand::hand::L6FrameProperty::JOINT_POSITION;

// 转换为整数（用于 CAN 帧）
uint8_t frameProperty = static_cast<uint8_t>(linkerhand::hand::L6FrameProperty::JOINT_POSITION);
```

---

### 问题：CMake 版本过低

**错误信息**:
```
CMake 3.5 or higher is required
```

**解决方案**:
1. 升级 CMake：
   ```bash
   # Ubuntu/Debian
   sudo apt-get update
   sudo apt-get install cmake
   
   # 或从源码编译最新版本
   ```

2. 检查 CMake 版本：
   ```bash
   cmake --version
   ```

---

## 运行时问题

### 问题：程序崩溃或段错误

**可能原因**:
1. 通信接口未正确初始化
2. 设备未连接
3. 参数数组长度不匹配
4. 内存访问错误

**解决方案**:
1. 检查设备连接：
   ```bash
   # CAN 总线
   ip link show can0
   cansend can0 123#DEADBEEF  # 测试 CAN 通信
   ```

2. 检查参数数组长度：
   ```cpp
   // L10 需要 10 个元素
   if (pose.size() != 10) {
       std::cerr << "错误: pose 数组长度应为 10" << std::endl;
       return;
   }
   hand.fingerMove(pose);
   ```

3. 使用调试工具：
   ```bash
   gdb ./your_program
   # 或
   valgrind --leak-check=full ./your_program
   ```

4. 添加错误检查：
   ```cpp
   try {
       LinkerHandApi hand(LINKER_HAND::L10, HAND_TYPE::RIGHT);
       // ...
   } catch (const std::exception& e) {
       std::cerr << "错误: " << e.what() << std::endl;
   }
   ```

---

### 问题：设备无响应

**可能原因**:
1. 通信接口配置错误
2. 设备未上电
3. 通信协议不匹配
4. 设备地址错误

**解决方案**:
1. 检查设备电源和连接

2. 验证通信接口：
   ```cpp
   // 尝试不同的通信接口
   LinkerHandApi hand1(LINKER_HAND::L10, HAND_TYPE::RIGHT, COMM_CAN_0);
   LinkerHandApi hand2(LINKER_HAND::L10, HAND_TYPE::RIGHT, COMM_CAN_1);
   LinkerHandApi hand3(LINKER_HAND::L10, HAND_TYPE::RIGHT, COMM_MODBUS);
   LinkerHandApi hand4(LINKER_HAND::L10, HAND_TYPE::RIGHT, COMM_ETHERCAT);
   ```

3. 检查通信接口状态：
   
   **CAN 总线**:
   ```bash
   # 查看 CAN 接口
   ip link show can0
   
   # 启动 CAN 接口（如果未启动）
   sudo ip link set can0 up type can bitrate 500000
   
   # 监听 CAN 消息
   candump can0
   ```
   
   **ModBus**:
   - 检查串口设备或网络连接
   - 验证 ModBus 配置参数
   
   **EtherCAT**:
   - 使用 EtherCAT 主站工具检查网络状态
   - 验证从站配置和连接

4. 检查设备 ID 和地址设置

---

### 问题：关节不运动或运动异常

**可能原因**:
1. 速度和扭矩设置不当
2. 参数值超出范围
3. 电机故障
4. 机械卡阻

**解决方案**:
1. 检查速度和扭矩设置：
   ```cpp
   // 设置合适的速度和扭矩
   std::vector<uint8_t> speed = {200, 200, 200, 200, 200};
   std::vector<uint8_t> torque = {255, 255, 255, 255, 255};
   hand.setSpeed(speed);
   hand.setTorque(torque);
   ```

2. 检查参数范围：
   ```cpp
   // 确保所有值在 0-255 范围内
   for (auto& val : pose) {
       if (val > 255) val = 255;
       if (val < 0) val = 0;
   }
   ```

3. 检查故障码：
   ```cpp
   auto fault_code = hand.getFaultCode();
   for (size_t i = 0; i < fault_code.size(); i++) {
       if (fault_code[i] != 0) {
           std::cout << "警告: 电机 " << i << " 故障码: " 
                     << static_cast<int>(fault_code[i]) << std::endl;
       }
   }
   ```

4. 检查温度：
   ```cpp
   auto temperature = hand.getTemperature();
   for (size_t i = 0; i < temperature.size(); i++) {
       if (temperature[i] > 80) {  // 假设阈值
           std::cout << "警告: 电机 " << i << " 温度过高: " 
                     << static_cast<int>(temperature[i]) << std::endl;
       }
   }
   ```

---

## 通信问题

### 问题：通信接口连接失败

**错误现象**:
- 设备无响应
- 数据读取失败
- 通信超时

**可能原因**:
- 通信接口未正确配置
- 设备未连接或未上电
- 通信协议不匹配
- 权限问题

**解决方案**:
根据使用的通信协议，参考以下配置：

### CAN 总线通信问题

**错误现象**:
- 设备无响应
- 数据读取失败
- 通信超时

**解决方案**:
1. **检查 CAN 接口配置**:
   ```bash
   # 查看 CAN 接口状态
   ip link show can0
   
   # 配置 CAN 接口
   sudo ip link set can0 type can bitrate 500000
   sudo ip link set can0 up
   ```

2. **检查 CAN 总线连接**:
   ```bash
   # 测试 CAN 通信
   cansend can0 123#DEADBEEF
   candump can0
   ```

3. **检查权限**:
   ```bash
   # 可能需要 root 权限或添加用户到 dialout 组
   sudo usermod -a -G dialout $USER
   # 重新登录后生效
   ```

4. **检查终端电阻**:
   - CAN 总线两端需要 120Ω 终端电阻
   - 检查硬件连接

### ModBus 通信问题

**错误现象**:
- ModBus 连接失败
- 数据读取超时
- 串口权限错误

**解决方案**:
1. **检查串口权限**:
   ```bash
   # 添加用户到 dialout 组
   sudo usermod -a -G dialout $USER
   # 重新登录后生效
   
   # 检查串口设备
   ls -l /dev/ttyUSB*  # USB 转串口
   ls -l /dev/ttyS*    # 串口
   ```

2. **检查串口配置**:
   - 确认串口设备路径正确
   - 检查波特率、数据位、停止位、校验位设置
   - 验证 ModBus 从站地址

3. **测试 ModBus 通信**:
   - 使用 ModBus 测试工具验证连接
   - 检查网络连接（ModBus TCP/IP）

### EtherCAT 通信问题

**错误现象**:
- EtherCAT 主站无法识别从站
- 网络拓扑错误
- 同步失败

**解决方案**:
1. **检查 EtherCAT 主站配置**:
   - 确保 EtherCAT 主站软件已正确安装
   - 验证主站配置与硬件匹配

2. **检查网络拓扑**:
   - 验证 EtherCAT 从站设备已正确连接
   - 检查网络线缆和连接器
   - 确认网络拓扑正确（环形或线性）

3. **检查从站状态**:
   - 使用 EtherCAT 主站工具检查从站状态
   - 验证从站配置和地址

4. **检查同步**:
   - 确保 EtherCAT 网络同步正常
   - 检查分布式时钟（DC）配置（如适用）

---

## API 使用问题

### 问题：参数数组长度错误

**错误现象**:
- 程序崩溃
- 关节运动异常
- 未定义行为

**解决方案**:
1. **根据型号使用正确的数组长度**:
   ```cpp
   std::vector<uint8_t> pose;
   
   switch (hand.handModel_) {
       case LINKER_HAND::L6:
       case LINKER_HAND::O6:
           pose.resize(6);
           break;
       case LINKER_HAND::L7:
           pose.resize(7);
           break;
       case LINKER_HAND::L10:
           pose.resize(10);
           break;
       case LINKER_HAND::L20:
           pose.resize(20);
           break;
       case LINKER_HAND::L21:
       case LINKER_HAND::L25:
           pose.resize(25);
           break;
   }
   ```

2. **添加参数验证**:
   ```cpp
   void safeFingerMove(LinkerHandApi& hand, const std::vector<uint8_t>& pose) {
       int expected_size = 0;
       switch (hand.handModel_) {
           case LINKER_HAND::L10: expected_size = 10; break;
           case LINKER_HAND::L20: expected_size = 20; break;
           // ...
       }
       
       if (pose.size() != expected_size) {
           std::cerr << "错误: 数组长度应为 " << expected_size 
                     << "，实际为 " << pose.size() << std::endl;
           return;
       }
       
       hand.fingerMove(pose);
   }
   ```

---

### 问题：功能不支持

**错误现象**:
- 调用特定型号不支持的功能
- 无响应或错误

**解决方案**:
1. **检查型号支持**:
   ```cpp
   // 仅 L20 支持 setCurrent
   if (hand.handModel_ == LINKER_HAND::L20) {
       hand.setCurrent(current);
   } else {
       std::cerr << "警告: setCurrent 仅支持 L20" << std::endl;
   }
   ```

2. **参考功能支持矩阵**:
   - 查看 [API 参考文档](API-Reference.md#版本兼容性)
   - 了解各型号支持的功能

---

### 问题：返回值理解错误

**错误现象**:
- 数据解析错误
- 单位换算错误

**解决方案**:
1. **理解返回值格式**:
   ```cpp
   // getForce() 返回三维向量
   auto force = hand.getForce();
   // force[finger][sensor_type][data]
   
   // getTemperature() 返回温度值（可能需要换算）
   auto temp = hand.getTemperature();
   // 具体换算方式请参考设备手册
   ```

2. **参考 API 文档**:
   - 查看 [API 参考文档](API-Reference.md) 了解返回值格式
   - 参考设备手册了解数据含义

---

## 性能问题

### 问题：响应延迟

**可能原因**:
1. 通信延迟
2. 处理速度慢
3. 系统负载高

**解决方案**:
1. **优化通信频率**:
   ```cpp
   // 避免过于频繁的 API 调用
   // 添加适当的延时
   std::this_thread::sleep_for(std::chrono::milliseconds(10));
   ```

2. **使用多线程**:
   ```cpp
   // 在单独线程中处理传感器数据
   std::thread sensor_thread([&hand]() {
       while (running) {
           auto force = hand.getForce();
           // 处理数据...
           std::this_thread::sleep_for(std::chrono::milliseconds(100));
       }
   });
   ```

3. **减少不必要的调用**:
   - 缓存不需要实时更新的数据
   - 批量处理操作

---

### 问题：CPU 占用过高

**可能原因**:
1. 轮询频率过高
2. 阻塞操作
3. 资源泄漏

**解决方案**:
1. **降低轮询频率**:
   ```cpp
   // 从 10ms 改为 100ms
   std::this_thread::sleep_for(std::chrono::milliseconds(100));
   ```

2. **使用事件驱动**:
   - 避免轮询，使用回调或事件
   - 仅在需要时读取数据

3. **检查资源管理**:
   ```cpp
   // 确保正确释放资源
   // 使用 RAII 原则
   ```

---

## 获取帮助

如果以上解决方案无法解决您的问题，请尝试以下方式获取帮助：

1. **查看文档**:
   - [API 参考文档](API-Reference.md)
   - [常见问题解答](FAQ.md)
   - [README](README.md)

2. **检查示例代码**:
   - 查看 `examples/` 目录下的示例
   - 参考示例代码的使用方式

3. **联系技术支持**:
   - **官方网站**: [https://linkerbot.cn](https://linkerbot.cn)
   - **技术支持**: [https://linkerbot.cn/aboutUs](https://linkerbot.cn/aboutUs)
   - **GitHub Issues**: [https://github.com/linker-bot/linkerhand-cpp-sdk/issues](https://github.com/linker-bot/linkerhand-cpp-sdk/issues)

4. **提供问题信息**:
   在寻求帮助时，请提供以下信息：
   - 错误信息或日志
   - 系统信息（操作系统、架构）
   - SDK 版本
   - 设备型号
   - 复现步骤
   - 相关代码片段
