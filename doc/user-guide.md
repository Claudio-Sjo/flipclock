# User Guide

## Normal Operation

The clock displays the current time in HH:MM:SS format with a flip animation when digits change. Below the time, the current date is shown as:

```
DayOfWeek  Day(ordinal)
     Month  Year
```

The background animates between day and night:
- **Daytime** (6:00–19:00): blue sky with a sun that tracks across the screen
- **Nighttime**: dark sky with twinkling stars

Sundays are displayed in red.

## Button Controls

### Normal Mode

| Button | Action         |
|--------|----------------|
| A      | Enter setup    |
| B      | —              |
| X      | —              |
| Y      | —              |

### Setup Mode

The currently selected field is highlighted in green, and "Setup" is displayed at the bottom of the screen.

| Button | Action                                          |
|--------|-------------------------------------------------|
| A      | Exit setup and save time to RTC                 |
| B      | Cycle to next field (Hours → Minutes → Day → Day of Week → Month → Year → Location) |
| X      | Increment selected field                        |
| Y      | Decrement selected field                        |

All fields wrap around at their boundaries (e.g., hours: 23 → 0, months: December → January, locations: Zagreb → Amsterdam). Day limits are month-aware (28/29/30/31).

While in setup mode, seconds are frozen at 0. On exit, the time is written to both the Pico's internal RTC and the DS3231 hardware RTC.

## Location Setting

When the **Location** field is selected, the screen displays "Location" with the city name below it. Use X/Y to cycle through 45 European capital cities (alphabetically sorted). The selected location is used for sunrise/sunset calculations that drive the day/night background animation.

## Button Press Types

Each button supports two press types:
- **Short press** (> 75 ms): standard action
- **Long press** (> 1 s): extended action (currently mapped but not differentiated in the menu handler)
