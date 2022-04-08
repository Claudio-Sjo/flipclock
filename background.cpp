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
#define BALLONS 20
#define STARS   50
#define OBJECTS MAX(BALLONS, STARS)

std::vector<pt> shapes;

bgEnum                 background = Stars;

struct repeating_timer oneMinuteTimer;

typedef struct _bgscreen
{
    uint8_t  bglight;
    uint16_t sunPosH;
    uint16_t sunPosV;
    uint16_t w1bh;
    uint16_t w1bv;
    uint16_t w1eh;
    uint16_t w1ev;
    uint8_t  w1r;
    uint8_t  w1g;
    uint8_t  w1b;
    uint16_t w2bh;
    uint16_t w2bv;
    uint16_t w2eh;
    uint16_t w2ev;
    uint8_t  w2r;
    uint8_t  w2g;
    uint8_t  w2b;
    uint16_t w3bh;
    uint16_t w3bv;
    uint16_t w3eh;
    uint16_t w3ev;
    uint8_t  w3r;
    uint8_t  w3g;
    uint8_t  w3b;
    uint16_t w4bh;
    uint16_t w4bv;
    uint16_t w4eh;
    uint16_t w4ev;
    uint8_t  w4r;
    uint8_t  w4g;
    uint8_t  w4b;
} bgscreen;

bgscreen bground;

void update_bground(uint32_t hh, uint32_t mm)
{
    // Depending on the hour, we decide to have one or more windows
    // During the night there's only one window w1
    if ((hh <= 6) || (hh > 19))
    {
        bground.bglight = 128;
        bground.w1bv    = 0;
        bground.w1bh    = 0;
        bground.w1ev    = 240;
        bground.w1eh    = 320;
        bground.w1r     = 0;
        bground.w1g     = 0;
        bground.w1b     = 100;
        bground.w2bh = bground.w2eh = bground.w2bv = bground.w2ev = 0;
        bground.w3bh = bground.w3eh = bground.w3bv = bground.w3ev = 0;
        bground.w4bh = bground.w4eh = bground.w4bv = bground.w4ev = 0;
        background                                                = Stars;
    }
    if ((hh > 6) && (hh <= 9))
    {
        int totMinutes = 60 * (hh - 6) + mm;

        bground.bglight = 128 + (128 / 4) * (hh - 6);
        bground.w1bh = bground.w1bv = 0;
        bground.w1ev                = 240;
        bground.w1eh                = totMinutes;
        bground.w1r                 = 128;
        bground.w1g                 = 212;
        bground.w1b                 = 255;
        bground.w2bv                = 0;
        bground.w2bh                = bground.w1eh;
        bground.w2ev                = 240;
        bground.w2eh                = 320;
        bground.w2r                 = 0;
        bground.w2g                 = 0;
        bground.w2b                 = 100;
        bground.w3bh = bground.w3eh = bground.w3bv = bground.w3ev = 0;
        bground.w4bh = bground.w4eh = bground.w4bv = bground.w4ev = 0;
        background = Balloons;
    }
    if ((hh > 9) && (hh <= 16))
    {
        bground.bglight = 255;
        bground.w1bh = bground.w1eh = bground.w1bv = bground.w1ev = 0;
        bground.w2bv                                              = 0;
        bground.w2bh                                              = 0;
        bground.w2ev                                              = 240;
        bground.w2eh                                              = 320;
        bground.w2r                                               = 128;
        bground.w2g                                               = 212;
        bground.w2b                                               = 255;
        bground.w3bh = bground.w3eh = bground.w3bv = bground.w3ev = 0;
        bground.w4bh = bground.w4eh = bground.w4bv = bground.w4ev = 0;
        background                                                = Balloons;
    }
    if ((hh > 16) && (hh <= 19))
    {
        int totMinutes  = 60 * (hh - 16) + mm;

        bground.bglight = bground.bglight = 255 - (128 / 4) * (hh - 6);
        bground.w1bh = bground.w1bv                               = 0;
        bground.w1ev                                              = 240;
        bground.w1eh                                              = 140 + totMinutes;
        bground.w1r                                               = 0;
        bground.w1g                                               = 0;
        bground.w1b                                               = 100;
        bground.w2bv                                              = 0;
        bground.w2bh                                              = bground.w1eh;
        bground.w2ev                                              = 240;
        bground.w2eh                                              = 320;
        bground.w2r                                               = 128;
        bground.w2g                                               = 212;
        bground.w2b                                               = 255;
        bground.w3bh = bground.w3eh = bground.w3bv = bground.w3ev = 0;
        bground.w4bh = bground.w4eh = bground.w4bv = bground.w4ev = 0;
        background                                                = Balloons;
    }
}

bool oneMinuteCallback(struct repeating_timer *rt)
{
    update_bground(t.hour, t.min);
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
    add_repeating_timer_ms(50, oneMinuteCallback, NULL, &oneMinuteTimer);
}

void draw_background(void)
{
    static uint16_t counter;

    pico_display.set_backlight(bground.bglight); // was 255

    pico_display.clear();

    if (bground.w1ev != 0)
    {
        pico_display.set_pen(bground.w1r, bground.w1g, bground.w1b); // Dark Blue

        std::vector<Point> poly;
        poly.push_back(Point(bground.w1bh, bground.w1bv));
        poly.push_back(Point(bground.w1eh, bground.w1bv));
        poly.push_back(Point(bground.w1eh, bground.w1ev));
        poly.push_back(Point(bground.w1bh, bground.w1ev));

        pico_display.polygon(poly);
    }

    if (bground.w2ev != 0)
    {
        pico_display.set_pen(bground.w2r, bground.w2g, bground.w2b); // Dark Blue

        std::vector<Point> poly;
        poly.push_back(Point(bground.w2bh, bground.w2bv));
        poly.push_back(Point(bground.w2eh, bground.w2bv));
        poly.push_back(Point(bground.w2eh, bground.w2ev));
        poly.push_back(Point(bground.w2bh, bground.w2ev));

        pico_display.polygon(poly);
    }

    if (bground.w3ev != 0)
    {
        pico_display.set_pen(bground.w3r, bground.w3g, bground.w3b); // Dark Blue

        std::vector<Point> poly;
        poly.push_back(Point(bground.w3bh, bground.w3bv));
        poly.push_back(Point(bground.w3eh, bground.w3bv));
        poly.push_back(Point(bground.w3eh, bground.w3ev));
        poly.push_back(Point(bground.w3bh, bground.w3ev));

        pico_display.polygon(poly);
    }

    if (bground.w4ev != 0)
    {
        pico_display.set_pen(bground.w4r, bground.w4g, bground.w4b); // Dark Blue

        std::vector<Point> poly;
        poly.push_back(Point(bground.w4bh, bground.w4bv));
        poly.push_back(Point(bground.w4eh, bground.w4bv));
        poly.push_back(Point(bground.w4eh, bground.w4ev));
        poly.push_back(Point(bground.w4bh, bground.w4ev));

        pico_display.polygon(poly);
    }

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