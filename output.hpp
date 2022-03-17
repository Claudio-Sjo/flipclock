// File output.hpp
#ifndef OUTPUT_HPP_HEADERFILE
#define OUTPUT_HPP_HEADERFILE
#include "pico_display_2.hpp"

using namespace pimoroni;

extern int w1top;
extern int w1dwn;
extern int w2top;
extern int w2dwn;

extern uint16_t     buffer[];
extern PicoDisplay2 pico_display;

void myPrintLowFont(Point location, const std::string str);
void myPrintDayLowFont(Point location, const std::string str);
void printDigit(Point location, uint8_t digit, int r, int g, int b);
void mergeDigitPrint(Point location, uint8_t before, uint8_t after, uint8_t sk, int r, int g, int b);
void from_hsv(float h, float s, float v, uint8_t &r, uint8_t &g, uint8_t &b);
void updateDisplay(void);

#endif