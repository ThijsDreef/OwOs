#include <inttypes.h>

enum Gestures {
    None = 0x0,
    SlideDown = 0x01,
    SlideUp = 0x02,
    SlideLeft = 0x03,
    SlideRight = 0x04,
    SingleTap = 0x05,
    DoubleTap = 0x0b,
    longPress = 0x0c
} typedef Gestures_t;

struct TouchInfo {
    uint16_t x;
    uint16_t y;
    uint8_t action;
    uint8_t finger;
    Gestures_t gesture;
} typedef TouchInfo_t;

void initTouch();
void sleepTouch();
TouchInfo_t getTouchInfo();