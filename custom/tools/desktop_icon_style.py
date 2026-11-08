"""Give icons the size and shadow of the Desktop icon (custom/icons/telegram).

The source app.png must be a circle filling the whole 256x256 canvas, like the
icons taken from Android. The circle is scaled to the Desktop circle radius,
the Desktop shadow is put under it, and app_icon.ico is rebuilt from the
result the same way the Desktop .ico is: every size is a Lanczos downscale.

Usage: python custom/tools/desktop_icon_style.py custom/icons/<name> [...]
Needs numpy and Pillow.
"""
import io
import struct
import sys
from pathlib import Path

import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parent.parent
DESKTOP = ROOT / 'icons' / 'telegram' / 'app.png'
SIZE = 256
SUPERSAMPLE = 4
ICO_SIZES = (16, 24, 32, 48, 64, 128, 256)


def desktop_shadow():
    """Shadow of the Desktop icon, including the part hidden under its edge."""
    desk = np.asarray(Image.open(DESKTOP).convert('RGBA')).astype(np.float64)
    n = SIZE
    center = n / 2.
    alpha = desk[..., 3] / 255.
    yy, xx = np.mgrid[0:n, 0:n]
    dx, dy = xx + .5 - center, yy + .5 - center
    r = np.hypot(dx, dy)
    r[r == 0] = 1
    ux, uy = dx / r, dy / r

    def along(radius):
        px = np.clip(np.round(center + ux * radius - .5).astype(int), 0, n - 1)
        py = np.clip(np.round(center + uy * radius - .5).astype(int), 0, n - 1)
        return py, px

    # Edge pixel = circle colour * coverage + black shadow under the rest.
    py, px = along(r - 3.5)
    inner = desk[py, px, 2]
    coverage = np.where(inner > 0, desk[..., 2] * alpha / np.maximum(inner, 1), 0)
    coverage = np.where(r > center, 0, np.clip(coverage, 0, 1))
    coverage = np.where(alpha >= 1, 1, coverage)
    shadow = np.where(
        (coverage < .999) & (alpha < 1),
        (alpha - coverage) / np.maximum(1 - coverage, 1e-6),
        np.nan)
    shadow = np.clip(shadow, 0, 1)
    for step in (1, 2, 3):
        py, px = along(r + step)
        fill = np.isnan(shadow) & (r > center - 8)
        shadow[fill] = shadow[py, px][fill]
    shadow = np.nan_to_num(shadow, nan=0.)
    radius = np.sqrt(coverage.sum() / np.pi)

    layer = np.zeros((n, n, 4), np.uint8)
    layer[..., 3] = np.clip(np.round(shadow * 255), 0, 255).astype(np.uint8)
    return Image.fromarray(layer, 'RGBA'), radius


def styled(source, shadow, radius):
    big = SIZE * SUPERSAMPLE
    diameter = int(round(radius * 2 * SUPERSAMPLE / 2.)) * 2
    offset = (big - diameter) // 2
    image = source.convert('RGBA').convert('RGBa')
    image = image.resize((big, big), Image.BILINEAR)
    canvas = Image.new('RGBa', (big, big), (0, 0, 0, 0))
    canvas.paste(image.resize((diameter, diameter), Image.BOX), (offset, offset))
    result = shadow.copy()
    result.alpha_composite(canvas.resize((SIZE, SIZE), Image.BOX).convert('RGBA'))
    return result


def png_bytes(image):
    out = io.BytesIO()
    image.save(out, 'PNG', optimize=True)
    return out.getvalue()


def write_ico(image, path):
    frames = [
        png_bytes(image if size == SIZE else image.resize((size, size), Image.LANCZOS))
        for size in ICO_SIZES
    ]
    header = struct.pack('<HHH', 0, 1, len(frames))
    offset = 6 + 16 * len(frames)
    entries = b''
    for size, data in zip(ICO_SIZES, frames):
        side = 0 if size == 256 else size
        entries += struct.pack('<BBBBHHII', side, side, 0, 0, 1, 32, len(data), offset)
        offset += len(data)
    path.write_bytes(header + entries + b''.join(frames))


def main(folders):
    shadow, radius = desktop_shadow()
    for folder in map(Path, folders):
        if folder.resolve() == DESKTOP.parent.resolve():
            continue
        result = styled(Image.open(folder / 'app.png'), shadow, radius)
        result.save(folder / 'app.png', optimize=True)
        write_ico(result, folder / 'app_icon.ico')
        print(f'{folder.name}: done')


if __name__ == '__main__':
    main(sys.argv[1:])
