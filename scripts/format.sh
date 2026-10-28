#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname "$0")/.."

find src \( -name '*.cpp' -o -name '*.hpp' -o -name '*.c' -o -name '*.h' \) -exec clang-format -i {} +
