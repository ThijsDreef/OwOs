#ifndef __OWOS_FONT
#define __OWOS_FONT

#include <inttypes.h>

struct character {
    uint8_t page;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint8_t xAdvance;
    uint8_t yOffset;
} typedef character_t;

struct kerning {
    uint8_t first;
    uint8_t second;
    uint8_t amount;
} typedef kerning_t;

const character_t* getCharacter(char character);
const kerning_t* getKerning(char first, char second);
const void* getFontPage(int page);

#endif