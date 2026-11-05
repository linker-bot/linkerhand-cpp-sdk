# Optional transport adapter

The default build preserves the packaged communication implementation. Enable the
new callback path explicitly while hardware acceptance remains pending:

```sh
cmake -S . -B build -DLINKERHAND_USE_TRANSPORT=ON \
  -DLINKERHAND_TRANSPORT_SOURCE_DIR=/absolute/path/to/linkerbot-transport-cpp
cmake --build build --parallel 4
```

Use transport commit `ff429d0c0333120dd6867ee7247549e53934ed46` as the initial source
baseline. Alternatively install a compatible `linkerbot-transport` 0.3 package built
with the matching compiler/CRT and omit the source directory option. No dependency is
fetched implicitly. Only `linkerbot::can` is linked; serial/Modbus is not enabled by this adapter.

`legacy_transport can0` (Linux) or `legacy_transport PCAN_USBBUS1` (Windows) reads an L7
left-hand version through the adapter. The web bridge also uses this path when enabled.
Classic CAN requires an explicit channel. For O20, use `socketcan:can0` on Linux or
vendor channel `0`/empty with the appropriate native library installed. Vendor defaults
must be checked against the device specification; select its library via the transport
library configuration. No automatic device discovery or identity verification is added.

`legacy/CanCallbacks.h` shares a dispatcher, maintains a bounded receive inbox and returns
legacy callback status codes. Each physical connection should have one dispatcher. Closing
an adapter unsubscribes only its inbox; its owner stops the physical dispatcher after all
SDK users have been destroyed. The RX ABI has no capacity input: Classic consumers must
provide eight bytes and FD consumers 64 bytes. `last_error()` and `overflows()` retain adapter
diagnostics; dispatcher health supplies underlying faults.

The bridge preserves legacy payload filtering in the prebuilt SDK because the complete
reply-ID contract is not public. It filters frame type but broadcasts matching frames to
subscribers; callers must not assume independent request correlation for devices with
shared response IDs. New protocol implementations with known routes should use Session.

TX returns after backend acceptance of one frame. This does not add a device ACK, deadline,
recovery, whole-command batching or `latest` scheduling. A multi-frame motion command
cannot be inferred from individual callbacks. No uncertain frame is replayed automatically.

Keep the default packaged path until the combinations listed in
[device validation](device-compatibility.md) have passed. L7 Modbus remains unchanged.
