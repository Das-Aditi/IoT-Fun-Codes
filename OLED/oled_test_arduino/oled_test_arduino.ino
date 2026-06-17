/*
 * OLED Display Test - Arduino (SSD1306 128x64)
 * 
 * Hardware: SSD1306 OLED via I2C
 * Libraries Required:
 *   - Adafruit_SSD1306
 *   - Adafruit_GFX
 * 
 * Connections (I2C):
 *   VCC -> 3.3V or 5V
 *   GND -> GND
 *   SDA -> A4 (Uno) / 20 (Mega) / D2 (ESP8266) / 21 (ESP32)
 *   SCL -> A5 (Uno) / 21 (Mega) / D1 (ESP8266) / 22 (ESP32)
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1   // No reset pin
#define SCREEN_ADDRESS 0x3C // Common I2C address (try 0x3D if not working)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ─── Test state ──────────────────────────────────────────────────────────────
int  testIndex   = 0;
long lastSwitch  = 0;
const int DELAY_MS = 2500; // Time per test (ms)

// ─── Setup ───────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println(F("OLED Test Suite Starting..."));

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("ERROR: SSD1306 not found! Check wiring / I2C address."));
    while (true); // Halt
  }

  display.clearDisplay();
  display.display();
  Serial.println(F("OLED init OK"));
}

// ─── Main Loop ───────────────────────────────────────────────────────────────
void loop() {
  if (millis() - lastSwitch >= DELAY_MS) {
    lastSwitch = millis();
    runTest(testIndex);
    testIndex = (testIndex + 1) % 10;
  }
}

// ─── Test Dispatcher ─────────────────────────────────────────────────────────
void runTest(int idx) {
  display.clearDisplay();

  switch (idx) {
    case 0: testWhiteFill();       break;
    case 1: testBlackFill();       break;
    case 2: testCheckerboard();    break;
    case 3: testText();            break;
    case 4: testLines();           break;
    case 5: testRectangles();      break;
    case 6: testCircles();         break;
    case 7: testProgressBar();     break;
    case 8: testScrollingText();   break;
    case 9: testPixelWalk();       break;
  }

  display.display();
  Serial.print(F("Test #")); Serial.println(idx);
}

// ─── Individual Tests ─────────────────────────────────────────────────────────

// TEST 0: All pixels ON (white fill)
void testWhiteFill() {
  display.fillScreen(SSD1306_WHITE);
  // Label won't show on white — just a bright-screen test
}

// TEST 1: All pixels OFF (black fill) — checks for dead pixels
void testBlackFill() {
  display.fillScreen(SSD1306_BLACK);
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(20, 28);
  display.print(F("Black Fill (OFF)"));
}

// TEST 2: Checkerboard pattern
void testCheckerboard() {
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      if ((x + y) % 2 == 0)
        display.drawPixel(x, y, SSD1306_WHITE);
    }
  }
}

// TEST 3: Text rendering at multiple sizes
void testText() {
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("Size 1 - ABCDEFGH"));

  display.setTextSize(2);
  display.setCursor(0, 16);
  display.println(F("Size 2"));

  display.setTextSize(1);
  display.setCursor(0, 40);
  display.println(F("0123456789 !@#$%^&*"));
  display.setCursor(0, 52);
  display.println(F("abcdefghijklmnopqrst"));
}

// TEST 4: Line drawing (diagonals, H/V lines)
void testLines() {
  // Diagonals
  display.drawLine(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, SSD1306_WHITE);
  display.drawLine(SCREEN_WIDTH - 1, 0, 0, SCREEN_HEIGHT - 1, SSD1306_WHITE);
  // Horizontal & vertical center lines
  display.drawLine(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH - 1, SCREEN_HEIGHT / 2, SSD1306_WHITE);
  display.drawLine(SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 1, SSD1306_WHITE);
}

// TEST 5: Rectangles (outline + filled)
void testRectangles() {
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  display.drawRect(10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20, SSD1306_WHITE);
  display.fillRect(30, 20, 68, 24, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(36, 28);
  display.print(F("Rectangles"));
}

// TEST 6: Circles
void testCircles() {
  display.drawCircle(32, 32, 30, SSD1306_WHITE);
  display.drawCircle(96, 32, 30, SSD1306_WHITE);
  display.fillCircle(64, 32, 15, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(54, 28);
  display.print(F("O"));
}

// TEST 7: Animated progress bar simulation (one frame)
void testProgressBar() {
  static int pct = 0;
  pct = (pct + 20) % 110;

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(28, 5);
  display.print(F("Progress Bar Test"));

  // Outer frame
  display.drawRect(4, 25, 120, 14, SSD1306_WHITE);
  // Fill
  int fillW = map(min(pct, 100), 0, 100, 0, 116);
  display.fillRect(6, 27, fillW, 10, SSD1306_WHITE);

  // Percentage text
  display.setCursor(50, 45);
  display.print(pct); display.print(F("%"));
}

// TEST 8: Scrolling text (horizontal)
void testScrollingText() {
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 24);
  display.print(F("  OLED  TEST  "));
  display.display();

  display.startscrollleft(0x00, 0x0F);
  delay(DELAY_MS - 200);
  display.stopscroll();
}

// TEST 9: Random pixel walk (noise test)
void testPixelWalk() {
  randomSeed(analogRead(0));
  for (int i = 0; i < 512; i++) {
    int x = random(SCREEN_WIDTH);
    int y = random(SCREEN_HEIGHT);
    display.drawPixel(x, y, SSD1306_WHITE);
  }
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(25, 56);
  display.print(F("Pixel Noise Test"));
}
