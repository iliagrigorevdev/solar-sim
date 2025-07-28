#!/bin/bash
mkdir -p bin
g++ simulation.cpp -o bin/simulation -std=c++11
./bin/simulation
