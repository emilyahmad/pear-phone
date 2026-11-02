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


enum PanelShape { WEDGE, ROUND_START, ROUND_END };

#define MAX_POST 140   // longest post, in characters
char statusText[MAX_POST + 1] = "Speed being miles morales isn't a speed move";   // the posted message

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

void drawBackground() {
  for (int y = LINE_THICKNESS; y <= SCREEN_H - 1 - LINE_THICKNESS; y++) {
    int xl, xr;
    rowExtent(y, xl, xr);
    tft.drawFastHLine(xl, y, xr - xl + 1, bgColor(y));
  }

#if STARS
  randomSeed(11);   // fixed seed: same stars every run
  for (int i = 0; i < 70; i++) {
    int x = random(8, 312), y = random(6, 234);
    if (!insideBorder(x, y, 4)) continue;
    uint16_t c = (i % 5 == 0) ? COL_GOLD : COL_LAVENDER;
    tft.drawPixel(x, y, c);
    if (i % 7 == 0) {
      tft.drawPixel(x - 1, y, c); tft.drawPixel(x + 1, y, c);
      tft.drawPixel(x, y - 1, c); tft.drawPixel(x, y + 1, c);
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

void centeredLabel(const char* s, int cx, int y) {
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  int w = strlen(s) * 6 - 1;
  tft.setCursor(cx - w / 2, y);
  tft.print(s);
}

// Word-wraps size-1 text into lines of at most maxChars characters (a word longer
// than a whole line gets split). Draws it when `draw` is true.
// Returns how many lines the text uses; endCol/endLine (optional) get the spot
// where the next character would go, for drawing a typing cursor.
int layoutText(const char* text, int x, int y, int maxChars, int lineH, uint16_t color,
               bool draw = true, int* endCol = nullptr, int* endLine = nullptr) {
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
    if (draw) tft.drawChar(x + col * 6, y + line * lineH, *p, color, color, 1);
    col++;
    lastLine = line;
  }
  if (endCol)  *endCol = col;
  if (endLine) *endLine = line;
  return lastLine + 1;
}

// ---- Icons ----------------------------------------------------------------------

void drawPlusIcon(int cx, int cy) {            // 15 x 15 plus sign (New post)
  tft.fillRoundRect(cx - 2, cy - 7, 5, 15, 1, COL_ICON);   // vertical bar
  tft.fillRoundRect(cx - 7, cy - 2, 15, 5, 1, COL_ICON);   // horizontal bar
}

void drawCameraIcon(int cx, int cy) {          // 22 x 15
  int x = cx - 11, y = cy - 7;
  tft.fillRect(cx - 4, y, 8, 3, COL_ICON);
  tft.fillRoundRect(x, y + 2, 22, 13, 2, COL_ICON);
  tft.fillCircle(cx, cy + 2, 4, COL_ICON_DK);
  tft.fillCircle(cx, cy + 2, 2, COL_ICON);
}

void drawHomeIcon(int cx, int cy) {
  tft.fillTriangle(cx - 11, cy - 1, cx + 11, cy - 1, cx, cy - 12, COL_CYAN);
  tft.fillRect(cx - 8, cy - 1, 16, 11, COL_CYAN);
  tft.fillRect(cx - 2, cy + 3, 5, 7, COL_PILL);   // door
}

void drawFriendsIcon(int cx, int cy) {         // three little people
  for (int i = 0; i < 3; i++) {
    int bx = cx - 10 + 10 * i;
    int h = (i == 1) ? 14 : 11;
    tft.fillCircle(bx, cy + 8 - h - 4, 3, COL_GRAY);
    tft.fillRoundRect(bx - 4, cy + 8 - h, 8, h, 3, COL_GRAY);
  }
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
const int FEELING_SLEEPY = 9;
int feeling = FEELING_SLEEPY;   // the posted feeling, shown on the card

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

void fist(int cx, int cy) {
  g->fillCircle(cx, cy, es(3), COL_FIST);
  g->drawCircle(cx, cy, es(3), COL_FACE_LN);
  g->drawFastHLine(cx - es(2), cy, es(4), COL_FACE_LN);     // finger crease
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
    case 1:   // Super pumped: fists pumping (taking turns), laughing
      fist(cx - es(8), cy - es(9) - (up ? 1 : 0));
      fist(cx + es(8), cy - es(9) - (up ? 0 : 1));
      closedEye(cx - es(4), cy - es(2), true);
      closedEye(cx + es(4), cy - es(2), true);
      grin(cx, cy + es(2), es(6), es(5));
      break;
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
      g->fillCircle(cx, cy + es(5), max(1, es(1.8f)), C565(220, 40, 40));
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
void drawFeelingStrip(int idx, int x0, int stripY, int w, int h, int emojiR, uint16_t above) {
  int ty = stripY + (h - 8) / 2;
  boldText("FEELING:", x0 + 8, ty, 1, COL_FEEL);

  // Emoji spot: keeps clear of the card's rounded bottom-right corner
  int ex = x0 + w - 12 - emojiR, ey = stripY + h / 2;
  int hw = emojiR * 13 / 10 + 2;                       // half width, incl. accessories + 1 px motion
  int top = max(ey - (emojiR * 14 / 10 + 2), stripY - 6);
  int bottom = min(ey + emojiR + 2, stripY + h - 1);

  int maxChars = (ex - hw - 2 - (x0 + 62)) / 6;
  const char* name = FEELINGS[idx];
  int lines = layoutText(name, 0, 0, maxChars, 10, 0, false);
  int ny = (lines > 1) ? stripY + (h - 18) / 2 : ty;
  layoutText(name, x0 + 62, ny, maxChars, 10, COL_MOOD);       // drawn twice = bold
  layoutText(name, x0 + 63, ny, maxChars, 10, COL_MOOD);

  EmojiSpot s = {};
  s.idx = idx; s.cx = ex; s.cy = ey; s.r = emojiR;
  s.x = ex - hw; s.y = top; s.w = 2 * hw + 1; s.h = bottom - top + 1;
  s.splitY = stripY; s.above = above; s.below = COL_STRIP;
  s.animate = true;                                    // the selected emoji is the only one that moves
  setEmojiSpot(0, s);
}

// Placeholder avatar (swap for your photo bitmap later)
void drawAvatarPlaceholder(int x, int y, int s) {
  int cx = x + s / 2, cy = y + s / 2;
  tft.fillRect(x, y, s, s, C565(232, 216, 192));
  tft.fillRect(cx - 16, y + s - 12, 32, 12, C565(245, 245, 245));   // shirt
  tft.fillCircle(cx, cy - 4, 17, C565(50, 30, 20));                 // curly hair
  tft.fillCircle(cx, cy - 1, 12, C565(190, 140, 105));              // face
  tft.drawRect(cx - 10, cy - 4, 8, 6, COL_ICON_DK);                 // glasses
  tft.drawRect(cx + 2,  cy - 4, 8, 6, COL_ICON_DK);
  tft.drawFastHLine(cx - 2, cy - 2, 4, COL_ICON_DK);
  tft.drawRect(x, y, s, s, ILI9341_WHITE);
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

void drawPanel(bool vertical, int x, int y, int len, int depth, PanelShape shape, int r) {
  // Fill
  for (int i = 0; i < len; i++) {
    int d = panelDepth(vertical, x, y, shape, i, len, depth, r);
    if (vertical) tft.drawFastHLine(x - d + 1, y + i, d, COL_PILL);
    else          tft.drawFastVLine(x + i, y, d, COL_PILL);
  }
  // Outline: straight edge, shaped edge, and the two end sides
  int prev = 0;
  for (int i = 0; i < len; i++) {
    int d = panelDepth(vertical, x, y, shape, i, len, depth, r);
    if (vertical) {
      tft.drawPixel(x, y + i, COL_PINK_EDG);
      int ex = x - d + 1;
      if (i == 0) tft.drawFastHLine(ex, y, d, COL_PINK_EDG);
      else        tft.drawLine(prev, y + i - 1, ex, y + i, COL_PINK_EDG);
      if (i == len - 1) tft.drawFastHLine(ex, y + i, d, COL_PINK_EDG);
      prev = ex;
    } else {
      tft.drawPixel(x + i, y, COL_PINK_EDG);
      int ey = y + d - 1;
      if (i == 0) tft.drawFastVLine(x, y, d, COL_PINK_EDG);
      else        tft.drawLine(x + i - 1, prev, x + i, ey, COL_PINK_EDG);
      if (i == len - 1) tft.drawFastVLine(x + i, y, d, COL_PINK_EDG);
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
  {  6,  68,  40,  99 },   // left nav panels (New + Photos)
  { 50,  72,  58,  80 },   // pink update panel
  { 102,  40, 200, 132 },  // message card
  { 104, 182, 196,  44 },  // bottom nav panels (Home + Friends)
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

void drawScreen() {
  clearEmojiSpots();
  tft.fillScreen(ILI9341_BLACK);   // outside the border stays black
  drawBackground();

  // Left nav: New (rounded top-left) + Photos (rounded bottom-left), mirror images
  drawPanel(true, 45, 68, 48, 40, ROUND_START, 24);  // New post: mirrors Photos: rounded top-left
  drawPanel(true, 45, 119, 48, 40, ROUND_END, 24);
  drawPlusIcon(26, 94);    centeredLabel("New",    26, 106);
  drawCameraIcon(26, 132); centeredLabel("Photos", 26, 144);

  // Pink UPDATE panel (partly tucked behind the card)
  tft.fillRoundRect(50, 72, 58, 80, 6, COL_PINK);
  drawAvatarPlaceholder(54, 78, 46);
  boldText("UPDATE", 59, 134, 1, COL_UPDATE);

  // Main card
  const int cx0 = 104, cy0 = 42, cw = 196, ch = 128, stripH = 28;
  tft.fillRoundRect(cx0, cy0, cw, ch, 6, COL_STRIP);
  tft.fillRoundRect(cx0, cy0, cw, ch - stripH, 6, COL_CARD);
  tft.fillRect(cx0, cy0 + ch - stripH - 6, cw, 6, COL_CARD);   // square off the strip's top
  tft.drawRoundRect(cx0 - 1, cy0 - 1, cw + 2, ch + 2, 7, COL_PINK_EDG);

  boldText("LOPI DOPI:", cx0 + 8, cy0 + 8, 2, COL_NAME);
  layoutText(statusText, cx0 + 8, cy0 + 34, 30, 11, COL_TEXT);

  const int stripY = cy0 + ch - stripH;
  drawFeelingStrip(feeling, cx0, stripY, cw, stripH, 8, COL_CARD);

  // Bottom nav: Home (rounded bottom-left) + Friends (rounded bottom-right), mirror images
  // Together they span the message card (x 104..299): two 97 px tabs with a 2 px gap
  drawPanel(false, 104, 182, 97, 44, ROUND_START, 26); // mirrors Friends: rounded bottom-left
  drawPanel(false, 203, 182, 97, 44, ROUND_END, 26);
  drawHomeIcon(152, 198);    centeredLabel("Home",    152, 211);   // centered in each tab
  drawFriendsIcon(251, 198); centeredLabel("Friends", 251, 211);

  drawTheSlapLogo();       // The Slap logo bitmap
  drawBorder(COL_BORDER);   // same coral border as before
}

// Home tab in the bottom nav (same box as its drawPanel() call in drawScreen())
bool homeTabAt(int x, int y) {
  return x >= 104 && x <= 200 && y >= 182 && y <= 225;
}

// New post tab in the left nav (same box as its drawPanel() call in drawScreen())
bool newTabAt(int x, int y) {
  return x >= 6 && x <= 45 && y >= 68 && y <= 115;
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
const int BIG_BOX_H = 142, BIG_STRIP_H = 26;
const int TEXT_X = BOX_X + 8, TEXT_Y = BOX_Y + 26;
const int TEXT_COLS = 41, TEXT_LINES = 4, TEXT_LINE_H = 11;

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

#define COL_HINT     C565(120, 140, 165)   // "What's on your mind?" placeholder
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
int pickedFeeling = FEELING_SLEEPY;

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

// A pill button in the card's top-right corner
void drawPill(int x, const char* label, uint16_t fill) {
  tft.fillRoundRect(x, PILL_Y, PILL_W, PILL_H, 8, fill);
  centerIn(x, PILL_Y, PILL_W, PILL_H, label, 1, ILI9341_WHITE);
}

// The card: light-blue body, cyan FEELING strip at the bottom, pink outline
void drawCard(int h, int stripH) {
  int stripY = BOX_Y + h - stripH;
  tft.fillRoundRect(BOX_X, BOX_Y, BOX_W, h, 6, COL_STRIP);
  tft.fillRoundRect(BOX_X, BOX_Y, BOX_W, h - stripH, 6, COL_CARD);
  tft.fillRect(BOX_X, stripY - 6, BOX_W, 6, COL_CARD);      // square off the strip's top
  tft.drawRoundRect(BOX_X - 1, BOX_Y - 1, BOX_W + 2, h + 2, 7, COL_PINK_EDG);
  boldText("LOPI DOPI:", BOX_X + 8, BOX_Y + 6, 2, COL_NAME);  // top left, like the posted card
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
  int col = 0, line = 0;
  if (draftLen == 0) layoutText("What's on your mind?", TEXT_X, TEXT_Y, TEXT_COLS, TEXT_LINE_H, COL_HINT);
  else layoutText(draft, TEXT_X, TEXT_Y, TEXT_COLS, TEXT_LINE_H, COL_TEXT, true, &col, &line);
  tft.drawFastVLine(TEXT_X + col * 6 - 1, TEXT_Y + line * TEXT_LINE_H - 1, 10, COL_PINK);
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
  layoutText(draft, TEXT_X, TEXT_Y, TEXT_COLS, TEXT_LINE_H, COL_TEXT);
  drawStrip(BIG_BOX_H, BIG_STRIP_H, 10);

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
  if (draftLen == 0) pickedFeeling = feeling;   // start from the current feeling
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
      drawStrip(BIG_BOX_H, BIG_STRIP_H, 10);
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

} // namespace slap
