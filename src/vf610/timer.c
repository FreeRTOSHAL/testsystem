/* SPDX-License-Identifier: MIT */
/*
 * Author: Andreas Werner <kernel@andy89.org>
 * Date: 2016
 */
#include <FreeRTOS.h>
#include <task.h>
#include <task.h>
#include <timer.h>
#include <gpio.h>

struct timer *timer;
struct gpio_pin *pin;
int n = 20000;
bool up = true;

static bool irqhandle(struct timer *timer, void *data) {
	(void) data;
	#if 1
	#ifndef CONFIG_ASSERT_DISABLED
		CONFIG_ASSERT(gpioPin_togglePin(pin) == 0);
	#else
		gpioPin_togglePin(pin);
	#endif
	#endif
	if (up) {
		n+=100;
	} else {
		n-=100;
	}
	if (n >= 1500) {
		up = false;
	} else if (n <= 500) {
		up = true;
	}
	CONFIG_ASSERT(timer_oneshot(timer, n) == 0);
	return false;
}
int32_t timerInit(struct gpio_pin *p) {
	int32_t ret;
	pin = p;
	timer = timer_init(0, 64, 20000, 700);
	CONFIG_ASSERT(timer != NULL);
	ret = timer_setOverflowCallback(timer, &irqhandle, NULL);
	CONFIG_ASSERT(ret == 0);
	CONFIG_ASSERT(timer_oneshot(timer, n) == 0);
	return 0;
}

