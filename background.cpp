#include <cstdlib>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>

#include "main.hpp"
#include "output.hpp"
#include "pico/stdlib.h"
#include "pico_display_2.hpp"

// It was 100 baloons, but for half screen we change to 50
#define OBJECTS 50
std::vector<pt> shapes;

bgEnum background = Stars;

void initialise_bg(void)
{
    for (int i = 0; i < OBJECTS; i++)
    {
        pt shape;

        shape.x  = rand() % (pico_display.bounds.w);
        shape.y  = rand() % w2dwn;
        shape.r  = (rand() % 10) + 3;
        shape.dx = float(rand() % 255) / 64.0f;
        shape.dy = float(rand() % 255) / 64.0f;
        shape.pen =
            pico_display.create_pen(rand() % 255, rand() % 255, rand() % 255);
        shapes.push_back(shape);
        shape.upDn  = true;
        shape.shine = float(rand() % 255) / 32.0f;
    }
}

void draw_background(void)
{
    static uint16_t counter;

    counter++;
    for (auto &shape : shapes)
    {
        if (background == Balloons)
        {
            shape.y = shape.y > w1dwn ? w1dwn : shape.y;

            shape.x += shape.dx;
            shape.y += shape.dy;
            if ((shape.x - shape.r) < 0)
            {
                shape.dx *= -1;
                shape.x = shape.r;
            }
            if ((shape.x + shape.r) >= (pico_display.bounds.w))
            {
                shape.dx *= -1;
                shape.x = pico_display.bounds.w - shape.r;
            }
            if ((shape.y - shape.r) < 0)
            {
                shape.dy *= -1;
                shape.y = shape.r;
            }
            if ((shape.y + shape.r) >= w1dwn)
            {
                shape.dy *= -1;
                shape.y = w1dwn - shape.r;
            }

            pico_display.set_pen(shape.pen);
            pico_display.circle(Point(shape.x, shape.y), shape.r);
        }
        if (background == Stars)
        {
            // Let's slowly move the stars from right to left
            shape.x -= 6 / 3600.0;

            if (shape.upDn == true)
                shape.shine += (shape.dx * 0.1);
            else
                shape.shine -= (shape.dx * 0.1);

            if (abs(shape.shine) > 5.0f)
                shape.upDn = (shape.upDn == true) ? false : true;

            int cross = int(abs(shape.shine));
            int diag  = cross / 2;

            if (shape.x < 5)
            {
                shape.x = rand() % (pico_display.bounds.w);
                shape.y = rand() % w2dwn;
            }

            pico_display.set_pen(255, 255, 0); // Shining yellow
            pico_display.line(Point(shape.x - cross, shape.y),
                              Point(shape.x + cross, shape.y));
            pico_display.line(Point(shape.x, shape.y - cross),
                              Point(shape.x, shape.y + cross));
            pico_display.line(Point(shape.x - diag, shape.y - diag),
                              Point(shape.x + diag, shape.y + diag));
            pico_display.line(Point(shape.x - diag, shape.y + diag),
                              Point(shape.x + diag, shape.y - diag));
            pico_display.circle(Point(shape.x, shape.y), 1);
        }
    }
}