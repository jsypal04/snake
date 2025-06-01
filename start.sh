#!/bin/bash

if [[ $1 == "--clean" ]] then
    source .env
    rm -rf build
    mkdir build
    cmake -S . -B build
    cmake --build build
    ./build/snake
else
    source .env
    cmake --build build
    ./build/snake
fi


