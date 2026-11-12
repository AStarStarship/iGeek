#!/usr/bin/env python3
# Copyright AStarship <https://astarship.net>.
"""Generate the original, deterministic Cards World vector deck (stdlib only)."""
from pathlib import Path

Ranks = ('A', '2', '3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K')
Suits = {
  'c': ('clubs', '#17392f', 'M0,-5 C-14,-22 -27,-6 -14,4 C-29,6 -20,25 -5,13 L-8,25 L8,25 L5,13 C20,25 29,6 14,4 C27,-6 14,-22 0,-5 Z'),
  'd': ('diamonds', '#ab303c', 'M0,-27 L19,0 L0,27 L-19,0 Z'),
  'h': ('hearts', '#ab303c', 'M0,24 C-8,14 -27,0 -22,-14 C-18,-28 -3,-25 0,-14 C3,-25 18,-28 22,-14 C27,0 8,14 0,24 Z'),
  's': ('spades', '#17392f', 'M0,-27 C-7,-16 -26,-4 -22,10 C-19,22 -7,21 -3,12 L-8,26 L8,26 L3,12 C7,21 19,22 22,10 C26,-4 7,-16 0,-27 Z'),
}
Layouts = {
  1: [(120, 168)],
  2: [(120, 88), (120, 248)],
  3: [(120, 88), (120, 168), (120, 248)],
  4: [(74, 88), (166, 88), (74, 248), (166, 248)],
  5: [(74, 88), (166, 88), (120, 168), (74, 248), (166, 248)],
  6: [(x, y) for x in (74, 166) for y in (88, 168, 248)],
  7: [(x, y) for x in (74, 166) for y in (88, 168, 248)] + [(120, 128)],
  8: [(x, y) for x in (74, 166) for y in (88, 168, 248)] + [(120, 128), (120, 208)],
  9: [(x, y) for x in (74, 166) for y in (78, 138, 198, 258)] + [(120, 168)],
  10: [(x, y) for x in (74, 166) for y in (78, 138, 198, 258)] + [(120, 108), (120, 228)],
}


def SuitUse(x, y, scale: float = 1.0, rotate=0):
  return f'<use href="#suit" transform="translate({x} {y}) rotate({rotate}) scale({scale})"/>'


def Court(rank, color):
  """Two-headed geometric portraits; distinct cap, crown and regalia."""
  crowns = {
    'J': '<path d="M83 92 L92 65 L140 73 L151 91 Z" fill="#17392f"/><path d="M130 73 L150 51 L145 79" fill="#b18a44"/>',
    'Q': '<path d="M85 91 L80 66 L103 78 L120 56 L137 78 L160 66 L155 91 Z" fill="#b18a44"/><circle cx="120" cy="67" r="5" fill="#ab303c"/>',
    'K': '<path d="M85 91 L83 60 L105 74 L120 48 L135 74 L157 60 L155 91 Z" fill="#b18a44"/><path d="M112 61 H128 M120 53 V70" stroke="#fff9e9" stroke-width="3"/>',
  }
  portrait = (
    '<path d="M60 168 L78 139 L107 129 L134 129 L163 139 L180 168 Z" fill="' + color + '"/>'
    '<path d="M85 168 L104 132 L120 149 L137 132 L156 168" fill="#b18a44"/>'
    '<path d="M97 88 L143 88 L141 119 L122 138 L101 122 Z" fill="#ead7b1"/>'
    '<path d="M97 90 L97 119 L106 125 L106 93 M143 90 L143 118 L135 125 L135 93" fill="#17392f"/>'
    '<path d="M108 105 H115 M127 105 H134 M120 108 L117 118 H123 M114 125 H128" fill="none" stroke="#17392f" stroke-width="2.5"/>'
    + crowns[rank] +
    '<path d="M67 162 L80 146 M160 146 L173 162" stroke="#fff9e9" stroke-width="4"/>'
  )
  return (
    '<rect x="53" y="48" width="134" height="240" rx="4" fill="#f0e7cf" stroke="#b18a44"/>'
    f'<g>{portrait}</g><g transform="rotate(180 120 168)">{portrait}</g>'
    '<path d="M57 168 H183" stroke="#b18a44" stroke-width="3"/>'
    + SuitUse(120, 168, 0.5)
  )


def Face(rank, suit):
  name, color, path = Suits[suit]
  label = {'A': 'Ace', 'J': 'Jack', 'Q': 'Queen', 'K': 'King'}.get(rank, rank)
  corner = (
    f'<text x="25" y="35" text-anchor="middle" font-family="Georgia,serif" font-size="27" font-weight="bold">{rank}</text>'
    + SuitUse(25, 55, 0.39)
  )
  if rank in ('J', 'Q', 'K'):
    art = Court(rank, color)
  else:
    number = 1 if rank == 'A' else int(rank)
    art = ''.join(SuitUse(x, y, 1.65 if number == 1 else 0.72, 180 if y > 168 else 0)
                  for x, y in Layouts[number])
  return (
    '<?xml version="1.0" encoding="UTF-8"?>\n'
    '<!-- Copyright AStarship <https://astarship.net>. -->\n'
    '<svg xmlns="http://www.w3.org/2000/svg" width="240" height="336" viewBox="0 0 240 336" role="img" aria-labelledby="title">'
    f'<title id="title">{label} of {name}</title><defs><path id="suit" d="{path}"/></defs>'
    '<rect x="1" y="1" width="238" height="334" rx="15" fill="#fffaf0" stroke="#d6c6a2" stroke-width="2"/>'
    '<rect x="8" y="8" width="224" height="320" rx="10" fill="none" stroke="#e9dfc9"/>'
    f'<g fill="{color}">{corner}<g transform="rotate(180 120 168)">{corner}</g>{art}</g></svg>\n'
  )


def Back():
  return '''<?xml version="1.0" encoding="UTF-8"?>
<!-- Copyright AStarship <https://astarship.net>. -->
<svg xmlns="http://www.w3.org/2000/svg" width="240" height="336" viewBox="0 0 240 336" role="img" aria-labelledby="title">
<title id="title">Face-down playing card</title>
<defs><pattern id="weave" width="24" height="24" patternUnits="userSpaceOnUse"><path d="M12 0 L24 12 L12 24 L0 12 Z" fill="none" stroke="#b18a44" stroke-width="0.8"/><circle cx="12" cy="12" r="1.5" fill="#b18a44"/></pattern></defs>
<rect x="1" y="1" width="238" height="334" rx="15" fill="#fffaf0" stroke="#d6c6a2" stroke-width="2"/>
<rect x="11" y="11" width="218" height="314" rx="9" fill="#123c31"/>
<rect x="18" y="18" width="204" height="300" rx="5" fill="url(#weave)" stroke="#d3b777" stroke-width="2"/>
<path d="M120 79 L184 168 L120 257 L56 168 Z" fill="#123c31" stroke="#d3b777" stroke-width="3"/>
<path d="M120 95 L171 168 L120 241 L69 168 Z" fill="none" stroke="#b18a44"/>
<path d="M120 119 L131 154 L158 168 L131 182 L120 217 L109 182 L82 168 L109 154 Z" fill="#d3b777"/>
<path d="M120 145 L139 168 L120 191 L101 168 Z" fill="#123c31"/>
</svg>
'''


def Generate():
  output = Path(__file__).resolve().parent / 'assets' / 'cards'
  output.mkdir(parents=True, exist_ok=True)
  for suit in Suits:
    for rank in Ranks:
      (output / f'{rank}{suit}.svg').write_text(Face(rank, suit), encoding='utf-8')
  (output / 'back.svg').write_text(Back(), encoding='utf-8')
  print(f'Generated 52 faces + back.svg in {output}')


if __name__ == '__main__':
  Generate()
