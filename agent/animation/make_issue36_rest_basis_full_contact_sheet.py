"""Assemble the full corrected source-vs-target technical contact sheet."""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


DEFAULT_FRAMES = (1, 6, 12, 17, 18, 20, 22, 27, 33, 39, 45, 52, 60)
PANEL = 300
LABEL = 34


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input-root", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--frames", nargs="*", type=int, default=DEFAULT_FRAMES)
    return parser.parse_args()


def fitted(path):
    image = Image.open(path).convert("RGB")
    image.thumbnail((PANEL, PANEL), Image.Resampling.LANCZOS)
    tile = Image.new("RGB", (PANEL, PANEL), (9, 13, 22))
    tile.paste(image, ((PANEL - image.width) // 2, (PANEL - image.height) // 2))
    return tile


def main():
    args = parse_args()
    frames = tuple(args.frames)
    columns = (("source", "front", "SOURCE FRONT"),
               ("coh", "front", "CORRECTED MALE FRONT"),
               ("source", "side", "SOURCE SIDE"),
               ("coh", "side", "CORRECTED MALE SIDE"))
    width = 120 + len(columns) * PANEL
    height = 70 + len(frames) * (PANEL + LABEL)
    sheet = Image.new("RGB", (width, height), (18, 22, 32))
    draw = ImageDraw.Draw(sheet)
    font = ImageFont.load_default()
    draw.text((18, 18), "Issue 36 corrected full rest-basis correspondence", fill=(245, 247, 250), font=font)
    for column, (_, _, title) in enumerate(columns):
        draw.text((120 + column * PANEL + 8, 46), title, fill=(252, 190, 76), font=font)
    for row, frame in enumerate(frames):
        y = 70 + row * (PANEL + LABEL)
        draw.text((18, y + PANEL // 2), f"f{frame:02d}", fill=(225, 230, 240), font=font)
        for column, (representation, view, _) in enumerate(columns):
            path = args.input_root / f"frame-{frame:02d}" / "visual" / representation / f"{view}.png"
            if not path.is_file():
                raise SystemExit(f"Missing correspondence render: {path}")
            tile = fitted(path)
            x = 120 + column * PANEL
            sheet.paste(tile, (x, y))
            draw.rectangle((x, y, x + PANEL - 1, y + PANEL - 1), outline=(90, 104, 126), width=1)
        draw.text((120, y + PANEL + 8), "source and target are frame-locked; target uses CoH runtime-local FK", fill=(155, 168, 190), font=font)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(args.output)
    print(f"ISSUE36_REST_BASIS_FULL_CONTACT_SHEET output={args.output} frames={','.join(map(str, frames))}")


if __name__ == "__main__":
    main()
