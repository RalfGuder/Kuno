# C64-Sprite-Pipeline Rework: Original-Pixel-Art als Quelle

**Datum:** 2026-05-10
**Branch:** `dev/c64`
**Vorgänger-Spec:** keine (erste Iteration der Pipeline existiert seit 2026-05-09 in `tools/sprites/build_c64_sprites.py`)

## Zusammenfassung

Die C64-Sprite-Pipeline (`tools/sprites/build_c64_sprites.py`) wird umgebaut, sodass sie pro Phase entweder eine **eigene 24×21-Quelldatei** aus `img/` liest (PNG oder TGA) oder weiterhin auf eine **Sheet-Region** zugreift. Die 26 Kuno-/Slimer-/Skelett-Phasen wechseln auf die originalen 1996er-Pixel-Art-Dateien aus `img/`, die 8 Gecko-/Wizrot-Phasen bleiben übergangsweise auf der bisherigen `src/main/resources/kuno-sprites.png`. Insgesamt **34 Phasen**.

## Problem

Der aktuelle Hires-Output (`tools/sprites/preview/sprites_built.png`) verliert beim Threshold-Pass dramatisch viel Detail: der Ritter wird zur blauen Silhouette ohne erkennbare Konturen, Slimer/Gecko zu grünen Klumpen, Wizrot-Phasen zu Pflanzen-Mustern, die Spawn-Phasen 0-2 zu „Sternenkonfetti". Ursache ist nicht der Konverter, sondern die **Quelle**: `src/main/resources/kuno-sprites.png` ist ein fremdes, antialiased Sprite-Sheet, das beim 1-Bit-Threshold zwangsläufig zur Silhouette zerfällt.

Im Repo liegen **die Original-Kuno-Sprites von 1996** als pixel-genau gemalte 24×21-PNG/TGA-Dateien (`KLINKS1.png`, `KRECHTS1.png`, `KWAIT1-5.png`, `KBEGINN1-4.TGA`, `SLIMER1-4.png`, `SKEL-1/2/3.png`, `KLEITER1/2.png`, …). Diese sind hires-tauglich, weil sie nie antialiased waren -- der Threshold wird auf ihnen deterministisch.

## Ziel

- Hires-Output zeigt klar lesbare Konturen (Helm, Beine, Arme), kein Konfetti mehr.
- Die im Spiel sichtbaren Sprites sind **Ralfs eigene Originale**, nicht ein fremdes Sheet.
- Pipeline kann TGA und PNG lesen.
- Phasen-Liste ist konsistent (`skelett` statt `kuno_dead`, neue `idle`/`ladder`-Phasen, gestrichene Inkonsistenzen).

## Nicht-Ziele

- **Kein Multicolor-Modus.** Hires bleibt; Mode-Switch ist explizit ausgeschlossen, wäre eigene spätere Iteration.
- **Kein neues Pixel-Art für Gecko/Wizrot.** Diese 8 Phasen bleiben auf alter Sheet-Quelle, bis sie separat neu gepixelt werden.
- **Kein Edge-Detection-Algorithmus.** Threshold bleibt simpel (Alpha + Helligkeit), die Quelle löst das Konturen-Problem.
- **Keine Rückwärtskompatibilität zur alten 27-Phasen-`sprite_phases.json`.** Die JSON wird in einem Schritt ersetzt.
- **Kein Encoding-Repair an `cpp/*.CPP`.** Die heute uncommitted CP437→UTF-8-Reparatur ist ein separates Thema; aus dem Scope dieser Spec.

## Phasen-Inventar (34 Phasen, Slots 0..33, Hardware-IDs 200..233)

| Slot | Phase-Name | Quelle | Anmerkung |
|------|------------|--------|-----------|
| 0..3 | `kuno_spawn_0..3` | `img/KBEGINN1-4.TGA` | TGA via Pillow |
| 4..5 | `kuno_walk_left_0..1` | `img/KLINKS1.png`, `KLINKS2.png` | |
| 6..7 | `kuno_walk_right_0..1` | `img/KRECHTS1.png`, `KRECHTS2.png` | |
| 8 | `kuno_stand_left` | `img/KLIST.png` | |
| 9 | `kuno_stand_right` | `img/KREST.png` | |
| 10 | `kuno_jump_left` | `img/KLISPR.png` | |
| 11 | `kuno_jump_right` | `img/KRESPR.png` | |
| 12..16 | `kuno_idle_0..4` | `img/KWAIT1-5.png` | **neu** -- 5 Phasen Idle/Wait |
| 17..18 | `kuno_ladder_0..1` | `img/KLEITER1-2.png` | **neu** -- Leiter |
| 19..20 | `slimer_left_0..1` | `img/SLIMER1.png`, `SLIMER2.png` | Annahme: 1/2 = links |
| 21..22 | `slimer_right_0..1` | `img/SLIMER3.png`, `SLIMER4.png` | Annahme: 3/4 = rechts |
| 23..25 | `skelett_0..2` | `img/SKEL-1/2/3.png` | **umbenannt** (war `kuno_dead`) |
| 26..27 | `gecko_left_0..1` | `kuno-sprites.png` + Pos | **bleibt sheet-basiert** |
| 28..29 | `gecko_right_0..1` | `kuno-sprites.png` + Pos | **bleibt sheet-basiert** |
| 30..33 | `wizrot_0..3` | `kuno-sprites.png` + Pos | **bleibt sheet-basiert** |

**Gesamt:** 26 file-basierte + 8 sheet-basierte = 34 Phasen in Slots 0..33 (Hardware-IDs 200..233). Bin-Größe: 34 × 64 = **2176 Byte**.

## Architektur

```
sprite_phases.json
        |
        v
load_config -- validiert Schema, Slot-Range, Duplikate, exakt-eines-von-{file,sheet}
        |
        v
build_bin
   |
   +-- für jede Phase:
   |     resolve_source(phase) -> 24x21 RGBA Image
   |       |
   |       +-- file-Variante:    open_image(path)   # Pillow, PNG+TGA
   |       +-- sheet-Variante:   sheet_cache[path].crop(pos, 24, 21)
   |
   |     pack_phase(image, threshold) -> 64 Bytes Hires
   |
   v
kuno_sprites.bin (2176 Byte: 34 * 64)
        +
build_inc -> kuno_sprites.inc (@define KUNO_SPAWN_0 200 ... etc.)
        +
render_preview -> sprites_built.png
```

## Komponenten-Änderungen in `tools/sprites/build_c64_sprites.py`

### Datentypen

```python
@dataclass(frozen=True)
class FileSource:
    path: Path        # absolut, basisrelativ aufgelöst aus JSON

@dataclass(frozen=True)
class SheetSource:
    path: Path
    pos: tuple[int, int]

Source = Union[FileSource, SheetSource]

@dataclass(frozen=True)
class Phase:
    name: str
    slot: int
    color: int
    src: Source       # ersetzt das aktuelle pos-Feld
```

`Config.source_image` entfällt als Pflichtfeld. Quellpfade werden pro Phase im `src`-Block geführt; relative JSON-Pfade werden weiterhin gegen das Verzeichnis der `sprite_phases.json` aufgelöst.

### `load_config`

- Akzeptiert pro Phase `src: { file: "..." }` ODER `src: { sheet: "...", pos: [x, y] }`.
- Wirft `ConfigError` bei: beides angegeben, keines angegeben, unbekannter `src`-Key.
- File-Variante: prüft Existenz **und** Dimension (24×21) -- so früh wie möglich, bevor Build läuft.
- Sheet-Variante: prüft Existenz, Dimensionen werden in `build_bin` geprüft (analog heute).

### `pack_phase`

Bleibt bytewise unverändert. Die Funktion erhält weiterhin ein 24×21-RGBA-Image und einen Threshold; **woher** das Image kommt, ist Sache des Aufrufers.

### `resolve_source` (neu)

```python
def resolve_source(src: Source, sheet_cache: dict[Path, Image.Image]) -> Image.Image:
    """24x21 RGBA Image aus Datei oder gecachtem Sheet."""
    if isinstance(src, FileSource):
        img = Image.open(src.path).convert("RGBA")
        if img.size != (24, 21):
            raise ValueError(f"{src.path}: expected 24x21, got {img.size}")
        return img
    # SheetSource
    sheet = sheet_cache.get(src.path)
    if sheet is None:
        sheet = Image.open(src.path).convert("RGBA")
        sheet_cache[src.path] = sheet
    x, y = src.pos
    return sheet.crop((x, y, x + 24, y + 21))
```

Sheet-Cache wird in `build_bin` als lokales Dict aufgebaut, sodass `kuno-sprites.png` für die 8 Gecko/Wizrot-Phasen nur **einmal** decodiert wird.

### `build_bin`

```python
def build_bin(cfg: Config) -> bytes:
    out = bytearray(cfg.total_slots * cfg.slot_bytes)
    sheet_cache: dict[Path, Image.Image] = {}
    for phase in cfg.phases:
        img = resolve_source(phase.src, sheet_cache)
        packed = pack_phase(img, cfg.threshold)
        offset = phase.slot * cfg.slot_bytes
        out[offset : offset + cfg.slot_bytes] = packed
    return bytes(out)
```

`PhaseOutOfBoundsError` wandert in `resolve_source` (für Sheet-Variante mit Out-of-Bounds-Pos).

### `build_inc`

Unverändert in der Logik. Die ausgegebenen `@define`-Namen erben sich aus den (neu benannten) Phase-Namen automatisch. Resultierender Inhalt:

```
@define KUNO_SPAWN_0       200
@define KUNO_SPAWN_1       201
...
@define KUNO_IDLE_0        212
@define KUNO_IDLE_1        213
...
@define KUNO_LADDER_0      217
@define KUNO_LADDER_1      218
@define SLIMER_LEFT_0      219
@define SLIMER_LEFT_1      220
@define SLIMER_RIGHT_0     221
@define SLIMER_RIGHT_1     222
@define SKELETT_0          223
@define SKELETT_1          224
@define SKELETT_2          225
@define GECKO_LEFT_0       226
@define GECKO_LEFT_1       227
@define GECKO_RIGHT_0      228
@define GECKO_RIGHT_1      229
@define WIZROT_0           230
@define WIZROT_1           231
@define WIZROT_2           232
@define WIZROT_3           233
```

### `render_preview`

Unverändert in der Logik. Layout passt sich automatisch an die Phasen-Anzahl an (siehe `cols = 7`, `rows` ableitet).

## Datenfluss

1. CLI: `python tools/sprites/build_c64_sprites.py`
2. `main()` lädt `sprite_phases.json` → `Config`
3. `build_bin(cfg)` öffnet pro file-Phase die Datei (Pillow erkennt `.tga` und `.png` ootb), pro sheet-Phase einmalig das Sheet, cropt, packt
4. `kuno_sprites.bin` (2176 Byte) wird geschrieben
5. `build_inc(cfg)` schreibt `kuno_sprites.inc`
6. `render_preview(cfg, bin_data)` schreibt `sprites_built.png` (Verifikations-Round-Trip)

## Fehlerbehandlung

| Fehler | Klasse | Zeitpunkt |
|--------|--------|-----------|
| JSON-Phase ohne `src` | `ConfigError` | `load_config` |
| JSON-Phase mit beiden `file` und `sheet` | `ConfigError` | `load_config` |
| File-Quelle existiert nicht | `FileNotFoundError` | `load_config` (eager) |
| File-Quelle ist nicht 24×21 | `ValueError` | `load_config` (eager) |
| Sheet-Quelle existiert nicht | `FileNotFoundError` | `load_config` (eager) |
| Sheet-Position außerhalb des Sheets | `PhaseOutOfBoundsError` | `resolve_source` (lazy) |
| Doppelter Slot | `DuplicateSlotError` | `load_config` |
| Slot ≥ `total_slots` | `SlotOutOfRangeError` | `load_config` |

Eager-Validation in `load_config` heißt: bevor irgendein Byte gepackt wird, ist klar, dass alle 34 Quellen existieren und passen.

## Tests in `tools/sprites/test_build_c64_sprites.py`

| Test | Was wird verifiziert |
|------|----------------------|
| `test_load_config_file_phase_ok` | Phase mit `file` validiert, Dimension geprüft |
| `test_load_config_sheet_phase_ok` | Phase mit `sheet`+`pos` validiert |
| `test_load_config_rejects_both_file_and_sheet` | `ConfigError` |
| `test_load_config_rejects_neither` | `ConfigError` |
| `test_load_config_rejects_wrong_dimension_file` | 32×32-PNG → `ValueError` |
| `test_load_config_rejects_missing_file` | `FileNotFoundError` |
| `test_pack_phase_from_file_png` | `KLINKS1.png` → bekannte 64-Byte-Output |
| `test_pack_phase_from_file_tga` | TGA-Roundtrip korrekt |
| `test_pack_phase_from_sheet_region` | Bestehender Sheet-Pfad bleibt (Gecko-Phase als Beispiel) |
| `test_build_bin_caches_sheet_once` | Sheet wird nur einmal `Image.open`'d (Mock-Counter) |
| `test_build_inc_naming` | `@define KUNO_IDLE_0`, `@define SKELETT_0` -- aufgeräumte Bezeichner |

Bestehende Tests werden migriert: alle, die direkt `Phase(name, slot, pos, color)` konstruieren, müssen auf neues Schema umgestellt werden.

## Migration in einem Schwung

1. **`sprite_phases.json` neu schreiben** -- 34 Phasen mit neuem `src`-Schema, 26 file-basiert, 8 sheet-basiert.
2. **Code-Änderungen in `build_c64_sprites.py`** (siehe Komponenten-Sektion).
3. **Tests umstellen** (siehe Test-Sektion).
4. **Build laufen lassen** -- `kuno_sprites.bin` (2176 B), `kuno_sprites.inc` (34 Einträge), `sprites_built.png` zur visuellen Verifikation.
5. **`main.ras` prüfen** -- falls `KUNO_DEAD_*` schon irgendwo referenziert ist, auf `SKELETT_*` umbiegen. Falls neue Phasen (`KUNO_IDLE_*`, `KUNO_LADDER_*`) noch nicht angesprochen werden, kein Eingriff nötig.

## Risiken und offene Punkte

- **Slimer-LR-Annahme**: dass `SLIMER1/2 = links` und `SLIMER3/4 = rechts` ist eine Vermutung aus den Dateinamen. Wenn die Originale anders nummeriert sind (z.B. alle 4 = ein Tier, oder 1/2 = Variante A, 3/4 = Variante B), muss das Mapping nach erstem `sprites_built.png`-Vergleich nachgezogen werden.
- **`kuno_dead` → `skelett` Bezeichner-Bruch**: falls `main.ras` aktuell `KUNO_DEAD_*` referenziert, fällt das beim TRSE-Build auf -- dann dort umbenennen. Falls nicht, kein Effekt.
- **Out-of-Repo-Sprite-Editing**: Die Original-PNGs werden ab jetzt produktiv -- bei Änderungen an `KLINKS1.png` etc. gilt der Build neu. Risiko: versehentliche Edits in `img/` brechen den C64-Build.
- **TGA-Konvertierung durch Pillow**: getestet wird zwar, aber nicht alle TGA-Subformate (RLE, paletted, 32-bit) sind explizit verifiziert. `KBEGINN1-4.TGA` werden im ersten Build empirisch geprüft.

## Nächste Schritte nach Approval

1. Implementierungsplan via `superpowers:writing-plans` aus dieser Spec erzeugen.
2. Plan ausführen (atomare Commits, Linie der bestehenden Spec-/Plan-/Skeleton-/Funktion-Reihenfolge).
3. Visueller Check: `sprites_built.png` zeigt klar lesbare Kuno-Konturen.
