# C64-Sprite-Pipeline Rework Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Pipeline `tools/sprites/build_c64_sprites.py` lädt pro Phase entweder eine eigene 24×21-Datei (PNG/TGA) aus `img/` oder weiterhin eine Sheet-Region; 34 Phasen in `sprite_phases.json`, davon 26 file-basiert (Kuno/Slimer/Skelett) und 8 sheet-basiert (Gecko/Wizrot).

**Architecture:** `Phase` bekommt ein `src`-Feld (Union aus `FileSource{path}` und `SheetSource{path,pos}`). `load_config` validiert XOR und prüft FileSource-Dimensionen eager. Eine neue Funktion `resolve_source` liefert pro Phase ein 24×21-RGBA-Image, mit Sheet-Cache für mehrfach genutzte Sheets. `pack_phase` bleibt unverändert.

**Tech Stack:** Python 3, Pillow (PNG+TGA), pytest. Existierende Datei: `tools/sprites/build_c64_sprites.py`. Tests: `tools/sprites/test_build_c64_sprites.py`.

**Spec:** `docs/superpowers/specs/2026-05-10-c64-sprite-pipeline-rework-design.md`

---

## File Structure

| Datei | Aktion |
|------|--------|
| `tools/sprites/build_c64_sprites.py` | modifizieren: `FileSource`, `SheetSource`, `Source`, `Phase.src`, `load_config`, `resolve_source` (neu), `build_bin` |
| `tools/sprites/test_build_c64_sprites.py` | modifizieren: `_write_config`-Helper auf neues Schema; neue Tests für `src`-Schema, XOR-Validation, Dimension-Validation, TGA-Loader, Sheet-Cache |
| `tools/sprites/sprite_phases.json` | neu schreiben: 34 Phasen mit `src` |
| `tools/sprites/fixtures/` | neu anlegen: kleine Fixture-PNGs/TGAs für Tests |
| `src/main/trse/Kuno/sprites/kuno_sprites.bin` | generiert (Build-Output) |
| `src/main/trse/Kuno/sprites/kuno_sprites.inc` | generiert (Build-Output) |
| `tools/sprites/preview/sprites_built.png` | generiert (Build-Output) |

---

## Task 1: Source-Datentypen `FileSource` und `SheetSource`

**Files:**
- Modify: `tools/sprites/build_c64_sprites.py`
- Test: `tools/sprites/test_build_c64_sprites.py`

- [ ] **Step 1: Failing Test schreiben**

Append in `tools/sprites/test_build_c64_sprites.py`:

```python
from build_c64_sprites import FileSource, SheetSource


def test_filesource_is_frozen_dataclass():
    src = FileSource(path=Path("img/KLINKS1.png"))
    assert src.path == Path("img/KLINKS1.png")
    with pytest.raises(Exception):
        src.path = Path("other.png")  # frozen


def test_sheetsource_is_frozen_dataclass():
    src = SheetSource(path=Path("sheet.png"), pos=(96, 0))
    assert src.path == Path("sheet.png")
    assert src.pos == (96, 0)
    with pytest.raises(Exception):
        src.pos = (0, 0)  # frozen
```

- [ ] **Step 2: Test laufen lassen, Fail bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py::test_filesource_is_frozen_dataclass tools/sprites/test_build_c64_sprites.py::test_sheetsource_is_frozen_dataclass -v`
Expected: FAIL mit `ImportError: cannot import name 'FileSource'`

- [ ] **Step 3: Implementation**

In `tools/sprites/build_c64_sprites.py`, nach dem bestehenden `from PIL import Image`-Block:

```python
from typing import Union


@dataclass(frozen=True)
class FileSource:
    """Single 24x21 image file (PNG or TGA) as sprite source."""
    path: Path


@dataclass(frozen=True)
class SheetSource:
    """Region of a larger sheet image, cropped at pos to 24x21."""
    path: Path
    pos: tuple[int, int]


Source = Union[FileSource, SheetSource]
```

- [ ] **Step 4: Test laufen lassen, Pass bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py::test_filesource_is_frozen_dataclass tools/sprites/test_build_c64_sprites.py::test_sheetsource_is_frozen_dataclass -v`
Expected: PASS

- [ ] **Step 5: Commit**

```
git add tools/sprites/build_c64_sprites.py tools/sprites/test_build_c64_sprites.py
git commit -m "Add FileSource and SheetSource dataclasses"
```

---

## Task 2: `Phase.src` ersetzt `Phase.pos`; `load_config` parst neues Schema

Dieser Task ist groß, weil sowohl Datentyp als auch alle bestehenden Tests umgestellt werden müssen.

**Files:**
- Modify: `tools/sprites/build_c64_sprites.py` (Phase-Klasse, load_config)
- Modify: `tools/sprites/test_build_c64_sprites.py` (alle Tests, die `pos` im JSON verwenden)

- [ ] **Step 1: Failing Test für neues Schema schreiben**

Append in `tools/sprites/test_build_c64_sprites.py`:

```python
def test_load_config_phase_with_file_src(tmp_path):
    img_path = tmp_path / "kuno.png"
    Image.new("RGBA", (24, 21), (0, 0, 0, 255)).save(img_path)
    cfg_data = {
        "output_bin": "out.bin",
        "output_inc": "out.inc",
        "preview_built": "preview.png",
        "sprite_size": [24, 21],
        "slot_bytes": 64,
        "threshold": 200,
        "sprite_index_base": 200,
        "total_slots": 1,
        "phases": [
            {"name": "p", "slot": 0, "color": 14, "src": {"file": "kuno.png"}},
        ],
    }
    cfg_path = tmp_path / "config.json"
    cfg_path.write_text(json.dumps(cfg_data))
    cfg = load_config(cfg_path)
    assert len(cfg.phases) == 1
    assert isinstance(cfg.phases[0].src, FileSource)
    assert cfg.phases[0].src.path == (tmp_path / "kuno.png").resolve()


def test_load_config_phase_with_sheet_src(tmp_path):
    sheet_path = tmp_path / "sheet.png"
    Image.new("RGBA", (640, 63), (255, 255, 255, 0)).save(sheet_path)
    cfg_data = {
        "output_bin": "out.bin",
        "output_inc": "out.inc",
        "preview_built": "preview.png",
        "sprite_size": [24, 21],
        "slot_bytes": 64,
        "threshold": 200,
        "sprite_index_base": 200,
        "total_slots": 1,
        "phases": [
            {"name": "g", "slot": 0, "color": 5,
             "src": {"sheet": "sheet.png", "pos": [96, 21]}},
        ],
    }
    cfg_path = tmp_path / "config.json"
    cfg_path.write_text(json.dumps(cfg_data))
    cfg = load_config(cfg_path)
    assert isinstance(cfg.phases[0].src, SheetSource)
    assert cfg.phases[0].src.path == (tmp_path / "sheet.png").resolve()
    assert cfg.phases[0].src.pos == (96, 21)
```

- [ ] **Step 2: Test laufen lassen, Fail bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py::test_load_config_phase_with_file_src -v`
Expected: FAIL — load_config kennt `src`-Feld nicht, KeyError oder ValidationError.

- [ ] **Step 3: `Phase`-Dataclass und `load_config` umstellen**

In `tools/sprites/build_c64_sprites.py`:

```python
@dataclass(frozen=True)
class Phase:
    name: str
    slot: int
    color: int
    src: Source
```

(Das alte `pos`-Feld entfällt.)

`Config.source_image` entfällt. `load_config` umschreiben:

```python
def load_config(config_path: Path) -> Config:
    config_path = Path(config_path)
    with config_path.open("r", encoding="utf-8") as f:
        raw = json.load(f)

    base_dir = config_path.parent

    def _parse_src(name: str, src_raw: dict) -> Source:
        if "file" in src_raw and "sheet" in src_raw:
            raise ConfigError(
                f"phase {name!r}: src has both 'file' and 'sheet'"
            )
        if "file" in src_raw:
            path = (base_dir / src_raw["file"]).resolve()
            if not path.exists():
                raise FileNotFoundError(f"phase {name!r}: file not found: {path}")
            with Image.open(path) as img:
                if img.size != (24, 21):
                    raise ValueError(
                        f"phase {name!r}: {path} expected 24x21, got {img.size}"
                    )
            return FileSource(path=path)
        if "sheet" in src_raw:
            path = (base_dir / src_raw["sheet"]).resolve()
            if not path.exists():
                raise FileNotFoundError(f"phase {name!r}: sheet not found: {path}")
            return SheetSource(path=path, pos=tuple(src_raw["pos"]))
        raise ConfigError(f"phase {name!r}: src has neither 'file' nor 'sheet'")

    phases = tuple(
        Phase(
            name=p["name"],
            slot=p["slot"],
            color=p["color"],
            src=_parse_src(p["name"], p["src"]),
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

`Config`-Dataclass anpassen (Feld `source_image` entfernen):

```python
@dataclass(frozen=True)
class Config:
    output_bin: Path
    output_inc: Path
    preview_built: Path
    sprite_size: tuple[int, int]
    slot_bytes: int
    threshold: int
    sprite_index_base: int
    total_slots: int
    phases: tuple[Phase, ...]
```

Neue Exception-Klasse hinzufügen:

```python
class ConfigError(ValueError):
    """Phase config has invalid src specification."""
```

- [ ] **Step 4: Bestehende Tests in `test_build_c64_sprites.py` migrieren**

`_write_config`-Helper umstellen, sodass die default-Phasen `src` statt `pos` verwenden:

```python
def _write_config(tmp_path: Path, overrides: dict) -> Path:
    src = tmp_path / "kuno-sprites.png"
    if not src.exists():
        Image.new("RGBA", (640, 63), (255, 255, 255, 0)).save(src)
    base = {
        "output_bin": "kuno_sprites.bin",
        "output_inc": "kuno_sprites.inc",
        "preview_built": "preview/sprites_built.png",
        "sprite_size": [24, 21],
        "slot_bytes": 64,
        "threshold": 200,
        "sprite_index_base": 200,
        "total_slots": 2,
        "phases": [
            {"name": "a", "slot": 0, "color": 14,
             "src": {"sheet": "kuno-sprites.png", "pos": [0, 0]}},
            {"name": "b", "slot": 1, "color": 5,
             "src": {"sheet": "kuno-sprites.png", "pos": [24, 0]}},
        ],
    }
    base.update(overrides)
    p = tmp_path / "config.json"
    p.write_text(json.dumps(base))
    return p
```

In allen Tests, die `phases`-Override geben, das alte `"pos": [x, y]` durch `"src": {"sheet": "kuno-sprites.png", "pos": [x, y]}` ersetzen. Konkret betroffen:
- `test_load_config_rejects_duplicate_slot`
- `test_load_config_rejects_slot_out_of_range`
- `test_build_bin_unused_slots_are_zero`
- `test_build_bin_phase_at_correct_slot_offset`
- `test_build_bin_rejects_phase_out_of_bounds`
- `test_build_inc_emits_define_per_phase`
- `test_build_inc_skips_unused_slots`

In `test_load_config_happy_path`: `cfg.phases[0].pos` → `cfg.phases[0].src.pos`, plus `assert isinstance(cfg.phases[0].src, SheetSource)`.

`test_load_config_rejects_missing_source_image` entfernen — `source_image` existiert nicht mehr; Existenz wird jetzt pro Phase im `src`-Block geprüft.

- [ ] **Step 5: Alle Tests laufen lassen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py -v`
Expected: alle Tests, die migriert wurden, PASS. Die zwei neuen `test_load_config_phase_with_*_src`-Tests PASS.

- [ ] **Step 6: Commit**

```
git add tools/sprites/build_c64_sprites.py tools/sprites/test_build_c64_sprites.py
git commit -m "Switch Phase schema to src union (FileSource | SheetSource)"
```

---

## Task 3: `load_config` lehnt ungültige `src`-Spezifikationen ab

**Files:**
- Modify: `tools/sprites/test_build_c64_sprites.py`

(Code dafür ist in Task 2 schon drin -- jetzt Tests, die das verifizieren.)

- [ ] **Step 1: Failing Tests schreiben**

```python
def test_load_config_rejects_src_with_both_file_and_sheet(tmp_path):
    img_path = tmp_path / "kuno.png"
    Image.new("RGBA", (24, 21)).save(img_path)
    sheet_path = tmp_path / "sheet.png"
    Image.new("RGBA", (640, 63)).save(sheet_path)
    cfg_data = {
        "output_bin": "out.bin", "output_inc": "out.inc",
        "preview_built": "p.png", "sprite_size": [24, 21],
        "slot_bytes": 64, "threshold": 200, "sprite_index_base": 200,
        "total_slots": 1,
        "phases": [{
            "name": "bad", "slot": 0, "color": 14,
            "src": {"file": "kuno.png", "sheet": "sheet.png", "pos": [0, 0]},
        }],
    }
    cfg_path = tmp_path / "c.json"
    cfg_path.write_text(json.dumps(cfg_data))
    from build_c64_sprites import ConfigError
    with pytest.raises(ConfigError) as exc:
        load_config(cfg_path)
    assert "bad" in str(exc.value)


def test_load_config_rejects_src_with_neither(tmp_path):
    cfg_data = {
        "output_bin": "out.bin", "output_inc": "out.inc",
        "preview_built": "p.png", "sprite_size": [24, 21],
        "slot_bytes": 64, "threshold": 200, "sprite_index_base": 200,
        "total_slots": 1,
        "phases": [{
            "name": "empty", "slot": 0, "color": 14,
            "src": {},
        }],
    }
    cfg_path = tmp_path / "c.json"
    cfg_path.write_text(json.dumps(cfg_data))
    from build_c64_sprites import ConfigError
    with pytest.raises(ConfigError) as exc:
        load_config(cfg_path)
    assert "empty" in str(exc.value)
```

- [ ] **Step 2: Tests laufen lassen, Pass bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py::test_load_config_rejects_src_with_both_file_and_sheet tools/sprites/test_build_c64_sprites.py::test_load_config_rejects_src_with_neither -v`
Expected: PASS (Code ist aus Task 2 schon implementiert).

- [ ] **Step 3: Commit**

```
git add tools/sprites/test_build_c64_sprites.py
git commit -m "Test: load_config rejects src with both/neither file and sheet"
```

---

## Task 4: `load_config` validiert FileSource-Dimension eager

**Files:**
- Modify: `tools/sprites/test_build_c64_sprites.py`

- [ ] **Step 1: Failing Test schreiben**

```python
def test_load_config_rejects_file_with_wrong_dimensions(tmp_path):
    bad_img = tmp_path / "wrong.png"
    Image.new("RGBA", (32, 32)).save(bad_img)
    cfg_data = {
        "output_bin": "out.bin", "output_inc": "out.inc",
        "preview_built": "p.png", "sprite_size": [24, 21],
        "slot_bytes": 64, "threshold": 200, "sprite_index_base": 200,
        "total_slots": 1,
        "phases": [{
            "name": "wrongsize", "slot": 0, "color": 14,
            "src": {"file": "wrong.png"},
        }],
    }
    cfg_path = tmp_path / "c.json"
    cfg_path.write_text(json.dumps(cfg_data))
    with pytest.raises(ValueError) as exc:
        load_config(cfg_path)
    assert "wrongsize" in str(exc.value)
    assert "24x21" in str(exc.value) or "(24, 21)" in str(exc.value)


def test_load_config_rejects_file_that_does_not_exist(tmp_path):
    cfg_data = {
        "output_bin": "out.bin", "output_inc": "out.inc",
        "preview_built": "p.png", "sprite_size": [24, 21],
        "slot_bytes": 64, "threshold": 200, "sprite_index_base": 200,
        "total_slots": 1,
        "phases": [{
            "name": "ghost", "slot": 0, "color": 14,
            "src": {"file": "does_not_exist.png"},
        }],
    }
    cfg_path = tmp_path / "c.json"
    cfg_path.write_text(json.dumps(cfg_data))
    with pytest.raises(FileNotFoundError) as exc:
        load_config(cfg_path)
    assert "does_not_exist.png" in str(exc.value)
```

- [ ] **Step 2: Tests laufen lassen, Pass bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py::test_load_config_rejects_file_with_wrong_dimensions tools/sprites/test_build_c64_sprites.py::test_load_config_rejects_file_that_does_not_exist -v`
Expected: PASS.

- [ ] **Step 3: Commit**

```
git add tools/sprites/test_build_c64_sprites.py
git commit -m "Test: load_config eager-validates FileSource existence and dimension"
```

---

## Task 5: `resolve_source` mit Sheet-Cache

**Files:**
- Modify: `tools/sprites/build_c64_sprites.py` (neue Funktion)
- Modify: `tools/sprites/test_build_c64_sprites.py` (Tests)

- [ ] **Step 1: Failing Tests schreiben**

```python
from build_c64_sprites import resolve_source


def test_resolve_source_filesource_returns_image(tmp_path):
    img_path = tmp_path / "p.png"
    img = Image.new("RGBA", (24, 21), (0, 0, 0, 255))
    img.putpixel((5, 5), (255, 0, 0, 255))
    img.save(img_path)
    cache = {}
    result = resolve_source(FileSource(path=img_path), cache)
    assert result.size == (24, 21)
    assert result.getpixel((5, 5)) == (255, 0, 0, 255)


def test_resolve_source_sheetsource_returns_cropped_region(tmp_path):
    sheet_path = tmp_path / "sheet.png"
    sheet = Image.new("RGBA", (640, 63), (255, 255, 255, 0))
    sheet.putpixel((50, 25), (0, 255, 0, 255))  # at sheet coords
    sheet.save(sheet_path)
    cache = {}
    result = resolve_source(
        SheetSource(path=sheet_path, pos=(48, 21)), cache
    )
    assert result.size == (24, 21)
    # sheet (50, 25) -> crop (48, 21) -> local (2, 4)
    assert result.getpixel((2, 4)) == (0, 255, 0, 255)


def test_resolve_source_caches_sheet_across_calls(tmp_path, monkeypatch):
    sheet_path = tmp_path / "sheet.png"
    Image.new("RGBA", (640, 63), (255, 255, 255, 0)).save(sheet_path)
    cache = {}

    open_calls = []
    real_open = Image.open

    def counting_open(*args, **kwargs):
        open_calls.append(args[0])
        return real_open(*args, **kwargs)

    monkeypatch.setattr("build_c64_sprites.Image.open", counting_open)

    src1 = SheetSource(path=sheet_path, pos=(0, 0))
    src2 = SheetSource(path=sheet_path, pos=(24, 0))
    resolve_source(src1, cache)
    resolve_source(src2, cache)

    sheet_opens = [c for c in open_calls if str(c) == str(sheet_path)]
    assert len(sheet_opens) == 1, f"sheet should open once, got {sheet_opens}"
```

- [ ] **Step 2: Tests laufen lassen, Fail bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py::test_resolve_source_filesource_returns_image -v`
Expected: FAIL mit `ImportError: cannot import name 'resolve_source'`.

- [ ] **Step 3: `resolve_source` implementieren**

In `tools/sprites/build_c64_sprites.py`:

```python
def resolve_source(
    src: Source, sheet_cache: dict[Path, Image.Image]
) -> Image.Image:
    """Return a 24x21 RGBA image for src, caching sheets across calls."""
    if isinstance(src, FileSource):
        return Image.open(src.path).convert("RGBA")
    # SheetSource
    sheet = sheet_cache.get(src.path)
    if sheet is None:
        sheet = Image.open(src.path).convert("RGBA")
        sheet_cache[src.path] = sheet
    x, y = src.pos
    sw, sh = sheet.size
    if x < 0 or y < 0 or x + 24 > sw or y + 21 > sh:
        raise PhaseOutOfBoundsError(
            f"sheet pos ({x},{y}) extends past image ({sw}x{sh})"
        )
    return sheet.crop((x, y, x + 24, y + 21))
```

- [ ] **Step 4: Tests laufen lassen, Pass bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py -k "resolve_source" -v`
Expected: alle drei `test_resolve_source_*` PASS.

- [ ] **Step 5: Commit**

```
git add tools/sprites/build_c64_sprites.py tools/sprites/test_build_c64_sprites.py
git commit -m "Add resolve_source with sheet cache"
```

---

## Task 6: `build_bin` nutzt `resolve_source` statt direktem Sheet-Crop

**Files:**
- Modify: `tools/sprites/build_c64_sprites.py` (build_bin neu schreiben)
- `tools/sprites/test_build_c64_sprites.py` (bestehende build_bin-Tests laufen weiter)

- [ ] **Step 1: `build_bin` umschreiben**

In `tools/sprites/build_c64_sprites.py`:

```python
def build_bin(cfg: Config) -> bytes:
    """Pack each phase into total_slots * slot_bytes of sprite data."""
    out = bytearray(cfg.total_slots * cfg.slot_bytes)
    sheet_cache: dict[Path, Image.Image] = {}
    for phase in cfg.phases:
        img = resolve_source(phase.src, sheet_cache)
        packed = pack_phase(img, cfg.threshold)
        offset = phase.slot * cfg.slot_bytes
        out[offset : offset + cfg.slot_bytes] = packed
    return bytes(out)
```

- [ ] **Step 2: Bestehende build_bin-Tests laufen lassen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py -k "build_bin" -v`
Expected: alle PASS (Tests verwenden `SheetSource` per `_write_config`-Helper aus Task 2).

- [ ] **Step 3: Vollen Test-Lauf**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py -v`
Expected: alle Tests PASS.

- [ ] **Step 4: Commit**

```
git add tools/sprites/build_c64_sprites.py
git commit -m "Rewrite build_bin to use resolve_source with sheet cache"
```

---

## Task 7: TGA-Loader-Test mit echter `KBEGINN1.TGA`

**Files:**
- Modify: `tools/sprites/test_build_c64_sprites.py` (TGA-Test)

- [ ] **Step 1: TGA-Test schreiben**

```python
def test_pack_phase_from_real_kbeginn_tga():
    """KBEGINN1.TGA aus img/ wird via Pillow korrekt geladen und gepackt."""
    repo_root = Path(__file__).resolve().parents[2]
    tga_path = repo_root / "img" / "KBEGINN1.TGA"
    assert tga_path.exists(), f"fixture missing: {tga_path}"
    img = Image.open(tga_path).convert("RGBA")
    assert img.size == (24, 21)
    out = pack_phase(img, threshold=200)
    assert len(out) == 64
    # mindestens ein Byte gesetzt -- spawn-Phase ist nicht komplett leer
    assert any(b != 0 for b in out[:63])


def test_resolve_source_loads_real_kbeginn_tga():
    repo_root = Path(__file__).resolve().parents[2]
    tga_path = repo_root / "img" / "KBEGINN1.TGA"
    cache = {}
    img = resolve_source(FileSource(path=tga_path), cache)
    assert img.size == (24, 21)
    assert img.mode == "RGBA"
```

- [ ] **Step 2: Tests laufen lassen, Pass bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py -k "kbeginn" -v`
Expected: PASS.

- [ ] **Step 3: Commit**

```
git add tools/sprites/test_build_c64_sprites.py
git commit -m "Test: TGA loading via Pillow with real KBEGINN1.TGA"
```

---

## Task 8: `sprite_phases.json` mit 34 Phasen schreiben

**Files:**
- Modify: `tools/sprites/sprite_phases.json` (komplett neu)

- [ ] **Step 1: Datei ersetzen**

`tools/sprites/sprite_phases.json` komplett mit folgendem Inhalt überschreiben:

```json
{
  "output_bin":   "../../src/main/trse/Kuno/sprites/kuno_sprites.bin",
  "output_inc":   "../../src/main/trse/Kuno/sprites/kuno_sprites.inc",
  "preview_built": "preview/sprites_built.png",
  "sprite_size":  [24, 21],
  "slot_bytes":   64,
  "threshold":    200,
  "sprite_index_base": 200,
  "total_slots":  34,
  "phases": [
    { "name": "kuno_spawn_0",      "slot": 0,  "color": 14, "src": { "file": "../../img/KBEGINN1.TGA" } },
    { "name": "kuno_spawn_1",      "slot": 1,  "color": 14, "src": { "file": "../../img/KBEGINN2.TGA" } },
    { "name": "kuno_spawn_2",      "slot": 2,  "color": 14, "src": { "file": "../../img/KBEGINN3.TGA" } },
    { "name": "kuno_spawn_3",      "slot": 3,  "color": 14, "src": { "file": "../../img/KBEGINN4.TGA" } },
    { "name": "kuno_walk_left_0",  "slot": 4,  "color": 14, "src": { "file": "../../img/KLINKS1.png" } },
    { "name": "kuno_walk_left_1",  "slot": 5,  "color": 14, "src": { "file": "../../img/KLINKS2.png" } },
    { "name": "kuno_walk_right_0", "slot": 6,  "color": 14, "src": { "file": "../../img/KRECHTS1.png" } },
    { "name": "kuno_walk_right_1", "slot": 7,  "color": 14, "src": { "file": "../../img/KRECHTS2.png" } },
    { "name": "kuno_stand_left",   "slot": 8,  "color": 14, "src": { "file": "../../img/KLIST.png" } },
    { "name": "kuno_stand_right",  "slot": 9,  "color": 14, "src": { "file": "../../img/KREST.png" } },
    { "name": "kuno_jump_left",    "slot": 10, "color": 14, "src": { "file": "../../img/KLISPR.png" } },
    { "name": "kuno_jump_right",   "slot": 11, "color": 14, "src": { "file": "../../img/KRESPR.png" } },
    { "name": "kuno_idle_0",       "slot": 12, "color": 14, "src": { "file": "../../img/KWAIT1.png" } },
    { "name": "kuno_idle_1",       "slot": 13, "color": 14, "src": { "file": "../../img/KWAIT2.png" } },
    { "name": "kuno_idle_2",       "slot": 14, "color": 14, "src": { "file": "../../img/KWAIT3.png" } },
    { "name": "kuno_idle_3",       "slot": 15, "color": 14, "src": { "file": "../../img/KWAIT4.png" } },
    { "name": "kuno_idle_4",       "slot": 16, "color": 14, "src": { "file": "../../img/KWAIT5.png" } },
    { "name": "kuno_ladder_0",     "slot": 17, "color": 14, "src": { "file": "../../img/KLEITER1.png" } },
    { "name": "kuno_ladder_1",     "slot": 18, "color": 14, "src": { "file": "../../img/KLEITER2.png" } },
    { "name": "slimer_left_0",     "slot": 19, "color": 5,  "src": { "file": "../../img/SLIMER1.png" } },
    { "name": "slimer_left_1",     "slot": 20, "color": 5,  "src": { "file": "../../img/SLIMER2.png" } },
    { "name": "slimer_right_0",    "slot": 21, "color": 5,  "src": { "file": "../../img/SLIMER3.png" } },
    { "name": "slimer_right_1",    "slot": 22, "color": 5,  "src": { "file": "../../img/SLIMER4.png" } },
    { "name": "skelett_0",         "slot": 23, "color": 1,  "src": { "file": "../../img/SKEL-1.png" } },
    { "name": "skelett_1",         "slot": 24, "color": 1,  "src": { "file": "../../img/SKEL-2.png" } },
    { "name": "skelett_2",         "slot": 25, "color": 1,  "src": { "file": "../../img/SKEL-3.png" } },
    { "name": "gecko_left_0",      "slot": 26, "color": 5,  "src": { "sheet": "../../src/main/resources/kuno-sprites.png", "pos": [96, 21] } },
    { "name": "gecko_left_1",      "slot": 27, "color": 5,  "src": { "sheet": "../../src/main/resources/kuno-sprites.png", "pos": [120, 21] } },
    { "name": "gecko_right_0",     "slot": 28, "color": 5,  "src": { "sheet": "../../src/main/resources/kuno-sprites.png", "pos": [144, 21] } },
    { "name": "gecko_right_1",     "slot": 29, "color": 5,  "src": { "sheet": "../../src/main/resources/kuno-sprites.png", "pos": [168, 21] } },
    { "name": "wizrot_0",          "slot": 30, "color": 5,  "src": { "sheet": "../../src/main/resources/kuno-sprites.png", "pos": [192, 21] } },
    { "name": "wizrot_1",          "slot": 31, "color": 5,  "src": { "sheet": "../../src/main/resources/kuno-sprites.png", "pos": [216, 21] } },
    { "name": "wizrot_2",          "slot": 32, "color": 5,  "src": { "sheet": "../../src/main/resources/kuno-sprites.png", "pos": [240, 21] } },
    { "name": "wizrot_3",          "slot": 33, "color": 5,  "src": { "sheet": "../../src/main/resources/kuno-sprites.png", "pos": [264, 21] } }
  ]
}
```

- [ ] **Step 2: Smoke-Test: Config laden**

Run aus Repo-Root: `python -c "from pathlib import Path; import sys; sys.path.insert(0, 'tools/sprites'); from build_c64_sprites import load_config; cfg = load_config(Path('tools/sprites/sprite_phases.json')); print(f'OK -- {len(cfg.phases)} phases')"`
Expected: `OK -- 34 phases` (oder Fehlermeldung mit konkretem Phasen-Namen, falls eine Datei in img/ fehlt — dann den Pfad prüfen).

- [ ] **Step 3: Commit**

```
git add tools/sprites/sprite_phases.json
git commit -m "Switch sprite_phases.json to 34 phases with file-based src for img/ originals"
```

---

## Task 9: Build laufen lassen, Output verifizieren

**Files:**
- Generate (über Build): `src/main/trse/Kuno/sprites/kuno_sprites.bin`, `src/main/trse/Kuno/sprites/kuno_sprites.inc`, `tools/sprites/preview/sprites_built.png`

- [ ] **Step 1: Build ausführen**

Run aus Repo-Root: `python tools/sprites/build_c64_sprites.py`
Expected: drei `Wrote ...`-Zeilen ohne Stack-Trace.

- [ ] **Step 2: Bin-Größe verifizieren**

Run: `python -c "from pathlib import Path; print(Path('src/main/trse/Kuno/sprites/kuno_sprites.bin').stat().st_size)"`
Expected: `2176`

- [ ] **Step 3: Inc-Datei verifizieren**

Inhalt von `src/main/trse/Kuno/sprites/kuno_sprites.inc` prüfen:
- erste @define-Zeile: `@define KUNO_SPAWN_0       200`
- letzte @define-Zeile: `@define WIZROT_3           233`
- 34 `@define`-Zeilen insgesamt
- kein `KUNO_DEAD_*` mehr

Run: `python -c "import re; lines = open('src/main/trse/Kuno/sprites/kuno_sprites.inc').readlines(); defs = [l for l in lines if l.startswith('@define')]; print(f'count={len(defs)}'); print('first:', defs[0].rstrip()); print('last:', defs[-1].rstrip()); assert 'KUNO_DEAD' not in ''.join(defs)"`
Expected: `count=34`, `first: @define KUNO_SPAWN_0       200`, `last: @define WIZROT_3           233`.

- [ ] **Step 4: Preview-PNG visuell prüfen**

`tools/sprites/preview/sprites_built.png` öffnen. Akzeptanzkriterien:
- Die ersten 4 Sprites (Spawn) zeigen erkennbare Kuno-Konturen (kein „Konfetti" mehr) -- vermutlich noch teilweise diffus, da KBEGINN-Phasen Materialisierungs-Effekte sind.
- Walk/Stand/Jump/Idle/Ladder zeigen klare Ritter-Silhouetten **mit Konturen** (Helm, Beine, Arme erkennbar).
- Slimer/Skelett zeigen lesbare Tier-/Skelett-Konturen.
- Gecko/Wizrot bleiben unverändert auf altem Niveau (das ist gewollt -- bleiben sheet-basiert).

Wenn Slimer-LR-Mapping falsch erscheint (z.B. `slimer_right_*` zeigt offensichtlich nach links), in `sprite_phases.json` die Zuordnung von `SLIMER1-4.png` umbiegen und Build wiederholen, bevor commit.

- [ ] **Step 5: Vollen Test-Lauf**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py -v`
Expected: alle PASS.

- [ ] **Step 6: Commit**

```
git add src/main/trse/Kuno/sprites/kuno_sprites.bin src/main/trse/Kuno/sprites/kuno_sprites.inc tools/sprites/preview/sprites_built.png
git commit -m "Build C64 sprites from img/ originals (34 phases, 2176 bytes)"
```

---

## Self-Review Checklist (vor Übergabe)

- Spec-Coverage: 34 Phasen ✓ (Task 8), file/sheet-Schema ✓ (Task 1, 2), XOR-Validation ✓ (Task 3), Dimension-Validation ✓ (Task 4), Sheet-Cache ✓ (Task 5), TGA-Loader ✓ (Task 7), Build-Verifikation ✓ (Task 9).
- Placeholder-Scan: keine TBDs/TODOs in Steps; alle Tests mit konkretem Code; alle Commands mit konkreten Erwartungswerten.
- Type-Konsistenz: `FileSource.path: Path` in Task 1 → in Task 5 weiter genutzt; `SheetSource.pos: tuple[int,int]` durchgehend; `resolve_source(src, sheet_cache)`-Signatur in Task 5 und 6 identisch; `Phase.src` Feldname in Task 2 und 8 identisch.
