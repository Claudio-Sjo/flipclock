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
}