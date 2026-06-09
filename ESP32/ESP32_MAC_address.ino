#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  
  // Set to Station Mode but DO NOT try to connect to a router
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); 
  delay(100);

  Serial.println("\n--- MAC ADDRESS SCANNER ---");
  Serial.print("ESP32 MAC Address: ");
  Serial.println(WiFi.macAddress()); 
  Serial.println("---------------------------");
}

void loop() {
  // Empty
}