// Filename clock.cpp
#include "DS3231_HAL/DS3231_HAL.h"
#include "fonts/bitmap_db.h"
#include "fonts/lowfontgen.h"
#include "hardware/rtc.h"
#include "input.hpp"
#include "main.hpp"
#include "output.hpp"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"
#include "pico_display_2.hpp"
#include <cstdlib>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <vector>

using namespace pimoroni;
extern int              dState;
extern int              sState;
extern volatile uint8_t scheduler;

/*
volatile uint8_t  hours   = 0;
volatile uint8_t  min     = 0;
volatile uint8_t  sec     = 0;
volatile uint8_t  day     = 1;
volatile uint8_t  dayweek = Monday;
volatile uint8_t  month   = January;
volatile uint16_t year    = 2022;
*/

datetime_t t = {
    .year  = 2020,
    .month = 06,
    .day   = 05,
    .dotw  = 5, // 0 is Sunday, so 5 is Friday
    .hour  = 15,
    .min   = 45,
    .sec   = 00};

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
            printDigit(digitPoint, t.hour / 10, r, g, b);
        digitPoint.x = 50;
        mergeDigitPrint(digitPoint, oldhh % 10, hh % 10, scheduler, r, g, b);
        if (scheduler > 10)
            oldhh = hh;
    }
    else
    {
        printDigit(digitPoint, t.hour / 10, r, g, b);
        digitPoint.x = 50;
        printDigit(digitPoint, t.hour % 10, r, g, b);
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
            printDigit(digitPoint, t.min / 10, r, g, b);
        digitPoint.x = 160;
        mergeDigitPrint(digitPoint, oldmm % 10, mm % 10, scheduler, r, g, b);
        if (scheduler > 10)
            oldmm = mm;
    }
    else
    {
        printDigit(digitPoint, t.min / 10, r, g, b);
        digitPoint.x = 160;
        printDigit(digitPoint, t.min % 10, r, g, b);
    }

    r = g = b = 255;

    digitPoint.x = 225;
    if (oldss != ss)
    {
        if ((oldss / 10) != (ss / 10))
            mergeDigitPrint(digitPoint, oldss / 10, ss / 10, scheduler, r, g, b);
        else
            printDigit(digitPoint, t.sec / 10, r, g, b);
        digitPoint.x = 270;
        mergeDigitPrint(digitPoint, oldss % 10, ss % 10, scheduler, r, g, b);
        if (scheduler > 10)
            oldss = ss;
    }
    else
    {
        printDigit(digitPoint, t.sec / 10, r, g, b);
        digitPoint.x = 270;
        printDigit(digitPoint, t.sec % 10, r, g, b);
    }
}

bool oneTwenthCallback(struct repeating_timer *rt)
{
    static int oldState = Clock;

    if (++scheduler >= 20)
    {
        if (dState == ClockSetup)
        {
            t.sec = 0;
        }
        else
        {
            rtc_get_datetime(&t);
        }
        scheduler = 0;
    }
    if ((oldState == ClockSetup) && (dState == Clock))
    {
        rtc_set_datetime(&t);
        SetRtcTime(&t);
    }
    oldState = dState;

    return true;
}
