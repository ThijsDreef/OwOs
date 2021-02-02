#include "devices/cst8165.h"
#include <hal/hal_i2c.h>
#include <hal/hal_gpio.h>
#include <bsp/bsp.h>
#include "util/delay.h"

#define I2C_ADDRESS_TOUCH_CONTROLLER 0x15

#define TOUCH_X_HIGH 3
#define TOUCH_X_LOW 4
#define TOUCH_Y_HIGH 5
#define TOUCH_ID 5

#define TOUCH_Y_LOW 6
#define GESTURE_IDNEX 1
#define TOUCH_EVENT_INDEX 3
#define TOUCH_STRIDE 6

struct hal_i2c_master_data data = {
    .address = I2C_ADDRESS_TOUCH_CONTROLLER,
    .len = 1,
    .buffer = NULL
};

static uint8_t pointerBuffer[63];
static uint8_t registerBuffer[2];

void readRegister(uint8_t address, uint8_t reg) {
    registerBuffer[0] = reg;
    data.len = 1;
    data.address = address;
    data.buffer = (void*)&registerBuffer;
    hal_i2c_master_write(1, &data, 5000, 0);

    registerBuffer[0] = 0x0;
    hal_i2c_master_read(1, &data, 5000, 1);
}

void readRegisterRange(uint8_t address, uint8_t reg, uint8_t amount, uint8_t* buffer) {
    registerBuffer[0] = reg;
    data.len = 1;
    data.address = address;
    data.buffer = (void*)&registerBuffer;
    hal_i2c_master_write(1, &data, 5000, 0);

    registerBuffer[0] = 0;
    data.len = amount;
    data.buffer = (void*)buffer;
    data.address = address;
    hal_i2c_master_read(1, &data, 5000, 1);
}

void initTouch() {
    hal_gpio_init_out(TOUCH_RESET_PIN, 1);
    delayMillis(50);
    hal_gpio_write(TOUCH_RESET_PIN, 0);
    delayMillis(5);
    hal_gpio_write(TOUCH_RESET_PIN, 1);
    delayMillis(50);
    // readRegister(0x15, 0x15);
    // readRegister(0x15, 0xa7);
}

void sleepTouch() {
    hal_gpio_write(TOUCH_RESET_PIN, 0);
    delayMillis(5);
    hal_gpio_write(TOUCH_RESET_PIN, 1);
    delayMillis(50);
    // register
    registerBuffer[0] = 0xA5;
    // data
    registerBuffer[1] = 0x03;

    data.len = 2;
    data.address = 0x15;
    data.buffer = (void*)&registerBuffer;
    hal_i2c_master_write(1, &data, 1000, 1);
}

TouchInfo_t getTouchInfo() {
    // read all pointer info
    // for now we just check the first
    readRegisterRange(0x15, 0, 63, pointerBuffer);

    TouchInfo_t result;
    result.x = ((pointerBuffer[TOUCH_X_HIGH] & 0xf) << 8) | pointerBuffer[TOUCH_X_LOW];
    result.y = ((pointerBuffer[TOUCH_Y_HIGH] & 0xf) << 8) | pointerBuffer[TOUCH_Y_LOW];
    result.action = pointerBuffer[TOUCH_EVENT_INDEX] >> 6;
    result.gesture = pointerBuffer[GESTURE_IDNEX];
    result.finger = pointerBuffer[TOUCH_ID] >> 4;

    return result;

}