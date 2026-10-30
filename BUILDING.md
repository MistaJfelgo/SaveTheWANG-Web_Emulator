# Building the source release

## Baseline and build identity

- MAME version: 0.289.
- Upstream commit: `39eb9e44c394df9aec8186f2d2ef7517ace232e1`.
- Web SDK: Emscripten 6.0.2. Emsdk checkout used locally: `d7b4130688bfa348392b921ab04015aa15c4f154`.
- Target: `SUBTARGET=wangweb SOURCES=src/mame/wang/wangpc.cpp`.
- Published runtime SHA-256: `02876a62a38b02e93dddeb342bc55a0b91e17a7f7244478a5de939c70a365b62`.

The complete modified source is already in `mame/`. **Do not apply the patch on top of it.** `patches/wang-browser-complete.patch` is a review/reconstruction artifact against the pinned upstream commit. The publication snapshot additionally marks modified files with dated comments; manifests record both compiled-input and published-file hashes.

## Windows prerequisites

Use 64-bit Windows and MSYS2 UCRT64. Update MSYS2 following its own restart prompts, then install the required tools from the UCRT64 shell:

```bash
pacman -Syu
pacman -S --needed bash git make mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-python
```

Use a short project path, such as `C:/src/SaveTheWANG-Web_Emulator`. A Windows helper maps the project to V: temporarily to avoid MAME generator problems with spaces/long paths. V: must be unused. The helper creates a short SDK junction and separate compiler cache on C: under the user's temporary directory; it does not delete or alter another mapping.

Install the pinned SDK from PowerShell in the repository root (the `tools/emsdk` folder must not already contain another checkout):

```powershell
git clone https://github.com/emscripten-core/emsdk.git tools/emsdk
git -C tools/emsdk checkout d7b4130688bfa348392b921ab04015aa15c4f154
& ./tools/emsdk/emsdk.bat install 6.0.2
& ./tools/emsdk/emsdk.bat activate 6.0.2
```

Then build:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/Build-Web.ps1 -Jobs 2
```

The helper invokes MSYS2 Bash, then `emmake make` with the recorded settings. It checks the compiler exit code rather than treating SDK stderr warnings as build failures. Logs are written to `build-web.log`. Start with two jobs because linking/optimization uses significant memory. Build output is `mame/wangweb.js` and `mame/wangweb.wasm`; a matched pair is copied to `web/runtime/` after success. ROMs and disk images are not staged by this source-only build.

The historical local invocation and original wrapper are retained under `scripts/original-*` for comparison. The original wrapper's private media-staging step is not part of the new source-only helper.

## Linux / another suitable Unix build host

Install the host prerequisites described in MAME's included documentation and install/activate Emscripten 6.0.2. Set `EMSDK` to that SDK checkout and invoke:

```bash
EMSDK=/path/to/emsdk WANG_WEB_JOBS=2 bash scripts/build-web.sh
```

Windows was the project's original build host. This portable wrapper is provided for other hosts; a complete clean Linux build has not been tested here.

The essential build command, from the modified `mame/` directory after activating the SDK, is:

```bash
emmake make SUBTARGET=wangweb SOURCES=src/mame/wang/wangpc.cpp REGENIE=1   PYTHON_EXECUTABLE=python CLANG_VERSION=23.0.0 OPTIMIZE=2 SYMBOLS=0   NO_USE_MIDI=1 NO_USE_PORTAUDIO=1 NO_USE_PULSEAUDIO=1 NO_USE_PIPEWIRE=1 -j2
```

`CLANG_VERSION=23.0.0` is explicit because the local MAME version detector misread this SDK's compiler banner. Preserve the build flags when reproducing this target. The target links SDL 3 / SDL_ttf via the pinned SDK's port recipes.

## Dependency sources

MAME's bundled dependencies are included under `mame/3rdparty/`. The installed Emscripten 6.0.2 source distribution is included under `third_party/emscripten/`, excluding compiler binaries, node_modules and generated caches. Use the SDK installation above for the actual host toolchain. Its runtime sources and port recipes are included for inspection and source availability.

The exact downloaded port archives are supplied under `third_party/port-sources/`. Their SHA-256 values are in `SOURCE_MANIFEST.json`; export verification also checks their SHA-512 against the SDK recipes. The SDK normally downloads those same verified archives on the first build. Included archives provide the linked-library source; this is not a claim that the complete host toolchain can be installed offline.

Port recipe versions: SDL 3.4.2, SDL_ttf 3.2.2, FreeType 2.13.3, HarfBuzz 3.2.0, and zlib as specified in the included `tools/ports/zlib.py`. Some local archive filenames abbreviate versions; use recipes/hashes rather than filenames as the authoritative identity.

Two zstd CLI-test symlinks in the upstream tree may be materialized as copies by Windows extraction. They do not participate in the Wang runtime build. Source verification records these separately.

## Native Windows emulator

In an MSYS2 UCRT64 shell, from `mame/` (use a short path or a temporary drive alias if needed):

```bash
make SUBTARGET=wangpc SOURCES=src/mame/wang/wangpc.cpp REGENIE=1 PYTHON_EXECUTABLE=python -j2
```

The resulting `wangpc.exe` includes the current source changes. With your own required firmware under `roms/`, a typical PCXC5-3 configuration is:

```text
wangpc.exe wangpchle -bios r3 -slot1 wdc,bios=r12 -slot2 mvc -slot3 emb128 -upd765:1 "" -rompath roms
```

Use `-hard <your CHD>` and `-flop1 <your working image>` as appropriate. Do not test writes against an archival original. This source package does not automatically overwrite any existing native executable or user's disk.

## Running the browser frontend

The source can compile without Wang ROMs. Booting requires your own firmware/media and their configuration. The frontend expects `web/media/` firmware ZIPs, frozen Winchester templates, `web/profiles/disks.json` and the image metadata it references. The media library and copyrighted project ROM/disk files are intentionally absent here. The interfaces and paths are documented in `docs/FRONTEND_MEDIA.md`.

For a deployment, serve a matched JS/WASM pair. If WASM lives on another origin, load `deployment-config.js` **before** `app.js` and set `window.WANG_WASM_URL`; CORS must permit the page origin. Use `application/wasm`. The frontend's instantiate hook uses the streamed bytes, so the runtime is downloaded once. No cloud deployment or proprietary-media download is performed by these build scripts.

## Validation limits

The original working runtime was built locally and tested with real firmware/DOS, disk swaps, memory reports, browser saves and restores. This publication checks the exported source, complete patch application, dependency hashes, notices and build-script syntax. A full clean compile from this newly exported directory is recorded separately in `VERIFICATION.md`; do not assume a bit-identical rebuild. Host paths, timestamps, library/tool versions and publication-only comment line shifts can affect binary bytes.
