/*
  08b — ESP-NOW Peer-to-Peer  |  RECEIVER (run on NodeMCU ESP32-S)
  ------------------------------------------------------------------
  Flash this on the NodeMCU ESP32-S FIRST.
  Open Serial Monitor → copy the MAC address it prints.
  Paste that MAC into the SENDER sketch (08a) before flashing the DevKit.

  On receiving a packet: prints all fields to Serial.
  LED on GPIO 2 blinks once per received packet (visual confirm).

  The data struct MUST match the sender exactly — same field names,
  same types, same order. If they differ, you'll get garbage data.
*/

#include <WiFi.h>
#include <esp_now.h>

#define LED_PIN 2

// Must be identical to the struct in the sender
typedef struct {
  uint32_t counter;
  float    temperature;
  char     message[32];
} SensorPacket;

SensorPacket incoming;

void onReceive(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
  memcpy(&incoming, data, sizeof(incoming));

  // Blink LED to confirm packet received
  digitalWrite(LED_PIN, HIGH);
  delay(50);
  digitalWrite(LED_PIN, LOW);

  // Print sender MAC
  char senderMac[18];
  snprintf(senderMac, sizeof(senderMac), "%02X:%02X:%02X:%02X:%02X:%02X",
           info->src_addr[0], info->src_addr[1], info->src_addr[2],
           info->src_addr[3], info->src_addr[4], info->src_addr[5]);

  Serial.println("──────────────────────────────────");
  Serial.print("From        : "); Serial.println(senderMac);
  Serial.print("Counter     : "); Serial.println(incoming.counter);
  Serial.print("Temperature : "); Serial.print(incoming.temperature, 1); Serial.println(" °C");
  Serial.print("Message     : "); Serial.println(incoming.message);
  Serial.print("Packet size : "); Serial.print(len); Serial.println(" bytes");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  Serial.println("ESP-NOW Receiver");
  Serial.println("──────────────────────────────────");
  Serial.print("My MAC (give this to sender): ");
  Serial.println(WiFi.macAddress());
  Serial.println("Waiting for packets...");

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERROR] ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(onReceive);
}

void loop() {
  // Nothing needed — onReceive fires automatically on incoming packets
}
