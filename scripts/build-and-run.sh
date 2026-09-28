#!/usr/bin/env bash
set -euo pipefail
mkdir -p build
cd build
cmake -S ../ -B . -D CMAKE_BUILD_TYPE="${1:-Debug}" -G Ninja
cmake --build . -j
./tests/tests

cd ..

