#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
[ "${BEACON_SKIP_BUILD:-0}" = 1 ] || bash tools/build-muos.sh
version=$(cat VERSION)
stage=build/package
mkdir -p "$stage/ports/last-beacon" "$stage/ROMS/PORTS" dist
cp build/muos/last-beacon "$stage/ports/last-beacon/"
cp README.md LICENSE VERSION "$stage/ports/last-beacon/"
cp -R docs "$stage/ports/last-beacon/"
mkdir -p "$stage/ports/last-beacon/licenses"
cp assets/fonts/*-OFL.txt "$stage/ports/last-beacon/licenses/"
cp licenses/*.txt "$stage/ports/last-beacon/licenses/"
cp packaging/LastBeacon.sh "$stage/ROMS/PORTS/"
chmod +x "$stage/ROMS/PORTS/LastBeacon.sh" "$stage/ports/last-beacon/last-beacon"
tar -czf "dist/last-beacon-$version-muos.tar.gz" -C "$stage" ports ROMS
(cd dist && shasum -a 256 "last-beacon-$version-muos.tar.gz" > SHA256SUMS)
printf 'Package: dist/last-beacon-%s-muos.tar.gz\n' "$version"
