#include "display.h"

// uint8_t screenBuf[2][SCREEN_HEIGHT][SCREEN_WIDTH];

void resetDisplay()
{
   // memset(screenBuf, 0, sizeof(screenBuf));
}

void updateBuffer(XOChip* xochip)
{
    // memcpy(screenBuf, xochip->bitplane, sizeof(xochip->bitplane));
}

void setColor(sfRectangleShape* rect, uint8_t bp0, uint8_t bp1)
{
    // TODO

    if(bp0 && bp1)
        sfRectangleShape_setFillColor(rect, sfColor_fromRGB(255,255,255));
    
    else if(bp0)
        sfRectangleShape_setFillColor(rect, sfColor_fromRGB(255,255,255));

    else if(bp1)
        sfRectangleShape_setFillColor(rect, sfColor_fromRGB(255,255,255));
}

void renderDisplay(XOChip* xochip, sfRenderWindow* window)
{
    sfRenderWindow_clear(window, sfBlack);

    sfRectangleShape* rect = sfRectangleShape_create();
    sfVector2f size = 
    {
        .x = sfRenderWindow_getSize(window).x / SCREEN_WIDTH,
        .y = sfRenderWindow_getSize(window).y / SCREEN_HEIGHT
    };
    sfRectangleShape_setSize(rect, size);
    sfRectangleShape_setFillColor(rect, sfColor_fromRGB(255,255,255));

    int y,x;
    for(y = 0; y < SCREEN_HEIGHT; y++)
    {
        for(x = 0; x < SCREEN_WIDTH; x++)
        {
            uint8_t bp0 = xochip->bitplane[0][y][x] & (xochip->bitmask & 0x1);
            uint8_t bp1 = xochip->bitplane[1][y][x] & (xochip->bitmask & 0x2);

            if(bp0 || bp1)
            {
                setColor(rect, bp0,bp1);
                sfVector2f pos = { .x = x*size.x, .y = y*size.y };
                sfRectangleShape_setPosition(rect, pos);
                sfRenderWindow_drawRectangleShape(window, rect, NULL);
            }
        }
    }

    sfRenderWindow_display(window);
    //updateBuffer(xochip);
}