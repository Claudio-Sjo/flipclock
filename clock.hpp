// Filename clock.hpp
#ifndef CLOCK_HPP_HEADERFILE
#define CLOCK_HPP_HEADERFILE
#include <cstdlib>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <vector>

extern volatile uint8_t  hours;
extern volatile uint8_t  min;
extern volatile uint8_t  sec;
extern volatile uint8_t  day;
extern volatile uint8_t  dayweek;
extern volatile uint8_t  month;
extern volatile uint16_t year;

void updateHour(uint8_t hh, uint8_t mm, uint8_t ss);

#endif