# Binary provenance

The initial maintenance baseline is repository commit
`eaf21dfd8d51bea557dae6a1b77711cd78b60dcc`. Packaged Linux libraries are named
`linkerhand_cpp_sdk.so.2.2.1`; Windows libraries are separated by compiler family.
File names alone do not establish reproducibility or the original compiler flags.

The repository does not contain the SDK implementation or its original build recipe.
Until those inputs are available, preserve the prebuilt SDK and vendor libraries unchanged.
Record hashes before publishing and distinguish package/tool changes from binary changes:

```sh
python3 tools/binary_manifest.py > binary-manifest.json
```

A source rebuild needs the implementation revision, compiler and CRT versions, build
options, dependency versions, exported-symbol comparison and device regression records.
Do not claim an internal model was removed based on header or example changes.
