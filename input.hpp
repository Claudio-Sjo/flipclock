// filemane input.hpp
// this file contains the lowlevel interface for input data from display  
#ifndef INPUT_HPP_HEADERFILE
#define INPUT_HPP_HEADERFILE

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

// Prototypes
bool Debounce(struct repeating_timer *t);
Key  ReadInput(void);

#endif