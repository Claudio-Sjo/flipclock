#include "main.hpp"
#include "fonts/bitmap_db.h"
#include "fonts/clockFonts.h"
#include "fonts/lowfontgen.h"
#include "pico/critical_section.h"
#include "pico/stdlib.h"
#include "pico_display_2.hpp"
#include <cstdlib>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <vector>

critical_section_t debounce_section;

static volatile uint16_t keyWrite  = 0;
static volatile uint16_t keyRead   = 0;
static volatile uint16_t keysReady = 0;

Key inputRing[MAXQUEUE];

using namespace pimoroni;

uint16_t     buffer[PicoDisplay2::WIDTH * PicoDisplay2::HEIGHT];
PicoDisplay2 pico_display(buffer);

bgEnum background = Balloons;

Button buttons[BUTTONS] =
    {

        {.button = pico_display.A,
         .pol    = true,
         .state  = Idle,
         .tick   = 0,
         .kShort = Key_A,
         .kLong  = Key_AL},

        {.button = pico_display.B,
         .pol    = true,
         .state  = Idle,
         .tick   = 0,
         .kShort = Key_B,
         .kLong  = Key_BL},

        {.button = pico_display.X,
         .pol    = true,
         .state  = Idle,
         .tick   = 0,
         .kShort = Key_X,
         .kLong  = Key_XL},

        {.button = pico_display.Y,
         .pol    = true,
         .state  = Idle,
         .tick   = 0,
         .kShort = Key_Y,
         .kLong  = Key_YL}};

static uint_fast16_t curBtn = 0;

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

volatile uint8_t  hours   = 0;
volatile uint8_t  min     = 0;
volatile uint8_t  sec     = 0;
volatile uint8_t  day     = 1;
volatile uint8_t  dayweek = Monday;
volatile uint8_t  month   = January;
volatile uint16_t year    = 2022;

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
        int chheigth = forte_24ptDescriptors[chidx].heightBits;

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

void myPrintLine(Point start, Point end, int r, int g, int b)
{
    Point s = start;
    Point e = end;

    int len       = end.x - start.x;
    int len_a     = abs(len);
    int direction = (len > 0) ? 1 : -1;

    pico_display.set_pen(r, g, b); // Black

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
            pico_display.set_pen(r, g, b); // White
            s.x = s.x + 1 * direction;
            e.x = e.x - 1 * direction;
            pico_display.line(s, e);
        }
        else
        {
            pico_display.set_pen(r, g, b); // White
            s   = start;
            s.x = s.x + 1 * direction;
            pico_display.line(s, s);
        }
    }
}

void printDigit(Point location, uint8_t digit, int r, int g, int b)
{
    int base = digit * 64 * 4;

    Point start = location;
    Point end   = location;

    for (int i = 0; i < 64; i++)
    {
        int ix  = 4 * i;
        start.x = location.x + digitFont[base + ix];
        end.x   = location.x + digitFont[base + ix + 1];
        myPrintLine(start, end, r, g, b);
        if (digitFont[base + ix] != digitFont[base + ix + 2])
        {
            start.x = location.x + digitFont[base + ix + 2];
            end.x   = location.x + digitFont[base + ix + 3];
            myPrintLine(start, end, r, g, b);
        }
        start.y = start.y + 1;
        end.y   = end.y + 1;
    }
}

void mergeDigitPrint(Point location, uint8_t before, uint8_t after, uint8_t sk, int r, int g, int b)
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
        myPrintLine(start, end, r, g, b);
        step    = (digitFont[baseAf + ix + 2] - digitFont[baseBe + ix + 2]);
        offset  = (step * sk) / 10;
        start.x = location.x + digitFont[baseBe + ix + 2] + offset;
        step    = (digitFont[baseAf + ix + 3] - digitFont[baseBe + ix + 3]);
        offset  = (step * sk) / 10;
        end.x   = location.x + digitFont[baseBe + ix + 3] + offset;
        myPrintLine(start, end, r, g, b);
        start.y = start.y + 1;
        end.y   = end.y + 1;
    }
}

void updateHour(uint8_t hh, uint8_t mm, uint8_t ss)
{
    static uint8_t oldhh, oldmm, oldss;
    Point          digitPoint(5, 20);
    int            r, g, b;

    if ((dState == ClockSetup) && (sState == Hours))
    {
        r = 0;
        g = 255;
        b = 0;
    }
    else
        r = g = b = 255;

    if (oldhh != hh)
    {
        if ((oldhh / 10) != (hh / 10))
            mergeDigitPrint(digitPoint, oldhh / 10, hh / 10, scheduler, r, g, b);
        else
            printDigit(digitPoint, hours / 10, r, g, b);
        digitPoint.x = 50;
        mergeDigitPrint(digitPoint, oldhh % 10, hh % 10, scheduler, r, g, b);
        if (scheduler > 10)
            oldhh = hh;
    }
    else
    {
        printDigit(digitPoint, hours / 10, r, g, b);
        digitPoint.x = 50;
        printDigit(digitPoint, hours % 10, r, g, b);
    }

    if ((dState == ClockSetup) && (sState == Minutes))
    {
        r = 0;
        g = 255;
        b = 0;
    }
    else
        r = g = b = 255;

    digitPoint.x = 115;
    if (oldmm != mm)
    {
        if ((oldmm / 10) != (mm / 10))
            mergeDigitPrint(digitPoint, oldmm / 10, mm / 10, scheduler, r, g, b);
        else
            printDigit(digitPoint, min / 10, r, g, b);
        digitPoint.x = 160;
        mergeDigitPrint(digitPoint, oldmm % 10, mm % 10, scheduler, r, g, b);
        if (scheduler > 10)
            oldmm = mm;
    }
    else
    {
        printDigit(digitPoint, min / 10, r, g, b);
        digitPoint.x = 160;
        printDigit(digitPoint, min % 10, r, g, b);
    }

    r = g = b = 255;

    digitPoint.x = 225;
    if (oldss != ss)
    {
        if ((oldss / 10) != (ss / 10))
            mergeDigitPrint(digitPoint, oldss / 10, ss / 10, scheduler, r, g, b);
        else
            printDigit(digitPoint, sec / 10, r, g, b);
        digitPoint.x = 270;
        mergeDigitPrint(digitPoint, oldss % 10, ss % 10, scheduler, r, g, b);
        if (scheduler > 10)
            oldss = ss;
    }
    else
    {
        printDigit(digitPoint, sec / 10, r, g, b);
        digitPoint.x = 270;
        printDigit(digitPoint, sec % 10, r, g, b);
    }
}

// Debouncing mechanism by Zuk

Key serveButton(Button *b)
{
    Key      key = NoKey;
    uint32_t now = time_us_32();
    bool     btn = b->pol == pico_display.is_pressed(b->button);
    switch (b->state)
    {
    case Idle:
        if (btn)
        {
            /* The button has been depressed */
            /* Remember when */
            b->tick = now;
            /* and move to Debouncing state */
            b->state = Debouncing;
        }
        break;
    case Debouncing:
        if (btn)
        {
            if (now - b->tick > CLICKDEBOUNCE)
                /* Real press */
                b->state = Pressed;
        }
        else
        {
            /* It was a bounce, start over */
            b->state = Idle;
        }
        break;
    case Pressed:
        if (btn)
        { /* Still pressed, is this a long press? */
            if (now - b->tick > CLICKLONG)
            {
                /* A long press has been detected */
                key = b->kLong;
                /* Wait for release */
                b->state = Waiting;
            }
        }
        else
        { /* Released, is it a short press? */
            if (now - b->tick > CLICKSHORT)
            {
                /* A short press has been detected */
                key = b->kShort;
            }
            /* Return to idle */
            b->state = Idle;
        }
        break;
    case Waiting:
        if (!btn)
        {
            b->state = Idle;
        }
        break;
    }
    return key;
}

Key ReadInput(void)
{
    Key key = NoKey;
    if (keysReady)
    {
        /* Read oldest key and move pointer */
        key = inputRing[keyRead++];
        /* Normalize to ring size */
        keyRead %= MAXQUEUE;
        /* Make slot available */
        keysReady--;
    }
    return key;
}

void queueKey(Key key)
{
    if (key == NoKey)
        return;
    // valid char received, enqueue it!
    sprintf(mainString, "valid key : %d", key);
    critical_section_enter_blocking(&debounce_section);
    /* Space left on input ring? */
    if (keysReady < MAXQUEUE)
    {
        inputRing[keyWrite++] = key;
        keyWrite %= MAXQUEUE; // pointer wrap around
        keysReady++;
    }
    critical_section_exit(&debounce_section);
}

bool Debounce(struct repeating_timer *t)
{
    Key key = serveButton(&buttons[curBtn]);
    if (++curBtn == BUTTONS)
        curBtn = 0;
    /* Did we find anything to do? */
    queueKey(key);

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
#define OBJECTS 50
    std::vector<pt> shapes;
    for (int i = 0; i < OBJECTS; i++)
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

    // Debouncing Timer
    add_repeating_timer_ms(2, Debounce, NULL, &debounceTimer);

    add_repeating_timer_ms(50, oneTwenthCallback, NULL, &oneTwenthtimer);

    while (true)
    {
        Key pressed = ReadInput();

        if (pressed == Key_A)
        {
            if (dState == ClockSetup)
            {
                dState = Clock;
                sState = Hours;
            }
            else
                dState = ClockSetup;
        } // key A
        if (pressed == Key_B)
        {
            if (dState == ClockSetup)
            {
                if (++sState > Year)
                    sState = Hours;
            }
        }
        if (pressed == Key_X)
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
                case Day:
                {
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
        if (pressed == Key_Y)
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
            pico_display.set_pen(0, 0, 100); // Dark Blue
            pico_display.clear();

            for (auto &shape : shapes)
            {
                if (background == Balloons)
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
                if (background == Stars)
                {
                    pico_display.set_pen(0,255,255); // Shining yellow
                    pico_display.line(Point(shape.x - 5, shape.y), Point(shape.x + 5, shape.y));
                    pico_display.line(Point(shape.x, shape.y - 5), Point(shape.x, shape.y + 5));
                    pico_display.line(Point(shape.x - 3, shape.y - 3), Point(shape.x + 3, shape.y +3));
                    pico_display.line(Point(shape.x - 3, shape.y + 3), Point(shape.x + 3, shape.y - 3));
                    pico_display.circle(Point(shape.x, shape.y), 2);
                }
            }

            updateHour(hours, min, sec);

            // Since HSV takes a float from 0.0 to 1.0 indicating hue,
            // then we can divide millis by the number of milliseconds
            // we want a full colour cycle to take. 5000 = 5 sec.
            uint8_t r = 0, g = 0, b = 0;
            from_hsv((float)millis() / 5000.0f, 1.0f, 0.5f + sinf(millis() / 100.0f / 3.14159f) * 0.5f, r, g, b);
            pico_display.set_led(r, g, b);

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
                pico_display.set_pen(255, 255, 255);
            myPrintLowFont(Point(5, 120), dayStr);

            if ((dState == ClockSetup) && (sState == Month))
                pico_display.set_pen(0, 255, 0);
            else
                pico_display.set_pen(255, 255, 255);
            myPrintLowFont(Point(160, 120), montStr);
            if (dayweek == Sunday)
                pico_display.set_pen(255, 0, 0); // Red
            else
                pico_display.set_pen(255, 255, 255);
            myPrintLowFont(Point(5, 160), dowStr);
            if ((dState == ClockSetup) && (sState == Year))
                pico_display.set_pen(0, 255, 0);
            else
                pico_display.set_pen(255, 255, 255);
            myPrintLowFont(Point(160, 160), yearStr);

            if (dState == ClockSetup)
            {
                pico_display.set_pen(0, 255, 0); // green
                myPrintLowFont(Point(100, 200), "Setup");
            }

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
