/*
  02 — Get MAC Address
  --------------------
  Reads and prints the unique MAC address of this ESP32.
  
  Every chip has a factory-burned MAC — useful for:
    - Identifying devices in a network
    - Whitelisting devices on a router
    - Unique device IDs without extra hardware

  Works on: ESP32 DevKit, NodeMCU ESP32-S
  Watch    : Open Serial Monitor at 115200 baud.
             You'll see the MAC printed once on boot.
*/

#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(500);  // Let Serial settle

  // WiFi.begin() is NOT needed just to read MAC
  // But WiFi must be initialized
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  String mac = WiFi.macAddress();

  Serial.println("===========================");
  Serial.println("  ESP32 MAC Address Finder ");
  Serial.println("===========================");
  Serial.print("MAC Address : ");
  Serial.println(mac);
  Serial.println();

  // Also print each byte individually (useful for some APIs)
  uint8_t raw[6];
  esp_read_mac(raw, ESP_MAC_WIFI_STA);
  Serial.print("Raw bytes   : ");
  for (int i = 0; i < 6; i++) {
    if (raw[i] < 0x10) Serial.print("0");
    Serial.print(raw[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
  Serial.println("===========================");
}

void loop() {
  // Nothing — MAC doesn't change, no need to repeat
}
