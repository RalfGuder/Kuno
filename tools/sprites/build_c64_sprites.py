"""Convert kuno-sprites.png into C64 hardware sprite data."""
from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path

from PIL import Image


class PhaseOutOfBoundsError(ValueError):
    """A phase position is outside the source image."""


class DuplicateSlotError(ValueError):
    """Two phases share the same slot."""


class SlotOutOfRangeError(ValueError):
    """A phase slot is >= total_slots."""


@dataclass(frozen=True)
class Phase:
    name: str
    slot: int
    pos: tuple[int, int]
    color: int


@dataclass(frozen=True)
class Config:
    source_image: Path
    output_bin: Path
    output_inc: Path
    preview_built: Path
    sprite_size: tuple[int, int]
    slot_bytes: int
    threshold: int
    sprite_index_base: int
    total_slots: int
    phases: tuple[Phase, ...]


def load_config(config_path: Path) -> Config:
    """Load and validate a sprite_phases.json from disk.

    Paths in the JSON are resolved relative to the config file's directory.
    Raises DuplicateSlotError, SlotOutOfRangeError, FileNotFoundError.
    """
    config_path = Path(config_path)
    with config_path.open("r", encoding="utf-8") as f:
        raw = json.load(f)

    base_dir = config_path.parent
    source = (base_dir / raw["source_image"]).resolve()
    if not source.exists():
        raise FileNotFoundError(f"source_image not found: {source}")

    phases = tuple(
        Phase(
            name=p["name"],
            slot=p["slot"],
            pos=tuple(p["pos"]),
            color=p["color"],
        )
        for p in raw["phases"]
    )

    total = raw["total_slots"]
    seen_slots: dict[int, str] = {}
    for ph in phases:
        if ph.slot >= total or ph.slot < 0:
            raise SlotOutOfRangeError(
                f"phase {ph.name!r} has slot {ph.slot}, must be in [0, {total})"
            )
        if ph.slot in seen_slots:
            raise DuplicateSlotError(
                f"slot {ph.slot} used by {seen_slots[ph.slot]!r} and {ph.name!r}"
            )
        seen_slots[ph.slot] = ph.name

    return Config(
        source_image=source,
        output_bin=(base_dir / raw["output_bin"]).resolve(),
        output_inc=(base_dir / raw["output_inc"]).resolve(),
        preview_built=(base_dir / raw["preview_built"]).resolve(),
        sprite_size=tuple(raw["sprite_size"]),
        slot_bytes=raw["slot_bytes"],
        threshold=raw["threshold"],
        sprite_index_base=raw["sprite_index_base"],
        total_slots=total,
        phases=phases,
    )


def pack_phase(image: Image.Image, threshold: int) -> bytes:
    """Pack a 24x21 RGBA image into 64 bytes of C64 hires sprite data.

    Layout: each row uses 3 bytes (24 bits), MSB-first. Row y starts at
    byte y*3. The 64th byte is slot padding (always 0). A pixel is
    foreground when alpha > 0 AND (R+G+B)/3 < threshold.
    """
    if image.size != (24, 21):
        raise ValueError(f"expected 24x21 image, got {image.size}")
    out = bytearray(64)
    for y in range(21):
        for x in range(24):
            r, g, b, a = image.getpixel((x, y))
            if a > 0 and (r + g + b) // 3 < threshold:
                byte_idx = y * 3 + x // 8
                bit_idx = 7 - (x % 8)
                out[byte_idx] |= 1 << bit_idx
    return bytes(out)


def main() -> None:
    raise NotImplementedError("Task 7 wires this up.")


if __name__ == "__main__":
    main()
