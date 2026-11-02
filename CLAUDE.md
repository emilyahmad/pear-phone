# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

A hardware recreation of the Pear Phone from *Victorious*: a 2.8" touchscreen in a pear-shaped case, with a home screen of apps, a TheSlap.com profile screen, and a new-post screen with an on-screen keyboard. `journals/JOURNAL.md` is the build log (wiring tables, pinouts, setup decisions). `AGENTS.md` holds the original home-screen task brief and an early copy of `home-screen.ino`; it predates the touch-enabled `phone/` sketch, so treat it as background rather than the current state.

## Hardware and toolchain

- **MCU:** Seeed XIAO ESP32-S3 Sense (current). Older sketches in `code/c3/` target the XIAO ESP32-C3, which is no longer the main board.
- **Display:** Hosyond 2.8" 240x320 IPS, ILI9341V driver over SPI, with FT6336G capacitive touch over I2C (address `0x38`).
- **Build:** Arduino IDE, no CLI or test suite. Board package `esp32` by Espressif, board `XIAO_ESP32S3`. S3 sketches use the `Adafruit_ILI9341` + `Adafruit_GFX` libraries. The C3 sketches use LovyanGFX with `code/c3/lgfx_config.h`.
- Verification happens on the real device: upload, then watch the Serial Monitor at 115200 baud.
- XIAO `D`-labels and raw GPIO numbers are different naming systems. The `TFT_*` pins use `D` macros, while the `TOUCH_*` pins in `phone.ino` use raw GPIO numbers.
- `INVERT_COLORS true` is required for this panel. `SPI_SPEED` is kept low (2 MHz) while the wiring is still on jumpers.

## Code layout

- `code/UI/phone/` is the main, combined sketch. `phone.ino` owns the hardware setup (SPI, `tft`, touch reset/I2C), the screen state machine, and tap routing. Each screen lives in a header (`home_screen.h`, `slap_screen.h`) inside its own namespace (`home::`, `slap::`) so the two screens' helper names don't clash.
  - Each screen namespace exposes a `ROTATION` (home is portrait 240x320 at rotation 2, slap is landscape 320x240 at rotation 1), a draw entry point, and hit-tests (`home::appAt`, `slap::homeTabAt`, `slap::newTabAt`). `phone.ino` calls `setRotation` before drawing and uses `mapTouch()` to turn raw portrait touch coordinates into the current screen's coordinates. `TOUCH_OFFSET` fixes taps that land in the wrong place.
  - Screen states in `phone.ino` are `SCREEN_HOME`, `SCREEN_SLAP` and `SCREEN_COMPOSE`. Tapping the app named by `APP_TO_OPEN` ("SplashFace") opens TheSlap. TheSlap's Home tab goes back home, and its New tab opens the compose screen. The compose screen is also in `slap::` (`showCompose()`, `composeTap()`) and uses `slap::ROTATION`. `composeTap()` returns `COMPOSE_MISS`/`NONE`/`CANCEL`/`POSTED`, and a post writes into `slap::statusText`.
  - Tap handling: `handleTap()` returns true when a tap hits something. If the first touch reading misses, `loop()` keeps retrying it for `TAP_RETRY_MS`, because the FT6336G's first reported point can be a few pixels off. Every tap logs both its raw and mapped coordinates to Serial, which is how touch calibration is done.
  - `loop()` calls `slap::update()` (sparkle animation) on every non-home screen.
  - The headers use the global `tft` declared in `phone.ino`, so they must be included after it.
  - `#define`s inside the headers are not namespaced. Prefer `constexpr` in the namespace for anything that might clash between screens (as `CORNER_RADIUS`, `SCREEN_W` and `SCREEN_H` already do).
- `code/UI/*.ino` (`home-screen.ino`, `slap-dot-com.ino`, `pear-outline.ino`) are standalone prototypes of each screen with no touch. The headers in `phone/` were copied from them. A change made in one place is not reflected in the other, so be clear about which version you are editing (normally `phone/`).
- `code/s3/` holds hardware bring-up tests (display, touch, board sanity check). `code/c3/` holds the earlier C3 experiments (hello world, WiFi server, quiz).
- `production/` is the KiCad PCB project (`source/` is the working project, `fabrication/` holds the Gerbers sent to JLC).

## UI conventions

- Pear shape illusion: the screen is rectangular, so parts of it are covered by the case. Screens draw black outside a coral (`255,92,92`) rounded-rectangle border (`CORNER_RADIUS` 70, `LINE_THICKNESS` 3), and backgrounds are clipped row by row with `rowExtent()` so they stay inside it.
- Drawing is pure Adafruit_GFX primitives (row-by-row `drawFastHLine` gradients, circles, triangles). The only bitmap is the TheSlap logo (`THE_SLAP_LOGO`, a `PROGMEM` RGB565 array with a 1-bit mask in `slap_screen.h`). There is no framebuffer, so redraws are full-screen and slow at the current SPI speed. Only small regions (like the slap screen's sparkles in `slap::update()`) are animated.
- App icons are drawn procedurally to match the PNGs in `images/UI/app-icons/` (numbered `1.png` to `12.png`, and the comment above each `icon*` function names its source PNG). Other design references live in `design-references/`.
- Home screen layout: rows of 2/3/3/4 icons (`ROW_COUNT`, `ROW_Y`), centered with `PITCH_X` spacing and curved upward by `ARC_DEPTH`/`ARC_HALF_WIDTH`. In `phone/home_screen.h`, both drawing and `appAt()` get icon positions from `appCenter()`, so layout changes belong there.
- The Arduino IDE auto-generates function prototypes, so in `.ino` files, any struct used in a function signature (like `App` in the standalone `home-screen.ino`) must be declared before the first function. Headers (`.h`) are not auto-prototyped.
