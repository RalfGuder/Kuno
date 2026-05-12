# C64-Sprite-Overlay-Mode: Outline + Fill je Phase

**Datum:** 2026-05-12
**Branch:** `dev/c64`
**Vorgänger-Spec:** [`2026-05-10-c64-sprite-pipeline-rework-design.md`](./2026-05-10-c64-sprite-pipeline-rework-design.md) (34 Phasen file-based, hires single-sprite -- jetzt produktiv ab Commit `2547c433`)

## Zusammenfassung

Die C64-Sprite-Pipeline wird um einen **Overlay-Mode** erweitert: pro Phase werden **zwei** Hires-Sprites erzeugt -- ein `_outline`- und ein `_fill`-Sprite -- die zur Laufzeit auf der gleichen Bildschirmposition gestackt werden. Damit lassen sich Konturen (Helm, Augen, Arm-/Bein-Trennung) gegen die Fläche (Körperfarbe) absetzen, ohne den Hires-Mode zu verlassen.

Die Aufteilung passiert **implizit per Helligkeit** im Konverter -- die `img/`-Originale bleiben unverändert.

## Problem

Nach dem file-based Rework (Commit `2547c433`) lädt die Pipeline alle 34 Phasen aus den 1996er-Original-PNGs/TGAs, aber das visuelle Ergebnis (`tools/sprites/preview/sprites_built.png`) zeigt weiterhin:

- **Kuno-Phasen** (Walk/Stand/Jump/Idle/Ladder/Spawn): solide blaue Silhouetten ohne Augen, ohne Arm-/Bein-Trennung.
- **Skelett**: leere Bilder (Knochen liegen über Helligkeits-Threshold).
- **KBEGINN1.TGA**: ganzes Bild blau, weil TGA-Hintergrund `(255,100,100)` unter Threshold rutscht.

Ursache ist **strukturell**: 1-Bit-Hires hat genau zwei Zustände (on/off, eine Vordergrundfarbe). Die Originale wurden 1996 für 256-Farb-VGA gemalt und enthalten Hut, Gesicht, Augen, Körper, Beine in unterschiedlichen Farben. Eine einzelne Helligkeits-Schwelle kann diese fünf Farben nicht differenzieren -- alles unter Threshold kollabiert zur Silhouette.

## Ziel

- Lesbare Kuno-Phasen: Helm/Hut sichtbar, Gesicht abgesetzt, Augen erkennbar, Arme/Beine voneinander getrennt.
- Skelett-Phasen sichtbar (Knochen erscheinen).
- Originale in `img/` bleiben unverändert; keine zusätzliche Bildbearbeitung nötig.
- Hardware-Sprite-Mode bleibt **Hires** (kein Multicolor, kein anderer Engine-Switch).
- TRSE-Code soll die zwei Sprite-IDs pro Animation Frame strukturell trivial finden (klar getrennte Slot-Bänke).

## Nicht-Ziele

- **Kein Multicolor-Modus.** Hires bleibt -- Spec 2026-05-10 hatte das bewusst ausgeschlossen, diese Spec hält daran fest.
- **Keine manuelle Outline-/Fill-Bild-Pflege.** Keine zusätzlichen Dateien in `img/`. Die zwei Sprite-Slices entstehen rein im Konverter aus der Original-PNG/TGA.
- **Keine TRSE-`.ras`-Änderungen in dieser Spec.** Die Spec endet bei Bin/Inc-Generierung. Die Anpassung der Animationsroutine in `main.ras` (`Sprite_BeideSetzen(x, y, frame_outline_id, frame_fill_id)`) ist Folge-Aufgabe.
- **Keine Per-Phase-Threshold-Override.** Zwei globale Thresholds (`dark_threshold`, `bright_threshold`) reichen für die jetzige Sprite-Auswahl. Per-Phase-Overrides nachziehen, falls notwendig.
- **Kein Color-Quantisierungs-Algorithmus** (k-means etc.). Reines RGB-Helligkeitsband.
- **Kein neuer Sprite-Mode neben Overlay.** Single-Sprite-Output wird ersatzlos gestrichen -- die Pipeline produziert ab dieser Iteration immer das Paar.

## Mode der Aufteilung: implizit per Helligkeit

Pro Pixel des 24×21-RGBA-Originals:

```
alpha = pixel.a
brightness = (r + g + b) // 3        # nur wenn alpha > 0

Outline-Bit (geht in outline-Sprite):
  alpha > 0  AND brightness < dark_threshold

Fill-Bit (geht in fill-Sprite):
  alpha > 0  AND dark_threshold <= brightness < bright_threshold

Above bright_threshold (z. B. weisses Highlight) oder alpha == 0:
  beide Bits aus
```

**Default-Schwellen:** `dark_threshold = 80`, `bright_threshold = 240`.

Konsequenz für die im aktuellen Build geprüften Sprites:

| Original-Farbe (Beispiel `KLINKS1.png`) | avg | Outline | Fill |
|------------------------------------------|-----|---------|------|
| `(0,0,0,0)` Hintergrund | -- | off | off |
| `(0,0,0)` schwarz, Augen | 0 | **on** | off |
| `(44,0,152)` dunkelblau, Hut/Körper | 65 | **on** | off |
| `(0,128,0)` grün, Detail | 42 | **on** | off |
| `(120,32,0)` braun | 50 | **on** | off |
| `(213,128,0)` orange, Gesicht/Haut | 113 | off | **on** |
| `(204,204,204)` hellgrau, `SKEL-1` Knochen | 204 | off | **on** |

Kuno-Silhouette: Augen + Hut + Körper landen in `_outline` (auf Farbe Schwarz), Hautfarbe + andere mittlere Farben in `_fill` (auf Körperfarbe blau). Skelett: Knochen-Hellgrau landet in `_fill` -- vorher unsichtbar, jetzt sichtbar.

### TGA-Hintergrund-Sonderfall

TGAs (`KBEGINN1-4`, `GECKO1-4`, `WIZROT1-4`) haben keinen Alpha-Kanal -- `Image.open(...).convert("RGBA")` setzt alle Pixel auf `alpha=255`. Damit würde der Hintergrund von `KBEGINN1.TGA` (`(255,100,100)` avg=151) als Fill-Pixel landen.

Lösung: **Background-Color-Probing** in `pack_phase_pair`:

- Pixel `(0,0)` der Quelle gilt als Hintergrund-Marker.
- Wenn dort `alpha == 0` -> Standard-Alpha-Maske (PNGs).
- Sonst: jedes Pixel, das exakt der RGB-Farbe von `(0,0)` entspricht, gilt als transparent (`outline=off, fill=off`), unabhängig von Helligkeit.

Damit funktionieren beide Quellarten (PNG mit Alpha-Kanal und TGA mit Background-Color) deterministisch im selben Konverter. Die Existenz eines Alpha-Kanals vor dem `convert("RGBA")` wird nicht benötigt; entscheidend ist der Pixelwert an `(0,0)` nach Konvertierung.

## Slot-Layout: getrennte Bänke

C64 hardware-Sprite-IDs sind 0..255 (`$D000` Pointer × 64 Byte). Mit 34 Phasen × 2 = 68 Slots werden zwei **klar getrennte** Bänke belegt:

| Bank | Slot-Range im `.bin` | Hardware-ID | Inhalt |
|------|----------------------|-------------|--------|
| Outline | `0..33` (Offset `0..2175`) | `200..233` | Dunkle Konturen (`<dark_threshold`) |
| Fill | `34..67` (Offset `2176..4351`) | `234..267` | Mittel-helle Flächen |

- `total_slots` = **68**
- `kuno_sprites.bin` = 68 × 64 = **4352 Byte** (vorher 2176).
- Pro Phase mit Slot-Index `s` (0..33) ist `outline_id = 200 + s`, `fill_id = 234 + s`. Differenz konstant 34 -> trivialer Index-Offset in TRSE.

`@define`-Block in `kuno_sprites.inc` wird pro Phase **doppelt** ausgegeben:

```
@define KUNO_WALK_LEFT_0_OUTLINE  200
@define KUNO_WALK_LEFT_0_FILL     234
@define KUNO_WALK_LEFT_1_OUTLINE  201
@define KUNO_WALK_LEFT_1_FILL     235
...
@define WIZROT_3_OUTLINE          233
@define WIZROT_3_FILL             267
```

Das alte Single-Sprite-Naming (`KUNO_WALK_LEFT_0`) gibt es nicht mehr. Da das TRSE-`main.ras` aktuell **keinen** dieser Namen referenziert (per Vorgänger-Spec verifiziert), ist der Bruch folgenlos.

## Schema-Änderung `sprite_phases.json`

Zwei neue Top-Level-Felder, sonst unverändert:

```json
{
  "output_bin":   "...",
  "output_inc":   "...",
  "preview_built": "preview/sprites_built.png",
  "sprite_size":  [24, 21],
  "slot_bytes":   64,
  "dark_threshold":   80,
  "bright_threshold": 240,
  "sprite_index_base": 200,
  "total_slots":  68,
  "phases": [
    { "name": "kuno_walk_left_0", "slot": 0, "color_outline": 0, "color_fill": 14,
      "src": { "file": "../../img/KLINKS1.png" } },
    ...
  ]
}
```

Änderungen gegenüber 2026-05-10-Schema:

- `threshold` -> `dark_threshold` + `bright_threshold`.
- `color` (einzeln) -> `color_outline` (default 0/schwarz) + `color_fill` (Body-Farbe wie bisher 14 für Kuno, 5 für Slimer/Gecko/Wizrot, 1 für Skelett).
- `total_slots` 34 -> 68.
- Phase-Slot ist weiterhin 0..33; die Pipeline berechnet die zwei Bank-Offsets daraus.

## Architektur

```
sprite_phases.json
        |
        v
load_config (eager-validate src.file, 24x21, slot in 0..33, dual-color fields)
        |
        v
build_bin
   |
   +-- für jede Phase:
   |     img = Image.open(phase.src.path).convert("RGBA")
   |     outline_bytes, fill_bytes = pack_phase_pair(img, dark, bright)
   |     out[phase.slot * 64 : ...] = outline_bytes              # Bank 0
   |     out[(34 + phase.slot) * 64 : ...] = fill_bytes          # Bank 1
   |
   v
kuno_sprites.bin (4352 Byte = 68 * 64)
        +
build_inc -> kuno_sprites.inc (68 @defines)
        +
render_preview -> sprites_built.png   # overlay-stacked!
```

## Komponenten-Änderungen in `tools/sprites/build_c64_sprites.py`

### Datentypen

```python
@dataclass(frozen=True)
class Phase:
    name: str
    slot: int                 # 0..33, NICHT 0..67
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
    total_slots: int          # 68
    phases: tuple[Phase, ...]
```

`FileSource` bleibt wie aktuell. Single-`threshold`-Feld und Single-`color`-Feld gibt es nicht mehr -- altes Schema wird vom Loader abgelehnt.

### `load_config`

- Liest `dark_threshold` und `bright_threshold` -- fehlt eines -> `ConfigError`.
- Liest `color_outline` und `color_fill` pro Phase -- fehlt eines -> `ConfigError`.
- Validiert `0 <= phase.slot < total_slots // 2` (also slot in `0..33` bei `total_slots=68`). Sonst `SlotOutOfRangeError`.
- `DuplicateSlotError` bleibt: zwei Phasen mit gleichem `slot` weiterhin verboten.
- Eager-Validation der Dateien (existiert, 24×21) bleibt.

### `pack_phase` -> `pack_phase_pair`

```python
def pack_phase_pair(
    image: Image.Image,
    dark_threshold: int,
    bright_threshold: int,
) -> tuple[bytes, bytes]:
    """Return (outline_bytes, fill_bytes), 64 bytes each.

    Background detection: pixel at (0,0) defines transparency. If its
    alpha is 0 (PNG case), the alpha mask drives transparency; otherwise
    any pixel with the same RGB tuple as (0,0) counts as transparent
    (TGA case).
    """
```

Die alte `pack_phase(image, threshold)` wird ersatzlos gestrichen.

### `build_bin`

```python
def build_bin(cfg: Config) -> bytes:
    out = bytearray(cfg.total_slots * cfg.slot_bytes)
    bank_offset = (cfg.total_slots // 2) * cfg.slot_bytes  # = 2176
    for phase in cfg.phases:
        img = Image.open(phase.src.path).convert("RGBA")
        outline, fill = pack_phase_pair(img, cfg.dark_threshold, cfg.bright_threshold)
        out[phase.slot * cfg.slot_bytes : (phase.slot + 1) * cfg.slot_bytes] = outline
        fill_offset = bank_offset + phase.slot * cfg.slot_bytes
        out[fill_offset : fill_offset + cfg.slot_bytes] = fill
    return bytes(out)
```

### `build_inc`

Doppelte Ausgabe pro Phase:

```python
def build_inc(cfg: Config) -> str:
    lines = ["// AUTO-GENERATED by tools/sprites/build_c64_sprites.py"]
    half = cfg.total_slots // 2
    suffix_width = max(len(p.name) + len("_OUTLINE") for p in cfg.phases)
    for phase in sorted(cfg.phases, key=lambda p: p.slot):
        out_idx = cfg.sprite_index_base + phase.slot
        fill_idx = cfg.sprite_index_base + half + phase.slot
        lines.append(f"@define {phase.name.upper()+'_OUTLINE':<{suffix_width}}  {out_idx}")
        lines.append(f"@define {phase.name.upper()+'_FILL':<{suffix_width}}  {fill_idx}")
    return "\n".join(lines) + "\n"
```

### `render_preview`

Pro Phase werden Outline- und Fill-Bank gemeinsam ausgelesen und übereinander auf das Vorschau-Sprite gemalt:

1. Weißer (oder Hintergrund-) Hintergrund pro Zelle.
2. Fill-Bits zuerst in `C64_PALETTE[phase.color_fill]`.
3. Outline-Bits drüber in `C64_PALETTE[phase.color_outline]`.

Damit visualisiert die Preview, was im realen C64-Spiel durch zwei gestackte Hardware-Sprites entsteht.

## Datenfluss

1. CLI: `python tools/sprites/build_c64_sprites.py`
2. `main()` lädt `sprite_phases.json` -> `Config` (68 Slots, 2 Thresholds, 2 Farben pro Phase)
3. `build_bin(cfg)` liest pro Phase die Datei, packt 2×64 Byte, schreibt in zwei getrennte Bank-Bereiche
4. `kuno_sprites.bin` (4352 Byte = 68 × 64) wird geschrieben
5. `build_inc(cfg)` schreibt `kuno_sprites.inc` mit 68 `@define`-Einträgen (`_OUTLINE` + `_FILL`)
6. `render_preview(cfg, bin_data)` schreibt `sprites_built.png` mit gestackten Overlays

## Fehlerbehandlung

| Fehler | Klasse | Zeitpunkt |
|--------|--------|-----------|
| Phase ohne `src.file` | `ConfigError` | `load_config` |
| `dark_threshold` oder `bright_threshold` fehlt | `ConfigError` | `load_config` |
| `color_outline` oder `color_fill` fehlt | `ConfigError` | `load_config` |
| `dark_threshold >= bright_threshold` | `ConfigError` | `load_config` |
| File existiert nicht | `FileNotFoundError` | `load_config` (eager) |
| File ist nicht 24×21 | `ValueError` | `load_config` (eager) |
| `total_slots` ungerade | `ConfigError` | `load_config` |
| Phase-Slot >= `total_slots/2` oder < 0 | `SlotOutOfRangeError` | `load_config` |
| Doppelter Slot | `DuplicateSlotError` | `load_config` |

## Tests in `tools/sprites/test_build_c64_sprites.py`

| Test | Was wird verifiziert |
|------|----------------------|
| `test_pack_phase_pair_all_transparent_returns_two_zero_blocks` | Reines transparentes PNG -> beide Banks 0 |
| `test_pack_phase_pair_dark_pixel_goes_to_outline` | Schwarzer Pixel -> outline-Byte gesetzt, fill-Byte 0 |
| `test_pack_phase_pair_midbright_pixel_goes_to_fill` | Pixel mit avg=150 -> fill gesetzt, outline 0 |
| `test_pack_phase_pair_very_bright_pixel_is_off_in_both` | Pixel mit avg=255 (weiß) -> beide off |
| `test_pack_phase_pair_alpha_zero_kills_both` | Alpha=0 trotz dunkler Farbe -> beide off |
| `test_pack_phase_pair_tga_background_color_treated_transparent` | (0,0) = `(255,100,100,255)` -> alle Pixel mit gleicher RGB -> beide off |
| `test_pack_phase_pair_threshold_boundary_dark` | brightness == dark_threshold -> fill (nicht outline) |
| `test_pack_phase_pair_threshold_boundary_bright` | brightness == bright_threshold -> beide off |
| `test_load_config_requires_dual_thresholds` | Fehlendes `dark_threshold` -> `ConfigError` |
| `test_load_config_requires_dual_colors` | Fehlendes `color_outline` -> `ConfigError` |
| `test_load_config_rejects_dark_ge_bright` | dark==bright -> `ConfigError` |
| `test_load_config_rejects_odd_total_slots` | total_slots=33 -> `ConfigError` |
| `test_load_config_slot_range_is_half_of_total` | slot=34 bei total=68 -> `SlotOutOfRangeError` |
| `test_build_bin_outline_lands_in_lower_bank` | Outline-Bytes bei `phase.slot * 64` |
| `test_build_bin_fill_lands_in_upper_bank` | Fill-Bytes bei `(34 + phase.slot) * 64` |
| `test_build_inc_emits_two_defines_per_phase` | `_OUTLINE` und `_FILL` pro Phase, Differenz immer 34 |
| `test_build_bin_real_kuno_walk_left_separates_eyes_from_body` | Augen-Pixel landet in outline, Körper-Pixel in fill |
| `test_build_bin_total_size_4352` | 68 × 64 = 4352 |

Bestehende `pack_phase`-Tests werden gelöscht (Funktion entfällt). Die Test-Helper auf neues Schema portieren.

## Migration in einem Schwung

1. **`sprite_phases.json`** umschreiben:
   - Top-Level `threshold` -> `dark_threshold: 80`, `bright_threshold: 240`.
   - `total_slots: 34` -> `total_slots: 68`.
   - Pro Phase: `color: X` -> `color_outline: 0` (schwarz) + `color_fill: X` (alte Farbe).
2. **`build_c64_sprites.py`**:
   - `pack_phase` -> `pack_phase_pair` (inkl. TGA-Background-Probe).
   - `Phase`-Felder, `Config`-Felder umstellen.
   - `load_config` neue Validation.
   - `build_bin` zwei-Bank-Layout.
   - `build_inc` doppel-define.
   - `render_preview` Overlay-Stack.
3. **Tests** komplett auf neue Funktionen umstellen.
4. **Build laufen lassen**: `kuno_sprites.bin` (4352 B), `kuno_sprites.inc` (68 Einträge), neue Preview.
5. **Visueller Check** der Preview: Augen + Hut sichtbar, Skelett-Knochen sichtbar, KBEGINN-Hintergrund weg.

## Risiken und offene Punkte

- **Threshold-Tuning**: `80`/`240` ist eine erste Schätzung aus den oben analysierten Pixel-Werten. Wenn Kuno-Gesicht-Orange (avg=113) im Fill landet, aber das Gesicht zu klein aussieht, lässt sich `dark_threshold` höher drehen, damit auch der dunkelblaue Body zum Fill rutscht und die Outline auf Hut+Augen reduziert wird. Iteration nach erstem visuellen Check erwartet.
- **TGA-Background-Probe an Pixel (0,0)** ist eine Konvention. Falls eine TGA-Datei zufällig im (0,0)-Pixel einen Vordergrund-Pixel hätte, würde diese Farbe global transparent gemacht. Risiko gering, weil `img/`-Originale durchgehend Border-Padding haben.
- **TRSE-Folge-Arbeit**: `main.ras` muss zwei Sprite-IDs (`*_OUTLINE` + `*_FILL`) pro Animation Frame setzen und die zugehörigen Farbregister (`$D027..$D02E`) auf die zwei Farben legen. Außerhalb dieser Spec, separater Plan.
- **Sprite-Budget**: C64 hat 8 Hardware-Sprites pro Rasterzeile (per Multiplexing skalierbar). Kuno mit Overlay belegt 2 -- bleibt Budget für Gegner und UI? Klären, ob `wizrot` und Gegner überhaupt Overlay brauchen oder ohne darstellbar bleiben (in diesem Schritt bekommen sie es auch, der Konsumenten-Code kann jederzeit nur `_FILL` nehmen).
- **`color_outline` derzeit hardcoded 0 (schwarz) in JSON**: Wenn später eine Phase einen weißen Highlight-Akzent braucht (z. B. ein hellgrauer Schwert-Reflex über dem blauen Body), kann das pro Phase auf eine andere C64-Farbe gesetzt werden. Schema deckt das ab, JSON-Default ist schwarz.

## Nächste Schritte nach Approval

1. Implementierungsplan unter `docs/superpowers/plans/2026-05-12-c64-sprite-overlay-mode.md` schreiben (Skeleton -> atomare Funktions-Commits analog zur 2026-05-10-Serie).
2. Skeleton-Commit: `pack_phase_pair`-Stub + neue Schema-Felder.
3. Pro Funktion ein atomarer Commit.
4. Build + visueller Check auf `sprites_built.png`.
5. TRSE-Anpassung in `main.ras` als separate Folge-Spec.
