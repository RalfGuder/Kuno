"""Convert kuno-sprites.png into C64 hardware sprite data."""
from __future__ import annotations

from PIL import Image


def pack_phase(image: Image.Image, threshold: int) -> bytes:
    """Pack a 24x21 RGBA image into 64 bytes of C64 hires sprite data.

    Layout: each row uses 3 bytes (24 bits), MSB-first. Row y starts at
    byte y*3. The 64th byte is slot padding (always 0). A pixel is
    foreground when alpha > 0 AND (R+G+B)/3 < threshold.
    """
    if image.size != (24, 21):
        raise ValueError(f"expected 24x21 image, got {image.size}")
    out = bytearray(64)
    for y in range(21):
        for x in range(24):
            r, g, b, a = image.getpixel((x, y))
            if a > 0 and (r + g + b) // 3 < threshold:
                byte_idx = y * 3 + x // 8
                bit_idx = 7 - (x % 8)
                out[byte_idx] |= 1 << bit_idx
    return bytes(out)


def main() -> None:
    raise NotImplementedError("Task 7 wires this up.")


if __name__ == "__main__":
    main()
