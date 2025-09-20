#!/bin/bash

if [[ $1 == "--clean" ]] then
    rm -rf build
    mkdir build
    cmake -S . -B build
    cmake --build build
    ./build/snake
else
    cmake --build build
    ./build/snake
fi
