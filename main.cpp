#include "main.hpp"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <cstdlib>
#include <vector>

#include "clock.hpp"
#include "fonts/bitmap_db.h"
#include "fonts/lowfontgen.h"
#include "input.hpp"
#include "output.hpp"
#include "pico/critical_section.h"
#include "pico/stdlib.h"
#include "pico_display_2.hpp"
#include "ui.hpp"
#include "background.hpp"

critical_section_t debounce_section;

using namespace pimoroni;

extern uint16_t     buffer[];
extern PicoDisplay2 pico_display;


// displayState dState = Clock;
// setupState   sState = Hours;
int dState = Clock;
int sState = Hours;

// Let's divide the screen in 2 windows
// Win1 is 2/3 of the screen from the top
// Win2 is 1/3 of the screen from bottom
int w1top;
int w1dwn;
int w2top;
int w2dwn;

// Main string for messages
char mainString[64];

volatile uint8_t scheduler = 0;

char dayStr[5];
char montStr[12];
char dowStr[12];
char yearStr[5];

// Here we play with internal timers
void oneSecCallback(void)
{
    if (++sec >= 60)
    {
        if (++min >= 60)
        {
            if (++hours >= 24)
                hours = 0;
            min = 0;
        }
        sec = 0;
    }
    // sprintf(mainString, "%02d:%02d:%02d", hours, min, sec);
}

bool oneTwenthCallback(struct repeating_timer *t)
{
    if (++scheduler >= 20)
    {
        oneSecCallback();
        scheduler = 0;
    }
    return true;
}

int main()
{
    static uint8_t oldsk;
    critical_section_init(&debounce_section);

    pico_display.init();
    pico_display.set_backlight(128); // was 255

    struct repeating_timer debounceTimer;

    struct repeating_timer oneTwenthtimer;

    // Strings for printing information

    w1top = 0;
    w1dwn = (pico_display.bounds.h / 3) * 2;
    w2top = w1dwn + 1;
    w2dwn = pico_display.bounds.h;

    Point text_location(pico_display.bounds.w / 4, w2top + ((w2dwn - w2top) / 3));
    Point mainS_location(5, w2top + ((w2dwn - w2top) / 3 + 16));
    Point scrolling_line_b(0, w2top);
    Point scrolling_line_e(pico_display.bounds.w, w2top);

    sprintf(mainString, "********");

    // Debouncing Timer
    add_repeating_timer_ms(2, Debounce, NULL, &debounceTimer);

    add_repeating_timer_ms(50, oneTwenthCallback, NULL, &oneTwenthtimer);

    while (true)
    {
        menuHandler(ReadInput());

        if (oldsk != scheduler)
        {
            pico_display.set_pen(0, 0, 100); // Dark Blue
            pico_display.clear();

            draw_background();

            updateHour(hours, min, sec);

            // Since HSV takes a float from 0.0 to 1.0 indicating hue,
            // then we can divide millis by the number of milliseconds
            // we want a full colour cycle to take. 5000 = 5 sec.
            /*
      uint8_t r = 0, g = 0, b = 0;
      from_hsv((float)millis() / 5000.0f, 1.0f, 0.5f + sinf(millis() / 100.0f
      / 3.14159f) * 0.5f, r, g, b); pico_display.set_led(r, g, b);
      */

            updateDisplay();

            pico_display.set_pen(255, 0, 0); // Red
            pico_display.text(mainString, mainS_location, 320);

            // pico_display.line(scrolling_line_b, scrolling_line_e);

            // update screen

            pico_display.update();
            oldsk = scheduler;
        }
    }

    return 0;
}
