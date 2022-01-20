#ifndef FONTSETS
#define FONTSETS

#include <stdint.h>

#define CHIP8_FONTSET_BYTES_PER_CHAR 5
#define CHIP8_FONTSET_NDIGITS 16
#define CHIP8_FONTSET_SIZE CHIP8_FONTSET_NDIGITS*CHIP8_FONTSET_BYTES_PER_CHAR

#define SCHIP_FONTSET_BYTES_PER_CHAR 10
#define SCHIP_FONTSET_NDIGITS 16
#define SCHIP_FONTSET_SIZE SCHIP_FONTSET_NDIGITS*SCHIP_FONTSET_BYTES_PER_CHAR

uint8_t chip8_character(uint8_t index);
uint8_t schip_character(uint8_t index);

#endif