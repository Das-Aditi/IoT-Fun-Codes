"""
OLED Display Test Suite - MicroPython (ESP32 / ESP8266)
Target: SSD1306 128x64 via I2C

Built-in driver: machine.I2C + ssd1306 module (included in MicroPython firmware)
No extra libraries required for most ESP boards.

Flash via: mpremote cp oled_test_micropython.py :main.py

Hardware Connections:
    VCC -> 3.3V
    GND -> GND
    SDA -> GPIO 21 (ESP32) / GPIO 4 (ESP8266)
    SCL -> GPIO 22 (ESP32) / GPIO 5 (ESP8266)
"""

import machine
import ssd1306
import time
import math
import framebuf

# ─── Config ───────────────────────────────────────────────────────────────────
I2C_SDA = 21   # Change for ESP8266: 4
I2C_SCL = 22   # Change for ESP8266: 5
I2C_FREQ = 400_000
OLED_ADDR = 0x3C
WIDTH  = 128
HEIGHT = 64
TEST_DELAY_MS = 2500

# ─── Init ─────────────────────────────────────────────────────────────────────
i2c = machine.I2C(0, sda=machine.Pin(I2C_SDA), scl=machine.Pin(I2C_SCL), freq=I2C_FREQ)
oled = ssd1306.SSD1306_I2C(WIDTH, HEIGHT, i2c, addr=OLED_ADDR)


def show():
    oled.show()

def clear():
    oled.fill(0)


# ─── Tests ────────────────────────────────────────────────────────────────────

def test_white_fill():
    """All pixels white."""
    oled.fill(1)
    show()

def test_black_fill():
    """All pixels off."""
    clear()
    oled.text("Black (OFF)", 10, 28, 1)
    show()

def test_checkerboard():
    """Checkerboard pattern."""
    clear()
    for y in range(HEIGHT):
        for x in range(WIDTH):
            oled.pixel(x, y, (x + y) % 2)
    show()

def test_text():
    """Text at different lines."""
    clear()
    oled.text("OLED Text Test", 0, 0, 1)
    oled.text("0123456789", 0, 12, 1)
    oled.text("ABCDEFGHIJKLMNOP", 0, 24, 1)
    oled.text("abcdefghijklmnop", 0, 36, 1)
    oled.text("!@#$%^&*()-=+[]{}", 0, 48, 1)
    show()

def test_lines():
    """Diagonal + cross-hair lines."""
    clear()
    oled.line(0, 0, WIDTH-1, HEIGHT-1, 1)
    oled.line(WIDTH-1, 0, 0, HEIGHT-1, 1)
    oled.hline(0, HEIGHT//2, WIDTH, 1)
    oled.vline(WIDTH//2, 0, HEIGHT, 1)
    show()

def test_rectangles():
    """Outline and filled rects."""
    clear()
    oled.rect(0, 0, WIDTH, HEIGHT, 1)
    oled.rect(10, 10, WIDTH-20, HEIGHT-20, 1)
    oled.fill_rect(30, 22, 68, 20, 1)
    show()

def test_circles():
    """Draw circles using trigonometry (no built-in circle in ssd1306)."""
    clear()
    def draw_circle(cx, cy, r, c=1):
        steps = max(16, r * 4)
        for i in range(steps):
            a = 2 * math.pi * i / steps
            x = int(cx + r * math.cos(a))
            y = int(cy + r * math.sin(a))
            if 0 <= x < WIDTH and 0 <= y < HEIGHT:
                oled.pixel(x, y, c)

    draw_circle(32, 32, 28)
    draw_circle(96, 32, 28)
    draw_circle(64, 32, 14)
    show()

def test_progress_bar():
    """Animated progress bar (3 frames)."""
    for pct in [33, 66, 100]:
        clear()
        oled.text("Loading...", 30, 5, 1)
        oled.rect(4, 25, 120, 14, 1)
        fill_w = int(pct / 100 * 116)
        oled.fill_rect(5, 26, fill_w, 12, 1)
        oled.text(f"{pct}%", 56, 45, 1)
        show()
        time.sleep_ms(700)

def test_sine_wave():
    """Sine wave across the full width."""
    clear()
    prev_y = HEIGHT // 2
    for x in range(WIDTH):
        angle = (x / WIDTH) * 4 * math.pi
        y = int(HEIGHT // 2 + (HEIGHT // 2 - 4) * math.sin(angle))
        oled.line(x-1, prev_y, x, y, 1)
        prev_y = y
    show()

def test_pixel_noise():
    """Pseudorandom pixels using LCG."""
    clear()
    seed = 12345
    for _ in range(400):
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF
        x = seed % WIDTH
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF
        y = seed % (HEIGHT - 10)
        oled.pixel(x, y, 1)
    oled.text("Noise Test", 28, 55, 1)
    show()

def test_grid():
    """8-pixel grid lines."""
    clear()
    for x in range(0, WIDTH, 8):
        oled.vline(x, 0, HEIGHT, 1)
    for y in range(0, HEIGHT, 8):
        oled.hline(0, y, WIDTH, 1)
    show()

def test_invert():
    """Invert display (hardware feature) — tests display controller."""
    oled.fill(0)
    oled.text("INVERT TEST", 18, 28, 1)
    show()
    time.sleep_ms(1000)
    oled.invert(True)
    time.sleep_ms(1000)
    oled.invert(False)

def test_contrast():
    """Sweep contrast from 0 to 255."""
    oled.fill(1)
    oled.text("Contrast Sweep", 8, 28, 0)
    show()
    for level in range(0, 256, 16):
        oled.contrast(level)
        time.sleep_ms(80)
    oled.contrast(200)  # Restore reasonable default

# ─── Suite ────────────────────────────────────────────────────────────────────
TESTS = [
    ("White Fill",    test_white_fill),
    ("Black Fill",    test_black_fill),
    ("Checkerboard",  test_checkerboard),
    ("Text",          test_text),
    ("Lines",         test_lines),
    ("Rectangles",    test_rectangles),
    ("Circles",       test_circles),
    ("Progress Bar",  test_progress_bar),
    ("Sine Wave",     test_sine_wave),
    ("Pixel Noise",   test_pixel_noise),
    ("Grid",          test_grid),
    ("Invert",        test_invert),
    ("Contrast Sweep",test_contrast),
]

def run_suite():
    print("OLED Test Suite — MicroPython")
    for name, fn in TESTS:
        print(f"  Running: {name}")
        clear()
        oled.text(name, 0, 28, 1)
        show()
        time.sleep_ms(500)
        fn()
        time.sleep_ms(TEST_DELAY_MS)

    clear()
    oled.text("All Tests Done!", 10, 28, 1)
    show()
    print("Done.")

run_suite()
