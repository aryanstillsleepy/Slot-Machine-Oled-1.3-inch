# Slot-Machine-Oled-1.3-inch
Slot machine that works, and also gives some outputs on button clicks that can be ignored or edited

# 🎰 Standalone OLED Slot Machine

A standalone Arduino-based slot machine running on a **128×64 SH1106 OLED display**.

The project features animated spinning reels, custom 16×16 pixel slot symbols, multiple button interactions, a three-of-a-kind win system, a pity mechanic, natural `777` jackpots, forced `777` testing, and custom jackpot/win animations.

---

## ✨ Features

* 128×64 SH1106 OLED display
* Hardware I²C communication
* Custom slot-machine artwork
* Seven slot symbols:

  * 🍒 Cherry
  * 🍋 Lemon
  * 🍊 Orange
  * 🍇 Grapes
  * 🔔 Bell
  * `BAR`
  * `7`
* Three independently stopping reels
* Animated reel spinning
* Single, double, and triple click detection
* Long-press detection
* Click-and-hold detection
* Three-of-a-kind win detection
* Natural `777` jackpot
* Pity system that increases the three-of-a-kind chance after unsuccessful spins
* Separate normal-win and jackpot celebrations
* Custom full-screen jackpot bitmap
* Serial Monitor debug output

---

# 🧰 Hardware

## OLED

The project uses a:

**SH1106 128×64 OLED**

Configured with U8g2:

```cpp
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(
  U8G2_R0,
  U8X8_PIN_NONE
);
```

### OLED Pins

| Function |    Pin |
| -------- | -----: |
| SDA      | GPIO 8 |
| SCL      | GPIO 9 |

The I²C bus is configured to run at:

```cpp
Wire.setClock(400000);
```

The project therefore uses a **400 kHz I²C clock**.

---

## Button

The slot machine uses one button:

| Function |    Pin |
| -------- | -----: |
| Button   | GPIO 4 |

The button uses the internal pull-up resistor:

```cpp
pinMode(BUTTON_PIN, INPUT_PULLUP);
```

Therefore:

* `HIGH` = button released
* `LOW` = button pressed

---

# 🎮 Button Controls

The button supports five different interactions.

| Input        | Action        |
| ------------ | ------------- |
| Single click | Quotes / Tips |
| Double click | Spin          |
| Triple click | History       |
| Click + hold | Force 777     |
| Long press   | Exit          |

### Important

The **double-click action is the normal way to start a spin**.

The click-and-hold action is detected when the second press of a potential double-click is held long enough.

---

# ⏱️ Button Timing

The button system uses the following timing values:

```cpp
#define DEBOUNCE_MS 35
#define CLICK_MAX_MS 350
#define DOUBLE_CLICK_GAP_MS 300
#define LONG_PRESS_MS 500
```

### Debounce

A button state must remain stable for:

**35 ms**

before it is accepted.

### Click

A press lasting less than:

**350 ms**

is treated as a click.

### Double-click window

The second click must occur within:

**300 ms**

of the first click.

### Long press

A press lasting at least:

**500 ms**

is treated as a long press.

---

# 🎰 Slot Layout

The OLED is 128×64 pixels.

The three reels are positioned at:

```cpp
REEL1_X = 30;
REEL2_X = 55;
REEL3_X = 80;

REEL_Y = 25;
```

Each slot symbol is:

**16×16 pixels**

The project therefore displays three 16×16 symbols across the machine artwork.

---

# 🎲 Symbols

There are seven symbols.

They are indexed from `0` to `6`:

| Index | Symbol |
| ----: | ------ |
|   `0` | Cherry |
|   `1` | Lemon  |
|   `2` | Orange |
|   `3` | Grapes |
|   `4` | Bell   |
|   `5` | BAR    |
|   `6` | Seven  |

The symbol count is defined as:

```cpp
const int SYMBOL_COUNT = 7;
```

The symbols are drawn using U8g2's XBMP drawing functions.

---

# 🖼️ Bitmap Format

The project uses custom monochrome bitmap graphics.

The main slot-machine artwork is:

```cpp
Slot_machine_blank[1024]
```

The display resolution is:

```text
128 × 64 = 8192 pixels
```

At one bit per pixel:

```text
8192 / 8 = 1024 bytes
```

Therefore the full-screen bitmap occupies exactly:

**1024 bytes**

The individual slot symbols are 16×16 pixels:

```text
16 × 16 / 8 = 32 bytes
```

Each sprite therefore occupies:

**32 bytes**

---

# 📁 Project Files

The project uses the following bitmap/header files:

```text
SLOT_MACHINE_BLANK.h
CHERRY_SPRITE.h
LEMON_SPRITE.h
ORANGE_SPRITE.h
GRAPES_SPRITE.h
BELL_SPRITE.h
BAR_SPRITE.h
SEVEN_SPRITE.h
Jackpot_Win.h
```

The main program includes them with:

```cpp
#include "SLOT_MACHINE_BLANK.h"

#include "CHERRY_SPRITE.h"
#include "LEMON_SPRITE.h"
#include "ORANGE_SPRITE.h"
#include "GRAPES_SPRITE.h"
#include "BELL_SPRITE.h"
#include "BAR_SPRITE.h"
#include "SEVEN_SPRITE.h"
#include "Jackpot_Win.h"
```

---

# 🖌️ Sprite Rendering

Symbols are selected using the `drawSymbol()` function.

The function maps the symbol index to its corresponding bitmap:

```cpp
drawSymbol(symbol, x, y);
```

For example:

```cpp
case 6:
  oled.drawXBMP(
    x,
    y,
    SEVEN_WIDTH,
    SEVEN_HEIGHT,
    seven_sprite_bitmap
  );
  break;
```

All seven symbols use the same 16×16 sprite dimensions.

---

# 🎡 Spin System

A normal spin begins with:

```cpp
startSpin();
```

The program first determines the final result.

The reels then visually spin before stopping one at a time.

### Spin speed

The reels update every:

```cpp
const unsigned long SPIN_INTERVAL = 70;
```

So the displayed reel symbol advances approximately every **70 ms**.

---

# 🛑 Reel Stop Timing

The three reels stop at different times.

| Reel   | Stop time |
| ------ | --------: |
| Reel 1 |   1050 ms |
| Reel 2 |   1750 ms |
| Reel 3 |   2450 ms |

Defined as:

```cpp
const unsigned long REEL1_STOP_TIME = 1050;
const unsigned long REEL2_STOP_TIME = 1750;
const unsigned long REEL3_STOP_TIME = 2450;
```

This produces a staggered reel-stop effect.

The final symbol is assigned when each reel stops:

```cpp
reel1 = result1;
reel2 = result2;
reel3 = result3;
```

---

# 🎲 Result Generation

Results are generated by:

```cpp
chooseSpinResult();
```

There are three possible result categories:

1. Natural `777`
2. Three of a kind
3. Losing combination

---

# 💰 Natural 777

The code gives a natural `777` a:

**5 / 1000 chance**

which corresponds to:

**0.5%**

```cpp
const int JACKPOT_CHANCE_PER_1000 = 5;
```

When triggered:

```cpp
result1 = 6;
result2 = 6;
result3 = 6;
```

Since symbol `6` is the seven:

```text
7 7 7
```

is produced.

A natural `777` does not affect the pity counter.

---

# 🎯 Three-of-a-Kind

The base three-of-a-kind chance is:

```cpp
const int BASE_THREE_KIND_CHANCE = 15;
```

This corresponds to:

**15%**

A three-of-a-kind result selects one non-seven symbol and puts it on all three reels.

For example:

```text
🍒 🍒 🍒
```

or:

```text
🔔 🔔 🔔
```

The seven is excluded from normal three-of-a-kind generation.

---

# 📈 Pity System

The project contains a pity system based on unsuccessful spins.

```cpp
int unsuccessfulSpins = 0;
```

Every **7 unsuccessful spins**, the three-of-a-kind chance increases.

The boost is:

```cpp
const int PITY_BOOST_PER_7 = 3;
```

So the chance increases by:

**+3 percentage points every 7 unsuccessful spins**

The maximum three-of-a-kind chance is:

```cpp
const int MAX_THREE_KIND_CHANCE = 30;
```

Therefore the chance can rise from:

```text
15%
18%
21%
24%
27%
30%
```

and then remains capped at **30%**.

### Pity formula

The code calculates:

```cpp
pityLevels = unsuccessfulSpins / 7;
```

and:

```cpp
threeKindChance =
  BASE_THREE_KIND_CHANCE +
  (pityLevels * PITY_BOOST_PER_7);
```

The result is capped at 30%.

---

# 🔄 Pity Reset Rules

The pity counter behaves differently depending on the result.

### Losing spin

The counter increases:

```cpp
unsuccessfulSpins++;
```

### Three-of-a-kind

The counter resets:

```cpp
unsuccessfulSpins = 0;
```

### 777

The pity counter is unchanged.

The code explicitly treats `777` separately:

```cpp
// 777 does not affect pity
```

---

# ❌ Losing Results

Losing results are generated by:

```cpp
chooseLosingResult();
```

The three selected symbols are:

* non-seven symbols
* all different from one another

The code therefore deliberately avoids producing a pair or three-of-a-kind in a losing result.

For example:

```text
🍒 🍋 🔔
```

is possible.

But:

```text
🍒 🍒 🔔
```

is not generated by `chooseLosingResult()`.

---

# 🏆 Win Detection

After all three reels stop, the result is passed to:

```cpp
startCelebration();
```

The program first checks for `777`:

```cpp
result1 == 6 &&
result2 == 6 &&
result3 == 6
```

If that is false, it checks whether all three symbols are equal:

```cpp
result1 == result2 &&
result2 == result3
```

This produces two celebration types:

```cpp
CELEBRATION_WIN
CELEBRATION_JACKPOT
```

---

# 🎉 Normal Win Animation

A normal three-of-a-kind win lasts:

**1500 ms**

```cpp
const unsigned long WIN_CELEBRATION_TIME = 1500;
```

The animation alternates between two screens every:

**180 ms**

```cpp
const unsigned long CELEBRATION_FRAME_TIME = 180;
```

### Frame 1

Displays:

```text
WIN!

[winning symbol]
```

### Frame 2

Returns to the slot-machine artwork with the three winning symbols.

The two frames alternate until the celebration finishes.

---

# 💎 Jackpot Animation

A `777` jackpot uses a longer celebration.

Total duration:

**7000 ms**

It has two phases.

## Phase 1 — 0 to 3 seconds

Duration:

```cpp
JACKPOT_PHASE1_TIME = 3000;
```

The program alternates between:

### Frame 1

```text
JACKPOT!

7   7   7
```

and:

### Frame 2

The slot-machine artwork with:

```text
7   7   7
```

The frame changes every **180 ms**.

---

## Phase 2 — 3 to 7 seconds

The custom full-screen bitmap:

```cpp
Jackpot_Win_bitmap
```

is displayed.

The bitmap is:

```text
128 × 64
```

and occupies:

```text
1024 bytes
```

It remains on-screen for the remainder of the jackpot celebration.

After 7 seconds, the normal slot-machine screen is restored.

---

# 🧪 Forced 777

The project also contains a testing/debug feature for forcing a jackpot.

The button action is:

**Click + Hold**

This calls:

```cpp
startForced777Spin();
```

The result is immediately set to:

```cpp
result1 = 6;
result2 = 6;
result3 = 6;
```

The reels still perform their normal animated spin and stop sequentially.

This provides a convenient way to test the entire jackpot animation without waiting for a natural `0.5%` jackpot.

---

# 🖱️ Button State Machine

The button system tracks several internal states:

```cpp
buttonLastState
buttonStableState
buttonIsPressed

clickCount
waitingForClicks

longPressHandled

clickHoldActive
clickHoldTriggered
```

The system first debounces the physical button, then determines whether the interaction is:

* click
* double click
* triple click
* long press
* click + hold

The click processing is intentionally delayed until the double-click window expires, allowing the program to determine whether a first click becomes a single, double, or triple click.

---

# 🖥️ Serial Monitor

The program uses:

```cpp
Serial.begin(115200);
```

The Serial Monitor can therefore be opened at:

**115200 baud**

The program prints useful debugging information including:

```text
SLOT: NORMAL SPIN START
SLOT: NORMAL SPIN COMPLETE
SLOT: FORCED 777 START
SLOT: FORCED 777 COMPLETE
```

It also reports:

```text
REEL 1 STOP: ...
REEL 2 STOP: ...
REEL 3 STOP: ...
```

and result information such as:

```text
RESULT: 777
RESULT: 3 OF A KIND - ...
```

Pity-system events are also reported:

```text
PITY: UNSUCCESSFUL SPINS = ...
PITY BOOST ACTIVATED: ...% 3-OF-A-KIND CHANCE
```

Celebration events are logged as well.

---

# 🧹 OLED Bitmap Preparation

The slot-machine artwork is stored in:

```cpp
Slot_machine_blank
```

The program creates a second buffer:

```cpp
uint8_t invertedBitmap[1024];
```

During setup, each byte is inverted:

```cpp
invertedBitmap[i] =
  ~pgm_read_byte(
    &Slot_machine_blank[i]
  );
```

This prepares the artwork for the way it is rendered by the display.

---

# 🧽 Edge Cleaning

The program also modifies the first and last bytes of every 128-pixel row.

Each row occupies:

```text
16 bytes
```

because:

```text
128 pixels / 8 = 16 bytes
```

The code cleans the outer edges using:

```cpp
invertedBitmap[row + 0] = 0x00;
invertedBitmap[row + 1] &= 0xE0;
invertedBitmap[row + 14] &= 0x07;
invertedBitmap[row + 15] = 0x00;
```

This is performed for all 64 display rows.

---

# 🔁 Main Program Loop

The main loop continuously updates three systems:

```cpp
void loop() {

  updateButton();

  updateSpin();

  updateCelebration();
}
```

This keeps:

1. Button input
2. Reel animation
3. Win/jackpot animation

running independently without blocking delays during normal operation.

---

# 🚀 Startup Sequence

During `setup()` the program:

1. Starts Serial communication at 115200 baud.
2. Initializes I²C.
3. Sets I²C to 400 kHz.
4. Starts the SH1106 OLED.
5. Loads and inverts the slot-machine bitmap.
6. Cleans the bitmap edges.
7. Seeds the random-number generator using `micros()`.
8. Initializes the button.
9. Draws the initial slot-machine screen.

---

# 📊 Timing Summary

| System                    |   Value |
| ------------------------- | ------: |
| Button debounce           |   35 ms |
| Maximum click duration    |  350 ms |
| Double-click gap          |  300 ms |
| Long press                |  500 ms |
| Reel update               |   70 ms |
| Reel 1 stop               | 1050 ms |
| Reel 2 stop               | 1750 ms |
| Reel 3 stop               | 2450 ms |
| Win animation frame       |  180 ms |
| Normal win duration       | 1500 ms |
| Jackpot phase 1           | 3000 ms |
| Jackpot phase 2           | 4000 ms |
| Total jackpot celebration | 7000 ms |

---

# 📐 Display / Graphics Summary

| Asset               |       Size |
| ------------------- | ---------: |
| OLED                |     128×64 |
| Machine bitmap      |     128×64 |
| Machine bitmap data | 1024 bytes |
| Slot sprites        |      16×16 |
| Each sprite         |   32 bytes |
| Number of symbols   |          7 |
| Jackpot bitmap      |     128×64 |
| Jackpot bitmap data | 1024 bytes |

---

# 🗂️ Suggested Project Structure

```text
SlotMachine/
│
├── SlotMachine.ino
│
├── SLOT_MACHINE_BLANK.h
├── Jackpot_Win.h
│
├── CHERRY_SPRITE.h
├── LEMON_SPRITE.h
├── ORANGE_SPRITE.h
├── GRAPES_SPRITE.h
├── BELL_SPRITE.h
├── BAR_SPRITE.h
└── SEVEN_SPRITE.h
```

---

# 🔧 Software Requirements

The source uses:

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
```

Therefore the project requires an Arduino-compatible environment with:

* Arduino core
* Wire/I²C support
* **U8g2** graphics library

---

# 🧠 System Flow

The overall game flow is:

```text
                    ┌──────────────┐
                    │   IDLE       │
                    └──────┬───────┘
                           │
                    Double Click
                           │
                           ▼
                  ┌─────────────────┐
                  │ Choose Result   │
                  └────────┬────────┘
                           │
                           ▼
                  ┌─────────────────┐
                  │  Spin Reels     │
                  └────────┬────────┘
                           │
                 ┌─────────┴─────────┐
                 │                   │
              Losing              Winning
                 │                   │
                 │             ┌─────┴─────┐
                 │             │           │
                 │          3-of-kind     777
                 │             │           │
                 │             ▼           ▼
                 │           WIN!       JACKPOT
                 │
                 ▼
              Pity +1
                 │
                 ▼
                IDLE
```

---

# 🧪 Testing

For testing the jackpot animation:

```text
Click + Hold
```

This forces a `777` result while preserving the reel-spin animation.

This is useful for testing:

* Reel stopping
* `777` detection
* Jackpot frame 1
* Jackpot frame 2
* Full-screen `Jackpot_Win` bitmap
* Celebration timing
* Return to the slot-machine screen

---

# ⚠️ Current Button Actions

The following actions are currently placeholders:

### Single click

```text
Quotes / Tips
```

### Triple click

```text
History
```

### Long press

```text
Exit
```

Their handlers currently print the corresponding action to Serial but do not implement additional navigation or screen functionality.

---

# 📜 License

No license is specified in the provided source.

Add a license here if the project is intended to be distributed publicly.

---

## Project Status

**Standalone OLED slot-machine prototype**

Current implemented systems:

* ✅ OLED rendering
* ✅ Slot-machine artwork
* ✅ Seven custom sprites
* ✅ Reel animation
* ✅ Staggered reel stopping
* ✅ Normal spin results
* ✅ Three-of-a-kind system
* ✅ Pity system
* ✅ Natural 777
* ✅ Forced 777 testing
* ✅ Win animation
* ✅ Jackpot animation
* ✅ Button interaction system
* ✅ Serial debugging

Placeholder actions:

* ⏳ Quotes / Tips
* ⏳ History
* ⏳ Exit behavior
