#include "xochip.h"

/* ---------------------------- chip8 instructions ---------------------------- */

// Clear screen
void cls(XOChip* xochip)
{
    memset(xochip->screen, 0, sizeof(xochip->screen));
}

// Return from subroutine
void ret(XOChip* xochip)
{
    xochip->PC = xochip->stack[--xochip->SP];
}

void sys(XOChip* xochip)
{
    //Ignore
}

// Jump to adr
void jp(XOChip* xochip, uint16_t adr)
{
    xochip->PC = adr;
    xochip->pcIncFlag = 0;
}

// Call subroutine at adr
void call(XOChip* xochip, uint16_t adr)
{
    xochip->stack[xochip->SP++] = xochip->PC;
    xochip->PC = adr;
    xochip->pcIncFlag = 0;
}

// Skip next instruction if Vx = kk
void skp_eq_vx_kk(XOChip* xochip, uint8_t x, uint8_t kk, uint8_t instSize)
{
    if(xochip->V[x] == kk)
        xochip->PC += instSize;
}

// Skip next instruction if Vx != kk
void skp_neq_vx_kk(XOChip* xochip, uint8_t x, uint8_t kk, uint8_t instSize)
{
    if(xochip->V[x] != kk)
        xochip->PC += instSize;
}

// Skip next instruction if Vx = Vy
void skp_eq_vx_vy(XOChip* xochip, uint8_t x, uint8_t y, uint8_t instSize)
{
    if(xochip->V[x] == xochip->V[y])
        xochip->PC += instSize;
}

// Load kk to Vx
void ld_vx_kk(XOChip* xochip, uint8_t x, uint8_t kk)
{
    xochip->V[x] = kk;
}

// Vx += kk
void add_vx_kk(XOChip* xochip, uint8_t x, uint8_t kk)
{
    xochip->V[x] += kk;
}

// Load Vy to Vx
void ld_vx_vy(XOChip* xochip, uint8_t x, uint8_t y)
{
    xochip->V[x] = xochip->V[y];
}

// Vx = Vx | Vy
void or_vx_vy(XOChip* xochip, uint8_t x, uint8_t y)
{
    xochip->V[x] = xochip->V[x] | xochip->V[y];
}

// Vx = Vx & Vy
void and_vx_vy(XOChip* xochip, uint8_t x, uint8_t y)
{
    xochip->V[x] = xochip->V[x] & xochip->V[y];
}

// Vx = Vx ^ Vy
void xor_vx_vy(XOChip* xochip, uint8_t x, uint8_t y)
{
    xochip->V[x] = xochip->V[x] ^ xochip->V[y];
}

// Vx += Vy
void add_vx_vy(XOChip* xochip, uint8_t x, uint8_t y)
{
    xochip->V[0xF] = ( (uint16_t)xochip->V[x] + (uint16_t)xochip->V[y] > 255 );
    xochip->V[x] += xochip->V[y];
}

// Vx -= Vy
void sub_vx_vy(XOChip* xochip, uint8_t x, uint8_t y)
{
    xochip->V[0xF] = ( xochip->V[x] >= xochip->V[y] );
    xochip->V[x] -= xochip->V[y];
}

// Vx = Vx >> 1
void shr_vx(XOChip* xochip, uint8_t x)
{
    xochip->V[0xF] = (xochip->V[x] & 1);
    xochip->V[x] = xochip->V[x] >> 1;
}

// Vx = Vy - Vx
void sub_neg_vx_vy(XOChip* xochip, uint8_t x, uint8_t y)
{
    xochip->V[0xF] = ( xochip->V[y] >= xochip->V[x] );
    xochip->V[x] = xochip->V[y] - xochip->V[x];
}

// Vx = Vx << 1
void shl_vx(XOChip* xochip, uint8_t x)
{
    xochip->V[0xF] = ( (xochip->V[x] & 0x80) != 0 );
    xochip->V[x] = xochip->V[x] << 1;
}

// Skip next instruction if Vx != Vy
void skp_neq_vx_vy(XOChip* xochip, uint8_t x, uint8_t y, uint8_t instSize)
{
    if(xochip->V[x] != xochip->V[y])
        xochip->PC += instSize;
}

// Load adr to I
void ld_i_adr(XOChip* xochip, uint16_t adr)
{
    xochip->I = adr;
}

// Jump to V0 + adr
void jp_v0_adr(XOChip* xochip, uint16_t adr)
{
    xochip->PC = xochip->V[0] + adr;
}

// Load rnd & kk to Vx
void rnd_vx_kk(XOChip* xochip, uint8_t x, uint8_t kk, uint8_t rnd)
{
    xochip->V[x] = rnd & kk;
}

// Draw sprite at display coordinates (Vx, Vy), stored at address I with n bytes.
void drw_vx_vy(XOChip* xochip, uint8_t x, uint8_t y, uint8_t n)
{
    // Reset collision register to false
    xochip->V[0xF] = 0;
    int i,j;
    for(i = 0; i < n; i++)
    {
        uint8_t sprite_ps = xochip->mem[xochip->I + i]; // Sprite pixel set (8 pixels)

        for(j = 0; j < 8; j++)
        {
            int py = (xochip->V[y]+i) % SCREEN_HEIGHT;
            int px = (xochip->V[x]+j) % SCREEN_WIDTH;
            uint8_t* pixel = &xochip->screen[py][px];
            uint8_t sprite_p = ( sprite_ps & (0x80 >> j) ) != 0; 
            uint8_t screen_p = *pixel;
            *pixel = screen_p ^ sprite_p;

            // If pixel is ereased set collision register to true
            if(screen_p == 1 && *pixel == 0)
                xochip->V[0xF] = 1;
        }
    }
}

// Skip next instruction if key with the value of Vx is pressed
void skp_p_vx(XOChip* xochip, uint8_t x, uint8_t instSize)
{
    if(xochip->keyboard[xochip->V[x]])
        xochip->PC += instSize;
}

// Skip next instruction if key with the value of Vx is not pressed
void skp_np_vx(XOChip* xochip, uint8_t x, uint8_t instSize)
{
    if(!xochip->keyboard[xochip->V[x]])
        xochip->PC += instSize;
}

// Vx = delay timer value
void ld_vx_dt(XOChip* xochip, uint8_t x)
{
    xochip->V[x] = xochip->delayTimer;
}

// Wait for a key press, store the value of the key in Vx
void ld_vx_k(XOChip* xochip, uint8_t x)
{
    int i = 0;
    for(; i < NUM_KEYS; i++)
    {
        if(xochip->keyboard[i])
            xochip->V[x] = i;
    }

    if(i == NUM_KEYS)
        xochip->pcIncFlag = 0;
}

// delay timer = Vx
void ld_dt_vx(XOChip* xochip, uint8_t x)
{
    xochip->delayTimer = xochip->V[x];
}

// sound timer = Vx
void ld_st_vx(XOChip* xochip, uint8_t x)
{
    xochip->soundTimer = xochip->V[x];
}

// I = I + Vx
void add_i_vx(XOChip* xochip, uint8_t x)
{
    xochip->I += xochip->V[x];
}

// I = location of sprite for digit Vx
void ld_i_vx(XOChip* xochip, uint8_t x)
{
    xochip->I = FONTSET_START_ADDRESS + xochip->V[x] * FONTSET_BYTES_PER_DIGIT;
}

// Store BCD representation of Vx in memory locations I, I+1, and I+2
void st_b_vx(XOChip* xochip, uint8_t x)
{
    xochip->mem[xochip->I] = xochip->V[x]/100;
    xochip->mem[xochip->I+1] = (xochip->V[x]/10) % 10;
    xochip->mem[xochip->I+2] = (xochip->V[x] % 100) % 10;
}

// Store registers V0 through Vx to memory starting at location I
void st_i_vx(XOChip* xochip, uint8_t x)
{
    int i = 0;
    for(; i <= x; i++)
        xochip->mem[xochip->I + i] = xochip->V[x];
}

// Load registers V0 through Vx from memory starting at location I
void ld_vx_i(XOChip* xochip, uint8_t x)
{
    int i = 0;
    for(; i <= x; i++)
        xochip->V[i] = xochip->mem[xochip->I + i];
}



/* ---------------------------- XO-Chip instructions ---------------------------- */



// Save the range of registers Vx-Vy to memory starting at location I
void st_vx_vy(XOChip* xochip, uint8_t x, uint8_t y)
{
    for(int i = 0; i <= abs(y-x); i++) {

        xochip->mem[xochip->I + i] = xochip->V[ (y>x) ? x+i : x-i ];
    }
}

// Load the range of registers Vx-Vy from memory starting at location I
void ld_vx_vy(XOChip* xochip, uint8_t x, uint8_t y)
{
    for(int i = 0; i <= abs(y-x); i++)
    {
        xochip->V[ (y>x) ? x+i : x-i ] = xochip->mem[xochip->I + i];
    }
}

// Store V0 - Vx to flag registers
void st_fl_vx(XOChip* xochip, uint8_t x)
{
	memcpy(xochip->F, xochip->V, (x+1)*sizeof(uint8_t));
}

// Load V0 - Vx from flag registers
void ld_fl_vx(XOChip* xochip, uint8_t x)
{
	memcpy(xochip->V, xochip->F, (x+1)*sizeof(uint8_t));
}

void ld_i_adr16(XOChip* xochip, uint16_t adr)
{
    xochip->I = adr;
}

void plane_n(XOChip* xochip, uint8_t n)
{
	//TODO
}

void audio(XOChip* xochip)
{
	//TODO
}

void pitch_vx(XOChip* xochip, uint8_t x)
{
	//TODO
}

void scrollup_n(XOChip* xochip, uint8_t n)
{
	//TODO
}
