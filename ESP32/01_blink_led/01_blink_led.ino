/*
  01 — Blink LED
  --------------
  The "Hello World" of hardware.
  Toggles the onboard LED (GPIO 2) on and off every second.

  Works on: ESP32 DevKit, NodeMCU ESP32-S
  Watch    : The blue/red onboard LED blinks. No Serial output needed.

  Try changing DELAY_MS to see faster/slower blink.
  Try changing LED_PIN to any free GPIO and wire an external LED + 220Ω resistor.
*/

#define LED_PIN   2       // Onboard LED on most ESP32 boards
#define DELAY_MS  1000    // Blink interval in milliseconds

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("01 — Blink LED started");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  Serial.println("LED ON");
  delay(DELAY_MS);

  digitalWrite(LED_PIN, LOW);
  Serial.println("LED OFF");
  delay(DELAY_MS);
}
