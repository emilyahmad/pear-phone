#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_CS   D0
#define TFT_DC   D1
#define TFT_RST  D2
#define TFT_SCK  D8
#define TFT_MISO D9
#define TFT_MOSI D10

#define TOUCH_SDA D4
#define TOUCH_SCL D5
#define TOUCH_INT D3
#define TOUCH_RST D6        // optional, not used in this test
#define FT_ADDR   0x38      // FT6336 / FT5x06 address

#define SPI_SPEED     2000000   // raise later once the wires are soldered
#define INVERT_COLORS true
#define ROTATION      1         // landscape, same as the profile screen

// Touch orientation. If the dot lands in the wrong place, change these (table in my message).
#define TOUCH_SWAP_XY true
#define TOUCH_FLIP_X  false
#define TOUCH_FLIP_Y  true

Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);

bool touchFound = false;
bool wasTouching = false;
uint32_t lastScan = 0;
int lastPx = -99, lastPy = -99;

// ---- I2C helpers ------------------------------------------------------------

bool readRegs(uint8_t reg, uint8_t* buf, int len) {
  Wire.beginTransmission(FT_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((int)FT_ADDR, len) != len) return false;
  for (int i = 0; i < len; i++) buf[i] = Wire.read();
  return true;
}

void i2cScan() {
  Serial.println("Scanning I2C...");
  int found = 0;
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  found device at 0x%02X\n", a);
      found++;
    }
  }
  if (!found) Serial.println("  nothing found -> check CTP_SDA/SCL wires, 3V3, GND, CTP_RST");
}

bool probeTouch() {
  uint8_t id = 0, vendor = 0;
  if (!readRegs(0xA3, &id, 1)) return false;
  readRegs(0xA8, &vendor, 1);
  Serial.printf("Touch chip ID: 0x%02X (FT6336U is usually 0x64), vendor: 0x%02X (usually 0x11)\n", id, vendor);
  return true;
}

// ---- Screen -----------------------------------------------------------------

// Native touch coordinates (240 x 320 portrait) -> screen coordinates
void mapTouch(int rx, int ry, int& x, int& y) {
  int a = rx, b = ry;
  if (TOUCH_SWAP_XY) { int t = a; a = b; b = t; }
  if (TOUCH_FLIP_X)  a = (tft.width()  - 1) - a;
  if (TOUCH_FLIP_Y)  b = (tft.height() - 1) - b;
  x = a; y = b;
}

void drawUI() {
  tft.fillScreen(ILI9341_BLACK);
  int w = tft.width(), h = tft.height();
  tft.fillRect(0,      0,      24, 24, ILI9341_RED);      // top-left
  tft.fillRect(w - 24, 0,      24, 24, ILI9341_GREEN);    // top-right
  tft.fillRect(0,      h - 24, 24, 24, ILI9341_BLUE);     // bottom-left
  tft.fillRect(w - 24, h - 24, 24, 24, ILI9341_YELLOW);   // bottom-right

  tft.fillRoundRect(w / 2 - 30, h - 28, 60, 22, 5, ILI9341_WHITE);   // clear button
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(w / 2 - 9, h - 20);
  tft.print("CLEAR");

  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(30, 6);
  tft.print("Touch the 4 colored corners, then draw");
}

void showCoords(int n, int x, int y) {
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);   // background color overwrites old text
  tft.setCursor(30, 20);
  tft.printf("touches=%d  x=%3d  y=%3d   ", n, x, y);
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n=== Touch test ===");

  SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
  tft.begin(SPI_SPEED);
  tft.setRotation(ROTATION);
  tft.invertDisplay(INVERT_COLORS);
  tft.fillScreen(ILI9341_BLACK);

  // Reset the touch controller, then start I2C
  pinMode(TOUCH_RST, OUTPUT);
  digitalWrite(TOUCH_RST, LOW);  delay(20);
  digitalWrite(TOUCH_RST, HIGH); delay(300);
  Wire.begin(TOUCH_SDA, TOUCH_SCL);
  Wire.setClock(400000);

  i2cScan();
  touchFound = probeTouch();

  if (touchFound) {
    Serial.println("Touch controller found. Touch the screen.");
    drawUI();
  } else {
    Serial.println("No touch controller answered at 0x38. Retrying every 2 s...");
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(10, 90);  tft.println("No touch chip found");
    tft.setTextSize(1);
    tft.setCursor(10, 120); tft.println("Check CTP_SDA/SCL/RST wires.");
    tft.setCursor(10, 134); tft.println("See serial monitor.");
  }
}

void loop() {
  if (!touchFound) {                      // keep retrying until the chip answers
    if (millis() - lastScan > 2000) {
      lastScan = millis();
      i2cScan();
      touchFound = probeTouch();
      if (touchFound) drawUI();
    }
    return;
  }

  uint8_t b[11];
  if (!readRegs(0x02, b, 11)) {           // 0x02 = touch count, then up to 2 touch points
    delay(50);
    return;
  }

  int n = b[0] & 0x0F;
  if (n > 2) n = 0;                       // garbage read, ignore

  if (n == 0) {
    if (wasTouching) Serial.println("released");
    wasTouching = false;
    lastPx = lastPy = -99;
    delay(10);
    return;
  }

  for (int i = 0; i < n; i++) {
    int o = 1 + i * 6;                    // second touch starts 6 bytes later
    int rx = ((b[o] & 0x0F) << 8) | b[o + 1];
    int ry = ((b[o + 2] & 0x0F) << 8) | b[o + 3];
    int x, y;
    mapTouch(rx, ry, x, y);

    if (i == 0) {
      // Clear button
      int w = tft.width(), h = tft.height();
      if (!wasTouching && x >= w / 2 - 30 && x <= w / 2 + 30 && y >= h - 28 && y <= h - 6) {
        drawUI();
        wasTouching = true;
        return;
      }
      showCoords(n, x, y);
      if (abs(x - lastPx) > 2 || abs(y - lastPy) > 2 || !wasTouching) {
        Serial.printf("raw(%3d,%3d) -> screen(%3d,%3d)  touches=%d\n", rx, ry, x, y, n);
        lastPx = x; lastPy = y;
      }
    }
    uint16_t c = (i == 0) ? ILI9341_MAGENTA : ILI9341_GREEN;   // 2nd finger is green
    tft.fillCircle(x, y, 4, c);
  }
  wasTouching = true;
  delay(10);
}