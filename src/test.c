#include <stdio.h>
#include <stdlib.h>
#include <FreeRTOS.h>
#include <task.h>
#include <gpio.h>
#include <iomux.h>
#include <uart.h>

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
	struct mux *mux = mux_init();
	struct gpio *gpio = gpio_init(1, mux);
	struct gpio_pin *pin = gpio_getPin(gpio, PTB17, GPIO_OUTPUT);
	struct uart *uart = uart_init(1, 115200);
	TickType_t lastWakeUpTime;
	for(;;) {
		gpio_togglePin(pin);
		CONFIG_ASSERT(uart_puts(uart, "=======\ntest\n=======\n", portMAX_DELAY) == 0);
		vTaskDelayUntil(&lastWakeUpTime, 1000 / portTICK_PERIOD_MS);
	}

}

void testTask2(void *data) {
	struct mux *mux = mux_init();
	struct gpio *gpio = gpio_init(1, mux);
	struct gpio_pin *pin = gpio_getPin(gpio, PTB17, GPIO_OUTPUT);
	TickType_t lastWakeUpTime;
	struct uart *uart = uart_init(1, 115200);
	for(;;) {
		gpio_togglePin(pin);
		vTaskDelayUntil(&lastWakeUpTime, 10 / portTICK_PERIOD_MS);
		gpio_togglePin(pin);
		vTaskDelayUntil(&lastWakeUpTime, 10 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(uart_puts(uart, "=======\n=======\ntest2\n=======\n=======\n", portMAX_DELAY) == 0);
		vTaskDelayUntil(&lastWakeUpTime, 5000 / portTICK_PERIOD_MS);
	}
}

void testTask3(void *data) {
	struct uart *uart = uart_init(1, 115200);
	for(;;) {
		uart_puts(uart, "----\n", 1000 / portTICK_PERIOD_MS);
	}
}

void main() {
	xTaskCreate( testTask, "Test Task", 128, NULL, 1, NULL);
	xTaskCreate( testTask2, "Test 2 Task", 128, NULL, 1, NULL);
	xTaskCreate( testTask3, "Test 3 Task", 128, NULL, 1, NULL);
	vTaskStartScheduler ();
	for(;;);
}
