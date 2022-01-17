#include <stdio.h>
#include <stdlib.h>
#include <SFML/Graphics.h>
#include <SFML/Audio.h>

#include "emulator.h"
#include "buzzer.h"

#define W_HEIGHT 320
#define W_WIDTH  640

//#define DEBUG

/* SFML */
sfClock* clock;
sfRenderWindow* window;
sfVideoMode mode;
static sfSound* buzzer;
sfEvent event;

/* flags */
uint8_t runningf = 1;
uint8_t address16f = 0; // Load 16bit address (XO-Chip instruction)

uint8_t fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void setupWindow(int scale);
void handleCloseEvent();
void loadRom(char* file, XOChip* xochip);
void resetSystem(XOChip* xochip);
void executeInstruction(XOChip* xochip);
void updateTimers(XOChip* xochip);
void handleInput(XOChip* xochip);

int main(int argc, char** argv)
{
    if(argc < 2)
        printf("usage: ./xochip.out rom.ch8 displayScale (displayScale is optional, default value is 1)\n");
    
    else
    {
        int scale = (argc == 3) ? atoi( argv[2] ) : 1;
        setupWindow(scale);
        clock = sfClock_create();
        buzzer = buzzerCreate();
        
        XOChip* xochip = malloc(sizeof(XOChip));
        resetSystem(xochip);
        loadRom(argv[1], xochip);

        float timer_60 = 0;  // 60Hz  timer
        float timer_500 = 0; // 500Hz timer

        while(runningf)
        {
            handleCloseEvent();

            float dt = sfTime_asSeconds( sfClock_restart(clock) );
            timer_60 += dt;
            timer_500 += dt;

            if(timer_60 >= 1.f/60.f)
            {
                // Update timers and display at a 60Hz frequency
                timer_60 = 0;
                updateTimers(xochip);
                renderDisplay(xochip, window);
            }

            if(timer_500 >= 1.f/500.f)
            {
                // The chip-8 runs on a 500Hz "clock"
                timer_500 = 0;
                executeInstruction(xochip);
            }

            handleInput(xochip);
        }

        free(xochip);
        sfClock_destroy(clock);
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

void handleCloseEvent()
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


void loadRom(char* file, XOChip* xochip)
{
    FILE *fp;
    int c, i, max = MEM_SIZE;

    fp = fopen(file, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "cannot open input file\n");
        return;
    }

    for (i = 0; i < max && (c = getc(fp)) != EOF; i++)
        xochip->mem[PROG_START + i] = (uint8_t)c;

    fclose(fp);
}

void loadFonts(XOChip* xochip)
{
    int i = 0;
    for(; i < FONTSET_SIZE; i++)
    {
        xochip->mem[FONTSET_START_ADDRESS + i] = fontset[i]; 
    }
}

void resetSystem(XOChip* xochip)
{
    address16f = 0;
    memset(xochip, 0, sizeof(XOChip));
    xochip->PC = PROG_START;
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

    // Load 16bit address
    if(address16f) {

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
            switch(lowbyte)
            {
                case 0xE0:
                    cls(xochip); 
                    break;

                case 0xEE:
                    ret(xochip);
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
            if(word[0] == 0)
                skp_eq_vx_vy(xochip, word[2], word[1], nextInstSize);
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
            drw_vx_vy(xochip, word[2], word[1], word[0]);
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

                case 0x33:
                    st_b_vx(xochip, word[2]);
                    break;

                case 0x55:
                    st_i_vx(xochip, word[2]);
                    break;

                case 0x65:
                    ld_vx_i(xochip, word[2]);
                    break;
            }
            break;
    }

    if(xochip->pcIncFlag)
        xochip->PC += 2;
}

void updateTimers(XOChip* xochip)
{
    if(xochip->delayTimer != 0)
        xochip->delayTimer--;

    if(xochip->soundTimer != 0)
    {
        xochip->soundTimer--;
        sfSound_play(buzzer);
    }
    else
        sfSound_stop(buzzer);
}

void handleInput(XOChip* xochip)
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