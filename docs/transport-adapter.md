# Callback transport integration

`include/legacy/CanCallbacks.h` adapts one shared linkerbot-transport CANDispatcher to the
legacy TX/RX callback ABI. It maintains a bounded inbox, reports overflow and last error,
and validates byte counts and IDs. The RX ABI has no capacity input: callers must supply
8 bytes for Classic CAN or 64 bytes for CAN FD. Closing an adapter unsubscribes its inbox
without stopping other dispatcher users.

This integration does not infer whole-command batches, device identity, request correlation
or device acknowledgments from individual frames. Legacy FD fields use payload byte counts
at the callback boundary; vendor wire DLC conversion belongs to the transport backend.
Keep callback owners alive through SDK destruction and stop the dispatcher afterward.

`include/legacy/ModbusCallbacks.h` keeps raw RTU frames and rejects unrepresentable response
lengths before copying. Split TX/RX callbacks do not create an atomic transaction contract;
applications must serialize complete operations on one line.

For optional adapter tests, configure:

```sh
cmake -S . -B build -DLINKERHAND_USE_TRANSPORT=ON \
  -DLINKERHAND_TRANSPORT_SOURCE_DIR=/absolute/path/to/linkerbot-transport-cpp
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure
```

The transport baseline is v0.3.4, commit `0e190032d65ed0b315337b45b3779cf4f650ed68`.
The optional source and installed-package paths require transport 0.3.4 or newer
in the 0.3 minor line. Rebuild callback consumers with matching transport headers
and libraries. The prebuilt legacy SDK and vendor binaries remain version 2.2.1
and are unchanged; this does not rebuild their unavailable implementation source
or establish new hardware, driver/firmware, bus timing or device-soak acceptance.
No implementation dependency is downloaded implicitly by this package.

Interactive connection selection and application examples are maintained in
[linkerbot-hand-tools](https://github.com/linker-bot/linkerbot-hand-tools).
