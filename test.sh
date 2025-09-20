#!/bin/bash

arg=$1

if [[ $arg == "--help" ]] then
    echo "Script to run unit tests."
    echo "  ./test.sh -> runs all unit tests"
    echo "  ./test.sh --help -> shows this help menu"
fi

cmake --build build
./build/tests
