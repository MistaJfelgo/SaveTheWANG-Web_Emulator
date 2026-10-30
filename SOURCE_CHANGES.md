# Local MAME changes

Modified for Save the Wang between 26 and 30 October 2026; publication snapshot dated 30 October 2026. Upstream commit: `39eb9e44c394df9aec8186f2d2ef7517ace232e1`.

| Source files | Changes |
| --- | --- |
| `src/mame/wang/wangpc.cpp` | R3 BIOS option; keyboard diagnostic/HLE configurations; guest-visible floppy-change IRQ acknowledgement/status; exact ST-225 drive identity; optional floppy connectors and absent-drive status; browser keyboard/media/control bridge. |
| `src/mame/wang/wangpckb.cpp`, `.h` | High-level Wang serial keyboard protocol and input support; original MCU-based option retained. |
| `src/devices/bus/wangpc/wdc.cpp` | Selectable R12 controller ROM definition and related local controller changes. |
| `src/devices/bus/wangpc/emb.cpp`, `.h`, `wangpc.cpp` | Configure one accessible 128 KB bank with `emb128`; original `emb` behavior preserved. |
| `src/devices/imagedev/floppy.cpp` | Retain loaded format pointer needed for track commit/save. |
| `src/lib/formats/mfi_dsk.cpp` | Reject incompatible floppy geometry safely. |
| `src/emu/machine.cpp` | Advance browser emulation by elapsed wall time, with bounded catch-up. |
| `src/mame/mame.lst` | Register the local Wang diagnostic/HLE driver variants. |
| `scripts/genie.lua` | Web runtime/filesystem export and compatibility settings. |
| `scripts/src/3rdparty.lua`, `scripts/src/devices.lua`, `scripts/src/mame/frontend.lua`, `scripts/src/osd/modules.lua` | SDL3/Emscripten build compatibility for the focused target. |

The complete patch is authoritative for exact edits. Original file license/copyright notices remain in place. Each modified published MAME file has one added dated comment referring here. `SOURCE_MANIFEST.json` maps the original compiled input to the annotated publication copy.

Frontend changes include private browser disk copies, MFI/IMG selectors, checked lazy media downloads, save-aware turn-off, Quick Help, progress, responsive styling, and timed warning highlights. They are ordinary HTML/CSS/JavaScript source under `web/`.

No firmware ROM bytes were patched to produce these fixes. The firmware names/checksums remain source definitions, not embedded project ROM dumps. The keyboard uses a high-level protocol implementation; it does not reproduce a missing mask ROM. The PM030-B-capacity model reuses the existing Wang memory-board interface; newly verified parity-fault circuitry is not claimed.
