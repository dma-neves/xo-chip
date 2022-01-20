#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SFML/Graphics.h>
#include <SFML/Audio.h>

#include "xochip.h"
#include "buzzer.h"
#include "display.h"
#include "fontsets.h"

#define W_HEIGHT 320
#define W_WIDTH  640

//#define DEBUG

/* SFML */
sfClock* sfclock;
sfRenderWindow* window;
sfVideoMode mode;
static sfSound* buzzer;
sfEvent event;

/* flags */
uint8_t runningf = 1;
uint8_t address16f = 0; // Load 16bit address (XO-Chip instruction)

void setupWindow(int scale);
void handleSfEvents();
int loadRom(char* file, XOChip* xochip);
void reset(XOChip* xochip);
void executeInstruction(XOChip* xochip);
void updateTimers(XOChip* xochip);
void handleInput(XOChip* xochip);

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        printf("usage: ./xochip.out rom.ch8 speedup displayScale colorScheme printFps\n");
        printf("default speedup: 1\ndefault displayScale: 1\ndefault colorScheme: 0\ndefault printFps: 0\n");
    }
    else
    {
        int speedup = (argc >= 3) ? atoi( argv[2] ) : 1;
        int scale = (argc >= 4) ? atoi( argv[3] ) : 1;
        int colorScheme = (argc >= 5) ? atoi( argv[4] ) : 0;
        int printFps = (argc >= 6) ? atoi( argv[5] ) : 0;
        setupWindow(scale);
        setupDisplay(colorScheme);

        sfclock = sfClock_create();
        buzzer = buzzerCreate();
        
        XOChip* xochip = malloc(sizeof(XOChip));
        reset(xochip);
        if( loadRom(argv[1], xochip) ) return -1;

        float timer_60 = 0;  // 60Hz  timer
        float timer_500 = 0; // 500Hz timer
        float timer_1 = 0;   // 1Hz timer (print fps)
        int fps = 0;

        while(runningf)
        {
            handleSfEvents();

            float dt = sfTime_asSeconds( sfClock_restart(sfclock) );
            timer_60 += dt;
            timer_500 += dt;
            timer_1 += dt;

            if(printFps && timer_1 >= 1.f)
            {
                // Print fps at 1Hz frequency
                printf("fps: %d\n", fps);
                timer_1 = 0;
                fps = 0;
            }

            if(timer_60 >= 1.f/60.f)
            {
                // Update timers and display at a 60Hz frequency
                timer_60 = 0;
                updateTimers(xochip);
                renderDisplay(xochip, window);
                fps++;
            }

            if(timer_500 >= 1.f/(500.f * speedup))
            {
                handleInput(xochip);
                // The chip-8 has a clock speed of 500Hz
                timer_500 = 0;
                executeInstruction(xochip);
            }

            handleInput(xochip);
        }

        free(xochip);
        sfClock_destroy(sfclock);
        sfRenderWindow_destroy(window);
        buzzerDestroy();
    }
}

void setupWindow(int scale)
{
    mode.height = W_HEIGHT*scale;
    mode.width = W_WIDTH*scale;
    mode.bitsPerPixel = 32;
    window = sfRenderWindow_create(mode, "XO-Chip", sfResize | sfClose, NULL);
}

void handleSfEvents()
{
    if(sfRenderWindow_hasFocus(window))
    {
        while(sfRenderWindow_pollEvent(window, &event))
        {
            if(event.type == sfEvtClosed || sfKeyboard_isKeyPressed(sfKeyEscape))
            {
                runningf = 0;
                sfRenderWindow_close(window);
            }
        }
    }
}

// Returns 0 if successful and !0 otherwise
int loadRom(char* file, XOChip* xochip)
{
    FILE *fp;
    int c, i, max = MEM_SIZE;

    fp = fopen(file, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "error: cannot open input file\n");
        return -1;
    }

    for (i = 0; i < max && (c = getc(fp)) != EOF; i++)
        xochip->mem[PROG_START + i] = (uint8_t)c;

    if(c != EOF)
    {
        fprintf(stderr, "error: rom to big (Max size: %d bytes)\n", max);
        fclose(fp);
        return -1;
    }

    return fclose(fp);
}

void loadFonts(XOChip* xochip)
{
    for(int i = 0; i < CHIP8_FONTSET_SIZE; i++)
        xochip->mem[CHIP8_FONTSET_START_ADDRESS + i] = chip8_character(i); 

    for(int i = 0; i < SCHIP_FONTSET_SIZE; i++)
        xochip->mem[SCHIP_FONTSET_START_ADDRESS + i] = schip_character(i); 
}

void reset(XOChip* xochip)
{
    address16f = 0;
    memset(xochip, 0, sizeof(XOChip));
    xochip->PC = PROG_START;
    xochip->bitmask = 1;
    xochip->screen_w = CHIP8_SCREEN_WIDTH;
    xochip->screen_h = CHIP8_SCREEN_HEIGHT;
    loadFonts(xochip);
    srand(time(NULL));
    resetDisplay();
}

void executeInstruction(XOChip* xochip)
{
    xochip->pcIncFlag = 1;

    uint8_t highbyte = xochip->mem[xochip->PC];
    uint8_t lowbyte = xochip->mem[xochip->PC+1];

    #ifdef DEBUG
        printf("highbyte: %X\n", highbyte);
        printf("lowbyte: %X\n", lowbyte);
        printRegs(xochip);
        getchar();
    #endif

    if(address16f) {

        // Load 16bit address
        uint16_t address = (uint16_t)highbyte << 8 | (uint16_t)lowbyte;
        ld_i_adr16(xochip, address);
        address16f = 0;
        xochip->PC += 2;
        return;
    }


    uint8_t word[4] =
    {
        lowbyte & 0x0F,
        lowbyte >> 4,
        highbyte & 0x0F,
        highbyte >> 4
    };
    uint16_t nibble = ( (uint16_t)lowbyte ) | ( (uint16_t)word[2] << 8 );
    uint8_t nextInstSize = (xochip->mem[xochip->PC+2] == 0xF0 && xochip->mem[xochip->PC+3] == 0x00) ? 4 : 2;

    switch(word[3])
    {
        case 0x0:

            if(word[1] == 0xC)
                scrolldown_n(xochip, word[0]);

            else if(word[1] == 0xD)
                scrollup_n(xochip, word[0]);

            else switch(lowbyte)
            {
                case 0xE0:
                    cls(xochip); 
                    break;

                case 0xEE:
                    ret(xochip);
                    break;

                case 0xFB:
                    scrollright(xochip);
                    break;

                case 0xFC:
                    scrollleft(xochip);
                    break;

                case 0xFD:
                    runningf = 0; // exit
                    break;

                case 0xFE:
                    lores(xochip);
                    break;

                case 0xFF:
                    hires(xochip);
                    break;

                default:
                    sys(xochip);
                    break;
            }
            break;

        case 0x1:
            jp(xochip, nibble);
            break;

        case 0x2:
            call(xochip, nibble);
            break;

        case 0x3:
            skp_eq_vx_kk(xochip, word[2], lowbyte, nextInstSize);
            break;

        case 0x4:
            skp_neq_vx_kk(xochip, word[2], lowbyte, nextInstSize);
            break;

        case 0x5:
            switch(word[0])
            {
                case 0x0:
                    skp_eq_vx_vy(xochip, word[2], word[1], nextInstSize);
                    break;

                case 0x2:
                    st_range_vx_vy(xochip, word[2], word[1]);
                    break;

                case 0x3:
                    ld_range_vx_vy(xochip, word[2], word[1]);
                    break;
            }
            break;

        case 0x6:
            ld_vx_kk(xochip, word[2], lowbyte);
            break;

        case 0x7:
            add_vx_kk(xochip, word[2], lowbyte);
            break;

        case 0x8:
            switch(word[0])
            {
                case 0x0:
                    ld_vx_vy(xochip, word[2], word[1]);
                    break;
                
                case 0x1:
                    or_vx_vy(xochip, word[2], word[1]);
                    break;

                case 0x2:
                    and_vx_vy(xochip, word[2], word[1]);
                    break;

                case 0x3:
                    xor_vx_vy(xochip, word[2], word[1]);
                    break;

                case 0x4:
                    add_vx_vy(xochip, word[2], word[1]);
                    break;

                case 0x5:
                    sub_vx_vy(xochip, word[2], word[1]);
                    break;

                case 0x6:
                    shr_vx(xochip, word[2]);
                    break;

                case 0x7:
                    sub_neg_vx_vy(xochip, word[2], word[1]);
                    break;

                case 0xE:
                    shl_vx(xochip, word[2]);
                    break;
            }
            break;

        case 0x9:
            skp_neq_vx_vy(xochip, word[2], word[1], nextInstSize);
            break;

        case 0xA:
            ld_i_adr(xochip, nibble);
            break;
        
        case 0xB:
            jp_v0_adr(xochip, nibble);
            break;
        
        case 0xC:
            rnd_vx_kk(xochip, word[2], lowbyte, rand()%256);
            break;

        case 0xD:
            //printf("A\n");
            if(word[0] != 0)
                drw_vx_vy(xochip, word[2], word[1], word[0]);
            else
                big_drw_vx_vy(xochip, word[2], word[1]);
            
            break;

        case 0xE:
            switch(lowbyte)
            {
                case 0x9E:
                    skp_p_vx(xochip, word[2], nextInstSize);
                    break;

                case 0xA1:
                    skp_np_vx(xochip, word[2], nextInstSize);
                    break;
            }
            break;

        case 0xF:
            switch(lowbyte)
            {
                case 0x00:
                    address16f = 1;
                    break;

                case 0x01:
                    plane_n(xochip, word[2]);
                    break;

                case 0x07:
                    ld_vx_dt(xochip, word[2]);
                    break;

                case 0x0A:
                    ld_vx_k(xochip, word[2]);
                    break;

                case 0x15:
                    ld_dt_vx(xochip, word[2]);
                    break;

                case 0x18:
                    ld_st_vx(xochip, word[2]);
                    break;
                
                case 0x1E:
                    add_i_vx(xochip, word[2]);
                    break;

                case 0x29:
                    ld_i_vx(xochip, word[2]);
                    break;

                case 0x30:
                    big_ld_i_vx(xochip, word[2]);
                    break;

                case 0x33:
                    st_b_vx(xochip, word[2]);
                    break;

                case 0x55:
                    st_i_vx(xochip, word[2]);
                    break;

                case 0x65:
                    ld_vx_i(xochip, word[2]);
                    break;

                case 0x75:
                    st_fl(xochip, word[2]);
                    break;

                case 0x85:
                    ld_fl(xochip, word[2]);
                    break;
            }
            break;
    }

    if(xochip->pcIncFlag)
        xochip->PC += 2;
}

void updateTimers(XOChip* xochip)
{
    if(xochip->delayTimer)
        xochip->delayTimer--;

    if(xochip->soundTimer)
    {
        xochip->soundTimer--;
        //sfSound_play(buzzer);
    }
    else
        ;
        // sfSound_stop(buzzer);
}

void handleInput(XOChip* xochip)
{
    if(sfRenderWindow_hasFocus(window))
    {
        xochip->keyboard[0x1] = sfKeyboard_isKeyPressed(sfKeyNum1);
        xochip->keyboard[0x2] = sfKeyboard_isKeyPressed(sfKeyNum2);
        xochip->keyboard[0x3] = sfKeyboard_isKeyPressed(sfKeyNum3);
        xochip->keyboard[0xC] = sfKeyboard_isKeyPressed(sfKeyNum4);

        xochip->keyboard[0x4] = sfKeyboard_isKeyPressed(sfKeyQ);
        xochip->keyboard[0x5] = sfKeyboard_isKeyPressed(sfKeyW);
        xochip->keyboard[0x6] = sfKeyboard_isKeyPressed(sfKeyE);
        xochip->keyboard[0xD] = sfKeyboard_isKeyPressed(sfKeyR);

        xochip->keyboard[0x7] = sfKeyboard_isKeyPressed(sfKeyA);
        xochip->keyboard[0x8] = sfKeyboard_isKeyPressed(sfKeyS);
        xochip->keyboard[0x9] = sfKeyboard_isKeyPressed(sfKeyD);
        xochip->keyboard[0xE] = sfKeyboard_isKeyPressed(sfKeyF);

        xochip->keyboard[0xA] = sfKeyboard_isKeyPressed(sfKeyZ);
        xochip->keyboard[0x0] = sfKeyboard_isKeyPressed(sfKeyX);
        xochip->keyboard[0xB] = sfKeyboard_isKeyPressed(sfKeyC);
        xochip->keyboard[0xF] = sfKeyboard_isKeyPressed(sfKeyV);
    }
}

/* --------------------- Debug --------------------- */

void printMem(XOChip* xochip)
{
    int i;
    for(i = PROG_START; i < PROG_START+250; i++)
    {
        printf("%X ", xochip->mem[i]);

        if((i+1)%16 == 0) printf("\n");
    }
    printf("\n");
}

void printRegs(XOChip* xochip)
{
    for(int i = 0; i < 16; i++)
        printf("V[%X] = %X\n", i, xochip->V[i]);


    printf("I = %X\n", xochip->I);
    printf("PC = %X\n", xochip->PC);
    printf("SP = %X\n", xochip->SP);
    printf("DT = %X\n", xochip->delayTimer);
    printf("ST = %X\n", xochip->soundTimer);
}