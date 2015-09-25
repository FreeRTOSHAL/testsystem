#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <gpio.h>
#include <iomux.h>
#include <uart.h>
#include <newlib_stub.h>
#include <buffertest.h>
#include <irq.h>
#include <vfxxx_irqtest.h>
#include <flextimer_test.h>
#include <ppm.h>
#include <pwmtest.h>
#include <rctest.h>

struct gpio *gpioA = NULL;
struct gpio *gpioB = NULL;
struct gpio *gpioC = NULL;
struct gpio *gpioD = NULL;
struct gpio *gpioE = NULL;

struct gpio_pin *pinPTB0 = NULL;
struct gpio_pin *pinPTB2 = NULL;
struct gpio_pin *pinPTA18 = NULL;
struct gpio_pin *pinPTA19 = NULL;

int32_t initGPIO() {
	struct mux *mux = mux_init();
	gpioA = gpio_init(0, mux);
	if (gpioA == NULL) {
		return -1;
	}
	gpioB = gpio_init(1, mux);
	if (gpioB == NULL) {
		return -1;
	}
	gpioC = gpio_init(2, mux);
	if (gpioC == NULL) {
		return -1;
	}
	gpioD = gpio_init(3, mux);
	if (gpioD == NULL) {
		return -1;
	}
	gpioE = gpio_init(4, mux);
	if (gpioE == NULL) {
		return -1;
	}
#ifndef CONFIG_PWM_TEST
	pinPTB0 = gpio_getPin(gpioA, PTB0, GPIO_OUTPUT);
	if (pinPTB0 == NULL) {
		return -1;
	}
	pinPTB2 = gpio_getPin(gpioA, PTB2, GPIO_OUTPUT);
	if (pinPTB2 == NULL) {
		return -1;
	}
#endif
	pinPTA18 = gpio_getPin(gpioA, PTA18, GPIO_OUTPUT);
	if (pinPTA18 == NULL) {
		return -1;
	}
	pinPTA19 = gpio_getPin(gpioA, PTA19, GPIO_OUTPUT);
	if (pinPTA19 == NULL) {
		return -1;
	}
	return 0;
}

void vApplicationMallocFailedHook( void ) {
	CONFIG_ASSERT(0);
}

void vApplicationTickHook() {
/*#ifndef CONFIG_UART_THREAD_SAVE
	static struct uart *uart;
	uart = uart_init(1, 115200);
	CONFIG_ASSERT(uart_puts(uart, "s\n", portMAX_DELAY) == 0);
#endif*/
#ifndef CONFIG_PWM_TEST
#ifndef CONFIG_ASSERT_DISABLED
	CONFIG_ASSERT(gpio_togglePin(pinPTB0) == 0);
#else
	gpio_togglePin(pinPTB0);
#endif
#endif
}

void vApplicationStackOverflowHook() {
	CONFIG_ASSERT(0);
}

void vApplicationIdleHook( void ) {
/*
#ifndef CONFIG_ASSERT_DISABLED
	CONFIG_ASSERT(gpio_togglePin(pinPTA19) == 0);
#else
	gpio_togglePin(pinPTA19);
#endif
*/
}

void testTask(void *data) {
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	for(;;) {
		//printf("Test\n");

#ifndef CONFIG_PWM_TEST
#ifndef CONFIG_ASSERT_DISABLED
		CONFIG_ASSERT(gpio_togglePin(pinPTB2) == 0);
#else
		gpio_togglePin(pinPTB2);
#endif
#endif
		vTaskDelayUntil(&lastWakeUpTime, 1);
	}

}

void testTask2(void *data) {
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	for(;;) {
#ifndef CONFIG_ASSERT_DISABLED
		CONFIG_ASSERT(gpio_togglePin(pinPTA18) == 0);
#else
		gpio_togglePin(pinPTA18);
#endif
		vTaskDelayUntil(&lastWakeUpTime, 1000 / portTICK_PERIOD_MS);
	}
}

void testTask3(void *data) {
	for(;;) {
		/*CONFIG_ASSERT(gpio_togglePin(pinPTA19) == 0);*/
	}
}

int main() {
	int32_t ret;
	ret = irq_init();
	CONFIG_ASSERT(ret == 0);
	
	struct uart *uart = uart_init(6, 115200);
#ifdef CONFIG_NEWLIB
	ret = newlib_init(uart, uart);
	CONFIG_ASSERT(ret == 0);
#endif
	ret = initGPIO();
	CONFIG_ASSERT(ret == 0);
	xTaskCreate( testTask, "Test Task", 512, NULL, 1, NULL);
	xTaskCreate( testTask2, "Test 2 Task", 512, NULL, 1, NULL);
	/*xTaskCreate( testTask3, "Test 3 Task", 128, NULL, 1, NULL);*/
#ifdef CONFIG_BUFFERTEST
	bufferInit();
#endif
#ifdef CONFIG_IRQTEST
	irqtest_init();
#endif
#ifdef CONFIG_VFXXX_FLEXTIMER_TEST
	ftmInit(pinPTA19);
#endif
#ifdef CONFIG_PPM
	{
		struct ppm *ppm = ppm_init(10, pinPTA19);
		ppm_start(ppm);
	}
#endif
#ifdef CONFIG_PWM_TEST
	pwmtest_init();
#endif
#ifdef CONFIG_RCTEST
	rcInit();
#endif
	vTaskStartScheduler ();
	for(;;);
	return 0;
}
