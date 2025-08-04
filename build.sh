#!/bin/bash
mkdir -p bin

emcc --bind simulation.cpp renderer.cpp -o bin/simulation.js     -std=c++11     -s FULL_ES2=1     --preload-file shader.vert     --preload-file shader.frag

cp index.html bin/index.html

python -m http.server --directory bin
