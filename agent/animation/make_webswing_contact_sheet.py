"""Combine the five deterministic Web Swing authoring views into one sheet."""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


VIEWS = ("front", "front-3-4", "side", "rear-3-4", "gameplay")


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input-dir", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--title", default="Web Swing authoring gate")
    return parser.parse_args()


def main():
    args = parse_args()
    images = []
    for view in VIEWS:
        path = args.input_dir / f"{view}.png"
        if not path.is_file():
            raise SystemExit(f"Missing deterministic view: {path}")
        images.append((view, Image.open(path).convert("RGB")))

    tile_width = max(image.width for _, image in images)
    tile_height = max(image.height for _, image in images)
    label_height = 34
    padding = 18
    columns = 3
    rows = 2
    sheet = Image.new(
        "RGB",
        (
            columns * tile_width + (columns + 1) * padding,
            rows * (tile_height + label_height) + (rows + 1) * padding,
        ),
        (24, 24, 29),
    )
    draw = ImageDraw.Draw(sheet)
    font = ImageFont.load_default()
    for index, (view, image) in enumerate(images):
        column = index % columns
        row = index // columns
        x = padding + column * tile_width
        y = padding + row * (tile_height + label_height)
        draw.text((x, y + 7), view.upper(), fill=(246, 246, 246), font=font)
        sheet.paste(image, (x, y + label_height))

    args.output.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(args.output)
    print(
        "WEBSWING_CONTACT_SHEET_CREATED "
        f"output={args.output} title={args.title} views={','.join(VIEWS)}"
    )


if __name__ == "__main__":
    main()
