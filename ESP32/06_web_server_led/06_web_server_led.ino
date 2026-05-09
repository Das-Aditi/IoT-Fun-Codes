/*
  06 — Simple Web Server (LED Control via Browser)
  --------------------------------------------------
  ESP32 connects to your router (STA mode) and hosts a tiny web server.
  Open the STA IP in any browser on the same network → you get a webpage
  with ON / OFF buttons that toggle the onboard LED.

  This is the foundation for ALL browser-based ESP32 control panels.

  Works on : ESP32 DevKit, NodeMCU ESP32-S
  Library  : WebServer.h — built into ESP32 Arduino core, no install needed
  Watch    : Serial Monitor at 115200. Copy the IP, paste in browser.

  Access in AP Mode (no router needed):
  ───────────────────────────────────────
  Change WiFi.mode to WIFI_AP and WiFi.softAP(...) instead of WiFi.begin().
  The ESP32's AP IP is always 192.168.4.1
  Connect phone/laptop to "ESP32-Lab-AP" → open 192.168.4.1 in browser.
  See 06b note at bottom for the exact swap.

  How the web server works:
  ─────────────────────────
  server.on("/", handleRoot)   → when browser GETs "/" serve the HTML page
  server.on("/on", handleOn)   → when browser GETs "/on" turn LED on, redirect back
  server.on("/off", handleOff) → same for off
  server.handleClient()        → must be called in loop() to process incoming requests
*/

#include <WiFi.h>
#include <WebServer.h>

const char* SSID     = "YOUR_WIFI_NAME";
const char* PASSWORD = "YOUR_WIFI_PASSWORD";

#define LED_PIN 2

WebServer server(80);  // HTTP runs on port 80

bool ledState = false;

// ── HTML page sent to the browser ─────────────────────────────
const char* htmlPage = R"rawhtml(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 LED Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: sans-serif; text-align: center; margin-top: 60px; background: #111; color: #eee; }
    h1   { font-size: 2em; margin-bottom: 10px; }
    .state { font-size: 1.2em; margin-bottom: 30px; color: #aaa; }
    a.btn {
      display: inline-block; padding: 16px 40px; margin: 10px;
      font-size: 1.1em; border-radius: 8px; text-decoration: none; font-weight: bold;
    }
    .on  { background: #00c853; color: #000; }
    .off { background: #d50000; color: #fff; }
  </style>
</head>
<body>
  <h1>ESP32 LED Control</h1>
  <p class="state">LED is: <strong>%STATE%</strong></p>
  <a class="btn on"  href="/on">Turn ON</a>
  <a class="btn off" href="/off">Turn OFF</a>
</body>
</html>
)rawhtml";

// ── Handlers ──────────────────────────────────────────────────
void handleRoot() {
  String page = String(htmlPage);
  page.replace("%STATE%", ledState ? "ON" : "OFF");
  server.send(200, "text/html", page);
}

void handleOn() {
  ledState = true;
  digitalWrite(LED_PIN, HIGH);
  Serial.println("[ACTION] LED → ON");
  server.sendHeader("Location", "/");
  server.send(303);   // redirect back to main page
}

void handleOff() {
  ledState = false;
  digitalWrite(LED_PIN, LOW);
  Serial.println("[ACTION] LED → OFF");
  server.sendHeader("Location", "/");
  server.send(303);
}

void handle404() {
  server.send(404, "text/plain", "Not found");
}

// ─────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }

  Serial.println("\n[OK] Connected!");
  Serial.print("Open in browser: http://");
  Serial.println(WiFi.localIP());

  server.on("/",    handleRoot);
  server.on("/on",  handleOn);
  server.on("/off", handleOff);
  server.onNotFound(handle404);
  server.begin();
  Serial.println("Web server started.");

  /*
    06b — AP MODE SWAP
    ──────────────────
    Replace the WiFi block above with:

      WiFi.mode(WIFI_AP);
      WiFi.softAP("ESP32-Lab-AP", "12345678");
      Serial.print("AP IP: ");
      Serial.println(WiFi.softAPIP());  // always 192.168.4.1

    Then connect your phone to "ESP32-Lab-AP"
    and open http://192.168.4.1 in the browser.
    No router, no internet needed at all.
  */
}

void loop() {
  server.handleClient();  // keep listening for browser requests
}
