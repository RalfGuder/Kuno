# C64-Sprite-Pipeline Rework: Original-Pixel-Art als Quelle

**Datum:** 2026-05-10
**Branch:** `dev/c64`
**Vorgänger-Spec:** keine (erste Iteration der Pipeline existiert seit 2026-05-09 in `tools/sprites/build_c64_sprites.py`)

## Zusammenfassung

Die C64-Sprite-Pipeline (`tools/sprites/build_c64_sprites.py`) wird umgebaut, sodass sie pro Phase **eine eigene 24×21-Quelldatei** aus `img/` liest (PNG oder TGA). Alle 34 Phasen verwenden die originalen 1996er-Kuno-Sprites; die antialiased Sammel-PNG `src/main/resources/kuno-sprites.png` wird als Quelle obsolet.

## Problem

Der aktuelle Hires-Output (`tools/sprites/preview/sprites_built.png`) verliert beim Threshold-Pass dramatisch viel Detail: der Ritter wird zur blauen Silhouette ohne erkennbare Konturen, Slimer/Gecko zu grünen Klumpen, Wizrot-Phasen zu Pflanzen-Mustern, die Spawn-Phasen 0-2 zu „Sternenkonfetti". Ursache ist nicht der Konverter, sondern die **Quelle**: `src/main/resources/kuno-sprites.png` ist ein fremdes, antialiased Sprite-Sheet, das beim 1-Bit-Threshold zwangsläufig zur Silhouette zerfällt.

Im Repo liegen unter `img/` **die Original-Kuno-Sprites von 1996** als pixel-genau gemalte 24×21-PNG/TGA-Dateien -- inklusive aller benötigten Phasen für Kuno (`KLINKS*`, `KRECHTS*`, `KWAIT*`, `KLEITER*`, `KLIST`, `KREST`, `KLISPR`, `KRESPR`, `KBEGINN*.TGA`), Slimer (`SLIMER1-4`), Skelett (`SKEL-1/2/3`), Gecko (`GECKO1-4.TGA`) und Wizrot (`WIZROT1-4.TGA`). Diese sind hires-tauglich, weil sie nie antialiased waren -- der Threshold wird auf ihnen deterministisch.

## Ziel

- Hires-Output zeigt klar lesbare Konturen (Helm, Beine, Arme, Augen), kein Konfetti mehr.
- Die im Spiel sichtbaren Sprites sind **Ralfs eigene Originale**, nicht ein fremdes Sheet.
- Pipeline kann TGA und PNG lesen.
- Phasen-Liste ist konsistent (`skelett` statt `kuno_dead`, neue `idle`/`ladder`-Phasen).

## Nicht-Ziele

- **Kein Multicolor-Modus.** Hires bleibt; Mode-Switch ist explizit ausgeschlossen, wäre eigene spätere Iteration.
- **Kein Edge-Detection-Algorithmus.** Threshold bleibt simpel (Alpha + Helligkeit), die Quelle löst das Konturen-Problem.
- **Keine Rückwärtskompatibilität zur alten 27-Phasen-`sprite_phases.json`.** Die JSON wird in einem Schritt ersetzt.
- **Kein Sheet-Source-Mode.** Da alle 34 Phasen file-basiert sind, entfällt die Sheet-Variante komplett. `kuno-sprites.png` als Quelle wird nicht mehr referenziert (Datei darf im Repo bleiben, hat aber keinen Konsumenten mehr).
- **Kein Encoding-Repair an `cpp/*.CPP`.** Die heute uncommitted CP437→UTF-8-Reparatur ist ein separates Thema; aus dem Scope dieser Spec.

## Phasen-Inventar (34 Phasen, Slots 0..33, Hardware-IDs 200..233)

Alle Phasen file-basiert. Pfade sind relativ zur `sprite_phases.json` (in `tools/sprites/`).

| Slot | Phase-Name | Quelldatei | Anmerkung |
|------|------------|------------|-----------|
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
| 26..27 | `gecko_left_0..1` | `img/GECKO1.TGA`, `GECKO2.TGA` | Annahme: 1/2 = links |
| 28..29 | `gecko_right_0..1` | `img/GECKO3.TGA`, `GECKO4.TGA` | Annahme: 3/4 = rechts |
| 30..33 | `wizrot_0..3` | `img/WIZROT1-4.TGA` | 4-Phasen-Animation, eine Richtung |

**Bin-Größe:** 34 × 64 = **2176 Byte**.

## Architektur

```
sprite_phases.json
        |
        v
load_config -- validiert Schema, Slot-Range, Duplikate, Datei existiert + 24×21
        |
        v
build_bin
   |
   +-- für jede Phase:
   |     img = Image.open(phase.src.path).convert("RGBA")  # PNG+TGA via Pillow
   |     pack_phase(img, threshold) -> 64 Bytes Hires
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
    """24x21 image file (PNG or TGA) as sprite source."""
    path: Path

@dataclass(frozen=True)
class Phase:
    name: str
    slot: int
    color: int
    src: FileSource     # ersetzt das aktuelle pos-Feld
```

`Config.source_image` entfällt als Pflichtfeld. Quellpfade werden pro Phase im `src.file`-Block geführt; relative JSON-Pfade werden weiterhin gegen das Verzeichnis der `sprite_phases.json` aufgelöst.

`SheetSource` wird **nicht** eingeführt. Die Source-Klasse als eigene Dataclass bleibt erhalten, weil sie der Validation einen klaren Anker gibt (`load_config` legt eine `FileSource`-Instanz nach Existenz- und Dimensions-Check an).

### `load_config`

- Akzeptiert pro Phase `src: { file: "..." }`. Fehlt `file` (oder andere Keys statt `file`) → `ConfigError`.
- File-Variante: prüft Existenz **und** Dimension (24×21) -- so früh wie möglich, bevor Build läuft.

### `pack_phase`

Bleibt bytewise unverändert. Die Funktion erhält weiterhin ein 24×21-RGBA-Image und einen Threshold.

### `build_bin`

```python
def build_bin(cfg: Config) -> bytes:
    out = bytearray(cfg.total_slots * cfg.slot_bytes)
    for phase in cfg.phases:
        img = Image.open(phase.src.path).convert("RGBA")
        packed = pack_phase(img, cfg.threshold)
        offset = phase.slot * cfg.slot_bytes
        out[offset : offset + cfg.slot_bytes] = packed
    return bytes(out)
```

`PhaseOutOfBoundsError` (für Sheet-Out-of-Bounds) wird ersatzlos entfernt -- es gibt keine Sheet-Pfade mehr. Falls die Klasse noch von Tests importiert wird, fällt das beim Test-Migration auf.

### `build_inc`

Unverändert. Die ausgegebenen `@define`-Namen erben sich aus den (neu benannten) Phase-Namen automatisch:

```
@define KUNO_SPAWN_0       200
@define KUNO_SPAWN_1       201
...
@define KUNO_IDLE_0        212
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

Unverändert. Layout passt sich automatisch an die Phasen-Anzahl an (`cols = 7`, `rows` ableitet).

## Datenfluss

1. CLI: `python tools/sprites/build_c64_sprites.py`
2. `main()` lädt `sprite_phases.json` → `Config`
3. `build_bin(cfg)` öffnet pro Phase die Datei (Pillow erkennt `.tga` und `.png` ootb), packt 64 Byte
4. `kuno_sprites.bin` (2176 Byte) wird geschrieben
5. `build_inc(cfg)` schreibt `kuno_sprites.inc`
6. `render_preview(cfg, bin_data)` schreibt `sprites_built.png` (Verifikations-Round-Trip)

## Fehlerbehandlung

| Fehler | Klasse | Zeitpunkt |
|--------|--------|-----------|
| JSON-Phase ohne `src` oder ohne `src.file` | `ConfigError` | `load_config` |
| File-Quelle existiert nicht | `FileNotFoundError` | `load_config` (eager) |
| File-Quelle ist nicht 24×21 | `ValueError` | `load_config` (eager) |
| Doppelter Slot | `DuplicateSlotError` | `load_config` |
| Slot ≥ `total_slots` | `SlotOutOfRangeError` | `load_config` |

Eager-Validation in `load_config` heißt: bevor irgendein Byte gepackt wird, ist klar, dass alle 34 Quellen existieren und passen.

## Tests in `tools/sprites/test_build_c64_sprites.py`

| Test | Was wird verifiziert |
|------|----------------------|
| `test_filesource_is_frozen_dataclass` | `FileSource(path)` konstruiert und frozen |
| `test_load_config_phase_with_file_src` | Phase mit `src.file` validiert; `Phase.src` ist `FileSource` |
| `test_load_config_rejects_phase_without_file_key` | `src: {}` → `ConfigError` |
| `test_load_config_rejects_file_with_wrong_dimensions` | 32×32-PNG → `ValueError` |
| `test_load_config_rejects_file_that_does_not_exist` | `FileNotFoundError` |
| `test_pack_phase_from_real_kbeginn_tga` | TGA aus `img/KBEGINN1.TGA` korrekt geladen |
| Bestehende `test_pack_phase_*` | Bytewise-Verhalten unverändert (8 Tests) |
| Bestehende `test_build_inc_*` | `@define`-Erzeugung weiter korrekt (mit migriertem `_write_config`-Helper) |

Bestehende Tests werden migriert: alle, die `Phase(name, slot, pos, color)` bzw. `pos`-Feld im JSON nutzen, müssen auf neues `src.file`-Schema umgestellt werden. Tests, die `PhaseOutOfBoundsError` testen, werden gelöscht (Klasse entfällt) -- aber wir müssen sie ersetzen mit Tests für die neuen Validation-Regeln.

## Migration in einem Schwung

1. **`sprite_phases.json` neu schreiben** -- 34 Phasen mit `src: { file: "..." }`, alle file-basiert.
2. **Code-Änderungen in `build_c64_sprites.py`**:
   - `FileSource`-Dataclass einführen
   - `Phase.pos` → `Phase.src: FileSource`
   - `Config.source_image` entfernen
   - `load_config` umbauen + eager-validate
   - `build_bin` direkt mit `Image.open(phase.src.path)`
   - `PhaseOutOfBoundsError` löschen
   - `ConfigError` einführen
3. **Tests umstellen** (`_write_config`-Helper auf neues Schema; `PhaseOutOfBoundsError`-Tests durch File-Validation-Tests ersetzen).
4. **Build laufen lassen** -- `kuno_sprites.bin` (2176 B), `kuno_sprites.inc` (34 Einträge), `sprites_built.png` zur visuellen Verifikation.

## Risiken und offene Punkte

- **Slimer-/Gecko-/Wizrot-Mapping-Annahmen**: Dass `SLIMER1/2 = links, 3/4 = rechts` und analog `GECKO1/2 = links, 3/4 = rechts` stimmen, ist eine Vermutung aus den Dateinamen. `WIZROT1-4` werden als 4-Phasen-Animation einer Richtung interpretiert. Wenn die Originale anders nummeriert sind, muss das Mapping nach dem ersten `sprites_built.png`-Vergleich nachgezogen werden.
- **`kuno_dead` → `skelett` Bezeichner-Bruch**: `.ras`-Dateien referenzieren aktuell **keine** `@define`-Bezeichner aus der `.inc` (per `grep` verifiziert), also folgenlos.
- **`img/` ist gitignored**: Bereits getrackte Files unter `img/` werden weiter aktualisiert, neue ignoriert. Build benötigt `img/`, das ist bei Ralf vorhanden. Bei einem Re-Clone müsste `img/` separat wiederhergestellt werden -- separate Aufgabe (`.gitignore`-Repair später).
- **Out-of-Repo-Sprite-Editing**: Die Original-PNGs werden ab jetzt produktiv -- bei Änderungen an `KLINKS1.png` etc. gilt der Build neu. Risiko: versehentliche Edits in `img/` brechen den C64-Build.
- **TGA-Konvertierung durch Pillow**: getestet wird zwar, aber nicht alle TGA-Subformate (RLE, paletted, 32-bit) sind explizit verifiziert. Die in `img/` vorhandenen TGAs werden im ersten Build empirisch geprüft.

## Nächste Schritte nach Approval

1. Implementierungsplan ausführen (siehe `docs/superpowers/plans/2026-05-10-c64-sprite-pipeline-rework.md`).
2. Visueller Check: `sprites_built.png` zeigt klar lesbare Kuno-Konturen.
