#ifndef MAIN_HPP_HEADERFILE
#define MAIN_HPP_HEADERFILE

#include <cstdlib>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <vector>

struct pt
{
    float    x;
    float    y;
    uint8_t  r;
    float    dx;
    float    dy;
    uint16_t pen;
};

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
    Sunday,
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday
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

typedef enum backGround
{
    Balloons,
    Stars
} bgEnum;

#endif