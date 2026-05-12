# C64-Sprite-Overlay-Mode Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Pipeline `tools/sprites/build_c64_sprites.py` erzeugt pro Phase **zwei** 64-Byte-Hires-Sprites (Outline + Fill) in zwei getrennten Slot-Bänken. Bin wächst auf 4352 Byte (68 × 64). `kuno_sprites.inc` enthält `_OUTLINE`- und `_FILL`-Suffix-`@defines`. Splitting per Helligkeit (`dark_threshold`, `bright_threshold`) plus TGA-Background-Probe an Pixel (0,0).

**Architecture:**
- `Phase` Felder: `name, slot (0..33), color_outline, color_fill, src: FileSource`.
- `Config` Felder: zwei Thresholds, `total_slots=68`, alles andere wie bisher.
- `pack_phase` -> `pack_phase_pair(image, dark, bright) -> (outline_bytes, fill_bytes)`.
- `build_bin`: Outline-Bytes in untere Bank (Offset `slot * 64`), Fill-Bytes in obere Bank (Offset `(34 + slot) * 64`).
- `build_inc`: doppelte `@define`-Ausgabe pro Phase, Differenz konstant 34.
- `render_preview`: stack Fill unten, Outline oben pro Cell, mit `C64_PALETTE`-Farben.

**Tech Stack:** Python 3, Pillow, pytest. Hauptdatei: `tools/sprites/build_c64_sprites.py`. Tests: `tools/sprites/test_build_c64_sprites.py`.

**Spec:** `docs/superpowers/specs/2026-05-12-c64-sprite-overlay-mode-design.md`

**Vorgänger-Stand (Commit `2547c433`):** 34 Phasen file-based, `pack_phase` single, `kuno_sprites.bin` = 2176 Byte. Diese Arbeit wird in einem Schwung auf Overlay umgestellt -- altes single-`pack_phase`-Verhalten entfällt.

---

## File Structure

| Datei | Aktion |
|------|--------|
| `tools/sprites/build_c64_sprites.py` | modifizieren: `pack_phase` -> `pack_phase_pair`; `Phase`/`Config`-Felder; `load_config` validiert duale Thresholds + Farben; `build_bin` zwei-Bank-Layout; `build_inc` doppel-define; `render_preview` overlay-stack |
| `tools/sprites/test_build_c64_sprites.py` | modifizieren: `_write_config`-Helper neue Felder; `pack_phase`-Tests durch `pack_phase_pair`-Tests ersetzt; neue Validation-Tests; neue Bank-Offset-Tests; neue `_OUTLINE`/`_FILL`-`build_inc`-Tests |
| `tools/sprites/sprite_phases.json` | neu schreiben: 34 Phasen mit `color_outline`/`color_fill`; `dark_threshold` + `bright_threshold`; `total_slots: 68` |
| `src/main/trse/Kuno/sprites/kuno_sprites.bin` | generiert (Build-Output, 4352 Byte) |
| `src/main/trse/Kuno/sprites/kuno_sprites.inc` | generiert (Build-Output, 68 `@define`-Zeilen) |
| `tools/sprites/preview/sprites_built.png` | generiert (Build-Output) |

---

## Task 1: `pack_phase_pair` einführen (ersetzt `pack_phase`)

**Files:**
- Modify: `tools/sprites/build_c64_sprites.py`
- Test: `tools/sprites/test_build_c64_sprites.py`

- [ ] **Step 1: Failing Tests schreiben**

In `tools/sprites/test_build_c64_sprites.py` neue Tests, alte `pack_phase`-Tests löschen:

```python
from build_c64_sprites import pack_phase_pair


def test_pack_phase_pair_all_transparent_returns_two_zero_blocks():
    img = Image.new("RGBA", (24, 21), (255, 255, 255, 0))
    outline, fill = pack_phase_pair(img, dark_threshold=80, bright_threshold=240)
    assert outline == bytes(64)
    assert fill == bytes(64)


def test_pack_phase_pair_dark_pixel_goes_to_outline():
    img = Image.new("RGBA", (24, 21), (255, 255, 255, 0))
    img.putpixel((0, 0), (0, 0, 0, 255))  # avg=0 < dark
    outline, fill = pack_phase_pair(img, dark_threshold=80, bright_threshold=240)
    assert outline[0] == 0x80
    assert fill[0] == 0x00


def test_pack_phase_pair_midbright_pixel_goes_to_fill():
    img = Image.new("RGBA", (24, 21), (255, 255, 255, 0))
    img.putpixel((1, 0), (150, 150, 150, 255))  # avg=150
    outline, fill = pack_phase_pair(img, dark_threshold=80, bright_threshold=240)
    assert outline[0] == 0x00
    assert fill[0] == 0x40  # bit 6 of byte 0


def test_pack_phase_pair_very_bright_pixel_is_off_in_both():
    img = Image.new("RGBA", (24, 21), (255, 255, 255, 0))
    img.putpixel((0, 0), (255, 255, 255, 255))  # avg=255 >= bright
    outline, fill = pack_phase_pair(img, dark_threshold=80, bright_threshold=240)
    assert outline == bytes(64)
    assert fill == bytes(64)


def test_pack_phase_pair_alpha_zero_kills_both():
    img = Image.new("RGBA", (24, 21), (255, 255, 255, 0))
    img.putpixel((0, 0), (0, 0, 0, 0))
    outline, fill = pack_phase_pair(img, dark_threshold=80, bright_threshold=240)
    assert outline == bytes(64)
    assert fill == bytes(64)


def test_pack_phase_pair_tga_background_color_treated_transparent():
    """When pixel (0,0) is opaque, its RGB defines the background."""
    img = Image.new("RGBA", (24, 21), (255, 100, 100, 255))  # all bg
    img.putpixel((5, 5), (0, 0, 0, 255))  # dark foreground
    outline, fill = pack_phase_pair(img, dark_threshold=80, bright_threshold=240)
    expected = bytearray(64)
    expected[5 * 3 + 0] = 0x04  # bit (7 - 5) of byte 15
    assert outline == bytes(expected)
    assert fill == bytes(64)


def test_pack_phase_pair_threshold_boundary_dark():
    """brightness == dark_threshold => fill (NOT outline; outline uses '<')."""
    img = Image.new("RGBA", (24, 21), (255, 255, 255, 0))
    img.putpixel((0, 0), (80, 80, 80, 255))  # avg=80
    outline, fill = pack_phase_pair(img, dark_threshold=80, bright_threshold=240)
    assert outline[0] == 0x00
    assert fill[0] == 0x80


def test_pack_phase_pair_threshold_boundary_bright():
    """brightness == bright_threshold => off in both (fill uses '<')."""
    img = Image.new("RGBA", (24, 21), (255, 255, 255, 0))
    img.putpixel((0, 0), (240, 240, 240, 255))
    outline, fill = pack_phase_pair(img, dark_threshold=80, bright_threshold=240)
    assert outline[0] == 0x00
    assert fill[0] == 0x00
```

- [ ] **Step 2: Tests laufen lassen, Fail bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py -k pack_phase_pair -v`
Expected: alle FAIL mit `ImportError: cannot import name 'pack_phase_pair'`.

- [ ] **Step 3: Implementation**

In `tools/sprites/build_c64_sprites.py` `pack_phase` durch `pack_phase_pair` ersetzen:

```python
def pack_phase_pair(
    image: Image.Image,
    dark_threshold: int,
    bright_threshold: int,
) -> tuple[bytes, bytes]:
    """Pack a 24x21 RGBA image into (outline_bytes, fill_bytes) each 64 bytes.

    Background detection: pixel at (0,0) defines the background. If its
    alpha is 0 the alpha channel drives transparency; otherwise any pixel
    matching its RGB is treated transparent (TGA case).

    Outline bit set when alpha > 0, not background, and avg(R,G,B) < dark_threshold.
    Fill bit set when alpha > 0, not background, and dark_threshold <= avg < bright_threshold.
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
```

Alte `pack_phase`-Funktion komplett entfernen.

- [ ] **Step 4: Tests laufen lassen, Pass bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py -k pack_phase_pair -v`
Expected: 8 PASS.

Existierende `pack_phase`-Tests werden im selben Schritt gelöscht (sie schlagen sonst mit `ImportError` fehl):
- `test_pack_phase_all_transparent_returns_zeros`
- `test_pack_phase_top_left_pixel_sets_msb_of_byte0`
- `test_pack_phase_top_right_pixel_sets_lsb_of_byte2`
- `test_pack_phase_pixel_at_8_0_starts_byte1`
- `test_pack_phase_bottom_left_pixel_at_byte60`
- `test_pack_phase_full_first_row`
- `test_pack_phase_padding_byte_is_zero`
- `test_pack_phase_threshold_treats_light_gray_as_background`
- `test_pack_phase_threshold_treats_dark_gray_as_foreground`
- `test_pack_phase_alpha_zero_is_transparent_regardless_of_color`

- [ ] **Step 5: Commit**

```
git add tools/sprites/build_c64_sprites.py tools/sprites/test_build_c64_sprites.py
git commit -m "Replace pack_phase with pack_phase_pair for hires overlay"
```

---

## Task 2: `Phase`/`Config` neue Felder

**Files:**
- Modify: `tools/sprites/build_c64_sprites.py`
- Test: `tools/sprites/test_build_c64_sprites.py`

- [ ] **Step 1: Failing Test**

Den `_write_config`-Helper im Test-Modul ändern -- neue Schema-Felder einführen, dann Happy-Path-Test anpassen:

```python
def _write_config(tmp_path: Path, overrides: dict) -> Path:
    _make_src(tmp_path, "a.png")
    _make_src(tmp_path, "b.png")
    base = {
        "output_bin": "kuno_sprites.bin",
        "output_inc": "kuno_sprites.inc",
        "preview_built": "preview/sprites_built.png",
        "sprite_size": [24, 21],
        "slot_bytes": 64,
        "dark_threshold": 80,
        "bright_threshold": 240,
        "sprite_index_base": 200,
        "total_slots": 4,
        "phases": [
            {"name": "a", "slot": 0, "color_outline": 0, "color_fill": 14,
             "src": {"file": "a.png"}},
            {"name": "b", "slot": 1, "color_outline": 0, "color_fill": 5,
             "src": {"file": "b.png"}},
        ],
    }
    base.update(overrides)
    p = tmp_path / "config.json"
    p.write_text(json.dumps(base))
    return p


def test_load_config_happy_path(tmp_path):
    cfg = load_config(_write_config(tmp_path, {}))
    assert cfg.dark_threshold == 80
    assert cfg.bright_threshold == 240
    assert cfg.total_slots == 4
    assert len(cfg.phases) == 2
    assert cfg.phases[0].color_outline == 0
    assert cfg.phases[0].color_fill == 14
```

- [ ] **Step 2: Test laufen lassen, Fail bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py::test_load_config_happy_path -v`
Expected: FAIL (`Phase` hat noch `color` statt `color_outline`/`color_fill`; `Config` kennt `threshold` statt `dark_threshold`).

- [ ] **Step 3: Implementation**

In `tools/sprites/build_c64_sprites.py`:

```python
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
```

`load_config` so anpassen, dass es die neuen Felder liest:

```python
phases_list.append(
    Phase(
        name=p["name"],
        slot=p["slot"],
        color_outline=p["color_outline"],
        color_fill=p["color_fill"],
        src=FileSource(path=src_path),
    )
)
...
return Config(
    output_bin=...,
    output_inc=...,
    preview_built=...,
    sprite_size=sprite_size,
    slot_bytes=raw["slot_bytes"],
    dark_threshold=raw["dark_threshold"],
    bright_threshold=raw["bright_threshold"],
    sprite_index_base=raw["sprite_index_base"],
    total_slots=total,
    phases=tuple(phases_list),
)
```

- [ ] **Step 4: Test laufen lassen, Pass bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py::test_load_config_happy_path -v`
Expected: PASS.

- [ ] **Step 5: Commit**

```
git add tools/sprites/build_c64_sprites.py tools/sprites/test_build_c64_sprites.py
git commit -m "Switch Phase/Config to dual thresholds and dual colors"
```

---

## Task 3: `load_config`-Validation für Dual-Schema

**Files:**
- Modify: `tools/sprites/build_c64_sprites.py`
- Test: `tools/sprites/test_build_c64_sprites.py`

- [ ] **Step 1: Failing Tests**

```python
def test_load_config_requires_dark_threshold(tmp_path):
    cfg = _write_config(tmp_path, {})
    raw = json.loads(cfg.read_text())
    del raw["dark_threshold"]
    cfg.write_text(json.dumps(raw))
    with pytest.raises(ConfigError):
        load_config(cfg)


def test_load_config_requires_bright_threshold(tmp_path):
    cfg = _write_config(tmp_path, {})
    raw = json.loads(cfg.read_text())
    del raw["bright_threshold"]
    cfg.write_text(json.dumps(raw))
    with pytest.raises(ConfigError):
        load_config(cfg)


def test_load_config_rejects_dark_ge_bright(tmp_path):
    cfg = _write_config(tmp_path, {"dark_threshold": 200, "bright_threshold": 200})
    with pytest.raises(ConfigError):
        load_config(cfg)


def test_load_config_requires_color_outline(tmp_path):
    cfg = _write_config(
        tmp_path,
        {"phases": [
            {"name": "a", "slot": 0, "color_fill": 14, "src": {"file": "a.png"}},
            {"name": "b", "slot": 1, "color_outline": 0, "color_fill": 5,
             "src": {"file": "b.png"}},
        ]},
    )
    with pytest.raises(ConfigError):
        load_config(cfg)


def test_load_config_requires_color_fill(tmp_path):
    cfg = _write_config(
        tmp_path,
        {"phases": [
            {"name": "a", "slot": 0, "color_outline": 0, "src": {"file": "a.png"}},
            {"name": "b", "slot": 1, "color_outline": 0, "color_fill": 5,
             "src": {"file": "b.png"}},
        ]},
    )
    with pytest.raises(ConfigError):
        load_config(cfg)


def test_load_config_rejects_odd_total_slots(tmp_path):
    cfg = _write_config(tmp_path, {"total_slots": 3})
    with pytest.raises(ConfigError):
        load_config(cfg)


def test_load_config_slot_range_is_half_of_total(tmp_path):
    cfg = _write_config(
        tmp_path,
        {"total_slots": 4,
         "phases": [
             {"name": "a", "slot": 2, "color_outline": 0, "color_fill": 14,
              "src": {"file": "a.png"}},
             {"name": "b", "slot": 1, "color_outline": 0, "color_fill": 5,
              "src": {"file": "b.png"}},
         ]},
    )
    with pytest.raises(SlotOutOfRangeError):
        load_config(cfg)
```

Außerdem den bestehenden `test_load_config_rejects_slot_out_of_range` löschen (wird durch `test_load_config_slot_range_is_half_of_total` ersetzt).

- [ ] **Step 2: Tests laufen lassen, Fail bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py -k load_config -v`
Expected: neue Tests FAIL.

- [ ] **Step 3: Implementation**

In `load_config` vor dem Phase-Loop:

```python
if "dark_threshold" not in raw or "bright_threshold" not in raw:
    raise ConfigError("config requires dark_threshold and bright_threshold")
if raw["dark_threshold"] >= raw["bright_threshold"]:
    raise ConfigError(
        f"dark_threshold ({raw['dark_threshold']}) must be < "
        f"bright_threshold ({raw['bright_threshold']})"
    )
if raw["total_slots"] % 2 != 0:
    raise ConfigError(f"total_slots must be even (got {raw['total_slots']})")
```

Im Phase-Loop:

```python
if "color_outline" not in p or "color_fill" not in p:
    raise ConfigError(
        f"phase {p.get('name')!r} requires color_outline and color_fill"
    )
```

Im Slot-Validation-Loop die Range-Schranke halbieren:

```python
half = total // 2
for ph in phases_list:
    if ph.slot >= half or ph.slot < 0:
        raise SlotOutOfRangeError(
            f"phase {ph.name!r} has slot {ph.slot}, must be in [0, {half})"
        )
```

- [ ] **Step 4: Tests laufen lassen, Pass bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py -k load_config -v`
Expected: alle PASS.

- [ ] **Step 5: Commit**

```
git add tools/sprites/build_c64_sprites.py tools/sprites/test_build_c64_sprites.py
git commit -m "Validate dual thresholds, dual colors, even total_slots"
```

---

## Task 4: `build_bin` zwei-Bank-Layout

**Files:**
- Modify: `tools/sprites/build_c64_sprites.py`
- Test: `tools/sprites/test_build_c64_sprites.py`

- [ ] **Step 1: Failing Tests**

```python
def test_build_bin_total_size_doubles(tmp_path):
    cfg_path = _write_config(tmp_path, {"total_slots": 6})
    cfg = load_config(cfg_path)
    data = build_bin(cfg)
    assert len(data) == 6 * 64


def test_build_bin_outline_lands_in_lower_bank(tmp_path):
    """Dark pixel at (0,0) of phase slot=1 lands at byte 1*64."""
    img = Image.new("RGBA", (24, 21), (255, 255, 255, 0))
    img.putpixel((0, 0), (0, 0, 0, 255))
    img.save(tmp_path / "dark.png")
    cfg = _write_config(
        tmp_path,
        {"total_slots": 4,
         "phases": [{"name": "x", "slot": 1,
                     "color_outline": 0, "color_fill": 14,
                     "src": {"file": "dark.png"}}]},
    )
    data = build_bin(load_config(cfg))
    assert data[1 * 64] == 0x80
    # nothing in upper bank for this phase
    assert data[(2 + 1) * 64] == 0x00  # half=2, fill offset


def test_build_bin_fill_lands_in_upper_bank(tmp_path):
    """Mid-bright pixel at (0,0) of phase slot=1 lands at byte (half+1)*64."""
    img = Image.new("RGBA", (24, 21), (255, 255, 255, 0))
    img.putpixel((0, 0), (150, 150, 150, 255))
    img.save(tmp_path / "mid.png")
    cfg = _write_config(
        tmp_path,
        {"total_slots": 4,
         "phases": [{"name": "x", "slot": 1,
                     "color_outline": 0, "color_fill": 14,
                     "src": {"file": "mid.png"}}]},
    )
    data = build_bin(load_config(cfg))
    assert data[1 * 64] == 0x00            # outline bank slot 1 empty
    assert data[(2 + 1) * 64] == 0x80      # fill bank slot 1 (half=2)
```

Den bestehenden `test_build_bin_phase_at_correct_slot_offset` löschen (deckt jetzt `_outline_lands_in_lower_bank` ab).

- [ ] **Step 2: Tests laufen lassen, Fail bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py::test_build_bin_outline_lands_in_lower_bank tools/sprites/test_build_c64_sprites.py::test_build_bin_fill_lands_in_upper_bank -v`
Expected: FAIL (alte `build_bin` ruft `pack_phase` mit single-threshold auf).

- [ ] **Step 3: Implementation**

```python
def build_bin(cfg: Config) -> bytes:
    out = bytearray(cfg.total_slots * cfg.slot_bytes)
    half = cfg.total_slots // 2
    bank_offset = half * cfg.slot_bytes
    for phase in cfg.phases:
        img = Image.open(phase.src.path).convert("RGBA")
        outline, fill = pack_phase_pair(img, cfg.dark_threshold, cfg.bright_threshold)
        out_off = phase.slot * cfg.slot_bytes
        out[out_off : out_off + cfg.slot_bytes] = outline
        fill_off = bank_offset + phase.slot * cfg.slot_bytes
        out[fill_off : fill_off + cfg.slot_bytes] = fill
    return bytes(out)
```

- [ ] **Step 4: Tests laufen lassen, Pass bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py -k build_bin -v`
Expected: PASS.

- [ ] **Step 5: Commit**

```
git add tools/sprites/build_c64_sprites.py tools/sprites/test_build_c64_sprites.py
git commit -m "Pack outline and fill into two slot banks"
```

---

## Task 5: `build_inc` doppelte `@define` pro Phase

**Files:**
- Modify: `tools/sprites/build_c64_sprites.py`
- Test: `tools/sprites/test_build_c64_sprites.py`

- [ ] **Step 1: Failing Test**

Den existierenden `test_build_inc_emits_define_per_phase` ersetzen:

```python
def test_build_inc_emits_outline_and_fill_per_phase(tmp_path):
    cfg_path = _write_config(
        tmp_path,
        {"sprite_index_base": 200, "total_slots": 4,
         "phases": [
             {"name": "kuno_walk_left_0", "slot": 0,
              "color_outline": 0, "color_fill": 14, "src": {"file": "a.png"}},
             {"name": "gecko_left_0", "slot": 1,
              "color_outline": 0, "color_fill": 5, "src": {"file": "b.png"}},
         ]},
    )
    cfg = load_config(cfg_path)
    text = build_inc(cfg)
    assert "@define KUNO_WALK_LEFT_0_OUTLINE" in text
    assert "@define KUNO_WALK_LEFT_0_FILL" in text
    assert "@define GECKO_LEFT_0_OUTLINE" in text
    assert "@define GECKO_LEFT_0_FILL" in text
    # outline ids: 200, 201 ; fill ids: 200 + half(2) + slot = 202, 203
    assert "_OUTLINE  200" in text or "_OUTLINE    200" in text
    assert "200" in text and "202" in text and "203" in text


def test_build_inc_fill_offset_is_half_of_total_slots(tmp_path):
    cfg_path = _write_config(tmp_path, {})  # total_slots = 4
    cfg = load_config(cfg_path)
    text = build_inc(cfg)
    # a slot=0: OUTLINE=200, FILL=202; b slot=1: OUTLINE=201, FILL=203
    assert "A_OUTLINE" in text and "A_FILL" in text
    out_a = next(ln for ln in text.splitlines() if "A_OUTLINE" in ln)
    fill_a = next(ln for ln in text.splitlines() if "A_FILL" in ln)
    out_id = int(out_a.split()[-1])
    fill_id = int(fill_a.split()[-1])
    assert fill_id - out_id == 2  # half = total_slots/2
```

Bestehenden `test_build_inc_skips_unused_slots` löschen (Phase-Liste, nicht Slot-Liste).

- [ ] **Step 2: Tests laufen lassen, Fail bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py -k build_inc -v`
Expected: FAIL.

- [ ] **Step 3: Implementation**

```python
def build_inc(cfg: Config) -> str:
    lines = ["// AUTO-GENERATED by tools/sprites/build_c64_sprites.py"]
    half = cfg.total_slots // 2
    name_width = max(
        len(p.name) + len("_OUTLINE") for p in cfg.phases
    ) if cfg.phases else 0
    for phase in sorted(cfg.phases, key=lambda p: p.slot):
        out_id = cfg.sprite_index_base + phase.slot
        fill_id = cfg.sprite_index_base + half + phase.slot
        upper = phase.name.upper()
        lines.append(f"@define {(upper + '_OUTLINE'):<{name_width}}  {out_id}")
        lines.append(f"@define {(upper + '_FILL'):<{name_width}}  {fill_id}")
    return "\n".join(lines) + "\n"
```

- [ ] **Step 4: Tests laufen lassen, Pass bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py -k build_inc -v`
Expected: PASS.

- [ ] **Step 5: Commit**

```
git add tools/sprites/build_c64_sprites.py tools/sprites/test_build_c64_sprites.py
git commit -m "Emit dual @define per phase with constant outline/fill offset"
```

---

## Task 6: `render_preview` Overlay-Stack

**Files:**
- Modify: `tools/sprites/build_c64_sprites.py`
- Test: keine neuen Unit-Tests; manuell verifizieren beim Build-Run (siehe Task 8). Optional Sanity-Test, dass `render_preview` ohne Exception läuft.

- [ ] **Step 1: Optionaler Smoke-Test**

```python
def test_render_preview_runs_without_error(tmp_path):
    cfg_path = _write_config(tmp_path, {})
    cfg = load_config(cfg_path)
    data = build_bin(cfg)
    img = render_preview(cfg, data)
    assert img.size[0] > 0 and img.size[1] > 0
```

Falls schon vorhanden, neue Schema-Felder durchziehen.

- [ ] **Step 2: Implementation**

```python
def render_preview(cfg: Config, bin_data: bytes) -> Image.Image:
    pw, ph_h = cfg.sprite_size
    scale = 4
    cols = 7
    half = cfg.total_slots // 2
    rows = (len(cfg.phases) + cols - 1) // cols
    pad_x, pad_y = 16, 50
    cell_w = pw * scale + pad_x
    cell_h = ph_h * scale + pad_y
    canvas_w = cell_w * cols + pad_x
    canvas_h = cell_h * rows + pad_y

    canvas = Image.new("RGB", (canvas_w, canvas_h), (40, 40, 50))
    sorted_phases = sorted(cfg.phases, key=lambda p: p.slot)

    for i, phase in enumerate(sorted_phases):
        out_off = phase.slot * cfg.slot_bytes
        fill_off = (half + phase.slot) * cfg.slot_bytes
        outline = bin_data[out_off : out_off + cfg.slot_bytes]
        fill = bin_data[fill_off : fill_off + cfg.slot_bytes]
        sprite_img = Image.new("RGB", (pw, ph_h), (255, 255, 255))
        fc = C64_PALETTE.get(phase.color_fill, (0, 0, 0))
        oc = C64_PALETTE.get(phase.color_outline, (0, 0, 0))
        for y in range(ph_h):
            for x in range(pw):
                bit = 1 << (7 - (x % 8))
                if fill[y * 3 + x // 8] & bit:
                    sprite_img.putpixel((x, y), fc)
                if outline[y * 3 + x // 8] & bit:
                    sprite_img.putpixel((x, y), oc)
        big = sprite_img.resize((pw * scale, ph_h * scale), Image.NEAREST)
        gx = (i % cols) * cell_w + pad_x
        gy = (i // cols) * cell_h + pad_y
        canvas.paste(big, (gx, gy))

    return canvas
```

- [ ] **Step 3: Smoke-Test laufen lassen, Pass bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py::test_render_preview_runs_without_error -v`
Expected: PASS.

- [ ] **Step 4: Commit**

```
git add tools/sprites/build_c64_sprites.py tools/sprites/test_build_c64_sprites.py
git commit -m "Stack outline over fill in preview render"
```

---

## Task 7: `sprite_phases.json` auf Dual-Schema umstellen

**Files:**
- Rewrite: `tools/sprites/sprite_phases.json`

- [ ] **Step 1: JSON neu schreiben**

- `threshold: 200` -> `dark_threshold: 80`, `bright_threshold: 240`.
- `total_slots: 34` -> `total_slots: 68`.
- Pro Phase: `color: X` -> `color_outline: 0` und `color_fill: X` (alte Farbe).
- Slot-Indizes bleiben 0..33.
- `src.file`-Pfade bleiben unverändert.

Beispiel:

```json
{ "name": "kuno_walk_left_0", "slot": 4, "color_outline": 0, "color_fill": 14,
  "src": { "file": "../../img/KLINKS1.png" } }
```

- [ ] **Step 2: load_config-Run testen**

Run:

```
python -c "from build_c64_sprites import load_config; from pathlib import Path; cfg = load_config(Path('tools/sprites/sprite_phases.json')); print(cfg.total_slots, len(cfg.phases))"
```
Expected: `68 34`.

- [ ] **Step 3: Commit**

```
git add tools/sprites/sprite_phases.json
git commit -m "Update sprite_phases.json to dual-threshold dual-color schema"
```

---

## Task 8: End-to-End Build + Visual-Check

**Files:**
- Generate: `src/main/trse/Kuno/sprites/kuno_sprites.bin` (4352 Byte erwartet)
- Generate: `src/main/trse/Kuno/sprites/kuno_sprites.inc` (68 `@define`-Einträge erwartet)
- Generate: `tools/sprites/preview/sprites_built.png`

- [ ] **Step 1: Build laufen lassen**

```
cd tools/sprites && python build_c64_sprites.py
```
Expected-Output:

```
Wrote ...kuno_sprites.bin (4352 bytes)
Wrote ...kuno_sprites.inc
Wrote ...sprites_built.png
```

- [ ] **Step 2: Pytest komplett**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py -v`
Expected: alle PASS, keine `pack_phase`-, `_skips_unused_slots`-, `_phase_at_correct_slot_offset`- oder `_slot_out_of_range`-Tests mehr -- die wurden durch neue ersetzt.

- [ ] **Step 3: Inc-Inspektion**

Read `src/main/trse/Kuno/sprites/kuno_sprites.inc`.
Erwartet:
- Erste Daten-Zeile: `@define KUNO_SPAWN_0_OUTLINE  200`.
- Direkt danach: `@define KUNO_SPAWN_0_FILL     234`.
- Letzte Daten-Zeile: `@define WIZROT_3_FILL         267`.
- Genau 68 `@define`-Zeilen (`grep -c '^@define' kuno_sprites.inc` -> `68`).

- [ ] **Step 4: Visueller Check**

Read `tools/sprites/preview/sprites_built.png`.
Erwartet:
- Kuno-Walk/Stand-Phasen zeigen blauen Body **plus** schwarze Outline-Pixel (Hut-Kontur, Augen-Dots, Bein-Trennung).
- `kuno_spawn_0..3` zeigen kleinen Charakter, **nicht** ganz blauen Rahmen.
- `skelett_0..2` zeigen helle Knochen (Fill-Bank aktiv).
- Slimer/Gecko/Wizrot zeigen grünen Body + schwarze Outline-Akzente.

Falls Augen oder Arme noch nicht deutlich:
- `dark_threshold` höher drehen (z. B. 100, 120), damit blauer Body in den Fill rutscht und nur Hut + Augen + schwarze Outline-Pixel im Outline bleiben.
- Iteration: JSON anpassen, Build neu, Preview prüfen.

- [ ] **Step 5: Commit Build-Outputs**

```
git add src/main/trse/Kuno/sprites/kuno_sprites.bin src/main/trse/Kuno/sprites/kuno_sprites.inc tools/sprites/preview/sprites_built.png
git commit -m "Regenerate sprite bin/inc/preview for overlay mode"
```

Optional: Threshold-Tuning-Commits separat halten, falls Iteration nötig.

---

## Risiken / Notizen

- **Threshold-Iteration**: nach Task 8 können visuell ein bis zwei JSON-Anpassungen folgen. Jede ist ein eigener Mikro-Commit (`Tune dark_threshold to N for clearer Kuno outline`).
- **TRSE-Folge**: `main.ras` bekommt keine Änderung in diesem Plan -- die `_OUTLINE`/`_FILL`-Bezeichner werden noch nirgends konsumiert. Wird in separater Spec/Plan adressiert.
- **`.gitignore`**: `tools/sprites/__pycache__/` weiterhin ungetrackt; keine Aufgabe dieses Plans.
- **Backwards-Compat**: Single-Sprite-Output entfällt komplett. Wer das alte `pack_phase` importiert (extern), bricht -- gibt es im Repo nicht.
