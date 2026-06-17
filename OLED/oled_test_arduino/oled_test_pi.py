#!/usr/bin/env python3
"""
OLED Display Test Suite - Raspberry Pi
Target: SSD1306 128x64 via I2C (luma.oled library)

Install dependencies:
    pip install luma.oled Pillow

Hardware connections (I2C):
    VCC -> Pin 1 (3.3V)
    GND -> Pin 6 (GND)
    SDA -> Pin 3 (GPIO2)
    SCL -> Pin 5 (GPIO3)

Enable I2C: sudo raspi-config -> Interface Options -> I2C -> Enable
Verify device: i2cdetect -y 1  (should show 0x3c)
"""

import time
import math
import random
from PIL import Image, ImageDraw, ImageFont

try:
    from luma.core.interface.serial import i2c
    from luma.oled.device import ssd1306
    HARDWARE = True
except ImportError:
    print("[INFO] luma.oled not found — running in SIMULATION mode (saving PNGs)")
    HARDWARE = False


# ─── Constants ────────────────────────────────────────────────────────────────
WIDTH  = 128
HEIGHT = 64
DELAY  = 2.5   # seconds per test


# ─── Device Init ──────────────────────────────────────────────────────────────
def init_device():
    if HARDWARE:
        serial = i2c(port=1, address=0x3C)
        return ssd1306(serial, width=WIDTH, height=HEIGHT)
    return None


def display_image(device, img, test_name, sim_index):
    """Show image on OLED or save as PNG in simulation mode."""
    if HARDWARE:
        device.display(img.convert(device.mode))
    else:
        fname = f"sim_{sim_index:02d}_{test_name}.png"
        img.save(fname)
        print(f"  [SIM] Saved: {fname}")


# ─── Helper: blank canvas ─────────────────────────────────────────────────────
def blank():
    return Image.new("1", (WIDTH, HEIGHT), 0), None  # will set draw below

def canvas():
    img = Image.new("1", (WIDTH, HEIGHT), 0)
    draw = ImageDraw.Draw(img)
    return img, draw

def default_font():
    try:
        return ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 10)
    except Exception:
        return ImageFont.load_default()


# ─── Test Functions ───────────────────────────────────────────────────────────

def test_white_fill():
    """All pixels ON — burn-in / brightness check."""
    img = Image.new("1", (WIDTH, HEIGHT), 1)
    return img, "white_fill"


def test_black_fill():
    """All pixels OFF — dead-pixel check."""
    img, draw = canvas()
    font = default_font()
    draw.text((10, 27), "Black Fill (OFF)", font=font, fill=1)
    return img, "black_fill"


def test_checkerboard():
    """Classic checkerboard — pixel-level accuracy test."""
    img = Image.new("1", (WIDTH, HEIGHT), 0)
    pixels = img.load()
    for y in range(HEIGHT):
        for x in range(WIDTH):
            pixels[x, y] = 1 if (x + y) % 2 == 0 else 0
    return img, "checkerboard"


def test_text_rendering():
    """Multiple font sizes and character sets."""
    img, draw = canvas()
    font_sm = default_font()
    try:
        font_lg = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 16)
    except Exception:
        font_lg = font_sm

    draw.text((0, 0),  "Small: ABCabc123!@#", font=font_sm, fill=1)
    draw.text((0, 14), "BOLD TEXT", font=font_lg, fill=1)
    draw.text((0, 34), "0123456789", font=font_sm, fill=1)
    draw.text((0, 46), "The quick brown fox", font=font_sm, fill=1)
    draw.text((0, 56), "jumps over lazy dog", font=font_sm, fill=1)
    return img, "text_rendering"


def test_lines():
    """Diagonal and cross-hair lines."""
    img, draw = canvas()
    draw.line([(0, 0), (WIDTH-1, HEIGHT-1)], fill=1)
    draw.line([(WIDTH-1, 0), (0, HEIGHT-1)], fill=1)
    draw.line([(0, HEIGHT//2), (WIDTH-1, HEIGHT//2)], fill=1)
    draw.line([(WIDTH//2, 0), (WIDTH//2, HEIGHT-1)], fill=1)
    return img, "lines"


def test_rectangles():
    """Outline and filled rectangles."""
    img, draw = canvas()
    draw.rectangle([(0, 0), (WIDTH-1, HEIGHT-1)], outline=1)
    draw.rectangle([(8, 8), (WIDTH-9, HEIGHT-9)], outline=1)
    draw.rectangle([(20, 20), (WIDTH-21, HEIGHT-21)], fill=1)
    return img, "rectangles"


def test_circles():
    """Circles at various positions."""
    img, draw = canvas()
    cx, cy = WIDTH // 2, HEIGHT // 2
    for r in [5, 15, 25]:
        draw.ellipse([(cx-r, cy-r), (cx+r, cy+r)], outline=1)
    draw.ellipse([(4, 4), (40, 40)], outline=1)
    draw.ellipse([(WIDTH-41, HEIGHT-41), (WIDTH-5, HEIGHT-5)], outline=1)
    return img, "circles"


def test_progress_bar(pct=75):
    """Progress bar widget — useful for real UI testing."""
    img, draw = canvas()
    font = default_font()
    draw.text((35, 2), "Loading...", font=font, fill=1)
    # Outer frame
    draw.rectangle([(4, 25), (123, 38)], outline=1)
    # Fill bar
    fill_w = int((pct / 100) * 117)
    draw.rectangle([(5, 26), (5 + fill_w, 37)], fill=1)
    # Label
    draw.text((50, 44), f"{pct}%", font=font, fill=1)
    return img, "progress_bar"


def test_sine_wave():
    """Sine wave — good for verifying pixel density."""
    img, draw = canvas()
    prev = None
    for x in range(WIDTH):
        angle = (x / WIDTH) * 2 * math.pi * 2
        y = int((HEIGHT // 2) + (HEIGHT // 2 - 4) * math.sin(angle))
        if prev:
            draw.line([prev, (x, y)], fill=1)
        prev = (x, y)
    return img, "sine_wave"


def test_pixel_noise():
    """Random pixels — stress tests individual pixel addressing."""
    img, draw = canvas()
    font = default_font()
    for _ in range(400):
        x = random.randint(0, WIDTH - 1)
        y = random.randint(0, HEIGHT - 12)
        draw.point((x, y), fill=1)
    draw.text((20, HEIGHT - 10), "Pixel Noise Test", font=font, fill=1)
    return img, "pixel_noise"


def test_grid():
    """Regular grid — tests uniform spacing and pixel stepping."""
    img, draw = canvas()
    for x in range(0, WIDTH, 8):
        draw.line([(x, 0), (x, HEIGHT-1)], fill=1)
    for y in range(0, HEIGHT, 8):
        draw.line([(0, y), (WIDTH-1, y)], fill=1)
    return img, "grid"


# ─── Test Suite Runner ────────────────────────────────────────────────────────
TESTS = [
    test_white_fill,
    test_black_fill,
    test_checkerboard,
    test_text_rendering,
    test_lines,
    test_rectangles,
    test_circles,
    test_progress_bar,
    test_sine_wave,
    test_pixel_noise,
    test_grid,
]


def run_suite(loops=1):
    device = init_device()
    mode = "HARDWARE" if HARDWARE else "SIMULATION"
    print(f"\n{'='*50}")
    print(f"  OLED Test Suite  |  Mode: {mode}")
    print(f"  {len(TESTS)} tests  |  {DELAY}s each  |  {loops} loop(s)")
    print(f"{'='*50}\n")

    for loop in range(loops):
        print(f"Loop {loop + 1}/{loops}")
        for i, test_fn in enumerate(TESTS):
            name = test_fn.__name__.replace("test_", "")
            print(f"  [{i+1:02d}/{len(TESTS)}] {name}...", end=" ")
            try:
                img, label = test_fn()
                display_image(device, img, label, i)
                print("OK")
            except Exception as e:
                print(f"FAIL — {e}")
            time.sleep(DELAY)

    print("\nAll tests complete.")
    if HARDWARE:
        device.cleanup()


if __name__ == "__main__":
    run_suite(loops=1)
