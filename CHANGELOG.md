# Changes

## Unreleased

- Limit active maintenance and current tool selectors to L7, L20_10/L20_11, L21, L25 and O20.
  G20 aliases remain available. Examples for migrated devices remain in repository history.
- Preserve the packaged SDK ABI and binary files. Correct package version reporting to 2.2.1.
- Add an optional linkerbot-transport callback path; keep the existing backend as the default
  pending hardware acceptance. Do not infer command batches or automatically replay frames.
- Bound raw L7 Modbus responses before copying into the legacy callback buffer.
- Run ARM64 CI on a native runner and add offline validation for both communication paths.
- Require CMake 3.16+ for this repository's consumer/test build. The imported SDK target name
  and public ABI remain unchanged.

Hardware acceptance and SDK implementation-source rebuilds are not part of these checks.
