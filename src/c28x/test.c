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
#include <gpio.h>
#include <iomux.h>
#include <can_test.h>

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
	int32_t ret;
	int i = 0;
	for (i = 0; i < count; i++) {
		if (*buf == '\n') {
			ret = uart_putc(stdio, '\r', portMAX_DELAY);
			if (ret < 0) {
				return -1;
			}
		}
		ret = uart_putc(stdio, *buf, portMAX_DELAY);
		if (ret < 0) {
			return -1;
		}
		buf++;
	}
	return i;
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
	struct gpio *gpio = gpio_init(GPIO_ID);
	int i = 0;
	struct gpio_pin *red = gpioPin_init(gpio, GPIO_50, GPIO_OUTPUT, GPIO_PULL_UP);
	struct gpio_pin *green = gpioPin_init(gpio, GPIO_29, GPIO_OUTPUT, GPIO_PULL_UP);
	CONFIG_ASSERT(red);
	CONFIG_ASSERT(green);
	gpioPin_setPin(red);
	gpioPin_clearPin(green);
	TickType_t pxPreviousWakeTime = xTaskGetTickCount();
	for (;;) {
		gpioPin_togglePin(red);
		gpioPin_togglePin(green);
		PRINTF("%d %lu\n", i++, pxPreviousWakeTime);
		vTaskDelayUntil(&pxPreviousWakeTime, 1000 / portTICK_PERIOD_MS);
	}
}

OS_DEFINE_TASK(test, 1000);

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

	ret = OS_CREATE_TASK(testTask, "Test Task", 1000, NULL, 1, test);
	CONFIG_ASSERT(ret == pdPASS);
#ifdef CONFIG_CAN_DISABLE
	{
		struct gpio *gpio = gpio_init(GPIO_ID);
		struct gpio_pin *cantxa = gpioPin_init(gpio, GPIO_31, GPIO_OUTPUT, GPIO_PULL_UP);
		CONFIG_ASSERT(cantxa);
		gpioPin_setPin(cantxa);
	}
#endif
#ifdef CONFIG_CAN_TEST
	can_test();
#endif

	PRINTF("Start Scheduler\n");
	vTaskStartScheduler ();
	for(;;);
}

