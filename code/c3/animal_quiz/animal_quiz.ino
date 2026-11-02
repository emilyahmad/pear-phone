/*******************************************************************************
 *  Animal Quiz - "What animal are you?"
 *  5 multiple choice questions, tap an answer, get your animal.
 *  Uses the same LovyanGFX config as press_me.ino (landscape 320x240).
 *******************************************************************************/

#include <LovyanGFX.h>
#include "../lgfx_config.h" // Configuration for our screen

LGFX tft;

enum Animal { CAT, DOG, OWL, FOX, ANIMAL_COUNT };

struct Option {
  const char* label;
  Animal animal;
};

struct Question {
  const char* line1;
  const char* line2;
  Option options[3];
};

static const Question questions[] = {
  { "Perfect Saturday", "night?",
    { {"Cozy nap at home", CAT}, {"Party w/ everyone", DOG}, {"Stargazing alone", OWL} } },
  { "Your friends say", "you are...",
    { {"Loyal", DOG}, {"Clever", FOX}, {"Wise", OWL} } },
  { "Pick a snack", "",
    { {"Fish", CAT}, {"Anything!", DOG}, {"Sneaky berries", FOX} } },
  { "How do you solve", "problems?",
    { {"Think it through", OWL}, {"Outsmart them", FOX}, {"Ignore, nap", CAT} } },
  { "Ideal weather", "",
    { {"Sunny, sunbeam", CAT}, {"Park day!", DOG}, {"Foggy night", OWL} } },
};
static const int NUM_QUESTIONS = sizeof(questions) / sizeof(questions[0]);

struct Result {
  const char* name;
  const char* line1;
  const char* line2;
  uint16_t color;
};

static const Result results[ANIMAL_COUNT] = {
  { "CAT", "Independent and chill.", "You nap on your terms.", TFT_ORANGE },
  { "DOG", "Loyal and social.",      "Everyone is your friend.", TFT_YELLOW },
  { "OWL", "Wise and nocturnal.",    "You think before you act.", TFT_CYAN },
  { "FOX", "Clever and sly.",        "Always a step ahead.", 0xFC00 },
};

// Layout
static const int BTN_X = 20;
static const int BTN_W = 280;
static const int BTN_H = 44;
static const int BTN_Y0 = 80;
static const int BTN_GAP = 52;

// State
static int currentQ = 0;
static int scores[ANIMAL_COUNT];
static bool showingResult = false;
static bool wasTouching = false;

static bool pointInRect(int x, int y, int rx, int ry, int rw, int rh) {
  return (x >= rx) && (x < rx + rw) && (y >= ry) && (y < ry + rh);
}

static void drawButton(int x, int y, int w, int h, const char* label, uint16_t fill) {
  tft.fillRoundRect(x, y, w, h, 10, fill);
  tft.drawRoundRect(x, y, w, h, 10, TFT_WHITE);
  tft.setTextSize(2);
  tft.setTextColor(TFT_BLACK, fill);
  tft.setCursor(x + (w - tft.textWidth(label)) / 2, y + (h - tft.fontHeight()) / 2);
  tft.print(label);
}

static void drawQuestion() {
  const Question& q = questions[currentQ];
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(1);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(20, 8);
  tft.printf("Question %d of %d", currentQ + 1, NUM_QUESTIONS);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(20, 24);
  tft.print(q.line1);
  tft.setCursor(20, 48);
  tft.print(q.line2);

  for (int i = 0; i < 3; i++) {
    drawButton(BTN_X, BTN_Y0 + i * BTN_GAP, BTN_W, BTN_H, q.options[i].label, TFT_LIGHTGREY);
  }
}

static void drawResult() {
  // Highest score wins; ties go to the earlier animal in the enum
  int best = 0;
  for (int i = 1; i < ANIMAL_COUNT; i++) {
    if (scores[i] > scores[best]) best = i;
  }
  const Result& r = results[best];

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(20, 20);
  tft.print("You are a...");

  tft.setTextSize(6);
  tft.setTextColor(r.color, TFT_BLACK);
  tft.setCursor((tft.width() - tft.textWidth(r.name)) / 2, 60);
  tft.print(r.name);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(20, 130);
  tft.print(r.line1);
  tft.setCursor(20, 154);
  tft.print(r.line2);

  drawButton(BTN_X, 186, BTN_W, BTN_H, "TRY AGAIN", r.color);
}

static void startQuiz() {
  for (int i = 0; i < ANIMAL_COUNT; i++) scores[i] = 0;
  currentQ = 0;
  showingResult = false;
  drawQuestion();
}

void setup() {
  Serial.begin(115200);
  delay(50);

  tft.init();
  tft.setBrightness(255);
  startQuiz();
}

void loop() {
  uint16_t tx, ty;
  bool touching = tft.getTouch(&tx, &ty);

  // Act only on the moment a new touch begins
  if (touching && !wasTouching) {
    if (showingResult) {
      if (pointInRect(tx, ty, BTN_X, 186, BTN_W, BTN_H)) startQuiz();
    } else {
      for (int i = 0; i < 3; i++) {
        if (pointInRect(tx, ty, BTN_X, BTN_Y0 + i * BTN_GAP, BTN_W, BTN_H)) {
          scores[questions[currentQ].options[i].animal]++;
          currentQ++;
          if (currentQ >= NUM_QUESTIONS) {
            showingResult = true;
            drawResult();
          } else {
            drawQuestion();
          }
          break;
        }
      }
    }
  }

  wasTouching = touching;
}
