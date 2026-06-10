
---

# Linker Hand C++ API Documentation

## API Overview

This document provides a detailed overview of the C++ API for the Linker Hand, including functions for controlling the hand's movements, retrieving sensor data, and setting operational parameters.

## 构造函数
```cpp
LinkerHandApi(const LINKER_HAND &handJoint, const HAND_TYPE &handType, const COMM_TYPE commType = COMM_TYPE::CAN);
```
描述
LinkerHandApi 类的构造函数，用于初始化 Linker 机械手 API。

参数
- handJoint: 机械手型号，可选值 `LINKER_HAND::{L6, L7, L10, L20, L21, L25, O6, G20, O20}`。
- handType: 左手或右手（`HAND_TYPE::LEFT` 或 `HAND_TYPE::RIGHT`）。
- commType: 通信协议类型，可选值 `COMM_TYPE::{CAN, MODBUS, ETHERCAT}`，默认为 `CAN`。CAN-FD 走 `COMM_TYPE::CAN`，由 `Communication::CommFactory` 在底层选择 `CanFD` 实现。

---

## 通信回调设置

为了适配不同的底层驱动（如不同的 CAN 卡或 Modbus 库），API 提供了回调机制。

| 函数 | 参数类型 | 描述 |
| --- | --- | --- |
| `setCanTxCallback` | `CanTxCallback` | 设置 CAN 发送回调函数 |
| `setCanRxCallback` | `CanRxCallback` | 设置 CAN 接收回调函数 |
| `setModbusTxCallback` | `ModbusTxCallback` | 设置 Modbus 发送回调函数 |
| `setModbusRxCallback` | `ModbusRxCallback` | 设置 Modbus 接收回调函数 |
| `freeCanCallback` | 无 | 释放/重置 CAN 相关回调 |
| `freeModbusCallback` | 无 | 释放/重置 Modbus 相关回调 |

---

## 成员函数

### 设置关节位置
```cpp
void fingerMove(const std::vector<uint8_t> &pose);
```
**Description**:  
设置关节的目标位置，用于控制手指的运动。  
**Parameters**:  
- `pose`: 一个包含目标位置数据长度为（O6/L6=6、L7=7、L10=10、L20=20、G20=16、L21=21、L25=25）
的 `std::vector<uint8_t>`数组，范围是0～255。
```
  O6/L6:  ["拇指根部", "拇指侧摆", "食指根部", "中指根部", "无名指根部", "小指根部"]
  
  L7:  ["大拇指弯曲", "大拇指横摆","食指弯曲", "中指弯曲", "无名指弯曲","小拇指弯曲","拇指旋转"]

  L10: ["拇指根部", "拇指侧摆","食指根部", "中指根部", "无名指根部","小指根部","食指侧摆","无名指侧摆","小指侧摆","拇指旋转"]

  L20: ["拇指根部", "食指根部", "中指根部", "无名指根部","小指根部","拇指侧摆","食指侧摆","中指侧摆","无名指侧摆","小指侧摆","拇指横摆","预留","预留","预留","预留","拇指尖部","食指末端","中指末端","无名指末端","小指末端"]
  
  L21: ["大拇指根部", "食指根部", "中指根部","无名指根部","小拇指根部","大拇指侧摆","食指侧摆","中指侧摆","无名指侧摆","小拇指侧摆","大拇指横滚","预留","预留","预留","预留","大拇指中部","预留","预留","预留","预留","大拇指指尖","食指指尖","中指指尖","无名指指尖","小拇指指尖"]

  G20: ["大拇指根部", "食指根部", "中指根部","无名指根部","小拇指根部","大拇指侧摆","食指侧摆","中指侧摆","无名指侧摆","小拇指侧摆","大拇指横滚","大拇指指尖","食指指尖","中指指尖","无名指指尖","小拇指指尖"]

  L25: ["大拇指根部", "食指根部", "中指根部","无名指根部","小拇指根部","大拇指侧摆","食指侧摆","中指侧摆","无名指侧摆","小拇指侧摆","大拇指横滚","预留","预留","预留","预留","大拇指中部","食指中部","中指中部","无名指中部","小拇指中部","大拇指指尖","食指指尖","中指指尖","无名指指尖","小拇指指尖"]
```
---

### 设置关节位置
```cpp
void fingerMoveArc(const std::vector<double> &pose);
```
**Description**:  
设置关节的目标位置，用于控制手指的运动。  
**Parameters**:  
- `pose`: 一个包含目标位置数据长度为（O6/L6=6、L7=7、L10=10、L20=20、G20=16、L21=21、L25=25）
的 `std::vector<double>`数组，范围为urdf限位。

---

### 设置速度
```cpp
void setSpeed(const std::vector<uint8_t> &speed);
```
**Description**:  
设置手部的运动速度。  
**Parameters**:  
- `speed`: 一个包含速度数据的 `std::vector<uint8_t>`，长度为5个元素对应每手指的速度值。

---

### 设置扭矩
```cpp
void setTorque(const std::vector<uint8_t> &torque);
```
**Description**:  
设置手部的扭矩。  
**Parameters**:  
- `torque`: 一个包含扭矩数据的 `std::vector<uint8_t>`，长度为5个元素对应手指的扭矩值。

---

### 获取法向压力、切向压力、切向方向、接近感应
```cpp
std::vector<std::vector<std::vector<uint8_t>>> getForce();
```
**Description**:  
获取手部传感器的综合数据，包括法向压力、切向压力、切向方向和接近感应。  
**Returns**:  
- 返回一个三维向量：第一维为传感器类型，第二维为手指，第三维为该手指对应的数据。

---

### 获取速度
```cpp
std::vector<uint8_t> getSpeed();
```
**Description**:  
获取当前设置的速度值。  
**Returns**:  
- 返回一个 `std::vector<uint8_t>`，包含当前的速度设置值。

---

### 获取当前关节状态
```cpp
std::vector<uint8_t> getState();
```
**Description**:  
获取当前关节的状态信息。  
**Returns**:  
- 返回一个 `std::vector<uint8_t>`，包含当前关节的状态数据。

---

### 获取当前关节状态
```cpp
std::vector<double> getStateArc();
```
**Description**:  
获取当前关节的状态信息。  
**Returns**:  
- 返回一个 `std::vector<double>`，包含当前关节的状态数据。

---

### 获取大拇指、食指、中指、无名指、小指的所有压力数据

压感数据统一通过 `getForce()` 获取，返回三维向量 `[传感器类型][手指][数据]`，传感器类型依次为法向压力、切向压力、切向方向、接近感应。`LinkerHandApi` 不再提供 `getPressure()` 接口。

---

### 获取版本号
```cpp
std::string getVersion();
```
**Description**:  
获取当前软件或硬件的版本号。  
**Returns**:  
- 返回一个字符串，表示当前的版本号。

---

### 获取电机温度
```cpp
std::vector<uint8_t> getTemperature();
```
**Description**:  
获取电机的温度数据。  
**Returns**:  
- 返回一个 `std::vector<uint8_t>`，包含电机的温度数据。

---

### 获取电机故障码
```cpp
std::vector<uint8_t> getFaultCode();
```
**Description**:  
获取电机的故障码。  
**Returns**:  
- 返回一个 `std::vector<uint8_t>`，包含电机的故障码。

---

### 获取当前最大扭矩
```cpp
std::vector<uint8_t> getTorque();
```
**Description**:  
获取电机的当前最大扭矩值。  
**Returns**:  
- 返回一个 `std::vector<uint8_t>`，包含电机的当前最大扭矩值。

---

### 设置电机使能（目前仅支持 L25）
```cpp
void setEnable(const std::vector<uint8_t> &enable);
```
**Description**:  
使能电机。  
**Parameters**:  
- `enable`: 一个包含使能数据的 `std::vector<uint8_t>`，每个元素对应一个电机的使能状态。

---

### 设置电机禁用（目前仅支持 L25）
```cpp
void setDisable(const std::vector<uint8_t> &disable);
```
**Description**:  
禁用电机。  
**Parameters**:  
- `disable`: 一个包含禁用数据的 `std::vector<uint8_t>`，每个元素对应一个电机的禁用状态。

---

### 清除电机故障码（目前仅支持 L20）
```cpp
void clearFaultCode();
```
**Description**:  
清除电机的故障码。

---

## Example Usage

以下是一个完整的示例代码，展示如何使用上述 API：

```cpp
#include "LinkerHandApi.h"

int main() {

    // 初始化灵巧手
    LinkerHandApi hand(LINKER_HAND::L10, HAND_TYPE::LEFT, COMM_TYPE::CAN);
    
    // 初始化CAN设备/自动搜索CAN设备（CommFactory 旧名 CanBusFactory 兼容保留）
    std::shared_ptr<Communication::ICanBus> bus = Communication::CommFactory::createCanBus("can0", 1000000);
    // std::shared_ptr<Communication::ICanBus> bus = Communication::CommFactory::createCanBus(HAND_TYPE::LEFT);
    
    // 设置CAN发送回调函数
    hand.setCanTxCallback([bus](uint32_t can_id, const uint8_t *data, uintptr_t data_len) -> int32_t {
        std::vector<uint8_t> data_vec(data, data + data_len);
        bus->send(data_vec, can_id);
        return 0;
    });

    // 设置CAN接收回调函数
    hand.setCanRxCallback([bus](uint32_t* can_id_out, uint8_t* data_out, uint8_t* len_out) -> int32_t {
        try {
            auto frame = bus->recv();
            if (frame.can_id == 0 && frame.can_dlc == 0) {
                return -1;
            }
            *can_id_out = frame.can_id;
            *len_out = frame.can_dlc;
            memcpy(data_out, frame.data, frame.can_dlc);
            return 0;
        } catch (const std::exception& e) {
            std::cerr << "CAN接收错误: " << e.what() << std::endl;
            return -1;
        }
    });
    std::cout << "-------------------------------------------" << std::endl;

    std::cout << "获取版本信息：" << std::endl;
    std::cout << hand.getVersion() << std::endl;
    std::cout << "-------------------------------------------" << std::endl;

    std::cout << "执行动作：握拳" << std::endl;
    std::vector<uint8_t> fist_pose = {120, 60, 0, 0, 0, 0, 255, 255, 255, 51};
    hand.fingerMove(fist_pose);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "-------------------------------------------" << std::endl;

    std::cout << "执行动作：张开" << std::endl;
    std::vector<uint8_t> open_pose = {255, 104, 255, 255, 255, 255, 255, 255, 255, 71};
    hand.fingerMove(open_pose);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "-------------------------------------------" << std::endl;
    
    std::cout << "程序结束！" << std::endl;
    return 0;
}

```

---

## Notes
- 在使用 API 之前，请确保手部设备已正确连接并初始化。
- 参数值（如速度、扭矩等）的具体范围和含义请参考设备的技术手册。

---

## Contact
- 如果有任何问题或需要进一步支持，请联系 [https://linkerbot.cn/aboutUs](https://linkerbot.cn/aboutUs)。

---
