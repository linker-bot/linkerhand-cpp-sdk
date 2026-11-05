# Legacy hand Web UI

The Python standard-library server drives `build/bin/web_bridge`. It supports L7,
L20_10 / L20_11, L21, L25 and O20. L7 supports CAN or Modbus; O20 uses CAN FD;
the remaining models use Classic CAN.

```sh
./build.sh -b
python3 webui/run.py --model L7 --side left --comm can --channel can0
```

Omit `--model` to select a device in the browser. Configure SocketCAN before connecting.
For O20, `--channel socketcan:can0` selects the Linux SocketCAN FD path; an empty channel
uses the packaged vendor default. Confirm the device's nominal/data rates and adapter.
Modbus takes a serial port such as `/dev/ttyUSB0` or `COM3`.

The server defaults to port 8080 and binds all interfaces. Use `--host 127.0.0.1` for local
access. It has no authentication and should run only on a trusted control network.
Connecting may set device parameters; inspect the bridge initialization before operating
hardware. Slider values are legacy 0–255 controls, not physical joint angles.

Model vector sizes include reserved entries for some devices. An empty or cached reading
is not confirmation of motion. Follow the [validation procedure](../docs/device-compatibility.md).
The [migration guide](../docs/migration.md) covers devices maintained by the newer SDK.
