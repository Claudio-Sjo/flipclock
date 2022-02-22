#include "pico/stdlib.h"
#include <cstdlib>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "fonts/bitmap_db.h"
#include "fonts/clockFonts.h"
#include "fonts/lowfontgen.h"
#include "pico_display_2.hpp"

using namespace pimoroni;

uint16_t     buffer[PicoDisplay2::WIDTH * PicoDisplay2::HEIGHT];
PicoDisplay2 pico_display(buffer);

// We need a state machine for properly handling the display
enum displayState
{
    Clock,
    ClockSetup,
    DateSetup,
    AlarmSetup,
};

enum setupState
{
    Hours,
    Minutes,
    Day,
    Month,
    Year
};

enum dayOfWeek
{
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    Sunday
};

enum monthOfYear
{
    January,
    February,
    March,
    April,
    May,
    June,
    July,
    August,
    September,
    October,
    November,
    December
};

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

volatile uint8_t          hours   = 0;
volatile uint8_t          min     = 0;
volatile uint8_t          sec     = 0;
volatile uint8_t          day     = 1;
volatile uint8_t   dayweek = Monday;
volatile uint8_t month   = January;
volatile uint16_t         year    = 2022;

volatile uint8_t scheduler = 0;

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

// HSV Conversion expects float inputs in the range of 0.00-1.00 for each channel
// Outputs are rgb in the range 0-255 for each channel
void from_hsv(float h, float s, float v, uint8_t &r, uint8_t &g, uint8_t &b)
{
    float i = floor(h * 6.0f);
    float f = h * 6.0f - i;
    v *= 255.0f;
    uint8_t p = v * (1.0f - s);
    uint8_t q = v * (1.0f - f * s);
    uint8_t t = v * (1.0f - (1.0f - f) * s);

    switch (int(i) % 6)
    {
    case 0:
        r = v;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = v;
        b = p;
        break;
    case 2:
        r = p;
        g = v;
        b = t;
        break;
    case 3:
        r = p;
        g = q;
        b = v;
        break;
    case 4:
        r = t;
        g = p;
        b = v;
        break;
    case 5:
        r = v;
        g = p;
        b = q;
        break;
    }
}

void myPrintLowFont(Point location, const std::string str)
{
    int x = location.x;
    int y = location.y;
    int ix;
    int cx      = 0;
    int cy      = 0;
    int charpos = 0;

    if (str.size() > 20)
        return;

    for (ix = 0; ix < str.size(); ix++)
    {
        // Search for the char
        int chidx    = str[ix];
        chidx        = chidx - forte_24ptFontInfo.startChar;
        int chsize   = forte_24ptDescriptors[chidx].widthBits;
        int choffset = forte_24ptDescriptors[chidx].offset;
        int chheigth = forte_24ptDescriptors[chidx].heightBytes;

        if (chsize != 0)
        {
            for (int hh = 0; hh < chheigth; hh++)
            {
                int byteOfs = (hh * ((chsize + 7) / 8));

                for (int chlen = 0; chlen < chsize; chlen++)
                {
                    int pbyte   = forte_24ptBitmaps[byteOfs + choffset + (chlen / 8)];
                    int chshift = chlen % 8;
                    if ((pbyte << chshift) & 0x80)
                        pico_display.pixel(Point(x + cx, y + cy));
                    cx++;
                }
                cx = charpos;
                cy++;
            }
        }
        charpos += chsize + 2;
        cx = charpos;
        cy = 0;
    }
}

void myPrintLine(Point start, Point end)
{
    Point s = start;
    Point e = end;

    int len       = end.x - start.x;
    int len_a     = abs(len);
    int direction = (len > 0) ? 1 : -1;

    pico_display.set_pen(0, 0, 0); // Black

    if (len_a < 3)
    {
        pico_display.line(start, end);
    }
    else
    {
        s.x = s.x + 1 * direction;
        e.x = e.x - 1 * direction;
        pico_display.line(s, e);

        if (len_a >= 4)
        {
            pico_display.set_pen(255, 255, 255); // White
            s.x = s.x + 1 * direction;
            e.x = e.x - 1 * direction;
            pico_display.line(s, e);
        }
        else
        {
            pico_display.set_pen(255, 255, 255); // White
            s   = start;
            s.x = s.x + 1 * direction;
            pico_display.line(s, s);
        }
    }
}

void printDigit(Point location, uint8_t digit)
{
    int base = digit * 64 * 4;

    Point start = location;
    Point end   = location;

    for (int i = 0; i < 64; i++)
    {
        int ix  = 4 * i;
        start.x = location.x + digitFont[base + ix];
        end.x   = location.x + digitFont[base + ix + 1];
        myPrintLine(start, end);
        if (digitFont[base + ix] != digitFont[base + ix + 2])
        {
            start.x = location.x + digitFont[base + ix + 2];
            end.x   = location.x + digitFont[base + ix + 3];
            myPrintLine(start, end);
        }
        start.y = start.y + 1;
        end.y   = end.y + 1;
    }
}

void mergeDigitPrint(Point location, uint8_t before, uint8_t after, uint8_t sk)
{
    int baseBe = before * 64 * 4;
    int baseAf = after * 64 * 4;

    Point start = location;
    Point end   = location;

    for (int i = 0; i < 64; i++)
    {
        int ix     = 4 * i;
        int step   = (digitFont[baseAf + ix] - digitFont[baseBe + ix]);
        int offset = (step * sk) / 10;
        start.x    = location.x + digitFont[baseBe + ix] + offset;
        step       = (digitFont[baseAf + ix + 1] - digitFont[baseBe + ix + 1]);
        offset     = (step * sk) / 10;
        end.x      = location.x + digitFont[baseBe + ix + 1] + offset;
        myPrintLine(start, end);
        step    = (digitFont[baseAf + ix + 2] - digitFont[baseBe + ix + 2]);
        offset  = (step * sk) / 10;
        start.x = location.x + digitFont[baseBe + ix + 2] + offset;
        step    = (digitFont[baseAf + ix + 3] - digitFont[baseBe + ix + 3]);
        offset  = (step * sk) / 10;
        end.x   = location.x + digitFont[baseBe + ix + 3] + offset;
        myPrintLine(start, end);
        start.y = start.y + 1;
        end.y   = end.y + 1;
    }
}

void updateHour(uint8_t hh, uint8_t mm, uint8_t ss)
{
    static uint8_t oldhh, oldmm, oldss;
    Point          digitPoint(5, 20);

    if ((dState == ClockSetup) && (sState == Hours))
        pico_display.set_pen(0, 255, 0);
    else
        pico_display.set_pen(255, 255, 255);

    if (oldhh != hh)
    {
        if ((oldhh / 10) != (hh / 10))
            mergeDigitPrint(digitPoint, oldhh / 10, hh / 10, scheduler);
        else
            printDigit(digitPoint, hours / 10);
        digitPoint.x = 50;
        mergeDigitPrint(digitPoint, oldhh % 10, hh % 10, scheduler);
        if (scheduler > 10)
            oldhh = hh;
    }
    else
    {
        printDigit(digitPoint, hours / 10);
        digitPoint.x = 50;
        printDigit(digitPoint, hours % 10);
    }

    if ((dState == ClockSetup) && (sState == Minutes))
        pico_display.set_pen(0, 255, 0);
    else
        pico_display.set_pen(255, 255, 255);

    digitPoint.x = 115;
    if (oldmm != mm)
    {
        if ((oldmm / 10) != (mm / 10))
            mergeDigitPrint(digitPoint, oldmm / 10, mm / 10, scheduler);
        else
            printDigit(digitPoint, min / 10);
        digitPoint.x = 160;
        mergeDigitPrint(digitPoint, oldmm % 10, mm % 10, scheduler);
        if (scheduler > 10)
            oldmm = mm;
    }
    else
    {
        printDigit(digitPoint, min / 10);
        digitPoint.x = 160;
        printDigit(digitPoint, min % 10);
    }

    pico_display.set_pen(255, 255, 255);

    digitPoint.x = 225;
    if (oldss != ss)
    {
        if ((oldss / 10) != (ss / 10))
            mergeDigitPrint(digitPoint, oldss / 10, ss / 10, scheduler);
        else
            printDigit(digitPoint, sec / 10);
        digitPoint.x = 270;
        mergeDigitPrint(digitPoint, oldss % 10, ss % 10, scheduler);
        if (scheduler > 10)
            oldss = ss;
    }
    else
    {
        printDigit(digitPoint, sec / 10);
        digitPoint.x = 270;
        printDigit(digitPoint, sec % 10);
    }
}

int main()
{

    static uint8_t oldsk;

    pico_display.init();
    pico_display.set_backlight(255);

    struct repeating_timer oneTwenthtimer;

    struct pt
    {
        float    x;
        float    y;
        uint8_t  r;
        float    dx;
        float    dy;
        uint16_t pen;
    };

    // Strings for printing information
    char dayStr[5];
    char montStr[12];
    char dowStr[12];
    char yearStr[5];

    w1top = 0;
    w1dwn = (pico_display.bounds.h / 3) * 2;
    w2top = w1dwn + 1;
    w2dwn = pico_display.bounds.h;

// It was 100 baloons, but for half screen we change to 50
#define BALOONS 0
    std::vector<pt> shapes;
    for (int i = 0; i < BALOONS; i++)
    {
        pt shape;
        shape.x   = rand() % (pico_display.bounds.w);
        shape.y   = rand() % w1dwn;
        shape.r   = (rand() % 10) + 3;
        shape.dx  = float(rand() % 255) / 64.0f;
        shape.dy  = float(rand() % 255) / 64.0f;
        shape.pen = pico_display.create_pen(rand() % 255, rand() % 255, rand() % 255);
        shapes.push_back(shape);
    }

    Point text_location(pico_display.bounds.w / 4, w2top + ((w2dwn - w2top) / 3));
    Point mainS_location(5, w2top + ((w2dwn - w2top) / 3 + 16));
    Point scrolling_line_b(0, w2top);
    Point scrolling_line_e(pico_display.bounds.w, w2top);

    sprintf(mainString, "********");

    add_repeating_timer_ms(50, oneTwenthCallback, NULL, &oneTwenthtimer);

    while (true)
    {
        if (pico_display.is_pressed(pico_display.A))
        {
            if (dState == ClockSetup)
            {
                dState     = Clock;
                sState = Hours;
            }
            else
                dState = ClockSetup;
        } // key A
        if (pico_display.is_pressed(pico_display.B))
        {
            if (dState == ClockSetup)
            {
                if (++sState > Year)
                    sState = Hours;
            }
        }
        if (pico_display.is_pressed(pico_display.X))
        {
            if (dState == ClockSetup)
            {
                int maxday = 31;

                switch (sState)
                {
                case Hours:
                    if (++hours > 23)
                        hours = 0;
                    break;
                case Minutes:
                    if (++min > 59)
                        min = 0;
                    break;
                case Day:{
                    switch (month)
                    {
                    case November:
                    case April:
                    case June:
                    case September:
                        maxday = 30;
                        break;
                    case February:
                        maxday = (year % 4) ? 28 : 29;
                        break;
                    }
                    if (++day > maxday)
                        day = 1;
                    dayweek = (day - 1) % 7;
                }
                    break;
                case Month:
                    if (++month > December)
                        month = January;
                    break;
                case Year:
                    if (year > 2100)
                        year = 2022;
                    break;
                }
            }
        }
        if (pico_display.is_pressed(pico_display.Y))
        {
            if (dState == ClockSetup)
            {
                int maxday = 31;

                switch (sState)
                {
                case Hours:
                    if (hours == 0)
                        hours = 0;
                    else
                        hours--;
                    break;
                case Minutes:
                    if (min == 0)
                        min = 59;
                    else
                        min--;
                    break;
                case Day:
                    switch (month)
                    {
                    case November:
                    case April:
                    case June:
                    case September:
                        maxday = 30;
                        break;
                    case February:
                        maxday = (year % 4) ? 28 : 29;
                        break;
                    }
                    if (day == 0)
                        day = maxday;
                    else
                        day--;
                    break;
                case Month:
                    if (month == January)
                        month = December;
                    else
                        month--;
                    break;
                case Year:
                    if (year == 2022)
                        year = 2100;
                    else
                        year--;
                    break;
                }
            }
        }
        if (oldsk != scheduler)
        {
            pico_display.set_pen(120, 40, 60);
            pico_display.clear();

            for (auto &shape : shapes)
            {
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

            updateHour(hours, min, sec);

            // Since HSV takes a float from 0.0 to 1.0 indicating hue,
            // then we can divide millis by the number of milliseconds
            // we want a full colour cycle to take. 5000 = 5 sec.
            uint8_t r = 0, g = 0, b = 0;
            from_hsv((float)millis() / 5000.0f, 1.0f, 0.5f + sinf(millis() / 100.0f / 3.14159f) * 0.5f, r, g, b);
            pico_display.set_led(r, g, b);

            // Preparing strings for printing data
            /*
    char dayStr[5];
    char montStr[12];
    char dowStr[12];
    char yearStr[5];
    */

            switch (day)
            {
            case 1:
            case 21:
            case 31:
                sprintf(dayStr, "%dst", day);
                break;
            case 2:
            case 22:
                sprintf(dayStr, "%dnd", day);
                break;
            case 3:
            case 23:
                sprintf(dayStr, "%drd", day);
                break;
            default:
                sprintf(dayStr, "%dth", day);
                break;
            }

            switch (month)
            {
            case January:
                sprintf(montStr, "January");
                break;
            case February:
                sprintf(montStr, "February");
                break;
            case March:
                sprintf(montStr, "March");
                break;
            case April:
                sprintf(montStr, "April");
                break;
            case May:
                sprintf(montStr, "May");
                break;
            case June:
                sprintf(montStr, "June");
                break;
            case July:
                sprintf(montStr, "July");
                break;
            case August:
                sprintf(montStr, "August");
                break;
            case September:
                sprintf(montStr, "September");
                break;
            case October:
                sprintf(montStr, "October");
                break;
            case November:
                sprintf(montStr, "November");
                break;
            case December:
                sprintf(montStr, "December");
                break;
            }

            switch (dayweek)
            {
            case Monday:
                sprintf(dowStr, "Monday");
                break;
            case Tuesday:
                sprintf(dowStr, "Tuesday");
                break;
            case Wednesday:
                sprintf(dowStr, "Wednesday");
                break;
            case Thursday:
                sprintf(dowStr, "Thursday");
                break;
            case Friday:
                sprintf(dowStr, "Friday");
                break;
            case Saturday:
                sprintf(dowStr, "Saturday");
                break;
            case Sunday:
                sprintf(dowStr, "Sunday");
                break;
            }

            sprintf(yearStr, "%d", year);

            if ((dState == ClockSetup) && (sState == Day))
                pico_display.set_pen(0, 255, 0);
            else
                pico_display.set_pen(255, 255, 255); // pico_display.text("Hello World", text_location, 320);
            myPrintLowFont(Point(5, 120), "21st");

            if ((dState == ClockSetup) && (sState == Month))
                pico_display.set_pen(0, 255, 0);
            else
                pico_display.set_pen(255, 255, 255); // pico_display.text("Hello World", text_location, 320);
            myPrintLowFont(Point(160, 120), "February");
            if (dayweek == Sunday)
                pico_display.set_pen(0, 0, 255);
            else
                pico_display.set_pen(255, 255, 255); // pico_display.text("Hello World", text_location, 320);
            myPrintLowFont(Point(5, 160), "Monday");
            if ((dState == ClockSetup) && (sState == Year))
                pico_display.set_pen(0, 255, 0);
            else
                pico_display.set_pen(255, 255, 255); // pico_display.text("Hello World", text_location, 320);
            myPrintLowFont(Point(160, 160), "2022");

            if (dState == ClockSetup)
            {
                pico_display.set_pen(0, 255, 0); // green
                myPrintLowFont(Point(100, 200), "Clock Setup");
            }
            // pico_display.text(mainString, mainS_location, 320);

            // pico_display.line(scrolling_line_b, scrolling_line_e);

            // update screen

            pico_display.update();
            oldsk = scheduler;
        }
    }

    return 0;
}
