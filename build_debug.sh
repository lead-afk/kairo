#!/usr/bin/env bash
set -e

if ! command -v g++-14 >/dev/null 2>&1; then
  echo "g++-14 not found. Install GCC 14 (g++-14) to build this C++23 project." >&2
  echo "   Ubuntu/Debian: sudo apt-get install g++-14" >&2
  exit 1
fi

if ! command -v gcc-14 >/dev/null 2>&1; then
  echo "gcc-14 not found. Install GCC 14 (gcc-14) to build this project." >&2
  echo "   Ubuntu/Debian: sudo apt-get install gcc-14" >&2
  exit 1
fi

cmake -S . -B build_debug \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/gcc14-vcpkg.cmake

cmake --build build_debug
mv build_debug/kairo ./kairo-debug
