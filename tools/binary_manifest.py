#!/usr/bin/env python3
"""Report hashes of packaged SDK and vendor files without following duplicate symlinks."""
import hashlib
import json
from pathlib import Path

root = Path(__file__).resolve().parents[1]
records = []
for directory in ("lib", "third_party"):
    for path in sorted((root / directory).rglob("*")):
        if path.is_file() and not path.is_symlink():
            records.append({"path": path.relative_to(root).as_posix(),
                            "sha256": hashlib.sha256(path.read_bytes()).hexdigest()})
print(json.dumps(records, indent=2))
