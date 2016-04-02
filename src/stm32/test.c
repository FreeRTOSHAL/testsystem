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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <gpio.h>
#include <devs.h>
#include <iomux.h>
#include <uart.h>
#include <newlib_stub.h>
#include <nlibc_stub.h>
#include <irq.h>
#include <timer.h>
#include <pwm.h>
#include <timertest.h>

#ifdef CONFIG_GPIO
static struct gpio *gpio = NULL;

static struct gpio_pin *ledPin = NULL;
static struct gpio_pin *userButton = NULL;
bool nucleo_userButtonISR(struct gpio_pin *pin, uint8_t pinID, void *data) {
	(void) pin;
	(void) pinID;
	(void) data;
	printf("User Button pressed\n");
	return false;
}

int32_t initGPIO() {
	struct gpio *gpio2;
	gpio = gpio_init(GPIO_ID);
	if (gpio == NULL) {
		return -1;
	}
	/* Test if GET Dev work */
	gpio2  = gpio_init(1);
	if (gpio2 != NULL) {
		return -1;
	}
	userButton = gpioPin_init(gpio, PTC13, GPIO_INPUT, GPIO_OPEN);
	if (userButton == NULL) {
		return -1;
	}
	{
		int32_t ret;
		ret = gpioPin_setCallback(userButton, nucleo_userButtonISR, NULL, GPIO_EITHER);
		if (ret < 0) {
			return -1;
		}
		ret = gpioPin_enableInterrupt(userButton);
		if (ret < 0) {
			return -1;
		}
	}
	ledPin = gpioPin_init(gpio, PTA5, GPIO_OUTPUT, GPIO_PULL_UP);
	if (ledPin == NULL) {
		return -1;
	}
	
	return 0;
}
#endif

void vApplicationMallocFailedHook( void ) {
	CONFIG_ASSERT(0);
}

void vApplicationTickHook() {

}

void vApplicationStackOverflowHook() {
	CONFIG_ASSERT(0);
}

void vApplicationIdleHook( void ) {
}

#if defined(CONFIG_GPIO) || defined(CONFIG_PWM)
void ledTask(void *data) {
	bool up = true;
	uint64_t n = 10000;
# ifdef CONFIG_PWM
	struct pwm *pwm = data;
	int32_t ret;
# endif
	TickType_t waittime = 20;
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	for(;;) {
		if (up) {
			if (n >= (20000 - 400)) {
				up = false;
			}
			n+=400;
		} else {
			if (n <= 400) {
				up = true;
			}
			n-=400;
		}
# ifdef CONFIG_PWM
		ret = pwm_setDutyCycle(pwm, n);
		CONFIG_ASSERT(ret == 0);
# endif
		if (n == 0 || n == 20000) {
			waittime = 1000;
# ifdef CONFIG_GPIO
			gpioPin_togglePin(ledPin);
# endif
		} else {
			waittime = 20;
		}
		vTaskDelayUntil(&lastWakeUpTime, waittime / portTICK_PERIOD_MS);
	}
}
#endif

#ifdef CONFIG_USE_STATS_FORMATTING_FUNCTIONS
void taskManTask(void *data) {
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	static char taskBuff[5 * 1024];
	for(;;) {
		vTaskList(taskBuff);
		printf("name\t\tState\tPrio\tStack\tTaskNr.\n");
		printf("%s", taskBuff);
		printf("blocked ('B'), ready ('R'), deleted ('D') or suspended ('S')\n");
#ifdef CONFIG_GENERATE_RUN_TIME_STATS
		printf("name\t\tTime\t\t%%\n");
		vTaskGetRunTimeStats(taskBuff);
		printf("%s", taskBuff);
#endif
		printf("\n");
		vTaskDelayUntil(&lastWakeUpTime, 1000 / portTICK_PERIOD_MS);
	}
}
#endif

int main() {
	int32_t ret;
	ret = irq_init();
	CONFIG_ASSERT(ret == 0);
	struct pwm *pwm = NULL;
#ifdef CONFIG_UART
	struct uart *uart = uart_init(UART2_ID, 115200);
#endif
#ifdef CONFIG_NEWLIB
	ret = newlib_init(uart, uart);
	CONFIG_ASSERT(ret == 0);
#endif
#ifdef CONFIG_NLIBC_PRINTF
	ret = nlibc_init(uart, uart);
	CONFIG_ASSERT(ret == 0);
#endif
	printf("Init Devices\n");
#ifdef CONFIG_INSTANCE_NAME
	hal_printNames();
#endif
#ifdef CONFIG_GPIO
	ret = initGPIO();
	CONFIG_ASSERT(ret == 0);
#endif
#if defined(CONFIG_GPIO) || defined(CONFIG_PWM)
	xTaskCreate(ledTask, "LED Task", 128, pwm, 1, NULL);
#endif
#ifdef CONFIG_USE_STATS_FORMATTING_FUNCTIONS
	xTaskCreate(taskManTask, "Task Manager Task", 512, NULL, 1, NULL);
#endif
#ifdef CONFIG_TIMERTEST
	timertest_init();
#endif
	printf("Start Scheduler\n");
	vTaskStartScheduler ();
	for(;;);
	return 0;
}
