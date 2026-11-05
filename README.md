# LinkerHand Legacy C++ SDK

Packaged C++17 SDK for L7, L20_10/L20_11 (`G20`), L21, L25 and O20.
This repository maintains public headers, prebuilt libraries, callback integration,
installation metadata, focused SDK tests and one read-only consumer example.
The SDK implementation source is not included and the binary remains version 2.2.1.

Interactive applications and historical demonstrations are maintained in
[linkerbot-hand-tools](https://github.com/linker-bot/linkerbot-hand-tools): CLI/TUI, Web UI,
pressure/palm visualization, motion demos and v2.2.1 examples, including O30 vendor CAN FD.
That project uses both this SDK and [linkerbot-hand-cpp-sdk](https://github.com/linker-bot/linkerbot-hand-cpp-sdk).
L6, O6, L20 Lite, L20, L30 and O30 protocol development belongs in the newer SDK.
Historical declarations remain in this package for ABI compatibility and explicit tool
compatibility paths; they are not new maintenance commitments.

## Build and validate

Use Linux x86_64/ARM64 or Windows x64 with the matching packaged C++ runtime, CMake 3.16+
and Python 3 for offline checks. macOS has no packaged SDK library or native CAN backend.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
cmake --install build --config Release --prefix /desired/prefix
```

`BUILD_EXAMPLES=ON` builds only [demo/main.cpp](demo/main.cpp), which registers callbacks
and reads an L7 version. CI builds this example but never executes it against hardware.
`BUILD_TESTING=OFF` omits offline tests; `LINKERHAND_USE_TRANSPORT=ON` enables optional
transport adapter compilation/tests. There is no Web server, TUI or device setup utility
in this package. Building consumers does not rebuild the SDK implementation.

`build.sh` and `build.bat` wrap these CMake commands. Installation uses CMake's package
layout without deleting unrelated prefix contents or editing the system PATH.
Uninstallation on Linux uses the recorded package manifest: `./build.sh --uninstall`.
Use an appropriate writable prefix or explicitly run the installer with needed privileges.

## CMake integration

```cmake
find_package(linkerhand-cpp-sdk 2.2.1 CONFIG REQUIRED)
add_executable(my_app main.cpp)
target_compile_features(my_app PRIVATE cxx_std_17)
target_link_libraries(my_app PRIVATE LinkerHand::linkerhand_cpp_sdk)
```

Set `CMAKE_PREFIX_PATH` to the package root or installation prefix. Arrange runtime library
availability for the selected platform and adapter; Windows consumers must match compiler/CRT.
Public enum values, callback signatures, imported target and binary names are preserved.
`L20_10` and `L20_11` alias `G20`; ordinary `L20` is different. `L10` aliases L20 Lite.

## Documentation

- [API reference](docs/API-Reference.md)
- [Callback transport integration](docs/transport-adapter.md)
- [Maintenance boundary](docs/maintenance-scope.md)
- [Migration to SDKs and shared tools](docs/migration.md)
- [Device acceptance records](docs/device-compatibility.md)
- [Binary provenance](docs/build-provenance.md) and [release checks](docs/releasing.md)
- [Troubleshooting](docs/TROUBLESHOOTING.md) and [FAQ](docs/FAQ.md)

No new hardware acceptance is implied by extraction. Device-specific behavior depends on
firmware and sensors; queue/backend acceptance is not device acknowledgment or mechanical
arrival. SDK and bundled dependency binaries remain unchanged.

See [LICENSE](LICENSE) for the SDK package license. Vendor components retain their notices.
