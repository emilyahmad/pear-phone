// Weather screen (portrait 240x320, rotation 2, same as home).
// Live data: the location comes from ip-api.com (from the phone's internet
// connection), the weather from Open-Meteo. Both are free and need no API key.
// Needs WiFi (secrets.h) and the ArduinoJson library (Library Manager).
// Uses home::'s drawing helpers, so it must be included after home_screen.h.
#pragma once

namespace weather {

constexpr int ROTATION = 2;                  // portrait, same as home
constexpr int SCREEN_W = 240;
constexpr int SCREEN_H = 320;

constexpr uint32_t REFRESH_MS = 10UL * 60 * 1000;   // fetch new data every 10 minutes
constexpr uint32_t RETRY_MS   = 30UL * 1000;        // after a failed fetch, wait this long
constexpr uint32_t WIFI_WAIT_MS = 15UL * 1000;      // "Connecting..." turns into "No WiFi" after this
constexpr int HTTP_TIMEOUT_MS = 5000;

// ---- Data -----------------------------------------------------------------------

struct Weather {
  bool  valid = false;
  float temp, feels, high, low, wind;
  int   humidity, code;
  bool  isDay;
  char  time[6];               // local "HH:MM" of the reading
};
Weather now;

char  city[32] = "";
float lat = 0, lon = 0;
bool  haveLocation = false;

uint32_t lastFetchMs = 0;      // last successful fetch
uint32_t lastTryMs = 0;        // last attempt (successful or not)
bool     triedOnce = false;
uint32_t openedMs = 0;         // when the screen was opened (for the WiFi wait)
const char* error = nullptr;   // last fetch error, shown under the data
bool     showing = false;      // the screen is on, so update() may draw

// GET a URL and parse the JSON reply into doc. Logs to Serial.
bool getJson(const char* url, JsonDocument& doc) {
  Serial.printf("GET %s\n", url);
  HTTPClient http;
  WiFiClient plain;
  WiFiClientSecure secure;
  bool https = strncmp(url, "https", 5) == 0;
  if (https) secure.setInsecure();   // skip certificate checks (fine for a hobby project)
  if (!http.begin(https ? (WiFiClient&)secure : plain, url)) return false;
  http.setTimeout(HTTP_TIMEOUT_MS);
  int status = http.GET();
  Serial.printf("  HTTP %d\n", status);
  if (status != 200) { http.end(); return false; }
  String body = http.getString();
  http.end();
  DeserializationError err = deserializeJson(doc, body);
  if (err) { Serial.printf("  JSON error: %s\n", err.c_str()); return false; }
  return true;
}

// Approximate location from the phone's public IP address
bool fetchLocation() {
  JsonDocument doc;
  if (!getJson("http://ip-api.com/json/?fields=status,city,lat,lon", doc)) return false;
  if (strcmp(doc["status"] | "", "success") != 0) return false;
  lat = doc["lat"] | 0.0f;
  lon = doc["lon"] | 0.0f;
  strlcpy(city, doc["city"] | "", sizeof(city));
  haveLocation = true;
  Serial.printf("  location: %s (%.4f, %.4f)\n", city, lat, lon);
  return true;
}

bool fetchWeather() {
  char url[400];
  snprintf(url, sizeof(url),
    "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"
    "&current=temperature_2m,apparent_temperature,relative_humidity_2m,weather_code,wind_speed_10m,is_day"
    "&daily=temperature_2m_max,temperature_2m_min&forecast_days=1"
    "&temperature_unit=fahrenheit&wind_speed_unit=mph&timezone=auto", lat, lon);
  JsonDocument doc;
  if (!getJson(url, doc)) return false;
  JsonObject cur = doc["current"];
  if (cur.isNull()) return false;
  Weather w;
  w.temp     = cur["temperature_2m"] | 0.0f;
  w.feels    = cur["apparent_temperature"] | 0.0f;
  w.humidity = cur["relative_humidity_2m"] | 0;
  w.code     = cur["weather_code"] | 0;
  w.wind     = cur["wind_speed_10m"] | 0.0f;
  w.isDay    = (cur["is_day"] | 1) != 0;
  w.high     = doc["daily"]["temperature_2m_max"][0] | w.temp;
  w.low      = doc["daily"]["temperature_2m_min"][0] | w.temp;
  const char* t = cur["time"] | "";                 // "2026-11-02T19:30"
  strlcpy(w.time, strlen(t) >= 16 ? t + 11 : "", sizeof(w.time));
  w.valid = true;
  now = w;
  Serial.printf("  %.1fF (feels %.1f), H %.1f L %.1f, code %d, wind %.1f mph, hum %d%%, %s, %s\n",
                w.temp, w.feels, w.high, w.low, w.code, w.wind, w.humidity,
                w.isDay ? "day" : "night", w.time);
  return true;
}

// ---- Conditions (WMO weather codes) -------------------------------------------------

enum Kind { CLEAR, PARTLY, CLOUDY, FOG, DRIZZLE, RAIN, SNOW, STORM };

Kind kindOf(int code) {
  if (code == 0) return CLEAR;
  if (code <= 2) return PARTLY;
  if (code == 3) return CLOUDY;
  if (code == 45 || code == 48) return FOG;
  if (code >= 51 && code <= 57) return DRIZZLE;
  if ((code >= 61 && code <= 67) || (code >= 80 && code <= 82)) return RAIN;
  if ((code >= 71 && code <= 77) || code == 85 || code == 86) return SNOW;
  if (code >= 95) return STORM;
  return CLOUDY;
}

const char* labelOf(int code) {
  switch (kindOf(code)) {
    case CLEAR:   return "Clear";
    case PARTLY:  return code == 1 ? "Mostly clear" : "Partly cloudy";
    case CLOUDY:  return "Cloudy";
    case FOG:     return "Fog";
    case DRIZZLE: return "Drizzle";
    case RAIN:    return code >= 80 ? "Showers" : "Rain";
    case SNOW:    return "Snow";
    default:      return "Thunderstorm";
  }
}

// ---- Drawing ----------------------------------------------------------------------

const int DATA_Y0 = 8, DATA_Y1 = 262;          // everything above the Home pill
const int PILL_X = 85, PILL_Y = 272, PILL_W = 70, PILL_H = 22;
const int STATUS_Y = 248;                      // "Updated 7:30 PM" / errors

bool night() { return now.valid && !now.isDay; }

// Sky color on row y: the Weather icon's blues by day, deep navy at night
uint16_t sky(int y) {
  float t = (float)y / (SCREEN_H - 1);
  return night() ? home::mix(0x0B1440, 0x2A3A7A, t) : home::mix(0x2A80EE, 0x7CE2FC, t);
}

// Repaints the sky on rows y0..y1, inside the pear border
void drawSky(int y0, int y1) {
  for (int y = y0; y <= y1; y++) {
    int xl, xr;
    home::rowExtent(y, xl, xr);
    tft.drawFastHLine(xl, y, xr - xl + 1, sky(y));
  }
}

uint16_t ink() { return ILI9341_WHITE; }

void centered(const char* s, int y, uint8_t size, uint16_t color) {
  int w = strlen(s) * 6 * size - size;
  tft.setTextSize(size);
  tft.setTextColor(color);
  tft.setCursor((SCREEN_W - w) / 2, y);
  tft.print(s);
}

// ---- Weather icon, drawn at any size (s = half its width, ~30 for the big one) ----

int S(int v, int s) { return v * s / 30; }

void sun(int cx, int cy, int r) {
  uint16_t c = home::rgb(0xF7D10F);
  for (int i = 0; i < 8; i++) {                         // rays
    float a = i * PI / 4;
    for (int w = -1; w <= 1; w++)
      tft.drawLine(cx + (int)((r + 3) * cosf(a)) + w, cy + (int)((r + 3) * sinf(a)),
                   cx + (int)((r + 8) * cosf(a)) + w, cy + (int)((r + 8) * sinf(a)), c);
  }
  tft.fillCircle(cx, cy, r, c);
}

// Crescent moon: a pale disc with a sky-colored bite, row by row
void moon(int cx, int cy, int r) {
  uint16_t c = home::rgb(0xF4F1C9);
  int bx = cx + r / 2, by = cy - r / 3, br = r * 85 / 100;
  for (int dy = -r; dy <= r; dy++) {
    int y = cy + dy, w = home::halfWidth(r, dy);
    tft.drawFastHLine(cx - w, y, 2 * w + 1, c);
    int bdy = y - by;
    if (bdy >= -br && bdy <= br) {
      int bw = home::halfWidth(br, bdy);
      int x0 = max(bx - bw, cx - w), x1 = min(bx + bw, cx + w);
      if (x1 >= x0) tft.drawFastHLine(x0, y, x1 - x0 + 1, sky(y));
    }
  }
}

void sunOrMoon(int cx, int cy, int r) {
  if (night()) moon(cx, cy, r); else sun(cx, cy, r);
}

void cloud(int cx, int cy, int s, uint16_t c) {
  tft.fillCircle(cx - S(12, s), cy + S(6, s), S(10, s), c);
  tft.fillCircle(cx + S(2, s),  cy - S(2, s), S(14, s), c);
  tft.fillCircle(cx + S(16, s), cy + S(6, s), S(10, s), c);
  tft.fillRoundRect(cx - S(22, s), cy + S(4, s), S(48, s), S(12, s), S(6, s), c);
}

void drops(int cx, int cy, int s, int n, uint16_t c) {   // slanted rain streaks
  for (int i = 0; i < n; i++) {
    int x = cx - S(16, s) + i * S(32, s) / (n - 1);
    for (int w = 0; w < 2; w++) tft.drawLine(x + w, cy, x - S(4, s) + w, cy + S(10, s), c);
  }
}

void flakes(int cx, int cy, int s, uint16_t c) {
  for (int i = 0; i < 3; i++) {
    int x = cx - S(14, s) + i * S(14, s), y = cy + (i % 2) * S(6, s), a = S(4, s);
    tft.drawFastHLine(x - a, y, 2 * a + 1, c);
    tft.drawLine(x - a + 1, y - a + 1, x + a - 1, y + a - 1, c);
    tft.drawLine(x - a + 1, y + a - 1, x + a - 1, y - a + 1, c);
    tft.drawFastVLine(x, y - a, 2 * a + 1, c);
  }
}

void drawIcon(int code, int cx, int cy, int s) {
  uint16_t white = ILI9341_WHITE, grey = home::rgb(0xC9D2DC), dark = home::rgb(0x6B7584);
  switch (kindOf(code)) {
    case CLEAR:
      sunOrMoon(cx, cy, S(16, s));
      break;
    case PARTLY:
      sunOrMoon(cx - S(8, s), cy - S(8, s), S(13, s));
      cloud(cx + S(4, s), cy + S(6, s), s, white);
      break;
    case CLOUDY:
      cloud(cx + S(8, s), cy - S(6, s), s * 3 / 4, grey);
      cloud(cx - S(2, s), cy + S(4, s), s, white);
      break;
    case FOG:
      cloud(cx, cy - S(8, s), s, grey);
      for (int i = 0; i < 3; i++)
        tft.fillRoundRect(cx - S(22, s) + (i % 2) * S(6, s), cy + S(14, s) + i * S(7, s),
                          S(38, s), max(2, S(3, s)), 1, white);
      break;
    case DRIZZLE:
    case RAIN:
      cloud(cx, cy - S(8, s), s, kindOf(code) == RAIN ? grey : white);
      drops(cx, cy + S(16, s), s, kindOf(code) == RAIN ? 5 : 3, home::rgb(0x9FD8FF));
      break;
    case SNOW:
      cloud(cx, cy - S(8, s), s, white);
      flakes(cx, cy + S(18, s), s, white);
      break;
    case STORM: {
      cloud(cx, cy - S(8, s), s, dark);
      uint16_t bolt = home::rgb(0xF7D10F);
      tft.fillTriangle(cx + S(2, s), cy + S(4, s), cx - S(8, s), cy + S(20, s), cx + S(1, s), cy + S(18, s), bolt);
      tft.fillTriangle(cx - S(2, s), cy + S(16, s), cx + S(8, s), cy + S(16, s), cx - S(4, s), cy + S(32, s), bolt);
      break;
    }
  }
}

// ---- Screen parts -------------------------------------------------------------------

// "7:30 PM" from "19:30"
void prettyTime(const char* hhmm, char* out, size_t n) {
  int h = atoi(hhmm), m = atoi(hhmm + 3);
  snprintf(out, n, "%d:%02d %s", (h % 12) ? h % 12 : 12, m, h < 12 ? "AM" : "PM");
}

bool wifiUp() { return WiFi.status() == WL_CONNECTED; }

// The bottom status line: an error, or when the data is from
void drawStatus(const char* msg = nullptr) {
  drawSky(STATUS_Y - 2, STATUS_Y + 9);
  char buf[40];
  if (!msg) {
    if (error) msg = error;
    else if (now.valid) {
      char t[12];
      prettyTime(now.time, t, sizeof(t));
      snprintf(buf, sizeof(buf), "Updated %s", t);
      msg = buf;
    } else return;
  }
  centered(msg, STATUS_Y, 1, ink());
}

// Message in the middle of the screen when there's no data yet
const char* waitingMessage() {
#ifndef WIFI_SSID
  return "WiFi not set up (add secrets.h)";
#endif
  if (!wifiUp()) return millis() - openedMs < WIFI_WAIT_MS ? "Connecting to WiFi..." : "No WiFi. Tap to retry";
  if (error) return "Couldn't load. Tap to retry";
  return "Loading weather...";
}

void drawData() {
  drawSky(DATA_Y0, DATA_Y1);
  tft.cp437(true);                                     // correct glyph numbers, \xF8 = degree sign

  if (!now.valid) {
    centered(waitingMessage(), 150, 1, ink());
    return;
  }

  // City (shrinks if it's long, cut off if it's still too long)
  char name[20];
  strlcpy(name, city[0] ? city : "Here", sizeof(name));
  centered(name, 22, strlen(name) <= 13 ? 2 : 1, ink());

  drawIcon(now.code, 120, 78, 30);

  // Big temperature with a drawn degree circle
  char buf[48];
  snprintf(buf, sizeof(buf), "%d", (int)lroundf(now.temp));
  int w = strlen(buf) * 36 - 6;
  int x = (SCREEN_W - w - 12) / 2;
  tft.setTextSize(6);
  tft.setTextColor(ink());
  tft.setCursor(x, 124);
  tft.print(buf);
  tft.drawCircle(x + w + 8, 130, 5, ink());
  tft.drawCircle(x + w + 8, 130, 4, ink());

  centered(labelOf(now.code), 182, 2, ink());
  snprintf(buf, sizeof(buf), "H:%d\xF8  L:%d\xF8", (int)lroundf(now.high), (int)lroundf(now.low));
  centered(buf, 204, 2, ink());
  snprintf(buf, sizeof(buf), "Feels %d\xF8  Wind %d mph  Hum %d%%",
           (int)lroundf(now.feels), (int)lroundf(now.wind), now.humidity);
  centered(buf, 230, 1, ink());

  drawStatus();
}

void drawHomePill(bool pressed = false) {
  uint16_t fill = pressed ? home::rgb(0xC9D2DC) : ILI9341_WHITE;
  tft.fillRoundRect(PILL_X, PILL_Y, PILL_W, PILL_H, PILL_H / 2, fill);
  uint16_t text = night() ? home::rgb(0x0B1440) : home::rgb(0x2A80EE);
  tft.setTextSize(1);
  tft.setTextColor(text);
  tft.setCursor(PILL_X + (PILL_W - 23) / 2, PILL_Y + 7);
  tft.print("Home");
}

// Fetches the location (once) and the weather, then redraws. Blocks ~1-2 s.
void refresh() {
  if (!wifiUp()) return;
  lastTryMs = millis();
  triedOnce = true;
  if (now.valid) drawStatus("Updating...");
  bool wasNight = night();
  error = nullptr;
  if (!haveLocation && !fetchLocation()) error = "Couldn't find location";
  else if (!fetchWeather()) error = "Couldn't update";
  else lastFetchMs = millis();
  if (!showing) return;
  if (night() != wasNight) {                           // day <-> night: the whole sky changes
    drawSky(0, SCREEN_H - 1);
    home::drawBorder(home::rgb(0xFF5C5C));
    drawHomePill();
  }
  if (error && now.valid) drawStatus();                // keep the old data, show the error
  else drawData();
}

// ---- Entry points for phone.ino ------------------------------------------------------

void show() {
  showing = true;
  openedMs = millis();
  tft.fillScreen(ILI9341_BLACK);                       // outside the border stays black
  drawSky(0, SCREEN_H - 1);
  home::drawBorder(home::rgb(0xFF5C5C));               // same coral border as home
  drawData();
  drawHomePill();
}

void hide() { showing = false; }

// Call every loop() while the weather screen is showing
void update() {
  static const char* shownWait = nullptr;
  bool stale = !now.valid || millis() - lastFetchMs >= REFRESH_MS;
  bool mayTry = !triedOnce || millis() - lastTryMs >= (error ? RETRY_MS : 0);
  if (wifiUp() && stale && mayTry) {
    refresh();
    shownWait = nullptr;
    return;
  }
  if (!now.valid) {                                    // keep the waiting message current
    const char* msg = waitingMessage();
    if (msg != shownWait) { shownWait = msg; drawData(); }
  }
}

bool homeAt(int x, int y) {
  return x >= PILL_X - 10 && x <= PILL_X + PILL_W + 10 && y >= PILL_Y - 8 && y <= PILL_Y + PILL_H + 10;
}

// Anywhere above the Home pill: refresh now (and retry WiFi if it's down)
bool retryAt(int x, int y) {
  return y >= DATA_Y0 && y < PILL_Y - 8;
}

void retry() {
  if (!wifiUp()) {
    WiFi.reconnect();
    openedMs = millis();                               // back to "Connecting..."
    drawData();
    return;
  }
  triedOnce = false;
  lastFetchMs = 0;
  if (!now.valid) drawData();                          // "Loading weather..."
  refresh();
}

} // namespace weather
