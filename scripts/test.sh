#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname "$0")/.."

export DEMOBUS_BIN=$(command -v demobus)
export NAMING_SERVER_BIN=$(command -v naming_server)

if [ "$#" -eq 0 ]; then
  ctest --test-dir build --parallel 0 --output-on-failure -V
  exit 0
fi

names=()
for path in "$@"; do
  names+=("$(basename "$path")")
done
regex=$(IFS='|'; echo "${names[*]}")

ctest --test-dir build --parallel 0 --output-on-failure -R "$regex" -V
