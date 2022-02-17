#include <string.h>
#include <math.h>
#include <vector>
#include <cstdlib>
#include <stdio.h>
#include "pico/stdlib.h"

#include "pico_display_2.hpp"

#include "clockFonts.h"


using namespace pimoroni;

uint16_t buffer[PicoDisplay2::WIDTH * PicoDisplay2::HEIGHT];
PicoDisplay2 pico_display(buffer);


// Let's divide the screen in 2 windows
// Win1 is 2/3 of the screen from the top
// Win2 is 1/3 of the screen from bottom
int w1top;
int w1dwn;
int w2top;
int w2dwn;

// Main string for messages
char mainString[64];

static uint8_t hours = 0;
static uint8_t min = 0;
static uint8_t sec = 0;

static uint8_t scheduler = 0;

// Here we play with internal timers
bool oneSecCallback(struct repeating_timer *t) {


  if (++sec >= 60){
    if (++min >= 60){
      if (++hours >= 24)
        hours = 0;
      min = 0;
    }
    sec = 0;
  }
  sprintf(mainString,"%02d:%02d:%02d",hours,min,sec);

  return true;
}

bool oneTwenthCallback(struct repeating_timer *t)
{

    if (++scheduler >= 20)
      scheduler = 0;

    return true;
}

// HSV Conversion expects float inputs in the range of 0.00-1.00 for each channel
// Outputs are rgb in the range 0-255 for each channel
void from_hsv(float h, float s, float v, uint8_t &r, uint8_t &g, uint8_t &b) {
  float i = floor(h * 6.0f);
  float f = h * 6.0f - i;
  v *= 255.0f;
  uint8_t p = v * (1.0f - s);
  uint8_t q = v * (1.0f - f * s);
  uint8_t t = v * (1.0f - (1.0f - f) * s);

  switch (int(i) % 6) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5: r = v; g = p; b = q; break;
  }
}


void printDigit(Point location,uint8_t digit)
{
  int base = digit * 64 * 4;

  Point start = location;
  Point end = location;

  for (int i = 0; i < 64; i++)
  {
    int ix = 4*i;
    start.x = location.x + digitFont[base + ix];
    end.x = location.x + digitFont[base + ix + 1];
    pico_display.line(start, end);
    if (digitFont[base + ix] != digitFont[base + ix + 2])
    {
     start.x = location.x + digitFont[base + ix +2];
     end.x = location.x + digitFont[base + ix + 3];
     pico_display.line(start, end);
    }
    start.y = start.y +1;
    end.y = end.y +1;
  }
}

void mergeDigitPrint(Point location, uint8_t before, uint8_t after, uint8_t sk)
{
    int baseBe = before * 64 * 4;
    int baseAf = after * 64 * 4;

    Point start = location;
    Point end   = location;

    for (int i = 0; i < 64; i++)
    {
        int ix  = 4 * i;
        int step = (digitFont[baseBe + ix] - digitFont[baseAf + ix]) / 10;
        int offset = step * sk;
        start.x    = location.x + digitFont[baseBe + ix] - offset;
        step       = (digitFont[baseBe + ix] + 1 - digitFont[baseAf + ix] + 1) / 10;
        offset     = step * sk;
        end.x      = location.x + digitFont[baseBe + ix + 1] - offset;
        pico_display.line(start, end);
        step       = (digitFont[baseBe + ix + 2] - digitFont[baseAf + ix] + 2) / 10;
        offset     = step * sk;
        start.x    = location.x + digitFont[baseBe + ix + 2] - offset;
        step       = (digitFont[baseBe + ix + 3] - digitFont[baseAf + ix + 3]) / 10;
        offset     = step * sk;
        end.x      = location.x + digitFont[baseBe + ix + 3] - offset;
        pico_display.line(start, end);
        start.y = start.y + 1;
        end.y   = end.y + 1;
    }
}


void updateHour(uint8_t hh, uint8_t mm, uint8_t ss)
{
  static uint8_t oldhh, oldmm, oldss;
  Point          digitPoint(5, 60);

  pico_display.set_pen(255, 255, 255);

  if (oldhh != hh)
  {
    if ((oldhh / 10) != (hh / 10))
        mergeDigitPrint(digitPoint, oldhh/10, hh/10, scheduler);
    else
      printDigit(digitPoint, hours / 10);
    digitPoint.x = 50;
    mergeDigitPrint(digitPoint, oldhh % 10, hh % 10, scheduler);
    if (scheduler > 10)
        oldhh = hh;
  }
  else
  {
      printDigit(digitPoint, hours / 10);
      digitPoint.x = 50;
      printDigit(digitPoint, hours % 10);
  }

  digitPoint.x = 115;
  if (oldmm != mm)
  {
      if ((oldmm / 10) != (mm / 10))
          mergeDigitPrint(digitPoint, oldmm / 10, mm / 10, scheduler);
      else
        printDigit(digitPoint, min / 10);
      digitPoint.x = 160;
      mergeDigitPrint(digitPoint, oldmm % 10, mm % 10, scheduler);
      if (scheduler > 10)
          oldmm = mm;
  }
  else
  {
      printDigit(digitPoint, min / 10);
      digitPoint.x = 160;
      printDigit(digitPoint, min % 10);
  }

  digitPoint.x = 225;
  if (oldss != ss)
  {
      if ((oldss / 10) != (ss / 10))
          mergeDigitPrint(digitPoint, oldss / 10, ss / 10, scheduler);
      else
        printDigit(digitPoint, sec / 10);
      digitPoint.x = 270;
      mergeDigitPrint(digitPoint, oldss % 10, ss % 10, scheduler);
      if (scheduler > 10)
          oldss = ss;
  }
  else
  {
      printDigit(digitPoint, sec / 10);
      digitPoint.x = 270;
      printDigit(digitPoint, sec % 10);
  }
}

int main() {
  pico_display.init();
  pico_display.set_backlight(255);

  struct repeating_timer oneSectimer;
  struct repeating_timer oneTwenthtimer;

  struct pt {
    float      x;
    float      y;
    uint8_t    r;
    float     dx;
    float     dy;
    uint16_t pen;
  };


w1top = 0;
w1dwn = (pico_display.bounds.h / 3) * 2;
w2top = w1dwn +1;
w2dwn = pico_display.bounds.h;

// It was 100 baloons, but for half screen we change to 50
#define BALOONS 50
  std::vector<pt> shapes;
  for(int i = 0; i < BALOONS; i++) {
    pt shape;
    shape.x = rand() % (pico_display.bounds.w);
    shape.y = rand() % w1dwn;
    shape.r = (rand() % 10) + 3;
    shape.dx = float(rand() % 255) / 64.0f;
    shape.dy = float(rand() % 255) / 64.0f;
    shape.pen = pico_display.create_pen(rand() % 255, rand() % 255, rand() % 255);
    shapes.push_back(shape);
  }

  Point text_location(pico_display.bounds.w/4, w2top + ((w2dwn - w2top)/3));
  Point mainS_location(pico_display.bounds.w/2 - 20, w2top + ((w2dwn - w2top)/3 + 16));
  Point scrolling_line_b(0,w2top);
  Point scrolling_line_e(pico_display.bounds.w,w2top);

  sprintf(mainString,"********");

  add_repeating_timer_ms(1000, oneSecCallback, NULL, &oneSectimer);
  add_repeating_timer_ms(50, oneTwenthCallback, NULL, &oneTwenthtimer);

  while(true) {
    if(pico_display.is_pressed(pico_display.A)) text_location.x -= 1;
    if(pico_display.is_pressed(pico_display.B)) text_location.x += 1;

    if(pico_display.is_pressed(pico_display.X)) text_location.y -= 1;
    if(pico_display.is_pressed(pico_display.Y)) text_location.y += 1;
  
    pico_display.set_pen(120, 40, 60);
    pico_display.clear();

    for(auto &shape : shapes) {
      shape.x += shape.dx;
      shape.y += shape.dy;
      if((shape.x - shape.r) < 0) {
        shape.dx *= -1;
        shape.x = shape.r;
      }
      if((shape.x + shape.r) >= (pico_display.bounds.w)) {
        shape.dx *= -1;
        shape.x = pico_display.bounds.w - shape.r;
      }
      if((shape.y - shape.r) < 0) {
        shape.dy *= -1;
        shape.y = shape.r;
      }
      if((shape.y + shape.r) >= w1dwn) {
        shape.dy *= -1;
        shape.y = w1dwn - shape.r;
      }

      pico_display.set_pen(shape.pen);
      pico_display.circle(Point(shape.x, shape.y), shape.r);

    }


  updateHour(hours,min,sec);

    // Since HSV takes a float from 0.0 to 1.0 indicating hue,
    // then we can divide millis by the number of milliseconds
    // we want a full colour cycle to take. 5000 = 5 sec.
    uint8_t r = 0, g = 0, b = 0;
    from_hsv((float)millis() / 5000.0f, 1.0f, 0.5f + sinf(millis() / 100.0f / 3.14159f) * 0.5f, r, g, b);
    pico_display.set_led(r, g, b);


    pico_display.set_pen(255, 255, 255);
    pico_display.text("Hello World", text_location, 320);
    pico_display.text(mainString, mainS_location, 320);

    pico_display.line(scrolling_line_b, scrolling_line_e);

    // update screen
    pico_display.update();
  }

    return 0;
}
