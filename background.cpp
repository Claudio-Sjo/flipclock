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
// During the night the moon will move as the sun in the day with Nigth shining
// It was 100 baloons, but for half screen we change to 50
#define BALLONS 20
#define STARS 50
#define OBJECTS MAX(BALLONS, STARS)
#define ONEMINUTE (60 * 1000) // in milliseconds
#define PAINT (4 * 1000)      // in milliseconds
#define SUNSIZE 20
#define SUNSDIA (2 * SUNSIZE)

std::vector<pt> shapes;

uint32_t sunrise = 6 * 1 + 0; // in minutes
uint32_t sunset = 19 * 1 + 0; // in minutes
float_t dayLength = 0.0;      // In minutes
float_t deltaH;
bgEnum background = NigthLigth;

struct repeating_timer oneMinuteTimer;
struct repeating_timer paintTimer;

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

rgb_t sunshine[] = {
    {255, 102, 25},
    {255, 255, 102}};

typedef struct bgscreen_type
{
    uint8_t bglight;
    int32_t sunPosH;
    int32_t sunPosV;
    rgb_t bgColor;
    rgb_t sunColor;
    bool upDn;
} bgscreen;

bgscreen bground;
bgscreen currBground;

void update_bground_target(uint32_t hh, uint32_t mm)
{

    // Depending on the hour, we decide to have one or more windows
    // During the night low ligth

    // Let's rewrite it properly
    uint32_t theTime = hh * 60 + mm;
    theTime = theTime / 60;

    if (theTime < (sunrise - 1))
    {
        bground.bglight = 128;
        bground.bgColor = colors[0];
        bground.sunColor = sunshine[0];
        background = NigthLigth;
    }
    else
    {
        if (theTime < sunrise)
        {
            bground.bglight = 128;
            bground.bgColor = colors[10];
            bground.sunColor = sunshine[0];
            background = NigthLigth;
        }
        else
        {
            if (theTime < (sunrise + 1))
            {
                bground.bglight = 255;
                bground.bgColor = colors[10];
                bground.sunColor = sunshine[1];
                bground.sunPosH = ((((hh - sunrise) * 60 + mm) - (SUNSIZE / 2)) * deltaH);
                background = DayLight;
            }
            else
            {
                if (theTime < (sunset - 1))
                {
                    bground.bglight = 255;
                    bground.bgColor = colors[10];
                    bground.sunColor = sunshine[1];
                    bground.sunPosH = ((((hh - sunrise) * 60 + mm) - (SUNSIZE / 2)) * deltaH);
                    background = DayLight;
                }
                else
                {
                    if (theTime < (sunset))
                    {
                        bground.bglight = 128;
                        bground.sunColor = sunshine[0];
                        bground.sunPosH = ((((hh - sunrise) * 60 + mm) - (SUNSIZE / 2)) * deltaH);
                        bground.bgColor = colors[10];
                        background = DayLight;
                    }
                    else
                    {
                        if (theTime < (sunset + 1))
                        {

                            bground.bglight = 128;
                            bground.sunColor = sunshine[0];
                            bground.bgColor = colors[0];
                            background = NigthLigth;
                        }
                        else
                        {
                            bground.bglight = 128;
                            bground.bgColor = colors[0];
                            bground.sunColor = sunshine[0];
                            background = NigthLigth;
                        }
                    }
                }
            }
        }
    }

    currBground.sunPosV = bground.sunPosV;
    currBground.sunPosH = bground.sunPosH;
    bground.upDn = true;
}

// paintCallback does change the background color smoothly
bool paintCallback(struct repeating_timer *rt)
{
    // Let's start with the brightness
    if (bground.bglight != currBground.bglight)
    {
        currBground.bglight = (bground.bglight > currBground.bglight) ? currBground.bglight + 1 : currBground.bglight - 1;
    }
    // Let's continue with colors, here we need to prioritize red at sunrise and sunset
    if (bground.bgColor.r != currBground.bgColor.r)
    {
        currBground.bgColor.r = (bground.bgColor.r > currBground.bgColor.r) ? currBground.bgColor.r + 1 : currBground.bgColor.r - 1;
    }
    else
    {
        if (bground.bgColor.b != currBground.bgColor.b)
        {
            currBground.bgColor.b = (bground.bgColor.b > currBground.bgColor.b) ? currBground.bgColor.b + 1 : currBground.bgColor.b - 1;
        }
        else
        {
            if (bground.bgColor.g != currBground.bgColor.g)
            {
                currBground.bgColor.g = (bground.bgColor.g > currBground.bgColor.g) ? currBground.bgColor.g + 1 : currBground.bgColor.g - 1;
            }
        }
    }

    // Let's also properly set the sun
    if (bground.sunColor.r != currBground.sunColor.r)
    {
        currBground.sunColor.r = (bground.sunColor.r > currBground.sunColor.r) ? currBground.sunColor.r + 1 : currBground.sunColor.r - 1;
    }
    else
    {
        if (bground.sunColor.b != currBground.sunColor.b)
        {
            currBground.sunColor.b = (bground.sunColor.b > currBground.sunColor.b) ? currBground.sunColor.b + 1 : currBground.sunColor.b - 1;
        }
        else
        {
            if (bground.sunColor.g != currBground.sunColor.g)
            {
                currBground.sunColor.g = (bground.sunColor.g > currBground.sunColor.g) ? currBground.sunColor.g + 1 : currBground.sunColor.g - 1;
            }
        }
    }

    return true;
}

bool oneMinuteCallback(struct repeating_timer *rt)
{
    update_bground_target(t.hour, t.min);
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

    dayLength = (sunset - sunrise) * 60.0;
    deltaH = (320.0 + SUNSIZE) / dayLength;

    for (int i = 0; i < OBJECTS; i++)
    {
        pt shape;

        shape.x = rand() % (pico_display.bounds.w);
        shape.y = rand() % w2dwn;
        shape.r = (rand() % 10) + 3;
        shape.dx = float(rand() % 255) / 64.0f;
        shape.dy = float(rand() % 255) / 64.0f;
        shape.pen =
            pico_display.create_pen(rand() % 255, rand() % 255, rand() % 255);
        shapes.push_back(shape);
        shape.upDn = true;
        shape.shine = float(rand() % 255) / 64.0f;
    }

    bground.bglight = 128;
    bground.sunColor = sunshine[0];
    bground.sunPosH = 0;
    bground.sunPosV = 0;

    update_bground_target(t.hour, t.min);

    currBground.bglight = bground.bglight;
    currBground.sunPosH = bground.sunPosH;
    currBground.sunPosV = bground.sunPosV;
    currBground.sunColor = bground.sunColor;
    currBground.bgColor.r = bground.bgColor.r;
    currBground.bgColor.g = bground.bgColor.g;
    currBground.bgColor.b = bground.bgColor.b;

    add_repeating_timer_ms(ONEMINUTE, oneMinuteCallback, NULL, &oneMinuteTimer);
    add_repeating_timer_ms(PAINT, paintCallback, NULL, &paintTimer);
}

void draw_background(void)
{
    static uint16_t counter;

    pico_display.set_backlight(currBground.bglight); // was 255

    pico_display.set_pen(currBground.bgColor.r, currBground.bgColor.g, currBground.bgColor.b);

    pico_display.clear();

    counter++;

    if (background == DayLight)
    {
        static float_t rlen = SUNSIZE;

        int r1, r2, r3, r4, r5, r6, r7, r8;

        if (bground.upDn == true)
            rlen += 0.1;
        else
            rlen -= 0.1;

        if (rlen > (SUNSIZE + SUNSIZE))
            bground.upDn = false;

        if (rlen < SUNSIZE)
            bground.upDn = true;

        r1 = rlen + SUNSIZE / 4 ;
        r3 = rlen;
        r2 = (SUNSIZE + SUNSIZE) - rlen;

        pico_display.set_pen(255, 255, 0); // Shining yellow

        // Plot the rays
        // r1
        pico_display.line(Point(currBground.sunPosH - r1, currBground.sunPosV),
                          Point(currBground.sunPosH + r1, currBground.sunPosV));
        pico_display.line(Point(currBground.sunPosH, currBground.sunPosV - r1),
                          Point(currBground.sunPosH, currBground.sunPosV + r1));
        // r2
        pico_display.line(Point(currBground.sunPosH - r2, currBground.sunPosV - r2/4),
                          Point(currBground.sunPosH + r2, currBground.sunPosV + r2/4));
        pico_display.line(Point(currBground.sunPosH + r2/4, currBground.sunPosV - r2),
                          Point(currBground.sunPosH - r2/4, currBground.sunPosV + r2));

        // r3
        pico_display.line(Point(currBground.sunPosH - r3 / 2, currBground.sunPosV - r3 / 2),
                          Point(currBground.sunPosH + r3 / 2, currBground.sunPosV + r3 / 2));
        pico_display.line(Point(currBground.sunPosH + r3 / 2, currBground.sunPosV - r3 / 2),
                          Point(currBground.sunPosH - r3 / 2, currBground.sunPosV + r3 / 2));

        // r4
        pico_display.line(Point(currBground.sunPosH - r2/4, currBground.sunPosV - r2/4),
                          Point(currBground.sunPosH + r2/4, currBground.sunPosV + r2/4));
        pico_display.line(Point(currBground.sunPosH + r2/4, currBground.sunPosV - r2/4),
                          Point(currBground.sunPosH - r2/4, currBground.sunPosV + r2/4));

        // r5
        pico_display.line(Point(currBground.sunPosH - r1, currBground.sunPosV - r1),
                          Point(currBground.sunPosH + r1, currBground.sunPosV + r1));
        pico_display.line(Point(currBground.sunPosH + r1, currBground.sunPosV - r1),
                          Point(currBground.sunPosH - r1, currBground.sunPosV + r1));

        // r7
        pico_display.line(Point(currBground.sunPosH + r1 / 2, currBground.sunPosV + r1 / 2),
                          Point(currBground.sunPosH - r1 / 2, currBground.sunPosV - r1 / 2));
        pico_display.line(Point(currBground.sunPosH - r1 / 2, currBground.sunPosV + r1 / 2),
                          Point(currBground.sunPosH + r1 / 2, currBground.sunPosV - r1 / 2));

        pico_display.set_pen(currBground.sunColor.r, currBground.sunColor.g, currBground.sunColor.b);
        pico_display.circle(Point(currBground.sunPosH, currBground.sunPosV), SUNSIZE);
    }

    if (background == NigthLigth)
    {
        for (auto &shape : shapes)
        {
            // Let's slowly move the Nigth from right to left
            shape.x -= 6 / 3600.0;

            if (shape.upDn == true)
                shape.shine += (shape.dx * 0.1);
            else
                shape.shine -= (shape.dx * 0.1);

            if (abs(shape.shine) > 5.0f)
                shape.upDn = (shape.upDn == true) ? false : true;

            int cross = int(abs(shape.shine));
            int diag = cross / 2;

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