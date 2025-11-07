#!/bin/bash
## NOT RECCOMENDED FOR USAGE, THIS JUST EXISTS AS MY FEDORA LAPTOP IS YET TO HAVE CMAKE 4 ON REPOSITORIES.
~/cmake-4.1.2-linux-x86_64/bin/cmake -Bcmake-build -S. -DCMAKE_BUILD_TYPE=Debug
~/cmake-4.1.2-linux-x86_64/bin/cmake --build cmake-build
./cmake-build/grblang
