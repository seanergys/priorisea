#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname "$0")/.."

rm -r build

cmake -S . -B build -DENABLE_COVERAGE=ON

cmake --build build -j

cmake --build build --target docs
