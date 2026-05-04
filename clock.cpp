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
            if (isEuropeanDST(&t))
            {
                if (++t.hour > 23)
                {
                    t.hour = 0;
                    // Day rollover not critical for display purposes
                }
            }
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

// European DST: starts last Sunday of March at 02:00, ends last Sunday of October at 03:00
// RTC stores winter time (CET). Returns true if summer time (CEST) applies.
bool isEuropeanDST(const datetime_t *dt)
{
    if (dt->month > March && dt->month < October)
        return true;
    if (dt->month < March || dt->month > October)
        return false;

    // Find last Sunday of the month: day 31 (March) or 31 (October) minus offset
    int lastDay = 31; // Both March and October have 31 days
    // Compute day-of-week of the 31st from current date's dotw
    // dotw: 0=Sunday
    int dow31    = (dt->dotw + (lastDay - dt->day)) % 7;
    int lastSun  = lastDay - dow31;

    if (dt->month == March)
    {
        // DST starts at 02:00 on last Sunday of March
        if (dt->day > lastSun)
            return true;
        if (dt->day == lastSun && dt->hour >= 2)
            return true;
        return false;
    }
    else // October
    {
        // DST ends at 03:00 (summer time) on last Sunday of October
        // Since RTC is in winter time, that's 02:00 winter time
        if (dt->day > lastSun)
            return false;
        if (dt->day == lastSun && dt->hour >= 2)
            return false;
        return true;
    }
}
