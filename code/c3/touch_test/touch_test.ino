/*******************************************************************************
 *  Touch diagnostic for the FT6336G on the XIAO ESP32-C3
 *
 *  1. Resets the touch chip and scans the I2C bus for it (expects 0x38)
 *  2. Shows the result on screen + Serial (115200)
 *  3. Draws a crosshair, coordinates, and a trail wherever you touch
 *  4. Shows the live level of the INT pin (it should drop LOW while touching)
 *  5. Tap the four corner targets to check the coordinate mapping
 *******************************************************************************/

#include <Wire.h>
#include <LovyanGFX.h>
#include "../lgfx_config.h" // Configuration for our screen

LGFX tft;

// Touch pins (XIAO D-label -> GPIO), must match lgfx_config.h
static const int PIN_SDA = 3; // D1
static const int PIN_SCL = 2; // D0
static const int PIN_RST = 4; // D2
static const int PIN_INT = 5; // D3
static const uint8_t TOUCH_ADDR = 0x38;

static bool i2cFound = false;
static int  i2cDevices = 0;
static uint32_t touchCount = 0;
static bool wasTouching = false;
static bool cornerHit[4] = {false, false, false, false};

static const int CORNER = 40;
static const int CLEAR_W = 90, CLEAR_H = 30;

static bool pointInRect(int x, int y, int rx, int ry, int rw, int rh) {
  return (x >= rx) && (x < rx + rw) && (y >= ry) && (y < ry + rh);
}

// Hardware reset the touch chip, then scan the bus for it
static void probeTouchChip() {
  pinMode(PIN_INT, INPUT_PULLUP);
  pinMode(PIN_RST, OUTPUT);
  digitalWrite(PIN_RST, LOW);
  delay(20);
  digitalWrite(PIN_RST, HIGH);
  delay(300); // FT6336 needs a moment to boot

  Wire.begin(PIN_SDA, PIN_SCL);
  Serial.println("Scanning I2C...");
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  device at 0x%02X\n", addr);
      i2cDevices++;
      if (addr == TOUCH_ADDR) i2cFound = true;
    }
  }
  if (i2cDevices == 0) Serial.println("  nothing found (check SDA/SCL wiring, 3.3V, GND)");
  Wire.end(); // hand the bus back to LovyanGFX
}

static void drawCorners() {
  const int xs[4] = {0, tft.width() - CORNER, 0, tft.width() - CORNER};
  const int ys[4] = {0, 0, tft.height() - CORNER, tft.height() - CORNER};
  for (int i = 0; i < 4; i++) {
    tft.fillRect(xs[i], ys[i], CORNER, CORNER, cornerHit[i] ? TFT_GREEN : TFT_RED);
  }
}

static void drawClearButton() {
  int x = (tft.width() - CLEAR_W) / 2, y = tft.height() - CLEAR_H;
  tft.fillRect(x, y, CLEAR_W, CLEAR_H, TFT_DARKGREY);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setCursor(x + 14, y + 7);
  tft.print("CLEAR");
}

static void drawHeader() {
  tft.fillRect(CORNER, 0, tft.width() - 2 * CORNER, 70, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(i2cFound ? TFT_GREEN : TFT_RED, TFT_BLACK);
  tft.setCursor(CORNER + 4, 4);
  tft.printf("I2C 0x38: %s (%d devices)", i2cFound ? "FOUND" : "NOT FOUND", i2cDevices);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(CORNER + 4, 16);
  tft.printf("Touches: %lu", (unsigned long)touchCount);
}

static void drawScreen() {
  tft.fillScreen(TFT_BLACK);
  drawCorners();
  drawClearButton();
  drawHeader();
  tft.setTextSize(1);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(CORNER + 4, 28);
  tft.print("Touch anywhere; tap corners");
}

// Live INT pin readout (low = chip says a touch is happening)
static void drawIntState() {
  static int last = -1;
  int level = digitalRead(PIN_INT);
  if (level == last) return;
  last = level;
  tft.fillRect(CORNER + 4, 40, 150, 10, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(level ? TFT_YELLOW : TFT_GREEN, TFT_BLACK);
  tft.setCursor(CORNER + 4, 40);
  tft.printf("INT pin: %s", level ? "HIGH (idle)" : "LOW (touch)");
}

void setup() {
  Serial.begin(115200);
  delay(1500); // give the serial monitor time to attach
  Serial.println("\n=== Touch test ===");

  probeTouchChip();

  tft.init();
  tft.setBrightness(255);
  Serial.printf("Screen %dx%d, I2C touch chip %s\n", tft.width(), tft.height(),
                i2cFound ? "found" : "NOT found");

  drawScreen();
}

void loop() {
  drawIntState();

  uint16_t x, y;
  bool touching = tft.getTouch(&x, &y);

  if (touching) {
    // Trail dot + crosshair at the touch point
    tft.fillCircle(x, y, 3, TFT_CYAN);
    tft.drawFastHLine(x - 12, y, 25, TFT_WHITE);
    tft.drawFastVLine(x, y - 12, 25, TFT_WHITE);

    // Coordinates readout
    tft.fillRect(CORNER + 4, 52, 150, 10, TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(CORNER + 4, 52);
    tft.printf("x=%u y=%u", x, y);

    if (!wasTouching) {
      touchCount++;
      Serial.printf("touch #%lu at (%u,%u)\n", (unsigned long)touchCount, x, y);
      drawHeader();

      // Corner targets turn green when hit
      const int xs[4] = {0, tft.width() - CORNER, 0, tft.width() - CORNER};
      const int ys[4] = {0, 0, tft.height() - CORNER, tft.height() - CORNER};
      for (int i = 0; i < 4; i++) {
        if (pointInRect(x, y, xs[i], ys[i], CORNER, CORNER)) cornerHit[i] = true;
      }
      drawCorners();

      if (pointInRect(x, y, (tft.width() - CLEAR_W) / 2, tft.height() - CLEAR_H, CLEAR_W, CLEAR_H)) {
        for (int i = 0; i < 4; i++) cornerHit[i] = false;
        touchCount = 0;
        drawScreen();
      }
    }
  }

  wasTouching = touching;
}
