/*
  04 — Wi-Fi AP Mode (Access Point)
  -----------------------------------
  ESP32 creates its OWN Wi-Fi network — no router needed.
  Other devices (phone, laptop) can connect directly to it.

  Use cases:
    - Config portal (let user enter their Wi-Fi credentials)
    - Local control without internet
    - Isolated sensor networks

  Works on: ESP32 DevKit, NodeMCU ESP32-S
  Watch    : Serial Monitor at 115200.
             Then scan Wi-Fi on your phone — you'll see "ESP32-Lab-AP".
             Connect to it (password: 12345678) and ping 192.168.4.1.

  Default AP IP is always 192.168.4.1 on ESP32.
*/

#include <WiFi.h>

// ── AP credentials — change freely ───────────────────────────
const char* AP_SSID     = "ESP32-Lab-AP";
const char* AP_PASSWORD = "12345678";     // Min 8 chars, or set "" for open network
// ─────────────────────────────────────────────────────────────

#define AP_CHANNEL    1    // Wi-Fi channel (1, 6, or 11 recommended)
#define MAX_CLIENTS   4    // Max simultaneous connections

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n04 — Wi-Fi AP Mode");

  WiFi.mode(WIFI_AP);
  bool result = WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL, 0, MAX_CLIENTS);

  if (!result) {
    Serial.println("[ERROR] Failed to start AP.");
    return;
  }

  Serial.println("[OK] Access Point started!");
  Serial.println("──────────────────────────────────");
  Serial.print("  SSID        : ");
  Serial.println(AP_SSID);
  Serial.print("  Password    : ");
  Serial.println(AP_PASSWORD);
  Serial.print("  AP IP       : ");
  Serial.println(WiFi.softAPIP());
  Serial.print("  MAC (AP)    : ");
  Serial.println(WiFi.softAPmacAddress());
  Serial.println("──────────────────────────────────");
  Serial.println("Connect your phone to the AP above.");
}

void loop() {
  static uint8_t lastCount = 0;
  uint8_t count = WiFi.softAPgetStationNum();

  if (count != lastCount) {
    lastCount = count;
    Serial.print("[INFO] Connected clients: ");
    Serial.println(count);
  }

  delay(2000);
}
