// TheSlap profile screen (landscape 320x240). Copied from ../slap-dot-com.ino.
// Everything is inside `namespace slap` so names don't clash with the home screen.
#pragma once

namespace slap {

constexpr int ROTATION = 1;   // 1 = content turned -90° from the old portrait view (3 = the other way)
#define STARS           true      // static background stars
#define BLINK_SPARKLES  true      // blinking sparkles (false = they stay on, no blinking)
#define BLINK_SLOWNESS  2.5f      // 1.0 = old speed, 2.5 = 2.5x slower, 4.0 = very slow
#define NUM_SPARKLES    16        // includes the big gold one at bottom left

constexpr int CORNER_RADIUS =  70;        // same border as before
constexpr int LINE_THICKNESS = 3;

constexpr int SCREEN_W = 320;             // landscape drawing space
constexpr int SCREEN_H = 240;

// 16-bit color from 8-bit RGB, usable in constants
#define C565(r,g,b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

#define COL_BORDER   C565(255,  92,  92)
#define COL_PILL     C565( 38,  14,  72)
#define COL_PINK     C565(240,  53, 154)
#define COL_PINK_EDG C565(225,  80, 165)
#define COL_CARD     C565(194, 240, 255)
#define COL_STRIP    C565(124, 224, 245)
#define COL_NAME     C565( 35,  20, 150)
#define COL_TEXT     C565( 15,  15,  45)
#define COL_FEEL     C565(224,  88,  58)
#define COL_MOOD     C565( 90,  40, 224)
#define COL_UPDATE   C565(255, 200,  80)
#define COL_ICON     C565(235, 235, 240)
#define COL_ICON_DK  C565( 60,  60,  70)
#define COL_CYAN     C565( 92, 224, 255)
#define COL_GRAY     C565(170, 170, 180)
#define COL_GOLD     C565(255, 215, 100)
#define COL_LAVENDER C565(215, 205, 255)
#define COL_HINT     C565(120, 140, 165)   // placeholder text ("What's on your mind?", ...)


enum PanelShape { WEDGE, ROUND_START, ROUND_END };

#define MAX_POST 140   // longest post, in characters
char statusText[MAX_POST + 1] = "";   // the posted message (empty: the card shows a hint instead)

// Every post, for the Past screen: the newest MAX_PAST, oldest first. Saved in
// flash by savePast() in phone.ino (key "past1"), so changing PastPost means a
// new key. Post numbers keep counting up after old posts are dropped.
constexpr int MAX_PAST = 20;
struct PastPost {
  uint16_t num;                   // 1 = the first post
  int8_t   feeling;               // NO_FEELING (-1) or an index into FEELINGS
  char     text[MAX_POST + 1];
};
PastPost past[MAX_PAST];
int numPast = 0;

// Adds a post as the newest one, dropping the oldest when the list is full
void addPast(const char* text, int feel) {
  uint16_t num = numPast ? past[numPast - 1].num + 1 : 1;
  if (numPast == MAX_PAST) {
    memmove(&past[0], &past[1], (MAX_PAST - 1) * sizeof(PastPost));
    numPast--;
  }
  PastPost& p = past[numPast++];
  p.num = num;
  p.feeling = feel;
  strlcpy(p.text, text, sizeof(p.text));
}

// The user's name, set on the first-boot setup screens (setup_screen.h).
// 12 characters + ":" at bold size 2 still fits the 196 px profile card.
constexpr int MAX_NAME = 12;
const char* const DEFAULT_NAME = "LOPI DOPI";
char userName[MAX_NAME + 1] = "LOPI DOPI";

// ---- Rounded-rectangle clipping ---------------------------------------------

// Inner edge of the border on row y
void rowExtent(int y, int& xl, int& xr) {
  int m = LINE_THICKNESS;
  int r = CORNER_RADIUS - LINE_THICKNESS;
  int top = m + r, bot = SCREEN_H - 1 - m - r;
  int dyc = 0;
  if (y < top) dyc = top - y;
  else if (y > bot) dyc = y - bot;
  int inset = 0;
  if (dyc > 0) {
    float v = (float)r * r - (float)dyc * dyc;
    inset = r - (int)floorf(sqrtf(v < 0 ? 0 : v));
  }
  xl = m + inset;
  xr = SCREEN_W - 1 - m - inset;
}

bool insideBorder(int x, int y, int margin) {
  if (y < LINE_THICKNESS + margin || y > SCREEN_H - 1 - LINE_THICKNESS - margin) return false;
  int xl, xr;
  rowExtent(y, xl, xr);
  return x >= xl + margin && x <= xr - margin;
}

// ---- Background -----------------------------------------------------------------

// Background gradient color on row y (also used to "erase" blinking sparkles)
uint16_t bgColor(int y) {
  float f = (float)y / (SCREEN_H - 1);
  return tft.color565(20 + (int)(40 * f), 8 + (int)(14 * f), 50 + (int)(50 * f));
}

// Gradient + stars. Pass a rectangle (x0, y0)-(x1, y1) to redraw just that part:
// the stars come from a fixed seed, so a partial redraw matches the full one.
void drawBackground(int x0 = 0, int y0 = 0, int x1 = SCREEN_W - 1, int y1 = SCREEN_H - 1) {
  for (int y = max(y0, LINE_THICKNESS); y <= min(y1, SCREEN_H - 1 - LINE_THICKNESS); y++) {
    int xl, xr;
    rowExtent(y, xl, xr);
    xl = max(xl, x0);
    xr = min(xr, x1);
    if (xr >= xl) tft.drawFastHLine(xl, y, xr - xl + 1, bgColor(y));
  }

#if STARS
  auto star = [&](int x, int y, uint16_t c) {
    if (x >= x0 && x <= x1 && y >= y0 && y <= y1) tft.drawPixel(x, y, c);
  };
  randomSeed(11);   // fixed seed: same stars every run
  for (int i = 0; i < 70; i++) {
    int x = random(8, 312), y = random(6, 234);
    if (!insideBorder(x, y, 4)) continue;
    uint16_t c = (i % 5 == 0) ? COL_GOLD : COL_LAVENDER;
    star(x, y, c);
    if (i % 7 == 0) {
      star(x - 1, y, c); star(x + 1, y, c);
      star(x, y - 1, c); star(x, y + 1, c);
    }
  }
#endif
}

void drawBorder(uint16_t color) {
  for (int t = 0; t < LINE_THICKNESS; t++) {
    tft.drawRoundRect(t, t, SCREEN_W - 2 * t, SCREEN_H - 2 * t, CORNER_RADIUS - t, color);
  }
}

// ---- Text helpers ---------------------------------------------------------------

// Faux-bold: prints the text twice, 1 px apart
void boldText(const char* s, int x, int y, uint8_t size, uint16_t color) {
  tft.setTextSize(size);
  tft.setTextColor(color);   // transparent background
  tft.setCursor(x, y);     tft.print(s);
  tft.setCursor(x + 1, y); tft.print(s);
}

// "NAME:" in capitals, like the show's cards
void drawName(int x, int y) {
  char s[MAX_NAME + 2];
  int n = 0;
  for (const char* p = userName; *p && n < MAX_NAME; p++) s[n++] = toupper((unsigned char)*p);
  s[n++] = ':';
  s[n] = 0;
  boldText(s, x, y, 2, COL_NAME);
}

void centeredLabel(const char* s, int cx, int y) {
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  int w = strlen(s) * 6 - 1;
  tft.setCursor(cx - w / 2, y);
  tft.print(s);
}

// Word-wraps text (font size `size`) into lines of at most maxChars characters (a
// word longer than a whole line gets split). Draws it when `draw` is true.
// Returns how many lines the text uses; endCol/endLine (optional) get the spot
// where the next character would go, for drawing a typing cursor.
int layoutText(const char* text, int x, int y, int maxChars, int lineH, uint16_t color,
               bool draw = true, int* endCol = nullptr, int* endLine = nullptr,
               uint8_t size = 1) {
  int col = 0, line = 0, lastLine = -1;
  for (const char* p = text; *p; p++) {
    if (*p == ' ') {                                // spaces: skipped at the start of a line
      if (col > 0 && col < maxChars) col++;
      continue;
    }
    if (p == text || p[-1] == ' ') {                // start of a word: wrap if it won't fit
      int wl = 0;
      while (p[wl] && p[wl] != ' ') wl++;
      if (col > 0 && col + wl > maxChars) { col = 0; line++; }
    }
    if (col >= maxChars) { col = 0; line++; }      // splits a too-long word
    if (draw) tft.drawChar(x + col * 6 * size, y + line * lineH, *p, color, color, size);
    col++;
    lastLine = line;
  }
  if (endCol)  *endCol = col;
  if (endLine) *endLine = line;
  return lastLine + 1;
}

// Line height for message text at each font size
int textLineH(uint8_t size) { return size == 2 ? 18 : 11; }

// Message text in a w x h box: big (size 2) when it fits, otherwise size 1.
// Returns the size used; endCol/endLine as in layoutText().
uint8_t fitText(const char* text, int x, int y, int w, int h, uint16_t color,
                bool draw = true, int* endCol = nullptr, int* endLine = nullptr) {
  uint8_t size = 2;
  for (; size > 1; size--) {
    int maxLines = (h - 8 * size) / textLineH(size) + 1;
    if (layoutText(text, 0, 0, w / (6 * size), textLineH(size), 0, false, nullptr, nullptr, size) <= maxLines) break;
  }
  layoutText(text, x, y, w / (6 * size), textLineH(size), color, draw, endCol, endLine, size);
  return size;
}

// ---- Icons ----------------------------------------------------------------------

void drawPlusIcon(int cx, int cy) {            // 15 x 15 plus sign (New post)
  tft.fillRoundRect(cx - 2, cy - 7, 5, 15, 1, COL_ICON);   // vertical bar
  tft.fillRoundRect(cx - 7, cy - 2, 15, 5, 1, COL_ICON);   // horizontal bar
}

void drawMinusIcon(int cx, int cy) {           // 15 x 5 minus sign (Clear), matches the plus
  tft.fillRoundRect(cx - 7, cy - 2, 15, 5, 1, COL_ICON);
}

void drawHomeIcon(int cx, int cy) {           // 15 x 15 house, same size as the plus
  tft.fillTriangle(cx - 7, cy - 1, cx + 7, cy - 1, cx, cy - 7, COL_CYAN);   // roof
  tft.fillRect(cx - 5, cy - 1, 11, 9, COL_CYAN);
  tft.fillRect(cx - 1, cy + 3, 3, 5, COL_PILL);   // door
}

void drawClockIcon(int cx, int cy) {           // 15 x 15 ring clock at 3 o'clock (Past posts)
  tft.fillCircle(cx, cy, 7, COL_GRAY);             // face
  tft.fillCircle(cx, cy, 5, COL_PILL);             // hollow it into a 2 px ring
  tft.fillRect(cx, cy - 4, 1, 5, COL_GRAY);        // minute hand, straight up
  tft.fillRect(cx, cy, 4, 1, COL_GRAY);            // hour hand, to the right
  tft.fillCircle(cx, cy, 1, COL_GRAY);             // centre dot
}

// ---- Feeling emojis (design-references/selecting-emojis.png) ---------------------
//
// Drawn for any radius r: every coordinate below is written for r = 10 and
// scaled by es(). Accessories (fists, heart, hair, cap) poke out up to ~3 px.
//
// Animated: each emoji has its own very small (1 px) movement over an 8-frame
// loop. Only the selected emoji (the big one in the FEELING strip) animates;
// the small ones in the picking tray stay in their resting pose. To avoid flicker, a frame is drawn into a small off-screen canvas
// (background included) and copied to the screen in one go.

const int NUM_FEELINGS = 11;
const char* const FEELINGS[NUM_FEELINGS] = {
  "Happy", "Super pumped", "Rizzler", "Blushy wushy", "Rock n roll baby",
  "Starstruck", "What the helly", "That's cap", "Shocked", "Sleepy",
  "Fighting to stay awake",
};
const int FEELING_HAPPY = 0;    // first in the list: where every new post starts
const int NO_FEELING = -1;      // after Clear: "FEELING:" with no name or emoji
int feeling = NO_FEELING;       // the posted feeling, shown on the card (none until the first post)

#define EMOJI_FRAME_MS 300      // time between an emoji's animation frames (higher = calmer)

#define COL_FACE_TOP C565(255, 228,  96)
#define COL_FACE_BOT C565(240, 168,  24)
#define COL_FACE_LN  C565(110,  60,   0)   // mouths, closed eyes
#define COL_FIST     C565(236, 176,  40)
#define COL_INK      C565( 20,  20,  30)

Adafruit_GFX* g = &tft;                    // where emojis get drawn (the screen or a canvas)
static int emR = 10;                       // radius of the emoji being drawn
int es(float v) { return (int)lroundf(v * emR / 10.0f); }

uint16_t lerp565(uint16_t a, uint16_t b, float t) {
  int r = ((a >> 11) & 31) + (int)((((b >> 11) & 31) - ((a >> 11) & 31)) * t);
  int gr = ((a >> 5) & 63) + (int)((((b >> 5) & 63)  - ((a >> 5) & 63))  * t);
  int bl = (a & 31)        + (int)(((b & 31)         - (a & 31))         * t);
  return (r << 11) | (gr << 5) | bl;
}

// Half of a filled ellipse: the lower half (smiles), or the upper half when up = true
void halfEllipse(int cx, int cy, int rx, int ry, uint16_t c, bool up = false) {
  for (int d = 0; d <= ry; d++) {
    float f = 1.0f - (float)(d * d) / (float)(ry * ry);
    int w = (int)(rx * sqrtf(f < 0 ? 0 : f));
    g->drawFastHLine(cx - w, up ? cy - d : cy + d, 2 * w + 1, c);
  }
}

// Closed eye: an arch (^) when happy = true, a droopy curve (u) otherwise
void closedEye(int cx, int cy, bool happy) {
  int w = es(3), h = happy ? -es(2) : es(2);
  g->drawLine(cx - w, cy, cx, cy + h, COL_FACE_LN);
  g->drawLine(cx, cy + h, cx + w, cy, COL_FACE_LN);
  g->drawLine(cx - w, cy + 1, cx, cy + h + 1, COL_FACE_LN);
  g->drawLine(cx, cy + h + 1, cx + w, cy + 1, COL_FACE_LN);
}

// Open eye: white, optional colored iris, black pupil (shifted by pdx, pdy to look around)
void openEye(int cx, int cy, int r, uint16_t iris, int pupil, int pdx = 0, int pdy = 0) {
  g->fillCircle(cx, cy, r, ILI9341_WHITE);
  if (iris != ILI9341_WHITE) g->fillCircle(cx + pdx, cy + pdy, r - 1, iris);
  g->fillCircle(cx + pdx, cy + pdy, pupil, COL_INK);
}

// Closed fist, knuckles up, in the same style as the rock hand: a wrist, a
// rounded-square hand with finger creases along the top and a thumb folded
// across the front. Fits in the same space as the old round fist.
void fist(int cx, int cy) {
  int w = es(7), h = es(6);
  int x = cx - w / 2, y = cy - h / 2;
  int ww = max(2, es(4));                                       // wrist
  g->fillRect(cx - ww / 2, y + h - 1, ww, es(3), COL_FIST);
  g->drawFastVLine(cx - ww / 2, y + h - 1, es(3), COL_FACE_LN);
  g->drawFastVLine(cx - ww / 2 + ww - 1, y + h - 1, es(3), COL_FACE_LN);
  g->fillRoundRect(x, y, w, h, 2, COL_FIST);                    // hand
  g->drawRoundRect(x, y, w, h, 2, COL_FACE_LN);
  for (int i = 1; i <= 3; i++)                                  // gaps between the 4 fingers
    g->drawFastVLine(x + i * w / 4, y, max(1, h / 2 - 1), COL_FACE_LN);
  int ty = y + h / 2 + (h >= 6 ? 1 : 0);                        // thumb across the front
  g->drawFastHLine(x + 1, ty, w * 2 / 3, COL_FACE_LN);
  g->drawFastVLine(x + w * 2 / 3, ty - 1, 2, COL_FACE_LN);      // thumb tip
}

void grin(int cx, int cy, int rx, int ry) {    // big open grin with white teeth
  halfEllipse(cx, cy, rx, ry, COL_FACE_LN);
  halfEllipse(cx, cy, rx - 1, ry - 1, C565(255, 190, 210));
  g->fillRect(cx - rx + 1, cy, 2 * rx - 1, max(1, ry / 2), ILI9341_WHITE);
}

void star5(int cx, int cy, int R, uint16_t c, float spin = 0) {
  float ri = R * 0.45f;
  for (int i = 0; i < 5; i++) {
    float a = -PI / 2 + spin + i * 2 * PI / 5;
    int ox = cx + (int)lroundf(R * cosf(a)),        oy = cy + (int)lroundf(R * sinf(a));
    int ix = cx + (int)lroundf(ri * cosf(a + PI / 5)), iy = cy + (int)lroundf(ri * sinf(a + PI / 5));
    int jx = cx + (int)lroundf(ri * cosf(a - PI / 5)), jy = cy + (int)lroundf(ri * sinf(a - PI / 5));
    g->fillTriangle(cx, cy, jx, jy, ox, oy, c);
    g->fillTriangle(cx, cy, ox, oy, ix, iy, c);
  }
}

// Draws emoji idx at (cx, cy) with radius r, at animation frame `frame` (0 = resting pose)
void drawEmoji(int idx, int cx, int cy, int r, int frame = 0) {
  emR = r;
  int f = frame & 7;
  const int8_t LOOK[8] = { 0, 0, 1, 1, 0, 0, -1, -1 };   // pupils: center, right, center, left
  bool up = (f & 2);                                       // true for 2 frames out of every 4

  // Whole-face movements
  if (idx == 2) cy += up ? 1 : 0;              // Rizzler: slow cool nod
  if (idx == 6) cx += (f & 1) ? 1 : 0;         // What the helly: trembling with rage
  if (idx == 9) cy += (f >= 4) ? 1 : 0;        // Sleepy: drowsy sway

  // Accessories that sit behind the face
  if (idx == 4) {                                            // spiky hair
    for (int i = -2; i <= 2; i++)
      g->fillTriangle(cx + es(i * 3.5f - 2.5f), cy - es(6), cx + es(i * 3.5f + 2.5f), cy - es(6),
                      cx + es(i * 3.5f), cy - es(13), C565(40, 140, 170));
  }
  // Face: yellow, lighter at the top
  for (int dy = -r; dy <= r; dy++) {
    int w = (int)sqrtf((r + 0.5f) * (r + 0.5f) - dy * dy);   // +0.5: no 1 px bumps at the edges
    g->drawFastHLine(cx - w, cy + dy, 2 * w + 1, lerp565(COL_FACE_TOP, COL_FACE_BOT, (dy + r) / (2.0f * r)));
  }

  switch (idx) {
    case 0:   // Happy: big eyes glancing around, toothy smile
      openEye(cx - es(4), cy - es(3), es(3), ILI9341_WHITE, 0);
      openEye(cx + es(4), cy - es(3), es(3), ILI9341_WHITE, 0);
      g->fillCircle(cx - es(4) + LOOK[f], cy - es(2), es(2), C565(30, 30, 120));
      g->fillCircle(cx + es(4) + LOOK[f], cy - es(2), es(2), C565(30, 30, 120));
      grin(cx, cy + es(3), es(4), es(3));
      break;
    case 1: { // Super pumped: both fists pumping up and down together, laughing
      const int8_t PUMP[4] = { 0, 1, 2, 1 };                 // px raised: down, up, top, down
      int lift = PUMP[f & 3];
      fist(cx - es(8), cy - es(9) - lift);
      fist(cx + es(8), cy - es(9) - lift);
      closedEye(cx - es(4), cy - es(2), true);
      closedEye(cx + es(4), cy - es(2), true);
      grin(cx, cy + es(2), es(6), es(5));
      break;
    }
    case 2:   // Rizzler: sunglasses, smirk
      g->fillRect(cx - es(10), cy - es(3), es(20), max(1, es(1.5f)), COL_INK);
      g->fillRoundRect(cx - es(8), cy - es(3), es(7), es(5), 2, COL_INK);
      g->fillRoundRect(cx + es(1), cy - es(3), es(7), es(5), 2, COL_INK);
      g->drawLine(cx + es(2), cy - es(7), cx + es(6), cy - es(6), COL_FACE_LN);
      g->drawLine(cx - es(3), cy + es(5), cx + es(2), cy + es(6), COL_FACE_LN);
      g->drawLine(cx + es(2), cy + es(6), cx + es(4), cy + es(4), COL_FACE_LN);
      break;
    case 3: { // Blushy wushy: floating heart, closed eyes, blush, kiss
      int hy = cy - (up ? 1 : 0);
      g->fillCircle(cx - es(8), hy - es(10), es(2), C565(240, 20, 30));
      g->fillCircle(cx - es(5), hy - es(10), es(2), C565(240, 20, 30));
      g->fillTriangle(cx - es(10), hy - es(9), cx - es(3), hy - es(9), cx - es(6.5f), hy - es(5), C565(240, 20, 30));
      closedEye(cx - es(4), cy - es(1), true);
      closedEye(cx + es(4), cy - es(1), true);
      g->fillCircle(cx - es(6), cy + es(3), es(2), C565(250, 140, 130));
      g->fillCircle(cx + es(6), cy + es(3), es(2), C565(250, 140, 130));
      { // kissy mouth: a puckered "3", pushed out a pixel when the heart floats up
        int mx = cx + (up ? 1 : 0), my = cy + es(5);
        int a = max(1, es(1.5f)), b = max(2, es(3));
        for (int t = 0; t < 2; t++) {                    // two passes = 2 px thick
          g->drawLine(mx - a + t, my - b, mx + a + t, my - b / 2, COL_FACE_LN);
          g->drawLine(mx + a + t, my - b / 2, mx + t, my, COL_FACE_LN);
          g->drawLine(mx + t, my, mx + a + t, my + b / 2, COL_FACE_LN);
          g->drawLine(mx + a + t, my + b / 2, mx - a + t, my + b, COL_FACE_LN);
        }
      }
      break;
    }
    case 4: { // Rock n roll baby: rocking hand over one eye, other eye wide, open mouth
      int hx = cx + ((f & 1) ? 1 : 0);
      openEye(cx + es(4), cy - es(2), es(3), C565(250, 240, 120), es(1.5f));
      g->fillCircle(cx + es(2), cy + es(5), es(2), COL_INK);
      g->fillRoundRect(hx - es(9), cy - es(3), es(7), es(9), 2, COL_FIST);     // hand
      g->drawRoundRect(hx - es(9), cy - es(3), es(7), es(9), 2, COL_FACE_LN);
      g->fillRect(hx - es(8), cy - es(8), max(1, es(1.5f)), es(6), COL_FIST);  // index finger
      g->fillRect(hx - es(4), cy - es(8), max(1, es(1.5f)), es(6), COL_FIST);  // pinky
      break;
    }
    case 5: { // Starstruck: wobbling star eyes, little "o"
      float spin = LOOK[f] * 0.25f;
      star5(cx - es(4), cy - es(2), es(4), C565(190, 170, 235), spin);
      star5(cx + es(4), cy - es(2), es(4), C565(80, 200, 235), -spin);
      g->fillCircle(cx, cy + es(6), max(1, es(1.2f)), C565(200, 60, 120));
      break;
    }
    case 6:   // What the helly: angry brows, dark-ringed eyes, gritted teeth
      for (int t = 0; t < 2; t++) {
        g->drawLine(cx - es(8), cy - es(7) + t, cx - es(2), cy - es(4) + t, COL_INK);
        g->drawLine(cx + es(8), cy - es(7) + t, cx + es(2), cy - es(4) + t, COL_INK);
      }
      openEye(cx - es(4), cy - es(1), es(3), C565(170, 190, 40), es(1));
      openEye(cx + es(4), cy - es(1), es(3), C565(170, 190, 40), es(1));
      g->drawCircle(cx - es(4), cy - es(1), es(3), COL_INK);
      g->drawCircle(cx + es(4), cy - es(1), es(3), COL_INK);
      g->fillRoundRect(cx - es(5), cy + es(3), es(10), es(4), 1, ILI9341_WHITE);
      g->drawRoundRect(cx - es(5), cy + es(3), es(10), es(4), 1, COL_INK);
      for (int i = -2; i <= 2; i += 2) g->drawFastVLine(cx + es(i), cy + es(3), es(4), COL_INK);
      break;
    case 7:   // That's cap: blue cap, green eyes glancing sideways, smirk
      halfEllipse(cx + es(1), cy - es(3), es(9), es(7), C565(40, 40, 170), true);
      g->fillRoundRect(cx - es(11), cy - es(5), es(12), es(3), 1, C565(40, 160, 230));
      openEye(cx - es(4), cy + es(1), es(3), C565(30, 150, 60), es(1), LOOK[f]);
      openEye(cx + es(4), cy + es(1), es(3), C565(30, 150, 60), es(1), LOOK[f]);
      g->drawLine(cx - es(1), cy + es(7), cx + es(3), cy + es(6), COL_FACE_LN);
      break;
    case 8: { // Shocked: huge eyes with darting pupils, open red mouth
      const int8_t DART_X[8] = { 0, 1, 0, -1, 0, 1, -1, 0 };
      const int8_t DART_Y[8] = { 0, 0, 1, 0, -1, 0, 0, 1 };
      openEye(cx - es(4), cy - es(2), es(4), ILI9341_WHITE, es(1), DART_X[f], DART_Y[f]);
      openEye(cx + es(4), cy - es(2), es(4), ILI9341_WHITE, es(1), DART_X[f], DART_Y[f]);
      g->drawCircle(cx - es(4), cy - es(2), es(4), C565(120, 210, 240));
      g->drawCircle(cx + es(4), cy - es(2), es(4), C565(120, 210, 240));
      halfEllipse(cx, cy + es(4), es(4), es(2) + (up ? 1 : 0), C565(210, 40, 60));
      break;
    }
    case 9:   // Sleepy: closed eyes, little "o" breathing in and out
      closedEye(cx - es(4), cy - es(1), false);
      closedEye(cx + es(4), cy - es(1), false);
      g->fillCircle(cx, cy + es(5), max(1, es(1.2f)) + (f >= 4 ? 1 : 0), COL_INK);
      break;
    default:  // Fighting to stay awake: fist up, rubbing one eye, other eye blinking shut
      fist(cx - es(8), cy - es(9));
      closedEye(cx - es(4), cy - es(2), true);
      if (f == 3 || f == 4) closedEye(cx + es(4), cy - es(2), false);   // dozing off...
      else openEye(cx + es(4), cy - es(2), es(2), ILI9341_WHITE, es(1));
      grin(cx - es(1), cy + es(3), es(5), es(4));
      fist(cx + es(7) + ((f & 1) ? 1 : 0), cy + es(3));                 // rubbing
      break;
  }
}

// ---- Animated emoji spots ------------------------------------------------------
//
// A spot is a small rectangle of the screen holding one emoji. Its background
// (one color above splitY, another below, plus the tray's highlight) is redrawn
// into the canvas with every frame.

struct EmojiSpot {
  int8_t   idx;                 // which emoji
  int16_t  cx, cy, r;           // emoji center and radius
  int16_t  x, y, w, h;          // the rectangle that gets redrawn
  int16_t  splitY;              // rows above this get `above`, the rest `below`
  uint16_t above, below;
  bool     trayHighlight;       // draw the tray's "selected" highlight behind it
  bool     animate;             // false = drawn once at frame 0 and left alone
  uint8_t  frame;
  uint32_t next;                // millis() of the next frame
};

const int MAX_SPOTS = 12;
EmojiSpot spots[MAX_SPOTS];     // spots[0] = the FEELING strip, 1.. = the emoji tray
int numSpots = 0;

void clearEmojiSpots() { numSpots = 0; }

// Draws a spot at its current frame
void renderSpot(const EmojiSpot& s) {
  GFXcanvas16 canvas(s.w, s.h);
  if (!canvas.getBuffer()) return;            // out of memory: skip this frame
  canvas.fillScreen(s.below);
  if (s.splitY > s.y) canvas.fillRect(0, 0, s.w, s.splitY - s.y, s.above);
  if (s.trayHighlight) {
    canvas.fillRoundRect(1, 1, s.w - 2, s.h - 2, 5, COL_CARD);
    canvas.drawRoundRect(1, 1, s.w - 2, s.h - 2, 5, COL_PINK_EDG);
  }
  g = &canvas;
  drawEmoji(s.idx, s.cx - s.x, s.cy - s.y, s.r, s.frame);
  g = &tft;
  tft.drawRGBBitmap(s.x, s.y, canvas.getBuffer(), s.w, s.h);
}

// Adds (or replaces, when slot >= 0) a spot and draws it at frame 0
void setEmojiSpot(int slot, const EmojiSpot& s) {
  if (slot < 0) slot = numSpots;
  if (slot >= MAX_SPOTS) return;
  spots[slot] = s;
  spots[slot].frame = 0;
  spots[slot].next = millis() + EMOJI_FRAME_MS + slot * (EMOJI_FRAME_MS / MAX_SPOTS);  // staggered
  if (slot >= numSpots) numSpots = slot + 1;
  renderSpot(spots[slot]);
}

// Call every loop(): advances at most one emoji per call, so taps stay responsive
void updateEmojis() {
  uint32_t now = millis();
  for (int i = 0; i < numSpots; i++) {
    EmojiSpot& s = spots[i];
    if (s.animate && (int32_t)(now - s.next) >= 0) {
      s.frame = (s.frame + 1) & 7;
      s.next = now + EMOJI_FRAME_MS;
      renderSpot(s);
      return;
    }
  }
}

// "FEELING: <name>" plus the (animated) emoji, inside a strip at (x0, stripY)
// of size w x h. `above` is the color just above the strip, where accessories
// like the heart or the hair can poke out. A name too long for one line wraps.
// big: "FEELING:" at size 2 on its own line with the feeling name under it
// (size 2 when it fits on one line, otherwise size 1). Needs h >= 44.
void drawFeelingStrip(int idx, int x0, int stripY, int w, int h, int emojiR, uint16_t above,
                      bool big = false) {
  int ty = stripY + (h - 8) / 2;
  if (big) boldText("FEELING:", x0 + 8, stripY + 4, 2, COL_FEEL);
  else     boldText("FEELING:", x0 + 8, ty, 1, COL_FEEL);
  if (idx == NO_FEELING) return;                       // cleared: just the label

  // Emoji spot: keeps clear of the card's rounded bottom-right corner
  int ex = x0 + w - 12 - emojiR, ey = stripY + h / 2;
  int hw = emojiR * 13 / 10 + 2;                       // half width, incl. accessories + 1 px motion
  int top = max(ey - (emojiR * 14 / 10 + 2), stripY - 6);
  int bottom = min(ey + emojiR + 2, stripY + h - 1);

  const char* name = FEELINGS[idx];
  if (big) {
    int nx = x0 + 8, avail = ex - hw - 2 - nx;
    int lineY = stripY + 24, lineH = h - 4 - 24;       // the space under "FEELING:"
    uint8_t size = (int)strlen(name) * 12 <= avail ? 2 : 1;
    int ny = lineY + (lineH - 8 * size) / 2;
    for (int b = 0; b < 2; b++)                        // drawn twice = bold
      layoutText(name, nx + b, ny, avail / (6 * size), 10, COL_MOOD, true, nullptr, nullptr, size);
  } else {
    int maxChars = (ex - hw - 2 - (x0 + 62)) / 6;
    int lines = layoutText(name, 0, 0, maxChars, 10, 0, false);
    int ny = (lines > 1) ? stripY + (h - 18) / 2 : ty;
    layoutText(name, x0 + 62, ny, maxChars, 10, COL_MOOD);     // drawn twice = bold
    layoutText(name, x0 + 63, ny, maxChars, 10, COL_MOOD);
  }

  EmojiSpot s = {};
  s.idx = idx; s.cx = ex; s.cy = ey; s.r = emojiR;
  s.x = ex - hw; s.y = top; s.w = 2 * hw + 1; s.h = bottom - top + 1;
  s.splitY = stripY; s.above = above; s.below = COL_STRIP;
  s.animate = true;                                    // the selected emoji is the only one that moves
  setEmojiSpot(0, s);
}

// ---- Avatar ---------------------------------------------------------------------
//
// The profile picture, built from the options picked on the avatar screen
// (tap the picture or the pink UPDATE panel). Drawn in a 100 x 100 design space
// and scaled to any size: 46 px on TheSlap screen, 100 px on the avatar screen.

enum AvatarOpt { OPT_GENDER, OPT_SKIN, OPT_HAIR, OPT_HAIR_COL, OPT_EYES, OPT_ACC,
                 OPT_SHIRT, OPT_BG, NUM_OPTS };
const char* const OPT_LABELS[NUM_OPTS] = {
  "Gender", "Skin", "Hair", "Hair col", "Eyes", "Accessory", "Shirt", "Backdrop" };

const char* const GENDERS[] = { "Girl", "Boy" };
const uint16_t SKINS[] = { C565(255, 219, 196), C565(241, 194, 150), C565(224, 172, 120),
                           C565(190, 140, 105), C565(141,  85,  36), C565( 92,  58,  33) };
const char* const HAIRS[] = { "Curly", "Short", "Long", "Ponytail", "Buns", "Buzz" };
const int HAIR_CURLY = 0, HAIR_SHORT = 1, HAIR_LONG = 2, HAIR_PONY = 3, HAIR_BUNS = 4, HAIR_BUZZ = 5;
// Dark brown, auburn, blonde, pink, blue
const uint16_t HAIR_COLS[] = { C565( 50,  30,  20), C565(140,  58,  30),
                               C565(230, 195,  90), C565(255, 110, 180), C565( 90, 140, 255) };
const char* const EYE_NAMES[] = { "Brown", "Blue", "Green", "Gray" };
const uint16_t EYE_COLS[] = { C565(110, 60, 25), C565(60, 140, 220), C565(60, 160, 90), C565(130, 140, 150) };
// Accessories: a pink bow (top right of the head), a headband, a knit hat, or round glasses
const char* const ACCS[] = { "None", "Bow", "Headband", "Hat", "Glasses" };
const int ACC_BOW = 1, ACC_HEADBAND = 2, ACC_HAT = 3, ACC_GLASSES = 4;
const uint16_t COL_BOW      = C565(255, 105, 180);
const uint16_t COL_HEADBAND = C565( 70, 190, 200);   // teal
const uint16_t COL_HAT      = C565(235,  85,  95);   // red knit
const uint16_t COL_HAT_CUFF = C565(250, 240, 225);   // cream cuff + pompom
const uint16_t SHIRTS[] = { C565(245, 245, 245), C565(240,  53, 154), C565( 92, 176, 228),
                            C565(150,  90, 230), C565( 80, 200, 120), C565(255, 200,  80) };
// Backdrop behind the avatar: cream, pink, sky, mint, lavender, peach
const uint16_t BACKDROPS[] = { C565(232, 216, 192), C565(255, 200, 222), C565(190, 225, 255),
                               C565(200, 240, 210), C565(220, 205, 255), C565(255, 215, 170) };

// How many choices each option has (same order as AvatarOpt)
const uint8_t OPT_COUNT[NUM_OPTS] = { 2, 6, 6, 5, 4, 5, 6, 6 };

// The saved avatar. Starts close to the old placeholder: curly dark hair, glasses.
const uint8_t DEFAULT_AVATAR[NUM_OPTS] = { 0, 3, HAIR_CURLY, 0, 0, ACC_GLASSES, 0, 0 };
uint8_t avatar[NUM_OPTS] = { 0, 3, HAIR_CURLY, 0, 0, ACC_GLASSES, 0, 0 };

// Draws avatar `a` into d with its top-left corner at (x, y), s px square.
// Animal Crossing-ish: big round head on a small body, tall sparkly eyes,
// blushy cheeks, a tiny smile, and chunky rounded hair.
void drawAvatarTo(Adafruit_GFX& d, int x, int y, int s, const uint8_t* a) {
  auto P = [&](float v) { return (int)lroundf(v * s / 100.0f); };   // design units -> px
  auto X = [&](float v) { return x + P(v); };
  auto Y = [&](float v) { return y + P(v); };
  auto R = [&](float v) { int r = P(v); return r < 1 ? 1 : r; };    // radii never vanish
  // Filled ellipse in design units (Adafruit_GFX has no fillEllipse)
  auto E = [&](float cx, float cy, float rx, float ry, uint16_t c) {
    float pcx = x + cx * s / 100.0f, pcy = y + cy * s / 100.0f;
    float prx = fmaxf(rx * s / 100.0f, 0.5f), pry = fmaxf(ry * s / 100.0f, 0.5f);
    for (int py = (int)ceilf(pcy - pry); py <= (int)floorf(pcy + pry); py++) {
      float t = (py - pcy) / pry;
      float hw = prx * sqrtf(fmaxf(0.0f, 1 - t * t));
      int xl = (int)lroundf(pcx - hw), xr = (int)lroundf(pcx + hw);
      d.drawFastHLine(xl, py, xr - xl + 1, c);
    }
  };
  bool big = s >= 60;                                                // room for small details

  bool girl = a[OPT_GENDER] == 0;
  int style = a[OPT_HAIR];
  uint16_t skin  = SKINS[a[OPT_SKIN]];
  uint16_t hair  = HAIR_COLS[a[OPT_HAIR_COL]];
  uint16_t shirt = SHIRTS[a[OPT_SHIRT]];
  uint16_t ink   = C565(45, 30, 35);                                 // eyes, mouth, glasses

  d.fillRect(x, y, s, s, BACKDROPS[a[OPT_BG]]);

  // Hair detail colors: strands are darker (lighter on the darkest hair, so
  // they still show) and the shine is a soft highlight
  uint16_t strand = a[OPT_HAIR_COL] == 0 ? lerp565(hair, 0xFFFF, 0.22f) : lerp565(hair, 0, 0.3f);
  uint16_t shine  = lerp565(hair, 0xFFFF, 0.4f);
  // 1 px line and quadratic curve in design units (strands only show on big avatars)
  auto L = [&](float x0, float y0, float x1, float y1) {
    if (big) d.drawLine(X(x0), Y(y0), X(x1), Y(y1), strand);
  };
  auto Q = [&](float x0, float y0, float cx, float cy, float x1, float y1) {
    float px = x0, py = y0;
    for (int i = 1; i <= 6; i++) {
      float t = i / 6.0f, u = 1 - t;
      float nx = u * u * x0 + 2 * u * t * cx + t * t * x1, ny = u * u * y0 + 2 * u * t * cy + t * t * y1;
      L(px, py, nx, ny);
      px = nx; py = ny;
    }
  };
  // Highlight along an elliptical arc (degrees, 0 = right, 270 = top)
  auto shineArc = [&](float cx, float cy, float rx, float ry, int deg0, int deg1) {
    if (!big) return;
    for (int deg = deg0; deg <= deg1; deg += 3) {
      float t = deg * PI / 180;
      E(cx + rx * cosf(t), cy + ry * sinf(t), 1.2f, 1.2f, shine);
    }
  };

  // Hair behind the head
  if (style == HAIR_LONG) {
    d.fillRoundRect(X(17), Y(34), P(66), P(56), R(16), hair);
    Q(21, 50, 19, 70, 23, 86); Q(26, 56, 24, 74, 28, 88);           // strands falling past the shoulders
    Q(79, 50, 81, 70, 77, 86); Q(74, 56, 76, 74, 72, 88);
  }
  if (style == HAIR_PONY) {                                          // tail hanging at the side
    E(86, 52, 9, 15, hair);
    Q(83, 40, 81, 52, 84, 64); Q(88, 41, 87, 54, 89, 63);
    shineArc(86, 52, 6, 11, 190, 230);
  }
  if (style == HAIR_BUNS) {
    for (int bx : { 24, 76 }) {
      E(bx, 20, 11, 11, hair);
      if (big) d.drawCircleHelper(X(bx), Y(20), R(6), bx < 50 ? 0x1 | 0x8 : 0x2 | 0x4, strand);   // wound-up swirl
      shineArc(bx, 20, 7.5f, 7.5f, 215, 250);
    }
  }
  if (style == HAIR_CURLY) {
    for (int deg = 160; deg <= 380; deg += 30) {                     // fluffy bumps around the top
      float t = deg * PI / 180;
      float bx = 50 + 33 * cosf(t), by = 42 + 28 * sinf(t);
      E(bx, by, 9, 9, hair);
      if (big) d.drawCircleHelper(X(bx), Y(by), R(4), 0x2 | 0x4, strand);   // little curl
    }
  }

  // Small body with rounded shoulders (drawn before the head so the chin overlaps it).
  // Girls: tank top (bare shoulders, thin straps, low scoop neck).
  // Boys: t-shirt (short sleeves, high crew neck).
  auto torso = [&](float x0, float w, float top, float rad, uint16_t c) {   // rounded top corners only
    int sr = R(rad), sTop = Y(top);
    for (int py = sTop; py < y + s; py++) {
      int dy = sTop + sr - py, inset = 0;
      if (dy > 0) inset = sr - (int)sqrtf((float)(sr * sr - dy * dy));
      d.drawFastHLine(X(x0) + inset, py, P(w) - 2 * inset, c);
    }
  };
  uint16_t seam = lerp565(shirt, 0, 0.18f);                          // hems and seams
  if (girl) {
    torso(28, 44, 72, 10, skin);                                     // shoulders and arms
    torso(33, 34, 77, 3, shirt);                                     // tank top body
    d.fillRect(X(34), Y(72), P(6), Y(78) - Y(72), shirt);            // straps
    d.fillRect(X(60), Y(72), P(6), Y(78) - Y(72), shirt);
    E(50, 77, 10.5f, 8, skin);                                       // low scoop neck
    if (big) {
      d.drawCircleHelper(X(50), Y(77), R(10.5f), 0x4 | 0x8, seam);   // neckline hem
      d.drawFastVLine(X(32.5f), Y(80), y + s - Y(80), seam);           // arm / top edge
      d.drawFastVLine(X(67.5f), Y(80), y + s - Y(80), seam);
    }
  } else {
    torso(26, 48, 72, 11, shirt);                                    // t-shirt with sleeves
    d.fillRect(X(23), Y(88), P(7), y + s - Y(88), skin);             // arms below the sleeves
    d.fillRect(X(70), Y(88), P(7), y + s - Y(88), skin);
    E(50, 73.5f, 6.5f, 3.5f, skin);                                  // high crew neck
    if (big) {
      d.drawCircleHelper(X(50), Y(73.5f), R(6.5f), 0x4 | 0x8, seam); // neck ribbing
      d.drawFastHLine(X(23), Y(88), P(7), seam);                     // sleeve hems
      d.drawFastHLine(X(70), Y(88), P(7), seam);
      d.drawFastVLine(X(30), Y(80), Y(88) - Y(80), seam);              // sleeve seams
      d.drawFastVLine(X(70), Y(80), Y(88) - Y(80), seam);
    }
  }

  // Hair mass framing the face (just a thin cap for the buzz cut)
  if (style == HAIR_BUZZ) E(50, 44, 30, 24, hair);
  else E(50, 42, 33, 28, hair);
  if (style == HAIR_PONY) E(79, 35, 4, 4, lerp565(hair, 0xFFFF, 0.45f));   // hair tie

  E(50, 49, 28, 24, skin);                                           // big round face

  // Fringe, then strands and shine on top
  switch (style) {
    case HAIR_CURLY:                                                 // row of curls
      for (int cx = 30; cx <= 70; cx += 10) {
        float cy = (cx == 30 || cx == 70) ? 33 : 29;
        E(cx, cy, 7, 7, hair);
        if (big) d.drawCircleHelper(X(cx), Y(cy), R(3.5f), 0x2 | 0x4, strand);
      }
      shineArc(50, 42, 27, 22, 222, 252);
      break;
    case HAIR_SHORT:                                                 // side swoop + a little tuft
      E(42, 28, 22, 9, hair);
      E(67, 29, 13, 7, hair);
      d.fillTriangle(X(47), Y(11), X(56), Y(19), X(45), Y(19), hair);
      Q(24, 27, 32, 30, 36, 36); Q(33, 22, 42, 26, 48, 35); Q(44, 20, 54, 24, 58, 33);   // swoop strands
      Q(62, 23, 68, 26, 74, 33);
      L(50, 13, 51, 19);                                             // tuft
      shineArc(50, 42, 27, 22, 228, 258);
      break;
    case HAIR_LONG: {                                                // side part, bangs swept across
      L(37, 14, 37, 24);                                             // the part
      E(28, 31, 7, 10, hair);                                        // short piece left of the part
      for (int i = 0; i <= 12; i++) {                                // swoop: curve of shrinking blobs
        float t = i / 12.0f, u = 1 - t;
        float cx = u * u * 45 + 2 * u * t * 62 + t * t * 75;
        float cy = u * u * 25 + 2 * u * t * 30 + t * t * 43;
        float r = 8 - 3 * t;
        E(cx, cy, r, r, hair);
      }
      d.fillRoundRect(X(19), Y(36), P(9), P(40), R(4), hair);        // side locks
      d.fillRoundRect(X(72), Y(38), P(9), P(38), R(4), hair);
      Q(40, 20, 58, 23, 74, 38); Q(43, 28, 60, 33, 71, 46);          // swoop strands
      Q(26, 24, 24, 30, 25, 38);
      L(23, 42, 23, 72); L(76, 44, 76, 72);                          // side lock strands
      shineArc(50, 42, 27, 22, 205, 228);
      shineArc(50, 42, 27, 22, 300, 318);
      break;
    }
    case HAIR_PONY:
    case HAIR_BUNS:                                                  // middle part
      E(38, 29, 14, 8, hair);
      E(62, 29, 14, 8, hair);
      L(50, 15, 50, 23);                                             // the part
      Q(49, 23, 38, 24, 28, 32); Q(51, 23, 62, 24, 72, 32);          // combed to the sides
      Q(47, 18, 34, 19, 25, 27); Q(53, 18, 66, 19, 75, 27);
      shineArc(50, 42, 27, 22, 222, 248);
      break;
    case HAIR_BUZZ:
      shineArc(50, 44, 25, 20, 228, 255);
      break;
  }

  // Blushy cheeks
  uint16_t blush = lerp565(skin, C565(255, 120, 150), 0.45f);
  E(31, 59, 5, 3, blush);
  E(69, 59, 5, 3, blush);

  // Tall oval eyes: dark outline, colored iris, pupil, one sparkle (top left)
  for (int ex : { 39, 61 }) {
    E(ex, 51, 5, 6.5f, ink);
    E(ex, 52.5f, 3.5f, 4.5f, EYE_COLS[a[OPT_EYES]]);
    E(ex, 53, 2, 2.5f, ink);
    E(ex - 1.5f, 48.5f, 1.8f, 1.8f, 0xFFFF);                         // sparkle
    if (girl && big) {                                               // two lashes on the outer side
      int o = ex < 50 ? -1 : 1;
      d.drawLine(X(ex + o * 4), Y(46), X(ex + o * 7), Y(43), ink);
      d.drawLine(X(ex + o * 5), Y(48.5f), X(ex + o * 8), Y(47), ink);
    }
  }

  // Tiny nose and smile
  E(50, 56.5f, 1.2f, 0.9f, lerp565(skin, 0, 0.3f));
  d.drawCircleHelper(X(50), Y(59), R(3.5f), 0x4 | 0x8, ink);
  if (big) d.drawCircleHelper(X(50), Y(59) - 1, R(3.5f), 0x4 | 0x8, ink);

  // Accessory
  int acc = a[OPT_ACC];
  if (acc == ACC_GLASSES) {                                          // big round glasses
    for (int ex : { 39, 61 }) {
      d.drawCircle(X(ex), Y(51), R(9.5f), ink);
      if (big) d.drawCircle(X(ex), Y(51), R(9.5f) - 1, ink);
    }
    d.drawFastHLine(X(48.5f), Y(50), X(51.5f) - X(48.5f) + 1, ink);  // bridge
    d.drawFastHLine(X(22), Y(50), X(29.5f) - X(22), ink);            // arms
    d.drawFastHLine(X(70.5f), Y(50), X(78) - X(70.5f), ink);
  } else if (acc == ACC_HEADBAND) {                                  // band over the top of the hair
    for (int deg = 196; deg <= 344; deg += 4) {
      float t = deg * PI / 180;
      E(50 + 31 * cosf(t), 41 + 23 * sinf(t), 2.8f, 2.8f, COL_HEADBAND);
    }
    if (big) {                                                       // shine along the band
      for (int deg = 230; deg <= 290; deg += 4) {
        float t = deg * PI / 180;
        E(50 + 31 * cosf(t), 40 + 23 * sinf(t), 0.8f, 0.8f, lerp565(COL_HEADBAND, 0xFFFF, 0.5f));
      }
    }
  } else if (acc == ACC_HAT) {                                       // knit beanie with a pompom
    d.fillRoundRect(X(20), Y(10), P(60), P(26), R(13), COL_HAT);     // crown
    if (big) {                                                       // knit ribs
      uint16_t rib = lerp565(COL_HAT, 0, 0.18f);
      for (int rx = 30; rx <= 70; rx += 10) d.drawFastVLine(X(rx), Y(rx == 30 || rx == 70 ? 17 : 13), Y(28) - Y(rx == 30 || rx == 70 ? 17 : 13), rib);
    }
    d.fillRoundRect(X(17), Y(28), P(66), P(9), R(4.5f), COL_HAT_CUFF);   // cuff
    E(50, 9, 6, 6, COL_HAT_CUFF);                                    // pompom
  } else if (acc == ACC_BOW) {                                       // pink bow, top right of the head
    uint16_t c = COL_BOW;
    E(63, 20, 7, 5.5f, c);
    E(77, 20, 7, 5.5f, c);
    if (big) {
      E(62, 19, 3, 2, lerp565(c, 0xFFFF, 0.4f));                     // shine on each loop
      E(76, 19, 3, 2, lerp565(c, 0xFFFF, 0.4f));
    }
    E(70, 20, 3, 3.5f, lerp565(c, 0, 0.25f));                        // knot
  }

  d.drawRect(x, y, s, s, 0xFFFF);                                    // white frame
}


// Draws the avatar off-screen first so it appears in one go (no flicker)
void drawAvatar(int x, int y, int s, const uint8_t* a) {
  GFXcanvas16 canvas(s, s);
  if (canvas.getBuffer()) {
    drawAvatarTo(canvas, 0, 0, s, a);
    tft.drawRGBBitmap(x, y, canvas.getBuffer(), s, s);
  } else {
    drawAvatarTo(tft, x, y, s, a);                                   // out of memory: draw directly
  }
}

// ---- Nav panels (shape from design-references/pear-corners.png) ------------------
//
// Each panel has one straight edge facing the content and one shaped outer edge.
//   WEDGE:      tapers to a point at the start, sweeps steeply out to full depth,
//               then stays at full depth for the last `r` px
//   ROUND_START: full depth, with a big rounded corner (radius `r`) at the start
//   ROUND_END:   full depth, with a big rounded corner (radius `r`) at the end
//                (ROUND_START is ROUND_END mirrored)
// Horizontal panels (bottom bar): straight top edge at y, column i of x..x+len-1
//   reaches `depth` px down at most.
// Vertical panels (left bar): straight right edge at x, row i of y..y+len-1
//   reaches `depth` px to the left at most.


int panelDepth(bool vertical, int x, int y, PanelShape shape, int i, int len, int depth, int r) {
  if (shape == WEDGE) {
    int taper = len - r;                 // length of the sloped part
    if (i >= taper) return depth;
    float t = (float)i / (taper - 1);
    int d = (int)lroundf(depth * (1.0f - (1.0f - t) * (1.0f - t)));  // ease-out sweep
    return d < 1 ? 1 : d;
  }
  if (shape == ROUND_START) i = len - 1 - i;   // mirror, then shape like ROUND_END
  int dx = i - (len - 1 - r);
  if (dx <= 0) return depth;
  return depth - r + (int)lroundf(sqrtf((float)(r * r - dx * dx)));
}

// Pixel / line that skip anything outside the rounded border, so a panel that
// reaches past it looks tucked under the case
void clipPixel(int x, int y, uint16_t c) {
  if (insideBorder(x, y, 0)) tft.drawPixel(x, y, c);
}
void clipLine(int x0, int y0, int x1, int y1, uint16_t c) {
  int dx = abs(x1 - x0), dy = -abs(y1 - y0);
  int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1, err = dx + dy;
  for (;;) {
    clipPixel(x0, y0, c);
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; }
    if (e2 <= dx) { err += dx; y0 += sy; }
  }
}

void drawPanel(bool vertical, int x, int y, int len, int depth, PanelShape shape, int r) {
  // Fill
  for (int i = 0; i < len; i++) {
    int d = panelDepth(vertical, x, y, shape, i, len, depth, r);
    if (vertical) {
      int xl, xr;
      rowExtent(y + i, xl, xr);
      int a = max(x - d + 1, xl), b = min(x, xr);
      if (b >= a) tft.drawFastHLine(a, y + i, b - a + 1, COL_PILL);
    } else {
      int h = d;                                       // trim the column to the border
      while (h > 0 && !insideBorder(x + i, y + h - 1, 0)) h--;
      if (h > 0 && insideBorder(x + i, y, 0)) tft.drawFastVLine(x + i, y, h, COL_PILL);
    }
  }
  // Outline: straight edge, shaped edge, and the two end sides
  int prev = 0;
  for (int i = 0; i < len; i++) {
    int d = panelDepth(vertical, x, y, shape, i, len, depth, r);
    if (vertical) {
      clipPixel(x, y + i, COL_PINK_EDG);
      int ex = x - d + 1;
      if (i == 0) clipLine(ex, y, x, y, COL_PINK_EDG);
      else        clipLine(prev, y + i - 1, ex, y + i, COL_PINK_EDG);
      if (i == len - 1) clipLine(ex, y + i, x, y + i, COL_PINK_EDG);
      prev = ex;
    } else {
      clipPixel(x + i, y, COL_PINK_EDG);
      int ey = y + d - 1;
      if (i == 0) clipLine(x, y, x, ey, COL_PINK_EDG);
      else        clipLine(x + i - 1, prev, x + i, ey, COL_PINK_EDG);
      if (i == len - 1) clipLine(x + i, y, x + i, ey, COL_PINK_EDG);
      prev = ey;
    }
  }
}


// The Slap logo: 63x20 RGB565 bitmap plus 1-bit transparency mask
// (scaled down to 50% from the original 126x40).
#define LOGO_W 63
#define LOGO_H 20
#define LOGO_X 199   // top-right, just inside the rounded border
#define LOGO_Y 10
const uint16_t THE_SLAP_LOGO[LOGO_W * LOGO_H] PROGMEM = {
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xA50B, 0x0000, 0x0000, 0x03B9, 0x020F, 0xA48A,
  0xFD25, 0xBCA8, 0xCCC8, 0xCCC7, 0x6B49, 0x4920, 0x0000, 0x0000, 0x52AA, 0x9B2C, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x4986, 0x5248, 0xA4EB, 0x0000, 0x20C6, 0xA4CB, 0xAD4B,
  0xCC87, 0xC52A, 0xFCC5, 0xFD2E, 0xFC45, 0xFC88, 0xFD0A, 0xCC87, 0x0018, 0x0000, 0xFFFF, 0x0000,
  0x0000, 0x0000, 0x3186, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x52AA, 0x9CCC, 0xCCCC, 0xCCCC, 0x9CCC, 0x79E7, 0x52AA, 0x9B2C,
  0x632C, 0x52AA, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x18E3, 0x7349, 0xCE2D, 0xC5ED,
  0xACCA, 0xCE8E, 0xFDE8, 0xFCCA, 0xFC45, 0xFC47, 0xFFFF, 0xFC48, 0xFF3B, 0xFE55, 0xFBE0, 0xCDCC,
  0x4A28, 0x0000, 0x5A68, 0x942B, 0x62A8, 0x51C5, 0x0000, 0x0000, 0x0000, 0x0000, 0x7BEF, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFEF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7BEF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF,
  0x0000, 0x7BC9, 0xD64D, 0xFFF1, 0xFF4D, 0xF3E4, 0xFFFF, 0xF4CB, 0xFE34, 0xFF19, 0xFC48, 0xFFFF,
  0xFC68, 0xFCAB, 0xFD07, 0xE62B, 0xBD8D, 0xB56D, 0x6B6A, 0x2108, 0x000A, 0x000B, 0x0004, 0x000B,
  0x0000, 0x0000, 0x39E7, 0x0000, 0x0000, 0x0000, 0x52AA, 0x3186, 0x52AA, 0x0000, 0x2948, 0x8C8C,
  0xC5CB, 0xC5AB, 0xA52C, 0x4249, 0x4249, 0x7C0B, 0x73EB, 0x428A, 0x0000, 0x52AA, 0xFFE0, 0x52AA,
  0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0x52AA, 0x52AA, 0xFFE0, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0xBDE7, 0x0000, 0x20A6, 0xBE2E, 0xFDE7, 0xFD4F, 0xFFDE, 0xFC06, 0xFFFF,
  0xFD0C, 0xFE34, 0xFF3A, 0xFC47, 0xFFFF, 0xFC68, 0xEE4B, 0xC62E, 0xA428, 0x9387, 0xAC28, 0xA408,
  0x9BC7, 0x93E8, 0x9BE8, 0xAC89, 0x632A, 0x0000, 0x0000, 0xFFF5, 0x0000, 0x0000, 0x0000, 0x0000,
  0x952D, 0xD5AA, 0xFD26, 0xFD07, 0xFD28, 0xFCE6, 0xE589, 0xDDAA, 0xFCC6, 0xFD07, 0xCCC8, 0x39E9,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x9CCC, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0xFFEF, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x4168, 0x0005, 0x2907, 0xAD2C, 0xFC64, 0xFF5C,
  0xFE13, 0xFD2D, 0xFFFF, 0xFC06, 0xFFFF, 0xFD0C, 0xFE33, 0xFF3B, 0xFC63, 0x9D6E, 0xD4E8, 0xFD28,
  0xFDAC, 0xFD6A, 0xFD6B, 0xFDAC, 0xFCE7, 0xFCE8, 0xFD68, 0xA44A, 0x226D, 0x01B2, 0x0000, 0x0174,
  0x3AEC, 0x29A9, 0x23F3, 0xD58A, 0xFCC7, 0xFE77, 0xFFFF, 0xFFFF, 0xFF7D, 0xFCE7, 0xFCC4, 0xFE76,
  0xFEB8, 0xF465, 0x3168, 0x02D9, 0x6FFE, 0x32AC, 0x44F4, 0x0015, 0x0000, 0x0000, 0x00F5, 0x67FF,
  0x2A4B, 0x2A4B, 0x36DF, 0x0004, 0x0000, 0x7BEF, 0x0000, 0x0000, 0x0000, 0x28E7, 0x6269, 0x946C,
  0xE5C9, 0xFCC9, 0xFFFF, 0xFC27, 0xFF3B, 0xFDD1, 0xFD0C, 0xFFDF, 0xFC05, 0xFFDF, 0xFD4E, 0xDCA6,
  0x6B8B, 0xF4E6, 0xFD6F, 0xFE77, 0xFFBE, 0xFF3B, 0xFE56, 0xF48A, 0xFF1A, 0xFCEC, 0xF442, 0xE4C6,
  0xC509, 0xADEE, 0xCD49, 0xE4C6, 0xE486, 0xCDAA, 0xFD26, 0xFE55, 0xFFFF, 0xFE96, 0xFD2E, 0xFDD2,
  0xFC67, 0xFC64, 0xFF9D, 0xFF5B, 0xEC86, 0x9CAB, 0xC4A7, 0xE4E5, 0xE4A5, 0xDCC6, 0xC4E9, 0x94CC,
  0xA4AA, 0xC4C9, 0xDCE6, 0xE4A5, 0xE4A5, 0xDCE7, 0xA48A, 0x6BEC, 0x0000, 0x62C8, 0xE70E, 0xACEC,
  0xACCA, 0x3167, 0x8C4C, 0xFD05, 0xFE34, 0xFFFF, 0xFE75, 0xFFFF, 0xFEB7, 0xFFBD, 0xFE12, 0xFD2D,
  0xFFDF, 0xFC45, 0xDE6C, 0xB56C, 0xBCEA, 0xECA6, 0xFBC2, 0xFF5B, 0xFD8F, 0xFC64, 0xFC44, 0xFF1A,
  0xFDD1, 0xFE54, 0xFE55, 0xFCE8, 0xFDE7, 0xFCE8, 0xFE33, 0xFE54, 0xFCE9, 0xF3A2, 0xFF7C, 0xFFDE,
  0xF3E3, 0xFE48, 0xFDC7, 0xFDC8, 0xFCC5, 0xFF7C, 0xFEB8, 0xFD26, 0xFDA8, 0xFD8E, 0xFE54, 0xFE76,
  0xFE33, 0xFCE9, 0xFE09, 0xFDA8, 0xFD2C, 0xFDF1, 0xFE75, 0xFE75, 0xFDB0, 0xFCE7, 0xDD69, 0x6BCD,
  0x49E7, 0x83AA, 0xC5AC, 0xD64D, 0xE6EF, 0xFF6D, 0xFC25, 0xFFBE, 0xFF19, 0xFCAA, 0xFC88, 0xFCEC,
  0xFE75, 0xFF5A, 0xFF9D, 0xFE14, 0xFCC5, 0x94AC, 0x5A68, 0x18C7, 0x746D, 0xFD27, 0xFF5C, 0xFD6E,
  0xFECA, 0xFD47, 0xFF1A, 0xFF5B, 0xFD4F, 0xFFBD, 0xFEB7, 0xF301, 0xFF7D, 0xFD70, 0xFD0D, 0xFFFF,
  0xF362, 0xFDB1, 0xFFFF, 0xFDF2, 0xFCA5, 0xFFEF, 0xFFF0, 0xFCA5, 0xFFBE, 0xFE97, 0xFCC4, 0xFC88,
  0xFEB8, 0xFE33, 0xFDD0, 0xFF9D, 0xFFBF, 0xFC66, 0xF424, 0xFFDF, 0xFFFF, 0xFEFA, 0xFEB9, 0xFFFF,
  0xFF1B, 0xFCC6, 0xACAA, 0x0000, 0x1867, 0x942B, 0x6288, 0xB58E, 0xC4E8, 0xFCEA, 0xFFFF, 0xFFBD,
  0xFF3B, 0xFF9C, 0xFE75, 0xFC68, 0xFD90, 0xFFFF, 0xFC48, 0xFEEC, 0xF7AF, 0xF74E, 0xD64D, 0xAD2B,
  0xFCC6, 0xFF5C, 0xFD4D, 0xFE8A, 0xFCE6, 0xFF5C, 0xFD4D, 0xF420, 0xFDD2, 0xFED7, 0xFC89, 0xFFFF,
  0xFD90, 0xFDB0, 0xFED8, 0xFC66, 0xFC23, 0xFE76, 0xFFFF, 0xFDD2, 0xFC43, 0xFF8E, 0xFCA6, 0xFFDF,
  0xFE55, 0xFD66, 0xFDC9, 0xFBE1, 0xF3E0, 0xF340, 0xFE55, 0xFFFF, 0xFCC9, 0xF3E3, 0xFFFF, 0xFE55,
  0xFC43, 0xFCC5, 0xFD2D, 0xFFFF, 0xFDD0, 0xD486, 0x0000, 0x0000, 0x0000, 0x0000, 0x0028, 0xABC7,
  0xFD6C, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFF3A, 0xFE76, 0xFD27, 0xFDA7, 0xFC85,
  0xFCA4, 0xDD28, 0x8C6B, 0xFCE6, 0xFF7C, 0xFD2C, 0xFEAA, 0xFCE7, 0xFF3C, 0xFD2A, 0xFDA6, 0xFE13,
  0xFE75, 0xFC68, 0xFFFF, 0xF3C3, 0xF3C0, 0xF320, 0xFDE9, 0xFF8F, 0xFC23, 0xFE56, 0xFFFF, 0xFDD2,
  0xFCE5, 0xFCC7, 0xFFFF, 0xFE14, 0xFD66, 0xFD67, 0xFCEB, 0xFE75, 0xFF7C, 0xFFFF, 0xFFFF, 0xFC87,
  0xF404, 0xFFFF, 0xFDF2, 0xFDE8, 0xFFF0, 0xFC65, 0xFFDF, 0xFE96, 0xE485, 0x0000, 0x52AA, 0x9CCC,
  0x0000, 0x0006, 0xCCE8, 0xFCE9, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFD6F,
  0xF445, 0xFD0D, 0xFF1A, 0xFF1A, 0xF404, 0xBD4B, 0xF486, 0xFFBF, 0xFCEC, 0xFECA, 0xFCE8, 0xFF7E,
  0xFD09, 0xFD65, 0xFE55, 0xFE76, 0xFB20, 0xFEFA, 0xFED8, 0xFDF2, 0xFD2E, 0xFD25, 0xFFAE, 0xFFAE,
  0xFC64, 0xFEB8, 0xFFFF, 0xFC87, 0xFC24, 0xFFFF, 0xFE54, 0xF340, 0xFDF3, 0xFFFF, 0xFEF8, 0xFCCB,
  0xFE54, 0xFFFF, 0xFC66, 0xFC25, 0xFFFF, 0xFDD2, 0xFDA6, 0xFFF0, 0xFC66, 0xFFFF, 0xFE76, 0xE485,
  0x0000, 0x0000, 0x0000, 0x8C2A, 0xD62D, 0xDE8E, 0xFC63, 0xFEB8, 0xFFFF, 0xFFDF, 0xFFFF, 0xFFFF,
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFBE, 0xFFFF, 0xFFFF, 0xFE14, 0xFC85, 0x8C4B, 0xFD07, 0xFD4B, 0xFD07,
  0xFF4E, 0xFD27, 0xFD2B, 0xFD28, 0xEDE9, 0xFCA7, 0xFCC7, 0xED67, 0xFCC6, 0xFD6D, 0xFDF0, 0xFCC8,
  0xFBA3, 0xFC87, 0xFD07, 0xFC43, 0xFE34, 0xFFFF, 0xFCA9, 0xFC04, 0xFFFF, 0xFDF1, 0xFBA3, 0xFFFF,
  0xFE55, 0xFBA0, 0xFC00, 0xFE13, 0xFFFF, 0xF445, 0xFC66, 0xFFFF, 0xFF5C, 0xFC66, 0xFCA4, 0xFD6F,
  0xFFFF, 0xFDAF, 0xC446, 0x0000, 0x20C7, 0x62A9, 0xAD0C, 0xAD0C, 0xC5ED, 0xFECC, 0xFC05, 0xFF1A,
  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFED8, 0xFCC9, 0xF444, 0x93A9, 0x3168,
  0xA44A, 0x8B07, 0xCD69, 0x532C, 0xA46A, 0xABA6, 0x9C4A, 0x62EA, 0xA44B, 0x9BEB, 0x5AAB, 0x8BAB,
  0xB42A, 0xBBC9, 0xD52A, 0xFCE8, 0xFFDF, 0xFEFA, 0xFF3B, 0xFFFF, 0xFF1A, 0xFBE2, 0xF4A9, 0xFFFF,
  0xF5F2, 0xF320, 0xFF7D, 0xFFBE, 0xFDF2, 0xF4A9, 0xFE96, 0xFFFF, 0xF424, 0xFC87, 0xFFFF, 0xFF9C,
  0xFF3B, 0xFE98, 0xFFFF, 0xFF3C, 0xFCC6, 0x9409, 0x0000, 0x3988, 0x5A69, 0x62C9, 0x6AC9, 0xA4AB,
  0x94ED, 0xC488, 0xFC85, 0xFDB0, 0xFF5B, 0xFFDF, 0xFFDF, 0xFFBE, 0xFEB6, 0xFD2B, 0xFC44, 0xBC07,
  0xAAE5, 0xB1E1, 0xA1E1, 0xBA00, 0x6104, 0x70E2, 0xCA00, 0x8162, 0x50C4, 0x8160, 0x8189, 0xE28C,
  0xCA09, 0xB9A8, 0xC9A8, 0xD20A, 0xC907, 0xC4F0, 0xFD66, 0xFDAF, 0xFE95, 0xFEB6, 0xFDF0, 0xFCC7,
  0xFE09, 0xFCE7, 0xFE11, 0xFD0A, 0xFD65, 0xFD09, 0xFE54, 0xFED7, 0xFCEA, 0xFD2B, 0xFE11, 0xFC23,
  0xFCA8, 0xFFFF, 0xFD8F, 0xFCCB, 0xFED7, 0xFE12, 0xFD07, 0xCCE9, 0x42AB, 0x0000, 0x0000, 0x0000,
  0x0007, 0x5A69, 0x0000, 0x0000, 0x21CC, 0xAC8A, 0xDC45, 0xFC85, 0xFCE7, 0xFCE8, 0xFCA6, 0xF445,
  0xBBE6, 0x8C0B, 0x00EE, 0xA9E1, 0xEB01, 0xA202, 0xAA22, 0xBA61, 0xCAA1, 0xC262, 0x81A3, 0x7983,
  0x9223, 0x8969, 0xE165, 0xD904, 0xD986, 0xDA28, 0xDAAA, 0xAA09, 0x4ACE, 0xAC89, 0xC446, 0xDC65,
  0xE465, 0xCC46, 0xA449, 0x7BEC, 0xB407, 0xC3E5, 0xABA6, 0x8C6C, 0xBCA8, 0xDC65, 0xE464, 0xC467,
  0xBC06, 0xB385, 0xDD08, 0xFD0A, 0xFFFF, 0xFD6E, 0xDC43, 0xD445, 0xD445, 0x9BE9, 0x532B, 0x0000,
  0x0000, 0x780F, 0x6186, 0x0000, 0x0000, 0x0000, 0xBDEF, 0x0000, 0x0000, 0x3A6D, 0x940B, 0xB428,
  0x9347, 0xAC89, 0x4A4A, 0x0154, 0x0000, 0xF800, 0x40E5, 0x30A4, 0x28A5, 0x3085, 0x38C4, 0x5903,
  0x38A5, 0x38C4, 0x8182, 0x7163, 0x5969, 0x7ACD, 0x728C, 0x8A8C, 0x61CB, 0x7AED, 0x7A2A, 0x000C,
  0x0007, 0x0016, 0x02D8, 0x03B7, 0x011A, 0x0000, 0x0000, 0x000B, 0x4947, 0x0017, 0x3003, 0x0827,
  0x0B36, 0x41EA, 0x00ED, 0x012F, 0x00D7, 0xAC28, 0xFD6C, 0xFFFF, 0xFDCE, 0x9B67, 0x1399, 0x4F7F,
  0x0009, 0x0000, 0x7BEF, 0x0000, 0x0000, 0x0000, 0x0000, 0x52AA, 0x0000, 0x0000, 0x7BEF, 0x0000,
  0x0000, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000, 0x0000, 0xBBE7, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x7163, 0xB241, 0xBA41, 0x5922, 0xBA20, 0xAA00, 0x89A2, 0x81A3,
  0x99A0, 0xB220, 0x89A3, 0x81A3, 0xA1E2, 0x91A2, 0x91A2, 0x99E2, 0xAA02, 0xA202, 0xAA02, 0x99E1,
  0x89A3, 0xF300, 0x7983, 0x99C2, 0xAA01, 0x99C2, 0x6123, 0x0810, 0xB448, 0xFD2A, 0xFEB7, 0xFD4A,
  0x9B47, 0x000C, 0x0000, 0x0000, 0x7BEF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x52AA, 0x9B2C, 0x0000, 0x0000, 0xBDE7, 0x6186, 0x0000, 0x0000, 0x0000,
  0x39E7, 0x3186, 0x3006, 0x3006, 0x3186, 0x9124, 0x1089, 0x7163, 0xAA22, 0x6143, 0x38C5, 0x89C3,
  0x81A2, 0xA202, 0xAA01, 0x91E2, 0x89A2, 0xBA61, 0x5924, 0xBA62, 0x91C2, 0x99E2, 0x99E2, 0xA202,
  0x99E2, 0xC261, 0x6123, 0x7184, 0x99C2, 0xA202, 0x6144, 0x89A2, 0xA202, 0x7163, 0x000F, 0x838A,
  0xE507, 0xF465, 0xDCC7, 0x7B8A, 0x7BE0, 0x9B2C, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x5000, 0x0000, 0x2866, 0x48E5,
  0x0006, 0xFFFF, 0x0827, 0x0007, 0x1026, 0x1046, 0x0007, 0x5104, 0x5103, 0x000A, 0x0807, 0x0007,
  0x1046, 0x0007, 0x0826, 0x0007, 0x1846, 0x1846, 0x1846, 0x0009, 0x1846, 0x0007, 0x2066, 0x2066,
  0x0806, 0x0000, 0x0000, 0x5EDB, 0xC7FD, 0x35B9, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

const uint8_t THE_SLAP_LOGO_MASK[((LOGO_W + 7) / 8) * LOGO_H] PROGMEM = {
  0x00, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x1F, 0xFF, 0xE6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xFF, 0xFC, 0x00, 0x02, 0x04, 0x00, 0x00,
  0x03, 0xFF, 0xFF, 0xF0, 0x07, 0xFE, 0x00, 0x00, 0x07, 0xFF, 0xDF, 0xF8, 0x2F, 0xFF, 0x00, 0x00,
  0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC,
  0xFF, 0xFF, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
  0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
  0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x3F, 0xFF, 0xFE, 0x6F, 0xFF, 0xFF, 0xFF, 0xFE,
  0x2F, 0xFF, 0xFE, 0x6F, 0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0xFF, 0x3F, 0xFF, 0xEF, 0xDF, 0xFF, 0xF8,
  0x00, 0x1C, 0x00, 0x3F, 0xE0, 0x08, 0x07, 0xC0, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0xFB, 0xF7, 0xC0,
  0x00, 0x00, 0x00, 0x7F, 0xFF, 0xFF, 0xF7, 0x80, 0x00, 0x00, 0x00, 0x20, 0x34, 0x01, 0x20, 0x00
};

// The Slap logo, in the top-right corner.
void drawTheSlapLogo() {
  tft.drawRGBBitmap(LOGO_X, LOGO_Y, THE_SLAP_LOGO, THE_SLAP_LOGO_MASK, LOGO_W, LOGO_H);
}

// ---- Blinking sparkles -----------------------------------------------------------

struct Sparkle {
  int16_t  x, y;
  uint8_t  size;          // 0 = single pixel, 1 = small plus, 2 = big 4-point star
  uint16_t color;
  bool     on;
  uint32_t nextToggle;
  uint16_t onMs, offMs;
};
Sparkle sparkles[NUM_SPARKLES];

// Areas covered by UI, so sparkles never get placed on top of them
struct Box { int16_t x, y, w, h; };
const Box UI_BOXES[] = {
  {  6,  68,  40,  99 },   // left nav panels (New + Clear)
  { 50,  72,  58,  80 },   // pink update panel
  { 102,  40, 200, 152 },  // message card
  { 103, 194, 198,  40 },  // bottom nav panels (Home + Past)
  { LOGO_X, LOGO_Y, LOGO_W, LOGO_H },  // The Slap logo
};
const int NUM_BOXES = sizeof(UI_BOXES) / sizeof(UI_BOXES[0]);

// The boxes for the screen that's showing (the new post screen swaps in its own)
const Box* activeBoxes = UI_BOXES;
int numActiveBoxes = NUM_BOXES;

bool hitsUI(int x, int y, int pad) {
  for (int i = 0; i < numActiveBoxes; i++) {
    const Box& b = activeBoxes[i];
    if (x >= b.x - pad && x <= b.x + b.w + pad && y >= b.y - pad && y <= b.y + b.h + pad) return true;
  }
  return false;
}

// Draws one pixel of a sparkle, or erases it back to the background gradient
void put(int x, int y, bool on, uint16_t color) {
  if (!insideBorder(x, y, 0)) return;
  tft.drawPixel(x, y, on ? color : bgColor(y));
}

void drawSparkle(int idx, bool on) {
  const Sparkle& s = sparkles[idx];
  int x = s.x, y = s.y;
  put(x, y, on, s.color);
  if (s.size >= 1) {
    put(x - 1, y, on, s.color); put(x + 1, y, on, s.color);
    put(x, y - 1, on, s.color); put(x, y + 1, on, s.color);
  }
  if (s.size == 2) {
    for (int i = 2; i <= 5; i++) {
      put(x - i, y, on, s.color); put(x + i, y, on, s.color);
      put(x, y - i, on, s.color); put(x, y + i, on, s.color);
    }
    for (int i = 1; i <= 3; i++) {
      put(x - i, y - i, on, s.color); put(x + i, y - i, on, s.color);
      put(x - i, y + i, on, s.color); put(x + i, y + i, on, s.color);
    }
  }
}

// Places the sparkles around the UI boxes and starts them blinking.
// (goldX, goldY) is where the big gold sparkle goes.
void initSparkles(const Box* boxes = UI_BOXES, int numBoxes = NUM_BOXES,
                  int goldX = 24, int goldY = 200) {
  activeBoxes = boxes;
  numActiveBoxes = numBoxes;
  randomSeed(23);
  uint32_t now = millis();

  // Sparkle 0: the big gold one
  sparkles[0] = { (int16_t)goldX, (int16_t)goldY, 2, COL_GOLD, false, now + 400,
                  (uint16_t)(1200 * BLINK_SLOWNESS), (uint16_t)(700 * BLINK_SLOWNESS) };

  for (int i = 1; i < NUM_SPARKLES; i++) {
    int x = 0, y = 0;
    bool placed = false;
    for (int tries = 0; tries < 300 && !placed; tries++) {
      x = random(10, 310);
      y = random(8, 232);
      placed = insideBorder(x, y, 8) && !hitsUI(x, y, 8) &&
               !(abs(x - goldX) < 20 && abs(y - goldY) < 12);   // keep clear of the gold one
    }
    if (!placed) x = -100;            // no room left: parked off-screen (put() skips it)
    sparkles[i].x = x;
    sparkles[i].y = y;
    sparkles[i].size = (i % 3 == 0) ? 1 : 0;
    sparkles[i].color = (i % 4 == 0) ? COL_GOLD : COL_LAVENDER;
    sparkles[i].on = false;
    sparkles[i].onMs  = (uint16_t)(random(500, 1400) * BLINK_SLOWNESS);
    sparkles[i].offMs = (uint16_t)(random(300, 1600) * BLINK_SLOWNESS);
    sparkles[i].nextToggle = now + (uint32_t)(random(0, 2000) * BLINK_SLOWNESS);
  }

#if !BLINK_SPARKLES
  for (int i = 0; i < NUM_SPARKLES; i++) { sparkles[i].on = true; drawSparkle(i, true); }
#endif
}

void updateSparkles() {
  uint32_t now = millis();
  for (int i = 0; i < NUM_SPARKLES; i++) {
    Sparkle& s = sparkles[i];
    if ((int32_t)(now - s.nextToggle) >= 0) {
      s.on = !s.on;
      drawSparkle(i, s.on);
      s.nextToggle = now + (s.on ? s.onMs : s.offMs);
    }
  }
}

// ---- Screen -------------------------------------------------------------------------

// The main card: name, status text, FEELING strip. Drawn on top of the pink
// UPDATE panel's right edge, so it can be redrawn on its own (Clear does).
void drawProfileCard() {
  clearEmojiSpots();               // the card's emoji is the only one on this screen
  const int cx0 = 104, cy0 = 42, cw = 196, ch = 148, stripH = 44;
  tft.fillRoundRect(cx0, cy0, cw, ch, 6, COL_STRIP);
  tft.fillRoundRect(cx0, cy0, cw, ch - stripH, 6, COL_CARD);
  tft.fillRect(cx0, cy0 + ch - stripH - 6, cw, 6, COL_CARD);   // square off the strip's top
  tft.drawRoundRect(cx0 - 1, cy0 - 1, cw + 2, ch + 2, 7, COL_PINK_EDG);

  drawName(cx0 + 8, cy0 + 8);
  const int stripY = cy0 + ch - stripH;
  const int tx = cx0 + 8, ty = cy0 + 34, tw = cw - 16, th = stripY - 2 - ty;
  if (statusText[0]) fitText(statusText, tx, ty, tw, th, COL_TEXT);
  else fitText("Click New to create a new post", tx, ty, tw, th, COL_HINT);

  drawFeelingStrip(feeling, cx0, stripY, cw, stripH, 11, COL_CARD, true);
}

void drawScreen() {
  clearEmojiSpots();
  tft.fillScreen(ILI9341_BLACK);   // outside the border stays black
  drawBackground();

  // Left nav: New (rounded top-left) + Clear (rounded bottom-left), mirror images
  drawPanel(true, 45, 68, 48, 40, ROUND_START, 24);  // New post: mirrors Clear: rounded top-left
  drawPanel(true, 45, 119, 48, 40, ROUND_END, 24);
  // Icon + label centered in each tab (x 6..45; y 68..115 and 119..166)
  drawPlusIcon(26, 86);   centeredLabel("New",   26, 98);
  drawMinusIcon(26, 135); centeredLabel("Clear", 26, 145);

  // Pink UPDATE panel (partly tucked behind the card)
  tft.fillRoundRect(50, 72, 58, 80, 6, COL_PINK);
  drawAvatar(54, 78, 46, avatar);
  boldText("UPDATE", 59, 134, 1, COL_UPDATE);

  drawProfileCard();

  // Bottom nav: Home (rounded bottom-left) + Past (rounded bottom-right), mirror images.
  // 40 px tall like New/Clear are 40 px deep; together they span the card's width
  // (x 103..300, 4 px gap), 3 px below it. The border clips Past's outer corner.
  drawPanel(false, 103, 194, 97, 40, ROUND_START, 24); // mirrors Past: rounded bottom-left
  drawPanel(false, 204, 194, 97, 40, ROUND_END, 24);
  // Icon + label centered in each tab, laid out like New/Clear
  drawHomeIcon(151, 207);    centeredLabel("Home",    151, 219);
  drawClockIcon(252, 207);   centeredLabel("Past",    252, 219);

  drawTheSlapLogo();       // The Slap logo bitmap
  drawBorder(COL_BORDER);   // same coral border as before
}

// Home tab in the bottom nav (same box as its drawPanel() call in drawScreen())
bool homeTabAt(int x, int y) {
  return x >= 103 && x <= 199 && y >= 194 && y <= 233;
}

// Past tab in the bottom nav (same box as its drawPanel() call in drawScreen())
bool pastTabAt(int x, int y) {
  return x >= 204 && x <= 300 && y >= 194 && y <= 233;
}

// New post tab in the left nav (same box as its drawPanel() call in drawScreen())
bool newTabAt(int x, int y) {
  return x >= 6 && x <= 45 && y >= 68 && y <= 115;
}

// Clear tab in the left nav (same box as its drawPanel() call in drawScreen())
bool clearTabAt(int x, int y) {
  return x >= 6 && x <= 45 && y >= 119 && y <= 166;
}

// Clears the status: no text, no feeling. The name and "FEELING:" stay.
void clearStatus() {
  statusText[0] = 0;
  feeling = NO_FEELING;
  drawProfileCard();
}

// Profile picture or the pink UPDATE panel around it (same box as its
// fillRoundRect() in drawScreen(); the card covers it from x = 104)
bool avatarTabAt(int x, int y) {
  return x >= 50 && x <= 103 && y >= 72 && y <= 151;
}

// Draw the whole screen and start the sparkles
void show() {
  drawScreen();
  initSparkles();   // after the screen is drawn, so sparkles start on a finished background
}

// Call every loop() while TheSlap or the new post screen is showing
void update() {
#if BLINK_SPARKLES
  updateSparkles();
#endif
  updateEmojis();
}

// ---- New post screen ------------------------------------------------------------
//
// Two steps, both on TheSlap's starry background with blinking sparkles:
//
// 1. Typing (design-references/keyboard-reference.png)
//    The message card at the top (name top left, Cancel pill top right, the
//    text being typed, FEELING strip) and a keyboard filling the bottom half,
//    following the rounded border, in the card's blues.
//      Go       -> step 2 (only if something was typed)
//      123/ABC  -> switches between letters and numbers/punctuation
//      Cancel   -> back to TheSlap without posting; the draft is kept
//
// 2. Picking a feeling (design-references/selecting-emojis.png)
//    The keyboard hides, the card grows, and a tray of the 11 feeling emojis
//    appears under it. Tapping an emoji selects it and updates "FEELING: x".
//      Back -> step 1, to keep editing the text
//      Post -> posts the text and feeling to the card on TheSlap

// Message card (step 1). Step 2 uses the same card, taller.
const int BOX_X = 28, BOX_Y = 33, BOX_W = 264, BOX_H = 92, BOX_STRIP_H = 20;
const int BIG_BOX_H = 142, BIG_STRIP_H = 32;
const int TEXT_X = BOX_X + 8, TEXT_Y = BOX_Y + 26;
const int TEXT_COLS = 41, TEXT_LINES = 4, TEXT_LINE_H = 11;   // size-1 limits
const int TEXT_W = TEXT_COLS * 6;

// Pills in the card's top-right corner. Their tap areas are much bigger than
// the pills: everything above the text, out to the screen edge.
const int PILL_W = 46, PILL_H = 17, PILL_Y = BOX_Y + 6;
const int PILL_R_X = BOX_X + BOX_W - PILL_W - 6;     // Cancel (step 1) / Post (step 2)
const int PILL_L_X = PILL_R_X - PILL_W - 6;          // Back (step 2)
const int TAP_TOP_Y1 = TEXT_Y + 6;
const int TAP_R_X0 = PILL_R_X - 3;                   // right pill: from here to the screen edge
const int TAP_L_X0 = PILL_L_X - 20;                  // left pill: from here to TAP_R_X0

// Keyboard (step 1): fills the border from just under the card to the bottom
const int KB_Y = BOX_Y + BOX_H + 1;
const int KB_BOTTOM = SCREEN_H - 1 - LINE_THICKNESS;
const int KEY_H = 21, KEY_W = 27, KEY_GAP = 3;
const int ROW_Y[4] = { KB_Y + 5, KB_Y + 30, KB_Y + 55, KB_Y + 80 };

// Emoji tray (step 2), under the big card
const int TRAY_X = BOX_X, TRAY_Y = BOX_Y + BIG_BOX_H + 5, TRAY_W = BOX_W, TRAY_H = 36;
const int EMOJI_PITCH = TRAY_W / NUM_FEELINGS;       // 24 px per emoji
const int EMOJI_R = 9;

#define COL_KB_BG    C565(165, 222, 246)   // keyboard background (a bit deeper than the card)
#define COL_KEY      C565(228, 248, 255)   // letter keys (lightest blue)
#define COL_KEY_SPEC C565( 92, 176, 228)   // shift, backspace, 123, Go, Cancel, Back (darker blue)
#define COL_KEY_SHAD C565( 70, 132, 190)   // 1 px shadow under each key
#define COL_KEY_DOWN C565( 52, 128, 200)   // pressed

// What composeTap() did
const int COMPOSE_MISS = -1;     // the tap didn't hit anything
const int COMPOSE_NONE = 0;      // handled, stay on this screen
const int COMPOSE_CANCEL = 1, COMPOSE_POSTED = 2;

// Where sparkles must not go, per step
const Box TYPING_BOXES[] = {
  { BOX_X - 1, BOX_Y - 1, BOX_W + 2, BOX_H + 2 },             // message card
  { 0, KB_Y - 2, SCREEN_W, SCREEN_H - KB_Y + 2 },             // keyboard
};
const Box PICKING_BOXES[] = {
  { BOX_X - 1, BOX_Y - 1, BOX_W + 2, BIG_BOX_H + 2 },         // big message card
  { TRAY_X - 1, TRAY_Y - 4, TRAY_W + 2, TRAY_H + 6 },         // emoji tray
};

char draft[MAX_POST + 1] = "";
int draftLen = 0;
bool shiftOn = true;          // one-shot capitals, on for the first letter
bool numbersOn = false;       // 123 layer showing
bool picking = false;         // step 2 (emoji tray) showing
int pickedFeeling = FEELING_HAPPY;

// Special key codes (every other key types its own character)
const char KEY_SHIFT = 1, KEY_NUMS = 2, KEY_GO = 3, KEY_BACK = '\b';

struct Key { int16_t x, y, w; char code; };
Key keys[40];
int numKeys = 0;

// Fills rows y0..y1, clipped to the inside of the rounded border
void fillInside(int y0, int y1, uint16_t color) {
  for (int y = y0; y <= y1; y++) {
    int xl, xr;
    rowExtent(y, xl, xr);
    tft.drawFastHLine(xl, y, xr - xl + 1, color);
  }
}

void addKey(int x, int y, int w, char code) {
  keys[numKeys++] = { (int16_t)x, (int16_t)y, (int16_t)w, code };
}

// A row of equal keys, centered on the screen
void addRow(const char* s, int y) {
  int n = strlen(s);
  int x = (SCREEN_W - (n * KEY_W + (n - 1) * KEY_GAP)) / 2;
  for (int i = 0; i < n; i++, x += KEY_W + KEY_GAP) addKey(x, y, KEY_W, s[i]);
}

// Same 4 rows as the reference: QWERTYUIOP / ASDFGHJKL / shift ZXCVBNM back / 123 space Go.
// The bottom row is narrower so it fits inside the border's curve.
void initKeys() {
  numKeys = 0;
  addRow(numbersOn ? "1234567890" : "QWERTYUIOP", ROW_Y[0]);
  addRow(numbersOn ? "-/:;()$&@"  : "ASDFGHJKL",  ROW_Y[1]);

  const int sideW = 34;                       // shift / backspace
  addKey(16, ROW_Y[2], sideW, KEY_SHIFT);
  addRow(numbersOn ? ".,?!'\"#" : "ZXCVBNM", ROW_Y[2]);
  addKey(SCREEN_W - 16 - sideW, ROW_Y[2], sideW, KEY_BACK);

  const int bigW = 50, left = 48, right = SCREEN_W - 48;   // 123 / Go, inside the bottom curve
  addKey(left, ROW_Y[3], bigW, KEY_NUMS);
  addKey(left + bigW + 4, ROW_Y[3], (right - bigW - 4) - (left + bigW + 4), ' ');
  addKey(right - bigW, ROW_Y[3], bigW, KEY_GO);
}

// Prints s centered in a box
void centerIn(int x, int y, int w, int h, const char* s, uint8_t size, uint16_t ink) {
  int tw = strlen(s) * 6 * size - size;
  tft.setTextSize(size);
  tft.setTextColor(ink);
  tft.setCursor(x + (w - tw) / 2, y + (h - 8 * size) / 2 + 1);
  tft.print(s);
}

void drawKey(const Key& k, bool pressed) {
  bool special = (k.code == KEY_SHIFT || k.code == KEY_BACK || k.code == KEY_NUMS || k.code == KEY_GO);
  uint16_t fill = special ? COL_KEY_SPEC : COL_KEY;
  if (k.code == KEY_SHIFT && shiftOn && !numbersOn) fill = COL_KEY;   // lit up while on
  if (pressed) fill = COL_KEY_DOWN;
  uint16_t ink = (pressed || fill == COL_KEY_SPEC) ? ILI9341_WHITE : COL_NAME;

  tft.fillRoundRect(k.x, k.y + 1, k.w, KEY_H, 5, COL_KEY_SHAD);   // shadow peeks out below
  tft.fillRoundRect(k.x, k.y, k.w, KEY_H, 5, fill);

  int cx = k.x + k.w / 2, cy = k.y + KEY_H / 2;
  switch (k.code) {
    case KEY_SHIFT:                                  // up arrow (blank on the 123 layer)
      if (!numbersOn) {
        tft.fillTriangle(cx - 6, cy, cx + 6, cy, cx, cy - 6, ink);
        tft.fillRect(cx - 3, cy, 7, 5, ink);
      }
      break;
    case KEY_BACK:                                   // tag shape with an x
      tft.drawLine(cx - 9, cy, cx - 4, cy - 5, ink);
      tft.drawLine(cx - 9, cy, cx - 4, cy + 5, ink);
      tft.drawFastHLine(cx - 4, cy - 5, 13, ink);
      tft.drawFastHLine(cx - 4, cy + 5, 13, ink);
      tft.drawFastVLine(cx + 8, cy - 5, 11, ink);
      tft.drawLine(cx - 1, cy - 2, cx + 4, cy + 2, ink);
      tft.drawLine(cx - 1, cy + 2, cx + 4, cy - 2, ink);
      break;
    case KEY_NUMS: centerIn(k.x, k.y, k.w, KEY_H, numbersOn ? "ABC" : "123", 1, ink); break;
    case KEY_GO:   centerIn(k.x, k.y, k.w, KEY_H, "Go", 1, ink); break;
    case ' ':      centerIn(k.x, k.y, k.w, KEY_H, "space", 1, ink); break;
    default: {
      bool letter = (k.code >= 'A' && k.code <= 'Z');
      char s[2] = { (letter && !shiftOn) ? (char)(k.code + 32) : k.code, 0 };
      centerIn(k.x, k.y, k.w, KEY_H, s, 2, ink);
    }
  }
}

void drawKeyboard() {
  fillInside(KB_Y, KB_BOTTOM, COL_KB_BG);
  for (int i = 0; i < numKeys; i++) drawKey(keys[i], false);
  drawBorder(COL_BORDER);   // repaint the border's inner edge over the fill
}

// A PILL_W x PILL_H pill button with a white label
void drawPillAt(int x, int y, const char* label, uint16_t fill) {
  tft.fillRoundRect(x, y, PILL_W, PILL_H, 8, fill);
  centerIn(x, y, PILL_W, PILL_H, label, 1, ILI9341_WHITE);
}

// A pill button in the card's top-right corner
void drawPill(int x, const char* label, uint16_t fill) {
  drawPillAt(x, PILL_Y, label, fill);
}

// The card: light-blue body, cyan FEELING strip at the bottom, pink outline
void drawCard(int h, int stripH) {
  int stripY = BOX_Y + h - stripH;
  tft.fillRoundRect(BOX_X, BOX_Y, BOX_W, h, 6, COL_STRIP);
  tft.fillRoundRect(BOX_X, BOX_Y, BOX_W, h - stripH, 6, COL_CARD);
  tft.fillRect(BOX_X, stripY - 6, BOX_W, 6, COL_CARD);      // square off the strip's top
  tft.drawRoundRect(BOX_X - 1, BOX_Y - 1, BOX_W + 2, h + 2, 7, COL_PINK_EDG);
  drawName(BOX_X + 8, BOX_Y + 6);  // top left, like the posted card
}

// Redraws just the FEELING strip of the card (its bottom corners stay rounded)
void drawStrip(int h, int stripH, int emojiR) {
  int stripY = BOX_Y + h - stripH;
  tft.fillRoundRect(BOX_X, stripY - 6, BOX_W, stripH + 6, 6, COL_STRIP);
  tft.fillRect(BOX_X, stripY - 6, BOX_W, 6, COL_CARD);
  drawFeelingStrip(pickedFeeling, BOX_X, stripY, BOX_W, stripH, emojiR, COL_CARD);
}

// Typed text and the typing cursor (only the text area is redrawn)
void drawComposeText() {
  int stripY = BOX_Y + BOX_H - BOX_STRIP_H;
  tft.fillRect(BOX_X + 2, TEXT_Y - 2, BOX_W - 4, stripY - TEXT_Y + 2, COL_CARD);
  int col = 0, line = 0, th = stripY - 2 - TEXT_Y;
  uint8_t size;
  if (draftLen == 0) size = fitText("What's on your mind?", TEXT_X, TEXT_Y, TEXT_W, th, COL_HINT);
  else size = fitText(draft, TEXT_X, TEXT_Y, TEXT_W, th, COL_TEXT, true, &col, &line);
  tft.drawFastVLine(TEXT_X + col * 6 * size - 1, TEXT_Y + line * textLineH(size) - 1, 8 * size + 2, COL_PINK);
}

// One slot of the emoji tray (a still spot), highlighted when it's the picked one
void drawEmojiSlot(int i) {
  int cx = TRAY_X + EMOJI_PITCH / 2 + i * EMOJI_PITCH;
  EmojiSpot s = {};
  s.idx = i; s.cx = cx; s.cy = TRAY_Y + TRAY_H / 2 + 1; s.r = EMOJI_R;
  s.x = cx - EMOJI_PITCH / 2; s.y = TRAY_Y + 2; s.w = EMOJI_PITCH; s.h = TRAY_H - 4;
  s.splitY = s.y; s.above = s.below = COL_STRIP;
  s.trayHighlight = (i == pickedFeeling);
  s.animate = false;                                   // tray emojis stay still
  setEmojiSpot(1 + i, s);
}

void drawTypingScreen() {
  clearEmojiSpots();
  tft.fillScreen(ILI9341_BLACK);   // outside the border stays black
  drawBackground();                // TheSlap's gradient + stars
  drawCard(BOX_H, BOX_STRIP_H);
  drawPill(PILL_R_X, "Cancel", COL_KEY_SPEC);
  drawStrip(BOX_H, BOX_STRIP_H, 8);
  drawComposeText();
  drawKeyboard();                  // also draws the border
  initSparkles(TYPING_BOXES, 2, 60, 18);   // big gold sparkle top left, above the card
}

void drawPickingScreen() {
  clearEmojiSpots();
  tft.fillScreen(ILI9341_BLACK);
  drawBackground();
  drawCard(BIG_BOX_H, BIG_STRIP_H);
  drawPill(PILL_L_X, "Back", COL_KEY_SPEC);
  drawPill(PILL_R_X, "Post", COL_PINK);
  fitText(draft, TEXT_X, TEXT_Y, TEXT_W, BOX_Y + BIG_BOX_H - BIG_STRIP_H - 2 - TEXT_Y, COL_TEXT);
  drawStrip(BIG_BOX_H, BIG_STRIP_H, 12);

  // Emoji tray, same cyan as the strip, rounded like the reference
  tft.fillRoundRect(TRAY_X, TRAY_Y, TRAY_W, TRAY_H, 8, COL_STRIP);
  tft.drawRoundRect(TRAY_X - 1, TRAY_Y - 1, TRAY_W + 2, TRAY_H + 2, 9, COL_PINK_EDG);
  for (int i = 0; i < NUM_FEELINGS; i++) drawEmojiSlot(i);

  drawBorder(COL_BORDER);
  initSparkles(PICKING_BOXES, 2, 60, 18);
}

// Called when opening the new post screen from TheSlap
void showCompose() {
  picking = false;
  numbersOn = false;
  shiftOn = (draftLen == 0);
  if (draftLen == 0) pickedFeeling = FEELING_HAPPY;   // a new post starts on Happy
  initKeys();
  drawTypingScreen();
}

// Adds one character if it fits (length limit and the lines of the box)
void addChar(char c) {
  if (draftLen >= MAX_POST) return;
  draft[draftLen] = c;
  draft[draftLen + 1] = 0;
  if (layoutText(draft, 0, 0, TEXT_COLS, TEXT_LINE_H, 0, false) > TEXT_LINES) {
    draft[draftLen] = 0;           // doesn't fit: undo
    return;
  }
  draftLen++;
}

void pressKey(const Key& k) {
  drawKey(k, true);                // quick flash so you can see the press
  delay(60);

  char c = k.code;
  if (c == KEY_GO) {
    if (draftLen == 0) { drawKey(k, false); return; }   // nothing to post yet
    picking = true;
    drawPickingScreen();
    return;
  }

  bool redrawAll = false;
  if (c == KEY_NUMS) {
    numbersOn = !numbersOn;
    initKeys();
    redrawAll = true;
  } else if (c == KEY_SHIFT) {
    if (!numbersOn) { shiftOn = !shiftOn; redrawAll = true; }
  } else if (c == KEY_BACK) {
    if (draftLen > 0) draft[--draftLen] = 0;
  } else {
    if (c >= 'A' && c <= 'Z') {
      if (!shiftOn) c += 32;       // lowercase
      else { shiftOn = false; redrawAll = true; }   // one-shot: letters go back to lowercase
    }
    addChar(c);
  }

  if (redrawAll) drawKeyboard();
  else drawKey(k, false);
  drawComposeText();
}

bool inRect(int x, int y, int x0, int y0, int x1, int y1) {
  return x >= x0 && x <= x1 && y >= y0 && y <= y1;
}

int typingTap(int x, int y) {
  if (inRect(x, y, TAP_L_X0, 0, SCREEN_W - 1, TAP_TOP_Y1)) {   // Cancel
    drawPill(PILL_R_X, "Cancel", COL_KEY_DOWN);                // flash so you can see it registered
    return COMPOSE_CANCEL;
  }
  for (int i = 0; i < numKeys; i++) {
    const Key& k = keys[i];
    // Hit box reaches into the gaps so taps between keys still count
    if (inRect(x, y, k.x - 2, k.y - 2, k.x + k.w + 1, k.y + KEY_H + 2)) {
      pressKey(k);
      return COMPOSE_NONE;
    }
  }
  return COMPOSE_MISS;
}

int pickingTap(int x, int y) {
  if (inRect(x, y, TAP_R_X0, 0, SCREEN_W - 1, TAP_TOP_Y1)) {   // Post
    drawPill(PILL_R_X, "Post", COL_KEY_DOWN);
    strcpy(statusText, draft);
    feeling = pickedFeeling;
    addPast(draft, pickedFeeling);   // also goes into the Past list
    draft[0] = 0;
    draftLen = 0;
    return COMPOSE_POSTED;
  }
  if (inRect(x, y, TAP_L_X0, 0, TAP_R_X0 - 1, TAP_TOP_Y1)) {   // Back to the keyboard
    drawPill(PILL_L_X, "Back", COL_KEY_DOWN);
    picking = false;
    shiftOn = false;
    initKeys();
    drawTypingScreen();
    return COMPOSE_NONE;
  }
  if (inRect(x, y, TRAY_X - 4, TRAY_Y - 6, TRAY_X + TRAY_W + 3, TRAY_Y + TRAY_H + 6)) {
    int i = constrain((x - TRAY_X) / EMOJI_PITCH, 0, NUM_FEELINGS - 1);
    if (i != pickedFeeling) {
      int old = pickedFeeling;
      pickedFeeling = i;
      drawEmojiSlot(old);
      drawEmojiSlot(i);
      drawStrip(BIG_BOX_H, BIG_STRIP_H, 12);
    }
    return COMPOSE_NONE;
  }
  return COMPOSE_MISS;
}

// Handles a tap on the new post screen. Returns COMPOSE_CANCEL or COMPOSE_POSTED
// when it's time to go back to TheSlap screen, COMPOSE_NONE when the tap did
// something here, or COMPOSE_MISS if it hit nothing.
int composeTap(int x, int y) {
  return picking ? pickingTap(x, y) : typingTap(x, y);
}

// ---- Avatar screen ----------------------------------------------------------------
//
// Opened by tapping the profile picture or the pink UPDATE panel on TheSlap.
// Left: a big preview in a pink panel, with Cancel / Save under it.
// Right: a card with one row per option (gender, skin, hair, ...). Tapping a
// row's "<" goes to the previous choice, anywhere else on the row goes to the
// next one, and the preview redraws. Save keeps the changes, Cancel drops them.

const int AV_X = 36, AV_Y = 48, AV_S = 100;                 // preview
const int AV_PILL_Y = AV_Y + AV_S + 10;                     // Cancel / Save, under the preview
const int AV_CANCEL_X = AV_X, AV_SAVE_X = AV_X + AV_S - PILL_W;
const int OPT_X = 150, OPT_Y = 40, OPT_W = 148, OPT_ROW_H = 19;
const int OPT_H = NUM_OPTS * OPT_ROW_H + 8;
// The options card can sit anywhere (the portrait setup screen reuses it), so the
// arrows are measured from its left edge (ox) and width (ow)
const int OPT_ARROW_L_DX = 58;                              // "<" box (16 px wide)
const int OPT_ARROW_R_DX = 18;                              // ">" box, from the right edge

// What avatarTap() did
const int AVATAR_MISS = -1;      // the tap didn't hit anything
const int AVATAR_NONE = 0;       // handled, stay on this screen
const int AVATAR_CANCEL = 1, AVATAR_SAVED = 2;

const Box AVATAR_BOXES[] = {
  { AV_X - 6, AV_Y - 6, AV_S + 12, AV_PILL_Y + PILL_H - AV_Y + 8 },      // preview + pills
  { OPT_X - 1, OPT_Y - 1, OPT_W + 2, OPT_H + 2 },                        // options card
};

uint8_t editAvatar[NUM_OPTS];    // the avatar being edited (copied to `avatar` on Save)

void drawAvatarPreview() {
  drawAvatar(AV_X, AV_Y, AV_S, editAvatar);
}

void drawAvatarPill(int x, const char* label, uint16_t fill) {
  drawPillAt(x, AV_PILL_Y, label, fill);
}

// One option row: label, "<", the current choice (a name or a color swatch), ">"
void drawOptRow(int i, int ox, int oy, int ow) {
  int ry = oy + 4 + i * OPT_ROW_H, cy = ry + 9;
  int arrowL = ox + OPT_ARROW_L_DX, arrowR = ox + ow - OPT_ARROW_R_DX;
  tft.fillRect(ox + 2, ry, ow - 4, OPT_ROW_H - 1, COL_CARD);
  if (i > 0) tft.drawFastHLine(ox + 6, ry - 1, ow - 12, COL_STRIP);   // divider
  boldText(OPT_LABELS[i], ox + 6, ry + 5, 1, COL_NAME);

  tft.fillTriangle(arrowL + 11, cy - 5, arrowL + 11, cy + 5, arrowL + 5, cy, COL_PINK);
  tft.fillTriangle(arrowR + 5, cy - 5, arrowR + 5, cy + 5, arrowR + 11, cy, COL_PINK);

  int v = editAvatar[i];
  int vx = arrowL + 16, vw = arrowR - vx;                    // value area between the arrows
  const char* name = nullptr;
  uint16_t swatch = 0;
  switch (i) {
    case OPT_GENDER:   name = GENDERS[v];   break;
    case OPT_SKIN:     swatch = SKINS[v];   break;
    case OPT_HAIR:     name = HAIRS[v];     break;
    case OPT_HAIR_COL: swatch = HAIR_COLS[v]; break;
    case OPT_EYES:     name = EYE_NAMES[v]; break;
    case OPT_ACC:      name = ACCS[v];      break;
    case OPT_SHIRT:    swatch = SHIRTS[v];  break;
    case OPT_BG:       swatch = BACKDROPS[v]; break;
  }
  if (name) {
    centerIn(vx, ry, vw, OPT_ROW_H - 1, name, 1, COL_TEXT);
  } else {
    tft.fillRoundRect(vx + vw / 2 - 15, cy - 5, 30, 11, 3, swatch);
    tft.drawRoundRect(vx + vw / 2 - 15, cy - 5, 30, 11, 3, COL_TEXT);
  }
}

// The whole options card (background, outline, one row per option)
void drawOptCard(int ox, int oy, int ow) {
  tft.fillRoundRect(ox, oy, ow, OPT_H, 6, COL_CARD);
  tft.drawRoundRect(ox - 1, oy - 1, ow + 2, OPT_H + 2, 7, COL_PINK_EDG);
  for (int i = 0; i < NUM_OPTS; i++) drawOptRow(i, ox, oy, ow);
}

// A tap on the options card: "<" goes to the previous choice, anywhere else on
// the row to the next one. Updates editAvatar and redraws the row. Returns true
// if the tap hit the card (the caller redraws its preview).
bool optTap(int x, int y, int ox, int oy, int ow) {
  if (!inRect(x, y, ox - 4, oy, ox + ow + 3, oy + OPT_H - 1)) return false;
  int i = constrain((y - oy - 4) / OPT_ROW_H, 0, NUM_OPTS - 1);
  int n = OPT_COUNT[i];
  int arrowL = ox + OPT_ARROW_L_DX;
  bool back = x >= arrowL - 4 && x < arrowL + 20;             // the "<"
  editAvatar[i] = (editAvatar[i] + (back ? n - 1 : 1)) % n;
  drawOptRow(i, ox, oy, ow);
  return true;
}

void drawAvatarScreen() {
  clearEmojiSpots();
  tft.fillScreen(ILI9341_BLACK);   // outside the border stays black
  drawBackground();

  tft.fillRoundRect(AV_X - 4, AV_Y - 4, AV_S + 8, AV_S + 8, 6, COL_PINK);   // like the UPDATE panel
  drawAvatarPreview();
  drawAvatarPill(AV_CANCEL_X, "Cancel", COL_KEY_SPEC);
  drawAvatarPill(AV_SAVE_X, "Save", COL_PINK);

  drawOptCard(OPT_X, OPT_Y, OPT_W);

  drawBorder(COL_BORDER);
  initSparkles(AVATAR_BOXES, 2, 24, 200);
}

// Called when opening the avatar screen from TheSlap
void showAvatarEditor() {
  memcpy(editAvatar, avatar, NUM_OPTS);
  drawAvatarScreen();
}

// Handles a tap on the avatar screen. Returns AVATAR_CANCEL or AVATAR_SAVED when
// it's time to go back to TheSlap, AVATAR_NONE when the tap changed something
// here, or AVATAR_MISS if it hit nothing.
int avatarTap(int x, int y) {
  int pillTop = AV_PILL_Y - 6, pillBot = AV_PILL_Y + PILL_H + 8;
  int split = (AV_CANCEL_X + PILL_W + AV_SAVE_X) / 2;          // middle of the gap between the pills
  if (inRect(x, y, AV_CANCEL_X - 6, pillTop, split - 1, pillBot)) {
    drawAvatarPill(AV_CANCEL_X, "Cancel", COL_KEY_DOWN);       // flash so you can see it registered
    return AVATAR_CANCEL;
  }
  if (inRect(x, y, split, pillTop, AV_SAVE_X + PILL_W + 6, pillBot)) {
    drawAvatarPill(AV_SAVE_X, "Save", COL_KEY_DOWN);
    memcpy(avatar, editAvatar, NUM_OPTS);
    return AVATAR_SAVED;
  }
  if (optTap(x, y, OPT_X, OPT_Y, OPT_W)) {
    drawAvatarPreview();
    return AVATAR_NONE;
  }
  return AVATAR_MISS;
}

// ---- Past posts screens -------------------------------------------------------------
//
// 1. The list: every saved post as a "POST #N" row, newest first, 5 to a page.
//    The ▲ / ▼ buttons on the right page toward newer / older posts; Back
//    (centered under the list) goes back to TheSlap.
// 2. One post: the message card with its text and feeling over the starry
//    background, and a Back pill that returns to the list (same page).

const int PAST_ROWS = 5;                                   // rows per page
const int PAST_ROW_X = 56, PAST_ROW_Y = 38, PAST_ROW_W = 188, PAST_ROW_H = 26, PAST_PITCH = 31;
const int PAST_TITLE_X = 100, PAST_TITLE_Y = 14;
const int PAST_ARROW_X = 256, PAST_ARROW_W = 32, PAST_ARROW_H = 44;
const int PAST_UP_Y = PAST_ROW_Y;                                          // ▲ (newer)
const int PAST_DOWN_Y = PAST_ROW_Y + (PAST_ROWS - 1) * PAST_PITCH + PAST_ROW_H - PAST_ARROW_H;  // ▼ (older)
// Rows + arrow column: the part redrawn when the page changes
const int PAST_LIST_X0 = 50, PAST_LIST_Y0 = 34, PAST_LIST_X1 = 292,
          PAST_LIST_Y1 = PAST_ROW_Y + (PAST_ROWS - 1) * PAST_PITCH + PAST_ROW_H + 3;   // 191

// Back pills on both past screens: 25% bigger than the other pills, centered
// left to right, and drawn by drawBigPill()
const int BIG_PILL_W = PILL_W * 5 / 4, BIG_PILL_H = PILL_H * 5 / 4;   // 57 x 21
const int BIG_PILL_X = BOX_X + (BOX_W - BIG_PILL_W) / 2;
// List screen: centered in the space between the list and the bottom border
const int PAST_BACK_Y = (PAST_LIST_Y1 + SCREEN_H - LINE_THICKNESS - BIG_PILL_H) / 2;

const int PAST_MISS = -1;        // the tap didn't hit anything
const int PAST_NONE = 0;         // handled, stay on this screen (page changed)
const int PAST_BACK = 1, PAST_OPEN = 2;
const int PAST_POST_MISS = -1, PAST_POST_BACK = 1;

const Box PAST_BOXES[] = {
  { PAST_TITLE_X - 4, PAST_TITLE_Y - 4, 128, 24 },                                    // title
  { BIG_PILL_X - 4, PAST_BACK_Y - 4, BIG_PILL_W + 8, BIG_PILL_H + 8 },                 // Back pill
  { PAST_LIST_X0, PAST_LIST_Y0, PAST_LIST_X1 - PAST_LIST_X0 + 1, PAST_LIST_Y1 - PAST_LIST_Y0 + 1 },
};
// Back pill on the single post screen: centered in the space between the
// bottom of the message card and the bottom border
const int PP_BACK_Y = (BOX_Y + BIG_BOX_H + SCREEN_H - LINE_THICKNESS - BIG_PILL_H) / 2;
const Box PAST_POST_BOXES[] = {
  { BOX_X - 1, BOX_Y - 1, BOX_W + 2, BIG_BOX_H + 2 },      // big message card
  { BIG_PILL_X - 4, PP_BACK_Y - 4, BIG_PILL_W + 8, BIG_PILL_H + 8 }, // Back pill
};

// A centered BIG_PILL_W x BIG_PILL_H "Back" pill at height y
void drawBigBack(int y, uint16_t fill) {
  tft.fillRoundRect(BIG_PILL_X, y, BIG_PILL_W, BIG_PILL_H, BIG_PILL_H / 2, fill);
  centerIn(BIG_PILL_X, y, BIG_PILL_W, BIG_PILL_H, "Back", 1, ILI9341_WHITE);
}

int pastPage = 0;                // page showing (0 = the newest posts)
int pastOpen = 0;                // index into past[] of the post being viewed

int pastPages() { return max(1, (numPast + PAST_ROWS - 1) / PAST_ROWS); }

// Index into past[] for row i of the current page, or -1 if that row is empty
int pastIndexAt(int row) {
  int k = pastPage * PAST_ROWS + row;                      // 0 = newest
  return k < numPast ? numPast - 1 - k : -1;
}

void drawPastRow(int row, bool pressed) {
  int idx = pastIndexAt(row);
  if (idx < 0) return;
  int x = PAST_ROW_X, y = PAST_ROW_Y + row * PAST_PITCH;
  tft.fillRoundRect(x, y, PAST_ROW_W, PAST_ROW_H, 6, pressed ? COL_KEY_DOWN : COL_CARD);
  tft.drawRoundRect(x - 1, y - 1, PAST_ROW_W + 2, PAST_ROW_H + 2, 7, COL_PINK_EDG);
  char s[16];
  snprintf(s, sizeof(s), "POST #%u", (unsigned)past[idx].num);
  boldText(s, x + 10, y + (PAST_ROW_H - 14) / 2, 2, pressed ? ILI9341_WHITE : COL_NAME);
  int cx = x + PAST_ROW_W - 12, cy = y + PAST_ROW_H / 2;   // ">"
  tft.fillTriangle(cx - 5, cy - 6, cx - 5, cy + 6, cx + 2, cy, COL_PINK);
}

void drawPastArrow(bool up, bool pressed) {
  int y = up ? PAST_UP_Y : PAST_DOWN_Y;
  tft.fillRoundRect(PAST_ARROW_X, y, PAST_ARROW_W, PAST_ARROW_H, 8, pressed ? COL_KEY_DOWN : COL_PILL);
  tft.drawRoundRect(PAST_ARROW_X - 1, y - 1, PAST_ARROW_W + 2, PAST_ARROW_H + 2, 9, COL_PINK_EDG);
  int cx = PAST_ARROW_X + PAST_ARROW_W / 2, cy = y + PAST_ARROW_H / 2;
  if (up) tft.fillTriangle(cx - 8, cy + 5, cx + 8, cy + 5, cx, cy - 6, COL_PINK);
  else    tft.fillTriangle(cx - 8, cy - 5, cx + 8, cy - 5, cx, cy + 6, COL_PINK);
}

// The rows, arrows and page number (over whatever background is already there)
void drawPastList() {
  if (numPast == 0) {
    tft.fillRoundRect(PAST_ROW_X, PAST_ROW_Y, PAST_ROW_W, PAST_ROW_H, 6, COL_CARD);
    tft.drawRoundRect(PAST_ROW_X - 1, PAST_ROW_Y - 1, PAST_ROW_W + 2, PAST_ROW_H + 2, 7, COL_PINK_EDG);
    centerIn(PAST_ROW_X, PAST_ROW_Y, PAST_ROW_W, PAST_ROW_H, "No posts yet", 1, COL_HINT);
    return;
  }
  for (int i = 0; i < PAST_ROWS; i++) drawPastRow(i, false);
  int pages = pastPages();
  if (pastPage > 0) drawPastArrow(true, false);
  if (pastPage < pages - 1) drawPastArrow(false, false);
  char s[8];
  snprintf(s, sizeof(s), "%d/%d", pastPage + 1, pages);
  int midY = (PAST_UP_Y + PAST_ARROW_H + PAST_DOWN_Y) / 2;
  centerIn(PAST_ARROW_X - 4, midY - 6, PAST_ARROW_W + 8, 12, s, 1, COL_LAVENDER);
}

void drawPastScreen() {
  clearEmojiSpots();
  tft.fillScreen(ILI9341_BLACK);   // outside the border stays black
  drawBackground();
  drawBigBack(PAST_BACK_Y, COL_KEY_SPEC);
  boldText("PAST POSTS", PAST_TITLE_X, PAST_TITLE_Y, 2, COL_LAVENDER);
  drawPastList();
  drawBorder(COL_BORDER);
  initSparkles(PAST_BOXES, 3, 24, 200);
}

// Called when opening the Past screen from TheSlap (starts at the newest posts)
void showPast() {
  pastPage = 0;
  drawPastScreen();
}

// Back from a post: the same page as before
void showPastAgain() {
  pastPage = constrain(pastPage, 0, pastPages() - 1);
  drawPastScreen();
}

// Switches page, redrawing only the list area
void setPastPage(int page) {
  pastPage = page;
  drawBackground(PAST_LIST_X0, PAST_LIST_Y0, PAST_LIST_X1, PAST_LIST_Y1);
  drawPastList();
  drawBorder(COL_BORDER);          // the list area touches the border's curve
}

// Handles a tap on the list. Returns PAST_BACK (go to TheSlap), PAST_OPEN (show
// past[pastOpen]), PAST_NONE (changed page) or PAST_MISS.
int pastTap(int x, int y) {
  // Generous tap area: everything under the list, around the pill
  if (inRect(x, y, BIG_PILL_X - 30, PAST_LIST_Y1 + 1, BIG_PILL_X + BIG_PILL_W + 30, SCREEN_H - 1)) {
    drawBigBack(PAST_BACK_Y, COL_KEY_DOWN);   // flash so you can see it registered
    return PAST_BACK;
  }
  int ax0 = PAST_ARROW_X - 6, ax1 = PAST_ARROW_X + PAST_ARROW_W + 6;
  if (pastPage > 0 && inRect(x, y, ax0, PAST_UP_Y - 4, ax1, PAST_UP_Y + PAST_ARROW_H + 8)) {
    drawPastArrow(true, true);
    setPastPage(pastPage - 1);
    return PAST_NONE;
  }
  if (pastPage < pastPages() - 1 && inRect(x, y, ax0, PAST_DOWN_Y - 8, ax1, PAST_DOWN_Y + PAST_ARROW_H + 4)) {
    drawPastArrow(false, true);
    setPastPage(pastPage + 1);
    return PAST_NONE;
  }
  for (int i = 0; i < PAST_ROWS; i++) {
    int ry = PAST_ROW_Y + i * PAST_PITCH;
    int idx = pastIndexAt(i);
    // Hit box reaches into the gaps so taps between rows still count
    if (idx >= 0 && inRect(x, y, PAST_ROW_X - 6, ry - 3, PAST_ROW_X + PAST_ROW_W + 4, ry + PAST_ROW_H + 2)) {
      drawPastRow(i, true);
      pastOpen = idx;
      return PAST_OPEN;
    }
  }
  return PAST_MISS;
}

void drawPastPostScreen() {
  const PastPost& p = past[pastOpen];
  clearEmojiSpots();
  tft.fillScreen(ILI9341_BLACK);
  drawBackground();
  drawCard(BIG_BOX_H, BIG_STRIP_H);
  drawBigBack(PP_BACK_Y, COL_KEY_SPEC);
  int stripY = BOX_Y + BIG_BOX_H - BIG_STRIP_H;
  fitText(p.text, TEXT_X, TEXT_Y, TEXT_W, stripY - 2 - TEXT_Y, COL_TEXT);
  drawFeelingStrip(p.feeling, BOX_X, stripY, BOX_W, BIG_STRIP_H, 12, COL_CARD);
  drawBorder(COL_BORDER);
  initSparkles(PAST_POST_BOXES, 2, 60, 18);
}

// Called when a row on the list is tapped (pastOpen is already set)
void showPastPost() {
  drawPastPostScreen();
}

// Handles a tap on the single post screen. Returns PAST_POST_BACK or PAST_POST_MISS.
int pastPostTap(int x, int y) {
  // Generous tap area: everything under the card, around the pill
  if (inRect(x, y, BIG_PILL_X - 30, BOX_Y + BIG_BOX_H + 3, BIG_PILL_X + BIG_PILL_W + 30, SCREEN_H - 1)) {
    drawBigBack(PP_BACK_Y, COL_KEY_DOWN);
    return PAST_POST_BACK;
  }
  return PAST_POST_MISS;
}

} // namespace slap
