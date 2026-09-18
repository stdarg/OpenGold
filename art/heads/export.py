#!/usr/bin/env python3
"""Export the reviewed native head pixels in heads.json. Requires Pillow 10.1+."""

import argparse
import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parent
EGA = (
    (0, 0, 0), (0, 0, 170), (0, 170, 0), (0, 170, 170),
    (170, 0, 0), (170, 0, 170), (170, 85, 0), (170, 170, 170),
    (85, 85, 85), (85, 85, 255), (85, 255, 85), (85, 255, 255),
    (255, 85, 85), (255, 85, 255), (255, 255, 85), (255, 255, 255),
)


def require(condition, message):
    if not condition:
        raise ValueError(message)


def component_image(catalog, head, size):
    width, height = catalog['canvases'][size]
    component = head['sizes'][size]
    x, y = component['offset']
    rows = component['rows']
    require(rows and len({len(row) for row in rows}) == 1, 'Unequal pixel rows')
    require(0 <= x and x + len(rows[0]) <= width, 'Head exceeds canvas width')
    require(0 <= y and y + len(rows) == height, 'Neck must reach canvas bottom')
    pixels = bytearray(width * height)
    for dy, row in enumerate(rows):
        for dx, char in enumerate(row):
            pixels[(y + dy) * width + x + dx] = catalog['pixel_legend'][char]
    require(set(pixels) == {0, 4, 8, 12}, 'Missing or invalid semantic colors')
    require(pixels[-width + 11] == pixels[-width + 12] == 12,
            'Neck must join the body at x=11 and x=12')
    palette = list(EGA)
    palette[4] = EGA[head['default_feature_color']]
    palette[8] = (0, 0, 0)
    palette[12] = EGA[head['default_skin_color']]
    image = Image.frombytes('P', (width, height), bytes(pixels))
    image.putpalette([channel for rgb in palette for channel in rgb])
    image.info['transparency'] = 0
    if head['species'] == 'goliath':
        expected = (10, 2, 15, 8) if size == 'tall' else (9, 5, 16, 10)
        require(image.convert('RGBA').getbbox() == expected,
                'Goliath visible bounds must equal the human reference')
    return image


def review_sheet(heads, components):
    # Original head artwork only; no extracted game bodies in this export.
    sheet = Image.new('RGB', (1000, 800), '#20262f')
    draw = ImageDraw.Draw(sheet)
    title = ImageFont.load_default(size=22)
    font = ImageFont.load_default(size=15)
    small = ImageFont.load_default(size=12)
    draw.text((20, 14), 'Combat heads - native components', font=title, fill='white')
    draw.text((20, 46), '10x nearest-neighbor previews; 1x samples above. Ready/action heads match.',
              font=font, fill='#c8d3df')
    for column in range(4):
        size = 'tall 24x8' if column % 2 == 0 else 'short 24x10'
        draw.text((20 + column * 245, 79), f'Variant {column // 2 + 1} / {size}',
                  font=font, fill='#c8d3df')
    for index, head in enumerate(heads):
        top = 110 + (index // 2) * 132
        for size_index, size in enumerate(('tall', 'short')):
            left = 12 + (index % 2 * 2 + size_index) * 245
            draw.text((left + 6, top), head['id'], font=font, fill='white')
            component = components[(head['id'], size)].convert('RGBA')
            for y in range(10):
                for x in range(24):
                    color = '#929eaa' if (x + y) % 2 else '#a4b0bc'
                    draw.rectangle((left + x*10, top + 22 + y*10,
                                    left + x*10 + 9, top + 31 + y*10), fill=color)
            enlarged = component.resize((240, component.height * 10), Image.Resampling.NEAREST)
            sheet.paste(enlarged, (left, top + 22), enlarged)
            # Native sample is separated from the enlarged component.
            draw.rectangle((left + 185, top + 4, left + 235, top + 18), fill='#a4b0bc')
            sheet.paste(component, (left + 198, top + 6), component)
    draw.text((20, 778), 'Goliath visible footprint: tall 5x6; short 7x5 - same as the human reference.',
              font=small, fill='#c8d3df')
    return sheet


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true', help='Validate existing exports without writing')
    args = parser.parse_args()
    catalog = json.loads((ROOT / 'heads.json').read_text())
    require(catalog['canvases'] == {'tall': [24, 8], 'short': [24, 10]}, 'Unexpected canvases')
    require(catalog['pixel_legend'] == {'.': 0, 'f': 4, '#': 8, 's': 12}, 'Unexpected indices')
    heads = catalog['heads']
    require(len(heads) == len({head['id'] for head in heads}) == 10, 'Expected ten unique heads')
    species = ('tiefling', 'goliath', 'gnome', 'dragonborn', 'orc')
    require(all(sum(head['species'] == s for head in heads) == 2 for s in species),
            'Expected two heads for each species')
    components = {}
    filenames = set()
    for head in heads:
        require((ROOT / head['source']).is_file(), 'Missing source sheet')
        for size in ('tall', 'short'):
            expected = component_image(catalog, head, size)
            components[(head['id'], size)] = expected
            for pose in ('ready', 'action'):
                filename = head['sizes'][size][pose]
                require(filename == f"{head['id']}-{size}-{pose}.png", 'Unexpected export path')
                require(filename not in filenames, 'Duplicate export path')
                filenames.add(filename)
                path = ROOT / filename
                if not args.check:
                    expected.save(path, bits=4, transparency=0, optimize=False)
                with Image.open(path) as actual:
                    require(actual.mode == 'P' and actual.size == expected.size,
                            f'{filename}: invalid indexed canvas')
                    require(actual.tobytes() == expected.tobytes(), f'{filename}: pixels differ')
                    require(actual.convert('RGBA').tobytes() == expected.convert('RGBA').tobytes(),
                            f'{filename}: palette or transparency differs')
    for size in ('tall', 'short'):
        require(len({components[(h['id'], size)].tobytes() for h in heads}) == 10,
                f'{size}: two designs have identical pixels')
    require({p.name for p in ROOT.glob('*-*-*.png')} == filenames, 'Unexpected component files')
    if not args.check:
        review_sheet(heads, components).save(ROOT / 'review.png')
    print('Verified 40 indexed components, ten distinct designs per size, neck alignment, '
          'human-sized Goliaths, transparency, palettes, and ready/action consistency.')


if __name__ == '__main__':
    main()
