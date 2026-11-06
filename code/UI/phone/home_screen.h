// Home screen (portrait 240x320, rotation 2). Copied from ../home-screen.ino.
// Everything is inside `namespace home` so names don't clash with the Slap screen.
#pragma once

namespace home {

constexpr int ROTATION = 2;               // portrait, flipped (use 0 to flip back)

#define RADIAL_BG      true      // false = flat dark maroon background

constexpr int CORNER_RADIUS =  70;        // same border as before (max 120)
constexpr int LINE_THICKNESS = 3;

#define ICON_R     16            // icon radius (32 px circles)
#define PITCH_X    54            // horizontal distance between icon centers
#define LABEL_GAP  4             // pixels between circle and label

// Curve: icons farther from the center line rise by ARC_DEPTH * (distance / ARC_HALF_WIDTH)^2
#define ARC_DEPTH       22.0f    // positive = smile (edges up), negative = frown, 0 = flat
#define ARC_HALF_WIDTH  110.0f   // smaller = tighter curve

constexpr int SCREEN_W = 240;
constexpr int SCREEN_H = 320;


struct App {
  const char* name;
  void (*drawIcon)(int cx, int cy, int r);
};

const uint16_t WHITE = ILI9341_WHITE;
const uint16_t BLACK = ILI9341_BLACK;

// ---- Icon drawing helpers ---------------------------------------------------

// 0xRRGGBB -> 16-bit color
uint16_t rgb(uint32_t hex) {
  return tft.color565((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF);
}

// Blend two 0xRRGGBB colors, t = 0 (a) .. 1 (b)
uint16_t mix(uint32_t a, uint32_t b, float t) {
  if (t < 0) t = 0;
  if (t > 1) t = 1;
  int r = ((a >> 16) & 0xFF) + (int)((((int)((b >> 16) & 0xFF)) - (int)((a >> 16) & 0xFF)) * t);
  int g = ((a >> 8) & 0xFF)  + (int)((((int)((b >> 8) & 0xFF))  - (int)((a >> 8) & 0xFF))  * t);
  int bl = (a & 0xFF)        + (int)((((int)(b & 0xFF))         - (int)(a & 0xFF))         * t);
  return tft.color565(r, g, bl);
}

// Half the width of a circle of radius r at vertical offset d
int halfWidth(int r, int d) {
  int v = r * r - d * d;
  return v > 0 ? (int)sqrtf((float)v) : 0;
}

// One horizontal row of a filled circle
void circleRow(int cx, int cy, int r, int dy, uint16_t c) {
  if (dy < -r || dy > r) return;
  int w = halfWidth(r, dy);
  tft.drawFastHLine(cx - w, cy + dy, 2 * w + 1, c);
}

// Circle with a top -> bottom gradient
void gradCircle(int cx, int cy, int r, uint32_t top, uint32_t bot) {
  for (int dy = -r; dy <= r; dy++)
    circleRow(cx, cy, r, dy, mix(top, bot, (dy + r) / (2.0f * r)));
}

// Circle with a top -> middle -> bottom gradient
void gradCircle3(int cx, int cy, int r, uint32_t top, uint32_t mid, uint32_t bot) {
  for (int dy = -r; dy <= r; dy++) {
    float t = (dy + r) / (2.0f * r);
    circleRow(cx, cy, r, dy, t < 0.5f ? mix(top, mid, t * 2) : mix(mid, bot, (t - 0.5f) * 2));
  }
}

// Circle with a left -> right gradient
void gradCircleH(int cx, int cy, int r, uint32_t left, uint32_t right) {
  for (int dx = -r; dx <= r; dx++) {
    int h = halfWidth(r, dx);
    tft.drawFastVLine(cx + dx, cy - h, 2 * h + 1, mix(left, right, (dx + r) / (2.0f * r)));
  }
}

void fillEllipse(int cx, int cy, int rx, int ry, uint16_t c) {
  for (int dy = -ry; dy <= ry; dy++) {
    float f = 1.0f - (float)(dy * dy) / (ry * ry);
    int w = (int)(rx * sqrtf(f < 0 ? 0 : f));
    tft.drawFastHLine(cx - w, cy + dy, 2 * w + 1, c);
  }
}

// Point at distance d and angle a (radians, 0 = right, clockwise) from the center
int px(int cx, float d, float a) { return cx + (int)lroundf(d * cosf(a)); }
int py(int cy, float d, float a) { return cy + (int)lroundf(d * sinf(a)); }

// ---- App icons (based on /images/UI/app-icons/, drawn for ICON_R = 16) ------

// 1.png: silver gradient, black camera with grey lens and yellow dot
void iconCamera(int cx, int cy, int r) {
  gradCircle(cx, cy, r, 0xEDEDEF, 0x9A9CA1);
  tft.fillRoundRect(cx - 4, cy - 9, 9, 4, 1, BLACK);    // top hump
  tft.fillRoundRect(cx - 10, cy - 6, 21, 13, 3, BLACK); // body
  tft.fillCircle(cx, cy, 5, rgb(0x9A9A9A));             // lens ring
  tft.fillCircle(cx, cy, 3, BLACK);
  tft.fillCircle(cx + 7, cy - 3, 1, rgb(0xF5C518));     // flash dot
}

// 2.png: green gradient, white speech bubble
void iconMessages(int cx, int cy, int r) {
  gradCircle(cx, cy, r, 0x5CF27A, 0x14C036);
  fillEllipse(cx + 1, cy - 1, 10, 7, WHITE);
  tft.fillTriangle(cx - 7, cy + 3, cx - 10, cy + 7, cx - 2, cy + 5, WHITE); // tail (short, stays clear of the circle edge)
}

// 3.png: blue, white "Sf"
void iconSplashFace(int cx, int cy, int r) {
  gradCircle(cx, cy, r, 0x2A4CA6, 0x3D66BC);
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(cx - 11, cy - 7);
  tft.print("Sf");
}

// 4.png: light blue -> deep blue -> cyan, white candlestick chart
void iconStocks(int cx, int cy, int r) {
  gradCircle3(cx, cy, r, 0xB4CCF5, 0x0E5FE0, 0x86DDF7);
  const int8_t bodyTop[8] = {2, 0, -2, -5, -6, -3, 0, 1};
  const int8_t bodyLen[8] = {3, 3, 4, 4, 3, 4, 3, 2};
  for (int i = 0; i < 8; i++) {
    int x = cx - 11 + i * 3;
    tft.drawFastVLine(x, cy + bodyTop[i] - 2, bodyLen[i] + 4, WHITE); // wick
    tft.fillRect(x, cy + bodyTop[i], 2, bodyLen[i], WHITE);           // body
  }
}

// 5.png / design-references/apple-maps-app.png: map tiles (green park, pink and
// yellow blocks, grey curved road, white streets), blue road up to a blue
// location dot with a white navigation arrow
const int MAPS_DOT_DX = -2, MAPS_DOT_DY = 2;   // location dot offset from center

// Map color at (u, v), both in -1..1 across the icon
uint32_t mapsColor(float u, float v) {
  float uc = (float)MAPS_DOT_DX / ICON_R, vc = (float)MAPS_DOT_DY / ICON_R;
  float diag = v - (0.15f + 0.45f * u);              // white street, top-left to bottom-right
  if (fabsf(diag) < 0.09f) return 0xFFFFFF;
  if (diag < 0) {                                    // above the street
    if (v < vc) {                                    // blue road up from the dot
      float bu = fabsf(u - uc);
      if (bu < 0.13f) return 0x2F7CF6;
      if (bu < 0.21f) return 0xFFFFFF;
    }
    if (u < uc) return 0xEDEDED;                     // light block, top left
    float d = sqrtf((u - 1) * (u - 1) + (v + 1) * (v + 1));
    if (d > 0.55f && d < 0.72f) return 0xA9A9A9;     // grey curved road
    if (d > 0.48f && d < 0.79f) return 0xFFFFFF;
    return 0x6CD66E;                                 // green park
  }
  if (u < -0.62f) return 0xF47FCB;                   // pink block, left
  if (u < -0.48f) return 0xFFFFFF;
  float y = (u - 0.3f) + 0.35f * (v - 0.8f);         // yellow road, bottom
  if (fabsf(y) < 0.16f) return 0xFFC830;
  if (fabsf(y) < 0.23f) return 0xFFFFFF;
  return 0xF3EFE6;                                   // beige block
}

void iconMaps(int cx, int cy, int r) {
  // Paint the map pixel by pixel, sending runs of one color as a single line
  for (int dy = -r; dy <= r; dy++) {
    int w = halfWidth(r, dy);
    int start = -w;
    uint32_t c = mapsColor((float)start / r, (float)dy / r);
    for (int dx = -w + 1; dx <= w + 1; dx++) {
      uint32_t n = dx <= w ? mapsColor((float)dx / r, (float)dy / r) : ~c;
      if (n != c) {
        tft.drawFastHLine(cx + start, cy + dy, dx - start, rgb(c));
        start = dx;
        c = n;
      }
    }
  }
  int x = cx + MAPS_DOT_DX, y = cy + MAPS_DOT_DY;
  tft.fillCircle(x, y, 7, WHITE);                           // dot outline
  tft.fillCircle(x, y, 6, rgb(0x1E7CF2));                   // blue dot
  tft.fillTriangle(x, y - 4, x - 3, y + 3, x + 3, y + 3, WHITE);   // arrow
  tft.fillTriangle(x - 3, y + 4, x + 3, y + 4, x, y + 1, rgb(0x1E7CF2)); // arrow notch
}

// 6.png: white, rainbow flower
void iconPhotos(int cx, int cy, int r) {
  tft.fillCircle(cx, cy, r, WHITE);
  const uint32_t petals[8] = {0xF9A11B, 0xF0E21C, 0x9CD54A, 0x3EC28F,
                              0x5A9FDE, 0x9B7CC8, 0xE0679F, 0xF47174};
  for (int i = 0; i < 8; i++) {
    float a = -PI / 2 + i * PI / 4;     // start at the top, go clockwise
    tft.fillCircle(px(cx, 6, a), py(cy, 6, a), 4, rgb(petals[i]));
  }
  tft.fillCircle(cx, cy, 1, WHITE);
}

// 7.png: blue -> cyan (left to right), yellow sun behind a white cloud
void iconWeather(int cx, int cy, int r) {
  gradCircleH(cx, cy, r, 0x2A80EE, 0x7CE2FC);
  tft.fillCircle(cx - 6, cy - 4, 5, rgb(0xF7D10F));
  tft.fillCircle(cx - 3, cy + 4, 4, WHITE);
  tft.fillCircle(cx + 2, cy + 1, 5, WHITE);
  tft.fillCircle(cx + 8, cy + 4, 4, WHITE);
  tft.fillRoundRect(cx - 7, cy + 3, 19, 6, 3, WHITE);
}

// 8.png / design-references/apple-notes-app.png: yellow top, row of grey
// dots, light grey paper with two grey ruled lines
void iconNotes(int cx, int cy, int r) {
  for (int dy = -r; dy <= r; dy++) {
    float t = (dy + r) / (2.0f * r);
    uint16_t c;
    if (dy <= -8)      c = mix(0xFFD52E, 0xFFE15A, (dy + r) / 8.0f);   // yellow band
    else if (dy == -7) c = rgb(0xFFF3C4);                               // soft edge
    else               c = mix(0xFBFBFB, 0xE6E6E6, t);                 // paper
    circleRow(cx, cy, r, dy, c);
  }
  int w = halfWidth(r, -5);
  for (int x = -w + 2; x <= w - 2; x += 3) tft.drawPixel(cx + x, cy - 5, rgb(0x9C9C9C)); // dots
  for (int ly : {1, 8}) tft.fillRoundRect(cx - 10, cy + ly, 21, 2, 1, rgb(0xB4B4B4));   // ruled lines
}

// 9.png: pink -> purple gradient, white ring with a music note
void iconPearTunes(int cx, int cy, int r) {
  gradCircle(cx, cy, r, 0xF660C4, 0xA64CF2);
  tft.drawCircle(cx, cy, 11, WHITE);
  tft.drawCircle(cx, cy, 10, WHITE);
  tft.fillCircle(cx - 4, cy + 4, 2, WHITE);           // note heads
  tft.fillCircle(cx + 3, cy + 3, 2, WHITE);
  tft.drawFastVLine(cx - 2, cy - 4, 8, WHITE);        // stems
  tft.drawFastVLine(cx + 5, cy - 5, 8, WHITE);
  tft.drawLine(cx - 2, cy - 4, cx + 5, cy - 5, WHITE); // beam
  tft.drawLine(cx - 2, cy - 3, cx + 5, cy - 4, WHITE);
}

// 10.png: silver gradient, dark inner circle, light grey outlined gear
void iconSettings(int cx, int cy, int r) {
  gradCircle(cx, cy, r, 0xE8E8EA, 0x96989C);
  uint16_t dark = rgb(0x4B4B4D), gear = rgb(0xA9A9AB);
  tft.fillCircle(cx, cy, 12, dark);
  for (int i = 0; i < 8; i++) {
    float a = i * PI / 4;
    tft.fillCircle(px(cx, 7, a), py(cy, 7, a), 2, gear);  // teeth
  }
  tft.fillCircle(cx, cy, 6, gear);
  tft.fillCircle(cx, cy, 4, dark);   // hollow middle
  tft.drawCircle(cx, cy, 2, gear);   // center ring
}

// 11.png: dark rim, white clock face with ticks, hands near 12
void iconClock(int cx, int cy, int r) {
  tft.fillCircle(cx, cy, r, rgb(0x2A2B26));
  tft.fillCircle(cx, cy, 12, WHITE);
  for (int i = 0; i < 12; i++) {
    float a = i * PI / 6;
    tft.drawLine(px(cx, 9, a), py(cy, 9, a), px(cx, 11, a), py(cy, 11, a), BLACK);
  }
  tft.drawLine(cx, cy, cx, cy - 9, BLACK);      // minute hand
  tft.drawLine(cx, cy, cx - 3, cy - 7, BLACK);  // hour hand
  tft.fillCircle(cx, cy, 1, BLACK);
}

// 12.png: teal -> cyan, curved black/white clapperboard stripe near the top
void iconVideos(int cx, int cy, int r) {
  gradCircle(cx, cy, r, 0x3ED6B0, 0x52C8E8);
  for (int dx = -r; dx <= r; dx++) {
    int h = halfWidth(r, dx);
    int top = -9 - (dx * dx) / 40;        // band dips in the middle
    int y0 = max(top, -h), y1 = min(top + 5, h);
    uint16_t c = ((dx + r) / 4) % 2 ? WHITE : BLACK;
    if (y1 >= y0) tft.drawFastVLine(cx + dx, cy + y0, y1 - y0 + 1, c);
  }
}

// Left to right, top to bottom
const App APPS[] = {
  // row 1 (2 apps)
  {"Messages",   iconMessages},   {"Camera",   iconCamera},
  // row 2 (3 apps)
  {"SplashFace", iconSplashFace}, {"Stocks",   iconStocks},  {"Maps",  iconMaps},
  // row 3 (3 apps)
  {"Photos",     iconPhotos},     {"Weather",  iconWeather}, {"Notes", iconNotes},
  // row 4 (4 apps)
  {"PearTunes",  iconPearTunes},  {"Settings", iconSettings},
  {"Clock",      iconClock},      {"Videos",   iconVideos},
};

const int NUM_ROWS = 4;
const int ROW_COUNT[NUM_ROWS] = {2, 3, 3, 4};
const int ROW_Y[NUM_ROWS]     = {70, 126, 182, 244};   // y of the row's center-line icon
const int SCREEN_CX = 120;

// ---- Background clipped to the rounded rectangle --------------------------

int clipL = 0, clipR = SCREEN_W - 1;   // current row's left/right limits

// Horizontal line clamped to the rounded rectangle's width on this row
void hl(int x0, int x1, int y, uint16_t c) {
  if (x0 < clipL) x0 = clipL;
  if (x1 > clipR) x1 = clipR;
  if (x1 >= x0) tft.drawFastHLine(x0, y, x1 - x0 + 1, c);
}

// Inner edge of the border (rounded rectangle) on row y
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

void drawBackground() {
  const int RINGS = 24;
  const int GX = 120, GY = 160;          // glow center (middle of the screen)
  const float RX0 = 125.0f, RY0 = 175.0f; // glow fades to the base color at the screen edges

  // Pink glow sampled from design-references/pink.png, center -> edge in 10% steps
  const uint8_t PINK[11][3] = {
    {238, 40, 152}, {233, 39, 143}, {220, 29, 123}, {195, 27, 104},
    {168, 23,  83}, {145, 19,  63}, {123, 14,  49}, {101, 12,  38},
    { 79,  9,  27}, { 56,  5,  16}, { 35,  2,   6},
  };
  uint16_t baseColor = tft.color565(34, 2, 5);   // near-black maroon edge

  uint16_t ringColor[RINGS];
  float rxk[RINGS], ryk[RINGS];
  for (int k = 0; k < RINGS; k++) {
    float f = (float)k / (RINGS - 1);    // 0 = outer, 1 = center
    float d = 1.0f - 0.95f * f;          // ring size: 1 = edge, 0.05 = center
    rxk[k] = RX0 * d;
    ryk[k] = RY0 * d;
    float p = d * 10.0f;                 // position in the PINK table
    int i = (int)p;
    if (i > 9) i = 9;
    float t = p - i;
    ringColor[k] = tft.color565(PINK[i][0] + (int)((PINK[i + 1][0] - PINK[i][0]) * t),
                                PINK[i][1] + (int)((PINK[i + 1][1] - PINK[i][1]) * t),
                                PINK[i][2] + (int)((PINK[i + 1][2] - PINK[i][2]) * t));
  }

  for (int y = LINE_THICKNESS; y <= SCREEN_H - 1 - LINE_THICKNESS; y++) {
    rowExtent(y, clipL, clipR);

#if RADIAL_BG
    int dy = y - GY;
    int dx[RINGS + 1];
    for (int k = 0; k < RINGS; k++) {
      float v = 1.0f - (float)(dy * dy) / (ryk[k] * ryk[k]);
      dx[k] = (v < 0) ? -1 : (int)(rxk[k] * sqrtf(v));
    }
    dx[RINGS] = -1;

    if (dx[0] < 0) {                      // above/below the glow: plain base color
      hl(clipL, clipR, y, baseColor);
      continue;
    }
    hl(clipL, GX - dx[0] - 1, y, baseColor);      // outer left
    hl(GX + dx[0] + 1, clipR, y, baseColor);      // outer right
    for (int k = 0; k < RINGS; k++) {
      if (dx[k] < 0) break;
      if (dx[k + 1] < 0) {                // innermost ring on this row: draw whole span
        hl(GX - dx[k], GX + dx[k], y, ringColor[k]);
      } else {
        hl(GX - dx[k], GX - dx[k + 1] - 1, y, ringColor[k]);
        hl(GX + dx[k + 1] + 1, GX + dx[k], y, ringColor[k]);
      }
    }
#else
    hl(clipL, clipR, y, baseColor);
#endif
  }
}

void drawBorder(uint16_t color) {
  for (int t = 0; t < LINE_THICKNESS; t++) {
    tft.drawRoundRect(t, t, SCREEN_W - 2 * t, SCREEN_H - 2 * t, CORNER_RADIUS - t, color);
  }
}

// ---- Apps ------------------------------------------------------------------

void drawApp(const App& a, int cx, int cy) {
  // 1 px black outline = slightly larger black circle under the icon
  tft.fillCircle(cx, cy, ICON_R + 1, ILI9341_BLACK);
  a.drawIcon(cx, cy, ICON_R);

  // Label: size-1 text (6 px per character), centered, with a 1 px black shadow
  tft.setTextSize(1);
  int w = strlen(a.name) * 6 - 1;
  int tx = cx - w / 2;
  int ty = cy + ICON_R + LABEL_GAP;
  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(tx + 1, ty + 1);
  tft.print(a.name);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(tx, ty);
  tft.print(a.name);
}

const int NUM_APPS = sizeof(APPS) / sizeof(APPS[0]);

// Center of app number idx: rows are centered, and icons rise toward the edges
void appCenter(int idx, int& cx, int& cy) {
  int row = 0;
  while (idx >= ROW_COUNT[row]) idx -= ROW_COUNT[row++];
  int n = ROW_COUNT[row];
  float startX = SCREEN_CX - (n - 1) * PITCH_X / 2.0f;   // centers each row
  cx = (int)lroundf(startX + idx * PITCH_X);
  float t = (cx - SCREEN_CX) / ARC_HALF_WIDTH;           // distance from center line
  cy = ROW_Y[row] - (int)lroundf(ARC_DEPTH * t * t);     // curve: edges rise
}

void drawHomeScreen() {
  tft.fillScreen(ILI9341_BLACK);          // everything outside the border stays black
  drawBackground();
  drawBorder(tft.color565(255, 92, 92));  // coral border, same as before

  for (int idx = 0; idx < NUM_APPS; idx++) {
    int cx, cy;
    appCenter(idx, cx, cy);
    drawApp(APPS[idx], cx, cy);
  }
}

// Which app is at screen point (x, y)? Returns its index in APPS, or -1.
// The tap area is a little bigger than the icon so it's easy to hit.
int appAt(int x, int y) {
  const int TAP_R = ICON_R + 8;
  for (int idx = 0; idx < NUM_APPS; idx++) {
    int cx, cy;
    appCenter(idx, cx, cy);
    int dx = x - cx, dy = y - cy;
    if (dx * dx + dy * dy <= TAP_R * TAP_R) return idx;
  }
  return -1;
}

} // namespace home
