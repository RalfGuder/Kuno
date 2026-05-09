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
    # ensure source image exists for happy-path tests
    src = tmp_path / "kuno-sprites.png"
    if not src.exists():
        Image.new("RGBA", (640, 63), (255, 255, 255, 0)).save(src)
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
