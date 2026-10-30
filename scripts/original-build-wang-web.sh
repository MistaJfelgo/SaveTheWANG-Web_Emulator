#!/usr/bin/env bash
set -e
source /v/tools/emsdk/emsdk_env.sh
export EMSDK="${WANG_SDK_SHORT:-${WANG_SDK_SHORT:?Set WANG_SDK_SHORT}}"
export EM_CONFIG="$EMSDK/.emscripten"
export EM_CACHE="${WANG_WEB_CACHE:-${WANG_WEB_CACHE:?Set WANG_WEB_CACHE}}"
cd /v/mame
emmake make SUBTARGET=wangweb SOURCES=src/mame/wang/wangpc.cpp REGENIE=1 PYTHON_EXECUTABLE=python CLANG_VERSION=23.0.0 OPTIMIZE=2 SYMBOLS=0 NO_USE_MIDI=1 NO_USE_PORTAUDIO=1 NO_USE_PULSEAUDIO=1 NO_USE_PIPEWIRE=1 -j${WANG_WEB_JOBS:-2}
