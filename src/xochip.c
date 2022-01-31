#include "xochip.h"

#include <stdio.h>

typedef enum _Direction{up, down, left, right} Direction;

/* ---------------------------- chip8 instructions ---------------------------- */

// Clear screen
void cls(XOChip* xochip)
{
    // TODO: improve sizeof

    if(xochip->bitmask == 1)
        memset(xochip->bitplane[0], 0, SCHIP_SCREEN_HEIGHT*SCHIP_SCREEN_WIDTH);
    
    else if(xochip->bitmask == 2)
        memset(xochip->bitplane[1], 0, SCHIP_SCREEN_HEIGHT*SCHIP_SCREEN_WIDTH);

    else if(xochip->bitmask == 3)
        memset(xochip->bitplane, 0, sizeof(xochip->bitplane));
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
    xochip->V[x] |= xochip->V[y];
}

// Vx = Vx & Vy
void and_vx_vy(XOChip* xochip, uint8_t x, uint8_t y)
{
    xochip->V[x] &= xochip->V[y];
}

// Vx = Vx ^ Vy
void xor_vx_vy(XOChip* xochip, uint8_t x, uint8_t y)
{
    xochip->V[x] ^=  xochip->V[y];
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
    xochip->V[0xF] = ( xochip->V[x] > xochip->V[y] );
    xochip->V[x] -= xochip->V[y];
}

// Vx = Vx >> 1
void shr_vx(XOChip* xochip, uint8_t x)
{
    // TODO: Check compatibility (shift Vy into Vx)
    
    xochip->V[0xF] = (xochip->V[x] & 1);
    xochip->V[x] >>= 1;
}

// Vx = Vy - Vx
void sub_neg_vx_vy(XOChip* xochip, uint8_t x, uint8_t y)
{
    xochip->V[0xF] = ( xochip->V[y] > xochip->V[x] );
    xochip->V[x] = xochip->V[y] - xochip->V[x];
}

// Vx = Vx << 1
void shl_vx(XOChip* xochip, uint8_t x)
{
    // TODO: Check compatibility (shift Vy into Vx)

    xochip->V[0xF] = xochip->V[x] >> 7;
    xochip->V[x] <<= 1;
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

void draw_plane(XOChip* xochip, int plane, uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    uint16_t adr = xochip->I;
    
    for(int row = 0; row < height; row++)
    {   
        // Sprite pixel set (8 or 16 pixels wide)
        uint16_t sprite_ps = (width == 8) ? 
            xochip->mem[adr++] : 
            ( (uint16_t)xochip->mem[adr++] << 8 ) | (uint16_t)xochip->mem[adr++];

        for(int col = 0; col < width; col++)
        {
            // TODO: Check compatibility (screen wrapping)

            int py = (xochip->V[y]+row) % xochip->screen_h;
            int px = (xochip->V[x]+col) % xochip->screen_w;

            if(px >= xochip->screen_w || py >= xochip->screen_h)
                continue;

            uint16_t bmask = (width == 8) ? (0x80 >> col) : (0x8000 >> col);

            uint8_t* pixel = &xochip->bitplane[plane][py][px];
            uint8_t sprite_p = (sprite_ps & bmask) != 0; 
            uint8_t old_p = *pixel;
            *pixel = old_p ^ sprite_p;

            // If pixel is ereased set collision register to true
            if(old_p == 1 && *pixel == 0)
                xochip->V[0xF] = 1;
        }
    }
}

void draw(XOChip* xochip, uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    // Reset collision register to false
    xochip->V[0xF] = 0;

    if(xochip->bitmask == 1 || xochip->bitmask == 3)
        draw_plane(xochip, 0, x, y, width, height);

    if(xochip->bitmask == 2 || xochip->bitmask == 3)
        draw_plane(xochip, 1, x, y, width, height);
}

// Draw sprite at display coordinates (Vx, Vy), stored at address I with n bytes.
void drw_vx_vy(XOChip* xochip, uint8_t x, uint8_t y, uint8_t n)
{
    draw(xochip, x, y, 8, n);
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
    xochip->V[0xF] = xochip->I > 0xFFF;
}

// I = location of sprite for digit Vx
void ld_i_vx(XOChip* xochip, uint8_t x)
{
    xochip->I = CHIP8_FONTSET_START_ADDRESS + xochip->V[x] * CHIP8_FONTSET_BYTES_PER_CHAR;
}

// Store BCD representation of Vx in memory locations I, I+1, and I+2
void st_b_vx(XOChip* xochip, uint8_t x)
{
    xochip->mem[xochip->I] = xochip->V[x]/100;
    xochip->mem[xochip->I+1] = (xochip->V[x]/10) % 10;
    xochip->mem[xochip->I+2] = xochip->V[x] % 10;
}

// Store registers V0 through Vx to memory starting at location I
void st_i_vx(XOChip* xochip, uint8_t x)
{
    // TODO: Check compatibility (i increment)

    memcpy(&xochip->mem[xochip->I], xochip->V, x+1);
}

// Load registers V0 through Vx from memory starting at location I
void ld_vx_i(XOChip* xochip, uint8_t x)
{
    // TODO: Check compatibility (i increment)

    memcpy(xochip->V, &xochip->mem[xochip->I], x+1);
}

/* ---------------------------- SuperChip instructions ---------------------------- */

void plane_scroll(XOChip* xochip, uint8_t plane, Direction dir, uint8_t n)
{   
    // TODO: Check screen wrapping

    int size = xochip->screen_h*xochip->screen_w;
    uint8_t buf[xochip->screen_h][xochip->screen_w];
    memset(buf, 0, size);

    int x_start = 0;
    int x_end = xochip->screen_w;
    int y_start = 0;
    int y_end = xochip->screen_h;

    int buf_y_offset = 0;
    int buf_x_offset = 0;

    switch (dir)
    {
        case up:
            y_start = n;
            buf_y_offset = -n;
            break;
        
        case down:
            y_end = xochip->screen_h-n;
            buf_y_offset = n;
            break;

        case right:
            x_end = xochip->screen_w-n;
            buf_x_offset = n;
            break;

        case left:
            x_start = n;
            buf_x_offset = -n;
            break;
    }

    for(int y = y_start; y < y_end; y++)
        for(int x = x_start; x < x_end; x++)
            buf[y+buf_y_offset][x+buf_x_offset] = xochip->bitplane[plane][y][x];

    memcpy(xochip->bitplane[plane], buf, size);
}

void scroll(XOChip* xochip, Direction dir, uint8_t n)
{
    if(xochip->bitmask == 1 || xochip->bitmask == 3)
        plane_scroll(xochip, 0, dir, n);

    if(xochip->bitmask == 2 || xochip->bitmask == 3)
        plane_scroll(xochip, 1, dir, n);   
}

void hires(XOChip* xochip)
{
    xochip->screen_h = SCHIP_SCREEN_HEIGHT;
    xochip->screen_w = SCHIP_SCREEN_WIDTH;
}

void lores(XOChip* xochip)
{
    xochip->screen_h = CHIP8_SCREEN_HEIGHT;
    xochip->screen_w = CHIP8_SCREEN_WIDTH;
}

void scrolldown_n(XOChip* xochip, uint8_t n)
{
    scroll(xochip, down, n);
}

void scrollright(XOChip* xochip)
{
    scroll(xochip, right, 4);
}

void scrollleft(XOChip* xochip)
{
    scroll(xochip, left, 4);
}

void big_drw_vx_vy(XOChip* xochip, uint8_t x, uint8_t y)
{
    draw(xochip, x, y, 16, 16);
}

void big_ld_i_vx(XOChip* xochip, uint8_t x)
{
    xochip->I = SCHIP_FONTSET_START_ADDRESS + xochip->V[x] * SCHIP_FONTSET_BYTES_PER_CHAR;
}

/* ---------------------------- XO-Chip instructions ---------------------------- */

// Save the range of registers Vx-Vy to memory starting at location I
void st_range_vx_vy(XOChip* xochip, uint8_t x, uint8_t y)
{
    for(int i = 0; i <= abs(y-x); i++) {

        xochip->mem[xochip->I + i] = xochip->V[ (y>x) ? x+i : x-i ];
    }
}

// Load the range of registers Vx-Vy from memory starting at location I
void ld_range_vx_vy(XOChip* xochip, uint8_t x, uint8_t y)
{
    for(int i = 0; i <= abs(y-x); i++)
    {
        xochip->V[ (y>x) ? x+i : x-i ] = xochip->mem[xochip->I + i];
    }
}

// Store V0 - Vx to flag registers
void st_fl(XOChip* xochip, uint8_t x)
{
	memcpy(xochip->F, xochip->V, (x+1));
}

// Load V0 - Vx from flag registers
void ld_fl(XOChip* xochip, uint8_t x)
{
	memcpy(xochip->V, xochip->F, (x+1));
}

void ld_i_adr16(XOChip* xochip, uint16_t adr)
{
    xochip->I = adr;
}

// Select drawing bitplanes
void plane_n(XOChip* xochip, uint8_t n)
{
    xochip->bitmask = n;
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
    scroll(xochip, up, n);
}
