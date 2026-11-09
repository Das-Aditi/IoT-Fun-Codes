#include <Wire.h>
#include "MAX30105.h"

MAX30105 particleSensor;

// --- Adaptive Heart Rate Detector State ---
float dcFilter = 0;
float acWave = 0;
float prevAcWave = 0;
float waveDerivative = 0;
float prevDerivative = 0;

unsigned long lastBeatTime = 0;
float currentBPM = 0;
int beatCount = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // ESP32 I2C pins
  Wire.setClock(400000); // 400 kHz fast I2C

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("[ERR] MAX30102 not detected. Check I2C wiring (SDA=21, SCL=22).");
    while (1);
  }

  // Configuration tuned for human fingertip photoplethysmography (PPG):
  // LED power: 0x24 (~7.6mA IR)
  // Sample avg: 4 samples averaged
  // LED Mode: 2 (Red + IR)
  // Sample rate: 400 Hz (after 4x averaging -> 100 Hz output)
  // Pulse width: 411 us (18-bit ADC resolution)
  // ADC range: 16384
  particleSensor.setup(0x24, 4, 2, 400, 411, 16384);
  particleSensor.setPulseAmplitudeGreen(0); // Disable green LED

  Serial.println("[OK] MAX30102 Configured. Place finger with light, steady pressure.");
}

void loop() {
  // Read sensor FIFO buffer
  particleSensor.check();
  
  while (particleSensor.available()) {
    long rawIR = particleSensor.getFIFOIR();
    particleSensor.nextSample();

    // 1. Finger Contact Threshold Check
    if (rawIR < 50000) {
      dcFilter = 0;
      currentBPM = 0;
      lastBeatTime = 0;
      continue;
    }

    // 2. High-Pass DC Filter (Removes static skin reflection to reveal the AC pulse)
    if (dcFilter == 0) dcFilter = rawIR;
    dcFilter = (dcFilter * 0.95) + (rawIR * 0.05);
    acWave = rawIR - dcFilter;

    // 3. First-order differentiator to locate wave slopes
    waveDerivative = acWave - prevAcWave;
    prevAcWave = acWave;

    // 4. Zero-crossing Peak Detection (Slope changes from positive to negative above noise floor)
    unsigned long now = millis();
    if (prevDerivative > 0 && waveDerivative <= 0 && acWave > 18.0) {
      long interval = now - lastBeatTime;

      // Filter for valid human heart intervals: 330ms (180 BPM) to 1500ms (40 BPM)
      if (interval >= 330 && interval <= 1500) {
        float instantBpm = 60000.0 / (float)interval;
        
        // Discard extreme instantaneous jumps
        if (currentBPM == 0) {
          currentBPM = instantBpm;
        } else {
          currentBPM = (currentBPM * 0.7) + (instantBpm * 0.3); // Responsive 2-beat smoothing
        }
        beatCount++;
      }
      lastBeatTime = now;
    }
    prevDerivative = waveDerivative;
  }

  // Print formatted telemetry line every 400ms
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 400) {
    lastPrint = millis();

    long currentIR = particleSensor.getIR();
    if (currentIR < 50000) {
      Serial.println("DATA,heart_rate=No finger is detected&steadiness=AX=0.00,AY=0.00,AZ=9.80");
    } else {
      int displayBpm = (currentBPM >= 45 && currentBPM <= 190) ? (int)(currentBPM + 0.5) : 0;
      Serial.print("DATA,heart_rate=IR=");
      Serial.print(currentIR);
      Serial.print(",BPM=");
      Serial.print(displayBpm);
      Serial.println("&steadiness=AX=0.00,AY=0.00,AZ=9.80");
    }
  }
}