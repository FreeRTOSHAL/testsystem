#include <stdio.h>
#include <stdlib.h>
#include <FreeRTOS.h>
#include <task.h>
#include <gpio.h>
#include <led.h>
#include "iomux.h"

void _exit() {

}
caddr_t _sbrk(int incr) {
	(void) incr;
	return 0;
}

void vApplicationTickHook() {

}

void vApplicationStackOverflowHook() {
	CONFIG_ASSERT(0);
}

void testTask(void *data) {
#if 0
	struct mux *mux = mux_init();
	struct gpio *gpio = gpio_init(1, mux);
	struct gpio_pin *pin = gpio_getPin(gpio, PTB17, GPIO_OUTPUT);
	TickType_t lastWakeUpTime;
	for(;;) {
		gpio_togglePin(pin);
		vTaskDelayUntil(&lastWakeUpTime, 1000 / portTICK_PERIOD_MS);
	}
#else
	struct led *led = led_init(0);
	TickType_t lastWakeUpTime;
	for(;;) {
		led_toggle(led);
		vTaskDelayUntil(&lastWakeUpTime, 1000 / portTICK_PERIOD_MS);
	}

#endif
}

void testTask2(void *data) {
	struct mux *mux = mux_init();
	struct gpio *gpio = gpio_init(1, mux);
	struct gpio_pin *pin = gpio_getPin(gpio, PTB17, GPIO_OUTPUT);
	TickType_t lastWakeUpTime;
	for(;;) {
		gpio_togglePin(pin);
		vTaskDelayUntil(&lastWakeUpTime, 100 / portTICK_PERIOD_MS);
		gpio_togglePin(pin);
		vTaskDelayUntil(&lastWakeUpTime, 100 / portTICK_PERIOD_MS);
		gpio_togglePin(pin);
		vTaskDelayUntil(&lastWakeUpTime, 100 / portTICK_PERIOD_MS);
		gpio_togglePin(pin);
		vTaskDelayUntil(&lastWakeUpTime, 100 / portTICK_PERIOD_MS);
		vTaskDelayUntil(&lastWakeUpTime, (5000 - (100 * 4)) / portTICK_PERIOD_MS);
	}
}

void main() {
	xTaskCreate( testTask, "Test Task", 128, NULL, 1, NULL);
	xTaskCreate( testTask2, "Test 2 Task", 128, NULL, 1, NULL);
	vTaskStartScheduler ();
	for(;;);
}
