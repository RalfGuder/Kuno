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
    pack_phase,
)


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
    assert out[0:63] == b"\xff" * 63
    assert out[63] == 0


def test_pack_phase_alpha_zero_is_transparent_regardless_of_color():
    img = _blank()
    img.putpixel((5, 5), (0, 0, 0, 0))
    out = pack_phase(img, threshold=200)
    assert out == bytes(64)


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
        "threshold": 200,
        "sprite_index_base": 200,
        "total_slots": 2,
        "phases": [
            {"name": "a", "slot": 0, "color": 14, "src": {"file": "a.png"}},
            {"name": "b", "slot": 1, "color": 5,  "src": {"file": "b.png"}},
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
    assert cfg.threshold == 200
    assert cfg.total_slots == 2
    assert len(cfg.phases) == 2
    assert cfg.phases[0].name == "a"
    assert cfg.phases[0].slot == 0
    assert cfg.phases[0].color == 14
    assert isinstance(cfg.phases[0].src, FileSource)
    assert cfg.phases[0].src.path.name == "a.png"


def test_load_config_rejects_phase_without_src(tmp_path):
    cfg = _write_config(
        tmp_path,
        {
            "phases": [
                {"name": "a", "slot": 0, "color": 14},
                {"name": "b", "slot": 1, "color": 5, "src": {"file": "b.png"}},
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
                {"name": "a", "slot": 0, "color": 14, "src": {}},
                {"name": "b", "slot": 1, "color": 5, "src": {"file": "b.png"}},
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
                {"name": "a", "slot": 0, "color": 14, "src": {"file": "ghost.png"}},
                {"name": "b", "slot": 1, "color": 5, "src": {"file": "b.png"}},
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
        "threshold": 200,
        "sprite_index_base": 200,
        "total_slots": 2,
        "phases": [
            {"name": "a", "slot": 0, "color": 14, "src": {"file": "wrong.png"}},
            {"name": "b", "slot": 1, "color": 5, "src": {"file": "b.png"}},
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
                {"name": "a", "slot": 0, "color": 14, "src": {"file": "a.png"}},
                {"name": "b", "slot": 0, "color": 5,  "src": {"file": "b.png"}},
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
                {"name": "a", "slot": 0, "color": 14, "src": {"file": "a.png"}},
                {"name": "b", "slot": 5, "color": 5,  "src": {"file": "b.png"}},
            ],
        },
    )
    with pytest.raises(SlotOutOfRangeError) as exc:
        load_config(cfg_path)
    assert "5" in str(exc.value) and "2" in str(exc.value)


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
                {"name": "a", "slot": 1, "color": 14, "src": {"file": "a.png"}},
            ],
        },
    )
    cfg = load_config(cfg_path)
    data = build_bin(cfg)
    assert data[0:64] == bytes(64)
    assert data[128:192] == bytes(64)


def test_build_bin_phase_at_correct_slot_offset(tmp_path):
    """A foreground pixel at (0,0) of phase slot=2 lands at byte 2*64."""
    img = Image.new("RGBA", (24, 21), (255, 255, 255, 0))
    img.putpixel((0, 0), (0, 0, 0, 255))
    img.save(tmp_path / "one_pixel.png")
    raw = {
        "output_bin": "kuno_sprites.bin",
        "output_inc": "kuno_sprites.inc",
        "preview_built": "preview/sprites_built.png",
        "sprite_size": [24, 21],
        "slot_bytes": 64,
        "threshold": 200,
        "sprite_index_base": 200,
        "total_slots": 3,
        "phases": [
            {"name": "x", "slot": 2, "color": 14, "src": {"file": "one_pixel.png"}},
        ],
    }
    p = tmp_path / "config.json"
    p.write_text(json.dumps(raw))
    cfg = load_config(p)
    data = build_bin(cfg)
    assert data[2 * 64] == 0x80
    assert data[2 * 64 + 1] == 0x00


def test_build_bin_loads_real_kbeginn_tga():
    """The 1996 TGA originals load via Pillow and pack without error."""
    cfg = load_config(Path(__file__).parent / "sprite_phases.json")
    data = build_bin(cfg)
    assert len(data) == cfg.total_slots * cfg.slot_bytes
    spawn_0_offset = 0 * cfg.slot_bytes
    spawn_slot = data[spawn_0_offset : spawn_0_offset + cfg.slot_bytes]
    assert any(b != 0 for b in spawn_slot[:63]), "spawn_0 TGA produced empty sprite"


def test_build_inc_emits_define_per_phase(tmp_path):
    cfg_path = _write_config(
        tmp_path,
        {
            "sprite_index_base": 200,
            "total_slots": 3,
            "phases": [
                {"name": "kuno_walk_left_0", "slot": 0, "color": 14, "src": {"file": "a.png"}},
                {"name": "gecko_left_0",     "slot": 2, "color": 5,  "src": {"file": "b.png"}},
            ],
        },
    )
    cfg = load_config(cfg_path)
    text = build_inc(cfg)
    lines = text.splitlines()
    assert any(line.startswith("// AUTO-GENERATED") for line in lines)
    assert "@define KUNO_WALK_LEFT_0" in text
    assert "@define GECKO_LEFT_0" in text
    assert "@define KUNO_WALK_LEFT_0  200" in text
    assert "@define GECKO_LEFT_0      202" in text


def test_build_inc_skips_unused_slots(tmp_path):
    """Empty slots produce no @define."""
    cfg_path = _write_config(
        tmp_path,
        {
            "total_slots": 5,
            "phases": [
                {"name": "a", "slot": 0, "color": 14, "src": {"file": "a.png"}},
            ],
        },
    )
    cfg = load_config(cfg_path)
    text = build_inc(cfg)
    assert text.count("@define") == 1
