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
#include <rproctest.h>
#include <mailbox_test.h>
#include <boardtest.h>
#include <remoteproc_trace.h>
#include <printClockTree.h>
#include <spitest.h>
#include <timertest.h>
#include <mputest.h>
#if defined(CONFIG_NEWLIB_UART) || defined(CONFIG_NLIBC_PRINTF)
# define PRINTF(...) printf(__VA_ARGS__)
#else
# define PRINTF(...) 
#endif

#ifdef CONFIG_GPIO
static struct gpio *gpio = NULL;

static struct gpio_pin *ledPin = NULL;

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
	/* Top LED GPIO2_7 error in Beagbeboard Schmatics GPIO2_17 is GPIO2_7 */
	ledPin = gpioPin_init(gpio, PAD_GPMC_A17, GPIO_OUTPUT, GPIO_PULL_UP);
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

#if defined(CONFIG_GPIO) && defined(CONFIG_INCLUDE_vTaskDelayUntil)
void ledTask(void *data) {
	bool up = true;
	uint64_t n = 10000;
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
		if (n == 0 || n == 20000) {
			waittime = 1000;
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
		PRINTF("name\t\tState\tPrio\tStack\tTaskNr.\n");
		PRINTF("%s", taskBuff);
		PRINTF("blocked ('B'), ready ('R'), deleted ('D') or suspended ('S')\n");
#ifdef CONFIG_GENERATE_RUN_TIME_STATS
		PRINTF("name\t\tTime\t\t%%\n");
		vTaskGetRunTimeStats(taskBuff);
		PRINTF("%s", taskBuff);
#endif
		PRINTF("\n");
		vTaskDelayUntil(&lastWakeUpTime, 1000 / portTICK_PERIOD_MS);
	}
}
#endif

int main() {
	int32_t ret;
	ret = irq_init();
	CONFIG_ASSERT(ret == 0);
#if defined(CONFIG_GPIO) && defined(CONFIG_INCLUDE_vTaskDelayUntil)
	struct pwm *pwm = NULL;
#endif
#ifdef CONFIG_UART
	struct uart *uart = uart_init(REMOTEPROC_TRACE_ID(0), 115200);
#endif
#ifdef CONFIG_NEWLIB_UART
	ret = newlib_init(uart, uart);
	CONFIG_ASSERT(ret == 0);
#endif
#ifdef CONFIG_NLIBC_PRINTF
	ret = nlibc_init(uart, uart);
	CONFIG_ASSERT(ret == 0);
#endif
	PRINTF("Init Devices\n");
#ifdef CONFIG_INSTANCE_NAME
	hal_printNames();
#endif
#ifdef CONFIG_PRINT_CLOCK_TREE
	printClockTree();
#endif
#ifdef CONFIG_GPIO
	ret = initGPIO();
	CONFIG_ASSERT(ret == 0);
#endif
#if defined(CONFIG_GPIO) && defined(CONFIG_INCLUDE_vTaskDelayUntil)
	xTaskCreate(ledTask, "LED Task", 128, pwm, 1, NULL);
#endif
#ifdef CONFIG_USE_STATS_FORMATTING_FUNCTIONS
	xTaskCreate(taskManTask, "Task Manager Task", 512, NULL, 1, NULL);
#endif
#ifdef CONFIG_MAILBOX_TEST
	mailbox_test();
#endif
#ifdef CONFIG_RPROC_TEST
	rprocTest_init();
#endif
#ifdef CONFIG_BOARD_TEST
	boardtest_init();
#endif
#ifdef CONFIG_SPI_TEST
	PRINTF("init SPI Test\n");
	spitest_init();
#endif
#ifdef CONFIG_TIMER_TEST
	timertest_init();
#endif
#ifdef CONFIG_MPU_TEST
	mputest_init();
#endif
	PRINTF("Start Scheduler\n");
	vTaskStartScheduler();
	for(;;);
	return 0;
}
