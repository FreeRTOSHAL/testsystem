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
//#include <devs.h>
#include <iomux.h>
#include <uart.h>
#include <adc_test_task.h>
# define PRINTF(...) printf(__VA_ARGS__)

#ifdef CONFIG_GPIO
static struct gpio *gpio = NULL;

static struct gpio_pin *ledPin = NULL;

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
	/* Top LED GPIO2_7 error in Beagbeboard Schmatics GPIO2_17 is GPIO2_7 */
	ledPin = gpioPin_init(gpio, PAD_GPMC_A17, GPIO_OUTPUT, GPIO_PULL_UP);
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
void userErrorHandler() {
	PRINTF("Error");
}

#if defined(CONFIG_GPIO) && defined(CONFIG_INCLUDE_vTaskDelayUntil)
void ledTask(void *data) {
	bool up = true;
	uint64_t n = 10000;
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
		if (n == 0 || n == 20000) {
			waittime = 1000;
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

#ifdef CONFIG_USE_STATS_FORMATTING_FUNCTIONS
OS_DEFINE_TASK(taskMan, 512);
#endif

int main() {
	int32_t ret;
	/*ret = irq_init();
	CONFIG_ASSERT(ret == 0);*/
#if defined(CONFIG_GPIO) && defined(CONFIG_INCLUDE_vTaskDelayUntil)
	struct pwm *pwm = NULL;
#endif
	PRINTF("Init Devices\n");
#ifdef CONFIG_INSTANCE_NAME
	hal_printNames();
#endif
#ifdef CONFIG_GPIO
	ret = initGPIO();
	CONFIG_ASSERT(ret == 0);
#endif
#ifdef CONFIG_USE_STATS_FORMATTING_FUNCTIONS
	OS_CREATE_TASK(taskManTask, "Task Manager Task", 512, NULL, 1, taskMan);
#endif
#ifdef CONFIG_ADC_TEST_TASK
	adc_test();
#endif
	PRINTF("Start Scheduler\n");
	vTaskStartScheduler();
	for(;;);
	return 0;
}
