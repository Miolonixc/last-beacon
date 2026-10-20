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
for license in assets/fonts/*-OFL.txt licenses/*.txt; do
 install -m 644 "$license" "$stage/ports/last-beacon/licenses/$(basename "$license")"
done
cp packaging/LastBeacon.sh "$stage/ROMS/PORTS/"
chmod +x "$stage/ROMS/PORTS/LastBeacon.sh" "$stage/ports/last-beacon/last-beacon"
tar -czf "dist/last-beacon-$version-muos.tar.gz" -C "$stage" ports ROMS
(cd dist && shasum -a 256 "last-beacon-$version-muos.tar.gz" > SHA256SUMS)
printf 'Package: dist/last-beacon-%s-muos.tar.gz\n' "$version"
