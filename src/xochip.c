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

// Jump to addr
void jp(XOChip* xochip, uint16_t addr)
{
    xochip->PC = addr;
    xochip->pcIncFlag = 0;
}

// Call subroutine at addr
void call(XOChip* xochip, uint16_t addr)
{
    xochip->stack[xochip->SP++] = xochip->PC;
    xochip->PC = addr;
    xochip->pcIncFlag = 0;
}

// Skip next instruction if Vx = kk
void se_vx_kk(XOChip* xochip, uint8_t x, uint8_t kk)
{
    if(xochip->V[x] == kk)
        xochip->PC += 2;
}

// Skip next instruction if Vx != kk
void sne_vx_kk(XOChip* xochip, uint8_t x, uint8_t kk)
{
    if(xochip->V[x] != kk)
        xochip->PC += 2;
}

// Skip next instruction if Vx = Vy
void se_vx_vy(XOChip* xochip, uint8_t x, uint8_t y)
{
    if(xochip->V[x] == xochip->V[y])
        xochip->PC += 2;
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
void subn_vx_vy(XOChip* xochip, uint8_t x, uint8_t y)
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
void sne_vx_vy(XOChip* xochip, uint8_t x, uint8_t y)
{
    if(xochip->V[x] != xochip->V[y])
        xochip->PC += 2;
}

// Load addr to I
void ld_i_addr(XOChip* xochip, uint16_t addr)
{
    xochip->I = addr;
}

// Jump to V0 + addr
void jp_v0_addr(XOChip* xochip, uint16_t addr)
{
    xochip->PC = xochip->V[0] + addr;
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
void skp_vx(XOChip* xochip, uint8_t x)
{
    if(xochip->keyboard[xochip->V[x]])
        xochip->PC += 2;
}

// Skip next instruction if key with the value of Vx is not pressed
void sknp_vx(XOChip* xochip, uint8_t x)
{
    if(!xochip->keyboard[xochip->V[x]])
        xochip->PC += 2;
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



void st_vx_vy(XOChip* xochip, uint8_t x, uint8_t y) {
	//TODO
}

void ld_vx_vy(XOChip* xochip, uint8_t x, uint8_t y) {
	//TODO
}

void st_fl_vx(XOChip* xochip, uint8_t x) {
	//TODO
}

void ld_fl_vx(XOChip* xochip, uint8_t x) {
	//TODO
}

void ld_i_addr16(XOChip* xochip, uint16_t addr) {
	//TODO
}

void plane_n(XOChip* xochip, uint8_t n) {
	//TODO
}

void audio(XOChip* xochip) {
	//TODO
}

void pitch_vx(XOChip* xochip, uint8_t x) {
	//TODO
}

void scrollup_n(XOChip* xochip, uint8_t n) {
	//TODO
}
