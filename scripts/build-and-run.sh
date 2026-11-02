#!/usr/bin/env bash

set -euo pipefail

BUILD_DIR="build"
BUILD_TYPE="${1:-Debug}"

mkdir -p "${BUILD_DIR}"

cmake \
    -S . \
    -B "${BUILD_DIR}" \
    -D CMAKE_BUILD_TYPE="${BUILD_TYPE}"

cmake --build "${BUILD_DIR}" -j

ctest \
    --test-dir "${BUILD_DIR}/tests" \
    --output-on-failure
