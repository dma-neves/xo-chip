#ifndef DISPLAY
#define DISPLAY

#include <SFML/Graphics.h>
#include "xochip.h"

void setupDisplay(int colorScheme);
void renderDisplay(XOChip* xochip, sfRenderWindow* window);
void resetDisplay();

#endif