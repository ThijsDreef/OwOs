#ifndef __OWOS_CHARGE
#define __OWOS_CHARGE

#include <inttypes.h>

void initCharge();
uint8_t getBatteryPercentage();
int getBatteryVoltageMv();

#endif