# SaveTheWANG-Web_Emulator

Modified MAME source and browser frontend for Save the Wang's PCXC5-3 emulator.

**Source release:** web application 2.3.2, published source snapshot dated 30 October 2026. **Runtime:** MAME 0.289, Emscripten 6.0.2. This is a derivative project, not an official MAME release.

This repository contains the complete pinned MAME source tree with our changes applied, the corresponding Emscripten runtime sources, upstream source archives for linked browser ports, frontend source, license notices, and build instructions. It does not rely on a patch alone. The optional patch is also included for reviewing changes against the upstream baseline.

- [Build instructions](BUILDING.md)
- [GPL and source notice](NOTICE.md)
- [What changed](SOURCE_CHANGES.md)
- [Source / binary mapping](SOURCE_MANIFEST.json)
- [Publication steps](PUBLISHING.md)
- [Upstream MAME license notices](licenses/MAME-COPYING)

## Scope

The current PCXC5-3 profile has 128 KB onboard RAM plus a PM030-B-capacity 128 KB expansion in slot 3, an MVC display, floppy A and an ST-225 Winchester at C. The keyboard uses a high-level protocol implementation. PC-S1-2 and PC-S2-2 profiles are pending their own ROM dumps and are not presented as verified machines.

Firmware and operating-system media are separate from the emulator source license. This repository intentionally contains no project ROM dumps, floppy/Winchester images, saved sessions, cloud credentials, compiled WASM, or native emulator executables. Obtain your own authorized firmware/media to run a rebuilt emulator. Original upstream MAME source assets and third-party source/test fixtures retain their upstream licenses.

## License

MAME as a whole is GPL-2.0-or-later; individual source files retain their existing, often more permissive, license headers. New project frontend/build-script code is provided under GPL-2.0-or-later. See [LICENSE](LICENSE), [NOTICE.md](NOTICE.md), and the preserved third-party notices. This does not grant trademark rights or change the rights to Wang firmware, software, or branding.

**Canonical repository:** https://github.com/MistaJfelgo/SaveTheWANG-Web_Emulator

The accompanying build metadata identifies the distributed WASM by SHA-256. The source snapshot adds dated change-notice comments to modified MAME files; those comments and canonical LF line endings are the only publication-only changes to those compiled inputs. A rebuild is not claimed to be byte-for-byte identical across environments.
