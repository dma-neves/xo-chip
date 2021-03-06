#ifndef XOCHIP
#define XOCHIP

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "fontsets.h"

#define MEM_SIZE   0xFFFF
#define PROG_START 0x200
#define NUM_KEYS 16
#define CHIP8_SCREEN_WIDTH 64
#define CHIP8_SCREEN_HEIGHT 32
#define SCHIP_SCREEN_WIDTH 128
#define SCHIP_SCREEN_HEIGHT 64
#define CHIP8_FONTSET_START_ADDRESS 0x0
#define SCHIP_FONTSET_START_ADDRESS 0x50

typedef struct _XOChip
{
    uint8_t mem[MEM_SIZE];          // 4096 byte memory
    uint16_t stack[16];             // stack 16 16-bit values

    uint8_t F[16];                  // flag registers (XO-Chip)
    uint8_t V[16];                  // 16 8-bit registers
    uint16_t I;                     // 16-bit register for addresses
    uint8_t SP;                     // 8-bit stack pointer
    uint16_t PC;                    // 16-bit program counter
    uint8_t delayTimer, soundTimer; // 8-bit registers for sound and delay timers
    uint8_t bitmask;                // Bitplane bitmask

    uint8_t screen_w;
    uint8_t screen_h;
    uint8_t bitplane[2][SCHIP_SCREEN_HEIGHT][SCHIP_SCREEN_WIDTH];
    uint8_t keyboard[NUM_KEYS];

    uint8_t pcIncFlag;
} XOChip;

/* -------------- chip8 instructions -------------- */

void cls(XOChip* xochip);
void ret(XOChip* xochip);
void sys(XOChip* xochip);
void jp(XOChip* xochip, uint16_t adr);
void call(XOChip* xochip, uint16_t adr);
void skp_eq_vx_kk(XOChip* xochip, uint8_t x, uint8_t kk, uint8_t instSize);
void skp_neq_vx_kk(XOChip* xochip, uint8_t x, uint8_t kk, uint8_t instSize);
void skp_eq_vx_vy(XOChip* xochip, uint8_t x, uint8_t y, uint8_t instSize);
void ld_vx_kk(XOChip* xochip, uint8_t x, uint8_t kk);
void add_vx_kk(XOChip* xochip, uint8_t x, uint8_t kk);
void ld_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void or_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void and_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void xor_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void add_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void sub_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void shr_vx(XOChip* xochip, uint8_t x);
void sub_neg_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void shl_vx(XOChip* xochip, uint8_t x);
void skp_neq_vx_vy(XOChip* xochip, uint8_t x, uint8_t y, uint8_t instSize);
void ld_i_adr(XOChip* xochip, uint16_t adr);
void jp_v0_adr(XOChip* xochip, uint16_t adr);
void rnd_vx_kk(XOChip* xochip, uint8_t x, uint8_t kk, uint8_t rnd);
void drw_vx_vy(XOChip* xochip, uint8_t x, uint8_t y, uint8_t n);
void skp_p_vx(XOChip* xochip, uint8_t x, uint8_t instSize);
void skp_np_vx(XOChip* xochip, uint8_t x, uint8_t instSize);
void ld_vx_dt(XOChip* xochip, uint8_t x);
void ld_vx_k(XOChip* xochip, uint8_t x);
void ld_dt_vx(XOChip* xochip, uint8_t x);
void ld_st_vx(XOChip* xochip, uint8_t x);
void add_i_vx(XOChip* xochip, uint8_t x);
void ld_i_vx(XOChip* xochip, uint8_t x);
void st_b_vx(XOChip* xochip, uint8_t x);
void st_i_vx(XOChip* xochip, uint8_t x);
void ld_vx_i(XOChip* xochip, uint8_t x);

/* -------------- Super Chip instructions -------------- */

void hires(XOChip* xochip);
void lores(XOChip* xochip);
void scrolldown_n(XOChip* xochip, uint8_t n);
void scrollright(XOChip* xochip);
void scrollleft(XOChip* xochip);
void big_drw_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void big_ld_i_vx(XOChip* xochip, uint8_t x);

// void st_fl(XOChip* xochip, uint8_t x); -> XO-Chip generalization
// void ld_fl(XOChip* xochip, uint8_t x); -> XO-Chip generalization
// void exit();                           -> Handled in main

/* -------------- XO-Chip instructions -------------- */

void st_range_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void ld_range_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void st_fl(XOChip* xochip, uint8_t x);
void ld_fl(XOChip* xochip, uint8_t x);
void ld_i_adr16(XOChip* xochip, uint16_t adr);
void plane_n(XOChip* xochip, uint8_t n);
void audio(XOChip* xochip);
void pitch_vx(XOChip* xochip, uint8_t x);
void scrollup_n(XOChip* xochip, uint8_t n);

#endif

