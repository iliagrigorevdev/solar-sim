#!/bin/bash
mkdir -p bin
g++ simulation.cpp renderer.cpp -o bin/simulation -std=c++11 `pkg-config --cflags --static --libs glfw3` -lGLESv2
./bin/simulation
