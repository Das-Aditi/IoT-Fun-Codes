# ESP32 Lab — Beginner to Real-World
## What is the ESP32?

A low-cost, low-power microcontroller with **built-in Wi-Fi + Bluetooth**. Think of it as a tiny computer that can sense, compute, and talk wirelessly — all on a chip the size of a postage stamp.

**Boards in this lab:**
| Board | Notes |
|---|---|
| ESP32 (DevKit) | 38 pins, more GPIOs, dual-core 240MHz |
| NodeMCU ESP32-S | Narrower, breadboard-friendly, same chip |

---
![ESP32 Pin Layout](https://esp32io.com/images/tutorial/ESP-WROOM-32-Dev-Module-pinout.jpg)

## Pin Types — Quick Reference

| Type | Pins | Use |
|---|---|---|
| **Digital I/O** | Most GPIOs | LED, button, relay, sensor ON/OFF |
| **Analog Input (ADC)** | GPIO 32–39 | Read voltage (sensors, potentiometers) |
| **PWM** | Almost any GPIO | Dim LEDs, control servo motors |
| **DAC (analog out)** | GPIO 25, 26 | True analog voltage output |
| **I2C** | GPIO 21 (SDA), 22 (SCL) | OLED displays, sensors (BMP280, MPU6050) |
| **SPI** | GPIO 18/19/23/5 | SD cards, TFT displays |
| **UART** | GPIO 1 (TX), 3 (RX) | Serial comms, GPS, Bluetooth serial |
| **Touch** | GPIO 4,13,14,15,27,32,33 | Capacitive touch buttons |
| **Strapping pins** | GPIO 0, 2, 12, 15 | Affect boot — avoid for inputs at startup |

> ⚠️ GPIO 34–39 are **input-only** (no internal pull-up). GPIO 6–11 are tied to onboard flash — do NOT use.

---

## Operating Modes

### STA Mode (Station)
ESP32 **connects to your home/office router** like any phone or laptop.  
Use case: Send sensor data to internet, fetch time from NTP, control via cloud.

### AP Mode (Access Point)
ESP32 **becomes the router** — creates its own Wi-Fi network.  
Use case: Local config pages, device-to-device comms without internet.

### AP+STA Dual Mode
Both at once — ESP32 connects to internet AND hosts its own AP.  
Use case: A bridge device, or a local web dashboard while staying cloud-connected.

---

## What Can You Build?

| Domain | Example |
|---|---|
| Home Automation | Smart switch, motion-triggered lights |
| Environmental | Temperature/humidity logger to Google Sheets |
| Security | Door sensor with Telegram alert |
| Display | OLED clock, weather station |
| Audio | Web-controlled buzzer, I2S speaker |
| Motor Control | RC car, servo arm via web page |
| Data Logging | SD card + RTC timestamped readings |
| Mesh Networks | ESP-NOW multi-node sensor grids |

---

## Lab Progression

```
01_blink_led          → GPIO basics, digital write
02_get_mac            → Device identity, Serial output
03_wifi_sta_mode      → Connect to router, get IP
04_wifi_ap_mode       → Host your own network
05_ap_sta_dual_mode   → Both simultaneously
... (coming next: web server, sensors, MQTT, BLE)
```

---

## Language & Toolchain Options

### What language is used here?
All sketches in this lab are written in **C++**, specifically the Arduino-flavoured subset of it. The `.ino` files are C++ under the hood — `setup()` and `loop()` are just Arduino's entry point conventions. The Arduino framework wraps Espressif's lower-level ESP-IDF so you don't have to deal with hardware registers directly.

---

### What else can you use?

| Language / Framework | Tool | Good for | Tradeoff |
|---|---|---|---|
| **C++ (Arduino)** | Arduino IDE, PlatformIO | Beginners, huge library ecosystem | Abstracted, less control |
| **C / C++ (ESP-IDF)** | VS Code + Espressif IDF plugin | Full hardware control, production firmware | Steeper learning curve, verbose |
| **MicroPython** | Thonny IDE, `mpremote` | Rapid prototyping, Python familiarity | Slower, less RAM, smaller library base |
| **CircuitPython** | Mu Editor | Educational, drag-and-drop file system | Limited ESP32 support vs MicroPython |
| **Lua** | NodeMCU firmware | Scripting, event-driven style | Mostly legacy now, less maintained |
| **JavaScript** | Moddable SDK, Espruino | JS familiarity | Niche, limited community |
| **Rust** | `esp-hal`, `esp-idf-hal` crates | Memory safety, modern tooling | Experimental, complex setup |

**Recommendation by use case:**
- Just starting out → **Arduino (C++)** — this lab
- Want more Python → **MicroPython** — great second step, most concepts transfer
- Building a real product → **ESP-IDF (C/C++)** — what Espressif themselves use
- Interested in modern systems programming → **Rust** — rapidly maturing for ESP32

---

## Setup

1. Install [Arduino IDE](https://www.arduino.cc/en/software) or use VS Code + PlatformIO
2. Add ESP32 board package:  
   `File → Preferences → Additional Boards URLs:`  
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. `Tools → Board → ESP32 Arduino → ESP32 Dev Module` (or NodeMCU-32S)
4. Select correct COM port, set Upload Speed to `115200`

---

*Each sketch folder has its own `.ino` file and a short comment block explaining what it does and what to watch on Serial Monitor.*
