"""Make compact raw-Mixamo / Blender-Male / CoH-runtime comparison sheets."""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


FRAMES = (18, 20, 22)
VIEWS = (
    ("front", "front", "Front"),
    ("front-3-4", "threequarter", "Three-quarter"),
    ("side", "side", "Side"),
)
PANEL_SIZE = (640, 640)
RUNTIME_CROP = (400, 180, 850, 630)


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-root", required=True, type=Path)
    parser.add_argument("--target-root", required=True, type=Path)
    parser.add_argument("--runtime-root", required=True, type=Path)
    parser.add_argument("--output-dir", required=True, type=Path)
    return parser.parse_args()


def font(size, bold=False):
    candidates = (
        Path(r"C:\Windows\Fonts\segoeuib.ttf") if bold else Path(r"C:\Windows\Fonts\segoeui.ttf"),
        Path(r"C:\Windows\Fonts\arialbd.ttf") if bold else Path(r"C:\Windows\Fonts\arial.ttf"),
    )
    for path in candidates:
        if path.exists():
            return ImageFont.truetype(str(path), size)
    return ImageFont.load_default()


def fit_panel(image):
    image = image.convert("RGB")
    image.thumbnail(PANEL_SIZE, Image.Resampling.LANCZOS)
    panel = Image.new("RGB", PANEL_SIZE, (10, 14, 22))
    left = (PANEL_SIZE[0] - image.width) // 2
    top = (PANEL_SIZE[1] - image.height) // 2
    panel.paste(image, (left, top))
    return panel


def runtime_panel(path):
    image = Image.open(path).convert("RGB")
    left, top, right, bottom = RUNTIME_CROP
    right = min(right, image.width)
    bottom = min(bottom, image.height)
    left = max(0, min(left, right - 1))
    top = max(0, min(top, bottom - 1))
    return fit_panel(image.crop((left, top, right, bottom)))


def add_centered(draw, box, text, font_obj, fill):
    left, top, right, bottom = box
    bounds = draw.textbbox((0, 0), text, font=font_obj)
    draw.text(
        ((left + right - (bounds[2] - bounds[0])) / 2,
         (top + bottom - (bounds[3] - bounds[1])) / 2),
        text,
        font=font_obj,
        fill=fill,
    )


def main():
    args = parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    title_font = font(28, bold=True)
    header_font = font(24, bold=True)
    row_font = font(22, bold=True)
    for frame in FRAMES:
        source_root = Path(str(args.source_root).format(frame=frame))
        target_root = Path(str(args.target_root).format(frame=frame))
        sheet = Image.new("RGB", (1980, 2030), (18, 22, 32))
        draw = ImageDraw.Draw(sheet)
        draw.text((28, 18), f"Issue 36 — frame {frame}: source → Male retarget → runtime", font=title_font, fill=(245, 247, 250))
        columns = (
            ("RAW MIXAMO", (70, 165, 710, 805)),
            ("BLENDER RETARGETED MALE", (720, 165, 1360, 805)),
            ("COH RUNTIME — SWINGV3", (1370, 165, 2010, 805)),
        )
        # Draw headers and the three aligned-view rows.
        for label, (left, top, right, bottom) in columns:
            add_centered(draw, (left, 96, right, 150), label, header_font, (252, 190, 76))
        row_top = 165
        row_height = 615
        for row_index, (source_view, runtime_view, view_label) in enumerate(VIEWS):
            y = row_top + row_index * row_height
            source_path = source_root / "visual" / "source" / f"{source_view}.png"
            target_path = target_root / "visual" / "coh" / f"{source_view}.png"
            runtime_path = args.runtime_root / f"SWINGV3_BOTTOM_FRAME{frame}_{runtime_view}.jpg"
            panels = (
                fit_panel(Image.open(source_path)),
                fit_panel(Image.open(target_path)),
                runtime_panel(runtime_path),
            )
            for index, panel in enumerate(panels):
                x = 70 + index * 650
                sheet.paste(panel, (x, y))
                draw.rectangle((x, y, x + 640, y + 640), outline=(100, 112, 132), width=2)
            draw.text((15, y + 285), view_label, font=row_font, fill=(220, 225, 235))
        output = args.output_dir / f"FRAME{frame}_raw-vs-retarget-vs-runtime.jpg"
        sheet.save(output, quality=90, optimize=True)
        print(f"ISSUE36_COMPARISON_SHEET {output}")


if __name__ == "__main__":
    main()
