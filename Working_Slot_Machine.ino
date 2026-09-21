#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

#include "SLOT_MACHINE_BLANK.h"

#include "CHERRY_SPRITE.h"
#include "LEMON_SPRITE.h"
#include "ORANGE_SPRITE.h"
#include "GRAPES_SPRITE.h"
#include "BELL_SPRITE.h"
#include "BAR_SPRITE.h"
#include "SEVEN_SPRITE.h"
#include "Jackpot_Win.h"


// =====================================================
// HARDWARE
// =====================================================

#define BUTTON_PIN 4

#define DEBOUNCE_MS 35
#define CLICK_MAX_MS 350
#define DOUBLE_CLICK_GAP_MS 300
#define LONG_PRESS_MS 500

#define OLED_SDA 8
#define OLED_SCL 9

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(
  U8G2_R0,
  U8X8_PIN_NONE
);

// =====================================================
// STATE
// =====================================================

enum AppState {
  STATE_SLOT
};

AppState currentState = STATE_SLOT;

// =====================================================
// SLOT SYMBOLS
// =====================================================

const int SYMBOL_COUNT = 7;

const int REEL1_X = 30;
const int REEL2_X = 55;
const int REEL3_X = 80;

const int REEL_Y = 25;

int reel1 = 0;
int reel2 = 2;
int reel3 = 4;

int result1 = 0;
int result2 = 0;
int result3 = 0;

bool spinning = false;

bool reel1Stopped = false;
bool reel2Stopped = false;
bool reel3Stopped = false;

bool forced777Spin = false;

// =====================================================
// SPIN TIMING
// =====================================================

const unsigned long SPIN_INTERVAL = 70;

const unsigned long REEL1_STOP_TIME = 1050;
const unsigned long REEL2_STOP_TIME = 1750;
const unsigned long REEL3_STOP_TIME = 2450;

unsigned long spinStartTime = 0;
unsigned long lastSpinUpdate = 0;

// =====================================================
// PROBABILITY / PITY SYSTEM
// =====================================================

int unsuccessfulSpins = 0;

const int BASE_THREE_KIND_CHANCE = 15;
const int PITY_BOOST_PER_7 = 3;
const int MAX_THREE_KIND_CHANCE = 30;

// Natural 777 = 0.5%
const int JACKPOT_CHANCE_PER_1000 = 5;

// =====================================================
// CELEBRATION SYSTEM
// =====================================================

enum CelebrationType {
  CELEBRATION_NONE,
  CELEBRATION_WIN,
  CELEBRATION_JACKPOT
};

CelebrationType celebrationType = CELEBRATION_NONE;

bool celebrating = false;
bool jackpotBitmapShown = false;
unsigned long celebrationStartTime = 0;
unsigned long lastCelebrationUpdate = 0;

const unsigned long CELEBRATION_FRAME_TIME = 180;

// Small win stays short and simple
const unsigned long WIN_CELEBRATION_TIME = 1500;

// Jackpot gets a little longer
const unsigned long JACKPOT_PHASE1_TIME = 3000;
const unsigned long JACKPOT_PHASE2_TIME = 4000;
const unsigned long JACKPOT_CELEBRATION_TIME =
  JACKPOT_PHASE1_TIME + JACKPOT_PHASE2_TIME;

int celebrationFrame = 0;

// =====================================================
// OLED BITMAP
// =====================================================

uint8_t invertedBitmap[1024];

void startCelebration();
void updateCelebration();

// =====================================================
// DRAW SYMBOL
// =====================================================

void drawSymbol(int symbol, int x, int y) {

  switch (symbol) {

    case 0:
      oled.drawXBMP(
        x, y,
        CHERRY_WIDTH,
        CHERRY_HEIGHT,
        cherry_sprite_bitmap
      );
      break;

    case 1:
      oled.drawXBMP(
        x, y,
        LEMON_WIDTH,
        LEMON_HEIGHT,
        lemon_sprite_bitmap
      );
      break;

    case 2:
      oled.drawXBMP(
        x, y,
        ORANGE_WIDTH,
        ORANGE_HEIGHT,
        orange_sprite_bitmap
      );
      break;

    case 3:
      oled.drawXBMP(
        x, y,
        GRAPES_WIDTH,
        GRAPES_HEIGHT,
        grapes_sprite_bitmap
      );
      break;

    case 4:
      oled.drawXBMP(
        x, y,
        BELL_WIDTH,
        BELL_HEIGHT,
        bell_sprite_bitmap
      );
      break;

    case 5:
      oled.drawXBMP(
        x, y,
        BAR_WIDTH,
        BAR_HEIGHT,
        bar_sprite_bitmap
      );
      break;

    case 6:
      oled.drawXBMP(
        x, y,
        SEVEN_WIDTH,
        SEVEN_HEIGHT,
        seven_sprite_bitmap
      );
      break;
  }
}

// =====================================================
// DRAW SLOT MACHINE
// =====================================================

void drawMachine() {

  oled.clearBuffer();

  oled.drawXBM(
    0,
    0,
    128,
    64,
    invertedBitmap
  );

  drawSymbol(
    reel1,
    REEL1_X,
    REEL_Y
  );

  drawSymbol(
    reel2,
    REEL2_X,
    REEL_Y
  );

  drawSymbol(
    reel3,
    REEL3_X,
    REEL_Y
  );

  oled.sendBuffer();
}

// =====================================================
// SMALL WIN - FRAME 1
// =====================================================

void drawWinMessage() {

  oled.clearBuffer();

  oled.setFont(
    u8g2_font_10x20_tf
  );

  oled.drawStr(
    34,
    25,
    "WIN!"
  );

  drawSymbol(
    result1,
    56,
    36
  );

  oled.sendBuffer();
}

// =====================================================
// SMALL WIN - FRAME 2
// =====================================================

void drawWinMachine() {

  oled.clearBuffer();

  oled.drawXBM(
    0,
    0,
    128,
    64,
    invertedBitmap
  );

  drawSymbol(
    result1,
    REEL1_X,
    REEL_Y
  );

  drawSymbol(
    result2,
    REEL2_X,
    REEL_Y
  );

  drawSymbol(
    result3,
    REEL3_X,
    REEL_Y
  );

  oled.sendBuffer();
}

// =====================================================
// JACKPOT - FRAME 1
// Actual 7 sprites
// =====================================================

void drawJackpotFrame1() {

  oled.clearBuffer();

  oled.setFont(
    u8g2_font_6x12_tf
  );

  oled.drawStr(
    32,
    14,
    "JACKPOT!"
  );

  drawSymbol(
    6,
    REEL1_X,
    REEL_Y
  );

  drawSymbol(
    6,
    REEL2_X,
    REEL_Y
  );

  drawSymbol(
    6,
    REEL3_X,
    REEL_Y
  );

  oled.sendBuffer();
}

// =====================================================
// JACKPOT - FRAME 2
// Actual 7 sprites + machine
// =====================================================

void drawJackpotFrame2() {

  oled.clearBuffer();

  oled.drawXBM(
    0,
    0,
    128,
    64,
    invertedBitmap
  );

  drawSymbol(
    6,
    REEL1_X,
    REEL_Y
  );

  drawSymbol(
    6,
    REEL2_X,
    REEL_Y
  );

  drawSymbol(
    6,
    REEL3_X,
    REEL_Y
  );

  oled.sendBuffer();
}

void drawJackpotBitmap() {
  oled.clearBuffer();

  oled.drawXBM(
    0,
    0,
    128,
    64,
    Jackpot_Win_bitmap
  );

  oled.sendBuffer();
}

// =====================================================
// START CELEBRATION
// =====================================================

void startCelebration() {
  bool is777 =
    result1 == 6 &&
    result2 == 6 &&
    result3 == 6;

  if (is777) {

    celebrationType = CELEBRATION_JACKPOT;
    jackpotBitmapShown = false;

    Serial.println("CELEBRATION: JACKPOT");

  } else if (
    result1 == result2 &&
    result2 == result3
  ) {

    celebrationType = CELEBRATION_WIN;

    Serial.println("CELEBRATION: 3 OF A KIND");

  } else {

    celebrationType = CELEBRATION_NONE;
    celebrating = false;
    return;
  }

  celebrating = true;
  jackpotBitmapShown=false;
  celebrationStartTime = millis();
  lastCelebrationUpdate = millis();
  celebrationFrame = 0;

  if (celebrationType == CELEBRATION_WIN) {
    drawWinMessage();
  }
  else if (celebrationType == CELEBRATION_JACKPOT) {
    drawJackpotFrame1();
  }
}

void updateCelebration() {
  if (!celebrating) return;

  unsigned long now = millis();
  unsigned long elapsed = now - celebrationStartTime;

  // =====================================================
  // NORMAL 3-OF-A-KIND CELEBRATION
  // =====================================================

  if (celebrationType == CELEBRATION_WIN) {

    if (now - lastCelebrationUpdate >= CELEBRATION_FRAME_TIME) {

      lastCelebrationUpdate = now;
      celebrationFrame++;

      if (celebrationFrame % 2 == 0) {
        drawWinMessage();
      } else {
        drawWinMachine();
      }
    }

    if (elapsed >= WIN_CELEBRATION_TIME) {

      celebrating = false;
      celebrationType = CELEBRATION_NONE;
      celebrationFrame = 0;

      drawMachine();

      Serial.println("CELEBRATION: WIN COMPLETE");
    }

    return;
  }

  // =====================================================
  // JACKPOT CELEBRATION
  // =====================================================

  if (celebrationType == CELEBRATION_JACKPOT) {

    // -----------------------------------------------------
    // PHASE 1
    // Original jackpot animation
    // 0 - 3 seconds
    // -----------------------------------------------------

    if (elapsed < JACKPOT_PHASE1_TIME) {

      if (now - lastCelebrationUpdate >= CELEBRATION_FRAME_TIME) {

        lastCelebrationUpdate = now;
        celebrationFrame++;

        if (celebrationFrame % 2 == 0) {
          drawJackpotFrame1();
        } else {
          drawJackpotFrame2();
        }
      }

      return;
    }

    // -----------------------------------------------------
    // PHASE 2
    // Jackpot_Win bitmap
    // 3 - 7 seconds
    // -----------------------------------------------------

    if (elapsed < JACKPOT_CELEBRATION_TIME) {

      if (!jackpotBitmapShown) {

        Serial.println("JACKPOT: SHOWING BITMAP");

        oled.clearBuffer();

        oled.drawXBM(
          0,
          0,
          128,
          64,
          Jackpot_Win_bitmap
        );

        oled.sendBuffer();

        jackpotBitmapShown = true;
      }

      return;
    }

    // -----------------------------------------------------
    // JACKPOT COMPLETE
    // -----------------------------------------------------

    celebrating = false;
    celebrationType = CELEBRATION_NONE;
    celebrationFrame = 0;
    jackpotBitmapShown = false;

    drawMachine();

    Serial.println("CELEBRATION: JACKPOT COMPLETE");

    return;
  }
}

// =====================================================
// CHOOSE LOSING RESULT
// =====================================================

// This remains the original behavior:
// three different non-7 symbols.
// No deliberate double weighting.

void chooseLosingResult() {

  int a =
    random(0, 6);

  int b;
  int c;

  do {

    b =
      random(0, 6);

  } while (
    b == a
  );

  do {

    c =
      random(0, 6);

  } while (
    c == a ||
    c == b
  );

  result1 = a;
  result2 = b;
  result3 = c;
}

// =====================================================
// CHOOSE SPIN RESULT
// =====================================================

void chooseSpinResult() {

  int pityLevels =
    unsuccessfulSpins / 7;

  int threeKindChance =
    BASE_THREE_KIND_CHANCE +
    (
      pityLevels *
      PITY_BOOST_PER_7
    );

  if (
    threeKindChance >
    MAX_THREE_KIND_CHANCE
  ) {

    threeKindChance =
      MAX_THREE_KIND_CHANCE;
  }

  // ---------------------------------------------------
  // NATURAL 777
  // ---------------------------------------------------

  int roll =
    random(0, 1000);

  if (
    roll <
    JACKPOT_CHANCE_PER_1000
  ) {

    result1 = 6;
    result2 = 6;
    result3 = 6;

    Serial.println(
      "RESULT: 777"
    );

    return;
  }

  // ---------------------------------------------------
  // THREE OF A KIND
  // ---------------------------------------------------

  roll =
    random(0, 100);

  if (
    roll <
    threeKindChance
  ) {

    int symbol =
      random(0, 6);

    result1 = symbol;
    result2 = symbol;
    result3 = symbol;

    Serial.print(
      "RESULT: 3 OF A KIND - "
    );

    Serial.println(
      symbol
    );

    return;
  }

  // ---------------------------------------------------
  // LOSING RESULT
  // ---------------------------------------------------

  chooseLosingResult();

  Serial.print(
    "RESULT: "
  );

  Serial.print(
    result1
  );

  Serial.print(
    " "
  );

  Serial.print(
    result2
  );

  Serial.print(
    " "
  );

  Serial.println(
    result3
  );
}

// =====================================================
// START NORMAL SPIN
// =====================================================

void startSpin() {

  if (
    spinning ||
    celebrating
  ) {
    return;
  }

  forced777Spin = false;

  reel1Stopped = false;
  reel2Stopped = false;
  reel3Stopped = false;

  chooseSpinResult();

  spinning = true;

  spinStartTime =
    millis();

  lastSpinUpdate =
    millis();

  Serial.println(
    "SLOT: NORMAL SPIN START"
  );
}

// =====================================================
// START FORCED 777
// =====================================================

void startForced777Spin() {

  if (
    spinning ||
    celebrating
  ) {
    return;
  }

  Serial.println(
    "SLOT: FORCED 777 START"
  );

  result1 = 6;
  result2 = 6;
  result3 = 6;

  reel1Stopped = false;
  reel2Stopped = false;
  reel3Stopped = false;

  forced777Spin = true;

  spinning = true;

  spinStartTime =
    millis();

  lastSpinUpdate =
    millis();
}

// =====================================================
// UPDATE SPIN
// =====================================================

void updateSpin() {

  if (!spinning) {
    return;
  }

  unsigned long now =
    millis();

  unsigned long elapsed =
    now -
    spinStartTime;

  // ---------------------------------------------------
  // REEL ANIMATION
  // ---------------------------------------------------

  if (
    now - lastSpinUpdate >=
    SPIN_INTERVAL
  ) {

    lastSpinUpdate =
      now;

    if (!reel1Stopped) {

      reel1++;

      if (
        reel1 >=
        SYMBOL_COUNT
      ) {

        reel1 = 0;
      }
    }

    if (!reel2Stopped) {

      reel2++;

      if (
        reel2 >=
        SYMBOL_COUNT
      ) {

        reel2 = 0;
      }
    }

    if (!reel3Stopped) {

      reel3++;

      if (
        reel3 >=
        SYMBOL_COUNT
      ) {

        reel3 = 0;
      }
    }

    drawMachine();
  }

  // ---------------------------------------------------
  // STOP REEL 1
  // ---------------------------------------------------

  if (
    !reel1Stopped &&
    elapsed >=
      REEL1_STOP_TIME
  ) {

    reel1 =
      result1;

    reel1Stopped =
      true;

    Serial.print(
      "REEL 1 STOP: "
    );

    Serial.println(
      reel1
    );

    drawMachine();
  }

  // ---------------------------------------------------
  // STOP REEL 2
  // ---------------------------------------------------

  if (
    !reel2Stopped &&
    elapsed >=
      REEL2_STOP_TIME
  ) {

    reel2 =
      result2;

    reel2Stopped =
      true;

    Serial.print(
      "REEL 2 STOP: "
    );

    Serial.println(
      reel2
    );

    drawMachine();
  }

  // ---------------------------------------------------
  // STOP REEL 3
  // ---------------------------------------------------

  if (
    !reel3Stopped &&
    elapsed >=
      REEL3_STOP_TIME
  ) {

    reel3 =
      result3;

    reel3Stopped =
      true;

    Serial.print(
      "REEL 3 STOP: "
    );

    Serial.println(
      reel3
    );

    drawMachine();
  }

  // ---------------------------------------------------
  // ALL REELS STOPPED
  // ---------------------------------------------------

  if (
    reel1Stopped &&
    reel2Stopped &&
    reel3Stopped
  ) {

    spinning = false;

    // -------------------------------------------------
    // FORCED 777
    // -------------------------------------------------

    if (forced777Spin) {

      Serial.println(
        "SLOT: FORCED 777 COMPLETE"
      );

      forced777Spin =
        false;
    }

    // -------------------------------------------------
    // NORMAL SPIN
    // -------------------------------------------------

    else {

      Serial.println(
        "SLOT: NORMAL SPIN COMPLETE"
      );

      updatePityAfterSpin();
    }

    startCelebration();
  }
}

// =====================================================
// UPDATE PITY
// =====================================================

void updatePityAfterSpin() {

  // 777 does not affect pity
  if (
    result1 == 6 &&
    result2 == 6 &&
    result3 == 6
  ) {

    Serial.println(
      "PITY: 777 - NO PITY BOOST"
    );

    return;
  }

  // 3-of-a-kind resets pity
  if (
    result1 == result2 &&
    result2 == result3
  ) {

    unsuccessfulSpins = 0;

    Serial.println(
      "PITY: 3 OF A KIND - RESET"
    );

    return;
  }

  // Losing spin
  unsuccessfulSpins++;

  Serial.print(
    "PITY: UNSUCCESSFUL SPINS = "
  );

  Serial.println(
    unsuccessfulSpins
  );

  // Every 7 unsuccessful spins
  if (
    (unsuccessfulSpins % 7) == 0
  ) {

    int pityLevel =
      unsuccessfulSpins / 7;

    int currentChance =
      BASE_THREE_KIND_CHANCE +
      (
        pityLevel *
        PITY_BOOST_PER_7
      );

    if (
      currentChance >
      MAX_THREE_KIND_CHANCE
    ) {

      currentChance =
        MAX_THREE_KIND_CHANCE;
    }

    Serial.print(
      "PITY BOOST ACTIVATED: "
    );

    Serial.print(
      currentChance
    );

    Serial.println(
      "% 3-OF-A-KIND CHANCE"
    );
  }
}

// =====================================================
// SINGLE CLICK
// =====================================================

void handleSlotSingleClick() {

  if (
    spinning ||
    celebrating
  ) {
    return;
  }

  Serial.println(
    "SLOT: SINGLE CLICK"
  );

  Serial.println(
    "-> Quotes / Tips"
  );
}

// =====================================================
// DOUBLE CLICK
// =====================================================

void handleSlotDoubleClick() {

  if (
    spinning ||
    celebrating
  ) {
    return;
  }

  Serial.println(
    "SLOT: DOUBLE CLICK"
  );

  Serial.println(
    "-> SPIN"
  );

  startSpin();
}

// =====================================================
// TRIPLE CLICK
// =====================================================

void handleSlotTripleClick() {

  if (
    spinning ||
    celebrating
  ) {
    return;
  }

  Serial.println(
    "SLOT: TRIPLE CLICK"
  );

  Serial.println(
    "-> HISTORY"
  );
}

// =====================================================
// CLICK + HOLD
// =====================================================

void handleSlotClickHold() {

  if (
    spinning ||
    celebrating
  ) {
    return;
  }

  Serial.println(
    "SLOT: CLICK + HOLD"
  );

  Serial.println(
    "-> FORCE 777"
  );

  startForced777Spin();
}

// =====================================================
// LONG PRESS
// =====================================================

void handleSlotLongPress() {

  if (
    spinning ||
    celebrating
  ) {
    return;
  }

  Serial.println(
    "SLOT: LONG PRESS"
  );

  Serial.println(
    "-> EXIT"
  );
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(
    115200
  );

  delay(500);

  // OLED
  Wire.begin(
    OLED_SDA,
    OLED_SCL
  );

  Wire.setClock(
    400000
  );

  oled.begin();

  // ---------------------------------------------------
  // LOAD MACHINE ARTWORK
  // ---------------------------------------------------

  for (
    int i = 0;
    i < 1024;
    i++
  ) {

    invertedBitmap[i] =
      ~pgm_read_byte(
        &Slot_machine_blank[i]
      );
  }

  // ---------------------------------------------------
  // CLEAN EDGES
  // ---------------------------------------------------

  for (
    int y = 0;
    y < 64;
    y++
  ) {

    int row =
      y * 16;

    invertedBitmap[
      row + 0
    ] = 0x00;

    invertedBitmap[
      row + 1
    ] &= 0xE0;

    invertedBitmap[
      row + 14
    ] &= 0x07;

    invertedBitmap[
      row + 15
    ] = 0x00;
  }

  randomSeed(
    micros()
  );

  setupButton();

  drawMachine();
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  updateButton();

  updateSpin();

  updateCelebration();
}