# Architecture

## Overview

The firmware runs a super-loop architecture on the RP2040 (Raspberry Pi Pico) with timer-driven callbacks for background tasks.

## Main Loop

```
main()
  ├── Hardware init (I2C, display, RTC, timers)
  └── loop:
        ├── menuHandler(ReadInput())     — process button presses
        ├── draw_background()            — sky/sun/stars animation
        ├── updateHour(h, m, s)          — render clock digits with flip animation
        ├── updateDisplay()              — render date/day-of-week text
        └── pico_display.update()        — push framebuffer to LCD
```

The loop is gated by a `scheduler` variable (0–19) incremented every 50 ms, giving an effective 20 Hz refresh rate. The display only redraws when `scheduler` changes.

## Timer Callbacks

| Interval | Callback              | Purpose                                      |
|----------|-----------------------|----------------------------------------------|
| 2 ms     | `Debounce()`          | Button state machine, scans one button per tick |
| 50 ms    | `oneTwenthCallback()` | Increments scheduler; reads RTC every 20 ticks (1 s) |
| 1 min    | `oneMinuteCallback()` | Updates background color/sun position targets |
| 4 s      | `paintCallback()`     | Smoothly transitions background colors toward targets |

## Module Dependency Graph

```
main.cpp
  ├── clock.cpp      — timekeeping, digit rendering orchestration
  ├── output.cpp     — display rendering (digits, fonts, lines)
  ├── input.cpp      — button debounce and key queue
  ├── ui.cpp         — menu state machine
  ├── background.cpp — animated sky (day/night cycle)
  ├── sunutils.cpp   — sunrise/sunset calculator (currently unused)
  ├── DS3231_HAL/    — RTC driver (adapted from NXP/FreeRTOS)
  └── MemI2C/        — Pico-native I2C memory access
```

## State Machine

The UI has two primary states controlled by `dState`:

- **Clock** — normal display mode, time is read from RTC
- **ClockSetup** — time/date editing mode, RTC reads are paused

Within `ClockSetup`, `sState` cycles through: Hours → Minutes → Day → DotW → Month → Year.

On exiting setup, the edited time is written to both the Pico's internal RTC and the DS3231.

## Flip Animation

The signature feature. Each digit (0–9) is stored as 64 scanlines × 4 span values (start1, end1, start2, end2). When a digit changes, `mergeDigitPrint()` linearly interpolates the span positions from the old digit to the new digit over 10 scheduler ticks (~500 ms), creating a smooth morphing effect.
