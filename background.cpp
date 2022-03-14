#include <cstdlib>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "main.hpp"
#include "pico/stdlib.h"
#include "pico_display_2.hpp"
#include "output.hpp"

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
    }
}

void draw_background(void)
{
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
            shape.x += 6 / 3600.0;

            if ((shape.x + 5) >= (pico_display.bounds.w))
            {
                shape.x = 5;
            }

            pico_display.set_pen(255, 255, 0); // Shining yellow
            pico_display.line(Point(shape.x - 5, shape.y),
                              Point(shape.x + 5, shape.y));
            pico_display.line(Point(shape.x, shape.y - 5),
                              Point(shape.x, shape.y + 5));
            pico_display.line(Point(shape.x - 3, shape.y - 3),
                              Point(shape.x + 3, shape.y + 3));
            pico_display.line(Point(shape.x - 3, shape.y + 3),
                              Point(shape.x + 3, shape.y - 3));
            pico_display.circle(Point(shape.x, shape.y), 2);
        }
    }
}