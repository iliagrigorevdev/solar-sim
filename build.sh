#!/bin/bash
emcc --bind simulation.cpp renderer.cpp -o docs/simulation.js -std=c++11 -s FULL_ES3=1 -s MAX_WEBGL_VERSION=2 --preload-file shader.vert --preload-file shader.frag
