#include "os/os.h"
#include "bsp/bsp.h"
#include "sysinit/sysinit.h"
#include "hal/hal_gpio.h"
#include "datetime/datetime.h"
#include "devices/charge.h"
#include "devices/cst8165.h"
#include "devices/st7789.h"
#include "ble/ble.h"

bool stateSwitch = false;
bool currentState = true;
bool isDown = false;
TouchInfo_t touch;
TouchInfo_t lastTouch;

os_stack_t buttonStack[32];
os_stack_t drawTaskStack[256];
os_stack_t touchStack[128];

struct os_task drawTask;
struct os_task buttonTask;
struct os_task touchTask;


static void buttonIrq(void *arg) {
    stateSwitch = true;
}

static void touchIrq(void *arg) {
    isDown = true;
}

void stateChangerTask() {
    while(1) {
        if (stateSwitch) {
            stateSwitch = false;
            currentState = !currentState;
            hal_gpio_write(LCD_BACKLIGHT_HIGH_PIN, currentState);
        }
        os_time_delay(OS_TICKS_PER_SEC / 2);
    }
}

void touchHandlerTask() {
    while(1) {
        if (isDown) {
            touch = getTouchInfo();
            isDown = touch.action == 2;
        }
        os_time_delay(OS_TICKS_PER_SEC / 60);
    }
}

void drawScreenTask(void* args) {
    initDisplay();
    fillScreen(0);

    struct os_timeval currentTime;
    struct os_timezone currentTimeZone;
    char characters[30];

    while (1) {
        if (!currentState) {
    

            os_gettimeofday(&currentTime, &currentTimeZone);
            struct clocktime ct;
            timeval_to_clocktime(&currentTime, &currentTimeZone, &ct);

            // year month day
            sprintf(characters, "%d/%02d/%02d ", ct.year, ct.mon, ct.day);
            drawString(characters, 40, 240 / 2 + 1);
            // hh:mm:ss
            sprintf(characters, "%02d:%02d:%02d ", ct.hour, ct.min, ct.sec);
            drawString(characters, 60, 240 / 2 - 36);

            fillRect(20, 119, 200, 2, color565(200, 200, 200));

            int x, y, w, h, lineWidth;
            x = 10;
            y = 10;
            w = 40;
            h = 15;
            lineWidth = 2;
            // topleft to topright
            fillRect(x, y, w, lineWidth, color565(255, 255, 255));
            // topright to bottomright
            fillRect(x + w, y, lineWidth, h, color565(255, 255, 255));
            // bottomleft to bottomright
            fillRect(x, y + h, w + lineWidth, lineWidth, color565(255, 255, 255));
            // top left to bottomleft
            fillRect(x, y, lineWidth, h, color565(255, 255, 255));


            int greenbarWidth = (w - lineWidth - 2);
            float percentageFilled = greenbarWidth * (getBatteryPercentage() / 100.0);
            // bar for enery
            fillRect(x + lineWidth + 1, y + lineWidth + 1, percentageFilled, h - lineWidth - 2, color565(0, 0, 255));
            fillRect(x + lineWidth + 1 + percentageFilled, y + lineWidth + 1, greenbarWidth - percentageFilled, h - lineWidth - 2, 0);



            
       }

        os_time_delay(OS_TICKS_PER_SEC / 24);
    }
}

void gpioInit() {
    hal_gpio_irq_init(PUSH_BUTTON_IN_PIN, buttonIrq, NULL, HAL_GPIO_TRIG_FALLING, HAL_GPIO_PULL_DOWN);
    hal_gpio_irq_enable(PUSH_BUTTON_IN_PIN);

    hal_gpio_irq_init(TOUCH_INT_PIN, touchIrq, NULL, HAL_GPIO_TRIG_RISING, HAL_GPIO_PULL_UP);
    hal_gpio_irq_enable(TOUCH_INT_PIN);

    hal_gpio_init_out(PUSH_BUTTON_OUT_PIN, 1);
    hal_gpio_init_out(LCD_BACKLIGHT_HIGH_PIN, 1);
}

void taskInit() {
    os_task_init(&drawTask, "draw task", drawScreenTask, NULL, 130, OS_WAIT_FOREVER, drawTaskStack, 256);
    os_task_init(&buttonTask, "state task", stateChangerTask, NULL, 128, OS_WAIT_FOREVER, buttonStack, 32);
    os_task_init(&touchTask, "touch task", touchHandlerTask, NULL, 131, OS_WAIT_FOREVER, touchStack, 128);
}

int main() {

	sysinit();

	gpioInit();
    taskInit();
    initTouch();
    initCharge();
    initBle();

	while (1) {
		os_eventq_run(os_eventq_dflt_get());
	}
	return 0;
}