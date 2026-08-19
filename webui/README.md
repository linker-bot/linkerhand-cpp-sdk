# Web 示教器（webui/）

一套开箱即用的灵巧手网页控制界面，全型号通用，`./build.sh -b` 编译后 `python3 webui/run.py` 即可使用。

支持型号：L6 / L7 / L10 / L20 / L21 / L25 / G20 / O6 / O20（O20 走 CAN-FD）。

## 前置

1. 编译 SDK：
   ```bash
   ./build.sh -b
   ```
2. **CAN / CAN-FD 型号**：拉起对应 CAN 接口并给手上电，例如：
   ```bash
   sudo ip link set can0 up type can bitrate 1000000
   # O20 原生 CAN-FD（可选，配合 --channel socketcan:can0）：
   # sudo ip link set can0 up type can bitrate 1000000 dbitrate 5000000 fd on
   ```
   也可用仓库根目录的 `can-autocfg.sh` 一次性配好自动激活/关闭。
3. **Modbus 型号（RS485 串口）**：给串口读写权限：
   ```bash
   sudo chmod 0777 /dev/ttyUSB0
   # 或把当前用户加入 dialout 组（重新登录后生效）：
   # sudo usermod -aG dialout $USER
   ```

## 运行

两种方式，任选其一。

**① 在线连接（推荐）**：不带 `--model` 启动，浏览器打开后在页面「设置」面板里选择型号 / 侧别 / 通信方式并连接，支持热重连与主动断开。

```bash
python3 webui/run.py
```

**② 命令行直连**：启动即按参数连接指定型号。

```bash
python3 webui/run.py --model O6  --side left
python3 webui/run.py --model L10 --side left  --channel can0
python3 webui/run.py --model O20 --side right                    # 厂商 CAN-FD 设备
python3 webui/run.py --model O20 --side right --channel socketcan:can0
python3 webui/run.py --model L10 --comm modbus --channel /dev/ttyUSB0
```

启动后浏览器打开 `http://<本机IP>:8080/`。

参数：

| 参数 | 说明 | 默认 |
|------|------|------|
| `--model` | 启动即连的型号（L6/O6/L7/L10/L20/G20/L21/L25/O20）；不填则在前端在线连接 | 无 |
| `--side` | `left` / `right` | `left` |
| `--comm` | `can` / `canfd` / `modbus`；缺省按型号自动（O20→canfd，其余→can） | 自动 |
| `--channel` | CAN 接口名（如 `can0`）/ Modbus 串口（如 `/dev/ttyUSB0`）；O20 可用 `socketcan:can0` | 自动 |
| `--host` | 监听地址 | `0.0.0.0` |
| `--port` | 监听端口 | `8080` |

## 功能

- 按型号动态渲染关节滑块，实时位置回读
- 速度 / 力矩设置与回读
- 触觉压感热力图（含掌心，O6 / G20）
- 温度 / 故障监控（温度 >50 黄、>60 红，故障码非 0 红）
- 版本信息展示、明暗主题切换

## 注意

- **未拉起 CAN / 未上电**：前端提示"等待 READY 超时"，检查总线与供电。
- **Modbus 串口无权限**（`Permission denied: /dev/ttyUSB0`）：先 `sudo chmod 0777 /dev/ttyUSB0`，或把用户加入 `dialout` 组（见「前置」第 3 步）。
- **触觉 / 温度 / 故障**：仅支持的型号有数据；无数据时不渲染或显示占位提示。
