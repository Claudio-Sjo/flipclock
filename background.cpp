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

// European capital cities location table
typedef struct {
    const char *name;
    float_t lat;
    float_t lng;
} location_t;

static const location_t locations[] = {
    {"Amsterdam",  52.37f,   4.90f},
    {"Andorra",    42.51f,   1.52f},
    {"Athens",     37.98f,  23.73f},
    {"Belgrade",   44.79f,  20.47f},
    {"Berlin",     52.52f,  13.41f},
    {"Bern",       46.95f,   7.45f},
    {"Bratislava", 48.15f,  17.11f},
    {"Brussels",   50.85f,   4.35f},
    {"Bucharest",  44.43f,  26.10f},
    {"Budapest",   47.50f,  19.04f},
    {"Chisinau",   47.01f,  28.86f},
    {"Copenhagen", 55.68f,  12.57f},
    {"Dublin",     53.35f,  -6.26f},
    {"Helsinki",   60.17f,  24.94f},
    {"Kyiv",       50.45f,  30.52f},
    {"Lisbon",     38.72f,  -9.14f},
    {"Ljubljana",  46.06f,  14.51f},
    {"London",     51.51f,  -0.13f},
    {"Luxembourg", 49.61f,   6.13f},
    {"Madrid",     40.42f,  -3.70f},
    {"Minsk",      53.90f,  27.57f},
    {"Monaco",     43.73f,   7.42f},
    {"Moscow",     55.76f,  37.62f},
    {"Nicosia",    35.19f,  33.38f},
    {"Oslo",       59.91f,  10.75f},
    {"Paris",      48.86f,   2.35f},
    {"Podgorica",  42.44f,  19.26f},
    {"Prague",     50.08f,  14.42f},
    {"Reykjavik",  64.15f, -21.94f},
    {"Riga",       56.95f,  24.11f},
    {"Rome",       41.90f,  12.50f},
    {"San Marino", 43.94f,  12.46f},
    {"Sarajevo",   43.86f,  18.41f},
    {"Skopje",     42.00f,  21.43f},
    {"Sofia",      42.70f,  23.32f},
    {"Stockholm",  59.33f,  18.07f},
    {"Tallinn",    59.44f,  24.75f},
    {"Tirana",     41.33f,  19.82f},
    {"Vaduz",      47.14f,   9.52f},
    {"Valletta",   35.90f,  14.51f},
    {"Vatican",    41.90f,  12.45f},
    {"Vienna",     48.21f,  16.37f},
    {"Vilnius",    54.69f,  25.28f},
    {"Warsaw",     52.23f,  21.01f},
    {"Zagreb",     45.81f,  15.98f},
};

#define NUM_LOCATIONS (sizeof(locations) / sizeof(locations[0]))
#define DEFAULT_LOCATION 30 // Rome

int currentLocation = DEFAULT_LOCATION;

#define UTC_OFFSET   1

extern float_t calculateSunrise(int32_t year, int32_t month, int32_t day, float_t lat, float_t lng, int32_t localOffset, int32_t daylightSavings);
extern float_t calculateSunset(int32_t year, int32_t month, int32_t day, float_t lat, float_t lng, int32_t localOffset, int32_t daylightSavings);

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

uint32_t sunrise = 6;  // recalculated from location
uint32_t sunset  = 19; // recalculated from location
float_t dayLength = 0.0;      // In minutes
float_t deltaH;
bgEnum background = NigthLigth;

struct repeating_timer oneMinuteTimer;
struct repeating_timer paintTimer;

typedef struct sunray_type
{
    float_t x;
    float_t y;
} sunray_t;

sunray_t rays[] = {
    {1.0, 0.0},   // Angle = 0
    {0.92, 0.38}, // Angle = 22.5
    {0.70, 0.70}, // Angle = 45
    {0.38, 0.92}, // Angle = 67.5
};

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

static void recalcSunTimes(void)
{
    int dst = isEuropeanDST(&t) ? 1 : 0;
    float_t lat = locations[currentLocation].lat;
    float_t lng = locations[currentLocation].lng;
    float_t sr = calculateSunrise(t.year, t.month, t.day, lat, lng, UTC_OFFSET, dst);
    float_t ss = calculateSunset(t.year, t.month, t.day, lat, lng, UTC_OFFSET, dst);
    sunrise = (uint32_t)sr;
    sunset  = (uint32_t)ss;
    dayLength = (sunset - sunrise) * 60.0;
    deltaH = (320.0 + SUNSIZE) / dayLength;
}

int getNumLocations(void) { return NUM_LOCATIONS; }
const char *getLocationName(int idx) { return locations[idx].name; }

bool oneMinuteCallback(struct repeating_timer *rt)
{
    recalcSunTimes();
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

    recalcSunTimes();

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
        static float_t rlen = 0;

        int r1, r2, rs;

        if (bground.upDn == true)
            rlen += 0.1;
        else
            rlen -= 0.1;

        if (rlen > (SUNSIZE))
            bground.upDn = false;

        if (rlen < 0.0)
            bground.upDn = true;

        r1 = rlen + SUNSIZE + SUNSIZE / 2;
        r2 = (SUNSIZE + SUNSIZE + SUNSIZE / 2) - rlen;

        pico_display.set_pen(255, 255, 0); // Shining yellow

        // Plot the rays
        for (int i = 0; i < 4; i++)
        {
            rs = r1;
            if (i % 2)
                rs = r2;

            pico_display.line(Point(currBground.sunPosH - rs * rays[i].x, currBground.sunPosV - rs * rays[i].y),
                              Point(currBground.sunPosH + rs * rays[i].x, currBground.sunPosV + rs * rays[i].y));
            pico_display.line(Point(currBground.sunPosH - rs * rays[i].y, currBground.sunPosV + rs * rays[i].x),
                              Point(currBground.sunPosH + rs * rays[i].y, currBground.sunPosV - rs * rays[i].x));
        }

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