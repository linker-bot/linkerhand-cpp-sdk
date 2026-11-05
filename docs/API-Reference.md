# Legacy C++ API reference

This reference covers maintained devices: L7, L20_10 / L20_11 (`G20`), L21, L25 and
O20. Other declarations remain for binary compatibility; see [migration](migration.md).
The implementation is prebuilt, so model-specific behavior must be checked against the
actual firmware and the [validation record](device-compatibility.md).

## Construction

```cpp
LinkerHandApi(const LINKER_HAND& model, const HAND_TYPE& side,
              const COMM_TYPE comm = COMM_TYPE::CAN);
```

The header also declares a scoped-enum overload.
Use `COMM_TYPE::CAN` for both Classic CAN and O20 CAN FD. L7 also supports
`COMM_TYPE::MODBUS`. EtherCAT is not a maintained communication path.
Construction does not replace application-owned transport setup or callback registration.

## CAN callbacks

```cpp
using CanTxCallback = std::function<int32_t(uint32_t id,
    const uint8_t* data, uintptr_t length)>;
using CanRxCallback = std::function<int32_t(uint32_t* id,
    uint8_t* data, uint8_t* length)>;
```

Register using `setCanTxCallback` and `setCanRxCallback`. Existing examples return `0`
on success and `-1` when no frame is available or I/O fails. Keep transport state alive
while callbacks may be invoked. Catch exceptions at the callback boundary and retain
separate diagnostics, because the return code cannot distinguish detailed errors.

TX length and RX output length are payload byte counts. The vendor `CanFDFrame.can_dlc`
is a wire DLC code and needs conversion; the SocketCAN FD variant already supplies bytes.
O20 uses extended CAN FD frames. Do not infer frame type solely from payload length.
Classic buffers hold at most 8 bytes; FD buffers hold at most 64 bytes. The RX callback
has no capacity argument: the caller and adapter must agree on the buffer bound.

`freeCanCallback` and `freeModbusCallback` are historical reset APIs. Their presence does
not establish a thread-drain guarantee. Keep callback owners alive through API destruction
unless a separately verified shutdown contract is available.

## Modbus callbacks

L7 callbacks pass complete raw RTU frames in existing examples. TX receives slave ID,
register address, bytes and length; RX receives slave ID and output pointers. CRC and
frame boundaries must match the packaged protocol implementation. These are not the same
contract as a structured `RtuSession` request. See [maintenance scope](maintenance-scope.md).

The old RX length output is `uint8_t`; it cannot represent a 256-byte ADU. Adapters must
reject oversize results before copying or narrowing. `IModbus::transact` expresses an
atomic raw exchange, but independent TX/RX callbacks do not automatically provide one.

## Control vectors

`setPosition` accepts normalized `uint8_t` values, not physical angles. Legacy API
position layouts are L7: 7, G20: 16, L21: 25, L25: 25 and O20: 16 entries. Reserved
entries are part of the legacy layout; do not remove them based on the marketing joint
count. O20 readback length can differ from its command length.

`setPositionArc` accepts `double` values using the packaged mapping. Validate the
mapping and units for the device before use. `testPositionArc` is a historical conversion
entry point, not a hardware test.

`setSpeed` and `setTorque` have model-specific vector layouts. Do not assume every model
uses either five entries or its full position count: Historical G20 usage uses five speed entries,
while L7 uses seven. Confirm the particular operation and firmware before sending.

`setPositionRaw`, `getPositionRaw`, range getters, `setSpeedRaw` and `setTorqueRaw` are
retained ABI methods associated with migrated devices. Applications select the appropriate SDK; shared tools own
raw-control presentation.

`setEnable` / `setDisable` are documented for L25; `clearFaultCode` is historically
associated with ordinary L20, which is outside active maintenance. A declaration alone
does not establish support on another model.

## Readings

Position, speed, torque, temperature, fault and version getters expose legacy containers
or strings. Empty values may mean missing data, failure or an unsupported operation.
No universal freshness or device acknowledgment contract is provided by these return types.

`getForce` returns finger matrices whose dimensions depend on the sensor and firmware.
Do not assign physical units to bytes without a verified sensor specification.
`getPalmForce`, `getFingerForceSum` and `getPalmForceSum` are available in the ABI;
Historical G20 interfaces expose palm/aggregate sensing. Treat support and dimensions as device-specific.

## Error channel

`trySetPosition`, `trySetSpeed` and `trySetTorque` return `HandError`; the header describes
parameter/initialization validation and CAN enqueue success. They do not report mechanical
arrival. Out-parameter `getPosition` / `getTorque` distinguish a nonempty result from
`OperationFailed`, but do not add a freshness guarantee.

## Minimal integration

[demo/main.cpp](../demo/main.cpp) registers callbacks and reads the L7 version.
[demo/CMakeLists.txt](../demo/CMakeLists.txt) links the packaged SDK target.
Use matching compiler/CRT binaries and arrange runtime library discovery. Hardware
examples may write parameters or move devices; CI only builds those executables.
