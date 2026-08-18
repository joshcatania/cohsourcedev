"""Reproduce the Issue #20 two-texture pilot from local stock .texture files."""

from __future__ import annotations

import argparse
import hashlib
import io
import json
import os
import shutil
import struct
import subprocess
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageFilter


BASE_RELATIVE = Path("texture_library/NPCS/Vehicles/Car_Truck/Cubevan_side.texture")
NORMAL_RELATIVE = Path("texture_library/NPCS/Vehicles/Car_COMMON/carsheen_bump_02.texture")
BASE_OVERRIDE = Path("bin/data") / BASE_RELATIVE
NORMAL_OVERRIDE = Path("bin/data") / NORMAL_RELATIVE

STOCK_BASE_SHA256 = "397B1DFA4BE426E5BB2CFC205CAB0C77D60F3CBE62D8177C651BC887B73642F0"
STOCK_NORMAL_SHA256 = "64E0944836182CDC822F3F6B3D9076DC4C302FA6C205639D7C378B28A34D3BD7"
PILOT_BASE_SHA256 = "E10F0E7C3ECF6278F6B9058F932A3EF7B4B00B31CA2DAE8B842D3619EC1C3613"
PILOT_NORMAL_SHA256 = "68F91388ABA8A3ECA10D54E5B5421C0E5D72B52E8F5A5A4E5C95186FDD72FDC4"


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest().upper()


def texture_info(path: Path) -> dict:
    data = path.read_bytes()
    if len(data) < 32:
        raise ValueError(f"{path} is not a .texture container")
    header_size, file_size, width, height, flags = struct.unpack_from("<IIIII", data, 0)
    name_end = data.find(b"\0", 32)
    if name_end < 0:
        raise ValueError(f"{path} has no stored texture name")
    name = data[32:name_end].decode("ascii")
    payload = data[header_size : header_size + file_size]
    if len(payload) != file_size or payload[:4] != b"DDS ":
        raise ValueError(f"{path} does not contain a complete DDS payload")
    return {
        "path": str(path),
        "bytes": len(data),
        "header_size": header_size,
        "file_size": file_size,
        "width": width,
        "height": height,
        "flags": flags,
        "name": name,
        "payload": payload,
    }


def image_from_texture(info: dict) -> Image.Image:
    return Image.open(io.BytesIO(info["payload"])).convert("RGBA")


def alpha_values(image: Image.Image) -> list[int]:
    return sorted(int(value) for value in np.unique(np.asarray(image, dtype=np.uint8)[:, :, 3]))


def make_base(image: Image.Image) -> Image.Image:
    rgb = image.convert("RGB").resize((512, 512), Image.Resampling.LANCZOS)
    rgb = rgb.filter(ImageFilter.UnsharpMask(radius=0.6, percent=12, threshold=3))
    alpha = image.getchannel("A").resize((512, 512), Image.Resampling.NEAREST)
    return Image.merge("RGBA", (*rgb.split(), alpha))


def make_normal_gloss(image: Image.Image) -> tuple[Image.Image, dict]:
    source = np.asarray(image, dtype=np.uint8)
    encoded = source[:, :, :3].astype(np.float32)
    vectors = encoded / 127.5 - 1.0
    resized = np.empty((128, 128, 3), dtype=np.float32)
    for channel in range(3):
        component = Image.fromarray(vectors[:, :, channel], mode="F")
        resized[:, :, channel] = np.asarray(
            component.resize((128, 128), Image.Resampling.LANCZOS), dtype=np.float32
        )
    lengths = np.linalg.norm(resized, axis=2, keepdims=True)
    lengths = np.maximum(lengths, np.finfo(np.float32).eps)
    normalized = resized / lengths
    out_rgb = np.clip((normalized * 0.5 + 0.5) * 255.0, 0.0, 255.0).astype(np.uint8)
    alpha = image.getchannel("A").resize((128, 128), Image.Resampling.NEAREST)
    output = Image.fromarray(np.dstack((out_rgb, np.asarray(alpha, dtype=np.uint8))), mode="RGBA")
    return output, {
        "input_alpha": alpha_values(image),
        "output_alpha": alpha_values(output),
        "vector_length_min": float(np.linalg.norm(normalized, axis=2).min()),
        "vector_length_max": float(np.linalg.norm(normalized, axis=2).max()),
    }


def write_tga(image: Image.Image, path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    image.save(path, format="TGA")


def patch_container(path: Path, expected_name: str, expected_flags: int) -> None:
    data = bytearray(path.read_bytes())
    header_size, file_size = struct.unpack_from("<II", data, 0)
    name_end = data.find(b"\0", 32)
    stored_name = data[32:name_end].decode("ascii")
    if stored_name.startswith("./"):
        del data[32:34]
        header_size -= 2
        struct.pack_into("<I", data, 0, header_size)
    struct.pack_into("<I", data, 16, expected_flags)
    path.write_bytes(data)
    info = texture_info(path)
    if info["name"] != expected_name:
        raise ValueError(f"stored name mismatch: {info['name']} != {expected_name}")
    if info["file_size"] != file_size or len(data) != info["header_size"] + info["file_size"]:
        raise ValueError(f"invalid patched container layout: {path}")


def package_with_gettex(package_root: Path, gettex: Path) -> None:
    texture_root = package_root / "texture_library"
    lock_path = package_root / "gettex.lock"
    environment = os.environ.copy()
    environment["COH_GETTEX_LOCK_PATH"] = str(lock_path)
    command = [
        str(gettex),
        "-nopig",
        "-noprompt",
        "-nopause",
        "-override",
        "-force",
        "-forcerepack",
        "./texture_library",
    ]
    completed = subprocess.run(
        command,
        cwd=package_root,
        env=environment,
        text=True,
        capture_output=True,
    )
    if completed.returncode != 0:
        raise RuntimeError(
            "GetTex failed with exit code "
            f"{completed.returncode}\n{completed.stdout}\n{completed.stderr}"
        )


def generate(stock_base: Path, stock_normal: Path, output_root: Path, gettex: Path) -> dict:
    stock_base = stock_base.resolve()
    stock_normal = stock_normal.resolve()
    output_root = output_root.resolve()
    output_root.mkdir(parents=True, exist_ok=True)
    package_root = output_root / "_package"
    if package_root.exists():
        shutil.rmtree(package_root)
    package_root.mkdir(parents=True)

    base_info = texture_info(stock_base)
    normal_info = texture_info(stock_normal)
    if sha256(stock_base) != STOCK_BASE_SHA256:
        raise ValueError(f"unexpected stock base hash: {sha256(stock_base)}")
    if sha256(stock_normal) != STOCK_NORMAL_SHA256:
        raise ValueError(f"unexpected stock normal/gloss hash: {sha256(stock_normal)}")
    base_image = image_from_texture(base_info)
    normal_image = image_from_texture(normal_info)
    base_output = make_base(base_image)
    normal_output, normal_metrics = make_normal_gloss(normal_image)

    base_tga = package_root / BASE_RELATIVE.with_suffix(".tga")
    normal_tga = package_root / NORMAL_RELATIVE.with_suffix(".tga")
    write_tga(base_output, base_tga)
    write_tga(normal_output, normal_tga)
    package_with_gettex(package_root, gettex.resolve())

    generated_base = package_root / BASE_RELATIVE
    generated_normal = package_root / NORMAL_RELATIVE
    if not generated_base.exists() or not generated_normal.exists():
        raise FileNotFoundError("GetTex did not produce both pilot containers")
    patch_container(generated_base, "texture_library/NPCS/Vehicles/Car_Truck/Cubevan_side.dds", 0x10)
    patch_container(generated_normal, "texture_library/NPCS/Vehicles/Car_COMMON/carsheen_bump_02.dds", 0x10C0)

    output_base = output_root / BASE_RELATIVE
    output_normal = output_root / NORMAL_RELATIVE
    output_base.parent.mkdir(parents=True, exist_ok=True)
    output_normal.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(generated_base, output_base)
    shutil.copy2(generated_normal, output_normal)

    base_result = texture_info(output_base)
    normal_result = texture_info(output_normal)
    base_result_image = image_from_texture(base_result)
    normal_result_image = image_from_texture(normal_result)
    result = {
        "stock": {
            "base": {"path": str(stock_base), "sha256": sha256(stock_base), "bytes": stock_base.stat().st_size},
            "normal_gloss": {"path": str(stock_normal), "sha256": sha256(stock_normal), "bytes": stock_normal.stat().st_size},
        },
        "pilot": {
            "base": {
                "path": str(output_base),
                "sha256": sha256(output_base),
                "bytes": output_base.stat().st_size,
                "payload_bytes": base_result["file_size"],
                "dimensions": [base_result_image.width, base_result_image.height],
                "alpha_values": alpha_values(base_result_image),
            },
            "normal_gloss": {
                "path": str(output_normal),
                "sha256": sha256(output_normal),
                "bytes": output_normal.stat().st_size,
                "payload_bytes": normal_result["file_size"],
                "dimensions": [normal_result_image.width, normal_result_image.height],
                "gloss_alpha_values": alpha_values(normal_result_image),
                **normal_metrics,
            },
        },
        "expected_hashes": {"base": PILOT_BASE_SHA256, "normal_gloss": PILOT_NORMAL_SHA256},
    }
    if result["pilot"]["base"]["sha256"] != PILOT_BASE_SHA256:
        raise ValueError(f"generated base hash mismatch: {result['pilot']['base']['sha256']}")
    if result["pilot"]["normal_gloss"]["sha256"] != PILOT_NORMAL_SHA256:
        raise ValueError(f"generated normal/gloss hash mismatch: {result['pilot']['normal_gloss']['sha256']}")
    if result["pilot"]["base"]["dimensions"] != [512, 512]:
        raise ValueError("generated base dimensions are not 512x512")
    if result["pilot"]["normal_gloss"]["dimensions"] != [128, 128]:
        raise ValueError("generated normal/gloss dimensions are not 128x128")
    if result["pilot"]["base"]["alpha_values"] != [0, 255]:
        raise ValueError("generated base alpha semantics changed")
    if result["pilot"]["normal_gloss"]["gloss_alpha_values"] != normal_metrics["input_alpha"]:
        raise ValueError("generated gloss alpha values changed")
    (output_root / "manifest.json").write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2))
    return result


def install(output_root: Path, repo_root: Path) -> None:
    manifest = output_root / "manifest.json"
    if not manifest.exists():
        raise FileNotFoundError(f"generate first; missing {manifest}")
    data = json.loads(manifest.read_text(encoding="utf-8"))
    for key, relative in (("base", BASE_RELATIVE), ("normal_gloss", NORMAL_RELATIVE)):
        source = output_root / relative
        if sha256(source) != data["pilot"][key]["sha256"]:
            raise ValueError(f"manifest hash mismatch for {source}")
        destination = repo_root / "bin" / "data" / relative
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, destination)
    future = __import__("datetime").datetime.now().timestamp() + 7200
    for relative in (BASE_RELATIVE, NORMAL_RELATIVE):
        os.utime(repo_root / "bin" / "data" / relative, (future, future))
    print("installed loose pilot overrides under bin/data/texture_library")


def remove(repo_root: Path) -> None:
    for relative in (BASE_RELATIVE, NORMAL_RELATIVE):
        destination = repo_root / "bin" / "data" / relative
        if destination.exists():
            destination.unlink()
            print(f"removed {destination}")
    print("packed stock behavior restored; no pigg was modified")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("generate", "install", "remove"))
    parser.add_argument("--stock-base", type=Path)
    parser.add_argument("--stock-normal-gloss", type=Path)
    parser.add_argument("--output-root", type=Path, required=True)
    parser.add_argument("--repo-root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--gettex", type=Path)
    args = parser.parse_args()
    if args.action == "generate":
        if not args.stock_base or not args.stock_normal_gloss or not args.gettex:
            parser.error("generate requires --stock-base, --stock-normal-gloss and --gettex")
        generate(args.stock_base, args.stock_normal_gloss, args.output_root, args.gettex)
    elif args.action == "install":
        install(args.output_root.resolve(), args.repo_root.resolve())
    else:
        remove(args.repo_root.resolve())
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as error:
        print(f"texture pilot failed: {error}", file=sys.stderr)
        raise
