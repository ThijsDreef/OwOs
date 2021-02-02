#include "os/os.h"
#include "bsp/bsp.h"
#include "sysinit/sysinit.h"
#include "hal/hal_gpio.h"

int main() {

	sysinit();
	while (1) {
		os_eventq_run(os_eventq_dflt_get());
	}
	return 0;
}