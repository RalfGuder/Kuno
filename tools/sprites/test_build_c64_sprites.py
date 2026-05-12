"""Tests for build_c64_sprites."""
import json
from pathlib import Path

import pytest
from PIL import Image

from build_c64_sprites import (
    ConfigError,
    DuplicateSlotError,
    FileSource,
    Phase,
    SlotOutOfRangeError,
    build_bin,
    build_inc,
    load_config,
    pack_phase_pair,
)


def _blank() -> Image.Image:
    return Image.new("RGBA", (24, 21), (255, 255, 255, 0))


def test_pack_phase_pair_all_transparent_returns_two_zero_blocks():
    img = _blank()
    outline, fill = pack_phase_pair(img, dark_threshold=80, bright_threshold=240)
    assert outline == bytes(64)
    assert fill == bytes(64)


def test_pack_phase_pair_dark_pixel_goes_to_outline():
    img = _blank()
    img.putpixel((1, 0), (0, 0, 0, 255))
    outline, fill = pack_phase_pair(img, dark_threshold=80, bright_threshold=240)
    assert outline[0] == 0x40  # bit (7 - 1) of byte 0
    assert fill == bytes(64)


def test_pack_phase_pair_midbright_pixel_goes_to_fill():
    img = _blank()
    img.putpixel((1, 0), (150, 150, 150, 255))  # avg 150
    outline, fill = pack_phase_pair(img, dark_threshold=80, bright_threshold=240)
    assert outline == bytes(64)
    assert fill[0] == 0x40  # bit (7 - 1) of byte 0


def test_pack_phase_pair_very_bright_pixel_is_off_in_both():
    img = _blank()
    img.putpixel((0, 0), (255, 255, 255, 255))
    outline, fill = pack_phase_pair(img, dark_threshold=80, bright_threshold=240)
    assert outline == bytes(64)
    assert fill == bytes(64)


def test_pack_phase_pair_alpha_zero_kills_both():
    img = _blank()
    img.putpixel((0, 0), (0, 0, 0, 0))
    outline, fill = pack_phase_pair(img, dark_threshold=80, bright_threshold=240)
    assert outline == bytes(64)
    assert fill == bytes(64)


def test_pack_phase_pair_tga_background_color_treated_transparent():
    """Opaque (0,0) pixel defines the background color for the whole sprite."""
    img = Image.new("RGBA", (24, 21), (255, 100, 100, 255))
    img.putpixel((5, 5), (0, 0, 0, 255))
    outline, fill = pack_phase_pair(img, dark_threshold=80, bright_threshold=240)
    expected_outline = bytearray(64)
    expected_outline[5 * 3] = 1 << (7 - 5)
    assert outline == bytes(expected_outline)
    assert fill == bytes(64)


def test_pack_phase_pair_threshold_boundary_dark():
    """brightness == dark_threshold goes to fill (outline uses '<')."""
    img = _blank()
    img.putpixel((1, 0), (80, 80, 80, 255))
    outline, fill = pack_phase_pair(img, dark_threshold=80, bright_threshold=240)
    assert outline[0] == 0x00
    assert fill[0] == 0x40


def test_pack_phase_pair_threshold_boundary_bright():
    """brightness == bright_threshold drops out of fill (fill uses '<')."""
    img = _blank()
    img.putpixel((1, 0), (240, 240, 240, 255))
    outline, fill = pack_phase_pair(img, dark_threshold=80, bright_threshold=240)
    assert outline[0] == 0x00
    assert fill[0] == 0x00


def _make_src(tmp_path: Path, name: str, fill=(0, 0, 0, 255)) -> str:
    """Create a 24x21 RGBA PNG at tmp_path/name and return its filename."""
    img = Image.new("RGBA", (24, 21), fill)
    img.save(tmp_path / name)
    return name


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
            {"name": "a", "slot": 0, "color_outline": 0, "color_fill": 14, "src": {"file": "a.png"}},
            {"name": "b", "slot": 1, "color_outline": 0, "color_fill": 5,  "src": {"file": "b.png"}},
        ],
    }
    base.update(overrides)
    p = tmp_path / "config.json"
    p.write_text(json.dumps(base))
    return p


def test_filesource_is_frozen_dataclass():
    src = FileSource(path=Path("img/KLINKS1.png"))
    assert src.path == Path("img/KLINKS1.png")
    with pytest.raises(Exception):
        src.path = Path("other.png")


def test_load_config_happy_path(tmp_path):
    cfg = load_config(_write_config(tmp_path, {}))
    assert cfg.dark_threshold == 80
    assert cfg.bright_threshold == 240
    assert cfg.total_slots == 4
    assert len(cfg.phases) == 2
    assert cfg.phases[0].name == "a"
    assert cfg.phases[0].slot == 0
    assert cfg.phases[0].color_outline == 0
    assert cfg.phases[0].color_fill == 14
    assert isinstance(cfg.phases[0].src, FileSource)
    assert cfg.phases[0].src.path.name == "a.png"


def test_load_config_rejects_phase_without_src(tmp_path):
    cfg = _write_config(
        tmp_path,
        {
            "phases": [
                {"name": "a", "slot": 0, "color_outline": 0, "color_fill": 14},
                {"name": "b", "slot": 1, "color_outline": 0, "color_fill": 5, "src": {"file": "b.png"}},
            ]
        },
    )
    with pytest.raises(ConfigError) as exc:
        load_config(cfg)
    assert "a" in str(exc.value)


def test_load_config_rejects_phase_without_file_key(tmp_path):
    cfg = _write_config(
        tmp_path,
        {
            "phases": [
                {"name": "a", "slot": 0, "color_outline": 0, "color_fill": 14, "src": {}},
                {"name": "b", "slot": 1, "color_outline": 0, "color_fill": 5, "src": {"file": "b.png"}},
            ]
        },
    )
    with pytest.raises(ConfigError):
        load_config(cfg)


def test_load_config_rejects_missing_source_file(tmp_path):
    cfg = _write_config(
        tmp_path,
        {
            "phases": [
                {"name": "a", "slot": 0, "color_outline": 0, "color_fill": 14, "src": {"file": "ghost.png"}},
                {"name": "b", "slot": 1, "color_outline": 0, "color_fill": 5, "src": {"file": "b.png"}},
            ]
        },
    )
    with pytest.raises(FileNotFoundError) as exc:
        load_config(cfg)
    assert "ghost.png" in str(exc.value)


def test_load_config_rejects_wrong_dimensions(tmp_path):
    Image.new("RGBA", (32, 32), (0, 0, 0, 255)).save(tmp_path / "wrong.png")
    _make_src(tmp_path, "b.png")
    raw = {
        "output_bin": "kuno_sprites.bin",
        "output_inc": "kuno_sprites.inc",
        "preview_built": "preview/sprites_built.png",
        "sprite_size": [24, 21],
        "slot_bytes": 64,
        "dark_threshold": 80,
        "bright_threshold": 240,
        "sprite_index_base": 200,
        "total_slots": 2,
        "phases": [
            {"name": "a", "slot": 0, "color_outline": 0, "color_fill": 14, "src": {"file": "wrong.png"}},
            {"name": "b", "slot": 1, "color_outline": 0, "color_fill": 5, "src": {"file": "b.png"}},
        ],
    }
    p = tmp_path / "config.json"
    p.write_text(json.dumps(raw))
    with pytest.raises(ValueError) as exc:
        load_config(p)
    assert "(32, 32)" in str(exc.value)


def test_load_config_rejects_duplicate_slot(tmp_path):
    cfg_path = _write_config(
        tmp_path,
        {
            "phases": [
                {"name": "a", "slot": 0, "color_outline": 0, "color_fill": 14, "src": {"file": "a.png"}},
                {"name": "b", "slot": 0, "color_outline": 0, "color_fill": 5,  "src": {"file": "b.png"}},
            ]
        },
    )
    with pytest.raises(DuplicateSlotError) as exc:
        load_config(cfg_path)
    assert "0" in str(exc.value)
    assert "a" in str(exc.value) and "b" in str(exc.value)


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


def test_build_bin_size_equals_total_slots_times_slot_bytes(tmp_path):
    cfg_path = _write_config(tmp_path, {"total_slots": 6})
    cfg = load_config(cfg_path)
    data = build_bin(cfg)
    assert len(data) == 6 * 64


def test_build_bin_unused_slots_are_zero(tmp_path):
    """Slots without a phase entry are zero-padded."""
    cfg_path = _write_config(
        tmp_path,
        {
            "total_slots": 4,
            "phases": [
                {"name": "a", "slot": 1, "color_outline": 0, "color_fill": 14, "src": {"file": "a.png"}},
            ],
        },
    )
    cfg = load_config(cfg_path)
    data = build_bin(cfg)
    assert data[0:64] == bytes(64)
    assert data[128:192] == bytes(64)


def test_build_bin_outline_lands_in_lower_bank(tmp_path):
    """A dark pixel of phase slot=1 lands at byte 1*64 in the lower bank."""
    img = Image.new("RGBA", (24, 21), (255, 255, 255, 0))
    img.putpixel((1, 0), (0, 0, 0, 255))
    img.save(tmp_path / "dark.png")
    cfg = _write_config(
        tmp_path,
        {"total_slots": 4,
         "phases": [
             {"name": "x", "slot": 1,
              "color_outline": 0, "color_fill": 14,
              "src": {"file": "dark.png"}},
         ]},
    )
    data = build_bin(load_config(cfg))
    assert data[1 * 64] == 0x40
    # nothing in upper bank for this phase
    assert data[(2 + 1) * 64] == 0x00


def test_build_bin_fill_lands_in_upper_bank(tmp_path):
    """A mid-bright pixel of phase slot=1 lands at byte (half+1)*64."""
    img = Image.new("RGBA", (24, 21), (255, 255, 255, 0))
    img.putpixel((1, 0), (150, 150, 150, 255))
    img.save(tmp_path / "mid.png")
    cfg = _write_config(
        tmp_path,
        {"total_slots": 4,
         "phases": [
             {"name": "x", "slot": 1,
              "color_outline": 0, "color_fill": 14,
              "src": {"file": "mid.png"}},
         ]},
    )
    data = build_bin(load_config(cfg))
    assert data[1 * 64] == 0x00            # outline bank slot 1 empty
    assert data[(2 + 1) * 64] == 0x40      # fill bank slot 1 (half=2)


def test_build_bin_loads_real_kbeginn_tga():
    """The 1996 TGA originals load via Pillow and pack without error."""
    cfg = load_config(Path(__file__).parent / "sprite_phases.json")
    data = build_bin(cfg)
    assert len(data) == cfg.total_slots * cfg.slot_bytes
    spawn_0_offset = 0 * cfg.slot_bytes
    spawn_slot = data[spawn_0_offset : spawn_0_offset + cfg.slot_bytes]
    assert any(b != 0 for b in spawn_slot[:63]), "spawn_0 TGA produced empty sprite"


def test_build_inc_emits_outline_and_fill_per_phase(tmp_path):
    cfg_path = _write_config(
        tmp_path,
        {
            "sprite_index_base": 200,
            "total_slots": 6,
            "phases": [
                {"name": "kuno_walk_left_0", "slot": 0,
                 "color_outline": 0, "color_fill": 14, "src": {"file": "a.png"}},
                {"name": "gecko_left_0", "slot": 1,
                 "color_outline": 0, "color_fill": 5,  "src": {"file": "b.png"}},
            ],
        },
    )
    cfg = load_config(cfg_path)
    text = build_inc(cfg)
    assert any(line.startswith("// AUTO-GENERATED") for line in text.splitlines())
    assert "@define KUNO_WALK_LEFT_0_OUTLINE" in text
    assert "@define KUNO_WALK_LEFT_0_FILL" in text
    assert "@define GECKO_LEFT_0_OUTLINE" in text
    assert "@define GECKO_LEFT_0_FILL" in text
    # 2 phases * 2 defines = 4 @define lines
    assert text.count("@define") == 4


def test_render_preview_runs_without_error(tmp_path):
    from build_c64_sprites import render_preview
    cfg_path = _write_config(tmp_path, {})
    cfg = load_config(cfg_path)
    data = build_bin(cfg)
    img = render_preview(cfg, data)
    assert img.size[0] > 0 and img.size[1] > 0


def test_build_inc_fill_offset_is_half_of_total_slots(tmp_path):
    cfg_path = _write_config(tmp_path, {})  # total_slots=4, half=2
    cfg = load_config(cfg_path)
    text = build_inc(cfg)
    out_a = next(ln for ln in text.splitlines() if "A_OUTLINE" in ln)
    fill_a = next(ln for ln in text.splitlines() if "A_FILL" in ln)
    out_id = int(out_a.split()[-1])
    fill_id = int(fill_a.split()[-1])
    # half = total_slots / 2 = 2
    assert fill_id - out_id == 2
