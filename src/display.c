#include "display.h"

sfColor background, bitplane0, bitplane1, bothBitPlanes;

// uint8_t screenBuf[2][SCHIP_SCREEN_HEIGHT][SCHIP_SCREEN_WIDTH];

void setupDisplay(int colorScheme)
{
    switch (colorScheme)
    {
    case 0:
        background = sfColor_fromRGB(0,0,0);
        bitplane0 = sfColor_fromRGB(255,255,255);
        bitplane1 = sfColor_fromRGB(255,100,100);
        bothBitPlanes = sfColor_fromRGB(100,100,100);
        break;
    
    case 1:
        background = sfColor_fromRGB(255,255,255);
        bitplane0 = sfColor_fromRGB(0,0,0);
        bitplane1 = sfColor_fromRGB(255,100,100);
        bothBitPlanes = sfColor_fromRGB(100,100,100);
        break;
    }
}

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
    if(bp0 && bp1)
        sfRectangleShape_setFillColor(rect, bothBitPlanes);
    
    else if(bp0)
        sfRectangleShape_setFillColor(rect, bitplane0);

    else if(bp1)
        sfRectangleShape_setFillColor(rect, bitplane1);
}

void renderDisplay(XOChip* xochip, sfRenderWindow* window)
{
    sfRenderWindow_clear(window, background);

    sfRectangleShape* rect = sfRectangleShape_create();
    sfVector2f size = 
    {
        .x = sfRenderWindow_getSize(window).x / xochip->screen_w,
        .y = sfRenderWindow_getSize(window).y / xochip->screen_h
    };
    sfRectangleShape_setSize(rect, size);
    sfRectangleShape_setFillColor(rect, sfColor_fromRGB(255,255,255));

    int y,x;
    for(y = 0; y < xochip->screen_h; y++)
    {
        for(x = 0; x < xochip->screen_w; x++)
        {
            uint8_t bp0 = xochip->bitplane[0][y][x] && (xochip->bitmask & 0x1);
            uint8_t bp1 = xochip->bitplane[1][y][x] && (xochip->bitmask & 0x2);

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