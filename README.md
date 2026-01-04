# LinkerHand-CPP-SDK

## 概述

LinkerHand-CPP-SDK 是由灵心巧手（北京）科技有限公司开发，用于 O6、L6、L7、L10、L20、L21、L25 型号灵巧手的驱动软件和 Demo 示例。

## 安装

### ubuntu

- 下载 SDK

```bash
git clone https://github.com/linker-bot/linkerhand-cpp-sdk.git
```

- 启动脚本
```bash
cd linker_hand_cpp_sdk/linker_hand
./script.sh
```
![alt text](linkerhand/img/script.png)
- 运行示例

```bash
cd build
./toolset_example
```

![alt text](linkerhand/img/example.png) 

## 快速开始

- 创建 main.cpp 文件，并添加以下代码：

```cpp
// main.cpp
#include "LinkerHandApi.h"

int main() {

    // 调用API接口
    LinkerHandApi hand(LINKER_HAND::L10, HAND_TYPE::RIGHT);

    // 获取版本信息
    std::cout << hand.getVersion() << std::endl;

    // 握拳
    std::vector<uint8_t> fist_pose = {101, 60, 0, 0, 0, 0, 255, 255, 255, 51};
    hand.fingerMove(fist_pose);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 张开
    std::vector<uint8_t> open_pose = {255, 104, 255, 255, 255, 255, 255, 255, 255, 71};
    hand.fingerMove(open_pose);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}
```

- 创建 CMakeLists.txt 文件，并添加以下配置：

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.5)
project(MyProject)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(LIB_SUBDIR "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(LIB_SUBDIR "aarch64")
else()
    message(WARNING "Unknown architecture, defaulting to x86_64")
    set(LIB_SUBDIR "x86_64")
endif()

#-----------------------------------------------------------------------------
# LINKER_HAND_CPP_SDK
#-----------------------------------------------------------------------------
find_library(LINKER_HAND_LIB
    NAMES linkerhand_cpp_sdk linkerhand_cpp
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib/${LIB_SUBDIR}
        /usr/local/lib/linkerhand-cpp-sdk/${LIB_SUBDIR}
        /usr/lib/linkerhand-cpp-sdk/${LIB_SUBDIR}
        ${CMAKE_INSTALL_PREFIX}/lib/linkerhand-cpp-sdk/${LIB_SUBDIR}
    NO_DEFAULT_PATH
)

set(LINKER_HAND_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    /usr/local/include/linkerhand-cpp-sdk
    /usr/include/linkerhand-cpp-sdk
    ${CMAKE_INSTALL_PREFIX}/include/linkerhand-cpp-sdk
)

if(NOT LINKER_HAND_LIB)
    message(FATAL_ERROR "linkerhand_cpp_sdk library not found!")
endif()

if(NOT LINKER_HAND_INCLUDE_DIR)
    message(FATAL_ERROR "LinkerHand headers not found!")
endif()

message(STATUS "Found linkerhand_cpp_sdk library: ${LINKER_HAND_LIB}")
message(STATUS "Found LinkerHand headers: ${LINKER_HAND_INCLUDE_DIR}")

#-----------------------------------------------------------------------------
# INCLUDE_DIRECTORIES
#-----------------------------------------------------------------------------
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LINKER_HAND_INCLUDE_DIR}
)

#-----------------------------------------------------------------------------
# EXECUTABLE
#-----------------------------------------------------------------------------
add_executable(my_project main.cpp)
target_link_libraries(my_project ${LINKER_HAND_LIB} pthread)

  ```

- 文件结构
```
├── example
│   ├── CMakeLists.txt
│   └── main.cpp
```
- 编译
```bash
cd example
mkdir build
cd build
cmake ..
make
```
- 运行

```bash
./my_project
```

- position 与手指关节对照表

```
L6/O6: ["大拇指弯曲", "大拇指横摆", "食指弯曲", "中指弯曲", "无名指弯曲", "小拇指弯曲"]

L7:  ["大拇指弯曲", "大拇指横摆","食指弯曲", "中指弯曲", "无名指弯曲","小拇指弯曲","拇指旋转"]

L10: ["拇指根部", "拇指侧摆","食指根部", "中指根部", "无名指根部","小指根部","食指侧摆","无名指侧摆","小指侧摆","拇指旋转"]

L20: ["拇指根部", "食指根部", "中指根部", "无名指根部","小指根部","拇指侧摆","食指侧摆","中指侧摆","无名指侧摆","小指侧摆","拇指横摆","预留","预留","预留","预留","拇指尖部","食指末端","中指末端","无名指末端","小指末端"]

L21: ["大拇指根部", "食指根部", "中指根部","无名指根部","小拇指根部","大拇指侧摆","食指侧摆","中指侧摆","无名指侧摆","小拇指侧摆","大拇指横滚","预留","预留","预留","预留","大拇指中部","预留","预留","预留","预留","大拇指指尖","食指指尖","中指指尖","无名指指尖","小拇指指尖"]

L25: ["大拇指根部", "食指根部", "中指根部","无名指根部","小拇指根部","大拇指侧摆","食指侧摆","中指侧摆","无名指侧摆","小拇指侧摆","大拇指横滚","预留","预留","预留","预留","大拇指中部","食指中部","中指中部","无名指中部","小拇指中部","大拇指指尖","食指指尖","中指指尖","无名指指尖","小拇指指尖"]
```

## 示例

| 序号 | 文件名称              | 描述                                                  |
| :--- | :-------------------- | :---------------------------------------------------- |
| 1    | examples              | 示例集合（支持 O6/L6、L7、L10、L20、L21、L25 灵巧手） |
| 2    | action_group_show_l10 | 灵巧手 L10 手指舞                                     |

## API 文档

- [C++ API 文档](linker_hand/docs/API-Reference.md)

## 版本更新
