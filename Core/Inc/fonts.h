#ifndef FONTS_H
#define FONTS_H

#include <stdint.h>

// Font arrays declared in Font_13.c and Font_19.c
extern const uint8_t Font_13p[2158];
extern const uint16_t Font_13p_info[159][2];
extern const uint8_t Font_19p[4256];
extern const uint16_t Font_19p_info[159][2];

// Font 1: 5x7 bitmap font (ASCII 32-126)
extern const uint8_t Font1[480];
// Font 1 scaled 2x: 10x14 bitmap font (ASCII 32-126)
extern const uint8_t Font1_2x[2688];

#endif
