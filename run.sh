#!/bin/bash
cmake -Bcmake-build -S.
cmake --build cmake-build
./cmake-build/grblang
