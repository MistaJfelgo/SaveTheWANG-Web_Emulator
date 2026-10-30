#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-or-later
# Save the Wang source build helper, 2026-10-30.
set -euo pipefail
project_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
sdk="${WANG_SDK_SHORT:-${EMSDK:-$project_root/tools/emsdk}}"
source "$sdk/emsdk_env.sh"
export EMSDK="$sdk"
export EM_CONFIG="$sdk/.emscripten"
if [[ -n "${WANG_WEB_CACHE:-}" ]]; then export EM_CACHE="$WANG_WEB_CACHE"; fi
cd "$project_root/mame"
emmake make SUBTARGET=wangweb SOURCES=src/mame/wang/wangpc.cpp REGENIE=1 PYTHON_EXECUTABLE=python CLANG_VERSION=23.0.0 OPTIMIZE=2 SYMBOLS=0 NO_USE_MIDI=1 NO_USE_PORTAUDIO=1 NO_USE_PULSEAUDIO=1 NO_USE_PIPEWIRE=1 -j"${WANG_WEB_JOBS:-2}"
mkdir -p "$project_root/web/runtime"
cp wangweb.js wangweb.wasm "$project_root/web/runtime/"
python "$project_root/scripts/record-build.py"
