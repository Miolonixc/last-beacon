#!/bin/bash
# Last Beacon / RG40XX H / muOS. System SDL2 supplies the Mali driver.
controlfolder=''
for candidate in /opt/system/Tools/PortMaster /opt/tools/PortMaster "${XDG_DATA_HOME:-$HOME/.local/share}/PortMaster" /mnt/mmc/MUOS/PortMaster /mnt/sdcard/MUOS/PortMaster /roms/ports/PortMaster; do
 if [ -f "$candidate/control.txt" ]; then controlfolder=$candidate; break; fi
done
if [ -n "$controlfolder" ]; then
 source "$controlfolder/control.txt"
 [ ! -f "$controlfolder/mod_${CFW_NAME}.txt" ] || source "$controlfolder/mod_${CFW_NAME}.txt"
 get_controls
 export SDL_GAMECONTROLLERCONFIG="$sdl_controllerconfig"
fi
if [ -d /opt/muos ] && [ -e /dev/fb0 ] && [ ! -e /dev/dri/card0 ]; then
 export SDL_VIDEODRIVER="${SDL_VIDEODRIVER:-mali}"
 export SDL_BLITTER_DISABLED=1 SDL_ROTATION=0 SDL_HQ_SCALER=0
fi
[ ! -S /run/pipewire-0 ] || export XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-/run}"
scriptdir=$(cd -- "$(dirname -- "$0")" && pwd)
GAMEDIR="${BEACON_PORT_DIR:-$scriptdir/last-beacon}"
if [ ! -d "$GAMEDIR" ]; then
 for base in /mnt/mmc/ports /mnt/sdcard/ports /roms/ports; do
  [ ! -d "$base/last-beacon" ] || { GAMEDIR="$base/last-beacon"; break; }
 done
fi
cd "$GAMEDIR" || exit 1
export BEACON_DATA="$GAMEDIR/save"
mkdir -p "$BEACON_DATA"
exec >> last-beacon.log 2>&1
finish(){ [ -z "$controlfolder" ] || pm_finish; }
trap finish EXIT
./last-beacon --fullscreen "$@"
status=$?
exit "$status"
