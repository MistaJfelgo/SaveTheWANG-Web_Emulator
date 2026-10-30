# Dependency source preservation

MAME's complete upstream source includes bundled dependencies under mame/3rdparty and their notices under mame/docs/legal and COPYING. No upstream copyright notices have been removed.

`emscripten/` contains the installed 6.0.2 source distribution: runtime JavaScript, libc/libc++, build/port recipes, compiler-driver sources and source assets. Generated caches, node_modules and executable toolchain shims are excluded. The actual LLVM/Node/Python host toolchain is installed with the pinned emsdk commands in BUILDING.md.

`port-sources/` contains the actual source archives downloaded for SDL, SDL_ttf, FreeType, HarfBuzz and zlib. They retain original license texts and source notices. SHA-256 values are recorded in SOURCE_MANIFEST.json; the archives also match the pinned port recipes' SHA-512 values. Those recipes describe the exact versions and upstream URLs. Archive filenames are local cache names and are not an independent version authority.

These archives are source, not compiled libraries. The wrapper does not silently fetch a different upstream library version. Installing the exact SDK permits its recipes to retrieve the same sources; no complete offline SDK installer is promised by this repository.
