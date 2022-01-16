#ifndef EMULATOR
#define EMULATOR

#include "xochip.h"

#include <SFML/Graphics.h>
#include <SFML/Audio.h>

void loadRom(char* file, XOChip* xochip);
void resetSystem(XOChip* xochip);
void executeNextInstruction(XOChip* xochip);
void updateTimers(XOChip* xochip, sfSound* buzzer);
void handleInput(XOChip* xochip);
void render(XOChip* xochip, sfRenderWindow* window);

// Debug
void printMem(XOChip* xochip);
void printRegs(XOChip* xochip);

#endif