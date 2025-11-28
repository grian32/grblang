#!/bin/bash
cmake -Bcmake-build -S.
cmake --build cmake-build
cp ./cmake-build/grblang ./tests/
