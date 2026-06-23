# 常见问题解答

本文档面向 `linkerhand-cpp-sdk/` 发布包，以及从 repository root 源码构建安装 SDK 的开发者。

## 通信与接口

### Q1: CAN、Modbus、EtherCAT 怎么选？

- 统一 API 层通过 `COMM_TYPE::CAN`、`COMM_TYPE::MODBUS`、`COMM_TYPE::ETHERCAT` 选择通信方式。
- 不同手型支持范围不同，具体以 `README.md` 型号表和现有 examples 为准。
- 需要自己管理底层总线时，优先参考 `examples/test_*` 和 `hand_teach_pendant/src/HandController.cpp` 的实际用法。

### Q2: Linux 下 CAN 默认怎么配？

```bash
sudo modprobe can
sudo modprobe can_raw
sudo ip link set can0 type can bitrate 1000000
sudo ip link set can0 up
ip link show can0
```

如果示例使用了 `can1` 或其他设备名，请按现场总线名替换。

### Q3: Windows 下还需要额外 DLL 吗？

需要。发布包已经带上 `third_party/PCAN_Basic/` 和对应导入库；客户工程除了 SDK 自身 DLL 外，还要确保 `PCANBasic.dll` 与可执行文件同目录，或能被系统搜索路径找到。

## 文档与支持

### Q4: 先看哪个文档最合适？

- 想了解总体接入方式：看 `README.md`
- 想确认接口签名：看 `docs/API-Reference.md`
- 遇到构建或运行问题：看 `docs/TROUBLESHOOTING.md`
