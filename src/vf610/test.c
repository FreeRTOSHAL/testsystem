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
#include <iomux.h>
#include <uart.h>
#include <newlib_stub.h>
#include <nlibc_stub.h>
#include <buffertest.h>
#include <irq.h>
#include <vfxxx_irqtest.h>
#include <flextimer_test.h>
#include <ppm.h>
#include <pwmtest.h>
#ifdef CONFIG_RCTEST
# include <rctest.h>
#endif
#include <timer.h>
#include <pwm.h>
#include <spitest.h>
#include <mputest.h>
#include <adctest.h>
#include <tpstest.h>
#include <linux_client.h>
#include <speedtest.h>

#ifdef CONFIG_GPIO
static struct gpio *gpio = NULL;

static struct gpio_pin *pinPTB0 = NULL;
static struct gpio_pin *pinPTB2 = NULL;
static struct gpio_pin *pinPTA18 = NULL;
static struct gpio_pin *pinPTA19 = NULL;

static struct gpio_pin *pinPTB17 = NULL;

int32_t initGPIO() {
	struct gpio *gpio2;
	gpio = gpio_init(0);
	if (gpio == NULL) {
		return -1;
	}
	/* Test if GET Dev work */
	gpio2  = gpio_init(1);
	if (gpio2 != NULL) {
		return -1;
	}
#ifndef CONFIG_PWM_TEST
	pinPTB0 = gpioPin_init(gpio, PTB0, GPIO_OUTPUT, GPIO_PULL_UP);
	if (pinPTB0 == NULL) {
		return -1;
	}
	pinPTB2 = gpioPin_init(gpio, PTB2, GPIO_OUTPUT, GPIO_PULL_UP);
	if (pinPTB2 == NULL) {
		return -1;
	}
#endif
	pinPTA18 = gpioPin_init(gpio, PTA18, GPIO_OUTPUT, GPIO_PULL_UP);
	if (pinPTA18 == NULL) {
		return -1;
	}
	pinPTA19 = gpioPin_init(gpio, PTA19, GPIO_OUTPUT, GPIO_PULL_UP);
	if (pinPTA19 == NULL) {
		return -1;
	}
	pinPTB17 = gpioPin_init(gpio, PTB17, GPIO_OUTPUT, GPIO_PULL_UP);
	if (pinPTB17 == NULL) {
		return -1;
	}
	return 0;
}
#endif

void vApplicationMallocFailedHook( void ) {
	CONFIG_ASSERT(0);
}

void vApplicationTickHook() {
/*#ifndef CONFIG_UART_THREAD_SAVE
	static struct uart *uart;
	uart = uart_init(1, 115200);
	CONFIG_ASSERT(uart_puts(uart, "s\n", portMAX_DELAY) == 0);
#endif*/
#ifdef CONFIG_GPIO
# ifndef CONFIG_PWM_TEST
#  ifndef CONFIG_ASSERT_DISABLED
	CONFIG_ASSERT(gpioPin_togglePin(pinPTB0) == 0);
#  else
	gpioPin_togglePin(pinPTB0);
#  endif
# endif
#endif
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName ) {
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

#ifdef CONFIG_GPIO
#  ifndef CONFIG_PWM_TEST
#   ifndef CONFIG_ASSERT_DISABLED
		CONFIG_ASSERT(gpioPin_togglePin(pinPTB2) == 0);
#  else
		gpioPin_togglePin(pinPTB2);
#  endif
# endif
#endif
		vTaskDelayUntil(&lastWakeUpTime, 1);
	}

}

void testTask2(void *data) {
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	for(;;) {
#ifdef CONFIG_GPIO
# ifndef CONFIG_ASSERT_DISABLED
		CONFIG_ASSERT(gpioPin_togglePin(pinPTA18) == 0);
# else
		gpioPin_togglePin(pinPTA18);
# endif
#endif
		vTaskDelayUntil(&lastWakeUpTime, 1000 / portTICK_PERIOD_MS);
	}
}

void testTask3(void *data) {
	for(;;) {
		/*CONFIG_ASSERT(gpio_togglePin(pinPTA19) == 0);*/
	}
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
			gpioPin_togglePin(pinPTB17);
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

#ifdef CONFIG_UARTTEST
void uart_test1(void *data) {
	struct uart *uart = uart_init(0, 115200);
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	for(;;) {
		uart_puts(uart, "Test\n", 0);
		vTaskDelayUntil(&lastWakeUpTime, 1000 / portTICK_PERIOD_MS);
	}
}
void uart_test2(void *data) {
	struct uart *uart = uart_init(1, 115200);
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	for(;;) {
		uart_puts(uart, "Test\n", 0);
		vTaskDelayUntil(&lastWakeUpTime, 1000 / portTICK_PERIOD_MS);
	}
}
OS_DEFINE_TASK(uartTask1, 512);
OS_DEFINE_TASK(uartTask2, 512);
void uarttest_init() {
	OS_CREATE_TASK(uart_test1, "UART Test Task 1", 512, NULL, 2, uartTask1);
	OS_CREATE_TASK(uart_test2, "UART Test Task 2", 512, NULL, 2, uartTask2);
}
#endif

OS_DEFINE_TASK(taskLED, 128);
OS_DEFINE_TASK(taskMan, 512);
int main() {
	int32_t ret;
	ret = irq_init();
	CONFIG_ASSERT(ret == 0);
#if !defined(CONFIG_PWM_TEST) && defined(CONFIG_FLEXTIMER)
	struct timer *ftm;
#endif
	struct pwm *pwm = NULL;
#ifdef CONFIG_UART
	struct uart *uart = uart_init(1, 115200);
#endif
#ifdef CONFIG_NEWLIB
	ret = newlib_init(uart, uart);
	CONFIG_ASSERT(ret == 0);
#endif
#ifdef CONFIG_NLIBC_PRINTF
	ret = nlibc_init(uart, uart);
	CONFIG_ASSERT(ret == 0);
#endif
#ifdef CONFIG_INSTANCE_NAME
	hal_printNames();
#endif
	printf("Init Devices\n");
#ifdef CONFIG_GPIO
	ret = initGPIO();
	CONFIG_ASSERT(ret == 0);
#endif
	/*xTaskCreate( testTask, "Test Task", 512, NULL, 1, NULL);
	xTaskCreate( testTask2, "Test 2 Task", 512, NULL, 1, NULL);*/
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
#if !defined(CONFIG_PWM_TEST) && defined(CONFIG_FLEXTIMER)
	ftm = timer_init(1, 32, 20000, 700);
	CONFIG_ASSERT(ftm != NULL);
	pwm = pwm_init(3); 
	CONFIG_ASSERT(pwm != NULL);
	ret = pwm_setPeriod(pwm, 24000);
	CONFIG_ASSERT(ret == 0);
	ret = pwm_setDutyCycle(pwm, 10000);
	CONFIG_ASSERT(ret == 0);
#ifdef CONFIG_RCTEST
	rcInit(ftm);
#endif
#endif
#if defined(CONFIG_GPIO) || defined(CONFIG_PWM)
	OS_CREATE_TASK(ledTask, "LED Task", 128, pwm, 1, taskLED);
#endif
#ifdef CONFIG_USE_STATS_FORMATTING_FUNCTIONS
	OS_CREATE_TASK(taskManTask, "Task Manager Task", 512, NULL, 1, taskMan);
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
#ifdef CONFIG_TPSTEST
	tpstest_init();
#endif
#ifdef CONFIG_UARTTEST
	uarttest_init();
#endif
#ifdef CONFIG_LINUX_CON
	{
		struct lc *lc = lc_init();
	}
#endif
#ifdef CONFIG_SPEEDTEST
	speedtest_init();
#endif
	printf("Start Scheduler\n");
	vTaskStartScheduler ();
	for(;;);
	return 0;
}
