// filemane input.cpp
// this file contains the lowlevel interface for input data from display  

#include "pico/critical_section.h"
#include "pico/stdlib.h"
#include "pico_display_2.hpp"
#include <cstdlib>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <vector>

using namespace pimoroni;

extern PicoDisplay2 pico_display(buffer);

Key inputRing[MAXQUEUE];

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
