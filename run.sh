#!/bin/bash
cmake -Bcmake-build -S. -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build
./cmake-build/grblang
