#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_CS   D0
#define TFT_DC   D1
#define TFT_RST  D2
#define TFT_SCK  D8
#define TFT_MISO D9
#define TFT_MOSI D10

#define SPI_SPEED     2000000   // raise later once the wires are soldered
#define INVERT_COLORS true

#define CORNER_RADIUS 70        // max is 120 (half the screen width). Higher = rounder corners
#define LINE_THICKNESS 3        // pixels

Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);

// Draws a rounded rectangle outline along the very edge of the screen.
// Thickness is built by drawing nested outlines, each one pixel further in.
void drawBorder(uint16_t color) {
  int w = tft.width();    // 240 in portrait
  int h = tft.height();   // 320 in portrait
  for (int t = 0; t < LINE_THICKNESS; t++) {
    tft.drawRoundRect(t, t, w - 2 * t, h - 2 * t, CORNER_RADIUS - t, color);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
  tft.begin(SPI_SPEED);
  tft.setRotation(2);                 // portrait 240x320 (use 0 to flip 180°)
  tft.invertDisplay(INVERT_COLORS);

  tft.fillScreen(ILI9341_BLACK);
  drawBorder(tft.color565(255, 92, 92));   // coral, like the reference
  Serial.println("Border drawn.");
}

void loop() {}