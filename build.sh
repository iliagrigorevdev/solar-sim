#!/bin/bash
emcc --bind simulation.cpp renderer.cpp -o docs/simulation.js -std=c++11 -s FULL_ES2=1 --preload-file shader.vert --preload-file shader.frag

python -m http.server --directory docs
