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
#include <timer.h>
#include <pwm.h>
#include <timertest.h>
#include <sdtest.h>
#include <semihosting.h>
#include <boardtest.h>
#include <spitest.h>
#if defined(CONFIG_NEWLIB) || defined(CONFIG_NLIBC_PRINTF)
# define PRINTF(...) printf(__VA_ARGS__)
#else
# define PRINTF(...) 
#endif

#ifdef CONFIG_GPIO
static struct gpio *gpio = NULL;

static struct gpio_pin *ledPin = NULL;
static struct gpio_pin *userButton = NULL;
bool nucleo_userButtonISR(struct gpio_pin *pin, uint32_t pinID, void *data) {
	(void) pin;
	(void) pinID;
	(void) data;
	PRINTF("User Button pressed\n");
	return false;
}

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
#if (defined(CONFIG_NUCLEO) || defined(CONFIG_PHOTON))
	userButton = gpioPin_init(gpio, PTC13, GPIO_INPUT, GPIO_OPEN);
#endif
#ifdef CONFIG_DISCOVERY
	userButton = gpioPin_init(gpio, PTA0, GPIO_INPUT, GPIO_OPEN);
#endif
	if (userButton == NULL) {
		return -1;
	}
	{
		int32_t ret;
		ret = gpioPin_setCallback(userButton, nucleo_userButtonISR, NULL, GPIO_EITHER);
		if (ret < 0) {
			return -1;
		}
		ret = gpioPin_enableInterrupt(userButton);
		if (ret < 0) {
			return -1;
		}
	}
#if (defined(CONFIG_NUCLEO) || defined(CONFIG_PHOTON))
	ledPin = gpioPin_init(gpio, PTA5, GPIO_OUTPUT, GPIO_PULL_UP);
#endif
#ifdef CONFIG_DISCOVERY
	ledPin = gpioPin_init(gpio, PTD14, GPIO_OUTPUT, GPIO_PULL_UP);
#endif
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

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName ) {
	CONFIG_ASSERT(0);
}

void vApplicationIdleHook( void ) {
}

#if (defined(CONFIG_GPIO) || defined(CONFIG_PWM)) && defined(CONFIG_INCLUDE_vTaskDelayUntil) && !defined(CONFIG_TIMER_TEST)
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
			gpioPin_togglePin(ledPin);
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
#if (defined(CONFIG_GPIO) || defined(CONFIG_PWM)) && defined(CONFIG_INCLUDE_xTaskDelayUntil) && !defined(CONFIG_TIMER_TEST)
	struct pwm *pwm = NULL;
#endif
#ifdef CONFIG_UART
	#ifdef CONFIG_STM32_STDOUT_UART1
	struct uart *uart = uart_init(UART1_ID, 115200);
	#endif
	#ifdef CONFIG_STM32_STDOUT_UART2
	struct uart *uart = uart_init(UART2_ID, 115200);
	#endif
	#ifdef CONFIG_STM32_STDOUT_SEMIHOSTING
	struct uart *uart = uart_init(SEMIHOSTING_UART_ID, 115200);
	#endif
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
#ifdef CONFIG_GPIO
	ret = initGPIO();
	CONFIG_ASSERT(ret == 0);
#endif
#if defined(CONFIG_PWM) && !defined(CONFIG_TIMER_TEST)
	{
		struct timer *timer = timer_init(TIMER4_ID, 500, 1, 0);
		CONFIG_ASSERT(timer);
		pwm = pwm_init(PWM4_4_ID);
		CONFIG_ASSERT(pwm);
		ret = pwm_setPeriod(pwm, 20000);
		CONFIG_ASSERT(ret == 0);
		ret = pwm_setDutyCycle(pwm, 10000);
		CONFIG_ASSERT(ret == 0);
	}
#endif
#if (defined(CONFIG_GPIO) || defined(CONFIG_PWM)) && defined(CONFIG_INCLUDE_vTaskDelayUntil) && !defined(CONFIG_TIMER_TEST)
	OS_CREATE_TASK(ledTask, "LED Task", 128, pwm, 1, taskLED);
#endif
#ifdef CONFIG_USE_STATS_FORMATTING_FUNCTIONS
	OS_CREATE_TASK(taskManTask, "Task Manager Task", 512, NULL, 1, taskMan);
#endif
	PRINTF("Start Scheduler\n");
	vTaskStartScheduler ();
	for(;;);
	return 0;
}
