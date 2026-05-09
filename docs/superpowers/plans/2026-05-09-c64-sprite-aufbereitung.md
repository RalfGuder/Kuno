# C64-Sprite-Aufbereitung Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Konvertiere `kuno-sprites.png` (640×63 RGBA, 27 belegte 24×21-Slots) deterministisch in C64-Hardware-Sprite-Daten, binde sie in `main.ras` ein und entferne die alten TRSE-Sample-Dateien.

**Architecture:** Ein eigenständiges Python-3-Skript (`tools/sprites/build_c64_sprites.py`), gesteuert von `tools/sprites/sprite_phases.json`. Das Skript liest das Quell-PNG, packt jede konfigurierte Phase nach C64-Hires-Sprite-Format (24 Bit/Zeile, MSB-first, 64-B-Slot), schreibt `kuno_sprites.bin` und ein TRSE-Pascal-`@define`-Include `kuno_sprites.inc`, und erzeugt eine Round-trip-Vorschau-PNG zur visuellen Verifikation.

**Tech Stack:** Python 3.11+, Pillow (`PIL`), pytest, TRSE (manuell), VICE-Emulator (manuell).

**Spec:** `docs/superpowers/specs/2026-05-09-c64-sprite-aufbereitung-design.md`

---

## File Structure

**New files:**
- `tools/sprites/build_c64_sprites.py` — Konvertierungsskript, Single-Entry-Point. Enthält `pack_phase`, `Config`-Loader, `build_bin`, `build_inc`, `render_preview`, `main`.
- `tools/sprites/sprite_phases.json` — Phasen-Konfiguration (27 Phasen, Pfade, Schwellwert).
- `tools/sprites/test_build_c64_sprites.py` — pytest-Tests für die Bit-Packing- und Validations-Logik.
- `tools/sprites/requirements.txt` — Pillow + pytest pinning.

**Generated files (vom Skript erzeugt, nicht committen außer `.bin`/`.inc`):**
- `src/main/trse/Kuno/sprites/kuno_sprites.bin` (1728 B)
- `src/main/trse/Kuno/sprites/kuno_sprites.inc` (TRSE-Pascal-Include)
- `tools/sprites/preview/sprites_built.png` (Round-trip-Vorschau)

**Modified files:**
- `src/main/trse/Kuno/main.ras` — `mySprites:incbin`-Pfad ändern, `@include` ergänzen.

**Deleted files:**
- `src/main/trse/Kuno/sprites/sprites.flf`
- `src/main/trse/Kuno/sprites/sprites.bin`
- `src/main/trse/Kuno/sprites/kuno.flf`

**Bereits committed (nicht anfassen):**
- `tools/sprites/preview/sprites_inventory.png` — Quell-Vorschau aus dem Brainstorming.

---

## Task 1: Projekt-Skeleton anlegen

**Files:**
- Create: `tools/sprites/requirements.txt`
- Create: `tools/sprites/build_c64_sprites.py` (Stub)
- Create: `tools/sprites/test_build_c64_sprites.py` (leer)

- [ ] **Step 1: Schreibe `tools/sprites/requirements.txt`**

```
Pillow>=10.0
pytest>=7.0
```

- [ ] **Step 2: Schreibe Stub `tools/sprites/build_c64_sprites.py`**

```python
"""Convert kuno-sprites.png into C64 hardware sprite data."""
from __future__ import annotations


def main() -> None:
    raise NotImplementedError("Task 7 wires this up.")


if __name__ == "__main__":
    main()
```

- [ ] **Step 3: Schreibe leeres Test-File `tools/sprites/test_build_c64_sprites.py`**

```python
"""Tests for build_c64_sprites."""
```

- [ ] **Step 4: Pillow und pytest installieren**

Run: `pip install -r tools/sprites/requirements.txt`
Expected: `Successfully installed Pillow-... pytest-...`

- [ ] **Step 5: Verify pytest läuft (auch wenn keine Tests da sind)**

Run: `pytest tools/sprites/test_build_c64_sprites.py -v`
Expected: `no tests ran` (Exit 5) oder `1 passed` mit collected=0 — beides OK; wichtig ist, dass pytest nicht ImportError wirft.

- [ ] **Step 6: Commit**

```bash
git add tools/sprites/requirements.txt tools/sprites/build_c64_sprites.py tools/sprites/test_build_c64_sprites.py
git commit -m "Add C64 sprite build script skeleton"
```

---

## Task 2: `pack_phase()` — Bit-Packing in C64-Hires-Format

Das ist die kritische Logik: 24×21-RGBA-Pixel → 64-Byte-Sprite-Daten, MSB-first.

**Files:**
- Modify: `tools/sprites/build_c64_sprites.py`
- Modify: `tools/sprites/test_build_c64_sprites.py`

- [ ] **Step 1: Schreibe Tests für `pack_phase`**

Inhalt von `tools/sprites/test_build_c64_sprites.py`:

```python
"""Tests for build_c64_sprites."""
from PIL import Image

from build_c64_sprites import pack_phase


def _blank() -> Image.Image:
    return Image.new("RGBA", (24, 21), (255, 255, 255, 0))


def test_pack_phase_all_transparent_returns_zeros():
    img = _blank()
    out = pack_phase(img, threshold=200)
    assert out == bytes(64)


def test_pack_phase_top_left_pixel_sets_msb_of_byte0():
    img = _blank()
    img.putpixel((0, 0), (0, 0, 0, 255))
    out = pack_phase(img, threshold=200)
    expected = bytearray(64)
    expected[0] = 0x80
    assert out == bytes(expected)


def test_pack_phase_top_right_pixel_sets_lsb_of_byte2():
    img = _blank()
    img.putpixel((23, 0), (0, 0, 0, 255))
    out = pack_phase(img, threshold=200)
    expected = bytearray(64)
    expected[2] = 0x01
    assert out == bytes(expected)


def test_pack_phase_pixel_at_8_0_starts_byte1():
    """Pixel x=8 is the MSB of the second byte in row 0."""
    img = _blank()
    img.putpixel((8, 0), (0, 0, 0, 255))
    out = pack_phase(img, threshold=200)
    expected = bytearray(64)
    expected[1] = 0x80
    assert out == bytes(expected)


def test_pack_phase_bottom_left_pixel_at_byte60():
    """Row 20 starts at byte 60 (20 * 3)."""
    img = _blank()
    img.putpixel((0, 20), (0, 0, 0, 255))
    out = pack_phase(img, threshold=200)
    expected = bytearray(64)
    expected[60] = 0x80
    assert out == bytes(expected)


def test_pack_phase_full_first_row():
    img = _blank()
    for x in range(24):
        img.putpixel((x, 0), (0, 0, 0, 255))
    out = pack_phase(img, threshold=200)
    assert out[0:3] == b"\xff\xff\xff"
    assert out[3:64] == bytes(61)


def test_pack_phase_padding_byte_is_zero():
    img = Image.new("RGBA", (24, 21), (0, 0, 0, 255))
    out = pack_phase(img, threshold=200)
    assert len(out) == 64
    assert out[63] == 0


def test_pack_phase_threshold_treats_light_gray_as_background():
    img = Image.new("RGBA", (24, 21), (220, 220, 220, 255))
    out = pack_phase(img, threshold=200)
    assert out == bytes(64)


def test_pack_phase_threshold_treats_dark_gray_as_foreground():
    img = Image.new("RGBA", (24, 21), (100, 100, 100, 255))
    out = pack_phase(img, threshold=200)
    # all 21 rows of 3 bytes each fully set
    assert out[0:63] == b"\xff" * 63
    assert out[63] == 0


def test_pack_phase_alpha_zero_is_transparent_regardless_of_color():
    img = _blank()
    img.putpixel((5, 5), (0, 0, 0, 0))  # black but fully transparent
    out = pack_phase(img, threshold=200)
    assert out == bytes(64)
```

- [ ] **Step 2: Run tests — sie müssen alle fehlschlagen mit ImportError**

Run: `cd tools/sprites && pytest test_build_c64_sprites.py -v`
Expected: `ImportError: cannot import name 'pack_phase' from 'build_c64_sprites'`

- [ ] **Step 3: Implementiere `pack_phase` in `build_c64_sprites.py`**

Ersetze den Stub-Inhalt mit:

```python
"""Convert kuno-sprites.png into C64 hardware sprite data."""
from __future__ import annotations

from PIL import Image


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
```

- [ ] **Step 4: Run tests — alle müssen passen**

Run: `cd tools/sprites && pytest test_build_c64_sprites.py -v`
Expected: `10 passed`

- [ ] **Step 5: Commit**

```bash
git add tools/sprites/build_c64_sprites.py tools/sprites/test_build_c64_sprites.py
git commit -m "Add pack_phase: 24x21 RGBA to C64 hires sprite bytes"
```

---

## Task 3: Config-Loader und Validation

JSON laden, in `Phase`/`Config`-Dataclasses überführen, Fehler-Cases abfangen.

**Files:**
- Modify: `tools/sprites/build_c64_sprites.py`
- Modify: `tools/sprites/test_build_c64_sprites.py`

- [ ] **Step 1: Tests für Config-Loader anhängen**

Am Ende von `tools/sprites/test_build_c64_sprites.py` einfügen:

```python
import json
from pathlib import Path

import pytest

from build_c64_sprites import (
    DuplicateSlotError,
    PhaseOutOfBoundsError,
    SlotOutOfRangeError,
    load_config,
)


def _write_config(tmp_path: Path, overrides: dict) -> Path:
    base = {
        "source_image": "kuno-sprites.png",
        "output_bin": "kuno_sprites.bin",
        "output_inc": "kuno_sprites.inc",
        "preview_built": "preview/sprites_built.png",
        "sprite_size": [24, 21],
        "slot_bytes": 64,
        "threshold": 200,
        "sprite_index_base": 200,
        "total_slots": 2,
        "phases": [
            {"name": "a", "slot": 0, "pos": [0, 0], "color": 14},
            {"name": "b", "slot": 1, "pos": [24, 0], "color": 5},
        ],
    }
    base.update(overrides)
    p = tmp_path / "config.json"
    p.write_text(json.dumps(base))
    return p


def test_load_config_happy_path(tmp_path):
    cfg = load_config(_write_config(tmp_path, {}))
    assert cfg.threshold == 200
    assert cfg.total_slots == 2
    assert len(cfg.phases) == 2
    assert cfg.phases[0].name == "a"
    assert cfg.phases[0].slot == 0
    assert cfg.phases[0].pos == (0, 0)
    assert cfg.phases[0].color == 14


def test_load_config_rejects_duplicate_slot(tmp_path):
    cfg_path = _write_config(
        tmp_path,
        {
            "phases": [
                {"name": "a", "slot": 0, "pos": [0, 0], "color": 14},
                {"name": "b", "slot": 0, "pos": [24, 0], "color": 5},
            ]
        },
    )
    with pytest.raises(DuplicateSlotError) as exc:
        load_config(cfg_path)
    assert "0" in str(exc.value)
    assert "a" in str(exc.value) and "b" in str(exc.value)


def test_load_config_rejects_slot_out_of_range(tmp_path):
    cfg_path = _write_config(
        tmp_path,
        {
            "total_slots": 2,
            "phases": [
                {"name": "a", "slot": 0, "pos": [0, 0], "color": 14},
                {"name": "b", "slot": 5, "pos": [24, 0], "color": 5},
            ],
        },
    )
    with pytest.raises(SlotOutOfRangeError) as exc:
        load_config(cfg_path)
    assert "5" in str(exc.value) and "2" in str(exc.value)


def test_load_config_rejects_missing_source_image(tmp_path):
    cfg = _write_config(tmp_path, {"source_image": "does_not_exist.png"})
    with pytest.raises(FileNotFoundError) as exc:
        load_config(cfg)
    assert "does_not_exist.png" in str(exc.value)
```

Note: `PhaseOutOfBoundsError` wird in Task 4 getestet, aber bereits hier importiert (existiert dann als Klasse).

- [ ] **Step 2: Run tests — neue müssen fehlschlagen mit ImportError**

Run: `cd tools/sprites && pytest test_build_c64_sprites.py -v`
Expected: ImportError für `DuplicateSlotError`, `PhaseOutOfBoundsError`, `SlotOutOfRangeError`, `load_config`

- [ ] **Step 3: Implementiere Config-Loader in `build_c64_sprites.py`**

Vor `def pack_phase(...)` einfügen:

```python
import json
from dataclasses import dataclass
from pathlib import Path


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
                f"phase {ph.name!r} has slot {ph.slot}, must be 0..{total - 1}"
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
```

Stelle sicher, dass die Test-Helper `_write_config` ein vorhandenes Source-Image referenziert. Die Tests setzen aktuell `"source_image": "kuno-sprites.png"` und schreiben die Config in `tmp_path` — das heißt, `tmp_path/kuno-sprites.png` muss existieren. Test-Helper erweitern:

```python
def _write_config(tmp_path: Path, overrides: dict) -> Path:
    # ensure source image exists for happy-path tests
    src = tmp_path / "kuno-sprites.png"
    if not src.exists():
        Image.new("RGBA", (640, 63), (255, 255, 255, 0)).save(src)
    base = {
        "source_image": "kuno-sprites.png",
        ...
    }
    ...
```

- [ ] **Step 4: Run tests**

Run: `cd tools/sprites && pytest test_build_c64_sprites.py -v`
Expected: alle 14 Tests `passed` (10 pack_phase + 4 load_config).

- [ ] **Step 5: Commit**

```bash
git add tools/sprites/build_c64_sprites.py tools/sprites/test_build_c64_sprites.py
git commit -m "Add config loader with slot/range/duplicate validation"
```

---

## Task 4: `build_bin()` — Phasen aus PNG extrahieren und packen

Liest das Quell-PNG, schneidet je Phase ein 24×21-Bild aus, schickt es durch `pack_phase`, schreibt ein `total_slots × 64`-Bytes-Block.

**Files:**
- Modify: `tools/sprites/build_c64_sprites.py`
- Modify: `tools/sprites/test_build_c64_sprites.py`

- [ ] **Step 1: Tests anhängen**

Am Ende von `test_build_c64_sprites.py`:

```python
from build_c64_sprites import build_bin


def test_build_bin_size_equals_total_slots_times_slot_bytes(tmp_path):
    cfg_path = _write_config(tmp_path, {"total_slots": 5})
    cfg = load_config(cfg_path)
    data = build_bin(cfg)
    assert len(data) == 5 * 64


def test_build_bin_unused_slots_are_zero(tmp_path):
    """Slots without a phase entry are zero-padded."""
    cfg_path = _write_config(
        tmp_path,
        {
            "total_slots": 3,
            "phases": [
                {"name": "a", "slot": 1, "pos": [0, 0], "color": 14},
            ],
        },
    )
    cfg = load_config(cfg_path)
    data = build_bin(cfg)
    assert data[0:64] == bytes(64)
    assert data[128:192] == bytes(64)


def test_build_bin_phase_at_correct_slot_offset(tmp_path):
    """A foreground pixel at (0,0) of phase slot=2 lands at byte 2*64."""
    src_dir = tmp_path
    img = Image.new("RGBA", (640, 63), (255, 255, 255, 0))
    img.putpixel((0, 0), (0, 0, 0, 255))  # one black pixel
    img.save(src_dir / "kuno-sprites.png")
    cfg_path = _write_config(
        tmp_path,
        {
            "total_slots": 3,
            "phases": [
                {"name": "x", "slot": 2, "pos": [0, 0], "color": 14},
            ],
        },
    )
    cfg = load_config(cfg_path)
    data = build_bin(cfg)
    assert data[2 * 64] == 0x80
    assert data[2 * 64 + 1] == 0x00


def test_build_bin_rejects_phase_out_of_bounds(tmp_path):
    cfg_path = _write_config(
        tmp_path,
        {
            "total_slots": 2,
            "phases": [
                {"name": "outside", "slot": 0, "pos": [620, 50], "color": 14},
                {"name": "inside", "slot": 1, "pos": [0, 0], "color": 5},
            ],
        },
    )
    cfg = load_config(cfg_path)
    with pytest.raises(PhaseOutOfBoundsError) as exc:
        build_bin(cfg)
    assert "outside" in str(exc.value)
    assert "620" in str(exc.value)
```

- [ ] **Step 2: Run — alle vier neue Tests müssen fehlschlagen mit ImportError**

Run: `cd tools/sprites && pytest test_build_c64_sprites.py -v`
Expected: ImportError für `build_bin`.

- [ ] **Step 3: Implementiere `build_bin`**

In `build_c64_sprites.py` nach `pack_phase` einfügen:

```python
def build_bin(cfg: Config) -> bytes:
    """Read source PNG and produce total_slots * slot_bytes of sprite data."""
    src = Image.open(cfg.source_image).convert("RGBA")
    sw, sh = src.size
    pw, ph = cfg.sprite_size

    out = bytearray(cfg.total_slots * cfg.slot_bytes)
    for phase in cfg.phases:
        x, y = phase.pos
        if x < 0 or y < 0 or x + pw > sw or y + ph > sh:
            raise PhaseOutOfBoundsError(
                f"phase {phase.name!r} at ({x},{y}) extends past image "
                f"({sw}x{sh}) with sprite_size ({pw}x{ph})"
            )
        crop = src.crop((x, y, x + pw, y + ph))
        packed = pack_phase(crop, cfg.threshold)
        offset = phase.slot * cfg.slot_bytes
        out[offset : offset + cfg.slot_bytes] = packed
    return bytes(out)
```

- [ ] **Step 4: Run tests**

Run: `cd tools/sprites && pytest test_build_c64_sprites.py -v`
Expected: alle 18 Tests `passed`.

- [ ] **Step 5: Commit**

```bash
git add tools/sprites/build_c64_sprites.py tools/sprites/test_build_c64_sprites.py
git commit -m "Add build_bin: extract phases from PNG into sprite block"
```

---

## Task 5: `build_inc()` — TRSE-Pascal-Konstanten generieren

**Files:**
- Modify: `tools/sprites/build_c64_sprites.py`
- Modify: `tools/sprites/test_build_c64_sprites.py`

- [ ] **Step 1: Tests anhängen**

```python
from build_c64_sprites import build_inc


def test_build_inc_emits_define_per_phase(tmp_path):
    cfg_path = _write_config(
        tmp_path,
        {
            "sprite_index_base": 200,
            "total_slots": 3,
            "phases": [
                {"name": "kuno_walk_left_0", "slot": 0, "pos": [0, 0], "color": 14},
                {"name": "gecko_left_0",     "slot": 2, "pos": [0, 21], "color": 5},
            ],
        },
    )
    cfg = load_config(cfg_path)
    text = build_inc(cfg)
    lines = text.splitlines()
    assert any(line.startswith("// AUTO-GENERATED") for line in lines)
    assert "@define KUNO_WALK_LEFT_0" in text
    assert "@define GECKO_LEFT_0" in text
    # index = base + slot
    assert "@define KUNO_WALK_LEFT_0  200" in text
    assert "@define GECKO_LEFT_0      202" in text


def test_build_inc_skips_unused_slots(tmp_path):
    """Empty slots produce no @define."""
    cfg_path = _write_config(
        tmp_path,
        {
            "total_slots": 5,
            "phases": [
                {"name": "a", "slot": 0, "pos": [0, 0], "color": 14},
            ],
        },
    )
    cfg = load_config(cfg_path)
    text = build_inc(cfg)
    assert text.count("@define") == 1
```

- [ ] **Step 2: Run tests — neue müssen mit ImportError fehlschlagen**

- [ ] **Step 3: Implementiere `build_inc`**

```python
def build_inc(cfg: Config) -> str:
    """Generate TRSE-Pascal @define lines, one per configured phase."""
    lines = [
        "// AUTO-GENERATED by tools/sprites/build_c64_sprites.py — do not edit",
        "",
    ]
    name_width = max((len(p.name) for p in cfg.phases), default=0)
    for phase in sorted(cfg.phases, key=lambda p: p.slot):
        idx = cfg.sprite_index_base + phase.slot
        upper = phase.name.upper()
        lines.append(f"@define {upper:<{name_width}}  {idx}")
    return "\n".join(lines) + "\n"
```

- [ ] **Step 4: Run tests**

Expected: alle 20 Tests `passed`.

- [ ] **Step 5: Commit**

```bash
git add tools/sprites/build_c64_sprites.py tools/sprites/test_build_c64_sprites.py
git commit -m "Add build_inc: TRSE-Pascal @define generator"
```

---

## Task 6: `render_preview()` — Round-trip-Vorschau-PNG

Decodiert die `.bin`-Bytes zurück zu Pixeln (mit der pro Phase konfigurierten C64-Farbe) und legt sie in einem Grid-PNG ab. Visuell verifizierbar — kein Unit-Test.

**Files:**
- Modify: `tools/sprites/build_c64_sprites.py`

- [ ] **Step 1: Implementiere `render_preview`**

In `build_c64_sprites.py`:

```python
# C64-Farbpalette (Approximationen der Hardware-Farben für die Vorschau)
C64_PALETTE = {
    0:  (0, 0, 0),         # black
    1:  (255, 255, 255),   # white
    2:  (136, 0, 0),       # red
    3:  (170, 255, 238),   # cyan
    4:  (204, 68, 204),    # purple
    5:  (0, 204, 85),      # green
    6:  (0, 0, 170),       # blue
    7:  (238, 238, 119),   # yellow
    8:  (221, 136, 85),    # orange
    9:  (102, 68, 0),      # brown
    10: (255, 119, 119),   # light red
    11: (51, 51, 51),      # dark grey
    12: (119, 119, 119),   # mid grey
    13: (170, 255, 102),   # light green
    14: (0, 136, 255),     # light blue
    15: (187, 187, 187),   # light grey
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
        fg = C64_PALETTE.get(phase.color, (0, 0, 0))
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
```

- [ ] **Step 2: Smoke-test manuell — keine pytest-Tests, aber kurz im REPL prüfen**

Run:
```python
python -c "
from pathlib import Path
import json, tempfile
from PIL import Image
from build_c64_sprites import load_config, build_bin, render_preview
# Use a tmp config for smoke
import os
os.chdir(r'D:\Projekte\Kuno\tools\sprites')
"
```

Diese Smoke-Aktion ist optional — Hauptverifikation kommt in Task 8 mit echten Daten.

- [ ] **Step 3: Commit**

```bash
git add tools/sprites/build_c64_sprites.py
git commit -m "Add render_preview: round-trip decoded PNG with C64 palette"
```

---

## Task 7: `main()`-Funktion und vollständige `sprite_phases.json`

Verdrahtet alles und schreibt die echte Konfiguration mit allen 27 Phasen.

**Files:**
- Modify: `tools/sprites/build_c64_sprites.py`
- Create: `tools/sprites/sprite_phases.json`

- [ ] **Step 1: Schreibe `tools/sprites/sprite_phases.json`**

```json
{
  "source_image": "../../src/main/resources/kuno-sprites.png",
  "output_bin":   "../../src/main/trse/Kuno/sprites/kuno_sprites.bin",
  "output_inc":   "../../src/main/trse/Kuno/sprites/kuno_sprites.inc",
  "preview_built": "preview/sprites_built.png",
  "sprite_size":  [24, 21],
  "slot_bytes":   64,
  "threshold":    200,
  "sprite_index_base": 200,
  "total_slots":  27,
  "phases": [
    { "name": "kuno_spawn_0",      "slot": 0,  "pos": [0, 0],   "color": 14 },
    { "name": "kuno_spawn_1",      "slot": 1,  "pos": [24, 0],  "color": 14 },
    { "name": "kuno_spawn_2",      "slot": 2,  "pos": [48, 0],  "color": 14 },
    { "name": "kuno_spawn_3",      "slot": 3,  "pos": [72, 0],  "color": 14 },
    { "name": "kuno_walk_left_0",  "slot": 4,  "pos": [96, 0],  "color": 14 },
    { "name": "kuno_walk_left_1",  "slot": 5,  "pos": [120, 0], "color": 14 },
    { "name": "kuno_jump_left",    "slot": 6,  "pos": [144, 0], "color": 14 },
    { "name": "kuno_stand_left",   "slot": 7,  "pos": [168, 0], "color": 14 },
    { "name": "kuno_walk_right_0", "slot": 8,  "pos": [192, 0], "color": 14 },
    { "name": "kuno_walk_right_1", "slot": 9,  "pos": [216, 0], "color": 14 },
    { "name": "kuno_jump_right",   "slot": 10, "pos": [240, 0], "color": 14 },
    { "name": "kuno_stand_right",  "slot": 11, "pos": [264, 0], "color": 14 },
    { "name": "slimer_left_0",     "slot": 12, "pos": [0, 21],  "color": 5  },
    { "name": "slimer_left_1",     "slot": 13, "pos": [24, 21], "color": 5  },
    { "name": "slimer_right_0",    "slot": 14, "pos": [48, 21], "color": 5  },
    { "name": "slimer_right_1",    "slot": 15, "pos": [72, 21], "color": 5  },
    { "name": "gecko_left_0",      "slot": 16, "pos": [96, 21], "color": 5  },
    { "name": "gecko_left_1",      "slot": 17, "pos": [120, 21],"color": 5  },
    { "name": "gecko_right_0",     "slot": 18, "pos": [144, 21],"color": 5  },
    { "name": "gecko_right_1",     "slot": 19, "pos": [168, 21],"color": 5  },
    { "name": "wizrot_0",          "slot": 20, "pos": [192, 21],"color": 5  },
    { "name": "wizrot_1",          "slot": 21, "pos": [216, 21],"color": 5  },
    { "name": "wizrot_2",          "slot": 22, "pos": [240, 21],"color": 5  },
    { "name": "wizrot_3",          "slot": 23, "pos": [264, 21],"color": 5  },
    { "name": "kuno_dead_0",       "slot": 24, "pos": [0, 42],  "color": 1  },
    { "name": "kuno_dead_1",       "slot": 25, "pos": [24, 42], "color": 1  },
    { "name": "kuno_dead_2",       "slot": 26, "pos": [48, 42], "color": 1  }
  ]
}
```

- [ ] **Step 2: Implementiere `main()`**

Ersetze den NotImplementedError-Stub:

```python
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
```

- [ ] **Step 3: Tests prüfen, dass Bestand stabil bleibt**

Run: `cd tools/sprites && pytest test_build_c64_sprites.py -v`
Expected: alle 20 Tests `passed`.

- [ ] **Step 4: Commit**

```bash
git add tools/sprites/build_c64_sprites.py tools/sprites/sprite_phases.json
git commit -m "Wire up main() and add full sprite_phases.json with 27 phases"
```

---

## Task 8: Skript-Lauf und visuelle Verifikation

- [ ] **Step 1: Skript laufen lassen**

Run: `python tools/sprites/build_c64_sprites.py`
Expected:
```
Wrote D:\Projekte\Kuno\src\main\trse\Kuno\sprites\kuno_sprites.bin (1728 bytes)
Wrote D:\Projekte\Kuno\src\main\trse\Kuno\sprites\kuno_sprites.inc
Wrote D:\Projekte\Kuno\tools\sprites\preview\sprites_built.png
```

- [ ] **Step 2: Output-Dateien verifizieren**

Run:
```bash
ls -la src/main/trse/Kuno/sprites/kuno_sprites.bin
ls -la src/main/trse/Kuno/sprites/kuno_sprites.inc
ls -la tools/sprites/preview/sprites_built.png
```
Expected: `.bin` ist genau 1728 Bytes, `.inc` enthält 27 `@define`-Zeilen plus Header, PNG existiert.

- [ ] **Step 3: `.inc` inspizieren**

Run: `head -30 src/main/trse/Kuno/sprites/kuno_sprites.inc`
Expected: Header-Kommentar, dann 27 Zeilen wie `@define KUNO_SPAWN_0       200`, `@define KUNO_SPAWN_1       201`, ..., `@define KUNO_DEAD_2        226`.

- [ ] **Step 4: Visuelle Verifikation**

Öffne `tools/sprites/preview/sprites_built.png` und vergleiche mit `tools/sprites/preview/sprites_inventory.png` (existiert bereits aus Brainstorming).
Expected: identische 27 Sprites, jeweils in der konfigurierten C64-Farbe (Kuno hellblau, Slimer/Gecko/Wizrot grün, Skelette weiß) auf weißem Hintergrund. Konturen müssen mit dem Original übereinstimmen.

Falls Konturen wegen Anti-Aliasing-Pixeln im Quell-PNG zerfasert wirken: in `sprite_phases.json` den `threshold` von `200` auf z. B. `180` oder `220` justieren und Skript erneut laufen lassen.

- [ ] **Step 5: Commit der Output-Dateien**

```bash
git add src/main/trse/Kuno/sprites/kuno_sprites.bin src/main/trse/Kuno/sprites/kuno_sprites.inc tools/sprites/preview/sprites_built.png
git commit -m "Generate kuno_sprites.bin/.inc and round-trip preview"
```

---

## Task 9: TRSE-Migration: `main.ras` anpassen, alte Dateien löschen

**Files:**
- Modify: `src/main/trse/Kuno/main.ras`
- Delete: `src/main/trse/Kuno/sprites/sprites.flf`
- Delete: `src/main/trse/Kuno/sprites/sprites.bin`
- Delete: `src/main/trse/Kuno/sprites/kuno.flf`

- [ ] **Step 1: `main.ras` anpassen**

Suche die Zeile:
```
mySprites:incbin("sprites/sprites.bin", @spriteLoc);
```

Ersetze durch (zwei Zeilen):
```
mySprites:incbin("sprites/kuno_sprites.bin", @spriteLoc);
```

Außerdem: `@include "sprites/kuno_sprites.inc"` direkt nach den anderen `@define`-Blöcken einfügen (vor `@define spriteLoc` würde nicht gehen wegen Reihenfolge; nach den existierenden `@define`s und vor den Konstanten-Deklarationen).

Konkrete Stelle: Nach Zeile mit `@define spriteLoc $3200`, vor `// Location of charset`:

```
@define spriteLoc $3200

@include "sprites/kuno_sprites.inc"

	// Location of charset
	const charsetLoc: address = $2000;
```

- [ ] **Step 2: Alte Dateien löschen**

Run:
```bash
git rm src/main/trse/Kuno/sprites/sprites.flf
git rm src/main/trse/Kuno/sprites/sprites.bin
git rm src/main/trse/Kuno/sprites/kuno.flf
```

Hinweis: `sprites.flf` und `kuno.flf` sind aktuell nicht im Repo committed (sie sind im aktuellen Branch ungetrackt — `git status` aus dem Brainstorming zeigt das). In dem Fall einfach `rm` statt `git rm` verwenden:

```bash
rm -f src/main/trse/Kuno/sprites/sprites.flf src/main/trse/Kuno/sprites/sprites.bin src/main/trse/Kuno/sprites/kuno.flf
```

- [ ] **Step 3: `git status` prüfen**

Run: `git status`
Expected: `main.ras` als modified, drei Sprite-Dateien als deleted (oder nur lokal entfernt), nichts Unerwartetes.

- [ ] **Step 4: Commit**

```bash
git add src/main/trse/Kuno/main.ras src/main/trse/Kuno/sprites/
git commit -m "Switch main.ras to kuno_sprites.bin and remove TRSE samples"
```

---

## Task 10: Manuelle End-to-End-Verifikation in TRSE und VICE

Diese Schritte sind nicht automatisierbar — TRSE und VICE haben keine Headless-CLI im Projekt-Setup.

- [ ] **Step 1: TRSE öffnen**

Starte die TRSE-IDE und öffne `src/main/trse/Kuno/Kuno.trse`.

- [ ] **Step 2: `main.ras` kompilieren**

Build/Compile auslösen.
Expected: `main.prg` und `main.sym` werden ohne Fehler aktualisiert. Insbesondere müssen die `KUNO_*`-, `SLIMER_*`-, `GECKO_*`-, `WIZROT_*`-, `KUNO_DEAD_*`-Konstanten aus `kuno_sprites.inc` als bekannt gelten.

Falls TRSE einen Fehler "Unknown identifier" wirft: prüfe, dass `@include "sprites/kuno_sprites.inc"` korrekt platziert ist (nach `@define`s, vor allen Verwendungen).

- [ ] **Step 3: `main.prg` im VICE-Emulator starten**

Lade `main.prg` in VICE.
Expected: Spiel startet, Titelbild zeigt Kuno-Sprite (oder einen der konfigurierten Sprites) korrekt — nicht als zufälliges Pixelmuster.

- [ ] **Step 4: Kompilierte Artefakte committen**

Run:
```bash
git add src/main/trse/Kuno/main.prg src/main/trse/Kuno/main.sym src/main/trse/Kuno/main.asm
git commit -m "Rebuild main.prg with new sprite data"
```

- [ ] **Step 5: Akzeptanzkriterien aus Spec abhaken**

Gehe `docs/superpowers/specs/2026-05-09-c64-sprite-aufbereitung-design.md` Abschnitt "Akzeptanzkriterien" durch und bestätige jeden Punkt:
1. ✅ `build_c64_sprites.py` und `sprite_phases.json` existieren wie spezifiziert.
2. ✅ Skript-Lauf erzeugt 1728-B `.bin` und 27-Zeilen `.inc` deterministisch.
3. ✅ `sprites_built.png` zeigt visuell dieselben 27 Sprites wie das Original.
4. ✅ Drei alte Dateien gelöscht.
5. ✅ `main.ras` referenziert `kuno_sprites.bin` und includiert `kuno_sprites.inc`.
6. ✅ Vier Fehlerfälle (`PhaseOutOfBoundsError`, `DuplicateSlotError`, `SlotOutOfRangeError`, `FileNotFoundError`) durch Tests verifiziert.
