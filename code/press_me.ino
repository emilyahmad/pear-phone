/*******************************************************************************
 *  MostlyBuilds - ESP32 SPI Capacitive Touchscreen Tutorial
 *  Author: MostlyBuilds
 *  Guide:  https://mostlybuilds.com/esp32-spi-capacitive-touch-screen-tutorial
 *
 *  Hello world with a button to demonstrate tap handling
 *******************************************************************************/
 
#include <LovyanGFX.h>
#include "lgfx_config.h" // Configuration for our screen

// Create an instance of LovyanGFX (LGFX)
LGFX tft;

// UI state
static uint32_t pressCount = 0;
static bool wasTouching = false;

// Button rectangle
static int btnX = 40;
static int btnY = 140;
static int btnW = 240;
static int btnH = 80;

// Function for determining if a point exists within a rectangle
static bool pointInRect(int x, int y, int rx, int ry, int rw, int rh) {
  return (x >= rx) && (x < rx + rw) && (y >= ry) && (y < ry + rh);
}

// Render the count from pressing the button
static void drawCount() {
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Clear just the count line area
  tft.fillRect(20, 70, 300, 24, TFT_BLACK);

  tft.setCursor(20, 70);
  tft.print("Count: ");
  tft.print(pressCount);
}

// Render the button that has different colors
// depending on the pressed state
static void drawButton(bool pressed) {
  const uint16_t bg      = TFT_BLACK;
  const uint16_t border  = TFT_WHITE;
  const uint16_t fillUp  = TFT_DARKGREY;
  const uint16_t fillDn  = 0x39E7; // darker gray
  const uint16_t textCol = TFT_BLACK;

  // Button fill + border
  tft.fillRoundRect(btnX, btnY, btnW, btnH, 10, pressed ? fillDn : fillUp);
  tft.drawRoundRect(btnX, btnY, btnW, btnH, 10, border);

  // Label
  const char* label = "PRESS ME";

  tft.setTextSize(2);
  tft.setTextColor(textCol, pressed ? fillDn : fillUp);

  // Center text using LovyanGFX helpers
  int tw = tft.textWidth(label);
  int th = tft.fontHeight();

  int tx = btnX + (btnW - tw) / 2;
  int ty = btnY + (btnH - th) / 2;

  tft.setCursor(tx, ty);
  tft.print(label);

  // Restore default text colors
  tft.setTextColor(TFT_WHITE, bg);
}

void setup() {
  Serial.begin(115200);
  delay(50);

  // Init the screen with the lgfx_config.h
  tft.init();

  // Set the screen brightness to the max
  tft.setBrightness(255); 

  // Clear screen
  tft.fillScreen(TFT_BLACK);

  // Basic text setup
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // (foreground, background)
  tft.setTextSize(2);

  tft.setCursor(20, 20);
  tft.println("Hello, world!");
  tft.setCursor(20, 45);
  tft.println("Tap the button:");

  drawCount();
  drawButton(false);
}

void loop() {
  static uint32_t last = 0;
  if (millis() - last > 250) {
    last = millis();
    tft.fillRect(20, 100, 300, 24, TFT_BLACK);
    tft.setCursor(20, 100);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.print("Tick ");
    tft.print(millis() / 1000);
    tft.print("s");
  }

  uint16_t rx, ry;
  bool touching = tft.getTouch(&rx, &ry);

  // Debug dot + log on new press (helps confirm mapping)
  if (touching && !wasTouching) {
    // Draw a red dot where the touch was detected
    tft.fillCircle(rx, ry, 4, TFT_RED);
    // Log the touch coordinate data + screen dimensions
    Serial.printf("raw(%u,%u) wh(%d,%d)\n", rx, ry, tft.width(), tft.height());
  }

  // Detect if the user is pressing within the button bounds
  if (touching && !wasTouching) {
    if (pointInRect(rx, ry, btnX, btnY, btnW, btnH)) {
      pressCount++;
      // Update the rendered count value
      drawCount();

      // Redraw the button in the pressed state
      drawButton(true);
    }
  }

  // On release, redraw the button in the normal non-pressed state
  if (!touching && wasTouching) {
    drawButton(false);
  }

  wasTouching = touching;
}
