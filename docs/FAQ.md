# Frequently asked questions

## Which devices are maintained here?

L7, L20_10 / L20_11 (`G20`), L21, L25 and O20. See
[scope](maintenance-scope.md) and [migration](migration.md).

## Does the build compile the SDK implementation?

No. It builds consumers of the included prebuilt SDK. The implementation source is absent.
The existing public ABI and binary files remain available for compatibility.

## Does CAN FD require a different COMM_TYPE?

No. O20 uses `COMM_TYPE::CAN` with CAN FD callbacks. Nominal/data bitrate and frame flags
must match the actual device. SocketCAN interface configuration is external to the SDK.

## Does a successful call mean the device executed the command?

No. Queue acceptance, backend dispatch, device acknowledgment and mechanical arrival are
different states. `trySet*` does not provide a device acknowledgment. Empty getter results
can mean no data or unsupported functionality; consult the model and firmware record.

## Can I use macOS or an arbitrary Windows compiler?

macOS has no packaged SDK or supported native backend. Windows consumers must match the
packaged compiler/CRT and architecture. Do not mix incompatible C++ runtimes across DLLs.

## Can I use EtherCAT?

The ABI contains historical declarations. EtherCAT is not a maintained tool path and is
not advertised as working. L7 is the maintained Modbus device.

## Where are the TUI, Web UI and complete v2.2.1 examples?

Use [linkerbot-hand-tools](https://github.com/linker-bot/linkerbot-hand-tools).
It preserves historical sources and uses both SDK generations. This package keeps only
SDK integration and a read-only consumer example. Device configuration scripts also
belong with the tools, not SDK installation.
