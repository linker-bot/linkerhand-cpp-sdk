# 常见问题解答

本文档面向从 repository root 源码构建、安装并接入 LinkerHand C++ SDK 的开发者。

## 安装与接入

### Q1: 源码构建后怎么快速验证？

Linux/macOS:

```bash
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
./bin/test_l10_can_0
```

Windows MinGW:

```cmd
mkdir build
cd build
cmake -G "MinGW Makefiles" -DCMAKE_MAKE_PROGRAM=mingw32-make ..
mingw32-make -j
build\bin\test_l10_can_0.exe
```

### Q2: 对外头文件为什么比最小 API 入口多？

SDK 不仅面向统一 API，也要支持 `examples/`、`hand_teach_pendant/` 和部分需要直连通信层的客户工程，所以会一起释放 `include/communication/` 下的通信相关头。新项目优先从 `LinkerHandApi.h` 开始接入。

### Q3: 安装后 `find_package` 应该怎么接？

从源码执行 `cmake --install` 或 `./build.sh -i` 后，可直接在下游工程里写：

```cmake
find_package(linkerhand-cpp-sdk CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE LinkerHand::linkerhand_cpp_sdk)
```

## 构建与运行

### Q4: `build.sh` / `build.bat` 能做什么？

它们服务于持有源码工程的开发者，用于重建、安装或卸载 SDK。日常一站式构建见 `./build.sh -b`，安装见 `sudo ./build.sh -i`。

### Q5: `--skip-tests` 为什么没有关闭 examples？

当前 `build.sh` 透传的是 `-DBUILD_TESTS=OFF`，这是 examples 子目录内部开关，不影响顶层 `BUILD_EXAMPLES` / `BUILD_PENDANT`。如果只想构建 SDK，请直接使用：

```bash
cmake -S . -B build -DBUILD_EXAMPLES=OFF -DBUILD_PENDANT=OFF
cmake --build build -j
```

### Q6: 哪些平台是当前主要验证目标？

- Linux `x86_64`
- Linux `aarch64`
- Windows `x64`（MinGW / MSVC 导入库）

其中 O20 的 CAN-FD 示例目前主要面向 Linux `x86_64`。

## 通信与接口

### Q7: CAN、Modbus、EtherCAT 怎么选？

- 统一 API 层通过 `COMM_TYPE::CAN`、`COMM_TYPE::MODBUS`、`COMM_TYPE::ETHERCAT` 选择通信方式。
- 不同手型支持范围不同，具体以 `README.md` 型号表和现有 examples 为准。
- 需要自己管理底层总线时，优先参考 `examples/test_*` 和 `hand_teach_pendant/src/HandController.cpp` 的实际用法。

### Q8: Linux 下 CAN 默认怎么配？

```bash
sudo modprobe can
sudo modprobe can_raw
sudo ip link set can0 type can bitrate 1000000
sudo ip link set can0 up
ip link show can0
```

如果示例使用了 `can1` 或其他设备名，请按现场总线名替换。

### Q9: Windows 下还需要额外 DLL 吗？

需要。除了 SDK 自身 DLL 外，还要确保 `PCANBasic.dll`（来自 `third_party/PCAN_Basic/`）与可执行文件同目录，或能被系统搜索路径找到。

### Q10: O20 的 CAN-FD 双手 / 手型怎么用？

O20 的 CAN-FD 走这样一条链路：

```
LinkerHandApi(O20, HAND_TYPE::LEFT|RIGHT, COMM_TYPE::CAN)
   └─ IHand (O20Hand)  ── 组 29-bit 扩展帧 ID（含 device_id: 0x01=右手 / 0x02=左手）
       └─ tx_callback ── 用户自建 Communication::CanFD(dev_num, ch_num) 只负责收发字节
```

- `Communication::CanFD` **不知道左右手**，构造参数只有物理适配器编号 `dev_num` 和通道号 `ch_num`。
- 左右手由 `LinkerHandApi` 构造函数的 `HAND_TYPE` 决定；`O20Hand` 内部把它编码到帧 ID 里。
- **一根 CAN-FD 适配器对应一只手**——双手要用两根适配器（先插的编号 0，后插的编号 1），各自建独立的 `CanFD` + `LinkerHandApi`。

单手最小样板（参考 `examples/test_o20_canfd_0.cpp`）：

```cpp
Communication::CanFD canfd(0, 0);
canfd.init();

LinkerHandApi api(LINKER_HAND::O20, HAND_TYPE::LEFT, COMM_TYPE::CAN);
api.setCanTxCallback([&](uint32_t can_id, const uint8_t* data, uintptr_t len) -> int32_t {
    canfd.send(std::vector<uint8_t>(data, data + len), can_id, /*extended=*/true);
    return 0;
});
api.setCanRxCallback([&](uint32_t* id_out, uint8_t* data_out, uint8_t* len_out) -> int32_t {
    auto frame = canfd.recv(10);
    if (!frame.valid) return -1;
    *id_out = frame.can_id;
    *len_out = Communication::CanFD::dlcToLen(frame.can_dlc);
    memcpy(data_out, frame.data, *len_out);
    return 0;
});
// 后续调用 api.getVersion() / setPosition() / getSpeed() ...
```

双手样板见 `examples/test_o20_canfd_double.cpp`——两只 `Communication::CanFD`（`dev_num=0/1`）+ 两只 `LinkerHandApi`（`LEFT/RIGHT`），tx/rx lambda **必须闭包捕获各自的 CanFD 实例**，切勿共用。

### Q11: 传错 HAND_TYPE 会怎么样？

`O20Hand` 在挂上 `setCanRxCallback` 时会主动发一次 `SYS_DEVICE_INFO=0x00` 探测——先按用户传入的 `HAND_TYPE` 对应的 `device_id` 探一次，超时（300ms）会自动换另一侧再探一次。命中后：

- **一致**：`stdout` 打印 `[LinkerHand O20] 手型探测: 固件自报 LEFT (hand_flag=2)，与用户传入一致。`
- **不一致**：**静默按固件自报值校正** `device_id`，并打印 `[LinkerHand O20] ⚠ 手型自动校正: 用户传入 HAND_TYPE::RIGHT ... 与固件自报 LEFT ... 不一致，已按固件校正 device_id -> 0x02。`
- **两侧都超时**：保留原 `device_id`，打印警告并提示排查供电 / 接线 / 波特率。

这个机制是为了兼容"用户构造时把左右手传反"或"不确定当前 CAN-FD 适配器上接的是哪只手"的场景。**推荐仍按实际情况正确传入 `HAND_TYPE`**，自动校正只作为兜底。

### Q12: `CanFD` 和 `CanFDSocket` 有什么区别？该用哪个？

SDK 提供两种 CAN-FD 后端，接口一致（都实现 `Communication::ICanFD`，`send` / `recv` / `isOpen` 用法相同），底层链路不同：

| | `Communication::CanFD` | `Communication::CanFDSocket` |
|---|---|---|
| 底层 | 厂商私有驱动 `third_party/libcanbus` | Linux 内核原生 SocketCAN |
| 构造 | `CanFD(dev_num, ch_num)` 扫 USB-CANFD 适配器 | `CanFDSocket("can0")` 打开内核接口 |
| 波特率 | 代码内配置（仲裁 1M / 数据 5M） | **内核侧** `ip link ... fd on` 配置 |
| 平台 | 跨平台；ARM/Ubuntu≤18 被 `USE_CANFD` 关闭 | **仅 Linux**；不受 `USE_CANFD` 门控，ARM 也可用 |
| 依赖 | 需 `libcanbus` | 只用内核 `linux/can.h`，零 third_party |

用 `CanFDSocket` 前先在内核侧把接口配成 CAN-FD 模式：

```bash
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 1000000 dbitrate 5000000 fd on restart-ms 100
sudo ip link set can0 up
sudo ip link set can0 txqueuelen 1000
```

回调接线与 `CanFD` 几乎一致，仅 `recv()` 返回的 `CanFDFrame.can_dlc` **直接是实际字节长度**（SocketCAN 用真实长度，不是 DLC 编码，无需 `dlcToLen`）：

```cpp
Communication::CanFDSocket canfd("can0");
canfd.init();
// tx: canfd.send(data_vec, can_id, true);
// rx: len = frame.can_dlc; memcpy(out, frame.data, len);
```

单手样板见 `examples/test_o20_canfd_socket_0.cpp`。手型逻辑（O20）与传输层完全解耦，换后端只需改这一处构造 + 回调，`LinkerHandApi` 用法不变。

## 文档与支持

### Q13: 先看哪个文档最合适？

- 想了解总体接入方式：看 `README.md`
- 想确认接口签名：看 `docs/API-Reference.md`
- 遇到构建或运行问题：看 `docs/TROUBLESHOOTING.md`

### Q14: 旧版参考目录还能直接当文档源吗？

不能。旧版目录只适合做历史对照，内容可能与当前命名空间、构建开关不一致。请以 repository root 的源码与文档为准。
