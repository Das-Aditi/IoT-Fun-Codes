# 🖥️ OLED Test Suite — SSD1306 128×64

A collection of test scripts for the **SSD1306 OLED display** across four embedded platforms.

---

## 📁 Files

| File | Platform | Language |
|---|---|---|
| `oled_test_arduino.ino` | Arduino (Uno / Nano / Mega) | C++ |
| `oled_test_pi.py` | Raspberry Pi | Python 3 |
| `oled_test_micropython.py` | ESP32 / ESP8266 | MicroPython |
| `oled_test_stm32.c` | STM32 (HAL) | C |

---

## 🔌 Wiring (I2C)

| OLED Pin | Arduino | Raspberry Pi | ESP32 | ESP8266 |
|---|---|---|---|---|
| VCC | 3.3V / 5V | Pin 1 (3.3V) | 3.3V | 3.3V |
| GND | GND | Pin 6 | GND | GND |
| SDA | A4 | GPIO 2 (Pin 3) | GPIO 21 | GPIO 4 |
| SCL | A5 | GPIO 3 (Pin 5) | GPIO 22 | GPIO 5 |

> Default I2C address: `0x3C` — try `0x3D` if the display doesn't respond.

---

## 🧪 Test Patterns

Each platform runs the same set of tests:

1. White fill — all pixels ON
2. Black fill — all pixels OFF
3. Checkerboard — pixel-level accuracy
4. Text rendering — font and character set
5. Lines — diagonals and crosshairs
6. Rectangles — outline and filled
7. Circles — trigonometric placement
8. Progress bar — composite UI element
9. Sine wave — floating-point rendering
10. Pixel noise — random pixel addressing
11. Grid — uniform 8px spacing
12. Invert — hardware display invert
13. Contrast sweep — brightness ramp 0→255

---

## ⚙️ Setup

**Arduino**
```
Library Manager → install: Adafruit SSD1306 + Adafruit GFX
```

**Raspberry Pi**
```bash
pip install luma.oled Pillow
sudo raspi-config   # Enable I2C under Interface Options
python3 oled_test_pi.py
```

**MicroPython (ESP32/ESP8266)**
```bash
mpremote cp oled_test_micropython.py :main.py
```

**STM32**  
Add [`stm32-ssd1306`](https://github.com/afiskon/stm32-ssd1306) driver to your project, then call `OLED_RunTestSuite()` from `main()`.

---

## 📄 Docs

See [`OLED_Testing_Guide.pdf`](./OLED_Testing_Guide.pdf) for full wiring diagrams, troubleshooting, and SSD1306 command reference.
