# Maintenance scope

The maintained devices are L7, L20_10 / L20_11 (`G20`), L21, L25 and O20.
The boundary is device ownership, not a claim that every historical firmware or
communication path is covered by the newer SDK.

## Compatibility

Preserve exported signatures, enum numeric values, binary names and CMake targets.
Historical symbols may remain available in prebuilt binaries without being maintained.
Tools and examples advertise only maintained devices. A removed tool option is a scope
change and must be noted in release notes; it does not remove a binary symbol.

Do not rename `G20` to ordinary `L20`: they are different enum values. `L20_10` and
`L20_11` alias `G20`; `L20_Lite` aliases `L10`. Preserve these relationships.

## Work boundaries

Package changes can update headers, documentation, examples, callbacks and tooling.
Removing internal model implementations, changing protocol workers or rebuilding the SDK
requires the implementation source and a reproducible release build. Do not patch binary
files or claim that package cleanup has reduced the binary's supported models.

Transport adoption must preserve frame content, order, model-specific flags and callback
return semantics. Keep L7 Modbus on its existing implementation until request/response
ownership, raw ADUs, supported functions and late-reply handling are verified.

## Release gates

A release records the package revision, unchanged or replaced binary hashes, compiler/CRT,
transport revision and validation scope. An offline build is not hardware acceptance.
Validate each claimed device/firmware/backend combination before advertising it as tested.
Keep the preceding package available for rollback. Never replay uncertain motion commands
as part of automatic recovery.

Public ABI breaks require a breaking release. Dependency upgrades require scoped regression
checks. Do not regenerate bundled vendor files merely to standardize comments or formatting.
