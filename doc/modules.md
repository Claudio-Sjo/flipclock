# Modules

## Firmware Modules

### main.cpp / main.hpp

Entry point. Initializes I2C, display, DS3231 RTC, and repeating timers. Runs the main rendering loop gated by the `scheduler` variable. Also contains EEPROM test code (writes 0x55/0xAA and reads back).

**Defines** (main.hpp):
- `pt` — particle/shape struct for background animation (position, velocity, color, shine)
- `displayState` enum — `Clock`, `ClockSetup`, `DateSetup`, `AlarmSetup`
- `setupState` enum — `Hours`, `Minutes`, `Day`, `Dotw`, `Month`, `Year`
- `dayOfWeek`, `monthOfYear`, `backGround` enums

### clock.cpp / clock.hpp

Timekeeping and digit rendering orchestration.

- `datetime_t t` — global time struct, shared across modules
- `updateHour(hh, mm, ss)` — renders HH:MM:SS digits. Detects changes and triggers flip animation via `mergeDigitPrint()`. Highlights hours/minutes in green during setup.
- `oneTwenthCallback()` — 50 ms timer. Increments `scheduler` (0–19). Every 20 ticks reads RTC (normal mode) or freezes seconds (setup mode). On setup exit, writes time to Pico RTC and DS3231.

### output.cpp / output.hpp

Display rendering engine. Owns the framebuffer and `PicoDisplay2` instance.

- `printDigit(location, digit, r, g, b)` — renders a digit using `digitFont[]` span data (64 scanlines × 4 values each)
- `mergeDigitPrint(location, before, after, sk, r, g, b)` — flip animation: interpolates spans between two digits based on scheduler position
- `myPrintLine(start, end, r, g, b)` — draws a horizontal line with anti-aliased edges (1-pixel fade on each end)
- `myPrintLowFont(location, str)` — renders text using the 24pt bitmap font
- `myPrintDayLowFont(location, str)` — renders mixed text: uppercase from 24pt font, lowercase from 12pt font
- `getStrLenLowFont()` / `getStrLenDayLowFont()` — compute pixel width of a string for centering
- `updateDisplay()` — formats and renders the date line (day-of-week, ordinal day, month, year) with green highlighting during setup
- `from_hsv(h, s, v, r, g, b)` — HSV to RGB conversion (currently unused)

### input.cpp / input.hpp

Button handling with debounce state machine.

4 buttons (A, B, X, Y) each with a 4-state machine:
- **Idle** → button pressed → **Debouncing**
- **Debouncing** → 30 ms elapsed → **Pressed** (or bounce → Idle)
- **Pressed** → released after 75 ms → short press; held > 1 s → long press → **Waiting**
- **Waiting** → released → **Idle**

Key events are queued in a 32-entry ring buffer with critical section protection. Each button generates two key codes: short press (`Key_A`..`Key_Y`) and long press (`Key_AL`..`Key_YL`).

### ui.cpp / ui.hpp

Menu state machine. Processes key events from `ReadInput()`:

| Button | Clock mode          | ClockSetup mode                    |
|--------|---------------------|------------------------------------|
| A      | Enter setup         | Exit setup (save time)             |
| B      | —                   | Cycle field (H→M→D→DoW→Mo→Y)      |
| X      | —                   | Increment selected field           |
| Y      | —                   | Decrement selected field           |

Handles month-aware day limits (28/29/30/31) and wrapping for all fields.

### background.cpp / background.hpp

Animated sky background with day/night cycle.

- **Daytime** (6:00–19:00): blue sky, sun with pulsating rays tracking across the screen
- **Nighttime**: dark blue sky with 50 twinkling star shapes drifting slowly left
- `update_bground_target()` — sets target colors/brightness based on current hour
- `paintCallback()` — smoothly transitions RGB values toward targets (1 step per 4 s per channel)
- `initialise_bg()` — seeds random star positions, starts 1-min and 4-s timers

Color palette: 11 blue shades from midnight blue to light azure, plus 2 sunshine tones (orange, yellow).

### sunutils.cpp

Sunrise/sunset calculator using the standard solar position algorithm. Takes year, month, day, latitude, longitude, UTC offset, and daylight savings flag. Returns decimal hours.

**Currently not wired in** — `background.cpp` uses hardcoded sunrise=6:00, sunset=19:00.

## Library Modules

### MemI2C/ (memI2C.c / memI2C.h)

Pico-native blocking I2C read/write for memory-mapped devices (EEPROMs, RTCs with multi-byte addresses). Directly manipulates RP2040 I2C hardware registers. Supports variable-length addresses (1–4 bytes, big-endian).

Key functions:
- `i2c_write_mem_blocking(i2c, addr, mem_addr, addr_len, data, len)`
- `i2c_read_mem_blocking(i2c, addr, mem_addr, addr_len, data, len)`

### DS3231_HAL/ (DS3231_HAL.c / DS3231_HAL.h / DS3231_def.h)

Hardware abstraction for the DS3231 RTC. Originally written for NXP i.MX RT with FreeRTOS, adapted for Pico using `MemI2C` for I2C transport.

Key functions:
- `InitRtc(callback, irq_rate, i2c)` — initialize RTC, configure interrupts
- `GetRtcTime(datetime_t*)` — read time (BCD to binary conversion)
- `SetRtcTime(datetime_t*)` — write time (binary to BCD conversion)
- `GetRtcTemp()` — read on-chip temperature sensor (0.25°C resolution)

`DS3231_def.h` provides a register-map struct with anonymous unions mirroring the DS3231's register layout.

### DS3231_HAL/ (Si5351_HAL.c / Si5351_HAL.h / Si5351_def.h)

Driver for the Si5351 programmable clock generator. Supports PLL configuration, multisynth divider computation (continued fractions), and per-output frequency setting. Uses millihertz as the base frequency unit.

**Included but not used** by the clock firmware.

### DS3231_HAL/ (ClockI2C.c / ClockI2C.h)

NXP LPI2C transport layer with FreeRTOS RTOS handles. **Legacy code**, not used on the Pico target.

## Font Data

### fonts/clockFonts.h

Pre-generated bitmap font data for the "Forte" typeface:
- **72pt** — digits 0–9, 64 lines tall, 33–47 px wide (main clock display)
- **24pt** — alphanumeric, 29 lines tall (date/text)
- **12pt** — lowercase subset (a, d, h, n, r, s, t, z), 12 lines tall (day abbreviations)

### fonts/lowfontgen.h

Extern declarations for the 24pt font data arrays.

### fonts/bitmap_db.h

Font data structures:
- `FONT_CHAR_INFO` — per-character: width, height, byte offset
- `FONT_INFO` — font descriptor: height, char range, space width, pointers to char info and bitmap data

### fontgenerator/

Offline tool that converts 72pt bitmap font into span-based `digitFont[]` format. For each digit, for each of 64 scanlines, outputs 4 values: begin/end of up to two horizontal pixel runs. This enables fast rendering by filling spans rather than testing individual pixels.
