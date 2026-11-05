# Migration to linkerbot-hand-cpp-sdk

Use [linkerbot-hand-cpp-sdk](https://github.com/linker-bot/linkerbot-hand-cpp-sdk)
for L6, O6, L20 Lite (`L10`), L20, L30 and O30. This repository maintains L7,
L20_10 / L20_11 (`G20`), L21, L25 and O20.

The newer SDK is not a drop-in replacement for `LinkerHandApi`. Verify model and firmware
identity, transport availability, address rules, joint ordering, units, limits, read
freshness, command completion and recovery behavior before migrating.

Existing Modbus paths for O6 and L20 Lite are outside this repository's active scope.
Their removal from current tools does not establish equivalent Modbus support in the newer
SDK. Keep the previous package until the required replacement path has been validated.

Historical examples and tool behavior remain available at the
[v2.2.1 tag](https://github.com/linker-bot/linkerhand-cpp-sdk/tree/v2.2.1).
Do not remove public enums from an existing application's headers to enforce the new scope.

`L20_10` and `L20_11` are not ordinary `L20`. The legacy aliases are preserved; applications
must select the appropriate device and firmware rather than migrate by a name prefix.

## Shared tools

CLI/TUI, Web UI, tactile/palm visualization and historical demonstrations now live in
[linkerbot-hand-tools](https://github.com/linker-bot/linkerbot-hand-tools). That repository
preserves the v2.2.1 tool sources and provides a shared runtime for both SDK generations.
The package keeps only one read-only integration example. `BUILD_EXAMPLES` no longer
builds device applications; application-specific targets and start-webui.sh were removed.
Use the tools project's explicit legacy compatibility selection for historical API paths.
