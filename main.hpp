#ifndef MAIN_HPP_HEADERFILE
#define MAIN_HPP_HEADERFILE

#include <cstdlib>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#define BUTTONS 4 // only 4 buttons on the pico display

#define MAXQUEUE      32
#define CLICKDEBOUNCE 30000   /* 30ms for debouncing   */
#define CLICKLONG     1000000 /* 1s for long click     */
#define CLICKSHORT    75000   /* 75ms for short click */

typedef enum
{
    NoKey = 0x80,
    // only <128 from UART
    Key_A,
    Key_AL,
    Key_B,
    Key_BL,
    Key_X,
    Key_XL,
    Key_Y,
    Key_YL
} Key;

typedef enum
{
    Idle,
    Debouncing,
    Pressed,
    Waiting
} BtnState;

typedef struct
{
    uint8_t  button; /* GPIO pin */
    bool     pol;    /* polarity: SET active high, RESET active low */
    BtnState state;  /* current state */
    uint32_t tick;   /* time of press */
    Key      kShort; /* key for short press */
    Key      kLong;  /* key for long press */
} Button;

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