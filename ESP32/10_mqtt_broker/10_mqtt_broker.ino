/*
  10 — MQTT Client (Bridge to Home Assistant / Node-RED)
  --------------------------------------------------------
  MQTT is the standard messaging protocol for IoT.
  Think of it as a group chat: devices publish to "topics",
  other devices subscribe and receive messages instantly.

  This sketch:
    - Connects to an MQTT broker (local or cloud)
    - Publishes temperature + humidity every 10 seconds → esp32/sensors
    - Subscribes to esp32/led → receives "ON"/"OFF" to control LED
    - Reconnects automatically if broker connection drops

  MQTT Broker options:
  ─────────────────────
  LOCAL  : Mosquitto on your PC/Pi — best for Home Assistant / Node-RED
           Install: https://mosquitto.org/download/
           Broker IP = your PC's local IP (e.g. 192.168.1.10), Port 1883

  CLOUD  : broker.hivemq.com (free, public, no account) Port 1883
           test.mosquitto.org (free, public)             Port 1883
           ⚠ Public brokers are shared — don't send sensitive data

  HOME ASSISTANT:
    - Enable MQTT integration → Settings → Devices → MQTT
    - Broker IP = HA device IP, Port 1883
    - Add credentials if auth enabled

  NODE-RED:
    - Install node-red-contrib-mqtt-broker or connect to Mosquitto
    - Use "mqtt in" node subscribed to "esp32/sensors"
    - Parse the JSON payload → wire to dashboard gauges

  Library needed: "PubSubClient" by Nick O'Leary
  Install: Arduino IDE → Library Manager → search "PubSubClient"

  Works on : ESP32 DevKit, NodeMCU ESP32-S
  Watch    : Serial Monitor at 115200 for connection and publish events.

  AP Mode Note:
  ─────────────
  MQTT requires an internet/LAN connection to reach the broker.
  If using AP mode without STA, you need the broker ON the same AP subnet
  or run a broker directly on a device connected to the ESP32's AP.
  Typical setup: ESP32 in STA mode → LAN → MQTT broker on same LAN.
*/

#include <WiFi.h>
#include <PubSubClient.h>

// ── Wi-Fi ─────────────────────────────────────────────────────
const char* SSID     = "YOUR_WIFI_NAME";
const char* PASSWORD = "YOUR_WIFI_PASSWORD";

// ── MQTT Broker ───────────────────────────────────────────────
const char* MQTT_BROKER   = "192.168.1.10";  // Change to your broker IP
const int   MQTT_PORT     = 1883;
const char* MQTT_USER     = "";              // Leave empty if no auth
const char* MQTT_PASS     = "";
const char* CLIENT_ID     = "esp32-lab-01"; // Unique per device

// ── Topics ────────────────────────────────────────────────────
const char* TOPIC_SENSORS = "esp32/sensors"; // ESP32 publishes here
const char* TOPIC_LED     = "esp32/led";     // ESP32 subscribes here
// ──────────────────────────────────────────────────────────────

#define LED_PIN 2

WiFiClient   wifiClient;
PubSubClient mqtt(wifiClient);

// Called when a subscribed message arrives
void onMessage(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  Serial.printf("[MQTT IN] %s → %s\n", topic, msg.c_str());

  if (String(topic) == TOPIC_LED) {
    if (msg == "ON")  { digitalWrite(LED_PIN, HIGH); Serial.println("LED ON"); }
    if (msg == "OFF") { digitalWrite(LED_PIN, LOW);  Serial.println("LED OFF"); }
  }
}

void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Connecting to MQTT broker...");
    bool ok = (strlen(MQTT_USER) > 0)
              ? mqtt.connect(CLIENT_ID, MQTT_USER, MQTT_PASS)
              : mqtt.connect(CLIENT_ID);

    if (ok) {
      Serial.println(" connected!");
      mqtt.subscribe(TOPIC_LED);
      Serial.printf("Subscribed to: %s\n", TOPIC_LED);
    } else {
      Serial.printf(" failed (rc=%d). Retry in 5s.\n", mqtt.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\n[OK] WiFi connected: " + WiFi.localIP().toString());

  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(onMessage);
  connectMQTT();
}

void loop() {
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();  // process incoming messages

  // Publish sensor data every 10 seconds
  static unsigned long lastPublish = 0;
  if (millis() - lastPublish >= 10000) {
    lastPublish = millis();

    // Stub values — replace with real DHT readings from sketch 07
    float temp = 25.4;
    float hum  = 62.0;

    // Publish as JSON — easy to parse in Node-RED / HA
    char payload[80];
    snprintf(payload, sizeof(payload),
             "{\"temperature\":%.1f,\"humidity\":%.1f,\"uptime\":%lu}",
             temp, hum, millis() / 1000);

    mqtt.publish(TOPIC_SENSORS, payload);
    Serial.printf("[MQTT OUT] %s → %s\n", TOPIC_SENSORS, payload);
  }
}
