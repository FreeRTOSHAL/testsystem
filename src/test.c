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
#include <flextimer.h>
#include <spitest.h>
#include <mputest.h>
#include <adctest.h>

static struct gpio *gpio = NULL;

static struct gpio_pin *pinPTB0 = NULL;
static struct gpio_pin *pinPTB2 = NULL;
static struct gpio_pin *pinPTA18 = NULL;
static struct gpio_pin *pinPTA19 = NULL;

static struct gpio_pin *pinPTB17 = NULL;

int32_t initGPIO() {
	gpio = gpio_init();
	if (gpio == NULL) {
		return -1;
	}
#ifndef CONFIG_PWM_TEST
	pinPTB0 = gpio_getPin(gpio, PTB0, GPIO_OUTPUT);
	if (pinPTB0 == NULL) {
		return -1;
	}
	pinPTB2 = gpio_getPin(gpio, PTB2, GPIO_OUTPUT);
	if (pinPTB2 == NULL) {
		return -1;
	}
#endif
	pinPTA18 = gpio_getPin(gpio, PTA18, GPIO_OUTPUT);
	if (pinPTA18 == NULL) {
		return -1;
	}
	pinPTA19 = gpio_getPin(gpio, PTA19, GPIO_OUTPUT);
	if (pinPTA19 == NULL) {
		return -1;
	}
	pinPTB17 = gpio_getPin(gpio, PTB17, GPIO_OUTPUT);
	if (pinPTB17 == NULL) {
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

void ledTask(void *data) {
	int32_t ret;
	bool up = true;
	uint64_t n = 10000;
	struct ftm *ftm = data;
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
		ret = ftm_setPWMDutyCycle(ftm, 1, n);
		CONFIG_ASSERT(ret == 0);
		if (n == 0 || n == 20000) {
			waittime = 1000;
			gpio_togglePin(pinPTB17);
		} else {
			waittime = 20;
		}
		vTaskDelayUntil(&lastWakeUpTime, waittime / portTICK_PERIOD_MS);
	}
}

#if CONFIG_USE_STATS_FORMATTING_FUNCTIONS > 0
void taskManTask(void *data) {
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	static char taskBuff[5 * 1024];
	for(;;) {
		vTaskList(taskBuff);
		printf("name\t\tState\tPrio\tStack\tTaskNr.\n");
		printf("%s", taskBuff);
		printf("blocked ('B'), ready ('R'), deleted ('D') or suspended ('S')\n");
#if CONFIG_GENERATE_RUN_TIME_STATS > 0
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
#ifndef CONFIG_PWM_TEST
	struct ftm *ftm;
#endif
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
#ifndef CONFIG_PWM_TEST
	ftm = ftm_init(3, 32, 20000, 700);
	CONFIG_ASSERT(ftm != NULL);
	ret = ftm_periodic(ftm, 24000);
	CONFIG_ASSERT(ret == 0);
	ret = ftm_setupPWM(ftm, 1);
	CONFIG_ASSERT(ret == 0);
	ret = ftm_setPWMDutyCycle(ftm, 1, 10000);
	CONFIG_ASSERT(ret == 0);
	xTaskCreate(ledTask, "LED Task", 128, ftm, 1, NULL);
#ifdef CONFIG_RCTEST
	rcInit(ftm);
#endif
#endif
#if CONFIG_USE_STATS_FORMATTING_FUNCTIONS > 0
	xTaskCreate(taskManTask, "Task Manager Task", 512, ftm, 1, NULL);
#endif
#ifdef CONFIG_SPITEST
	spitest_init();
#endif
#ifdef CONFIG_MPUTEST
	mputest_init();
#endif
#ifdef CONFIG_ADCTEST
	adctest_init();
#endif
	vTaskStartScheduler ();
	for(;;);
	return 0;
}
