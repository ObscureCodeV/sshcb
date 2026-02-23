#!/usr/bin/env bash
set -e

echo "🪟 Building for Windows..."

rm -rf build

cmake -B build \
  -DCMAKE_C_COMPILER="zig" \
  -DCMAKE_C_COMPILER_ARG1="cc" \
  -DCMAKE_C_FLAGS="-target x86_64-windows-gnu" \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5

cmake --build build

echo "✅ Done! Output: build/app.exe"
