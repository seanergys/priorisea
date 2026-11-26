#!/usr/bin/env bash

set -euo pipefail

export SEANERBROKER_BIN=$(command -v seanerbroker)
export NAMING_SERVER_BIN=$(command -v naming_server)

cd "$(dirname "$0")/.."

rm -r build

cmake -S . -B build -DENABLE_COVERAGE=ON

cmake --build build -j

cmake --build build --target docs
