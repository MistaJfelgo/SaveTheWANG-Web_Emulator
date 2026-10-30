# Publishing this repository

Prepared for **https://github.com/MistaJfelgo/SaveTheWANG-Web_Emulator**, using the connected GitHub account name. If the repository will belong to another account/organization, update that URL in README.md, NOTICE.md, docs/gpl-notice.html and this file before publishing.

1. Create a **public**, empty GitHub repository named `SaveTheWANG-Web_Emulator`. Do not initialize it with a separate README/license.
2. Use the contents of this source package as its root. Do not copy the parent project workspace: it contains private media and build tools that do not belong here.
3. From this exported folder, using Git:

```bash
git init -b main
git --literal-pathspecs add --force --pathspec-from-file=SOURCE_FILES.txt
git diff --cached --stat
git commit -m "Publish modified Wang emulator source, notices and build instructions"
git remote add origin https://github.com/MistaJfelgo/SaveTheWANG-Web_Emulator.git
git push -u origin main
git tag -a source-v2.3.2 -m "Corresponding source for the recorded Wang browser runtime"
git push origin source-v2.3.2
```

The full source tree has many files; use Git/GitHub Desktop to publish it. The outer source ZIP is a transport artifact, not a file to commit into the repository. Upstream source is approximately 862 MiB uncompressed. Each included file is below GitHub's 100 MiB file limit; this does not guarantee every network/upload method will accept the full push at once.

4. Make a GitHub Release for the source tag. Attach the source ZIP if desired, or use GitHub's tag source download. Keep source tags/releases available for the deployed runtime versions; don't move a source tag to unrelated code.
5. Put the notice from `docs/gpl-notice.html` on the emulator project page or its legal/source section and link directly to this public repository/tag. Keep the GPL text and original MAME/third-party notices available with the program. The notice does not replace actual public source availability.
6. Keep `SOURCE_MANIFEST.json` with the release. Future WASM builds need corresponding source snapshots and updated hashes/build settings. A frontend-only version change can reuse the same runtime hash; document that relationship.

No repository has been created, committed, pushed or published by the preparation script. The GitHub URL becomes usable only after you create/publish the repository.

## Why the staging command is explicit

MAME's upstream .gitignore excludes some files that its own Git history already tracks. A fresh repository has no such tracking history. SOURCE_FILES.txt is an allowlist of this reviewed source export; the explicit command stages exactly those files, including COPYING and build inputs, while excluding any SDK installation or build output you may create later. Do not replace it with a blanket forced add of the parent workspace.

Modified-source hashes use canonical LF line endings. Git checkout policies may convert other upstream text files; the transported ZIP and its checksum are the exact publication artifact. The source manifest records the original compiled inputs separately from the publication comments.
