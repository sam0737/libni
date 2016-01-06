#!/bin/bash
set -euo pipefail

if [ $# -lt 1 ]; then
    echo "Usage: $0 <CMake build directory> <extra flags>..."
    exit 1
fi

BUILD_DIR="$1"

if [ -z "$1" ] || [ ! -d "$BUILD_DIR" ]; then
    echo "$BUILD_DIR does not exist"
    exit 1
fi

if hash ctest 2>/dev/null; then
    cd "$BUILD_DIR"
    ctest "${@:2}"
else
    echo "CTest not found in path."
    for test in $(find "$BUILD_DIR/test" -type f -executable); do
        echo "Runing $test..."
        "./$test" "${@:2}"
    done
fi
