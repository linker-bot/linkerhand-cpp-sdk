# 故障排查指南

本文档覆盖发布包消费、源码构建、运行示例和通信联调中的常见问题。

## 编译与配置

### 1. `cmake ..` 失败或提示缺少依赖

先确认基础工具链和依赖：

```bash
cmake --version
g++ --version
pkg-config --modversion libusb-1.0
```

如果构建网页示教器，还要确认 `Boost::system` 可用。只想先验证 SDK 本体时，可显式关闭示例和示教器：

```bash
cmake -S linkerhand -B linkerhand/build -DBUILD_EXAMPLES=OFF -DBUILD_PENDANT=OFF
cmake --build linkerhand/build -j
```

### 2. 找不到 `LinkerHandApi.h` 或 `CommFactory.h`（旧名 `CanBusFactory.h`）

发布包消费和 install 消费的 include 路径不同：

- 发布包：直接加 `include/`、`include/api`、`include/core`、`include/communication`
- install 后：加 `<prefix>/include/linkerhand-cpp-sdk` 及其三个子目录

若直接用发布包根目录 `CMakeLists.txt` 构建 examples，无需手动补这些路径。

### 3. 链接时报 `undefined reference`

优先检查三件事：

```bash
ldd lib/linux/x86_64/linkerhand_cpp_sdk.so
file lib/linux/x86_64/linkerhand_cpp_sdk.so
uname -m
```

- 架构必须匹配
- Linux 侧还需要 `pthread`
- 如果手动链接，确认没有把 MinGW、MSVC、Linux 三种产物混用

### 4. Windows 下找不到 `PCANBasic.dll`

确认以下文件在运行时可被找到：

- `linkerhand_cpp_sdk.dll`
- `PCANBasic.dll`

最稳妥的方式是把它们复制到最终 `.exe` 同目录。

## 运行与设备联调

### 5. 示例程序启动后无响应

优先排查环境而不是盲改代码：

```bash
ip link show can0
```

确认：

- 机械手已上电
- 设备名与示例中的总线名一致，例如 `can0`
- 波特率与现场配置一致，常见为 `1000000`

若需要重新配置 CAN：

```bash
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 1000000
sudo ip link set can0 up
```

### 6. CAN-FD 相关构建或运行异常

O20 的 CAN-FD 主要面向 Linux `x86_64`。在 ARM + Ubuntu 18 及以下环境，顶层 CMake 会自动禁用 CAN-FD。先看配置日志里是否出现 `CANFD is disabled` 提示，再决定是换平台还是调整目标。

### 7. Modbus 无法通信

先确认不是代码问题：

- 串口设备名是否正确，例如 `/dev/ttyUSB0`
- 当前用户是否有串口权限
- 从站地址、波特率、校验位是否与设备一致

发布包中的 Modbus 示例可作为最小回归入口，先让示例通，再接自己的业务程序。

### 8. EtherCAT 相关构建失败

`USE_ETHERCAT` 默认关闭。只有在现场确实需要时再打开：

```bash
cmake -S linkerhand -B linkerhand/build -DUSE_ETHERCAT=ON
```

打开后仍失败，先检查系统是否已安装 `libethercat` 和对应 `pkg-config` 信息，而不是直接修改源码。

## 打包与发布

### 9. `./pack.sh pack` 后发布目录缺文件

先检查脚本自检输出。当前打包会显式校验：

- `README.md`
- `docs/API-Reference.md`
- `docs/FAQ.md`
- `docs/TROUBLESHOOTING.md`
- `examples/CMakeLists.txt`
- `hand_teach_pendant/CMakeLists.txt`
- 各平台 `lib/` 目录是否生成库文件

如果缺的是某个平台库，先看对应 Docker 编译日志，而不是手动往发布目录补文件。

### 10. 发布包能直接代表 install 结果吗？

不能完全等同。发布包面向“解压即用”，会同时携带示例、示教器和通信层头文件；install 更偏系统集成与 `find_package` 消费。两者都来自同一主工作区，但用途不同。

### 11. 为什么不再从旧版参考目录复制文档？

旧版目录长期手工维护，很多内容与当前仓库状态不再一致。现在的发布文档都应从 `linkerhand/docs/` 输出，由 `pack.sh` 直接复制，避免一边改代码、一边忘记同步旧目录。

## 排查原则

- 连续编译或运行失败两次以上，先回看日志和环境，不继续盲试。
- 先检查平台、依赖、设备名、库路径，再怀疑源码逻辑。
- 对外发布只以 `linkerhand/` 当前内容和 `pack.sh` 生成物为准。
