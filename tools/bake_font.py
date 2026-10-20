#!/usr/bin/env python3
"""Rebuild the Harbor glyph proof sheet; Python standard library only."""
from pathlib import Path
import re
root = Path(__file__).resolve().parents[1]
source = (root / 'assets/fonts/legacy/harbor.hpp').read_text()
glyphs = re.findall(r"\{'(.)',\{([0-9,]+)\}\}", source)
svg = ['<svg xmlns="http://www.w3.org/2000/svg" width="576" height="240" viewBox="0 0 144 60">', '<rect width="144" height="60" fill="#0c1926"/>']
for n, (char, rows) in enumerate(glyphs):
    for y, value in enumerate(map(int, rows.split(','))):
        for x in range(5):
            if value & (1 << (4-x)):
                svg.append(f'<rect x="{(n%16)*9+x+2}" y="{(n//16)*12+y+2}" width="1" height="1" fill="#f0e5c1"/>')
svg.append('</svg>')
(root / 'assets/fonts/legacy/harbor-atlas.svg').write_text('\n'.join(svg))
print(f'Baked {len(glyphs)} glyphs')
