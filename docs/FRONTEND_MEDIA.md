# Supplying your own media

The source-only frontend includes an empty disk catalog so the page itself can load. It cannot boot until you supply firmware and the starting media required by the selected profile. None are downloaded automatically by the build helpers.

`web/profiles/machines.json` records the runtime driver, BIOS, firmware ZIP names and expansion slot arguments. Current PCXC5-3 firmware ZIP names are `wangpc.zip` and `wangpc_wdc.zip`, placed under `web/media/`; use the names/checksums in MAME's ROM definitions. This is not a license or source for the ROM bytes. HLE keyboard operation does not require the original keyboard MCU dump.

The Winchester choices expect your own frozen starting CHDs named `ST225-ready.chd`, `ST225-initialized.chd` and `ST225-uninitialized.chd` under `web/media/`. They are different states, not aliases for one image. See the controller geometry and source definitions; no installed OS disk is provided. For a floppy-only experimental configuration, remove the Winchester slot argument, set `winchester` false and add your own bootable floppy to the catalog.

To add a disk to `web/profiles/disks.json`, add a record to `disks` with `id`, `label`, `notes`, `catalogStatus`, `captured`, `recoveryURL`, and an `images` array. Each image supplies `file`, `label`, `localPath` (relative to web/), `objectKey` (only for remote hosting), `sha256`, `bytes`, `format` (`mfi` or `img`), `preferred`, `status`, `availability`, and `bootable`. Use the file's actual SHA-256 and byte size. Set bootable true only with supporting evidence; it controls startup eligibility. Update the display counts under `stats` to describe your catalog.

Example structure (replace the illustrative hash/size/path with your own authorized file):

```json
{"id":"LOCAL001","label":"My boot disk","notes":"Owner-provided media","catalogStatus":"Local","captured":false,"recoveryURL":"https://example.org/","images":[{"file":"my-boot.mfi","label":"My boot disk","localPath":"media/my-boot.mfi","objectKey":"my-boot.mfi","sha256":"REPLACE_WITH_64_HEX_DIGITS","bytes":0,"format":"mfi","preferred":true,"status":"local","availability":"Owner-provided","bootable":true}]}
```

Without a deployment-config.js setting, the loader uses local `runtime/wangweb.wasm` and image `localPath` values. A cross-origin deployment may set `window.WANG_WASM_URL` and `window.WANG_DISK_BASE_URL` before app.js. The loader accepts a matching compiled JS/WASM pair and never fetches proprietary firmware from this source repository.

For a local static server, from the repository root:

```text
python -m http.server 8765 --bind 127.0.0.1 --directory web
```

Browser saves stay in that browser and origin. Use disposable copies when testing disk writes. Original captures should stay outside the served directory.
