#ifndef MAIN_HPP_HEADERFILE
#define MAIN_HPP_HEADERFILE

#include <cstdlib>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <vector>


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

typedef enum backGround
{
    Balloons,
    Stars
} bgEnum;


#endif