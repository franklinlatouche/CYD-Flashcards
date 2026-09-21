# CYD Flashcards

A touchscreen flashcard trainer for the ESP32 "Cheap Yellow Display" (CYD), built for studying CompTIA A+, cybersecurity fundamentals, and IT helpdesk/enterprise support concepts (Active Directory, Intune/MDM, networking, hardware).

## Summary

This project turns a low-cost ESP32 dev board with an integrated touchscreen into a standalone, offline flashcard device. Cards are grouped by topic (`[Security]`, `[Networking]`, `[Hardware]`, `[Intune]`) so related concepts can be studied together, and navigation is fully touch-driven: flip a card to reveal its definition, or step through the deck in either direction with on-screen arrows. Everything runs from a single Arduino sketch with no external dependencies (no wifi, no SD card, no cloud sync), the full deck lives in flash memory and updates with a re-upload.

**Features**
- 70+ terms covering CompTIA A+, cybersecurity, networking, and enterprise IT/Intune topics
- Touch navigation: tap to flip, tap left/right arrows to move between cards, long-press to shuffle
- Auto word-wrap with dynamic font sizing, long definitions never run off screen
- Dark color scheme, high-contrast text for readability
- Deck is a single array in the sketch, adding or editing cards requires no other code changes

## Hardware

| Component | Detail |
|---|---|
| Board | ESP32-2432S028 ("Cheap Yellow Display", wifi-only variant) |
| Display | 2.8" TFT, 320x240, ILI9341 driver (SPI) |
| Touch | XPT2046 resistive touch controller (SPI, separate bus from display) |
| MCU | ESP32 (dual-core, wifi/BT capable, wifi unused by this sketch) |

## Project Directory

```
cyd_flashcards/
  cyd_flashcards.ino   - full sketch: deck data, touch handling, display draw
  README.md
```

## First Run

1. **Install Arduino IDE** (2.x). Get it from arduino.cc if not already installed.

2. **Add ESP32 board support**:
   - File > Preferences > "Additional Boards Manager URLs", add:
     `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools > Board > Boards Manager, search `esp32` (by Espressif Systems), install.

3. **Install USB-serial driver** (only if the board doesn't show up as a COM/tty port):
   - Most CYD boards use CH340 or CP2102 USB-serial chips. Check Device Manager (Windows) or `ls /dev/tty.*` (macOS) / `dmesg | grep tty` (Linux) after plugging in.
   - CH340 driver: search "CH340 driver" for your OS if the port doesn't appear.
   - Linux: usually works out of the box, add your user to the `dialout` group if you get a permissions error (`sudo usermod -aG dialout $USER`, then log out/in).

4. **Install libraries** via Library Manager (Sketch > Include Library > Manage Libraries):
   - `TFT_eSPI` by Bodmer
   - `XPT2046_Touchscreen` by Paul Stoffregen

5. **Configure TFT_eSPI for this board**. This is the step people get stuck on, since the default library setup doesn't know the CYD's pin mapping.
   - Locate the installed library folder: usually `~/Arduino/libraries/TFT_eSPI/` (Linux/macOS) or `Documents\Arduino\libraries\TFT_eSPI\` (Windows). Note: if you have multiple TFT_eSPI installs or a renamed copy (e.g. from another project), confirm which one Arduino IDE actually compiles against.
   - Open `User_Setup_Select.h` inside that folder and comment out the default include line:
     ```cpp
     //#include <User_Setup.h>           // Default setup is root library folder
     ```
   - Add a CYD-specific setup file. This repo's development environment uses a custom file, `Setup_CYD_2432S028.h`, placed in the library's `User_Setups/` folder with these settings for the board's ILI9341 panel:
     ```cpp
     #define ILI9341_2_DRIVER   // not ILI9341_DRIVER, see note below
     #define TFT_WIDTH  240
     #define TFT_HEIGHT 320
     #define TFT_MISO 12
     #define TFT_MOSI 13
     #define TFT_SCLK 14
     #define TFT_CS   15
     #define TFT_DC    2
     #define TFT_RST  -1   // not connected, tied to EN
     #define TFT_BL   21
     #define TFT_BACKLIGHT_ON HIGH
     #define TFT_INVERSION_ON  // see note below
     ```
     Create this file yourself (or search "CYD TFT_eSPI User_Setup" for community-maintained alternatives), then reference it from `User_Setup_Select.h`:
     ```cpp
     #include <User_Setups/Setup_CYD_2432S028.h>
     ```
   - Some board batches use an ST7789 driver instead of ILI9341, check the back of your board or seller listing if the screen stays blank with the above settings.
   - Wrong setup file = blank/garbled screen, this is the #1 cause of "nothing shows up."
   - **Panel variant quirks found during bring-up on this exact unit** (worth checking if yours behaves the same):
     - With the standard `ILI9341_DRIVER`, no rotation value (0-3, or the extra 4-7 BMP-mirroring values) produced correct non-mirrored landscape text, this specific clone controller mirrors column addressing regardless of the MADCTL rotation bits. Switching to `ILI9341_2_DRIVER` fixed it outright.
     - `ILI9341_2_DRIVER` inverts color polarity versus `ILI9341_DRIVER` on this panel, without `TFT_INVERSION_ON` the dark theme rendered washed out/inverted (light background instead of black).
     - After either driver or `User_Setup_Select.h` change, **fully close and reopen Arduino IDE** before re-uploading. The IDE can cache compiled TFT_eSPI library objects and silently skip picking up header changes otherwise, showing no effect from an edit that actually did apply.

6. **Confirm touch controller type**. This sketch assumes resistive touch (XPT2046, wired over SPI, matches the `#define XPT2046_*` pins at the top of the .ino). Some newer CYD batches ship capacitive touch (GT911, wired over I2C instead). Check your board silkscreen/listing. If it's GT911, the touch include and `setup()` init need to change (different library, different pins), the display code stays the same.

7. **Board settings in Arduino IDE**:
   - Tools > Board > select `ESP32 Dev Module`.
   - Tools > Upload Speed: `921600` (drop to `115200` if uploads fail/hang).
   - Tools > Port: pick the COM/tty port the board enumerated as in step 3.
   - Tools > Flash Size: `4MB` (default is fine for this sketch, no SPIFFS/partitioning needed).

8. **Upload**: click Upload (arrow icon). Hold the board's `BOOT` button if it doesn't auto-enter flash mode. Watch the IDE output for "Connecting...", press and hold BOOT until it starts uploading. Some CYD boards need this, others auto-reset fine.

9. **Verify**: screen should show "TERM" + first card term, `<`/`>` arrows at the edges, and a `1 / N` counter at the bottom (N = current deck size). Tap center to flip to definition, tap arrows to move between cards.

## Usage

- Tap center of screen: flip card between term and definition.
- Tap `<` (left edge): previous card, resets to term side.
- Tap `>` (right edge): next card, resets to term side.
- Long-press (~800ms) anywhere: reshuffle deck order, resets to card 1.
- Add/edit terms: edit `deck[]` array in `cyd_flashcards.ino`, no other code changes needed (deck size auto-updates).
- Category tags: term strings are prefixed with `[Security]`, `[Networking]`, `[Hardware]`, or `[Intune]` where the topic maps cleanly to one domain, general OS/helpdesk terms are left untagged. On screen, the tag renders as its own small line under the "TERM" header, separate from the term text itself, so it reads as a category label rather than part of the term.

## How It Works

- `deck[]`: array of `{term, def}` structs, source of truth for content. Term strings optionally start with a `[Tag]` prefix (e.g. `[Security] Firewall`).
- `splitTag()`: parses that prefix out of a term string into a separate tag and the remaining term text. `drawCard()` renders the tag as its own line under the "TERM" header on the term side only, the definition side and untagged terms are unaffected.
- `order[]`: shuffled index array, controls draw order without mutating `deck`.
- Touch is polled each loop via `ts.touched()`. On press, `ts.getPoint()` grabs raw X and maps it to screen pixels (`TS_MINX`/`TS_MAXX` calibration constants near the top of the file). On release, press duration and X position decide the action: short tap in the `ARROW_ZONE` on either edge = prev/next, short tap elsewhere = flip, long hold = shuffle.
- If arrow taps feel reversed or the zones don't line up with the drawn `<`/`>` glyphs, the raw touch calibration is off for your specific panel, adjust `TS_MINX`/`TS_MAXX`/`TS_MINY`/`TS_MAXY` (add a `Serial.print(p.x)` in `loop()` to see raw values while tapping the edges).
- `wrapAndDraw()`: word-wraps text and vertically centers the resulting block around the middle of the screen. Terms use a larger font (font 4) since they're short, definitions use a smaller font (font 2) with tighter line spacing since some run over 100 characters, this keeps every card readable without running into the footer counter or side arrows regardless of text length.
- No wifi, no persistence: state resets on reboot/reshuffle only, deck is fully static per-flash.
