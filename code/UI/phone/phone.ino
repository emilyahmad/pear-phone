/*******************************************************************************
 *  Pear Phone: home screen + TheSlap, switched by tapping
 *
 *  - Starts on the home screen.
 *  - Tap the SplashFace app  -> TheSlap profile screen.
 *  - Tap the Home tab on TheSlap -> back to the home screen.
 *  - Tap the New tab on TheSlap  -> new post screen with a keyboard.
 *    POST puts your message on the card, Cancel goes back without posting.
 *
 *  Board: XIAO_ESP32S3. Wiring: see the touchscreen pinout in the journal.
 *  The two screens live in the home_screen.h and slap_screen.h tabs.
 *
 *  Touch calibration: open the Serial Monitor (115200). Every tap prints the
 *  raw touch point and where the sketch thinks you tapped. If taps land in the
 *  wrong place, change TOUCH_OFFSET below (try 2 first, then 1 and 3).
 *******************************************************************************/

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

#define SPI_SPEED      2000000   // raise later once the wires are soldered
#define INVERT_COLORS  true

// FT6336G capacitive touch (I2C)
#define TOUCH_SDA   5      // D4
#define TOUCH_SCL   6      // D5
#define TOUCH_INT   4      // D3
#define TOUCH_RST   43     // D6
#define TOUCH_ADDR  0x38
#define TOUCH_OFFSET 0     // extra 90° turns (0-3) if taps don't line up with the screen

Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);

#include "home_screen.h"
#include "slap_screen.h"

const int SCREEN_HOME = 0;
const int SCREEN_SLAP = 1;
const int SCREEN_COMPOSE = 2;   // new post + keyboard
int currentScreen = SCREEN_HOME;

const char* APP_TO_OPEN = "SplashFace";   // app on the home screen that opens TheSlap

// ---- Touch ---------------------------------------------------------------------

void touchBegin() {
  pinMode(TOUCH_INT, INPUT_PULLUP);
  pinMode(TOUCH_RST, OUTPUT);
  digitalWrite(TOUCH_RST, LOW);   // hardware reset
  delay(20);
  digitalWrite(TOUCH_RST, HIGH);
  delay(300);                     // the chip needs a moment to boot
  Wire.begin(TOUCH_SDA, TOUCH_SCL);

  Wire.beginTransmission(TOUCH_ADDR);
  if (Wire.endTransmission() == 0) Serial.println("Touch chip found at 0x38.");
  else Serial.println("Touch chip NOT found: check SDA/SCL, 3V3 and GND.");
}

// Reads one touch point in the panel's own portrait coordinates (240x320).
// Returns false when nothing is touching the screen.
bool readRawTouch(int& rx, int& ry) {
  Wire.beginTransmission(TOUCH_ADDR);
  Wire.write(0x02);                                   // TD_STATUS register
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((uint8_t)TOUCH_ADDR, (uint8_t)5) != 5) return false;
  uint8_t points = Wire.read() & 0x0F;
  uint8_t xh = Wire.read(), xl = Wire.read();
  uint8_t yh = Wire.read(), yl = Wire.read();
  if (points == 0 || points > 2) return false;
  rx = ((xh & 0x0F) << 8) | xl;
  ry = ((yh & 0x0F) << 8) | yl;
  return true;
}

// Turns a raw touch point into screen coordinates for the given rotation
void mapTouch(int rx, int ry, int rotation, int& x, int& y) {
  switch ((rotation + TOUCH_OFFSET) & 3) {
    case 0:  x = rx;        y = ry;        break;
    case 1:  x = ry;        y = 239 - rx;  break;
    case 2:  x = 239 - rx;  y = 319 - ry;  break;
    default: x = 319 - ry;  y = rx;        break;
  }
}

// ---- Screens -------------------------------------------------------------------

void showHome() {
  currentScreen = SCREEN_HOME;
  tft.setRotation(home::ROTATION);
  home::drawHomeScreen();
  Serial.println("Home screen drawn.");
}

void showSlap() {
  currentScreen = SCREEN_SLAP;
  tft.setRotation(slap::ROTATION);
  slap::show();
  Serial.println("TheSlap screen drawn.");
}

void showCompose() {
  currentScreen = SCREEN_COMPOSE;
  tft.setRotation(slap::ROTATION);
  slap::showCompose();
  Serial.println("New post screen drawn.");
}

// Called with a touch point in screen coordinates. Returns true if it hit
// something (so the rest of that touch is ignored), false if it missed.
bool handleTap(int x, int y) {
  if (currentScreen == SCREEN_HOME) {
    int idx = home::appAt(x, y);
    if (idx < 0) return false;
    Serial.printf("  -> tapped %s\n", home::APPS[idx].name);
    if (strcmp(home::APPS[idx].name, APP_TO_OPEN) == 0) showSlap();
    return true;
  } else if (currentScreen == SCREEN_SLAP) {
    if (slap::homeTabAt(x, y)) {
      Serial.println("  -> tapped Home tab");
      showHome();
      return true;
    } else if (slap::newTabAt(x, y)) {
      Serial.println("  -> tapped New tab");
      showCompose();
      return true;
    }
    return false;
  } else {   // SCREEN_COMPOSE
    int result = slap::composeTap(x, y);
    if (result == slap::COMPOSE_POSTED) {
      Serial.printf("  -> posted: %s\n", slap::statusText);
      showSlap();
    } else if (result == slap::COMPOSE_CANCEL) {
      Serial.println("  -> cancelled post");
      showSlap();
    }
    return result != slap::COMPOSE_MISS;
  }
}

// ---- Main ----------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(1500);
  SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
  tft.begin(SPI_SPEED);
  tft.invertDisplay(INVERT_COLORS);
  touchBegin();

  showHome();
}

// How long a touch keeps being checked if its first reading missed everything.
// The first point the touch chip reports can be a few pixels off (worst near the
// edges), so instead of giving up we keep checking the next few readings.
#define TAP_RETRY_MS 150

void loop() {
  static bool wasTouching = false;
  static bool tapDone = false;        // this touch already hit something
  static uint32_t touchStart = 0;

  int rx, ry;
  bool touching = readRawTouch(rx, ry);
  if (touching && !wasTouching) {                    // a new touch starts
    touchStart = millis();
    tapDone = false;
  }
  if (touching && !tapDone && millis() - touchStart <= TAP_RETRY_MS) {
    int rotation = (currentScreen == SCREEN_HOME) ? home::ROTATION : slap::ROTATION;
    int x, y;
    mapTouch(rx, ry, rotation, x, y);
    Serial.printf("tap raw=(%d,%d) screen=(%d,%d)\n", rx, ry, x, y);
    tapDone = handleTap(x, y);
  }
  wasTouching = touching;

  if (currentScreen != SCREEN_HOME) slap::update();  // blinking sparkles (Slap + new post)
  delay(10);
}
