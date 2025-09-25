#!/bin/bash

if [[ $1 == "--clean" ]] then
    rm -rf build
    mkdir build
    cmake -S . -B build
    cmake --build build
    ./build/snake
else
    if cmake --build build; then
        ./build/snake
    fi
fi
