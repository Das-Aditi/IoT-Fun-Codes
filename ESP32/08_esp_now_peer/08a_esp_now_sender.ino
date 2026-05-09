/*
  08 — ESP-NOW Peer-to-Peer  |  SENDER (run on ESP32 DevKit)
  ------------------------------------------------------------
  ESP-NOW is Espressif's proprietary wireless protocol.
  No router, no internet, no Wi-Fi handshake overhead.
  Two ESP32s talk directly to each other at ~1km range (open air).

  This sketch (SENDER) sends a small data packet every 2 seconds.
  The RECEIVER sketch (08b) runs on your NodeMCU ESP32-S.

  What it sends: { counter, temperature_stub, a message string }
  In real use: replace temp_stub with actual sensor reading.

  How to set up:
  ──────────────
  Step 1: Flash 08b_esp_now_receiver.ino onto NodeMCU ESP32-S
  Step 2: Open Serial Monitor on the receiver → copy its MAC address
  Step 3: Paste that MAC into RECEIVER_MAC below
  Step 4: Flash THIS sketch onto ESP32 DevKit
  Step 5: Open Serial Monitors on both → watch packets fly

  No Wi-Fi credentials needed. No router. Works indoors through walls.

  ESP-NOW facts:
  ──────────────
  - Max payload : 250 bytes per packet
  - Latency     : ~1ms (far faster than MQTT/HTTP)
  - Range       : ~200m indoors, ~1km open air with PCB antenna
  - Encryption  : Optional (PMK + LMK)
  - Can broadcast to 20 peers simultaneously
  - Works alongside Wi-Fi (use WIFI_AP_STA mode if combining)
*/

#include <WiFi.h>
#include <esp_now.h>

// ── Paste the RECEIVER's MAC address here ─────────────────────
uint8_t RECEIVER_MAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
// Example: {0x24, 0x6F, 0x28, 0xAB, 0xCD, 0xEF}
// ──────────────────────────────────────────────────────────────

// Data structure — must be identical on sender and receiver
typedef struct {
  uint32_t counter;
  float    temperature;
  char     message[32];
} SensorPacket;

SensorPacket outgoing;

void onSent(const uint8_t* mac, esp_now_send_status_t status) {
  Serial.print("Send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  Serial.print("Sender MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERROR] ESP-NOW init failed");
    return;
  }

  esp_now_register_send_cb(onSent);

  // Register the receiver as a peer
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, RECEIVER_MAC, 6);
  peer.channel = 0;
  peer.encrypt = false;

  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("[ERROR] Failed to add peer");
    return;
  }

  Serial.println("ESP-NOW Sender ready. Sending every 2s...");
}

void loop() {
  outgoing.counter++;
  outgoing.temperature = 24.5 + (float)(outgoing.counter % 10) * 0.1;
  snprintf(outgoing.message, sizeof(outgoing.message), "Hello from DevKit #%lu", outgoing.counter);

  esp_err_t result = esp_now_send(RECEIVER_MAC, (uint8_t*)&outgoing, sizeof(outgoing));
  if (result != ESP_OK) {
    Serial.println("[ERROR] Send failed");
  } else {
    Serial.printf("[SENT] Counter: %lu  Temp: %.1f  Msg: %s\n",
                  outgoing.counter, outgoing.temperature, outgoing.message);
  }

  delay(2000);
}
