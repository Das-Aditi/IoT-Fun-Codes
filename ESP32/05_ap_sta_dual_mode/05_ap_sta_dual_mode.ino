/*
  05 — AP + STA Dual Mode
  -------------------------
  ESP32 connects to your router (STA) AND hosts its own AP simultaneously.

  Why useful?
    - Act as a Wi-Fi bridge or repeater
    - Host a local config dashboard while staying online
    - Receive commands locally AND push data to cloud

  Works on: ESP32 DevKit, NodeMCU ESP32-S
  Watch    : Serial Monitor at 115200.
             ESP32 gets an IP from your router (STA IP).
             It also creates "ESP32-Dual-AP" (AP IP: 192.168.4.1).

  Note: Both radios share one antenna — performance is reduced compared to single mode.
        Best practice: use different channels if possible (router auto-selects).
*/

#include <WiFi.h>

// ── STA credentials (your router) ─────────────────────────────
const char* STA_SSID     = "YOUR_WIFI_NAME";
const char* STA_PASSWORD = "YOUR_WIFI_PASSWORD";
// ── AP credentials (ESP32's own network) ──────────────────────
const char* AP_SSID      = "ESP32-Dual-AP";
const char* AP_PASSWORD  = "dualmode1";   // Min 8 chars
// ──────────────────────────────────────────────────────────────

#define CONNECT_TIMEOUT_SEC 15

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n05 — AP + STA Dual Mode");

  // WIFI_AP_STA enables both modes simultaneously
  WiFi.mode(WIFI_AP_STA);

  // Start AP first (instant)
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.println("[OK] AP started");
  Serial.print("  AP SSID  : "); Serial.println(AP_SSID);
  Serial.print("  AP IP    : "); Serial.println(WiFi.softAPIP());

  // Now connect to router
  Serial.print("\nConnecting STA to: ");
  Serial.println(STA_SSID);
  WiFi.begin(STA_SSID, STA_PASSWORD);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if ((millis() - start) > (CONNECT_TIMEOUT_SEC * 1000UL)) {
      Serial.println("\n[WARN] STA connection timed out. AP still running.");
      return;
    }
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n[OK] STA connected!");
  Serial.println("──────────────────────────────────");
  Serial.print("  STA IP   : "); Serial.println(WiFi.localIP());
  Serial.print("  RSSI     : "); Serial.print(WiFi.RSSI()); Serial.println(" dBm");
  Serial.println("──────────────────────────────────");
  Serial.println("Both AP and STA are running.");
  Serial.println("Phone connects to AP → ESP32 → Router → Internet");
}

void loop() {
  static unsigned long lastPrint = 0;

  if (millis() - lastPrint > 5000) {
    lastPrint = millis();

    Serial.println("\n[STATUS]");
    Serial.print("  STA     : ");
    Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
    Serial.print("  STA IP  : "); Serial.println(WiFi.localIP());
    Serial.print("  AP clients: "); Serial.println(WiFi.softAPgetStationNum());
  }

  // Reconnect STA if dropped (AP stays up regardless)
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WARN] STA lost. Reconnecting...");
    WiFi.reconnect();
    delay(5000);
  }
}
