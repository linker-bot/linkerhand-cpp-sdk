# Changes

## Unreleased

- Rename the package to `linkerhand-legacy-cpp-sdk` and the imported CMake target to
  `LinkerHand::linkerhand_legacy_cpp_sdk`. Update installation paths and the Windows
  default prefix; consumers must update package lookups, target names and explicit paths.
  Prebuilt library filenames and the public C++ ABI remain unchanged.

- Move interactive tools, the Web server/UI, device setup utility and historical examples
  to linkerbot-hand-tools. Keep public SDK integration, callback adapters, binary dependencies
  and one read-only consumer example. BUILD_EXAMPLES now builds only that example.
- Replace consumer build wrappers with SDK package build/install commands. SDK and vendor
  binaries, enum values and public ABI are unchanged.


- Limit active maintenance and current tool selectors to L7, L20_10/L20_11, L21, L25 and O20.
  G20 aliases remain available. Examples for migrated devices remain in repository history.
- Preserve the packaged SDK ABI and binary files. Correct package version reporting to 2.2.1.
- Add an optional linkerbot-transport callback path; keep the existing backend as the default
  pending hardware acceptance. Do not infer command batches or automatically replay frames.
- Bound raw L7 Modbus responses before copying into the legacy callback buffer.
- Run ARM64 CI on a native runner and add offline validation for both communication paths.
- Require CMake 3.16+ for this repository's consumer/test build. The public C++ ABI remains unchanged.

Hardware acceptance and SDK implementation-source rebuilds are not part of these checks.
