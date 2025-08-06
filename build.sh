#!/bin/bash
emcc --bind simulation.cpp renderer.cpp quadtree.cpp -o public/simulation.js -std=c++14 -s FULL_ES2=1 -s MAX_WEBGL_VERSION=1 --preload-file shader.vert --preload-file shader.frag
