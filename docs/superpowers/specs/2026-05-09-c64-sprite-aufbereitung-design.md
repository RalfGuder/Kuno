# C64-Sprite-Aufbereitung für Kuno

**Datum:** 2026-05-09
**Branch-Kontext:** `dev/c64`
**Spec-Typ:** Design

## Motivation

Im TRSE/C64-Port (`src/main/trse/Kuno/`) liegen für die Bereiche Charset, Game-Tiles und Titelbild bereits aufbereitete Daten vor. Die Sprite-Daten fehlen — `sprites/sprites.flf` und `sprites/sprites.bin` enthalten noch das ungenutzte TRSE-Sample, `sprites/kuno.flf` ist ein früher manueller Versuch mit nur einem Sprite. Das einzige digital vorhandene Original-Spritesheet ist `src/main/resources/kuno-sprites.png` (640×63, RGBA), das aus dem 1996er-DOS-Spiel stammt und 27 belegte Sprite-Slots im 24×21-Raster enthält.

Ziel: Diese 27 Sprites reproduzierbar aus dem PNG in C64-Hardware-Sprite-Daten überführen, in `main.ras` als `incbin` einbinden und die alten `.flf`/`.bin`-Reste entfernen.

## Entscheidungen

- **Sprite-Modus:** Hires/Monochrom (24×21, 1 Farbe + transparent). Volle Original-Auflösung, einfacher Workflow, sparsam mit den 8 gleichzeitig nutzbaren Hardware-Sprite-Slots.
- **Umfang:** Alles aus `kuno-sprites.png` — Kuno + Slimer + Gecko + Wiz-Rotator + Kuno-Sterbe-Animation = 27 Sprites.
- **Workflow:** Reproduzierbares Python-Skript (Pillow + stdlib `json`), keine TRSE-Editor-Abhängigkeit. Künftiges Editing geschieht durch Bearbeiten des Quell-PNGs und erneuten Skript-Lauf.
- **Konfigurationsformat:** JSON.
- **Sprite-Index-Basis:** 200 (`$3200/64`), entspricht der bereits in `main.ras` definierten `@spriteLoc $3200`.
- **Reserve-Strategie:** Die freien 24×21-Zellen im 26×3-Raster des Quell-PNGs bleiben unangetastet — künftige Sprites können dort ergänzt werden, ohne das Layout der bestehenden zu verschieben. Im `.bin`-Output dagegen wird **nur die belegte Anzahl Slots geschrieben** (`total_slots: 27`), kein Padding für die ungenutzten PNG-Raster-Positionen. Wenn neue Phasen dazukommen, werden Slot-Index und `total_slots` in der JSON erweitert, das `.inc` regeneriert sich automatisch.

## Architektur & Datenfluss

```
  src/main/resources/kuno-sprites.png            (640×63 RGBA, 27 belegte 24×21-Slots)
        │
        ▼
  tools/sprites/build_c64_sprites.py             (Python 3 + Pillow)
        │   1. PNG laden
        │   2. Phasen anhand sprite_phases.json ausschneiden
        │   3. binär schwellwerten (Helligkeit < threshold → Vordergrund)
        │   4. zu C64-Sprite-Bytes packen (24 Bit/Zeile, MSB-first, 64-B-Slot)
        │   5. .bin und .inc schreiben
        │   6. Round-trip-Vorschau sprites_built.png erzeugen
        ▼
  src/main/trse/Kuno/sprites/kuno_sprites.bin    (27 × 64 B = 1728 B)
  src/main/trse/Kuno/sprites/kuno_sprites.inc    (TRSE-Pascal @define-Konstanten)
        │
        ▼
  src/main/trse/Kuno/main.ras
        ├── incbin "sprites/kuno_sprites.bin" → @spriteLoc ($3200)
        └── @include "sprites/kuno_sprites.inc"
```

Single Source of Truth bleibt das PNG. Der Build ist deterministisch und idempotent.

## Komponenten

### `tools/sprites/build_c64_sprites.py`

Ein einziges Python-3-Skript. Aufruf ohne Argumente:

```
python tools/sprites/build_c64_sprites.py
```

Liest `tools/sprites/sprite_phases.json`, schreibt Output-Dateien an die in der Konfig deklarierten Pfade. Bricht hart ab bei Fehlern (out-of-bounds Phase, fehlende Datei) — keine stillen Defaults.

Abhängigkeiten: nur `Pillow` und Python-stdlib (`json`, `pathlib`, `struct`).

### `tools/sprites/sprite_phases.json`

Konfiguration mit folgender Struktur:

```json
{
  "source_image": "../../src/main/resources/kuno-sprites.png",
  "output_bin":   "../../src/main/trse/Kuno/sprites/kuno_sprites.bin",
  "output_inc":   "../../src/main/trse/Kuno/sprites/kuno_sprites.inc",
  "preview_built":"preview/sprites_built.png",
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

Felder im Detail:
- `slot` — Logischer Sprite-Index (0-basiert), bestimmt die Position im `.bin`. Der C64-Sprite-Pointer-Wert ist `sprite_index_base + slot`.
- `pos` — `[x, y]` der oberen linken Ecke im Quell-PNG.
- `color` — C64-Farbcode (0–15). Rein dokumentarisch im `.bin` — Hires-Sprite-Bytes sind monochrom; die Farbe wird zur Laufzeit über VIC-Register `$D027`–`$D02E` gesetzt. Wird auch von der Round-trip-Vorschau verwendet.
- `total_slots` — Gesamtgröße der Bin-Datei in 64-Byte-Slots. Reserve-Slots ohne `phases`-Eintrag werden als Null-Bytes geschrieben.

### `src/main/trse/Kuno/sprites/kuno_sprites.bin`

Genau `total_slots × slot_bytes` Bytes (hier: 1728 B). Layout:

```
$3200 + slot × 64:  63 Bytes Pixel-Daten + 1 Byte Padding
```

### `src/main/trse/Kuno/sprites/kuno_sprites.inc`

Auto-generierte TRSE-Pascal-Datei mit `@define`-Konstanten pro Phase, sodass `main.ras` mit lesbaren Namen statt magischen Indizes arbeiten kann:

```pascal
// AUTO-GENERATED by tools/sprites/build_c64_sprites.py — do not edit
@define KUNO_SPAWN_0       200
@define KUNO_SPAWN_1       201
@define KUNO_SPAWN_2       202
@define KUNO_SPAWN_3       203
@define KUNO_WALK_LEFT_0   204
@define KUNO_WALK_LEFT_1   205
@define KUNO_JUMP_LEFT     206
@define KUNO_STAND_LEFT    207
@define KUNO_WALK_RIGHT_0  208
@define KUNO_WALK_RIGHT_1  209
@define KUNO_JUMP_RIGHT    210
@define KUNO_STAND_RIGHT   211
@define SLIMER_LEFT_0      212
@define SLIMER_LEFT_1      213
@define SLIMER_RIGHT_0     214
@define SLIMER_RIGHT_1     215
@define GECKO_LEFT_0       216
@define GECKO_LEFT_1       217
@define GECKO_RIGHT_0      218
@define GECKO_RIGHT_1      219
@define WIZROT_0           220
@define WIZROT_1           221
@define WIZROT_2           222
@define WIZROT_3           223
@define KUNO_DEAD_0        224
@define KUNO_DEAD_1        225
@define KUNO_DEAD_2        226
```

## Datenstruktur: C64-Hardware-Sprite (Hires/Mono)

Pro Sprite genau **63 Bytes** Pixel-Daten + 1 Byte Padding = **64-Byte-Slot**.

```
Pixel-Layout je Zeile (24 Bit = 3 Bytes), MSB-first:

  Pixel:     0  1  2  3  4  5  6  7   8  9 10 11 12 13 14 15  16 17 ... 23
  Bit:       7  6  5  4  3  2  1  0   7  6  5  4  3  2  1  0   7  6 ...  0
  Byte:     ┌─────── Byte 0 ───────┐ ┌─────── Byte 1 ───────┐ ┌── Byte 2 ──┐

Bit=1 → Pixel hat Sprite-Farbe (aus VIC-Register $D027–$D02E)
Bit=0 → transparent

21 Zeilen → 63 Bytes; das 64. Byte bleibt 0 (Slot-Padding).
```

Pseudo-Code für die Pack-Routine:

```python
def pack_phase(phase_image: Image, threshold: int) -> bytes:
    out = bytearray(64)
    for y in range(21):
        for x in range(24):
            r, g, b, a = phase_image.getpixel((x, y))
            is_fg = a > 0 and (r + g + b) / 3 < threshold
            if is_fg:
                byte_idx = y * 3 + x // 8
                bit_idx  = 7 - (x % 8)
                out[byte_idx] |= (1 << bit_idx)
    return bytes(out)
```

## Phasen-Inventar (Mapping aus dem Quell-PNG)

| Slot  | Position    | Phase-Name                | C64-Idx | Original-Buffer (cpp) |
|------:|:------------|:--------------------------|:--------|:----------------------|
| 0–3   | (0–72, 0)   | `kuno_spawn_0..3`         | 200–203 | `KBEGINN1..4`         |
| 4–5   | (96–120, 0) | `kuno_walk_left_0..1`     | 204–205 | `KLINKS1..2`          |
| 6     | (144, 0)    | `kuno_jump_left`          | 206     | `KLINKSSPR`           |
| 7     | (168, 0)    | `kuno_stand_left`         | 207     | `KLINKSST`            |
| 8–9   | (192–216, 0)| `kuno_walk_right_0..1`    | 208–209 | `KRECHTS1..2`         |
| 10    | (240, 0)    | `kuno_jump_right`         | 210     | `KRECHTSSPR`          |
| 11    | (264, 0)    | `kuno_stand_right`        | 211     | `KRECHTSST`           |
| 12–13 | (0–24, 21)  | `slimer_left_0..1`        | 212–213 | `SLIMER1..2`          |
| 14–15 | (48–72, 21) | `slimer_right_0..1`       | 214–215 | `SLIMER1..2` rechts   |
| 16–17 | (96–120, 21)| `gecko_left_0..1`         | 216–217 | `GECKO1..2`           |
| 18–19 | (144–168,21)| `gecko_right_0..1`        | 218–219 | `GECKO3..4`           |
| 20–23 | (192–264,21)| `wizrot_0..3`             | 220–223 | `WIZROT1..4`          |
| 24–26 | (0–48, 42)  | `kuno_dead_0..2`          | 224–226 | `SKELETT1..3`         |

Slot-Belegung im 26×3-Raster (`#` = belegt, `.` = Reserve, freie Slots werden als Null-Bytes geschrieben):

```
       Spalte: 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
Reihe 0 (y=0):  #  #  #  #  #  #  #  #  #  #  #  #  .  .  .  .  .  .  .  .  .  .  .  .  .  .
Reihe 1 (y=21): #  #  #  #  #  #  #  #  #  #  #  #  .  .  .  .  .  .  .  .  .  .  .  .  .  .
Reihe 2 (y=42): #  #  #  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .
```

Hinweis zu Reserven: Das `.bin` enthält genau `total_slots = 27` Slots — Reserven entstehen erst, wenn `total_slots` in der JSON erhöht wird. So vermeiden wir initial 51 leere Slots im `.bin`.

## Validierung

### Round-trip-Vorschau
Das Skript erzeugt zusätzlich `tools/sprites/preview/sprites_built.png` — eine Visualisierung, die die geschriebenen `.bin`-Bytes zurück zu Pixeln decodiert (mit der pro Phase konfigurierten C64-Farbe). Der visuelle Vergleich `kuno-sprites.png` ↔ `sprites_built.png` ist die primäre Akzeptanzprüfung.

### Schwellwert-Justierung
Der Schwellwert `200` (Helligkeit) ist ein Startwert. Falls Anti-Aliasing-Ränder im PNG zu unsauberen Sprite-Konturen führen, wird der Wert in der JSON nachjustiert — kein Code-Change.

### Fehlerverhalten des Skripts
- **Out-of-bounds Phase**: `pos + sprite_size` liegt außerhalb des PNGs → Skript bricht ab mit `PhaseOutOfBounds: <name> at (<x>,<y>)`.
- **Doppelter Slot**: zwei Phasen mit demselben `slot` → Skript bricht ab mit `DuplicateSlot: <slot> used by <name1> and <name2>`.
- **Fehlende Quelle**: `source_image` existiert nicht → `FileNotFoundError` mit Pfad-Hinweis.
- **Slot ≥ total_slots**: → Skript bricht ab mit `SlotOutOfRange: <slot> >= total_slots=<n>`.

## Migration & Cleanup

### Zu löschen
```
src/main/trse/Kuno/sprites/sprites.flf    (TRSE-Sample, 1.787 B)
src/main/trse/Kuno/sprites/sprites.bin    (TRSE-Sample, 768 B)
src/main/trse/Kuno/sprites/kuno.flf       (früher manueller Versuch, 401 B)
```

### Neu zu erstellen
```
tools/sprites/build_c64_sprites.py
tools/sprites/sprite_phases.json
tools/sprites/preview/sprites_built.png   (Skript-Output)
src/main/trse/Kuno/sprites/kuno_sprites.bin
src/main/trse/Kuno/sprites/kuno_sprites.inc
```

Bereits existierend: `tools/sprites/preview/sprites_inventory.png` (Quell-Vorschau zur Mapping-Verifikation).

### Anpassung in `src/main/trse/Kuno/main.ras`
```diff
- mySprites:incbin("sprites/sprites.bin", @spriteLoc);
+ mySprites:incbin("sprites/kuno_sprites.bin", @spriteLoc);
+ @include "sprites/kuno_sprites.inc"
```

### Verifikationsschritte
1. `python tools/sprites/build_c64_sprites.py` läuft fehlerfrei durch.
2. Visueller Vergleich `kuno-sprites.png` ↔ `sprites_built.png` — beide zeigen identischen monochromen Sprite-Inhalt.
3. TRSE öffnet `Kuno.trse` und kompiliert `main.ras` → `main.prg` ohne Fehler. *(manuell, GUI-Tool)*
4. `main.prg` startet im VICE-Emulator und zeigt mindestens einen Kuno-Sprite korrekt an (z. B. auf dem Titelbildschirm). *(manuell, GUI-Tool)*

Schritte 3+4 sind Mensch-Aufgaben — TRSE und VICE haben keine Headless-CLI im Projekt-Setup.

## Außerhalb des Scopes

Die folgenden Punkte sind **explizit nicht** Teil dieser Spec:

- **Animations-State-Machine in `main.ras`** — wann welcher Sprite-Index angezeigt wird, ist Aufgabe der Spielcode-Logik, nicht der Daten-Aufbereitung. Die `.inc`-Konstanten sind Voraussetzung dafür, aber die Logik kommt in einer eigenen Iteration.
- **Sprite-Multiplexing** (mehr als 8 gleichzeitig sichtbare Sprites per Raster-Interrupt) — separates Thema.
- **Multicolor-Sprites** — wurde explizit zugunsten von Hires/Mono verworfen.
- **Layered-Sprites** (mehrere Hardware-Sprites pro Figur für Mehrfarbigkeit) — wurde verworfen.
- **Neu zu pixelnde Sprites** für die fehlenden `.tga`-Dateien (Hexe, Schlüssel, Wappen, Tore, Schalter, Sammelobjekte) — gehört in eine spätere Iteration, sobald entweder die Originale wieder auftauchen oder eine bewusste Neuschöpfung beauftragt wird.
- **TRSE-`.flf`-Generierung** — bewusst weggelassen; der TRSE-Sprite-Editor ist nicht mehr im Workflow.

## Akzeptanzkriterien

1. `tools/sprites/build_c64_sprites.py` und `tools/sprites/sprite_phases.json` existieren mit dem oben spezifizierten Inhalt.
2. Skript-Lauf erzeugt `kuno_sprites.bin` (1728 B) und `kuno_sprites.inc` (27 `@define`-Zeilen) deterministisch.
3. `tools/sprites/preview/sprites_built.png` existiert und zeigt visuell dieselben 27 Sprites wie `kuno-sprites.png`.
4. Die drei genannten alten Dateien (`sprites.flf`, `sprites.bin`, `kuno.flf`) sind gelöscht.
5. `main.ras` referenziert `kuno_sprites.bin` und includiert `kuno_sprites.inc`.
6. Alle vier Fehlerfälle aus „Validierung" lösen klare Abbrüche aus (per Unit-Test oder manuellem Provoke verifizierbar).
