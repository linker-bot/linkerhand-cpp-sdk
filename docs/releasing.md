# Release checks

Keep package changes separate from SDK binary rebuilds. Do not create a release tag
until the intended device/firmware/backend acceptance records are available.

1. Record the package commit and transport revision. Generate `tools/binary_manifest.py`
   output and compare SDK/vendor hashes with the prior package; explain every binary change.
2. Build the default and optional transport paths on native Linux x86_64/ARM64 and
   Windows x64 MSVC. Run offline CTest checks. Keep Ubuntu 20.04/22.04 compatibility builds.
   Windows MinGW and vendor-device support remain subject to matching-runtime acceptance.
3. Verify installation and a separate `find_package` consumer. Check version/range behavior,
   runtime dependencies and installed public headers. Interactive application validation belongs
   in linkerbot-hand-tools; SDK CI does not operate hardware.
4. Complete the applicable [hardware record](device-compatibility.md), including firmware,
   driver and adapter details. Keep untested combinations explicitly unverified.
5. Document tool selector changes, migration gaps, binary provenance and known limitations.
   Preserve the previous package and dependency revisions for rollback.

CI exercises source compilation, offline frame/callback behavior, model scope and CMake
version matching. It does not validate timing on a physical bus, mechanical arrival,
firmware mappings or the missing SDK implementation build.

The optional transport path stays opt-in until accepted on the relevant hardware. L7 RTU
remains on the packaged backend with bounded raw callbacks. Internal binary model pruning
and full RTU transaction migration require the original implementation/verified contracts.
