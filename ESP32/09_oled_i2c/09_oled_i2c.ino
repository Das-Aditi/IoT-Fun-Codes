/*
  09 — OLED Display via I2C (SSD1306, 128x64)
  ---------------------------------------------
  Drives a 0.96" OLED display. Shows:
    - Device IP address
    - Wi-Fi signal strength
    - Uptime counter
    - A simple animation (scrolling bar)

  The SSD1306 is the most common OLED controller — used in nearly every
  cheap 0.96" and 1.3" display you'll find online.

  Wiring (I2C — only 4 wires):
  ──────────────────────────────
  OLED VCC → 3.3V
  OLED GND → GND
  OLED SCL → GPIO 22
  OLED SDA → GPIO 21

  Default I2C address is 0x3C (some displays use 0x3D — check with I2C scanner)

  Libraries needed (install via Library Manager):
  ─────────────────────────────────────────────────
  1. "Adafruit SSD1306"
  2. "Adafruit GFX Library"  (auto-installed as dependency)

  Works on : ESP32 DevKit, NodeMCU ESP32-S
  Watch    : OLED screen directly. Serial at 115200 for debug.

  AP Mode — Accessing ESP32 Settings Page via OLED:
  ──────────────────────────────────────────────────
  In AP mode, ESP32 has no IP from a router. The OLED becomes critical —
  it displays 192.168.4.1 so users know where to go in their browser.
  Typical captive portal flow:
    1. ESP32 boots in AP mode, OLED shows "Connect to: ESP32-Config"
    2. OLED shows IP: 192.168.4.1
    3. User connects phone → opens 192.168.4.1 → sees settings web page
    4. User enters their Wi-Fi credentials → ESP32 switches to STA mode
    5. OLED updates to show new STA IP from router
  This pattern (AP for config, STA for operation) is how commercial
  IoT products like Sonoff, Tasmota, and Shelly work at first boot.
*/

#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1       // No reset pin on most modules
#define OLED_ADDRESS  0x3C

const char* SSID     = "YOUR_WIFI_NAME";
const char* PASSWORD = "YOUR_WIFI_PASSWORD";

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void drawStatus() {
  display.clearDisplay();

  // Line 1 — title
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("ESP32 Lab Monitor");

  // Divider
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  // Line 2 — IP
  display.setCursor(0, 14);
  display.print("IP: ");
  display.println(WiFi.localIP().toString());

  // Line 3 — RSSI
  display.setCursor(0, 24);
  display.print("Signal: ");
  display.print(WiFi.RSSI());
  display.println(" dBm");

  // Line 4 — Uptime
  unsigned long secs = millis() / 1000;
  display.setCursor(0, 34);
  display.printf("Up: %02lu:%02lu:%02lu", secs/3600, (secs%3600)/60, secs%60);

  // Line 5 — Progress bar (animated)
  int barWidth = (int)((millis() / 100) % 128);
  display.drawRect(0, 54, 128, 8, SSD1306_WHITE);
  display.fillRect(0, 54, barWidth, 8, SSD1306_WHITE);

  display.display();
}

void showBootScreen(String line1, String line2 = "") {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println(line1);
  if (line2.length()) {
    display.setCursor(0, 35);
    display.println(line2);
  }
  display.display();
}

void setup() {
  Serial.begin(115200);

  // Init OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("[ERROR] OLED not found. Check wiring and I2C address.");
    while (true);
  }

  display.clearDisplay();
  display.display();

  // Boot splash
  showBootScreen("ESP32 Lab", "Starting...");
  delay(1000);

  // Connect Wi-Fi
  showBootScreen("Connecting WiFi...", SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > 15000) {
      showBootScreen("WiFi failed.", "Check creds.");
      Serial.println("[ERROR] WiFi timeout");
      return;
    }
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n[OK] Connected: " + WiFi.localIP().toString());
  showBootScreen("Connected!", WiFi.localIP().toString());
  delay(1500);
}

void loop() {
  drawStatus();
  delay(500);
}
