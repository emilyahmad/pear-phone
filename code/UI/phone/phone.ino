/*******************************************************************************
 *  Pear Phone: home screen + TheSlap, switched by tapping
 *
 *  - First boot: "hello" writes itself in cursive. Tap, type your name, pick
 *    your avatar, and you land on the home screen. The name and avatar are
 *    saved in flash, so later boots go straight to the home screen.
 *  - Tap the Settings app -> who's signed in, and Sign out (tap it twice),
 *    which wipes the saved name/avatar and starts the hello setup again.
 *  - Tap the SplashFace app  -> TheSlap profile screen (your name + avatar).
 *  - Tap the Home tab on TheSlap -> back to the home screen.
 *  - Tap the New tab on TheSlap  -> new post screen with a keyboard.
 *    POST puts your message on the card, Cancel goes back without posting.
 *  - Tap the Clear tab on TheSlap -> empties the status text and feeling.
 *  - Tap the Past tab on TheSlap -> your saved posts (newest 20), newest first.
 *    The arrows page through older ones; tap a post to see it on its card.
 *  - Tap your picture (or the pink UPDATE panel) on TheSlap -> avatar screen.
 *    Change gender, skin, hair, eyes, accessory, shirt, backdrop; Save or Cancel.
 *
 *  Board: XIAO_ESP32S3. Wiring: see the touchscreen pinout in the journal.
 *  The screens live in the home_screen.h, slap_screen.h and setup_screen.h tabs.
 *
 *  Touch calibration: open the Serial Monitor (115200). Every tap prints the
 *  raw touch point and where the sketch thinks you tapped. If taps land in the
 *  wrong place, change TOUCH_OFFSET below (try 2 first, then 1 and 3).
 *******************************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <esp_system.h>       // esp_reset_reason()
#include <Preferences.h>      // saved name + avatar (flash, survives power-off)

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
#include "setup_screen.h"       // uses home:: and slap::, so it comes after them

const int SCREEN_HOME = 0;
const int SCREEN_SLAP = 1;
const int SCREEN_COMPOSE = 2;   // new post + keyboard
const int SCREEN_AVATAR = 3;    // avatar editor
const int SCREEN_HELLO = 4;     // first-boot setup: cursive hello
const int SCREEN_NAME = 5;      //   ... type your name
const int SCREEN_PICK = 6;      //   ... pick your avatar
const int SCREEN_SETTINGS = 7;  // Settings app (Sign out)
const int SCREEN_PAST = 8;      // list of past posts
const int SCREEN_PAST_POST = 9; // one past post
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

// TheSlap's screens are landscape; home, setup and Settings are portrait
bool onSlapScreen() {
  return currentScreen == SCREEN_SLAP || currentScreen == SCREEN_COMPOSE || currentScreen == SCREEN_AVATAR ||
         currentScreen == SCREEN_PAST || currentScreen == SCREEN_PAST_POST;
}

// Rotation of the screen that's showing (for turning touches into screen coordinates)
int screenRotation() {
  return onSlapScreen() ? slap::ROTATION : home::ROTATION;
}

void showHello() {
  currentScreen = SCREEN_HELLO;
  tft.setRotation(onboard::ROTATION);
  onboard::showHello();
  Serial.println("Hello screen drawn.");
}

void showName() {
  currentScreen = SCREEN_NAME;
  tft.setRotation(onboard::ROTATION);
  onboard::showName();
  Serial.println("Name screen drawn.");
}

void showPick() {
  currentScreen = SCREEN_PICK;
  tft.setRotation(onboard::ROTATION);
  onboard::showPick();
  Serial.println("Avatar setup screen drawn.");
}

void showSettings() {
  currentScreen = SCREEN_SETTINGS;
  tft.setRotation(onboard::ROTATION);
  onboard::showSettings();
  Serial.println("Settings screen drawn.");
}

// ---- Saved profile (name + avatar) ------------------------------------------------

// Namespace "pearphone": setup (bool), name (string), avatar3 (bytes, one per
// slap::AvatarOpt), status (string, the last post), feeling (int, -1 = none) and
// past1 (bytes: up to slap::MAX_PAST slap::PastPost entries, oldest first).
// Older avatar saves are converted on load:
//   "avatar"  : gender, skin, hair, hair col, eyes, glasses (0-3), bow (0-3), shirt
//   "avatar2" : same order as now, but 6 hair colors (1 = a near-black that looked
//               like dark brown) and accessories None/Pink/Red/Lilac bow/Glasses
Preferences prefs;

uint8_t migrateHairCol(uint8_t c) { return c <= 1 ? 0 : c - 1; }   // dropped color 1 -> dark brown

void migrateAvatar1(const uint8_t* old, uint8_t* a) {
  memcpy(a, old, 5);                                          // gender .. eyes
  a[slap::OPT_HAIR_COL] = migrateHairCol(old[3]);
  a[slap::OPT_ACC] = old[6] ? slap::ACC_BOW : (old[5] ? slap::ACC_GLASSES : 0);   // any bow -> the pink bow, else glasses
  a[slap::OPT_SHIRT] = old[7];
  a[slap::OPT_BG] = 0;
}

void migrateAvatar2(const uint8_t* old, uint8_t* a) {
  memcpy(a, old, slap::NUM_OPTS);
  a[slap::OPT_HAIR_COL] = migrateHairCol(old[slap::OPT_HAIR_COL]);
  uint8_t acc = old[slap::OPT_ACC];
  a[slap::OPT_ACC] = (acc >= 1 && acc <= 3) ? slap::ACC_BOW : acc;   // any bow -> the pink bow
}

// Reads a byte array saved under key, if it exists with exactly len bytes
bool readBytes(const char* key, uint8_t* buf, size_t len) {
  return prefs.isKey(key) && prefs.getBytesLength(key) == len && prefs.getBytes(key, buf, len) == len;
}

// Loads the saved profile. Returns true if setup was finished before.
bool loadProfile() {
  if (!prefs.getBool("setup", false)) return false;
  String name = prefs.getString("name", slap::DEFAULT_NAME);
  strlcpy(slap::userName, name.c_str(), sizeof(slap::userName));

  uint8_t a[slap::NUM_OPTS], old[8];
  bool found = readBytes("avatar3", a, sizeof(a));
  if (!found && readBytes("avatar2", old, sizeof(old))) { migrateAvatar2(old, a); found = true; }
  if (!found && readBytes("avatar", old, sizeof(old)))  { migrateAvatar1(old, a); found = true; }
  if (found) {
    bool ok = true;
    for (int i = 0; i < slap::NUM_OPTS; i++) if (a[i] >= slap::OPT_COUNT[i]) ok = false;
    if (ok) memcpy(slap::avatar, a, sizeof(a));
  }
  String status = prefs.getString("status", "");
  strlcpy(slap::statusText, status.c_str(), sizeof(slap::statusText));
  int f = prefs.getInt("feeling", slap::NO_FEELING);
  slap::feeling = (f >= 0 && f < slap::NUM_FEELINGS) ? f : slap::NO_FEELING;
  loadPast();
  Serial.printf("Loaded profile: %s\n", slap::userName);
  return true;
}

void saveProfile() {
  prefs.putString("name", slap::userName);
  prefs.putBytes("avatar3", slap::avatar, slap::NUM_OPTS);
  for (const char* oldKey : { "avatar", "avatar2" })          // older layouts, no longer needed
    if (prefs.isKey(oldKey)) prefs.remove(oldKey);
  prefs.putBool("setup", true);
  Serial.printf("Saved profile: %s\n", slap::userName);
}

// Saves the Past list (every post, oldest first)
void savePast() {
  prefs.putBytes("past1", slap::past, slap::numPast * sizeof(slap::PastPost));
  Serial.printf("Saved %d past posts\n", slap::numPast);
}

// Loads the Past list. A save from before the Past screen existed has no list
// yet, so its current post (if any) becomes post #1.
void loadPast() {
  slap::numPast = 0;
  if (!prefs.isKey("past1")) {
    if (slap::statusText[0]) {
      slap::addPast(slap::statusText, slap::feeling);
      savePast();
    }
    return;
  }
  size_t len = prefs.getBytesLength("past1");
  if (len % sizeof(slap::PastPost) != 0 || len / sizeof(slap::PastPost) > (size_t)slap::MAX_PAST) return;
  if (len > 0 && prefs.getBytes("past1", slap::past, len) != len) return;
  slap::numPast = len / sizeof(slap::PastPost);
  for (int i = 0; i < slap::numPast; i++) {
    slap::PastPost& p = slap::past[i];
    p.text[sizeof(p.text) - 1] = 0;
    if (p.feeling < slap::NO_FEELING || p.feeling >= slap::NUM_FEELINGS) p.feeling = slap::NO_FEELING;
  }
}

// Saves the current post (text + feeling) so it is still there after a power-off
void saveStatus() {
  prefs.putString("status", slap::statusText);
  prefs.putInt("feeling", slap::feeling);
  Serial.printf("Saved status: %s\n", slap::statusText);
}

// Wipes the saved profile, puts the defaults back and starts setup again
void signOut() {
  prefs.clear();
  strlcpy(slap::userName, slap::DEFAULT_NAME, sizeof(slap::userName));
  memcpy(slap::avatar, slap::DEFAULT_AVATAR, slap::NUM_OPTS);
  slap::statusText[0] = 0;                                    // new user: empty card
  slap::feeling = slap::NO_FEELING;
  slap::numPast = 0;
  onboard::resetSetup();
  Serial.println("Signed out.");
  showHello();
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

void showPast() {
  currentScreen = SCREEN_PAST;
  tft.setRotation(slap::ROTATION);
  slap::showPast();
  Serial.println("Past posts screen drawn.");
}

// Back to the list from a post (same page as before)
void showPastAgain() {
  currentScreen = SCREEN_PAST;
  tft.setRotation(slap::ROTATION);
  slap::showPastAgain();
  Serial.println("Past posts screen drawn.");
}

void showPastPost() {
  currentScreen = SCREEN_PAST_POST;
  tft.setRotation(slap::ROTATION);
  slap::showPastPost();
  Serial.printf("Past post #%u drawn.\n", (unsigned)slap::past[slap::pastOpen].num);
}

void showAvatar() {
  currentScreen = SCREEN_AVATAR;
  tft.setRotation(slap::ROTATION);
  slap::showAvatarEditor();
  Serial.println("Avatar screen drawn.");
}

// Called with a touch point in screen coordinates. Returns true if it hit
// something (so the rest of that touch is ignored), false if it missed.
bool handleTap(int x, int y) {
  if (currentScreen == SCREEN_HOME) {
    int idx = home::appAt(x, y);
    if (idx < 0) return false;
    Serial.printf("  -> tapped %s\n", home::APPS[idx].name);
    if (strcmp(home::APPS[idx].name, APP_TO_OPEN) == 0) showSlap();
    else if (strcmp(home::APPS[idx].name, "Settings") == 0) showSettings();
    return true;
  } else if (currentScreen == SCREEN_HELLO) {
    if (onboard::helloTap(x, y) == onboard::HELLO_DONE) {
      Serial.println("  -> hello: continue");
      showName();
    }
    return true;                                   // any tap counts here
  } else if (currentScreen == SCREEN_NAME) {
    int result = onboard::nameTap(x, y);
    if (result == onboard::NAME_DONE) {
      Serial.printf("  -> name: %s\n", slap::userName);
      showPick();
    }
    return result != onboard::NAME_MISS;
  } else if (currentScreen == SCREEN_PICK) {
    int result = onboard::pickTap(x, y);
    if (result == onboard::PICK_BACK) {
      Serial.println("  -> avatar setup: back");
      showName();
    } else if (result == onboard::PICK_DONE) {
      Serial.println("  -> avatar setup: done");
      saveProfile();
      showHome();
    }
    return result != onboard::PICK_MISS;
  } else if (currentScreen == SCREEN_SETTINGS) {
    int result = onboard::settingsTap(x, y);
    if (result == onboard::SETTINGS_HOME) {
      Serial.println("  -> settings: home");
      showHome();
    } else if (result == onboard::SETTINGS_SIGN_OUT) {
      Serial.println("  -> settings: sign out");
      signOut();
    } else if (result == onboard::SETTINGS_NONE) {
      Serial.println("  -> settings: sign out armed (tap again)");
    }
    return result != onboard::SETTINGS_MISS;
  } else if (currentScreen == SCREEN_SLAP) {
    if (slap::homeTabAt(x, y)) {
      Serial.println("  -> tapped Home tab");
      showHome();
      return true;
    } else if (slap::newTabAt(x, y)) {
      Serial.println("  -> tapped New tab");
      showCompose();
      return true;
    } else if (slap::clearTabAt(x, y)) {
      Serial.println("  -> tapped Clear tab");
      slap::clearStatus();
      saveStatus();
      return true;
    } else if (slap::pastTabAt(x, y)) {
      Serial.println("  -> tapped Past tab");
      showPast();
      return true;
    } else if (slap::avatarTabAt(x, y)) {
      Serial.println("  -> tapped avatar / UPDATE");
      showAvatar();
      return true;
    }
    return false;
  } else if (currentScreen == SCREEN_AVATAR) {
    int result = slap::avatarTap(x, y);
    if (result == slap::AVATAR_SAVED) {
      Serial.println("  -> saved avatar");
      saveProfile();
      showSlap();
    } else if (result == slap::AVATAR_CANCEL) {
      Serial.println("  -> cancelled avatar changes");
      showSlap();
    }
    return result != slap::AVATAR_MISS;
  } else if (currentScreen == SCREEN_PAST) {
    int result = slap::pastTap(x, y);
    if (result == slap::PAST_BACK) {
      Serial.println("  -> past: back");
      showSlap();
    } else if (result == slap::PAST_OPEN) {
      Serial.printf("  -> past: open post #%u\n", (unsigned)slap::past[slap::pastOpen].num);
      showPastPost();
    } else if (result == slap::PAST_NONE) {
      Serial.printf("  -> past: page %d\n", slap::pastPage + 1);
    }
    return result != slap::PAST_MISS;
  } else if (currentScreen == SCREEN_PAST_POST) {
    if (slap::pastPostTap(x, y) == slap::PAST_POST_MISS) return false;
    Serial.println("  -> past post: back");
    showPastAgain();
    return true;
  } else if (currentScreen == SCREEN_COMPOSE) {
    int result = slap::composeTap(x, y);
    if (result == slap::COMPOSE_POSTED) {
      Serial.printf("  -> posted: %s\n", slap::statusText);
      saveStatus();
      savePast();
      showSlap();
    } else if (result == slap::COMPOSE_CANCEL) {
      Serial.println("  -> cancelled post");
      showSlap();
    }
    return result != slap::COMPOSE_MISS;
  }
  return false;
}

// ---- Main ----------------------------------------------------------------------

// Why the board last reset. "BROWNOUT" means the power dipped: use a better
// USB cable/port or shorter power wires.
const char* resetReason() {
  switch (esp_reset_reason()) {
    case ESP_RST_POWERON:  return "power on";
    case ESP_RST_SW:       return "software reset / upload";
    case ESP_RST_PANIC:    return "CRASH (exception)";
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:      return "WATCHDOG (code got stuck)";
    case ESP_RST_BROWNOUT: return "BROWNOUT (power dipped)";
    case ESP_RST_USB:      return "USB reset";
    default:               return "other";
  }
}

// Power-on splash: a small white lightning bolt in the middle of a black screen.
#define BOOT_SPLASH_MS 1500   // how long the bolt stays up (counted from when it's drawn)

void showBootBolt() {
  tft.setRotation(home::ROTATION);          // portrait 240x320
  tft.fillScreen(ILI9341_BLACK);
  uint16_t white = ILI9341_WHITE;
  // Bolt outline, clockwise from the top tip, centred on (120,160), ~68 px tall
  const int16_t x1 = 131, y1 = 126;   // top tip
  const int16_t x2 = 104, y2 = 165;   // upper-left corner
  const int16_t x3 = 118, y3 = 165;   // inner notch (left)
  const int16_t x4 = 109, y4 = 194;   // bottom tip
  const int16_t x5 = 136, y5 = 153;   // lower-right corner
  const int16_t x6 = 122, y6 = 153;   // inner notch (right)
  tft.fillTriangle(x1, y1, x2, y2, x6, y6, white);   // upper half
  tft.fillTriangle(x2, y2, x3, y3, x6, y6, white);
  tft.fillTriangle(x6, y6, x3, y3, x5, y5, white);   // lower half
  tft.fillTriangle(x3, y3, x4, y4, x5, y5, white);
}

void setup() {
  // Bring the display up first so the bolt appears as soon as there's power
  SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);
  tft.begin(SPI_SPEED);
  tft.invertDisplay(INVERT_COLORS);
  showBootBolt();
  uint32_t splashStart = millis();

  Serial.begin(115200);
  delay(1500);                      // give the Serial Monitor time to connect (bolt is showing)
  Serial.printf("\n--- Pear Phone boot (last reset: %s) ---\n", resetReason());
  Serial.println("Display started, boot bolt shown.");
  touchBegin();

  prefs.begin("pearphone", false);
  while (millis() - splashStart < BOOT_SPLASH_MS) delay(10);   // finish the splash
  if (loadProfile()) showHome();
  else showHello();                 // first boot (or after Sign out): run setup
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
    int rotation = screenRotation();
    int x, y;
    mapTouch(rx, ry, rotation, x, y);
    Serial.printf("tap raw=(%d,%d) screen=(%d,%d)\n", rx, ry, x, y);
    tapDone = handleTap(x, y);
  }
  wasTouching = touching;

  if (onSlapScreen()) slap::update();                          // blinking sparkles (Slap, new post, avatar)
  else if (currentScreen == SCREEN_HELLO) onboard::helloUpdate();  // writes "hello"
  else if (currentScreen == SCREEN_SETTINGS) onboard::settingsUpdate();
  delay(10);
}
