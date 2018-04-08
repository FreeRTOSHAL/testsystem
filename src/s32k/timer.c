/*
 * Copyright (c) 2016 Andreas Werner <kernel@andy89.org>
 * 
 * Permission is hereby granted, free of charge, to any person 
 * obtaining a copy of this software and associated documentation 
 * files (the "Software"), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, 
 * publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE.
 */
#include <FreeRTOS.h>
#include <task.h>
#include <task.h>
#include <timer.h>
#include <gpio.h>
#include <devs.h>

struct timer *timer;
struct gpio_pin **rgb;
uint32_t rgbPins = BIT(0);
uint32_t state = 0x0;
uint64_t n = 10000;
bool up = true;
bool on = true;

static bool irqhandle(struct timer *timer, void *data) {
	(void) data;
	if (on) {
		if (rgbPins & BIT(0)) {
			gpioPin_setPin(rgb[0]);
		}
		if (rgbPins & BIT(1)) {
			gpioPin_setPin(rgb[1]);
		}
		if (rgbPins & BIT(2)) {
			gpioPin_setPin(rgb[2]);
		}
	} else {
		if (rgbPins & BIT(0)) {
			gpioPin_clearPin(rgb[0]);
		}
		if (rgbPins & BIT(1)) {
			gpioPin_clearPin(rgb[1]);
		}
		if (rgbPins & BIT(2)) {
			gpioPin_clearPin(rgb[2]);
		}
	}
	if (on) {
		if (up) {
			n+=100;
		} else {
			n-=100;
		}
		if (n >= 19800) {
			printf("down\n");
			up = false;
			if (state == 7) {
				state = 0;
			}
			switch (state++) {
				case 0:
					rgbPins = BIT(0);
					break;
				case 1:
					rgbPins = BIT(1);
					break;
				case 2:
					rgbPins = BIT(2);
					break;
				case 3:
					rgbPins = BIT(0) | BIT(1);
					break;
				case 4:
					rgbPins = BIT(1) | BIT(2);
					break;
				case 5:
					rgbPins = BIT(2) | BIT(0);
					break;
				case 6:
					rgbPins = BIT(0) | BIT(1) | BIT(2);
					break;
			}
		} else if (n <= 16000) {
		/*} else if (n <= 10000) {
		} else if (n <= 100) {*/
			printf("up\n");
			up = true;
		}
	}
#ifdef CONFIG_TIMER_TEST_ONESHOT
	if (on) {
		CONFIG_ASSERT(timer_oneshot(timer, n) == 0);
	} else {
		CONFIG_ASSERT(timer_oneshot(timer, 20000 - n) == 0);
	}
#else
	if (on) {
		CONFIG_ASSERT(timer_periodic(timer, n) == 0);
	} else {
		CONFIG_ASSERT(timer_periodic(timer, 20000 - n) == 0);
	}
#endif
	on = !on;
	return false;
}
int32_t timertest_init(struct gpio_pin **rgbPins) {
	int32_t ret;
	rgb = rgbPins;
	timer = timer_init(FLEXTIMER0_ID, 128, 20000, 700);
	CONFIG_ASSERT(timer != NULL);
	ret = timer_setOverflowCallback(timer, &irqhandle, NULL);
	CONFIG_ASSERT(ret == 0);
	gpioPin_setPin(rgb[0]);
	gpioPin_setPin(rgb[1]);
	gpioPin_setPin(rgb[2]);
#ifdef CONFIG_TIMER_TEST_ONESHOT
	CONFIG_ASSERT(timer_oneshot(timer, n) == 0);
#else
	CONFIG_ASSERT(timer_periodic(timer, n) == 0);
#endif
	return 0;
}

