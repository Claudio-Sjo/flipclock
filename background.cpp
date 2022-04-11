#include <cstdlib>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>

#include "clock.hpp"
#include "main.hpp"
#include "output.hpp"
#include "pico/stdlib.h"
#include "pico_display_2.hpp"

// 2022/04/06 - New strategy
// - draw_background shall set all the screen background starting from the colors, windows etc.
// - The object shown will change from day to night
// Durung the day the sun will move from left to rigth
// The daylight will change depending on the time
// During the day from time to time birds will move around and clouds will be up in the sky
// During the night the moon will move as the sun in the day with stars shining
// It was 100 baloons, but for half screen we change to 50
#define BALLONS   20
#define STARS     50
#define OBJECTS   MAX(BALLONS, STARS)
#define ONEMINUTE (60 * 1000) // in milliseconds

std::vector<pt> shapes;

uint32_t sunrise    = 6;
uint32_t sunset     = 19;
bgEnum   background = Stars;

struct repeating_timer oneMinuteTimer;

typedef struct rgb_type
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_t;

rgb_t colors[] = {
    {34, 0, 102},   // Midnight Blue
    {0, 0, 128},    // Navy Blue
    {0, 30, 179},   // Zapphre
    {0, 68, 204},   // Sapphire
    {0, 115, 230},  // True Blue
    {0, 128, 255},  // Azure
    {25, 178, 255}, // Spiro Disco Blue
    {51, 187, 255},
    {77, 195, 255},
    {102, 204, 255},
    {128, 212, 255},
};

typedef struct bgscreen_type
{
    uint8_t  bglight;
    uint16_t sunPosH;
    uint16_t sunPosV;
    rgb_t    w1color;
} bgscreen;

bgscreen bground;

void update_bground(uint32_t hh, uint32_t mm)
{
    // Depending on the hour, we decide to have one or more windows
    // During the night there's only one window w1
    if ((hh <= (sunrise - 1)) || (hh > (sunset + 1)))
    {
        bground.bglight = 128;
        bground.w1color = colors[0];
        background      = Stars;
    }
    if ((hh > (sunrise - 1)) && (hh <= sunrise))
    {
        int totMinutes = 60 * (hh - 6) + mm;

        bground.bglight = 128;
        bground.w1color = colors[mm / 6];
        background      = Stars;
    }
    if ((hh > (sunrise)) && (hh <= sunrise + 1))
    {
        int totMinutes = 60 * (hh - 6) + mm;

        bground.bglight = 128 + (mm * 2);
        bground.w1color = colors[10];
        background      = Balloons;
    }
    if ((hh > sunrise + 1) && (hh <= sunset - 1))
    {
        bground.bglight = 255;
        bground.w1color = colors[10];
        background      = Balloons;
    }
    if ((hh > sunset - 1) && (hh <= sunset))
    {
        int totMinutes = 60 * (hh - 6) + mm;

        bground.bglight = 128 + ((60 - mm) * 2);
        bground.w1color = colors[10];
        background      = Balloons;
    }
    if ((hh > (sunset)) && (hh <= sunset + 1))
    {
        int totMinutes = 60 * (hh - 6) + mm;

        bground.bglight = 128;
        bground.w1color = colors[(60 - mm) / 6];
        background      = Stars;
    }
}

bool oneMinuteCallback(struct repeating_timer *rt)
{
    update_bground(t.hour, t.min);
    /* Testing the updates a bit faster than on real time
    static uint32_t hh = 0;
    static uint32_t mm = 0;

    if (++mm > 59)
    {
        mm = 0;
        if (++hh > 24)
        {
            hh = 0;
        }
    }
    update_bground(hh, mm);
     End of test part */
    return true;
}

void initialise_bg(void)
{

    for (int i = 0; i < OBJECTS; i++)
    {
        pt shape;

        shape.x  = rand() % (pico_display.bounds.w);
        shape.y  = rand() % w2dwn;
        shape.r  = (rand() % 10) + 3;
        shape.dx = float(rand() % 255) / 64.0f;
        shape.dy = float(rand() % 255) / 64.0f;
        shape.pen =
            pico_display.create_pen(rand() % 255, rand() % 255, rand() % 255);
        shapes.push_back(shape);
        shape.upDn  = true;
        shape.shine = float(rand() % 255) / 64.0f;
    }
    add_repeating_timer_ms(ONEMINUTE, oneMinuteCallback, NULL, &oneMinuteTimer);
    update_bground(t.hour, t.min);
}

void draw_background(void)
{
    static uint16_t counter;

    pico_display.set_backlight(bground.bglight); // was 255

    pico_display.set_pen(bground.w1color.r, bground.w1color.g, bground.w1color.b);

    pico_display.clear();

    counter++;
    for (auto &shape : shapes)
    {
        if (background == Balloons)
        {
            shape.y = shape.y > w1dwn ? w1dwn : shape.y;

            shape.x += shape.dx;
            shape.y += shape.dy;
            if ((shape.x - shape.r) < 0)
            {
                shape.dx *= -1;
                shape.x = shape.r;
            }
            if ((shape.x + shape.r) >= (pico_display.bounds.w))
            {
                shape.dx *= -1;
                shape.x = pico_display.bounds.w - shape.r;
            }
            if ((shape.y - shape.r) < 0)
            {
                shape.dy *= -1;
                shape.y = shape.r;
            }
            if ((shape.y + shape.r) >= w1dwn)
            {
                shape.dy *= -1;
                shape.y = w1dwn - shape.r;
            }

            pico_display.set_pen(shape.pen);
            pico_display.circle(Point(shape.x, shape.y), shape.r);
        }
        if (background == Stars)
        {
            // Let's slowly move the stars from right to left
            shape.x -= 6 / 3600.0;

            if (shape.upDn == true)
                shape.shine += (shape.dx * 0.1);
            else
                shape.shine -= (shape.dx * 0.1);

            if (abs(shape.shine) > 5.0f)
                shape.upDn = (shape.upDn == true) ? false : true;

            int cross = int(abs(shape.shine));
            int diag  = cross / 2;

            if (shape.x < 5)
            {
                shape.x = rand() % (pico_display.bounds.w);
                shape.y = rand() % w2dwn;
            }

            pico_display.set_pen(255, 255, 0); // Shining yellow
            pico_display.line(Point(shape.x - cross, shape.y),
                              Point(shape.x + cross, shape.y));
            pico_display.line(Point(shape.x, shape.y - cross),
                              Point(shape.x, shape.y + cross));
            pico_display.line(Point(shape.x - diag, shape.y - diag),
                              Point(shape.x + diag, shape.y + diag));
            pico_display.line(Point(shape.x - diag, shape.y + diag),
                              Point(shape.x + diag, shape.y - diag));
            pico_display.circle(Point(shape.x, shape.y), 1);
        }
    }
}