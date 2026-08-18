"""Reproduce the accepted manifest-driven texture pilots from local stock files."""

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


def make_base_to(image: Image.Image, size: tuple[int, int]) -> Image.Image:
    rgb = image.convert("RGB").resize(size, Image.Resampling.LANCZOS)
    rgb = rgb.filter(ImageFilter.UnsharpMask(radius=0.6, percent=12, threshold=3))
    alpha = image.getchannel("A").resize(size, Image.Resampling.NEAREST)
    return Image.merge("RGBA", (*rgb.split(), alpha))


def make_base(image: Image.Image) -> Image.Image:
    return make_base_to(image, (512, 512))


def make_base_restoration(image: Image.Image, size: tuple[int, int]) -> Image.Image:
    """Apply a bounded same-resolution restoration without inventing artwork."""
    if image.size != size:
        raise ValueError(f"same-resolution restoration requires {image.size} == {size}")
    rgb = image.convert("RGB").filter(
        ImageFilter.UnsharpMask(radius=0.65, percent=8, threshold=5)
    )
    alpha = image.getchannel("A").copy()
    return Image.merge("RGBA", (*rgb.split(), alpha))


def make_mask_to(image: Image.Image, size: tuple[int, int]) -> Image.Image:
    """Resize an authored mask by nearest-neighbor replication only."""
    return image.convert("RGBA").resize(size, Image.Resampling.NEAREST)


def make_normal_gloss_to(image: Image.Image, size: tuple[int, int]) -> tuple[Image.Image, dict]:
    source = np.asarray(image, dtype=np.uint8)
    encoded = source[:, :, :3].astype(np.float32)
    vectors = encoded / 127.5 - 1.0
    resized = np.empty((size[1], size[0], 3), dtype=np.float32)
    for channel in range(3):
        component = Image.fromarray(vectors[:, :, channel], mode="F")
        resized[:, :, channel] = np.asarray(component.resize(size, Image.Resampling.LANCZOS), dtype=np.float32)
    lengths = np.linalg.norm(resized, axis=2, keepdims=True)
    lengths = np.maximum(lengths, np.finfo(np.float32).eps)
    normalized = resized / lengths
    out_rgb = np.clip((normalized * 0.5 + 0.5) * 255.0, 0.0, 255.0).astype(np.uint8)
    alpha = image.getchannel("A").resize(size, Image.Resampling.NEAREST)
    output = Image.fromarray(np.dstack((out_rgb, np.asarray(alpha, dtype=np.uint8))), mode="RGBA")
    return output, {
        "input_alpha": alpha_values(image),
        "output_alpha": alpha_values(output),
        "vector_length_min": float(np.linalg.norm(normalized, axis=2).min()),
        "vector_length_max": float(np.linalg.norm(normalized, axis=2).max()),
    }


def make_normal_gloss(image: Image.Image) -> tuple[Image.Image, dict]:
    return make_normal_gloss_to(image, (128, 128))


def make_statue_hero_albedo(
    image: Image.Image,
    target: tuple[int, int],
    detail_image: Image.Image | None,
    detail_strength: float,
    albedo_contrast: float,
    albedo_gain: float,
) -> tuple[Image.Image, dict]:
    """Restore an authored albedo using the statue's existing surface data.

    The statue's diffuse layers are unusually small relative to the model. A
    plain resize would only move the same blur to a larger container, so the
    hero path combines a restrained albedo restoration with a low-amplitude
    high-pass luminance cue derived from the already-authored normal/AO layer.
    It does not synthesize new UV islands or invent color; alpha remains a
    nearest-neighbor copy of the source.
    """
    source = image.convert("RGBA")
    restored = make_base_to(source, target)
    rgb = np.asarray(restored.convert("RGB"), dtype=np.float32)
    if detail_image is None:
        detail = source.convert("L")
    else:
        detail = detail_image.convert("L")
    detail = detail.resize(target, Image.Resampling.LANCZOS)
    detail = detail.filter(ImageFilter.GaussianBlur(radius=0.75))
    detail_values = np.asarray(detail, dtype=np.float32)
    low_values = np.asarray(detail.filter(ImageFilter.GaussianBlur(radius=2.25)), dtype=np.float32)
    high_values = detail_values - low_values
    detail_std = float(high_values.std())
    if detail_std > 1e-4 and detail_strength:
        cue = np.clip(high_values / detail_std * float(detail_strength), -24.0, 24.0)
        rgb += cue[:, :, None]
    rgb = 127.5 + (rgb - 127.5) * float(albedo_contrast)
    rgb *= float(albedo_gain)
    output_rgb = np.clip(rgb, 0.0, 255.0).astype(np.uint8)
    output = Image.fromarray(
        np.dstack((output_rgb, np.asarray(source.getchannel("A").resize(target, Image.Resampling.NEAREST)))),
        mode="RGBA",
    )
    return output, {
        "operation": "statue-hero-albedo-restoration",
        "detail_strength": float(detail_strength),
        "albedo_contrast": float(albedo_contrast),
        "albedo_gain": float(albedo_gain),
        "detail_highpass_std": detail_std,
        "alpha_policy": "nearest-preserve",
    }


def make_statue_hero_normal(
    image: Image.Image,
    target: tuple[int, int],
    normal_strength: float,
) -> tuple[Image.Image, dict]:
    """Resize/restore an authored normal while preserving its gloss alpha."""
    source = np.asarray(image, dtype=np.uint8)
    encoded = source[:, :, :3].astype(np.float32)
    vectors = encoded / 127.5 - 1.0
    resized = np.empty((target[1], target[0], 3), dtype=np.float32)
    for channel in range(3):
        component = Image.fromarray(vectors[:, :, channel], mode="F")
        resized[:, :, channel] = np.asarray(component.resize(target, Image.Resampling.LANCZOS), dtype=np.float32)
    lengths = np.linalg.norm(resized, axis=2, keepdims=True)
    lengths = np.maximum(lengths, np.finfo(np.float32).eps)
    normalized = resized / lengths
    neutral = np.zeros_like(normalized)
    neutral[:, :, 2] = 1.0
    strengthened = neutral + (normalized - neutral) * float(normal_strength)
    strengthened /= np.maximum(np.linalg.norm(strengthened, axis=2, keepdims=True), np.finfo(np.float32).eps)
    out_rgb = np.clip((strengthened * 0.5 + 0.5) * 255.0, 0.0, 255.0).astype(np.uint8)
    alpha = np.asarray(image.getchannel("A").resize(target, Image.Resampling.NEAREST), dtype=np.uint8)
    output = Image.fromarray(np.dstack((out_rgb, alpha)), mode="RGBA")
    return output, {
        "operation": "statue-hero-vector-normal-restoration",
        "normal_strength": float(normal_strength),
        "gloss_alpha_policy": "nearest-preserve",
        "vector_length_min": float(np.linalg.norm(strengthened, axis=2).min()),
        "vector_length_max": float(np.linalg.norm(strengthened, axis=2).max()),
    }


def make_statue_hero_detail(
    image: Image.Image,
    target: tuple[int, int],
) -> tuple[Image.Image, dict]:
    """Restore authored material detail after resizing without inventing pixels."""
    source = image.convert("RGBA")
    restored = make_base_to(source, target)
    sharpened = restored.convert("RGB").filter(ImageFilter.UnsharpMask(radius=1.05, percent=75, threshold=3))
    output = Image.fromarray(
        np.dstack((np.asarray(sharpened, dtype=np.uint8), np.asarray(source.getchannel("A").resize(target, Image.Resampling.NEAREST)))),
        mode="RGBA",
    )
    return output, {
        "operation": "statue-hero-detail-restoration",
        "resize_filter": "lanczos",
        "detail_filter": "unsharp-radius-1.05-percent-75-threshold-3",
        "alpha_policy": "nearest-preserve",
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
        name_end -= 2
    expected_bytes = expected_name.encode("ascii") + b"\0"
    old_name_length = name_end - 32 + 1
    data[32:name_end + 1] = expected_bytes
    header_size += len(expected_bytes) - old_name_length
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


def _manifest_int(value: object) -> int:
    if isinstance(value, int):
        return value
    return int(str(value), 0)


def _manifest_entries(manifest_path: Path) -> tuple[dict, list[dict]]:
    spec = json.loads(manifest_path.read_text(encoding="utf-8"))
    if spec.get("schema") != "coh.texture-pilot.v1":
        raise ValueError(f"unsupported texture pilot manifest schema: {spec.get('schema')}")
    entries = spec.get("entries")
    if not isinstance(entries, list) or not entries:
        raise ValueError("manifest must contain a non-empty entries list")
    ids = [entry.get("id") for entry in entries]
    if any(not item for item in ids) or len(set(ids)) != len(ids):
        raise ValueError("manifest entry ids must be unique and non-empty")
    return spec, entries


def _manifest_image(
    image: Image.Image,
    semantic: str,
    target: tuple[int, int],
    transform_mode: str = "upscale_2x",
    detail_image: Image.Image | None = None,
    detail_strength: float = 0.0,
    normal_strength: float = 1.0,
    albedo_contrast: float = 1.0,
    albedo_gain: float = 1.0,
) -> tuple[Image.Image, dict]:
    if transform_mode == "upscale_2x":
        if semantic == "base":
            output = make_base_to(image, target)
            return output, {"input_alpha": alpha_values(image), "output_alpha": alpha_values(output)}
        if semantic == "normal_gloss":
            return make_normal_gloss_to(image, target)
        if semantic == "mask":
            output = make_mask_to(image, target)
            return output, {"input_alpha": alpha_values(image), "output_alpha": alpha_values(output)}
    elif transform_mode == "restore_same_resolution":
        if target != image.size:
            raise ValueError(f"restoration target must match source dimensions: {target} != {image.size}")
        if semantic == "base":
            output = make_base_restoration(image, target)
            return output, {
                "input_alpha": alpha_values(image),
                "output_alpha": alpha_values(output),
                "restoration": "unsharp-radius-0.65-percent-8-threshold-5",
            }
        if semantic == "normal_gloss":
            output, metrics = make_normal_gloss_to(image, target)
            metrics["restoration"] = "vector-renormalize-same-resolution"
            return output, metrics
        if semantic == "mask":
            output = image.convert("RGBA").copy()
            return output, {
                "input_alpha": alpha_values(image),
                "output_alpha": alpha_values(output),
                "restoration": "exact-pixel-preserve",
            }
    elif transform_mode == "statue_hero_albedo":
        if semantic != "base":
            raise ValueError("statue_hero_albedo requires base semantic")
        return make_statue_hero_albedo(
            image,
            target,
            detail_image,
            detail_strength,
            albedo_contrast,
            albedo_gain,
        )
    elif transform_mode == "statue_hero_normal":
        if semantic != "normal_gloss":
            raise ValueError("statue_hero_normal requires normal_gloss semantic")
        return make_statue_hero_normal(image, target, normal_strength)
    elif transform_mode == "statue_hero_mask":
        if semantic != "mask":
            raise ValueError("statue_hero_mask requires mask semantic")
        output = make_mask_to(image, target)
        return output, {
            "operation": "statue-hero-authored-mask-nearest",
            "alpha_policy": "nearest-preserve",
        }
    elif transform_mode == "statue_hero_detail":
        if semantic != "base":
            raise ValueError("statue_hero_detail requires base semantic")
        return make_statue_hero_detail(image, target)
    else:
        raise ValueError(f"unsupported manifest transform mode: {transform_mode}")
    raise ValueError(f"unsupported manifest semantic: {semantic}")


def _alpha_codec_metrics(source: Image.Image, output: Image.Image) -> dict:
    source_alpha = np.asarray(source.getchannel("A"), dtype=np.int16)
    output_alpha = np.asarray(output.getchannel("A"), dtype=np.int16)
    if source_alpha.shape != output_alpha.shape:
        raise ValueError("alpha comparison dimensions do not match")
    difference = output_alpha - source_alpha
    return {
        "source_min": int(source_alpha.min()),
        "source_max": int(source_alpha.max()),
        "output_min": int(output_alpha.min()),
        "output_max": int(output_alpha.max()),
        "mean_absolute_error": float(np.abs(difference).mean()),
        "max_absolute_error": int(np.abs(difference).max()),
    }


def generate_manifest(manifest_path: Path, stock_root: Path, output_root: Path, gettex: Path) -> dict:
    spec, entries = _manifest_entries(manifest_path.resolve())
    stock_root = stock_root.resolve()
    output_root = output_root.resolve()
    output_root.mkdir(parents=True, exist_ok=True)
    package_root = output_root / "_package"
    if package_root.exists():
        shutil.rmtree(package_root)
    package_root.mkdir(parents=True)

    generated_entries = []
    for entry in entries:
        stock_relative = Path(entry["stock_path"])
        stock_path = stock_root / stock_relative
        if not stock_path.exists():
            raise FileNotFoundError(f"manifest stock input is missing: {stock_path}")
        stock_hash = sha256(stock_path)
        if stock_hash != entry["stock_sha256"].upper():
            raise ValueError(f"stock hash mismatch for {stock_path}: {stock_hash}")
        info = texture_info(stock_path)
        if info["name"] != entry["stored_name"]:
            raise ValueError(f"stored name mismatch for {stock_path}: {info['name']} != {entry['stored_name']}")
        expected_dimensions = [info["width"], info["height"]]
        if expected_dimensions != entry["stock_dimensions"]:
            raise ValueError(f"stock dimensions mismatch for {stock_path}: {expected_dimensions}")
        if info["flags"] != _manifest_int(entry["stock_flags"]):
            raise ValueError(f"stock flags mismatch for {stock_path}: 0x{info['flags']:x}")

        image = image_from_texture(info)
        target = tuple(entry["target_dimensions"])
        transform_mode = entry.get("transform_mode", "upscale_2x")
        if any(not isinstance(value, int) or value <= 0 for value in target):
            raise ValueError(f"invalid target dimensions for {entry['id']}: {target}")
        if transform_mode == "upscale_2x":
            if target[0] != info["width"] * 2 or target[1] != info["height"] * 2:
                raise ValueError(f"entry {entry['id']} is not a conservative 2x transform")
        elif transform_mode == "restore_same_resolution":
            if target != (info["width"], info["height"]):
                raise ValueError(f"entry {entry['id']} restoration changes dimensions")
        elif transform_mode in (
            "statue_hero_albedo",
            "statue_hero_normal",
            "statue_hero_mask",
            "statue_hero_detail",
        ):
            if max(target) > 1024:
                raise ValueError(f"entry {entry['id']} exceeds the 1024-pixel hero cap")
        else:
            raise ValueError(f"unsupported transform mode for {entry['id']}: {transform_mode}")
        if max(target) > 1024:
            raise ValueError(f"entry {entry['id']} exceeds the 1024-pixel pilot cap")

        detail_image = None
        detail_source = entry.get("detail_source")
        if detail_source:
            detail_entry = next((item for item in entries if item.get("id") == detail_source), None)
            if detail_entry is None:
                raise ValueError(f"detail source is not present for {entry['id']}: {detail_source}")
            detail_path = stock_root / Path(detail_entry["stock_path"])
            detail_image = image_from_texture(texture_info(detail_path))
        transformed, transform_metrics = _manifest_image(
            image,
            entry["semantic"],
            target,
            transform_mode,
            detail_image=detail_image,
            detail_strength=float(entry.get("detail_strength", 0.0)),
            normal_strength=float(entry.get("normal_strength", 1.0)),
            albedo_contrast=float(entry.get("albedo_contrast", 1.0)),
            albedo_gain=float(entry.get("albedo_gain", 1.0)),
        )
        package_relative = Path("texture_library") / "TexturePilotInputs" / f"{entry['id']}.tga"
        package_tga = package_root / package_relative
        write_tga(transformed, package_tga)
        generated_entries.append({
            "id": entry["id"],
            "stock_path": entry["stock_path"],
            "install_path": entry["install_path"],
            "stored_name": entry["stored_name"],
            "semantic": entry["semantic"],
            "transform_mode": transform_mode,
            "stock_sha256": stock_hash,
            "stock_dimensions": expected_dimensions,
            "stock_flags": info["flags"],
            "target_dimensions": list(target),
            "transform": transform_metrics,
            "package_relative": str(package_relative).replace("\\", "/"),
        })

    package_with_gettex(package_root, gettex.resolve())
    for generated in generated_entries:
        entry = next(item for item in entries if item["id"] == generated["id"])
        generated_path = package_root / Path(generated["package_relative"]).with_suffix(".texture")
        if not generated_path.exists():
            raise FileNotFoundError(f"GetTex did not produce {generated_path}")
        patch_container(generated_path, entry["stored_name"], _manifest_int(entry["stock_flags"]))
        output_path = output_root / Path(entry["install_path"])
        output_path.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(generated_path, output_path)
        output_info = texture_info(output_path)
        output_image = image_from_texture(output_info)
        if [output_info["width"], output_info["height"]] != entry["target_dimensions"]:
            raise ValueError(f"generated dimensions mismatch for {entry['id']}")
        if output_info["name"] != entry["stored_name"]:
            raise ValueError(f"generated stored name mismatch for {entry['id']}")
        if output_info["flags"] != _manifest_int(entry["stock_flags"]):
            raise ValueError(f"generated flags changed for {entry['id']}")
        output_alpha = alpha_values(output_image)
        source_info = texture_info(stock_root / Path(entry["stock_path"]))
        detail_image = None
        detail_source = entry.get("detail_source")
        if detail_source:
            detail_entry = next((item for item in entries if item.get("id") == detail_source), None)
            if detail_entry is None:
                raise ValueError(f"detail source is not present for {entry['id']}: {detail_source}")
            detail_path = stock_root / Path(detail_entry["stock_path"])
            detail_image = image_from_texture(texture_info(detail_path))
        transformed, _ = _manifest_image(
            image_from_texture(source_info),
            entry["semantic"],
            tuple(entry["target_dimensions"]),
            entry.get("transform_mode", "upscale_2x"),
            detail_image=detail_image,
            detail_strength=float(entry.get("detail_strength", 0.0)),
            normal_strength=float(entry.get("normal_strength", 1.0)),
            albedo_contrast=float(entry.get("albedo_contrast", 1.0)),
            albedo_gain=float(entry.get("albedo_gain", 1.0)),
        )
        alpha_codec = _alpha_codec_metrics(transformed, output_image)
        if (
            alpha_codec["mean_absolute_error"] > 1.0
            or alpha_codec["max_absolute_error"] > 16
            or abs(alpha_codec["source_min"] - alpha_codec["output_min"])
            > int(entry.get("alpha_range_tolerance", 2))
            or abs(alpha_codec["source_max"] - alpha_codec["output_max"])
            > int(entry.get("alpha_range_tolerance", 2))
        ):
            raise ValueError(f"alpha semantics changed for {entry['id']}: {alpha_codec}")
        generated.update({
            "output_sha256": sha256(output_path),
            "output_bytes": output_path.stat().st_size,
            "output_dimensions": [output_info["width"], output_info["height"]],
            "output_flags": output_info["flags"],
            "output_alpha": output_alpha,
            "alpha_codec": alpha_codec,
        })
        expected_output_hash = entry.get("expected_output_sha256")
        if expected_output_hash and generated["output_sha256"] != expected_output_hash.upper():
            raise ValueError(f"expected output hash mismatch for {entry['id']}: {generated['output_sha256']}")

    result = dict(spec)
    result["generated_by"] = "agent/texture_pilot.py"
    result["generated_entries"] = generated_entries
    (output_root / "manifest.json").write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2))
    return result


def _generated_manifest(output_root: Path, manifest_path: Path) -> tuple[dict, dict]:
    spec, entries = _manifest_entries(manifest_path.resolve())
    generated_path = output_root.resolve() / "manifest.json"
    if not generated_path.exists():
        raise FileNotFoundError(f"generate first; missing {generated_path}")
    generated = json.loads(generated_path.read_text(encoding="utf-8"))
    if [item.get("id") for item in generated.get("generated_entries", [])] != [item["id"] for item in entries]:
        raise ValueError("generated manifest entries do not match the source manifest")
    for entry, output in zip(entries, generated["generated_entries"]):
        output_path = output_root / Path(entry["install_path"])
        if not output_path.exists():
            raise FileNotFoundError(f"generated output is missing: {output_path}")
        actual = sha256(output_path)
        if actual != output["output_sha256"]:
            raise ValueError(f"generated output hash mismatch for {entry['id']}: {actual}")
        expected = entry.get("expected_output_sha256")
        if expected and actual != expected.upper():
            raise ValueError(f"source manifest output hash mismatch for {entry['id']}: {actual}")
    return spec, generated


def install_manifest(manifest_path: Path, output_root: Path, repo_root: Path) -> None:
    spec, generated = _generated_manifest(output_root, manifest_path)
    destinations = []
    for entry, output in zip(spec["entries"], generated["generated_entries"]):
        source = output_root.resolve() / Path(entry["install_path"])
        destination = repo_root.resolve() / "bin" / "data" / Path(entry["install_path"])
        if destination.exists() and sha256(destination) != output["output_sha256"]:
            raise ValueError(f"refusing to overwrite unrelated loose file: {destination}")
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, destination)
        destinations.append(destination)
    future = __import__("datetime").datetime.now().timestamp() + 7200
    for destination in destinations:
        os.utime(destination, (future, future))
    print(f"installed {len(destinations)} manifest-selected loose Atlas texture overrides")


def remove_manifest(manifest_path: Path, repo_root: Path) -> None:
    spec, _ = _manifest_entries(manifest_path.resolve())
    removed = 0
    for entry in spec["entries"]:
        destination = repo_root.resolve() / "bin" / "data" / Path(entry["install_path"])
        if not destination.exists():
            continue
        expected = entry.get("expected_output_sha256")
        if not expected or sha256(destination) != expected.upper():
            raise ValueError(f"refusing to remove unrelated loose file: {destination}")
        destination.unlink()
        removed += 1
    print(f"removed {removed} manifest-selected loose Atlas texture overrides")


def restore_manifest(manifest_path: Path, repo_root: Path) -> None:
    spec, _ = _manifest_entries(manifest_path.resolve())
    remove_manifest(manifest_path, repo_root)
    for archive in spec.get("packed_sources", []):
        path = repo_root.resolve() / Path(archive["path"])
        if not path.exists():
            raise FileNotFoundError(f"packed stock source is missing: {path}")
        actual = sha256(path)
        if actual != archive["sha256"].upper():
            raise ValueError(f"packed stock source changed: {path}")
    print("packed stock behavior restored; selected loose paths are absent and packed sources match the manifest")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("generate", "install", "remove", "restore"))
    parser.add_argument("--manifest", type=Path)
    parser.add_argument("--stock-root", type=Path)
    parser.add_argument("--stock-base", type=Path)
    parser.add_argument("--stock-normal-gloss", type=Path)
    parser.add_argument("--output-root", type=Path, required=True)
    parser.add_argument("--repo-root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--gettex", type=Path)
    args = parser.parse_args()
    if args.manifest:
        manifest_path = args.manifest.resolve()
        if args.action == "generate":
            if not args.stock_root or not args.gettex:
                parser.error("manifest generate requires --stock-root and --gettex")
            generate_manifest(manifest_path, args.stock_root, args.output_root, args.gettex)
        elif args.action == "install":
            install_manifest(manifest_path, args.output_root, args.repo_root)
        elif args.action == "restore":
            restore_manifest(manifest_path, args.repo_root)
        else:
            remove_manifest(manifest_path, args.repo_root)
        return 0
    if args.action == "generate":
        if not args.stock_base or not args.stock_normal_gloss or not args.gettex:
            parser.error("generate requires --stock-base, --stock-normal-gloss and --gettex")
        generate(args.stock_base, args.stock_normal_gloss, args.output_root, args.gettex)
    elif args.action == "install":
        install(args.output_root.resolve(), args.repo_root.resolve())
    elif args.action == "remove":
        remove(args.repo_root.resolve())
    else:
        parser.error("restore requires --manifest")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as error:
        print(f"texture pilot failed: {error}", file=sys.stderr)
        raise
