// First-boot setup and the Settings app. Portrait 240x320, same rotation as
// the home screen, on the home screen's pink glow.
//
//   1. Hello:  "hello" writes itself in cursive. Tap to continue.
//   2. Name:   type your name on a portrait keyboard, then Continue.
//   3. Avatar: pick your look (same options as TheSlap's avatar editor), Done.
// phone.ino saves the name and avatar to flash after step 3, so this only runs
// once. Settings shows who's signed in and has Sign out, which wipes the saved
// profile and starts setup again.
//
// Uses home:: (background, border) and slap:: (name, avatar, colors, helpers),
// so it's included after both headers.
#pragma once

namespace onboard {

constexpr int ROTATION = home::ROTATION;
constexpr int SCREEN_W = home::SCREEN_W;   // 240
constexpr int SCREEN_H = home::SCREEN_H;   // 320

const uint16_t COL_EDGE    = C565(255,  92,  92);   // coral border
const uint16_t COL_SIGNOUT = C565(220,  40,  60);

// ---- Shared helpers ----------------------------------------------------------------

void drawBase() {
  tft.fillScreen(ILI9341_BLACK);   // outside the border stays black
  home::drawBackground();
  home::drawBorder(COL_EDGE);
}

// Text centered across the screen with a 1 px black shadow. Size 2+ is faux-bold.
void centeredText(const char* s, int y, uint8_t size, uint16_t color) {
  int w = strlen(s) * 6 * size - size;
  int x = (SCREEN_W - w) / 2;
  int passes = size >= 2 ? 2 : 1;
  tft.setTextSize(size);
  tft.setTextColor(ILI9341_BLACK);
  for (int p = 0; p < passes; p++) { tft.setCursor(x + 1 + p, y + 1); tft.print(s); }
  tft.setTextColor(color);
  for (int p = 0; p < passes; p++) { tft.setCursor(x + p, y); tft.print(s); }
}

void drawPill(int x, int y, int w, int h, const char* label, uint16_t fill, uint8_t size = 1) {
  tft.fillRoundRect(x, y, w, h, h / 2, fill);
  slap::centerIn(x, y, w, h, label, size, ILI9341_WHITE);
}

// Called before a fresh run of the setup screens (first boot or after Sign out)
char nameBuf[slap::MAX_NAME + 1] = "";
int nameLen = 0;
bool pickFresh = true;   // avatar screen not opened yet in this run

void resetSetup() {
  nameBuf[0] = 0;
  nameLen = 0;
  pickFresh = true;
}

// ---- 1. Hello -----------------------------------------------------------------------
//
// "hello" as one continuous cursive stroke: cubic Bézier segments (start, two
// control points, end) in a 184 x 86 design box, baseline at y 80. Each segment
// starts where the last one ended. Tweak the points to change the lettering.

const uint8_t HELLO_PATH[][8] = {
  // h: lead-in, tall loop, stem down, hump
  {   2, 80,  18, 70,  36, 42,  36, 20 },
  {  36, 20,  36,  6,  24,  4,  23, 18 },
  {  23, 18,  22, 40,  20, 62,  18, 80 },
  {  18, 80,  24, 60,  34, 52,  41, 56 },
  {  41, 56,  47, 60,  44, 72,  46, 78 },
  {  46, 78,  48, 84,  56, 82,  62, 72 },
  // e
  {  62, 72,  72, 66,  80, 56,  74, 52 },
  {  74, 52,  66, 48,  58, 62,  62, 74 },
  {  62, 74,  66, 84,  78, 82,  86, 72 },
  // first l
  {  86, 72,  98, 56, 106, 28, 102, 12 },
  { 102, 12,  98,  2,  88,  8,  90, 26 },
  {  90, 26,  92, 46,  92, 70,  98, 78 },
  {  98, 78, 102, 84, 110, 80, 116, 70 },
  // second l
  { 116, 70, 128, 54, 132, 28, 128, 12 },
  { 128, 12, 124,  2, 114,  8, 116, 26 },
  { 116, 26, 118, 46, 118, 70, 124, 78 },
  { 124, 78, 128, 84, 136, 80, 142, 70 },
  // o, then a little tail
  { 142, 70, 144, 56, 158, 50, 164, 60 },
  { 164, 60, 168, 70, 160, 82, 150, 80 },
  { 150, 80, 140, 78, 140, 62, 152, 56 },
  { 152, 56, 160, 52, 170, 54, 182, 50 },
};
constexpr int HELLO_SEGS = sizeof(HELLO_PATH) / sizeof(HELLO_PATH[0]);

constexpr float HELLO_SCALE = 0.86f;          // design units -> pixels (word is ~158 px wide)
constexpr int HELLO_DESIGN_W = 184, HELLO_DESIGN_H = 86;
constexpr int HELLO_CY = 150;                 // vertical center of the word
const int HELLO_X0 = (SCREEN_W - (int)(HELLO_DESIGN_W * HELLO_SCALE)) / 2;
const int HELLO_Y0 = HELLO_CY - (int)(HELLO_DESIGN_H * HELLO_SCALE / 2);
constexpr uint32_t HELLO_MS = 2600;           // time to write the whole word
constexpr int PEN_R = 2;                      // pen radius (stroke is 5 px wide)
constexpr float PEN_STEP = 1.2f;              // pixels between pen dots

// What helloTap() did
constexpr int HELLO_NONE = 0;   // finished the animation early, stay here
constexpr int HELLO_DONE = 1;   // go to the name screen

int helloSeg = 0, helloStep = 0, helloSteps = 0;
int helloDots = 0, helloTotal = 0;
uint32_t helloStart = 0;
bool helloFinished = false;

// Point at t (0..1) along segment s, in screen pixels
void helloPoint(int s, float t, int& x, int& y) {
  const uint8_t* p = HELLO_PATH[s];
  float u = 1 - t;
  float a = u * u * u, b = 3 * u * u * t, c = 3 * u * t * t, d = t * t * t;
  float px = a * p[0] + b * p[2] + c * p[4] + d * p[6];
  float py = a * p[1] + b * p[3] + c * p[5] + d * p[7];
  x = HELLO_X0 + (int)lroundf(px * HELLO_SCALE);
  y = HELLO_Y0 + (int)lroundf(py * HELLO_SCALE);
}

// Pen dots for segment s (from the length of its control polygon)
int helloStepsFor(int s) {
  const uint8_t* p = HELLO_PATH[s];
  float len = 0;
  for (int i = 0; i < 3; i++) {
    float dx = p[2 * i + 2] - p[2 * i], dy = p[2 * i + 3] - p[2 * i + 1];
    len += sqrtf(dx * dx + dy * dy);
  }
  int n = (int)(len * HELLO_SCALE / PEN_STEP);
  return n < 4 ? 4 : n;
}

void stampPen(int x, int y) {
  float f = helloTotal ? (float)helloDots / helloTotal : 1;
  tft.fillCircle(x, y, PEN_R, home::mix(0xFFC8E6, 0xFFFFFF, f));   // pale pink -> white
  helloDots++;
}

// Draws the next pen dot. Returns false when the word is complete.
bool helloNextDot() {
  if (helloSeg >= HELLO_SEGS) return false;
  if (helloStep == 0) helloSteps = helloStepsFor(helloSeg);
  helloStep++;
  int x, y;
  helloPoint(helloSeg, (float)helloStep / helloSteps, x, y);
  stampPen(x, y);
  if (helloStep >= helloSteps) { helloSeg++; helloStep = 0; }
  return true;
}

void finishHello() {
  while (helloNextDot()) {}
  helloFinished = true;
  centeredText("Tap to continue", 268, 1, ILI9341_WHITE);
}

void showHello() {
  drawBase();
  helloSeg = helloStep = helloDots = 0;
  helloTotal = 0;
  for (int s = 0; s < HELLO_SEGS; s++) helloTotal += helloStepsFor(s);
  helloFinished = false;
  int x, y;
  helloPoint(0, 0, x, y);
  stampPen(x, y);                  // starting dot
  helloStart = millis();
}

// Call every loop() while the hello screen is showing: writes the word over HELLO_MS
void helloUpdate() {
  if (helloFinished) return;
  uint32_t elapsed = millis() - helloStart;
  int due = elapsed >= HELLO_MS ? helloTotal : (int)((uint64_t)helloTotal * elapsed / HELLO_MS);
  while (helloDots < due) {
    if (!helloNextDot()) break;
  }
  if (helloSeg >= HELLO_SEGS) finishHello();
}

// Any tap counts: the first one skips to the end of the animation if it's
// still drawing, the next one moves on.
int helloTap(int x, int y) {
  if (!helloFinished) { finishHello(); return HELLO_NONE; }
  return HELLO_DONE;
}

// ---- 2. Name ------------------------------------------------------------------------
//
// Title, a name field, a portrait keyboard (letters, shift, backspace, space)
// and a Continue pill that lights up once something's typed.

constexpr int FIELD_X = 20, FIELD_Y = 78, FIELD_W = 200, FIELD_H = 36;
constexpr int KB_X = 6, KB_Y = 128, KB_W = 228, KB_H = 118;          // keyboard panel
constexpr int NK_W = 19, NK_H = 22, NK_GAP = 3;
const int NK_ROW_Y[4] = { KB_Y + 8, KB_Y + 34, KB_Y + 60, KB_Y + 86 };
constexpr int NK_SIDE_W = 28;                                          // shift / backspace
constexpr int CONT_X = 60, CONT_Y = 258, CONT_W = 120, CONT_H = 26;   // Continue

// What nameTap() did
constexpr int NAME_MISS = -1;   // the tap didn't hit anything
constexpr int NAME_NONE = 0;    // handled, stay here
constexpr int NAME_DONE = 1;    // name saved into slap::userName, go to the avatar screen

constexpr char NK_SHIFT = 1, NK_BACK = '\b';

slap::Key nkeys[32];
int numNKeys = 0;
bool nameShift = true;          // one-shot capitals

void addNKey(int x, int y, int w, char code) {
  nkeys[numNKeys++] = { (int16_t)x, (int16_t)y, (int16_t)w, code };
}

void addNRow(const char* s, int y) {
  int n = strlen(s);
  int x = (SCREEN_W - (n * NK_W + (n - 1) * NK_GAP)) / 2;
  for (int i = 0; i < n; i++, x += NK_W + NK_GAP) addNKey(x, y, NK_W, s[i]);
}

void initNameKeys() {
  numNKeys = 0;
  addNRow("QWERTYUIOP", NK_ROW_Y[0]);
  addNRow("ASDFGHJKL", NK_ROW_Y[1]);
  addNKey(11, NK_ROW_Y[2], NK_SIDE_W, NK_SHIFT);
  addNRow("ZXCVBNM", NK_ROW_Y[2]);
  addNKey(SCREEN_W - 11 - NK_SIDE_W, NK_ROW_Y[2], NK_SIDE_W, NK_BACK);
  addNKey(50, NK_ROW_Y[3], SCREEN_W - 100, ' ');
}

void drawNKey(const slap::Key& k, bool pressed) {
  bool special = (k.code == NK_SHIFT || k.code == NK_BACK);
  uint16_t fill = special ? COL_KEY_SPEC : COL_KEY;
  if (k.code == NK_SHIFT && nameShift) fill = COL_KEY;    // lit up while on
  if (pressed) fill = COL_KEY_DOWN;
  uint16_t ink = (pressed || fill == COL_KEY_SPEC) ? ILI9341_WHITE : COL_NAME;

  tft.fillRoundRect(k.x, k.y + 1, k.w, NK_H, 4, COL_KEY_SHAD);   // shadow peeks out below
  tft.fillRoundRect(k.x, k.y, k.w, NK_H, 4, fill);

  int cx = k.x + k.w / 2, cy = k.y + NK_H / 2;
  if (k.code == NK_SHIFT) {                                       // up arrow
    tft.fillTriangle(cx - 6, cy, cx + 6, cy, cx, cy - 6, ink);
    tft.fillRect(cx - 3, cy, 7, 5, ink);
  } else if (k.code == NK_BACK) {                                 // tag shape with an x
    tft.drawLine(cx - 9, cy, cx - 4, cy - 5, ink);
    tft.drawLine(cx - 9, cy, cx - 4, cy + 5, ink);
    tft.drawFastHLine(cx - 4, cy - 5, 13, ink);
    tft.drawFastHLine(cx - 4, cy + 5, 13, ink);
    tft.drawFastVLine(cx + 8, cy - 5, 11, ink);
    tft.drawLine(cx - 1, cy - 2, cx + 4, cy + 2, ink);
    tft.drawLine(cx - 1, cy + 2, cx + 4, cy - 2, ink);
  } else if (k.code == ' ') {
    slap::centerIn(k.x, k.y, k.w, NK_H, "space", 1, ink);
  } else {
    char s[2] = { nameShift ? k.code : (char)(k.code + 32), 0 };
    slap::centerIn(k.x, k.y, k.w, NK_H, s, 2, ink);
  }
}

void drawNameKeys() {
  for (int i = 0; i < numNKeys; i++) drawNKey(nkeys[i], false);
}

// Typed name (or the hint) and the cursor, inside the field
void drawNameText() {
  tft.fillRect(FIELD_X + 4, FIELD_Y + 3, FIELD_W - 8, FIELD_H - 6, COL_CARD);
  int tx = FIELD_X + 10, ty = FIELD_Y + 11;
  if (nameLen == 0) {
    tft.setTextSize(2);
    tft.setTextColor(COL_HINT);
    tft.setCursor(tx, ty);
    tft.print("Your name");
  } else {
    slap::boldText(nameBuf, tx, ty, 2, COL_NAME);
  }
  int cx = tx + nameLen * 12 + (nameLen ? 1 : -2);
  tft.fillRect(cx, ty - 2, 2, 19, COL_PINK);
}

void drawContinue(bool pressed = false) {
  uint16_t fill = pressed ? COL_KEY_DOWN : (nameLen > 0 ? COL_PINK : COL_GRAY);
  drawPill(CONT_X, CONT_Y, CONT_W, CONT_H, "Continue", fill, 2);
}

void showName() {
  nameShift = (nameLen == 0 || nameBuf[nameLen - 1] == ' ');
  initNameKeys();
  drawBase();
  centeredText("What's your name?", 46, 2, ILI9341_WHITE);

  tft.fillRoundRect(FIELD_X, FIELD_Y, FIELD_W, FIELD_H, 6, COL_CARD);
  tft.drawRoundRect(FIELD_X - 1, FIELD_Y - 1, FIELD_W + 2, FIELD_H + 2, 7, COL_PINK_EDG);
  drawNameText();

  tft.fillRoundRect(KB_X, KB_Y, KB_W, KB_H, 8, COL_KB_BG);
  drawNameKeys();
  drawContinue();
}

void pressNKey(const slap::Key& k) {
  drawNKey(k, true);              // quick flash so you can see the press
  delay(60);

  bool wasEmpty = (nameLen == 0);
  bool redrawKeys = false;
  char c = k.code;
  if (c == NK_SHIFT) {
    nameShift = !nameShift;
    redrawKeys = true;
  } else if (c == NK_BACK) {
    if (nameLen > 0) nameBuf[--nameLen] = 0;
    bool wantShift = (nameLen == 0 || nameBuf[nameLen - 1] == ' ');
    if (wantShift != nameShift) { nameShift = wantShift; redrawKeys = true; }
  } else if (nameLen < slap::MAX_NAME) {
    if (c == ' ') {
      if (nameLen == 0 || nameBuf[nameLen - 1] == ' ') c = 0;     // no leading or double spaces
      else if (!nameShift) { nameShift = true; redrawKeys = true; } // capitalize the next word
    } else if (nameShift) {
      nameShift = false;            // one-shot: back to lowercase after this letter
      redrawKeys = true;
    } else {
      c += 32;                      // lowercase
    }
    if (c) { nameBuf[nameLen++] = c; nameBuf[nameLen] = 0; }
  }

  if (redrawKeys) drawNameKeys();
  else drawNKey(k, false);
  drawNameText();
  if (wasEmpty != (nameLen == 0)) drawContinue();
}

// Handles a tap on the name screen. Returns NAME_DONE when it's time for the
// avatar screen, NAME_NONE when the tap did something here, NAME_MISS otherwise.
int nameTap(int x, int y) {
  if (slap::inRect(x, y, CONT_X - 10, CONT_Y - 6, CONT_X + CONT_W + 10, CONT_Y + CONT_H + 8)) {
    if (nameLen == 0) return NAME_NONE;                    // nothing typed yet
    while (nameLen > 0 && nameBuf[nameLen - 1] == ' ') nameBuf[--nameLen] = 0;   // trailing space
    strcpy(slap::userName, nameBuf);
    drawContinue(true);
    return NAME_DONE;
  }
  for (int i = 0; i < numNKeys; i++) {
    const slap::Key& k = nkeys[i];
    // Hit box reaches into the gaps so taps between keys still count
    if (slap::inRect(x, y, k.x - 1, k.y - 2, k.x + k.w, k.y + NK_H + 2)) {
      pressNKey(k);
      return NAME_NONE;
    }
  }
  return NAME_MISS;
}

// ---- 3. Avatar ----------------------------------------------------------------------
//
// Preview at the top with Back (left) and Done (right) beside it, and TheSlap's
// options card under it (slap::drawOptCard / slap::optTap).

constexpr int PICK_AV_X = 80, PICK_AV_Y = 50, PICK_AV_S = 80;
constexpr int PICK_OPT_X = 46, PICK_OPT_Y = 142, PICK_OPT_W = 148;
constexpr int PICK_PILL_Y = 80, PICK_PILL_W = 50, PICK_PILL_H = 20;
constexpr int PICK_BACK_X = 18, PICK_DONE_X = SCREEN_W - 18 - PICK_PILL_W;

// What pickTap() did
constexpr int PICK_MISS = -1;
constexpr int PICK_NONE = 0;
constexpr int PICK_BACK = 1;    // back to the name screen (the choices so far are kept)
constexpr int PICK_DONE = 2;    // copied into slap::avatar, setup finished

void drawPickPreview() {
  slap::drawAvatar(PICK_AV_X, PICK_AV_Y, PICK_AV_S, slap::editAvatar);
}

void showPick() {
  if (pickFresh) {               // first visit this run: start from the current avatar
    memcpy(slap::editAvatar, slap::avatar, slap::NUM_OPTS);
    pickFresh = false;
  }
  drawBase();
  centeredText("Make your avatar", 24, 2, ILI9341_WHITE);
  tft.fillRoundRect(PICK_AV_X - 4, PICK_AV_Y - 4, PICK_AV_S + 8, PICK_AV_S + 8, 6, COL_PINK);
  drawPickPreview();
  drawPill(PICK_BACK_X, PICK_PILL_Y, PICK_PILL_W, PICK_PILL_H, "Back", COL_KEY_SPEC);
  drawPill(PICK_DONE_X, PICK_PILL_Y, PICK_PILL_W, PICK_PILL_H, "Done", COL_PINK);
  slap::drawOptCard(PICK_OPT_X, PICK_OPT_Y, PICK_OPT_W);
}

int pickTap(int x, int y) {
  int top = PICK_PILL_Y - 12, bot = PICK_PILL_Y + PICK_PILL_H + 12;
  if (slap::inRect(x, y, 4, top, PICK_AV_X - 6, bot)) {
    drawPill(PICK_BACK_X, PICK_PILL_Y, PICK_PILL_W, PICK_PILL_H, "Back", COL_KEY_DOWN);
    return PICK_BACK;
  }
  if (slap::inRect(x, y, PICK_AV_X + PICK_AV_S + 6, top, SCREEN_W - 4, bot)) {
    drawPill(PICK_DONE_X, PICK_PILL_Y, PICK_PILL_W, PICK_PILL_H, "Done", COL_KEY_DOWN);
    memcpy(slap::avatar, slap::editAvatar, slap::NUM_OPTS);
    return PICK_DONE;
  }
  if (slap::optTap(x, y, PICK_OPT_X, PICK_OPT_Y, PICK_OPT_W)) {
    drawPickPreview();
    return PICK_NONE;
  }
  return PICK_MISS;
}

// ---- Settings -----------------------------------------------------------------------
//
// Who's signed in (avatar + name), Sign out, and Home. Sign out needs a second
// tap within SIGNOUT_CONFIRM_MS so it can't happen by accident.

constexpr int SET_CARD_X = 20, SET_CARD_Y = 66, SET_CARD_W = 200, SET_CARD_H = 118;
constexpr int SET_AV_S = 64, SET_AV_X = (SCREEN_W - SET_AV_S) / 2, SET_AV_Y = 78;
constexpr int SIGNOUT_X = 40, SIGNOUT_Y = 204, SIGNOUT_W = 160, SIGNOUT_H = 30;
constexpr int SET_HOME_X = 80, SET_HOME_Y = 250, SET_HOME_W = 80, SET_HOME_H = 24;
constexpr uint32_t SIGNOUT_CONFIRM_MS = 3000;

// What settingsTap() did
constexpr int SETTINGS_MISS = -1;
constexpr int SETTINGS_NONE = 0;
constexpr int SETTINGS_HOME = 1;
constexpr int SETTINGS_SIGN_OUT = 2;   // confirmed: phone.ino wipes the profile

bool signOutArmed = false;
uint32_t signOutArmedAt = 0;

void drawSignOut(bool pressed = false) {
  uint16_t fill = pressed ? COL_KEY_DOWN : COL_SIGNOUT;
  if (signOutArmed) drawPill(SIGNOUT_X, SIGNOUT_Y, SIGNOUT_W, SIGNOUT_H, "Tap again to sign out", fill, 1);
  else              drawPill(SIGNOUT_X, SIGNOUT_Y, SIGNOUT_W, SIGNOUT_H, "Sign out", fill, 2);
}

void showSettings() {
  signOutArmed = false;
  drawBase();
  centeredText("Settings", 36, 2, ILI9341_WHITE);

  tft.fillRoundRect(SET_CARD_X, SET_CARD_Y, SET_CARD_W, SET_CARD_H, 8, COL_CARD);
  tft.drawRoundRect(SET_CARD_X - 1, SET_CARD_Y - 1, SET_CARD_W + 2, SET_CARD_H + 2, 9, COL_PINK_EDG);
  tft.fillRoundRect(SET_AV_X - 3, SET_AV_Y - 3, SET_AV_S + 6, SET_AV_S + 6, 5, COL_PINK);
  slap::drawAvatar(SET_AV_X, SET_AV_Y, SET_AV_S, slap::avatar);

  char upper[slap::MAX_NAME + 1];
  int n = 0;
  for (const char* p = slap::userName; *p && n < slap::MAX_NAME; p++) upper[n++] = toupper((unsigned char)*p);
  upper[n] = 0;
  int w = n * 12 - 2;
  slap::boldText(upper, (SCREEN_W - w) / 2, SET_AV_Y + SET_AV_S + 12, 2, COL_NAME);

  drawSignOut();
  drawPill(SET_HOME_X, SET_HOME_Y, SET_HOME_W, SET_HOME_H, "Home", COL_KEY_SPEC);
}

// Call every loop() while Settings is showing: un-arms Sign out after a while
void settingsUpdate() {
  if (signOutArmed && millis() - signOutArmedAt > SIGNOUT_CONFIRM_MS) {
    signOutArmed = false;
    drawSignOut();
  }
}

int settingsTap(int x, int y) {
  if (slap::inRect(x, y, SIGNOUT_X - 6, SIGNOUT_Y - 6, SIGNOUT_X + SIGNOUT_W + 6, SIGNOUT_Y + SIGNOUT_H + 5)) {
    if (signOutArmed) {
      drawSignOut(true);
      return SETTINGS_SIGN_OUT;
    }
    signOutArmed = true;
    signOutArmedAt = millis();
    drawSignOut();
    return SETTINGS_NONE;
  }
  if (slap::inRect(x, y, SET_HOME_X - 10, SET_HOME_Y - 5, SET_HOME_X + SET_HOME_W + 10, SET_HOME_Y + SET_HOME_H + 8)) {
    drawPill(SET_HOME_X, SET_HOME_Y, SET_HOME_W, SET_HOME_H, "Home", COL_KEY_DOWN);
    return SETTINGS_HOME;
  }
  return SETTINGS_MISS;
}

} // namespace onboard
