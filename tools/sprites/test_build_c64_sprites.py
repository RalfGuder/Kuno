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
