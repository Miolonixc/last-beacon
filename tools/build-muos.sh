#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
image=$(cat toolchain/image.lock)
docker run --rm --platform linux/arm64 --mount "type=bind,source=$PWD,target=/workspace" \
 --workdir /workspace --entrypoint /bin/bash "$image" -c '
set -euo pipefail
make OUT=build/muos CXXFLAGS="-O2 -std=c++17 -Wall -Wextra -Wpedantic -Werror -mcpu=cortex-a53 -static-libstdc++ -static-libgcc" SDL_LIBS="-L/usr/local/lib -lSDL2 -pthread -lm -ldl" all test
readelf -d build/muos/last-beacon > build/muos/dependencies.txt
readelf --version-info build/muos/last-beacon > build/muos/symbol-versions.txt
SDL_VIDEODRIVER=dummy build/muos/last-beacon --screenshot build/muos/play.bmp
'
