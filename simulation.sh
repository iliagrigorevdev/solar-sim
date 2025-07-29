#!/bin/bash
mkdir -p bin
g++ simulation.cpp renderer.cpp -o bin/simulation -std=c++11 -lSDL2 -lGLESv2
./bin/simulation
