#include <stdio.h>
#include <stdlib.h>
#include <SFML/Graphics.h>
#include <SFML/Audio.h>

#include "emulator.h"
#include "buzzer.h"

#define W_HEIGHT 320
#define W_WIDTH  640

sfClock* clock;
sfRenderWindow* window;
sfVideoMode mode;
static sfSound* buzzer;
sfEvent event;
float timer_60 = 0;  // 60Hz  timer
float timer_500 = 0; // 500Hz timer
int running;

void handleCloseEvent()
{
    while(sfRenderWindow_pollEvent(window, &event))
    {
        if(event.type == sfEvtClosed || sfKeyboard_isKeyPressed(sfKeyEscape))
        {
            running = 0;
            sfRenderWindow_close(window);
        }
    }
}

void setupWindow(int scale)
{
    mode.height = W_HEIGHT*scale;
    mode.width = W_WIDTH*scale;
    mode.bitsPerPixel = 32;
    window = sfRenderWindow_create(mode, "XO-Chip", sfResize | sfClose, NULL);
}

int main(int argc, char** argv)
{
    if(argc < 2)
        printf("usage: ./xochip.out rom.ch8 displayScale (displayScale is optional, default value is 1)\n");
    
    else
    {
        int scale = 1;
        if(argc == 3)
            scale = atoi( argv[2] );

        setupWindow(scale);
        clock = sfClock_create();
        running = 1;
        buzzer = buzzerCreate();
        
        XOChip* xochip = malloc(sizeof(XOChip));
        resetSystem(xochip);
        loadRom(argv[1], xochip);

        while(running)
        {
            handleCloseEvent();

            float dt = sfTime_asSeconds( sfClock_restart(clock) );
            timer_60 += dt;
            timer_500 += dt;

            if(timer_60 >= 1.f/60.f)
            {
                // Update timers and display at a 60Hz frequency
                timer_60 = 0;
                updateTimers(xochip, buzzer);
                render(xochip, window);
            }

            if(timer_500 >= 1.f/500.f)
            {
                // The chip-8 runs on a 500Hz "clock"
                timer_500 = 0;
                executeNextInstruction(xochip);
            }

            handleInput(xochip);
        }

        sfClock_destroy(clock);
        sfRenderWindow_destroy(window);
        buzzerDestroy();
    }
}