#include "devices/st7789.h"
#include "resources/font.c"
#include <hal/hal_gpio.h>
#include <hal/hal_spi.h>
#include <os/os.h>
#include <os/os_dev.h>
#include <assert.h>
#include "util/delay.h"
#include <bus/bus.h>


// Bible for this driver : https://www.newhavendisplay.com/appnotes/datasheets/LCDs/ST7789V.pdf
#define DISPLAY_SPI   0         //  Mynewt SPI port 0
#define DISPLAY_CS   25         //  LCD_CS (P0.25): Chip select
#define DISPLAY_DC   18         //  LCD_RS (P0.18): Clock/data pin (CD)
#define DISPLAY_RST  26         //  LCD_RESET (P0.26): Display reset
#define DISPLAY_BACK  23        //  P0.23 LCD_BACKLIGHT_HIGH   OUT */

#define ST_CMD_DELAY   0x80     //  special signifier for command lists

#define NOP     0x00            //
#define SWRESET 0x01            //
#define RDDID   0x04            //
#define RDDST   0x09            //

#define SLPIN   0x10            //
#define SLPOUT  0x11            //
#define PTLON   0x12            //
#define NORON   0x13            //

#define INVOFF  0x20            //
#define INVON   0x21            //
#define DISPOFF 0x28            //
#define DISPON  0x29            //
#define CASET   0x2A            //  page 198 of the bible
#define RASET   0x2B            //  page 200 of the bible
#define RAMWR   0x2C            //
#define RAMRD   0x2E            //

#define PTLAR   0x30            //
#define COLMOD  0x3A            //  page 224 of the bible
#define MADCTL  0x36            //  page 215 of the bible

#define MADCTL_MY  0x80         //
#define MADCTL_MX  0x40         //
#define MADCTL_MV  0x20         //
#define MADCTL_ML  0x10         //
#define MADCTL_RGB 0x00         //

#define VSCSAD  0x37            //
#define VSCRDEF 0x33            //

struct os_dev* display_device = NULL;
uint8_t colset[4];
uint8_t rowset[4];


uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void hardReset() {
    hal_gpio_write(DISPLAY_RST, 1);
    delayMillis(10);
    hal_gpio_write(DISPLAY_RST, 0);
    delayMillis(10);
    hal_gpio_write(DISPLAY_RST, 1);
    delayMillis(10);
}

void disp_cmd(uint8_t cmd, const uint8_t* data, uint16_t len) {
    if (cmd) {
        hal_gpio_write(DISPLAY_DC, 0);
        assert(0 == bus_node_write(display_device, &cmd, 1, BUS_NODE_LOCK_DEFAULT_TIMEOUT, BUS_F_NONE));
    }
    if (len > 0) {
        hal_gpio_write(DISPLAY_DC, 1);
        assert(0 == bus_node_write(display_device, (void*)data, len, BUS_NODE_LOCK_DEFAULT_TIMEOUT, BUS_F_NONE));
    }
}

void setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    // set columns
    colset[0] = x0 >> 8;
    colset[1] = x0 & 0xff;
    colset[2] = x1 >> 8;
    colset[3] = x1 & 0xff;
    disp_cmd(CASET, colset, 4);

    // set rows
    rowset[0] = y0 >> 8;
    rowset[1] = y0 & 0xff;
    rowset[2] = y1 >> 8;
    rowset[3] = y1 & 0xff;
    disp_cmd(RASET, rowset, 4);
}

void fillScreen(uint16_t color) {
    uint16_t* buffer = os_malloc(480 * 20);
    for (int i = 0; i < 240 * 20; i++) buffer[i] = color;
    setAddressWindow(0, 0, 240, 240);
    disp_cmd(RAMWR, NULL, 0);
    for (int y = 0; y < 24; y++) {
        disp_cmd(0, (uint8_t*)buffer, 9600);
    }   
    os_free(buffer);
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    uint16_t* buffer = os_malloc(w * h* 2);
    for (int i = 0; i < w * h; i++) buffer[i] = color;
    setAddressWindow(x, y, x + w - 1, y + h - 1);
    disp_cmd(RAMWR, NULL, 0);
    disp_cmd(0, (uint8_t*)buffer, w*h*2);
    os_free(buffer);
}

void drawCharacter(uint16_t x, uint16_t y, char character, uint16_t color) {
    if (character == 32) {
        fillRect(x, y, 16, 16, 0x0);
        return;
    }
    character -= 33;
    if (character <= 0 || character > 96) return;
    uint16_t* buffer = os_malloc(16 * 16 * 2);
    for (int i = 0; i < 256; i++) buffer[i] = ((fontData[character / 32][i] >> (character % 32)) & 1) * color;
    setAddressWindow(x, y, x + 15, y + 15);
    disp_cmd(RAMWR, NULL, 0);
    disp_cmd(0, (uint8_t*)buffer, 16*16*2);
    os_free(buffer);
}

void drawString(char* characters, uint16_t x, uint16_t y) {
    while (*characters) {
        drawCharacter(x * 16, y * 16, *characters, color565(255, 255, 255));
        x ++;
        if (x > 14) {
            y++;
            x = 0;
        }
        characters++;
    }
}

void resetDisplay() {
    hardReset();
    disp_cmd(SWRESET, NULL, 0);
    delayMillis(200);
    disp_cmd(SLPOUT, NULL, 0);
    delayMillis(200);

    // set color mode
    static const uint8_t color_mode[] = {0x55};
    disp_cmd(COLMOD, color_mode, 1);

    // set memory data acces control
    static const uint8_t madctl[] = {0x00};
    disp_cmd(MADCTL,madctl, 1);

    disp_cmd(INVON, NULL, 0);
    delayMillis(10);
    disp_cmd(DISPON, NULL, 0);
    delayMillis(10);
}

void initDisplay() {
    display_device = os_dev_open("spidisplay", 0, NULL);
    assert (display_device != NULL);
    hal_gpio_init_out(DISPLAY_RST, 1);
    hal_gpio_init_out(DISPLAY_DC, 0);

    resetDisplay();
}


