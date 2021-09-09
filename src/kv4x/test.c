/* SPDX-License-Identifier: MIT */
/*
 * Author: Andreas Werner <kernel@andy89.org>
 * Date: 2016
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
#include <semihosting.h>
#include <cantest.h>
#include <timertest.h>
#if defined(CONFIG_NEWLIB) || defined(CONFIG_NLIBC_PRINTF)
# define PRINTF(...) printf(__VA_ARGS__)
#else
# define PRINTF(...) 
#endif

void vApplicationMallocFailedHook( void ) {
	CONFIG_ASSERT(0);
}

void vApplicationTickHook() {

}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName ) {
	CONFIG_ASSERT(0);
}

void vApplicationIdleHook( void ) {
}

#if defined(CONFIG_GPIO) && defined(CONFIG_INCLUDE_xTaskDelayUntil)
static struct gpio *gpio = NULL;
static struct gpio_pin *ledRGBPin[3];
int32_t initGPIO() {
	gpio = gpio_init(GPIO_ID);
	if (gpio == NULL) {
		return -1;
	}
	ledRGBPin[0] = gpioPin_init(gpio, PTA4, GPIO_OUTPUT, GPIO_PULL_UP); // R
	if (ledRGBPin[0] == NULL) {
		return -1;
	}
	ledRGBPin[1] = gpioPin_init(gpio, PTA5, GPIO_OUTPUT, GPIO_PULL_UP); // B
	if (ledRGBPin[1] == NULL) {
		return -1;
	}
	ledRGBPin[2] = gpioPin_init(gpio, PTA12, GPIO_OUTPUT, GPIO_PULL_UP); // G
	if (ledRGBPin[2] == NULL) {
		return -1;
	}
	return 0;
}
#ifndef CONFIG_TIMER_KV4X_TEST
void ledTask(void *data) {
	TickType_t waittime = 1000;
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	if (ledRGBPin[0]) {
		gpioPin_setPin(ledRGBPin[0]);
		gpioPin_clearPin(ledRGBPin[1]);
		gpioPin_clearPin(ledRGBPin[2]);
	}
	for(;;) {
		if (ledRGBPin[0]) {
			gpioPin_togglePin(ledRGBPin[0]);
			gpioPin_togglePin(ledRGBPin[1]);
			gpioPin_togglePin(ledRGBPin[2]);
		}
		xTaskDelayUntil(&lastWakeUpTime, 1000 / portTICK_PERIOD_MS);
	}
}
#endif
#endif


#ifdef CONFIG_USE_STATS_FORMATTING_FUNCTIONS
static char taskBuff[1 * 1024];
void taskManTask(void *data) {
	TickType_t lastWakeUpTime = xTaskGetTickCount();
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

OS_DEFINE_TASK(taskLED, 128);
OS_DEFINE_TASK(taskMan, 512);
int main() {
	int32_t ret;
	ret = irq_init();
	CONFIG_ASSERT(ret == 0);
#ifdef CONFIG_UART
# ifdef CONFIG_KV4X_STDOUT_SEMIHOSTING
	struct uart *uart = uart_init(SEMIHOSTING_UART_ID, 115200);
# endif
	CONFIG_ASSERT(uart != NULL);
# ifdef CONFIG_NEWLIB
	ret = newlib_init(uart, uart);
	CONFIG_ASSERT(ret == 0);
# endif
# ifdef CONFIG_NLIBC_PRINTF
	ret = nlibc_init(uart, uart);
	CONFIG_ASSERT(ret == 0);
# endif
#endif
	PRINTF("Init Devices\n");
#ifdef CONFIG_INSTANCE_NAME
	hal_printNames();
#endif
#if defined(CONFIG_GPIO) && defined(CONFIG_INCLUDE_xTaskDelayUntil)
	ret = initGPIO();
	CONFIG_ASSERT(ret == 0);
# ifndef CONFIG_TIMER_KV4X_TEST
	OS_CREATE_TASK(ledTask, "LED Task", 128, NULL, 1, taskLED);
# endif
#endif
#ifdef CONFIG_USE_STATS_FORMATTING_FUNCTIONS
	OS_CREATE_TASK(taskManTask, "Task Manager Task", 512, NULL, 1, taskMan);
#endif
#ifdef CONFIG_CAN_TEST
	can_test();
#endif
#ifdef CONFIG_TIMER_KV4X_TEST
	timertest_init(ledRGBPin);
#endif
	PRINTF("Start Scheduler\n");
	vTaskStartScheduler ();
	for(;;);
	return 0;
}
