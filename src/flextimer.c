#include <FreeRTOS.h>
#include <task.h>
#include <task.h>
#include <flextimer.h>
#include <gpio.h>

struct ftm *ftm;
struct gpio_pin *pin;
int n = 20000;
bool up = true;

static void irqhandle(struct ftm *ftm, void *data) {
	(void) data;
	#if 1
	#ifndef CONFIG_ASSERT_DISABLED
		CONFIG_ASSERT(gpioPin_togglePin(pin) == 0);
	#else
		gpioPin_togglePin(pin);
	#endif
	#endif
	/*if (up) {
		n+=100;
	} else {
		n-=100;
	}
	if (n >= 1500) {
		up = false;
	} else if (n <= 500) {
		up = true;
	}*/
	CONFIG_ASSERT(ftm_oneshot(ftm, n) == 0);
	
}
int32_t ftmInit(struct gpio_pin *p) {
	int32_t ret;
	pin = p;
	ftm = ftm_init(0, 64, 20000, 700);
	CONFIG_ASSERT(ftm != NULL);
	ret = ftm_setOverflowHandler(ftm, &irqhandle, NULL);
	CONFIG_ASSERT(ret == 0);
	CONFIG_ASSERT(ftm_oneshot(ftm, n) == 0);

	return 0;
}

