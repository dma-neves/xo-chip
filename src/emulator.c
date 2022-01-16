#include "emulator.h"
#include "display.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

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
    memset(xochip, 0, sizeof(XOChip));
    xochip->PC = PROG_START;
    loadFonts(xochip);
    srand(time(NULL));

    resetDisplay();
}

void render(XOChip* xochip, sfRenderWindow* window)
{
    renderDisplay(xochip, window);
}

void executeNextInstruction(XOChip* xochip)
{
    xochip->pcIncFlag = 1;

    uint8_t highbyte = xochip->mem[xochip->PC];
    uint8_t lowbyte = xochip->mem[xochip->PC+1];

    // printf("highbyte: %X\n", highbyte);
    // printf("lowbyte: %X\n", lowbyte);
    // printRegs(xochip);
    // getchar();

    uint8_t word[4] =
    {
        lowbyte & 0x0F,
        lowbyte >> 4,
        highbyte & 0x0F,
        highbyte >> 4
    };

    uint16_t nibble = ( (uint16_t)lowbyte ) | ( (uint16_t)word[2] << 8 );

    switch(word[3])
    {
        case 0x0:
            if(lowbyte == 0xE0)
                cls(xochip);
            else if(lowbyte == 0xEE)
                ret(xochip);
            else
                sys(xochip);
            
            break;

        case 0x1:
            jp(xochip, nibble);
            break;

        case 0x2:
            call(xochip, nibble);
            break;

        case 0x3:
            se_vx_kk(xochip, word[2], lowbyte);
            break;

        case 0x4:
            sne_vx_kk(xochip, word[2], lowbyte);
            break;

        case 0x5:
            if(word[0] == 0)
                se_vx_vy(xochip, word[2], word[1]);
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
                    subn_vx_vy(xochip, word[2], word[1]);
                    break;

                case 0xE:
                    shl_vx(xochip, word[2]);
                    break;
            }
            break;

        case 0x9:
            sne_vx_vy(xochip, word[2], word[1]);
            break;

        case 0xA:
            ld_i_addr(xochip, nibble);
            break;
        
        case 0xB:
            jp_v0_addr(xochip, nibble);
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
                    skp_vx(xochip, word[2]);
                    break;

                case 0xA1:
                    sknp_vx(xochip, word[2]);
                    break;
            }
            break;

        case 0xF:
            switch(lowbyte)
            {
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

void updateTimers(XOChip* xochip, sfSound* buzzer)
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