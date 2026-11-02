#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_CS   D0
#define TFT_DC   D1
#define TFT_RST  D2
#define TFT_SCK  D8
#define TFT_MISO D9
#define TFT_MOSI D10

#define SPI_SPEED 2000000   // raise later once wires are soldered

Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);
  delay(1500);
  SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
  tft.begin(SPI_SPEED);
  tft.setRotation(1);
  tft.invertDisplay(true);   // <-- the fix for inverted colors

  tft.fillScreen(ILI9341_RED);   Serial.println("RED");   delay(1500);
  tft.fillScreen(ILI9341_GREEN); Serial.println("GREEN"); delay(1500);
  tft.fillScreen(ILI9341_BLUE);  Serial.println("BLUE");  delay(1500);
  tft.fillScreen(ILI9341_WHITE); Serial.println("WHITE"); delay(1500);

  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.setCursor(10, 10);
  tft.println("Hello!");

  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(10, 60);
  tft.println("XIAO ESP32S3");

  tft.setTextColor(ILI9341_CYAN);
  tft.setCursor(10, 100);
  tft.println("Display works!");

  tft.fillRect(10,  180, 60, 40, ILI9341_RED);
  tft.fillRect(80,  180, 60, 40, ILI9341_GREEN);
  tft.fillRect(150, 180, 60, 40, ILI9341_BLUE);
  tft.fillRect(220, 180, 60, 40, ILI9341_WHITE);

  Serial.println("Done.");
}

void loop() {}