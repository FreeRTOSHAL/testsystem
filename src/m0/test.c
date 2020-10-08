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
#include <semihosting.h>
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
		/*PRINTF("name\t\tTime\t\t%%\n");
		vTaskGetRunTimeStats(taskBuff);
		PRINTF("%s", taskBuff);*/
#endif
		PRINTF("\n");
		vTaskDelayUntil(&lastWakeUpTime, 1000 / portTICK_PERIOD_MS);
	}
}
#endif

void testTask(void *data) {
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	for(;;) {
		PRINTF("Test %ld\n", lastWakeUpTime);
		vTaskDelayUntil(&lastWakeUpTime, 10 / portTICK_PERIOD_MS);
	}
}

static int global = 0x4242;
static int globalBSS;

OS_DEFINE_TASK(taskMan, 512);
OS_DEFINE_TASK(test, 512);
int main() {
	int32_t ret;
	ret = irq_init();
	CONFIG_ASSERT(ret == 0);
#ifdef CONFIG_UART
	struct uart *uart = uart_init(0, 115200);
#endif
#ifdef CONFIG_NEWLIB
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
#ifdef CONFIG_USE_STATS_FORMATTING_FUNCTIONS
	OS_CREATE_TASK(taskManTask, "Task Manager Task", 512, NULL, 1, taskMan);
#endif
	OS_CREATE_TASK(testTask, "Test Task", 512, NULL, 1, test);
	CONFIG_ASSERT(global == 0x4242);
	CONFIG_ASSERT(globalBSS == 0x0);
	PRINTF("Start Scheduler\n");
	vTaskStartScheduler ();
	for(;;);
	return 0;
}
