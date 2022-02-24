// Filename output.cpp

#include "fonts/bitmap_db.h"
#include "fonts/clockFonts.h"
#include "fonts/lowfontgen.h"
#include "input.hpp"
#include "main.hpp"
#include "pico/stdlib.h"
#include "pico_display_2.hpp"
#include <cstdlib>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <vector>

uint16_t     buffer[PicoDisplay2::WIDTH * PicoDisplay2::HEIGHT];
PicoDisplay2 pico_display(buffer);

void myPrintLowFont(Point location, const std::string str)
{
    int x = location.x;
    int y = location.y;
    int ix;
    int cx      = 0;
    int cy      = 0;
    int charpos = 0;

    if (str.size() > 20)
        return;

    for (ix = 0; ix < str.size(); ix++)
    {
        // Search for the char
        int chidx    = str[ix];
        chidx        = chidx - forte_24ptFontInfo.startChar;
        int chsize   = forte_24ptDescriptors[chidx].widthBits;
        int choffset = forte_24ptDescriptors[chidx].offset;
        int chheigth = forte_24ptDescriptors[chidx].heightBits;

        if (chsize != 0)
        {
            for (int hh = 0; hh < chheigth; hh++)
            {
                int byteOfs = (hh * ((chsize + 7) / 8));

                for (int chlen = 0; chlen < chsize; chlen++)
                {
                    int pbyte   = forte_24ptBitmaps[byteOfs + choffset + (chlen / 8)];
                    int chshift = chlen % 8;
                    if ((pbyte << chshift) & 0x80)
                        pico_display.pixel(Point(x + cx, y + cy));
                    cx++;
                }
                cx = charpos;
                cy++;
            }
        }
        charpos += chsize + 2;
        cx = charpos;
        cy = 0;
    }
}

void myPrintLine(Point start, Point end, int r, int g, int b)
{
    Point s = start;
    Point e = end;

    int len       = end.x - start.x;
    int len_a     = abs(len);
    int direction = (len > 0) ? 1 : -1;

    pico_display.set_pen(r, g, b); // Black

    if (len_a < 3)
    {
        pico_display.line(start, end);
    }
    else
    {
        s.x = s.x + 1 * direction;
        e.x = e.x - 1 * direction;
        pico_display.line(s, e);

        if (len_a >= 4)
        {
            pico_display.set_pen(r, g, b); // White
            s.x = s.x + 1 * direction;
            e.x = e.x - 1 * direction;
            pico_display.line(s, e);
        }
        else
        {
            pico_display.set_pen(r, g, b); // White
            s   = start;
            s.x = s.x + 1 * direction;
            pico_display.line(s, s);
        }
    }
}

void printDigit(Point location, uint8_t digit, int r, int g, int b)
{
    int base = digit * 64 * 4;

    Point start = location;
    Point end   = location;

    for (int i = 0; i < 64; i++)
    {
        int ix  = 4 * i;
        start.x = location.x + digitFont[base + ix];
        end.x   = location.x + digitFont[base + ix + 1];
        myPrintLine(start, end, r, g, b);
        if (digitFont[base + ix] != digitFont[base + ix + 2])
        {
            start.x = location.x + digitFont[base + ix + 2];
            end.x   = location.x + digitFont[base + ix + 3];
            myPrintLine(start, end, r, g, b);
        }
        start.y = start.y + 1;
        end.y   = end.y + 1;
    }
}

void mergeDigitPrint(Point location, uint8_t before, uint8_t after, uint8_t sk, int r, int g, int b)
{
    int baseBe = before * 64 * 4;
    int baseAf = after * 64 * 4;

    Point start = location;
    Point end   = location;

    for (int i = 0; i < 64; i++)
    {
        int ix     = 4 * i;
        int step   = (digitFont[baseAf + ix] - digitFont[baseBe + ix]);
        int offset = (step * sk) / 10;
        start.x    = location.x + digitFont[baseBe + ix] + offset;
        step       = (digitFont[baseAf + ix + 1] - digitFont[baseBe + ix + 1]);
        offset     = (step * sk) / 10;
        end.x      = location.x + digitFont[baseBe + ix + 1] + offset;
        myPrintLine(start, end, r, g, b);
        step    = (digitFont[baseAf + ix + 2] - digitFont[baseBe + ix + 2]);
        offset  = (step * sk) / 10;
        start.x = location.x + digitFont[baseBe + ix + 2] + offset;
        step    = (digitFont[baseAf + ix + 3] - digitFont[baseBe + ix + 3]);
        offset  = (step * sk) / 10;
        end.x   = location.x + digitFont[baseBe + ix + 3] + offset;
        myPrintLine(start, end, r, g, b);
        start.y = start.y + 1;
        end.y   = end.y + 1;
    }
}

