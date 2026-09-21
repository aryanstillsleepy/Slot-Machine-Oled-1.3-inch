// =====================================================
// BUTTON SYSTEM
// STANDALONE SLOT MACHINE
// =====================================================

#define BUTTON_PIN 4

#define DEBOUNCE_MS 35
#define CLICK_MAX_MS 350
#define DOUBLE_CLICK_GAP_MS 300
#define LONG_PRESS_MS 500

// =====================================================
// BUTTON STATE
// =====================================================

bool buttonLastState = HIGH;
bool buttonStableState = HIGH;

unsigned long buttonLastChangeTime = 0;
unsigned long buttonPressTime = 0;
unsigned long lastClickTime = 0;

bool buttonIsPressed = false;

int clickCount = 0;

bool waitingForClicks = false;

bool longPressHandled = false;

bool clickHoldActive = false;
bool clickHoldTriggered = false;

// =====================================================
// SETUP
// =====================================================

void setupButton() {

  pinMode(
    BUTTON_PIN,
    INPUT_PULLUP
  );

  buttonLastState = HIGH;
  buttonStableState = HIGH;

  buttonLastChangeTime = millis();

  buttonPressTime = 0;
  lastClickTime = 0;

  buttonIsPressed = false;

  clickCount = 0;
  waitingForClicks = false;

  longPressHandled = false;

  clickHoldActive = false;
  clickHoldTriggered = false;
}

// =====================================================
// UPDATE BUTTON
// =====================================================

void updateButton() {

  bool reading =
    digitalRead(BUTTON_PIN);

  // ---------------------------------------------------
  // DEBOUNCE
  // ---------------------------------------------------

  if (
    reading != buttonLastState
  ) {

    buttonLastChangeTime =
      millis();

    buttonLastState =
      reading;
  }

  if (
    millis() -
    buttonLastChangeTime <
    DEBOUNCE_MS
  ) {

    return;
  }

  // ---------------------------------------------------
  // STABLE STATE CHANGED
  // ---------------------------------------------------

  if (
    reading != buttonStableState
  ) {

    buttonStableState =
      reading;

    // =================================================
    // PRESSED
    // =================================================

    if (
      buttonStableState == LOW
    ) {

      buttonIsPressed = true;

      buttonPressTime =
        millis();

      longPressHandled = false;

      clickHoldActive = false;
      clickHoldTriggered = false;

      // ------------------------------------------------
      // SECOND PRESS AFTER FIRST CLICK
      // ------------------------------------------------

      if (
        clickCount == 1 &&
        waitingForClicks &&
        millis() - lastClickTime <=
          DOUBLE_CLICK_GAP_MS
      ) {

        clickHoldActive = true;
      }
    }

    // =================================================
    // RELEASED
    // =================================================

    else {

      if (!buttonIsPressed) {
        return;
      }

      buttonIsPressed = false;

      // ------------------------------------------------
      // LONG PRESS WAS ALREADY HANDLED
      // ------------------------------------------------

      if (longPressHandled) {

        longPressHandled = false;

        waitingForClicks = false;
        clickCount = 0;

        clickHoldActive = false;
        clickHoldTriggered = false;

        return;
      }

      // ------------------------------------------------
      // CLICK + HOLD WAS ALREADY HANDLED
      // ------------------------------------------------

      if (clickHoldTriggered) {

        clickHoldActive = false;
        clickHoldTriggered = false;

        waitingForClicks = false;
        clickCount = 0;

        // IMPORTANT:
        // Release does absolutely nothing.
        return;
      }

      // ------------------------------------------------
      // NORMAL CLICK
      // ------------------------------------------------

      unsigned long pressDuration =
        millis() - buttonPressTime;

      if (
        pressDuration < CLICK_MAX_MS
      ) {

        clickCount++;

        lastClickTime =
          millis();

        waitingForClicks = true;
      }
    }
  }

  // ===================================================
  // LONG PRESS / CLICK + HOLD
  // ===================================================

  if (
    buttonIsPressed &&
    !longPressHandled
  ) {

    unsigned long pressDuration =
      millis() - buttonPressTime;

    if (
      pressDuration >= LONG_PRESS_MS
    ) {

      // ------------------------------------------------
      // CLICK + HOLD
      // ------------------------------------------------

      if (clickHoldActive) {

        clickHoldTriggered = true;

        // Fire exactly once.

        longPressHandled = true;

        waitingForClicks = false;
        clickCount = 0;

        handleSlotClickHold();
      }

      // ------------------------------------------------
      // NORMAL LONG PRESS
      // ------------------------------------------------

      else {

        longPressHandled = true;

        waitingForClicks = false;
        clickCount = 0;

        handleSlotLongPress();
      }
    }
  }

  // ===================================================
  // PROCESS CLICKS
  // ===================================================

  processClicks();
}

// =====================================================
// PROCESS CLICKS
// =====================================================

void processClicks() {

  if (!waitingForClicks) {
    return;
  }

  if (
    millis() - lastClickTime <=
    DOUBLE_CLICK_GAP_MS
  ) {

    return;
  }

  waitingForClicks = false;

  // ---------------------------------------------------
  // SINGLE
  // ---------------------------------------------------

  if (clickCount == 1) {

    handleSlotSingleClick();
  }

  // ---------------------------------------------------
  // DOUBLE
  // ---------------------------------------------------

  else if (clickCount == 2) {

    handleSlotDoubleClick();
  }

  // ---------------------------------------------------
  // TRIPLE
  // ---------------------------------------------------

  else if (clickCount >= 3) {

    handleSlotTripleClick();
  }

  clickCount = 0;
}