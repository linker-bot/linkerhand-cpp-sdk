# Device validation

The table records maintenance ownership and pending acceptance work, not hardware results.
No physical device was exercised as part of the scope change.

| Model | Existing path | Acceptance status |
| --- | --- | --- |
| L7 | Classic CAN / Modbus RTU | Pending device, firmware and adapter record |
| L20_10 / L20_11 | Classic CAN | Pending separate firmware records |
| L21 | Classic CAN | Pending device, firmware and adapter record |
| L25 | Classic CAN | Pending device, firmware and adapter record |
| O20 | CAN FD | Pending per-backend and platform records |

For each run, record date, operator, model, hardware revision, firmware, side, bus address,
OS/architecture, compiler/CRT, adapter/driver, package commit and transport commit.
Attach the exact commands, results and relevant logs. Do not include credentials.

Start with identity/version and read-only telemetry. Validate parameter writes and readback
before bounded movement. Check frame lengths, joint order, reserved slots, sensor layout,
timeouts, disconnect/recovery, repeated open/close and supported shared-bus operation.
A control write response does not establish mechanical arrival.

Linux aarch64 build coverage does not imply a compatible O20 vendor library. Windows needs
matching architecture and CRT. CI builds do not validate USB timing, bus arbitration or
firmware behavior. Mark unavailable combinations unverified rather than passed.
