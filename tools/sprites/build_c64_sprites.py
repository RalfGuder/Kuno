"""Convert per-phase 24x21 image files into C64 hardware sprite data."""
from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path

from PIL import Image


@dataclass(frozen=True)
class FileSource:
    """24x21 image file (PNG or TGA) as sprite source."""
    path: Path


class ConfigError(ValueError):
    """The sprite_phases.json schema is malformed."""


class DuplicateSlotError(ValueError):
    """Two phases share the same slot."""


class SlotOutOfRangeError(ValueError):
    """A phase slot is >= total_slots."""


@dataclass(frozen=True)
class Phase:
    name: str
    slot: int
    color_outline: int
    color_fill: int
    src: FileSource


@dataclass(frozen=True)
class Config:
    output_bin: Path
    output_inc: Path
    preview_built: Path
    sprite_size: tuple[int, int]
    slot_bytes: int
    dark_threshold: int
    bright_threshold: int
    sprite_index_base: int
    total_slots: int
    phases: tuple[Phase, ...]


def load_config(config_path: Path) -> Config:
    """Load and validate sprite_phases.json from disk.

    Per-phase src.file paths are resolved relative to the config file's
    directory. Each source is eagerly validated for existence and 24x21
    dimensions, so no byte is packed until all sources are known good.

    Raises ConfigError, FileNotFoundError, ValueError, DuplicateSlotError,
    SlotOutOfRangeError.
    """
    config_path = Path(config_path)
    with config_path.open("r", encoding="utf-8") as f:
        raw = json.load(f)

    if "dark_threshold" not in raw or "bright_threshold" not in raw:
        raise ConfigError("config requires dark_threshold and bright_threshold")
    if raw["dark_threshold"] >= raw["bright_threshold"]:
        raise ConfigError(
            f"dark_threshold ({raw['dark_threshold']}) must be < "
            f"bright_threshold ({raw['bright_threshold']})"
        )
    if raw["total_slots"] % 2 != 0:
        raise ConfigError(f"total_slots must be even (got {raw['total_slots']})")

    base_dir = config_path.parent
    sprite_size = tuple(raw["sprite_size"])

    phases_list: list[Phase] = []
    for p in raw["phases"]:
        src = p.get("src")
        if not isinstance(src, dict) or "file" not in src:
            raise ConfigError(
                f"phase {p.get('name')!r} requires src.file in sprite_phases.json"
            )
        if "color_outline" not in p or "color_fill" not in p:
            raise ConfigError(
                f"phase {p.get('name')!r} requires color_outline and color_fill"
            )
        src_path = (base_dir / src["file"]).resolve()
        if not src_path.exists():
            raise FileNotFoundError(
                f"phase {p['name']!r} src.file not found: {src_path}"
            )
        with Image.open(src_path) as probe:
            if probe.size != sprite_size:
                raise ValueError(
                    f"phase {p['name']!r} src {src_path} has size {probe.size}, "
                    f"expected {sprite_size}"
                )
        phases_list.append(
            Phase(
                name=p["name"],
                slot=p["slot"],
                color_outline=p["color_outline"],
                color_fill=p["color_fill"],
                src=FileSource(path=src_path),
            )
        )

    total = raw["total_slots"]
    half = total // 2
    seen_slots: dict[int, str] = {}
    for ph in phases_list:
        if ph.slot >= half or ph.slot < 0:
            raise SlotOutOfRangeError(
                f"phase {ph.name!r} has slot {ph.slot}, must be in [0, {half})"
            )
        if ph.slot in seen_slots:
            raise DuplicateSlotError(
                f"slot {ph.slot} used by {seen_slots[ph.slot]!r} and {ph.name!r}"
            )
        seen_slots[ph.slot] = ph.name

    return Config(
        output_bin=(base_dir / raw["output_bin"]).resolve(),
        output_inc=(base_dir / raw["output_inc"]).resolve(),
        preview_built=(base_dir / raw["preview_built"]).resolve(),
        sprite_size=sprite_size,
        slot_bytes=raw["slot_bytes"],
        dark_threshold=raw["dark_threshold"],
        bright_threshold=raw["bright_threshold"],
        sprite_index_base=raw["sprite_index_base"],
        total_slots=total,
        phases=tuple(phases_list),
    )


def pack_phase_pair(
    image: Image.Image,
    dark_threshold: int,
    bright_threshold: int,
) -> tuple[bytes, bytes]:
    """Pack a 24x21 RGBA image into two 64-byte hires sprite blocks.

    Returns (outline_bytes, fill_bytes). Both share the same 3-bytes-per-row,
    MSB-first layout used for single-sprite hires; byte 63 is always 0.

    Background detection: the pixel at (0, 0) defines transparency. If its
    alpha is 0, the alpha channel drives transparency (PNG case). Otherwise
    any pixel matching its RGB is treated as background (TGA case where
    the source has no real alpha channel).

    Pixel classification (when not background and alpha > 0):
        avg(R,G,B) <  dark_threshold                       -> outline bit
        dark_threshold <= avg(R,G,B) < bright_threshold    -> fill bit
        avg(R,G,B) >= bright_threshold                     -> neither
    """
    if image.size != (24, 21):
        raise ValueError(f"expected 24x21 image, got {image.size}")
    bg_r, bg_g, bg_b, bg_a = image.getpixel((0, 0))
    outline = bytearray(64)
    fill = bytearray(64)
    for y in range(21):
        for x in range(24):
            r, g, b, a = image.getpixel((x, y))
            if a == 0:
                continue
            if bg_a != 0 and (r, g, b) == (bg_r, bg_g, bg_b):
                continue
            brightness = (r + g + b) // 3
            byte_idx = y * 3 + x // 8
            bit = 1 << (7 - (x % 8))
            if brightness < dark_threshold:
                outline[byte_idx] |= bit
            elif brightness < bright_threshold:
                fill[byte_idx] |= bit
    return bytes(outline), bytes(fill)


def build_bin(cfg: Config) -> bytes:
    """Pack every phase's source file and produce total_slots * slot_bytes bytes.

    Transitional layout until Task 4 introduces the two-bank split: only
    the outline block is written. Fill bytes are discarded for now.
    """
    out = bytearray(cfg.total_slots * cfg.slot_bytes)
    for phase in cfg.phases:
        img = Image.open(phase.src.path).convert("RGBA")
        outline, _ = pack_phase_pair(img, cfg.dark_threshold, cfg.bright_threshold)
        offset = phase.slot * cfg.slot_bytes
        out[offset : offset + cfg.slot_bytes] = outline
    return bytes(out)


def build_inc(cfg: Config) -> str:
    """Generate TRSE-Pascal @define lines, one per configured phase."""
    lines = [
        "// AUTO-GENERATED by tools/sprites/build_c64_sprites.py",
    ]
    name_width = max((len(p.name) for p in cfg.phases), default=0)
    for phase in sorted(cfg.phases, key=lambda p: p.slot):
        idx = cfg.sprite_index_base + phase.slot
        upper = phase.name.upper()
        lines.append(f"@define {upper:<{name_width}}  {idx}")
    return "\n".join(lines) + "\n"


C64_PALETTE = {
    0:  (0, 0, 0),
    1:  (255, 255, 255),
    2:  (136, 0, 0),
    3:  (170, 255, 238),
    4:  (204, 68, 204),
    5:  (0, 204, 85),
    6:  (0, 0, 170),
    7:  (238, 238, 119),
    8:  (221, 136, 85),
    9:  (102, 68, 0),
    10: (255, 119, 119),
    11: (51, 51, 51),
    12: (119, 119, 119),
    13: (170, 255, 102),
    14: (0, 136, 255),
    15: (187, 187, 187),
}


def render_preview(cfg: Config, bin_data: bytes) -> Image.Image:
    """Decode .bin slots back to a labelled grid PNG using each phase's color."""
    pw, ph_h = cfg.sprite_size
    scale = 4
    cols = 7
    rows = (len(cfg.phases) + cols - 1) // cols
    pad_x, pad_y = 16, 50
    cell_w = pw * scale + pad_x
    cell_h = ph_h * scale + pad_y
    canvas_w = cell_w * cols + pad_x
    canvas_h = cell_h * rows + pad_y

    canvas = Image.new("RGB", (canvas_w, canvas_h), (40, 40, 50))
    sorted_phases = sorted(cfg.phases, key=lambda p: p.slot)

    for i, phase in enumerate(sorted_phases):
        offset = phase.slot * cfg.slot_bytes
        slot_bytes = bin_data[offset : offset + cfg.slot_bytes]
        sprite_img = Image.new("RGB", (pw, ph_h), (255, 255, 255))
        fg = C64_PALETTE.get(phase.color_fill, (0, 0, 0))
        for y in range(ph_h):
            for x in range(pw):
                byte = slot_bytes[y * 3 + x // 8]
                if byte & (1 << (7 - (x % 8))):
                    sprite_img.putpixel((x, y), fg)
        big = sprite_img.resize((pw * scale, ph_h * scale), Image.NEAREST)
        gx = (i % cols) * cell_w + pad_x
        gy = (i // cols) * cell_h + pad_y
        canvas.paste(big, (gx, gy))

    return canvas


def main() -> None:
    config_path = Path(__file__).parent / "sprite_phases.json"
    cfg = load_config(config_path)

    bin_data = build_bin(cfg)
    inc_text = build_inc(cfg)

    cfg.output_bin.parent.mkdir(parents=True, exist_ok=True)
    cfg.output_bin.write_bytes(bin_data)
    cfg.output_inc.write_text(inc_text, encoding="utf-8")

    cfg.preview_built.parent.mkdir(parents=True, exist_ok=True)
    preview = render_preview(cfg, bin_data)
    preview.save(cfg.preview_built)

    print(f"Wrote {cfg.output_bin} ({len(bin_data)} bytes)")
    print(f"Wrote {cfg.output_inc}")
    print(f"Wrote {cfg.preview_built}")


if __name__ == "__main__":
    main()
