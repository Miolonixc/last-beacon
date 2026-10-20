#!/usr/bin/env bash
# Optional native cross build when Docker is unavailable. System SDL2 is NOT distributed.
set -euo pipefail
cd "$(dirname "$0")/.."
sdl_library=${1:-build/sysroot/libSDL2.so}
include_dir=${SDL2_INCLUDE_DIR:-$(sdl2-config --prefix)/include/SDL2}
[ -f "$sdl_library" ] || { echo 'Pass the muOS AArch64 libSDL2 shared library as argument 1.' >&2; exit 1; }
mkdir -p build/muos
flags=(-target aarch64-linux-gnu.2.31 -mcpu=cortex_a53 -O2 -s -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wno-nullability-completeness)
zig c++ "${flags[@]}" -I"$include_dir" src/main.cpp "$sdl_library" -Wl,--allow-shlib-undefined -o build/muos/last-beacon
zig c++ "${flags[@]}" -UNDEBUG tests/world_test.cpp -o build/muos/world-test
printf 'Zig %s; aarch64-linux-gnu.2.31; cortex_a53; dynamic system SDL2; static C++ runtime\n' "$(zig version)" > build/muos/compiler.txt
printf 'Built build/muos/last-beacon and world-test. Run world-test on the device.\n'
