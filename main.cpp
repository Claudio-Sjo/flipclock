#include "main.hpp"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <cstdlib>
#include <vector>

#include "background.hpp"
#include "clock.hpp"
#include "fonts/bitmap_db.h"
#include "fonts/lowfontgen.h"
#include "input.hpp"
#include "output.hpp"
#include "pico/critical_section.h"
#include "pico/stdlib.h"
#include "pico_display_2.hpp"
#include "ui.hpp"

#include "DS3231_HAL/DS3231_HAL.h"
#include "MemI2C/memI2C.h"
#include "hardware/clocks.h"
#include "hardware/i2c.h"
#include "hardware/pll.h"
#include "hardware/rtc.h"
#include "hardware/structs/clocks.h"
#include "hardware/structs/pll.h"
#include "pico/util/datetime.h"

critical_section_t debounce_section;

using namespace pimoroni;

extern uint16_t     buffer[];
extern PicoDisplay2 pico_display;

// rtc relates stuffs
extern uint8_t buf[];
extern char   *week[];

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

volatile uint8_t scheduler = 0;

char dayStr[5];
char montStr[12];
char dowStr[12];
char yearStr[5];

int main()
{
    static uint8_t oldsk;
    critical_section_init(&debounce_section);

    pico_display.init();

    struct repeating_timer debounceTimer;

    struct repeating_timer oneTwenthtimer;

    // Strings for printing information

    w1top = 0;
    w1dwn = (pico_display.bounds.h / 3) * 2;
    w2top = w1dwn + 1;
    w2dwn = pico_display.bounds.h;

    Point text_location(pico_display.bounds.w / 4, w2top + ((w2dwn - w2top) / 3));
    Point mainS_location(5, w2top + ((w2dwn - w2top) / 3 + 16));
    Point scrolling_line_b(0, w2top);
    Point scrolling_line_e(pico_display.bounds.w, w2top);

    sprintf(mainString, "********");

    // rtc related part

    char  datetime_buf[256];
    char *datetime_str = &datetime_buf[0];

#define addr      0x68
#define add24lc65 0x50
#define I2C_SCL   5 // GPIO5
#define I2C_SDA   4 // GPIO4

    // gpio_pull_up(I2C_SDA);
    // gpio_pull_up(I2C_SCL);

    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    sprintf(mainString, "mem I2C Test Program ...");

    uint8_t memDataw[] = {0x55, 0xaa};
    uint8_t memDatar[2];

    int rc_w = i2c_write_mem_blocking(i2c0, add24lc65, 0, 2, memDataw, 2);
    sleep_ms(10);
    int rc_r = i2c_read_mem_blocking(i2c0, add24lc65, 0, 2, memDatar, 2);

    // Debouncing Timer
    add_repeating_timer_ms(2, Debounce, NULL, &debounceTimer);

    add_repeating_timer_ms(50, oneTwenthCallback, NULL, &oneTwenthtimer);


    /*
        // Test 32kHz
        gpio_pull_up(22);
        clock_configure_gpin(clk_rtc, 22, 32768, 32768);
    */
    // Test DS3231 Clock
    bool DS3231res = InitRtc(NULL, RtcIntSqwOff, i2c0);
    GetRtcTime(&t);

    rtc_init();
    rtc_set_datetime(&t);

    initialise_bg();

    uint32_t fRead = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);

    while (true)
    {
        menuHandler(ReadInput());

        if (oldsk != scheduler)
        {

            draw_background();

            updateHour(t.hour, t.min, t.sec);

            // Since HSV takes a float from 0.0 to 1.0 indicating hue,
            // then we can divide millis by the number of milliseconds
            // we want a full colour cycle to take. 5000 = 5 sec.
            /*
      uint8_t r = 0, g = 0, b = 0;
      from_hsv((float)millis() / 5000.0f, 1.0f, 0.5f + sinf(millis() / 100.0f
      / 3.14159f) * 0.5f, r, g, b); pico_display.set_led(r, g, b);
      */

            updateDisplay();

            // ds3231ReadTime();
            // buf[0] = buf[0] & 0x7F; // sec
            // buf[1] = buf[1] & 0x7F; // min
            // buf[2] = buf[2] & 0x3F; // hour
            // buf[3] = buf[3] & 0x07; // week
            // buf[4] = buf[4] & 0x3F; // day
            // buf[5] = buf[5] & 0x1F; // mouth

            // rtc part
            // rtc_get_datetime(&tloc);
            // datetime_to_str(datetime_str, sizeof(datetime_buf), &tloc);
            sprintf(mainString, "mem i2c : %d %d %x %x %x %x", rc_w, rc_r, memDataw[0], memDataw[1], memDatar[0], memDatar[1]);

            // sprintf(mainString, "%02x:%02x:%02x  ", buf[2], buf[1], buf[0]);
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
