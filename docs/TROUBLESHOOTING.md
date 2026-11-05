# Troubleshooting

## Build and loader failures

Confirm OS, architecture, compiler and CRT match the packaged library. Set
`CMAKE_PREFIX_PATH` to the package root or installation prefix. Inspect CMake's selected
library path. On Linux, inspect `ldd` output for missing dependencies; on Windows, keep the
matching runtime DLLs and adapter driver available. macOS is unsupported by this package.

For vendor CAN FD, distinguish a missing top-level library from a missing dependency or
wrong architecture. A library being loadable does not prove adapter compatibility.

## No replies

Check power, wiring, termination, interface state, nominal/data rates, model, side and bus
address. Use the device's confirmed protocol. Do not assume left/right names determine the
physical channel. SocketCAN must be configured before opening the transport.

Start with version and telemetry reads. Capture CAN IDs, standard/extended and FD/BRS flags,
payload lengths and timestamps. Vendor wire DLC codes and payload byte counts differ.
Register all required receive IDs; shared response IDs need protocol-level demultiplexing.

## Modbus

L7 requires the actual serial port, baud rate, parity and stop bits. Use device permissions
provided by the OS group or udev configuration. Requests and replies must be serialized on
one physical line. A delayed response may be confused with a subsequent request; do not
retry uncertain writes without reconciling device state.

## Failure reports

Include the fields listed in [device validation](device-compatibility.md), a minimal
reproduction and exact error output. Check [migration](migration.md) when the model is no
longer in this repository's maintenance scope.
