#include <inttypes.h>

uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
void fillScreen(uint16_t color);
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void drawCharacter(uint16_t x, uint16_t y, char character, uint16_t color);
void drawString(char* characters, uint16_t x, uint16_t y);
void resetDisplay();
void initDisplay();