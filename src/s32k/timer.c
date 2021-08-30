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
#include <devs.h>
#include <pwm.h>
#include <capture.h>
#include <adctest.h>

#define MAX_TIME 21000
struct timer *timer;
#ifndef CONFIG_TIMER_TEST_PWM
struct gpio_pin **rgb;
#else
struct pwm *pwm[3];
#endif
#ifdef CONFIG_CAPTURE_TEST
struct capture *capture;
#endif
uint32_t rgbPins = BIT(0);
uint32_t state = 0x0;
float n = MAX_TIME / 2;
uint32_t m;
bool up = true;
bool on = true;

static bool irqhandle(struct timer *timer, void *data) {
	float tmp;
	(void) data;
#ifndef CONFIG_TIMER_TEST_PWM
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
#endif
	if (on) {
		if (up) {
			n+=0.05;
		} else {
			n-=0.05;
		}
		if (n >= 1.0) {
			n = 1.0;
			up = false;
		} else if (n <= 0) {
			n = 0.0;
			up = true;
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
		}
	}
#ifndef CONFIG_TIMER_TEST_PWM
	m = MAX_TIME - (n * MAX_TIME);
# ifdef CONFIG_TIMER_TEST_ONESHOT
	if (on) {
		CONFIG_ASSERT(timer_oneshot(timer, m) == 0);
	} else {
		CONFIG_ASSERT(timer_oneshot(timer, MAX_TIME - m) == 0);
	}
# else
	if (on) {
		CONFIG_ASSERT(timer_periodic(timer, m) == 0);
	} else {
		CONFIG_ASSERT(timer_periodic(timer, MAX_TIME - m) == 0);
	}
# endif
	on = !on;
#else
		tmp = n;
		tmp *= 0.3;
#ifdef CONFIG_ADC_TEST
		tmp *= adc_val;
#endif
		m = MAX_TIME - (tmp * MAX_TIME);
	if (rgbPins & BIT(0)) {
		CONFIG_ASSERT(pwm_setDutyCycle(pwm[0], m) == 0);
	} else {
		CONFIG_ASSERT(pwm_setDutyCycle(pwm[0], MAX_TIME) == 0);
	}
	if (rgbPins & BIT(1)) {
		CONFIG_ASSERT(pwm_setDutyCycle(pwm[1], m) == 0);
	} else {
		CONFIG_ASSERT(pwm_setDutyCycle(pwm[1], MAX_TIME) == 0);
	}
	if (rgbPins & BIT(2)) {
		CONFIG_ASSERT(pwm_setDutyCycle(pwm[2], m) == 0);
	} else {
		CONFIG_ASSERT(pwm_setDutyCycle(pwm[2], MAX_TIME) == 0);
	}
#endif
	return false;
}

#ifdef CONFIG_CAPTURE_TEST
bool captureCallback(struct capture *capture, uint32_t index, uint64_t time, void *data) {
	double feq = 1 / (((double) time) / 1000000.);
	printf("Capture: %f Hz", feq);
	return false;
}
#endif

int32_t timertest_init(struct gpio_pin **rgbPins) {
	int32_t ret;
#ifndef CONFIG_TIMER_TEST_PWM
	rgb = rgbPins;
#else
	(void) rgbPins;
#endif
	timer = timer_init(FLEXTIMER0_ID, 128, MAX_TIME, 0);
	CONFIG_ASSERT(timer != NULL);
	ret = timer_setOverflowCallback(timer, &irqhandle, NULL);
	CONFIG_ASSERT(ret == 0);
#ifndef CONFIG_TIMER_TEST_PWM
	gpioPin_setPin(rgb[0]);
	gpioPin_setPin(rgb[1]);
	gpioPin_setPin(rgb[2]);
#else
	pwm[0] = pwm_init(FLEXTIMER0_PWM0_PTD15_ID);
	CONFIG_ASSERT(pwm[0]);
	pwm[1] = pwm_init(FLEXTIMER0_PWM1_PTD16_ID);
	CONFIG_ASSERT(pwm[1]);
	pwm[2] = pwm_init(FLEXTIMER0_PWM2_PTD0_ID);
	CONFIG_ASSERT(pwm[2]);
#endif
#ifndef CONFIG_TIMER_TEST_PWM
# ifdef CONFIG_TIMER_TEST_ONESHOT
	CONFIG_ASSERT(timer_oneshot(timer, n) == 0);
# else
	CONFIG_ASSERT(timer_periodic(timer, n) == 0);
# endif
#else
	CONFIG_ASSERT(pwm_setPeriod(pwm[0], MAX_TIME) == 0);
#endif
#ifdef CONFIG_CAPTURE_TEST
	capture = capture_init(FLEXTIMER0_CAPTURE3_PTC3_ID);
	CONFIG_ASSERT(capture);
	CONFIG_ASSERT(capture_setCallback(capture, captureCallback, NULL) == 0);
#endif
	return 0;
}

