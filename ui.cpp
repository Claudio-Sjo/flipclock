// Filename ui.cpp
#include "clock.hpp"
#include "fonts/bitmap_db.h"
#include "fonts/lowfontgen.h"
#include "input.hpp"
#include "main.hpp"
#include "pico/stdlib.h"
#include "pico_display_2.hpp"
#include <cstdlib>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <vector>

extern int dState;
extern int sState;

void menuHandler(Key pressed)
{
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
                if (++t.hour > 23)
                    t.hour = 0;
                break;
            case Minutes:
                if (++t.min > 59)
                    t.min = 0;
                break;
            case Day:
            {
                switch (t.month)
                {
                case November:
                case April:
                case June:
                case September:
                    maxday = 30;
                    break;
                case February:
                    maxday = (t.year % 4) ? 28 : 29;
                    break;
                }
                if (++t.day > maxday)
                    t.day = 1;
                t.dotw = (t.day - 1) % 7;
            }
            break;
            case Month:
                if (++t.month > December)
                    t.month = January;
                break;
            case Year:
                if (++t.year > 2100)
                    t.year = 2022;
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
                if (t.hour == 0)
                    t.hour = 23;
                else
                    t.hour--;
                break;
            case Minutes:
                if (t.min == 0)
                    t.min = 59;
                else
                    t.min--;
                break;
            case Day:
                switch (t.month)
                {
                case November:
                case April:
                case June:
                case September:
                    maxday = 30;
                    break;
                case February:
                    maxday = (t.year % 4) ? 28 : 29;
                    break;
                }
                if (t.day == 0)
                    t.day = maxday;
                else
                    t.day--;
                break;
            case Month:
                if (t.month == January)
                    t.month = December;
                else
                    t.month--;
                break;
            case Year:
                if (t.year == 2022)
                    t.year = 2100;
                else
                    t.year--;
                break;
            }
        }
    }
}