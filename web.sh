#!/bin/bash
mkdir -p bin

EMSDK=~/Documents/projects/external/emsdk
source "${EMSDK}/emsdk_env.sh"

emcc simulation.cpp renderer.cpp -o bin/simulation.html \
    -std=c++11 \
    -s USE_SDL=2 \
    -s FULL_ES2=1 \
    --preload-file shader.vert \
    --preload-file shader.frag

python -m http.server --directory bin
