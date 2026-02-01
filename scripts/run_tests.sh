#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build"

mkdir -p "$BUILD_DIR"

cmake -S "$ROOT_DIR" -B "$BUILD_DIR"

cmake --build "$BUILD_DIR" --target test_thread_safe_queue test_thread_pool

"$BUILD_DIR/test_thread_safe_queue"
"$BUILD_DIR/test_thread_pool"
