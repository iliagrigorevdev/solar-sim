#!/bin/bash

# Format C++ files
clang-format -i -style=file *.cpp *.h

emcc --bind simulation.cpp renderer.cpp quadtree.cpp -o public/simulation.js -std=c++14 -s FULL_ES3=1 -s MAX_WEBGL_VERSION=2 --preload-file shader.vert --preload-file shader.frag
