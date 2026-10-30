# Publication verification

The complete tracked MAME tree was exported from commit 39eb9e44c394df9aec8186f2d2ef7517ace232e1. The complete local patch passed git apply --check against the clean export before modified files were overlaid. Each published modified file was compared with its compiled input after removing one dated notice and normalizing LF line endings; executable source is unchanged.

All five dependency source archives match the SHA-512 values in the included Emscripten recipes. Published SHA-256 values are in SOURCE_MANIFEST.json. PowerShell/Bash/Python/JavaScript build/frontend syntax checks passed. The source-only helpers retain the actual focused build flags, use configurable SDK/cache paths, and omit proprietary media staging.

The original runtime was compiled and tested with real firmware/DOS, Winchester writes/save/restore, live disk changes and 256 KB memory reports. A full clean compile from this new publication export has NOT been performed; no new binary or bit-identical rebuild is claimed. The original runtime hash and build settings are recorded in SOURCE_MANIFEST.json.

No project ROM dumps, guest disk images, saved sessions, SDK binaries, cloud credentials or original workstation configuration are included. Upstream source assets and third-party test fixtures remain licensed by their respective projects. Git staging and ZIP integrity are checked separately when packaging finishes.

Run python scripts/verify-source.py before publication. SOURCE_FILES.txt is the explicit safe staging list described in PUBLISHING.md. Preparation and verification do not create or publish a GitHub repository.
