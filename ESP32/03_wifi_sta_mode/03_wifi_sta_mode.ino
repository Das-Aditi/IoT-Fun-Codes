/*
  03 — Wi-Fi STA Mode (Station)
  ------------------------------
  ESP32 connects to your existing Wi-Fi router, just like a phone.
  Once connected it prints:
    - Connection status
    - Assigned local IP address
    - Signal strength (RSSI in dBm)

  Works on: ESP32 DevKit, NodeMCU ESP32-S
  Watch    : Serial Monitor at 115200. Dots appear while connecting.
             On success you'll see the IP — use it to reach the ESP32 on your LAN.

  Next step: combine with a web server (WebServer.h) to serve a webpage at that IP.
*/

#include <WiFi.h>

// ── Change these to your network credentials ──────────────────
const char* SSID     = "YOUR_WIFI_NAME";
const char* PASSWORD = "YOUR_WIFI_PASSWORD";
// ──────────────────────────────────────────────────────────────

#define CONNECT_TIMEOUT_SEC  15

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n03 — Wi-Fi STA Mode");
  Serial.print("Connecting to: ");
  Serial.println(SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if ((millis() - start) > (CONNECT_TIMEOUT_SEC * 1000UL)) {
      Serial.println("\n[ERROR] Connection timed out. Check SSID/Password.");
      return;
    }
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n[OK] Connected!");
  Serial.println("─────────────────────────────");
  Serial.print("  IP Address  : ");
  Serial.println(WiFi.localIP());
  Serial.print("  Subnet Mask : ");
  Serial.println(WiFi.subnetMask());
  Serial.print("  Gateway     : ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("  Signal RSSI : ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  Serial.println("─────────────────────────────");
}

void loop() {
  // Reconnect if dropped
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WARN] Wi-Fi lost. Reconnecting...");
    WiFi.reconnect();
    delay(5000);
  }
}
