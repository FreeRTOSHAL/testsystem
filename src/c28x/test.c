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
#include <file.h>
#include <uart.h>
#include <devs.h>
#include <irq.h>
# define PRINTF(...) printf(__VA_ARGS__)

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

void userErrorHandler(void) {

}
struct uart *stdio;
int stdio_dopen(const char *path, unsigned flags, int llv_fd) {
	return 0;
}
int stdio_dclose(int dev_fd) {
	return 0;
}
int stdio_dread(int dev_fd, char *buf, unsigned count) {
	return uart_read(stdio, buf, count, portMAX_DELAY);
}

int stdio_dwrite(int dev_fd, const char *buf, unsigned count) {
	return uart_write(stdio, buf, count, portMAX_DELAY);
}
off_t stdio_dlseek(int dev_fd, off_t offset, int origin) {
	return 0;
}
int stdio_dunlink(const char *path) {
	return 0;
}
int stdio_drename(const char *old_name, const char *new_name) {
	return 0;
}

void testTask(void *data) {
	int i = 0;
	TickType_t pxPreviousWakeTime = xTaskGetTickCount();
	for (;;) {
		PRINTF("%d %lu\n", i++, pxPreviousWakeTime);
		vTaskDelayUntil(&pxPreviousWakeTime, 100 / portTICK_PERIOD_MS);
	}
}

OS_DEFINE_TASK(test, 500);

int main() {
	int32_t ret;
	ret = irq_init();
	CONFIG_ASSERT(ret == 0);
	stdio = uart_init(SCI0_ID, 115200);
	CONFIG_ASSERT(stdio);
	ret = add_device("uart", _SSA,
		stdio_dopen,
		stdio_dclose,
		stdio_dread,
		stdio_dwrite,
		stdio_dlseek,
		stdio_dunlink,
		stdio_drename
	);
	CONFIG_ASSERT(ret >= 0);
	FILE *fid = fopen("uart", "w");
	freopen("uart:", "w", stdout);
	setvbuf(stdout, NULL, _IONBF, 0);

	ret = OS_CREATE_TASK(testTask, "Test Task", 500, NULL, 2, test);
	CONFIG_ASSERT(ret == pdPASS);

	PRINTF("Start Scheduler\n");
	vTaskStartScheduler ();
	for(;;);
	return 0;
}

