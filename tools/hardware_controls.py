#!/usr/bin/env python3
"""Explicit muOS diagnostic: run only while Last Beacon exclusively owns the display."""
import json
import math
import os
import struct
import subprocess
import time
from pathlib import Path

root = Path('/mnt/mmc/ports/last-beacon')
audit = root / 'evidence/live.json'
if not audit.exists() or time.time() - audit.stat().st_mtime > 3:
    raise SystemExit('Start LastBeacon.sh --audit evidence/live.json first; audit must be fresh.')
fd = os.open('/dev/input/event1', os.O_WRONLY | os.O_NONBLOCK)

def event(kind, code, value):
    now = time.time()
    stamp = (int(now), int((now % 1) * 1000000))
    os.write(fd, struct.pack('llHHi', *stamp, kind, code, value))
    os.write(fd, struct.pack('llHHi', *stamp, 0, 0, 0))

def button(code):
    event(1, code, 1)
    time.sleep(.12)
    event(1, code, 0)
    time.sleep(.25)

def axis(code, value, duration=.15):
    end = time.monotonic() + duration
    while time.monotonic() < end:
        event(3, code, value)
        time.sleep(.003)
    event(3, code, 0)
    time.sleep(.25)

def state():
    return json.loads(audit.read_text())

def expect(phase):
    end = time.monotonic() + 2
    while time.monotonic() < end:
        current = state()
        if current['phase'] == phase:
            return current
        time.sleep(.1)
    raise AssertionError(('phase', phase, current))

def capture(name):
    subprocess.run(['fbgrab', str(root / 'evidence' / name)], check=True, capture_output=True)

try:
    expect(0)
    capture('device-menu.png')
    axis(16, 1)
    assert state()['mission'] == 1
    button(304)  # Locked expedition must stay in the menu.
    expect(0)
    axis(16, -1)
    assert state()['mission'] == 0
    button(311)  # Start must begin the expedition, not exit the menu.
    before = expect(1)
    axis(17, 1, .45)
    dpad = state()
    assert math.hypot(dpad['x']-before['x'], dpad['y']-before['y']) > .4
    axis(2, 3000, .35)
    stick = state()
    assert math.hypot(stick['x']-dpad['x'], stick['y']-dpad['y']) > .1
    button(305)  # B dash.
    assert state()['cooldown'] > .7
    capture('device-play.png')
    button(311)
    paused = expect(2)
    capture('device-pause.png')
    axis(17, 1, .3)
    assert abs(state()['time'] - paused['time']) < .001
    button(305)
    expect(1)
    button(311)
    expect(2)
    button(310)
    expect(0)
    button(307)  # X sound toggle; toggle back so the diagnostic ends with sound enabled.
    button(307)
    final = state()
    assert final['controller'] and final['audio'] and final['notes'] > 0
    result = {'passed': True, 'dpad_distance': math.hypot(dpad['x']-before['x'], dpad['y']-before['y']),
              'stick_distance': math.hypot(stick['x']-dpad['x'], stick['y']-dpad['y']), 'state': final}
    (root / 'evidence/controls.json').write_text(json.dumps(result, indent=2)+'\n')
    button(310)  # Select exits from title back to Ports.
    print(json.dumps(result))
finally:
    for code in (2, 3, 16, 17):
        event(3, code, 0)
    for code in (304, 305, 307, 310, 311):
        event(1, code, 0)
    os.close(fd)
