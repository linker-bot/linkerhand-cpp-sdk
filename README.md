# LinkerHand Legacy C++ SDK

This repository maintains the packaged C++ SDK for devices not covered by
[linkerbot-hand-cpp-sdk](https://github.com/linker-bot/linkerbot-hand-cpp-sdk).
Maintenance covers compatibility fixes, communication adapters, examples and documentation.
New device models belong in the newer SDK.

## Maintained devices

| Device | Existing communication paths | Notes |
| --- | --- | --- |
| L7 | Classic CAN, Modbus RTU | Modbus remains on the packaged implementation until its transaction contract is verified. |
| L20_10 / L20_11 | Classic CAN | Both names alias `G20` in the public API. This does not establish identical firmware behavior. |
| L21 | Classic CAN | Existing examples use a 25-entry position vector including reserved slots. |
| L25 | Classic CAN | Includes model-specific enable/disable methods. |
| O20 | CAN FD | SocketCAN on Linux or the packaged vendor adapter, subject to platform support. |

L6, O6, L20 Lite, L20, L30 and O30 are maintained in the newer SDK.
`L10` aliases L20 Lite and is outside this repository's maintenance scope.
See [migration](docs/migration.md) before moving an existing application.

## Package boundary

This repository contains public headers, examples, tools and prebuilt SDK libraries.
The SDK implementation source is not included. Building this repository builds consumers
of those libraries; it does not rebuild or remove code from the SDK binary.
Public enum values, callback signatures, library names and the CMake package remain
unchanged for existing consumers. See [maintenance scope](docs/maintenance-scope.md).

## Build

Use Linux x86_64/aarch64 or Windows x64 with the matching packaged library and C++17
compiler and CMake 3.16+. macOS has no packaged SDK library or supported native CAN backend.
Windows applications must use the matching MSVC or MinGW runtime and library.
Build success is not evidence of device or driver compatibility.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4
```

Executables are written to `build/bin`. Examples that send motion targets require
an explicitly selected, powered device and its verified configuration. Read each example
before running it. CI must not execute hardware examples.

## Connect a device

Configure a Linux Classic CAN interface using the actual interface and bitrate:

```sh
sudo ip link set can0 down
sudo ip link set can0 type can bitrate 1000000 fd off
sudo ip link set can0 up
```

For O20, configure CAN FD nominal/data rates according to the device and firmware
specification. `can-autocfg.sh` installs system configuration; it is optional and
requires administrator privileges. Serial access should use the appropriate device group
or udev rule rather than world-writable permissions.

The read-only [integration example](demo/main.cpp) registers CAN callbacks and reads
an L7 version. [API reference](docs/API-Reference.md) describes callback and error contracts.

## Web and terminal tools

```sh
./start-webui.sh
python3 webui/run.py --model L7 --side left --comm can --channel can0
```

The web interface is intended for a trusted local control network. Choose a maintained
model and an available transport. See [Web UI](webui/README.md) for connection behavior.
`hand_tui` provides interactive device checks when its build prerequisites are available.

## Integrate with CMake

```cmake
cmake_minimum_required(VERSION 3.16)
project(legacy_hand_app LANGUAGES CXX)
find_package(linkerhand-cpp-sdk 2.0 CONFIG REQUIRED)
add_executable(legacy_hand_app main.cpp)
target_compile_features(legacy_hand_app PRIVATE cxx_std_17)
target_link_libraries(legacy_hand_app PRIVATE LinkerHand::linkerhand_cpp_sdk)
```

Set `CMAKE_PREFIX_PATH` to the package root or installation prefix. On Linux, arrange
for the SDK and its runtime dependencies to be available to the dynamic loader.
`build.sh -i` installs the package; review `build.sh --help` before system installation.

See the [optional transport adapter](docs/transport-adapter.md) for the new communication path.

## Maintenance and validation

- [Scope and compatibility policy](docs/maintenance-scope.md)
- [Migration to the newer SDK](docs/migration.md)
- [Device validation records](docs/device-compatibility.md)
- [Binary provenance](docs/build-provenance.md)
- [Troubleshooting](docs/TROUBLESHOOTING.md) and [FAQ](docs/FAQ.md)

Report the model, firmware, side, protocol, platform, adapter, package revision and a
minimal reproduction. Do not treat an empty getter result or a successful enqueue as
proof of device execution. See [contributing](CONTRIBUTING.md).

The project is distributed under the [MIT license](LICENSE). Bundled third-party
components retain their own notices and documentation.

See [release checks](docs/releasing.md) for the package acceptance procedure.
