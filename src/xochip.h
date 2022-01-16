#ifndef CHIP8
#define CHIP8

#include <stdint.h>
#include <string.h>

#define MEM_SIZE   0xFFF
#define PROG_START 0x200
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define NUM_KEYS 16
#define FONTSET_START_ADDRESS 0x50
#define FONTSET_BYTES_PER_DIGIT 5
#define FONTSET_NDIGITS 16
#define FONTSET_SIZE FONTSET_NDIGITS*FONTSET_BYTES_PER_DIGIT

typedef struct _XOChip
{
    uint8_t mem[MEM_SIZE];          // 4096 byte memory

    uint8_t V[16];                  // 16 8-bit registers
    uint16_t I;                     // 16-bit register for addresses
    uint8_t SP;                     // 8-bit stack pointer
    uint16_t PC;                    // 16-bit program counter
    uint8_t delayTimer, soundTimer; // 8-bit registers for sound and delay timers

    uint16_t stack[16];             // stack 16 16-bit values

    uint8_t screen[SCREEN_HEIGHT][SCREEN_WIDTH];
    uint8_t keyboard[NUM_KEYS];

    uint8_t pcIncFlag;
} XOChip;

/* -------------- chip8 instructions -------------- */

void cls(XOChip* xochip);
void ret(XOChip* xochip);
void sys(XOChip* xochip);
void jp(XOChip* xochip, uint16_t addr);
void call(XOChip* xochip, uint16_t addr);
void se_vx_kk(XOChip* xochip, uint8_t x, uint8_t kk);
void sne_vx_kk(XOChip* xochip, uint8_t x, uint8_t kk);
void se_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void ld_vx_kk(XOChip* xochip, uint8_t x, uint8_t kk);
void add_vx_kk(XOChip* xochip, uint8_t x, uint8_t kk);
void ld_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void or_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void and_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void xor_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void add_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void sub_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void shr_vx(XOChip* xochip, uint8_t x);
void subn_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void shl_vx(XOChip* xochip, uint8_t x);
void sne_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void ld_i_addr(XOChip* xochip, uint16_t addr);
void jp_v0_addr(XOChip* xochip, uint16_t addr);
void rnd_vx_kk(XOChip* xochip, uint8_t x, uint8_t kk, uint8_t rnd);
void drw_vx_vy(XOChip* xochip, uint8_t x, uint8_t y, uint8_t n);
void skp_vx(XOChip* xochip, uint8_t x);
void sknp_vx(XOChip* xochip, uint8_t x);
void ld_vx_dt(XOChip* xochip, uint8_t x);
void ld_vx_k(XOChip* xochip, uint8_t x);
void ld_dt_vx(XOChip* xochip, uint8_t x);
void ld_st_vx(XOChip* xochip, uint8_t x);
void add_i_vx(XOChip* xochip, uint8_t x);
void ld_i_vx(XOChip* xochip, uint8_t x);
void st_b_vx(XOChip* xochip, uint8_t x);
void st_i_vx(XOChip* xochip, uint8_t x);
void ld_vx_i(XOChip* xochip, uint8_t x);

/* -------------- XO-Chip instructions -------------- */

void st_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void ld_vx_vy(XOChip* xochip, uint8_t x, uint8_t y);
void st_fl_vx(XOChip* xochip, uint8_t x);
void ld_fl_vx(XOChip* xochip, uint8_t x);
void ld_i_addr16(XOChip* xochip, uint16_t addr);
void plane_n(XOChip* xochip, uint8_t n);
void audio(XOChip* xochip);
void pitch_vx(XOChip* xochip, uint8_t x);
void scrollup_n(XOChip* xochip, uint8_t n);

#endif

