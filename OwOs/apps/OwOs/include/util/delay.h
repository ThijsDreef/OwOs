#ifndef __OWOS_DELAY
#define __OWOS_DELAY

#include <os/os.h>

inline void delayMillis(uint16_t ms) { os_time_delay(ms * (OS_TICKS_PER_SEC / 1000.0)); }

#endif