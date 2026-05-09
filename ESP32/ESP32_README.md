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

### Accessing ESP32 in AP Mode

When running in AP mode, the ESP32's IP is always fixed at `192.168.4.1` — no router involved.

1. Flash any AP-mode sketch
2. On your phone or laptop, connect to the Wi-Fi network named in `AP_SSID`
3. Open a browser → go to `http://192.168.4.1`
4. You'll see the ESP32's page — no internet required

This is the same pattern commercial IoT devices use on first boot (Sonoff, Shelly, Tasmota, Wiz bulbs):

```
Device boots → no Wi-Fi saved → starts in AP mode
User connects phone → opens 192.168.4.1 → enters home Wi-Fi credentials
Device restarts in STA mode → normal operation
```

### AP+STA Dual Mode
Both simultaneously — ESP32 connects to internet AND hosts its own AP.
Use case: A bridge device, or a local dashboard while staying cloud-connected.

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

## Sketch Index

| # | Folder | What it does | New concept |
|---|---|---|---|
| 01 | `01_blink_led` | Toggle onboard LED on and off | GPIO basics, digital write |
| 02 | `02_get_mac` | Read and print device MAC address | Device identity, Serial output |
| 03 | `03_wifi_sta_mode` | Connect to router, print assigned IP | STA mode, RSSI, reconnect |
| 04 | `04_wifi_ap_mode` | ESP32 hosts its own Wi-Fi network | AP mode, 192.168.4.1 |
| 05 | `05_ap_sta_dual_mode` | AP and STA running simultaneously | WIFI_AP_STA, bridge mode |
| 06 | `06_web_server_led` | Control LED from any browser on the LAN | HTTP web server, HTML over Wi-Fi |
| 07 | `07_dht_sensor` | Live temp & humidity on Serial + browser | External sensor, JSON endpoint |
| 08 | `08_esp_now_peer` | Two boards talk directly, no router | ESP-NOW, struct packets, peer MAC |
| 09 | `09_oled_i2c` | OLED showing IP, signal strength, uptime | I2C, Adafruit GFX, display layers |
| 10 | `10_mqtt_broker` | Publish sensor data, receive LED commands | MQTT, pub/sub, Home Assistant |

---

## Language & Toolchain Options

All sketches are written in **C++** — Arduino-flavoured. The `.ino` files are C++ under the hood; `setup()` and `loop()` are just Arduino's entry point conventions. The Arduino framework wraps Espressif's ESP-IDF so you don't have to deal with hardware registers directly.

### Alternatives

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

## Libraries Needed

Install via **Arduino IDE → Tools → Manage Libraries**:

| Library | Sketch | Search for |
|---|---|---|
| DHT sensor library | 07 | `DHT sensor library` by Adafruit |
| Adafruit Unified Sensor | 07 | `Adafruit Unified Sensor` (dependency) |
| Adafruit SSD1306 | 09 | `Adafruit SSD1306` |
| Adafruit GFX Library | 09 | auto-installed with SSD1306 |
| PubSubClient | 10 | `PubSubClient` by Nick O'Leary |

`WebServer.h` and `esp_now.h` are built into the ESP32 Arduino core — no install needed.

---

## Sketch 08 — ESP-NOW Setup Order

Flash the **receiver first** (08b → NodeMCU ESP32-S):
1. Open Serial Monitor → copy the MAC address it prints
2. Paste that MAC into `RECEIVER_MAC[]` in 08a
3. Flash the **sender** (08a → ESP32 DevKit)
4. Watch both Serial Monitors — packets arrive every 2 seconds

---

## Sketch 10 — Quick MQTT Test

No Home Assistant needed to test. Change `MQTT_BROKER` to `broker.hivemq.com` (free public broker), then install **MQTT Explorer** on your PC — subscribe to `esp32/sensors` to see JSON, publish `ON` or `OFF` to `esp32/led` to control the LED.

---

## Setup

1. Install [Arduino IDE](https://www.arduino.cc/en/software) or VS Code + PlatformIO
2. Add ESP32 board package — `File → Preferences → Additional Boards Manager URLs`:
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. `Tools → Board → ESP32 Arduino → ESP32 Dev Module` (or NodeMCU-32S)
4. Select the correct COM port, upload speed `115200`

---

## Coming in Batch 3

```
11 — WiFiManager      → captive config portal, no hardcoded credentials
12 — NTP time sync    → real clock time without RTC hardware
13 — Google Sheets    → HTTP POST data logging to a spreadsheet
14 — BLE scan         → detect nearby Bluetooth devices
15 — Deep sleep       → wake on timer, battery-powered sensor node
```

---

*Each `.ino` has a comment block at the top with wiring, required libraries, and what to watch on Serial Monitor.*
