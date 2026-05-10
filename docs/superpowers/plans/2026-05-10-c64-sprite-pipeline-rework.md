# C64-Sprite-Pipeline Rework Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Pipeline `tools/sprites/build_c64_sprites.py` lädt pro Phase eine eigene 24×21-Datei (PNG/TGA) aus `img/`. Alle 34 Phasen sind file-basiert (`KLINKS*`, `KRECHTS*`, `KWAIT*`, `KLEITER*`, `KLIST/KREST/KLISPR/KRESPR`, `KBEGINN*.TGA`, `SLIMER1-4.png`, `SKEL-1/2/3.png`, `GECKO1-4.TGA`, `WIZROT1-4.TGA`).

**Architecture:** `Phase` bekommt ein `src: FileSource`-Feld (FileSource ist eine eigene Dataclass mit `path: Path`). `load_config` validiert pro Phase: Datei existiert und ist 24×21. `build_bin` öffnet pro Phase direkt mit Pillow. Kein Sheet-Modus, kein Sheet-Cache.

**Tech Stack:** Python 3, Pillow (PNG+TGA), pytest. Existierende Datei: `tools/sprites/build_c64_sprites.py`. Tests: `tools/sprites/test_build_c64_sprites.py`.

**Spec:** `docs/superpowers/specs/2026-05-10-c64-sprite-pipeline-rework-design.md`

---

## File Structure

| Datei | Aktion |
|------|--------|
| `tools/sprites/build_c64_sprites.py` | modifizieren: `FileSource`-Klasse neu; `Phase.src` ersetzt `Phase.pos`; `Config.source_image` entfernt; `load_config` umgebaut + eager-validates; `build_bin` direkt mit `Image.open`; `PhaseOutOfBoundsError` entfernt; `ConfigError` neu |
| `tools/sprites/test_build_c64_sprites.py` | modifizieren: `_write_config`-Helper auf neues Schema; neue Tests für `FileSource`, `src`-Schema, Existenz-/Dimensions-Validation, TGA; `PhaseOutOfBoundsError`-Test entfernt |
| `tools/sprites/sprite_phases.json` | neu schreiben: 34 Phasen mit `src.file` |
| `src/main/trse/Kuno/sprites/kuno_sprites.bin` | generiert (Build-Output, 2176 Byte) |
| `src/main/trse/Kuno/sprites/kuno_sprites.inc` | generiert (Build-Output, 34 `@define`-Zeilen) |
| `tools/sprites/preview/sprites_built.png` | generiert (Build-Output) |

---

## Task 1: `FileSource`-Dataclass einführen

**Files:**
- Modify: `tools/sprites/build_c64_sprites.py`
- Test: `tools/sprites/test_build_c64_sprites.py`

- [ ] **Step 1: Failing Test schreiben**

Append in `tools/sprites/test_build_c64_sprites.py`:

```python
from build_c64_sprites import FileSource


def test_filesource_is_frozen_dataclass():
    src = FileSource(path=Path("img/KLINKS1.png"))
    assert src.path == Path("img/KLINKS1.png")
    with pytest.raises(Exception):
        src.path = Path("other.png")  # frozen
```

- [ ] **Step 2: Test laufen lassen, Fail bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py::test_filesource_is_frozen_dataclass -v`
Expected: FAIL mit `ImportError: cannot import name 'FileSource'`

- [ ] **Step 3: Implementation**

In `tools/sprites/build_c64_sprites.py`, nach dem bestehenden `from PIL import Image`-Block:

```python
@dataclass(frozen=True)
class FileSource:
    """24x21 image file (PNG or TGA) as sprite source."""
    path: Path
```

- [ ] **Step 4: Test laufen lassen, Pass bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py::test_filesource_is_frozen_dataclass -v`
Expected: PASS

- [ ] **Step 5: Commit**

```
git add tools/sprites/build_c64_sprites.py tools/sprites/test_build_c64_sprites.py
git commit -m "Add FileSource dataclass for sprite phase source"
```

---

## Task 2: `Phase.src` ersetzt `Phase.pos`; `load_config` parst `src.file`-Schema; alle bestehenden Tests migrieren

Dieser Task ist groß, weil sowohl Datentyp als auch alle bestehenden Tests umgestellt werden müssen.

**Files:**
- Modify: `tools/sprites/build_c64_sprites.py` (Phase, Config, load_config, ConfigError, PhaseOutOfBoundsError löschen, build_bin)
- Modify: `tools/sprites/test_build_c64_sprites.py` (alle Tests, die `pos` im JSON oder `Phase.pos` verwenden)

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
    assert cfg.phases[0].name == "p"
    assert cfg.phases[0].slot == 0
    assert cfg.phases[0].color == 14
```

- [ ] **Step 2: Test laufen lassen, Fail bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py::test_load_config_phase_with_file_src -v`
Expected: FAIL — `src`-Feld unbekannt oder Phase-Schema-Mismatch.

- [ ] **Step 3: Datentypen, Exceptions und `load_config` umstellen**

In `tools/sprites/build_c64_sprites.py`:

- `Phase`-Dataclass anpassen:

```python
@dataclass(frozen=True)
class Phase:
    name: str
    slot: int
    color: int
    src: FileSource
```

- `Config`-Dataclass: `source_image`-Feld entfernen:

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

- Alte `PhaseOutOfBoundsError`-Klasse löschen (wird nicht mehr verwendet -- es gibt keine Sheet-Crops mehr).
- Neue Exception:

```python
class ConfigError(ValueError):
    """Phase config has invalid src specification."""
```

- `load_config` umschreiben:

```python
def load_config(config_path: Path) -> Config:
    config_path = Path(config_path)
    with config_path.open("r", encoding="utf-8") as f:
        raw = json.load(f)

    base_dir = config_path.parent

    def _parse_src(name: str, src_raw: dict) -> FileSource:
        if "file" not in src_raw:
            raise ConfigError(f"phase {name!r}: src missing 'file' key")
        path = (base_dir / src_raw["file"]).resolve()
        if not path.exists():
            raise FileNotFoundError(f"phase {name!r}: file not found: {path}")
        with Image.open(path) as img:
            if img.size != (24, 21):
                raise ValueError(
                    f"phase {name!r}: {path} expected 24x21, got {img.size}"
                )
        return FileSource(path=path)

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

- `build_bin` umschreiben (von Sheet-Crop auf direkten File-Open):

```python
def build_bin(cfg: Config) -> bytes:
    """Pack each phase into total_slots * slot_bytes of sprite data."""
    out = bytearray(cfg.total_slots * cfg.slot_bytes)
    for phase in cfg.phases:
        img = Image.open(phase.src.path).convert("RGBA")
        packed = pack_phase(img, cfg.threshold)
        offset = phase.slot * cfg.slot_bytes
        out[offset : offset + cfg.slot_bytes] = packed
    return bytes(out)
```

- [ ] **Step 4: Bestehende Tests in `test_build_c64_sprites.py` migrieren**

Den `_write_config`-Helper umstellen, sodass er pro Phase eine eigene 24×21-PNG anlegt und im JSON `src.file` referenziert:

```python
def _write_config(tmp_path: Path, overrides: dict) -> Path:
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
            {"name": "a", "slot": 0, "color": 14, "src": {"file": "a.png"}},
            {"name": "b", "slot": 1, "color": 5,  "src": {"file": "b.png"}},
        ],
    }
    base.update(overrides)
    # Stelle sicher: für jede Phase im (möglicherweise overridden) base existiert eine 24x21-PNG
    for phase in base["phases"]:
        rel = phase["src"]["file"]
        f = tmp_path / rel
        f.parent.mkdir(parents=True, exist_ok=True)
        if not f.exists():
            Image.new("RGBA", (24, 21), (255, 255, 255, 0)).save(f)
    p = tmp_path / "config.json"
    p.write_text(json.dumps(base))
    return p
```

In allen Tests, die `phases`-Override geben, das alte `"pos": [x, y]` durch `"src": {"file": "<phase_name>.png"}` ersetzen. Konkret betroffen:
- `test_load_config_rejects_duplicate_slot`
- `test_load_config_rejects_slot_out_of_range`
- `test_build_bin_unused_slots_are_zero`
- `test_build_bin_phase_at_correct_slot_offset`
- `test_build_inc_emits_define_per_phase`
- `test_build_inc_skips_unused_slots`

In `test_load_config_happy_path`: `cfg.phases[0].pos` → `cfg.phases[0].src.path`, plus `assert isinstance(cfg.phases[0].src, FileSource)`.

Folgende Tests **löschen** (passen nicht mehr zum Schema):
- `test_load_config_rejects_missing_source_image` (`source_image` existiert nicht mehr)
- `test_build_bin_rejects_phase_out_of_bounds` (`PhaseOutOfBoundsError` ist gelöscht)

Den Test `test_build_bin_phase_at_correct_slot_offset` so umschreiben, dass die für Phase `x` benötigte PNG schwarzes Pixel an (0,0) hat:

```python
def test_build_bin_phase_at_correct_slot_offset(tmp_path):
    """A foreground pixel at (0,0) of phase slot=2 lands at byte 2*64."""
    # Phase-Datei mit einem schwarzen Pixel an (0, 0) anlegen
    phase_img = Image.new("RGBA", (24, 21), (255, 255, 255, 0))
    phase_img.putpixel((0, 0), (0, 0, 0, 255))
    phase_img.save(tmp_path / "x.png")
    cfg_path = _write_config(
        tmp_path,
        {
            "total_slots": 3,
            "phases": [
                {"name": "x", "slot": 2, "color": 14, "src": {"file": "x.png"}},
            ],
        },
    )
    cfg = load_config(cfg_path)
    data = build_bin(cfg)
    assert data[2 * 64] == 0x80
    assert data[2 * 64 + 1] == 0x00
```

(Achtung: Reihenfolge der Helper-Schritte beachten -- erst die Phase-PNG anlegen, dann `_write_config` aufrufen, weil der Helper sonst eine leere Default-PNG anlegt. Der Helper überschreibt nicht, falls `if not f.exists()` matcht.)

- [ ] **Step 5: Alle Tests laufen lassen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py -v`
Expected: alle Tests, die migriert wurden, PASS. Der neue `test_load_config_phase_with_file_src` PASS. Tests `test_load_config_rejects_missing_source_image` und `test_build_bin_rejects_phase_out_of_bounds` sind gelöscht (nicht mehr in der Datei).

- [ ] **Step 6: Commit**

```
git add tools/sprites/build_c64_sprites.py tools/sprites/test_build_c64_sprites.py
git commit -m "Switch Phase schema to file-based src (FileSource)"
```

---

## Task 3: `load_config` lehnt ungültige `src`-Spezifikationen ab

**Files:**
- Modify: `tools/sprites/test_build_c64_sprites.py`

(Code dafür ist in Task 2 schon drin -- jetzt Tests, die das verifizieren.)

- [ ] **Step 1: Failing Tests schreiben**

```python
def test_load_config_rejects_phase_without_file_key(tmp_path):
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

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py::test_load_config_rejects_phase_without_file_key -v`
Expected: PASS (Code ist aus Task 2 schon implementiert).

- [ ] **Step 3: Commit**

```
git add tools/sprites/test_build_c64_sprites.py
git commit -m "Test: load_config rejects src without 'file' key"
```

---

## Task 4: `load_config` validiert FileSource-Existenz und -Dimension eager

**Files:**
- Modify: `tools/sprites/test_build_c64_sprites.py`

- [ ] **Step 1: Failing Tests schreiben**

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
    assert "24, 21" in str(exc.value) or "(24, 21)" in str(exc.value) or "32, 32" in str(exc.value)


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
    assert "does_not_exist.png" in str(exc.value) or "ghost" in str(exc.value)
```

- [ ] **Step 2: Tests laufen lassen, Pass bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py -k "wrong_dimensions or does_not_exist" -v`
Expected: PASS.

- [ ] **Step 3: Commit**

```
git add tools/sprites/test_build_c64_sprites.py
git commit -m "Test: load_config eager-validates FileSource existence and dimension"
```

---

## Task 5: TGA-Loader-Test mit echter `KBEGINN1.TGA`

**Files:**
- Modify: `tools/sprites/test_build_c64_sprites.py`

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
```

- [ ] **Step 2: Test laufen lassen, Pass bestätigen**

Run: `python -m pytest tools/sprites/test_build_c64_sprites.py::test_pack_phase_from_real_kbeginn_tga -v`
Expected: PASS.

- [ ] **Step 3: Commit**

```
git add tools/sprites/test_build_c64_sprites.py
git commit -m "Test: TGA loading via Pillow with real KBEGINN1.TGA"
```

---

## Task 6: `sprite_phases.json` mit 34 Phasen schreiben (alles file-basiert)

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
    { "name": "gecko_left_0",      "slot": 26, "color": 5,  "src": { "file": "../../img/GECKO1.TGA" } },
    { "name": "gecko_left_1",      "slot": 27, "color": 5,  "src": { "file": "../../img/GECKO2.TGA" } },
    { "name": "gecko_right_0",     "slot": 28, "color": 5,  "src": { "file": "../../img/GECKO3.TGA" } },
    { "name": "gecko_right_1",     "slot": 29, "color": 5,  "src": { "file": "../../img/GECKO4.TGA" } },
    { "name": "wizrot_0",          "slot": 30, "color": 5,  "src": { "file": "../../img/WIZROT1.TGA" } },
    { "name": "wizrot_1",          "slot": 31, "color": 5,  "src": { "file": "../../img/WIZROT2.TGA" } },
    { "name": "wizrot_2",          "slot": 32, "color": 5,  "src": { "file": "../../img/WIZROT3.TGA" } },
    { "name": "wizrot_3",          "slot": 33, "color": 5,  "src": { "file": "../../img/WIZROT4.TGA" } }
  ]
}
```

- [ ] **Step 2: Smoke-Test: Config laden**

Run aus Repo-Root: `python -c "from pathlib import Path; import sys; sys.path.insert(0, 'tools/sprites'); from build_c64_sprites import load_config; cfg = load_config(Path('tools/sprites/sprite_phases.json')); print(f'OK -- {len(cfg.phases)} phases')"`
Expected: `OK -- 34 phases` (oder Fehlermeldung mit konkretem Phasen-Namen, falls eine Datei in img/ fehlt -- dann den Pfad prüfen).

- [ ] **Step 3: Commit**

```
git add tools/sprites/sprite_phases.json
git commit -m "Switch sprite_phases.json to 34 file-based phases from img/ originals"
```

---

## Task 7: Build laufen lassen, Output verifizieren

**Files:**
- Generate: `src/main/trse/Kuno/sprites/kuno_sprites.bin`, `src/main/trse/Kuno/sprites/kuno_sprites.inc`, `tools/sprites/preview/sprites_built.png`

- [ ] **Step 1: Build ausführen**

Run aus Repo-Root: `python tools/sprites/build_c64_sprites.py`
Expected: drei `Wrote ...`-Zeilen ohne Stack-Trace.

- [ ] **Step 2: Bin-Größe verifizieren**

Run: `python -c "from pathlib import Path; print(Path('src/main/trse/Kuno/sprites/kuno_sprites.bin').stat().st_size)"`
Expected: `2176`

- [ ] **Step 3: Inc-Datei verifizieren**

Run: `python -c "lines = open('src/main/trse/Kuno/sprites/kuno_sprites.inc').readlines(); defs = [l for l in lines if l.startswith('@define')]; print(f'count={len(defs)}'); print('first:', defs[0].rstrip()); print('last:', defs[-1].rstrip()); assert 'KUNO_DEAD' not in ''.join(defs), 'KUNO_DEAD should be gone'"`
Expected: `count=34`, `first: @define KUNO_SPAWN_0       200`, `last: @define WIZROT_3           233`.

- [ ] **Step 4: Preview-PNG visuell prüfen**

`tools/sprites/preview/sprites_built.png` öffnen. Akzeptanzkriterien:
- Die ersten 4 Sprites (Spawn) zeigen erkennbare Kuno-Konturen (kein „Konfetti" mehr) -- vermutlich noch teilweise diffus, da KBEGINN-Phasen Materialisierungs-Effekte sind.
- Walk/Stand/Jump/Idle/Ladder zeigen klare Ritter-Silhouetten **mit Konturen** (Helm, Beine, Arme erkennbar).
- Slimer/Skelett zeigen lesbare Tier-/Skelett-Konturen.
- Gecko/Wizrot zeigen lesbare Gegner-Konturen (statt der bisherigen Pflanzen-/Klumpen-Reste).

Wenn Slimer/Gecko-LR-Mapping falsch erscheint (z.B. `slimer_right_*` zeigt offensichtlich nach links), in `sprite_phases.json` die Zuordnung umbiegen und Build wiederholen, bevor commit.

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

- Spec-Coverage: 34 Phasen ✓ (Task 6), file-Schema ✓ (Task 1, 2), src-Validation ✓ (Task 3), Existenz/Dimension-Validation ✓ (Task 4), TGA-Loader ✓ (Task 5), Build-Verifikation ✓ (Task 7).
- Placeholder-Scan: keine TBDs/TODOs in Steps; alle Tests mit konkretem Code; alle Commands mit konkreten Erwartungswerten.
- Type-Konsistenz: `FileSource.path: Path` in Task 1 → in Task 2 als `Phase.src.path` weiter genutzt; `Phase.src` Feldname in Task 2 und 6 identisch; `ConfigError` in Task 2 erzeugt, in Task 3 als Erwartung verwendet.
- Spec-Drift: Keine `SheetSource`/`Source`-Union/`resolve_source`/`PhaseOutOfBoundsError` mehr im Plan -- konsistent mit der aktualisierten Spec ohne Sheet-Modus.
