/* http://stackoverflow.com/questions/7064531/sunrise-sunset-times-in-c */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>

#include "clock.hpp"
#include "main.hpp"
#include "output.hpp"
#include "pico/stdlib.h"
#include "pico_display_2.hpp"

#define ZENITH -.83

float_t calculateSunrise(int32_t year,int32_t month,int32_t day,float_t lat, float_t lng,int32_t localOffset, int32_t daylightSavings) {
   //1. first calculate the day of the year
   float_t N1 = floor(275 * month / 9);
   float_t N2 = floor((month + 9) / 12);
   float_t N3 = (1 + floor((year - 4 * floor(year / 4) + 2) / 3));
   float_t N = N1 - (N2 * N3) + day - 30;

   //2. convert the longitude to hour value and calculate an approximate time
   float_t lngHour = lng / 15.0;
   float_t t = N + ((6 - lngHour) / 24);   //if rising time is desired:

   //3. calculate the Sun's mean anomaly
   float_t M = (0.9856 * t) - 3.289;

   //4. calculate the Sun's true longitude
   float_t L = fmod(M + (1.916 * sin((M_PI/180)*M)) + (0.020 * sin(2 *(M_PI/180) * M)) + 282.634,360.0);

   //5a. calculate the Sun's right ascension
   float_t RA = fmod(180 / M_PI * atan(0.91764 * tan((M_PI / 180) * L)), 360.0);

   //5b. right ascension value needs to be in the same quadrant as L
   float_t Lquadrant  = floor( L/90) * 90;
   float_t RAquadrant = floor(RA/90) * 90;
   RA = RA + (Lquadrant - RAquadrant);

   //5c. right ascension value needs to be converted into hours
   RA = RA / 15;

   //6. calculate the Sun's declination
   float_t sinDec = 0.39782 * sin((M_PI/180)*L);
   float_t cosDec = cos(asin(sinDec));

   //7a. calculate the Sun's local hour angle
   float_t cosH = (sin((M_PI/180)*ZENITH) - (sinDec * sin((M_PI/180)*lat))) / (cosDec * cos((M_PI/180)*lat));
   /*
   if (cosH >  1)
   the sun never rises on this location (on the specified date)
   if (cosH < -1)
   the sun never sets on this location (on the specified date)
   */

   //7b. finish calculating H and convert into hours
   float_t H = 360 - (180/M_PI)*acos(cosH);   //   if if rising time is desired:
   H = H / 15;

   //8. calculate local mean time of rising/setting
   float_t T = H + RA - (0.06571 * t) - 6.622;

   //9. adjust back to UTC
   float_t UT = fmod(T - lngHour,24.0);

   //10. convert UT value to local time zone of latitude/longitude
   return UT + localOffset + daylightSavings;
}

float_t calculateSunset(int32_t year,int32_t month,int32_t day,float_t lat, float_t lng,int32_t localOffset, int32_t daylightSavings) {
   //1. first calculate the day of the year
   float_t N1 = floor(275 * month / 9);
   float_t N2 = floor((month + 9) / 12);
   float_t N3 = (1 + floor((year - 4 * floor(year / 4) + 2) / 3));
   float_t N = N1 - (N2 * N3) + day - 30;

   //2. convert the longitude to hour value and calculate an approximate time
   float_t lngHour = lng / 15.0;
   float_t t = N + ((18 - lngHour) / 24);   // if setting time is desired:

   //3. calculate the Sun's mean anomaly
   float_t M = (0.9856 * t) - 3.289;

   //4. calculate the Sun's true longitude
   float_t L = fmod(M + (1.916 * sin((M_PI/180)*M)) + (0.020 * sin(2 *(M_PI/180) * M)) + 282.634,360.0);

   //5a. calculate the Sun's right ascension
   float_t RA = fmod(180/M_PI*atan(0.91764 * tan((M_PI/180)*L)),360.0);

   //5b. right ascension value needs to be in the same quadrant as L
   float_t Lquadrant  = floor( L/90) * 90;
   float_t RAquadrant = floor(RA/90) * 90;
   RA = RA + (Lquadrant - RAquadrant);

   //5c. right ascension value needs to be converted into hours
   RA = RA / 15;

   //6. calculate the Sun's declination
   float_t sinDec = 0.39782 * sin((M_PI/180)*L);
   float_t cosDec = cos(asin(sinDec));

   //7a. calculate the Sun's local hour angle
   float_t cosH = (sin((M_PI/180)*ZENITH) - (sinDec * sin((M_PI/180)*lat))) / (cosDec * cos((M_PI/180)*lat));
   /*
   if (cosH >  1)
   the sun never rises on this location (on the specified date)
   if (cosH < -1)
   the sun never sets on this location (on the specified date)
   */

   //7b. finish calculating H and convert into hours
   float_t H = (180/M_PI)*acos(cosH);    // if setting time is desired:
   H = H / 15;

   //8. calculate local mean time of rising/setting
   float_t T = H + RA - (0.06571 * t) - 6.622;

   //9. adjust back to UTC
   float_t UT = fmod(T - lngHour,24.0);

   //10. convert UT value to local time zone of latitude/longitude
   return UT + localOffset + daylightSavings;
}
