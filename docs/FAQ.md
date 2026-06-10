# 常见问题解答

本文档面向 `pack.sh` 生成的 `linkerhand-cpp-sdk/` 发布包，以及从 `linkerhand/` 源码构建安装 SDK 的开发者。

## 安装与发布包

### Q1: 发布包和源码仓库是什么关系？

`linkerhand/` 是唯一主开发工作区，`./pack.sh pack` 会基于它生成可直接发布的 `linkerhand-cpp-sdk/`。发布包内包含预编译库、示例、示教器、文档和必要的第三方依赖。

### Q2: 发布包解压后怎么快速验证？

Linux/macOS:

```bash
cd linkerhand-cpp-sdk
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
./bin/test_l10_can_0
```

Windows MinGW:

```cmd
cd linkerhand-cpp-sdk
mkdir build
cd build
cmake -G "MinGW Makefiles" -DCMAKE_MAKE_PROGRAM=mingw32-make ..
mingw32-make -j
build\bin\test_l10_can_0.exe
```

### Q3: 发布包里的头文件为什么比最小 API 入口多？

发布包不仅面向统一 API，也要支持 `examples/`、`hand_teach_pendant/` 和部分需要直连通信层的客户工程，所以会一起发布 `include/communication/` 下的通信相关头。新项目优先从 `LinkerHandApi.h` 开始接入。

### Q4: 安装后 `find_package` 应该怎么接？

从源码执行 `cmake --install` 或 `./build.sh -i` 后，可直接在下游工程里写：

```cmake
find_package(linkerhand-cpp-sdk CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE LinkerHand::linkerhand_cpp_sdk)
```

这条路径适合系统安装后的二次开发；如果只是消费发布包，直接用发布包根目录的 `CMakeLists.txt` 构建 examples 或参考 `README.md` 手动链接即可。

## 构建与运行

### Q5: 发布包里的 `build.sh` / `build.bat` 能做什么？

它们主要服务于持有源码工程的开发者，用于重建、安装或卸载 SDK。客户从发布包消费预编译库时，通常不需要执行 `-b` / `-i`，直接用 `cmake ..` 构建 examples 即可。

### Q6: `--skip-tests` 为什么没有关闭 examples？

当前 `build.sh` 透传的是 `-DBUILD_TESTS=OFF`，这是 examples 子目录内部开关，不影响顶层 `BUILD_EXAMPLES` / `BUILD_PENDANT`。如果只想构建 SDK，请直接使用：

```bash
cmake -S linkerhand -B linkerhand/build -DBUILD_EXAMPLES=OFF -DBUILD_PENDANT=OFF
cmake --build linkerhand/build -j
```

### Q7: 哪些平台是当前主要验证目标？

- Linux `x86_64`
- Linux `aarch64`
- Windows `x64`（MinGW / MSVC 导入库）

其中 O20 的 CAN-FD 示例目前主要面向 Linux `x86_64`。

## 通信与接口

### Q8: CAN、Modbus、EtherCAT 怎么选？

- 统一 API 层通过 `COMM_TYPE::CAN`、`COMM_TYPE::MODBUS`、`COMM_TYPE::ETHERCAT` 选择通信方式。
- 不同手型支持范围不同，具体以 `README.md` 型号表和现有 examples 为准。
- 需要自己管理底层总线时，优先参考 `examples/test_*` 和 `hand_teach_pendant/src/HandController.cpp` 的实际用法。

### Q9: Linux 下 CAN 默认怎么配？

```bash
sudo modprobe can
sudo modprobe can_raw
sudo ip link set can0 type can bitrate 1000000
sudo ip link set can0 up
ip link show can0
```

如果示例使用了 `can1` 或其他设备名，请按现场总线名替换。

### Q10: Windows 下还需要额外 DLL 吗？

需要。发布包已经带上 `third_party/PCAN_Basic/` 和对应导入库；客户工程除了 SDK 自身 DLL 外，还要确保 `PCANBasic.dll` 与可执行文件同目录，或能被系统搜索路径找到。

## 文档与支持

### Q11: 先看哪个文档最合适？

- 想了解总体接入方式：看 `README.md`
- 想确认接口签名：看 `docs/API-Reference.md`
- 遇到构建或运行问题：看 `docs/TROUBLESHOOTING.md`

### Q12: 旧版参考目录还能直接当文档源吗？

不能。旧版目录只适合做历史对照，内容可能与当前命名空间、打包布局、构建开关不一致。对外发布请以 `linkerhand/` 和 `pack.sh` 产物为准。
