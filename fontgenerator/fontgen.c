// This is the SW for generating the fonts to be used in the clock

#include "fontgen.h"
#include "bitmap_db.h"
#include <stdint.h>
#include <stdio.h>

void main()
{
    uint16_t len;
    uint16_t digit;
    uint32_t firstByte;
    uint16_t line;
    uint16_t lines;
    uint8_t bytePointer;
    uint32_t currOffset;
    char stringLine[255];

    len = forte_72ptDescriptors[0].widthBits / 8;
    if ((len * 8) < forte_72ptDescriptors[0].widthBits)
        len++;
    lines = forte_72ptDescriptors[1].offset / len;

    printf("// font header for clock\n");
    printf("//\n");

    printf("uint8_t digitFont[] = {\n");

    for (digit = 0; digit < 10; digit++)
    {
        len = forte_72ptDescriptors[digit].widthBits / 8;
        if ((len * 8) < forte_72ptDescriptors[digit].widthBits)
            len++;
        firstByte = forte_72ptDescriptors[digit].offset;

        for (line = 0; line < lines; line++)
        {
            int l1b, l1e, l2b, l2e;
            l1b = l1e = l2b = l2e = 0;
            int search = 0;

            for (bytePointer = 0; bytePointer < len; bytePointer++)
            {
                uint8_t currBit;
                uint8_t currByte = forte_72ptBitmaps[firstByte + (line * len) + bytePointer];
                uint8_t counter = 0;

                for (currBit = 0x80; currBit > 0; currBit = currBit >> 1)
                {
                    uint8_t theBit = currByte & currBit;
                    
                    switch (search) {
                        case 0 : if (theBit) {
                            l1b = (bytePointer * 8) + counter;
                            search++;
                        }
                        break;
                        case 1:
                            if (!theBit)
                            {
                                l1e = (bytePointer * 8) + counter;
                                search++;
                            }
                            break;
                        case 2:
                            if (theBit)
                            {
                                l2b = (bytePointer * 8) + counter;
                                search++;
                            }
                            break;
                        case 3:
                            if (!theBit)
                            {
                                l2e = (bytePointer * 8) + counter;
                                search++;
                            }
                            break;
                    }
                    counter++;
                }

            }
            if (l2b == 0)
                l2b = l1b;
            if (l2e == 0)
                l2e = l1e;

            printf("%d,%d,%d,%d,\n",l1b,l1e,l2b,l2e);
        }
        printf("\n");
    }

    printf("0}");
}