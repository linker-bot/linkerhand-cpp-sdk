# Contributing

Changes should address L7, L20_10 / L20_11, L21, L25 or O20. Submit new model support
to [linkerbot-hand-cpp-sdk](https://github.com/linker-bot/linkerbot-hand-cpp-sdk).

Keep each pull request limited to one reviewable concern. Describe the behavior changed,
why it is needed, validation performed and remaining limitations. Use factual English
for commit messages and pull requests. Do not present offline checks as hardware results.

Preserve the public ABI and prebuilt libraries unless a separately verified rebuild is
available. Keep vendor headers, licenses and manuals intact. Update comments when their
contracts change; avoid speculation about implementation details absent from this package.

For bugs, include model, hardware/firmware, side/address, protocol, adapter/driver,
OS/architecture, compiler/CRT, package revision, reproduction and logs.

Interactive applications and historical demonstrations belong in
[linkerbot-hand-tools](https://github.com/linker-bot/linkerbot-hand-tools).
Keep this repository focused on SDK consumers, integration contracts, packaging and tests.
