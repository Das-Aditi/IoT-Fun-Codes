/*
  07 — DHT11 / DHT22 Temperature & Humidity Sensor
  --------------------------------------------------
  Reads temperature and humidity every 2 seconds and prints to Serial.
  Also serves a live auto-refreshing web page with the readings.

  DHT11  → cheaper, ±2°C accuracy, 20–80% RH range, blue
  DHT22  → better,  ±0.5°C accuracy, 0–100% RH range, white
  Both use the same wiring and almost the same code (just change DHTTYPE).

  Wiring:
  ───────
  DHT PIN 1 (VCC)  → 3.3V
  DHT PIN 2 (DATA) → GPIO 4  (+ 10kΩ pull-up resistor to 3.3V)
  DHT PIN 3        → not connected
  DHT PIN 4 (GND)  → GND

  Library needed: "DHT sensor library" by Adafruit
  Install via: Arduino IDE → Tools → Manage Libraries → search "DHT sensor library"
  Also install: "Adafruit Unified Sensor" (dependency, same place)

  Works on : ESP32 DevKit, NodeMCU ESP32-S
  Watch    : Serial Monitor at 115200 for raw readings.
             Also accessible from browser at STA IP (or 192.168.4.1 in AP mode).

  AP Mode Access:
  ───────────────
  Swap WiFi.begin() for WiFi.softAP() as shown in sketch 06.
  Connect to ESP32's AP → open 192.168.4.1 → see live temperature page.
  Perfect for a standalone weather station with no internet required.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

const char* SSID     = "YOUR_WIFI_NAME";
const char* PASSWORD = "YOUR_WIFI_PASSWORD";

#define DHTPIN  4         // GPIO pin connected to DHT data line
#define DHTTYPE DHT11     // Change to DHT22 if using DHT22

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

float lastTemp = 0;
float lastHumidity = 0;

// Auto-refresh page every 5 seconds
const char* htmlTemplate = R"rawhtml(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Weather</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta http-equiv="refresh" content="5">
  <style>
    body { font-family: monospace; background: #0d1117; color: #c9d1d9;
           display: flex; flex-direction: column; align-items: center; padding-top: 60px; }
    h1   { font-size: 1.8em; color: #58a6ff; }
    .card { background: #161b22; border: 1px solid #30363d; border-radius: 10px;
            padding: 30px 50px; margin: 10px; text-align: center; }
    .val  { font-size: 3em; font-weight: bold; color: #f0f6fc; }
    .unit { font-size: 1.2em; color: #8b949e; }
    .note { font-size: 0.8em; color: #484f58; margin-top: 30px; }
  </style>
</head>
<body>
  <h1>ESP32 Weather Station</h1>
  <div class="card">
    <div class="unit">Temperature</div>
    <div class="val">%TEMP% °C</div>
  </div>
  <div class="card">
    <div class="unit">Humidity</div>
    <div class="val">%HUM% %%</div>
  </div>
  <p class="note">Auto-refreshes every 5 seconds</p>
</body>
</html>
)rawhtml";

void handleRoot() {
  String page = String(htmlTemplate);
  page.replace("%TEMP%", String(lastTemp, 1));
  page.replace("%HUM%",  String(lastHumidity, 1));
  server.send(200, "text/html", page);
}

// /data endpoint returns raw JSON — useful for fetching from other apps
void handleData() {
  String json = "{\"temperature\":" + String(lastTemp, 2) +
                ",\"humidity\":"    + String(lastHumidity, 2) + "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\n[OK] Connected: http://" + WiFi.localIP().toString());

  server.on("/",     handleRoot);
  server.on("/data", handleData);
  server.begin();
}

void loop() {
  server.handleClient();

  static unsigned long lastRead = 0;
  if (millis() - lastRead >= 2000) {
    lastRead = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("[ERROR] Failed to read from DHT sensor. Check wiring.");
      return;
    }

    lastTemp     = t;
    lastHumidity = h;

    Serial.printf("Temp: %.1f°C   Humidity: %.1f%%\n", t, h);
  }
}
