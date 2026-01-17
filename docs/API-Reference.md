# LinkerHand C++ API 参考文档

## 目录

- [API 概述](#api-概述)
- [快速索引](#快速索引)
- [构造函数](#构造函数)
- [控制接口](#控制接口)
- [状态查询接口](#状态查询接口)
- [传感器接口](#传感器接口)
- [电机管理接口](#电机管理接口)
- [使用示例](#使用示例)
- [注意事项](#注意事项)
- [版本兼容性](#版本兼容性)

---

## API 概述

LinkerHand C++ API 提供了完整的接口用于控制灵心巧手系列灵巧手设备。支持 O6、L6、L7、L10、L20、L21、L25 等多种型号，支持 CAN、ModBus、EtherCAT 等多种通信协议。

---

## 快速索引

### 控制接口
- [`fingerMove()`](#fingermove) - 设置关节位置（0-255）
- [`fingerMoveArc()`](#fingermovearc) - 设置关节位置（弧度制）
- [`setSpeed()`](#setspeed) - 设置运动速度
- [`setTorque()`](#settorque) - 设置扭矩限制

### 状态查询接口
- [`getState()`](#getstate) - 获取关节状态
- [`getStateArc()`](#getstatearc) - 获取关节状态（弧度制）
- [`getSpeed()`](#getspeed) - 获取当前速度设置
- [`getTorque()`](#gettorque) - 获取当前扭矩设置
- [`getVersion()`](#getversion) - 获取版本信息

### 传感器接口
- [`getForce()`](#getforce) - 获取压力数据（法向、切向、方向、接近感应）
- [`getTemperature()`](#gettemperature) - 获取电机温度
- [`getFaultCode()`](#getfaultcode) - 获取电机故障码
- [`getCurrent()`](#getcurrent) - 获取电机电流

### 电机管理接口
- [`setCurrent()`](#setcurrent) - 设置电流（仅 L20）
- [`setEnable()`](#setenable) - 使能电机（仅 L25）
- [`setDisable()`](#setdisable) - 禁用电机（仅 L25）
- [`clearFaultCode()`](#clearfaultcode) - 清除故障码（仅 L20）

---

## 构造函数

### LinkerHandApi

```cpp
LinkerHandApi(const LINKER_HAND &handModel, const HAND_TYPE &handType, const COMM_TYPE commType = COMM_CAN_0);
```

**功能描述**
创建并初始化 LinkerHand API 实例。

**参数说明**
- `handModel` (LINKER_HAND): 机械手型号
  - 可选值: `LINKER_HAND::O6`, `LINKER_HAND::L6`, `LINKER_HAND::L7`, `LINKER_HAND::L10`, `LINKER_HAND::L20`, `LINKER_HAND::L21`, `LINKER_HAND::L25`
- `handType` (HAND_TYPE): 机械手类型
  - `HAND_TYPE::LEFT` - 左手
  - `HAND_TYPE::RIGHT` - 右手
- `commType` (COMM_TYPE): 通信类型（可选，默认 `COMM_CAN_0`）
  - `COMM_CAN_0` - CAN 总线 0
  - `COMM_CAN_1` - CAN 总线 1
  - `COMM_MODBUS` - ModBus 通信协议
  - `COMM_ETHERCAT` - EtherCAT 工业以太网通信

**返回值**
无

**使用示例**
```cpp
#include "LinkerHandApi.h"

int main() {
    // 创建 L10 型号右手实例，使用默认 CAN0 通信
    LinkerHandApi hand(LINKER_HAND::L10, HAND_TYPE::RIGHT);

    // 创建 L20 型号左手实例，使用 CAN1 通信
    LinkerHandApi hand(LINKER_HAND::L20, HAND_TYPE::LEFT, COMM_CAN_1);

    return 0;
}
```

**注意事项**
- 构造函数会自动初始化通信接口
- 确保在创建实例前，通信接口已正确配置（如 CAN 总线已启动、ModBus 端口已配置、EtherCAT 网络已初始化）
- 不同型号支持的通信协议可能不同，请参考设备手册
- 使用 CAN 总线前，确保总线已正确配置和启动
- 使用 ModBus 前，确保串口或网络连接已建立
- 使用 EtherCAT 前，确保 EtherCAT 主站已正确配置

---

## 控制接口

### fingerMove

```cpp
void fingerMove(const std::vector<uint8_t> &pose);
```

**功能描述**
设置关节的目标位置，用于控制手指的运动。使用 0-255 的数值范围。

**参数说明**
- `pose` (std::vector<uint8_t>): 关节位置数组
  - **L6/O6**: 长度为 6
  - **L7**: 长度为 7
  - **L10**: 长度为 10
  - **L20**: 长度为 20
  - **L21**: 长度为 25
  - **L25**: 长度为 25
  - 取值范围: 0-255（0 表示完全弯曲，255 表示完全伸直）

**返回值**
无

**关节映射关系**

**L6/O6** (6 个关节):
```
[0] 大拇指弯曲
[1] 大拇指横摆
[2] 食指弯曲
[3] 中指弯曲
[4] 无名指弯曲
[5] 小拇指弯曲
```

**L7** (7 个关节):
```
[0] 大拇指弯曲
[1] 大拇指横摆
[2] 食指弯曲
[3] 中指弯曲
[4] 无名指弯曲
[5] 小拇指弯曲
[6] 拇指旋转
```

**L10** (10 个关节):
```
[0] 拇指根部
[1] 拇指侧摆
[2] 食指根部
[3] 中指根部
[4] 无名指根部
[5] 小指根部
[6] 食指侧摆
[7] 无名指侧摆
[8] 小指侧摆
[9] 拇指旋转
```

**L20** (20 个关节):
```
[0-4]  拇指根部、食指根部、中指根部、无名指根部、小指根部
[5-9]  拇指侧摆、食指侧摆、中指侧摆、无名指侧摆、小指侧摆
[10]   拇指横摆
[11-14] 预留
[15-19] 拇指尖部、食指末端、中指末端、无名指末端、小指末端
```

**L21** (25 个关节):
```
[0-4]  大拇指根部、食指根部、中指根部、无名指根部、小拇指根部
[5-9]  大拇指侧摆、食指侧摆、中指侧摆、无名指侧摆、小拇指侧摆
[10]   大拇指横滚
[11-14] 预留
[15]   大拇指中部
[16-19] 预留
[20-24] 大拇指指尖、食指指尖、中指指尖、无名指指尖、小拇指指尖
```

**L25** (25 个关节):
```
[0-4]  大拇指根部、食指根部、中指根部、无名指根部、小拇指根部
[5-9]  大拇指侧摆、食指侧摆、中指侧摆、无名指侧摆、小拇指侧摆
[10]   大拇指横滚
[11-14] 预留
[15-19] 大拇指中部、食指中部、中指中部、无名指中部、小拇指中部
[20-24] 大拇指指尖、食指指尖、中指指尖、无名指指尖、小拇指指尖
```

**使用示例**
```cpp
// L10 握拳
std::vector<uint8_t> fist_pose = {101, 60, 0, 0, 0, 0, 255, 255, 255, 51};
hand.fingerMove(fist_pose);

// L10 张开
std::vector<uint8_t> open_pose = {255, 104, 255, 255, 255, 255, 255, 255, 255, 71};
hand.fingerMove(open_pose);
```

**注意事项**
- 数组长度必须与型号匹配，否则可能导致未定义行为
- 建议在调用前先设置速度和扭矩
- 动作执行需要时间，建议在调用后添加适当的延时

---

### fingerMoveArc

```cpp
void fingerMoveArc(const std::vector<double> &pose);
```

**功能描述**
设置关节的目标位置，使用弧度制。适用于需要精确角度控制的场景。

**参数说明**
- `pose` (std::vector<double>): 关节位置数组（弧度制）
  - **L10**: 长度为 10
  - **L20**: 长度为 20
  - **L21**: 长度为 25
  - **L25**: 长度为 25
  - 取值范围: 根据 URDF 限位定义

**返回值**
无

**使用示例**
```cpp
// L10 使用弧度制控制
std::vector<double> pose_arc = {0.5, 0.3, 0.2, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
hand.fingerMoveArc(pose_arc);
```

**注意事项**
- 仅支持 L10、L20、L21、L25 型号
- 弧度值必须在 URDF 定义的限位范围内
- 建议参考设备的 URDF 文件了解各关节的限位范围

---

### setSpeed

```cpp
void setSpeed(const std::vector<uint8_t> &speed);
```

**功能描述**
设置手部的运动速度。速度值影响关节运动的快慢。

**参数说明**
- `speed` (std::vector<uint8_t>): 速度数组
  - 长度: 5 个元素（对应 5 根手指）
  - 取值范围: 0-255
  - 值越大，运动速度越快

**返回值**
无

**使用示例**
```cpp
// 设置中等速度
std::vector<uint8_t> speed = {200, 200, 200, 200, 200};
hand.setSpeed(speed);

// 设置不同手指的不同速度
std::vector<uint8_t> speed_custom = {255, 200, 200, 200, 150};
hand.setSpeed(speed_custom);
```

**注意事项**
- 速度设置会影响所有后续的 `fingerMove()` 调用
- 建议在初始化时设置速度
- 过高的速度可能导致运动不稳定

---

### setTorque

```cpp
void setTorque(const std::vector<uint8_t> &torque);
```

**功能描述**
设置手部的扭矩限制。扭矩值影响手指的抓握力度。

**参数说明**
- `torque` (std::vector<uint8_t>): 扭矩数组
  - 长度: 5 个元素（对应 5 根手指）
  - 取值范围: 0-255
  - 值越大，允许的最大扭矩越大

**返回值**
无

**使用示例**
```cpp
// 设置最大扭矩
std::vector<uint8_t> torque = {255, 255, 255, 255, 255};
hand.setTorque(torque);

// 设置较小的扭矩（轻柔抓握）
std::vector<uint8_t> torque_light = {128, 128, 128, 128, 128};
hand.setTorque(torque_light);
```

**注意事项**
- 扭矩设置会影响所有后续的 `fingerMove()` 调用
- 建议在初始化时设置扭矩
- 过高的扭矩可能导致设备损坏，请谨慎设置

---

## 状态查询接口

### getState

```cpp
std::vector<uint8_t> getState();
```

**功能描述**
获取当前关节的状态信息，返回值为 0-255 的数值。

**参数说明**  
无

**返回值**
- `std::vector<uint8_t>`: 当前关节状态数组
  - 长度: 根据型号不同（L10=10, L20=20, L21=25, L25=25）
  - 取值范围: 0-255

**使用示例**
```cpp
auto state = hand.getState();
for (size_t i = 0; i < state.size(); i++) {
    std::cout << "关节 " << i << ": " << static_cast<int>(state[i]) << std::endl;
}
```

**注意事项**
- 返回值反映的是当前关节的实际位置
- 如果关节正在运动，返回值会持续变化

---

### getStateArc

```cpp
std::vector<double> getStateArc();
```

**功能描述**
获取当前关节的状态信息，返回值为弧度制。

**参数说明**  
无

**返回值**
- `std::vector<double>`: 当前关节状态数组（弧度制）
  - 长度: 根据型号不同（L10=10, L20=20, L21=25, L25=25）

**使用示例**
```cpp
auto state_arc = hand.getStateArc();
for (size_t i = 0; i < state_arc.size(); i++) {
    std::cout << "关节 " << i << ": " << state_arc[i] << " 弧度" << std::endl;
}
```

**注意事项**
- 仅支持 L10、L20、L21、L25 型号
- 返回值反映的是当前关节的实际位置（弧度制）

---

### getSpeed

```cpp
std::vector<uint8_t> getSpeed();
```

**功能描述**
获取当前设置的速度值。

**参数说明**  
无

**返回值**
- `std::vector<uint8_t>`: 当前速度设置数组
  - 长度: 5 个元素

**使用示例**
```cpp
auto speed = hand.getSpeed();
std::cout << "当前速度设置: ";
for (auto s : speed) {
    std::cout << static_cast<int>(s) << " ";
}
std::cout << std::endl;
```

---

### getTorque

```cpp
std::vector<uint8_t> getTorque();
```

**功能描述**
获取当前设置的最大扭矩值。

**参数说明**  
无

**返回值**
- `std::vector<uint8_t>`: 当前扭矩设置数组
  - 长度: 5 个元素

**使用示例**
```cpp
auto torque = hand.getTorque();
std::cout << "当前扭矩设置: ";
for (auto t : torque) {
    std::cout << static_cast<int>(t) << " ";
}
std::cout << std::endl;
```

---

### getVersion

```cpp
std::string getVersion();
```

**功能描述**
获取当前软件或硬件的版本号。

**参数说明**  
无

**返回值**
- `std::string`: 版本号字符串

**使用示例**
```cpp
std::string version = hand.getVersion();
std::cout << "版本号: " << version << std::endl;
```

---

## 传感器接口

### getForce

```cpp
std::vector<std::vector<std::vector<uint8_t>>> getForce();
```

**功能描述**
获取手部传感器的综合数据，包括法向压力、切向压力、切向方向和接近感应。

**参数说明**  
无

**返回值**
- `std::vector<std::vector<std::vector<uint8_t>>>`: 三维向量
  - 第一维: 手指索引（0-4，对应拇指、食指、中指、无名指、小指）
  - 第二维: 传感器类型（法向压力、切向压力、切向方向、接近感应）
  - 第三维: 传感器数据矩阵

**使用示例**
```cpp
auto force_data = hand.getForce();

// 遍历每根手指
for (size_t finger = 0; finger < force_data.size(); finger++) {
    std::cout << "手指 " << finger << ":" << std::endl;
    
    // 遍历传感器类型
    for (size_t sensor_type = 0; sensor_type < force_data[finger].size(); sensor_type++) {
        std::cout << "  传感器类型 " << sensor_type << ": ";
        
        // 遍历传感器数据
        for (auto value : force_data[finger][sensor_type]) {
            std::cout << static_cast<int>(value) << " ";
        }
        std::cout << std::endl;
    }
}
```

**注意事项**
- 返回值结构复杂，建议参考示例代码理解数据结构
- 不同型号的传感器配置可能不同

---

### getTemperature

```cpp
std::vector<uint8_t> getTemperature();
```

**功能描述**
获取电机的温度数据。

**参数说明**  
无

**返回值**
- `std::vector<uint8_t>`: 电机温度数组
  - 长度: 根据型号不同
  - 单位: 摄氏度（具体换算方式请参考设备手册）

**使用示例**
```cpp
auto temperature = hand.getTemperature();
for (size_t i = 0; i < temperature.size(); i++) {
    std::cout << "电机 " << i << " 温度: " << static_cast<int>(temperature[i]) << std::endl;
}
```

**注意事项**
- 温度值可能需要根据设备手册进行换算
- 建议定期检查温度，避免过热

---

### getFaultCode

```cpp
std::vector<uint8_t> getFaultCode();
```

**功能描述**
获取电机的故障码。用于诊断设备故障。

**参数说明**  
无

**返回值**
- `std::vector<uint8_t>`: 故障码数组
  - 长度: 根据型号不同
  - 0 表示正常，非 0 表示存在故障

**使用示例**
```cpp
auto fault_code = hand.getFaultCode();
for (size_t i = 0; i < fault_code.size(); i++) {
    if (fault_code[i] != 0) {
        std::cout << "警告: 电机 " << i << " 故障码: " 
                  << static_cast<int>(fault_code[i]) << std::endl;
    }
}
```

**注意事项**
- 故障码的具体含义请参考设备手册
- 出现故障码时，建议停止操作并检查设备

---

### getCurrent

```cpp
std::vector<uint8_t> getCurrent();
```

**功能描述**
获取电机的当前电流值。

**参数说明**  
无

**返回值**
- `std::vector<uint8_t>`: 电流数组
  - 长度: 根据型号不同
  - 单位: 安培（具体换算方式请参考设备手册）

**使用示例**
```cpp
auto current = hand.getCurrent();
for (size_t i = 0; i < current.size(); i++) {
    std::cout << "电机 " << i << " 电流: " << static_cast<int>(current[i]) << std::endl;
}
```

**注意事项**
- 电流值可能需要根据设备手册进行换算
- 异常高的电流可能表示负载过大或故障

---

## 电机管理接口

### setCurrent

```cpp
void setCurrent(const std::vector<uint8_t> &current);
```

**功能描述**
设置电机的电流值。**目前仅支持 L20 型号。**

**参数说明**
- `current` (std::vector<uint8_t>): 电流数组
  - 长度: 根据电机数量
  - 取值范围: 请参考设备手册

**返回值**
无

**使用示例**
```cpp
// 仅 L20 支持
if (hand.handModel_ == LINKER_HAND::L20) {
    std::vector<uint8_t> current = {100, 100, 100, 100, 100};
    hand.setCurrent(current);
}
```

**注意事项**
- 仅支持 L20 型号
- 电流设置需要谨慎，过高的电流可能损坏设备

---

### setEnable

```cpp
void setEnable(const std::vector<uint8_t> &enable = std::vector<uint8_t>(5, 0));
```

**功能描述**
使能电机。**目前仅支持 L25 型号。**

**参数说明**
- `enable` (std::vector<uint8_t>): 使能数组（可选，默认全 0）
  - 长度: 5 个元素
  - 0 表示禁用，非 0 表示使能

**返回值**
无

**使用示例**
```cpp
// 仅 L25 支持
if (hand.handModel_ == LINKER_HAND::L25) {
    std::vector<uint8_t> enable = {1, 1, 1, 1, 1}; // 使能所有手指
    hand.setEnable(enable);
}
```

**注意事项**
- 仅支持 L25 型号
- 默认参数为全 0（禁用状态）

---

### setDisable

```cpp
void setDisable(const std::vector<uint8_t> &disable = std::vector<uint8_t>(5, 1));
```

**功能描述**
禁用电机。**目前仅支持 L25 型号。**

**参数说明**
- `disable` (std::vector<uint8_t>): 禁用数组（可选，默认全 1）
  - 长度: 5 个元素
  - 0 表示使能，非 0 表示禁用

**返回值**
无

**使用示例**
```cpp
// 仅 L25 支持
if (hand.handModel_ == LINKER_HAND::L25) {
    std::vector<uint8_t> disable = {1, 1, 1, 1, 1}; // 禁用所有手指
    hand.setDisable(disable);
}
```

**注意事项**
- 仅支持 L25 型号
- 默认参数为全 1（禁用状态）

---

### clearFaultCode

```cpp
void clearFaultCode(const std::vector<uint8_t> &torque = std::vector<uint8_t>(5, 1));
```

**功能描述**
清除电机的故障码。**目前仅支持 L20 型号。**

**参数说明**
- `torque` (std::vector<uint8_t>): 扭矩数组（可选，默认全 1）
  - 长度: 5 个元素

**返回值**
无

**使用示例**
```cpp
// 仅 L20 支持
if (hand.handModel_ == LINKER_HAND::L20) {
    hand.clearFaultCode();
}
```

**注意事项**
- 仅支持 L20 型号
- 清除故障码前，请确保故障原因已解决

---

## 使用示例

### 完整示例：基本控制流程

```cpp
#include "LinkerHandApi.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    // 1. 初始化 API
    LinkerHandApi hand(LINKER_HAND::L10, HAND_TYPE::RIGHT);

    // 2. 获取版本信息
    std::cout << "SDK Version: " << hand.getVersion() << std::endl;

    // 3. 设置速度和扭矩
    std::vector<uint8_t> speed = {200, 200, 200, 200, 200};
    std::vector<uint8_t> torque = {255, 255, 255, 255, 255};
    hand.setSpeed(speed);
    hand.setTorque(torque);

    // 4. 握拳动作
    std::vector<uint8_t> fist_pose = {101, 60, 0, 0, 0, 0, 255, 255, 255, 51};
    hand.fingerMove(fist_pose);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 5. 张开动作
    std::vector<uint8_t> open_pose = {255, 104, 255, 255, 255, 255, 255, 255, 255, 71};
    hand.fingerMove(open_pose);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 6. 获取状态信息
    auto state = hand.getState();
    std::cout << "当前关节状态: ";
    for (auto s : state) {
        std::cout << static_cast<int>(s) << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

### 示例：使用弧度制控制（L10/L20/L21/L25）

```cpp
#include "LinkerHandApi.h"

int main() {
    LinkerHandApi hand(LINKER_HAND::L10, HAND_TYPE::RIGHT);

    // 使用弧度制控制关节
    std::vector<double> pose_arc = {0.5, 0.3, 0.2, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    hand.fingerMoveArc(pose_arc);

    // 获取弧度制关节状态
    auto state_arc = hand.getStateArc();
    for (size_t i = 0; i < state_arc.size(); i++) {
        std::cout << "关节 " << i << ": " << state_arc[i] << " 弧度" << std::endl;
    }

    return 0;
}
```

### 示例：读取传感器数据

```cpp
#include "LinkerHandApi.h"
#include <iostream>

int main() {
    LinkerHandApi hand(LINKER_HAND::L10, HAND_TYPE::RIGHT);

    // 获取压力数据
    auto force_data = hand.getForce();

    // 获取电机温度
    auto temperature = hand.getTemperature();

    // 获取故障码
    auto fault_code = hand.getFaultCode();

    // 获取电流
    auto current = hand.getCurrent();

    // 处理数据...

    return 0;
}
```

---

## 注意事项

1. **初始化顺序**
   - 建议在使用 API 前先设置速度和扭矩
   - 确保通信接口已正确配置

2. **参数范围**
   - `fingerMove()` 的参数范围是 0-255
   - `fingerMoveArc()` 的参数范围由 URDF 限位定义
   - 速度和扭矩的范围是 0-255

3. **型号差异**
   - 不同型号支持的关节数量不同
   - 部分功能仅特定型号支持（如 `setCurrent()` 仅 L20 支持）
   - 请根据实际使用的型号调整代码

4. **通信协议**
   - 默认使用 CAN0，可通过构造函数参数修改
   - 使用 CAN 总线前，确保总线已正确配置和启动
   - 使用 ModBus 前，确保串口或网络连接已建立
   - 使用 EtherCAT 前，确保 EtherCAT 主站已正确配置

5. **错误处理**
   - 建议定期检查故障码
   - 出现故障时及时停止操作
   - 温度过高时应暂停使用

6. **性能考虑**
   - 动作执行需要时间，建议添加适当的延时
   - 频繁调用 API 可能影响性能
   - 建议在循环中适当添加延时

---

## 版本兼容性

### 支持的型号
- ✅ O6
- ✅ L6
- ✅ L7
- ✅ L10
- ✅ L20
- ✅ L21
- ✅ L25

### 功能支持矩阵

| 功能 | O6 | L6 | L7 | L10 | L20 | L21 | L25 |
|------|----|----|----|-----|-----|-----|-----|
| `fingerMove()` | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| `fingerMoveArc()` | ❌ | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ |
| `setCurrent()` | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ |
| `setEnable()` / `setDisable()` | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ |
| `clearFaultCode()` | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ |

### 通信协议支持
- ✅ CAN (CAN0, CAN1)
- ✅ ModBus
- ✅ EtherCAT

---

## 获取帮助

- **官方网站**: [https://linkerbot.cn](https://linkerbot.cn)
- **技术支持**: [https://linkerbot.cn/aboutUs](https://linkerbot.cn/aboutUs)
- **GitHub**: [https://github.com/linker-bot/linkerhand-cpp-sdk](https://github.com/linker-bot/linkerhand-cpp-sdk)
- **故障排查**: 请参考 [故障排查指南](TROUBLESHOOTING.md)
- **常见问题**: 请参考 [常见问题解答](FAQ.md)
