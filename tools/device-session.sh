#!/bin/bash
# Explicit hardware verification on muOS; never runs in normal gameplay or CI.
set -eu
cd /mnt/mmc/ports/last-beacon
if pgrep -f '^\./nightfall.aarch64|^\./district.aarch64|^\./arena-client' >/dev/null; then
 echo 'Another game owns the screen; retry after it exits.' >&2
 exit 3
fi
menu_pids=$(ps -eo pid,comm | awk '$2 ~ /^mux/ && $2 !~ /muxrtc|muxwebserv/ {print $1}')
saver_pid=$(ps -eo pid,args | awk '$2 == "python3" && $3 == "/mnt/mmc/ports/ocean-dreams/saver.py" {print $1}')
for p in $menu_pids $saver_pid; do kill -STOP "$p" 2>/dev/null || true; done
game_pid=''
record_pid=''
finish(){
 [ -z "$record_pid" ] || { kill -INT "$record_pid" 2>/dev/null || true; wait "$record_pid" 2>/dev/null || true; }
 if [ -n "$game_pid" ]; then
  for child in $(pgrep -P "$game_pid" -f '^./last-beacon ' || true); do kill -TERM "$child" 2>/dev/null || true; done
  kill -TERM "$game_pid" 2>/dev/null || true
  wait "$game_pid" 2>/dev/null || true
 fi
 for p in $menu_pids $saver_pid; do kill -CONT "$p" 2>/dev/null || true; done
}
trap finish EXIT
trap 'exit 130' INT TERM
if [ "${1:-controls}" = controls ]; then
 rm -f evidence/live.json
 XDG_RUNTIME_DIR=/run pw-record --target alsa_output._sys_devices_platform_soc_soc_03000000_codec_mach_sound_card0.stereo-fallback -P '{"stream.capture.sink":true}' --rate 44100 --channels 2 evidence/audio.wav > evidence/audio-record.log 2>&1 &
 record_pid=$!
 bash /mnt/mmc/ROMS/PORTS/LastBeacon.sh --audit evidence/live.json --report evidence/controls-final.json --frames 2400 &
 game_pid=$!
 for attempt in $(seq 1 40); do [ ! -f evidence/live.json ] || break; sleep .1; done
 python3 hardware_controls.py
 wait "$game_pid"
 game_pid=''
else
 bash /mnt/mmc/ROMS/PORTS/LastBeacon.sh --demo --level "${2:-3}" --frames 1800 --report evidence/performance.json &
 game_pid=$!
 sleep 5
 fbgrab evidence/device-expedition.png
 # Read this game's memory only.
 beacon_pid=$(pgrep -f '^./last-beacon ' | head -n 1)
 grep -E 'VmRSS|VmHWM' "/proc/$beacon_pid/status" > evidence/memory.txt
 wait "$game_pid"
 game_pid=''
fi
