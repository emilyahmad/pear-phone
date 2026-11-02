#include <LovyanGFX.h>
#include "../lgfx_config.h"

LGFX tft;

void setup() {
  Serial.begin(115200);
  delay(50);

  tft.init();
  tft.setBrightness(255);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 20);

  tft.println("Hello, world!");
  tft.println("LovyanGFX on ESP32");
}

void loop() {
  static uint32_t last = 0;
  static int x = 20;

  if (millis() - last > 250) {
    last = millis();
    tft.fillRect(20, 80, 240, 24, TFT_BLACK);
    tft.setCursor(20, 80);
    tft.print("Tick ");
    tft.print(millis() / 1000);
    tft.print("s");
  }
}
